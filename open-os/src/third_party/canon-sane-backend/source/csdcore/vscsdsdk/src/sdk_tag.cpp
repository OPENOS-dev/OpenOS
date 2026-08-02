/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <cstdlib>
#include <cstddef>
#include <string.h>
#include "ceilogwrite.h"
#include "csdtags.h"
#include "sdk_tag.h"

CCsdTagBase::CCsdTagBase(long id, ICsdTags2 *parent):m_ref(1),m_id(id), m_parent(parent)
{
	//WriteLog("CCsdTagBase::CCsdTagBase(0x%x)", this);
}
CCsdTagBase::~CCsdTagBase()
{
	//WriteLog("CCsdTagBase::~CCsdTagBase(0x%x)", this);
}
long CCsdTagBase::QueryInterface(REFIID id, void **ppOut)
{
	return -1;
}
unsigned long CCsdTagBase::AddRef()
{
	//WriteLog((char*)"CCsdTagBase::AddRef()");
	m_ref++;
	return m_ref;
}
unsigned long CCsdTagBase::Release()
{
	//WriteLog((char*)"CCsdTagBase::Release()");
	m_ref--;
	if (m_ref<=0) {
		delete this;
		return 0;
	}
	return m_ref;
}
int CCsdTagBase::id()
{
	return (int)m_id;
}
char *CCsdTagBase::id_name()
{
	return NULL;
}
ICsdTags2 *CCsdTagBase::parent()
{
	return m_parent;
}
void CCsdTagBase::update(ICsdTag *sender)
{
}
void CCsdTagBase::update_def(ICsdTag* sender)
{
}
int CCsdTagBase::get(void *lpParam)
{
	return 0;
}
int CCsdTagBase::get_default(void *lpParam)
{
	return 0;
}
int CCsdTagBase::set(long long lParam)
{
	return 0;
}
int CCsdTagBase::change_default(LPFNGETVALUE lpfn, void* callback_param)
{
	return 0;
}
int CCsdTagBase::set_default()
{
	SDKWriteLog((char*)"CsdTagBase::set_default()");
	return 0;
}
int CCsdTagBase::change_default(long long lParam)
{
	SDKWriteLog((char*)"CsdTagBase::change_default()");
	return 0;
}

