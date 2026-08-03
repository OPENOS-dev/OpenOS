// Copyright 2024 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

use base::error;
use base::warn;
use base::RecvTube;
use metrics_events::MetricEventType;
use metrics_rs::MetricsLibrary;
use virtio_sys::virtio_ids;

use crate::metrics_requests::MetricsRequest;

#[derive(Default)]
pub struct MetricsRequestHandler {
    connected: bool,
}

const K_ONE_HOUR_MS: i32 = 60 * 60 * 1000;

fn virtio_id_to_wakeup_metric(virtio_id: u32) -> &'static str {
    match virtio_id {
        virtio_ids::VIRTIO_ID_NET => "Arc.Wakeup.VirtioNet",
        virtio_ids::VIRTIO_ID_BLOCK => "Arc.Wakeup.VirtioBlock",
        virtio_ids::VIRTIO_ID_BALLOON => "Arc.Wakeup.VirtioBalloon",
        virtio_ids::VIRTIO_ID_GPU => "Arc.Wakeup.VirtioGpu",
        virtio_ids::VIRTIO_ID_VSOCK => "Arc.Wakeup.VirtioVsock",
        virtio_ids::VIRTIO_ID_FS => "Arc.Wakeup.VirtioFs",
        virtio_ids::VIRTIO_ID_WL => "Arc.Wakeup.VirtioWayland",
        _ => "Arc.Wakeup.VirtioOther",
    }
}

impl MetricsRequestHandler {
    pub fn new() -> Self {
        let connected = MetricsLibrary::get().is_some();
        if !connected {
            error!("Failed to initialize metrics library");
        }

        MetricsRequestHandler { connected }
    }

    pub fn handle_tube_readable(&self, tube: &RecvTube) {
        if !self.connected {
            return;
        }

        let req = match tube.recv::<MetricsRequest>() {
            Ok(req) => req,
            Err(e) => {
                warn!("unexpected error receiving agent metrics request: {}", e);
                return;
            }
        };

        let metrics = MetricsLibrary::get().expect("metrics disappeared");
        let mut metrics = metrics.lock().expect("poisioned lock");

        let res = match req {
            #[allow(unreachable_code, unused_variables, clippy::match_single_binding)]
            MetricsRequest::Enum { event_code, sample } => {
                let (name, max): (&str, i32) = match event_code {
                    _ => return,
                };
                if sample > max as i64 {
                    warn!("Enum value {} too large for {}", sample, name);
                    return;
                }
                metrics.send_enum_to_uma(name, sample as i32, max)
            }
            #[allow(unreachable_code, unused_variables, clippy::match_single_binding)]
            MetricsRequest::EventCount { event_code } => {
                let name = match event_code {
                    _ => return,
                };
                // See MetricsLibrary::SendBoolToUMA
                metrics.send_enum_to_uma(name, 1, 2)
            }
            MetricsRequest::RegularHistogram { event_code, sample } => {
                let (name, min, max, nbuckets) = match event_code {
                    MetricEventType::RtcWakeup => ("Arc.Wakeup.RTC", 0, K_ONE_HOUR_MS, 50),
                    MetricEventType::VirtioWakeup { virtio_id } => {
                        (virtio_id_to_wakeup_metric(virtio_id), 0, K_ONE_HOUR_MS, 50)
                    }
                    _ => return,
                };
                let sample = sample.try_into().unwrap_or(i32::MAX);
                metrics.send_to_uma(name, sample, min, max, nbuckets)
            }
        };
        if let Err(err) = res {
            error!("Failed to upload metrics: {:?}", err);
        }
    }

    pub fn shutdown(&self) {}

    pub fn will_log_event(event_code: &MetricEventType) -> bool {
        matches!(
            event_code,
            MetricEventType::RtcWakeup | MetricEventType::VirtioWakeup { .. }
        )
    }
}
