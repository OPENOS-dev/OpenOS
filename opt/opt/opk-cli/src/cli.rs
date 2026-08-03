// OPK CLI - Command line interface definitions
// Uses clap derive for ergonomic CLI argument parsing.
// Mirror of apt's familiar CLI with Open-OS enhancements.

use clap::{Parser, Subcommand, ValueEnum};

/// Open Package Keeper - Open-OS Package Manager
///
/// OPK is the official package manager for Open-OS.
/// It provides apt-like functionality with enhanced dependency resolution,
/// transaction safety, and a modern terminal UI.
///
/// Examples:
///   opk install firefox vlc        Install packages
///   opk search "text editor"       Search repository
///   opk update && opk upgrade      Update and upgrade all
///   opk repo add https://...       Add a repository
#[derive(Parser)]
#[command(
    name = "opk",
    version = env!("CARGO_PKG_VERSION"),
    about = "Open Package Keeper for Open-OS",
    long_about = None,
    after_help = "Run 'opk help <command>' for more information on a specific command.",
    disable_help_subcommand = true,
)]
pub struct Opts {
    #[command(subcommand)]
    pub command: Command,
}

#[derive(Subcommand)]
pub enum Command {
    /// Install one or more packages
    ///
    /// Resolves dependencies automatically and installs from configured repositories.
    /// Use --no-deps to install only the specified package without dependencies.
    ///
    /// Examples:
    ///   opk install firefox
    ///   opk install vlc mpv  # install multiple
    ///   opk install -y firefox   # skip confirmation
    #[command(visible_alias = "i")]
    Install {
        /// Package names to install
        #[arg(required = true, num_args = 1..)]
        packages: Vec<String>,

        /// Assume yes to all prompts
        #[arg(short = 'y', long)]
        yes: bool,

        /// Skip dependency resolution (install only specified package)
        #[arg(long)]
        no_deps: bool,
    },

    /// Remove one or more packages
    ///
    /// Removes packages and their configuration files by default.
    /// Use --purge to also remove system-wide configuration.
    ///
    /// Examples:
    ///   opk remove firefox
    ///   opk remove --purge vlc
    #[command(visible_alias = "rm")]
    Remove {
        /// Package names to remove
        #[arg(required = true, num_args = 1..)]
        packages: Vec<String>,

        /// Also remove configuration files (purge)
        #[arg(short = 'P', long)]
        purge: bool,

        /// Assume yes to all prompts
        #[arg(short = 'y', long)]
        yes: bool,
    },

    /// Update package list from repositories
    ///
    /// Fetches the latest package metadata from all configured repositories.
    /// Run this before: opk upgrade
    #[command(visible_alias = "u")]
    Update,

    /// Upgrade all installed packages to latest versions
    ///
    /// Resolves dependencies and upgrades packages safely.
    /// Use --dry-run to preview without applying changes.
    ///
    /// Examples:
    ///   opk upgrade
    ///   opk upgrade --dry-run
    ///   opk upgrade -y
    #[command(visible_alias = "up")]
    Upgrade {
        /// Assume yes to all prompts
        #[arg(short = 'y', long)]
        yes: bool,

        /// Show what would be done without executing
        #[arg(long)]
        dry_run: bool,
    },

    /// Search for packages by name or description
    ///
    /// Searches across all configured repositories.
    /// Use --all to search in descriptions as well.
    ///
    /// Examples:
    ///   opk search browser
    ///   opk search "text editor" --all
    ///   opk search firefox --names-only
    #[command(visible_alias = "s")]
    Search {
        /// Search query string
        #[arg(required = true)]
        query: String,

        /// Search in descriptions and tags too
        #[arg(short = 'a', long)]
        all: bool,

        /// Only search package names
        #[arg(short = 'n', long)]
        names_only: bool,
    },

    /// Show detailed information about a package
    ///
    /// Displays version, description, dependencies, size, and more.
    ///
    /// Examples:
    ///   opk info firefox
    #[command(visible_alias = "show")]
    Info {
        /// Package name
        #[arg(required = true)]
        package: String,
    },

    /// List packages
    ///
    /// By default lists all available packages.
    /// Use --installed to show only installed packages.
    ///
    /// Examples:
    ///   opk list
    ///   opk list --installed
    ///   opk list --upgradable
    #[command(visible_alias = "ls")]
    List {
        /// Show only installed packages
        #[arg(short = 'i', long)]
        installed: bool,

        /// Show upgradable packages only
        #[arg(long)]
        upgradable: bool,
    },

    /// Clean up downloaded package caches
    ///
    /// Removes all cached .opk files from /var/cache/opk/archives/.
    /// Safe to run anytime - does not affect installed packages.
    #[command(visible_alias = "cc")]
    Clean,

    /// Download source code for a package
    ///
    /// Downloads the source tarball for a package.
    /// Sources are placed in the current directory.
    #[command(visible_alias = "src")]
    Source {
        /// Package name to fetch source for
        #[arg(required = true)]
        package: String,
    },

    /// Manage OPK repositories
    ///
    /// Add, remove, or list configured package repositories.
    ///
    /// Examples:
    ///   opk repo list
    ///   opk repo add https://repo.openos.org/community
    ///   opk repo remove community
    #[command(visible_alias = "r")]
    Repo {
        #[command(subcommand)]
        action: RepoAction,
    },

    /// Show OPK version information
    #[command()]
    Version,
}

#[derive(Subcommand)]
pub enum RepoAction {
    /// Add a new repository
    Add {
        /// Repository URL
        #[arg(required = true)]
        url: String,

        /// Repository name (auto-detected if omitted)
        #[arg(short = 'n', long)]
        name: Option<String>,
    },

    /// Remove a repository
    Remove {
        /// Repository name or URL
        #[arg(required = true)]
        target: String,
    },

    /// List all configured repositories
    #[command(visible_alias = "ls")]
    List,
}
