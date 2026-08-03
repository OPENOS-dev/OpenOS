// OPK Package Format
//
// Defines the .opk package format which is a tar.gz archive
// containing:
//   opk.json          - Package manifest (this struct)
//   data.tar.gz       - Actual package files
//   control.tar.gz    - Pre/post install scripts
//   checksums.sha256  - File integrity hashes
//
// The format is inspired by Debian's .deb but with modern JSON metadata
// and SHA-256 verification.

use serde::{Deserialize, Serialize};

/// Package manifest - equivalent to DEBIAN/control in .deb
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PackageManifest {
    /// Package name (unique identifier)
    pub name: String,

    /// Package version (semver)
    pub version: String,

    /// Short description (one line, < 80 chars)
    pub description: String,

    /// Package section/category
    /// Examples: base, devel, graphics, net, utils, games, science
    pub section: String,

    /// Maintainer email or name
    pub maintainer: String,

    /// Project homepage URL
    pub homepage: String,

    /// SPDX license identifier
    pub license: String,

    /// Target architecture (amd64, arm64, all)
    pub architecture: String,

    /// Packaged file size in bytes
    pub package_size: u64,

    /// Estimated installed size in bytes
    pub installed_size: u64,

    /// Package filename (e.g., firefox_120.0_amd64.opk)
    pub filename: String,

    /// SHA-256 hash of the package file
    pub sha256: String,

    /// List of dependencies (package names)
    #[serde(default)]
    pub depends: Vec<String>,

    /// Virtual packages provided
    #[serde(default)]
    pub provides: Vec<String>,

    /// Packages that conflict with this one
    #[serde(default)]
    pub conflicts: Vec<String>,

    /// Priority: required, important, standard, optional, extra
    #[serde(default = "default_priority")]
    pub priority: String,

    /// Package tags for search
    #[serde(default)]
    pub tags: Vec<String>,

    /// Repository origin
    #[serde(default)]
    pub origin: String,
}

fn default_priority() -> String {
    "optional".to_string()
}

/// Repository index file (Packages.json)
/// This is what's served at the repo URL
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct RepositoryIndex {
    /// Repository metadata
    pub origin: String,
    pub label: String,
    pub description: String,
    pub components: Vec<String>,
    pub architectures: Vec<String>,

    /// List of all packages in this index
    pub packages: Vec<PackageManifest>,

    /// SHA-256 of the Packages.json itself (for verification)
    pub index_sha256: String,

    /// Generation timestamp
    pub generated_at: String,
}

/// OPK Package file layout:
///
/// mypackage_1.0.0_amd64.opk
/// ├── opk.json              (PackageManifest in JSON)
/// ├── data.tar.gz           (Actual files, relative to /)
/// │   ├── usr/
/// │   │   ├── bin/
/// │   │   ├── lib/
/// │   │   └── share/
/// │   └── etc/
/// ├── control.tar.gz        (Install scripts)
/// │   ├── preinst           (Before install)
/// │   ├── postinst          (After install)
/// │   ├── prerm             (Before removal)
/// │   └── postrm            (After removal)
/// └── checksums.sha256      (File integrity)
///
impl PackageManifest {
    /// Generate the standard filename for this package
    pub fn standard_filename(&self) -> String {
        format!("{}_{}_{}.opk", self.name, self.version, self.architecture)
    }

    /// Validate that this manifest has all required fields
    pub fn validate(&self) -> Result<(), String> {
        if self.name.is_empty() {
            return Err("Package name is required".to_string());
        }
        if self.version.is_empty() {
            return Err("Package version is required".to_string());
        }
        if self.description.is_empty() {
            return Err("Package description is required".to_string());
        }
        if self.maintainer.is_empty() {
            return Err("Maintainer is required".to_string());
        }
        if !["amd64", "arm64", "armhf", "i386", "all"].contains(&self.architecture.as_str()) {
            return Err(format!("Invalid architecture: {}", self.architecture));
        }
        if self.package_size == 0 {
            return Err("Package size must be > 0".to_string());
        }
        Ok(())
    }
}
