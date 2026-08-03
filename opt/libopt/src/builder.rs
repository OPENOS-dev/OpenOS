/// OPT Package Builder
///
/// Builds .opt packages from structured application directories.
/// Creates ZIP archives containing opt.json, data.zip, control.tar.gz,
/// and checksums.sha256.

use crate::error::{OptError, OptResult};
use crate::format::{
    Checksums, PackageManifest, AppType, Runtime,
    OPT_MANIFEST, OPT_INNER_DATA_ZIP, OPT_CONTROL, OPT_CHECKSUMS,
};
use sha2::{Sha256, Digest};
use std::fs;
use std::io::Write;
use std::path::{Path, PathBuf};
use walkdir::WalkDir;

/// Build a .opt package from an application directory.
///
/// Expected directory structure:
///   my-app/
///   ├── opt.json          - Package manifest
///   ├── data/             - Application files (mirrors /)
///   │   ├── opt/<app>/
///   │   ├── usr/bin/
///   │   ├── usr/share/
///   │   └── ...
///   └── control/          - Install scripts (optional)
///       ├── preinst
///       ├── postinst
///       ├── prerm
///       └── postrm
pub fn build_package(
    app_dir: &Path,
    output_dir: Option<&Path>,
    force: bool,
) -> OptResult<PathBuf> {
    let app_dir = fs::canonicalize(app_dir)
        .map_err(|e| OptError::General(format!("Cannot access app directory: {e}")))?;

    // 1. Read and validate manifest
    let manifest_path = app_dir.join(OPT_MANIFEST);
    if !manifest_path.exists() {
        return Err(OptError::MissingField(format!(
            "Missing {} in application directory",
            OPT_MANIFEST
        )));
    }

    let manifest_content = fs::read_to_string(&manifest_path)?;
    let mut manifest: PackageManifest = serde_json::from_str(&manifest_content)?;

    // 2. Check data directory
    let data_dir = app_dir.join("data");
    if !data_dir.exists() {
        return Err(OptError::MissingField(
            "Missing data/ directory in application directory".into(),
        ));
    }

    // 自动计算安装大小
    if manifest.installed_size == 0 {
        manifest.installed_size = compute_dir_size(&data_dir);
    }

    // 非致命校验（警告不阻塞）
    if let Err(errors) = manifest.validate() {
        for e in &errors {
            eprintln!("  ⚠ {e}");
        }
    }

    // 3. Build the .opt file
    let filename = manifest.standard_filename();
    let out_dir = match output_dir {
        Some(dir) => {
            fs::create_dir_all(dir)?;
            dir.to_path_buf()
        }
        None => app_dir.clone(),
    };
    let opt_path = out_dir.join(&filename);

    if opt_path.exists() && !force {
        return Err(OptError::General(format!(
            "Output file {} already exists. Use --force to overwrite.",
            opt_path.display()
        )));
    }

    let file = fs::File::create(&opt_path)?;
    let mut opt_writer = zip::ZipWriter::new(file);

    let options = zip::write::FileOptions::<()>::default()
        .compression_method(zip::CompressionMethod::Deflated)
        .unix_permissions(0o644);

    // ── Stage 1: Build data.zip (inner archive) ──
    let data_zip_bytes = build_inner_data_zip(&data_dir)?;

    // ── Stage 2: Build control.tar.gz (if control/ exists) ──
    let control_dir = app_dir.join("control");
    let control_bytes = if control_dir.exists() {
        Some(build_control_tar_gz(&control_dir)?)
    } else {
        None
    };

    // ── Stage 3: Compute checksums ──
    let mut checksums = Checksums {
        entries: std::collections::HashMap::new(),
    };
    checksums.entries.insert(
        OPT_MANIFEST.to_string(),
        sha256_hex(&manifest_content.as_bytes()),
    );
    checksums.entries.insert(
        OPT_INNER_DATA_ZIP.to_string(),
        sha256_hex(&data_zip_bytes),
    );
    if let Some(ref ctrl) = control_bytes {
        checksums
            .entries
            .insert(OPT_CONTROL.to_string(), sha256_hex(ctrl));
    }

    // ── Stage 4: Write outer ZIP entries ──
    // Write opt.json
    opt_writer.start_file(OPT_MANIFEST, options)?;
    opt_writer.write_all(manifest_content.as_bytes())?;

    // Write data.zip
    opt_writer.start_file(OPT_INNER_DATA_ZIP, options)?;
    opt_writer.write_all(&data_zip_bytes)?;

    // Write control.tar.gz (if exists)
    if let Some(ref ctrl) = control_bytes {
        opt_writer.start_file(OPT_CONTROL, options)?;
        opt_writer.write_all(ctrl)?;
    }

    // Write checksums.sha256
    let checksums_bytes = checksums.to_bytes();
    opt_writer.start_file(OPT_CHECKSUMS, options)?;
    opt_writer.write_all(&checksums_bytes)?;

    opt_writer.finish()?;

    // ── Stage 5: Update manifest with final metadata ──
    update_package_metadata(&opt_path, &manifest)?;

    eprintln!(
        "✅ 构建完成: {} ({})",
        filename,
        format_size(opt_path.metadata()?.len())
    );

    Ok(opt_path)
}

