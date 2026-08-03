/// 仓库管理器 — OPT/APT 双通道仓库管理
///
/// 支持两种仓库类型:
///   - opt: 原生 .opt 仓库，通过 Packages.json 索引
///   - apt: Debian/Ubuntu APT 仓库，通过 Packages.gz 索引
///
/// 配置存储路径: ~/.config/opt/config.toml
/// 缓存路径:     ~/.cache/opt/

use crate::error::{OptError, OptResult};
use crate::format::PackageManifest;
use serde::{Deserialize, Serialize};
use std::fs;
use std::path::PathBuf;

// ──────────────────────────────────────────────
// 仓库类型
// ──────────────────────────────────────────────

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq)]
#[serde(rename_all = "lowercase")]
pub enum RepoType {
    /// 原生 OPT 仓库 (通过 Packages.json)
    Opt,
    /// Debian/Ubuntu APT 仓库 (通过 Packages.gz)
    Apt,
}

// ──────────────────────────────────────────────
// 仓库配置
// ──────────────────────────────────────────────

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct RepoConfig {
    /// 仓库名称（唯一标识）
    pub name: String,

    /// 仓库类型: "opt" 或 "apt"
    #[serde(rename = "type")]
    pub repo_type: RepoType,

    /// 仓库 URL（OPT: 根目录，APT: mirror URL）
    pub url: String,

    /// APT 特有: 发行版代号 (bookworm, noble 等)
    #[serde(default)]
    pub suite: String,

    /// APT 特有: 组件列表 (main, contrib, non-free)
    #[serde(default)]
    pub components: Vec<String>,

    /// APT 特有: 架构 (amd64, arm64)
    #[serde(default = "default_arch")]
    pub architectures: Vec<String>,

    /// 是否启用
    #[serde(default = "default_enabled")]
    pub enabled: bool,
}

fn default_arch() -> Vec<String> {
    vec!["amd64".to_string()]
}
fn default_enabled() -> bool {
    true
}

// ──────────────────────────────────────────────
// 全局配置
// ──────────────────────────────────────────────

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct OptConfig {
    #[serde(default)]
    pub repos: Vec<RepoConfig>,

    /// 是否自动分享转换后的包到官方仓库
    #[serde(default)]
    pub contribute: bool,

    /// 是否启动时自动扫描本地 .opt 并分享独有包
    #[serde(default)]
    pub auto_scan: bool,

    /// 本地 openos-repo 仓库路径
    #[serde(default = "default_repo_path")]
    pub repo_path: String,
}

fn default_repo_path() -> String {
    let home = dirs::home_dir()
        .unwrap_or_else(|| std::path::PathBuf::from("~"));
    home.join("CODE/openos-repo").to_string_lossy().to_string()
}

impl Default for OptConfig {
    fn default() -> Self {
        OptConfig {
            repos: vec![],
            contribute: false,
            auto_scan: false,
            repo_path: default_repo_path(),
        }
    }
}

// ──────────────────────────────────────────────
// 包搜索索引
// ──────────────────────────────────────────────

/// 一个包在索引中的记录（合并 OPT 和 APT 来源）
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PackageEntry {
    pub name: String,
    pub version: String,
    pub description: String,
    pub section: String,
    pub architecture: String,
    pub repo_name: String,
    pub repo_type: RepoType,
    pub filename: Option<String>,
    pub depends: Vec<String>,
    pub provides: Vec<String>,
    pub installed_size: u64,
}

impl PackageEntry {
    pub fn to_manifest(&self) -> PackageManifest {
        let mut m = PackageManifest {
            name: self.name.clone(),
            version: self.version.clone(),
            description: self.description.clone(),
            section: self.section.clone(),
            maintainer: String::new(),
            homepage: String::new(),
            license: String::new(),
            architecture: self.architecture.clone(),
            package_size: 0,
            installed_size: self.installed_size,
            filename: self.filename.clone().unwrap_or_default(),
            sha256: String::new(),
            depends: self.depends.clone(),
            recommends: vec![],
            suggests: vec![],
            provides: self.provides.clone(),
            conflicts: vec![],
            replaces: vec![],
            priority: "optional".to_string(),
            tags: vec![],
            origin: self.repo_name.clone(),
            desktop_file: String::new(),
            appstream_id: String::new(),
            categories: vec![],
            screenshots: vec![],
            app_type: crate::format::AppType::Cli,
            runtime: crate::format::Runtime::Native,
            min_kernel: String::new(),
            chromium_features: vec![],
            permissions: vec![],
            xterm: false,
            schema: None,
        };
        if self.repo_type == RepoType::Apt {
            m.runtime = crate::format::Runtime::Crostini;
        }
        m
    }
}

