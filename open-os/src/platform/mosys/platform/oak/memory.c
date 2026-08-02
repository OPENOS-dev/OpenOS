/*
 * Copyright 2014 Google LLC
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
#include "mosys/platform.h"

#include "oak.h"

#define OAK_DIMM_COUNT 2

enum oak_memory_config {
	HYNIX_DDR3_H9CCNNN8GTMLAR_NUD_1G,
	HYNIX_DDR3_H9CCNNNBJTALAR_NUD_2G,
	HYNIX_DDR3_H9CCNNNBLTBLAR_NUD_2G,
	MICRON_DDR3_MT52L256M32D1PF_107WTB_1G,
	MICRON_DDR3_MT52L512M32D2PF_107WTB_2G,
	SAMSUNG_DDR3_K4E6E304EB_EGCF_2G,
	SAMSUNG_DDR3_K4E6E304EE_EGCE_2G,
	SAMSUNG_DDR3_K4E8E304EE_EGCE_1G,
	SAMSUNG_DDR3_K4E8E324EB_EGCF_1G,
	SAMSUNG_DDR3_K4E6E304ED_EGCG_4G,
	NANYA_DDR3_NT6CL512T32AM_H0_4G,
	SAMSUNG_DDR3_K4E6E304EC_EGCG_4G,
	MEM_UNKNOWN,
};

static int get_memory_config(struct platform_intf *intf)
{
	uint32_t ram_code;

	if (fdt_get_ram_code(&ram_code) < 0) {
		lprintf(LOG_ERR, "Unable to obtain RAM code.\n");
		return -1;
	}

	switch (ram_code) {
	case 0:
		return HYNIX_DDR3_H9CCNNN8GTMLAR_NUD_1G;
	case 1:
		return SAMSUNG_DDR3_K4E8E304EE_EGCE_1G;
	case 2:
		return SAMSUNG_DDR3_K4E6E304EE_EGCE_2G;
	case 3:
		return HYNIX_DDR3_H9CCNNNBLTBLAR_NUD_2G;
	case 4:
		return SAMSUNG_DDR3_K4E6E304EB_EGCF_2G;
	case 5:
		return SAMSUNG_DDR3_K4E8E324EB_EGCF_1G;
	case 6:
		return MICRON_DDR3_MT52L512M32D2PF_107WTB_2G;
	case 7:
		return HYNIX_DDR3_H9CCNNNBJTALAR_NUD_2G;
	case 8:
		return MICRON_DDR3_MT52L256M32D1PF_107WTB_1G;
	case 9:
		return SAMSUNG_DDR3_K4E6E304ED_EGCG_4G;
	case 10:
		return NANYA_DDR3_NT6CL512T32AM_H0_4G;
	case 11:
		return SAMSUNG_DDR3_K4E6E304EC_EGCG_4G;

	default:
		lprintf(LOG_ERR, "Unable to determine memory configuration\n");
	}

	return MEM_UNKNOWN;
}

/*
 * dimm_count  -  return total number of dimm slots
 *
 * @intf:       platform interface
 *
 * returns dimm slot count
 */
static int dimm_count(struct platform_intf *intf)
{
	return OAK_DIMM_COUNT;
}

static int get_mem_info(struct platform_intf *intf, int dimm,
			const struct nonspd_mem_info **info)
{
	switch (get_memory_config(intf)) {
	case HYNIX_DDR3_H9CCNNN8GTMLAR_NUD_1G:
		*info = &hynix_lpddr3_h9ccnnn8gtmlar_nud;
		break;
	case HYNIX_DDR3_H9CCNNNBJTALAR_NUD_2G:
		*info = &hynix_lpddr3_h9ccnnnbjtalar_nud;
		break;
	case HYNIX_DDR3_H9CCNNNBLTBLAR_NUD_2G:
		*info = &hynix_lpddr3_h9ccnnnbltblar_nud;
		break;
	case MICRON_DDR3_MT52L256M32D1PF_107WTB_1G:
		*info = &micron_lpddr3_mt52l256m32d1pf_107wtb;
		break;
	case MICRON_DDR3_MT52L512M32D2PF_107WTB_2G:
		*info = &micron_lpddr3_mt52l512m32d2pf_107wtb;
		break;
	case SAMSUNG_DDR3_K4E6E304EB_EGCF_2G:
		*info = &samsung_lpddr3_k4e6e304eb_egcf;
		break;
	case SAMSUNG_DDR3_K4E6E304EE_EGCE_2G:
		*info = &samsung_lpddr3_k4e6e304ee_egce;
		break;
	case SAMSUNG_DDR3_K4E8E304EE_EGCE_1G:
		*info = &samsung_lpddr3_k4e8e304ee_egce;
		break;
	case SAMSUNG_DDR3_K4E8E324EB_EGCF_1G:
		*info = &samsung_lpddr3_k4e8e324eb_egcf;
		break;
	case SAMSUNG_DDR3_K4E6E304ED_EGCG_4G:
		*info = &samsung_lpddr3_k4e6e304ed_egcg;
		break;
	case NANYA_DDR3_NT6CL512T32AM_H0_4G:
		*info = &nanya_lpddr3_nt6cl512t32am_h0;
		break;
	case SAMSUNG_DDR3_K4E6E304EC_EGCG_4G:
		*info = &samsung_lpddr3_k4e6e304ec_egcg;
		break;
	default:
		return -1;
	}
	return 0;
}

struct memory_cb oak_memory_cb = {
	.dimm_count = dimm_count,
	.nonspd_mem_info = &get_mem_info,
};