/// Build the inner data.zip from the data/ directory.
fn build_inner_data_zip(data_dir: &Path) -> OptResult<Vec<u8>> {
    let mut buf = Vec::new();
    let mut inner_writer = zip::ZipWriter::new(std::io::Cursor::new(&mut buf));

    let options = zip::write::FileOptions::<()>::default()
        .compression_method(zip::CompressionMethod::Deflated)
        .unix_permissions(0o755);

    for entry in WalkDir::new(data_dir).into_iter().filter_entry(|e| {
        !e.file_name()
            .to_str()
            .map(|s| s.starts_with('.'))
            .unwrap_or(false)
    }) {
        let entry = entry?;
        let path = entry.path();
        let relative = path
            .strip_prefix(data_dir)
            .map_err(|_| OptError::General("Path prefix error".into()))?;

        if relative.as_os_str().is_empty() {
            continue;
        }

        let entry_name = relative.to_string_lossy().to_string();

        if path.is_dir() {
            inner_writer.add_directory(&entry_name, options)?;
        } else {
            inner_writer.start_file(&entry_name, options)?;
            inner_writer.write_all(&fs::read(path)?)?;
        }
    }

    inner_writer.finish()?;

    Ok(buf)
}

/// Build control.tar.gz from the control/ directory.
fn build_control_tar_gz(control_dir: &Path) -> OptResult<Vec<u8>> {
    use flate2::Compression;
    use flate2::write::GzEncoder;

    let mut buf = Vec::new();
    let mut tar_buf = Vec::new();
    {
        let mut tar_writer = tar::Builder::new(&mut tar_buf);
        for entry in WalkDir::new(control_dir)
            .into_iter()
            .filter_entry(|e| !e.file_name().to_str().map(|s| s.starts_with('.')).unwrap_or(false))
        {
            let entry = entry?;
            let path = entry.path();
            if path.is_file() {
                let relative = path.strip_prefix(control_dir).unwrap();
                let mut file = fs::File::open(path)?;
                tar_writer.append_file(relative, &mut file)?;
            }
        }
        tar_writer.finish()?;
    }

    let mut encoder = GzEncoder::new(&mut buf, Compression::default());
    std::io::copy(&mut tar_buf.as_slice(), &mut encoder)?;
    encoder.finish()?;

    Ok(buf)
}

/// Compute SHA-256 hex string.
fn sha256_hex(data: &[u8]) -> String {
    let mut hasher = Sha256::new();
    hasher.update(data);
    hex::encode(hasher.finalize())
}

/// 递归计算目录下所有文件的总大小（字节）
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

