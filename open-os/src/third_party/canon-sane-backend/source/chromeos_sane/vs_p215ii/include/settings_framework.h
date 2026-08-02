/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef __SETTINGS_Framework_CLASS_HEADER_DEFINED__
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
	CWindow		&window(long index);
	CScanCmd    &scancmd(long index);
	CMode       &mode();
	CScanParam	&scanParam_both();
	CScanParam  &scanParam_bothr();
	CScanParam  &scanParam_sep(long side);
	CScanParam  &scanParam_sep_rescan(long side);
	CScanParam  &scanParam_option();
	CScanParam  &scanParam_margin();
	CInquiryCmd &inquiry_ex(long index);
	long         margin_1200dpi();
	long         sensor_version();
	long         scanner_needs_shading_data();
	long         has_card();
	bool         card_scan();
	void         card_scan(bool c);
	enum {
		FRONT = 0,
		BACK,
		F_B_COUNT
	};
	enum {
		TO_SCANNER = 0,
		FROM_CLIENT,
		S_C_COUNT
	};
private:
	CWindow         m_window[S_C_COUNT];/*0:scanner, 1:client*/
	CScanCmd        m_scan[S_C_COUNT];//0:scanner, 1:client
	CMode           m_mode;
	CScanParam      m_scan_option;
	CScanParam      m_scan_margin;
	CScanParam      m_scan_both;  //batch, carrier, max_num_batch_numPaper, max_paper_length
	CScanParam      m_scan_bothr; //autoSize_deskew, deskewPosition
	CScanParam      m_scan_sep[2];/*0:front 1:back*/  //autoColor_*, noiseRemove, noiseRemove_level, 3Dgamma
	CScanParam		m_scan_sepr[2];/*0:front 1:back*/ //bleed_background0-0, bleed_background_level1-7, edge Emphasis0-0, edge_level1-5
									//dropoutcolor, emphasis color, gammaMode, colorgammamode
	CInquiryCmd     m_inqex;
	bool            m_card_scan;
};

#endif
