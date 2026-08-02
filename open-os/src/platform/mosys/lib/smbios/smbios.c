/* Copyright 2012 Google LLC
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

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/file.h"
#include "lib/smbios.h"
#include "mosys/log.h"
#include "mosys/platform.h"

/*
 * smbios_parse_string_table  -  parse strings into table structure
 *
 * @ptr:	pointer to start of strings
 * @strings:	array of SMBIOS_MAX_STRINGS SMBIOS_MAX_STRING_LENGTH-char
 *              strings to fill in.
 *
 * returns number of strings present in table, < 0 on error.
 */
static int smbios_parse_string_table(
	char *ptr, char strings[SMBIOS_MAX_STRINGS][SMBIOS_MAX_STRING_LENGTH])
{
	int id, len, i;

	if (ptr == NULL || strings == NULL)
		return -1;

	/* The first string is purposefully nulled out because these fields
	 * directly indexed by SMBIOS string indexes (1-based). If a 0 is
	 * accidentaly used as an index make sure there is an empty string
	 * waiting. */
	strings[0][0] = '\0';

	for (id = 1; *ptr; ptr += len + 1) {
		len = strlen(ptr);
		/* Fill in string if there is room left. */
		if (id < SMBIOS_MAX_STRINGS) {
			/* filter non-ascii characters */
			for (i = 0; i < len && i < SMBIOS_MAX_STRING_LENGTH - 1;
			     i++) {
				if (isprint(ptr[i]))
					strings[id][i] = ptr[i];
				else
					strings[id][i] = '.';
			}
			strings[id][i] = '\0';
		}
		++id;
	}

	return id - 1;
}

/*
 * smbios_find_table  -  locate specified SMBIOS table in memory
 *
 * @intf:	platform interface
 * @type:	smbios table type to locate
 * @instance:	what instance to retrieve (0-based)
 * @table:	OUTPUT buffer to store table data
 *
 * returns 0 to indicate success
 * returns <0 to indicate failure
 */
int smbios_find_table(struct platform_intf *intf, enum smbios_types type,
		      int instance, struct smbios_table *table)
{
	char filepath[PATH_MAX];

	if (!table || type > SMBIOS_TYPE_END)
		return -1;

	snprintf(filepath, sizeof(filepath),
		 "/sys/firmware/dmi/entries/%d-%d/raw", type, instance);

	if (read_file(filepath, (char *)table, sizeof(*table), LOG_DEBUG) < 0)
		return -1;

	smbios_parse_string_table((char *)table + table->header.length,
				  table->string);

	return 0;
}
