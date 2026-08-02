/* Copyright 2023 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <zephyr/drivers/adc/voltage_divider.h>
#include <zephyr/logging/log.h>

#include "sim_gpio.h"

LOG_MODULE_DECLARE(sim);

struct sim_ctx {
	/* GPIO Context */
	struct gpio_ctx gpio;
};

static struct sim_ctx sim_gpios[] = {GPIO_LIST_CTX(sim)};

static const struct voltage_divider_dt_spec vcc_sim_host_sns =
	VOLTAGE_DIVIDER_DT_SPEC_GET(DT_PATH(gpio, vcc_sim_host_sns));

int measure_sim_host_adc(int32_t *millivolt)
{
	int err = adc_channel_setup_dt(&vcc_sim_host_sns.port);
	if (err != 0) {
		return err;
	}

	/* Structure defining an ADC sampling sequence */
	struct adc_sequence sequence = {.buffer = millivolt,
					/* buffer size in bytes, not number of samples */
					.buffer_size = sizeof(*millivolt)};

	err = adc_sequence_init_dt(&vcc_sim_host_sns.port, &sequence);
	if (err != 0) {
		return err;
	}

	err = adc_read(vcc_sim_host_sns.port.dev, &sequence);
	if (err != 0) {
		return err;
	}

	err = adc_raw_to_millivolts_dt(&vcc_sim_host_sns.port, millivolt);
	if (err != 0) {
		return err;
	}

	err = voltage_divider_scale_dt(&vcc_sim_host_sns, millivolt);
	return err;
}

static struct gpio_ctx *find_gpio(enum GPIO_LABEL label, int idx)
{
	for (int i = 0; i < ARRAY_SIZE(sim_gpios); i++) {
		if (sim_gpios[i].gpio.label != label) {
			continue;
		}
		if (sim_gpios[i].gpio.idx != idx) {
			continue;
		}
		return &sim_gpios[i].gpio;
	}
	return NULL;
}

int read_gpio(enum GPIO_LABEL label, int idx)
{
	struct gpio_ctx *gpio = find_gpio(label, idx);

	if (gpio) {
		return gpio_pin_get_dt(&gpio->spec);
	}
	return 0;
}

int write_gpio(enum GPIO_LABEL label, int idx, bool state)
{
	struct gpio_ctx *gpio = find_gpio(label, idx);

	if (gpio) {
		gpio_pin_set_dt(&gpio->spec, state);
	}
	return 0;
}

void sim_gpio_init(void)
{
	for (int i = 0; i < ARRAY_SIZE(sim_gpios); i++) {
		gpio_flags_t flags = GPIO_OUTPUT;

		if (sim_gpios[i].gpio.label == GPIO_LABEL_SIM_CD) {
			flags = GPIO_INPUT;
		}
		gpio_pin_configure_dt(&sim_gpios[i].gpio.spec, flags);
	}

	// Set VSIM to 1.8v
	write_gpio(GPIO_LABEL_VSIM_VCC_SEL, 0, 0);
	// Enable SIM host
	write_gpio(GPIO_LABEL_SIM_HOST_EN, 0, 1);
}
