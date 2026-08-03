/* Copyright 2023 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <stdio.h>

#include <zephyr/shell/shell.h>

#include "drivers/console_util.h"
#include "sim.h"
#include "sim.h"
#include "sim_button.h"
#include "sim_info.h"
#include "sim_timer.h"
#include "sim_gpio.h"

LOG_MODULE_DECLARE(console);

/**
 * Connects a specific SIM slot to the DUT.
 *
 * Connects the SIM slot to the DUT and if needed starts the disconnect
 * sequence for an existing slot. This assumes that a user knows which slots
 * are valid for their application.
 *
 * -n Slot number in the range of 0 to 7
 */
static void connect_cmd(const struct shell *shell, size_t argc, char **argv)
{
	struct param slot_number = PARAM_PARSER_LONG("-n", "SIM Slot number 0-7", 0, NUM_SIMS - 1);
	struct param *args[] = {&slot_number};
	struct command cmd = CMD_PARSER("Connect to a specific SIM Slot", args);

	if (!console_util_parse(argc, argv, &cmd)) {
		return;
	}
	if (!slot_number.found) {
		LOG_ERR("No valid SIM slot");
		return;
	}
	struct mux_state new_state = {.slot_number = (uint8_t)slot_number.dst_long.dst,
				      .enabled = true};

	sim_slot_change(&new_state, MUX_SOURCE_CONSOLE);
}

/**
 * Ejects the currently connected SIM slot.
 */
static void eject_cmd(const struct shell *shell, size_t argc, char **argv)
{
	struct command cmd = CMD_HELP("Eject the SIM Slot");

	if (!console_util_parse(argc, argv, &cmd)) {
		return;
	}

	struct mux_state new_state = *sim_slot_get_current_mux();

	new_state.enabled = false;
	sim_slot_change(&new_state, MUX_SOURCE_CONSOLE);
}

/**
 * Prints a list of SIM slots and if they have been detected.
 */
static void status_cmd(const struct shell *shell, size_t argc, char **argv)
{
	struct command cmd = CMD_HELP("Print the Starfish status");

	if (!console_util_parse(argc, argv, &cmd)) {
		return;
	}

	int millivolt = 0;
	LOG_INF("ADC Start");
	int res = measure_sim_host_adc(&millivolt);
	if (res != 0) {
		LOG_ERR("ADC Error %d", res);
	} else {
		LOG_INF("VCC SIM = %d", millivolt);
	}

	uint8_t detected = sim_slot_get_detected_sims();

	for (int i = 0; i < NUM_SIMS; i++) {
		char *status;

		if (detected & BIT(i)) {
			status = "Found";
		} else {
			status = "None";
		}

		LOG_INF("SIM %d = %s", i, status);
	}
}

/**
 * Read and edit the stored attributes of the SIM slots.
 *
 * SIM slots have 2 user writable flags, a numeric slot number and a
 * human friendly slot string allowing users or tools to identify what
 * should be stored on a given Starfish device.
 *
 * -n Slot number in range of 0-7
 * -I Edit the SIM slot's ID by providing a positive number under 32-bits
 * -N Edit the SIM slot's name by providing a text string under 64 bytes long.
 * -R Reset the SIM slot's attributes
 */
static void slot_cmd(const struct shell *shell, size_t argc, char **argv)
{
	struct param slot_number = PARAM_PARSER_LONG(
		"-n", "SIM Slot number 0-7, required when editing stored info", 0, NUM_SIMS - 1);
	struct param slot_id = PARAM_PARSER_LONG("-I", "Edit stored SIM Slot ID", 0, INT_MAX);
	struct param slot_name = PARAM_PARSER_STR("-N", "Edit stored SIM Slot name");
	struct param slot_reset = PARAM_PARSER_FLAG("-R", "Reset stored SIM Slot Descriptors");

	struct param *args[] = {&slot_number, &slot_id, &slot_name, &slot_reset};

	struct command cmd = CMD_PARSER("List or Edit the stored SIM Slot Descriptors \n"
					"By default this will list all the descriptors",
					args);

	if (!console_util_parse(argc, argv, &cmd)) {
		return;
	}

	int number = (int)(slot_number.dst_long.dst);

	// Check if we're editing the SIM slots
	if (slot_id.found || slot_name.found || slot_reset.found) {
		if (!slot_number.found) {
			LOG_ERR("No valid SIM slot");
			return;
		}

		if (slot_reset.found && (slot_id.found || slot_name.found)) {
			LOG_ERR("Conflicting settings");
			return;
		}
		if (slot_id.found) {
			int32_t id = (int32_t)(slot_id.dst_long.dst);

			sim_setting_set_slot_id(number, &id);
		}
		if (slot_name.found) {
			struct slot_name name;

			snprintf(name.name, sizeof(name.name), "%s", slot_name.dst_str.dst);
			sim_setting_set_slot_name(number, &name);
		}
		if (slot_reset.found) {
			sim_setting_set_slot_id(number, NULL);
			sim_setting_set_slot_name(number, NULL);
		}
	}
	// Print the results
	for (int i = 0; i < NUM_SIMS; i++) {
		if (slot_number.found && number != i) {
			continue;
		}

		int id;
		struct slot_name name;
		bool valid = sim_setting_get_slot_id(i, &id);

		if (!valid) {
			id = 0;
		}

		valid = sim_setting_get_slot_name(i, &name);
		if (!valid) {
			name.name[0] = '\0';
		}
		LOG_INF("Slot Number:%u, Slot ID:%d, Slot Name:%s", i, id, name.name);
	}
}

