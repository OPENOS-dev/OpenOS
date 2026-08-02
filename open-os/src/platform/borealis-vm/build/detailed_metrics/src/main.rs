// Copyright 2022 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

use clap::Parser;
use std::collections::HashMap;
use std::{fs::File, io::Write, thread, time};

mod diffs;
mod i915gemobjects;
mod meminfo;
mod swap;
mod vmstat;

#[derive(Debug, Parser)]
#[clap(author, version, about, long_about = None)]
struct Cli {
    // Output path for JSON metrics.
    #[clap(short, long, default_value_t = String::from("/tmp/metrics.json"), value_parser)]
    output: String,

    // Number of collection cycles to perform before exiting, or 0 to run forever.
    #[clap(long, default_value_t = 0, value_parser)]
    runs: u64,

    // Number of seconds to pause in between collection cycles.
    #[clap(long, default_value_t = 1, value_parser)]
    period: u64,

    // Output a small selection of memory stats rather than full diffs.
    #[clap(long, default_value_t = false, value_parser)]
    quick_stats: bool,

    // Don't print to stdout.
    #[clap(short, long, default_value_t = false, value_parser)]
    quiet: bool,
}

fn main() {
    let cli = Cli::parse();
    eprintln!("High volume metric collector: outputting to {}", cli.output);

    let mut logfile = File::create(cli.output).unwrap();
    let mut vmstat_watcher = vmstat::VmstatWatcher::new();
    let mut run_count: u64 = 0;
    loop {
        if cli.quick_stats {
            // Every time period, output some selected memory stats.
            if let (Ok(meminfo), Ok(vmstat), Ok(swap), Ok(i915gemobjects)) = (
                meminfo::MeminfoWatcher::current(),
                vmstat::VmstatWatcher::current(),
                swap::SwapWatcher::current(),
                i915gemobjects::I915gemObjectsWatcher::current(),
            ) {
                let mut stats = HashMap::new();
                // Include selected values from /proc/meminfo.
                for key in &[
                    "MemTotal",
                    "MemFree",
                    "MemAvailable",
                    "Buffers",
                    "Cached",
                    "SwapCached",
                    "Active",
                    "Inactive",
                    "Unevictable",
                    "Mlocked",
                    "SwapTotal",
                    "SwapFree",
                    "Dirty",
                    "Writeback",
                    "Shmem",
                    "Slab",
                ] {
                    // TODO(b/277264626) Pre-construct stats dict and update values.
                    stats.insert(key.to_string(), meminfo.get(&key.to_string()).unwrap());
                }
                // Include selected values from /proc/vmstat.
                for key in &[
                    "pgpgin",
                    "pgpgout",
                    "pswpin",
                    "pswpout",
                    "oom_kill",
                    "balloon_inflate",
                    "balloon_deflate",
                ] {
                    if let Some(value) = vmstat.get(&key.to_string()) {
                        // balloon_* are only present in the guest.
                        stats.insert(key.to_string(), value);
                    }
                }
                // Include everything we collect about swap.
                for (key, value) in &swap {
                    stats.insert(key.clone(), value);
                }

                // Include everything we collect about i915 gem objects.
                for (key, value) in &i915gemobjects {
                    stats.insert(key.clone(), value);
                }

                let serialized = serde_json::to_string(&stats).unwrap();
                if !cli.quiet {
                    println!("{}", serialized);
                }
                writeln!(logfile, "{}", serialized).unwrap_or_else(|err| {
                    println!("Error writing to logfile: {}", err);
                });
            }
        } else {
            // Every second, run all the watchers (right now there's
            // just vmstat, but more will come), collect the output of
            // any that reported anything, and dump it to the logfile
            // as JSON.  Right now it's just dumping the vmstat output
            // with {:?}, but we want this to be machine readable and
            // fairly forward-compatible, given that these json files
            // will be uploaded with `bdt report -u` and possibly
            // preserved for quite some time.

            // Report diffs in /proc/vmstat.
            match vmstat_watcher.diff_since_last() {
                Ok(diffs) => {
                    if !cli.quiet {
                        println!("vmstat diff: {:?}", diffs);
                    }
                    write!(logfile, "{:?}", diffs).unwrap_or_else(|err| {
                        println!("Error writing to logfile: {}", err);
                    });
                }
                Err(err) => {
                    println!("Error collecting /proc/vmstat diffs: {}", err);
                }
            }
        }

        run_count += 1;
        if cli.runs > 0 && run_count >= cli.runs {
            // We've hit the run count specified with --runs; exit.
            break;
        }
        thread::sleep(time::Duration::from_secs(cli.period));
    }
}
