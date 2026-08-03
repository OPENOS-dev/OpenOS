/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef __CSD_CORE_TAGS_INTERFACE_DEFINED_HEADER__
#define __CSD_CORE_TAGS_INTERFACE_DEFINED_HEADER__

#include "scanctrl_interface.h"
#include "virtual_scanner_interface.h"
#include "tag_interface.h"

class ICsdTags : public IUnknown
{
public:
	virtual int get(int tagid, void *lpParam)=0;
	virtual int get_default(int tagid, void *lpParam)=0;
	virtual int set(int tagid, long long lParam)=0;
	virtual int set_default(int tagid)=0;
	virtual int change_default(int tagid, long long lParam) = 0;
	virtual int choice_flag(int tagid, long *lpFlag) = 0;
	virtual int choice_count(int tagid, long *lpCount)=0;
	virtual int choice(int tagid, int index, void *lpParam)=0;
	typedef int(STDMETHODCALLTYPE *LPFNSETVALUE)(const char *key, char *pin, void *callback_param);
	typedef int(STDMETHODCALLTYPE *LPFNGETVALUE)(const char *key, char *pout, long size/*of pout*/, char *def, void *callback_param);
	virtual int change_default(LPFNGETVALUE lpfn, void *callback_param)=0;
	virtual int save_value(LPFNSETVALUE lpfn, void* callback_param)=0;
	virtual int restore_value(LPFNGETVALUE lpfn, void* callback_param)=0;
	virtual void save_value(int tagid)=0;
	virtual void restore_value(int tagid)=0;
	virtual void flush_value(int tagid)=0;
};

class ICsdTags2 : public ICsdTags
{
public:
	virtual ICsdTag *get(int tagid)=0;
 	virtual void add(ICsdTag *)=0;
};

ICsdTags *csdtags(IScanCtrl *pscan, IVirtualScanner *pscanner, IUnknown *handle);

#endif