/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <cstddef>
#include <memory>
#include <vector>
#include "ceilogwrite.h"
#include "tags_interface.h"
#include "tag_enum_interface.h"

class CCsdTags : public ICsdTags2
{
public:
	CCsdTags(IScanCtrl *pscan, IVirtualScanner *pscanner, IUnknown *h);
	virtual ~CCsdTags();
	long STDMETHODCALLTYPE QueryInterface(REFIID id, void **ppOut);
	unsigned long STDMETHODCALLTYPE AddRef();
	unsigned long STDMETHODCALLTYPE Release();
public:
	int get(int tagid, void *lpParam);
	int get_default(int tagid, void *lpParam);
	int set(int tagid, long long lParam);
	int set_default(int tagid);
	int change_default(int tagid, long long lParam);
	int choice_flag(int tagid, long *lpFlag);
	int choice_count(int tagid, long *lpCount);
	int choice(int tagid, int index, void *lpParam);
	int change_default(LPFNGETVALUE lpfn, void *callback_param);
	int save_value(LPFNSETVALUE lpfn, void* callback_param);
	int restore_value(LPFNGETVALUE lpfn, void* callback_param);
	void save_value(int tagid);
	void restore_value(int tagid);
	void flush_value(int tagid);	

	void update(ICsdTag *sender);

	long init();
	void uninit();

public:
	ICsdTag *get(int tagid);
	void add(ICsdTag *tag);
private:
	void notify(ICsdTag *sender);
	void notify_def(ICsdTag *sender);
private:
	long m_ref;
	typedef std::vector< ICsdTag* > TAGLIST;
	TAGLIST m_tags;
	IUnknown *m_handle;
	IScanCtrl *m_pscan;
	IVirtualScanner *m_pscanner;
};
CCsdTags::CCsdTags(IScanCtrl *pscan, IVirtualScanner *pscanner, IUnknown *h):m_ref(1), m_handle(h), m_pscan(pscan), m_pscanner(pscanner)
{
	//WriteLog((char*)"CCsdTags::CCsdTags()");
}
CCsdTags::~CCsdTags()
{
	//WriteLog((char*)"CCsdTags::~CCsdTags() start");
	uninit();
	//WriteLog((char*)"CCsdTags::~CCsdTags() end");
}
long CCsdTags::QueryInterface(REFIID id, void **ppOut)
{
	return -1;
}
unsigned long CCsdTags::AddRef()
{
	m_ref++;
	return m_ref;
}
unsigned long CCsdTags::Release()
{
	m_ref--;
	if (m_ref<=0) {
		delete this;
		return 0;
	}
	return m_ref;
}
void CCsdTags::add(ICsdTag *tag)
{
	m_tags.push_back(tag);
}
long CCsdTags::init()
{
	//WriteLog((char*)"CCsdTags::init() start");
	enum_csdtags(this, m_pscan, m_pscanner, m_handle);
	//WriteLog((char*)"m_tags.size() %d", m_tags.size());
	//WriteLog((char*)"CCsdTags::init() end");
	return 0;
}
void CCsdTags::uninit()
{
	//WriteLog((char*)"CCsdTags::uninit() start");
	TAGLIST::reverse_iterator itr = m_tags.rbegin();
	for (;itr!=m_tags.rend(); itr++) {
		(*itr)->Release();
	}
	//WriteLog((char*)"CCsdTags::uninit() end");
}

