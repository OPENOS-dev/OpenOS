/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <stdio.h>
#include <memory.h>
#include <vector>
#include "ceilogwrite.h"
#include "sdk_image_util.h"
#include "ipsdk.h"

namespace {
class CLineOrderColorImageWrapper : public ICeiImage
{
public:
	CLineOrderColorImageWrapper(ICeiImage *p);
	~CLineOrderColorImageWrapper();
	long STDMETHODCALLTYPE QueryInterface(REFIID id, void **ppOut);
	unsigned long STDMETHODCALLTYPE AddRef();
	unsigned long STDMETHODCALLTYPE Release();
	char *img();
	long width();
	long height();
	long xdpi();
	long ydpi();
	long spp();
	long bps();
	long sync();
	long size();
	long comptype();//0:non, 1:jpeg
	long compinfo();//comptype is none:not used, comptype is jpeg:quality 		
private:
	ICeiImage *m_img;
	long m_h;
};
CLineOrderColorImageWrapper::CLineOrderColorImageWrapper(ICeiImage *pimg): m_img(pimg), m_h(0)
{
	m_h = m_img->height()*3;
}
CLineOrderColorImageWrapper::~CLineOrderColorImageWrapper()
{
}
long CLineOrderColorImageWrapper::QueryInterface(REFIID id, void **ppOut)
{
	return -1;
}
unsigned long CLineOrderColorImageWrapper::AddRef()
{
	return 1;
}
unsigned long CLineOrderColorImageWrapper::Release()
{
	return 1;
}
char *CLineOrderColorImageWrapper::img(){return m_img->img();}
long CLineOrderColorImageWrapper::width(){return m_img->width();}
long CLineOrderColorImageWrapper::height(){return m_h;}
long CLineOrderColorImageWrapper::xdpi(){return m_img->xdpi();}
long CLineOrderColorImageWrapper::ydpi(){return m_img->ydpi();}
long CLineOrderColorImageWrapper::spp(){return 1;}
long CLineOrderColorImageWrapper::bps(){return m_img->bps();}
long CLineOrderColorImageWrapper::sync(){return m_img->sync();}
long CLineOrderColorImageWrapper::comptype(){return 0;}
long CLineOrderColorImageWrapper::size(){return m_img->size();}
long CLineOrderColorImageWrapper::compinfo(){return 0;}
void mirror_lineorder_color(ICeiImage *pimg)
{
	CLineOrderColorImageWrapper img(pimg);
	ceisdk_mirror(&img);
}
void mirror(ICeiImage* pimg)
{
	if (pimg->spp() == 3) {
		mirror_lineorder_color(pimg);
	}
	else {
		ceisdk_mirror(pimg);
	}
}
}
void ceisdk_mirror_lineorder(ICeiImage* pimg)
{
	mirror(pimg);
}