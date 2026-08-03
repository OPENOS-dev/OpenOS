// Copyright 2021 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fuzzer_provider.h"
#include <memory>
#include <utility>

extern "C" {
#include "pinweaver_eal.h"
#include "tpm_storage.h"
} // extern "C"

std::unique_ptr<FuzzedDataProvider> g_data_provider;

uint32_t g_pure_random_prob = 5;
// Probability in % of generating an error response.
uint32_t g_error_response_prob = 10;
// Probability in % of having an written NV space.
uint32_t g_written_nv_prob = 95;

inline void FuzzInit() {
  pinweaver_eal_storage_start();

  uint8_t root_hash[256 / 8];
  uint32_t restart_count;
  pinweaver_eal_storage_init_state(root_hash, &restart_count);
}

inline void FuzzLog() {
  struct pw_log_storage_t log;
  memset(&log, 0xA1, sizeof(log));
  if (g_data_provider->ConsumeBool()) {
    log.storage_version = g_data_provider->ConsumeIntegral<uint16_t>();
  } else {
    log.storage_version = 0;
  }
  size_t num_ops = g_data_provider->ConsumeIntegralInRange<size_t>(1, 5);
  for (int n = 0; n < num_ops; ++n) {
    if (g_data_provider->ConsumeBool()) {
      pinweaver_eal_storage_get_log(&log);
    } else {
      pinweaver_eal_storage_set_log(&log);
    }
  }
}

inline void FuzzTreeData() {
  struct pw_long_term_storage_t data;
  if (g_data_provider->ConsumeBool()) {
    pinweaver_eal_storage_get_tree_data(&data);
  } else {
    memset(&data, 0xA1, sizeof(data));
  }
  if (g_data_provider->ConsumeBool()) {
    data.storage_version = g_data_provider->ConsumeIntegral<uint16_t>();
  } else {
    data.storage_version = 0;
  }
  pinweaver_eal_storage_set_tree_data(&data);
  pinweaver_eal_storage_get_tree_data(&data);
}

inline void FuzzDeriveKeys() {
  struct merkle_tree_t merkle_tree;
  FillWithRandomData(&merkle_tree, sizeof(merkle_tree));
  pinweaver_eal_derive_keys(&merkle_tree);
}

inline void FuzzInitialize() {
  pinweaver_eal_storage_initialize_owner();
}

inline void FuzzAll() {
  FuzzInit();
  FuzzInitialize();
  FuzzLog();
  FuzzTreeData();
  FuzzDeriveKeys();
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  g_data_provider.reset(new FuzzedDataProvider(data, size));
  FuzzAll();
  return 0;
}
