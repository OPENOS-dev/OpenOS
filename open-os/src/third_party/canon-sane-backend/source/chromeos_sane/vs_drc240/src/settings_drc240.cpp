/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <cstddef>
#include <memory.h>
#include "ceilogwrite.h"
#include "vssdk.h"
#include "settings_framework.h"
#include "sdk_command_util.h"
CSettingsFramework::CSettingsFramework(LPVSCSD_SDK_INIT_INFORMATION pinfo) :
	m_ref(1)
{
	WriteLog((char*)"SettingsFramework::CSettingsFramework(%p) this is 0x%x", pinfo, this);
	m_info.dwSize = sizeof(m_info);
	long sz = sizeof(m_info);
	if (sz > pinfo->dwSize) sz = pinfo->dwSize;
	memcpy(&m_info, pinfo, sz);
	m_info.dwSize = sizeof(m_info);
	m_scan_option.page_code(CScanParam::PAGE_CODE_OPTION);
	m_scan_margin.page_code(CScanParam::PAGE_CODE_MARGIN);
	m_scan_both.page_code(CScanParam::PAGE_CODE_SCAN_BOTH);
	m_scan_bothr.page_code(CScanParam::PAGE_CODE_SCAN_BOTH_RESCAN);/*scan both rescan*/
	m_scan_sep[FRONT].page_code(CScanParam::PAGE_CODE_SCAN_SEP);
	m_scan_sepr[FRONT].page_code(CScanParam::PAGE_CODE_SCAN_SEP_RESCAN);/*scan sep rescan*/
	m_scan_sep[BACK].page_code(CScanParam::PAGE_CODE_SCAN_SEP);
	m_scan_sepr[BACK].page_code(CScanParam::PAGE_CODE_SCAN_SEP_RESCAN);/*scan sep rescan*/

	m_inqex[TO_SCANNER].evpd(true);
	CScsiCommand scanner(m_info.pscanner);
	scanner.read(m_inqex[TO_SCANNER]);
	m_inqex[FROM_CLIENT] = m_inqex[TO_SCANNER];
	m_inqex[FROM_CLIENT].window_width(2552 * 2);
	m_inqex[FROM_CLIENT].basic_xdpi(600);
}
CSettingsFramework::~CSettingsFramework()
{
	//WriteLog((char*)"CSettingsFramework::~CSettingsFramework()");
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
long CSettingsFramework::margin_1200dpi()
{
	return 472;
}
long CSettingsFramework::sensor_version()
{
	return 0;
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
CWindow &CSettingsFramework::window(long index)
{
	return m_window[index];
}
CScanCmd &CSettingsFramework::scancmd(long index)
{
	return m_scan[index];
}
CMode &CSettingsFramework::mode()
{
	return m_mode;
}
CScanParam& CSettingsFramework::scanParam_both()
{
	return m_scan_both;
}
CScanParam& CSettingsFramework::scanParam_bothr()
{
	return m_scan_bothr;
}
CScanParam &CSettingsFramework::scanParam_sep(long side)
{
	return m_scan_sep[side];
}
CScanParam &CSettingsFramework::scanParam_sep_rescan(long side)
{
	return m_scan_sepr[side];
}
CScanParam  &CSettingsFramework::scanParam_option()
{
	return m_scan_option;
}
CScanParam& CSettingsFramework::scanParam_margin()
{
	return m_scan_margin;
}
CInquiryCmd& CSettingsFramework::inquiry_ex(long index)
{
	return m_inqex[index];
}
long CSettingsFramework::scanner_needs_shading_data()
{
	return 0;
}
long CSettingsFramework::has_card()
{
	return 0;
}
bool CSettingsFramework::card_scan()
{
	return false;
}
void CSettingsFramework::card_scan(bool)
{}
void CSettingsFramework::store(CCommand &in)
{
	unsigned char cdb = (unsigned char)in.cdb()[0];
	switch (cdb) {
		case opSetWindow:
		{
			CWindow &w = (CWindow&)in;
			m_window[0] = w;
			m_window[1] = w;
		}
		break;
		case opScan:
		{
			CScanCmd &s = (CScanCmd&)in;
			m_scan[0] = s;
			m_scan[1] = s;
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
				m_scan_sepr[sp.side()?1:0] = sp;
			}
			else if (sp.page_code() == CScanParam::PAGE_CODE_OPTION) {
				m_scan_option = sp;
			}
			else if (sp.page_code() == CScanParam::PAGE_CODE_MARGIN) {
				m_scan_margin = sp;
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
