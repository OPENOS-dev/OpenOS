use clap::{Parser, Subcommand};

/// OPENOS 桌面包管理器
///
/// 管理 .opt 包，用于 OPENOS 桌面环境的应用安装与管理。
/// 提供类 apt 的命令行接口，支持安装、移除、构建、验证等操作。
///
/// 示例:
///   opt install firefox.opt           安装本地的 .opt 包
///   opt build ./my-app/               从应用目录构建 .opt 包
///   opt check firefox.opt             验证 .opt 包的完整性
///   opt list                          列出已安装的包
#[derive(Parser)]
#[command(
    name = "opt",
    version = env!("CARGO_PKG_VERSION"),
    about = "OPENOS 桌面包管理器",
    long_about = None,
    after_help = "运行 'opt help <command>' 查看具体命令的帮助。",
    disable_help_subcommand = true,
)]
pub struct Cli {
    #[command(subcommand)]
    pub command: Command,
}

#[derive(Subcommand)]
pub enum Command {
    /// 安装一个 .opt 包
    ///
    /// 从本地 .opt 文件或已配置的仓库中安装包。
    /// 自动解析依赖关系。
    ///
    /// 示例:
    ///   opt install firefox.opt            安装本地的 .opt 文件
    ///   opt install firefox                从仓库安装
    ///   opt install -y firefox.opt         跳过确认直接安装
    #[command(visible_alias = "i")]
    Install {
        /// 包名或 .opt 文件路径
        #[arg(required = true)]
        package: String,

        /// 跳过所有确认提示
        #[arg(short = 'y', long)]
        yes: bool,

        /// 跳过依赖解析（只安装指定的包）
        #[arg(long)]
        no_deps: bool,
    },

    /// 移除已安装的包
    ///
    /// 删除包及其文件。使用 --purge 同时删除配置文件。
    ///
    /// 示例:
    ///   opt remove firefox
    ///   opt remove --purge firefox
    #[command(visible_alias = "rm")]
    Remove {
        /// 要移除的包名
        #[arg(required = true)]
        package: String,

        /// 同时删除配置文件
        #[arg(short = 'P', long)]
        purge: bool,

        /// 跳过所有确认提示
        #[arg(short = 'y', long)]
        yes: bool,
    },

    /// 从仓库更新包列表
    ///
    /// 从所有已配置的仓库获取最新的包元数据。
    /// 在 opt upgrade 之前运行。
    #[command(visible_alias = "u")]
    Update,

    /// 升级所有已安装的包到最新版本
    ///
    /// 安全地解析依赖并升级包。
    #[command(visible_alias = "up")]
    Upgrade {
        /// 跳过所有确认提示
        #[arg(short = 'y', long)]
        yes: bool,

        /// 仅预览，不实际执行
        #[arg(long)]
        dry_run: bool,
    },

    /// 搜索包
    ///
    /// 在所有已配置的仓库中搜索。
    #[command(visible_alias = "s")]
    Search {
        /// 搜索关键词
        #[arg(required = true)]
        query: String,

        /// 同时在描述和标签中搜索
        #[arg(short = 'a', long)]
        all: bool,
    },

    /// 显示包的详细信息
    #[command(visible_alias = "show")]
    Info {
        /// 包名
        #[arg(required = true)]
        package: String,
    },

    /// 列出已安装的包
    #[command(visible_alias = "ls")]
    List {
        /// 只显示可升级的包
        #[arg(long)]
        upgradable: bool,
    },

    /// 从应用目录构建 .opt 包
    ///
    /// 从结构化的应用目录创建 .opt 包文件。
    /// 目录应包含:
    ///   opt.json        - 包元数据
    ///   data/           - 应用文件（相对于 / 的路径）
    ///   control/        - 安装脚本（可选）
    ///
    /// 示例:
    ///   opt build ./my-app/
    ///   opt build ./my-app/ -o ./output/
    #[command(visible_alias = "b")]
    Build {
        /// 应用目录路径
        #[arg(required = true)]
        path: String,

        /// 输出目录（.opt 文件存放位置）
        #[arg(short = 'o', long)]
        output: Option<String>,

        /// 强制重建（即使有验证警告）
        #[arg(short = 'f', long)]
        force: bool,
    },

