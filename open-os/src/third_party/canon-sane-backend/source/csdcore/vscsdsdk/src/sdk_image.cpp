/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <cstddef>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#ifdef _WIN32
#include <Windows.h>
#endif
#include <memory.h>
#ifdef _WIN32
#include <Windows.h>
#endif
#include <memory>
#include "ceilogwrite.h"
#include "sdk_image_util.h"
#include "ceilogwrite.h"
namespace {
	char* get_tmp_path(char* p)
	{
#ifdef _WIN32
		GetTempPath(MAX_PATH, p);
		GetTempFileName(p, "csd", 0, p);
#else
		strcpy(p, "/tmp/");
#endif
		return p;
	}
	char* allocator(long sz)
	{
#ifdef _WIN32
#ifndef _WIN64
		char* out=NULL;
		enum {
			MAX_RETRY_COUNT = 10
		};
		long c = MAX_RETRY_COUNT;
		while (c) {
			try
			{
				if (c < MAX_RETRY_COUNT) SDKWriteLog("new char [%d] %d times", sz, MAX_RETRY_COUNT - c + 1);
				out = new char[sz];
				if (c < MAX_RETRY_COUNT) SDKWriteLog("new char [%d] %d times", sz, MAX_RETRY_COUNT - c + 1);
				c = 0;
			}
			catch (std::bad_alloc&) {
				SDKWriteLog("std::bad_alloc is thrown in %d %s", __LINE__, __FILE__);
				Sleep(1000);
				c--;
			}
		}
		return out;
#else
		return new char[sz];
#endif
#else
		return new char[sz];
#endif
	}
}
CVSCSDSDKImage::CVSCSDSDKImage():
m_ref(1), 
m_pimg(NULL), 
m_w(0), 
m_h(0), 
m_xdpi(0), 
m_ydpi(0), 
m_spp(0), 
m_bps(0), 
m_sync(0), 
m_comptype(0), 
m_size(0), 
m_rgb_order(0), 
m_compinfo(0),
m_malloc(false)
{
	//SDKWriteLog((char*)"[image]alloc (0x%x)", this);
}
CVSCSDSDKImage::~CVSCSDSDKImage()
{
	//SDKWriteLog((char*)"[image]delete (0x%x)", this);
	if (m_buffer.get()) {

	} else if (m_malloc) {
		if (m_pimg) {
			free(m_pimg);
			m_pimg=NULL;
			m_size=0;
		}
	} else {
		if (m_pimg) {
			if (m_size==1) delete m_pimg;
			else 			 delete []m_pimg;
			m_pimg=NULL;
			m_size=0;
		}
	}
}
long CVSCSDSDKImage::QueryInterface(REFIID id, void **ppOut)
{
	return -1;
}
unsigned long CVSCSDSDKImage::AddRef()
{
	m_ref++;
	return m_ref;
}
unsigned long CVSCSDSDKImage::Release()
{
	m_ref--;
	if (m_ref<=0) {
		delete this;
		return 0;
	}
	return m_ref;
}
char *CVSCSDSDKImage::img(){return m_pimg;}
long CVSCSDSDKImage::width(){return m_w;}
long CVSCSDSDKImage::height(){return m_h;}
long CVSCSDSDKImage::xdpi(){return m_xdpi;}
long CVSCSDSDKImage::ydpi(){return m_ydpi;}
long CVSCSDSDKImage::spp(){return m_spp;}
long CVSCSDSDKImage::bps(){return m_bps;}
long CVSCSDSDKImage::sync(){return m_sync;}
long CVSCSDSDKImage::comptype(){return m_comptype;}
long CVSCSDSDKImage::size(){return m_size;}
long CVSCSDSDKImage::rgb_order(){return m_rgb_order;}
long CVSCSDSDKImage::compinfo(){return m_compinfo;}
void CVSCSDSDKImage::width(long v)
{
	m_w=v;
}
void CVSCSDSDKImage::height(long v)
{
	if (m_pimg==NULL) {
		m_h=v;
	} else {
		if (comptype()) {
			m_h = v;
		} else {
			long new_size = m_size;
			if (m_rgb_order) {
				new_size = sync() * v * m_spp;
			} else {
				new_size = sync() * v;
			}
			if (new_size > m_size) {
				//printf("warning: size will be larger than original size.\r\n");
			} else {
				m_size = new_size;
				m_h = v;
			}			
		}
	}
}
void CVSCSDSDKImage::xdpi(long v)
{
	m_xdpi=v;
}
void CVSCSDSDKImage::ydpi(long v)
{
	m_ydpi=v;
}
void CVSCSDSDKImage::spp(long v)
{
	m_spp=v;
}
void CVSCSDSDKImage::bps(long v)
{
	m_bps=v;
}
void CVSCSDSDKImage::sync(long v)
{
	m_sync=v;
}
void CVSCSDSDKImage::comptype(long v)
{
	m_comptype=v;
}
void CVSCSDSDKImage::compinfo(long cinfo)
{
	m_compinfo=cinfo;
}
void CVSCSDSDKImage::size(long v)
{
	if (v>0) {
		if (m_pimg) {
			if (m_size==1) delete m_pimg;
			else 			 delete []m_pimg;
			m_size=0;
		}
		m_pimg = allocator(v);
		if (m_pimg) {
			m_size = v;
		}
	}
}
void CVSCSDSDKImage::attach(char *ptr, long size, bool bmalloc)
{
	m_pimg = ptr;
	m_size = size;
	m_malloc = bmalloc;
}
void CVSCSDSDKImage::attach(ICeiImage *pbuffer, long size)
{
	m_buffer.reset(pbuffer);
	m_pimg = m_buffer->img();
	m_size = size;
}
void CVSCSDSDKImage::rgb_order(long v)
{
	m_rgb_order=v;
}
CVSCSDSDKImage *create_vscsdsdk_image()
{
	return new CVSCSDSDKImage();
}
ICeiImage *clone_vscsdsdk_image(ICeiImage *pIn)
{
	CVSCSDSDKImage *p = create_vscsdsdk_image();
	if (p == NULL) return NULL;
	p->width(pIn->width());
	p->height(pIn->height());
	p->spp(pIn->spp());
	p->bps(pIn->bps());
	p->xdpi(pIn->xdpi());
	p->ydpi(pIn->ydpi());
	p->sync(pIn->sync());
	p->comptype(pIn->comptype());
	p->compinfo(pIn->compinfo());
	p->size(pIn->size());
	if (p->img()) {
		memcpy(p->img(), pIn->img(), p->size());
	}
	else {
		delete p;
		return NULL;
	}
	return (ICeiImage *)p;
}
CVSCSDSDKImage* ceisdk_create_vscsdsdk_image()
{
	return create_vscsdsdk_image();
}
ICeiImage* ceisdk_clone_vscsdsdk_image(ICeiImage* pIn)
{
	return clone_vscsdsdk_image(pIn);
}
class CSdkFileImage : public ICeiImage
{
public:
	CSdkFileImage(ICeiImage* pin) :
		m_p(NULL),
		m_ref(1),
		m_w(0),
		m_h(0),
		m_xdpi(0),
		m_ydpi(0),
		m_spp(0),
		m_bps(0),
		m_sync(0),
		m_size(0)
	{
		m_w = pin->width();
		m_h = pin->height();
		m_xdpi = pin->xdpi();
		m_ydpi = pin->ydpi();
		m_spp = pin->spp();
		m_bps = pin->bps();
		m_sync = pin->sync();
		m_size = pin->size();
		FILE* fp = fopen(get_tmp_path(m_path), "wb");
		if (fp) {
			fwrite(pin->img(), pin->size(), 1, fp);
			fclose(fp);
			pin->Release();
		}
		else {
			WriteLog("fopen(%s) error %s", m_path, strerror(errno));
		}
	}
	virtual ~CSdkFileImage() {
		if (m_path[0]) {
			remove(m_path);
		}
		if (m_p) delete[] m_p;
		m_p = NULL;
	}
	long STDMETHODCALLTYPE QueryInterface(REFIID id, void** ppOut) { return -1; }
	unsigned long STDMETHODCALLTYPE AddRef() { m_ref++; return m_ref; }
	unsigned long STDMETHODCALLTYPE Release()
	{
		m_ref--;
		if (m_ref == 0) { delete this; return 0; }
		return m_ref;
	}
	char* img() {
		if (m_p) return m_p;
		FILE* fp = fopen(m_path, "rb");
		if (fp) {
			m_p = new char[m_size];
			size_t r = fread(m_p, m_size, 1, fp);
			if (r!=(size_t)m_size) WriteLog("fread() warning");
			fclose(fp);
			remove(m_path);
			m_path[0] = 0;
		}
		return m_p;
	}
	long width() { return m_w; }
	long height() { return m_h; }
	long xdpi() { return m_xdpi; }
	long ydpi() { return m_ydpi; }
	long spp() { return m_spp; }
	long bps() { return m_bps; }
	long sync() { return m_sync; }
	long size() { return m_size; }
	long comptype() { return 0; }
	long compinfo() { return 0; }
	long rgb_order() { return 0; }
private:
	char* m_p;
	long m_ref, m_w, m_h, m_xdpi, m_ydpi, m_spp, m_bps, m_sync, m_size;
	char m_path[256 + 1];
};
ICeiImage* ceisdk_create_file_image(ICeiImage *pIn/*will be released() in the api*/)
{
	return new CSdkFileImage(pIn);

}