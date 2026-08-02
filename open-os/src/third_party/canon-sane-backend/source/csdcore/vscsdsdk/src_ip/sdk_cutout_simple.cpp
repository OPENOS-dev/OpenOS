/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <stdio.h>
#include <errno.h>
#include <memory.h>
#include "ipsdk.h"
#include "sdk_image_util.h"
#include "ceilogwrite.h"

namespace {
	inline void ceimemcpy(char* dst, char* src, long size)
	{
		while (size) {
			*dst = *src;
			dst++;
			src++;
			size--;
		}
	}
	void move(ICeiImage* pimg, long x, long y, long wmax, long hmax, long sync)
	{
		if (pimg->width() == wmax && pimg->height() == hmax) return;
		char* dst = pimg->img();
		char* src = pimg->img() + y * pimg->sync() + x * pimg->spp();
		for (long h = 0; h < hmax; h++) {
			ceimemcpy(dst, src, sync);
			dst += sync;
			src += pimg->sync();
		}
	}
}
void ceisdk_cutout_simple(ICeiImage** ppInOut, long x, long y, long w, long h)
{
	ICeiImage* pimg = *ppInOut;
	CVSCSDSDKImage* pnew = create_vscsdsdk_image();
	if (pnew == NULL) return;
	pnew->width(w);
	pnew->height(h);
	pnew->spp(pimg->spp());
	pnew->bps(pimg->bps());
	pnew->xdpi(pimg->xdpi());
	pnew->ydpi(pimg->ydpi());
	pnew->sync(pnew->width() * pnew->spp());
	pnew->attach(pimg, pnew->sync() * pnew->height());
	move(pimg,
		x,
		y,
		pnew->width(),
		pnew->height(),
		pnew->sync());
	*ppInOut = pnew;
}