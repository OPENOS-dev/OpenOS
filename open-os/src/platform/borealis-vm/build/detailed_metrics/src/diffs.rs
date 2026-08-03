// Copyright 2022 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

use std::collections::HashMap;

// Diff two hashmaps, returning deltas for all different values.
pub fn diff_maps(
    current: &HashMap<String, i64>,
    prev: &HashMap<String, i64>,
) -> HashMap<String, i64> {
    let mut diff = HashMap::new();
    for (key, current_value) in current {
        match prev.get(key) {
            Some(prev_value) => {
                let value_diff = current_value - prev_value;
                if value_diff != 0 {
                    diff.insert(key.clone(), value_diff);
                }
            }
            None => {
                diff.insert(key.clone(), *current_value);
            }
        }
    }
    for (key, prev_value) in prev {
        if !current.contains_key(key) {
            diff.insert(key.clone(), -prev_value);
        }
    }

    diff
}
