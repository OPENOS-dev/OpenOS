/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <memory>
#include <mutex>
#include "ceilogwrite.h"
#include "ceiqueue.h"
#include "ceithread.h"
#include "csdtags.h"
#include "csderr.h"
#include "message_queue_interface.h"
#include "scanctrl_interface.h"
#include "sequence_thread_interface.h"
#include "tags_interface.h"
#include "sdk_message.h"
#include "global_apis.h"

namespace {
	char *msg2str(ICeiMessage *pmsg)
	{
		if (pmsg==NULL) return ((char*)"MID_NULL");
		switch (pmsg->type()) {
		case ICeiMessage::MID_BATCH_START:return ((char*)"pop:MID_BATCH_START");break;
		case ICeiMessage::MID_ERROR:return ((char*)"pop:MID_ERROR");;break;
		case ICeiMessage::MID_PAGE_START:return ((char*)"pop:MID_PAGE_START");;break;
		case ICeiMessage::MID_IMAGE_START:return ((char*)"pop:MID_IMAGE_START");;break;
		case ICeiMessage::MID_IMAGE:return ((char*)"pop:MID_IMAGE");;break;
		case ICeiMessage::MID_IMAGE_END:return ((char*)"pop:MID_IMAGE_END");break;
		case ICeiMessage::MID_INFO_START:return ((char*)"pop:MID_INFO_START");;break;
		case ICeiMessage::MID_INFO:return ((char*)"pop:MID_INFO");;break;
		case ICeiMessage::MID_INFO_END:return ((char*)"pop:MID_INFO_END");;break;
		case ICeiMessage::MID_PAGE_END:return ((char*)"pop:MID_PAGE_END");;break;
		case ICeiMessage::MID_BATCH_END:return ((char*)"pop:MID_BATCH_END");break;
		}
		return ((char*)"pop:MID_UNKNOWN");
	}
	bool is_output_jpeg_image(ICsdTags *tags)
	{
		long v=0;
		tags->get(CSDP_COMPRESSION, &v);
		return v>0;
	}	
}
class CQueueBetweenTh1AndTh2 : public IMessageQueue
{
public:
	CQueueBetweenTh1AndTh2();
	virtual ~CQueueBetweenTh1AndTh2();
	long push(ICeiMessage *pin);
	long pop(ICeiMessage **ppout);
	long peek(ICeiMessage **ppout, long order/*from 1*/);
	long count();
	void clear();
	long STDMETHODCALLTYPE QueryInterface(REFIID id, void **ppOut);
	unsigned long STDMETHODCALLTYPE AddRef();
	unsigned long STDMETHODCALLTYPE Release();
private:
	CCeiQueue<ICeiMessage*> m_queue;
};
CQueueBetweenTh1AndTh2::CQueueBetweenTh1AndTh2()
{
	//WriteLog((char*)"CQueueBetweenTh1AndTh2::CQueueBetweenTh1AndTh2()");
	m_queue.init(500);
}
CQueueBetweenTh1AndTh2::~CQueueBetweenTh1AndTh2()
{
	//WriteLog((char*)"CQueueBetweenTh1AndTh2::~CQueueBetweenTh1AndTh2()");
	clear();
}
void CQueueBetweenTh1AndTh2::clear()
{
	SDKWriteLog((char*)"clear-queue start");
	while (m_queue.count()) {
		ICeiMessage* pmsg = NULL;
		m_queue.pop(pmsg);
		if (pmsg) {
			SDKWriteLog("%s", msg2str(pmsg));
			if (pmsg->Release()&&pmsg->type()!=ICeiMessage::MID_ERROR) {
				SDKWriteLog("leak? %d %s", __LINE__, __FILE__);
			}
		}
		pmsg = NULL;
	}
	SDKWriteLog((char*)"clear-queue end");
}
long CQueueBetweenTh1AndTh2::QueryInterface(REFIID id, void **ppOut)
{
	return -1;
}
unsigned long CQueueBetweenTh1AndTh2::AddRef()
{
	return 1;
}
unsigned long CQueueBetweenTh1AndTh2::Release()
{
	return 1;
}
long CQueueBetweenTh1AndTh2::push(ICeiMessage *pin)
{
	//WriteLog((char*)"CQueueBetweenTh1AndTh2::push(%s) start", msg2str(pin));
	m_queue.push(pin);
	//WriteLog((char*)"CQueueBetweenTh1AndTh2::push() end");
	return 0;
}
long CQueueBetweenTh1AndTh2::pop(ICeiMessage **ppout)
{
	//WriteLog((char*)"CQueueBetweenTh1AndTh2::pop() start");
	if (ppout==NULL) return -1;
	ICeiMessage *p=NULL;
	m_queue.pop(p);
	*ppout = p;
	//WriteLog((char*)"CQueueBetweenTh1AndTh2::pop(%s) end", msg2str(p));
	return 0;
}
long CQueueBetweenTh1AndTh2::peek(ICeiMessage **ppout, long order/*from 1*/)
{
	if (ppout==NULL) return -1;
	ICeiMessage *p=NULL;	
	m_queue.peek(p, order);
	*ppout = p;
	return 0;
}
long CQueueBetweenTh1AndTh2::count()
{
	return m_queue.count();
}
class CScannedImageCtrl : public IScannedImageCtrl
{
public:
	CScannedImageCtrl();
	virtual ~CScannedImageCtrl();
	long STDMETHODCALLTYPE QueryInterface(REFIID id, void **ppOut);
	unsigned long STDMETHODCALLTYPE AddRef();
	unsigned long STDMETHODCALLTYPE Release();
	void scan_start();
	void increment();
	void decrement();
	void scan_end(long err);

