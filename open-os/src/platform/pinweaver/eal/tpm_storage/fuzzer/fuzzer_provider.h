// Copyright 2021 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FUZZER_PROVIDER_H_
#define FUZZER_PROVIDER_H_

#include <fuzzer/FuzzedDataProvider.h>

#include <memory>

extern "C" {
#include "tss_types.h"
}

// Correct mocked salting key
constexpr TPM2B_ECC_PARAMETER kSaltingX = {.size = 32,
                                           .buffer = {
                                               0x77,
                                               0x01,
                                           }};
constexpr TPM2B_ECC_PARAMETER kSaltingY = {.size = 32,
                                           .buffer = {
                                               0x77,
                                               0x02,
                                           }};
constexpr TPMS_ECC_POINT kPubSaltingKey = {.x = kSaltingX, .y = kSaltingY};

extern std::unique_ptr<FuzzedDataProvider> g_data_provider;

// Probability in % of generating a pure random value or byte stream.
extern uint32_t g_pure_random_prob;
// Probability in % of generating an error response.
extern uint32_t g_error_response_prob;
// Probability in % of having an written NV space.
extern uint32_t g_written_nv_prob;

bool ConsumeBoolWithProbability(uint32_t probability);
void FillWithRandomData(void *buffer, size_t size);
void Fill2BInternal(uint16_t *size, unsigned char *buffer, uint16_t max_size);

template <class T> T Consume2B() {
  T tpm2b;
  Fill2BInternal(&tpm2b.size, tpm2b.buffer, sizeof(tpm2b.buffer));
  return tpm2b;
}

#endif // FUZZER_PROVIDER_H_