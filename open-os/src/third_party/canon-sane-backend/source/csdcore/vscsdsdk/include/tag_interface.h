/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef __CSD_CORE_TAG_INTERFACE_DEFINED_HEADER__
#define __CSD_CORE_TAG_INTERFACE_DEFINED_HEADER__


#include "unknown.h"

class ICsdTag : public IUnknown
{
public:
	virtual int id()=0;
	virtual void update(ICsdTag *sender)=0;
	virtual void update_def(ICsdTag *sender)=0;
	virtual int get(void *lpParam)=0;
	virtual int get_default(void *lpParam)=0;
	virtual int set(long long lParam)=0;
	virtual int set_default()=0;
	virtual int change_default(long long lParam) = 0;
	virtual int choice_flag(long *lpFlag) = 0;
	virtual int choice_count(long *lpCount)=0;
	virtual int choice(int index, void *lpParam)=0;
	typedef enum tagCSDTAG_CHOICE_FLAG{
		CHOICE_ANY=0,
		CHOICE_RANGE=1,
		CHOICE_LIST
	}CSDTAG_CHOICE_FLAG;
	typedef enum tagCSDTAG_CHOICE_RANGE{
		RANGE_LOW=-1,
		RANGE_STEP=-2,
		RANGE_HIGH=-3
	}CSDTAG_CHOICE_RANGE;
	virtual CSDTAG_CHOICE_FLAG choice_flag()=0;
	typedef int(STDMETHODCALLTYPE *LPFNSETVALUE)(const char *key, char *pin, void *callback_param);
	typedef int(STDMETHODCALLTYPE *LPFNGETVALUE)(const char *key, char *pout, long size/*of pout*/, char *def, void *callback_param);
	virtual int save_value(LPFNSETVALUE lpfn, void *callback_param) = 0;
	virtual int change_default(LPFNGETVALUE lpfn, void *callback_param) = 0;
	virtual int restore_value(LPFNGETVALUE lpfn, void *callback_param) = 0;
	virtual void save()=0;
	virtual void restore()=0;
	virtual void flush()=0;
};



#endif