// ──────────────────────────────────────────────
// 仓库管理器
// ──────────────────────────────────────────────

pub struct RepoManager {
    /// 配置路径
    config_path: PathBuf,
    /// 缓存路径
    cache_dir: PathBuf,
    /// 当前配置
    config: OptConfig,
    /// 搜索索引（所有仓库合并）
    index: Vec<PackageEntry>,
    /// 索引是否已加载
    index_loaded: bool,
}

impl RepoManager {
    /// 创建仓库管理器
    pub fn new() -> Self {
        let config_dir = dirs::config_dir()
            .unwrap_or_else(|| PathBuf::from("~/.config"))
            .join("opt");
        let cache_dir = dirs::cache_dir()
            .unwrap_or_else(|| PathBuf::from("~/.cache"))
            .join("opt");
        let config_path = config_dir.join("config.toml");

        let is_new_config = !config_path.exists();
        let mut config = if config_path.exists() {
            fs::read_to_string(&config_path)
                .ok()
                .and_then(|s| toml::from_str(&s).ok())
                .unwrap_or_default()
        } else {
            OptConfig::default()
        };

        // 首次运行：自动添加官方源
        if is_new_config || config.repos.is_empty() {
            // OPENOS 官方 OPT 源
            let opt_repo = RepoConfig {
                name: "openos-community".to_string(),
                repo_type: RepoType::Opt,
                url: "https://open-code-studio.github.io/openos-repo".to_string(),
                suite: String::new(),
                components: vec![],
                architectures: vec!["amd64".to_string()],
                enabled: true,
            };

            // Debian 官方 APT 源（bookworm，使用国内镜像）
            let apt_repo = RepoConfig {
                name: "debian-bookworm".to_string(),
                repo_type: RepoType::Apt,
                url: "https://mirrors.tuna.tsinghua.edu.cn/debian".to_string(),
                suite: "bookworm".to_string(),
                components: vec!["main".to_string(), "contrib".to_string(), "non-free".to_string(), "non-free-firmware".to_string()],
                architectures: vec!["amd64".to_string()],
                enabled: true,
            };

            for default_repo in [opt_repo, apt_repo] {
                let name = default_repo.name.clone();
                if !config.repos.iter().any(|r| r.name == name) {
                    config.repos.push(default_repo);
                }
            }
            // 保存配置
            if let Some(parent) = config_path.parent() {
                let _ = fs::create_dir_all(parent);
            }
            if let Ok(toml_str) = toml::to_string_pretty(&config) {
                let _ = fs::write(&config_path, toml_str);
            }
        }

        RepoManager {
            config_path,
            cache_dir,
            config,
            index: vec![],
            index_loaded: false,
        }
    }

    /// 获取仓库列表
    pub fn list_repos(&self) -> &[RepoConfig] {
        &self.config.repos
    }

    /// 是否开启分享模式
    pub fn is_contribute_enabled(&self) -> bool {
        self.config.contribute
    }

    pub fn is_auto_scan_enabled(&self) -> bool {
        self.config.auto_scan
    }

    /// 获取本地仓库路径
    pub fn repo_path(&self) -> &str {
        &self.config.repo_path
    }

    /// 设置配置值
    pub fn set_config(&mut self, key: &str, value: &str) -> OptResult<()> {
        match key {
            "contribute" => {
                self.config.contribute = value == "true" || value == "1" || value == "yes";
                self.save_config()?;
                Ok(())
            }
            "auto-scan" => {
                self.config.auto_scan = value == "true" || value == "1" || value == "yes";
                self.save_config()?;
                Ok(())
            }
            "repo-path" => {
                self.config.repo_path = value.to_string();
                self.save_config()?;
                Ok(())
            }
            _ => Err(OptError::General(format!("未知配置项: {key}"))),
        }
    }

