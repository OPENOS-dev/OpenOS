// Copyright 2024 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

use glob::glob;
use lazy_static::lazy_static;
use regex::Regex;
use std::collections::HashMap;
use std::error::Error;
use std::fs;

#[derive(Debug)]
pub struct I915gemObjectsWatcher {}

impl I915gemObjectsWatcher {
    // Parse /sys/kernel/debug/dri/0/i915_gem_objects into a HashMap.
    pub fn current() -> Result<HashMap<String, i64>, Box<dyn Error>> {
        // TODO(b/277264626) update a passed-in &mut HashMap rather than making a new one.
        lazy_static! {
            static ref RE: Regex = Regex::new(r"shrinkable.*?(\d+) bytes$").unwrap();
        }
        let mut gem_objects = HashMap::new();
        // Only need the first match since all other entries are just symlinks.
        let paths: Vec<_> = glob("/sys/kernel/debug/dri/*/i915_gem_objects")
            .unwrap()
            .flatten()
            .collect();
        if let Some(path) = paths.first() {
            if let Ok(contents) = fs::read_to_string(path) {
                for line in contents.split('\n') {
                    if let Some(captures) = RE.captures(line) {
                        gem_objects.insert(
                            "i915_gem_objects bytes".to_string(),
                            captures.get(1).unwrap().as_str().parse::<i64>().unwrap(),
                        );
                    }
                }
            }
        }

        Ok(gem_objects)
    }
}
