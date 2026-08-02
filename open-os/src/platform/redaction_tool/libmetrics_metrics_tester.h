// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_FEEDBACK_REDACTION_TOOL_LIBMETRICS_METRICS_TESTER_H_
#define COMPONENTS_FEEDBACK_REDACTION_TOOL_LIBMETRICS_METRICS_TESTER_H_

#include <memory>

#include "redaction_tool/metrics_tester.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_refptr.h"
#include "metrics/fake_metrics_library.h"

namespace redaction {

// A class used for testing to retrieve bucket values for histograms.
// This is the default implementation for the `MetricsTester` in CrOS.
class LibMetricsMetricsTester : public MetricsTester {
 public:
    LibMetricsMetricsTester() = default;
    LibMetricsMetricsTester(const LibMetricsMetricsTester&) = delete;
    LibMetricsMetricsTester operator=(const LibMetricsMetricsTester&) = delete;
    ~LibMetricsMetricsTester() override = default;

  // redaction::MetricsTester:
  size_t GetBucketCount(std::string_view histogram_name,
                        int histogram_value) override;
  size_t GetNumBucketEntries(std::string_view histogram_name) override;
  std::unique_ptr<RedactionToolMetricsRecorder> SetupRecorder() override;

 private:
  FakeMetricsLibrary* fake_metrics_library();

  scoped_refptr<base::RefCountedData<std::unique_ptr<MetricsLibraryInterface>>>
    fake_metrics_library_;
};

}  // namespace redaction

#endif  // COMPONENTS_FEEDBACK_REDACTION_TOOL_LIBMETRICS_METRICS_TESTER_H_
