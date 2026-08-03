/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <stdio.h>
#include <cstddef>
#include <errno.h>
#include <memory>
#include <vector>
#include "ceilogwrite.h"
#include <memory.h>
#include "csdcore_interface.h"
#include "tags_interface.h"
#include "scanctrl_interface.h"
#include "virtual_scanner_interface.h"
#include "csdsdk.h"
#include "csdtags.h"
#include "global_apis.h"

namespace {
#ifdef _DEBUG
	long tagget(ICsdTags* ptags, long tagid, long def = 0)
	{
		long out = def;
		ptags->get((int)tagid, &out);
		return out;
	}
#endif
	int STDMETHODCALLTYPE save_values_callback(const char *key, char *pin, void *)
	{
		SDKWriteLog("%s=%s", key, pin);
#ifdef _DEBUG
		printf("%s=%s\r\n", key, pin);
#endif
		return 0;
	}
	void printf_tags(ICsdTags *ptags)
	{
		if (!IsLogMode()) {
#ifdef _DEBUG
			printf("\r\n[SCAN]\r\n");
			ptags->save_value(save_values_callback, NULL);
			printf("CSDP_READAHEAD=%ld\r\n", tagget(ptags, CSDP_READAHEAD));
			printf("\r\n");
#endif
			return;
		}
		SDKWriteLog((char*)"[SCAN]");
		ptags->save_value(save_values_callback, NULL);
		//SDKWriteLog("\r\n");
	}
	#define CASE_CSD_ERROR(x) case x:return #x;
	const char *csderr3Tostr(long err)
	{
		switch (err) {
		CASE_CSD_ERROR(CSD3_OK)
		CASE_CSD_ERROR(CSD3_NOPAGE)
		CASE_CSD_ERROR(CSD3_NODEVICE)
		CASE_CSD_ERROR(CSD3_BADPARMNO)
		CASE_CSD_ERROR(CSD3_BADFILE)
		CASE_CSD_ERROR(CSD3_BADPARM)
		CASE_CSD_ERROR(CSD3_NOPAPER)
		CASE_CSD_ERROR(CSD3_JAM)
		CASE_CSD_ERROR(CSD3_COVEROPEN)
		CASE_CSD_ERROR(CSD3_POWERON)
		CASE_CSD_ERROR(CSD3_BADFILE0)
		CASE_CSD_ERROR(CSD3_BADFILE1)
		CASE_CSD_ERROR(CSD3_COUNTONLY)
		CASE_CSD_ERROR(CSD3_COUNTMISS)
		CASE_CSD_ERROR(CSD3_ABORTED)
		CASE_CSD_ERROR(CSD3_RESFAIL)
		CASE_CSD_ERROR(CSD3_NOTREADY)
		CASE_CSD_ERROR(CSD3_HARDERROR)
		CASE_CSD_ERROR(CSD3_NOTSELECTED)
		CASE_CSD_ERROR(CSD3_NEWFILE)
		CASE_CSD_ERROR(CSD3_DOUBLEFEED)
		CASE_CSD_ERROR(CSD3_SKEWFEED)
		CASE_CSD_ERROR(CSD3_FILMEND)
		CASE_CSD_ERROR(CSD3_NOCAMERA)
		CASE_CSD_ERROR(CSD3_BADLOGFILE)
		CASE_CSD_ERROR(CSD3_FILMERROR)
		CASE_CSD_ERROR(CSD3_NOMEM)
		CASE_CSD_ERROR(CSD3_UNKNOWN)
		CASE_CSD_ERROR(CSD3_ENDOFPAGE)
		CASE_CSD_ERROR(CSD3_CANCEL)
		CASE_CSD_ERROR(CSD3_NOCARTRIDGE)
		CASE_CSD_ERROR(CSD3_COUNTMISSTOOMANY)
		CASE_CSD_ERROR(CSD3_COUNTMISSTOOFEW)
		CASE_CSD_ERROR(CSD3_STAPLEDETECTED)
		CASE_CSD_ERROR(CSD3_DELIVERYFULL)
		CASE_CSD_ERROR(CSD3_DETECTED_BATCHSEP)
		CASE_CSD_ERROR(CSD3_COMM)
		CASE_CSD_ERROR(CSD3_SCANNER_NOMEM)
		CASE_CSD_ERROR(CSD3_FEEDERRORDETECTED)
		CASE_CSD_ERROR(CSD3_CONNECT_ERROR_WIFI)
		CASE_CSD_ERROR(CSD3_CONNECT_ERROR_USB)
		CASE_CSD_ERROR(CSD3_SOFTWARE)
		CASE_CSD_ERROR(CSD3_NOTFINDMODULE)
		CASE_CSD_ERROR(CSD3_DRIVERBUSY)
		CASE_CSD_ERROR(CSD3_SEQUENCEERR)
		CASE_CSD_ERROR(CSD3_BADPATH)
		CASE_CSD_ERROR(CSD3_BADACCESS)
		CASE_CSD_ERROR(CSD3_DISKFULL)		
		}
		printf("csderr3Tostr(%ld) return CSD3_UNKNOWN(truly)\r\n", err);
		return "CSD3_UNKNOWN(truly)";
	}
}
class CCsdTagsInternal : public ICsdTags
{
public:
	CCsdTagsInternal();
	virtual ~CCsdTagsInternal();
	long STDMETHODCALLTYPE QueryInterface(REFIID id, void **ppOut);
	unsigned long STDMETHODCALLTYPE AddRef();
	unsigned long STDMETHODCALLTYPE Release();
public:
	int get(int tagid, void *lpParam);
	int get_default(int tagid, void *lpParam);
	int set(int tagid, long long lParam);
	int set_default(int tagid);
	int change_default(int tagid, long long lParam);
	int choice_flag(int tagid, long *lpFlag);
	int choice_count(int tagid, long *lpCount);
	int choice(int tagid, int index, void *lpParam);
	int change_default(LPFNGETVALUE lpfn, void *callback_param);
	int save_value(LPFNSETVALUE lpfn, void* callback_param);
	int restore_value(LPFNGETVALUE lpfn, void* callback_param);
	void save_value(int tagid);
	void restore_value(int tagid);
	void flush_value(int tagid);
public:
	void set(ICsdTags *ptags);
	void scan_start();
	void scan_end();
private:
	ICsdTags *m_ptags;
	bool m_skip;
};
CCsdTagsInternal::CCsdTagsInternal():m_ptags(NULL), m_skip(false)
{
}
CCsdTagsInternal::~CCsdTagsInternal()
{
}
long CCsdTagsInternal::QueryInterface(REFIID id, void **ppOut)
{
	return -1;
}
unsigned long CCsdTagsInternal::AddRef()
{
	return 1;
}
unsigned long CCsdTagsInternal::Release()
{
	return 1;
}
void CCsdTagsInternal::scan_start()
{
	m_skip=true;
}
void CCsdTagsInternal::scan_end()
{
	m_skip=false;
}
void CCsdTagsInternal::set(ICsdTags *ptags)
{
	m_ptags=ptags;
}
int CCsdTagsInternal::get(int tagid, void *lpParam)
{
	return m_ptags->get(tagid, lpParam);
}
int CCsdTagsInternal::get_default(int tagid, void *lpParam)
{
	return m_ptags->get_default(tagid, lpParam);
}
int CCsdTagsInternal::set(int tagid, long long lParam)
{
	if (m_skip && tagid!=CSDP_WINDOW && tagid!=CSDP_PRESCAN) {
		return -1;
	}
	return m_ptags->set(tagid, lParam);
}
int CCsdTagsInternal::set_default(int tagid)
{
	if (m_skip) return -1;
	return m_ptags->set_default(tagid);
}
int CCsdTagsInternal::change_default(int tagid, long long lParam)
{
	if (m_skip) return -1;
	return m_ptags->change_default(tagid, lParam);
}
int CCsdTagsInternal::choice_flag(int tagid, long *lpFlag)
{
	long f = 0;
	int ret = m_ptags->choice_flag(tagid, &f);
	switch (f) {
	default:
	case ICsdTag::CHOICE_ANY:*lpFlag = ICsdCore::CHOICE_ANY; break;
	case ICsdTag::CHOICE_RANGE:*lpFlag = ICsdCore::CHOICE_RANGE; break;
	case ICsdTag::CHOICE_LIST:*lpFlag = ICsdCore::CHOICE_LIST; break;
	}
	return ret;
}
int CCsdTagsInternal::choice_count(int tagid, long *lpCount)
{
	return m_ptags->choice_count(tagid, lpCount);
}
int CCsdTagsInternal::choice(int tagid, int index, void *lpParam)
{
	return m_ptags->choice(tagid, index, lpParam);
}
int CCsdTagsInternal::change_default(LPFNGETVALUE lpfn, void *callback_param)
{
	if (m_skip) return 0;
	return m_ptags->change_default(lpfn, callback_param);
}
int CCsdTagsInternal::save_value(LPFNSETVALUE lpfn, void* callback_param)
{
	if (m_skip) return 0;
	return m_ptags->save_value(lpfn, callback_param);
}
int CCsdTagsInternal::restore_value(LPFNGETVALUE lpfn, void *callback_param)
{
	if (m_skip) return 0;
	return m_ptags->restore_value(lpfn, callback_param);
}
void CCsdTagsInternal::save_value(int tagid)
{
	if (m_skip) return;
	m_ptags->save_value(tagid);
}
void CCsdTagsInternal::restore_value(int tagid)
{
	if (m_skip) return;
	m_ptags->restore_value(tagid);	
}
void CCsdTagsInternal::flush_value(int tagid)
{
	if (m_skip) return;
	m_ptags->flush_value(tagid);	
}
class CCsdScanContainer : public IScanCtrl
{
public:
	CCsdScanContainer();
	~CCsdScanContainer();
	long STDMETHODCALLTYPE QueryInterface(REFIID id, void **ppOut);
	unsigned long STDMETHODCALLTYPE AddRef();
	unsigned long STDMETHODCALLTYPE Release();	
	void init(IVirtualScanner *s, IUnknown *h, ICsdTags *t);
	void uninit();
	long scan_start();
	long prescan_start();
	long get_image(ICeiImage **ppOut);
	long get_information(long id, void *pout);
	int clear_image(char *pimg_ptr);
	long scan_end();
	long scanning();
	long abort();
	long stop();
private:
	XInterface<IScanCtrl> m_scan;
	XInterface<ICeiImage> m_img;
	IVirtualScanner *m_pscanner;
	IUnknown *m_handle;
	CCsdTagsInternal m_tags;
};
CCsdScanContainer::CCsdScanContainer():m_pscanner(NULL), m_handle(NULL)
{
}
CCsdScanContainer::~CCsdScanContainer()
{
	uninit();
}
long CCsdScanContainer::QueryInterface(REFIID id, void **ppOut)
{
	return -1;
}
unsigned long CCsdScanContainer::AddRef()
{
	return 1;
}
unsigned long CCsdScanContainer::Release()
{
	return 1;
}
void CCsdScanContainer::init(IVirtualScanner *s, IUnknown *h, ICsdTags *t)
{
	m_pscanner = s;
	m_handle = h;
	m_tags.set(t);
}
void CCsdScanContainer::uninit()
{
	m_scan=NULL;
	m_img=NULL;
}
long CCsdScanContainer::scan_start()
{
	//WriteLog((char*)"CCsdScanContainer::scan_start() start");
	if (m_scan.get()) {
		long prescan = 0;
		m_scan->get_information(CSDP_PRESCAN, (void*)&prescan);
		if (!prescan) m_scan = NULL;
	}
	m_scan.reset(scan_control(m_pscanner, (IUnknown*)&m_tags, m_handle, m_scan.Detach()));
	if (m_scan.get()==NULL) {
		SDKWriteLog((char*)"m_scan is NULL");
		return ENOMEM;
	}
	m_tags.scan_start();
	long out = m_scan->scan_start();
	//WriteLog((char*)"CCsdScanContainer::scan_start() end %d");
	return out;
}
long CCsdScanContainer::prescan_start()
{
	//WriteLog((char*)"CCsdScanContainer::prescan_start() start");
	if (m_scan.get()==NULL) m_scan.reset(prescan_control(m_pscanner, (IUnknown*)&m_tags, m_handle));
	if (m_scan.get() == NULL) {
		SDKWriteLog((char*)"m_scan is NULL");
		return ENOMEM;
	}
	m_tags.scan_start();
	long out = m_scan->scan_start();
	//WriteLog((char*)"CCsdScanContainer::prescan_start() end %d");
	return out;
}
long CCsdScanContainer::get_image(ICeiImage **ppOut)
{
	long ret = 0;
	if (m_scan.get()) {
		ret = m_scan->get_image(ppOut);
		if (ret) return ret;
		m_img=*ppOut;
    } else {
        ret = CSD3_NOMEM;
    }
	return ret;
}
long CCsdScanContainer::get_information(long id, void *pout)
{
	if (m_scan.get()) return m_scan->get_information(id, pout);
	return -1;
}
int CCsdScanContainer::clear_image(char *)
{
	m_img=NULL;
	return 0;
}
long CCsdScanContainer::scan_end()
{
	m_img=NULL;
	if (m_scan.get()) {
		m_scan->scan_end();
		m_scan=NULL;
		m_tags.scan_end();
	}
	return 0;
}
long CCsdScanContainer::scanning()
{
	if (m_scan.get()) return m_scan->scanning();
	return 0;
}
long CCsdScanContainer::abort()
{
	if (m_scan.get()) return m_scan->abort();
	return -1;
}
long CCsdScanContainer::stop()
{
	if (m_scan.get()) return m_scan->stop();
	return -1;
}

