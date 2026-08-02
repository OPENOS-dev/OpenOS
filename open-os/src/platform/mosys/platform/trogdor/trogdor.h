/* Copyright 2020 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GOOGLE_TROGDOR_H
#define GOOGLE_TROGDOR_H

#include "mosys/platform.h"

#define TROGDOR_HOST_FIRMWARE_ROM_SIZE (8192 * 1024)

/* platform callbacks */
extern struct memory_cb trogdor_memory_cb;

#endif /* GOOGLE_TROGDOR_H */
