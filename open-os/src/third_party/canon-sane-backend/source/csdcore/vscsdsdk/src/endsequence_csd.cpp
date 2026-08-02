/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <vector>
#include <memory>
#include <string.h>
#include "ceilogwrite.h"
#include "csderr.h"
#include "message_queue_interface.h"
#include "image_interface.h"
#include "image_info_interface.h"
#include "message_queue_interface.h"
#include "end_sequence_interface.h"

class CCsdSDKEndSequence : public IEndSequence
{
public:
	CCsdSDKEndSequence(IMessageQueue *q);
	virtual ~CCsdSDKEndSequence();
	void on_batch_start(ICeiMessage *pmsg);
	void on_page_start(ICeiMessage *pmsg);
	void on_image_start(ICeiMessage *pmsg);
	void on_image(ICeiMessage *pmsg);
	void on_image_end(ICeiMessage *pmsg);
	void on_info_start(ICeiMessage *pmsg);
	void on_info(ICeiMessage *pmsg);
	void on_info_end(ICeiMessage *pmsg);
	void on_page_end(ICeiMessage *pmsg);
	void on_error(ICeiMessage *pmsg);
	void on_batch_end(ICeiMessage *pmsg);
	long get_image(ICeiImage **ppOut);
	long get_information(long id, void *ppOut);
	long STDMETHODCALLTYPE QueryInterface(REFIID id, void **ppOut);
	unsigned long STDMETHODCALLTYPE AddRef();
	unsigned long STDMETHODCALLTYPE Release();	
private:
	long m_ref;
	IMessageQueue *m_pprev;
	XInterface<ICeiImage>m_image;
	XInterface<ICeiImageInformation>m_info;
	long m_csd_error;
};
CCsdSDKEndSequence::CCsdSDKEndSequence(IMessageQueue *q):m_ref(1), m_pprev(q), m_csd_error(CSD3_OK)
{
}
CCsdSDKEndSequence::~CCsdSDKEndSequence()
{
}
long CCsdSDKEndSequence::QueryInterface(REFIID id, void **ppOut)
{
	return -1;
}
unsigned long CCsdSDKEndSequence::AddRef()
{
	m_ref++;
	return m_ref;
}
unsigned long CCsdSDKEndSequence::Release()
{
	m_ref--;
	if (m_ref<=0) {
		delete this;
		return 0;
	}
	return m_ref;
}
void CCsdSDKEndSequence::on_batch_start(ICeiMessage *pmsg)
{
	//WriteLog((char*)"CCsdSDKEndSequence::on_batch_start()");
}
void CCsdSDKEndSequence::on_page_start(ICeiMessage *pmsg)
{
	//WriteLog((char*)"CCsdSDKEndSequence::on_page_start()");
}
void CCsdSDKEndSequence::on_image_start(ICeiMessage *pmsg)
{
	//WriteLog((char*)"CCsdSDKEndSequence::on_image_start()");
}
void CCsdSDKEndSequence::on_image(ICeiMessage *pmsg)
{
	//WriteLog((char*)"CCsdSDKEndSequence::on_image()");
	ICeiImage *p=NULL;
	pmsg->get((void**)&p, true);
	if (p) {
		m_image = p;
	}
}
void CCsdSDKEndSequence::on_image_end(ICeiMessage *pmsg)
{
	//WriteLog((char*)"CCsdSDKEndSequence::on_image_end()");
}
void CCsdSDKEndSequence::on_info_start(ICeiMessage *pmsg)
{
	//WriteLog((char*)"CCsdSDKEndSequence::on_info_start()");
}
void CCsdSDKEndSequence::on_info(ICeiMessage *pmsg)
{
	//WriteLog((char*)"CCsdSDKEndSequence::on_info()");
	ICeiImageInformation *p=NULL;
	pmsg->get((void**)&p, true);
	if (p) {
		m_info = p;
	}
}
void CCsdSDKEndSequence::on_info_end(ICeiMessage *pmsg)
{
	//WriteLog((char*)"CCsdSDKEndSequence::on_info_end()");
}
void CCsdSDKEndSequence::on_page_end(ICeiMessage *pmsg)
{
	//WriteLog((char*)"CCsdSDKEndSequence::on_page_end()");
}
void CCsdSDKEndSequence::on_error(ICeiMessage *pmsg)
{
	//WriteLog((char*)"CCsdSDKEndSequence::on_error()");
	long error=0;
	pmsg->get((void**)&error);
	if (error) {
		m_csd_error = error;
		//SDKWriteLog((char*)"m_csd_error %d", m_csd_error);
	}
}
void CCsdSDKEndSequence::on_batch_end(ICeiMessage *pmsg)
{
	//WriteLog((char*)"CCsdSDKEndSequence::on_batch_end()");
}
long CCsdSDKEndSequence::get_image(ICeiImage **ppOut)
{
	//WriteLog((char*)"CCsdSDKEndSequence::get_image() start");
	long ret = m_csd_error;
	if (m_csd_error) {
		//SDKWriteLog((char*)"m_csd_error is %d", m_csd_error);
		ret = m_csd_error;
	} else if (m_image.get()) {
		SDKWriteLog("image ptr:0x%lx", m_image->img());
		SDKWriteLog("width:%ld", m_image->width());
		SDKWriteLog("height:%ld", m_image->height());
		SDKWriteLog("xdpi:%ld", m_image->xdpi());
		SDKWriteLog("ydpi:%ld", m_image->ydpi());
		SDKWriteLog("spp:%ld", m_image->spp());
		SDKWriteLog("bps:%ld", m_image->bps());
		SDKWriteLog("sync:%ld", m_image->sync());
		SDKWriteLog("size:%ld", m_image->size());
		SDKWriteLog("comptype:%ld", m_image->comptype());
		SDKWriteLog("compinfo:%ld", m_image->compinfo());
		*ppOut = m_image.Detach();
	} else {
		SDKWriteLog((char*)"m_document.get() is NULL");
		ret = CSD3_NOPAGE;
	}
	//WriteLog((char*)"CCsdSDKEndSequence::get_image() end %d", ret);
	return ret;
}
long CCsdSDKEndSequence::get_information(long id, void *ppOut)
{
	return m_info->information(id, ppOut);
}
IEndSequence *end_sequence(IMessageQueue *q, IUnknown *)
{
	return new CCsdSDKEndSequence(q);
}