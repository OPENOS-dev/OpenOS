/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef __CSD_TAG_SDK_BASIC_TAG_HEADER_DEFINED__
#define __CSD_TAG_SDK_BASIC_TAG_HEADER_DEFINED__

#include <vector>
#include "sdk_tag.h"
#include "virtual_scanner_interface.h"
class CYDpi : public CCsdTagBase
{
public:
	CYDpi(ICsdTags2 *parent);
	virtual ~CYDpi();
	int get(void *lpParam);
	int get_default(void *lpParam);
	int set(long long lParam);
	int set_default();
	int choice_count(long *lpCount);
	int choice(int index, void *lpParam);
	ICsdTag::CSDTAG_CHOICE_FLAG choice_flag();
	void save();
	void restore();
	void flush();
private:
	ICsdTag *m_pxdpi;
};
class CScanSide : public CCsdTagLong
{
public:
	CScanSide(ICsdTags2 *parent);
	virtual ~CScanSide();
	char *id_name();
private:
	long m_choice[2];
};
#endif
