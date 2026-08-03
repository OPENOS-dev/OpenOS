/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <memory>
#include <mutex>
#include "ceilogwrite.h"
#include "ceiqueue.h"
#include "ceithread.h"
#include "message_queue_interface.h"
#include "scanctrl_interface.h"
#include "sequence_thread_interface.h"
#include "scanner_connector_interface.h"

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
		return ((char*)"MID_UNKNOWN");
	}
}
class CQueueBetweenScanAndEnd : public IMessageQueue
{
public:
	CQueueBetweenScanAndEnd();
	virtual ~CQueueBetweenScanAndEnd();
	long push(ICeiMessage *pin);
	long pop(ICeiMessage **ppout);
	long peek(ICeiMessage **ppout, long order/*from 1*/);
	long count();
	long STDMETHODCALLTYPE QueryInterface(REFIID id, void **ppOut);
	unsigned long STDMETHODCALLTYPE AddRef();
	unsigned long STDMETHODCALLTYPE Release();
private:
	CCeiQueue<ICeiMessage*> m_queue;
};
CQueueBetweenScanAndEnd::CQueueBetweenScanAndEnd()
{
	//WriteLog((char*)"CQueueBetweenScanAndEnd::CQueueBetweenScanAndEnd()");
	m_queue.init(200);
}
CQueueBetweenScanAndEnd::~CQueueBetweenScanAndEnd()
{
	//WriteLog((char*)"CQueueBetweenScanAndEnd::~CQueueBetweenScanAndEnd()");
	SDKWriteLog((char*)"clear-queue start");
	while (m_queue.count()) {
		ICeiMessage *pmsg=NULL;
		m_queue.pop(pmsg);
		if (pmsg) {
			SDKWriteLog("%s", msg2str(pmsg));
			pmsg->Release();
		}
		pmsg=NULL;
	}
	SDKWriteLog((char*)"clear-queue end");
}
long CQueueBetweenScanAndEnd::QueryInterface(REFIID id, void **ppOut)
{
	return -1;
}
unsigned long CQueueBetweenScanAndEnd::AddRef()
{
	return 1;
}
unsigned long CQueueBetweenScanAndEnd::Release()
{
	return 1;
}
long CQueueBetweenScanAndEnd::push(ICeiMessage *pin)
{
	//WriteLog((char*)"CQueueBetweenScanAndEnd::push(%s) start", msg2str(pin));
	m_queue.push(pin);
	//WriteLog((char*)"CQueueBetweenScanAndEnd::push() end");
	return 0;
}
long CQueueBetweenScanAndEnd::pop(ICeiMessage **ppout)
{
	//WriteLog((char*)"CQueueBetweenScanAndEnd::pop() start");
	if (ppout==NULL) return -1;
	ICeiMessage *p=NULL;
	m_queue.pop(p);
	*ppout = p;
	//WriteLog((char*)"CQueueBetweenScanAndEnd::pop(%s) end", msg2str(p));
	return 0;
}
long CQueueBetweenScanAndEnd::peek(ICeiMessage **ppout, long order/*from 1*/)
{
	if (ppout==NULL) return -1;
	ICeiMessage *p=NULL;	
	m_queue.peek(p, order);
	*ppout = p;
	return 0;
}
long CQueueBetweenScanAndEnd::count()
{
	return m_queue.count();
}
class CVSScan : public IScanCtrl
{
public:
	CVSScan(IScannerConnector *pscanner, IScannedImageCtrl *psci, IUnknown *handle);
	virtual ~CVSScan();
	long STDMETHODCALLTYPE QueryInterface(REFIID id, void **ppOut);
	unsigned long STDMETHODCALLTYPE AddRef();
	unsigned long STDMETHODCALLTYPE Release();
	long scan_start();
	long scan_end();
	long get_image(ICeiImage **ppOut);
	long get_information(long id, void *);
	long scanning();
	long abort();
	long stop();
private:
	static void *scan_thread_static(void *pscan);
	void scan_thread();
	std::unique_ptr<ceithread>m_th;
	XInterface<IStartSequenceThread>m_start;
	XInterface<IEndSequenceThread>m_end;
	long m_scanning;
	std::mutex m_mutex;
	CQueueBetweenScanAndEnd m_queue;
private:
	IUnknown *m_handle;
	IScannerConnector *m_pscanner;
	IScannedImageCtrl *m_scanned_image_ctrl;
	long m_ref;
};
CVSScan::CVSScan(IScannerConnector *pscanner, IScannedImageCtrl *psci, IUnknown *handle):
m_scanning(0),
m_handle(handle),
m_pscanner(pscanner), 
m_scanned_image_ctrl(psci),
m_ref(1)
{
	//WriteLog((char*)"CVSScan::CVSScan()");
}
CVSScan::~CVSScan()
{
	//WriteLog((char*)"CVSScan::~CVSScan()");
	scan_end();
}
long CVSScan::QueryInterface(REFIID id, void **ppOut)
{
	return -1;
}
unsigned long CVSScan::AddRef(){
	m_ref++;
	return m_ref;
}
unsigned long CVSScan::Release(){
	m_ref--;
	if (m_ref<=0) {
		delete this;
		return 0;
	}
	return m_ref;
}
void *CVSScan::scan_thread_static(void *pscan) 
{
	CVSScan *p=(CVSScan*)pscan;
	p->scan_thread();
#ifndef _WIN32
	pthread_exit(NULL);
#endif
	return NULL;
}
void CVSScan::scan_thread()
{
	if (m_start.get()) {
		m_scanning=1;
		m_start->proc();
		m_scanning=0;
	}
}
long  CVSScan::scan_start()
{
	//WriteLog((char*)"CVSScan::scan_start() start");
	std::lock_guard<std::mutex> lg(m_mutex);
	if (m_th.get()==NULL) {
		WriteLog_setname("end");
		m_th.reset(new ceithread);
		if (m_th.get()==NULL) return ENOMEM;
		m_start.reset(scan_sequence_thread((IUnknown*)m_pscanner, NULL, m_scanned_image_ctrl, &m_queue, m_handle));
		if (m_start.get()==NULL) return ENOMEM;
		m_end.reset(end_sequence_thread(&m_queue, m_scanned_image_ctrl, m_handle));
		if (m_end.get()==NULL) return ENOMEM;
		m_th->create(scan_thread_static, (void*)this);
	}
	//WriteLog((char*)"CVSScan::scan_start() end");
	return 0;
}
long  CVSScan::scan_end()
{
	//WriteLog((char*)"CVSScan::scan_end() start");
	std::lock_guard<std::mutex> lg(m_mutex);
	if (m_th.get()) {
		if (m_th->joinable()) {
			SDKWriteLog((char*)"m_th->join() in");
			m_th->join();
			SDKWriteLog((char*)"m_th->join() out");
		}
		m_th.reset(NULL);
		WriteLog_setname(NULL);
	}
	m_start.reset(NULL);
	m_end.reset(NULL);
	//WriteLog((char*)"CVSScan::scan_end() end");
	return 0;
}
long CVSScan::get_image(ICeiImage **ppOut)
{
	//SDKWriteLog((char*)"CVSScan::get_image() start");
	std::lock_guard<std::mutex> lg(m_mutex);
	if (m_end.get()==NULL) return ENOMEM;
	long out = m_end->get_image(ppOut);
	if (!out) {
		ICeiImage *pimg = *ppOut;
		SDKWriteLog("image ptr:0x%lx", pimg->img());
		SDKWriteLog("width:%ld", pimg->width());
		SDKWriteLog("height:%ld", pimg->height());
		SDKWriteLog("xdpi:%ld", pimg->xdpi());
		SDKWriteLog("ydpi:%ld", pimg->ydpi());
		SDKWriteLog("spp:%ld", pimg->spp());
		SDKWriteLog("bps:%ld", pimg->bps());
		SDKWriteLog("sync:%ld", pimg->sync());
		SDKWriteLog("size:%ld", pimg->size());
		SDKWriteLog("comptype:%ld", pimg->comptype());
		SDKWriteLog("compinfo:%ld", pimg->compinfo());
	}
	//SDKWriteLog((char*)"CVSScan::get_image() end");
	return out;
}
long CVSScan::get_information(long id, void *pout)
{
	//SDKWriteLog((char*)"CVSScan::get_information() start");
	std::lock_guard<std::mutex> lg(m_mutex);
	if (m_end.get()==NULL) {
		SDKWriteLog("no memory L:%d F:%s", __LINE__, __FILE__);
		return ENOMEM;
	}
	long out = m_end->get_information(id, pout);
	//SDKWriteLog((char*)"CVSScan::get_information() end");
	return out;
}
long  CVSScan::scanning()
{
	//std::lock_guard<std::mutex> lg(m_mutex); // Do not enable this line.
	return m_scanning;
}
long  CVSScan::abort()
{
	std::lock_guard<std::mutex> lg(m_mutex);
	if (m_start.get()) return m_start->abort();
	return 0;
}
long  CVSScan::stop()
{
	std::lock_guard<std::mutex> lg(m_mutex);
	if (m_start.get()) return m_start->stop();
	return 0;
}	
IScanCtrl *scan_control(IUnknown *pscanner, IUnknown *option, IUnknown *handle, IScanCtrl */*not_used*/)
{
	return new CVSScan((IScannerConnector*)pscanner, (IScannedImageCtrl*)option, handle);
}