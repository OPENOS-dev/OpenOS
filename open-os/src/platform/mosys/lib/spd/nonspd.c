/* Copyright 2014 Google LLC
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
 * nonspd.c: Functions for pretty printing memory info for systems without SPD.
 */

#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "jedec_id.h"
#include "lib/nonspd.h"
#include "lib/val2str.h"
#include "mosys/platform.h"
#include "mosys/kv_pair.h"
#include "mosys/log.h"
#include "mosys/mosys.h"

/*
 * nonspd_print_field  -  add common DDR SPD fields into key=value pair
 *
 * @kv:         key=value pair
 * @info:       nonspd memory info
 * @type:       type of field to retrieve
 *
 * returns 1 to indicate data added to key=value pair
 * returns 0 to indicate no data added
 * returns <0 to indicate error
 *
 */
int nonspd_print_field(struct kv_pair *kv, const struct nonspd_mem_info *info,
		       enum spd_field_type type)
{
	int ret = 0;

	switch (type) {
	case SPD_GET_DRAM_TYPE:
		switch (info->dram_type) {
		case SPD_DRAM_TYPE_DDR3:
			kv_pair_add(kv, "dram", "DDR3");
			break;
		case SPD_DRAM_TYPE_DDR4:
			kv_pair_add(kv, "dram", "DDR4");
			break;
		case SPD_DRAM_TYPE_JEDEC_LPDDR3:
		case SPD_DRAM_TYPE_LPDDR3:
			kv_pair_add(kv, "dram", "LPDDR3");
			break;
		case SPD_DRAM_TYPE_LPDDR4:
			kv_pair_add(kv, "dram", "LPDDR4");
			break;
		case SPD_DRAM_TYPE_LPDDR4X:
			kv_pair_add(kv, "dram", "LPDDR4X");
			break;
		case SPD_DRAM_TYPE_DDR5:
			kv_pair_add(kv, "dram", "DDR5");
			break;
		case SPD_DRAM_TYPE_LPDDR5:
			kv_pair_add(kv, "dram", "LPDDR5");
			break;
		default:
			break;
		}
		ret = 1;
		break;

	case SPD_GET_MODULE_TYPE:
		switch (info->dram_type) {
		case SPD_DRAM_TYPE_DDR3:
		case SPD_DRAM_TYPE_LPDDR3:
		case SPD_DRAM_TYPE_JEDEC_LPDDR3:
			kv_pair_add(kv, "module",
				    val2str(info->module_type.ddr3_type,
					    ddr3_module_type_lut));
			ret = 1;
			break;
		case SPD_DRAM_TYPE_DDR4:
		default:
			ret = -1;
			break;
		}
		break;

	case SPD_GET_MFG_ID: {
		uint8_t manuf_lsb = info->module_mfg_id.lsb & 0x7f;
		uint8_t manuf_msb = info->module_mfg_id.msb & 0x7f;
		const char *tstr;

		tstr = jedec_manufacturer(manuf_lsb, manuf_msb);

		if (tstr != NULL) {
			kv_pair_fmt(kv, "module_mfg", "%u-%u: %s",
				    manuf_lsb + 1, manuf_msb, tstr);
		} else if (info->revision[0] || info->revision[1]) {
			kv_pair_fmt(kv, "module_mfg", "%02x,%02x%02x",
				    info->module_mfg_id.lsb, info->revision[0],
				    info->revision[1]);
		} else {
			kv_pair_fmt(kv, "module_mfg", "%u-%u", manuf_lsb + 1,
				    manuf_msb);
		}
		ret = 1;
		break;
	}

	case SPD_GET_PART_NUMBER: {
		char part[sizeof(info->part_num) + 1];

		memcpy(part, &info->part_num[0], sizeof(info->part_num));
		part[sizeof(info->part_num)] = '\0';
		kv_pair_fmt(kv, "part_number", "%s", part);

		ret = 1;
		break;
	}

	case SPD_GET_SIZE: {
		/* translate mbits to mbytes */
		kv_pair_fmt(kv, "size_mb", "%u", info->module_size_mbits / 8);
		ret = 1;
		break;
	}

	case SPD_GET_RANKS: {
		kv_pair_fmt(kv, "ranks", "%d", info->num_ranks);
		ret = 1;
		break;
	}

	case SPD_GET_WIDTH: {
		kv_pair_fmt(kv, "width", "%d", info->device_width);
		ret = 1;
		break;
	}

	default:
		break;
	}

	return ret;
}

// This one is reserved for storing mem info from SMBIOS if no explicit entry
// was added above.
static struct nonspd_mem_info part_extracted_from_smbios = {
	.part_num = { 'U', 'N', 'P', 'R', 'O', 'V', 'I', 'S', 'I', 'O', 'N',
		      'E', 'D' },
};

