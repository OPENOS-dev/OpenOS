/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <cctype>
#include <cstddef>
#include <stdio.h>
#include <string.h>
#include <memory>
#include <mutex>
#include "csdsdk.h"
#ifdef _WIN32
#include <Windows.h>
#else
#include "windows_types.h"
#endif
#ifndef CSD_API
#define CSD_API
#endif
#include "csdscan.h"
#include "global_apis.h"
#include "ceilogwrite.h"

namespace {
	FILE *g_fp = NULL;
	int STDMETHODCALLTYPE ceisave_value(const char *key, char *pin, void *)
	{
		if (g_fp == NULL) return -1;
		fprintf(g_fp, "%s=%s\n", key, pin);
		return 0;
	}
	char *g_path = NULL;
	int STDMETHODCALLTYPE ceirestore_value(const char *key, char *pout, long size/*of pout*/, char *def, void *)
	{
		if (g_path == NULL) return -1;
		ceisdk_get_private_profile_string("Csd", key, pout, size, def, (const char*)g_path);
		return 0;
	}
	int convert_index(INT32 iIndex)
	{
		int i = (int)iIndex;
		if (i >= 0) return i;
        int low = CSDCHOICE_LOW;
        int step = CSDCHOICE_STEP;
        int high = CSDCHOICE_HIGH;
		if (i==low) {
            return ICsdCore::RANGE_LOW;
        } else if (i==step) {
            return ICsdCore::RANGE_STEP;
        } else if (i==high) {
            return ICsdCore::RANGE_HIGH;
        }
		printf("WARNING:convert_index() return 0\r\n");
		printf("WARNING:CSDCHOICE_HIGH:%x\r\n", CSDCHOICE_HIGH);
		printf("WARNING:iIndex:%lx\r\n", iIndex);
		return 0;
	}
	char *tolower_str(char *s)
	{
		char *out = s;
		while (*s) {
			*s = ::tolower(*s);
			s++;
		}
		return out;
	}
	#if 0
	char *rm_ch(char *s, char c)
	{
		char *dst = s;
		char *src = s;
		while (*src) {
			if (*src == c) {
				src++;
			}
			else {
				*dst = *src;
				dst++;
				src++;
			}
		}
		*dst = 0;
		return s;
	}
	#endif	
	#define CSD_ERROR_CONVERT_DEFINE(x) {CSD3_##x, CSD_##x},
	long csdsdk_error2csderr(long csdsdk_error)
	{
		static struct {
			long sdkerr;
			long csderr;
		} tbl[] = {
			CSD_ERROR_CONVERT_DEFINE(OK)
			CSD_ERROR_CONVERT_DEFINE(NOPAGE)
			CSD_ERROR_CONVERT_DEFINE(NODEVICE)
			CSD_ERROR_CONVERT_DEFINE(BADPARMNO)
			CSD_ERROR_CONVERT_DEFINE(BADFILE)
			CSD_ERROR_CONVERT_DEFINE(BADPARM )
			CSD_ERROR_CONVERT_DEFINE(NOPAPER)
			CSD_ERROR_CONVERT_DEFINE(JAM)
			CSD_ERROR_CONVERT_DEFINE(COVEROPEN)
			CSD_ERROR_CONVERT_DEFINE(POWERON)
			CSD_ERROR_CONVERT_DEFINE(BADFILE0)
			CSD_ERROR_CONVERT_DEFINE(BADFILE1)
			CSD_ERROR_CONVERT_DEFINE(COUNTONLY)
			CSD_ERROR_CONVERT_DEFINE(COUNTMISS)
			CSD_ERROR_CONVERT_DEFINE(ABORTED)
			CSD_ERROR_CONVERT_DEFINE(RESFAIL)
			CSD_ERROR_CONVERT_DEFINE(NOTREADY)
			CSD_ERROR_CONVERT_DEFINE(HARDERROR)
			CSD_ERROR_CONVERT_DEFINE(NOTSELECTED)
			CSD_ERROR_CONVERT_DEFINE(NEWFILE)
			CSD_ERROR_CONVERT_DEFINE(DOUBLEFEED)
			CSD_ERROR_CONVERT_DEFINE(SKEWFEED)
			CSD_ERROR_CONVERT_DEFINE(FILMEND)
			CSD_ERROR_CONVERT_DEFINE(NOCAMERA)
			CSD_ERROR_CONVERT_DEFINE(BADLOGFILE	)
			CSD_ERROR_CONVERT_DEFINE(FILMERROR)
			CSD_ERROR_CONVERT_DEFINE(NOMEM)
			CSD_ERROR_CONVERT_DEFINE(UNKNOWN)
			CSD_ERROR_CONVERT_DEFINE(ENDOFPAGE)
			CSD_ERROR_CONVERT_DEFINE(CANCEL)
			CSD_ERROR_CONVERT_DEFINE(NOCARTRIDGE)
			CSD_ERROR_CONVERT_DEFINE(COUNTMISSTOOMANY)
			CSD_ERROR_CONVERT_DEFINE(COUNTMISSTOOFEW)
			CSD_ERROR_CONVERT_DEFINE(STAPLEDETECTED)
			CSD_ERROR_CONVERT_DEFINE(DELIVERYFULL)
			CSD_ERROR_CONVERT_DEFINE(DETECTED_BATCHSEP)
			CSD_ERROR_CONVERT_DEFINE(COMM)
			CSD_ERROR_CONVERT_DEFINE(SCANNER_NOMEM)
			CSD_ERROR_CONVERT_DEFINE(FEEDERRORDETECTED)
			CSD_ERROR_CONVERT_DEFINE(CONNECT_ERROR_WIFI)
			CSD_ERROR_CONVERT_DEFINE(CONNECT_ERROR_USB)
			CSD_ERROR_CONVERT_DEFINE(SOFTWARE)
			CSD_ERROR_CONVERT_DEFINE(NOTFINDMODULE)
			CSD_ERROR_CONVERT_DEFINE(DRIVERBUSY	)
			CSD_ERROR_CONVERT_DEFINE(SEQUENCEERR)
			{0,0}
		};
		for (long i=0; i<(long)(sizeof(tbl)/sizeof(tbl[0])); i++) {
			if (tbl[i].sdkerr==csdsdk_error) {
				return tbl[i].csderr;
			}
		}
		return CSD_NOMEM;	
	}
	ICsdCore*g_driver=NULL;
	class CScannerConnectorWrapper : public IScannerConnector
	{
	public:
		CScannerConnectorWrapper(ICeiSti*psti);
		~CScannerConnectorWrapper();
		long STDMETHODCALLTYPE QueryInterface(REFIID id, void **ppOut);
		unsigned long STDMETHODCALLTYPE AddRef();
		unsigned long STDMETHODCALLTYPE Release();
		long lock(long timeout);
		void unlock();
		long exec_write(char *cdb, long cdb_size, char *data, long data_size);
		long exec_read(char *cdb, long cdb_size, char *data, long data_size);
		long exec_none(char *cdb, long cdb_size);		
	private:
		ICeiSti*m_psti;
	};
	CScannerConnectorWrapper::CScannerConnectorWrapper(ICeiSti*psti):m_psti(psti){}
	CScannerConnectorWrapper::~CScannerConnectorWrapper(){}
	long CScannerConnectorWrapper::QueryInterface(REFIID id, void **ppOut){return -1;}
	unsigned long CScannerConnectorWrapper::AddRef(){return 1;}
	unsigned long CScannerConnectorWrapper::Release(){return 1;}
	long CScannerConnectorWrapper::lock(long timeout)
	{
		return m_psti->Lock(timeout);
	}
	void CScannerConnectorWrapper::unlock()
	{
		m_psti->Unlock();
	}
	long CScannerConnectorWrapper::exec_write(char *cdb, long , char *data, long data_size)
	{
		return m_psti->ExecWrite((void*)cdb, (void*)data, data_size);
	}
	long CScannerConnectorWrapper::exec_read(char *cdb, long , char *data, long data_size)
	{
		return m_psti->ExecRead((void*)cdb, (void*)data, data_size);
	}
	long CScannerConnectorWrapper::exec_none(char *cdb, long cdb_size)
	{
		return m_psti->ExecNone((void*)cdb, cdb_size);
	}
	std::unique_ptr<CScannerConnectorWrapper>g_scanner_wrapper;
}

