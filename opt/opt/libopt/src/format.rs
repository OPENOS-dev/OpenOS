/// OPT Package Format
///
/// Defines the .opt package format which is a ZIP archive containing:
///   opt.json            - Package manifest
///   data.zip            - Actual application files (inner ZIP)
///   control.tar.gz      - Pre/post install scripts
///   checksums.sha256    - File integrity hashes
///
/// The format is designed for ChromiumOS desktop app distribution,
/// complementing the .opk mobile format.

use serde::{Deserialize, Serialize};
use std::collections::HashMap;

// ──────────────────────────────────────────────
// Supported architecture list
// ──────────────────────────────────────────────
pub const VALID_ARCHITECTURES: &[&str] = &["amd64", "arm64", "armhf", "i386", "all"];

// ──────────────────────────────────────────────
// Application type
// ──────────────────────────────────────────────
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq)]
#[serde(rename_all = "lowercase")]
pub enum AppType {
    Gui,
    Cli,
    Daemon,
    Library,
    Service,
}

impl std::fmt::Display for AppType {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            AppType::Gui => write!(f, "gui"),
            AppType::Cli => write!(f, "cli"),
            AppType::Daemon => write!(f, "daemon"),
            AppType::Library => write!(f, "library"),
            AppType::Service => write!(f, "service"),
        }
    }
}

// ──────────────────────────────────────────────
// Runtime type
// ──────────────────────────────────────────────
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq)]
#[serde(rename_all = "lowercase")]
pub enum Runtime {
    Native,
    Crostini,
    Arcvm,
    Flatpak,
    Snap,
    Container,
}

impl std::fmt::Display for Runtime {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Runtime::Native => write!(f, "native"),
            Runtime::Crostini => write!(f, "crostini"),
            Runtime::Arcvm => write!(f, "arcvm"),
            Runtime::Flatpak => write!(f, "flatpak"),
            Runtime::Snap => write!(f, "snap"),
            Runtime::Container => write!(f, "container"),
        }
    }
}

// ──────────────────────────────────────────────
// Package manifest (opt.json)
// ──────────────────────────────────────────────
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PackageManifest {
    // ── Core fields (aligned with OPK) ──
    pub name: String,
    pub version: String,
    pub description: String,
    pub section: String,
    pub maintainer: String,

    #[serde(default)]
    pub homepage: String,

    pub license: String,
    pub architecture: String,
    pub package_size: u64,
    pub installed_size: u64,
    pub filename: String,
    pub sha256: String,

    // ── Dependencies ──
    #[serde(default)]
    pub depends: Vec<String>,

    #[serde(default)]
    pub recommends: Vec<String>,

    #[serde(default)]
    pub suggests: Vec<String>,

    #[serde(default)]
    pub provides: Vec<String>,

    #[serde(default)]
    pub conflicts: Vec<String>,

    #[serde(default)]
    pub replaces: Vec<String>,

    // ── Priority & tags ──
    #[serde(default = "default_priority")]
    pub priority: String,

    #[serde(default)]
    pub tags: Vec<String>,

    #[serde(default)]
    pub origin: String,

    // ── Desktop-specific fields (OPT only) ──
    pub desktop_file: String,

    #[serde(default)]
    pub appstream_id: String,

    #[serde(default)]
    pub categories: Vec<String>,

    #[serde(default)]
    pub screenshots: Vec<String>,

    pub app_type: AppType,
    pub runtime: Runtime,

    #[serde(default)]
    pub min_kernel: String,

    #[serde(default)]
    pub chromium_features: Vec<String>,

    #[serde(default)]
    pub permissions: Vec<String>,

    #[serde(default)]
    pub xterm: bool,

    // ── Optional schema ref ──
    #[serde(rename = "$schema", skip_serializing_if = "Option::is_none")]
    pub schema: Option<String>,
}

fn default_priority() -> String {
    "optional".to_string()
}

