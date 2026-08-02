// Copyright 2023 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

use lazy_static::lazy_static;
use regex::Regex;
use std::collections::HashMap;
use std::error::Error;
use std::fs;

#[derive(Debug)]
pub struct MeminfoWatcher {}

impl MeminfoWatcher {
    // Parse /proc/meminfo into a HashMap.
    pub fn current() -> Result<HashMap<String, i64>, Box<dyn Error>> {
        // TODO(b/277264626) update a passed-in &mut HashMap rather than making a new one.
        lazy_static! {
            static ref RE: Regex = Regex::new(r"(.+):\s+(\d+) kB").unwrap();
        }
        let mut meminfo = HashMap::new();
        let contents = fs::read_to_string("/proc/meminfo")?;
        for line in contents.split('\n') {
            if let Some(captures) = RE.captures(line) {
                meminfo.insert(
                    captures.get(1).unwrap().as_str().to_string(),
                    captures.get(2).unwrap().as_str().parse::<i64>().unwrap(),
                );
            }
        }

        Ok(meminfo)
    }
}
