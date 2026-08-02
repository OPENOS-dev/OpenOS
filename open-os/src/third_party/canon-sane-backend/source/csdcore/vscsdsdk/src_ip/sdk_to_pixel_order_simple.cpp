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
	inline void to_pixel_order(char *dst, char *src, long width)
	{
		char *r = src;
		char *g = r + width;
		char *b = g + width;
		for (long w = 0; w < width; w++) {
			dst[w * 3] = *r;
			dst[w * 3 + 1] = *g;
			dst[w * 3 + 2] = *b;
			r++;
			g++;
			b++;
		}
	}
	inline void to_pixel_order(ICeiImage *pimg)
	{
		long bsize = pimg->width() * 3;
		char *block = new char[bsize];
		char *ptr = pimg->img();
		long hmax = pimg->height();
		for (long h = 0; h < hmax; h++) {
			memcpy(block, ptr, bsize);
			to_pixel_order(ptr, block, pimg->width());
			ptr += bsize;
		}
		delete[] block;
	}
}
void ceisdk_to_pixelorder_simple(ICeiImage **ppInOut)
{
	ICeiImage *in = *ppInOut;
	if (in->spp() == 3) {
		CVSCSDSDKImage *p = create_vscsdsdk_image();
		p->width(in->width());
		p->height(in->height());
		p->spp(in->spp());
		p->bps(in->bps());
		p->xdpi(in->xdpi());
		p->ydpi(in->ydpi());
		p->sync(in->width() * in->spp());
		p->attach(in, p->sync() * p->height());
		to_pixel_order(in);
		*ppInOut = p;
	}
}