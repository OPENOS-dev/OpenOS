/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef __SANE_OPTIONS_BASIC_CLASS_DEFINED_HEADER__
#define __SANE_OPTIONS_BASIC_CLASS_DEFINED_HEADER__

#include <vector>
#include <string>
#include "option_util.h"
class CNumOfOptions : public CSaneOptionBase
{
public:
	CNumOfOptions(long num);
	virtual ~CNumOfOptions();
	SANE_Status get(void *value, SANE_Int *info=NULL);
	SANE_Option_Descriptor*descriptor(){return &m_desc;}
private:	
	static SANE_Option_Descriptor m_desc;	
	long m_num;
};
class CScanSide : public CSaneOptionCsdCore
{
public:
	CScanSide(ISaneCsdCore *p);
	virtual ~CScanSide();
	SANE_Status get(void *value, SANE_Int *info=NULL);
	SANE_Status set(void *value, SANE_Int *info=NULL, bool bauto=false);
public:
	SANE_Option_Descriptor*descriptor(){return &m_desc;}
private:	
	static SANE_Option_Descriptor m_desc;
	static SANE_String_Const m_list[];
	static SANE_String_Const m_list_flatbed[];
};
class CScanMode : public CSaneOptionCsdCore
{
public:
	CScanMode(ISaneCsdCore *p);
	virtual ~CScanMode();
	SANE_Status get(void *value, SANE_Int *info=NULL);
	SANE_Status set(void *value, SANE_Int *info=NULL, bool bauto=false);
public:
	SANE_Option_Descriptor*descriptor(){return &m_desc;}
private:	
	static SANE_Option_Descriptor m_desc;
	static SANE_String_Const m_list[];
};
class CResolution : public CSaneOptionCsdCore
{
public:
	CResolution(ISaneCsdCore *p);
	virtual ~CResolution();
	SANE_Status get(void *value, SANE_Int *info=NULL);
	SANE_Status set(void *value, SANE_Int *info=NULL, bool bauto=false);
public:
	SANE_Option_Descriptor*descriptor(){return &m_desc;}
private:	
	static SANE_Option_Descriptor m_desc;
	static SANE_Word m_list[];
};
class CAutoSizeDeskew : public CSaneOptionCsdCore
{
public:
	CAutoSizeDeskew(ISaneCsdCore *p);
	virtual ~CAutoSizeDeskew();
	SANE_Status get(void *value, SANE_Int *info=NULL);
	SANE_Status set(void *value, SANE_Int *info=NULL, bool bauto=false);
	SANE_Option_Descriptor*descriptor(){return &m_desc;}
private:	
	static SANE_Option_Descriptor m_desc;
};
class CBrightness : public CSaneOptionCsdCore
{
public:
	CBrightness(ISaneCsdCore *p);
	virtual ~CBrightness();
	SANE_Status get(void *value, SANE_Int *info=NULL);
	SANE_Status set(void *value, SANE_Int *info=NULL, bool bauto=false);
public:
	SANE_Option_Descriptor*descriptor(){return &m_desc;}
private:	
	static SANE_Option_Descriptor m_desc;
	SANE_Range m_range;
};
class CContrast : public CSaneOptionCsdCore
{
public:
	CContrast(ISaneCsdCore *p);
	virtual ~CContrast();
	SANE_Status get(void *value, SANE_Int *info=NULL);
	SANE_Status set(void *value, SANE_Int *info=NULL, bool bauto=false);
public:
	SANE_Option_Descriptor*descriptor(){return &m_desc;}
private:	
	static SANE_Option_Descriptor m_desc;
	SANE_Range m_range;
};
class CDoubleFeedDetection : public CSaneOptionCsdCore
{
public:
	CDoubleFeedDetection(ISaneCsdCore *p, long csdp);
	virtual ~CDoubleFeedDetection();
	SANE_Status get(void *value, SANE_Int *info=NULL);
	SANE_Status set(void *value, SANE_Int *info=NULL, bool bauto=false);
	SANE_Option_Descriptor*descriptor(){return &m_desc;}
private:	
	static SANE_Option_Descriptor m_desc;
	long m_csdp;
};
class CSerialNumber : public CSaneOptionCsdCore
{
public:
	CSerialNumber(ISaneCsdCore *p);
	virtual ~CSerialNumber();
	SANE_Status get(void *value, SANE_Int *info=NULL);
	SANE_Status set(void *value, SANE_Int *info=NULL, bool bauto=false);
public:
	SANE_Option_Descriptor*descriptor(){return &m_desc;}
private:	
	static SANE_Option_Descriptor m_desc;
};
class CAdfStatus : public CSaneOptionCsdCore
{
public:
	CAdfStatus(ISaneCsdCore *p);
	virtual ~CAdfStatus();
	SANE_Status get(void *value, SANE_Int *info=NULL);
	SANE_Status set(void *value, SANE_Int *info=NULL, bool bauto=false);
public:
	SANE_Option_Descriptor*descriptor(){return &m_desc;}
private:	
	static SANE_Option_Descriptor m_desc;
};
class CButtonStatus : public CSaneOptionCsdCore
{
public:
	CButtonStatus(ISaneCsdCore *p);
	virtual ~CButtonStatus();
	SANE_Status get(void *value, SANE_Int *info=NULL);
	SANE_Status set(void *value, SANE_Int *info=NULL, bool bauto=false);
public:
	SANE_Option_Descriptor*descriptor(){return &m_desc;}
private:	
	static SANE_Option_Descriptor m_desc;
};
class CScanCount : public CSaneOptionCsdCore
{
public:
	CScanCount(ISaneCsdCore *p);
	virtual ~CScanCount();
	SANE_Status get(void *value, SANE_Int *info=NULL);
	SANE_Status set(void *value, SANE_Int *info=NULL, bool bauto=false);
public:
	SANE_Option_Descriptor*descriptor(){return &m_desc;}
private:	
	static SANE_Option_Descriptor m_desc;
};
#endif
