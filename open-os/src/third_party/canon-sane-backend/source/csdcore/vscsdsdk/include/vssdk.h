/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef __VS_SDK_DEFINED_HEADER_DEFINED__
#define __VS_SDK_DEFINED_HEADER_DEFINED__

#include "vserr.h"
#include "sdk_def.h"
#include "virtual_scanner_interface.h"

long  VSSDKCreateVirtualScanner(IVirtualScanner **ppOut, LPVSCSD_SDK_INIT_INFORMATION pinfo, IUnknown *handle);
#endif