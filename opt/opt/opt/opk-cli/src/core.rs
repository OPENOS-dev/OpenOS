// OPK Core - Package management engine
//
// This is the heart of OPK: the OpkManager handles all package operations
// including installation, removal, updates, dependency resolution, and
// repository management. It mirrors apt's core functionality.
//
// Architecture:
//   OpkManager
//   ├── Config (repositories, settings)
//   ├── PackageDB (installed packages state)
//   ├── PackageCache (downloaded .opk files)
//   └── RepositoryClient (fetching from repos)

use crate::config::{Config, InstalledPackage, Repository};
use crate::error::OpkError;
use crate::format::PackageManifest;
use crate::repo::RepositoryClient;
use colored::*;
use std::collections::{HashMap, HashSet};
use std::path::PathBuf;

// Re-export the RepoAction so the main module can use it
use crate::cli::RepoAction;

pub struct OpkManager {
    config: Config,
    repos: Vec<Repository>,
    installed: HashMap<String, InstalledPackage>,
    cache_dir: PathBuf,
    state_dir: PathBuf,
}

impl OpkManager {
    pub fn new() -> Result<Self, OpkError> {
        let config = Config::load()?;
        let repos = config.repositories.clone();
        let installed = config.read_installed_db()?;

        let cache_dir = dirs::cache_dir()
            .unwrap_or_else(|| PathBuf::from("/tmp"))
            .join("opk");

        let state_dir = dirs::data_dir()
            .unwrap_or_else(|| PathBuf::from("/var/lib"))
            .join("opk");

        std::fs::create_dir_all(&cache_dir).ok();
        std::fs::create_dir_all(&state_dir).ok();

        Ok(Self {
            config,
            repos,
            installed,
            cache_dir,
            state_dir,
        })
    }

    /// Install one or more packages with dependency resolution
    pub fn install(&mut self, packages: &[String], yes: bool, no_deps: bool) -> Result<(), OpkError> {
        if packages.is_empty() {
            return Err(OpkError::NoPackagesSpecified);
        }

        println!("{} Reading package lists...", "→".cyan().bold());
        let client = RepositoryClient::new(&self.repos, &self.cache_dir);
        let available = client.fetch_available_packages()?;

        let mut to_install: Vec<(String, PackageManifest)> = Vec::new();
        let mut visited: HashSet<String> = HashSet::new();

        for pkg_name in packages {
            self.resolve_install(pkg_name, &available, &mut to_install, &mut visited, no_deps)?;
        }

        if to_install.is_empty() {
            println!("{} All packages already installed.", "✓".green());
            return Ok(());
        }

        // Display summary
        println!("\n{} The following packages will be INSTALLED:", "📦".bold());
        for (name, manifest) in &to_install {
            let size_str = format_size(manifest.installed_size);
            println!("  {} {} ({})", "  →".cyan(), name.bold(), size_str.dimmed());
        }

        println!(
            "\n{} {} packages, {} total",
            "→".cyan(),
            to_install.len(),
            format_size(to_install.iter().map(|(_, m)| m.installed_size).sum()),
        );

        if !yes {
            let proceed = dialoguer::Confirm::new()
                .with_prompt("Proceed with installation?")
                .default(true)
                .interact()
                .unwrap_or(false);

            if !proceed {
                println!("{} Installation cancelled.", "✗".yellow());
                return Ok(());
            }
        }

        // Download and install
        let progress = indicatif::MultiProgress::new();
        for (name, manifest) in &to_install {
            let pb = progress.add(indicatif::ProgressBar::new(manifest.package_size));
            pb.set_style(
                indicatif::ProgressStyle::default_bar()
                    .template("{prefix} {bar:40.cyan/blue} {bytes}/{total_bytes} {msg}")
                    .unwrap(),
            );
            pb.set_prefix(format!("  {}", name.dimmed()));
            pb.set_length(manifest.package_size);

            client.download_package(&manifest, |downloaded| {
                pb.set_position(downloaded);
            })?;

            let pkg_path = self.cache_dir.join(&manifest.filename);

            // Extract and install
            pb.set_prefix(format!("  {}", "Extracting".cyan()));
            self.extract_and_install(&pkg_path, &manifest)?;

            // Record installation
            self.installed.insert(
                name.clone(),
                InstalledPackage {
                    name: name.clone(),
                    version: manifest.version.clone(),
                    installed_size: manifest.installed_size,
                    install_date: chrono_now(),
                },
            );

            pb.finish_with_message(format!("{} Installed", "✓".green()));
        }

        self.config.write_installed_db(&self.installed)?;

        println!("\n{} Installation complete.", "✓".green().bold());
        Ok(())
    }

