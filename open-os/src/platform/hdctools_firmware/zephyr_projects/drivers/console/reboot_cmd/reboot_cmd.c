/* Copyright 2023 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <zephyr/shell/shell.h>
#include <zephyr/sys/reboot.h>

LOG_MODULE_DECLARE(console);

static void reboot_cmd(const struct shell *shell, size_t argc, char **argv)
{
	sys_reboot(SYS_REBOOT_WARM);
}

/* Creating reboot command */
SHELL_CMD_REGISTER(reboot, NULL, "Reboot", reboot_cmd);
