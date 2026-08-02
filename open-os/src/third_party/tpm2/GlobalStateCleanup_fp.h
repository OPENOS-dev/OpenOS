// Copyright 2023 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GLOBAL_STATE_CLEANUP_FP_H
#define GLOBAL_STATE_CLEANUP_FP_H

// Cleanup all global and static variables that should be initialize with zero.
// (The .bss section data.)
void GlobalStateCleanup(void);

#endif // GLOBAL_STATE_CLEANUP_FP_H
