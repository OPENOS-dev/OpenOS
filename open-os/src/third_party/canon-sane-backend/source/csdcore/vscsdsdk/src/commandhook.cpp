/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <memory>
#include <map>
#include "command.h"
#include "commandhook_interface.h"

class CCommandHook : public ICommandHook
{
public:
	CCommandHook(IScannerConnector *s, IUnknown *h);
	virtual ~CCommandHook();
	long init();
	void uninit();
	long exec_read(char *cdb, long cdb_size, char *data, long data_size);
	long exec_write(char *cdb, long cdb_size, char *data, long data_size);
	long exec_none(char *cdb, long cdb_size);
	long STDMETHODCALLTYPE QueryInterface(REFIID id, void **ppOut);
	unsigned long STDMETHODCALLTYPE AddRef();
	unsigned long STDMETHODCALLTYPE Release();
private:
	IScannerConnector *m_pscanner;
	long m_ref;
	typedef std::map<unsigned char, ICommandHook*> COMMANDHOOKS;
	COMMANDHOOKS m_hooks;
	IUnknown *m_handle;
};


CCommandHook::CCommandHook(IScannerConnector *s, IUnknown *h):m_pscanner(s), m_ref(1), m_handle(h)
{
}
CCommandHook::~CCommandHook()
{
    uninit();
}
long CCommandHook::QueryInterface(REFIID id, void **ppOut)
{
	return -1;
}
unsigned long CCommandHook::AddRef()
{
	m_ref++;
	return m_ref;
}
unsigned long CCommandHook::Release()
{
	m_ref--;
	if (m_ref<=0) {
		delete this;
		return 0;
	}
	return m_ref;
}
long CCommandHook::init()
{
	unsigned char tbl[] = {
		(unsigned char)opTestUnitReady,
		(unsigned char)opRequestSense,
		(unsigned char)opInquiry,
		(unsigned char)opModeSelect,
		(unsigned char)opReserveUnit,
		(unsigned char)opReleaseUnit,
		(unsigned char)opModeSense,
		(unsigned char)opScan,
		(unsigned char)opReceiveDiagnostic,
		(unsigned char)opSendDiagnostic,	
		(unsigned char)opSetWindow,
		(unsigned char)opGetWindow,
		(unsigned char)opRead,
		(unsigned char)opSend,
		(unsigned char)opObjectPosition,
		(unsigned char)opRescan,
		(unsigned char)opDiscard,
		(unsigned char)opGetScannerStatus,
		(unsigned char)opGetScanMode,
		(unsigned char)opDefineScanMode,
		(unsigned char)opStopBatch,
		(unsigned char)opSleepControl,
		(unsigned char)opGetAdjustData,
		(unsigned char)opSetAdjustData,
		(unsigned char)opCheckScanSize,
		(unsigned char)opService,
		(unsigned char)opGetMemory,
		(unsigned char)opLockOperationPanel,
		(unsigned char)opUnlockOperationPanel,
		(unsigned char)opGetScanParameter,
		(unsigned char)opSetScanParameter,
		(unsigned char)opRunSubsidiary
	};
	for (long i=0; i<(long)(sizeof(tbl)/sizeof(tbl[0])); i++) {
		ICommandHook *p = commandhook(tbl[i], m_pscanner, m_handle); 
		if (p) {
			m_hooks[tbl[i]] = p;
		}
	}
	return 0;
}
void CCommandHook::uninit()
{
	COMMANDHOOKS::iterator itr = m_hooks.begin();
	for (; itr!=m_hooks.end(); itr++) {
		if (itr->second) {
            (itr->second)->Release();
		}
	}
	m_hooks.clear();
}
long CCommandHook::exec_read(char *cdb, long cdb_size, char *data, long data_size)
{
	if (cdb==NULL) return -1;
	COMMANDHOOKS::iterator itr = m_hooks.find((unsigned char)cdb[0]);
	if (itr!=m_hooks.end()&&itr->second) return (itr->second)->exec_read(cdb, cdb_size, data, data_size);
	return m_pscanner->exec_read(cdb, cdb_size, data, data_size);
}
long CCommandHook::exec_write(char *cdb, long cdb_size, char *data, long data_size)
{
	if (cdb==NULL) return -1;
	COMMANDHOOKS::iterator itr = m_hooks.find((unsigned char)cdb[0]);
	if (itr!=m_hooks.end()&&itr->second) return (itr->second)->exec_write(cdb, cdb_size, data, data_size);
	return m_pscanner->exec_write(cdb, cdb_size, data, data_size);
}
long CCommandHook::exec_none(char *cdb, long cdb_size)
{
	if (cdb==NULL) return -1;
	COMMANDHOOKS::iterator itr = m_hooks.find((unsigned char)cdb[0]);
	if (itr!=m_hooks.end()&&itr->second) return (itr->second)->exec_none(cdb, cdb_size);
	return m_pscanner->exec_none(cdb, cdb_size);
}
ICommandHook *commandhook(IScannerConnector *s, IUnknown *h)
{
	std::unique_ptr<CCommandHook>hook(new CCommandHook(s, h));
	if (hook.get()==NULL) return NULL;
	if (hook->init()) return NULL;
	return (ICommandHook*)hook.release();
}
