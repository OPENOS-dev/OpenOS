/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <cstddef>
#include "ceilogwrite.h"
#include "message_queue_interface.h"
#include "scanned_imagectrl_interface.h"
#include "sequence_thread_interface.h"
#include "end_sequence_interface.h"
#include "vserr.h"
class CEndSequenceThread : public IEndSequenceThread
{
public:
	CEndSequenceThread(IMessageQueue *q, IScannedImageCtrl *psic, IUnknown *h);
	virtual ~CEndSequenceThread();
	long get_image(ICeiImage **ppOut);
	long get_information(long id, void *pout);
	long STDMETHODCALLTYPE QueryInterface(REFIID id, void **ppOut);
	unsigned long STDMETHODCALLTYPE AddRef();
	unsigned long STDMETHODCALLTYPE Release();
private:
	void on_batch_start(ICeiMessage *pmsg);
	void on_page_start(ICeiMessage *pmsg);
	void on_image_start(ICeiMessage *pmsg);
	void on_image(ICeiMessage *pmsg);
	void on_image_end(ICeiMessage *pmsg);
	void on_info_start(ICeiMessage *pmsg);
	void on_info(ICeiMessage *pmsg);
	void on_info_end(ICeiMessage *pmsg);
	void on_page_end(ICeiMessage *pmsg);
	void on_batch_end(ICeiMessage *pmsg);
	void on_error(ICeiMessage *pmsg);
	void reload();
private:
	IMessageQueue *m_pprev;
	IScannedImageCtrl *m_psic;
	long m_ref;
	XInterface<IEndSequence>m_end;
};
CEndSequenceThread::CEndSequenceThread(IMessageQueue *q, IScannedImageCtrl *psic, IUnknown *h):
m_pprev(q), 
m_psic(psic),
m_ref(1)
{
	//WriteLog((char*)"CEndSequenceThread::CEndSequenceThread()");
	m_end.reset(end_sequence(q, h));
}
CEndSequenceThread::~CEndSequenceThread()
{
	//WriteLog((char*)"CEndSequenceThread::~CEndSequenceThread()");
}
long CEndSequenceThread::QueryInterface(REFIID id, void **ppOut)
{
	return -1;
}
unsigned long CEndSequenceThread::AddRef(){
	m_ref++;
	return m_ref;
}
unsigned long CEndSequenceThread::Release(){
	m_ref--;
	if (m_ref<=0) {
		delete this;
		return 0;
	}
	return m_ref;
}	
void CEndSequenceThread::on_batch_start(ICeiMessage *pmsg)
{
	m_end->on_batch_start(pmsg);
}
void CEndSequenceThread::on_page_start(ICeiMessage *pmsg)
{	
	m_end->on_page_start(pmsg);
}
void CEndSequenceThread::on_image_start(ICeiMessage *pmsg)
{	
	m_end->on_image_start(pmsg);
}
void CEndSequenceThread::on_image(ICeiMessage *pmsg)
{	
	m_end->on_image(pmsg);
}
void CEndSequenceThread::on_image_end(ICeiMessage *pmsg)
{	
	m_end->on_image_end(pmsg);
}
void CEndSequenceThread::on_info_start(ICeiMessage *pmsg)
{	
	m_end->on_info_start(pmsg);
}
void CEndSequenceThread::on_info(ICeiMessage *pmsg)
{	
	m_end->on_info(pmsg);
}
void CEndSequenceThread::on_info_end(ICeiMessage *pmsg)
{
	m_end->on_info_end(pmsg);	
}
void CEndSequenceThread::on_page_end(ICeiMessage *pmsg)
{
	m_end->on_page_end(pmsg);	
}
void CEndSequenceThread::on_batch_end(ICeiMessage *pmsg)
{
	m_end->on_batch_end(pmsg);
}
void CEndSequenceThread::on_error(ICeiMessage *pmsg)
{
	m_end->on_error(pmsg);
}
void CEndSequenceThread::reload()
{
	//WriteLog((char*)"CEndSequenceThread::reload() start");
	ICeiMessage *pmsg=NULL;
	bool bloop=true;
	while (bloop) {
		m_pprev->pop(&pmsg);
		switch (pmsg->type()) {
		case ICeiMessage::MID_BATCH_START:
		SDKWriteLog((char*)"pop:MID_BATCH_START");
		on_batch_start(pmsg);
		break;
		case ICeiMessage::MID_PAGE_START:
		SDKWriteLog((char*)"pop:MID_PAGE_START");
		on_page_start(pmsg);
		break;
		case ICeiMessage::MID_IMAGE_START:
		SDKWriteLog((char*)"pop:MID_IMAGE_START");
		on_image_start(pmsg);
		break;
		case ICeiMessage::MID_IMAGE:
		SDKWriteLog((char*)"pop:MID_IMAGE");
		on_image(pmsg);
		break;
		case ICeiMessage::MID_IMAGE_END:
		SDKWriteLog((char*)"pop:MID_IMAGE_END");
		on_image_end(pmsg);
		break;
		case ICeiMessage::MID_INFO_START:
		SDKWriteLog((char*)"pop:MID_INFO_START");
		on_info_start(pmsg);
		break;
		case ICeiMessage::MID_INFO:
		SDKWriteLog((char*)"pop:MID_INFO");
		on_info(pmsg);
		break;
		case ICeiMessage::MID_INFO_END:
		SDKWriteLog((char*)"pop:MID_INFO_END");
		on_info_end(pmsg);
		break;
		case ICeiMessage::MID_PAGE_END:
		SDKWriteLog((char*)"pop:MID_PAGE_END");
		on_page_end(pmsg);
		bloop=false;
		break;
		case ICeiMessage::MID_BATCH_END:
		SDKWriteLog((char*)"pop:MID_BATCH_END");
		on_batch_end(pmsg);
		bloop=false;
		break;
		case ICeiMessage::MID_ERROR:
		SDKWriteLog((char*)"pop:MID_ERROR");
		on_error(pmsg);
		bloop=false;
		break;
		}
		pmsg->Release();
		pmsg=NULL;
	}
	//WriteLog((char*)"CEndSequenceThread::reload() end");
}
long CEndSequenceThread::get_image(ICeiImage **ppOut)
{
	//WriteLog((char*)"CEndSequenceThread::get_image() start");
	if (m_end.get() == NULL) {
        *ppOut = NULL;
		SDKWriteLog((char*)"no memory L:%d F:%s", __LINE__, __FILE__);
		return VS3_ABORT;
	}
	if (m_pprev == NULL) {
        *ppOut = NULL;
		SDKWriteLog((char*)"no memory L:%d F:%s", __LINE__, __FILE__);
		return VS3_ABORT;
	}
	if (*ppOut==(ICeiImage *)12345 && m_pprev->count() == 0) {
        *ppOut = NULL;
		SDKWriteLog((char*)"no memory L:%d F:%s", __LINE__, __FILE__);
		return VS3_ABORT;
    }
    *ppOut = NULL;
	reload();
    long ret = 0;
    bool bloop=true;
    while (bloop) {
        ret = m_end->get_image(ppOut);
        bloop=false;
        if (ret==VS3_RELOAD_AGAIN) {
            reload();
            bloop=true;
        }
    }
    
	//WriteLog((char*)"CEndSequenceThread::get_image() end");
	return ret;
}
long CEndSequenceThread::get_information(long id, void *pout)
{
	//WriteLog((char*)"CEndSequenceThread::get_information() start");
	long ret = m_end->get_information(id, pout);;
	//WriteLog((char*)"CEndSequenceThread::get_information() end");
	return ret;	
}
IEndSequenceThread *end_sequence_thread(IMessageQueue *q, IScannedImageCtrl *sic, IUnknown *handle)
{
	return (IEndSequenceThread *)new CEndSequenceThread(q, sic, handle);
}
