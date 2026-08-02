/// OPT Package Installer — 跨平台安装器
///
/// 支持任意 UNIX 系统（Linux/macOS/ChromiumOS/BSD）。
/// 默认安装到 ~/.local/opt/，无需 root 权限。
/// 自动生成 ELF 启动脚本，处理动态库依赖路径。
///
/// 路径约定:
///   $prefix/opt/<pkg>/           ← 应用文件
///   $prefix/bin/<pkg>            ← 启动脚本
///   $prefix/share/applications/  ← .desktop 文件
///   $prefix/var/lib/opt/         ← 包数据库
///   $prefix/var/log/opt/         ← 安装日志

use crate::error::{OptError, OptResult};
use crate::format::{
    Checksums, PackageManifest,
    OPT_MANIFEST, OPT_INNER_DATA_ZIP, OPT_CHECKSUMS, OPT_REQUIRED_ENTRIES,
};
use sha2::{Digest, Sha256};
use std::fs;
use std::io::{Read, Write};
use std::path::{Path, PathBuf};

// ──────────────────────────────────────────────
// 前缀检测
// ──────────────────────────────────────────────

/// 检测安装前缀: OPT_PREFIX 环境变量 > 默认 ~/.local
fn detect_prefix() -> PathBuf {
    if let Ok(val) = std::env::var("OPT_PREFIX") {
        if !val.is_empty() {
            return PathBuf::from(val);
        }
    }
    dirs::home_dir()
        .unwrap_or_else(|| PathBuf::from("~"))
        .join(".local")
}

/// 获取前缀下的路径
fn prefix_dir(prefix: &Path, sub: &str) -> PathBuf {
    prefix.join(sub)
}

fn opt_dir(prefix: &Path, name: &str) -> PathBuf {
    prefix_dir(prefix, "opt").join(name)
}

fn bin_dir(prefix: &Path) -> PathBuf {
    prefix_dir(prefix, "bin")
}

fn db_dir(prefix: &Path) -> PathBuf {
    prefix_dir(prefix, "var/lib/opt/packages")
}

fn log_dir(prefix: &Path) -> PathBuf {
    prefix_dir(prefix, "var/log/opt")
}

fn installed_db_path(prefix: &Path, name: &str) -> PathBuf {
    db_dir(prefix).join(format!("{name}.json"))
}

fn file_manifest_path(prefix: &Path, name: &str) -> PathBuf {
    db_dir(prefix).join(format!("{name}-files.txt"))
}

// ──────────────────────────────────────────────
// 包验证
// ──────────────────────────────────────────────

pub fn validate_package(path: &str) -> OptResult<PackageManifest> {
    let opt_path = Path::new(path);
    if !opt_path.exists() {
        return Err(OptError::NotFound(format!("文件不存在：{path}")));
    }

    let file = fs::File::open(opt_path)?;
    let mut archive = zip::ZipArchive::new(file)?;

    for entry_name in OPT_REQUIRED_ENTRIES {
        if archive.by_name(entry_name).is_err() {
            return Err(OptError::InvalidPackage(format!(
                "缺少必要条目: {entry_name}"
            )));
        }
    }

    let manifest_bytes = {
        let mut entry = archive.by_name(OPT_MANIFEST)?;
        let mut buf = Vec::new();
        entry.read_to_end(&mut buf)?;
        buf
    };
    let manifest: PackageManifest = serde_json::from_slice(&manifest_bytes)?;

    if let Err(errors) = manifest.validate() {
        return Err(OptError::Validation(format!(
            "Manifest validation failed: {}",
            errors.join("; ")
        )));
    }

    let checksum_bytes = {
        let mut entry = archive.by_name(OPT_CHECKSUMS)?;
        let mut buf = Vec::new();
        entry.read_to_end(&mut buf)?;
        buf
    };
    let checksums =
        Checksums::from_bytes(&checksum_bytes).map_err(|e| OptError::InvalidPackage(e))?;

    let entry_names: Vec<String> = archive.file_names().map(|s| s.to_string()).collect();

    for entry_name in &entry_names {
        if entry_name == OPT_CHECKSUMS { continue; }
        let actual_hash = {
            let mut entry = archive.by_name(entry_name)?;
            let mut hasher = Sha256::new();
            std::io::copy(&mut entry, &mut hasher)?;
            hex::encode(hasher.finalize())
        };
        if !checksums.verify(entry_name, &actual_hash) {
            let expected = checksums.entries.get(entry_name).cloned().unwrap_or_default();
            return Err(OptError::ChecksumMismatch {
                file: entry_name.to_string(), expected, actual: actual_hash,
            });
        }
    }

    Ok(manifest)
}

