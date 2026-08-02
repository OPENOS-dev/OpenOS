/* Copyright 2023 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __STARFISH_SIM_GPIO_H__
#define __STARFISH_SIM_GPIO_H__

#include <stdint.h>

#include "gpio_helper.h"

/*
 * Initialize the SIM Pins
 */
void sim_gpio_init(void);

int write_gpio(enum GPIO_LABEL label, int idx, bool state);
int read_gpio(enum GPIO_LABEL label, int idx);

int measure_sim_host_adc(int *millivolt);

#endif /* __STARFISH_SIM_GPIO_H__ */