impl PackageManifest {
    /// Generate the standard filename for this package.
    pub fn standard_filename(&self) -> String {
        format!("{}_{}_{}.opt", self.name, self.version, self.architecture)
    }

    /// Validate that all required fields are present and valid.
    pub fn validate(&self) -> Result<(), Vec<String>> {
        let mut errors: Vec<String> = Vec::new();

        if self.name.is_empty() {
            errors.push("Package name is required".into());
        }
        if self.version.is_empty() {
            errors.push("Package version is required".into());
        }
        if self.description.is_empty() {
            errors.push("Package description is required".into());
        }
        if self.maintainer.is_empty() {
            errors.push("Maintainer is required".into());
        }
        if self.license.is_empty() {
            errors.push("License is required".into());
        }
        if self.desktop_file.is_empty() {
            errors.push("desktop_file is required for .opt packages".into());
        }
        if !VALID_ARCHITECTURES.contains(&self.architecture.as_str()) {
            errors.push(format!(
                "Invalid architecture '{}'. Must be one of: {:?}",
                self.architecture, VALID_ARCHITECTURES
            ));
        }
        // 大小字段在 build 时自动计算，此处不校验
        // 若需校验可在此添加自定义断言

        if errors.is_empty() {
            Ok(())
        } else {
            Err(errors)
        }
    }
}

// ──────────────────────────────────────────────
// Repository index (Packages.json)
// ──────────────────────────────────────────────
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct RepositoryIndex {
    pub origin: String,
    pub label: String,
    pub description: String,
    pub components: Vec<String>,
    pub architectures: Vec<String>,

    #[serde(default)]
    pub packages: Vec<PackageManifest>,

    pub index_sha256: String,
    pub generated_at: String,
}

// ──────────────────────────────────────────────
// Checksums file
// ──────────────────────────────────────────────
#[derive(Debug, Clone)]
pub struct Checksums {
    pub entries: HashMap<String, String>,
}

impl Checksums {
    pub fn from_bytes(bytes: &[u8]) -> Result<Self, String> {
        let content = std::str::from_utf8(bytes).map_err(|e| format!("Invalid UTF-8: {e}"))?;
        let mut entries = HashMap::new();

        for line in content.lines() {
            let line = line.trim();
            if line.is_empty() {
                continue;
            }
            let parts: Vec<&str> = line.splitn(2, "  ").collect();
            if parts.len() != 2 {
                return Err(format!("Invalid checksum line: {line}"));
            }
            entries.insert(parts[1].to_string(), parts[0].to_string());
        }

        Ok(Checksums { entries })
    }

    pub fn to_bytes(&self) -> Vec<u8> {
        let mut lines: Vec<String> = self
            .entries
            .iter()
            .map(|(file, hash)| format!("{hash}  {file}"))
            .collect();
        lines.sort();
        lines.join("\n").into_bytes()
    }

    pub fn verify(&self, file: &str, expected_hash: &str) -> bool {
        self.entries
            .get(file)
            .map(|h| h == expected_hash)
            .unwrap_or(false)
    }
}

// ──────────────────────────────────────────────
// OPT package file layout:
//
// mypackage_1.0.0_amd64.opt
// ├── opt.json              (PackageManifest in JSON)
// ├── data.zip             (Inner ZIP with app files)
// ├── control.tar.gz       (Install scripts)
// │   ├── preinst
// │   ├── postinst
// │   ├── prerm
// │   └── postrm
// └── checksums.sha256     (File hashes)
// ──────────────────────────────────────────────
pub const OPT_INNER_DATA_ZIP: &str = "data.zip";
pub const OPT_MANIFEST: &str = "opt.json";
pub const OPT_CONTROL: &str = "control.tar.gz";
pub const OPT_CHECKSUMS: &str = "checksums.sha256";

/// Standard entries that must be present in a valid .opt package.
pub const OPT_REQUIRED_ENTRIES: &[&str] = &[
    OPT_MANIFEST,
    OPT_INNER_DATA_ZIP,
    OPT_CHECKSUMS,
];