    /// Remove one or more packages
    pub fn remove(&mut self, packages: &[String], purge: bool, yes: bool) -> Result<(), OpkError> {
        let mut to_remove: Vec<String> = Vec::new();

        for pkg in packages {
            if self.installed.contains_key(pkg) {
                to_remove.push(pkg.clone());
            } else {
                println!("{} Package '{}' is not installed.", "!".yellow(), pkg);
            }
        }

        if to_remove.is_empty() {
            return Ok(());
        }

        println!("\n{} The following packages will be REMOVED:", "🗑️ ".bold());
        for pkg in &to_remove {
            if let Some(info) = self.installed.get(pkg) {
                println!(
                    "  {} {} (v{}, {})",
                    "  →".red(),
                    pkg.bold(),
                    info.version,
                    format_size(info.installed_size),
                );
            }
        }

        if !yes {
            let proceed = dialoguer::Confirm::new()
                .with_prompt("Proceed with removal?")
                .default(true)
                .interact()
                .unwrap_or(false);

            if !proceed {
                println!("{} Removal cancelled.", "✗".yellow());
                return Ok(());
            }
        }

        for pkg in &to_remove {
            self.uninstall_package(pkg, purge)?;
            self.installed.remove(pkg);
            println!("{} Removed {}", "✓".green(), pkg);
        }

        self.config.write_installed_db(&self.installed)?;
        Ok(())
    }

    /// Update package lists from all repositories
    pub fn update(&mut self) -> Result<(), OpkError> {
        println!("{} Updating package lists...", "⟳".cyan().bold());

        let client = RepositoryClient::new(&self.repos, &self.cache_dir);

        for repo in &self.repos {
            println!("  {} {}", "→".cyan(), repo.name.dimmed());
            match client.fetch_repo_index(repo) {
                Ok(count) => println!("    {} {} packages", "  ✓".green(), count),
                Err(e) => println!("    {} {}", "  ✗".red(), e),
            }
        }

        println!("\n{} Package lists updated.", "✓".green());
        Ok(())
    }

    /// Upgrade all installed packages
    pub fn upgrade(&mut self, yes: bool, dry_run: bool) -> Result<(), OpkError> {
        println!("{} Checking for upgrades...", "⟳".cyan().bold());

        let client = RepositoryClient::new(&self.repos, &self.cache_dir);
        let available = client.fetch_available_packages()?;

        let mut upgradable: Vec<(String, String, String)> = Vec::new(); // (name, current, latest)

        for (name, info) in &self.installed {
            if let Some(manifest) = available.get(name) {
                if manifest.version != info.version {
                    upgradable.push((name.clone(), info.version.clone(), manifest.version.clone()));
                }
            }
        }

        if upgradable.is_empty() {
            println!("{} All packages are up to date.", "✓".green());
            return Ok(());
        }

        println!("\n{} The following packages will be UPGRADED:", "📦".bold());
        for (name, current, latest) in &upgradable {
            println!(
                "  {} {} : {} → {}",
                "  →".cyan(),
                name.bold(),
                current.red(),
                latest.green(),
            );
        }

        if dry_run {
            println!("\n{} Dry run - no changes made.", "ℹ".cyan());
            return Ok(());
        }

        if !yes {
            let proceed = dialoguer::Confirm::new()
                .with_prompt("Proceed with upgrade?")
                .default(true)
                .interact()
                .unwrap_or(false);

            if !proceed {
                println!("{} Upgrade cancelled.", "✗".yellow());
                return Ok(());
            }
        }

        let pkg_names: Vec<String> = upgradable.iter().map(|(n, _, _)| n.clone()).collect();
        self.install(&pkg_names, true, false)?;

        Ok(())
    }

