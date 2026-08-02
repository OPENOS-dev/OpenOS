// Copyright 2024 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

use std::result::Result;
use std::time::Duration;

use metrics_events::MetricEventType;

use crate::MetricsRequestHandler;

/// A logging struct meant for use in tracking and periodically
/// logging a single metric. The metric is aggregated over the
/// designated time period. Intended for use with high-frequency metrics.
pub struct PeriodicLogger;

impl PeriodicLogger {
    pub fn new(event_code: MetricEventType, _: Duration) -> Result<PeriodicLogger, String> {
        if MetricsRequestHandler::will_log_event(&event_code) {
            unimplemented!()
        }
        Ok(PeriodicLogger)
    }

    /// Indicate the event has occurred with the given
    /// value to be aggregated over the given time period.
    pub fn log(&self, _: i64) {}
}
