/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <cstddef>
#include <stdlib.h>
#include "ceilogwrite.h"
#include "sdk_information_util.h"
CCeiImageInformationCmd::CCeiImageInformationCmd():m_ref(1)
{
}
CCeiImageInformationCmd::~CCeiImageInformationCmd()
{
	INFOLIST::iterator itr = m_list.begin();
	for (;itr!=m_list.end(); itr++) {
		delete (*itr);
	}
	m_list.clear();
}
long CCeiImageInformationCmd::QueryInterface(REFIID id, void **ppOut)
{
	return -1;
}
unsigned long CCeiImageInformationCmd::AddRef()
{
	m_ref++;
	return m_ref;
}
unsigned long CCeiImageInformationCmd::Release()
{
	m_ref--;
	if (m_ref<=0) {
		delete this;
		return 0;
	}
	return m_ref;
}
int CCeiImageInformationCmd::information(long , void *p)
{
	//SDKWriteLog((char*)"CCeiImageInformationCmd::information() start");
	if (p==NULL) SDKWriteLog((char*)"p is NULL");
	CStreamCmd *pin = (CStreamCmd *)p;
	INFOLIST::iterator itr = m_list.begin();
	for (;itr!=m_list.end(); itr++) {
		if (pin->cdb()[0]==(*itr)->cdb()[0] &&
			pin->transfer_data_type()==(*itr)->transfer_data_type() &&
			pin->transfer_identification()==(*itr)->transfer_identification()) {
			//SDKWriteLog((char*)"copy in");
			*pin = *(*itr);
			//SDKWriteLog((char*)"copy out");
			break;
		}
	}
	//SDKWriteLog((char*)"CCeiImageInformationCmd::information() end");
	return 0;
}
void CCeiImageInformationCmd::set(CStreamCmd *pin)
{
	bool badd = true;
	INFOLIST::iterator itr = m_list.begin();
	for (; itr != m_list.end(); itr++) {
		if (pin->cdb()[0] == (*itr)->cdb()[0] &&
			pin->transfer_data_type() == (*itr)->transfer_data_type() &&
			pin->transfer_identification() == (*itr)->transfer_identification()) {
			//SDKWriteLog((char*)"copy in");
			delete (*itr);
			(*itr)=pin;
			badd = false;
			//SDKWriteLog((char*)"copy out");
			break;
		}
	}
	if (badd) m_list.push_back(pin);
}
CCeiImageInformationTag::CCeiImageInformationTag():m_ref(1)
{
}
CCeiImageInformationTag::~CCeiImageInformationTag()
{
	INFOLIST::iterator itr = m_list.begin();
	for (; itr != m_list.end(); itr++) {
		if (*itr)(*itr)->Release();
	}
	m_list.clear();
}
long CCeiImageInformationTag::QueryInterface(REFIID id, void **ppOut)
{
	return -1;
}
unsigned long CCeiImageInformationTag::AddRef()
{
	m_ref++;
	return m_ref;
}
unsigned long CCeiImageInformationTag::Release()
{
	m_ref--;
	if (m_ref<=0) {
		delete this;
		return 0;
	}
	return m_ref;
}
int CCeiImageInformationTag::information(long id, void *p)
{
	//WriteLog((char*)"CCeiImageInformationTag::information(%ld, p) start", id);
	INFOLIST::iterator itr = m_list.begin();
	for (;itr!=m_list.end(); itr++) {
		if ((*itr)->id()==id) {
			(*itr)->get(p);
			break;
		}
	}
	//WriteLog((char*)"CCeiImageInformationTag::information(%ld, p) end", id);
	return 0;	
}
void CCeiImageInformationTag::set(ICsdTag *pin)
{
	m_list.push_back(pin);
}
CCeiImageInformationCmd *create_vscsdsdk_information_cmd()
{
	return new CCeiImageInformationCmd;
}
CCeiImageInformationTag *create_vscsdsdk_information_tag()
{
	return new CCeiImageInformationTag;
}
////////////////////////////////////////////
//
//
///////////////////////////////////////////
CCeiVolatileCsdTagBase::CCeiVolatileCsdTagBase(int id):m_ref(1), m_id(id)
{}
CCeiVolatileCsdTagBase::~CCeiVolatileCsdTagBase()
{}
long CCeiVolatileCsdTagBase::QueryInterface(REFIID id, void **ppOut)
{
	return -1;
}
unsigned long CCeiVolatileCsdTagBase::AddRef()
{
	m_ref++;
	return m_ref;
}
unsigned long CCeiVolatileCsdTagBase::Release()
{
	m_ref--;
	if (m_ref<=0) {
		delete this;
		return 0;
	}
	return m_ref;
}
int CCeiVolatileCsdTagBase::id(){return m_id;}
void CCeiVolatileCsdTagBase::update(ICsdTag *sender){}
void CCeiVolatileCsdTagBase::update_def(ICsdTag *sender){}
int CCeiVolatileCsdTagBase::get_default(void *lpParam){return -1;}
int CCeiVolatileCsdTagBase::set_default(){return -1;}
int CCeiVolatileCsdTagBase::change_default(long long lParam) { return -1; }
int CCeiVolatileCsdTagBase::change_default(LPFNGETVALUE lpfn, void *callback_param) { return -1; }
int CCeiVolatileCsdTagBase::choice_flag(long *lpFlag) { return -1; }
int CCeiVolatileCsdTagBase::choice_count(long *lpCount){return -1;}
int CCeiVolatileCsdTagBase::choice(int index, void *lpParam){return -1;}
ICsdTag::CSDTAG_CHOICE_FLAG CCeiVolatileCsdTagBase::choice_flag(){return ICsdTag::CHOICE_ANY;}
int CCeiVolatileCsdTagBase::save_value(LPFNSETVALUE lpfn, void *callback_param) { return -1; }
int CCeiVolatileCsdTagBase::restore_value(LPFNGETVALUE lpfn, void *callback_param) { return -1; }
void CCeiVolatileCsdTagBase::save(){}
void CCeiVolatileCsdTagBase::restore(){}
void CCeiVolatileCsdTagBase::flush(){}

