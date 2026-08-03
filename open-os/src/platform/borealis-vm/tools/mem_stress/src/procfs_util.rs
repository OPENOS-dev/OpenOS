// Copyright 2024 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

use anyhow::*;
use procfs::process::Process;
use sysinfo::System;
// Needed for getting pid by name
//use sysinfo::Process as SysProcess;

#[derive(Debug)]
pub struct MemPageData {
    pub statm_vm: u64,
    pub statm_rss: u64,
    pub statm_shared: u64,
}

// Initial placeholder.
// TODO: get swap in/out of proc from rolling up pmap.
pub fn get_memory_stat(proc_pid: i32) -> Result<MemPageData> {
    let ps = Process::new(proc_pid)?;

    let statm = ps.statm()?;
    Ok(MemPageData {
        statm_vm: statm.size,
        statm_rss: statm.resident,
        statm_shared: statm.shared,
    })
}

pub fn _get_pid(proc_name: &String) -> Result<Vec<u32>> {
    let s = System::new_all();

    let mut ret: Vec<u32> = Vec::new();
    for process in s.processes_by_exact_name(proc_name) {
        println!("{} {}", process.pid(), process.name());
        ret.push(process.pid().as_u32());
    }
    Ok(ret)
}

pub fn get_self_pid() -> Result<i32> {
    let p = Process::myself()?;
    Ok(p.pid())
}
