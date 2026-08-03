mod cli;

use clap::Parser;
use cli::{Cli, Command, ConfigAction};
use libopt::error::{OptError, OptResult};
use libopt::repo::{PackageEntry, RepoManager, RepoType};

fn main() {
    let cli = Cli::parse();

    // 初始化仓库管理器（只执行一次）
    static ONCE: std::sync::Once = std::sync::Once::new();
    static mut MANAGER: Option<RepoManager> = None;
    ONCE.call_once(|| unsafe { MANAGER = Some(RepoManager::new()) });
    let rm = unsafe { MANAGER.as_mut().unwrap() };

    let result = match cli.command {
        Command::Install { package, yes, no_deps } => {
            if std::path::Path::new(&package).exists() && package.ends_with(".opt") {
                libopt::installer::install_package(&package, yes, no_deps)
            } else {
                install_from_repo(rm, &package, yes, no_deps)
            }
        }

        Command::Remove { package, purge, yes } =>
            libopt::installer::remove_package(&package, purge, yes),

        Command::Update => match rm.update_all() {
            Ok(results) => {
                for r in &results { println!("  ✅ {r}"); }
                let total = rm.build_index().unwrap_or(0);
                println!("📦 共 {total} 个包可用");
                Ok(())
            }
            Err(e) => {
                eprintln!("更新失败: {e}");
                eprintln!("提示: 先通过 'opt repo add <url>' 添加仓库");
                Err(e)
            }
        },

        Command::Upgrade { yes: _, dry_run: _ } => {
            eprintln!("opt upgrade: 尚未实现（差异升级逻辑待完成）");
            Ok(())
        }

        Command::Search { query, all } => match rm.search(&query, all) {
            Ok(results) => {
                if results.is_empty() {
                    eprintln!("未找到匹配 '{query}' 的包。");
                    eprintln!("提示: 先运行 'opt update' 同步仓库索引。");
                } else {
                    println!("搜索 \"{query}\" 的结果：");
                    println!("{:-<80}", "");
                    for p in &results {
                        println!("  {:<22} {:<14} {:<8} {}",
                            p.name, p.version, p.architecture, p.description);
                    }
                    println!("\n共 {} 个匹配结果", results.len());
                }
                Ok(())
            }
            Err(e) => Err(e),
        },

        Command::Info { package } => {
            // 先查本地安装的
            match libopt::installer::show_package(&package) {
                Ok(m) => { print_manifest_info(&m); Ok(()) }
                Err(_) => {
                    // 再查仓库索引
                    match rm.info(&package) {
                        Ok(entry) => { print_entry_info(&entry, rm); Ok(()) }
                        Err(e) => Err(e),
                    }
                }
            }
        }

        Command::List { upgradable: _ } => match libopt::installer::list_installed() {
            Ok(pkgs) => {
                if pkgs.is_empty() {
                    eprintln!("暂无已安装的包。");
                } else {
                    println!("已安装的包：");
                    println!("{:-<60}", "");
                    for p in &pkgs {
                        println!("  {:<20} {:<12} {:<8}  {}",
                            p.name, p.version, p.architecture, p.description);
                    }
                    println!("\n共 {} 个包", pkgs.len());
                }
                Ok(())
            }
            Err(e) => Err(e),
        },

        Command::Build { path, output, force } => {
            let app_dir = std::path::Path::new(&path);
            let out_dir = output.as_ref().map(|p| std::path::Path::new(p));
            match libopt::builder::build_package(app_dir, out_dir, force) {
                Ok(opt_path) => { println!("{}", opt_path.display()); Ok(()) }
                Err(e) => Err(e),
            }
        }

        Command::Scaffold { name, output } => {
            let out_dir = output
                .map(|p| std::path::PathBuf::from(p))
                .unwrap_or_else(|| std::env::current_dir().unwrap_or_default());
            match libopt::builder::create_scaffold(&name, &out_dir) {
                Ok(path) => {
                    println!("📁 脚手架已创建: {}", path.display());
                    println!("下一步: cd {} && opt build .", path.display());
                    Ok(())
                }
                Err(e) => Err(e),
            }
        }

        Command::Validate { path, verbose } => match libopt::installer::validate_package(&path) {
            Ok(manifest) => {
                if verbose {
                    println!("✅ 有效的 .opt 包：");
                    print_manifest_info(&manifest);
                } else {
                    println!("✅ {} {} ({}) — 检验通过",
                        manifest.name, manifest.version, manifest.architecture);
                }
                Ok(())
            }
            Err(e) => Err(e),
        },

        Command::Extract { path, output } => {
            let out_dir = match output {
                Some(dir) => std::path::PathBuf::from(dir),
                None => {
                    let name = std::path::Path::new(&path)
                        .file_stem().and_then(|s| s.to_str()).unwrap_or("extracted");
                    std::path::PathBuf::from(name)
                }
            };
            extract_package(&path, &out_dir)
        }

        Command::Import { source, output, force } => {
            let out_dir = output.as_ref().map(|p| std::path::Path::new(p));
            match libopt::importer::import(&source, out_dir, force) {
                Ok(opt_path) => {
                    println!("✅ 导入完成: {}", opt_path.display());
                    println!("安装: opt install {}", opt_path.display());
                    Ok(())
                }
                Err(e) => Err(e),
            }
        }

        Command::Version => {
            println!("opt {} — OPENOS 桌面包管理器", env!("CARGO_PKG_VERSION"));
            Ok(())
        }

        Command::Completion { shell } => {
            use clap::CommandFactory;
            let mut cmd = cli::Cli::command();
            let bin_name = "opt";
            clap_complete::generate(shell, &mut cmd, bin_name, &mut std::io::stdout());
            Ok(())
        }

        Command::Config { action } => match action {
            ConfigAction::List => {
                for (key, val) in rm.list_config() {
                    println!("  {:<20} {}", key, val);
                }
                Ok(())
            }
            ConfigAction::Get { key } => {
                match rm.get_config(&key) {
                    Some(val) => { println!("{val}"); Ok(()) }
                    None => Err(OptError::General(format!("未知配置: {key}"))),
                }
            }
            ConfigAction::Set { key, value } => {
                match rm.set_config(&key, &value) {
                    Ok(()) => { println!("✅ {key} = {value}"); Ok(()) }
                    Err(e) => Err(e),
                }
            }
        },

        Command::Share { target } => {
            // 如果是 .opt 文件路径，直接分享
            if std::path::Path::new(&target).exists() && target.ends_with(".opt") {
                rm.share_package(&target)
            } else {
                // 否则查找已安装的包
                let prefix = std::env::var("OPT_PREFIX")
                    .unwrap_or_else(|_| String::new());
                let prefix_path = if prefix.is_empty() {
                    dirs::home_dir().unwrap_or_else(|| std::path::PathBuf::from("~"))
                        .join(".local").join("opt")
                } else {
                    std::path::PathBuf::from(prefix).join("opt")
                };

                // 查找包目录下的 .opt 文件
                let cache = dirs::cache_dir()
                    .unwrap_or_else(|| std::path::PathBuf::from("/tmp"))
                    .join("opt").join("downloads");

                let search_paths = [
                    cache.join(format!("{target}.opt")),
                    prefix_path.join(&target).join(format!("{target}.opt")),
                ];

                let mut found = None;
                for p in &search_paths {
                    if p.exists() {
                        found = Some(p.clone());
                        break;
                    }
                }

                match found {
                    Some(path) => rm.share_package(&path.to_string_lossy()),
                    None => Err(OptError::NotFound(format!(
                        "未找到 '{}' 的 .opt 文件", target
                    ))),
                }
            }
        }
    };

    if let Err(e) = result {
        eprintln!("❌ 错误：{e}");
        std::process::exit(1);
    }

    // 后台自动扫描本地 .opt 并分享独有包（不阻塞）
    if rm.is_auto_scan_enabled() && rm.is_contribute_enabled() {
        let _repo_path = rm.repo_path().to_string();
        std::thread::Builder::new()
            .name("opt-scan".into())
            .spawn(move || {
                let mut bg_rm = RepoManager::new();
                bg_rm.auto_scan_and_share();
            })
            .ok();
    }
}