// ──────────────────────────────────────────────
// 安装（默认前缀）
// ──────────────────────────────────────────────

pub fn install_package(path: &str, yes: bool, no_deps: bool) -> OptResult<()> {
    let prefix = detect_prefix();
    install_package_to(path, &prefix, yes, no_deps)
}

// ──────────────────────────────────────────────
// 安装（指定前缀）
// ──────────────────────────────────────────────

pub fn install_package_to(path: &str, prefix: &Path, yes: bool, no_deps: bool) -> OptResult<()> {
    let opt_path = Path::new(path);
    if !opt_path.exists() {
        return Err(OptError::NotFound(format!("文件不存在: {path}")));
    }

    let manifest = validate_package(path)?;
    let name = &manifest.name;

    // 检查是否已安装
    let db_path = installed_db_path(prefix, name);
    if db_path.exists() {
        let installed = read_installed_manifest(prefix, name)?;
        return Err(OptError::AlreadyInstalled {
            name: name.clone(),
            version: installed.version,
        });
    }

    // 依赖检查
    if !no_deps {
        check_dependencies(prefix, &manifest)?;
    }

    // 确认
    if !yes {
        eprintln!("包名:     {}", manifest.name);
        eprintln!("版本:     {}", manifest.version);
        eprintln!("大小:     {} (安装后: {})",
            format_size(manifest.package_size),
            format_size(manifest.installed_size));
        eprintln!("运行时:   {:?}", manifest.runtime);
        eprintln!("安装到:   {}", opt_dir(prefix, name).display());
        if !manifest.depends.is_empty() {
            eprintln!("依赖:     {}", manifest.depends.join(", "));
        }
        eprint!("安装? [Y/n] ");
        std::io::stdout().flush().ok();
        let mut input = String::new();
        std::io::stdin().read_line(&mut input).ok();
        if input.trim().to_lowercase() == "n" {
            eprintln!("已取消。");
            return Ok(());
        }
    }

    let file = fs::File::open(opt_path)?;
    let mut archive = zip::ZipArchive::new(file)?;

    // 创建目录
    let app_dir = opt_dir(prefix, name);
    let _bin_dir = bin_dir(prefix);
    fs::create_dir_all(&app_dir)?;
    fs::create_dir_all(db_dir(prefix))?;
    fs::create_dir_all(log_dir(prefix))?;

    let mut installed_files: Vec<String> = Vec::new();

    // 提取 data.zip 到 $prefix/opt/<name>/
    if let Ok(mut data_entry) = archive.by_name(OPT_INNER_DATA_ZIP) {
        let data_zip_bytes = read_all(&mut data_entry)?;
        let data_cursor = std::io::Cursor::new(data_zip_bytes);
        let mut inner_archive = zip::ZipArchive::new(data_cursor)?;

        for i in 0..inner_archive.len() {
            let mut entry = inner_archive.by_index(i)?;
            let name_in_zip = entry.name().to_string();
            if name_in_zip.ends_with('/') { continue; }

            let target = app_dir.join(&name_in_zip);
            if let Some(parent) = target.parent() {
                fs::create_dir_all(parent)?;
            }
            let mut out_file = fs::File::create(&target)?;
            std::io::copy(&mut entry, &mut out_file)?;

            // 保留可执行权限
            #[cfg(unix)]
            if let Some(mode) = entry.unix_mode() {
                use std::os::unix::fs::PermissionsExt;
                let _ = fs::set_permissions(&target, fs::Permissions::from_mode(mode));
            }

            installed_files.push(name_in_zip);
        }
    }

    // 生成启动脚本
    generate_launcher(prefix, &manifest, &app_dir)?;

    // 复制 .desktop 文件到标准位置
    let desktop_src = app_dir.join("usr/share/applications").join(&manifest.desktop_file);
    if desktop_src.exists() {
        let desktop_dst = prefix_dir(prefix, "share/applications").join(&manifest.desktop_file);
        if let Some(parent) = desktop_dst.parent() {
            fs::create_dir_all(parent)?;
        }
        fs::copy(&desktop_src, &desktop_dst)?;
        installed_files.push(format!("share/applications/{}", manifest.desktop_file));
    }

    // 保存包数据库
    let mut manifest_db = manifest.clone();
    manifest_db.package_size = opt_path.metadata()?.len();
    fs::write(&db_path, serde_json::to_string_pretty(&manifest_db)?)?;
    fs::write(&file_manifest_path(prefix, name), installed_files.join("\n"))?;

    // 日志
    let log_line = format!(
        "[{}] INSTALL {} {} (from {}) prefix={}\n",
        chrono::Local::now().format("%Y-%m-%d %H:%M:%S"),
        name, manifest.version, path, prefix.display()
    );
    let log_path = log_dir(prefix).join(format!("{name}.log"));
    fs::OpenOptions::new().create(true).append(true).open(&log_path)?
        .write_all(log_line.as_bytes())?;

    eprintln!("✅ 已安装: {} v{}", name, manifest.version);
    eprintln!("   路径: {}", app_dir.display());
    eprintln!("   启动: {}",
        bin_dir(prefix).join(name).display());

    Ok(())
}

