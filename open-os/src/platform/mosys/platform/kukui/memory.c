/*
 * Copyright 2019 Google LLC
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *    * Neither the name of Google LLC nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "lib/fdt.h"
#include "lib/nonspd.h"

#include "mosys/log.h"
#include "mosys/mosys.h"
#include "mosys/platform.h"

#include "kukui.h"

#define DIMM_COUNT 1

/*
 * dimm_count  -  return total number of dimm slots
 *
 * @intf:       platform interface
 *
 * returns dimm slot count
 */
static int dimm_count(struct platform_intf *intf)
{
	return DIMM_COUNT;
}

static const struct nonspd_mem_info *kukui_dram[] = {
	[0] = &samsung_lpddr4x_k4ube3d4aa_mgcr,
	[1] = &hynix_lpddr4x_h9hcnnncpmalhr_nee,
	[2] = &micron_lpddr4x_mt53e1g32d4nq_046wte,
	[3] = &samsung_lpddr4x_kmdh6001da_b422,
	[4] = &samsung_lpddr4x_kmdp6001da_b425,
	[5] = &micron_lpddr4x_mt29vzzzad8dqksl,
	[6] = &samsung_lpddr4x_kmdv6001da_b620,
	[7] = &sandisk_lpddr4x_sdada4cr_128g,
	[8] = &samsung_lpddr4x_k4ube3d4aa_mgcl,
	[9] = &micron_lpddr4x_mt53e2g32d4nq_046wta,
	[10] = &hynix_lpddr4x_h9hcnnncpmmlxr_nee,
	[11] = &micron_lpddr4x_mt29vzzzad9gqfsm_046,
	/* Table shared by Fennel, Cerise, Stern, Makomo, Munna, offset = 0x10 */
	[0x10] = &samsung_lpddr4x_k4ube3d4aa_mgcr,
	[0x11] = &hynix_lpddr4x_h9hcnnncpmalhr_nee,
	[0x12] = &micron_lpddr4x_mt53e1g32d4nq_046wte,
	[0x13] = &samsung_lpddr4x_k4ube3d4aa_mgcl,
	[0x14] = &hynix_lpddr4x_h9hcnnncpmmlxr_nee,
	[0x15] = &hynix_lpddr4x_h9hcnnnfammlxr_nee,
	[0x16] = &micron_lpddr4x_mt53e2g32d4nq_046wta,
	[0x17] = &micron_lpddr4x_mt53e1g32d2np_046wta,
	[0x18] = &micron_lpddr4x_mt53e1g32d2np_046wtb,
	[0x19] = &hynix_lpddr4x_h54g56cyrbx247,
	[0x1a] = &samsung_lpddr4x_k4ube3d4ab_mgcl,
	[0x1b] = &hynix_lpddr4x_h54g68cyrbx248,

	/* Table shared by Kakadu and its variants, offset = 0x20 */
	[0x20] = &samsung_lpddr4x_k4ube3d4aa_mgcr,
	[0x21] = &hynix_lpddr4x_h9hcnnncpmalhr_nee,
	[0x22] = &micron_lpddr4x_mt53e1g32d4nq_046wte,
	[0x23] = &samsung_lpddr4x_kmdh6001da_b422,
	[0x24] = &samsung_lpddr4x_kmdp6001da_b425,
	[0x25] = &micron_lpddr4x_mt29vzzzad8dqksl,
	[0x26] = &samsung_lpddr4x_kmdv6001da_b620,
	[0x27] = &sandisk_lpddr4x_sdada4cr_128g,
	[0x28] = &micron_lpddr4x_mt29vzzzcd9gqkpr_046,
	[0x29] = &foresee_lpddr4x_feprf6432_58a1930,
	/* Cozmo dram table, offset = 0x30 */
	[0x30] = &samsung_lpddr4x_k4ube3d4aa_mgcr,
	[0x31] = &hynix_lpddr4x_h9hcnnncpmalhr_nee,
	[0x32] = &micron_lpddr4x_mt53e1g32d4nq_046wte,
	[0x33] = &samsung_lpddr4x_k4ube3d4ab_mgcl,
	[0x34] = &hynix_lpddr4x_h54g68cyrbx248,
	[0x38] = &samsung_lpddr4x_k4ube3d4aa_mgcl,
	[0x39] = &micron_lpddr4x_mt53e2g32d4nq_046wta,
	[0x3a] = &hynix_lpddr4x_h9hcnnncpmmlxr_nee,
	[0x3b] = &hynix_lpddr4x_h9hcnnnfammlxr_nee,

	/* Kappa dram table, offset = 0x40 */
	[0x40] = &samsung_lpddr4x_k4ube3d4aa_mgcr,
	[0x41] = &hynix_lpddr4x_h9hcnnncpmalhr_nee,
	[0x42] = &micron_lpddr4x_mt53e1g32d4nq_046wte,
	[0x43] = &micron_lpddr4x_mt53e1g32d2np_046wta,
	[0x44] = &micron_lpddr4x_mt53e1g32d2np_046wtb,
	[0x45] = &hynix_lpddr4x_h54g56cyrbx247,
	[0x46] = &samsung_lpddr4x_k4ube3d4ab_mgcl,
	[0x48] = &samsung_lpddr4x_k4ube3d4aa_mgcl,
	[0x49] = &micron_lpddr4x_mt53e2g32d4nq_046wta,
	[0x4a] = &hynix_lpddr4x_h9hcnnncpmmlxr_nee,
	[0x4b] = &micron_lpddr4x_mt29vzzzad9gqfsm_046,

	/* Table shared by Burnet and Esche, offset = 0x50 */
	[0x50] = &samsung_lpddr4x_k4ube3d4aa_mgcr,
	[0x51] = &micron_lpddr4x_mt53e2g32d4nq_046wtc,
	[0x52] = &micron_lpddr4x_mt53e1g32d4nq_046wte,
	[0x53] = &samsung_lpddr4x_k4ube3d4aa_mgcl,
	[0x54] = &hynix_lpddr4x_h9hcnnncpmmlxr_nee,
	[0x55] = &hynix_lpddr4x_h9hcnnnfammlxr_nee,
	[0x56] = &micron_lpddr4x_mt53e2g32d4nq_046wta,
	[0x57] = &micron_lpddr4x_mt53e1g32d2np_046wta,
	[0x58] = &micron_lpddr4x_mt53e1g32d2np_046wtb,
	[0x59] = &hynix_lpddr4x_h54g56cyrbx247,
	[0x5a] = &samsung_lpddr4x_k4ube3d4ab_mgcl,
	[0x5b] = &hynix_lpddr4x_h54g68cyrbx248,
};

static int get_mem_info(struct platform_intf *intf, int dimm,
			const struct nonspd_mem_info **info)
{
	uint32_t ram_code;

	if (fdt_get_ram_code(&ram_code) < 0) {
		lprintf(LOG_ERR, "Unable to obtain RAM code.\n");
		return -1;
	}
	if (ram_code >= ARRAY_SIZE(kukui_dram) || !kukui_dram[ram_code]) {
		lprintf(LOG_ERR, "Invalid RAM code: %d.\n", ram_code);
		return -1;
	}
	*info = kukui_dram[ram_code];
	return 0;
}

struct memory_cb kukui_memory_cb = {
	.dimm_count = dimm_count,
	.nonspd_mem_info = get_mem_info,
};
