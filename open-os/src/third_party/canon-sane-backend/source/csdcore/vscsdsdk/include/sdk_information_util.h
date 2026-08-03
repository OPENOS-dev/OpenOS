/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef __SDK_IMAGE_INFORMATION_INTERFACE_CLASS_HEADER_DEFINED__
#define __SDK_IMAGE_INFORMATION_INTERFACE_CLASS_HEADER_DEFINED__

#include <vector>
#include <string>
#include "command.h"
#include "image_info_interface.h"
#include "tag_interface.h"

class CCeiImageInformationCmd : public ICeiImageInformation 
{
public:
	CCeiImageInformationCmd();
	virtual ~CCeiImageInformationCmd();
	long STDMETHODCALLTYPE QueryInterface(REFIID id, void **ppOut);
	unsigned long STDMETHODCALLTYPE AddRef();
	unsigned long STDMETHODCALLTYPE Release();	
	int information(long id, void *);
public:
	void set(CStreamCmd *pin);
private:
	long m_ref;
	typedef std::vector<CStreamCmd *> INFOLIST;
	INFOLIST m_list;
};
class CCeiImageInformationTag : public ICeiImageInformation 
{
public:
	CCeiImageInformationTag();
	virtual ~CCeiImageInformationTag();
	long STDMETHODCALLTYPE QueryInterface(REFIID id, void **ppOut);
	unsigned long STDMETHODCALLTYPE AddRef();
	unsigned long STDMETHODCALLTYPE Release();	
	int information(long id, void *);
public:
	void set(ICsdTag *pin);
private:
	long m_ref;
	typedef std::vector<ICsdTag *> INFOLIST;
	INFOLIST m_list;
};
class CCeiVolatileCsdTagBase: public ICsdTag
{
public:
	CCeiVolatileCsdTagBase(int id);
	virtual ~CCeiVolatileCsdTagBase();
	long STDMETHODCALLTYPE QueryInterface(REFIID id, void **ppOut);
	unsigned long STDMETHODCALLTYPE AddRef();
	unsigned long STDMETHODCALLTYPE Release();
	int id();
	void update(ICsdTag *sender);
	void update_def(ICsdTag *sender);
	int get_default(void *lpParam);
	int set_default();
	int change_default(long long lParam);
	int change_default(LPFNGETVALUE lpfn, void *callback_param);
	int choice_flag(long *lpFlag);
	int choice_count(long *lpCount);
	int choice(int index, void *lpParam);
	ICsdTag::CSDTAG_CHOICE_FLAG choice_flag();
	int save_value(LPFNSETVALUE lpfn, void *callback_param);
	int restore_value(LPFNGETVALUE lpfn, void *callback_param);
	void save();
	void restore();
	void flush();	
private:
	long m_ref;
	int m_id;
};
 class CCeiVolatileCsdTagLong : public CCeiVolatileCsdTagBase
{
public:
	CCeiVolatileCsdTagLong(int id);
	virtual ~CCeiVolatileCsdTagLong();
	int get(void *lpParam);
	int set(long long lParam);
private:
	long long m_v;
};
class CCeiVolatileCsdTagAsci : public CCeiVolatileCsdTagBase
{
public:
	CCeiVolatileCsdTagAsci(int id);
	virtual ~CCeiVolatileCsdTagAsci();
	int get(void *lpParam);
	int set(long long lParam);
private:
	std::string m_v;
};
CCeiImageInformationCmd *create_vscsdsdk_information_cmd();
CCeiImageInformationTag *create_vscsdsdk_information_tag();
CCeiVolatileCsdTagLong *create_longtag(int id, long v);
CCeiVolatileCsdTagAsci *create_ascitag(int id, char *v);

#endif