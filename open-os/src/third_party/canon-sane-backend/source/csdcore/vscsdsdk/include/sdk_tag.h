/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef __CSDSDK_TAG_DEFINE_BASE_HEADER__
#define __CSDSDK_TAG_DEFINE_BASE_HEADER__

#include <list>
#include <string>
#include "tags_interface.h"
#include "scanctrl_interface.h"
#include "virtual_scanner_interface.h"

class CCsdTagBase : public ICsdTag
{
public:
	CCsdTagBase(long id, ICsdTags2 *parent);
	virtual ~CCsdTagBase();
	long STDMETHODCALLTYPE QueryInterface(REFIID id, void **ppOut);
	unsigned long STDMETHODCALLTYPE AddRef();
	unsigned long STDMETHODCALLTYPE Release();
 	int id();
	virtual char *id_name();
	virtual void update(ICsdTag *sender);		
	virtual void update_def(ICsdTag *sender);		
	virtual int get(void *lpParam);
	virtual int get_default(void *lpParam);
	virtual int set(long long lParam);
	virtual int set_default();
	virtual int change_default(long long lParam);
	virtual int change_default(LPFNGETVALUE lpfn, void *callback_param);
	virtual int choice_flag(long *lpFlag);
	virtual int choice_count(long *lpCount);
	virtual int choice(int index, void *lpParam);
	virtual ICsdTag::CSDTAG_CHOICE_FLAG choice_flag();
	virtual int save_value(LPFNSETVALUE lpfn, void* callback_param);
	virtual int restore_value(LPFNGETVALUE lpfn, void* callback_param);
	virtual void save();
	virtual void restore();
	virtual void flush();
	
	enum {
		LOW = 0,
		STEP,
		HIGH,
		CHOICE_COUNT3
	};
protected:
	ICsdTags2 *parent();
private:
	long m_ref;
	long m_id;
	ICsdTags2 *m_parent;
};
class CCsdTagLongBase : public CCsdTagBase
{
public:
	CCsdTagLongBase(long id, ICsdTags2 *parent);
	virtual ~CCsdTagLongBase();
public:
	int get_default(void *lpParam);
	virtual int set_default();
	int change_default(long long lParam);
	int change_default(LPFNGETVALUE lpfn, void *callback_param);
	int choice_count(long *lpCount);
	int choice(int index, void *lpParam);
private:
		int choice_any(int index, void *lpParam);
		int choice_range(int index, void *lpParam);
		int choice_list(int index, void *lpParam);	
protected:
	long m_def;
	long *m_pchoice;
	unsigned char m_choice_size;
};
class CCsdTagLong : public CCsdTagLongBase
{
public:
	CCsdTagLong(long id, ICsdTags2 *parent);
	virtual ~CCsdTagLong();
protected:
	virtual int get(void *lpParam);
	virtual int set(long long lParam);
	virtual int save_value(LPFNSETVALUE lpfn, void *callback_param);
	virtual int restore_value(LPFNGETVALUE lpfn, void *callback_param);
	void save();
	void restore();
	void flush();
protected:
	int set_range(long long lParam);
	int set_list(long long lParam);
	typedef std::list<long long> LONGLIST;
	LONGLIST m_v;
};
class CCsdTagLongMulti : public CCsdTagLongBase
{
public:
	CCsdTagLongMulti(long id, ICsdTags2 *parent);
	virtual ~CCsdTagLongMulti();
protected:
	virtual int get(void *lpParam);
	virtual int set(long long lParam);
	virtual int save_value(LPFNSETVALUE lpfn, void *callback_param);
	virtual int restore_value(LPFNGETVALUE lpfn, void* callback_param);
	void save();
	void restore();
	void flush();
private:
	long cur_window();
	int get(long w, void *lpParam);
	int set(long w, long long lParam);
	int set_range(long w, long long lParam);
	int set_list(long w, long long lParam);
protected:
	typedef std::list<long long> LONGLIST;
	LONGLIST m_v[8];//front m_v[0]:1,m_v[1]:2,m_v[2]:3,m_v[3]:4 back m_v[4]:1,m_v[5]:2,m_v[6]:3,v[7]:4
	ICsdTag *m_pwindow;
};
class CCsdTagLongScan : public CCsdTagLongBase
{
public:
	CCsdTagLongScan(long id, ICsdTags2 *parent, IScanCtrl *pscan);
	virtual ~CCsdTagLongScan();
protected:
	virtual int get(void *lpParam);
	virtual int set(long long lParam);
	void save();
	void restore();
	void flush();
protected:
	IScanCtrl *m_pscan;
};
class CCsdTagLongScanner : public CCsdTagLongBase
{
public:
	CCsdTagLongScanner(long id, ICsdTags2 *parent, IVirtualScanner *pscanner);
	virtual ~CCsdTagLongScanner();
protected:
	virtual int get(void *lpParam);
	virtual int set(long long lParam);
	virtual int set_default();
	void save();
	void restore();
	void flush();
protected:
	IVirtualScanner *m_pscanner;
};
class CCsdTagAsciBase : public CCsdTagBase
{
public:
	CCsdTagAsciBase(long id, ICsdTags2 *parent);
	virtual ~CCsdTagAsciBase();
	virtual ICsdTag::CSDTAG_CHOICE_FLAG choice_flag();
};
class CCsdTagAsci : public CCsdTagAsciBase
{
public:
	CCsdTagAsci(long id, ICsdTags2 *parent);
	virtual ~CCsdTagAsci();
	virtual int get(void* lpParam);
	virtual int get_default(void* lpParam);
	virtual int set(long long lParam);
	virtual int set_default();
	virtual int save_value(LPFNSETVALUE lpfn, void *callback_param);
	virtual int restore_value(LPFNGETVALUE lpfn, void* callback_param);
	virtual int change_default(LPFNGETVALUE lpfn, void* callback_param);
	virtual void save();
	virtual void restore();
	virtual void flush();
protected:
	typedef std::list< std::string > STRINGLIST;
	STRINGLIST m_v;
	std::string m_def;
};
class CCsdTagAsciScan : public CCsdTagAsciBase
{
public:
	CCsdTagAsciScan(long id, ICsdTags2 *parent, IScanCtrl *pscan);
	virtual ~CCsdTagAsciScan();
protected:
	virtual int get(void *lpParam);
	virtual int set(long long lParam);
	void save();
	void restore();
	void flush();
protected:
	IScanCtrl *m_pscan;
};
class CCsdTagAsciScanner : public CCsdTagAsciBase
{
public:
	CCsdTagAsciScanner(long id, ICsdTags2 *parent, IVirtualScanner *pscanner);
	virtual ~CCsdTagAsciScanner();
protected:
	virtual int get(void *lpParam);
	virtual int set(long long lParam);
	void save();
	void restore();
	void flush();
protected:
	IVirtualScanner *m_pscanner;
};
#endif