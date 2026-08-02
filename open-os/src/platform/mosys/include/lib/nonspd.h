/*
 * Copyright 2012 Google LLC
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
 *
 * SPD-like memory info for memory without real SPD data.
 */

#ifndef LIB_NONSPD_H__
#define LIB_NONSPD_H__

#include <inttypes.h>

#include "lib/spd.h"
#include "lib/smbios.h"
#include "mosys/log.h"

#define PART_NUM_LEN 33

struct nonspd_mem_info {
	/* DRAM type */
	enum spd_dram_type dram_type;

	/* DIMM type */
	union {
		enum ddr3_module_type ddr3_type;
	} module_type;

	/* Module Size (in mbits) */
	unsigned int module_size_mbits;

	/* Number of ranks */
	unsigned int num_ranks;

	/* SDRAM device width (including ECC) */
	unsigned int device_width;

	/* Module Manufacturer ID */
	struct {
		uint8_t lsb;
		uint8_t msb;
	} module_mfg_id;

	/* Module Manufacturing Location */
	uint8_t mfg_loc;

	/* Module Manufacturing Date */
	struct {
		uint8_t year;
		uint8_t week;
	} module_mfg_date;

	/* Module Part Number */
	uint8_t part_num[PART_NUM_LEN];

	/* DRAM Manufacturer ID */
	struct {
		uint8_t lsb;
		uint8_t msb;
	} dram_mfg_id;

	/* Module revision code */
	uint8_t revision[2];
};

extern int nonspd_print_field(struct kv_pair *kv,
			      const struct nonspd_mem_info *info,
			      enum spd_field_type type);

int spd_set_nonspd_info_from_smbios(struct platform_intf *intf, int dimm,
				    const struct nonspd_mem_info **info);

#include "lib/nonspd_modules.h"

#endif /* LIB_NONSPD_H__ */