// ──────────────────────────────────────────────
// 生成启动脚本
// ──────────────────────────────────────────────

/// 为应用生成 shell 启动脚本。
/// 自动设置 LD_LIBRARY_PATH 以便程序能找到自带的 .so 库。
fn generate_launcher(prefix: &Path, manifest: &PackageManifest, app_dir: &Path) -> OptResult<()> {
    let launcher_path = bin_dir(prefix).join(&manifest.name);
    fs::create_dir_all(bin_dir(prefix))?;

    // 查找主二进制：优先 usr/bin/ 下同名的，否则第一个可执行文件
    let binary = find_main_binary(app_dir, &manifest.name);

    let launcher = format!(
        r#"#!/bin/sh
# OPT Launcher: {name} v{version}
# 自动生成 — 请勿手动编辑

APP_DIR="{app_dir}"
export LD_LIBRARY_PATH="$APP_DIR/usr/lib:$APP_DIR/usr/lib/x86_64-linux-gnu:$APP_DIR/lib:$APP_DIR/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH"
export PATH="$APP_DIR/usr/bin:$APP_DIR/bin:$PATH"

{binary} "$@"
"#,
        name = manifest.name,
        version = manifest.version,
        app_dir = app_dir.display(),
        binary = binary,
    );

    fs::write(&launcher_path, launcher)?;

    // 设置可执行权限
    #[cfg(unix)]
    {
        use std::os::unix::fs::PermissionsExt;
        fs::set_permissions(&launcher_path, fs::Permissions::from_mode(0o755))?;
    }

    Ok(())
}

/// 在 app_dir 中查找主二进制文件
fn find_main_binary(app_dir: &Path, pkg_name: &str) -> String {
    // 优先找同名文件（多种路径）
    let candidates = [
        format!("usr/bin/{pkg_name}"),
        format!("usr/sbin/{pkg_name}"),
        format!("bin/{pkg_name}"),
        format!("sbin/{pkg_name}"),
        format!("opt/{pkg_name}/{pkg_name}"),
        format!("opt/{pkg_name}/bin/{pkg_name}"),
        format!("{pkg_name}"),
        format!("bin/{pkg_name}"),
    ];

    for c in &candidates {
        if app_dir.join(c).exists() {
            return format!("$APP_DIR/{c}");
        }
    }

    // fallback: 扫描 usr/bin 下第一个可执行文件
    for subdir in &["usr/bin", "usr/sbin", "bin", "sbin"] {
        let dir = app_dir.join(subdir);
        if let Ok(entries) = fs::read_dir(&dir) {
            for entry in entries.flatten() {
                let path = entry.path();
                if path.is_file() {
                    if let Ok(meta) = path.metadata() {
                        #[cfg(unix)]
                        {
                            use std::os::unix::fs::PermissionsExt;
                            if meta.permissions().mode() & 0o111 != 0 {
                                return format!("$APP_DIR/{subdir}/{}",
                                    path.file_name().unwrap().to_string_lossy());
                            }
                        }
                        #[cfg(not(unix))]
                        {
                            return format!("$APP_DIR/{subdir}/{}",
                                path.file_name().unwrap().to_string_lossy());
                        }
                    }
                }
            }
        }
    }

    // 最后保底
    format!("$APP_DIR/usr/bin/{pkg_name}")
}

// ──────────────────────────────────────────────
// 移除
// ──────────────────────────────────────────────

pub fn remove_package(name: &str, purge: bool, yes: bool) -> OptResult<()> {
    let prefix = detect_prefix();
    remove_package_from(name, &prefix, purge, yes)
}