/// 从仓库安装包（支持 OPT / APT 自动检测）
fn install_from_repo(rm: &mut RepoManager, package: &str, yes: bool, no_deps: bool) -> OptResult<()> {
    // 先在索引里搜索
    let entry = match rm.info(package) {
        Ok(e) => e,
        Err(_) => {
            // 索引里没有，尝试通过 APT 仓库导入
            return install_from_apt(rm, package, yes, no_deps);
        }
    };

    let url = rm.get_download_url(&entry)
        .ok_or_else(|| OptError::General(format!("无法获取 '{}' 的下载地址", package)))?;

    println!("📦 正在下载 {} {}...", entry.name, entry.version);
    let response = ureq::get(&url)
        .call()
        .map_err(|e| OptError::Network(format!("下载失败: {e}")))?;

    let cache = dirs::cache_dir()
        .unwrap_or_else(|| std::path::PathBuf::from("/tmp"))
        .join("opt").join("downloads");
    std::fs::create_dir_all(&cache)?;

    let filename = entry.filename.as_deref().unwrap_or("download.opt");
    let save_path = cache.join(filename);
    let mut file = std::fs::File::create(&save_path)?;
    let mut reader = response.into_reader();
    let mut body = Vec::new();
    std::io::Read::read_to_end(&mut reader, &mut body)
        .map_err(|e| OptError::Network(format!("读取失败: {e}")))?;
    std::io::copy(&mut body.as_slice(), &mut file)?;

    println!("  → 已下载到: {}", save_path.display());

    if entry.repo_type == RepoType::Opt {
        libopt::installer::install_package(&save_path.to_string_lossy(), yes, no_deps)
    } else {
        // APT 包：先导入为 .opt，再安装
        let import_dir = cache.join("import").join(&entry.name);
        let opt_path = libopt::importer::import_deb(
            &save_path.to_string_lossy(), Some(&import_dir), true)?;
        libopt::installer::install_package(&opt_path.to_string_lossy(), yes, no_deps)
    }
}

