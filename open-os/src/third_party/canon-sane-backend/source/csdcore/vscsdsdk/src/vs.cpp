/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <memory>
#include <map>
#include <mutex>
#include <map>
#include <memory.h>
#include "ceilogwrite.h"
#include "command.h"
#include "scanner_connector_interface.h"
#include "scanctrl_interface.h"
#include "commandhook_interface.h"
#include "vssdk.h"
#include "global_apis.h"
namespace {
	char *trim(char *s)
	{
		long l = (long)strlen(s);
		if (l<3) return s;
		return &s[l-3];
	}
	void WriteLog_exec(char *cdb, long cdb_size, char *data=NULL, long data_size=0)
	{
		if (!IsLogMode()) return;
		char c[16];
		char s[1024]={0};
		if (data) {
			strcpy(s, "cdb:");
			for (long i=0; i<cdb_size; i++) {
				sprintf(c, "%02x ", cdb[i]);
				strcat(s, trim(c));
			}
			strcat(s, " data:");
			long max = data_size;
			if (max>64) max = 64;
			for (long j=0; j<max; j++) {
				sprintf(c, "%02x ", data[j]);
				strcat(s, trim(c));
			}
			SDKWriteLog((char*)"%s", s);
		} else {
			for (long i=0; i<cdb_size; i++) {
				sprintf(c, "%02x ", cdb[i]);
				strcat(s, trim(c));
			}
			SDKWriteLog((char*)"cdb:%s", s);			
		}
	}
}
class CScannerConnector : public IScannerConnector
{
public:
	CScannerConnector();
	virtual ~CScannerConnector();
	void set(IScannerConnector *p);
	long lock(long timeout);
	void unlock();
	long  exec_write(char *cdb, long cdb_size, char *data, long data_size);
	long  exec_read(char *cdb, long cdb_size, char *data, long data_size);
	long  exec_none(char *cdb, long cdb_size);
	long STDMETHODCALLTYPE QueryInterface(REFIID id, void **ppOut);
	unsigned long STDMETHODCALLTYPE AddRef();
	unsigned long STDMETHODCALLTYPE Release();
public:
	void block();
	void unblock();
private:
	IScannerConnector *m_psti;
	bool m_block;
    std::mutex m_mutex;
};
CScannerConnector::CScannerConnector():m_psti(NULL), m_block(false)
{
}
CScannerConnector::~CScannerConnector()
{
}
long CScannerConnector::QueryInterface(REFIID id, void **ppOut)
{
	return -1;
}
unsigned long CScannerConnector::AddRef()
{
	return 1;
}
unsigned long CScannerConnector::Release()
{
	return 1;
}
void CScannerConnector::block()
{
	m_block=true;
}
void CScannerConnector::unblock()
{
	m_block=false;
}
void CScannerConnector::set(IScannerConnector *p)
{
	m_psti = p;
}
long CScannerConnector::lock(long timeout)
{
    std::lock_guard<std::mutex> lg(m_mutex);
	if (m_psti) return m_psti->lock(timeout);
	return -1;
}
void CScannerConnector::unlock()
{
    std::lock_guard<std::mutex> lg(m_mutex);
	if (m_psti) m_psti->unlock();
}
long CScannerConnector::exec_write(char *cdb, long cdb_size, char *data, long data_size)
{
    std::lock_guard<std::mutex> lg(m_mutex);
	if (m_block) {
		SDKWriteLog((char*)"SCSI comman is blocked.");
		return VS3_OK;
	}
	if (m_psti) {
		WriteLog_exec(cdb, cdb_size , data, data_size);
		long ret = m_psti->exec_write(cdb, cdb_size, data, data_size);
		return ret;
	}
	return -1;
}
long CScannerConnector::exec_read(char *cdb, long cdb_size, char *data, long data_size)
{
    std::lock_guard<std::mutex> lg(m_mutex);
	if (m_psti) {
		long ret = m_psti->exec_read(cdb, cdb_size, data, data_size);
		WriteLog_exec(cdb, cdb_size ,data, data_size);
		return ret;
	}
	return -1;
}
long CScannerConnector::exec_none(char *cdb, long cdb_size)
{
    std::lock_guard<std::mutex> lg(m_mutex);
	if (m_psti) {
		WriteLog_exec(cdb, cdb_size);
		long ret = m_psti->exec_none(cdb, cdb_size);
		return ret;
	}
	return -1;
}
class CVS : public IVirtualScanner2
{
public:
	CVS();
	virtual ~CVS();
	long init(LPVSCSD_SDK_INIT_INFORMATION pinfo, IUnknown *handle);
	void uninit();
	long STDMETHODCALLTYPE QueryInterface(REFIID id, void **ppOut);
	unsigned long STDMETHODCALLTYPE AddRef();
	unsigned long STDMETHODCALLTYPE Release();
	long scan_start(IScannedImageCtrl *psci);
	long image(ICeiImage **ppOut, ICeiImageInformation **ppiOut);
	long scanning();/*returned value TRUE or FALSE*/
	long abort();
	long stop();
	long scan_end();
	long exec_write(char *cdb, long cdb_size, char *data, long data_size);
	long exec_read(char *cdb, long cdb_size, char *data, long data_size);
	long exec_none(char *cdb, long cdb_size);
    long set(long type, void *v);
    long get(long type, void *p);
private:
	long m_ref;
	VSCSD_SDK_INIT_INFORMATION m_info;
	XInterface<IScanCtrl>m_scan;
	XInterface<ICommandHook>m_hook;
	XInterface<IUnknown> m_handle;
	CScannerConnector m_scanner_connector;
    std::mutex m_scan_mutex;
private:
	long  exec_write_scan_start(char *cdb, long cdb_size, char *data, long data_size);
	long  exec_read_image(char *cdb, long cdb_size, char *data, long data_size);
	long  exec_none_scan_end(char *cdb, long cdb_size);	
};
CVS::CVS():m_ref(1)
{
	SDKWriteLog((char*)"CVS::CVS()");
	memset(&m_info, 0, sizeof(m_info));
}
CVS::~CVS()
{
	uninit();
	SDKWriteLog((char*)"CVS::~CVS()");
	WriteLog_uninit();
}
long CVS::init(LPVSCSD_SDK_INIT_INFORMATION pinfo, IUnknown *handle)
{
	if (pinfo==NULL) return -1;
	long sz = pinfo->dwSize;
	if (sz>(long)(sizeof(m_info))) sz = sizeof(m_info);
	memcpy(&m_info, pinfo, sz);
	m_info.dwSize = sizeof(m_info);
	m_scanner_connector.set(pinfo->pscanner);
	m_handle=handle;
	m_hook.reset(commandhook(&m_scanner_connector, handle));
	if (m_hook.get()==NULL) return VS3_NOMEM;
	return VS3_OK;
}
void CVS::uninit()
{
	scan_end();
	m_scan.reset(NULL);
	m_handle=NULL;
	m_hook.reset(NULL);
}
long CVS::QueryInterface(REFIID id, void **ppOut)
{

	if (memcmp(&IID_IVirtualScanner2, &id, sizeof(REFIID)) == 0) {
		*ppOut = dynamic_cast<IUnknown*>(dynamic_cast<IVirtualScanner2*>(this));
		AddRef();
		return 0;
	}
	return -1;
}
unsigned long CVS::AddRef()
{
	m_ref++;
	return m_ref;
}
unsigned long CVS::Release()
{
	m_ref--;
	if (m_ref<=0) {
		delete this;
		return 0;
	}
	return m_ref;
}
long CVS::set(long type, void *v)
{
	if (m_handle.get()) {
		XInterface<IInternalVirtualScanner> vs;
		if (!m_handle->QueryInterface(IID_IInternalVirtualScanner, (void**)&vs)) {
			return vs->set_vsvalue(type, v);
		}
	}
    return 0;
}
long CVS::get(long type, void *p)
{
	if (m_handle.get()) {
		XInterface<IInternalVirtualScanner> vs;
		if (!m_handle->QueryInterface(IID_IInternalVirtualScanner, (void **)&vs)) {
			return vs->get_vsvalue(type, p);
		}
	}
	return 0;
}
long CVS::scan_start(IScannedImageCtrl *psci)
{
	SDKWriteLog((char*)"CVS::scan_start() start");
    std::lock_guard<std::mutex> lg(m_scan_mutex);
	m_scan.reset(scan_control(&m_scanner_connector, psci, m_handle, NULL));
	if (m_scan.get()==NULL) {
		SDKWriteLog((char*)"m_scan is NULL: L:%d F:%s", __LINE__, __FILE__);
		return ENOMEM;
	}
	long out = m_scan->scan_start();
	SDKWriteLog((char*)"CVS::scan_start() end %ld\r\n", out);
	return out;
}
long  CVS::image(ICeiImage **ppOut, ICeiImageInformation **ppiOut)
{
	SDKWriteLog((char*)"CVS::image() start");
    std::lock_guard<std::mutex> lg(m_scan_mutex);
	long ret = VS3_OK;
	if (m_scan.get()) {
		ret = m_scan->get_image(ppOut);
		if (!ret) {
			if (ppiOut) {
				m_scan->get_information(0, (void*)ppiOut);
			}
		}
	}
	SDKWriteLog((char*)"CVS::image() end %ld\r\n", ret);
	return ret;
}
long  CVS::scanning()
{
    std::lock_guard<std::mutex> lg(m_scan_mutex);
	long out = 0;
	if (m_scan.get()) {
		SDKWriteLog("CVS::scanning() start");
		out = m_scan->scanning();
		SDKWriteLog("CVS::scanning() end %ld", out);
	}
	
	return out;
}
long  CVS::scan_end()
{
	SDKWriteLog((char*)"CVS::scan_end() start");
    std::lock_guard<std::mutex> lg(m_scan_mutex);
	if (m_scan.get()) m_scan->scan_end();
	m_scan.reset(NULL);	
	SDKWriteLog((char*)"CVS::scan_end() end\r\n");
	return VS3_OK;
}
long  CVS::abort()
{
	SDKWriteLog("CVS::abort() start");
    std::lock_guard<std::mutex> lg(m_scan_mutex);
	long out = 0;
	if (m_scan.get()) out = m_scan->abort();
	SDKWriteLog("CVS::abort() end %ld", out);
	return out;
}
long  CVS::stop()
{
	SDKWriteLog("CVS::stop() start");
    std::lock_guard<std::mutex> lg(m_scan_mutex);
	long out = 0;
	if (m_scan.get()) out = m_scan->stop();
	SDKWriteLog("CVS::stop() end %ld", out);
	return out;
}
long  CVS::exec_write(char *cdb, long cdb_size, char *data, long data_size)
{
	long ret = VS3_OK;
	if (cdb[0]==opScan&&data[0]==0) { 
		ret = exec_write_scan_start(cdb, cdb_size, data, data_size);
	} else {
		if (m_hook.get()) {
			ret = m_hook->exec_write(cdb, cdb_size, data, data_size);
		} else {
			ret = VS3_NOMEM;
		}		
	}
	return ret;
}
long  CVS::exec_read(char *cdb, long cdb_size, char *data, long data_size)
{
	long ret = VS3_OK;
	if (scanning()) {
		ret = exec_read_image(cdb, cdb_size, data, data_size);
	} else {
		ret = m_hook->exec_read(cdb, cdb_size, data, data_size);
	}
	return ret;
}
long  CVS::exec_none(char *cdb, long cdb_size)
{
	long ret = VS3_OK;
	if (cdb[0]==opObjectPosition&&cdb[1]==0) {
		ret = exec_none_scan_end(cdb, cdb_size);
	} else {
		ret = m_hook->exec_none(cdb, cdb_size);
	}
	return ret;
}
long  CVS::exec_read_image(char *cdb, long cdb_size, char *data, long data_size)
{
	return VS3_NOMEM;//use CVS::image().
}
long  CVS::exec_write_scan_start(char *cdb, long cdb_size, char *data, long data_size)
{
	long ret = VS3_OK;
	m_scanner_connector.block();
	ret = m_hook->exec_write(cdb, cdb_size, data, data_size);
	m_scanner_connector.unblock();
	return ret;
}
long  CVS::exec_none_scan_end(char *cdb, long cdb_size)
{
	long ret = VS3_OK;
	if (!scanning()) {
		ret = m_hook->exec_none(cdb, cdb_size);
	}
	scan_end();
	return ret;
}

