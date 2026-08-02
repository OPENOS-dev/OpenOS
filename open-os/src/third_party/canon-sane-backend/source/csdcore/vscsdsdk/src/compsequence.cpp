/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <vector>
#include <memory>
#include <string.h>
#include "ceilogwrite.h"
#include "csdtags.h"
#include "sdk_message.h"
#include "sdk_image_util.h"
#include "ipsdk.h"
#include "sense2vs_error.h"
#include "message_queue_interface.h"
#include "mid_sequence_interface.h"
#include "tags_interface.h"

class CCompSequence : public IMidSequence
{
public:
	CCompSequence(IMessageQueue *prevq, IMessageQueue *nextq, ICsdTags* tags);
	virtual ~CCompSequence();
	long STDMETHODCALLTYPE QueryInterface(REFIID id, void **ppOut);
	unsigned long STDMETHODCALLTYPE AddRef();
	unsigned long STDMETHODCALLTYPE Release();

	void proc();
private:
	long m_ref;
	IMessageQueue *m_pprev;
	IMessageQueue *m_pnext;
	ICsdTags* m_ptags;
	XInterface<ICeiJpeg>m_comp;
	long m_error;
};
CCompSequence::CCompSequence(IMessageQueue *q1, IMessageQueue *q2, ICsdTags* ptags):m_ref(1), m_pprev(q1), m_pnext(q2), m_ptags(ptags), m_error(0)
{
	long quality = 90;
	ptags->get(CSDP_JPEGQUALITY, &quality);
	//WriteLog((char*)"CSDP_JPEGQUALITY:%ld", quality);
	m_comp.reset(jpeg_comp(quality));
}
CCompSequence::~CCompSequence()
{
}
long CCompSequence::QueryInterface(REFIID id, void **ppOut)
{
	return -1;
}
unsigned long CCompSequence::AddRef()
{
	m_ref++;
	return m_ref;
}
unsigned long CCompSequence::Release()
{
	m_ref--;
	if (m_ref<=0) {
		delete this;
		return 0;
	}
	return m_ref;
}
void CCompSequence::proc()
{
	SDKWriteLog((char*)"CCompSequence::proc() start");
	ICeiMessage *pmsg=NULL;
	bool bloop=true;
	while (bloop) {
		m_pprev->pop(&pmsg);
		switch (pmsg->type()) {
		case ICeiMessage::MID_BATCH_START:
		SDKWriteLog((char*)"[COMP]ICeiMessage::MID_BATCH_START");
		m_pnext->push(pmsg);
		break;
		case ICeiMessage::MID_PAGE_START:
		SDKWriteLog((char*)"[COMP]ICeiMessage::MID_PAGE_START");
		m_pnext->push(pmsg);
		break;
		case ICeiMessage::MID_IMAGE_START:
		SDKWriteLog((char*)"[COMP]ICeiMessage::MID_IMAGE_START");
		m_pnext->push(pmsg);
		break;
		case ICeiMessage::MID_IMAGE:{
			SDKWriteLog((char*)"[COMP]ICeiMessage::MID_IMAGE");
			ICeiImage *pimg=NULL;
			pmsg->get((void**)&pimg, true);
			pmsg->Release();
			pmsg=NULL;
			if (pimg&&pimg->comptype()==0&&pimg->bps()>1) {
				WriteLog((char*)"compressing...");
				m_comp->run(&pimg);
				WriteLog((char*)"done:quality %ld", pimg->compinfo());
			}
			m_pnext->push(create_message(ICeiMessage::MID_IMAGE, (IUnknown *)pimg));
		}
		break;
		case ICeiMessage::MID_IMAGE_END:
		SDKWriteLog((char*)"[COMP]ICeiMessage::MID_IMAGE_END");
		m_pnext->push(pmsg);
		break;
		case ICeiMessage::MID_INFO_START:
		SDKWriteLog((char*)"[COMP]ICeiMessage::MID_INFO_START");
		m_pnext->push(pmsg);
		break;
		case ICeiMessage::MID_INFO:
		SDKWriteLog((char*)"[COMP]ICeiMessage::MID_INFO");
		m_pnext->push(pmsg);
		break;
		case ICeiMessage::MID_INFO_END:
		SDKWriteLog((char*)"[COMP]ICeiMessage::MID_INFO_END");
		m_pnext->push(pmsg);
		break;
		case ICeiMessage::MID_PAGE_END:
		SDKWriteLog((char*)"[COMP]ICeiMessage::MID_PAGE_END");
		m_pnext->push(pmsg);
		break;
		case ICeiMessage::MID_BATCH_END:
		SDKWriteLog((char*)"[COMP]ICeiMessage::MID_BATCH_END");
		m_pnext->push(pmsg);
		bloop=false;
		break;
		default:
		case ICeiMessage::MID_ERROR:
		SDKWriteLog((char*)"[COMP]ICeiMessage::MID_ERROR");
		m_pnext->push(pmsg);
		break;
		}
		pmsg=NULL;
	}
	SDKWriteLog((char*)"CCompSequence::proc() end");
}
IMidSequence *comp_sequence(IMessageQueue *prevq, IMessageQueue *nextq, IUnknown *tags)
{
	return new CCompSequence(prevq, nextq, (ICsdTags*)tags);
}