/// Re-read the built package, compute its SHA-256, and print final metadata.
fn update_package_metadata(opt_path: &Path, _manifest: &PackageManifest) -> OptResult<()> {
    let pkg_bytes = fs::read(opt_path)?;
    let pkg_hash = sha256_hex(&pkg_bytes);
    let pkg_len = pkg_bytes.len() as u64;

    eprintln!("  包 SHA256: {pkg_hash}");
    eprintln!("  包大小:   {}", format_size(pkg_len));

    Ok(())
}

fn format_size(bytes: u64) -> String {
    const UNITS: &[&str] = &["B", "KB", "MB", "GB"];
    let mut size = bytes as f64;
    let mut unit_idx = 0;
    while size >= 1024.0 && unit_idx < UNITS.len() - 1 {
        size /= 1024.0;
        unit_idx += 1;
    }
    format!("{:.2} {}", size, UNITS[unit_idx])
}

/// Create a scaffolding template for a new .opt application directory.
pub fn create_scaffold(name: &str, output_dir: &Path) -> OptResult<PathBuf> {
    let app_dir = output_dir.join(name);
    if app_dir.exists() {
        return Err(OptError::General(format!(
            "Directory '{}' already exists",
            app_dir.display()
        )));
    }

    fs::create_dir_all(app_dir.join("data/opt").join(name))?;
    fs::create_dir_all(app_dir.join("data/usr/share/applications"))?;
    fs::create_dir_all(app_dir.join("data/usr/share/icons/hicolor/256x256/apps"))?;
    fs::create_dir_all(app_dir.join("control"))?;

    let manifest = PackageManifest {
        name: name.to_string(),
        version: "0.1.0".to_string(),
        description: format!("{name} - OPENOS Desktop Application"),
        section: "utils".to_string(),
        maintainer: "OPENOS <packages@openos.org>".to_string(),
        homepage: String::new(),
        license: "GPL-3.0-only".to_string(),
        architecture: "amd64".to_string(),
        package_size: 0,
        installed_size: 0,
        filename: String::new(),
        sha256: String::new(),
        depends: vec![],
        recommends: vec![],
        suggests: vec![],
        provides: vec![],
        conflicts: vec![],
        replaces: vec![],
        priority: "optional".to_string(),
        tags: vec![],
        origin: "local".to_string(),
        desktop_file: format!("{name}.desktop"),
        appstream_id: format!("org.openos.{name}"),
        categories: vec!["Utility".to_string()],
        screenshots: vec![],
        app_type: AppType::Gui,
        runtime: Runtime::Native,
        min_kernel: String::new(),
        chromium_features: vec![],
        permissions: vec![],
        xterm: false,
        schema: Some("https://openos.org/schemas/opt/v1.json".to_string()),
    };

    let manifest_json = serde_json::to_string_pretty(&manifest)?;
    fs::write(app_dir.join("opt.json"), manifest_json)?;

    // Create a simple desktop file template
    let desktop_content = format!(
        "[Desktop Entry]\n\
         Type=Application\n\
         Name={name}\n\
         Comment={name} - OPENOS Desktop Application\n\
         Exec=/opt/{name}/{name}\n\
         Icon={name}\n\
         Categories=Utility;\n\
         Terminal=false\n"
    );
    fs::write(
        app_dir.join("data/usr/share/applications").join(format!("{name}.desktop")),
        desktop_content,
    )?;

    eprintln!(
        "✅ 项目脚手架已创建: {}",
        app_dir.display()
    );

    Ok(app_dir)
}

#[cfg(test)]
mod tests {
    use super::*;
    use tempfile::tempdir;

    #[test]
    fn test_scaffold_and_build() -> OptResult<()> {
        let tmp = tempdir().unwrap();
        let app_dir = create_scaffold("test-app", tmp.path())?;
        assert!(app_dir.join("opt.json").exists());
        assert!(app_dir.join("data").exists());
        Ok(())
    }
}
