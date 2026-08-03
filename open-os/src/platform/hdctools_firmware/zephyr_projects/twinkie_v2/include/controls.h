/* Copyright 2023 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _CONTROLS_H_
#define _CONTROLS_H_

#include <stdbool.h>

/**
 * @brief Initializes the control module, configure both cc enable pins as active
 *
 * @return 0 on success
 */
int controls_init(void);

/**
 * @brief Enables the cc1 pin
 *
 * @param en true for enable, false for disable
 */
void en_cc1(bool en);

/**
 * @brief Enables the cc2 pin
 *
 * @param en true for enable, false for disable
 */
void en_cc2(bool en);
#endif
