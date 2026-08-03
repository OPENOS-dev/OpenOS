// OPK - Open Package Keeper
// Main entrypoint for the opk CLI tool.
//
// OPK is an apt-compatible package manager for Open-OS.
// It handles: install, remove, update, upgrade, search, info, list, clean
//
// Command reference:
//   opk install <pkg>...    Install one or more packages
//   opk remove  <pkg>...    Remove one or more packages
//   opk update              Update package lists from repositories
//   opk upgrade             Upgrade all installed packages
//   opk search  <query>     Search for packages
//   opk info    <pkg>       Show package details
//   opk list    [--installed] List packages (installed or available)
//   opk clean               Clean downloaded package caches
//   opk source <pkg>        Download source of a package
//   opk repo    <add|remove|list>  Manage repositories
//   opk version              Show version information

mod cli;
mod config;
mod core;
mod error;
mod format;
mod repo;

use anyhow::Result;
use clap::Parser;

use cli::Opts;
use core::OpkManager;

fn main() -> Result<()> {
    env_logger::Builder::from_env(env_logger::Env::default().default_filter_or("warn"))
        .format_timestamp_millis()
        .init();

    let opts = Opts::parse();
    let mut manager = OpkManager::new()?;

    match opts.command {
        cli::Command::Install { packages, yes, no_deps } => {
            manager.install(&packages, yes, no_deps)?;
        }
        cli::Command::Remove { packages, purge, yes } => {
            manager.remove(&packages, purge, yes)?;
        }
        cli::Command::Update => {
            manager.update()?;
        }
        cli::Command::Upgrade { yes, dry_run } => {
            manager.upgrade(yes, dry_run)?;
        }
        cli::Command::Search { query, all, names_only } => {
            manager.search(&query, all, names_only)?;
        }
        cli::Command::Info { package } => {
            manager.info(&package)?;
        }
        cli::Command::List { installed, upgradable } => {
            manager.list(installed, upgradable)?;
        }
        cli::Command::Clean => {
            manager.clean()?;
        }
        cli::Command::Source { package } => {
            manager.source(&package)?;
        }
        cli::Command::Repo { action } => {
            manager.repo(action)?;
        }
        cli::Command::Version => {
            manager.version()?;
        }
    }

    Ok(())
}