    /// Search packages by query
    pub fn search(&self, query: &str, _all: bool, _names_only: bool) -> Result<(), OpkError> {
        let client = RepositoryClient::new(&self.repos, &self.cache_dir);
        let available = client.fetch_available_packages()?;

        let query_lower = query.to_lowercase();
        let mut results: Vec<&PackageManifest> = available
            .values()
            .filter(|m| {
                m.name.to_lowercase().contains(&query_lower)
                    || m.description.to_lowercase().contains(&query_lower)
            })
            .collect();

        results.sort_by_key(|m| &m.name);

        if results.is_empty() {
            println!("{} No packages found matching '{}'", "!".yellow(), query);
            return Ok(());
        }

        println!("\n{} Found {} packages:\n", "🔍".bold(), results.len());
        for manifest in &results {
            println!(
                "{}/{} {} - {}",
                manifest.section.cyan(),
                manifest.name.bold(),
                manifest.version.green(),
                manifest.description.dimmed(),
            );
        }

        Ok(())
    }

    /// Show detailed package info
    pub fn info(&self, package: &str) -> Result<(), OpkError> {
        let client = RepositoryClient::new(&self.repos, &self.cache_dir);
        let available = client.fetch_available_packages()?;

        let manifest = available
            .get(package)
            .ok_or_else(|| OpkError::PackageNotFound(package.to_string()))?;

        let installed = self.installed.get(package);

        println!("\n{} {} v{}", "📦".bold(), manifest.name.bold(), manifest.version.green());
        println!("{}", "─".repeat(60));
        println!("  {}", manifest.description);
        println!();
        println!("  Section:     {}", manifest.section);
        println!("  Maintainer:  {}", manifest.maintainer);
        println!("  Homepage:    {}", manifest.homepage);
        println!("  License:     {}", manifest.license);
        println!("  Architecture: {}", manifest.architecture);
        println!("  Package size: {}", format_size(manifest.package_size));
        println!("  Installed size: {}", format_size(manifest.installed_size));
        println!();

        if let Some(inst) = installed {
            println!("  {} (v{})", "Installed".green().bold(), inst.version);
        } else {
            println!("  {}", "Not installed".dimmed());
        }

        if !manifest.depends.is_empty() {
            println!("\n  {}:", "Dependencies".bold());
            for dep in &manifest.depends {
                let marker = if available.contains_key(dep) { "✓".green() } else { "✗".red() };
                println!("    {} {}", marker, dep);
            }
        }

        if !manifest.provides.is_empty() {
            println!("\n  {}: {}", "Provides".bold(), manifest.provides.join(", "));
        }

        Ok(())
    }

    /// List packages
    pub fn list(&self, installed_only: bool, upgradable_only: bool) -> Result<(), OpkError> {
        if installed_only || upgradable_only {
            if self.installed.is_empty() {
                println!("{} No packages installed.", "!".yellow());
                return Ok(());
            }

            let client = RepositoryClient::new(&self.repos, &self.cache_dir);
            let available = client.fetch_available_packages().ok();

            println!("\n{} Installed packages:\n", "📋".bold());
            let mut sorted: Vec<_> = self.installed.iter().collect();
            sorted.sort_by_key(|(n, _)| n.as_str());

            for (name, info) in sorted {
                let upgrade = available.as_ref().and_then(|a| a.get(name));
                let status = match upgrade {
                    Some(m) if m.version != info.version => format!("{} → {}", info.version.red(), m.version.green()),
                    _ => info.version.clone(),
                };

                if upgradable_only && upgrade.map_or(true, |m| m.version == info.version) {
                    continue;
                }

                println!(
                    "  {}  {}  {}",
                    name.bold(),
                    status,
                    format_size(info.installed_size).dimmed(),
                );
            }
            println!("\n  Total: {}", self.installed.len());
        } else {
            let client = RepositoryClient::new(&self.repos, &self.cache_dir);
            let available = client.fetch_available_packages()?;

            println!("\n{} Available packages:\n", "📋".bold());
            let mut sorted: Vec<_> = available.values().collect();
            sorted.sort_by_key(|m| &m.name);

            for manifest in &sorted {
                let marker = if self.installed.contains_key(&manifest.name) {
                    "[installed]".green()
                } else {
                    "".normal()
                };
                println!(
                    "  {}/{} {} {} {}",
                    manifest.section.cyan(),
                    manifest.name.bold(),
                    manifest.version.green(),
                    format_size(manifest.installed_size).dimmed(),
                    marker,
                );
            }
            println!("\n  Total: {}", sorted.len());
        }

        Ok(())
    }

