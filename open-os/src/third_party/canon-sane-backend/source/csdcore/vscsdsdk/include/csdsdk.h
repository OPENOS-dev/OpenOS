/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef __CSDSDK_MAIN_HEADER_DEFINED__
#define __CSDSDK_MAIN_HEADER_DEFINED__

#include "sdk_def.h"
#include "csderr.h"
#include "csdcore_interface.h"

long CsdSDKCreateCsdCore(ICsdCore **ppOut, LPVSCSD_SDK_INIT_INFORMATION pinfo, IUnknown *handle);
#endif