	bool adf_empty();

	void init(ICsdTags *ptags);
	void uninit();
private:
	cei_semaphore m_counter;
	bool m_scanner_empty;
	long m_decrement_count;
	long m_max_decrement_count;
	long m_max_decrement;
	long m_debug_counter;
};
CScannedImageCtrl::CScannedImageCtrl() :m_scanner_empty(0), m_decrement_count(1), m_max_decrement_count(1), m_max_decrement(1), m_debug_counter(0)
{
	//SDKWriteLog("CScannedImageCtrl::CScannedImageCtrl()");
}
CScannedImageCtrl::~CScannedImageCtrl()
{
	uninit();
	if (m_debug_counter != 1 && m_debug_counter != 2) {
		SDKWriteLog("increment-decrement counter : %d %s", m_debug_counter, m_debug_counter == 0 ? "OK" : "NG something wrong happened.");
	}
	else {
		SDKWriteLog("WARNING:increment-decrement counter : %d", m_debug_counter);
		SDKWriteLog("WARNING:The scan thread got the counter up but driver did not get image. This is not bug, but the counter(0) after scanning is ideal.");
	}
#ifdef _WIN32
#ifdef _DEBUG
	if (m_debug_counter) {
		if (m_debug_counter != 1 && m_debug_counter != 2) {
			char s[256];
			sprintf(s, "increment-decrement counter : %d NG something wrong happened.", m_debug_counter);
			MessageBox(0, s, "Warning", MB_OK);
		}
	}
#endif
#endif
	//SDKWriteLog("CScannedImageCtrl::~CScannedImageCtrl()");
}
long CScannedImageCtrl::QueryInterface(REFIID id, void **ppOut)
{
	return 1;
}
unsigned long CScannedImageCtrl::AddRef()
{
	return 1;
}
unsigned long CScannedImageCtrl::Release()
{
	return 1;
}
void CScannedImageCtrl::init(ICsdTags* ptags)
{
	m_max_decrement_count = m_decrement_count = 1;
	m_max_decrement = 1;
	long v = 10;
	ptags->get(CSDP_MAX_AHEAD_PAGES, &v);
	SDKWriteLog("CSDP_MAX_AHEAD_PAGES:%d", v);
	if (v <= 0) v = 10;
	v = ceisdk_get_private_profile_int("CSDP_MAX_AHEAD_PAGES", "count", v);
	long folio = 0;
	ptags->get(CSDP_FOLIO, &folio);
	long source = 0;
	ptags->get(CSDP_FEEDER, &source);
	if (source || folio) {
		//duplex
		v = (v + 1) / 2 * 2;
	}

	//m_max_decrement_count will be decided.
	long window_count[2] = { 1, 1 };
	ptags->get(CSDP_WINDOWCOUNT_FRONT, &window_count[0]);
	ptags->get(CSDP_WINDOWCOUNT_BACK, &window_count[1]);
	if (source) {
		//duplex
		m_max_decrement_count = 2;
		m_max_decrement_count += (window_count[0]-1);
		m_max_decrement_count += (window_count[1]-1);
	}
	else {
		//simplex
		m_max_decrement_count += (window_count[0]-1);
	}
	long split = 0;
	ptags->get(CSDP_SPLITIMAGE, &split);
	if (split && folio) {
		v = 5000;
		m_max_decrement_count = 1;
		m_max_decrement = 1;
		SDKWriteLog("m_max_decrement_count is not decided. so v is %d", v);
	} else if (split) {
		m_max_decrement_count *= 2;
		if (source) {
			m_max_decrement = 2;
		}
	}
	else if (folio) {
		if (m_max_decrement_count > 1) m_max_decrement_count /= 2;
	}

	if (source||folio) {
		m_max_decrement = 2;//call decrement() twice in one time.
	}

	SDKWriteLog("Max ahead images : %d, max decrement count : %d, max decrement : %d", v, m_max_decrement_count, m_max_decrement);
	m_counter.init((int)v, (int)v);
}
void CScannedImageCtrl::uninit()
{
}
void CScannedImageCtrl::increment()
{
	
	SDKWriteLog("increment() start");
	m_counter.lock();
	m_debug_counter++;
	SDKWriteLog("increment() end");

}
void CScannedImageCtrl::decrement()
{
	if (m_decrement_count==m_max_decrement_count) {
		SDKWriteLog("decrement() start");
		for (long i = 0; i < m_max_decrement; i++) {	
			m_counter.unlock();
			m_debug_counter--;
			m_decrement_count = 1;
		}
		if (m_max_decrement == 1) SDKWriteLog("decrement() end");
		else                    SDKWriteLog("decrement() end x%d", m_max_decrement);
	}
	else {
		m_decrement_count++;
	}
}
void CScannedImageCtrl::scan_start()
{
	m_scanner_empty = 0;
}
void CScannedImageCtrl::scan_end(long err)
{
	if (err) {

	}
	else {
		m_scanner_empty = 1;
	}
}
bool CScannedImageCtrl::adf_empty()
{
	return m_scanner_empty;
}
class CImageBetweenCsdCoreAndApp : public ICeiImage
{
public:
	CImageBetweenCsdCoreAndApp(IScannedImageCtrl *psic, ICeiImage *pin);
	virtual ~CImageBetweenCsdCoreAndApp();
	long STDMETHODCALLTYPE QueryInterface(REFIID id, void **ppOut);
	unsigned long STDMETHODCALLTYPE AddRef();
	unsigned long STDMETHODCALLTYPE Release();
	char *img();
	long width();
	long height();
	long xdpi();
	long ydpi();
	long spp();
	long bps();
	long sync();
	long size();
	long comptype();//0:non, 1:jpeg
	long compinfo();//comptype is none:not used, comptype is jpeg:quality 	
private:
	long m_ref;
	XInterface<ICeiImage> m_img;
	IScannedImageCtrl *m_psic;
};
CImageBetweenCsdCoreAndApp::CImageBetweenCsdCoreAndApp(IScannedImageCtrl *psic, ICeiImage *pin):m_ref(1), m_psic(psic)
{
	m_img.reset(pin);
}
CImageBetweenCsdCoreAndApp::~CImageBetweenCsdCoreAndApp()
{
	m_psic->decrement();
}
long CImageBetweenCsdCoreAndApp::QueryInterface(REFIID id, void **ppOut)
{
	return  -1;
}
unsigned long CImageBetweenCsdCoreAndApp::AddRef()
{
	m_ref++;
	return m_ref;
}
unsigned long CImageBetweenCsdCoreAndApp::Release()
{
	m_ref--;
	if (m_ref <= 0) {
		delete this;
		return 0;
	}
	return m_ref;
}
char *CImageBetweenCsdCoreAndApp::img()
{
	return m_img->img();
}
long CImageBetweenCsdCoreAndApp::width()
{
	return m_img->width();
}
long CImageBetweenCsdCoreAndApp::height()
{
	return m_img->height();
}
long CImageBetweenCsdCoreAndApp::xdpi()
{
	return m_img->xdpi();
}
long CImageBetweenCsdCoreAndApp::ydpi()
{
	return m_img->ydpi();
}
long CImageBetweenCsdCoreAndApp::spp()
{
	return m_img->spp();
}
long CImageBetweenCsdCoreAndApp::bps()
{
	return m_img->bps();
}
long CImageBetweenCsdCoreAndApp::sync()
{
	return m_img->sync();
}
long CImageBetweenCsdCoreAndApp::size()
{
	return m_img->size();
}
long CImageBetweenCsdCoreAndApp::comptype()
{
	return m_img->comptype();
}
long CImageBetweenCsdCoreAndApp::compinfo()
{
	return m_img->compinfo();
}
class CCsdScan : public IScanCtrl
{
public:
	CCsdScan(IVirtualScanner *p, ICsdTags *t, IUnknown *h, IScanCtrl *ps);
	virtual ~CCsdScan();
	long STDMETHODCALLTYPE QueryInterface(REFIID id, void **ppOut);
	unsigned long STDMETHODCALLTYPE AddRef();
	unsigned long STDMETHODCALLTYPE Release();
	virtual long scan_start()=0;
	virtual long get_image(ICeiImage **ppOut);
	virtual long get_information(long id, void *pout);
	long scanning();
	long abort();
	long stop();
	virtual long scan_end()=0;
protected:
	long get_information_is_scan_done(void *pout);
	bool skip_image(bool& bloop);
protected:
	XInterface<IStartSequenceThread>m_start;
	XInterface<IEndSequenceThread>m_end;
protected:
	long m_scanning;
	std::mutex m_mutex;
	IUnknown *m_handle;
	IVirtualScanner *m_pscanner;
	CScannedImageCtrl m_scanned_image_ctl;
	long m_ref;
	ICsdTags *m_tags;
	XInterface<IScanCtrl> m_prescan;
protected:
	long getlong(long id, long def = 0);
};
CCsdScan::CCsdScan(IVirtualScanner *s, ICsdTags *t, IUnknown *h, IScanCtrl* ps):m_scanning(0), m_handle(h), m_pscanner(s), m_ref(1), m_tags(t)
{
	//SDKWriteLog((char*)"CCsdScan::CCsdScan()");
	if (ps) {
		m_prescan.reset(ps);
		m_tags->set(CSDP_PRESCAN, 0);
	}
	m_scanned_image_ctl.init(t);
}
CCsdScan::~CCsdScan()
{
	//SDKWriteLog((char*)"CCsdScan::~CCsdScan()");
}
long CCsdScan::QueryInterface(REFIID id, void **ppOut)
{
	return -1;
}
unsigned long CCsdScan::AddRef(){
	m_ref++;
	return m_ref;
}
unsigned long CCsdScan::Release(){
	m_ref--;
	if (m_ref<=0) {
		delete this;
		return 0;
	}
	return m_ref;
}
long CCsdScan::getlong(long id, long def)
{
	long v = def;
	m_tags->get(id, &v);
	return v;
}
bool CCsdScan::skip_image(bool& bloop)
{
	bool out = false;
	
	if (getlong(CSDP_SKIPBLANKPAGE)) {
		long blank = 0;
		m_end->get_information(CSDP_BLANKPAGE_DETECTED, &blank);
		if (blank) {
			SDKWriteLog("image is blank and will be skipped.");
			bloop = true;
			return true;
		}
	}
	if (getlong(CSDP_FEEDER)) {
		//duplex
		bloop = false;
		return false;
	}
	else {
		long fo = getlong(CSDP_FEEDER_OPTION, -1);
		if (fo < 0) {
			//CSDP_FEEDER_OPTION is not created.
		}
		else {
			long side = 0;
			m_end->get_information(CSDP_LASTPAGE_SIDE, &side);
			if (fo) {
				//simplex(back)
				if (side) {
					//back
					bloop = false;
					out = false;
				}
				else {
					//front
					SDKWriteLog("front image will be skipped due to CSDP_FEEDER_OPTION:1");
					bloop = true;
					out = true;
				}
			}
			else {
				//simplex(front)
				if (side) {
					//back
					SDKWriteLog("back image will be skipped due to CSDP_FEEDER_OPTION:0");
					bloop = true;
					out = true;
				}
				else {
					//front
					bloop = false;
					out = false;
				}
			}
		}
	}
	return out;
}
long CCsdScan::get_image(ICeiImage **ppOut)
{
	//SDKWriteLog((char*)"CCsdScan::get_image() start");
	std::lock_guard<std::mutex> lg(m_mutex);
	
	ICeiImage *pimg = NULL;
	long out = 0;

	if (m_prescan.get()) {
		out = m_prescan->get_image(&pimg);
		if (!out && pimg) {
			CImageBetweenCsdCoreAndApp* p = new CImageBetweenCsdCoreAndApp(&m_scanned_image_ctl, pimg);
			if (p == NULL) return CSD3_NOMEM;
			*ppOut = (ICeiImage*)p;
		}
		else {
			m_prescan = NULL;
			//WriteLog("m_prescan->get_image() return %d", out);
			if (getlong(CSDP_PRESCAN_OPTION)) return CSD3_NOPAGE;
		}
	}
	if (pimg==NULL) {
		if (m_end.get() == NULL) return CSD3_NOMEM;

		bool bloop = true;
		while (bloop) {
			bloop = false;
			out = m_end->get_image(&pimg);
			if (!out && pimg) {
                if (skip_image(bloop)) {
					CImageBetweenCsdCoreAndApp skipped(&m_scanned_image_ctl, pimg);
				}
				else {
					CImageBetweenCsdCoreAndApp* p = new CImageBetweenCsdCoreAndApp(&m_scanned_image_ctl, pimg);
					if (p == NULL) return CSD3_NOMEM;
					*ppOut = (ICeiImage*)p;
				}
			}
		}
	}
	//SDKWriteLog((char*)"CCsdScan::get_image() end\r\n");
	return out;
}
long CCsdScan::get_information_is_scan_done(void *p)
{
	long *pout = (long *)p;
	*pout =  m_scanned_image_ctl.adf_empty();
	return 0;
}
long CCsdScan::get_information(long id, void *pout)
{
	SDKWriteLog((char*)"CCsdScan::get_information(%ld) start", id);
	std::lock_guard<std::mutex> lg(m_mutex);

	long out = 0;
	if (id == CSDP_PRESCAN) {
		long* p = (long*)pout;
		*p = 0;
	} else	if (m_prescan.get()) {
		out = m_prescan->get_information(id, pout);
	}
	else {
		if (id == CSDP_IS_SCAN_DONE) {
			out = get_information_is_scan_done(pout);
		}
		else {
			if (m_end.get() == NULL) {
				SDKWriteLog("m_end.get() == NULL");
				return CSD3_NOMEM;
			}
			out = m_end->get_information(id, pout);
		}
	}
	SDKWriteLog((char*)"CCsdScan::get_information() end %ld\r\n", out);
	return out;
}
long  CCsdScan::scanning()
{
	std::lock_guard<std::mutex> lg(m_mutex);
	return m_scanning;
}
long  CCsdScan::abort()
{
	std::lock_guard<std::mutex> lg(m_mutex);
	if (m_start.get()) return m_start->abort();
	return 0;
}
long  CCsdScan::stop()
{
	std::lock_guard<std::mutex> lg(m_mutex);
	if (m_start.get()) return m_start->stop();
	return 0;
}
class CCsdPrescan : public CCsdScan
{
public:
	CCsdPrescan(IVirtualScanner* p, ICsdTags* t, IUnknown* h);
	virtual ~CCsdPrescan();
	long scan_start();
	long get_image(ICeiImage** ppOut);
	long get_information(long id, void* pout);
	long scan_end();
	CQueueBetweenTh1AndTh2 m_scan2backup;
	CQueueBetweenTh1AndTh2 m_backup2ip;
	CQueueBetweenTh1AndTh2 m_ip2comp;
	CQueueBetweenTh1AndTh2 m_ip2end;
	CQueueBetweenTh1AndTh2 m_comp2end;
	XInterface<IMidSequenceThread>  m_backup;
	XInterface<IMidSequenceThread>  m_ip;
	XInterface<IMidSequenceThread>  m_comp;
};
CCsdPrescan::CCsdPrescan(IVirtualScanner* s, ICsdTags* t, IUnknown* h) :CCsdScan(s, t, h, NULL)
{
}
CCsdPrescan::~CCsdPrescan()
{
	scan_end();
}
long  CCsdPrescan::scan_start()
{
	//SDKWriteLog((char*)"CCsdPrescan::scan_start() start");
	std::lock_guard<std::mutex> lg(m_mutex);
	m_scanning = 1;
	if (m_backup.get()) {
		printf("scan sequence start\r\n");
		m_ip2end.clear();
		m_comp2end.clear();
		m_scan2backup.push(create_message(ICeiMessage::MID_BATCH_START, (void*)0));
		m_scan2backup.push(create_message(ICeiMessage::MID_BATCH_END, (void*)0));
		m_backup->proc();
		printf("scan sequence end\r\n");
		printf("ip sequence start\r\n");
		m_ip->proc();
		printf("ip sequence end\r\n");
		if (is_output_jpeg_image(m_tags)) {
			printf("comp sequence start\r\n");
			m_comp->proc();
			printf("comp sequence end\r\n");
		}
	}
	else {
		m_start.reset(scan_sequence_thread(m_pscanner, (IUnknown*)m_tags, &m_scanned_image_ctl, &m_scan2backup, m_handle));
		m_backup.reset(backup_sequence_thread(&m_scan2backup, &m_backup2ip, &m_scanned_image_ctl, m_handle));
		if (is_output_jpeg_image(m_tags)) {
			m_ip.reset(ip_sequence_thread(&m_backup2ip, &m_ip2comp, &m_scanned_image_ctl, m_handle));
			m_comp.reset(comp_sequence_thread(&m_ip2comp, &m_comp2end, &m_scanned_image_ctl, m_tags));
			m_end.reset(end_sequence_thread(&m_comp2end, &m_scanned_image_ctl, m_handle));
		}
		else {
			m_ip.reset(ip_sequence_thread(&m_backup2ip, &m_ip2end, &m_scanned_image_ctl, m_handle));
			m_end.reset(end_sequence_thread(&m_ip2end, &m_scanned_image_ctl, m_handle));
		}
		printf("scan sequence start\r\n");
		m_start->proc();
		printf("scan sequence end\r\n");
		m_backup->proc();
		printf("ip sequence start\r\n");
		m_ip->proc();
		printf("ip sequence end\r\n");
		if (is_output_jpeg_image(m_tags)) {
			printf("comp sequence start\r\n");
			m_comp->proc();
			printf("comp sequence end\r\n");
		}
	}
	//SDKWriteLog((char*)"CCsdPrescan::scan_start() end\r\n");
	return 0;
}
long CCsdPrescan::get_image(ICeiImage** ppOut)
{
	ICeiImage* pimg = NULL;
	long out = m_end->get_image(&pimg);
	if (out || pimg==NULL) return out;
	*ppOut = new CImageBetweenCsdCoreAndApp(&m_scanned_image_ctl, pimg);
	return 0;
}
long CCsdPrescan::get_information(long id, void* pout)
{
	SDKWriteLog((char*)"CCsdPrescan::get_information(%ld) start", id);
	if (id == CSDP_PRESCAN && pout) {
		long* p = (long*)pout;
		*p = 1;
		return 0;
	}
	long out = CCsdScan::get_information(id, pout);
	SDKWriteLog((char*)"CCsdPrescan::get_information() end %d", out);
	return out;
}
long CCsdPrescan::scan_end()
{
	//SDKWriteLog((char*)"CCsdPrescan::scan_end() start");
	std::lock_guard<std::mutex> lg(m_mutex);
	m_start.reset(NULL);
	m_ip.reset(NULL);
	m_comp.reset(NULL);
	m_end.reset(NULL);
	m_backup.reset(NULL);
	m_scanning = 0;
	//SDKWriteLog((char*)"CCsdPrescan::scan_end() end\r\n");
	return 0;
}
class CCsdScanAheadOn : public CCsdScan
{
public:
	CCsdScanAheadOn(IVirtualScanner* p, ICsdTags* t, IUnknown* h, IScanCtrl* ps);
	virtual ~CCsdScanAheadOn();
	long scan_start();
	long scan_end();
private:
	static void* scan_thread_static(void* pscan);
	static void* ip_thread_static(void* pscan);
	static void* comp_thread_static(void* pscan);
	void scan_thread();
	void ip_thread();
	void comp_thread();
	std::unique_ptr<ceithread> m_scan_thread;
	std::unique_ptr<ceithread> m_ip_thread;
	std::unique_ptr<ceithread> m_comp_thread;
private:
	XInterface<IMidSequenceThread> m_ip;
	XInterface<IMidSequenceThread> m_comp;
	CQueueBetweenTh1AndTh2 m_scan2ip;
	CQueueBetweenTh1AndTh2 m_ip2comp;
	CQueueBetweenTh1AndTh2 m_ip2end;
	CQueueBetweenTh1AndTh2 m_comp2end;
private:
	long scan_start_threads();
};
CCsdScanAheadOn::CCsdScanAheadOn(IVirtualScanner* s, ICsdTags* t, IUnknown* h, IScanCtrl* ps) : CCsdScan(s, t, h ,ps)
{
}
CCsdScanAheadOn::~CCsdScanAheadOn()
{
	scan_end();
}
void* CCsdScanAheadOn::scan_thread_static(void* pscan)
{
	CCsdScanAheadOn* p = (CCsdScanAheadOn*)pscan;
	p->scan_thread();
#ifndef _WIN32
	pthread_exit(NULL);
#endif
	return NULL;
}
void CCsdScanAheadOn::scan_thread()
{
	if (m_start.get()) {
		m_scanning = 1;
		m_start->proc();
		m_scanning = 0;
	}
}
void* CCsdScanAheadOn::ip_thread_static(void* pscan)
{
	CCsdScanAheadOn* p = (CCsdScanAheadOn*)pscan;
	p->ip_thread();
#ifndef _WIN32
	pthread_exit(NULL);
#endif
	return NULL;
}
void CCsdScanAheadOn::ip_thread()
{
	if (m_ip.get()) {
		m_ip->proc();
	}
}
void* CCsdScanAheadOn::comp_thread_static(void* pscan)
{
	CCsdScanAheadOn* p = (CCsdScanAheadOn*)pscan;
	p->comp_thread();
#ifndef _WIN32
	pthread_exit(NULL);
#endif
	return NULL;
}
void CCsdScanAheadOn::comp_thread()
{
	if (m_comp.get()) {
		m_comp->proc();
	}
}
long CCsdScanAheadOn::scan_start_threads()
{
	if (m_scan_thread.get() == NULL) {
		m_scan_thread.reset(new ceithread);
		if (m_scan_thread.get() == NULL) {
			SDKWriteLog("no memory L:%d F:%s", __LINE__, __FILE__);
			return CSD3_NOMEM;
		}
		m_ip_thread.reset(new ceithread);
		if (m_ip_thread.get() == NULL) {
			SDKWriteLog("no memory L:%d F:%s", __LINE__, __FILE__);
			return CSD3_NOMEM;
		}
		m_start.reset(scan_sequence_thread(m_pscanner, (IUnknown*)m_tags, &m_scanned_image_ctl, &m_scan2ip, m_handle));
		if (m_start.get() == NULL) {
			SDKWriteLog("no memory L:%d F:%s", __LINE__, __FILE__);
			return CSD3_NOMEM;
		}
		if (is_output_jpeg_image(m_tags)) {
			m_comp_thread.reset(new ceithread);
			if (m_comp_thread.get() == NULL) {
				SDKWriteLog("no memory L:%d F:%s", __LINE__, __FILE__);
				return CSD3_NOMEM;
			}
			m_ip.reset(ip_sequence_thread(&m_scan2ip, &m_ip2comp, &m_scanned_image_ctl, m_handle));
			if (m_ip.get() == NULL) {
				SDKWriteLog("no memory L:%d F:%s", __LINE__, __FILE__);
				return CSD3_NOMEM;
			}
			m_comp.reset(comp_sequence_thread(&m_ip2comp, &m_comp2end, &m_scanned_image_ctl, m_handle));
			if (m_comp.get() == NULL) {
				SDKWriteLog("no memory L:%d F:%s", __LINE__, __FILE__);
				return CSD3_NOMEM;
			}
			m_end.reset(end_sequence_thread(&m_comp2end, &m_scanned_image_ctl, m_handle));
			if (m_end.get() == NULL) {
				SDKWriteLog("no memory L:%d F:%s", __LINE__, __FILE__);
				return CSD3_NOMEM;
			}
			m_scan_thread->create(scan_thread_static, (void*)this);
			m_ip_thread->create(ip_thread_static, (void*)this);
			m_comp_thread->create(comp_thread_static, (void*)this);
		}
		else {
			m_ip.reset(ip_sequence_thread(&m_scan2ip, &m_ip2end, &m_scanned_image_ctl, m_handle));
			if (m_ip.get() == NULL) {
				SDKWriteLog("no memory L:%d F:%s", __LINE__, __FILE__);
				return CSD3_NOMEM;
			}
			m_end.reset(end_sequence_thread(&m_ip2end, &m_scanned_image_ctl, m_handle));
			if (m_end.get() == NULL) {
				SDKWriteLog("no memory L:%d F:%s", __LINE__, __FILE__);
				return CSD3_NOMEM;
			}
			m_scan_thread->create(scan_thread_static, (void*)this);
			m_ip_thread->create(ip_thread_static, (void*)this);
		}
	}
	return CSD3_OK;
}
long CCsdScanAheadOn::scan_start()
{
	//SDKWriteLog((char*)"CCsdScan::scan_start() start");
	std::lock_guard<std::mutex> lg(m_mutex);
	long out = CSD3_OK;
	if (m_prescan.get()) {
		m_prescan->scan_start();
	}
	if (getlong(CSDP_PRESCAN_OPTION)) {
		//driver has to send only scanned images.
	}
	else {
		out = scan_start_threads();
	}
	//SDKWriteLog((char*)"CCsdScan::scan_start() end\r\n");
	return out;
}
long  CCsdScanAheadOn::scan_end()
{
	//SDKWriteLog((char*)"CCsdScan::scan_end() start");
	std::lock_guard<std::mutex> lg(m_mutex);
	if (m_scan_thread.get()) {
		if (m_end.get()) {
			SDKWriteLog((char*)"m_end->get_image(12345)->release in");
			ICeiImage* pimg = (ICeiImage*)12345;
			while (1) {
				long ret = m_end->get_image(&pimg);
				if (ret) break;
                if (pimg && pimg!=(ICeiImage*)12345) pimg->Release();
				pimg = NULL;
			}
			SDKWriteLog((char*)"m_end->get_image()->release out");
		}
		if (m_scan_thread->joinable()) {
			if (m_start.get()) {
				SDKWriteLog((char*)"m_start->stop() in");
				m_start->stop();
				SDKWriteLog((char*)"m_start->stop() out");
			}
			SDKWriteLog((char*)"m_scan_thread->join() in");
			m_scan_thread->join();
			SDKWriteLog((char*)"m_scan_thread->join() out");
		}
		m_scan_thread.reset(NULL);
		if (m_ip_thread->joinable()) {
			SDKWriteLog((char*)"m_ip_thread->join() in");
			m_ip_thread->join();
			SDKWriteLog((char*)"m_ip_thread->join() out");
		}
		m_ip_thread.reset(NULL);
		if (m_comp_thread.get() && m_comp_thread->joinable()) {
			SDKWriteLog((char*)"m_comp_thread->join() in");
			m_comp_thread->join();
			SDKWriteLog((char*)"m_comp_thread->join() out");
		}
		m_comp_thread.reset(NULL);
	}
	m_start.reset(NULL);
	m_ip.reset(NULL);
	m_comp.reset(NULL);
	m_end.reset(NULL);
	m_prescan = NULL;
	//SDKWriteLog((char*)"CCsdScan::scan_end() end\r\n");
	return 0;
}
class CCsdScanAheadOff : public CCsdScan
{
public:
	CCsdScanAheadOff(IVirtualScanner* p, ICsdTags* t, IUnknown* h, IScanCtrl* ps);
	virtual ~CCsdScanAheadOff();
	long scan_start();
	long scan_end();
private:
	XInterface<IMidSequenceThread> m_ip;
	XInterface<IMidSequenceThread> m_comp;
	CQueueBetweenTh1AndTh2 m_scan2ip;
	CQueueBetweenTh1AndTh2 m_ip2comp;
	CQueueBetweenTh1AndTh2 m_ip2end;
	CQueueBetweenTh1AndTh2 m_comp2end;
};
CCsdScanAheadOff::CCsdScanAheadOff(IVirtualScanner* s, ICsdTags* t, IUnknown* h, IScanCtrl* ps) :CCsdScan(s, t, h, ps)
{
	//SDKWriteLog("CCsdScanAheadOff::CCsdScanAheadOff()");
}
CCsdScanAheadOff::~CCsdScanAheadOff()
{
	scan_end();
	//SDKWriteLog("CCsdScanAheadOff::~CCsdScanAheadOff()");
}
long  CCsdScanAheadOff::scan_start()
{
	//SDKWriteLog((char*)"CCsdScanAheadOff::scan_start() start");
	std::lock_guard<std::mutex> lg(m_mutex);
	if (m_prescan.get()) {
		m_prescan->scan_start();
	}

	m_start.reset(scan_sequence_thread(m_pscanner, (IUnknown*)m_tags, &m_scanned_image_ctl, &m_scan2ip, m_handle));
	if (m_start.get() == NULL) {
		SDKWriteLog("no memory L:%d F:%s", __LINE__, __FILE__);
		return CSD3_NOMEM;
	}
	if (is_output_jpeg_image(m_tags)) {
		m_ip.reset(ip_sequence_thread(&m_scan2ip, &m_ip2comp, &m_scanned_image_ctl, m_handle));
		if (m_ip.get() == NULL) {
			SDKWriteLog("no memory L:%d F:%s", __LINE__, __FILE__);
			return CSD3_NOMEM;
		}
		m_comp.reset(comp_sequence_thread(&m_ip2comp, &m_comp2end, &m_scanned_image_ctl, m_handle));
		if (m_comp.get() == NULL) {
			SDKWriteLog("no memory L:%d F:%s", __LINE__, __FILE__);
			return CSD3_NOMEM;
		}
		m_end.reset(end_sequence_thread(&m_comp2end, &m_scanned_image_ctl, m_handle));
		if (m_end.get() == NULL) {
			SDKWriteLog("no memory L:%d F:%s", __LINE__, __FILE__);
			return CSD3_NOMEM;
		}
		m_start->proc();
		m_ip->proc();
		m_comp->proc();
	}
	else {
		m_ip.reset(ip_sequence_thread(&m_scan2ip, &m_ip2end, &m_scanned_image_ctl, m_handle));
		if (m_ip.get() == NULL) {
			SDKWriteLog("no memory L:%d F:%s", __LINE__, __FILE__);
			return CSD3_NOMEM;
		}
		m_end.reset(end_sequence_thread(&m_ip2end, &m_scanned_image_ctl, m_handle));
		if (m_end.get() == NULL) {
			SDKWriteLog("no memory L:%d F:%s", __LINE__, __FILE__);
			return CSD3_NOMEM;
		}
		m_start->proc();
		m_ip->proc();
	}


	
	//SDKWriteLog((char*)"CCsdScanAheadOff::scan_start() end\r\n");
	return 0;
}
long  CCsdScanAheadOff::scan_end()
{
	//SDKWriteLog((char*)"CCsdScanAheadOff::scan_end() start");
	std::lock_guard<std::mutex> lg(m_mutex);
	m_start.reset(NULL);
	m_ip.reset(NULL);
	m_comp.reset(NULL);
	m_end.reset(NULL);
	m_prescan = NULL;
	//SDKWriteLog((char*)"CCsdScanAheadOff::scan_end() end\r\n");
	return 0;
}

IScanCtrl *scan_control(IUnknown *p, IUnknown *t, IUnknown *h, IScanCtrl *prescan)
{
	IScanCtrl* pout = NULL;
	ICsdTags* pt = (ICsdTags*)t;
	long ahead = 0;
	pt->get(CSDP_READAHEAD, (void*)&ahead);
	if (ahead) {
		pout = (IScanCtrl*) new CCsdScanAheadOn((IVirtualScanner*)p, (ICsdTags*)t, h, prescan);
	}
	else {
		pout = (IScanCtrl*) new CCsdScanAheadOff((IVirtualScanner*)p, (ICsdTags*)t, h, prescan);
	}
	return pout;
}
IScanCtrl* prescan_control(IUnknown* p, IUnknown* t, IUnknown* h)
{
	return new CCsdPrescan((IVirtualScanner*)p, (ICsdTags*)t, h);
}
