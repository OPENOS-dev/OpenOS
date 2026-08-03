/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef _CEI_VS_OUT_SDK_LINUX_H_INCLUDED_
#define _CEI_VS_OUT_SDK_LINUX_H_INCLUDED_

#include "vserr.h"
#include "sdk_def.h"
#include "virtual_scanner_interface.h"
extern "C" {
long STDMETHODCALLTYPE CreateVirtualScanner(IVirtualScanner **ppOut, LPVSCSD_SDK_INIT_INFORMATION pinfo);
}
#endif
