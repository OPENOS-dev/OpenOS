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
 * fdt.c: Helper functions for getting data out of the device tree.
 */

#include <arpa/inet.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>

#include "lib/fdt.h"
#include "lib/file.h"
#include "lib/nonspd.h"

#include "mosys/alloc.h"
#include "mosys/log.h"
#include "mosys/mosys.h"
#include "mosys/platform.h"

#define FDT_ROOT "/proc/device-tree/"
/* FIXME: assume coreboot for now */
#define FDT_RAM_CODE_PATH FDT_ROOT "firmware/coreboot/ram-code"

#define LPDDR3_COMPAT_PATH FDT_ROOT "lpddr-channel0/rank@0/compatible"

static int fdt_get_uint32_val(const char *path, uint32_t *val)
{
	/* Leave room for a null-terminator in the buffer */
	char buf[sizeof(*val) + 1];

	if (read_file(path, buf, ARRAY_SIZE(buf), LOG_DEBUG) < 0)
		return -1;

	*val = ntohl(*(uint32_t *)buf);
	return 0;
}

int fdt_get_ram_code(uint32_t *ram_code)
{
	if (fdt_get_uint32_val(FDT_RAM_CODE_PATH, ram_code) < 0) {
		lprintf(LOG_ERR, "%s: Error when reading RAM code\n", __func__);
		return -1;
	}

	if (*ram_code == 0xffffffff) {
		lprintf(LOG_ERR, "%s: ram_code is invalid.\n", __func__);
		return -1;
	}

	lprintf(LOG_DEBUG, "%s: ram_code: %u\n", __func__, *ram_code);
	return 0;
}

static int fdt_extract_info_from_compatible(const char *path,
					    uint8_t *lpddr_type,
					    uint8_t *manufacturer_id,
					    uint8_t *revision_id1,
					    uint8_t *revision_id2)
{
	char buf[32];

	if (read_file(path, buf, sizeof(buf), LOG_ERR) <= 0) {
		lprintf(LOG_ERR, "%s: Error when reading Manufacturer ID\n",
			__func__);
		return -1;
	}

	/*
	 * Manufacturer ID is in ascii value gated between '-' and ','
	 * characters.  Revision ID is the hex number after the ','.
	 */
	if (sscanf(buf, "lpddr%1hhu-%2hhx,%2hhx%2hhx", lpddr_type,
		   manufacturer_id, revision_id1, revision_id2) != 4) {
		lprintf(LOG_ERR,
			"%s: Error when parsing LPDDR type/Manufacturer/Revision ID\n",
			__func__);
		return -1;
	}

	lprintf(LOG_DEBUG,
		"%s: lpddr_type: %u, manufacturer_id: %#02x, revision_id: %#02x %#02x\n",
		__func__, *lpddr_type, *manufacturer_id, *revision_id1,
		*revision_id2);
	return 0;
}

int fdt_get_ram_ids(uint8_t *manufacturer_id, uint8_t *revision_id1,
		    uint8_t *revision_id2)
{
	uint8_t lpddr_type;

	return fdt_extract_info_from_compatible(LPDDR3_COMPAT_PATH, &lpddr_type,
						manufacturer_id, revision_id1,
						revision_id2);
}

size_t fdt_channel_count(void)
{
	int status = 0;
	size_t channel_count = 0;
	char filepath[PATH_MAX];

	while (status == 0) {
		snprintf(filepath, sizeof(filepath),
			 "/proc/device-tree/lpddr-channel%zu/name",
			 channel_count);
		status = access(filepath, F_OK);

		if (status == 0)
			channel_count++;
	}

	return channel_count;
}

int fdt_get_channel_info(int channel, struct fdt_memory_channel *chan_info)
{
	int status = 0, rank_count = 0;
	char filepath[PATH_MAX], channel_path[PATH_MAX];
	uint8_t lpddr_type, mfg_id, rev_id1, rev_id2;
	uint32_t rank_size, channel_io_width, rank_io_width, density;

	if (!chan_info)
		return -1;

	snprintf(channel_path, sizeof(channel_path),
		 "/proc/device-tree/lpddr-channel%d", channel);

	/* Get channel-io-width */
	snprintf(filepath, sizeof(filepath), "%s/io-width", channel_path);
	if (fdt_get_uint32_val(filepath, &channel_io_width)) {
		lprintf(LOG_ERR, "%s: Error when reading channel-io-width\n",
			__func__);
		return -1;
	}

	/* Get rank-io-width */
	snprintf(filepath, sizeof(filepath), "%s/rank@%d/io-width",
		 channel_path, rank_count);
	if (fdt_get_uint32_val(filepath, &rank_io_width)) {
		lprintf(LOG_ERR, "%s: Error when reading rank-io-width\n",
			__func__);
		return -1;
	}

	if (rank_io_width == 0 || channel_io_width % rank_io_width) {
		lprintf(LOG_ERR,
			"%s: channel_io_width is not a multiple of "
			"rank_io_width\n",
			__func__);
		return -1;
	}

	/* Calculate rank_count and density */
	density = 0;
	while (status == 0) {
		snprintf(filepath, sizeof(filepath), "%s/rank@%d/density",
			 channel_path, rank_count);
		status = fdt_get_uint32_val(filepath, &rank_size);

		if (status == 0) {
			density +=
				rank_size * (channel_io_width / rank_io_width);
			rank_count++;
		}
	}

	if (rank_count == 0) {
		lprintf(LOG_ERR, "%s: Error when reading rank size\n",
			__func__);
		return -1;
	}

	/* Get LPDDR type, manufacturer ID, revision IDs */
	snprintf(filepath, sizeof(filepath), "%s/rank@0/compatible",
		 channel_path);
	if (fdt_extract_info_from_compatible(filepath, &lpddr_type, &mfg_id,
					     &rev_id1, &rev_id2)) {
		lprintf(LOG_ERR,
			"%s: Error when reading info from compatible\n",
			__func__);
		return -1;
	}

	chan_info->lpddr_type = lpddr_type;
	chan_info->manufacturer_id = mfg_id;
	chan_info->revision[0] = rev_id1;
	chan_info->revision[1] = rev_id2;
	chan_info->io_width = rank_io_width;
	chan_info->density_mbits = density;
	chan_info->num_ranks = rank_count;

	return 0;
}
