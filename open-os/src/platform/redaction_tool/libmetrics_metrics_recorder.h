// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_FEEDBACK_REDACTION_TOOL_LIBMETRICS_METRICS_RECORDER_H_
#define COMPONENTS_FEEDBACK_REDACTION_TOOL_LIBMETRICS_METRICS_RECORDER_H_

#include <memory>

#include "base/memory/ref_counted.h"
#include "base/memory/scoped_refptr.h"
#include "metrics/metrics_library.h"
#include "redaction_tool_metrics_recorder.h"

namespace redaction {

// This is the ChromiumOS specific metrics recorder. It uses the metrics library
// from //platform2/metrics.
class LibMetricsMetricsRecorder : public RedactionToolMetricsRecorder {
 public:
  explicit LibMetricsMetricsRecorder(
      scoped_refptr<
        base::RefCountedData<std::unique_ptr<MetricsLibraryInterface>>>
          metrics_library);
  LibMetricsMetricsRecorder(const LibMetricsMetricsRecorder&) = delete;
  LibMetricsMetricsRecorder& operator=(const LibMetricsMetricsRecorder&) =
      delete;
  virtual ~LibMetricsMetricsRecorder();

  // redaction::RedactionToolMetricsRecorder:
  void RecordPIIRedactedHistogram(PIIType pii_type) override;
  void RecordCreditCardRedactionHistogram(CreditCardDetection step) override;
  void RecordRedactionToolCallerHistogram(RedactionToolCaller step) override;
  void RecordTimeSpentRedactingHistogram(base::TimeDelta elapsed_time) override;

 private:
  scoped_refptr<base::RefCountedData<std::unique_ptr<MetricsLibraryInterface>>>
    metrics_library_;
};

}  // namespace redaction

#endif  // COMPONENTS_FEEDBACK_REDACTION_TOOL_LIBMETRICS_METRICS_RECORDER_H_
