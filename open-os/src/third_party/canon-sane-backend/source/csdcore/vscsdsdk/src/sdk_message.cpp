/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <cstddef>
#include "ceilogwrite.h"
#include "command.h"
#include "message_interface.h"

namespace {
	#if 0
	char *type2str(ICeiMessage::MESSAGE_TYPE type)
	{
		switch (type) {
		case ICeiMessage::MID_BATCH_START:return (char*)"MID_BATCH_START";
			case ICeiMessage::MID_ERROR:return (char*)"MID_ERROR";
			case ICeiMessage::MID_PAGE_START:return (char*)"MID_PAGE_START";
			case ICeiMessage::MID_IMAGE_START:return (char*)"MID_IMAGE_START";
			case ICeiMessage::MID_IMAGE:return (char*)"MID_IMAGE";
			case ICeiMessage::MID_IMAGE_END:return (char*)"MID_IMAGE_END";
			case ICeiMessage::MID_INFO_START:return (char*)"MID_INFO_START";
			case ICeiMessage::MID_INFO:return (char*)"MID_INFO";
			case ICeiMessage::MID_INFO_END:return (char*)"MID_INFO_END";
			case ICeiMessage::MID_PAGE_END:return (char*)"MID_PAGE_END";
			case ICeiMessage::MID_BATCH_END:return (char*)"MID_BATCH_END";
			default:break;
		}
		return (char*)"Unknown messag type";
	}
	#endif
}

