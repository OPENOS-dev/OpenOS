/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <cstddef>
#include "ceilogwrite.h"
#include "command.h"
#include "sdk_command_util.h"
#include "sdk_pagesize_table.h"
#include "csdtags.h"
#include "sdk_tag_pagesize.h"

CPageSize::CPageSize(ICsdTags2 *parent, IVirtualScanner *pscanner):CCsdTagBase(CSDP_PAGESIZE, parent),  m_def_index(0)
{
	//WriteLog((char*)"CPageSize::CPageSize() start");
	m_cur_index.push_back(0);

	CScsiVSCommand scanner(pscanner);
	CInquiryCmd inqex;
	inqex.evpd(true);
	scanner.read(inqex);
	CMode mode;
	scanner.read(mode);

	long scanner_max_width_1200dpi = 0;
	long scanner_max_length_1200dpi = 0;
	pagesize_table_get((char*)"A4", &scanner_max_width_1200dpi, &scanner_max_length_1200dpi);

	if (inqex.basic_xdpi() && inqex.basic_ydpi() && inqex.window_width() && inqex.window_length() && mode.mud())
	{
		scanner_max_width_1200dpi = inqex.window_width() * mode.mud() / inqex.basic_xdpi();
		scanner_max_length_1200dpi = inqex.window_length() * mode.mud() / inqex.basic_ydpi();
	}
	maximum_pagesize_set(scanner_max_width_1200dpi, scanner_max_length_1200dpi);

	long max_table = pagesize_table_count();

	char *s = NULL;
	long width_1200dpi = 0;
	long length_1200dpi = 0;
	for (long i=0; i<max_table; i++) {
		pagesize_table_choice(i, &s, &width_1200dpi, &length_1200dpi);
		if (width_1200dpi<=scanner_max_width_1200dpi && 
			length_1200dpi<=scanner_max_length_1200dpi) {
			//WriteLog((char*)"%s, %ld %ld", s, width_1200dpi, length_1200dpi);
			m_choice.push_back(s);
		} else {
			//WriteLog((char*)"not used:%ld:%ld %ld:%ld", width_1200dpi, scanner_max_width_1200dpi,  length_1200dpi, scanner_max_length_1200dpi);
		}
	}
	m_choice.push_back((char*)"Maximum");
	for (unsigned char j = 0; j < (unsigned char)m_choice.size(); j++) {
		if (strcmp("A4", m_choice[j]) == 0) {
			*m_cur_index.rbegin() = m_def_index = j;
			break;
		}
	}

	
	//WriteLog((char*)"CPageSize::CPageSize() end");
}
CPageSize::~CPageSize()
{
	//WriteLog("CPageSize::~CPageSize");
}
char *CPageSize::id_name()
{
	return (char*)"CSDP_PAGESIZE";
}
int CPageSize::get(void *lpParam)
{
	char *pout = (char*)lpParam;
	strcpy(pout, m_choice[*m_cur_index.rbegin()]);
	return 0;
}
int CPageSize::get_default(void *lpParam)
{
	char *pout = (char*)lpParam;
	strcpy(pout, m_choice[m_def_index]);
	return 0;
}
int CPageSize::set(long long lParam)
{
	char *pin = (char*)lParam;
	for (long i=0; i<(long)m_choice.size(); i++) {
		if (strcmp(pin, m_choice[i])==0) {
			*m_cur_index.rbegin() = (char)i;
			break;
		}
	}
	return 0;
}
int CPageSize::set_default()
{
	*m_cur_index.rbegin()=(char)m_def_index;
	return 0;
}
int CPageSize::choice_count(long *lpCount)
{
	*lpCount = (long)m_choice.size();
	return 0;
}
int CPageSize::choice(int index, void *lpParam)
{
	char *pout = (char*)lpParam;
	strcpy(pout, m_choice[index]);
	return 0;
}
ICsdTag::CSDTAG_CHOICE_FLAG CPageSize::choice_flag()
{
	return ICsdTag::CHOICE_LIST;
}
int CPageSize::save_value(LPFNSETVALUE lpfn, void* callback_param)
{
	char s[512] = { 0 };
	this->get((void*)s);
	return lpfn(id_name(), s, callback_param);
}
int CPageSize::restore_value(LPFNGETVALUE lpfn, void* callback_param)
{
	char s[16] = { 0 };
	char sdef[16] = { 0 };
	get_default((void*)sdef);
	int ret = lpfn(id_name(), s, sizeof(s), sdef, callback_param);
	if (ret) {
		//do nothing
	}
	else {
		set((long long)s);
	}
	return 0;
}
int CPageSize::change_default(long long lParam)
{
    char *pin = (char*)lParam;
    for (long i=0; i<(long)m_choice.size(); i++) {
        if (strcmp(pin, m_choice[i])==0) {
            m_def_index = (char)i;
            break;
        }
    }
    return 0;
}
int CPageSize::change_default(LPFNGETVALUE lpfn, void* callback_param)
{
	char s[64] = { 0 };
	int ret = lpfn(id_name(), s, sizeof(s), m_choice[m_def_index], callback_param);
	if (ret) {
		//do nothing
	}
	else {
		for (unsigned char i = 0; i < (unsigned char)m_choice.size(); i++) {
			if (strcmp(s, m_choice[i]) == 0) {
				m_def_index = i;
				break;
			}
		}

	}	
	return 0;
}
void CPageSize::save()
{
	long  v = *m_cur_index.rbegin();
	m_cur_index.push_back((char)v);
}
void CPageSize::restore()
{
	if (m_cur_index.size()>1) {
		m_cur_index.pop_back();
	}
}
void CPageSize::flush()
{
	if (m_cur_index.size()>1) {
		long v = *m_cur_index.rbegin();
		m_cur_index.pop_back();
		*m_cur_index.rbegin() = (char)v;
	}
}
CPageSizeEx::CPageSizeEx(ICsdTags2* parent, IVirtualScanner* pscanner) :CCsdTagAsci(CSDP_PAGESIZE, parent)
{
	//WriteLog((char*)"CPageSizeEx::CPageSizeEx() start");	
	m_page.reset(new CPageSize(parent, pscanner));
	char s[64] = { 0 };
	m_page->get(s);
	m_def = *m_v.rbegin() = s;
	//WriteLog((char*)"CPageSizeEx::CPageSizeEx() end");
}
CPageSizeEx::~CPageSizeEx()
{
	//WriteLog("CPageSizeEx::~CPageSizeEx");
}
char* CPageSizeEx::id_name()
{
	return m_page->id_name();
}
int CPageSizeEx::get_default(void* lpParam)
{
	return m_page->get_default(lpParam);
}
int CPageSizeEx::set(long long lParam)
{
	m_page->set(lParam);
	return CCsdTagAsci::set(lParam);
}
int CPageSizeEx::set_default()
{
	m_page->set_default();
    char s[32]={0};
    m_page->get(s);
	return CCsdTagAsci::set((long long)s);
}
int CPageSizeEx::choice_count(long* lpCount)
{
	int out = m_page->choice_count(lpCount);
	*lpCount += 1;
	return out;
}
int CPageSizeEx::choice(int index, void* lpParam)
{
	long out = 0;
	long cnt = 0;
	m_page->choice_count(&cnt);
	if (index == cnt) {
		CCsdTagAsci::get(lpParam);
	}
	else {
		out = m_page->choice(index, lpParam);
	}
	return out;
}
ICsdTag::CSDTAG_CHOICE_FLAG CPageSizeEx::choice_flag()
{
	return m_page->choice_flag();
}
int CPageSizeEx::change_default(long long lParam)
{
    return m_page->change_default(lParam);
}
int CPageSizeEx::save_value(LPFNSETVALUE lpfn, void* callback_param)
{
	int out = CCsdTagAsci::save_value(lpfn, callback_param);
	//m_page->save_value(lpfn, callback_param);
	return out;
}
int CPageSizeEx::restore_value(LPFNGETVALUE lpfn, void* callback_param)
{
	int out = CCsdTagAsci::restore_value(lpfn, callback_param);
	m_page->restore_value(lpfn, callback_param);
	return out;
}
int CPageSizeEx::change_default(LPFNGETVALUE lpfn, void* callback_param)
{
	m_page->change_default(lpfn, callback_param);
	return CCsdTagAsci::change_default(lpfn, callback_param);
}
void CPageSizeEx::save()
{
	m_page->save();
	CCsdTagAsci::save();
}
void CPageSizeEx::restore()
{
	m_page->restore();
	CCsdTagAsci::restore();
}
void CPageSizeEx::flush()
{
	m_page->flush();
	CCsdTagAsci::flush();
}
