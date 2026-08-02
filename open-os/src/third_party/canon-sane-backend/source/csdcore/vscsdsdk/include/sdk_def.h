/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef __VSCSD_SDK_DEFINE_HEADER_DEFINEED_STRUCT_HEADER__
#define __VSCSD_SDK_DEFINE_HEADER_DEFINEED_STRUCT_HEADER__

#include "scanner_connector_interface.h"

typedef struct tagVSCSD_SDK_INIT_INFORMATION
{
	long dwSize;/* [in]size of this structure */
	IScannerConnector *pscanner;
	char lib_path[256];
	char scanner_name[16];//ex dr-m260
	bool simulation;
}VSCSD_SDK_INIT_INFORMATION, *LPVSCSD_SDK_INIT_INFORMATION;


#endif