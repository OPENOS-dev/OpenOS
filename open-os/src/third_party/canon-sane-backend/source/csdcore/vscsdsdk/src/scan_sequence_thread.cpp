/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <memory>
#include "ceilogwrite.h"
#include "sdk_message.h"
#include "sequence_thread_interface.h"
#include "scan_sequence_interface.h"

class CCallback : public IScanSequenceCallback2
{
public:
	CCallback(IScannedImageCtrl *pcanned_image_ctrl);
	virtual ~CCallback();
	long STDMETHODCALLTYPE QueryInterface(REFIID id, void **ppOut);
	unsigned long STDMETHODCALLTYPE AddRef();
	unsigned long STDMETHODCALLTYPE Release();	
	bool stop_request();
	void batch_start();
	void page_start();
	void page_end(long number_of_scanned_image);
	void batch_end(long error_happened);


	void stop_request(bool b);
private:
	long m_ref;
	bool m_req;
	IScannedImageCtrl *m_scanned_image_ctrl;
	long m_num;
};
CCallback::CCallback(IScannedImageCtrl *pscanned_image_ctrl) :
	m_ref(1),
	m_req(false),
	m_scanned_image_ctrl(pscanned_image_ctrl),
	m_num(0)
{}
CCallback::~CCallback()
{}
long CCallback::QueryInterface(REFIID id, void **ppOut)
{
	return -1;
}
unsigned long CCallback::AddRef()
{
	m_ref++;
	return m_ref;
}
unsigned long CCallback::Release()
{
	m_ref--;
	if (m_ref<=0) {
		delete this;
		return 0;
	}
	return m_ref;
}
bool CCallback::stop_request()
{
	return m_req;
}
void CCallback::stop_request(bool b)
{
	m_req = b;
}
void CCallback::batch_start()
{
	m_scanned_image_ctrl->scan_start();
}
void CCallback::batch_end(long error_happened)
{
	if (m_req) error_happened = 1;
	m_scanned_image_ctrl->scan_end(error_happened);
}
void CCallback::page_start()
{
	if (m_num) {
		for (long i = 0; i < m_num; i++) {
			m_scanned_image_ctrl->increment();
		}
	}
}
void CCallback::page_end(long number_of_scanned_image)
{
	if (m_num) {

	}
	else {
		m_num = number_of_scanned_image;
		for (long i = 0; i < number_of_scanned_image; i++) {
			m_scanned_image_ctrl->increment();
		}
	}
}

class CScanSequenceThread : public IStartSequenceThread
{
public:
	CScanSequenceThread(IUnknown *pscanner, IUnknown *option, IScannedImageCtrl *psic, IMessageQueue *q, IUnknown *handle);
	virtual ~CScanSequenceThread();
	void proc();
	long abort();
	long stop();
	long STDMETHODCALLTYPE QueryInterface(REFIID id, void **ppOut);
	unsigned long STDMETHODCALLTYPE AddRef();
	unsigned long STDMETHODCALLTYPE Release();	
private:
	IMessageQueue *m_pnext;
	XInterface<IScanSequence>m_pseq;
	XInterface<IScanSequenceCallback2> m_stop;
	IScannedImageCtrl *m_psic;
	long m_ref;	
};
CScanSequenceThread::CScanSequenceThread(IUnknown *pscanner, IUnknown *option, IScannedImageCtrl *psic, IMessageQueue *q, IUnknown *handle):
m_pnext(q),
m_psic(psic),
m_ref(1)
{
	//WriteLog((char*)"CScanSequenceThread::CScanSequenceThread(IScannerConnector)");
	if (option) {
		//csd
		m_pseq.reset(scan_sequence(m_pnext, (IUnknown*)pscanner, handle, option, (IUnknown*)psic));
		if (m_pseq.get()==NULL) SDKWriteLog((char*)"m_seq.get() is NULL");
	} else {
		//vs
		m_pseq.reset(scan_sequence(m_pnext,(IUnknown*)pscanner, handle, NULL, NULL));
		if (m_pseq.get()==NULL) SDKWriteLog((char*)"m_seq.get() is NULL");
	}
    m_stop.reset(scan_sequence_callback(m_psic));
}
CScanSequenceThread::~CScanSequenceThread()
{
	//WriteLog((char*)"CScanSequenceThread::~CScanSequenceThread()");
}
long CScanSequenceThread::QueryInterface(REFIID id, void **ppOut)
{
	return -1;
}
unsigned long CScanSequenceThread::AddRef()
{
	m_ref++;
	return m_ref;
}
unsigned long CScanSequenceThread::Release()
{
	m_ref--;
	if (m_ref<=0) {
		delete this;
		return 0;
	}
	return m_ref;
}
long CScanSequenceThread::abort()
{
	if (m_stop.get()) m_stop->stop_request(true);
	return 0;
}
long CScanSequenceThread::stop()
{
	if (m_stop.get()) m_stop->stop_request(true);
	return 0;
}
void CScanSequenceThread::proc()
{
	WriteLog_setname("Scan");
	//WriteLog((char*)"CScanSequenceThread::proc() start");
	if (m_pnext) {
		m_pnext->push(create_message(ICeiMessage::MID_BATCH_START, (void*)0));
		if (m_pseq.get()) {
			m_pseq->main(m_stop.get());
		} else {
			SDKWriteLog((char*)"NULL error L:%d F:%s", __LINE__, __FILE__);
		}
		m_pnext->push(create_message(ICeiMessage::MID_BATCH_END, (void*)0));
	} else {
		SDKWriteLog((char*)"NULL error L:%d F:%s", __LINE__, __FILE__);
	}
	//WriteLog((char*)"CScanSequenceThread::proc() end");
	WriteLog_setname(NULL);
}
/*
in case of vs
pscanner is IScannerConnector
option is not used
in case of csd
pscanner is IVirtualScanner
option is ICsdTags
*/
IStartSequenceThread *scan_sequence_thread(IUnknown *pscanner, IUnknown *option, IScannedImageCtrl *sic, IMessageQueue *q, IUnknown *handle)
{
	return (IStartSequenceThread*)new CScanSequenceThread(pscanner, option, sic ,q, handle);
}
IScanSequenceCallback2 *scan_sequence_callback(IUnknown *opt)
{
	return (IScanSequenceCallback2*)new CCallback((IScannedImageCtrl *)opt);
}
