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

class CCutOffSetImage : public ICeiImage
{
public:
	CCutOffSetImage(ICeiImage *pin, long offset);
	virtual ~CCutOffSetImage();
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
	long m_ref;
	char *m_pimg;
	XInterface<ICeiImage>m_img;
	long m_height;
	long m_size;
};
CCutOffSetImage::CCutOffSetImage(ICeiImage *img, long offset): m_ref(1), m_pimg(NULL), m_height(0), m_size(0)
{
	m_img.reset(img);
	if (offset>0) {
		m_pimg = m_img->img() + m_img->width() * offset * m_img->spp();
	} else if (offset==0) {
		m_height = m_img->height();
	} else {
		m_pimg = m_img->img();
		offset *= -1;
	}	
	m_height = m_img->height() - offset;
	m_size = img->width() * m_img->spp() * m_height;
}
CCutOffSetImage::~CCutOffSetImage()
{
}
long CCutOffSetImage::QueryInterface(REFIID id, void **ppOut)
{
	return -1;
}
unsigned long CCutOffSetImage::AddRef()
{
	m_ref++;
	return m_ref;
}
unsigned long CCutOffSetImage::Release()
{
	m_ref--;
	if (m_ref<=0) {
		delete this;
		return 0;
	}
	return m_ref;
}
char *CCutOffSetImage::img(){return m_pimg;}
long CCutOffSetImage::width(){return m_img->width();}
long CCutOffSetImage::height(){return m_height;}
long CCutOffSetImage::xdpi(){return m_img->xdpi();}
long CCutOffSetImage::ydpi(){return m_img->ydpi();}
long CCutOffSetImage::spp(){return m_img->spp();}
long CCutOffSetImage::bps(){return m_img->bps();}
long CCutOffSetImage::sync(){return m_img->sync();}
long CCutOffSetImage::comptype(){return 0;}
long CCutOffSetImage::size(){return m_size;}
long CCutOffSetImage::compinfo(){return 0;}
void ceisdk_cut_offset_simple(ICeiImage **ppfront, ICeiImage **ppback, long offset)
{
	ICeiImage *pfront = *ppfront;
	ICeiImage *pback = *ppback;
	*ppfront = new CCutOffSetImage(pfront, offset);
	*ppback = new CCutOffSetImage(pback, -offset);
}
void ceisdk_cut_offset_simple(ICeiImage** ppInOut, long offset)
{
	ICeiImage* pimg = *ppInOut;
	*ppInOut = new CCutOffSetImage(pimg, offset);
}