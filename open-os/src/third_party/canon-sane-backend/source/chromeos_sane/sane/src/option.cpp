/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <cstddef>
#include <string.h>
#include <string>
#include "sane/saneopts.h"
#include "option.h"
#include "log.h"
#include "csdtags.org.h"
SANE_Option_Descriptor CNumOfOptions::m_desc = {
	SANE_NAME_NUM_OPTIONS,
	SANE_TITLE_NUM_OPTIONS,
	SANE_DESC_NUM_OPTIONS,
	SANE_TYPE_INT,
	SANE_UNIT_NONE,
	sizeof(SANE_Word),
	SANE_CAP_SOFT_DETECT,
	SANE_CONSTRAINT_NONE,
	{NULL}
};
CNumOfOptions::CNumOfOptions(long num):CSaneOptionBase(), m_num(num)
{
}
CNumOfOptions::~CNumOfOptions()
{}
SANE_Status CNumOfOptions::get(void *value, SANE_Int *info)
{
	if (value==NULL) return SANE_STATUS_INVAL;
	*(SANE_Word*)value = m_num;
	return SANE_STATUS_GOOD;
}
SANE_String_Const CScanSide::m_list[] = {"Simplex", "Duplex", NULL};
SANE_Option_Descriptor CScanSide::m_desc = {
	SANE_NAME_SCAN_SOURCE,
	SANE_TITLE_SCAN_SOURCE,
	SANE_DESC_SCAN_SOURCE,
	SANE_TYPE_STRING,
	SANE_UNIT_NONE,
	256,
	SANE_CAP_SOFT_DETECT | SANE_CAP_SOFT_SELECT,
	SANE_CONSTRAINT_STRING_LIST,
	{CScanSide::m_list}
};
CScanSide::CScanSide(ISaneCsdCore *p):CSaneOptionCsdCore(p)
{
}
CScanSide::~CScanSide()
{
}
SANE_Status CScanSide::get(void *value, SANE_Int *)
{
	if (value==NULL) return SANE_STATUS_INVAL;
	INT32 v;
	m_csdcore->CsdParGet(CSDP_FEEDER, &v);
	if (v) strcpy((char*)value, CScanSide::m_list[1]);//duplex
	else   strcpy((char*)value, CScanSide::m_list[0]);//simplex
	return SANE_STATUS_GOOD;
}
SANE_Status CScanSide::set(void *value, SANE_Int *info, bool)
{
	if (value==NULL) return SANE_STATUS_INVAL;	
	std::string s = (char *)value;
	if (s==CScanSide::m_list[1]) {
		m_csdcore->CsdParSet(CSDP_FEEDER, CSD_FEEDER_DUPLEX);
	} else {
		m_csdcore->CsdParSet(CSDP_FEEDER, CSD_FEEDER_SIMPLEX);
	}
	return SANE_STATUS_GOOD;
}
SANE_String_Const CScanMode::m_list[6] = {
	SANE_VALUE_SCAN_MODE_LINEART,
	SANE_VALUE_SCAN_MODE_GRAY,
	SANE_VALUE_SCAN_MODE_COLOR,
	0
};
SANE_Option_Descriptor CScanMode::m_desc = {
	SANE_NAME_SCAN_MODE,
	SANE_TITLE_SCAN_MODE,
	SANE_DESC_SCAN_MODE,
	SANE_TYPE_STRING,
	SANE_UNIT_NONE,
	256,
	SANE_CAP_SOFT_DETECT | SANE_CAP_SOFT_SELECT,
	SANE_CONSTRAINT_STRING_LIST,
	{CScanMode::m_list}
};
CScanMode::CScanMode(ISaneCsdCore *p):CSaneOptionCsdCore(p)
{
}
CScanMode::~CScanMode()
{
}
SANE_Status CScanMode::get(void *value, SANE_Int *)
{
	if (value==NULL) return SANE_STATUS_INVAL;
	INT32 spp=3;
	INT32 bps=8;
	m_csdcore->CsdParGet(CSDP_SAMPLESPERPIXEL, &spp);
	m_csdcore->CsdParGet(CSDP_BITSPERSAMPLE, &bps);
	if (spp==3) {
		strcpy((char*)value, SANE_VALUE_SCAN_MODE_COLOR);
	} else {
		if (bps==1) {
			strcpy((char*)value, SANE_VALUE_SCAN_MODE_LINEART);
		} else {
		     strcpy((char*)value, SANE_VALUE_SCAN_MODE_GRAY);
		}
	}
	return SANE_STATUS_GOOD;
}
SANE_Status CScanMode::set(void *value, SANE_Int *info, bool bauto)
{
	if (value==NULL) return SANE_STATUS_INVAL;
	SaneWriteLog((char*)("CScanMode::set(%s)"), (char *)value);
	std::string s = (char *)value;		
	if (s==SANE_VALUE_SCAN_MODE_COLOR) {
		m_csdcore->CsdParSet(CSDP_SAMPLESPERPIXEL, 3);
		m_csdcore->CsdParSet(CSDP_BITSPERSAMPLE, 8);
	} else if (s==SANE_VALUE_SCAN_MODE_GRAY) {
		m_csdcore->CsdParSet(CSDP_SAMPLESPERPIXEL, 1);
		m_csdcore->CsdParSet(CSDP_BITSPERSAMPLE, 8);	
	} else {
        m_csdcore->CsdParSet(CSDP_SAMPLESPERPIXEL, 1);
		m_csdcore->CsdParSet(CSDP_BITSPERSAMPLE, 1);
	}
	if (info) *info = SANE_INFO_RELOAD_PARAMS;
	return SANE_STATUS_GOOD;
}
SANE_Word CResolution::m_list[] = {
	4,
	100,
	200,
	300,
	600
};
SANE_Option_Descriptor CResolution::m_desc = {
	SANE_NAME_SCAN_RESOLUTION,
	SANE_TITLE_SCAN_RESOLUTION,
	SANE_DESC_SCAN_RESOLUTION,
	SANE_TYPE_INT,
	SANE_UNIT_DPI,
	sizeof(SANE_Int),
	SANE_CAP_SOFT_DETECT | SANE_CAP_SOFT_SELECT,
	SANE_CONSTRAINT_WORD_LIST,
	{(const SANE_String_Const *)CResolution::m_list}
};
CResolution::CResolution(ISaneCsdCore *p):CSaneOptionCsdCore(p)
{
}
CResolution::~CResolution()
{
}
SANE_Status CResolution::get(void *value, SANE_Int *)
{
	if (value==NULL) return SANE_STATUS_INVAL;
	SANE_Word *p = (SANE_Word*)value;
	long v=300;
	m_csdcore->CsdParGet(CSDP_XRESOLUTION, &v);
	*p = v;
	return SANE_STATUS_GOOD;
}
SANE_Status CResolution::set(void *value, SANE_Int *info, bool bauto)
{
	if (value==NULL) return SANE_STATUS_INVAL;
	SANE_Word *p = (SANE_Word *)value;
	if (info) *info = SANE_INFO_RELOAD_PARAMS;
	m_csdcore->CsdParSet(CSDP_XRESOLUTION, *p);
	return SANE_STATUS_GOOD;
}
SANE_Option_Descriptor CAutoSizeDeskew::m_desc = {
	SANE_I18N("autosize"),
	SANE_I18N("autosize"),
	SANE_I18N("automatically crop and deskewed images"),
	SANE_TYPE_BOOL,
	SANE_UNIT_NONE,
	sizeof(SANE_Bool),
	SANE_CAP_SOFT_DETECT | SANE_CAP_SOFT_SELECT,
	SANE_CONSTRAINT_NONE,
	{NULL}
};
CAutoSizeDeskew::CAutoSizeDeskew(ISaneCsdCore *p):CSaneOptionCsdCore(p)
{
}
CAutoSizeDeskew::~CAutoSizeDeskew()
{
}
SANE_Status CAutoSizeDeskew::get(void *value, SANE_Int *)
{
	if (value==NULL) return SANE_STATUS_INVAL;
	long v=0;
	m_csdcore->CsdParGet(CSDP_AUTOSIZE, &v);
	*(SANE_Bool*)value = v?SANE_TRUE:SANE_FALSE;
	return SANE_STATUS_GOOD;
}
SANE_Status CAutoSizeDeskew::set(void *value, SANE_Int *info, bool)
{
	if (value==NULL) return SANE_STATUS_INVAL;	
	m_csdcore->CsdParSet(CSDP_AUTOSIZE, *(SANE_Bool*)value==SANE_TRUE?1:0);
	m_csdcore->CsdParSet(CSDP_DESKEW, *(SANE_Bool*)value==SANE_TRUE?1:0);
	if (info) *info = SANE_INFO_RELOAD_PARAMS;
	return SANE_STATUS_GOOD;
}
SANE_Option_Descriptor CBrightness::m_desc = {
	SANE_NAME_BRIGHTNESS,
	SANE_TITLE_BRIGHTNESS,
	SANE_DESC_BRIGHTNESS,
	SANE_TYPE_INT,
	SANE_UNIT_PERCENT,
	sizeof(SANE_Word),
	SANE_CAP_SOFT_DETECT | SANE_CAP_SOFT_SELECT,
	SANE_CONSTRAINT_RANGE,
	{NULL}
};
CBrightness::CBrightness(ISaneCsdCore *p):CSaneOptionCsdCore(p)
{
	m_range.min = -100;
	m_range.max = 100;
	m_range.quant = 1;
	m_desc.constraint.range = (SANE_Range *)&m_range;
}
CBrightness::~CBrightness()
{
}
SANE_Status CBrightness::get(void *value, SANE_Int *)
{
	if (value==NULL) return SANE_STATUS_INVAL;
	long v=128;
	m_csdcore->CsdParGet(CSDP_BRIGHTNESS, &v);
	SANE_Word *pout = (SANE_Word*)value;
  v += 1; // 小数点以下切り捨て分の調整
  *pout = ((v - 1) * 201) / 256 - 100;
	return SANE_STATUS_GOOD;
}
SANE_Status CBrightness::set(void *value, SANE_Int *info, bool bauto)
{
	if (value==NULL) return SANE_STATUS_INVAL;
    SANE_Word value_app = *(SANE_Word *)value;
    long value_drv = ((value_app + 100) * 256) / 201 + 1;
	m_csdcore->CsdParSet(CSDP_BRIGHTNESS, value_drv);
	return SANE_STATUS_GOOD;
}
SANE_Option_Descriptor CContrast::m_desc = {
	SANE_NAME_CONTRAST,
	SANE_TITLE_CONTRAST,
	SANE_DESC_CONTRAST,
	SANE_TYPE_INT,
	SANE_UNIT_PERCENT,
	sizeof(SANE_Word),
	SANE_CAP_SOFT_DETECT | SANE_CAP_SOFT_SELECT,
	SANE_CONSTRAINT_RANGE,
	{NULL}
};
CContrast::CContrast(ISaneCsdCore *p):CSaneOptionCsdCore(p)
{
	m_range.min = -100;
	m_range.max = 100;
	m_range.quant = 1;
	m_desc.constraint.range = (SANE_Range *)&m_range;
}
CContrast::~CContrast()
{
}
SANE_Status CContrast::get(void *value, SANE_Int *)
{
	if (value==NULL) return SANE_STATUS_INVAL;
	long v=128;
	m_csdcore->CsdParGet(CSDP_CONTRAST, &v);
	SANE_Word *pout = (SANE_Word*)value;
    v += 1;
    *pout = ((v - 1) * 201) / 256 - 100;
	return SANE_STATUS_GOOD;
}
SANE_Status CContrast::set(void *value, SANE_Int *info, bool bauto)
{
	if (value==NULL) return SANE_STATUS_INVAL;
  SANE_Word value_app = *(SANE_Word *)value;
  long value_drv = ((value_app + 100) * 256) / 201 + 1;
	m_csdcore->CsdParSet(CSDP_CONTRAST, value_drv);
	return SANE_STATUS_GOOD;
}
SANE_Option_Descriptor CDoubleFeedDetection::m_desc = {
	SANE_I18N("DoubleFeedDetection"),
	SANE_I18N("double feed detection"),
	SANE_I18N("double feed detection"),
	SANE_TYPE_BOOL,
	SANE_UNIT_NONE,
	sizeof(SANE_Bool),
	SANE_CAP_SOFT_DETECT | SANE_CAP_SOFT_SELECT,
	SANE_CONSTRAINT_NONE,
	{NULL}
};
CDoubleFeedDetection::CDoubleFeedDetection(ISaneCsdCore *p, long csdp):CSaneOptionCsdCore(p), m_csdp(csdp)
{
}
CDoubleFeedDetection::~CDoubleFeedDetection()
{
}
SANE_Status CDoubleFeedDetection::get(void *value, SANE_Int *)
{
	if (value==NULL) return SANE_STATUS_INVAL;
	long v=0;
	m_csdcore->CsdParGet(m_csdp, &v);
	*(SANE_Bool*)value = v?SANE_TRUE:SANE_FALSE;
	return SANE_STATUS_GOOD;
}
SANE_Status CDoubleFeedDetection::set(void *value, SANE_Int *, bool)
{
	if (value==NULL) return SANE_STATUS_INVAL;	
	m_csdcore->CsdParSet(m_csdp, *(SANE_Bool*)value==SANE_TRUE?1:0);
	return SANE_STATUS_GOOD;
}
SANE_Option_Descriptor CSerialNumber::m_desc = {
	SANE_I18N("serial-number"),
	SANE_I18N("serial-number"),
	SANE_I18N("serial number"),
	SANE_TYPE_STRING,
	SANE_UNIT_NONE,
	256,
	SANE_CAP_SOFT_DETECT,
	SANE_CONSTRAINT_NONE,
	{NULL}
};
CSerialNumber::CSerialNumber(ISaneCsdCore *p):CSaneOptionCsdCore(p)
{
}
CSerialNumber::~CSerialNumber()
{
}
SANE_Status CSerialNumber::get(void *value, SANE_Int *)
{
	if (value==NULL) return SANE_STATUS_INVAL;
	char s[256]={0};
	m_csdcore->CsdParGet(CSDP_SERIAL_NUMBER, s);
	char *pout = (char*)value;
	strcpy(pout, s);
	return SANE_STATUS_GOOD;
}
SANE_Status CSerialNumber::set(void *value, SANE_Int *info, bool bauto)
{
	if (value==NULL) return SANE_STATUS_INVAL;
	return SANE_STATUS_GOOD;
}
SANE_Option_Descriptor CAdfStatus::m_desc = {
	SANE_I18N("adf-status"),
	SANE_I18N("adf-status"),
	SANE_I18N("adf status"),
	SANE_TYPE_INT,
	SANE_UNIT_NONE,
	sizeof(SANE_Int),
	SANE_CAP_SOFT_DETECT,
	SANE_CONSTRAINT_NONE,
	{NULL}
};
CAdfStatus::CAdfStatus(ISaneCsdCore *p):CSaneOptionCsdCore(p)
{
}
CAdfStatus::~CAdfStatus()
{
}
SANE_Status CAdfStatus::get(void *value, SANE_Int *)
{
	if (value==NULL) return SANE_STATUS_INVAL;
	long v=0;
	v = m_csdcore->CsdParGet(CSDP_FEEDER_LOADED, &v);
	SANE_Int *p = (SANE_Int *)value;
	*p = v?0:1;
	return SANE_STATUS_GOOD;
}
SANE_Status CAdfStatus::set(void *value, SANE_Int *info, bool bauto)
{
	if (value==NULL) return SANE_STATUS_INVAL;
	return SANE_STATUS_GOOD;
}
SANE_Option_Descriptor CButtonStatus::m_desc = {
	SANE_I18N("button-status"),
	SANE_I18N("button-status"),
	SANE_I18N("button status"),
	SANE_TYPE_INT,
	SANE_UNIT_NONE,
	sizeof(SANE_Int),
	SANE_CAP_SOFT_DETECT,
	SANE_CONSTRAINT_NONE,
	{NULL}
};
CButtonStatus::CButtonStatus(ISaneCsdCore *p):CSaneOptionCsdCore(p)
{
}
CButtonStatus::~CButtonStatus()
{
}
SANE_Status CButtonStatus::get(void *value, SANE_Int *)
{
	if (value==NULL) return SANE_STATUS_INVAL;
	long v=0;
	m_csdcore->CsdParGet(CSDP_SCANNER_BUTTON, &v);
	SANE_Int *p = (SANE_Int *)value;
	if (v&CSD_SCANNER_BUTTON_START) {
		*p = 1;
	} else if (v&CSD_SCANNER_BUTTON_STOP) {
		*p = 2;	
	} else {
		*p = 0;
	}
	return SANE_STATUS_GOOD;
}
SANE_Status CButtonStatus::set(void *value, SANE_Int *info, bool bauto)
{
	if (value==NULL) return SANE_STATUS_INVAL;
	return SANE_STATUS_GOOD;
}
SANE_Option_Descriptor CScanCount::m_desc = {
	SANE_I18N("scan-count"),
	SANE_I18N("scan-count"),
	SANE_I18N("scan count"),
	SANE_TYPE_INT,
	SANE_UNIT_NONE,
	sizeof(SANE_Int),
	SANE_CAP_SOFT_DETECT,
	SANE_CONSTRAINT_NONE,
	{NULL}
};
CScanCount::CScanCount(ISaneCsdCore *p):CSaneOptionCsdCore(p)
{
}
CScanCount::~CScanCount()
{
}
SANE_Status CScanCount::get(void *value, SANE_Int *)
{
	if (value==NULL) return SANE_STATUS_INVAL;
	long v=0;
	m_csdcore->CsdParGet(CSDP_TOTALPAGECOUNT, &v);
	SANE_Int *p = (SANE_Int *)value;
	*p = v;
	return SANE_STATUS_GOOD;
}
SANE_Status CScanCount::set(void *value, SANE_Int *info, bool bauto)
{
	if (value==NULL) return SANE_STATUS_INVAL;
	return SANE_STATUS_GOOD;
}