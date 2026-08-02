/* Copyright 2023 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __DFU_RUNTIME_H__
#define __DFU_RUNTIME_H__

#include <zephyr/toolchain.h>

/*
 * Sets up the device to enter DFU mode. The system will reboot.
 */
FUNC_NORETURN void dfu_enter(void);

#endif /* __DFU_RUNTIME_H__ */
