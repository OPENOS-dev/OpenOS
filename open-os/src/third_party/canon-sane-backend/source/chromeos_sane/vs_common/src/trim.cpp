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
long dummy_pixel_600dpi();
namespace {
enum {
	FRONT=0,
	BACK,
	FRONT_BACK_SIZE
};
inline void cei_memcopy(char *dst, char *src, long size)
{
	long loop = size / sizeof(long);
	long *pldst = (long *)dst;
	long *plsrc = (long *)src;
	while (loop--) {
		*pldst = *plsrc;
		pldst++;
		plsrc++;
	}
	loop = size % sizeof(long);
	dst = (char*)pldst;
	src = (char*)plsrc;
	while (loop--) {
		*dst = *src;
		dst++;
		src++;
	}
}
const long MUD = 600;
const long VALID_WIDTH_MUD = 5104;
void trim_invalid_pixel_gray_or_lineorder_color(ICeiImage **ppInOut)
{
	ICeiImage *in = *ppInOut;
	CVSCSDSDKImage *p = create_vscsdsdk_image();
	if (p) {
		p->width(VALID_WIDTH_MUD * in->xdpi() / MUD);
		p->height(in->height());
		p->spp(in->spp());
		p->bps(in->bps());
		p->xdpi(in->xdpi());
		p->ydpi(in->ydpi());
		p->sync(p->width());
		p->attach(in, p->sync()*p->height()*p->spp());
		char *src = p->img() + dummy_pixel_600dpi() * in->xdpi() / MUD;
		char *dst = p->img(); 
		long hmax = p->height()*p->spp();
		for (long h=0; h<hmax; h++) {
			cei_memcopy(dst, src, p->sync());
			dst+=p->sync();
			src+=in->sync();
		}
		*ppInOut = p;
	}
}
}
void trim_invalid_pixel(ICeiImage **ppInOut)
{
	trim_invalid_pixel_gray_or_lineorder_color(ppInOut);	
}
long valid_width_600dpi()
{
	return VALID_WIDTH_MUD;
}
