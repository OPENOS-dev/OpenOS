/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License v2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 021110-1307, USA.
 */

#ifndef _FFU_H_
#define _FFU_H_

#include <linux/types.h>

/*
 * eMMC5.0 Field Firmware Update (FFU) opcodes
*/
#define MMC_FFU_INVOKE_OP 302

#define FFU_NAME_LEN 80  /* Name of the firmware file udev should find */

enum mmc_ffu_hack_type {
	MMC_OVERRIDE_FFU_ARG = 0,
	MMC_HACK_LEN,
};

struct mmc_ffu_hack {
	enum mmc_ffu_hack_type type;
	__u64 value;
};

struct mmc_ffu_args {
	char name[FFU_NAME_LEN];
	__u32 hack_nb;
	struct mmc_ffu_hack hack[0];
};

#endif /* _FFU_H_ */
