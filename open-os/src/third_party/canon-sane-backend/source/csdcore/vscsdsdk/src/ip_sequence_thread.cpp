/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <cstddef>
#include "ceilogwrite.h"
#include "sense2vs_error.h"
#include "message_queue_interface.h"
#include "sequence_thread_interface.h"
#include "mid_sequence_interface.h"

class CIPSequenceThread : public IMidSequenceThread
{
public:
	CIPSequenceThread(IMessageQueue *prevq, IMessageQueue *nextq, IUnknown *h);
	virtual ~CIPSequenceThread();
	long STDMETHODCALLTYPE QueryInterface(REFIID id, void **ppOut);
	unsigned long STDMETHODCALLTYPE AddRef();
	unsigned long STDMETHODCALLTYPE Release();
	void proc();
private:
	IMessageQueue *m_pprev;
	IMessageQueue *m_pnext;
	long m_ref;
	XInterface<IMidSequence>m_mid;
};
CIPSequenceThread::CIPSequenceThread(IMessageQueue *prevq, IMessageQueue *nextq, IUnknown *h):m_pprev(prevq), m_pnext(nextq), m_ref(1)
{
	//WriteLog((char*)"CIPSequenceThread::CIPSequenceThread()");
	m_mid.reset(ip_sequence(prevq, nextq, h));
}
CIPSequenceThread::~CIPSequenceThread()
{
	//WriteLog((char*)"CIPSequenceThread::~CIPSequenceThread()");
}
long CIPSequenceThread::QueryInterface(REFIID id, void **ppOut)
{
	return -1;
}
unsigned long CIPSequenceThread::AddRef(){
	m_ref++;
	return m_ref;
}
unsigned long CIPSequenceThread::Release(){
	m_ref--;
	if (m_ref<=0) {
		delete this;
		return 0;
	}
	return m_ref;
}	
void CIPSequenceThread::proc()
{
	//WriteLog((char*)"CIPSequenceThread::proc() start");
	WriteLog_setname("IP");
	m_mid->proc();
	WriteLog_setname(NULL);
	//WriteLog((char*)"CIPSequenceThread::proc() end");
}
IMidSequenceThread *ip_sequence_thread(IMessageQueue *prevq, IMessageQueue *nextq, IScannedImageCtrl *sic, IUnknown *handle)
{
	return (IMidSequenceThread *)new CIPSequenceThread(prevq, nextq, handle);
}