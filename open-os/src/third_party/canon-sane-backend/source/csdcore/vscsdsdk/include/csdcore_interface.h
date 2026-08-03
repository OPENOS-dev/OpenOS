/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef __CSDCORE_virtual_intERFACE_DEFINED_HEADER__DEFINED__
#define __CSDCORE_virtual_intERFACE_DEFINED_HEADER__DEFINED__

#include "sdk_def.h"
#include "image_interface.h"

class ICsdCore : public IUnknown
{
public:
	virtual int probe(LPVSCSD_SDK_INIT_INFORMATION pinfo)=0;
	virtual int terminate()=0;
	virtual int tagget(int tagid, void *lpParam)=0;
	virtual int tagset(int tagid, long long lParam)=0;
	virtual int tagget_choice_flag(int tagid, long *lpFlag) = 0;
	virtual int tagget_choice_count(int tagid, long *lpCount)=0;
	virtual int tagget_choice(int tagid, int iIndex, void *lpVoid)=0;
	typedef int(STDMETHODCALLTYPE *LPFNSETVALUE)(const char *key, char *pin, void *callback_param);
	typedef int(STDMETHODCALLTYPE *LPFNGETVALUE)(const char *key, char *pout, long size/*of pout*/, char *def, void *callback_param);
	virtual int tagchange_default(LPFNGETVALUE lpfn, void *callback_param=NULL) = 0;
	virtual int tagchange_default(int tagid, void* lpParam) = 0;
	virtual int tagget_default(int tagid, void *lpParam) = 0;
	virtual int tagset_default(int tagid) = 0;
	virtual int save_value(LPFNSETVALUE lpfn, void *callback_param=NULL) = 0;
	virtual int restore_value(LPFNGETVALUE lpfn, void* callback_param=NULL) = 0;
	virtual void save_value(int tagid)=0;
	virtual void restore_value(int tagid)=0;
	virtual void flush_value(int tagid)=0;
	virtual int scan_start()=0;
	virtual int prescan_start() = 0;
	virtual int image(ICeiImage **ppOut)=0;
	virtual int clear_image(char *pimg_ptr)=0;
	virtual int scan_end()=0;
	virtual int stop()=0;
	virtual int abort()=0;
	typedef enum tagCSDTAG_CHOICE_FLAG {
		CHOICE_ANY = 0,
		CHOICE_RANGE = 1,
		CHOICE_LIST
	}CSDTAG_CHOICE_FLAG;
	typedef enum tagCSDTAG_CHOICE_RANGE {
		RANGE_LOW = -1,
		RANGE_STEP = -2,
		RANGE_HIGH = -3
	}CSDTAG_CHOIE_RANGE;
	typedef enum tagCSDPASSTHRU
	{
		PTID_VIRTUAL_SCANNER=0,
	}CSDPASSTHRU;
	virtual int get_passthru(long id, void* p) = 0;
	virtual int set_passthru(long id, void* p) = 0;
};

#endif