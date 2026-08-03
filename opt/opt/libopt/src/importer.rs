/// .deb 包导入器 — 将 Debian 包转换为 .opt 格式
///
/// 流程:
///   1. 解析 .deb (ar 归档) → 找到 data.tar.*
///   2. 提取文件到临时目录
///   3. 扫描 ELF 二进制 + 动态库依赖
///   4. 生成 opt.json 元数据
///   5. 构建 .opt 包

use crate::builder;
use crate::error::{OptError, OptResult};
use crate::format::PackageManifest;
use std::fs;
use std::io::Read;
use std::path::{Path, PathBuf};

// ──────────────────────────────────────────────
// ar 归档解析器（.deb 文件格式）
// ──────────────────────────────────────────────

const AR_MAGIC: &[u8; 8] = b"!<arch>\n";

struct ArEntry {
    name: String,
    size: usize,
}

/// 解析 ar 归档，返回文件条目列表
fn parse_ar_archive(data: &[u8]) -> OptResult<Vec<ArEntry>> {
    if data.len() < 8 || &data[..8] != AR_MAGIC {
        return Err(OptError::InvalidPackage("不是有效的 ar 归档".into()));
    }

    let mut entries = Vec::new();
    let mut offset = 8; // 跳过魔数

    while offset + 60 <= data.len() {
        let header = &data[offset..offset + 60];

        // 解析文件名（去掉尾部空格和 '/'）
        let name_bytes = &header[..16];
        let name_len = name_bytes.iter().position(|&b| b == b' ' || b == b'/')
            .unwrap_or(16);
        let name = String::from_utf8_lossy(&name_bytes[..name_len]).to_string();

        // 解析文件大小（第 48-57 字节）
        let size_str = std::str::from_utf8(&header[48..58]).unwrap_or("0").trim();
        let size: usize = size_str.parse().unwrap_or(0);

        // 跳过头部
        offset += 60;

        // 特殊条目
        if name.starts_with("/") {
            // 符号表 / 长名表，跳过
            offset += size;
            if offset % 2 != 0 { offset += 1; }
            continue;
        }

        entries.push(ArEntry { name, size });

        // 跳到下一个条目（对齐到偶数）
        offset += size;
        if offset % 2 != 0 { offset += 1; }
    }

    Ok(entries)
}

/// 从 ar 归档中提取指定条目的数据
fn extract_ar_entry(data: &[u8], entry_name: &str) -> OptResult<Vec<u8>> {
    let entries = parse_ar_archive(data)?;
    let mut offset = 8;

    for entry in &entries {
        offset += 60; // 跳过头部

        if entry.name == entry_name {
            let end = offset + entry.size;
            if end > data.len() {
                return Err(OptError::InvalidPackage(format!(
                    "条目 '{}' 数据不完整", entry_name
                )));
            }
            return Ok(data[offset..end].to_vec());
        }

        offset += entry.size;
        if offset % 2 != 0 { offset += 1; }
    }

    Err(OptError::NotFound(format!(
        "在 ar 归档中未找到 '{}'", entry_name
    )))
}

/// 检测压缩类型并解压
fn decompress_data(data: &[u8], filename: &str) -> OptResult<Vec<u8>> {
    if filename.ends_with(".gz") {
        let mut decoder = flate2::read::GzDecoder::new(data);
        let mut buf = Vec::new();
        decoder.read_to_end(&mut buf)?;
        Ok(buf)
    } else if filename.ends_with(".xz") || filename.ends_with(".lzma") {
        let mut decoder = xz2::read::XzDecoder::new(data);
        let mut buf = Vec::new();
        decoder.read_to_end(&mut buf)?;
        Ok(buf)
    } else if filename.ends_with(".zst") {
        return Err(OptError::General(
            "zst 压缩暂不支持，请使用 gz 格式的 .deb 包".into()
        ));
    } else {
        // 未压缩
        Ok(data.to_vec())
    }
}

/// 提取 tar 归档到目录
fn extract_tar(data: &[u8], output_dir: &Path) -> OptResult<()> {
    let mut archive = tar::Archive::new(data);
    fs::create_dir_all(output_dir)?;

    for entry in archive.entries()? {
        let mut entry = entry?;
        let path = entry.path()?.to_path_buf();

        // 跳过绝对路径 / ./ 前缀
        let relative = path.strip_prefix("/")
            .unwrap_or(&path)
            .strip_prefix("./")
            .unwrap_or(&path)
            .to_path_buf();

        if relative.as_os_str().is_empty() {
            continue;
        }

        let target = output_dir.join(&relative);
        if let Some(parent) = target.parent() {
            fs::create_dir_all(parent)?;
        }

        entry.unpack(&target)?;
    }

    Ok(())
}

