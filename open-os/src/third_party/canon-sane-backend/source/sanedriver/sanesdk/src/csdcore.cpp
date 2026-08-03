/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <Windows.h>
#include <tchar.h>
#else
#include <dlfcn.h>
#endif
#include "CeiUSB.h"
#include "csdcore_interface.h"
#include "log.h"

namespace {
	char *extract_ip(char *src, char *dst)
	{
		char *p = strstr(src, "network:");
		if (p) {
			p += strlen("network:");
			char *d = dst;
			while (*p) {
				if (*p==':') {
					break;
				}
				p++;
			}
			p++;
			while (*p) {
				if (*p==':') break;
				*d = *p;
				d++;
				p++;
			}
			*d=0;
		}
		return dst;
	}
	#if 0
	void make_cur_dir(char *cur_dir)
	{
		readlink("/proc/self/exe", cur_dir, 256);
		char *p = strrchr(cur_dir, '/');
		if (p) {
			*p=0;
		}
	}
	#endif
	long cdb_size(void * pcdb)
	{
		unsigned char *p = (unsigned char*)pcdb;
		char cdb_size_tbl[256] = {
		6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
		6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
		6,  6,  6,  6,  10, 10, 6,  6,  10, 6,  10, 6,  6,  6,  6,  6,
		6,  10, 6,  6,  6,  6,  6,  6,  6,  6,  6,  10, 6,  6,  6,  6,
		6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
		6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
		6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
		6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
		6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
		6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
		6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
		6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
		6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
		6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
		10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
		10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
		};
		return cdb_size_tbl[*p];
	}
	class CNetDll
	{
	public:
		CNetDll();
		~CNetDll();
		long load(char *plib_path);
		void unload();
		long CreateCeiUSB2(ICeiUSBLinux2 **ppObject);
	private:
		void *m_hm;
		LPFNCREATECEIUSB2 m_lpfn;
	};
	CNetDll::CNetDll() :m_hm(NULL), m_lpfn(NULL)
	{}
	CNetDll::~CNetDll()
	{
		unload();
	}
	long CNetDll::load(char *plib_path)
	{
#ifdef _WIN32
		HMODULE hm = LoadLibrary("CeiNWLinux.dll");
		if (hm == NULL) return -1;
		m_hm = (void*)hm;
		m_lpfn = (LPFNCREATECEIUSB2)GetProcAddress(hm, "CreateCeiUSB2");
		if (m_lpfn == NULL) {
			return -1;
		}
#else
		char path[256];
		sprintf(path, "%sCeiNWLinux.so", plib_path);
		m_hm= dlopen(path, RTLD_LAZY);
		if (m_hm == NULL) {
			SaneWriteLog("dlopen(%s) error %s", path, dlerror());
			return -1;
		}
		m_lpfn = (LPFNCREATECEIUSB2)dlsym(m_hm, "CreateCeiUSB2");
		if (m_lpfn == NULL) {
			return -1;
		}	
#endif
		return 0;
	}
	void CNetDll::unload()
	{
		if (m_hm) {
#ifdef _WIN32
			HMODULE hm = (HMODULE)m_hm;
			FreeLibrary(hm);
#else
			dlclose(m_hm);
#endif
			m_hm=NULL;
		}
	}
	long CNetDll::CreateCeiUSB2(ICeiUSBLinux2 **ppObject)
	{
		if (m_lpfn) return m_lpfn(ppObject);
		return -1;
	}
	class CUsbDll
	{
	public:
		CUsbDll();
		~CUsbDll();
		long load(char *plib_path);
		void unload();
		long CreateCeiUSB2(ICeiUSBLinux2 **ppObject);
	private:
		void *m_hm;
		LPFNCREATECEIUSB2 m_lpfn;
	};
	CUsbDll::CUsbDll() :m_hm(NULL), m_lpfn(NULL)
	{}
	CUsbDll::~CUsbDll()
	{
		unload();
	}
	long CUsbDll::load(char *plib_path)
	{
#ifdef _WIN32
		HMODULE hm = LoadLibrary("CeiUSBLinux.dll");
		if (hm == NULL) return -1;
		m_hm = (void*)hm;
		m_lpfn = (LPFNCREATECEIUSB2)GetProcAddress(hm, "CreateCeiUSB2");
		if (m_lpfn == NULL) {
			return -1;
		}
#else
		char path[256];
		sprintf(path, "%sCeiUSBLinux.so", plib_path);
		m_hm= dlopen(path, RTLD_LAZY);
		if (m_hm == NULL) {
			SaneWriteLog("dlopen(%s) error %s", path, dlerror());
			return -1;
		}
		m_lpfn = (LPFNCREATECEIUSB2)dlsym(m_hm, "CreateCeiUSB2");
		if (m_lpfn == NULL) {
			return -1;
		}	
#endif
		return 0;
	}
	void CUsbDll::unload()
	{
		if (m_hm) {
#ifdef _WIN32
			HMODULE hm = (HMODULE)m_hm;
			FreeLibrary(hm);
#else
			dlclose(m_hm);
#endif
			m_hm=NULL;
		}
	}
	long CUsbDll::CreateCeiUSB2(ICeiUSBLinux2 **ppObject)
	{
		if (m_lpfn) return m_lpfn(ppObject);
		return -1;
	}
	class CUsb : public ICeiSti
	{
	public:
		CUsb(char *p);
		virtual ~CUsb();
	public:
		INT32 STDMETHODCALLTYPE InitSTI(TCHAR *pConnectionName/*ex DR-M260 */);
		void  STDMETHODCALLTYPE UninitSTI();
		INT32 STDMETHODCALLTYPE Lock(DWORD timeout);
		void  STDMETHODCALLTYPE Unlock();
		INT32 STDMETHODCALLTYPE  SendCustom(void *pCmdBuf, int nSize, int Direction, void *data, int Length);
		INT32 STDMETHODCALLTYPE  ExecWrite(void *lpCDB, void *lpData, unsigned long dwDataLen);
		INT32 STDMETHODCALLTYPE  ExecRead(void *lpCDB, void *lpData, unsigned long dwDataLen);
		INT32 STDMETHODCALLTYPE  ExecNone(void *lpCDB, unsigned int wCdbLen);
		INT32 STDMETHODCALLTYPE  GetSenseData(BYTE *lpSenseData, unsigned long dwDataLen);
		DWORD STDMETHODCALLTYPE  GetInformationByte();
	private:
		void lib_path(char *p);
		long m_ref;
		CUsbDll m_dll;
		XInterface<ICeiUSBLinux2>m_usb;
		char *m_plib_path;
	};
	CUsb::CUsb(char *p):m_ref(1), m_plib_path(NULL)
	{lib_path(p);}
	CUsb::~CUsb()
	{
	}
	void CUsb::lib_path(char *p)
	{
		m_plib_path = p;
	}
	INT32 CUsb::InitSTI(TCHAR *pConnectionName/*DR-M260*/)
	{
		//SaneWriteLog(("CUsb::InitSTI(%s) start"), pConnectionName);
		long ret = m_dll.load(m_plib_path);
		if (ret) {
			SaneWriteLog("ERROR:m_dll.load(%s) error %ld", m_plib_path, ret);
			return -1;
		}
		ret = m_dll.CreateCeiUSB2(&m_usb);
		if (ret) return ret;
		INT32 out = m_usb->init(pConnectionName);
		//SaneWriteLog(("CUsb::InitSTI() end %d"), out);
		return out;
	}
	void  CUsb::UninitSTI()
	{
		m_usb.reset(NULL);
		m_dll.unload();
		delete this;
	}
	INT32 CUsb::Lock(DWORD timeout)
	{
		return m_usb->lock(timeout);
	}
	void  CUsb::Unlock()
	{
		m_usb->unlock();
	}
	INT32 CUsb::SendCustom(void *pCmdBuf, int nSize, int Direction, void *data, int Length)
	{
		return -1;
	}
	INT32 CUsb::ExecWrite(void *lpCDB, void *lpData, unsigned long dwDataLen)
	{
		return m_usb->exec_write((char*)lpCDB, cdb_size(lpCDB), (char*)lpData, dwDataLen);
	}
	INT32 CUsb::ExecRead(void *lpCDB, void *lpData, unsigned long dwDataLen)
	{
		return m_usb->exec_read((char*)lpCDB, cdb_size(lpCDB), (char*)lpData, dwDataLen);
	}
	INT32 CUsb::ExecNone(void *lpCDB, unsigned int wCdbLen)
	{
		return m_usb->exec_none((char*)lpCDB, wCdbLen);
	}
	INT32 CUsb::GetSenseData(BYTE *lpSenseData, unsigned long dwDataLen)
	{
		return -1;
	}
	DWORD CUsb::GetInformationByte()
	{
		return -1;
	}
	class CNet : public ICeiSti
	{
	public:
		CNet(char *p);
		virtual ~CNet();
	public:
		INT32 STDMETHODCALLTYPE InitSTI(TCHAR *pConnectionName/*ex DR-M260 */);
		void  STDMETHODCALLTYPE UninitSTI();
		INT32 STDMETHODCALLTYPE Lock(DWORD timeout);
		void  STDMETHODCALLTYPE Unlock();
		INT32 STDMETHODCALLTYPE  SendCustom(void *pCmdBuf, int nSize, int Direction, void *data, int Length);
		INT32 STDMETHODCALLTYPE  ExecWrite(void *lpCDB, void *lpData, unsigned long dwDataLen);
		INT32 STDMETHODCALLTYPE  ExecRead(void *lpCDB, void *lpData, unsigned long dwDataLen);
		INT32 STDMETHODCALLTYPE  ExecNone(void *lpCDB, unsigned int wCdbLen);
		INT32 STDMETHODCALLTYPE  GetSenseData(BYTE *lpSenseData, unsigned long dwDataLen);
		DWORD STDMETHODCALLTYPE  GetInformationByte();
	private:
		void lib_path(char *p);
		long m_ref;
		CNetDll m_dll;
		XInterface<ICeiUSBLinux2>m_usb;
		char *m_plib_path;
	};
	CNet::CNet(char *p):m_ref(1), m_plib_path(NULL)
	{lib_path(p);}
	CNet::~CNet()
	{
	}
	void CNet::lib_path(char *p)
	{
		m_plib_path = p;
	}
	INT32 CNet::InitSTI(TCHAR *pConnectionName/*network:4:X.X.X.X:hostname*/)
	{
		//SaneWriteLog(("CNet::InitSTI(%s) start"), pConnectionName);
		long ret = m_dll.load(m_plib_path);
		if (ret) {
			SaneWriteLog("ERROR:m_dll.load(%s) error %ld", m_plib_path, ret);
			return -1;
		}
		ret = m_dll.CreateCeiUSB2(&m_usb);
		if (ret) return ret;
		char ip[256]={0};
		extract_ip(pConnectionName, ip);
		INT32 out = m_usb->init(ip);
		if (out) SaneWriteLog("m_usb->init(%s) error %d", ip, out);
		//SaneWriteLog(("CNet::InitSTI() end %d"), out);
		return out;
	}
	void  CNet::UninitSTI()
	{
		m_usb.reset(NULL);
		m_dll.unload();
		delete this;
	}
	INT32 CNet::Lock(DWORD timeout)
	{
		return m_usb->lock(timeout);
	}
	void  CNet::Unlock()
	{
		m_usb->unlock();
	}
	INT32 CNet::SendCustom(void *pCmdBuf, int nSize, int Direction, void *data, int Length)
	{
		return -1;
	}
	INT32 CNet::ExecWrite(void *lpCDB, void *lpData, unsigned long dwDataLen)
	{
		return m_usb->exec_write((char*)lpCDB, cdb_size(lpCDB), (char*)lpData, dwDataLen);
	}
	INT32 CNet::ExecRead(void *lpCDB, void *lpData, unsigned long dwDataLen)
	{
		return m_usb->exec_read((char*)lpCDB, cdb_size(lpCDB), (char*)lpData, dwDataLen);
	}
	INT32 CNet::ExecNone(void *lpCDB, unsigned int wCdbLen)
	{
		return m_usb->exec_none((char*)lpCDB, wCdbLen);
	}
	INT32 CNet::GetSenseData(BYTE *lpSenseData, unsigned long dwDataLen)
	{
		return -1;
	}
	DWORD CNet::GetInformationByte()
	{
		return -1;
	}
	class CCsdCoreDll
	{
	public:
		CCsdCoreDll();
		~CCsdCoreDll();
	public:
		long load(char *plib_path);
		void unload();
	public:
		INT32 CsdInit(LPINIT_INFORMATION pInfo);
		INT32 CsdUninit();
		INT32 CsdProbeEx(LPPROBE_INFORMATION pInfo);
		INT32 CsdTerminate();
		INT32 CsdParGet(UINT uiParam, LPVOID lpParam);
		INT32 CsdParSet(UINT uiParam, LPARAM lParam);
		INT32 CsdStartScan(LPCSTR lpFileName = NULL, LPVOID lpReserved1 = NULL, LPVOID lpReserved2 = NULL);
		//INT32 CsdStartPrescan(LPCSTR lpFileName = NULL, LPVOID lpReserved1 = NULL, LPVOID lpReserved2 = NULL);
		INT32 CsdReadPage(LPCEIIMAGEINFO lpImage);
		INT32 CsdReleaseImage(LPCEIIMAGEINFO lpInfo);
		INT32 CsdFlashScannedImage();
		INT32 CsdStopScan();
		INT32 CsdAbortScan();
		INT32 CsdParGetChoiceCount(UINT uiParNo, UINT32 *lpCount);
		INT32 CsdParGetChoice(UINT uiParNo, INT32 iIndex, LPVOID lpVoid);
	private:
		void *GetProcAddress(const char *name);
		void *m_hd;
	private:
		TCHAR *path(char *buffer, char *plib_path);
		long proc();
	private:
		typedef INT32(WINAPI *LPFNCSDINIT)(LPINIT_INFORMATION pInfo);
		typedef INT32(WINAPI *LPFNCSDUNINIT)();
		typedef INT32(WINAPI *LPFNCSDPROBEEX)(LPPROBE_INFORMATION pInfo);
		typedef INT32(WINAPI *LPFNCSDTERMINATE)();
		typedef INT32(WINAPI *LPFNCSDPARGET)(UINT uiParam, LPVOID lpParam);
		typedef INT32(WINAPI *LPFNCSDPARSET)(UINT uiParam, LPARAM lParam);
		typedef INT32(WINAPI *LPFNCSDSTART)(LPCSTR lpFileName, LPVOID lpReserved1, LPVOID lpReserved2);
		typedef INT32(WINAPI *LPFNCSDIMAGE)(LPCEIIMAGEINFO lpImage);
		typedef INT32(WINAPI *LPFNCSD)();
		typedef INT32(WINAPI *LPFNCSDPARGETCHOICECOUNT)(UINT uiParNo, UINT32 *lpCount);
		typedef INT32(WINAPI *LPFNCSDPARGETCHOICE)(UINT uiParNo, INT32 iIndex, LPVOID lpVoid);
		LPFNCSDINIT m_lpfnCsdInit;
		LPFNCSDUNINIT m_lpfnCsdUninit;
		LPFNCSDPROBEEX m_lpfnCsdProbeEx;
		LPFNCSDTERMINATE m_lpfnCsdTerminate;
		LPFNCSDPARGET m_lpfnCsdParGet;
		LPFNCSDPARSET m_lpfnCsdParSet;
		LPFNCSDPARGETCHOICECOUNT m_lpfnCsdParGetChoiceCount;
		LPFNCSDPARGETCHOICE m_lpfnCsdParGetChoice;
		LPFNCSDSTART m_lpfnCsdStartScan;
		LPFNCSDSTART m_lpfnCsdStartPrescan;
		LPFNCSDIMAGE m_lpfnCsdReadPage;
		LPFNCSDIMAGE m_lpfnCsdReleaseImage;
		LPFNCSD m_lpfnCsdFlashScannedImage;
		LPFNCSD m_lpfnCsdStopScan;
		LPFNCSD m_lpfnCsdAbortScan;
	};
	CCsdCoreDll::CCsdCoreDll():m_hd(NULL)
	{
	}
	CCsdCoreDll::~CCsdCoreDll()
	{
		unload();
	}
	void *CCsdCoreDll::GetProcAddress(const char *name)
	{
		if (m_hd == NULL) return NULL;
#ifdef _WIN32
		HMODULE hm = (HMODULE)m_hd;
		return ::GetProcAddress(hm, name);
#else
		return dlsym(m_hd, name);
#endif
	}
	long CCsdCoreDll::load(char *plib_path)
	{
		//printf("CCsdCoreDll::load() start\r\n");
		if (m_hd) return 0;
		char buf[256] = { 0 };
#ifdef _WIN32
		m_hd = LoadLibrary(path(buf, plib_path));
#else
		m_hd = dlopen(path(buf, plib_path), RTLD_LAZY);
#endif
		if (m_hd == NULL) {
			//printf(_T("dlopen(%s) error %s\r\n"), buf[0] ? buf : path(buf), dlerror());
			return -1;
		}
		long out = proc();
		//printf("CCsdCoreDll::load() end %d\r\n", out);
		return out;
	}
	void  CCsdCoreDll::unload()
	{
#ifdef _WIN32
		HMODULE hm = (HMODULE)m_hd;
		if (m_hd) FreeLibrary(hm);
#else
		if (m_hd) dlclose(m_hd);
#endif
		m_hd = NULL;
	}
	TCHAR *CCsdCoreDll::path(char *buffer, char *plib_path)
	{
#ifdef _WIN32
		strcpy(buffer, "./CsdCore.dll");
#else
		sprintf(buffer, "%sCsdCore.so", plib_path);
#endif
		return buffer;
	}
	long CCsdCoreDll::proc()
	{
		m_lpfnCsdInit = (LPFNCSDINIT)GetProcAddress("CsdInit");
		m_lpfnCsdUninit = (LPFNCSDUNINIT)GetProcAddress("CsdUninit");
		m_lpfnCsdProbeEx = (LPFNCSDPROBEEX)GetProcAddress("CsdProbeEx");
		m_lpfnCsdTerminate = (LPFNCSDTERMINATE)GetProcAddress("CsdTerminate");
		m_lpfnCsdParGet = (LPFNCSDPARGET)GetProcAddress("CsdParGetA");
		m_lpfnCsdParSet = (LPFNCSDPARSET)GetProcAddress("CsdParSetA");
		m_lpfnCsdStartScan = (LPFNCSDSTART)GetProcAddress("CsdStartScanA");
		m_lpfnCsdStartPrescan = (LPFNCSDSTART)GetProcAddress("CsdStartPrescanA");
		m_lpfnCsdReadPage = (LPFNCSDIMAGE)GetProcAddress("CsdReadPage");
		m_lpfnCsdReleaseImage = (LPFNCSDIMAGE)GetProcAddress("CsdReleaseImage");
		m_lpfnCsdFlashScannedImage = (LPFNCSD)GetProcAddress("CsdFlashScannedImage");
		m_lpfnCsdStopScan = (LPFNCSD)GetProcAddress("CsdStopScan");
		m_lpfnCsdAbortScan = (LPFNCSD)GetProcAddress("CsdAbortScan");
		m_lpfnCsdParGetChoiceCount = (LPFNCSDPARGETCHOICECOUNT)GetProcAddress("CsdParGetChoiceCount");
		m_lpfnCsdParGetChoice = (LPFNCSDPARGETCHOICE)GetProcAddress("CsdParGetChoiceA");
		return 0;
	}
	INT32 CCsdCoreDll::CsdInit(LPINIT_INFORMATION pInfo)
	{
		if (m_lpfnCsdInit) return m_lpfnCsdInit(pInfo);
		return E_FAIL;
	}
	INT32 CCsdCoreDll::CsdUninit()
	{
		if (m_lpfnCsdUninit) return m_lpfnCsdUninit();
		return E_FAIL;
	}
	INT32 CCsdCoreDll::CsdProbeEx(LPPROBE_INFORMATION pInfo)
	{
		if (m_lpfnCsdProbeEx) return m_lpfnCsdProbeEx(pInfo);
		return E_FAIL;
	}
	INT32 CCsdCoreDll::CsdTerminate()
	{
		if (m_lpfnCsdTerminate) return m_lpfnCsdTerminate();
		return E_FAIL;
	}
	INT32 CCsdCoreDll::CsdParGet(UINT uiParam, LPVOID lpParam)
	{
		if (m_lpfnCsdParGet) return m_lpfnCsdParGet(uiParam, lpParam);
		return E_FAIL;
	}
	INT32 CCsdCoreDll::CsdParGetChoiceCount(UINT uiParNo, UINT32 *lpCount)
	{
		if (m_lpfnCsdParGetChoiceCount) return m_lpfnCsdParGetChoiceCount(uiParNo, lpCount);
		return E_FAIL;
	}
	INT32 CCsdCoreDll::CsdParGetChoice(UINT uiParNo, INT32 iIndex, LPVOID lpVoid)
	{
		if (m_lpfnCsdParGetChoice) return m_lpfnCsdParGetChoice(uiParNo, iIndex, lpVoid);
		return E_FAIL;
	}
	INT32 CCsdCoreDll::CsdParSet(UINT uiParam, LPARAM lpParam)
	{
		if (m_lpfnCsdParSet) return m_lpfnCsdParSet(uiParam, lpParam);
		return E_FAIL;
	}
	INT32 CCsdCoreDll::CsdStartScan(LPCSTR lpFileName, LPVOID lpReserved1, LPVOID lpReserved2)
	{
		if (m_lpfnCsdStartScan) return m_lpfnCsdStartScan(lpFileName, lpReserved1, lpReserved2);
		return E_FAIL;
	}
	//INT32 CCsdCoreDll::CsdStartPrescan(LPCSTR lpFileName, LPVOID lpReserved1, LPVOID lpReserved2)
	//{
	//	if (m_lpfnCsdStartPrescan) return m_lpfnCsdStartPrescan(lpFileName, lpReserved1, lpReserved2);
	//	return E_FAIL;
	//}
	INT32 CCsdCoreDll::CsdReadPage(LPCEIIMAGEINFO lpImage)
	{
		if (m_lpfnCsdReadPage) return m_lpfnCsdReadPage(lpImage);
		return E_FAIL;
	}
	INT32 CCsdCoreDll::CsdReleaseImage(LPCEIIMAGEINFO lpInfo)
	{
		if (m_lpfnCsdReleaseImage) return m_lpfnCsdReleaseImage(lpInfo);
		return E_FAIL;
	}
	INT32 CCsdCoreDll::CsdFlashScannedImage()
	{
		if (m_lpfnCsdFlashScannedImage) return m_lpfnCsdFlashScannedImage();
		return E_FAIL;
	}
	INT32 CCsdCoreDll::CsdStopScan()
	{
		if (m_lpfnCsdStopScan) return m_lpfnCsdStopScan();
		return E_FAIL;
	}
	INT32 CCsdCoreDll::CsdAbortScan()
	{
		if (m_lpfnCsdAbortScan) return m_lpfnCsdAbortScan();
		return E_FAIL;
	}
}
class CSaneCsdCore : public ISaneCsdCore
{
public:
	CSaneCsdCore();
	virtual ~CSaneCsdCore();
	long init(const char *dev, LPINIT_INFORMATION pInitInfo, LPPROBE_INFORMATION pProbeInfo);
	void uninit();
	long STDMETHODCALLTYPE QueryInterface(REFIID id, void **ppOut);
	unsigned long STDMETHODCALLTYPE AddRef();
	unsigned long STDMETHODCALLTYPE Release();
	INT32 CsdParGet(UINT uiParam, LPVOID lpParam);
	INT32 CsdParSet(UINT uiParam, LPARAM pParam);
	INT32 CsdParGetChoiceCount(UINT uiParNo, UINT32 *lpCount);
	INT32 CsdParGetChoice(UINT uiParNo, INT32 iIndex, LPVOID lpVoid);
	INT32 CsdStartScan();
	INT32 CsdReadPage(LPCEIIMAGEINFO2 lpImage);
	INT32 CsdReleaseImage(LPCEIIMAGEINFO2 lpInfo);
	INT32 CsdFlashScannedImage();
	INT32 CsdStopScan();
	INT32 CsdAbortScan();	
private:
	long m_ref;
	CCsdCoreDll m_dll;
	ICeiSti *m_connector;
	bool m_initdone;
	bool m_probedone;
};
CSaneCsdCore::CSaneCsdCore():m_ref(1), m_connector(NULL), m_initdone(false), m_probedone(false)
{
}
CSaneCsdCore::~CSaneCsdCore()
{
	uninit();
}
long CSaneCsdCore::QueryInterface(REFIID id, void **ppOut)
{
	return -1;
}
unsigned long CSaneCsdCore::AddRef()
{
	m_ref++;
	return m_ref;
}
unsigned long CSaneCsdCore::Release()
{
	m_ref--;
	if (m_ref<=0) {
		delete this;
		return  0;
	}
	return m_ref;
}
long CSaneCsdCore::init(const char *dev, LPINIT_INFORMATION pInitInfo, LPPROBE_INFORMATION pProbeInfo)
{
	//SaneWriteLog(("CSaneCsdCore::init() start"));
	long ret = 0;

	if (strstr(dev, "network")) {
       m_connector = new CNet((char*)pInitInfo->szLibraryFilePath);
	   if (m_connector==NULL) return CSD_NOMEM;
	   ret = m_connector->InitSTI((TCHAR *)dev);
	} else {
       m_connector = new CUsb((char*)pInitInfo->szLibraryFilePath);
	   if (m_connector==NULL) return CSD_NOMEM;
	   ret = m_connector->InitSTI((TCHAR *)pInitInfo->szProductName);
	}
	if (ret) {
		SaneWriteLog(_T("ERROR:m_connector->InitSTI() error %ld"), ret);
		return CSD_NOMEM;
	}
	ret = m_dll.load((char*)pInitInfo->szLibraryFilePath);
	if (ret) {
		SaneWriteLog(_T("ERROR:m_dll.load(%s) error %ld"), pInitInfo->szLibraryFilePath, ret);
		return CSD_NOMEM;
	}
	INT32 csd = m_dll.CsdInit(pInitInfo);
	if (csd!=CSD_OK) {
		SaneWriteLog(_T("ERROR:m_dll.CsdInit() error %ld"), csd);
		return CSD_NOMEM;
	}
	m_initdone=true;
	pProbeInfo->pSti=m_connector;
	ret = m_dll.CsdProbeEx(pProbeInfo);
	if (ret) {
		SaneWriteLog(_T("ERROR:m_dll.CsdProbeEx() error %ld"), ret);
		return ret;
	}
	m_probedone=true;
	//SaneWriteLog(("CSaneCsdCore::init() end"));
	return CSD_OK;
}
void CSaneCsdCore::uninit()
{
	if (m_probedone) m_dll.CsdTerminate();
	m_probedone=false;
	if (m_initdone) m_dll.CsdUninit();
	m_initdone=false;
	if (m_connector) {
		m_connector->UninitSTI();
		m_connector=NULL;
	}
}
INT32 CSaneCsdCore::CsdParGet(UINT uiParam, LPVOID lpParam)
{
	return m_dll.CsdParGet(uiParam, lpParam);
}
INT32 CSaneCsdCore::CsdParSet(UINT uiParam, LPARAM lParam)
{
	return m_dll.CsdParSet(uiParam, lParam);
}
INT32 CSaneCsdCore::CsdParGetChoiceCount(UINT uiParNo, UINT32 *lpCount)
{
	return m_dll.CsdParGetChoiceCount(uiParNo, lpCount);
}
INT32 CSaneCsdCore::CsdParGetChoice(UINT uiParNo, INT32 iIndex, LPVOID lpVoid)
{
	return m_dll.CsdParGetChoice(uiParNo, iIndex, lpVoid);
}
INT32 CSaneCsdCore::CsdStartScan()
{
	return m_dll.CsdStartScan(NULL, NULL, NULL);
}
INT32 CSaneCsdCore::CsdReadPage(LPCEIIMAGEINFO2 lpImage )
{
	return m_dll.CsdReadPage((LPCEIIMAGEINFO)lpImage);
}
INT32 CSaneCsdCore::CsdReleaseImage(LPCEIIMAGEINFO2 lpImage)
{
	return m_dll.CsdReleaseImage((LPCEIIMAGEINFO)lpImage);
}
INT32 CSaneCsdCore::CsdFlashScannedImage()
{
	return m_dll.CsdFlashScannedImage();
}
INT32 CSaneCsdCore::CsdStopScan()
{
	return m_dll.CsdStopScan();
}
INT32 CSaneCsdCore::CsdAbortScan()
{
	return m_dll.CsdAbortScan();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
ISaneCsdCore *create_csdcore_for_sane(const char *dev, LPINIT_INFORMATION pInitInfo, LPPROBE_INFORMATION pProbeInfo)
{
	//SaneWriteLog(("create_csdcore_for_sane() start"));
	CSaneCsdCore *p = new CSaneCsdCore;
	if (p==NULL) {
		printf("out of memory L:%d F:%s\r\n", __LINE__, __FILE__);
		return NULL;
	}
	long ret = p->init(dev, pInitInfo, pProbeInfo);
	if (ret) {
		delete p;
		printf("p->init() error %ld\r\n", ret);
		return NULL;
	}
	//SaneWriteLog(("create_csdcore_for_sane() end"));
	return (ISaneCsdCore *)p;
}
