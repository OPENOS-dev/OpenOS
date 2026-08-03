// OPK Repository Client
//
// Handles communication with OPK package repositories.
// Fetches repository indexes, downloads packages, and verifies checksums.
//
// Protocol:
//   GET ${repo_url}/Packages.json          → Repository index
//   GET ${repo_url}/pool/${name}/${file}   → Package file

use crate::config::Repository;
use crate::error::OpkError;
use crate::format::{PackageManifest, RepositoryIndex};
use std::collections::HashMap;
use std::path::{Path, PathBuf};

pub struct RepositoryClient<'a> {
    repos: &'a [Repository],
    cache_dir: &'a Path,
}

impl<'a> RepositoryClient<'a> {
    pub fn new(repos: &'a [Repository], cache_dir: &'a Path) -> Self {
        Self { repos, cache_dir }
    }

    /// Fetch all available packages from all enabled repositories
    pub fn fetch_available_packages(&self) -> Result<HashMap<String, PackageManifest>, OpkError> {
        let mut all_packages = HashMap::new();

        for repo in self.repos {
            if !repo.enabled {
                continue;
            }

            let index = self.fetch_repo_index(repo)?;
            for pkg in &index.packages {
                // Later repos override earlier ones (like apt preferences)
                all_packages.insert(pkg.name.clone(), pkg.clone());
            }
        }

        Ok(all_packages)
    }

    /// Fetch a single repository's package index
    pub fn fetch_repo_index(&self, repo: &Repository) -> Result<Vec<PackageManifest>, OpkError> {
        let index_url = format!("{}/Packages.json", repo.url.trim_end_matches('/'));
        let cache_key = format!("{}.idx", repo.name.replace('/', "_"));
        let cache_path = self.cache_dir.join(&cache_key);

        // Try cached index first (valid for 1 hour)
        if let Ok(meta) = std::fs::metadata(&cache_path) {
            if let Ok(modified) = meta.modified() {
                if let Ok(elapsed) = modified.elapsed() {
                    if elapsed.as_secs() < 3600 {
                        let content = std::fs::read_to_string(&cache_path)?;
                        let index: RepositoryIndex = serde_json::from_str(&content)?;
                        return Ok(index.packages);
                    }
                }
            }
        }

        // Fetch fresh index
        let response: RepositoryIndex = ureq::get(&index_url)
            .set("User-Agent", "OPK/0.1.0 (Open-OS)")
            .call()?
            .into_json()?;

        // Cache the index
        let content = serde_json::to_string(&response)?;
        std::fs::write(&cache_path, content)?;

        Ok(response.packages)
    }

    /// Download a package file from the repository
    pub fn download_package<F>(&self, manifest: &PackageManifest, progress: F) -> Result<(), OpkError>
    where
        F: Fn(u64),
    {
        // Try each repository
        for repo in self.repos {
            if !repo.enabled {
                continue;
            }

            let pkg_url = format!(
                "{}/pool/{}/{}",
                repo.url.trim_end_matches('/'),
                manifest.name.chars().next().unwrap_or('_'),
                manifest.filename,
            );

            let cache_path = self.cache_dir.join("archives").join(&manifest.filename);

            // Check if already cached
            if cache_path.exists() {
                if self.verify_checksum(&cache_path, &manifest.sha256)? {
                    return Ok(());
                }
                // Corrupted cache, re-download
                std::fs::remove_file(&cache_path).ok();
            }

            std::fs::create_dir_all(cache_path.parent().unwrap())?;

            let response = match ureq::get(&pkg_url)
                .set("User-Agent", "OPK/0.1.0 (Open-OS)")
                .call()
            {
                Ok(r) => r,
                Err(_) => continue, // Try next repo
            };

            let total_size: u64 = response
                .header("Content-Length")
                .and_then(|v| v.parse().ok())
                .unwrap_or(manifest.package_size);

            let mut reader = response.into_reader();
            let mut downloaded: u64 = 0;
            let mut buf = [0u8; 8192];
            let mut file = std::fs::File::create(&cache_path)?;

            loop {
                let n = std::io::Read::read(&mut reader, &mut buf)?;
                if n == 0 {
                    break;
                }
                std::io::Write::write_all(&mut file, &buf[..n])?;
                downloaded += n as u64;
                progress(downloaded.min(total_size));
            }

            // Verify checksum
            if self.verify_checksum(&cache_path, &manifest.sha256)? {
                return Ok(());
            }

            std::fs::remove_file(&cache_path).ok();
            return Err(OpkError::ChecksumMismatch(manifest.name.clone()));
        }

        Err(OpkError::DownloadFailed(
            manifest.name.clone(),
            "No repository had the package".to_string(),
        ))
    }

    /// Download source tarball
    pub fn download_source(&self, manifest: &PackageManifest, dest: &Path) -> Result<(), OpkError> {
        for repo in self.repos {
            if !repo.enabled {
                continue;
            }

            let src_url = format!(
                "{}/source/{}/{}_{}.src.tar.gz",
                repo.url.trim_end_matches('/'),
                manifest.name.chars().next().unwrap_or('_'),
                manifest.name,
                manifest.version,
            );

            match ureq::get(&src_url)
                .set("User-Agent", "OPK/0.1.0 (Open-OS)")
                .call()
            {
                Ok(response) => {
                    let mut file = std::fs::File::create(dest)?;
                    std::io::copy(&mut response.into_reader(), &mut file)?;
                    return Ok(());
                }
                Err(_) => continue,
            }
        }

        Err(OpkError::DownloadFailed(
            manifest.name.clone(),
            "Source not found in any repository".to_string(),
        ))
    }

    /// Verify SHA-256 checksum of a downloaded file
    fn verify_checksum(&self, path: &Path, expected: &str) -> Result<bool, OpkError> {
        use sha2::{Digest, Sha256};
        let mut file = std::fs::File::open(path)?;
        let mut hasher = Sha256::new();
        std::io::copy(&mut file, &mut hasher)?;
        let hash = format!("{:x}", hasher.finalize());
        Ok(hash == expected)
    }
}
