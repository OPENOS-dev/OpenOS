// Copyright 2024 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! Structs used to transport log requests between client processes and the logging controller

use metrics_events::MetricEventType;
use serde::Deserialize;
use serde::Serialize;

#[derive(Serialize, Deserialize, Debug)]
pub enum MetricsRequest {
    EventCount {
        event_code: MetricEventType,
    },
    Enum {
        event_code: MetricEventType,
        sample: i64,
    },
    RegularHistogram {
        event_code: MetricEventType,
        sample: i64,
    },
}
