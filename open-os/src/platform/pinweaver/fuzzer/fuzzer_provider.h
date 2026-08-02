/* Copyright 2021 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <fuzzer/FuzzedDataProvider.h>

#include <memory>

extern std::unique_ptr<FuzzedDataProvider> g_data_provider;

extern uint64_t g_time_base;