class CBackupSequence : public IMidSequence
{
public:
	CBackupSequence(IMessageQueue* prevq, IMessageQueue* nextq, ICsdTags* tags);
	virtual ~CBackupSequence();
	long STDMETHODCALLTYPE QueryInterface(REFIID id, void** ppOut);
	unsigned long STDMETHODCALLTYPE AddRef();
	unsigned long STDMETHODCALLTYPE Release();

	void proc();
private:
	long m_ref;
	IMessageQueue* m_pprev;
	IMessageQueue* m_pnext;
	long m_error;
	typedef std::vector<ICeiMessage*> MSGLIST;
	MSGLIST m_msgs;
};
CBackupSequence::CBackupSequence(IMessageQueue* q1, IMessageQueue* q2, ICsdTags* ptags) :m_ref(1), m_pprev(q1), m_pnext(q2), m_error(0)
{
}
CBackupSequence::~CBackupSequence()
{
	MSGLIST::iterator itr = m_msgs.begin();
	for (; itr != m_msgs.end(); itr++) {
		if ((*itr)->Release() && (*itr)->type()!=ICeiMessage::MID_ERROR) {
			SDKWriteLog("Leak?:%d %s", __LINE__, __FILE__);
		}
	}
	m_msgs.clear();
}
long CBackupSequence::QueryInterface(REFIID id, void** ppOut)
{
	return -1;
}
unsigned long CBackupSequence::AddRef()
{
	m_ref++;
	return m_ref;
}
unsigned long CBackupSequence::Release()
{
	m_ref--;
	if (m_ref <= 0) {
		delete this;
		return 0;
	}
	return m_ref;
}
void CBackupSequence::proc()
{
	SDKWriteLog((char*)"CBackupSequence::proc() start");
	ICeiMessage* pmsg = NULL;
	bool bloop = true;
	while (bloop) {
		m_pprev->pop(&pmsg);
		switch (pmsg->type()) {
		case ICeiMessage::MID_BATCH_START:
			SDKWriteLog((char*)"[BACKUP]ICeiMessage::MID_BATCH_START");
			m_pnext->push(pmsg);
			break;
		case ICeiMessage::MID_BATCH_END:
			SDKWriteLog((char*)"[BACKUP]ICeiMessage::MID_BATCH_END");

			{
				MSGLIST::iterator itr = m_msgs.begin();
				for (; itr != m_msgs.end(); itr++) 
				{
					if ((*itr)->type() == ICeiMessage::MID_IMAGE) {
						ICeiImage* pimg = NULL;
						(*itr)->get((void**)&pimg, false);
						m_pnext->push(create_message(ICeiMessage::MID_IMAGE, (IUnknown*)clone_vscsdsdk_image(pimg)));
					}
					else if ((*itr)->type() == ICeiMessage::MID_INFO) {
						ICeiImageInformation* pinfo = NULL;
						(*itr)->get((void**)&pinfo, false);
						pinfo->AddRef();//same as clone
						m_pnext->push(create_message(ICeiMessage::MID_INFO, (IUnknown*)pinfo));
					} else {
						(*itr)->AddRef();
						m_pnext->push(*itr);
					}
				}
			}

			m_pnext->push(pmsg);
			bloop = false;
			break;
		default:
			m_msgs.push_back(pmsg);
			break;
		}
		pmsg = NULL;
	}
	SDKWriteLog((char*)"CBackupSequence::proc() end");
}
IMidSequence* backup_sequence(IMessageQueue* prevq, IMessageQueue* nextq, IUnknown* tags)
{
	return new CBackupSequence(prevq, nextq, (ICsdTags*)tags);
}