    /// 获取配置值
    pub fn get_config(&self, key: &str) -> Option<String> {
        match key {
            "contribute" => Some(if self.config.contribute { "true" } else { "false" }.to_string()),
            "auto-scan" => Some(if self.config.auto_scan { "true" } else { "false" }.to_string()),
            "repo-path" => Some(self.config.repo_path.clone()),
            _ => None,
        }
    }

    /// 列出所有配置
    pub fn list_config(&self) -> Vec<(&str, String)> {
        vec![
            ("contribute", if self.config.contribute { "true" } else { "false" }.to_string()),
            ("auto-scan", if self.config.auto_scan { "true" } else { "false" }.to_string()),
            ("repo-path", self.config.repo_path.clone()),
        ]
    }

    /// 本地 .opt 文件的扫描结果
    fn scan_local_pool(&self) -> Vec<PackageEntry> {
        let pool_dir = std::path::Path::new(&self.config.repo_path).join("pool");
        if !pool_dir.exists() { return vec![]; }

        let mut entries = vec![];
        if let Ok(dir) = std::fs::read_dir(&pool_dir) {
            for entry in dir.flatten() {
                let path = entry.path();
                if path.extension().map(|e| e == "opt").unwrap_or(false) {
                    let filename = path.file_name()
                        .and_then(|s| s.to_str())
                        .unwrap_or("")
                        .to_string();
                    let name_stem = filename.replace(".opt", "");
                    let parts: Vec<&str> = name_stem.splitn(3, '_').collect();
                    let name = parts.first().unwrap_or(&"unknown").to_string();
                    let version = parts.get(1).unwrap_or(&"0.0.0").to_string();
                    let arch = parts.get(2).unwrap_or(&"amd64").to_string();
                    let size = path.metadata().map(|m| m.len()).unwrap_or(0);

                    entries.push(PackageEntry {
                        name,
                        version,
                        description: format!("本地 .opt 包 — {filename}"),
                        section: "local".to_string(),
                        architecture: arch,
                        repo_name: "local-pool".to_string(),
                        repo_type: RepoType::Opt,
                        filename: Some(filename.clone()),
                        depends: vec![],
                        provides: vec![],
                        installed_size: size,
                    });
                }
            }
        }
        entries
    }

