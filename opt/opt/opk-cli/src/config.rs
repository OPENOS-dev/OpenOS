// OPK Configuration management
//
// Handles reading/writing OPK configuration files:
// - /etc/opk/opk.conf (main config)
// - /etc/opk/sources.list (repository sources)
// - /var/lib/opk/status (installed packages database)
//
// Format mirrors Debian's dpkg/apt layout for familiarity.

use crate::error::OpkError;
use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use std::path::PathBuf;

/// Repository configuration
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Repository {
    pub name: String,
    pub url: String,
    pub enabled: bool,
    #[serde(default = "default_components")]
    pub components: Vec<String>,
}

fn default_components() -> Vec<String> {
    vec!["main".to_string()]
}

/// Main OPK configuration
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Config {
    #[serde(default = "default_repos")]
    pub repositories: Vec<Repository>,
    #[serde(default = "default_arch")]
    pub architecture: String,
    #[serde(default)]
    pub options: HashMap<String, String>,
}

fn default_repos() -> Vec<Repository> {
    vec![Repository {
        name: "openos-stable".to_string(),
        url: "https://repo.openos.org/stable".to_string(),
        enabled: true,
        components: vec!["main".to_string(), "universe".to_string(), "multiverse".to_string()],
    }]
}

fn default_arch() -> String {
    "amd64".to_string()
}

/// Installed package record
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct InstalledPackage {
    pub name: String,
    pub version: String,
    pub installed_size: u64,
    pub install_date: String,
    // pub files: Vec<String>,  // List of installed files
}

impl Config {
    /// Load configuration from standard paths
    pub fn load() -> Result<Self, OpkError> {
        let config_path = Self::config_path();

        if config_path.exists() {
            let content = std::fs::read_to_string(&config_path)?;
            let config: Config = toml::from_str(&content)?;
            return Ok(config);
        }

        // Return default config and write it
        let config = Config {
            repositories: default_repos(),
            architecture: default_arch(),
            options: HashMap::new(),
        };
        config.save()?;
        Ok(config)
    }

    /// Save configuration to disk
    pub fn save(&self) -> Result<(), OpkError> {
        let config_path = Self::config_path();
        if let Some(parent) = config_path.parent() {
            std::fs::create_dir_all(parent)?;
        }

        let content = toml::to_string_pretty(self)?;
        std::fs::write(&config_path, content)?;
        Ok(())
    }

    /// Read the installed packages database
    pub fn read_installed_db(&self) -> Result<HashMap<String, InstalledPackage>, OpkError> {
        let status_path = Self::status_path();

        if !status_path.exists() {
            return Ok(HashMap::new());
        }

        let content = std::fs::read_to_string(&status_path)?;
        let packages: Vec<InstalledPackage> = serde_json::from_str(&content)
            .unwrap_or_default();

        Ok(packages.into_iter().map(|p| (p.name.clone(), p)).collect())
    }

    /// Write the installed packages database
    pub fn write_installed_db(&self, packages: &HashMap<String, InstalledPackage>) -> Result<(), OpkError> {
        let status_path = Self::status_path();
        if let Some(parent) = status_path.parent() {
            std::fs::create_dir_all(parent)?;
        }

        let list: Vec<&InstalledPackage> = packages.values().collect();
        let content = serde_json::to_string_pretty(&list)?;
        std::fs::write(&status_path, content)?;
        Ok(())
    }

    fn config_path() -> PathBuf {
        PathBuf::from("/etc/opk/opk.conf")
    }

    fn status_path() -> PathBuf {
        PathBuf::from("/var/lib/opk/status")
    }
}