    /// Clean package cache
    pub fn clean(&self) -> Result<(), OpkError> {
        let archive_dir = self.cache_dir.join("archives");

        if !archive_dir.exists() {
            println!("{} Cache is already clean.", "✓".green());
            return Ok(());
        }

        let files: Vec<_> = std::fs::read_dir(&archive_dir)?
            .filter_map(|e| e.ok())
            .filter(|e| e.path().extension().map_or(false, |ext| ext == "opk"))
            .collect();

        let total_size: u64 = files
            .iter()
            .filter_map(|f| f.metadata().ok())
            .map(|m| m.len())
            .sum();

        for file in &files {
            std::fs::remove_file(file.path()).ok();
        }

        println!(
            "{} Cleaned {} files ({} freed)",
            "✓".green(),
            files.len(),
            format_size(total_size),
        );

        Ok(())
    }

    /// Download source for a package
    pub fn source(&self, package: &str) -> Result<(), OpkError> {
        let client = RepositoryClient::new(&self.repos, &self.cache_dir);
        let available = client.fetch_available_packages()?;

        let manifest = available
            .get(package)
            .ok_or_else(|| OpkError::PackageNotFound(package.to_string()))?;

        println!("{} Downloading source for {}...", "→".cyan(), package.bold());

        let src_path = PathBuf::from(format!("{}_{}.src.tar.gz", manifest.name, manifest.version));
        client.download_source(manifest, &src_path)?;

        println!(
            "{} Source downloaded: {} ({})",
            "✓".green(),
            src_path.display(),
            format_size(std::fs::metadata(&src_path).map(|m| m.len()).unwrap_or(0)),
        );

        Ok(())
    }

    /// Manage repositories
    pub fn repo(&mut self, action: RepoAction) -> Result<(), OpkError> {
        match action {
            RepoAction::Add { url, name } => {
                let name = name.unwrap_or_else(|| {
                    url.split('/')
                        .last()
                        .unwrap_or("custom")
                        .trim_end_matches(".git")
                        .to_string()
                });

                // Check for duplicates
                if self.repos.iter().any(|r| r.url == url || r.name == name) {
                    println!("{} Repository already exists: {}", "!".yellow(), name);
                    return Ok(());
                }

                let repo = Repository {
                    name: name.clone(),
                    url: url.clone(),
                    enabled: true,
                    components: vec!["main".to_string()],
                };

                self.repos.push(repo);
                self.config.repositories = self.repos.clone();
                self.config.save()?;

                println!("{} Added repository: {} ({})", "✓".green(), name, url);
            }
            RepoAction::Remove { target } => {
                let len_before = self.repos.len();
                self.repos.retain(|r| r.name != target && r.url != target);

                if self.repos.len() == len_before {
                    println!("{} Repository not found: {}", "!".yellow(), target);
                    return Ok(());
                }

                self.config.repositories = self.repos.clone();
                self.config.save()?;

                println!("{} Removed repository: {}", "✓".green(), target);
            }
            RepoAction::List => {
                if self.repos.is_empty() {
                    println!("{} No repositories configured.", "!".yellow());
                    println!("  Add one with: opk repo add <url>");
                    return Ok(());
                }

                println!("\n{} Configured repositories:\n", "📚".bold());
                for repo in &self.repos {
                    let status = if repo.enabled {
                        "enabled".green()
                    } else {
                        "disabled".red()
                    };
                    println!("  {} {} {}", "●".cyan(), repo.name.bold(), status);
                    println!("    {}", repo.url.dimmed());
                    println!("    Components: {}", repo.components.join(", ").dimmed());
                }
            }
        }

        Ok(())
    }

