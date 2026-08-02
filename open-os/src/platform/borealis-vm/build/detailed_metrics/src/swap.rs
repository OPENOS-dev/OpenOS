// Copyright 2023 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

use std::collections::HashMap;
use std::error::Error;
use std::fs;

#[derive(Debug)]
pub struct SwapWatcher {}

impl SwapWatcher {
    // Build a HashMap of various interesting swap numbers.
    pub fn current() -> Result<HashMap<String, i64>, Box<dyn Error>> {
        // TODO(b/277264626) update a passed-in &mut HashMap rather than making a new one.
        let mut swap = HashMap::new();

        // Host: ZRAM usage.
        if let Ok(contents) = fs::read_to_string("/sys/block/zram0/mm_stat") {
            let mm_stat: Vec<i64> = contents
                .split_whitespace()
                .map(|s| s.parse::<i64>().unwrap())
                .collect();
            // Uncompressed data in zram.
            swap.insert("zram_orig_data_size".to_string(), mm_stat[0]);
            // Compressed data size.
            swap.insert("zram_compr_data_size".to_string(), mm_stat[1]);
            // Actual mem used by compressed data.
            swap.insert("zram_mem_used_total".to_string(), mm_stat[2]);
            // Max amount of memory to use for compressed data.
            swap.insert("zram_mem_limit".to_string(), mm_stat[3]);
        }

        Ok(swap)
    }
}