CCeiVolatileCsdTagLong::CCeiVolatileCsdTagLong(int id):CCeiVolatileCsdTagBase(id), m_v(0){}
CCeiVolatileCsdTagLong::~CCeiVolatileCsdTagLong(){}
int CCeiVolatileCsdTagLong::get(void *lpParam)
{
	long *pout = (long*)lpParam;
	*pout=(long)m_v;
	return 0;	
}
int CCeiVolatileCsdTagLong::set(long long lParam)
{
	m_v=lParam;
	return  0;
}
CCeiVolatileCsdTagAsci::CCeiVolatileCsdTagAsci(int id):CCeiVolatileCsdTagBase(id) {}
CCeiVolatileCsdTagAsci::~CCeiVolatileCsdTagAsci(){}
int CCeiVolatileCsdTagAsci::get(void *lpParam)
{
	char *pout = (char*)lpParam;
	if (m_v.size()) strcpy(pout, m_v.c_str());
	else *pout=0;
	return 0;
}
int CCeiVolatileCsdTagAsci::set(long long lParam)
{
	char *pin = (char*)lParam;
	m_v = pin;
	return 0;
}
CCeiImageInformationCmd *create_vscsdsdk_information_cmd();
CCeiImageInformationTag *create_vscsdsdk_information_tag();
CCeiVolatileCsdTagLong *create_longtag(int id, long v)
{
	CCeiVolatileCsdTagLong *p = new CCeiVolatileCsdTagLong(id);
	p->set((long)v);
	return p;
}
CCeiVolatileCsdTagAsci *create_ascitag(int id, char *v)
{
	CCeiVolatileCsdTagAsci *p = new CCeiVolatileCsdTagAsci(id);
	p->set((long long)v);
	return p;	
}