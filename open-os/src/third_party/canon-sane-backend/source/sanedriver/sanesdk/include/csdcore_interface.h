/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef _SANE_CSDCORE_DEFINE_
#define _SANE_CSDCORE_DEFINE_
#ifdef _WIN32
#include <Windows.h>
#include <unknown.h>
#else
#include "windows_types.h"
#include "unknown.h"
#endif
#ifndef CSD_API 
#define CSD_API
#endif
#include "csdscan.h"

class ISaneCsdCore : public IUnknown
{
public:
	virtual INT32 CsdParGet(UINT uiParam, LPVOID lpParam)=0;
	virtual INT32 CsdParSet(UINT uiParam, LPARAM pParam)=0;
	virtual INT32 CsdParGetChoiceCount(UINT uiParNo, UINT32 *lpCount)=0;
	virtual INT32 CsdParGetChoice(UINT uiParNo, INT32 iIndex, LPVOID lpVoid)=0;
	virtual INT32 CsdStartScan()=0;
	virtual INT32 CsdReadPage(LPCEIIMAGEINFO2 lpImage)=0;
	virtual INT32 CsdReleaseImage(LPCEIIMAGEINFO2 lpInfo)=0;
	virtual INT32 CsdFlashScannedImage()=0;
	virtual INT32 CsdStopScan()=0;
	virtual INT32 CsdAbortScan()=0;	
};

ISaneCsdCore *create_csdcore_for_sane(const char *dev, LPINIT_INFORMATION pInitInfo, LPPROBE_INFORMATION pProbeInfo);

#endif