long  VSSDKCreateVirtualScanner(IVirtualScanner **ppOut, LPVSCSD_SDK_INIT_INFORMATION pInfo, IUnknown *handle)
{
	if (ppOut==NULL) {
		printf("ppOut is NULL\r\n");
		return VS3_INVALID_ARG;
	}
	if (pInfo==NULL) {
		printf("pInfo is NULL\r\n");
		return VS3_INVALID_ARG;
	}
	ceisdk_set_module_name((char*)"vs");
	if (pInfo->lib_path[0]) {
		ceisdk_set_library_path(pInfo->lib_path);
	}
	if (pInfo->scanner_name[0]) {
		ceisdk_set_scanner_name(pInfo->scanner_name);
	}
	WriteLog_init();
	if (handle==NULL) {
		printf("handle is NULL\r\n");
		SDKWriteLog("handle is NULL");
		return VS3_INVALID_ARG;
	}
	//WriteLog(((char*)"[VSSDK]VSSDKCreateVirtualScanner() start"));	
	std::unique_ptr<CVS>p(new CVS);
	if (p.get()==NULL) {
		SDKWriteLog(((char*)"no memroy"));
		return VS3_NOMEM;
	}
	long ret = p->init(pInfo, handle);
	if (ret!=VS3_OK) {
		SDKWriteLog(((char*)"p->init() error %d"), ret);
		return ret;
	}
	*ppOut = (IVirtualScanner*)p.release();
	//WriteLog(((char*)"[VSSDK]VSSDKCreateVirtualScanner() end"));
	return VS3_OK;
}
