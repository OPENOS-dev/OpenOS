// Copyright 2021 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fuzzer_provider.h"
#include <memory>
#include <utility>

extern "C" {
#include "pinweaver_eal_tpm.h"
#include "tss.h"
#include "tss_serde.h"
} // extern "C"

std::unique_ptr<FuzzedDataProvider> g_data_provider;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  g_data_provider.reset(new FuzzedDataProvider(data, size));
  // TODO
  return 0;
}
