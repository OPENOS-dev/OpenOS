/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <stdio.h>
#include <errno.h>
#include "ipsdk.h"
#include "sdk_image_util.h"
#include "ceilogwrite.h"

namespace {
const unsigned char g_BitAccess[]={
	0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01
};
const unsigned char g_BitAccessr[]={
	0x7f,0xbf,0xdf,0xef,0xf7,0xfb,0xfd,0xfe
};
}

class CAcsBase : public ICeiImgAccessor
{
public:
	CAcsBase(ICeiImage *pin):m_img(pin), m_ref(1){
		if (pin==NULL) printf("ERROR:pin is NULL\r\n");
		m_out[0]=m_out[1]=m_out[2]=0;
	}
	virtual ~CAcsBase(){}
	long STDMETHODCALLTYPE QueryInterface(REFIID id, void **ppOut)
	{
		return -1;
	}
	unsigned long STDMETHODCALLTYPE AddRef()
	{
		m_ref++;
		return m_ref;
	}
	unsigned long STDMETHODCALLTYPE Release()
	{
		m_ref--;
		if (m_ref<=0) {
			delete this;
			return 0;
		}
		return m_ref;
	}		
protected:
	ICeiImage *m_img;
	unsigned char m_out[3];
private:
	long m_ref;
};
class CAcsBinary : public CAcsBase
{
public:
	CAcsBinary(ICeiImage *pin, long threshold) : CAcsBase(pin), m_threshold(threshold){
		//printf("CAcsBinary::CAcsBinary()\r\n");
	}
	virtual ~CAcsBinary(){}
	char * get(long x, long y)
	{
		char * pix = m_img->img() + y * m_img->sync() + x / 8;
		if(pix[0] & g_BitAccess[x%8]) {
			m_out[0]=m_out[1]=m_out[2] = 0;//black
			return (char*)m_out;
		}
		m_out[0]=m_out[1]=m_out[2] = (unsigned char)0xff;//white
		return (char*)m_out;
	}
	void set(long x, long y, char *pixel)
	{
		//printf("CAcsBinary::set(%ld, %ld, %02x)\r\n", x, y, (unsigned char)pixel[0]);
		char* pix = m_img->img() + y * m_img->sync() + x / 8;
		if ((unsigned char)pixel[0]<= m_threshold)
			pix[0] |= g_BitAccess[x%8];
		else
			pix[0] &= g_BitAccessr[x%8];
	}
private:
	long m_threshold;
};
class CAcsGray : public CAcsBase
{
public:
	CAcsGray(ICeiImage *pin) : CAcsBase(pin) {
		//printf("CAcsGray::CAcsGray()\r\n");
	}
	virtual ~CAcsGray(){}
	char * get(long x, long y)
	{
		if (x >= m_img->width()) x = m_img->width() - 1;
		if (y >= m_img->height()) y = m_img->height() - 1;
		m_out[0] = m_out[1] = m_out[2] = (unsigned char)(*(m_img->img()+x+m_img->sync()*y));
		return (char*)m_out;
	}
	void set(long x, long y, char *pixel)
	{
		*(m_img->img()+x+m_img->sync()*y)=*pixel;
	}
};
class CAcsColor : public CAcsBase
{
public:
	CAcsColor(ICeiImage *pin) : CAcsBase(pin) {
		//printf("CAcsColor::CAcsColor()\r\n");
	}
	virtual ~CAcsColor(){}
	char * get(long x, long y)
	{
		if (x >= m_img->width()) x = m_img->width() - 1;
		if (y >= m_img->height()) y = m_img->height() - 1;
		//printf("CAcsColor::get(%ld, %ld) start\r\n", x, y);
		m_out[0] = (unsigned char)(*(m_img->img()+x*3+m_img->sync()*y));
		m_out[1] = (unsigned char)(*(m_img->img()+x*3+1+m_img->sync()*y));
		m_out[2] = (unsigned char)(*(m_img->img()+x*3+2+m_img->sync()*y));	
		//printf("CAcsColor::get(%ld, %ld) end %02x %02x %02x\r\n", x, y, m_out[0], m_out[1], m_out[2]);
		return (char*)m_out;
	}
	void set(long x, long y, char *pixel)
	{
		//printf("CAcsColor::set(%ld, %ld, %02x, %02x, %02x) start\r\n", x, y, (unsigned char)pixel[0], (unsigned char)pixel[1], (unsigned char)pixel[2]);
		//printf("width:%ld length:%ld\r\n", m_img->width(), m_img->height());
		char *ptr =m_img->img();
		*(ptr + x*3   + m_img->sync()*y)=pixel[0];
		*(ptr + x*3+1 + m_img->sync()*y)=pixel[1];
		*(ptr + x*3+2 + m_img->sync()*y)=pixel[2];
		//printf("CAcsColor::set() end\r\n");
	}
};


ICeiImgAccessor *create_image_accessor(ICeiImage *pin, long threshold)
{
	ICeiImgAccessor * pout = NULL;
	switch (pin->spp()*pin->bps()) {
	case 1:
	pout = (ICeiImgAccessor *)new CAcsBinary(pin, threshold);
	break;
	case 8:
	pout = (ICeiImgAccessor *)new CAcsGray(pin);
	break;
	case 24:
	pout = (ICeiImgAccessor *)new CAcsColor(pin);
	break;
	}	
	return pout;
}