class CCsdCore : public ICsdCore
{
public:
	CCsdCore(LPVSCSD_SDK_INIT_INFORMATION pinfo, IUnknown *h);
	virtual ~CCsdCore();
	long STDMETHODCALLTYPE QueryInterface(REFIID id, void **ppOut);
	unsigned long STDMETHODCALLTYPE AddRef();
	unsigned long STDMETHODCALLTYPE Release();
public:
	int probe(LPVSCSD_SDK_INIT_INFORMATION pinfo);
	int terminate();
	int tagget(int uiParam, void *lpParam);
	int tagset(int uiParam, long long lParam);
	int tagget_choice_flag(int uiParNo, long *lpflag);
	int tagget_choice_count(int uiParNo, long *lpCount);
	int tagget_choice(int uiParNo, int iIndex, void *lpVoid);
	int tagchange_default(LPFNGETVALUE lpfn, void *callback_param);
	int tagchange_default(int tagid, void* lpParam);
	int tagget_default(int tagid, void *lpParam);
	int tagset_default(int tagid);
	int save_value(LPFNSETVALUE lpfn, void *callback_param);
	int restore_value(LPFNGETVALUE lpfn, void* callback_param);
	void save_value(int tagid);
	void restore_value(int tagid);
	void flush_value(int tagid);	
	int scan_start();
	int prescan_start();
	int image(ICeiImage **ppOut);
	int clear_image(char *pimg_ptr);
	int scan_end();
	int stop();
	int abort();
	int get_passthru(long id, void* p);
	int set_passthru(long id, void* p);
private:
	long m_ref;
	VSCSD_SDK_INIT_INFORMATION 	    m_info;
	XInterface<IVirtualScanner>	    m_scanner;
	CCsdScanContainer				m_scan;
	XInterface<ICsdTags>			m_tags;
	XInterface<IUnknown>			m_handle;
};
CCsdCore::CCsdCore(LPVSCSD_SDK_INIT_INFORMATION pinfo, IUnknown *h):m_ref(1)
{
	SDKWriteLog("CCsdCore::CCsdCore()");
	m_handle = h;
	if (pinfo) {
		long sz = sizeof(m_info);
		if (sz>pinfo->dwSize) sz = pinfo->dwSize;
		memcpy(&m_info, pinfo, sz);
		m_info.dwSize=sizeof(m_info);
	} else {
		memset(&m_info, 0, sizeof(m_info));
		m_info.dwSize = sizeof(m_info);
	}
}
CCsdCore::~CCsdCore()
{
	SDKWriteLog("CCsdCore::~CCsdCore()");
	//terminate();
	WriteLog_uninit();
}
long CCsdCore::QueryInterface(REFIID id, void **ppOut)
{
	return -1;
}
unsigned long CCsdCore::AddRef()
{
	m_ref++;
	return m_ref;
}
unsigned long CCsdCore::Release()
{
	m_ref--;
	if (m_ref<=0) {
		delete this;
		return 0;
	}
	return m_ref;
}
int CCsdCore::probe(LPVSCSD_SDK_INIT_INFORMATION pinfo)
{
	SDKWriteLog("CCsdCore::probe() start");
	if (pinfo->simulation) {
          	// Should never be reached in ChromeOS
          	return CSD3_SOFTWARE;
	}
	else {
		m_scanner.reset(create_virtual_scanner(pinfo->pscanner, m_handle));
		if (m_scanner.get() == NULL) {
			SDKWriteLog("ERROR:m_scanner is NULL L:%d F:%s", __LINE__, __FILE__);
			return CSD3_NOMEM;
		}
	}
	//
	char test_unit_ready_command[6]={0};
	m_scanner->exec_none(test_unit_ready_command, sizeof(test_unit_ready_command));
	//
	m_tags.reset(csdtags(&m_scan, m_scanner.get(), m_handle));
	m_scan.init(m_scanner.get(), m_handle, m_tags.get());
	SDKWriteLog("CCsdCore::probe() end\r\n");
	return CSD3_OK;
}
int CCsdCore::terminate()
{
	SDKWriteLog("CCsdCore::terminate() start");
	m_scan.uninit();
	m_tags.reset(NULL);
	SDKWriteLog("CCsdCore::terminate() end\r\n");
	return CSD3_OK;
}
int CCsdCore::tagget(int tagid, void *lpParam)
{
	return m_tags->get(tagid, lpParam);
}
int CCsdCore::tagset(int tagid, long long lParam)
{
	return m_tags->set(tagid, lParam);
}
int CCsdCore::tagget_choice_flag(int tagid, long *lpflag)
{
	return m_tags->choice_flag(tagid, lpflag);
}
int CCsdCore::tagget_choice_count(int tagid, long *lpCount)
{
	return m_tags->choice_count(tagid, lpCount);
}
int CCsdCore::tagget_choice(int tagid, int index, void *lpParam)
{
	return m_tags->choice(tagid, index, lpParam);
}
int CCsdCore::tagchange_default(LPFNGETVALUE lpfn, void *callback_param)
{
	return m_tags->change_default(lpfn, callback_param);
}
int CCsdCore::tagchange_default(int tagid, void* lpParam)
{
	return m_tags->change_default(tagid, (long long)lpParam);
}
int CCsdCore::tagget_default(int tagid, void *lpParam)
{
	return m_tags->get_default(tagid, lpParam);
}
int CCsdCore::tagset_default(int tagid)
{
	return m_tags->set_default(tagid);
}
int CCsdCore::save_value(LPFNSETVALUE lpfn, void *callback_param)
{
	return m_tags->save_value(lpfn, callback_param);
}
int  CCsdCore::restore_value(LPFNGETVALUE lpfn, void *callback_param)
{
	return m_tags->restore_value(lpfn, callback_param);
}
void CCsdCore::save_value(int tagid)
{
	m_tags->save_value(tagid);
}
void CCsdCore::restore_value(int tagid)
{
	m_tags->restore_value(tagid);
}
void CCsdCore::flush_value(int tagid)
{
	m_tags->flush_value(tagid);
}
int CCsdCore::scan_start()
{
	SDKWriteLog("CCsdCore::scan_start() start");
	printf_tags(m_tags.get());
	int out = (int)m_scan.scan_start();
	SDKWriteLog("CCsdCore::scan_start() end %d\r\n", out);
	return out;
}
int CCsdCore::prescan_start()
{
	SDKWriteLog("CCsdCore::prescan_start() start");
	printf_tags(m_tags.get());
	int out = (int)m_scan.prescan_start();
	SDKWriteLog("CCsdCore::prescan_start() end %d\r\n", out);
	return out;
}
int CCsdCore::image(ICeiImage **ppOut)
{
	SDKWriteLog("CCsdCore::image() start");
	int out = (int)m_scan.get_image(ppOut);
	//if (!out) {
	//	SDKWriteLog("ptr is 0x%lx", (*ppOut)->img());
	//}
	SDKWriteLog("CCsdCore::image() end %s\r\n", csderr3Tostr(out));
	return out;
}
int CCsdCore::clear_image(char *pimg_ptr)
{
	SDKWriteLog("CCsdCore::clear_image(0x%lx) start", pimg_ptr);
	int out = m_scan.clear_image(pimg_ptr);
	SDKWriteLog("CCsdCore::clear_image() end %d\r\n", out);
	return out;
}
int CCsdCore::scan_end()
{
	SDKWriteLog("CCsdCore::scan_end() start");
	int out = (int)m_scan.scan_end();
	SDKWriteLog("CCsdCore::scan_end() end %d\r\n", out);
	return out;
}
int CCsdCore::stop()
{
	SDKWriteLog("CCsdCore::stop() start");
	int out = (int)m_scan.stop();
	SDKWriteLog("CCsdCore::stop() end %d\r\n", out);
	return out;
}
int CCsdCore::abort()
{
	SDKWriteLog("CCsdCore::abort() start");
	int out =  (int)m_scan.abort();
	SDKWriteLog("CCsdCore::abort() end %d\r\n", out);
	return out;
}
int CCsdCore::get_passthru(long id, void* p)
{
	if (id == ICsdCore::PTID_VIRTUAL_SCANNER)
	{
		IVirtualScanner** ppout = (IVirtualScanner**)p;
		*ppout = m_scanner.get();
		m_scanner->AddRef();
	}
	return 0;
}
int CCsdCore::set_passthru(long id, void* p)
{
	return 0;
}
long  CsdSDKCreateCsdCore(ICsdCore **ppOut, LPVSCSD_SDK_INIT_INFORMATION pinfo, IUnknown *handle)
{
	if (ppOut==NULL) {
		printf("ppOut is NULL\r\n");
		return CSD3_BADPARM;
	}
	if (pinfo==NULL) {
		printf("ppOut is NULL\r\n");
		return CSD3_BADPARM;
	}
	ceisdk_set_module_name((char*)"csdcore");
	if (pinfo->lib_path[0]) {
		ceisdk_set_library_path(pinfo->lib_path);
	}
	if (pinfo->scanner_name[0]) {
		ceisdk_set_scanner_name(pinfo->scanner_name);
	}	
	WriteLog_init();
	//WriteLog(((char*)"[CsdSDK]CsdSDKCreateCsdCore() start"));	
	std::unique_ptr<CCsdCore>p(new CCsdCore(pinfo, handle));
	if (p.get()==NULL) {
		printf("no memroy L:%d F:%s\r\n", __LINE__, __FILE__);
		SDKWriteLog(((char*)"no memroy L:%d F:%s"), __LINE__, __FILE__);
		return CSD3_NOMEM;
	}
	*ppOut = (ICsdCore*)p.release();
	//WriteLog(((char*)"[CsdSDK]CsdSDKCreateCsdCore() end"));
	return CSD3_OK;
}
