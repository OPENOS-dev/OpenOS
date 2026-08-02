// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "redaction_tool/libmetrics_metrics_recorder.h"

#include "base/notreached.h"
#include "base/time/time.h"

namespace {
constexpr char kTimeSpentRedactingHistogram[] =
    "Feedback.RedactionTool.TimeSpentRedactingCrash";
}

namespace redaction {

LibMetricsMetricsRecorder::LibMetricsMetricsRecorder(
    scoped_refptr<
      base::RefCountedData<std::unique_ptr<MetricsLibraryInterface>>>
        metrics_library)
    : metrics_library_(metrics_library) {
  CHECK(metrics_library_);
}

LibMetricsMetricsRecorder::~LibMetricsMetricsRecorder() = default;

void LibMetricsMetricsRecorder::RecordPIIRedactedHistogram(PIIType pii_type) {
  metrics_library_->data->SendEnumToUMA(kPIIRedactedHistogram, pii_type);
}

void LibMetricsMetricsRecorder::RecordCreditCardRedactionHistogram(
    CreditCardDetection step) {
  metrics_library_->data->SendEnumToUMA(kCreditCardRedactionHistogram, step);
}

void LibMetricsMetricsRecorder::RecordRedactionToolCallerHistogram(
    RedactionToolCaller step) {
  metrics_library_->data->SendEnumToUMA(kRedactionToolCallerHistogram, step);
}

void LibMetricsMetricsRecorder::RecordTimeSpentRedactingHistogram(
    base::TimeDelta elapsed_time) {
  metrics_library_->data->SendTimeToUMA(kTimeSpentRedactingHistogram,
      elapsed_time, base::Milliseconds(1), base::Minutes(3), 50);
}

std::unique_ptr<RedactionToolMetricsRecorder>
RedactionToolMetricsRecorder::Create() {
  NOTREACHED() << "Don't use RedactionToolMetricsRecorder::Create() in"
    "CrOS code. Instantiate LibMetricsMetricsRecorder explicitly instead.";
}

std::string_view
RedactionToolMetricsRecorder::GetTimeSpentRedactingHistogramNameForTesting() {
  return kTimeSpentRedactingHistogram;
}

}  // namespace redaction
