/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <cstddef>
#include <memory.h>
#include "ceilogwrite.h"
#include "vssdk.h"
#include "settings_framework.h"
#include "command.h"
#include "sdk_command_util.h"
CSettingsFramework::CSettingsFramework(LPVSCSD_SDK_INIT_INFORMATION pinfo):m_ref(1)
{
	//WriteLog((char*)"SettingsFramework::CSettingsFramework() this is 0x%x", this);
	m_info.dwSize = sizeof(m_info);
	if (pinfo) {
		long sz = sizeof(m_info);
		if (sz>pinfo->dwSize) sz =pinfo->dwSize;
		memcpy(&m_info, pinfo, sz);
		m_info.dwSize = sizeof(m_info);
	}
	m_scan_option.page_code(CScanParam::PAGE_CODE_OPTION);
	m_margin.page_code(CScanParam::PAGE_CODE_MARGIN);
	m_scan_both.page_code(CScanParam::PAGE_CODE_SCAN_BOTH);
	m_scan_bothr.page_code(CScanParam::PAGE_CODE_SCAN_BOTH_RESCAN);/*scan both rescan*/
	m_scan_sep[FRONT].page_code(CScanParam::PAGE_CODE_SCAN_SEP);
	m_scan_sepr[TO_SCANNER][FRONT].page_code(CScanParam::PAGE_CODE_SCAN_SEP_RESCAN);/*scan sep rescan*/
	m_scan_sepr[FROM_CLIENT][FRONT].page_code(CScanParam::PAGE_CODE_SCAN_SEP_RESCAN);/*scan sep rescan*/
	m_scan_sep[BACK].page_code(CScanParam::PAGE_CODE_SCAN_SEP);
	m_scan_sepr[TO_SCANNER][BACK].page_code(CScanParam::PAGE_CODE_SCAN_SEP_RESCAN);/*scan sep rescan*/
	m_scan_sepr[FROM_CLIENT][BACK].page_code(CScanParam::PAGE_CODE_SCAN_SEP_RESCAN);/*scan sep rescan*/
	m_inqex.evpd(true);
	m_inqex.cdb()[4] = 48;
	m_inqex.transfer_length(m_inqex.cdb()[4]);
	CScsiCommand scanner(m_info.pscanner);
	scanner.read(m_inqex);
}
CSettingsFramework::~CSettingsFramework()
{
	//WriteLog((char*)"SettingsFramework::~CSettingsFramework()");
}
long CSettingsFramework::QueryInterface(REFIID id, void **ppOut)
{
	if (memcmp(&IID_IInternalVirtualScanner, &id, sizeof(REFIID)) == 0) {
		*ppOut = dynamic_cast<IUnknown*>(dynamic_cast<IInternalVirtualScanner*>(this));
		AddRef();
		return 0;
	}
	return -1;
}
unsigned long CSettingsFramework::AddRef()
{
	m_ref++;
	return m_ref;
}
unsigned long CSettingsFramework::Release()
{
	m_ref--;
	if (m_ref<=0) {
		delete this;
		return 0;
	}
	return m_ref;
}
long CSettingsFramework::set_vsvalue(long type, void* v)
{
	return 0;
}
long CSettingsFramework::get_vsvalue(long type, void* p)
{
	return 0;
}
VSCSD_SDK_INIT_INFORMATION &CSettingsFramework::info()
{
	return m_info;
}
CWindow &CSettingsFramework::window_from_client(long index)
{
	return m_window[FROM_CLIENT][index];
}
CWindow &CSettingsFramework::window_to_scanner(long index)
{
	return m_window[TO_SCANNER][index];
}
CScanParam &CSettingsFramework::scanparam_option()
{
	return m_scan_option;
}
CScanParam &CSettingsFramework::scanparam_both()
{
	return m_scan_both;
}
CScanParam &CSettingsFramework::scanparam_bothr()
{
	return m_scan_bothr;
}
CScanParam& CSettingsFramework::scanparam_margin()
{
	return m_margin;
}
CScanParam &CSettingsFramework::scanparam_sep(int back)
{
	return m_scan_sep[back];
}
CScanParam &CSettingsFramework::scanparam_sepr_to_scanner(int back)
{
	return m_scan_sepr[TO_SCANNER][back];
}
CScanParam& CSettingsFramework::scanparam_sepr_from_client(int back)
{
	return m_scan_sepr[FROM_CLIENT][back];
}
CScanCmd &CSettingsFramework::scancmd_to_scanner()
{
	return m_scan[TO_SCANNER];
}
CScanCmd& CSettingsFramework::scancmd_from_client()
{
	return m_scan[FROM_CLIENT];
}
CMode &CSettingsFramework::mode()
{
	return m_mode;
}
CInquiryCmd& CSettingsFramework::inquiry_ex(long)
{
	return m_inqex;
}
void CSettingsFramework::store(CCommand &in)
{
	unsigned char cdb = (unsigned char)in.cdb()[0];
	switch (cdb) {
		case opSetWindow:
		{
			CWindow &w = (CWindow&)in;
			if (w.side()) {
				m_window[TO_SCANNER][BACK] = w;//back
				m_window[FROM_CLIENT][BACK] = w;//back
			}
			else {
				m_window[TO_SCANNER][FRONT] = w;//front
				m_window[FROM_CLIENT][FRONT] = w;//front
			}
		}
		break;
		case opScan:
		{
			CScanCmd &s = (CScanCmd&)in;
			m_scan[TO_SCANNER] = s;
			m_scan[FROM_CLIENT] = s;
		}
		break;
		case opSetScanParameter:
		{
			CScanParam &sp = (CScanParam&)in;
			if (sp.page_code() == CScanParam::PAGE_CODE_SCAN_BOTH) {
				m_scan_both = sp;
			}
			else if (sp.page_code() == CScanParam::PAGE_CODE_SCAN_BOTH_RESCAN) {
				m_scan_bothr = sp;
			}
			else if (sp.page_code() == CScanParam::PAGE_CODE_SCAN_SEP) {
				m_scan_sep[sp.side()?1:0] = sp;
			}
			else if (sp.page_code() == CScanParam::PAGE_CODE_SCAN_SEP_RESCAN) {
				m_scan_sepr[TO_SCANNER][sp.side()?1:0] = sp;
				m_scan_sepr[FROM_CLIENT][sp.side()?1:0] = sp;
			}
			else if (sp.page_code() == CScanParam::PAGE_CODE_OPTION) {
				m_scan_option = sp;
			}
			else if (sp.page_code() == CScanParam::PAGE_CODE_MARGIN) {
				m_margin = sp;
			}

		}
		break;
		default:
		break;
	}
}
IUnknown *CreateScannerDependentClass(LPVSCSD_SDK_INIT_INFORMATION pinfo)
{
	return (IUnknown*)new CSettingsFramework(pinfo);
}
