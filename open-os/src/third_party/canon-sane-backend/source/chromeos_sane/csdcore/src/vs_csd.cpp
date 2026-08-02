/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <cstddef>
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif
#include "virtual_scanner_interface.h"
#include "settings_csd.h"
#include "CeiVS3.h"

namespace {
	char *remove_ch(char *s, char c)
    {
        char *src = s;
        char *dst = s;
        while (*src) {
            if (*src==c) {
                src++;
            } else {
                *dst = *src;
                src++;
                dst++;
            }
        }
        *dst=0;
        return s;
    }
	char *tolower_str(char *s)
    {
        for (char *p = s; *p; p++) *p = tolower(*p);
        return (s);
    }
	char *make_path(char *path, char *name)
	{
		char *p = path + strlen(path);
		p--;
		if (*p=='/') strcat(path, name);
		else {
			strcat(path, "/");
			strcat(path, name);
		}
		return path;
	}

	class CVSWDll
	{
	public:
		CVSWDll();
		~CVSWDll();
		long CreateVirtualScanner(IVirtualScanner **ppOut, LPVSCSD_SDK_INIT_INFORMATION pinfo);
		long load(char *path);
		void unload();	
	private:	
		typedef long (STDMETHODCALLTYPE *LPFNCREATEVIRTUALSCANNER)(IVirtualScanner **ppOut, LPVSCSD_SDK_INIT_INFORMATION pinfo);
		LPFNCREATEVIRTUALSCANNER m_lpfn;
		void *m_hd;
	};
	CVSWDll::CVSWDll():m_lpfn(NULL), m_hd(NULL) 
	{
	}
	CVSWDll::~CVSWDll()
	{
		unload();
	}
	long CVSWDll::CreateVirtualScanner(IVirtualScanner **ppOut, LPVSCSD_SDK_INIT_INFORMATION pinfo)
	{
		if (m_lpfn) {
			return m_lpfn(ppOut, pinfo);
		}
		return -1;
	}
	long CVSWDll::load(char *path)
	{
		if (m_hd) return 0;
#ifdef _WIN32
		m_hd = (void*)LoadLibrary(path);
#else
		m_hd = dlopen(path, RTLD_LAZY);
#endif
		if (m_hd) {
#ifdef _WIN32
			HMODULE hm = (HMODULE)m_hd;
			m_lpfn = (LPFNCREATEVIRTUALSCANNER)GetProcAddress(hm, "CreateVirtualScanner");
#else
			m_lpfn = (LPFNCREATEVIRTUALSCANNER)dlsym(m_hd, "CreateVirtualScanner");
#endif
			if (m_lpfn==NULL) return -1;
		}
		else {
			return -1;
		}
		return 0;
	}
	void CVSWDll::unload()
	{
		if (m_hd) {
#ifdef _WIN32
			HMODULE hm = (HMODULE)m_hd;
			FreeLibrary(hm);
#else
			dlclose(m_hd);
#endif
			m_hd=NULL;
		}
	}
}

