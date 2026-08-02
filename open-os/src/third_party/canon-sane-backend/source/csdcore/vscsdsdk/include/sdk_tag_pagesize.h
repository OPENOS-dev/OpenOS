/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef __CSD_TAG_SDK_BASIC_PAGESIZE_TAG_HEADER_DEFINED__
#define __CSD_TAG_SDK_BASIC_PAGESIZE_TAG_HEADER_DEFINED__

#include <vector>
#include <memory>
#include "sdk_tag.h"
#include "virtual_scanner_interface.h"

class CPageSize : public CCsdTagBase
{
public:
	CPageSize(ICsdTags2 *parent, IVirtualScanner *pscanner);
	virtual ~CPageSize();
	char *id_name();
	int get(void *lpParam);
	int get_default(void *lpParam);
	int set(long long lParam);
	int set_default();
	int choice_count(long *lpCount);
	int choice(int index, void *lpParam);
	ICsdTag::CSDTAG_CHOICE_FLAG choice_flag();
    int change_default(long long lParam);
	int change_default(LPFNGETVALUE lpfn, void* callback_param);
	int save_value(LPFNSETVALUE lpfn, void* callback_param);
	int restore_value(LPFNGETVALUE lpfn, void* callback_param);
	void save();
	void restore();
	void flush();
private:
	typedef std::vector<char *> CHAR_PTR_LIST;
	CHAR_PTR_LIST m_choice;
	typedef std::vector<unsigned char> INDEX_LIST;
	INDEX_LIST m_cur_index;
	unsigned char m_def_index;
};

class CPageSizeEx : public CCsdTagAsci
{
public:
	CPageSizeEx(ICsdTags2* parent, IVirtualScanner* pscanner);
	virtual ~CPageSizeEx();
	char* id_name();
	//int get(void* lpParam);
	int get_default(void* lpParam);
	int set(long long lParam);
	int set_default();
	int choice_count(long* lpCount);
	int choice(int index, void* lpParam);
	ICsdTag::CSDTAG_CHOICE_FLAG choice_flag();
    int change_default(long long lParam);
	int change_default(LPFNGETVALUE lpfn, void* callback_param);
	int save_value(LPFNSETVALUE lpfn, void* callback_param);
	int restore_value(LPFNGETVALUE lpfn, void* callback_param);
	void save();
	void restore();
	void flush();
private:
	std::unique_ptr<CPageSize>m_page;
};

#endif
