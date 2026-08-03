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
 * Serial Presence Detect (SPD) code for access of SPDs on DIMMs.
 */

#ifndef LIB_SPD_H__
#define LIB_SPD_H__

#include <inttypes.h>

#include "lib/val2str.h"

/* different types of DRAM (fundamental memory type) for SPD */
enum spd_dram_type {
	SPD_DRAM_TYPE_UNDEFINED = 0,
	SPD_DRAM_TYPE_DDR3 = 0x0b,
	SPD_DRAM_TYPE_LPDDR3 = 0xf1,
	SPD_DRAM_TYPE_JEDEC_LPDDR3 = 0x0f,
	SPD_DRAM_TYPE_DDR4 = 0x0c,
	SPD_DRAM_TYPE_LPDDR4 = 0x10,
	SPD_DRAM_TYPE_LPDDR4X = 0x11,
	SPD_DRAM_TYPE_DDR5 = 0x12,
	SPD_DRAM_TYPE_LPDDR5 = 0x13,
};

enum ddr3_module_type {
	DDR3_MODULE_TYPE_UNDEFINED = 0,
	DDR3_MODULE_TYPE_RDIMM,
	DDR3_MODULE_TYPE_UDIMM,
	DDR3_MODULE_TYPE_SO_DIMM,
	DDR3_MODULE_TYPE_MICRO_DIMM,
	DDR3_MODULE_TYPE_MINI_RDIMM,
	DDR3_MODULE_TYPE_MINI_UDIMM,
	DDR3_MODULE_TYPE_MINI_CDIMM,
	DDR3_MODULE_TYPE_72b_SO_UDIMM,
	DDR3_MODULE_TYPE_72b_SO_RDIMM,
	DDR3_MODULE_TYPE_72b_SO_CDIMM,
	DDR3_MODULE_TYPE_LRDIMM,
};

static const struct valstr ddr3_module_type_lut[] = {
	{ DDR3_MODULE_TYPE_UNDEFINED, "Undefined" },
	{ DDR3_MODULE_TYPE_RDIMM, "RDIMM" },
	{ DDR3_MODULE_TYPE_UDIMM, "UDIMM" },
	{ DDR3_MODULE_TYPE_SO_DIMM, "SO-DIMM" },
	{ DDR3_MODULE_TYPE_MICRO_DIMM, "MICRO-DIMM" },
	{ DDR3_MODULE_TYPE_MINI_RDIMM, "MINI-RDIMM" },
	{ DDR3_MODULE_TYPE_MINI_UDIMM, "MINI-UDIMM" },
	{ DDR3_MODULE_TYPE_MINI_CDIMM, "MINI-CDIMM" },
	{ DDR3_MODULE_TYPE_72b_SO_UDIMM, "72b-SO-UDIMM" },
	{ DDR3_MODULE_TYPE_72b_SO_RDIMM, "72b-SO-RDIMM" },
	{ DDR3_MODULE_TYPE_72b_SO_CDIMM, "72b-SO-CDIMM" },
	{ DDR3_MODULE_TYPE_LRDIMM, "LRDIMM" },
};

/*
 * various SPD fields that can be retrieved
 * these are found in different locations on DDR/DDR2 vs. FBDIMM
 */
enum spd_field_type {
	SPD_GET_DRAM_TYPE, /* DRAM type */
	SPD_GET_MODULE_TYPE, /* DIMM type */
	SPD_GET_MFG_ID, /* Module Manufacturer ID */
	SPD_GET_PART_NUMBER, /* Module Part Number */
	SPD_GET_SIZE, /* Module Size (in MB) */
	SPD_GET_RANKS, /* Number of ranks */
	SPD_GET_WIDTH, /* SDRAM device width */
};

#endif /* LIB_SPD_H__ */