ICsdTag *CCsdTags::get(int tagid)
{
	ICsdTag *out = NULL;
	//WriteLog((char*)"CCsdTags::get(%d) start", tagid);
	TAGLIST::iterator itr =m_tags.begin();
	for (;itr!=m_tags.end(); itr++) {
		if ((*itr)->id()==tagid) {
			out = (*itr);
			//WriteLog((char*)"%d is found:%x", tagid, out);
			break;
		}
	}
	//WriteLog((char*)"CCsdTags::get() end");
	return out;
}
int CCsdTags::get(int tagid, void *lpParam)
{
	ICsdTag *ptag = CCsdTags::get(tagid);
	if (ptag) {
		return ptag->get(lpParam);
	}
	return -1;
}
int CCsdTags::get_default(int tagid, void *lpParam)
{
	TAGLIST::iterator itr =m_tags.begin();
	for (;itr!=m_tags.end(); itr++) {
		if ((*itr)->id()==tagid) return (*itr)->get_default(lpParam);
	}
	return -1;	
}
int CCsdTags::set(int tagid, long long lParam)
{
	TAGLIST::iterator itr =m_tags.begin();
	for (;itr!=m_tags.end(); itr++) {
		if ((*itr)->id()==tagid) {
			int ret = (*itr)->set(lParam);
			notify((*itr));
			return ret;
		}
	}
	return -1;
}
int CCsdTags::set_default(int tagid)
{
	if (tagid) {
		TAGLIST::iterator itr =m_tags.begin();
		for (;itr!=m_tags.end(); itr++) {
			if ((*itr)->id()==tagid) return (*itr)->set_default();
		}
		return  -1;
	} else {
		TAGLIST::iterator itr =m_tags.begin();
		for (;itr!=m_tags.end(); itr++) {
			(*itr)->set_default();
		}
	}
	return 0;
}
int CCsdTags::change_default(int tagid, long long lParam)
{
	TAGLIST::iterator itr = m_tags.begin();
	for (; itr != m_tags.end(); itr++) {
		if ((*itr)->id() == tagid) {
			int ret = (*itr)->change_default(lParam);
			notify_def((*itr));
			return ret;
		}
	}
	return  -1;
}
int CCsdTags::choice_flag(int tagid, long *lpFlag)
{
	if (tagid) {
		ICsdTag *p = get(tagid);
		if (p) return p->choice_flag(lpFlag);
		return  -1;
	}
	else {
		return  -1;
	}
	return 0;
}
int CCsdTags::choice_count(int tagid, long *lpCount)
{
	if (tagid) {
		TAGLIST::iterator itr =m_tags.begin();
		for (;itr!=m_tags.end(); itr++) {
			if ((*itr)->id() == tagid) {
				return (*itr)->choice_count(lpCount);
			}
		}
		return  -1;
	} else {
		*lpCount = (long)m_tags.size();
	}
	return 0;	
}
int CCsdTags::choice(int tagid, int index, void *lpParam)
{
	//WriteLog((char*)"CCsdTags::choice(%d, %d, 0x%x) start", tagid, index, lpParam);
	int out = 0;
	if (tagid) {
		TAGLIST::iterator itr =m_tags.begin();
		for (;itr!=m_tags.end(); itr++) {
			if ((*itr)->id() == tagid) {
				//WriteLog("(*itr)->choice(%d, 0x%x)", index, lpParam);
				return (*itr)->choice(index, lpParam);
			}
		}
		out = -1;
	} else {
		if (index<(int)m_tags.size()) {
			long *pout = (long*)lpParam;
			*pout = m_tags[index]->id();
		} else {
			out = -1;
		}
	}
	//WriteLog((char*)"CCsdTags::choice() end %d", out);
	return out;
}
void CCsdTags::update(ICsdTag *sender)
{
	TAGLIST::iterator itr =m_tags.begin();
	for (;itr!=m_tags.end(); itr++) {
		if (sender!=(*itr)) (*itr)->update(sender);
	}
}
int CCsdTags::change_default(LPFNGETVALUE lpfn, void *callback_param)
{
	int ret = 0;
	TAGLIST::iterator itr = m_tags.begin();
	for (; itr != m_tags.end(); itr++) {
		ret = (*itr)->change_default(lpfn, callback_param);
		if (ret) return ret;
	}
	return 0;
}
int CCsdTags::save_value(LPFNSETVALUE lpfn, void* callback_param)
{
	TAGLIST::iterator itr = m_tags.begin();
	for (; itr != m_tags.end(); itr++) {
		(*itr)->save_value(lpfn, callback_param);
	}
	return 0;
}
int CCsdTags::restore_value(LPFNGETVALUE lpfn, void* callback_param)
{
	TAGLIST::iterator itr = m_tags.begin();
	for (; itr != m_tags.end(); itr++) {
		(*itr)->restore_value(lpfn, callback_param);
	}
	return 0;
}
void CCsdTags::save_value(int tagid)
{
	if (tagid) {
		TAGLIST::iterator itr =m_tags.begin();
		for (;itr!=m_tags.end(); itr++) {
			if (tagid==(*itr)->id()) {
				(*itr)->save();
				break;
			}
		}
	} else {
		TAGLIST::iterator itr =m_tags.begin();
		for (;itr!=m_tags.end(); itr++) {
			//WriteLog((char*)"(%d)->save() start", (*itr)->id());
			(*itr)->save();
			//WriteLog((char*)"(%d)->save() end", (*itr)->id());
		}	
	}
}
void CCsdTags::restore_value(int tagid)
{
	if (tagid) {
		TAGLIST::iterator itr =m_tags.begin();
		for (;itr!=m_tags.end(); itr++) {
			if (tagid==(*itr)->id()) {
				(*itr)->restore();
				break;
			}
		}
	} else {
		TAGLIST::iterator itr =m_tags.begin();
		for (;itr!=m_tags.end(); itr++) {
			(*itr)->restore();
		}	
	}
}
void CCsdTags::flush_value(int tagid)
{
	if (tagid) {
		TAGLIST::iterator itr =m_tags.begin();
		for (;itr!=m_tags.end(); itr++) {
			if (tagid==(*itr)->id()) {
				(*itr)->flush();
				break;
			}
		}
	} else {
		TAGLIST::iterator itr =m_tags.begin();
		for (;itr!=m_tags.end(); itr++) {
			(*itr)->flush();
		}	
	}
}
void CCsdTags::notify(ICsdTag *sender)
{
	TAGLIST::iterator itr =m_tags.begin();
	for (;itr!=m_tags.end(); itr++) {
		(*itr)->update(sender);
	}	
}
void CCsdTags::notify_def(ICsdTag* sender)
{
	TAGLIST::iterator itr = m_tags.begin();
	for (; itr != m_tags.end(); itr++) {
		(*itr)->update_def(sender);
	}
}
ICsdTags *csdtags(IScanCtrl *pscan, IVirtualScanner *pscanner, IUnknown *h)
{
	//WriteLog((char*)"csdtag() start");
	std::unique_ptr<CCsdTags>ptags(new CCsdTags(pscan, pscanner, h));
	if (ptags.get()==NULL) return NULL;
	ptags->init();
	//WriteLog((char*)"csdtag() end");
	return (ICsdTags*)ptags.release();
}

