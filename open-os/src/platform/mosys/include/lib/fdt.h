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

#ifndef MOSYS_LIB_FDT_H__
#define MOSYS_LIB_FDT_H__

#include <inttypes.h>

#include <mosys/platform.h>

/*
 * fdt_get_ram_code - Obtain RAM code from FDT ram-code node
 *
 * returns 0 to indicate success, <0 to indicate failure.
 */
extern int fdt_get_ram_code(uint32_t *ram_code);

/*
 * get_ram_ids - Get RAM manufacturer and revision ids from device tree.
 *
 * @manufacturer_id : manufacturer ID
 * @revision_id1 :    revision ID1
 * @revision_id2 :    revision ID2
 *
 * returns 0 to indicate success, <0 to indicate failure.
 */
extern int fdt_get_ram_ids(uint8_t *manufacturer_id, uint8_t *revision_id1,
			   uint8_t *revision_id2);

struct fdt_memory_channel {
	uint8_t lpddr_type;
	uint8_t manufacturer_id;
	uint8_t revision[2];
	size_t io_width;
	size_t density_mbits;
	size_t num_ranks;
};

size_t fdt_channel_count(void);
int fdt_get_channel_info(int channel, struct fdt_memory_channel *mem_info);

#endif /* MOSYS_LIB_FDT_H__ */
