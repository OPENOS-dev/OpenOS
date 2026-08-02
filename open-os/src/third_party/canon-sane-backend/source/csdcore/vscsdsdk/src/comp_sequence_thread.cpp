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

class CCompSequenceThread : public IMidSequenceThread
{
public:
	CCompSequenceThread(IMessageQueue *prevq, IMessageQueue *nextq, IUnknown *tags);
	virtual ~CCompSequenceThread();
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
CCompSequenceThread::CCompSequenceThread(IMessageQueue *prevq, IMessageQueue *nextq, IUnknown *tags):m_pprev(prevq), m_pnext(nextq), m_ref(1)
{
	//WriteLog((char*)"CCompSequenceThread::CCompSequenceThread()");
	m_mid.reset(comp_sequence(prevq, nextq, tags));
}
CCompSequenceThread::~CCompSequenceThread()
{
	//WriteLog((char*)"CCompSequenceThread::~CCompSequenceThread()");
}
long CCompSequenceThread::QueryInterface(REFIID id, void **ppOut)
{
	return -1;
}
unsigned long CCompSequenceThread::AddRef(){
	m_ref++;
	return m_ref;
}
unsigned long CCompSequenceThread::Release(){
	m_ref--;
	if (m_ref<=0) {
		delete this;
		return 0;
	}
	return m_ref;
}	
void CCompSequenceThread::proc()
{
	WriteLog_setname("CMP");
	//SDKWriteLog((char*)"CCompSequenceThread::proc() start");
	m_mid->proc();
	//SDKWriteLog((char*)"CCompSequenceThread::proc() end");
	WriteLog_setname(NULL);
}
IMidSequenceThread *comp_sequence_thread(IMessageQueue *prevq, IMessageQueue *nextq, IScannedImageCtrl *sic, IUnknown *tags)
{
	return (IMidSequenceThread *)new CCompSequenceThread(prevq, nextq, tags);
}





class CBackupSequenceThread : public IMidSequenceThread
{
public:
	CBackupSequenceThread(IMessageQueue* prevq, IMessageQueue* nextq, IUnknown* tags);
	virtual ~CBackupSequenceThread();
	long STDMETHODCALLTYPE QueryInterface(REFIID id, void** ppOut);
	unsigned long STDMETHODCALLTYPE AddRef();
	unsigned long STDMETHODCALLTYPE Release();
	void proc();
private:
	IMessageQueue* m_pprev;
	IMessageQueue* m_pnext;
	long m_ref;
	XInterface<IMidSequence>m_mid;
};
CBackupSequenceThread::CBackupSequenceThread(IMessageQueue* prevq, IMessageQueue* nextq, IUnknown* tags) :m_pprev(prevq), m_pnext(nextq), m_ref(1)
{
	//WriteLog((char*)"CBackupSequenceThread::CBackupSequenceThread()");
	m_mid.reset(backup_sequence(prevq, nextq, tags));
}
CBackupSequenceThread::~CBackupSequenceThread()
{
	//WriteLog((char*)"CBackupSequenceThread::~CBackupSequenceThread()");
}
long CBackupSequenceThread::QueryInterface(REFIID id, void** ppOut)
{
	return -1;
}
unsigned long CBackupSequenceThread::AddRef() {
	m_ref++;
	return m_ref;
}
unsigned long CBackupSequenceThread::Release() {
	m_ref--;
	if (m_ref <= 0) {
		delete this;
		return 0;
	}
	return m_ref;
}
void CBackupSequenceThread::proc()
{
	WriteLog_setname("Backup");
	//SDKWriteLog((char*)"CBackupSequenceThread::proc() start");
	m_mid->proc();
	//SDKWriteLog((char*)"CBackupSequenceThread::proc() end");
	WriteLog_setname(NULL);
}
IMidSequenceThread* backup_sequence_thread(IMessageQueue* prevq, IMessageQueue* nextq, IScannedImageCtrl* sic, IUnknown* tags)
{
	return (IMidSequenceThread*)new CBackupSequenceThread(prevq, nextq, tags);
}