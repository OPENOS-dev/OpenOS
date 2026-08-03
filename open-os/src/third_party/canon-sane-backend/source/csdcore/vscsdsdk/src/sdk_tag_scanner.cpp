/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <cstddef>
#include <time.h>
#include <math.h>
#include <climits>
#include "ceilogwrite.h"
#include "csdtags.h"
#include "csderr.h"
#include "sdk_tag_scanner.h"
#include "sdk_command_util.h"
#include "global_apis.h"
CFeederLoaded::CFeederLoaded(ICsdTags2 *parent, IVirtualScanner *pscanner) :CCsdTagLongScanner(CSDP_FEEDER_LOADED, parent, pscanner)
{
}
CFeederLoaded::~CFeederLoaded()
{
}
int CFeederLoaded::get(void *lpParam)
{
	//SDKWriteLog("CFeederLoaded::get() start");
	if (m_pscanner->scanning()) {
		//SDKWriteLog("CFeederLoaded::get() end(scanning)");
		return 0;
	}
	long ret = 0;
	long *pout = (long*)lpParam;
	CScsiVSCommand scanner(m_pscanner);
	CObjectPositionCmd obj(CObjectPositionCmd::MediumPosition);
	ret = scanner.none(obj);
	if (ret) {
		if (pout) *pout = 0;
		CSenseCmd sns;
		scanner.read(sns);
		if (sns.is_no_paper()) {
			ret = CSD3_NOPAGE;
		}
		else {
			if (sns.is_cover_open()) {
				ret = CSD3_COVEROPEN;
			}
			else if (sns.is_double_feed_error()) {
				ret = CSD3_DOUBLEFEED;
			}
			else if (sns.is_jam_error()) {
				ret = CSD3_JAM;
			} else {
				ret = CSD3_HARDERROR;
			}
		}
	}
	else {
		ret = 0;
	}
	//SDKWriteLog("CFeederLoaded::get() end %d");
	return (int)ret;
}
int CFeederLoaded::set(long long Param)
{
	return 0;
}
CSerialNumber::CSerialNumber(ICsdTags2 *parent, IVirtualScanner *pscanner):CCsdTagAsciScanner(CSDP_SERIAL_NUMBER, parent, pscanner)
{
	memset(m_s, 0, sizeof(m_s));
}
CSerialNumber::~CSerialNumber()
{
}
int CSerialNumber::get(void *lpParam)
{
	if (m_pscanner->scanning()) return 0;
	char *pout = (char *)lpParam;
	if (m_s[0]) {
		strcpy(pout, m_s);
	} else {
		CStreamCmd s(CStreamCmd::SERVICEDATA, 0);
		CScsiVSCommand scanner(m_pscanner);
		scanner.read(s);
		strcpy(pout, s.serial_number());
		strcpy(m_s, s.serial_number());
	}
	return 0;
}
CTotalCounter::CTotalCounter(ICsdTags2 *parent, IVirtualScanner *pscanner) :CCsdTagLongScanner(CSDP_TOTALPAGECOUNT, parent, pscanner)
{
}
CTotalCounter::~CTotalCounter()
{
}
int CTotalCounter::get(void *lpParam)
{
	if (m_pscanner->scanning()) return 0;
	long *pout = (long*)lpParam;
	CScsiVSCommand scanner(m_pscanner);
	CStreamCmd svc(CStreamCmd::USERDATA, 0);
	scanner.read(svc);
	*pout = svc.paper_counter();
	return 0;
}
int CTotalCounter::set(long long Param)
{
	return 0;
}
CRollerCounter::CRollerCounter(ICsdTags2 *parent, IVirtualScanner *pscanner) :CCsdTagLongScanner(CSDP_ROLLER_COUNTER, parent, pscanner)
{
}
CRollerCounter::~CRollerCounter()
{
}
int CRollerCounter::get(void *lpParam)
{
	if (m_pscanner->scanning()) return 0;
	long *pout = (long*)lpParam;
	CScsiVSCommand scanner(m_pscanner);
	CStreamCmd svc(CStreamCmd::USERDATA, 0);
	scanner.read(svc);
	*pout = svc.paper_counter() - svc.parts1_counter();
	return 0;
}
int CRollerCounter::set(long long Param)
{
	if (m_pscanner->scanning()) return 0;
	CStreamCmd svc(CStreamCmd::USERDATA, 0);
	CScsiVSCommand scanner(m_pscanner);
	scanner.read(svc);
	long cur = svc.paper_counter() - svc.parts1_counter();
	if (cur != Param) {
		svc.parts1_counter(svc.paper_counter() - (long)Param);
		svc.parts1_time((long)time(NULL));
		scanner.write(svc);
	}
	return 0;
}
CMaxRollerCounter::CMaxRollerCounter(ICsdTags2* parent, IVirtualScanner* pscanner) :CCsdTagLongScanner(CSDP_MAX_ROLLER_COUNTER, parent, pscanner)
{
}
CMaxRollerCounter::~CMaxRollerCounter()
{
}
int CMaxRollerCounter::get(void* lpParam)
{
	if (m_pscanner->scanning()) return 0;
	long* pout = (long*)lpParam;
	CScsiVSCommand scanner(m_pscanner);
	CStreamCmd svc(CStreamCmd::USERDATA, 1);
	scanner.read(svc);
	if (svc.enable_parts1_warning()) {
		*pout = svc.parts1_counter_limit();
	}
	else {
		*pout = LONG_MAX;
	}
	return 0;
}
int CMaxRollerCounter::set(long long Param)
{
	if (m_pscanner->scanning()) return 0;
	CStreamCmd svc(CStreamCmd::USERDATA, 0);
	CScsiVSCommand scanner(m_pscanner);
	scanner.read(svc);
	long cur = svc.parts1_counter_limit();
	if (cur != Param) {
		svc.parts1_counter_limit((long)Param);
		scanner.write(svc);
	}
	return 0;
}
CFirmwareVersion::CFirmwareVersion(ICsdTags2 *parent, IVirtualScanner *pscanner):CCsdTagAsciScanner(CSDP_FIRMVERSION, parent, pscanner)
{
	memset(m_s, 0, sizeof(m_s));
}
CFirmwareVersion::~CFirmwareVersion()
{
}
int CFirmwareVersion::get(void *lpParam)
{
	char *pout = (char *)lpParam;
	if (m_s[0]) {
		strcpy(pout, m_s);
	} else {		
		if (m_pscanner->scanning()) {

		} else {
			CInquiryCmd inq;
			CScsiVSCommand scanner(m_pscanner);
			scanner.read(inq);
			strcpy(pout, inq.product_revision_level());
			char *p = strstr(pout, " ");
			if (p) *p=0;
		}	
	}
	return 0;
}
CScannerButton::CScannerButton(ICsdTags2* parent, IVirtualScanner* pscanner) :CCsdTagLongScanner(CSDP_SCANNER_BUTTON, parent, pscanner)
{
}
CScannerButton::~CScannerButton()
{
}
int CScannerButton::get(void* lpParam)
{
	if (m_pscanner->scanning()) return 0;

	long* pout = (long*)lpParam;

	CScsiVSCommand scanner(m_pscanner);
	CStreamCmd panel(CStreamCmd::PANEL, 0);
	scanner.read(panel);
	if (panel.start_key()) {
		*pout |= CSD_SCANNER_BUTTON_START;
	}
	if (panel.stop_key()) {
		*pout |= CSD_SCANNER_BUTTON_STOP;
	}
    if (panel.dfr_key()) {
        *pout |= CSD_SCANNER_BUTTON_DFD;
    }
    if (panel.non_sep_key()) {
        *pout |= CSD_SCANNER_BUTTON_NON_SEP;
    }
	return 0;
}
