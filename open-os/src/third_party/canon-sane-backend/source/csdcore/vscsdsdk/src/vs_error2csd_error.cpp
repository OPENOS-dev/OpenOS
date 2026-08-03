/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <cstddef>
#include "vs_error2csd_error.h"
#include "csderr.h"

long vserror2csderr(long vserr)
{
	struct {
		long vserr;
		long csderr;
	}tbl[] = {
		{VS3_NOPAGE, CSD3_NOPAGE},
		{VS3_JAM, CSD3_JAM},
		{VS3_COVEROPEN, CSD3_COVEROPEN},
		{VS3_DOUBLEFEED ,CSD3_DOUBLEFEED},
		{VS3_SKEW, CSD3_SKEWFEED},
		{VS3_SCANNER_LOCKED, CSD3_SCANNER_LOCKED},
        {VS3_CANCEL, CSD3_CANCEL},
        {VS3_DEVICE_NOT_FOUND, CSD3_NODEVICE},
        {VS3_HARDERROR, CSD3_HARDERROR},
	};
	for (long i = 0; i < (long)(sizeof(tbl) / sizeof(tbl[0])); i++) {
		if (tbl[i].vserr == vserr) return tbl[i].csderr;
	}
	return CSD3_UNKNOWN;
}