    /// 分享包到官方仓库：复制 .opt → 重建索引 → git commit
    pub fn share_package(&mut self, opt_path: &str) -> OptResult<()> {
        if !self.config.contribute {
            return Err(OptError::General(
                "分享功能未开启。运行 'opt config set contribute true' 开启。".into()
            ));
        }

        let repo_dir = std::path::Path::new(&self.config.repo_path);
        if !repo_dir.exists() {
            return Err(OptError::General(format!(
                "本地仓库 '{}' 不存在。请先 clone:\n  git clone https://github.com/Open-code-Studio/openos-repo.git {}",
                repo_dir.display(), repo_dir.display()
            )));
        }

        let opt_file = std::path::Path::new(opt_path);
        let filename = opt_file.file_name()
            .and_then(|s| s.to_str())
            .ok_or_else(|| OptError::General("无效的 .opt 文件名".into()))?;

        // 复制到 pool/
        let pool_dir = repo_dir.join("pool");
        std::fs::create_dir_all(&pool_dir)?;
        std::fs::copy(opt_path, pool_dir.join(filename))?;
        eprintln!("  📋 已复制到仓库: pool/{filename}");

        // 重建索引
        let script_path = repo_dir.join("scripts/build-index.sh");
        if script_path.exists() {
            let status = std::process::Command::new("bash")
                .arg(&script_path)
                .current_dir(repo_dir)
                .status()
                .map_err(|e| OptError::General(format!("运行 build-index.sh 失败: {e}")))?;
            if !status.success() {
                return Err(OptError::General("索引重建失败".into()));
            }
        }

        // git add + commit
        let git_add = std::process::Command::new("git")
            .args(["add", "-A"])
            .current_dir(repo_dir)
            .status()
            .map_err(|e| OptError::General(format!("git add 失败: {e}")))?;

        if !git_add.success() {
            return Err(OptError::General("git add 失败，仓库可能未初始化".into()));
        }

        let commit_msg = format!("add: {} (自动分享)", filename);
        let git_commit = std::process::Command::new("git")
            .args(["commit", "-m", &commit_msg])
            .current_dir(repo_dir)
            .status()
            .map_err(|e| OptError::General(format!("git commit 失败: {e}")))?;

        if git_commit.success() {
            eprintln!("  ✅ 已提交: {commit_msg}");

            // 检查 gh CLI 是否可用
            let has_gh = std::process::Command::new("gh")
                .args(["--version"])
                .output()
                .map(|o| o.status.success())
                .unwrap_or(false);

            // 推送到 origin
            match std::process::Command::new("git")
                .args(["push"])
                .current_dir(repo_dir)
                .status()
            {
                Ok(status) if status.success() => {
                    eprintln!("  🚀 已推送到 GitHub");
                    if has_gh {
                        // 用 gh 创建 PR
                        let pr_body = format!(
                            "## 自动分享\n\n包文件: `{}`\n\n由 opt 自动转换并提交。",
                            filename
                        );
                        let pr_title = format!("add: {} (自动分享)", filename);
                        match std::process::Command::new("gh")
                            .args(["pr", "create", "--title", &pr_title, "--body", &pr_body])
                            .current_dir(repo_dir)
                            .output()
                        {
                            Ok(out) if out.status.success() => {
                                let url = String::from_utf8_lossy(&out.stdout);
                                eprintln!("  📬 PR 已创建: {}", url.trim());
                            }
                            _ => {
                                eprintln!("  ⚠ gh PR 创建失败，请手动创建 PR");
                            }
                        }
                    } else {
                        eprintln!("  💡 安装 gh CLI 可自动创建 PR: brew install gh");
                    }
                }
                _ => {
                    eprintln!("  ⚠ 推送失败");
                }
            }
        } else {
            eprintln!("  ⚠ 无变更或 commit 失败");
        }

        Ok(())
    }

    /// 自动扫描本地 pool 中的 .opt 文件，将 APT 已有的包分享到仓库
    pub fn auto_scan_and_share(&mut self) {
        if !self.config.auto_scan || !self.config.contribute {
            return;
        }

        let pool_dir = std::path::Path::new(&self.config.repo_path).join("pool");
        if !pool_dir.exists() { return; }

        // 先构建索引
        let _ = self.build_index();

        let pool_files: Vec<_> = std::fs::read_dir(&pool_dir)
            .map(|d| d.flatten()
                .filter(|e| e.path().extension().map(|x| x == "opt").unwrap_or(false))
                .map(|e| e.path())
                .collect())
            .unwrap_or_default();

        if pool_files.is_empty() { return; }

        eprintln!("\n🔍 正在扫描本地 .opt 文件...");
        let mut shared = 0;

        for opt_path in &pool_files {
            let filename = opt_path.file_name()
                .and_then(|s| s.to_str())
                .unwrap_or("");

            // 从文件名提取包名
            let pkg_name = filename.replace(".opt", "")
                .splitn(3, '_')
                .next()
                .unwrap_or("")
                .to_string();

            if pkg_name.is_empty() { continue; }

            // 检查是否已在 OPT 索引中
            if self.index.iter().any(|p| p.name == pkg_name && p.repo_type == RepoType::Opt) {
                continue; // 已分享过，跳过
            }

            // 检查 APT 是否有此包
            let apt_exists = self.index.iter()
                .any(|p| p.name == pkg_name && p.repo_type == RepoType::Apt);

            if !apt_exists {
                continue; // APT 没有 → 不上传（可能是自制包）
            }

            // APT 有且 OPT 没有 → 分享
            eprintln!("  📤 分享 {} (APT 已有，转为 .opt)...", filename);
            if let Err(e) = self.share_package(&opt_path.to_string_lossy()) {
                eprintln!("  ⚠ 分享失败: {e}");
            } else {
                shared += 1;
            }
        }

        if shared > 0 {
            eprintln!("  ✅ 已分享 {shared} 个包到仓库");
        }
    }

