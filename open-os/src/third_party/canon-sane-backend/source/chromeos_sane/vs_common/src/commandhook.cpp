/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <memory>
#include "vssdk.h"
#include "ceilogwrite.h"
#include "sdk_command_util.h"
#include "commandhook_interface.h"
#include "settings_framework.h"

class CCommandHookBase : public ICommandHook
{
public:
	CCommandHookBase(IScannerConnector *s, CSettingsFramework *p);
	virtual ~CCommandHookBase();
	virtual long exec_write(char *cdb, long cdb_size, char *data, long data_size);
	virtual long exec_read(char *cdb, long cdb_size, char *data, long data_size);
	virtual long exec_none(char *cdb, long cdb_size);
	long STDMETHODCALLTYPE QueryInterface(REFIID id, void **ppOut);
	unsigned long STDMETHODCALLTYPE AddRef();
	unsigned long STDMETHODCALLTYPE Release();
private:
	long m_ref;
protected:
	IScannerConnector *m_pscanner;
	CSettingsFramework *m_psettings;
};
CCommandHookBase::CCommandHookBase(IScannerConnector *s, CSettingsFramework *p):m_ref(1), m_pscanner(s), m_psettings(p){}
CCommandHookBase::~CCommandHookBase(){}
long CCommandHookBase::exec_write(char *cdb, long cdb_size, char *data, long data_size) {return -1;}
long CCommandHookBase::exec_read(char *cdb, long cdb_size, char *data, long data_size){return -1;}
long CCommandHookBase::exec_none(char *cdb, long cdb_size){return -1;}
long CCommandHookBase::QueryInterface(REFIID id, void **ppOut){return -1;}
unsigned long CCommandHookBase::AddRef()
{
	m_ref++;
	return m_ref;
}
unsigned long CCommandHookBase::Release()
{
	m_ref--;
	if (m_ref<=0) {
		delete this;
		return 0;
	}
	return m_ref;
}
class CScanCmdFramework : public CCommandHookBase
{
public:
	CScanCmdFramework(IScannerConnector *s, CSettingsFramework *p);
	virtual ~CScanCmdFramework();
	long exec_write(char *cdb, long cdb_size, char *data, long data_size);
};
CScanCmdFramework::CScanCmdFramework(IScannerConnector *s, CSettingsFramework *p):CCommandHookBase(s, p)
{
	//WriteLog((char*)"CScanCmdFramework::CScanCmdFramework()");
}
CScanCmdFramework::~CScanCmdFramework()
{
	//WriteLog((char*)"CScanCmdFramework::~CScanCmdFramework()");
}
long CScanCmdFramework::exec_write(char *cdb, long cdb_size, char *data, long data_size)
{
	//WriteLog((char*)"CScanCmdFramework::exec_write()");
	CScanCmd sp(cdb, cdb_size, data, data_size);
	m_psettings->store(sp);		
	return 0;//Scan command is not sent to the scanner because scan command will be called in ScanSequence class.
}
class CSetwindowFramework : public CCommandHookBase
{
public:
	CSetwindowFramework(IScannerConnector *s, CSettingsFramework *p);
	virtual ~CSetwindowFramework();
	long exec_write(char *cdb, long cdb_size, char *data, long data_size);
};
CSetwindowFramework::CSetwindowFramework(IScannerConnector *s, CSettingsFramework *p):CCommandHookBase(s, p)
{
	//WriteLog((char*)"CSetwindowFramework::CSetwindowFramework()");
}
CSetwindowFramework::~CSetwindowFramework()
{
	//WriteLog((char*)"CSetwindowFramework::~CSetwindowFramework()");
}
long CSetwindowFramework::exec_write(char *cdb, long cdb_size, char *data, long data_size)
{
	//WriteLog((char*)"CSetwindowFramework::exec_write()");
	CWindow window(cdb, cdb_size, data, data_size);
	m_psettings->store(window);		
	return 0;//Window command is not sent to the scanner this time.
}
class CInquieryFramework : public CCommandHookBase
{
public:
	CInquieryFramework(IScannerConnector *s, CSettingsFramework *p);
	virtual ~CInquieryFramework();
	long exec_read(char *cdb, long cdb_size, char *data, long data_size);
};
CInquieryFramework::CInquieryFramework(IScannerConnector *s, CSettingsFramework *p):CCommandHookBase(s, p)
{
	//WriteLog((char*)"CInquieryFramework::CInquieryFramework()");
}
CInquieryFramework::~CInquieryFramework()
{
	//WriteLog((char*)"CInquieryFramework::~CInquieryFramework()");
}
long CInquieryFramework::exec_read(char *cdb, long cdb_size, char *data, long data_size)
{
	//WriteLog((char*)"CInquieryFramework::exec_read()");
	long ret = 0;
	
	CInquiryCmd inq(cdb, cdb_size, data, data_size);
	if (inq.evpd()) {
		inq = m_psettings->inquiry_ex(CSettingsFramework::FROM_CLIENT);
	}
	else {
		ret = m_pscanner->exec_read(cdb, cdb_size, data, data_size);
	}
	return ret;
	//return m_pscanner->exec_read(cdb, cdb_size, data, data_size);	
}
class CSetScanParameter : public CCommandHookBase
{
public:
	CSetScanParameter(IScannerConnector *s, CSettingsFramework *p);
	virtual ~CSetScanParameter();
	long exec_write(char *cdb, long cdb_size, char *data, long data_size);
};
CSetScanParameter::CSetScanParameter(IScannerConnector *s, CSettingsFramework *p) :CCommandHookBase(s, p)
{
	//WriteLog((char*)"CSetScanParameter::CSetScanParameter()");
}
CSetScanParameter::~CSetScanParameter()
{
	//WriteLog((char*)"CSetScanParameter::~CSetScanParameter()");
}
long CSetScanParameter::exec_write(char *cdb, long cdb_size, char *data, long data_size)
{
	//WriteLog((char*)"CSetScanParameter::exec_write()");
	CScanParam param(cdb, cdb_size, data, data_size);
	m_psettings->store(param);
	//return m_pscanner->exec_read(cdb, cdb_size, data, data_size);
	return 0;
}

ICommandHook *commandhook(unsigned char cdb, IScannerConnector *s, IUnknown *h)
{
	ICommandHook *pout = NULL;
	switch (cdb) {
	case opScan:
		pout= new CScanCmdFramework(s, (CSettingsFramework*)h);
		break;			
	case opSetWindow:
		pout =  new CSetwindowFramework(s, (CSettingsFramework *)h);
		break;
	case opInquiry:
		pout = new CInquieryFramework(s, (CSettingsFramework*)h);
		break;
	case opSetScanParameter:
		pout = new CSetScanParameter(s, (CSettingsFramework*)h);
		break;
	//............
	default:
	break;
	}
	return pout;
}