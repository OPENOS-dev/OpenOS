// Copyright 2023 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string.h>

#define GLOBAL_C
#include "InternalRoutines.h"
#include "PlatformData.h"

void GlobalStateCleanup(void) {
  memset(&global_struct, 0, sizeof(global_struct));
  memset(&global_plt_struct, 0, sizeof(global_plt_struct));
}