    /// 添加仓库
    pub fn add_repo(&mut self, repo: RepoConfig) -> OptResult<()> {
        // 检查重名
        if self.config.repos.iter().any(|r| r.name == repo.name) {
            return Err(OptError::General(format!(
                "仓库 '{}' 已存在",
                repo.name
            )));
        }
        self.config.repos.push(repo);
        self.save_config()?;
        Ok(())
    }

    /// 移除仓库
    pub fn remove_repo(&mut self, name: &str) -> OptResult<()> {
        let len = self.config.repos.len();
        self.config.repos.retain(|r| r.name != name);
        if self.config.repos.len() == len {
            return Err(OptError::NotFound(format!("仓库 '{}' 未找到", name)));
        }
        self.save_config()?;
        self.index_loaded = false;
        Ok(())
    }

    /// 更新所有仓库的索引（下载 Packages.json / Packages.gz）
    pub fn update_all(&mut self) -> OptResult<Vec<String>> {
        let mut results = vec![];
        fs::create_dir_all(&self.cache_dir)?;

        for repo in &self.config.repos.clone() {
            if !repo.enabled {
                continue;
            }
            match repo.repo_type {
                RepoType::Opt => results.push(self.update_opt_repo(repo)?),
                RepoType::Apt => results.push(self.update_apt_repo(repo)?),
            }
        }

        // 更新后重载索引
        self.index_loaded = false;
        self.build_index()?;

        Ok(results)
    }

    /// 构建搜索索引（从缓存读取并合并所有仓库）
    pub fn build_index(&mut self) -> OptResult<usize> {
        self.index.clear();
        let mut total = 0;

        for repo in &self.config.repos {
            if !repo.enabled {
                continue;
            }
            let cache_file = self.cache_dir.join(format!("{}.json", repo.name));
            if !cache_file.exists() {
                continue;
            }
            let content = fs::read_to_string(&cache_file)
                .map_err(|e| OptError::Io(e))?;
            let entries: Vec<PackageEntry> = serde_json::from_str(&content)
                .map_err(|e| OptError::Json(e))?;
            total += entries.len();
            self.index.extend(entries);
        }

        // 自动扫描本地 pool 目录中的 .opt 文件
        let local_entries = self.scan_local_pool();
        if !local_entries.is_empty() {
            total += local_entries.len();
            self.index.extend(local_entries);
        }

        self.index_loaded = true;
        Ok(total)
    }

    /// 搜索包
    pub fn search(&mut self, query: &str, search_all: bool) -> OptResult<Vec<PackageEntry>> {
        if !self.index_loaded {
            self.build_index()?;
        }

        let query_lower = query.to_lowercase();
        let mut results: Vec<PackageEntry> = self
            .index
            .iter()
            .filter(|p| {
                let name_match = p.name.to_lowercase().contains(&query_lower);
                let desc_match = search_all
                    && p.description.to_lowercase().contains(&query_lower);
                name_match || desc_match
            })
            .cloned()
            .collect();

        // 按来源优先级排序: OPT → local → APT
        results.sort_by(|a, b| {
            let priority = |r: &RepoType| match r {
                RepoType::Opt => 0,
                _ => 1,
            };
            let a_local = a.repo_name == "local-pool";
            let b_local = b.repo_name == "local-pool";
            let a_prio = if a_local { 1 } else { priority(&a.repo_type) };
            let b_prio = if b_local { 1 } else { priority(&b.repo_type) };
            a_prio.cmp(&b_prio).then(a.name.cmp(&b.name))
        });

        Ok(results)
    }

    /// 获取包详情（优先返回 OPT 源）
    pub fn info(&mut self, name: &str) -> OptResult<PackageEntry> {
        if !self.index_loaded {
            self.build_index()?;
        }

        // 先找 OPT 源
        for entry in &self.index {
            if entry.name == name && entry.repo_type == RepoType::Opt {
                return Ok(entry.clone());
            }
        }

        // 再找 APT 源
        self.index
            .iter()
            .find(|p| p.name == name)
            .cloned()
            .ok_or_else(|| OptError::NotFound(format!("包 '{}' 未找到", name)))
    }