/**
 * Adjust the default timing of operations like switching the SIM cards.
 *
 * -d Adjust the minimum disconnection time
 * -t New value in milliseconds
 * -S Store value to persistent for next reboot
 */
static void timer_cmd(const struct shell *shell, size_t argc, char **argv)
{
	struct param disconnect =
		PARAM_PARSER_FLAG("-d", "Minimum disconnection time between switching slots");
	struct param time = PARAM_PARSER_LONG("-t", "New value in milliseconds", 0, 100000);
	struct param reset = PARAM_PARSER_FLAG("-r", "Reset the value to default");
	struct param store = PARAM_PARSER_FLAG("-S", "Store the value");

	struct param *args[] = {&disconnect, &time, &reset, &store};

	struct command cmd = CMD_PARSER("List or Edit the built in timers", args);

	if (!console_util_parse(argc, argv, &cmd)) {
		return;
	}
	if (disconnect.found && reset.found) {
		LOG_ERR("Conflicting settings");
		return;
	}
	if (!disconnect.found) {
		LOG_ERR("Require 1 timer");
		return;
	}
	if (disconnect.found || reset.found) {
		int value = -1;

		if (time.found) {
			value = time.dst_long.dst;
		}
		sim_timer_set(value, reset.found, store.found);
	}
}

/*
 * Configure the SIM slot boot mode. This determines the initial state
 * when a device reboots.
 *
 * -D Disable the SIM slot when we reboot.
 * -R Remember the last SIM slot and it's state.
 */
static void bootmode_cmd(const struct shell *shell, size_t argc, char **argv)
{
	struct param disconnect = PARAM_PARSER_FLAG("-D", "Boot with SIM disconnected");
	struct param remember = PARAM_PARSER_FLAG("-R", "Remember last SIM Slot state");
	struct param *args[] = {&disconnect, &remember};
	struct command cmd = CMD_PARSER("Configure the SIM card boot mode", args);

	if (!console_util_parse(argc, argv, &cmd)) {
		return;
	}
	if (disconnect.found && remember.found) {
		LOG_ERR("Conflicting settings");
		return;
	}
	if (disconnect.found || remember.found) {
		sim_slot_save_mux_boot(remember.found);
	}
}

/*
 * Access the button control interface.
 *
 * This can update the button interface to turn it on or off. The command
 * can force the state to prevent switches and set the mode for reboot.
 * It will then print the button control state to the console.
 *
 * -e or -d Enable and Disable the buttons respectively and one of these is
 *        required to perform all updates.
 * -f Forces the mode and disables the mode switch
 * -S Stores the mode in the device
 */
static void button_cmd(const struct shell *shell, size_t argc, char **argv)
{
	struct param enable = PARAM_PARSER_FLAG("-e", "Enable the buttons");
	struct param disable = PARAM_PARSER_FLAG("-d", "Disable the buttons");
	struct param reset = PARAM_PARSER_FLAG("-r", "Reset the value to default");
	struct param store = PARAM_PARSER_FLAG("-S", "Store the state on boot");
	struct param *args[] = {&enable, &disable, &reset, &store};
	struct command cmd = CMD_PARSER("Configure the buttons and boot mode", args);

	if (!console_util_parse(argc, argv, &cmd)) {
		return;
	}
	if ((enable.found + disable.found + reset.found) != 1) {
		LOG_ERR("Invalid settings");
		return;
	}

	struct button_ctrl new_state = {
		.enabled = enable.found,
	};

	sim_button_save(&new_state, reset.found, store.found);

	const struct button_ctrl *state = sim_button_get_state();

	LOG_INF("Buttons Enabled:%d", state->enabled);
}

static void erase_cmd(const struct shell *shell, size_t argc, char **argv)
{
	struct command cmd = CMD_HELP("Erase the memory");

	if (!console_util_parse(argc, argv, &cmd)) {
		return;
	}
}

SHELL_STATIC_SUBCMD_SET_CREATE(sim_cmds, SHELL_CMD(connect, NULL, "Connect sim card", connect_cmd),
			       SHELL_CMD(eject, NULL, "Eject sim card", eject_cmd),
			       SHELL_CMD(status, NULL, "Print sim card status.", status_cmd),
			       SHELL_CMD(erase, NULL, "Erase persistent memory", erase_cmd),
			       SHELL_CMD(slot, NULL, "Slot Descriptors", slot_cmd),
			       SHELL_CMD(timer, NULL, "Timers", timer_cmd),
			       SHELL_CMD(bootmode, NULL, "Boot mode", bootmode_cmd),
			       SHELL_CMD(button, NULL, "Button mode", button_cmd),
			       SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(sim, &sim_cmds, "Sim card commands", NULL);
