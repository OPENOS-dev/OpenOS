// Copyright 2024 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

use base::warn;
use base::AsRawDescriptor;
use base::RawDescriptor;
use base::SendTube;
use metrics_events::MetricEventType;
use metrics_events::RecordDetails;
use sync::Mutex;

use crate::metrics_requests::MetricsRequest;
use crate::MetricsClientDestructor;
use crate::MetricsRequestHandler;

/// Tube connected to the metrics controller.
static CONTROLLER_TUBE: Mutex<Option<SendTube>> = Mutex::new(None);

/// Sets the Tube where metrics are sent. Panics if called more than once.
pub fn initialize(send_tube: SendTube) {
    let mut tube = CONTROLLER_TUBE.lock();
    if tube.is_some() {
        // Technically we could replace tube; however:
        // 1. this would EPIPE the other end.
        // 2. this should never be done in production.
        panic!("Attempt to initialize metrics client twice.");
    }
    *tube = Some(send_tube);
}

#[cfg(test)]
pub fn force_initialize(send_tube: SendTube) {
    // Safe because:
    //      1. CLIENT_TUBE is guaranteed to be initialized.
    //      2. Holding the lock prevents any other thread from accessing it.
    let mut tube = CONTROLLER_TUBE.lock();
    *tube = Some(send_tube);
}

pub fn push_descriptors(keep_rds: &mut Vec<RawDescriptor>) {
    if let Some(tube) = CONTROLLER_TUBE.lock().as_ref() {
        keep_rds.push(tube.as_raw_descriptor());
    }
}

pub fn get_destructor() -> MetricsClientDestructor {
    MetricsClientDestructor::new(|| {
        // Let Tube close by going out of scope.
        let _ = CONTROLLER_TUBE.lock().take();
    })
}

/// Returns true if the metrics client is initialized.
pub fn is_initialized() -> bool {
    CONTROLLER_TUBE.lock().is_some()
}

pub fn set_auth_token(_: &str) {}
pub fn set_graphics_api(_: &str) {}
pub fn set_package_name(_: &str) {}
pub fn merge_session_invariants(_: &[u8]) {}

pub fn log_descriptor(event_code: MetricEventType, descriptor: i64) {
    if MetricsRequestHandler::will_log_event(&event_code) {
        send_request(MetricsRequest::Enum {
            event_code,
            sample: descriptor,
        })
    }
}

pub fn log_event(event_code: MetricEventType) {
    if MetricsRequestHandler::will_log_event(&event_code) {
        send_request(MetricsRequest::EventCount { event_code })
    }
}

pub fn log_metric(event_code: MetricEventType, value: i64) {
    if MetricsRequestHandler::will_log_event(&event_code) {
        send_request(MetricsRequest::RegularHistogram {
            event_code,
            sample: value,
        })
    }
}

pub fn log_metric_with_details(event_code: MetricEventType, _: i64,  _: &RecordDetails) {
    if MetricsRequestHandler::will_log_event(&event_code) {
        unimplemented!()
    }
}

pub fn log_histogram_metric(event_code: MetricEventType, value: i64) {
    log_metric(event_code, value)
}

pub fn log_high_frequency_descriptor_event(event_code: MetricEventType, _: i64, _: i64) {
    if MetricsRequestHandler::will_log_event(&event_code) {
        unimplemented!()
    }
}

pub fn log_event_with_details(event_code: MetricEventType, _: &RecordDetails) {
    if MetricsRequestHandler::will_log_event(&event_code) {
        unimplemented!()
    }
}

/// Sends a metrics request to the server. Does nothing if the metrics client is not initialized.
fn send_request(req: MetricsRequest) {
    let controller_lock = CONTROLLER_TUBE.lock();
    match controller_lock.as_ref() {
        Some(controller_tube) => {
            if let Err(e) = controller_tube.send(&req) {
                warn!("Failed to send metrics to metrics controller: {}", e);
            }
        }
        None => {
            warn!(
                "Failed to send metrics to metrics controller because no controller Tube was set."
            );
        }
    }
}
