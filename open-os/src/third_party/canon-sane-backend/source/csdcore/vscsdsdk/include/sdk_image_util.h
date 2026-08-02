/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef __VS_IMAGE_CLASS_UTIL_HEADER_DEFINED__
#define __VS_IMAGE_CLASS_UTIL_HEADER_DEFINED__

#include "image_interface.h"

class CVSCSDSDKImage : public ICeiImage
{
public:
	CVSCSDSDKImage();
	virtual ~CVSCSDSDKImage();
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
	long rgb_order();
	
	void width(long v);
	void height(long v);
	void xdpi(long v);
	void ydpi(long v);
	void spp(long v);
	void bps(long v);
	void sync(long v);
	void comptype(long v);//0:non, 1:jpeg
	void compinfo(long info);//comptype is none:not used, comptype is jpeg:quality 
	void size(long v);//v will be allocated by new.
	void attach(char *ptr, long size, bool bmalloc);//If bmalloc is true, ptr is created by malloc(). If bmalloc is false, ptr is created by new.
	void attach(ICeiImage *pbuffer, long size);
	void rgb_order(long v);
	
private:
	long m_ref;
	char *m_pimg;
	long m_w, m_h, m_xdpi, m_ydpi, m_spp, m_bps, m_sync, m_comptype, m_size, m_rgb_order, m_compinfo;
	bool m_malloc;
	XInterface<ICeiImage>m_buffer;
};

CVSCSDSDKImage *create_vscsdsdk_image();
ICeiImage *clone_vscsdsdk_image(ICeiImage *pIn);
CVSCSDSDKImage* ceisdk_create_vscsdsdk_image();
ICeiImage* ceisdk_clone_vscsdsdk_image(ICeiImage* pIn);
ICeiImage* ceisdk_create_file_image(ICeiImage* pIn/*will be released() in the api*/);

#endif