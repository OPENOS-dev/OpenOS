/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef __CSD_TAG_DEFINE_SCANNER_RELATED_CLASS_HEADER_DEFINED__
#define __CSD_TAG_DEFINE_SCANNER_RELATED_CLASS_HEADER_DEFINED__
#include "sdk_tag.h"
#include "virtual_scanner_interface.h"

class CFeederLoaded : public CCsdTagLongScanner
{
public:
	CFeederLoaded(ICsdTags2 *parent, IVirtualScanner *pscanner);
	virtual ~CFeederLoaded();
	int set(long long Param);
	int get(void *lpParam);
};
class CSerialNumber : public CCsdTagAsciScanner
{
public:
	CSerialNumber(ICsdTags2 *parent, IVirtualScanner *pscanner);
	virtual ~CSerialNumber();
	int get(void *lpParam);
private:
	char m_s[16+1];
};
class CTotalCounter : public CCsdTagLongScanner
{
public:
	CTotalCounter(ICsdTags2 *parent, IVirtualScanner *pscanner);
	virtual ~CTotalCounter();
	int set(long long Param);
	int get(void *lpParam);
};
class CRollerCounter : public CCsdTagLongScanner
{
public:
	CRollerCounter(ICsdTags2 *parent, IVirtualScanner *pscanner);
	virtual ~CRollerCounter();
	int set(long long Param);
	int get(void *lpParam);
};
class CMaxRollerCounter : public CCsdTagLongScanner
{
public:
	CMaxRollerCounter(ICsdTags2* parent, IVirtualScanner* pscanner);
	virtual ~CMaxRollerCounter();
	int set(long long Param);
	int get(void* lpParam);
};
class CFirmwareVersion : public CCsdTagAsciScanner
{
public:
	CFirmwareVersion(ICsdTags2 *parent, IVirtualScanner *pscanner);
	virtual ~CFirmwareVersion();
	int get(void *lpParam);
private:
	char m_s[16 + 1];
};
class CScannerButton : public CCsdTagLongScanner
{
public:
	CScannerButton(ICsdTags2* parent, IVirtualScanner* pscanner);
	virtual ~CScannerButton();
	int get(void* lpParam);
};
#endif
