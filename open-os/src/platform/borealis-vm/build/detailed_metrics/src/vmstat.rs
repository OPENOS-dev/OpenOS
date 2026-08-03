// Copyright 2022 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

use std::collections::HashMap;
use std::error::Error;
use std::fs;

#[derive(Debug)]
pub struct VmstatWatcher {
    // Most recently read
    previous: HashMap<String, i64>,
}

impl VmstatWatcher {
    // Create empty VmstatWatcher.
    pub fn new() -> VmstatWatcher {
        VmstatWatcher {
            previous: HashMap::new(),
        }
    }

    // Reread /proc/vmstat and return map of differences since the last read.
    pub fn diff_since_last(&mut self) -> Result<HashMap<String, i64>, Box<dyn Error>> {
        let latest = VmstatWatcher::current()?;
        let diff = crate::diffs::diff_maps(&latest, &self.previous);
        self.previous = latest;

        Ok(diff)
    }

    // Parse /proc/vmstat into a HashMap.
    pub fn current() -> Result<HashMap<String, i64>, Box<dyn Error>> {
        // TODO(b/277264626) update a passed-in &mut HashMap rather than making a new one.
        let mut vmstat = HashMap::new();
        let contents = fs::read_to_string("/proc/vmstat")?;
        for line in contents.split('\n') {
            if let Some((k, v)) = line.split_once(' ') {
                if let Ok(i) = v.parse::<i64>() {
                    vmstat.insert(k.to_string(), i);
                }
            }
        }

        Ok(vmstat)
    }
}
