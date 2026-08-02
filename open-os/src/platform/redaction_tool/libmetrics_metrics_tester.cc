// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "redaction_tool/libmetrics_metrics_tester.h"

#include <algorithm>
#include <string_view>

#include "redaction_tool/libmetrics_metrics_recorder.h"

namespace redaction {

std::unique_ptr<MetricsTester> MetricsTester::Create() {
  return std::make_unique<LibMetricsMetricsTester>();
}

size_t LibMetricsMetricsTester::GetBucketCount(std::string_view histogram_name,
                                               int histogram_value) {
  return std::ranges::count(
      fake_metrics_library()->GetCalls(std::string(histogram_name)),
      histogram_value);
}

size_t LibMetricsMetricsTester::GetNumBucketEntries(
    std::string_view histogram_name) {
  return fake_metrics_library()->NumCalls(std::string(histogram_name));
}

FakeMetricsLibrary* LibMetricsMetricsTester::fake_metrics_library() {
  return dynamic_cast<FakeMetricsLibrary*>(fake_metrics_library_->data.get());
}

std::unique_ptr<RedactionToolMetricsRecorder>
LibMetricsMetricsTester::SetupRecorder() {
  fake_metrics_library_ =
    base::MakeRefCounted<
      base::RefCountedData<std::unique_ptr<MetricsLibraryInterface>>>(
        std::forward<std::unique_ptr<FakeMetricsLibrary>>(
          std::make_unique<FakeMetricsLibrary>()));

  return std::make_unique<LibMetricsMetricsRecorder>(fake_metrics_library_);
}

}  // namespace redaction