/// 扫描可执行文件
fn find_binaries(dir: &Path) -> Vec<PathBuf> {
    let mut bins = vec![];
    let search_dirs = ["usr/bin", "usr/sbin", "usr/games", "bin", "sbin", "opt"];

    for subdir in &search_dirs {
        let path = dir.join(subdir);
        if !path.exists() { continue; }
        if let Ok(entries) = fs::read_dir(&path) {
            for entry in entries.flatten() {
                let p = entry.path();
                if !p.is_file() {
                    // opt/ 下可能还有子目录，递归扫描
                    if *subdir == "opt" && p.is_dir() {
                        if let Ok(sub) = fs::read_dir(&p) {
                            for s in sub.flatten() {
                                let sp = s.path();
                                if sp.is_file() && is_executable(&sp) { bins.push(sp); }
                            }
                        }
                    }
                    continue;
                }
                if is_executable(&p) { bins.push(p); }
            }
        }
    }
    bins
}

#[cfg(unix)]
fn is_executable(path: &Path) -> bool {
    use std::os::unix::fs::PermissionsExt;
    fs::metadata(path).map(|m| m.permissions().mode() & 0o111 != 0).unwrap_or(false)
}
#[cfg(not(unix))]
fn is_executable(_path: &Path) -> bool { true }

// ──────────────────────────────────────────────
// 主导入函数
// ──────────────────────────────────────────────

/// 从任意 Linux 应用导入并构建 .opt 包
///
/// 支持输入类型:
///   - .deb 文件 → 解析 ar 归档提取
///   - 目录     → 直接作为 .opt data 目录
///   - .AppImage → 提取并打包
///   - ELF 二进制 → 自动创建项目结构
pub fn import(source: &str, output_dir: Option<&Path>, force: bool) -> OptResult<PathBuf> {
    let src_path = Path::new(source);
    if !src_path.exists() {
        return Err(OptError::NotFound(format!("文件不存在: {source}")));
    }

    // 根据输入类型选择导入方式
    if source.ends_with(".deb") {
        import_deb(source, output_dir, force)
    } else if source.ends_with(".AppImage") || source.ends_with(".appimage") {
        import_appimage(source, output_dir, force)
    } else if src_path.is_dir() {
        import_directory(source, output_dir, force)
    } else {
        // 当作 ELF 二进制处理
        import_binary(source, output_dir, force)
    }
}