IUnknown *CreateCsdCoreDependentClass(LPVSCSD_SDK_INIT_INFORMATION pinfo);

CSD_API
INT32 WINAPI CsdInit(LPINIT_INFORMATION pInfo)
{
	if (pInfo == NULL) return CSD_OK;
	if (g_driver) return CSD_OK;

	VSCSD_SDK_INIT_INFORMATION info = { sizeof(info) };
	if (pInfo->szLibraryFilePath && pInfo->szLibraryFilePath[0]) strcpy(info.lib_path, pInfo->szLibraryFilePath);
	if (pInfo->szProductName&&pInfo->szProductName[0]) {
		strcpy(info.scanner_name, pInfo->szProductName);
		tolower_str(info.scanner_name);
	}
	long ret = CsdSDKCreateCsdCore(&g_driver, &info, CreateCsdCoreDependentClass(&info));
	if (ret) return csdsdk_error2csderr(ret);
	WriteLog("CsdInit()");
	return CSD_OK;
}
CSD_API
INT32 WINAPI CsdUninit()
{
	WriteLog("CsdUninit()");
	if (g_driver) {
		g_driver->Release();
		g_driver=NULL;
	}
	g_scanner_wrapper.reset(NULL);
	return CSD_OK;
}
CSD_API
INT32 WINAPI CsdProbeEx(LPPROBE_INFORMATION pInfo)
{
	VSCSD_SDK_INIT_INFORMATION info={sizeof(info)};

	if (pInfo==NULL) return CSD_BADPARM;

	g_scanner_wrapper.reset(new CScannerConnectorWrapper(pInfo->pSti));
	if (g_scanner_wrapper.get()==NULL) return CSD_NOMEM;

	info.pscanner = g_scanner_wrapper.get();
	info.simulation = pInfo->SimulationMode ? true : false;

	INT32 ret = csdsdk_error2csderr(g_driver->probe(&info));
	WriteLog("CsdProbeEx()");
	return ret;
}
CSD_API
INT32 WINAPI CsdTerminate()
{
	WriteLog("CsdTerminate()");
	if (g_driver) {
		g_driver->terminate();
		g_driver->Release();
		g_driver=NULL;
	}
	g_scanner_wrapper.reset(NULL);	
	return CSD_OK;
}
CSD_API
INT32 WINAPI CsdParGetA(UINT uiParam, LPVOID lpParam)
{
	if (g_driver==NULL) return CSD_NOMEM;
	return csdsdk_error2csderr(g_driver->tagget(uiParam, lpParam));
}
CSD_API
INT32 WINAPI CsdParSetA(UINT uiParam, LPARAM lParam)
{
	if (g_driver==NULL) return CSD_NOMEM;
	if (uiParam == 0 && lParam == 0) {
		return g_driver->tagset_default(0);
	}
	return csdsdk_error2csderr(g_driver->tagset(uiParam, lParam));
}
CSD_API
INT32 WINAPI CsdParGetChoiceFlags(UINT uiParNo, UINT32 *lpFlags)
{
	if (g_driver == NULL) return CSD_NOMEM;
	long f = 0;
	long ret = g_driver->tagget_choice_flag(uiParNo, &f);
	switch (f) {
	default:
	case ICsdCore::CHOICE_ANY:*lpFlags = CSDCHOICEFLAG_ANY; break;
	case ICsdCore::CHOICE_RANGE:*lpFlags = CSDCHOICEFLAG_RANGE; break;
	case ICsdCore::CHOICE_LIST:*lpFlags = CSDCHOICEFLAG_LIST; break;
	}
	return csdsdk_error2csderr(ret);
}
CSD_API
INT32 WINAPI CsdParGetChoiceCount(UINT uiParNo, UINT32 *lpCount)
{
	if (g_driver==NULL) return CSD_NOMEM;
	long cnt=0;
	long ret = g_driver->tagget_choice_count(uiParNo, &cnt);
	*lpCount=cnt;
	return csdsdk_error2csderr(ret);
}
CSD_API
INT32 WINAPI CsdParGetChoiceA(UINT uiParNo, INT32 iIndex, LPVOID lpVoid)
{
	if (g_driver==NULL) return CSD_NOMEM;
	return csdsdk_error2csderr(g_driver->tagget_choice(uiParNo, convert_index(iIndex), lpVoid));
}
CSD_API
INT32 WINAPI CsdStartScanA(LPCSTR , LPVOID, LPVOID)
{
	if (g_driver==NULL) return CSD_NOMEM;
	return g_driver->scan_start();
}
CSD_API
INT32 WINAPI CsdReadPage(LPCEIIMAGEINFO lpImage)
{
	if (g_driver==NULL) return CSD_NOMEM;
	if (lpImage==NULL) return CSD_NOMEM;
	ICeiImage *pimg=NULL;
	INT32 out = g_driver->image(&pimg);
	if (!out) {
		lpImage->lpImage=(BYTE*)pimg->img();
		lpImage->lXpos=0;
		lpImage->lYpos=0;
		lpImage->lWidth=pimg->width();
		lpImage->lHeight=pimg->height();
		lpImage->lSync=pimg->sync();
		lpImage->tImageSize=pimg->size();
		lpImage->lBps=pimg->bps();
		lpImage->lSpp=pimg->spp();
		lpImage->dwRGBOrder=0;
		lpImage->lXResolution=pimg->xdpi();
		lpImage->lYResolution=pimg->ydpi();
		if (lpImage->cbSize>=sizeof(CEIIMAGEINFO2)) {
			CEIIMAGEINFO2 *lpImage2 = (CEIIMAGEINFO2*)lpImage;
			lpImage2->nFileType=0;	
			lpImage2->nCompType=pimg->comptype()?COMPTYPE_JPEG:COMPTYPE_NONE;			/* Compression Type @@COMPTYPE_XXX */
			lpImage2->nJpegQuality=pimg->comptype()?pimg->compinfo():0;
			lpImage2->dwReserved=0;
		}
	}
	return csdsdk_error2csderr(out);
}
CSD_API
INT32 WINAPI CsdReleaseImage(LPCEIIMAGEINFO lpImage)
{
	if (g_driver==NULL) return CSD_NOMEM;
	g_driver->clear_image((char*)lpImage->lpImage);
	return CSD_OK;
}
CSD_API
INT32 WINAPI CsdFlashScannedImage()
{
	if (g_driver==NULL) return CSD_NOMEM;
	g_driver->scan_end();
	return CSD_OK;
}
CSD_API
INT32 WINAPI CsdStopScan()
{
	if (g_driver==NULL) return CSD_NOMEM;
	return csdsdk_error2csderr(g_driver->stop());
}
CSD_API
INT32 WINAPI CsdAbortScan()
{
	if (g_driver==NULL) return CSD_NOMEM;
	return csdsdk_error2csderr(g_driver->abort());
}
CSD_API
INT32 WINAPI CsdParSaveFileA(LPSTR lpszFileName)
{
	if (g_driver == NULL) return CSD_NOMEM;
	if (lpszFileName) {
		FILE* fp = fopen(lpszFileName, "w");
		if (fp) {
			fprintf(fp, "[Csd]\n");
			g_fp = fp;
			g_driver->save_value(ceisave_value);
			g_fp = NULL;
			fclose(fp);
		}
	}
	else {
		g_driver->save_value(0);
	}
	return CSD_OK;
}
CSD_API
INT32 WINAPI CsdParRestoreFileA(LPSTR lpszFileName)
{
	if (g_driver == NULL) return CSD_NOMEM;
	if (lpszFileName) {
		if (lpszFileName == (LPSTR)1) {
			//flush
			g_driver->flush_value(0);
		}
		else {
			g_path = lpszFileName;
			csdsdk_error2csderr(g_driver->restore_value(ceirestore_value));
			g_path = NULL;
		}
	}
	else {
		g_driver->restore_value(0);
	}
	return CSD_OK;
}
