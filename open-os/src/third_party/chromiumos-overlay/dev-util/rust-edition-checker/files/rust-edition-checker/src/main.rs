// Copyright 2024 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

use std::collections::HashSet;
use std::fmt;
use std::io::Write;
use std::process::Command;

use anyhow::{bail, Context, Result};
use clap::Parser;
use serde::de::{Deserializer, Visitor};
use serde::Deserialize;

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
struct Edition(u32);

const DEFAULT_MIN_EDITION: Edition = Edition(2021);

impl std::fmt::Display for Edition {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(f, "{}", self.0)
    }
}

fn parse_edition_flag(s: &str) -> Result<Edition> {
    Ok(Edition(s.parse()?))
}

/// Runs `cargo metadata` in PWD, and ensures that packages part of the local workspace have a
/// new-enough Rust edition.
///
/// If all is OK, this prints nothing and exits successfully. If editions are out-of-date, this
/// will print messages describing the issues and exit successfully. In the case of any other
/// error, this exits with a non-zero exit code.
#[derive(Parser)]
struct Args {
    /// The minimum Rust edition that is considered OK at the moment.
    #[clap(long, value_parser = parse_edition_flag, default_value_t = DEFAULT_MIN_EDITION)]
    min_edition: Edition,
}

impl<'de> Deserialize<'de> for Edition {
    fn deserialize<D>(deserializer: D) -> Result<Edition, D::Error>
    where
        D: Deserializer<'de>,
    {
        struct EditionVisitor;
        impl<'de> Visitor<'de> for EditionVisitor {
            type Value = Edition;
            fn expecting(&self, formatter: &mut fmt::Formatter) -> fmt::Result {
                formatter.write_str("a string containing an edition (e.g., \"2021\")")
            }

            fn visit_str<E: serde::de::Error>(self, s: &str) -> Result<Self::Value, E> {
                match s.parse() {
                    Ok(x) => Ok(Edition(x)),
                    Err(x) => Err(E::custom(format!(
                        "Failed parsing {s:?} as a Rust edition: {x}"
                    ))),
                }
            }
        }

        deserializer.deserialize_str(EditionVisitor)
    }
}

#[derive(Deserialize)]
struct PackageTarget {
    name: String,
    edition: Edition,
}

#[derive(Deserialize)]
struct CargoPackage {
    name: String,
    version: String,
    id: String,
    targets: Vec<PackageTarget>,
}

#[derive(Deserialize)]
struct CargoManifest {
    packages: Vec<CargoPackage>,
    workspace_members: Vec<String>,
}

fn diagnose_old_editions<W: Write>(
    mut output: W,
    min_edition: Edition,
    manifest: &CargoManifest,
) -> Result<()> {
    let members: HashSet<&str> = manifest
        .workspace_members
        .iter()
        .map(String::as_str)
        .collect();

    // Diagnose packages that are directly a part of the current workspace. Deps aren't worth
    // diagnosing.
    let diagnosable_packages = manifest
        .packages
        .iter()
        .filter(|x| members.contains(x.id.as_str()));

    struct TooOld<'a> {
        package_name: &'a str,
        package_version: &'a str,
        target: &'a PackageTarget,
    }

    let mut found_diagnosable_packages = false;
    let mut too_old_targets: Vec<TooOld<'_>> = Vec::new();
    for package in diagnosable_packages {
        found_diagnosable_packages = true;
        too_old_targets.extend(
            package
                .targets
                .iter()
                .filter(|x| x.edition < min_edition)
                .map(|target| TooOld {
                    package_name: &package.name,
                    package_version: &package.version,
                    target,
                }),
        );
    }

    if !found_diagnosable_packages {
        bail!("unexpected: no diagnosable package editions found in manifest");
    }

    if too_old_targets.is_empty() {
        return Ok(());
    }

    fn too_old_key<'a>(x: &TooOld<'a>) -> (&'a str, &'a str, &'a str) {
        (x.package_name, x.package_version, x.target.name.as_str())
    }

    too_old_targets.sort_unstable_by_key(too_old_key);
    too_old_targets.dedup_by_key(|x| too_old_key(x));

    if too_old_targets.len() == 1 {
        let t = too_old_targets.pop().unwrap();
        writeln!(
            output,
            "Target {} in package {}-{} has Rust edition {}, which is not recommended.",
            t.target.name, t.package_name, t.package_version, t.target.edition.0
        )?;
    } else {
        writeln!(output, "Targets have outdated Rust editions:")?;
        for t in too_old_targets {
            writeln!(
                output,
                "- {} in package {}-{} has edition {}",
                t.target.name, t.package_name, t.package_version, t.target.edition.0
            )?;
        }
    }

    writeln!(
        output,
        concat!(
            "\n",
            "The minimum recommended edition is {}. Please consider upgrading for the newest ",
            "Rust features and lints.",
        ),
        min_edition.0,
    )?;
    Ok(())
}

fn run_cargo_manifest() -> Result<Vec<u8>> {
    let output = Command::new("cargo")
        .args(["--offline", "metadata", "--format-version=1"])
        .output()
        .context("running `cargo metadata`")?;
    if !output.status.success() {
        eprintln!(
            "Running cargo failed; stderr:\n{}",
            String::from_utf8_lossy(&output.stderr)
        );
        eprintln!(concat!(
            "NOTE: `cargo metadata` was passed `--offline`. Be sure your directory is set up ",
            "to allow offline operations before running this script.",
        ));
        bail!("`cargo metadata` failed");
    }
    Ok(output.stdout)
}

fn main() -> Result<()> {
    let args = Args::parse();
    let manifest: CargoManifest =
        serde_json::from_slice(&run_cargo_manifest()?).context("parsing cargo-manifest")?;
    diagnose_old_editions(std::io::stdout().lock(), args.min_edition, &manifest)
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn self_test() {
        let raw_manifest = run_cargo_manifest().expect("failed running `cargo manifest`");
        let manifest: CargoManifest =
            serde_json::from_slice(&raw_manifest).expect("failed parsing cargo manifest");
        let mut output = Vec::<u8>::new();
        diagnose_old_editions(&mut output, DEFAULT_MIN_EDITION, &manifest)
            .expect("diagnosing old editions");
        assert_eq!(String::from_utf8_lossy(&output), "");
    }
}
