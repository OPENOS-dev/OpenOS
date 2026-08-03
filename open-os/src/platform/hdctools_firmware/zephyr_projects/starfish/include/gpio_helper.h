/* Copyright 2023 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __STARFISH_GPIO_H__
#define __STARFISH_GPIO_H__

#include <zephyr/drivers/gpio.h>

enum GPIO_LABEL {
	/*
	 * SIM_HOST_EN: Enable and disable the SIM TXS4555 level translator
	 * Active high and deactivation starts starts a shutdown sequence
	 */
	GPIO_LABEL_SIM_HOST_EN,
	/*
	 * VSIM_VCC_SEL: Controls SIM voltage provided by the TXS4555
	 * Low sets internal SIM cards to 1.8V, high to 3.0V
	 */
	GPIO_LABEL_VSIM_VCC_SEL,
	/*
	 * VCC_SIM_HOST_SNS: Analog input to measure the VCC_SIM_HOST voltage.
	 */
	GPIO_LABEL_VCC_SIM_HOST_SNS,
	/*
	 * SIM_DET_CNTRL: Controls if DET_V_OUT is connected to the EN or OFF
	 * Active high connects DET_V_OUT to DET_V_EN
	 * Low connects DET_V_OUT to DET_V_OFF
	 */
	GPIO_LABEL_SIM_DET_CNTRL,
	/*
	 * SIM_MUX_CLK_EN: Enable the CLK signal mux
	 */
	GPIO_LABEL_SIM_MUX_CLK_EN,
	/*
	 * SIM_MUX_RST_EN: Enable the RST signal mux
	 */
	GPIO_LABEL_SIM_MUX_RST_EN,
	/*
	 * SIM_MUX_DATA_EN: Enable the DATA signal mux
	 */
	GPIO_LABEL_SIM_MUX_DATA_EN,
	/*
	 * SIM_SEL_NEXT: Button to select the next highest sim slot
	 */
	GPIO_LABEL_BUTTON_NEXT,
	/*
	 * SIM_SEL_PREV: Button to select the next lower sim slot
	 */
	GPIO_LABEL_BUTTON_PREV,
	/*
	 * Renamed from SIM_CNTRL_MANUAL_REMOTE for clarity:
	 * Special purpose mode button. Allows switching between manual/remote
	 * mode and toggling devices
	 */
	GPIO_LABEL_BUTTON_MODE,
	/*
	 * Renamed from SIM_SEL0 - SIM_SEL2 for clarity:
	 * Selects all of the mux address to the slots 0-7.
	 */
	GPIO_LABEL_SIM_MUX_ADDR,
	/*
	 * Renamed from MODE0_LED and MODE1_LED due to inconsistent naming:
	 * Controls the 2 mode LEDs allowing for multiple state representations.
	 */
	GPIO_LABEL_LED_MODE,
	/*
	 * Renamed from UIM1_ENABLED_LED - UIM7_ENABLED_LED, ESIM_ENABLED_LED
	 * Original naming convention is inconsistent naming convention.
	 * Controls the LED Status for slots 0-7
	 */
	GPIO_LABEL_LED_SLOT,
	/*
	 * Renamed from UIM1_CD - UIM7_CD, ESIM_CD:
	 * Original naming convention is inconsistent naming convention.
	 * Reads the Chip Detect Status for slots 0-7
	 */
	GPIO_LABEL_SIM_CD,
};

/*
 * Defines a GPIO context state which
 */
struct gpio_ctx {
	/* GPIO pin spec */
	struct gpio_dt_spec spec;
	/* GPIO group label */
	enum GPIO_LABEL label;
	/* GPIO group index */
	int idx;
};

#define _GPIO_CTX(node, y, idx_)                                                                   \
	{                                                                                          \
		.gpio = {                                                                          \
			.spec = GPIO_DT_SPEC_GET_BY_IDX(node, gpios, idx_),                        \
			.label = _CONCAT(GPIO_LABEL_, DT_STRING_UPPER_TOKEN(node, label)),         \
			.idx = idx_,                                                               \
		}                                                                                  \
	}

#define COMMA          (, )
#define _GPIO_CB(node) DT_FOREACH_PROP_ELEM_SEP(node, gpios, _GPIO_CTX, COMMA)

#define GPIO_LIST_CTX(node) DT_FOREACH_CHILD_SEP(DT_PATH(gpio, node), _GPIO_CB, COMMA)

#endif /* __STARFISH_GPIO_H__ */
