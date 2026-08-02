// Copyright 2021 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FUZZER_PROVIDER_H_
#define FUZZER_PROVIDER_H_

#include <fuzzer/FuzzedDataProvider.h>

#include <memory>

extern std::unique_ptr<FuzzedDataProvider> g_data_provider;

#endif // FUZZER_PROVIDER_H_