int CCsdTagBase::choice_count(long *lpCount)
{
	SDKWriteLog((char*)"CsdTagBase::choice_count(0x%x)", lpCount);
	return 0;
}
int CCsdTagBase::choice(int index, void *lpParam)
{
	SDKWriteLog((char*)"CsdTagBase::choice(%d, 0x%x)", index, lpParam);
	return 0;
}
int CCsdTagBase::choice_flag(long *lpFlag)
{
	*lpFlag = choice_flag();
	return 0;
}
ICsdTag::CSDTAG_CHOICE_FLAG CCsdTagBase::choice_flag()
{
	return ICsdTag::CHOICE_ANY;
}
int CCsdTagBase::save_value(LPFNSETVALUE lpfn, void* callback_param)
{
	return 0;
}
int CCsdTagBase::restore_value(LPFNGETVALUE lpfn, void* callback_param)
{
	return 0;
}
void CCsdTagBase::save()
{
}
void CCsdTagBase::restore()
{
}
void CCsdTagBase::flush()
{
}
CCsdTagLongBase::CCsdTagLongBase(long id, ICsdTags2 *p):CCsdTagBase(id, p), m_def(0), m_pchoice(NULL), m_choice_size(0)
{
}
CCsdTagLongBase::~CCsdTagLongBase()
{
}
int CCsdTagLongBase::get_default(void *lpParam)
{
	long *pout = (long*)lpParam;
	*pout = m_def;
	return 0;
}
int CCsdTagLongBase::set_default()
{
	return set(m_def);
}
int CCsdTagLongBase::change_default(long long lParam)
{
	m_def = (long)lParam;
	return 0;
}
int CCsdTagLongBase::change_default(LPFNGETVALUE lpfn, void *callback_param)
{
	char *n = id_name();
	if (n) {
		char s[16] = { 0 };
		char sdef[16] = { 0 };
		long def = 0;
		get_default((void*)&def);
		sprintf(sdef, "%ld", def);
		int ret = lpfn(n, s, sizeof(s), sdef, callback_param);
		if (ret) {
			//do nothing
		}
		else {
			m_def = atoi(s);
		}
	}
	return 0;
}
int CCsdTagLongBase::choice_count(long *lpCount)
{
	if (choice_flag()==ICsdTag::CHOICE_LIST) {
		*lpCount = m_choice_size;
	} else {
		*lpCount = 1;
	}
	
	return 0;
}
int CCsdTagLongBase::choice(int index, void *lpParam)
{
	//WriteLog((char*)"CCsdTagLongBase::choice(%d, 0x%x) start", index, lpParam);
	int out = 0;
	switch (choice_flag()) {
		default:
		case ICsdTag::CHOICE_ANY:out = choice_any(index, lpParam);break;
		case ICsdTag::CHOICE_RANGE:out = choice_range(index, lpParam);break;
		case ICsdTag::CHOICE_LIST:out = choice_list(index, lpParam);break;
	}
	//WriteLog((char*)"CCsdTagLongBase::choice() end");
	return out;
}
int CCsdTagLongBase::choice_any(int index, void *lpParam)
{
	return get(lpParam);
}
int CCsdTagLongBase::choice_range(int index, void *lpParam)
{
	//WriteLog((char*)"CCsdTagLongBase::choice_range(%d, 0x%x) start", index, lpParam);
	int out = 0;
	if (m_pchoice) {
		long *pout = (long *)lpParam;
		switch ((CSDTAG_CHOICE_RANGE)index) {
		case ICsdTag::RANGE_LOW: *pout=m_pchoice[0];break;
		case ICsdTag::RANGE_STEP:*pout=m_pchoice[1];break;
		case ICsdTag::RANGE_HIGH:*pout=m_pchoice[2];break; 
		default:
			*pout=m_pchoice[0];
			break;
		}
	} else {
		out = (int)m_def;
	}
	//WriteLog((char*)"CCsdTagLongBase::choice_range() end");
	return out;
}
int CCsdTagLongBase::choice_list(int index, void *lpParam)
{
	int out = 0;
	if (m_pchoice) {
		long *pout = (long *)lpParam;
		switch ((CSDTAG_CHOICE_RANGE)index) {
		case ICsdTag::RANGE_LOW: *pout=m_pchoice[0];break;
		case ICsdTag::RANGE_STEP:break;
		case ICsdTag::RANGE_HIGH:*pout=m_pchoice[m_choice_size?m_choice_size-1:0];break; 
		default:
		if (index>=0 && index < m_choice_size) {
			*pout = m_pchoice[index];
		}
		break;
		}
	} else {
		out = get(lpParam);
	}
	return out;
}
CCsdTagLong::CCsdTagLong(long id, ICsdTags2 *p):CCsdTagLongBase(id, p)
{
	m_v.push_back(0);
}
CCsdTagLong::~CCsdTagLong()
{
}
int CCsdTagLong::get(void *lpParam)
{
	long *p = (long *)lpParam;
	*p = (long)*m_v.rbegin();
	return 0;
}
int CCsdTagLong::set_range(long long lParam)
{
	if (m_pchoice) {
		if (m_pchoice[1] == 1) {
			if (lParam >= m_pchoice[0] && lParam <= m_pchoice[2]) {
				*m_v.rbegin() = lParam;
			}
		}
		else if (m_pchoice[1] > 1) {
			for (long i = m_pchoice[0]; i <= m_pchoice[2]; i += m_pchoice[1]) {
				if (i == lParam) {
					*m_v.rbegin() = lParam;
				}
			}
		}
	}
	else {
		*m_v.rbegin() = lParam;
	}
	return 0;
}
int CCsdTagLong::set_list(long long lParam)
{
	if (m_pchoice) {
		for (long i = 0; i <= m_choice_size; i++) {
			if (m_pchoice[i] == lParam) {
				*m_v.rbegin() = lParam;
			}
		}
	}
	else {
		*m_v.rbegin() = lParam;
	}
	return 0;
}
int CCsdTagLong::set(long long lParam)
{
	int out = 0;
	switch (choice_flag()) {
	default:
	case ICsdTag::CHOICE_ANY:*m_v.rbegin() = lParam;break;
	case ICsdTag::CHOICE_RANGE:out = set_range(lParam); break;
	case ICsdTag::CHOICE_LIST:out = set_list(lParam); break;
	}
	return out;
}
int CCsdTagLong::save_value(LPFNSETVALUE lpfn, void *callback_param)
{
	char *n = id_name();
	if (n) {
		char s[16];
		long v = 0;
		get((void*)&v);
		sprintf(s, "%ld", v);
		return lpfn(n, s, callback_param);
	}
	return 0;
}
int CCsdTagLong::restore_value(LPFNGETVALUE lpfn, void *callback_param)
{
	char *n = id_name();
	if (n) {
		char s[16] = { 0 };
		char sdef[16] = { 0 };
		long def = 0;
		get_default((void*)&def);
		sprintf(sdef, "%ld", def);
		int ret = lpfn(n, s, sizeof(s), sdef, callback_param);
		if (ret) {
			//do nothing
		} else {
			set(atoi(s));
		}
	}
	return 0;
}
void CCsdTagLong::save()
{
	long long v = (long long)*m_v.rbegin();
	m_v.push_back(v);
}
void CCsdTagLong::restore()
{
	if (m_v.size()>1) {
		m_v.pop_back();
	}
}
void CCsdTagLong::flush()
{
	if (m_v.size()>1) {
		long long v = *m_v.rbegin();
		m_v.pop_back();
		*m_v.rbegin() = v;
	}
}
CCsdTagLongMulti::CCsdTagLongMulti(long id, ICsdTags2 *p):CCsdTagLongBase(id, p), m_pwindow(NULL)
{
	//WriteLog((char*)"CCsdTagLongMulti::CCsdTagLongMulti() start");
	for (long i=0; i<(long)(sizeof(m_v)/sizeof(m_v[0])); i++) {
		m_v[i].push_back(0);
	}
	m_pwindow = p->get(CSDP_WINDOW);
	//WriteLog((char*)"CCsdTagLongMulti::CCsdTagLongMulti() end");
}
long CCsdTagLongMulti::cur_window()
{
	//printf("m_pwindow %x\r\n", m_pwindow);
	long max = (long)(sizeof(m_v)/sizeof(m_v[0]))/2;
	long w=0;
	m_pwindow->get(&w);
	if (w>0) {
		if (w>max) w=max; 
	} else if (w<0) {
		if (w<-max) w=-max;
	}
	//printf("current window is %ld\r\n", w);
	return w;
}
int CCsdTagLongMulti::get(long w, void *lpParam)
{
	long *pout = (long *)lpParam;
	*pout = (long)*m_v[w].rbegin();
	return 0;
}
int CCsdTagLongMulti::set_range(long w, long long lParam)
{
	if (m_pchoice) {
		for (long i = m_pchoice[0]; i <= m_pchoice[2]; i += m_pchoice[1]) {
			if (i == lParam) {
				*m_v[w].rbegin() = lParam;
			}
		}
	}
	else {
		*m_v[w].rbegin() = lParam;
	}
	return 0;
}
int CCsdTagLongMulti::set_list(long w, long long lParam)
{
	if (m_pchoice) {
		for (long i = 0; i <= m_choice_size; i++) {
			if (m_pchoice[i] == lParam) {
				*m_v[w].rbegin() = lParam;
			}
		}
	}
	else {
		*m_v[w].rbegin() = lParam;
	}
	return 0;
}
int CCsdTagLongMulti::set(long w, long long lParam)
{
	int out = 0;
	switch (choice_flag()) {
	default:
	case ICsdTag::CHOICE_ANY:*m_v[w].rbegin() = lParam; break;
	case ICsdTag::CHOICE_RANGE:out = set_range(w, lParam); break;
	case ICsdTag::CHOICE_LIST:out = set_list(w, lParam); break;
	}
	return out;
}
int CCsdTagLongMulti::get(void *lpParam)
{
	int ret = 0;
	long w = cur_window();
	if (w) {
		if (w>0) {
			w--;
		} else {
			long max = (long)(sizeof(m_v)/sizeof(m_v[0]))/2;
			w=max - 1 + (-w);
		}
		ret = get(w, lpParam);
	} else {
		ret = get(0, lpParam);
	}
	return ret;
}
int CCsdTagLongMulti::set(long long lParam)
{
	//WriteLog((char*)"CCsdTagLongMulti::set(%ld) start", lParam);
	long w = cur_window();
	if (w) {
		if (w>0) {
			w--;
		} else {
			long max = (long)(sizeof(m_v)/sizeof(m_v[0]))/2;
			w=max - 1 + (-w);
		}
		*m_v[w].rbegin()=lParam;
	} else {
		for (long i=0; i<(long)(sizeof(m_v)/sizeof(m_v[0])); i++) {
			set(i, lParam);
		}		
	}
	//WriteLog((char*)"CCsdTagLongMulti::set() end");
	return 0;
}
int CCsdTagLongMulti::save_value(LPFNSETVALUE lpfn, void *callback_param)
{
	//front m_v[0]:1,m_v[1]:2,m_v[2]:3,m_v[3]:4 back m_v[4]:1,m_v[5]:2,m_v[6]:3,v[7]:4
	char *n = id_name();
	if (n) {
		long i = 0;
		char s[16];
		char key[64];
		long long v = 0;
		//front
		for (i = 0; i < (long)(sizeof(m_v) / (2*sizeof(m_v[0]))); i++) {
			sprintf(key, "%s(%ld)", n, (long)(sizeof(m_v) / (2 * sizeof(m_v[0])))-i);
			v = *m_v[(long)(sizeof(m_v) / (2 * sizeof(m_v[0]))) - i - 1].rbegin();
			sprintf(s, "%lld", v);
			lpfn(key, s, callback_param);
		}
		for (long j=-1; i < (long)(sizeof(m_v) / sizeof(m_v[0])); i++, j--) {			
			sprintf(key, "%s(%ld)", n, j);
			v = *m_v[i].rbegin();
			sprintf(s, "%lld", v);
			lpfn(key, s, callback_param);
		}
	}
	return 0;
}
int CCsdTagLongMulti::restore_value(LPFNGETVALUE lpfn, void *callback_param)
{
	char *n = id_name();
	if (n) {
		long i = 0, j = 0;
		char s[16] = { 0 };
		char key[64];
		char sdef[16] = { 0 };
		long def = 0;
		get_default((void*)&def);
		sprintf(sdef, "%ld", def);
		//front
		for (i = 0; i < (long)((sizeof(m_v) / sizeof(m_v[0]))/2); i++) {
			sprintf(key, "%s(%ld)", n, i+1);
			memset(s, 0, sizeof(s));
			int ret = lpfn(key, s, sizeof(s), sdef, callback_param);
			if (ret) {
				//do nothing
			}
			else {
				set(i, atoi(s));
			}
		}
		long max = sizeof(m_v) / sizeof(m_v[0]);
		for (i = max/2, j=-1; i <max; i++, j--) {
			sprintf(key, "%s(%ld)", n, j);
			memset(s, 0, sizeof(s));
			int ret = lpfn(key, s, sizeof(s), sdef, callback_param);
			if (ret) {
				//do nothing
			}
			else {
				set(i, atoi(s));
			}
		}
	}
	return 0;
}
void CCsdTagLongMulti::save()
{
	for (long i=0; i<(long)(sizeof(m_v)/sizeof(m_v[0])); i++) {
		long long v = *m_v[i].rbegin();
		m_v[i].push_back(v);
	}
}
void CCsdTagLongMulti::restore()
{
	for (long i=0; i<(long)(sizeof(m_v)/sizeof(m_v[0])); i++) {
		if (m_v[i].size()>1) {
			m_v[i].pop_back();
		}
	}
}
void CCsdTagLongMulti::flush()
{
	for (long i=0; i<(long)(sizeof(m_v)/sizeof(m_v[0])); i++) {
		if (m_v[i].size()>1) {
			long long v = *m_v[i].rbegin();
			m_v[i].pop_back();
			*m_v[i].rbegin() = v;
		}
	}
}	
CCsdTagLongMulti::~CCsdTagLongMulti()
{
}
CCsdTagLongScan::CCsdTagLongScan(long id, ICsdTags2 *p, IScanCtrl *pscan):CCsdTagLongBase(id, p), m_pscan(pscan)
{}
CCsdTagLongScan::~CCsdTagLongScan()
{}
int CCsdTagLongScan::get(void *lpParam)
{
	return (int)m_pscan->get_information(id(), lpParam);
}
int CCsdTagLongScan::set(long long lParam)
{
	return 0;
}
void CCsdTagLongScan::save()
{}
void CCsdTagLongScan::restore()
{}
void CCsdTagLongScan::flush()
{}
CCsdTagLongScanner::CCsdTagLongScanner(long id, ICsdTags2 *p, IVirtualScanner *pscanner):CCsdTagLongBase(id, p), m_pscanner(pscanner)
{}
CCsdTagLongScanner::~CCsdTagLongScanner()
{}
int CCsdTagLongScanner::get(void *lpParam)
{
	return 0;
}
int CCsdTagLongScanner::set(long long lParam)
{
	return 0;
}
int CCsdTagLongScanner::set_default()
{
	return 0;
}
void CCsdTagLongScanner::save()
{}
void CCsdTagLongScanner::restore()
{}
void CCsdTagLongScanner::flush()
{}
CCsdTagAsciBase::CCsdTagAsciBase(long id, ICsdTags2 *p):CCsdTagBase(id, p)
{
}
CCsdTagAsciBase::~CCsdTagAsciBase()
{
}
ICsdTag::CSDTAG_CHOICE_FLAG CCsdTagAsciBase::choice_flag()
{
	return ICsdTag::CHOICE_ANY;
}
CCsdTagAsciScan::CCsdTagAsciScan(long id, ICsdTags2 *p, IScanCtrl *pscan):CCsdTagAsciBase(id, p), m_pscan(pscan)
{}
CCsdTagAsciScan::~CCsdTagAsciScan()
{}
int CCsdTagAsciScan::get(void *lpParam)
{
	return (int)m_pscan->get_information(id(), lpParam);
}
int CCsdTagAsciScan::set(long long lParam)
{
	return 0;
}
void CCsdTagAsciScan::save()
{}
void CCsdTagAsciScan::restore()
{}
void CCsdTagAsciScan::flush()
{}
CCsdTagAsciScanner::CCsdTagAsciScanner(long id, ICsdTags2 *p, IVirtualScanner *pscanner):CCsdTagAsciBase(id, p), m_pscanner(pscanner)
{}
CCsdTagAsciScanner::~CCsdTagAsciScanner()
{}
int CCsdTagAsciScanner::get(void *lpParam)
{
	return 0;
}
int CCsdTagAsciScanner::set(long long lParam)
{
	return 0;
}
void CCsdTagAsciScanner::save()
{}
void CCsdTagAsciScanner::restore()
{}
void CCsdTagAsciScanner::flush()
{}
CCsdTagAsci::CCsdTagAsci(long id, ICsdTags2 *p) :CCsdTagAsciBase(id, p), m_def("")
{
	m_v.push_back("");
}
CCsdTagAsci::~CCsdTagAsci()
{
}
int CCsdTagAsci::get(void* lpParam)
{
	char* p = (char*)lpParam;
	strcpy(p, m_v.rbegin()->c_str());
	return 0;
}
int CCsdTagAsci::get_default(void* lpParam)
{
	char* p = (char*)lpParam;
	strcpy(p, m_def.c_str());
	return 0;
}
int CCsdTagAsci::set(long long lParam)
{
	char* p = (char*)lParam;
	*m_v.rbegin() = p;
	return 0;
}
int CCsdTagAsci::set_default()
{
	return set((long long)m_def.c_str());
}
int CCsdTagAsci::change_default(LPFNGETVALUE lpfn, void *callback_param)
{
	char *n = id_name();
	if (n) {
		char s[64] = { 0 };
		int ret = lpfn(n, s, sizeof(s), (char*)m_def.c_str(), callback_param);
		if (ret) {
			//do nothing
		}
		else {
			m_def = s;
		}
	}
	return 0;
}
int CCsdTagAsci::save_value(LPFNSETVALUE lpfn, void *callback_param)
{
	char *n = id_name();
	if (n) {
		char s[512] = { 0 };
		this->get((void*)s);
		return lpfn(n, s, callback_param);
	}
	return 0;
}
int CCsdTagAsci::restore_value(LPFNGETVALUE lpfn, void *callback_param)
{
	char *n = id_name();
	if (n) {
		char s[512] = { 0 };
		char sdef[512] = { 0 };
		get_default((void*)sdef);
		int ret = lpfn(n, s, sizeof(sdef), sdef, callback_param);
		if (ret) {
			//do nothing
		}
		else {
			set((long long)s);
		}
	}
	return 0;
}
void CCsdTagAsci::save()
{
	std::string v = *m_v.rbegin();
	m_v.push_back(v);
}
void CCsdTagAsci::restore()
{
	if (m_v.size() > 1) {
		m_v.pop_back();
	}
}
void CCsdTagAsci::flush()
{
	if (m_v.size() > 1) {
		std::string v = *m_v.rbegin();
		m_v.pop_back();
		*m_v.rbegin() = v;
	}
}
