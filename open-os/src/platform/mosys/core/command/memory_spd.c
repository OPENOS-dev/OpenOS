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
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "mosys/command_list.h"
#include "mosys/kv_pair.h"
#include "mosys/log.h"
#include "mosys/platform.h"

#include "lib/nonspd.h"

static int memory_nonspd_print_geometry(struct platform_intf *intf, int dimm)
{
	struct kv_pair *kv;
	const struct nonspd_mem_info *info;
	int rc;

	if (!intf->cb->memory->nonspd_mem_info) {
		errno = ENOSYS;
		return -1;
	}

	if (intf->cb->memory->nonspd_mem_info(intf, dimm, &info) < 0)
		return -1;

	kv = kv_pair_new();

	kv_pair_fmt(kv, "dimm", "%u", dimm);
	nonspd_print_field(kv, info, SPD_GET_SIZE);
	nonspd_print_field(kv, info, SPD_GET_RANKS);
	nonspd_print_field(kv, info, SPD_GET_WIDTH);

	rc = kv_pair_print(kv);
	kv_pair_free(kv);

	return rc;
}

static int memory_spd_print_geometry_cmd(struct platform_intf *intf,
					 struct platform_cmd *cmd, int argc,
					 char **argv)
{
	int dimm;
	int last_dimm;
	int rv = 0;

	if (!intf->cb->memory || !intf->cb->memory->dimm_count) {
		errno = ENOSYS;
		return -1;
	}

	last_dimm = intf->cb->memory->dimm_count(intf);

	if (argc) {
		dimm = strtol(argv[0], NULL, 0);
		if ((dimm < 0) || (dimm > (last_dimm - 1))) {
			lprintf(LOG_ERR, "Invalid DIMM: %d\n", dimm);
			errno = EINVAL;
			return -1;
		}
		return memory_nonspd_print_geometry(intf, dimm);
	}

	for (dimm = 0; dimm < last_dimm; dimm++) {
		rv |= memory_nonspd_print_geometry(intf, dimm);
	}

	return rv;
}

static int memory_nonspd_print_id(struct platform_intf *intf, int dimm)
{
	struct kv_pair *kv;
	const struct nonspd_mem_info *info;
	int rc;

	if (!intf->cb->memory->nonspd_mem_info) {
		errno = ENOSYS;
		return -1;
	}

	if (intf->cb->memory->nonspd_mem_info(intf, dimm, &info) < 0)
		return -1;

	kv = kv_pair_new();

	kv_pair_fmt(kv, "dimm", "%u", dimm);
	nonspd_print_field(kv, info, SPD_GET_MFG_ID);
	nonspd_print_field(kv, info, SPD_GET_PART_NUMBER);

	rc = kv_pair_print(kv);
	kv_pair_free(kv);

	return rc;
}

static int memory_spd_print_id_cmd(struct platform_intf *intf,
				   struct platform_cmd *cmd, int argc,
				   char **argv)
{
	int dimm;
	int last_dimm;
	int rv = 0;

	if (!intf->cb->memory || !intf->cb->memory->dimm_count) {
		errno = ENOSYS;
		return -1;
	}

	last_dimm = intf->cb->memory->dimm_count(intf);

	if (argc) {
		dimm = strtol(argv[0], NULL, 0);
		if ((dimm < 0) || (dimm > (last_dimm - 1))) {
			lprintf(LOG_ERR, "Invalid DIMM: %d\n", dimm);
			errno = EINVAL;
			return -1;
		}
		return memory_nonspd_print_id(intf, dimm);
	}

	for (dimm = 0; dimm < last_dimm; dimm++) {
		rv |= memory_nonspd_print_id(intf, dimm);
	}

	return rv;
}