/// 从 .deb 包导入
pub fn import_deb(source: &str, output_dir: Option<&Path>, force: bool) -> OptResult<PathBuf> {
    let deb_path = Path::new(source);
    if !deb_path.exists() {
        return Err(OptError::NotFound(format!("文件不存在: {source}")));
    }

    // 1. 读取 .deb 文件
    let deb_data = fs::read(deb_path)?;
    eprintln!("📦 正在解析 .deb 包...");

    // 2. 找到 data.tar.* 条目
    let entries = parse_ar_archive(&deb_data)?;
    let data_entry = entries.iter()
        .find(|e| e.name.starts_with("data.tar"))
        .ok_or_else(|| OptError::InvalidPackage(
            "未找到 data.tar.* 条目".into()
        ))?;

    eprintln!("  → 发现数据归档: {} ({} bytes)", data_entry.name, data_entry.size);

    // 3. 提取 data.tar 数据
    let compressed_data = extract_ar_entry(&deb_data, &data_entry.name)?;
    let tar_data = decompress_data(&compressed_data, &data_entry.name)?;

    // 4. 创建临时目录并提取
    let tmp_dir = std::env::temp_dir().join(format!("opt-import-{}", std::process::id()));
    if tmp_dir.exists() {
        fs::remove_dir_all(&tmp_dir).ok();
    }
    fs::create_dir_all(&tmp_dir)?;

    eprintln!("  → 正在提取文件...");
    extract_tar(&tar_data, &tmp_dir)?;

    // 5. 从 debian-binary 和 control.tar 获取元数据
    let mut pkg_name = deb_path.file_stem()
        .and_then(|s| s.to_str())
        .unwrap_or("imported")
        .to_string();
    let mut pkg_version = "0.1.0".to_string();
    let mut pkg_desc = String::new();
    let mut pkg_section = "utils".to_string();
    let mut pkg_maintainer = "Imported <import@openos.org>".to_string();
    let mut pkg_arch = "amd64".to_string();

    // 尝试从 control.tar 读取包信息
    if let Some(control_entry) = entries.iter().find(|e| e.name.starts_with("control.tar")) {
        if let Ok(ctrl_compressed) = extract_ar_entry(&deb_data, &control_entry.name) {
            if let Ok(ctrl_tar) = decompress_data(&ctrl_compressed, &control_entry.name) {
                let ctrl_dir = tmp_dir.join(".control");
                fs::create_dir_all(&ctrl_dir).ok();
                if let Ok(mut archive) = tar::Archive::new(ctrl_tar.as_slice()).entries() {
                    while let Some(Ok(mut entry)) = archive.next() {
                        let name = entry.path().ok()
                            .and_then(|p| p.file_name().map(|s| s.to_string_lossy().to_string()))
                            .unwrap_or_default();
                        if name == "control" {
                            let mut content = String::new();
                            entry.read_to_string(&mut content).ok();
                            for line in content.lines() {
                                if let Some((key, val)) = line.split_once(':') {
                                    let val = val.trim();
                                    match key.trim() {
                                        "Package" => pkg_name = val.to_string(),
                                        "Version" => pkg_version = val.to_string(),
                                        "Description" => {
                                            pkg_desc = val.to_string();
                                        }
                                        "Section" => pkg_section = val.to_string(),
                                        "Maintainer" => pkg_maintainer = val.to_string(),
                                        "Architecture" => pkg_arch = val.to_string(),
                                        _ => {}
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // 截取描述的第一行
    let pkg_desc = pkg_desc.lines().next().unwrap_or(&pkg_desc).to_string();

    eprintln!("  → 包名: {pkg_name}, 版本: {pkg_version}, 架构: {pkg_arch}");

    // 6. 扫描可执行文件
    let binaries = find_binaries(&tmp_dir);
    if binaries.is_empty() {
        eprintln!("  ⚠ 未找到可执行文件，可能不是应用程序包");
    } else {
        eprintln!("  → 发现 {} 个可执行文件", binaries.len());
        for b in &binaries {
            eprintln!("    - {}", b.strip_prefix(&tmp_dir).unwrap_or(b).display());
        }
    }

    // 7. 创建 .opt 项目目录
    let out_dir = output_dir
        .map(|p| p.to_path_buf())
        .unwrap_or_else(|| PathBuf::from(&pkg_name));

    if out_dir.exists() {
        if !force {
            return Err(OptError::General(format!(
                "输出目录 '{}' 已存在，使用 -f 覆盖", out_dir.display()
            )));
        }
        fs::remove_dir_all(&out_dir)?;
    }

    let opt_data_dir = out_dir.join("data");
    fs::create_dir_all(&opt_data_dir)?;

    // 8. 复制文件到 opt 项目结构
    eprintln!("  → 正在创建 .opt 项目...");

    // 复制所有提取的文件，移除开头的 ./
    for entry in walkdir::WalkDir::new(&tmp_dir)
        .into_iter()
        .filter_entry(|e| {
            !e.file_name().to_str().map(|s| s.starts_with('.')).unwrap_or(false)
        })
    {
        let entry = entry?;
        let path = entry.path();
        if path == &tmp_dir { continue; }

        let relative = path.strip_prefix(&tmp_dir).unwrap();
        let target = opt_data_dir.join(relative);

        if path.is_dir() {
            fs::create_dir_all(&target)?;
        } else {
            if let Some(parent) = target.parent() {
                fs::create_dir_all(parent)?;
            }
            fs::copy(path, &target)?;
        }
    }

    // 9. 生成 opt.json
    let mut desktop_file = String::new();
    let desktop_dir = opt_data_dir.join("usr/share/applications");
    if desktop_dir.exists() {
        if let Ok(entries) = fs::read_dir(&desktop_dir) {
            for e in entries.flatten() {
                let name = e.file_name().to_string_lossy().to_string();
                if name.ends_with(".desktop") {
                    desktop_file = name;
                    break;
                }
            }
        }
    }

    let installed_size = compute_dir_size(&opt_data_dir);

    let manifest = PackageManifest {
        name: pkg_name.clone(),
        version: pkg_version.clone(),
        description: if pkg_desc.is_empty() {
            format!("{pkg_name} - 从 .deb 导入")
        } else {
            pkg_desc
        },
        section: pkg_section,
        maintainer: pkg_maintainer,
        homepage: String::new(),
        license: "Proprietary".to_string(),
        architecture: pkg_arch,
        package_size: 0,
        installed_size,
        filename: String::new(),
        sha256: String::new(),
        depends: vec![],
        recommends: vec![],
        suggests: vec![],
        provides: vec![],
        conflicts: vec![],
        replaces: vec![],
        priority: "optional".to_string(),
        tags: vec!["imported".to_string()],
        origin: "debian-import".to_string(),
        desktop_file,
        appstream_id: format!("org.openos.{pkg_name}"),
        categories: vec!["Utility".to_string()],
        screenshots: vec![],
        app_type: crate::format::AppType::Cli,
        runtime: crate::format::Runtime::Crostini,
        min_kernel: String::new(),
        chromium_features: vec!["crostini".to_string()],
        permissions: vec![],
        xterm: true,
        schema: Some("https://openos.org/schemas/opt/v1.json".to_string()),
    };

    let manifest_json = serde_json::to_string_pretty(&manifest)?;
    fs::write(out_dir.join("opt.json"), manifest_json)?;

    // 10. 清理临时目录
    fs::remove_dir_all(&tmp_dir).ok();

    eprintln!();

    // 11. 构建 .opt 包
    builder::build_package(&out_dir, None, true)
}

/// 从目录导入（将任意 Linux 应用目录转为 .opt）
fn import_directory(source: &str, output_dir: Option<&Path>, force: bool) -> OptResult<PathBuf> {
    let src = Path::new(source);
    let name = src.file_name().and_then(|s| s.to_str()).unwrap_or("app");

    let out_dir = output_dir.map(|p| p.to_path_buf()).unwrap_or_else(|| {
        PathBuf::from(format!("{name}-opt"))
    });

    if out_dir.exists() && !force {
        return Err(OptError::General(format!(
            "输出目录 '{}' 已存在，使用 -f 覆盖", out_dir.display()
        )));
    }
    if out_dir.exists() {
        fs::remove_dir_all(&out_dir)?;
    }

    // 直接复制目录作为 data
    let data_dir = out_dir.join("data");
    copy_dir_recursive(src, &data_dir)?;

    // 生成 opt.json
    let installed_size = compute_dir_size(&data_dir);
    let binaries = find_binaries(&data_dir);

    let manifest = PackageManifest {
        name: name.to_string(),
        version: "0.1.0".to_string(),
        description: format!("{name} - 导入的 Linux 应用"),
        section: "utils".to_string(),
        maintainer: "Imported <import@openos.org>".to_string(),
        homepage: String::new(),
        license: "Proprietary".to_string(),
        architecture: "amd64".to_string(),
        package_size: 0,
        installed_size,
        filename: String::new(),
        sha256: String::new(),
        depends: vec![],
        recommends: vec![],
        suggests: vec![],
        provides: vec![],
        conflicts: vec![],
        replaces: vec![],
        priority: "optional".to_string(),
        tags: vec!["imported".to_string()],
        origin: "local-import".to_string(),
        desktop_file: String::new(),
        appstream_id: format!("org.openos.{name}"),
        categories: vec!["Utility".to_string()],
        screenshots: vec![],
        app_type: crate::format::AppType::Cli,
        runtime: crate::format::Runtime::Crostini,
        min_kernel: String::new(),
        chromium_features: vec!["crostini".to_string()],
        permissions: vec![],
        xterm: true,
        schema: Some("https://openos.org/schemas/opt/v1.json".to_string()),
    };

    fs::write(out_dir.join("opt.json"), serde_json::to_string_pretty(&manifest)?)?;

    eprintln!("  → 发现 {} 个可执行文件", binaries.len());

    builder::build_package(&out_dir, None, true)
}

/// 从 ELF 二进制文件导入（自动包裹为 .opt）
fn import_binary(source: &str, output_dir: Option<&Path>, force: bool) -> OptResult<PathBuf> {
    let src = Path::new(source);
    let name = src.file_stem().and_then(|s| s.to_str()).unwrap_or("app");

    let out_dir = output_dir.map(|p| p.to_path_buf()).unwrap_or_else(|| {
        PathBuf::from(format!("{name}-opt"))
    });

    if out_dir.exists() && !force {
        return Err(OptError::General(format!(
            "输出目录 '{}' 已存在，使用 -f 覆盖", out_dir.display()
        )));
    }
    if out_dir.exists() {
        fs::remove_dir_all(&out_dir)?;
    }

    // 创建 opt 项目结构: data/opt/<name>/<binary>
    let app_dir = out_dir.join("data").join("opt").join(name);
    fs::create_dir_all(&app_dir)?;
    fs::copy(src, app_dir.join(name))?;

    // 设为可执行
    #[cfg(unix)]
    {
        use std::os::unix::fs::PermissionsExt;
        let _ = fs::set_permissions(app_dir.join(name), fs::Permissions::from_mode(0o755));
    }

    eprintln!("  → 已打包 ELF 二进制: opt/{name}/{name}");

    let installed_size = src.metadata().map(|m| m.len()).unwrap_or(0);

    let manifest = PackageManifest {
        name: name.to_string(),
        version: "0.1.0".to_string(),
        description: format!("{name} - 自包含 ELF 应用"),
        section: "utils".to_string(),
        maintainer: "Imported <import@openos.org>".to_string(),
        homepage: String::new(),
        license: "Proprietary".to_string(),
        architecture: "amd64".to_string(),
        package_size: 0,
        installed_size,
        filename: String::new(),
        sha256: String::new(),
        depends: vec![],
        recommends: vec![],
        suggests: vec![],
        provides: vec![],
        conflicts: vec![],
        replaces: vec![],
        priority: "optional".to_string(),
        tags: vec!["imported-elf".to_string()],
        origin: "local-import".to_string(),
        desktop_file: String::new(),
        appstream_id: format!("org.openos.{name}"),
        categories: vec!["Utility".to_string()],
        screenshots: vec![],
        app_type: crate::format::AppType::Cli,
        runtime: crate::format::Runtime::Crostini,
        min_kernel: String::new(),
        chromium_features: vec!["crostini".to_string()],
        permissions: vec![],
        xterm: true,
        schema: Some("https://openos.org/schemas/opt/v1.json".to_string()),
    };

    fs::write(out_dir.join("opt.json"), serde_json::to_string_pretty(&manifest)?)?;

    builder::build_package(&out_dir, None, true)
}

/// 从 AppImage 导入（提取并打包）
fn import_appimage(source: &str, output_dir: Option<&Path>, force: bool) -> OptResult<PathBuf> {
    let src = Path::new(source);
    eprintln!("  → AppImage 检测到，正在提取...");

    // 尝试用 --appimage-extract 提取
    let status = std::process::Command::new(src)
        .arg("--appimage-extract")
        .current_dir(src.parent().unwrap_or(Path::new(".")))
        .status()
        .map_err(|e| OptError::General(format!("AppImage 提取失败: {e}")))?;

    if !status.success() {
        return Err(OptError::General("AppImage 提取失败".into()));
    }

    // AppImage 提取到 ./squashfs-root/
    let extract_dir = src.parent().unwrap_or(Path::new(".")).join("squashfs-root");
    if !extract_dir.exists() {
        return Err(OptError::General("未找到提取目录 squashfs-root".into()));
    }

    let result = import_directory(
        &extract_dir.to_string_lossy(),
        output_dir,
        force,
    );

    // 清理提取目录
    fs::remove_dir_all(&extract_dir).ok();

    result
}

/// 递归复制目录
fn copy_dir_recursive(src: &Path, dst: &Path) -> std::io::Result<()> {
    if src.is_dir() {
        fs::create_dir_all(dst)?;
        for entry in fs::read_dir(src)? {
            let entry = entry?;
            let file_type = entry.file_type()?;
            let src_path = entry.path();
            let dst_path = dst.join(entry.file_name());
            if file_type.is_dir() {
                copy_dir_recursive(&src_path, &dst_path)?;
            } else {
                fs::copy(&src_path, &dst_path)?;
            }
        }
        Ok(())
    } else {
        if let Some(parent) = dst.parent() {
            fs::create_dir_all(parent)?;
        }
        fs::copy(src, dst)?;
        Ok(())
    }
}

/// 计算目录大小
fn compute_dir_size(dir: &Path) -> u64 {
    let mut total = 0u64;
    if let Ok(entries) = fs::read_dir(dir) {
        for entry in entries.flatten() {
            let path = entry.path();
            if path.is_dir() {
                total += compute_dir_size(&path);
            } else if let Ok(meta) = path.metadata() {
                total += meta.len();
            }
        }
    }
    total
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_ar_parse_empty() {
        let result = parse_ar_archive(b"not ar");
        assert!(result.is_err());
    }
}