    /// 获取包下载 URL
    pub fn get_download_url(&self, entry: &PackageEntry) -> Option<String> {
        let repo = self.config.repos.iter().find(|r| r.name == entry.repo_name)?;
        let filename = entry.filename.as_ref()?;
        Some(match repo.repo_type {
            RepoType::Opt => format!("{}/{}", repo.url.trim_end_matches('/'), filename),
            RepoType::Apt => {
                let comp = entry.section.split('/').next().unwrap_or("main");
                format!(
                    "{}/dists/{}/{}/binary-{}/{}",
                    repo.url.trim_end_matches('/'),
                    repo.suite,
                    comp,
                    repo.architectures.first().map(|s| s.as_str()).unwrap_or("amd64"),
                    filename
                )
            }
        })
    }

    // ── 私有方法 ──

    /// 更新 OPT 仓库缓存
    fn update_opt_repo(&self, repo: &RepoConfig) -> OptResult<String> {
        let index_url = format!("{}/Packages.json", repo.url.trim_end_matches('/'));
        eprintln!("  ⟳ 正在下载 {}...", index_url);

        let response = ureq::get(&index_url)
            .call()
            .map_err(|e| OptError::Network(format!("无法下载索引: {e}")))?;

        let index: crate::format::RepositoryIndex = response
            .into_json()
            .map_err(|e| OptError::Network(format!("索引格式错误: {e}")))?;

        // 转换为统一 PackageEntry 格式
        let entries: Vec<PackageEntry> = index
            .packages
            .iter()
            .map(|m| PackageEntry {
                name: m.name.clone(),
                version: m.version.clone(),
                description: m.description.clone(),
                section: m.section.clone(),
                architecture: m.architecture.clone(),
                repo_name: repo.name.clone(),
                repo_type: RepoType::Opt,
                filename: Some(m.filename.clone()),
                depends: m.depends.clone(),
                provides: m.provides.clone(),
                installed_size: m.installed_size,
            })
            .collect();

        // 写入缓存
        let cache_file = self.cache_dir.join(format!("{}.json", repo.name));
        fs::write(&cache_file, serde_json::to_string(&entries)?)?;

        Ok(format!("{}: {} 个包已缓存", repo.name, entries.len()))
    }

    /// 更新 APT 仓库缓存
    fn update_apt_repo(&self, repo: &RepoConfig) -> OptResult<String> {
        use std::io::Read;
        let arch = repo.architectures.first().map(|s| s.as_str()).unwrap_or("amd64");
        let mut all_entries = vec![];

        for component in &repo.components {
            let packages_url = format!(
                "{}/dists/{}/{}/binary-{}/Packages.gz",
                repo.url.trim_end_matches('/'),
                repo.suite,
                component,
                arch
            );
            eprintln!("  ⟳ 正在下载 {}...", packages_url);

            let response = ureq::get(&packages_url)
                .call()
                .map_err(|e| {
                    OptError::Network(format!("无法下载 APT 索引 {component}: {e}"))
                })?;

            // 解压 gz
            let mut body_reader = response.into_reader();
            let mut compressed = Vec::new();
            body_reader.read_to_end(&mut compressed)
                .map_err(|e| OptError::Network(format!("读取失败: {e}")))?;
            let mut decoder = flate2::read::GzDecoder::new(&compressed[..]);
            let mut decompressed = Vec::new();
            std::io::copy(&mut decoder, &mut decompressed)
                .map_err(|e| OptError::Network(format!("解压失败: {e}")))?;

            // 解析 Packages 格式
            let entries = parse_apt_packages(
                &String::from_utf8_lossy(&decompressed),
                &repo.name,
                component,
            );
            all_entries.extend(entries);
        }

        let cache_file = self.cache_dir.join(format!("{}.json", repo.name));
        fs::write(&cache_file, serde_json::to_string(&all_entries)?)?;

        Ok(format!("{}: {} 个包已缓存", repo.name, all_entries.len()))
    }

    /// 保存配置到文件
    fn save_config(&self) -> OptResult<()> {
        if let Some(parent) = self.config_path.parent() {
            fs::create_dir_all(parent)?;
        }
        let toml_str = toml::to_string_pretty(&self.config)
            .map_err(|e| OptError::General(format!("配置序列化失败: {e}")))?;
        fs::write(&self.config_path, toml_str)?;
        Ok(())
    }
}