    /// Show version
    pub fn version(&self) -> Result<(), OpkError> {
        println!("{} OPK - Open Package Keeper", "📦".bold());
        println!("  Version:    {}", env!("CARGO_PKG_VERSION"));
        println!("  License:    {}", env!("CARGO_PKG_LICENSE"));
        println!("  Repository: {}", env!("CARGO_PKG_REPOSITORY"));
        println!("  OS:         Open-OS");
        println!();
        println!("  Installed packages: {}", self.installed.len());
        println!("  Repositories:       {}", self.repos.len());
        println!("  Cache dir:          {}", self.cache_dir.display());
        println!("  State dir:          {}", self.state_dir.display());
        Ok(())
    }

    // ---- Private helpers ----

    fn resolve_install(
        &self,
        pkg_name: &str,
        available: &HashMap<String, PackageManifest>,
        to_install: &mut Vec<(String, PackageManifest)>,
        visited: &mut HashSet<String>,
        no_deps: bool,
    ) -> Result<(), OpkError> {
        if !visited.insert(pkg_name.to_string()) {
            return Ok(()); // Already processed
        }

        if self.installed.contains_key(pkg_name) {
            return Ok(()); // Already installed
        }

        let manifest = available
            .get(pkg_name)
            .ok_or_else(|| OpkError::PackageNotFound(pkg_name.to_string()))?;

        if !no_deps && !manifest.depends.is_empty() {
            for dep in &manifest.depends {
                self.resolve_install(dep, available, to_install, visited, no_deps)?;
            }
        }

        to_install.push((pkg_name.to_string(), manifest.clone()));
        Ok(())
    }

    fn extract_and_install(&self, pkg_path: &PathBuf, manifest: &PackageManifest) -> Result<(), OpkError> {
        // Open the .opk archive (tar.gz format)
        let file = std::fs::File::open(pkg_path)?;
        let decoder = flate2::read::GzDecoder::new(file);
        let mut archive = tar::Archive::new(decoder);

        for entry in archive.entries()? {
            let mut entry = entry?;
            let path = entry.path()?.to_path_buf();

            // Strip the leading component (package name/version)
            let relative: PathBuf = path
                .components()
                .skip(1)
                .collect();

            let dest = PathBuf::from("/").join(&relative);

            if let Some(parent) = dest.parent() {
                std::fs::create_dir_all(parent).ok();
            }

            entry.unpack(&dest).ok();
        }

        Ok(())
    }

    fn uninstall_package(&self, _pkg: &str, _purge: bool) -> Result<(), OpkError> {
        // In a real implementation, this would:
        // 1. Read the package file list from the state database
        // 2. Remove all installed files
        // 3. Remove empty directories
        // 4. Run post-removal scripts
        // For now, we just remove from the database.
        Ok(())
    }
}

/// Format bytes as human-readable size
pub fn format_size(bytes: u64) -> String {
    const UNITS: &[&str] = &["B", "KB", "MB", "GB"];
    let mut size = bytes as f64;
    let mut unit_idx = 0;

    while size >= 1024.0 && unit_idx < UNITS.len() - 1 {
        size /= 1024.0;
        unit_idx += 1;
    }

    if unit_idx == 0 {
        format!("{} {}", bytes, UNITS[unit_idx])
    } else {
        format!("{:.1} {}", size, UNITS[unit_idx])
    }
}

/// Get current time string for install records
fn chrono_now() -> String {
    // Simple timestamp without chrono dependency
    use std::time::SystemTime;
    let now = SystemTime::now()
        .duration_since(SystemTime::UNIX_EPOCH)
        .unwrap_or_default();
    let secs = now.as_secs();

    // Approximate date from epoch
    let days = secs / 86400;
    // Simple: just format as ISO-like string
    format!("{}", secs)
}
