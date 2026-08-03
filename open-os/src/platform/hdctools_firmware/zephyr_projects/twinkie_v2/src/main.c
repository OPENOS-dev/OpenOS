/* Copyright 2023 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <zephyr/kernel.h>
#include <zephyr/smf.h>
#include <zephyr/drivers/gpio.h>

#include <zephyr/logging/log.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/shell/shell.h>

#include "controls.h"
#include "meas.h"
#include "view.h"
#include "model.h"

int main(void)
{
	const struct device *dev_pd_analyzer = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

	if (usb_enable(NULL)) {
		return -1;
	}

	meas_init();
	controls_init();
	model_init(dev_pd_analyzer);
	view_init();

	return 0;
}