    /// 创建一个 .opt 项目脚手架（应用模板）
    ///
    /// 在指定目录生成一个完整的 .opt 应用模板，
    /// 包含 opt.json、data/ 目录结构和 .desktop 文件。
    /// 然后可直接用 opt build 编译成 .opt 包。
    ///
    /// 示例:
    ///   opt scaffold my-app
    ///   opt scaffold my-app -o ./projects/
    #[command(visible_alias = "new")]
    Scaffold {
        /// 应用名称
        #[arg(required = true)]
        name: String,

        /// 输出目录（省略则在当前目录创建）
        #[arg(short = 'o', long)]
        output: Option<String>,
    },

    /// 验证 .opt 包文件的完整性
    ///
    /// 检查 .opt 文件的结构和完整性，但不安装。
    ///
    /// 示例:
    ///   opt check firefox.opt
    ///   opt check --verbose firefox.opt
    #[command(visible_alias = "check")]
    Validate {
        /// .opt 文件路径
        #[arg(required = true)]
        path: String,

        /// 显示详细验证信息
        #[arg(short = 'v', long)]
        verbose: bool,
    },

    /// 解包 .opt 文件内容
    ///
    /// 将包文件提取到目录中，方便查看内容。
    ///
    /// 示例:
    ///   opt extract firefox.opt
    ///   opt extract firefox.opt -o ./extracted/
    #[command(visible_alias = "x")]
    Extract {
        /// .opt 文件路径
        #[arg(required = true)]
        path: String,

        /// 输出目录
        #[arg(short = 'o', long)]
        output: Option<String>,
    },

    /// 从任意 Linux 应用导入并构建 .opt 包
    ///
    /// 支持多种输入:
    ///   - .deb 包        → 解析并提取
    ///   - 应用目录       → 直接打包
    ///   - ELF 二进制文件 → 自动包裹
    ///   - .AppImage      → 提取并打包
    ///
    /// 示例:
    ///   opt import ./firefox_120.0_amd64.deb        从 .deb 导入
    ///   opt import ./my-linux-app/                  从目录导入
    ///   opt import ./some-binary                    从 ELF 二进制导入
    ///   opt import ./app.AppImage                   从 AppImage 导入
    #[command(visible_alias = "im")]
    Import {
        /// .deb 文件、目录、ELF 二进制或 AppImage 路径
        #[arg(required = true)]
        source: String,

        /// 输出目录（.opt 项目目录）
        #[arg(short = 'o', long)]
        output: Option<String>,

        /// 强制覆盖已存在的输出目录
        #[arg(short = 'f', long)]
        force: bool,
    },

    /// 管理 OPT 配置
    ///
    /// 查看和修改 opt 配置项。
    ///
    /// 示例:
    ///   opt config list                    列出所有配置
    ///   opt config get contribute          查看配置值
    ///   opt config set contribute true     开启分享模式
    ///   opt config set repo-path ./my-repo 设置本地仓库路径
    #[command(visible_alias = "cfg")]
    Config {
        #[command(subcommand)]
        action: ConfigAction,
    },

    /// 分享包到官方仓库
    ///
    /// 将已安装的 .opt 包提交到 OPENOS 包仓库。
    /// 需要先开启分享模式: opt config set contribute true
    ///
    /// 示例:
    ///   opt share wechat
    ///   opt share ./my-app.opt
    #[command(visible_alias = "sh")]
    Share {
        /// 包名或 .opt 文件路径
        #[arg(required = true)]
        target: String,
    },

    /// 生成 shell 自动补全脚本
    ///
    /// 输出补全脚本到标准输出，支持 bash/zsh/fish/powershell/elvish。
    /// 安装到 shell:
    ///   bash:  opt completion bash > /usr/local/share/bash-completion/completions/opt
    ///   zsh:   opt completion zsh > /usr/local/share/zsh/site-functions/_opt
    ///   fish:  opt completion fish > ~/.config/fish/completions/opt.fish
    Completion {
        /// shell 类型
        #[arg(value_enum)]
        shell: clap_complete::Shell,
    },

    /// 显示版本信息
    Version,
}

#[derive(Subcommand)]
pub enum ConfigAction {
    /// 列出所有配置
    List,
    /// 获取配置值
    Get {
        /// 配置项名称
        #[arg(required = true)]
        key: String,
    },
    /// 设置配置值
    Set {
        /// 配置项名称
        #[arg(required = true)]
        key: String,
        /// 配置项值
        #[arg(required = true)]
        value: String,
    },
}
