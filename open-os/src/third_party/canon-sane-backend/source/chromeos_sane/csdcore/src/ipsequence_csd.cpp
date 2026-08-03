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
#include "sdk_information_util.h"
#include "sdk_tag.h"
#include "sense2vs_error.h"
#include "message_queue_interface.h"
#include "mid_sequence_interface.h"
#include "settings_csd.h"
#include "global_apis.h"
#include "sdk_def.h"
#include "ipsdk.h"
#include "csdtags.org.h"
#include "csderr.h"
namespace ipseq{
    template<class T>
    void setlong(T* ptags, int tagid, long v)
    {
        ptags->set(tagid, v);
    }
    template<class T>
    long getlong(T* ptags, int tagid, long def = 0)
    {
        long v = def;
        ptags->get(tagid, &v);
        return v;
    }
}
class CIPSequenceCsd : public IMidSequence
{
public:
    CIPSequenceCsd(IMessageQueue* prevq, IMessageQueue* nextq, CSettingsCsd* p);
    virtual ~CIPSequenceCsd();
    void proc();
    long STDMETHODCALLTYPE QueryInterface(REFIID id, void** ppOut);
    unsigned long STDMETHODCALLTYPE AddRef();
    unsigned long STDMETHODCALLTYPE Release();
private:
    void error_check();
    void on_page_start(ICeiMessage* pmsg);
    void on_image_start(ICeiMessage* pmsg);
    void on_image(ICeiMessage* pmsg);
    void on_image_end(ICeiMessage* pmsg);
    void on_info_start(ICeiMessage* pmsg);
    void on_info(ICeiMessage* pmsg);
    void on_info_end(ICeiMessage* pmsg);
    void on_page_end(ICeiMessage* pmsg);
    void on_page_end1(long bfront);
private:
    void image_process(ICeiImage** ppInOut);
private:
    long getlong(int id, long def = 0);
    void setlong(int id, long v);
private:
    enum {
        FRONT = 0,
        BACK,
        FRONT_BACK
    };
    long                            m_ref;
    IMessageQueue*                  m_pprev;
    IMessageQueue*                  m_pnext;
    CSettingsCsd*                   m_psettings;
    ICsdTags    *                   m_ptags;
    long                            m_index;
	XInterface<ICeiImage>           m_image[FRONT_BACK];
    XInterface<ICeiImageInformation>m_info[FRONT_BACK];
};
CIPSequenceCsd::CIPSequenceCsd(IMessageQueue *q1, IMessageQueue *q2, CSettingsCsd *p):
m_ref(1), 
m_pprev(q1),
m_pnext(q2), 
m_psettings(p),
m_index(0)
{
	m_ptags=m_psettings->tags();
}
CIPSequenceCsd::~CIPSequenceCsd()
{
}
long CIPSequenceCsd::QueryInterface(REFIID id, void **ppOut)
{
	return -1;
}
unsigned long CIPSequenceCsd::AddRef()
{
	m_ref++;
	return m_ref;
}
unsigned long CIPSequenceCsd::Release()
{
	m_ref--;
	if (m_ref<=0) {
		delete this;
		return 0;
	}
	return m_ref;
}
void CIPSequenceCsd::error_check()
{
	if (m_pprev==NULL) WriteLog((char*)"m_pprev is NULL");
	if (m_pnext==NULL) WriteLog((char*)"m_pnext is NULL");
}
long CIPSequenceCsd::getlong(int id, long def)
{
	return ipseq::getlong(m_ptags, id, def);
}
void CIPSequenceCsd::setlong(int id, long v)
{
    ipseq::setlong(m_ptags, id, v);
}
void CIPSequenceCsd::on_page_start(ICeiMessage *pmsg)
{
	pmsg->Release();    
    m_image[FRONT] = NULL;
    m_image[BACK] = NULL;
    m_info[FRONT] = NULL;
    m_info[BACK] = 0;
}
void CIPSequenceCsd::on_image_start(ICeiMessage *pmsg)
{
    long bfront = 0;
    pmsg->get((void**)&bfront);
	pmsg->Release();	
    m_index = bfront ? 0 : 1;
}
void CIPSequenceCsd::on_image(ICeiMessage *pmsg)
{
	pmsg->get((void**)&m_image[m_index], true);
	pmsg->Release();
}
void CIPSequenceCsd::on_image_end(ICeiMessage *pmsg)
{
    pmsg->Release();	
}
void CIPSequenceCsd::on_info_start(ICeiMessage *pmsg)
{
    long bfront = 0;
    pmsg->get((void**)&bfront);
    pmsg->Release();
    m_index = bfront ? 0 : 1;
}
void CIPSequenceCsd::on_info(ICeiMessage *pmsg)
{
	pmsg->get((void**)&m_info[m_index], true);
	pmsg->Release();
}
void CIPSequenceCsd::on_info_end(ICeiMessage *pmsg)
{
	pmsg->Release();	
}
void CIPSequenceCsd::image_process(ICeiImage** ppInOut)
{

}
void CIPSequenceCsd::on_page_end(ICeiMessage* pmsg)
{
    if (pmsg) {
        pmsg->Release();
    }
    pmsg = NULL;
    try {
        if (getlong(CSDP_FEEDER)) {
            on_page_end1(1/*front*/);
            on_page_end1(0/*back*/);
        }
        else {
            if (getlong(CSDP_FEEDER_OPTION)) {
                on_page_end1(0/*back*/);
            }
            else {
                on_page_end1(1/*front*/);
            }
        }
    }
    catch (std::bad_alloc&) {
        WriteLog("std::bad_alloc is thrown");
        m_pnext->push(create_message(ICeiMessage::MID_ERROR, CSD3_NOMEM));
    }
}
void CIPSequenceCsd::on_page_end1(long bfront)
{
	WriteLog((char*)"on_page_end1(%s) start", bfront?"front":"back");
    ICeiImage* pimg = m_image[bfront ? 0 : 1].Detach();
    if (pimg == NULL) {
        WriteLog("m_image[%s] is NULL", bfront ? "front" : "back");
        m_pnext->push(create_message(ICeiMessage::MID_ERROR, (long long)CSD3_NOMEM));
        return;
    }
    image_process(&pimg);
    m_pnext->push(create_message(ICeiMessage::MID_PAGE_START, bfront));
    m_pnext->push(create_message(ICeiMessage::MID_IMAGE_START, bfront));
    m_pnext->push(create_message(ICeiMessage::MID_IMAGE, (IUnknown*)pimg));
    m_pnext->push(create_message(ICeiMessage::MID_IMAGE_END, bfront));
    m_pnext->push(create_message(ICeiMessage::MID_INFO_START, bfront));
    {
        CCeiImageInformationTag* pinfo = create_vscsdsdk_information_tag();
        pinfo->set(create_longtag(CSDP_LASTPAGE_SIDE, bfront ? 0 : 1));
        m_pnext->push(create_message(ICeiMessage::MID_INFO, (IUnknown*)pinfo));
    }
    m_pnext->push(create_message(ICeiMessage::MID_INFO_END, bfront));
    m_pnext->push(create_message(ICeiMessage::MID_PAGE_END, bfront));
    WriteLog("on_page_end1() end");
}
void CIPSequenceCsd::proc()
{
	WriteLog((char*)"proc() start");
	ICeiMessage *pmsg=NULL;
	bool bloop=true;
	while (bloop) {
		m_pprev->pop(&pmsg);
		switch (pmsg->type()) {
		case ICeiMessage::MID_BATCH_START:
		WriteLog((char*)"MID_BATCH_START");
		m_pnext->push(pmsg);
		break;
		case ICeiMessage::MID_ERROR:
		WriteLog((char*)"MID_ERROR");
		m_pnext->push(pmsg);
		break;
		case ICeiMessage::MID_PAGE_START:
		WriteLog((char*)"MID_PAGE_START");
		on_page_start(pmsg);
		break;
		case ICeiMessage::MID_IMAGE_START:
		WriteLog((char*)"MID_IMAGE_START");
		on_image_start(pmsg);
		break;
		case ICeiMessage::MID_IMAGE:
		WriteLog((char*)"MID_IMAGE");
		on_image(pmsg);
		break;
		case ICeiMessage::MID_IMAGE_END:
		WriteLog((char*)"MID_IMAGE_END");
		on_image_end(pmsg);
		break;
		case ICeiMessage::MID_INFO_START:
		WriteLog((char*)"MID_INFO_START");
		on_info_start(pmsg);
		break;
		case ICeiMessage::MID_INFO:
		WriteLog((char*)"MID_INFO");
		on_info(pmsg);
		break;
		case ICeiMessage::MID_INFO_END:
		WriteLog((char*)"MID_INFO_END");
		on_info_end(pmsg);
		break;
		case ICeiMessage::MID_PAGE_END:
		WriteLog((char*)"MID_PAGE_END");
		on_page_end(pmsg);
		break;
		default:		
		case ICeiMessage::MID_BATCH_END:
		WriteLog((char*)"MID_BATCH_END");
		bloop=false;
		m_pnext->push(pmsg);
		break;
		}
		pmsg=NULL;
	}	
	WriteLog((char*)"proc() end");
}
IMidSequence *ip_sequence(IMessageQueue *prevq, IMessageQueue *nextq, IUnknown *handle)
{
	return new CIPSequenceCsd(prevq, nextq, (CSettingsCsd *)handle);
}