/// 从 APT 仓库下载并导入包
fn install_from_apt(rm: &mut RepoManager, package: &str, yes: bool, no_deps: bool) -> OptResult<()> {
    // 找有没有 APT 类型的仓库
    let apt_repos: Vec<_> = rm.list_repos().iter()
        .filter(|r| r.repo_type == RepoType::Apt && r.enabled)
        .map(|r| r.clone())
        .collect();

    if apt_repos.is_empty() {
        return Err(OptError::NotFound(format!(
            "包 '{package}' 未找到。\n\
             提示: 添加 APT 仓库后再试:\n  \
             opt repo add http://deb.debian.org/debian\n  \
             opt update"
        )));
    }

    // 尝试从每个 APT 仓库下载 .deb
    for repo in &apt_repos {
        eprintln!("🔍 正在搜索 APT 仓库 '{}'...", repo.name);

        // 构造 APT 包 URL
        let url = format!(
            "{}/dists/{}/main/binary-{}/Packages.gz",
            repo.url.trim_end_matches('/'),
            repo.suite,
            repo.architectures.first().map(|s| s.as_str()).unwrap_or("amd64"),
        );

        // 下载并解析 Packages.gz 找包
        let response = match ureq::get(&url).call() {
            Ok(r) => r,
            Err(_) => continue,
        };

        let mut reader = response.into_reader();
        let mut compressed = Vec::new();
        if std::io::Read::read_to_end(&mut reader, &mut compressed).is_err() { continue; }

        let mut decoder = flate2::read::GzDecoder::new(&compressed[..]);
        let mut decompressed = Vec::new();
        if std::io::copy(&mut decoder, &mut decompressed).is_err() { continue; }
        let packages_index = String::from_utf8_lossy(&decompressed);

        // 搜索包名
        let mut found = false;
        let mut deb_filename = String::new();
        let mut pkg_version = String::new();

        for stanza in packages_index.split("\n\n") {
            if stanza.contains(&format!("\nPackage: {}\n", package))
                || stanza.contains(&format!("Package: {}\n", package))
            {
                for line in stanza.lines() {
                    if let Some((key, val)) = line.split_once(':') {
                        match key.trim() {
                            "Filename" => deb_filename = val.trim().to_string(),
                            "Version" => pkg_version = val.trim().to_string(),
                            _ => {}
                        }
                    }
                }
                if !deb_filename.is_empty() {
                    found = true;
                    break;
                }
            }
        }

        if !found { continue; }

        let deb_url = format!(
            "{}/{}",
            repo.url.trim_end_matches('/'),
            deb_filename
        );

        eprintln!("📦 发现 {} v{}", package, pkg_version);
        eprintln!("  → 正在下载 .deb...");

        let deb_response = ureq::get(&deb_url)
            .call()
            .map_err(|e| OptError::Network(format!("下载失败: {e}")))?;

        let cache = dirs::cache_dir()
            .unwrap_or_else(|| std::path::PathBuf::from("/tmp"))
            .join("opt").join("downloads");
        std::fs::create_dir_all(&cache)?;

        let deb_path = cache.join(format!("{}_{}.deb", package, pkg_version));
        let mut deb_file = std::fs::File::create(&deb_path)?;
        let mut deb_reader = deb_response.into_reader();
        let mut deb_body = Vec::new();
        std::io::Read::read_to_end(&mut deb_reader, &mut deb_body)
            .map_err(|e| OptError::Network(format!("读取失败: {e}")))?;
        std::io::copy(&mut deb_body.as_slice(), &mut deb_file)?;

        eprintln!("  → 已下载: {}", deb_path.display());

        // 导入 .deb → .opt
        let import_dir = cache.join("import").join(package);
        let opt_path = libopt::importer::import_deb(
            &deb_path.to_string_lossy(), Some(&import_dir), true)?;

        eprintln!("  → 正在安装...");
        libopt::installer::install_package(
            &opt_path.to_string_lossy(), yes, no_deps)?;

        // 如果开启了分享模式，自动分享到仓库
        if rm.is_contribute_enabled() {
            eprintln!("  📤 分享模式已开启，正在提交到仓库...");
            if let Err(e) = rm.share_package(&opt_path.to_string_lossy()) {
                eprintln!("  ⚠ 分享失败（不影响安装）: {e}");
            }
        }

        return Ok(());
    }

    Err(OptError::NotFound(format!(
        "包 '{package}' 在所有 APT 仓库中均未找到"
    )))
}