static int memory_nonspd_print_type(struct platform_intf *intf, int dimm)
{
	struct kv_pair *kv;
	const struct nonspd_mem_info *info;
	int rc;

	if (!intf->cb->memory->nonspd_mem_info) {
		errno = ENOSYS;
		return -1;
	}

	if (intf->cb->memory->nonspd_mem_info(intf, dimm, &info) < 0)
		return -1;

	kv = kv_pair_new();
	kv_pair_fmt(kv, "dimm", "%u", dimm);
	nonspd_print_field(kv, info, SPD_GET_DRAM_TYPE);
	nonspd_print_field(kv, info, SPD_GET_MODULE_TYPE);

	rc = kv_pair_print(kv);
	kv_pair_free(kv);

	return rc;
}

static int memory_spd_print_type_cmd(struct platform_intf *intf,
				     struct platform_cmd *cmd, int argc,
				     char **argv)
{
	int dimm;
	int last_dimm;
	int rv = 0;

	if (!intf->cb->memory || !intf->cb->memory->dimm_count) {
		errno = ENOSYS;
		return -1;
	}

	last_dimm = intf->cb->memory->dimm_count(intf);

	if (argc) {
		dimm = strtol(argv[0], NULL, 0);
		if ((dimm < 0) || (dimm > (last_dimm - 1))) {
			lprintf(LOG_ERR, "Invalid DIMM: %d\n", dimm);
			errno = EINVAL;
			return -1;
		}
		return memory_nonspd_print_type(intf, dimm);
	}

	for (dimm = 0; dimm < last_dimm; dimm++) {
		rv |= memory_nonspd_print_type(intf, dimm);
	}

	return rv;
}

static int memory_spd_print_all_cmd(struct platform_intf *intf,
				    struct platform_cmd *cmd, int argc,
				    char **argv)
{
	int rc = 0, dimm;

	if (!intf->cb->memory || !intf->cb->memory->dimm_count) {
		errno = ENOSYS;
		return -1;
	}

	if (argc) {
		dimm = strtol(argv[0], NULL, 0);
		if ((dimm < 0) ||
		    (dimm > (intf->cb->memory->dimm_count(intf) - 1))) {
			lprintf(LOG_ERR, "Invalid DIMM: %d\n", dimm);
			errno = EINVAL;
			return -1;
		}
	}

	rc |= memory_spd_print_type_cmd(intf, cmd, argc, argv);
	rc |= memory_spd_print_id_cmd(intf, cmd, argc, argv);
	rc |= memory_spd_print_geometry_cmd(intf, cmd, argc, argv);

	return rc;
}

static struct platform_cmd memory_spd_print_cmds[] = {
	{
		.name = "geometry",
		.desc = "Print module geometry and capacity",
		.usage = "<dimm number>",
		.type = ARG_TYPE_GETTER,
		.arg = {
			.func = memory_spd_print_geometry_cmd,
		},
	},
	{
		.name = "id",
		.desc = "Print module ID information",
		.usage = "<dimm number>",
		.type = ARG_TYPE_GETTER,
		.arg = {
			.func = memory_spd_print_id_cmd,
		},
	},
	{
		.name = "type",
		.desc = "Print module and dram type information",
		.usage = "<dimm number>",
		.type = ARG_TYPE_GETTER,
		.arg = {
			.func = memory_spd_print_type_cmd,
		},
	},
	{
		.name = "all",
		.desc = "Print all of the above",
		.usage = "<dimm number>",
		.type = ARG_TYPE_GETTER,
		.arg = {
			.func = memory_spd_print_all_cmd,
		},
	},
	{ NULL },
};

static struct platform_cmd memory_spd_cmds[] = {
	{
		.name = "print",
		.desc = "Print SPD Information",
		.usage = "<dimm number>",
		.type = ARG_TYPE_SUB,
		.arg = {
			.sub = memory_spd_print_cmds,
		},
	},
	{ NULL },
};

static struct platform_cmd memory_cmds[] = {
	{
		.name = "spd",
		.desc = "Information from SPD",
		.type = ARG_TYPE_SUB,
		.arg = {
			.sub = memory_spd_cmds,
		},
	},
	{ NULL },
};

struct platform_cmd cmd_memory = {
	.name = "memory",
	.desc = "Memory Information",
	.type = ARG_TYPE_SUB,
	.arg = {
		.sub = memory_cmds,
	},
};