static enum spd_dram_type map_smbios_mem_type_to_spd(struct smbios_table *table)
{
	char *part_number = table->string[table->data.mem_device.part_number];
	static const struct {
		enum spd_dram_type type;
		const char *prefix;
	} part_number_matches[] = {
		/* Hynix */
		{ SPD_DRAM_TYPE_DDR3, "h5t" },
		{ SPD_DRAM_TYPE_LPDDR3, "h9c" },
		{ SPD_DRAM_TYPE_LPDDR4, "h9h" },

		/* Samsung */
		{ SPD_DRAM_TYPE_DDR3, "k4b" },
		{ SPD_DRAM_TYPE_LPDDR3, "k3q" },
		{ SPD_DRAM_TYPE_LPDDR3, "k4e" },
		{ SPD_DRAM_TYPE_LPDDR4, "k3u" },
		{ SPD_DRAM_TYPE_LPDDR4, "k4f" },

		/* Micron */
		{ SPD_DRAM_TYPE_DDR4, "mt40" },
		{ SPD_DRAM_TYPE_DDR3, "mt41" },
		{ SPD_DRAM_TYPE_LPDDR3, "mt52" },
		{ SPD_DRAM_TYPE_LPDDR4, "mt53" },
	};

	switch (table->data.mem_device.type) {
	case SMBIOS_MEMORY_TYPE_DDR3:
		return SPD_DRAM_TYPE_DDR3;
	case SMBIOS_MEMORY_TYPE_DDR4:
		return SPD_DRAM_TYPE_DDR4;
	case SMBIOS_MEMORY_TYPE_LPDDR3:
		return SPD_DRAM_TYPE_LPDDR3;
	case SMBIOS_MEMORY_TYPE_LPDDR4:
		return SPD_DRAM_TYPE_LPDDR4;
	case SMBIOS_MEMORY_TYPE_DDR5:
		return SPD_DRAM_TYPE_DDR5;
	case SMBIOS_MEMORY_TYPE_LPDDR5:
		return SPD_DRAM_TYPE_LPDDR5;
	case SMBIOS_MEMORY_TYPE_OTHER:
	case SMBIOS_MEMORY_TYPE_UNKNOWN:
		/* Do our best to figure it out from part numbers */
		for (size_t i = 0; i < ARRAY_SIZE(part_number_matches); i++) {
			if (!strncasecmp(part_number,
					 part_number_matches[i].prefix,
					 strlen(part_number_matches[i].prefix)))
				return part_number_matches[i].type;
		}

		/* Fall thru */
	default:
		lprintf(LOG_ERR, "%s: Unknown SMBIOS memory type: %d\n",
			__func__, table->data.mem_device.type);
		return 0;
	}
}

static int extract_mem_info_from_smbios(struct smbios_table *table,
					struct nonspd_mem_info *info)
{
	const char *smbios_part_num;
	size_t smbios_part_num_len, max_part_num_len;
	uint32_t size;

	max_part_num_len = sizeof(info->part_num) - 1;
	smbios_part_num = table->string[table->data.mem_device.part_number];
	smbios_part_num_len = strlen(smbios_part_num);

	if (!smbios_part_num_len || smbios_part_num_len > max_part_num_len) {
		lprintf(LOG_ERR,
			"%s: SMBIOS Memory info table: part num is missing. "
			"Or len of part number %lu is larger then buffer %lu.",
			__func__, (unsigned long)smbios_part_num_len,
			(unsigned long)max_part_num_len);
		return -1;
	}

	size = (table->data.mem_device.size & 0x7fff) * 8;
	info->module_size_mbits =
		(table->data.mem_device.size & 0x8000 ? size * 1024 : size);

	strncpy((char *)info->part_num, smbios_part_num, max_part_num_len);

	info->dram_type = map_smbios_mem_type_to_spd(table);
	info->num_ranks = table->data.mem_device.attributes & 0xf;
	info->device_width = table->data.mem_device.data_width;

	return 0;
}

int spd_set_nonspd_info_from_smbios(struct platform_intf *intf, int dimm,
				    const struct nonspd_mem_info **info)
{
	struct smbios_table table;

	if (smbios_find_table(intf, SMBIOS_TYPE_MEMORY, dimm, &table) < 0) {
		lprintf(LOG_ERR, "%s: SMBIOS Memory info table missing\n",
			__func__);
		return -1;
	}

	/* memory device from SMBIOS is mapped into a nonspd_mem_info */
	if (extract_mem_info_from_smbios(&table, &part_extracted_from_smbios))
		return -1;

	*info = &part_extracted_from_smbios;

	return 0;
}
