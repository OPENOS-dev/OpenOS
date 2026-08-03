/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <cstddef>
#include <stdio.h>
#include <errno.h>
#include <memory.h>
#include <memory>
#include "ipsdk.h"
#include "sdk_image_util.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif
#include "global_apis.h"
#include "ceilogwrite.h"
namespace {
	class CAccessor : public ICeiImgAccessor
	{
	public:
		CAccessor(ICeiImage *pin)
		{
			//printf("CAccessor::CAccessor()\r\n");
			//printf("width:%ld height:%ld\r\n", pin->width(), pin->height());
			//printf("spp:%ld bps:%ld\r\n", pin->spp(), pin->bps());
			//printf("sync:%ld xdpi:%ld ydpi:%ld\r\n", pin->sync(), pin->xdpi(), pin->ydpi());			
			m_img.reset(create_image_accessor(pin));
		}
		long STDMETHODCALLTYPE QueryInterface(REFIID id, void **ppOut)
		{
			return -1;
		}
		unsigned long STDMETHODCALLTYPE AddRef()
		{
			return 1;
		}
		unsigned long STDMETHODCALLTYPE Release()
		{
			return 1;
		}		
		~CAccessor(){}
		char * get(long x, long y)
		{
			return m_img->get(x, y);
		}
		void set(long x, long y, char *pixel)
		{
			m_img->set(x, y, pixel);
		}	
	private:
		XInterface<ICeiImgAccessor>m_img;
	};
}
long resolution_convert_internal(ICeiImage **ppInOut, long dst_w, long dst_h, long dst_xdpi, long dst_ydpi)
{
	//printf("resolution_convert_internal(ppInOut, %ld, %ld, %ld) start\r\n", dst_w, dst_h, dst_dpi);
	ICeiImage *in = *ppInOut;

	if (in->xdpi()==dst_xdpi&&in->ydpi()==dst_ydpi) return 0;
	if (in->width()==dst_w && in->height()==dst_h) {
		//printf("resolution_convert_internal() end (skipped)\r\n");
		return 0;
	}
	CVSCSDSDKImage *pout = create_vscsdsdk_image();
	pout->width(dst_w);
	pout->height(dst_h);
	pout->xdpi(dst_xdpi);
	pout->ydpi(dst_ydpi);
	pout->spp(in->spp());
	pout->bps(in->bps());
	switch (pout->spp()*pout->bps()) {
		case 1:
		pout->sync((pout->width()+7)/8);
		break;
		case 8:
		pout->sync(pout->width());
		break;
		case 24:
		pout->sync(pout->width()*3);
		break;
	}
	pout->size(pout->sync()*pout->height());
	CAccessor src(in), dst(pout);

	//printf("src\r\n");
	//printf("(%ld, %ld)\r\n", in->width(), in->height());
	//printf("dst\r\n");
	//printf("(%ld, %ld)\r\n", pout->width(), pout->height());

	for (long h=0; h<pout->height(); h++) {
		for (long w=0; w<pout->width(); w++) {
			dst.set(w, h, src.get(w*in->xdpi() / dst_xdpi, h*in->ydpi() / dst_ydpi));
		}
	}
	in->Release();
	*ppInOut = pout;
	//printf("resolution_convert_internal(ppInOut, %ld, %ld, %ld) end\r\n", dst_w, dst_h, dst_dpi);
	return 0;
}
long resolution_convert_internal(ICeiImage** ppInOut, long dst_w, long dst_h, long dst_dpi)
{
	return resolution_convert_internal(ppInOut, dst_w, dst_h, dst_dpi, dst_dpi);
}