class CCsdVS : public IVirtualScanner2
{
public:
	CCsdVS(IScannerConnector *pscanner, IUnknown *handle);
	virtual ~CCsdVS();
	long STDMETHODCALLTYPE QueryInterface(REFIID id, void **ppOut);
	unsigned long STDMETHODCALLTYPE AddRef();
	unsigned long STDMETHODCALLTYPE Release();
	long init();
	void uninit();
	long scan_start(IScannedImageCtrl *psci);
	long image(ICeiImage **ppOut, ICeiImageInformation **ppiOut);//must be called after scan scsi command
	long scanning();/*returned value TRUE or FALSE*/
	long abort();
	long stop();
	long scan_end();
	long exec_write(char *cdb, long cdb_size, char *data, long data_size);
	long exec_read(char *cdb, long cdb_size, char *data, long data_size);
	long exec_none(char *cdb, long cdb_size);
	long set(long type, void* v);
	long get(long type, void* p);
private:
	long m_ref;
	XInterface<IVirtualScanner>m_vs;	
	CVSWDll      m_dll;
	CSettingsCsd *m_psettings;
	IScannerConnector *m_pscanner;
};
CCsdVS::CCsdVS(IScannerConnector *pscanner, IUnknown *handle):
m_ref(1),
m_psettings((CSettingsCsd*)handle),
m_pscanner(pscanner)
{
}
CCsdVS::~CCsdVS()
{
	uninit();
}
long CCsdVS::QueryInterface(REFIID id, void **ppOut)
{
	if (memcmp(&IID_IVirtualScanner2, &id, sizeof(REFIID)) == 0) {
		*ppOut = dynamic_cast<IUnknown*>(dynamic_cast<IVirtualScanner2*>(this));
		AddRef();
		return 0;
	}
	return -1;
}
unsigned long CCsdVS::AddRef()
{
	m_ref++;
	return m_ref;
}
unsigned long CCsdVS::Release()
{
	m_ref--;
	if (m_ref<=0) {
		delete this;
		return 0;
	}
	return m_ref;
}
long CCsdVS::set(long type, void* v)
{
	XInterface<IVirtualScanner2>vs;
	if (m_vs.get() && !m_vs->QueryInterface(IID_IVirtualScanner2, (void**)&vs)) {
		return vs->set(type, v);
	}
	return 0;
}
long CCsdVS::get(long type, void* p)
{
	XInterface<IVirtualScanner2>vs;
	if (m_vs.get() && !m_vs->QueryInterface(IID_IVirtualScanner2, (void**)&vs)) {
		return vs->get(type, p);
	}
	return 0;
}
long CCsdVS::init()
{
	VSCSD_SDK_INIT_INFORMATION &info = m_psettings->info();
	info.pscanner = m_pscanner;
	long ret = 0;
	char path[256];
	char name[32];
	strcpy(name, info.scanner_name);
	remove_ch(name, '-');
	tolower_str(name);
#ifdef _WIN32
    make_path(path, name);
	strcat(path, "vs.dll");
#elif __APPLE__
   strcat(path, "vs.framework/vs");
#else
	snprintf(path, sizeof(path),"%s%s%s", info.lib_path, name, "vs.so");
#endif
	ret = m_dll.load(path);
	if (ret) return ret;
	ret = m_dll.CreateVirtualScanner(&m_vs, &info);
	if (ret) return ret;
	return 0;
}
void CCsdVS::uninit()
{
	m_vs.reset(NULL);
	m_dll.unload();
}
long CCsdVS::scan_start(IScannedImageCtrl *psci)
{
	if (m_vs.get()==NULL) return VS3_NOMEM;
	return m_vs->scan_start(psci);	
}
long CCsdVS::image(ICeiImage **ppOut, ICeiImageInformation **ppiOut)
{
	if (m_vs.get()==NULL) return VS3_NOMEM;
	return m_vs->image(ppOut, ppiOut);
}
long CCsdVS::scanning()
{
	if (m_vs.get()==NULL) return 0;
	return m_vs->scanning();
}
long CCsdVS::abort()
{
	if (m_vs.get()==NULL) return VS3_NOMEM;
	return m_vs->abort();
}
long CCsdVS::stop()
{
	if (m_vs.get()==NULL) return VS3_NOMEM;
	return m_vs->stop();
}
long CCsdVS::scan_end()
{
	if (m_vs.get()==NULL) return VS3_NOMEM;
	return m_vs->scan_end();	
}
long CCsdVS::exec_write(char *cdb, long cdb_size, char *data, long data_size)
{
	if (m_vs.get()==NULL) return VS3_NOMEM;
	return m_vs->exec_write(cdb, cdb_size, data, data_size);
}
long CCsdVS::exec_read(char *cdb, long cdb_size, char *data, long data_size)
{
	if (m_vs.get()==NULL) return VS3_NOMEM;
	return m_vs->exec_read(cdb, cdb_size, data, data_size);
}
long CCsdVS::exec_none(char *cdb, long cdb_size)
{
	if (m_vs.get()==NULL) return VS3_NOMEM;
	return m_vs->exec_none(cdb, cdb_size);	
}
IVirtualScanner *create_virtual_scanner(IScannerConnector *pscanner, IUnknown *handle)
{
	CCsdVS * p = new CCsdVS(pscanner, handle);
	if (p->init()) {
		delete p;
		return NULL;
	}
	return (IVirtualScanner*)p;
}
