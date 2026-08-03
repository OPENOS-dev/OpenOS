/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/#ifndef __SETTINGS_Framework_CLASS_HEADER_DEFINED__
#define __SETTINGS_Framework_CLASS_HEADER_DEFINED__

#include "sdk_def.h"
#include "unknown.h"
#include "sdk_command_util.h"

class CSettingsFramework : public IInternalVirtualScanner
{ 
public:
	CSettingsFramework(LPVSCSD_SDK_INIT_INFORMATION pinfo);
	virtual ~CSettingsFramework();

	long STDMETHODCALLTYPE QueryInterface(REFIID id, void **ppOut);
	unsigned long STDMETHODCALLTYPE AddRef();
	unsigned long STDMETHODCALLTYPE Release();

	long set_vsvalue(long type, void* v);
	long get_vsvalue(long type, void* p);
public:
	VSCSD_SDK_INIT_INFORMATION &info();
private:
	long m_ref;
	VSCSD_SDK_INIT_INFORMATION m_info;
public:
	void store(CCommand &in);
public:
	CWindow    &window_from_client(long index);
	CWindow    &window_to_scanner(long index);
	CScanCmd   &scancmd_to_scanner();
	CScanCmd   &scancmd_from_client();
	CMode      &mode();
	CScanParam &scanparam_option();
	CScanParam &scanparam_both();
	CScanParam &scanparam_bothr();
	CScanParam &scanparam_margin();
	CScanParam &scanparam_sep(int back);
	CScanParam &scanparam_sepr_to_scanner(int back);
	CScanParam &scanparam_sepr_from_client(int back);
	CInquiryCmd& inquiry_ex(long);
	enum {
		TO_SCANNER = 0,
		FROM_CLIENT,
		TOSCANNER_FROMCLIENT
	}; 
	enum {
		FRONT=0,
		BACK,
		FRONT_BACK
	};
private:
	CWindow			m_window[TOSCANNER_FROMCLIENT][FRONT_BACK];/*0:front 1:back*/
	CScanCmd		m_scan[TOSCANNER_FROMCLIENT];
	CMode			m_mode;
	CScanParam      m_scan_option;
	CScanParam      m_scan_both;
	CScanParam      m_scan_bothr;
	CScanParam      m_margin;
	CScanParam      m_scan_sep[FRONT_BACK];/*0:front 1:back*/
	CScanParam		m_scan_sepr[TOSCANNER_FROMCLIENT][FRONT_BACK];/*0:front 1:back*/
	CInquiryCmd     m_inqex;
};

#endif