pub fn remove_package_from(name: &str, prefix: &Path, purge: bool, yes: bool) -> OptResult<()> {
    let db_path = installed_db_path(prefix, name);
    if !db_path.exists() {
        return Err(OptError::NotFound(format!("包 '{name}' 未安装")));
    }

    let manifest: PackageManifest = {
        let content = fs::read_to_string(&db_path)?;
        serde_json::from_str(&content)?
    };

    if !yes {
        eprint!("移除 {} v{}? [y/N] ", manifest.name, manifest.version);
        std::io::stdout().flush().ok();
        let mut input = String::new();
        std::io::stdin().read_line(&mut input).ok();
        if input.trim().to_lowercase() != "y" {
            eprintln!("已取消。");
            return Ok(());
        }
    }

    // 删除应用目录
    let app_dir = opt_dir(prefix, name);
    if app_dir.exists() {
        fs::remove_dir_all(&app_dir)?;
    }

    // 删除启动脚本
    let launcher = bin_dir(prefix).join(name);
    if launcher.exists() {
        fs::remove_file(&launcher)?;
    }

    // 删除 .desktop 文件
    let desktop = prefix_dir(prefix, "share/applications").join(&manifest.desktop_file);
    if desktop.exists() {
        fs::remove_file(&desktop)?;
    }

    // 清理 DB
    fs::remove_file(&db_path)?;
    let fm_path = file_manifest_path(prefix, name);
    if fm_path.exists() {
        fs::remove_file(&fm_path)?;
    }

    if purge {
        // 删除空父目录
        let _ = fs::remove_dir(&app_dir);
    }

    eprintln!("✅ 已移除: {} v{}", manifest.name, manifest.version);
    Ok(())
}

// ──────────────────────────────────────────────
// 查询
// ──────────────────────────────────────────────

pub fn list_installed() -> OptResult<Vec<PackageManifest>> {
    let prefix = detect_prefix();
    let db = db_dir(&prefix);
    if !db.exists() { return Ok(vec![]); }

    let mut packages = Vec::new();
    for entry in fs::read_dir(db)? {
        let entry = entry?;
        let path = entry.path();
        if path.is_file() && path.extension().map(|e| e == "json").unwrap_or(false) {
            if path.file_stem().and_then(|s| s.to_str()).map(|s| s.ends_with("-files")).unwrap_or(false) {
                continue;
            }
            if let Ok(content) = fs::read_to_string(&path) {
                if let Ok(m) = serde_json::from_str::<PackageManifest>(&content) {
                    packages.push(m);
                }
            }
        }
    }
    packages.sort_by(|a, b| a.name.cmp(&b.name));
    Ok(packages)
}

pub fn show_package(name: &str) -> OptResult<PackageManifest> {
    let prefix = detect_prefix();
    let db_path = installed_db_path(&prefix, name);
    if !db_path.exists() {
        return Err(OptError::NotFound(format!("包 '{name}' 未找到")));
    }
    let content = fs::read_to_string(&db_path)?;
    Ok(serde_json::from_str(&content)?)
}

// ──────────────────────────────────────────────
// 内部工具
// ──────────────────────────────────────────────

fn read_installed_manifest(prefix: &Path, name: &str) -> OptResult<PackageManifest> {
    let path = installed_db_path(prefix, name);
    let content = fs::read_to_string(&path)?;
    Ok(serde_json::from_str(&content)?)
}

fn read_all<R: std::io::Read>(reader: &mut R) -> OptResult<Vec<u8>> {
    let mut buf = Vec::new();
    reader.read_to_end(&mut buf)?;
    Ok(buf)
}

fn check_dependencies(prefix: &Path, manifest: &PackageManifest) -> OptResult<()> {
    for dep in &manifest.depends {
        let dep_name = dep.split_whitespace().next().unwrap_or(dep)
            .trim_end_matches(|c: char| c == ':' || c == '|');
        let db_path = installed_db_path(prefix, dep_name);
        if !db_path.exists() {
            return Err(OptError::UnsatisfiedDependency(format!(
                "包 '{}' 需要依赖 '{}'，尚未安装", manifest.name, dep_name
            )));
        }
    }
    for conflict in &manifest.conflicts {
        let db_path = installed_db_path(prefix, conflict);
        if db_path.exists() {
            return Err(OptError::PackageConflict(format!(
                "包 '{}' 与已安装的 '{}' 冲突", manifest.name, conflict
            )));
        }
    }
    Ok(())
}

fn format_size(bytes: u64) -> String {
    const UNITS: &[&str] = &["B", "KB", "MB", "GB"];
    let mut size = bytes as f64;
    let mut unit_idx = 0;
    while size >= 1024.0 && unit_idx < UNITS.len() - 1 {
        size /= 1024.0; unit_idx += 1;
    }
    format!("{:.2} {}", size, UNITS[unit_idx])
}