class CCeiMessage : public ICeiMessage
{
public:
	CCeiMessage(ICeiMessage::MESSAGE_TYPE type, void *v);
	CCeiMessage(ICeiMessage::MESSAGE_TYPE type, long long  v);
	CCeiMessage(ICeiMessage::MESSAGE_TYPE type, CStreamCmd *v);
	CCeiMessage(ICeiMessage::MESSAGE_TYPE type, CSenseCmd *v);
	CCeiMessage(ICeiMessage::MESSAGE_TYPE type, IUnknown *v);
	virtual ~CCeiMessage();
	long type();
	long get(void **ppout, bool brelease);
	long STDMETHODCALLTYPE QueryInterface(REFIID id, void **ppOut);
	unsigned long STDMETHODCALLTYPE AddRef();
	unsigned long STDMETHODCALLTYPE Release();
private:
	ICeiMessage::MESSAGE_TYPE m_type;
	void *m_v;
	enum {
		VTYPE_VOID=0,
		VTYPE_LONG,
		VTYPE_STREAM,
		VTYPE_SENSE,
		VTYPE_UNKNOWN
	}m_vtype;
	long m_ref;
	void clear_void();
	void clear_long();
	void clear_stream();
	void clear_sense();
	void clear_unknown();
	long detach(void **ppout);
};
CCeiMessage::CCeiMessage(ICeiMessage::MESSAGE_TYPE type, void *v):m_type(type), m_v(v), m_vtype(VTYPE_VOID), m_ref(1)
{
	//WriteLog((char*)"CCeiMessage::CCeiMessage(0x%x:%s, void *v)", this, type2str(type));
}
CCeiMessage::CCeiMessage(ICeiMessage::MESSAGE_TYPE type, long long v):m_type(type), m_v((void*)v), m_vtype(VTYPE_LONG), m_ref(1)
{
	//WriteLog((char*)"CCeiMessage::CCeiMessage(0x%x:%s, long v:%ld)", this, type2str(type), v);
}
CCeiMessage::CCeiMessage(ICeiMessage::MESSAGE_TYPE type, CStreamCmd *v):m_type(type), m_v(v), m_vtype(VTYPE_STREAM), m_ref(1)
{
	//WriteLog((char*)"CCeiMessage::CCeiMessage(0x%x:%s, CStreamCmd *v)", this, type2str(type));
}
CCeiMessage::CCeiMessage(ICeiMessage::MESSAGE_TYPE type, CSenseCmd *v):m_type(type), m_v(v), m_vtype(VTYPE_SENSE), m_ref(1)
{
	//WriteLog((char*)"CCeiMessage::CCeiMessage(0x%x:%s, CSenseCmd *v)", this, type2str(type));
}
CCeiMessage::CCeiMessage(ICeiMessage::MESSAGE_TYPE type, IUnknown *v):m_type(type), m_v(v), m_vtype(VTYPE_UNKNOWN), m_ref(1)
{
	//WriteLog((char*)"CCeiMessage::CCeiMessage(0x%x:%s, IUnknown *v)", this, type2str(type));
}
CCeiMessage::~CCeiMessage()
{
	//WriteLog((char*)"CCeiMessage::~CCeiMessage(0x%x:%s)", this, type2str((ICeiMessage::MESSAGE_TYPE)type()));
	switch (m_vtype) {
	case VTYPE_VOID:
	clear_void();
	break;
	case VTYPE_LONG:
	clear_long();
	break;	
	case VTYPE_STREAM:
	clear_stream();
	break;
	case VTYPE_SENSE:
	clear_sense();
	break;
	case VTYPE_UNKNOWN:
	clear_unknown();
	break;
	}
}
void CCeiMessage::clear_void()
{
	switch (type()) {
	case ICeiMessage::MID_IMAGE:{
		if (m_v) {
			//WriteLog((char*)"ICeiMessage::MID_IMAGE:delete []p");
			CStreamCmd *p = (CStreamCmd*)m_v;
			delete p;
			m_v=NULL;
		}
	}break;
	case ICeiMessage::MID_INFO:{
		if (m_v) {
			//WriteLog((char*)"ICeiMessage::MID_INFO:delete []p");
			CStreamCmd *p = (CStreamCmd*)m_v;
			delete p;
			m_v=NULL;
		}
	}break;
	case ICeiMessage::MID_ERROR:{
		if (m_v) {
			//WriteLog((char*)"ICeiMessage::MID_ERROR:delete []p");
			CSenseCmd *p = (CSenseCmd*)m_v;
			delete p;
			m_v=NULL;
		}

	}break;
	default:break;
	}
}
void CCeiMessage::clear_long()
{
}
void CCeiMessage::clear_stream()
{
	if (m_v) {
		CStreamCmd *p = (CStreamCmd*)m_v;
		delete p;
	}
}
void CCeiMessage::clear_sense()
{
	if (m_v) {
		CSenseCmd *p = (CSenseCmd*)m_v;
		delete p;	
	}
}
void CCeiMessage::clear_unknown()
{
	if (m_v) {
		IUnknown *p = (IUnknown*)m_v;
		p->Release();
	}
}
long CCeiMessage::type()
{
	return m_type;
}
long CCeiMessage::get(void **ppout, bool brelease)
{
	if (ppout==NULL) return -1;

	switch (m_vtype) {
	case VTYPE_VOID:
	{
		*ppout = m_v;
	}
		break;
	case VTYPE_LONG:
	{
		long *pout = (long *)ppout;
		*pout = (long)m_v;
	}
		break;
	case VTYPE_STREAM:
	{
		CStreamCmd **ppstream = (CStreamCmd**)ppout;
		*ppstream = (CStreamCmd*)m_v;
	}
		break;
	case VTYPE_SENSE:
	{
		CSenseCmd **ppsns = (CSenseCmd**)ppout;
		*ppsns = (CSenseCmd*)m_v;
	}
		break;
	case VTYPE_UNKNOWN:
		*ppout = m_v;
		break;
	}


	
	if (brelease) m_v=NULL;
	return 0;
}
long CCeiMessage::detach(void **ppout)
{
	return get(ppout, true);
}
unsigned long CCeiMessage::AddRef()
{
	m_ref++;
	return m_ref;
}
unsigned long CCeiMessage::Release()
{
	m_ref--;
	if (m_ref<=0) {
		delete this;
		return 0;
	}
	return m_ref;
}
long CCeiMessage::QueryInterface(REFIID id, void **ppOut)
{
	return -1;
}
ICeiMessage *create_message(ICeiMessage::MESSAGE_TYPE type, void *v)
{
	return new CCeiMessage(type, v);
}
ICeiMessage *create_message(ICeiMessage::MESSAGE_TYPE type, long long v)
{
	return new CCeiMessage(type, v);
}
ICeiMessage *create_message(ICeiMessage::MESSAGE_TYPE type, CStreamCmd *v)
{
	return new CCeiMessage(type, v);
}
ICeiMessage *create_message(ICeiMessage::MESSAGE_TYPE type, CSenseCmd *v)
{
	return new CCeiMessage(type, v);
}
ICeiMessage *create_message(ICeiMessage::MESSAGE_TYPE type, IUnknown *v)
{
	return new CCeiMessage(type, v);
}