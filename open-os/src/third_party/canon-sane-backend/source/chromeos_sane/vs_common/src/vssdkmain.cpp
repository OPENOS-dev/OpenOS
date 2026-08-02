/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <cstddef>
#include <stdio.h>
#include "vssdk.h"
#include "CeiVS3.h"

IUnknown *CreateScannerDependentClass(LPVSCSD_SDK_INIT_INFORMATION pInfo);

long STDMETHODCALLTYPE CreateVirtualScanner(IVirtualScanner **ppOut, LPVSCSD_SDK_INIT_INFORMATION pInfo)
{
	return VSSDKCreateVirtualScanner(ppOut, pInfo, CreateScannerDependentClass(pInfo));
}