fn print_entry_info(entry: &PackageEntry, rm: &RepoManager) {
    println!("包名:           {}", entry.name);
    println!("版本:           {}", entry.version);
    println!("描述:           {}", entry.description);
    println!("架构:           {}", entry.architecture);
    println!("分类:           {}", entry.section);
    println!("仓库:           {} ({:?})", entry.repo_name, entry.repo_type);
    println!("安装后大小:     {}", format_size(entry.installed_size));
    if !entry.depends.is_empty() {
        println!("依赖:           {}", entry.depends.join(", "));
    }
    if !entry.provides.is_empty() {
        println!("提供:           {}", entry.provides.join(", "));
    }
    if let Some(url) = rm.get_download_url(entry) {
        println!("下载地址:       {url}");
        println!("\n安装:  opt install {}", entry.name);
    }
}

fn print_manifest_info(m: &libopt::format::PackageManifest) {
    println!("包名:           {}", m.name);
    println!("版本:           {}", m.version);
    println!("描述:           {}", m.description);
    println!("架构:           {}", m.architecture);
    println!("分类:           {}", m.section);
    println!("维护者:         {}", m.maintainer);
    println!("许可证:         {}", m.license);
    println!("运行方式:       {:?}", m.runtime);
    println!("应用类型:       {:?}", m.app_type);
    println!("桌面文件:       {}", m.desktop_file);
    println!("应用分类:       {}", m.categories.join(", "));
    println!("包大小:         {} (安装后: {})",
        format_size(m.package_size), format_size(m.installed_size));
    if !m.depends.is_empty() { println!("依赖:           {}", m.depends.join(", ")); }
    if !m.provides.is_empty() { println!("提供:           {}", m.provides.join(", ")); }
    if !m.permissions.is_empty() { println!("权限:           {}", m.permissions.join(", ")); }
    if !m.chromium_features.is_empty() { println!("特性:           {}", m.chromium_features.join(", ")); }
}

fn extract_package(path: &str, out_dir: &std::path::Path) -> OptResult<()> {
    use std::fs;
    let opt_path = std::path::Path::new(path);
    if !opt_path.exists() {
        return Err(OptError::NotFound(format!("文件不存在：{path}")));
    }
    let file = fs::File::open(opt_path)?;
    let mut archive = zip::ZipArchive::new(file)?;
    fs::create_dir_all(out_dir)?;
    for i in 0..archive.len() {
        let mut entry = archive.by_index(i)?;
        let name = entry.name().to_string();
        if name.ends_with('/') { fs::create_dir_all(out_dir.join(&name))?; continue; }
        let target = out_dir.join(&name);
        if let Some(parent) = target.parent() { fs::create_dir_all(parent)?; }
        let mut out = fs::File::create(&target)?;
        std::io::copy(&mut entry, &mut out)?;
    }
    eprintln!("✅ 已解包到：{}", out_dir.display());
    Ok(())
}

fn format_size(bytes: u64) -> String {
    const UNITS: &[&str] = &["B", "KB", "MB", "GB"];
    let mut size = bytes as f64;
    let mut unit_idx = 0;
    while size >= 1024.0 && unit_idx < UNITS.len() - 1 { size /= 1024.0; unit_idx += 1; }
    format!("{:.2} {}", size, UNITS[unit_idx])
}
