/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <memory.h>
#include <memory>
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif
#include "ipsdk.h"
#include "sdk_image_util.h"
#include "global_apis.h"
#include "ceilogwrite.h"

namespace {
	class CAccessor : public ICeiImgAccessor
	{
	public:
		CAccessor(ICeiImage *pin, long threshold)
		{
			//printf("CAccessor::CAccessor()\r\n");
			//printf("width:%ld height:%ld\r\n", pin->width(), pin->height());
			//printf("spp:%ld bps:%ld\r\n", pin->spp(), pin->bps());
			//printf("sync:%ld xdpi:%ld ydpi:%ld\r\n", pin->sync(), pin->xdpi(), pin->ydpi());
			m_img.reset(create_image_accessor(pin, threshold));
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
long gray2binary_internal(ICeiImage *pIn, ICeiImage **ppOut, long threshold)
{
	//printf("gray2binary_internal() start\r\n");
	ICeiImage *pin = pIn;
	if (pin->bps()==1 || pin->spp()==3) {
		printf("gray2binary_internal() end (skipped)\r\n");
		return 0;
	}
	CVSCSDSDKImage *pout = create_vscsdsdk_image();
	pout->width(pin->width());
	pout->height(pin->height());
	pout->xdpi(pin->xdpi());
	pout->ydpi(pin->ydpi());
	pout->spp(1);
	pout->bps(1);
	pout->sync((pout->width()+7)/8);
	pout->size(pout->sync()*pout->height());
	CAccessor src(pin, threshold), dst(pout, threshold);
	for (long h = 0; h<pin->height(); h++) {
		for (long w=0; w<pin->width(); w++) {
			//printf("(%ld, %ld)\r\n", w, h);
			dst.set(w, h, src.get(w, h));
		}
	}			
	*ppOut = pout;
	//printf("gray2binary_internal() end\r\n");	
	return 0;
}
long gray2binary_internal(ICeiImage **ppInOut, long threshold)
{
	ICeiImage *pIn = *ppInOut;
	ICeiImage *pOut = NULL;
	long ret = gray2binary_internal(pIn, &pOut, threshold);
	if (!ret) {
		pIn->Release();
		*ppInOut = pOut;
	}
	return ret;
}
namespace {
	long get_threshold_binary_from_ini(long brightness, const char* setion)
	{
		/* Ý’è’l‚Í32–ˆ‚ÅŠi”[‚·‚é */
		int table_number = (int)brightness / 32;
		int table_index = (int)brightness % 32;

		char table_string[256] = { 0 };
		char table_key[32] = { 0 };
		sprintf(table_key, "BinThreshold_%d", table_number);
		ceisdk_get_private_profile_string(setion, table_key, table_string, 256);

		char* table_addr = table_string;
		while (table_index > 0) {
			if (*table_addr == '\0') break;
			if (*table_addr == ',') { table_index--; }
			table_addr++;
		}
		return atoi(table_addr);
	}
}
long gray2binary_internal2(ICeiImage** ppInOut, long threshold)
{
	return gray2binary_internal(ppInOut, get_threshold_binary_from_ini(threshold, "Binary_Simple") );
}