// ──────────────────────────────────────────────
// APT Packages.gz 解析器
// ──────────────────────────────────────────────

/// 解析 APT 的 Packages 文件格式（Stanza 格式）
///
/// 每段用空行分隔，字段格式: "Key: Value"
/// ```
/// Package: firefox
/// Version: 120.0
/// Architecture: amd64
/// Description: Mozilla Firefox
/// ...
/// ```
fn parse_apt_packages(content: &str, repo_name: &str, component: &str) -> Vec<PackageEntry> {
    let mut entries = vec![];

    // 按空行分割段落
    for stanza in content.split("\n\n") {
        let stanza = stanza.trim();
        if stanza.is_empty() {
            continue;
        }

        let mut fields: std::collections::HashMap<String, String> = std::collections::HashMap::new();

        for line in stanza.lines() {
            if let Some((key, value)) = line.split_once(':') {
                let key = key.trim().to_string();
                let value = value.trim().to_string();
                // 处理多行值（以空格开头的续行）
                fields.insert(key, value);
            }
        }

        let name = match fields.get("Package") {
            Some(n) => n.clone(),
            None => continue,
        };

        let version = fields.get("Version").cloned().unwrap_or_default();
        let description = fields
            .get("Description")
            .cloned()
            .unwrap_or_default()
            .lines()
            .next()
            .unwrap_or("")
            .to_string();
        let arch = fields
            .get("Architecture")
            .cloned()
            .unwrap_or_else(|| "amd64".to_string());
        let section = fields
            .get("Section")
            .cloned()
            .unwrap_or_default();
        let section = format!("{}/{}", component, section);
        let size = fields
            .get("Installed-Size")
            .and_then(|s| s.parse::<u64>().ok())
            .unwrap_or(0);
        let filename = fields.get("Filename").cloned();
        let depends = fields
            .get("Depends")
            .map(|d| parse_apt_deps(d))
            .unwrap_or_default();
        let provides = fields
            .get("Provides")
            .map(|d| parse_apt_deps(d))
            .unwrap_or_default();

        entries.push(PackageEntry {
            name,
            version,
            description,
            section,
            architecture: arch,
            repo_name: repo_name.to_string(),
            repo_type: RepoType::Apt,
            filename,
            depends,
            provides,
            installed_size: size * 1024, // APT 的 Installed-Size 单位是 KB
        });
    }

    entries
}

/// 解析 APT 依赖字符串
/// "libc6 (>= 2.34), firefox-esr | firefox" → ["libc6", "firefox-esr"]
fn parse_apt_deps(deps: &str) -> Vec<String> {
    deps.split(',')
        .map(|part| {
            part.split('|')
                .next()
                .unwrap_or("")
                .trim()
                .split_whitespace()
                .next()
                .unwrap_or("")
                .trim_matches(|c: char| c == '(' || c == ')' || c == ' ')
                .to_string()
        })
        .filter(|s| !s.is_empty() && !s.starts_with('('))
        .collect()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_parse_apt_deps() {
        let deps = parse_apt_deps("libc6 (>= 2.34), firefox-esr | firefox, libgtk-3-0");
        assert_eq!(deps, vec!["libc6", "firefox-esr", "libgtk-3-0"]);
    }

    #[test]
    fn test_parse_apt_packages_basic() {
        let content = "Package: firefox\n\
                       Version: 120.0\n\
                       Architecture: amd64\n\
                       Section: web\n\
                       Description: Mozilla Firefox web browser\n\
                       Installed-Size: 150000\n\
                       Filename: pool/main/f/firefox/firefox_120.0_amd64.deb\n\n\
                       Package: vlc\n\
                       Version: 3.0.20\n\
                       Architecture: amd64\n\
                       Description: VLC media player\n";

        let entries = parse_apt_packages(content, "debian", "main");
        assert_eq!(entries.len(), 2);
        assert_eq!(entries[0].name, "firefox");
        assert_eq!(entries[0].version, "120.0");
        assert_eq!(entries[0].installed_size, 150000 * 1024);
        assert_eq!(entries[1].name, "vlc");
    }
}
