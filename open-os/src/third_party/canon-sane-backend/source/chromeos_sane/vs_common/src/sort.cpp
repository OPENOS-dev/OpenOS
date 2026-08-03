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
inline void sort_duplex(char *fl, char *fc, char *fr, char *bl, char *bc, char *br, char *line, long len)
{
	for (long i=0; i<len; i++) {
		*bl = line[i * 3 * 2    ];
		*br = line[i * 3 * 2 + 1];
		*fc = line[i * 3 * 2 + 2];
		*bc = line[i * 3 * 2 + 3];
		*fl = line[i * 3 * 2 + 4];
		*fr = line[i * 3 * 2 + 5];	
		fl++;
		fc++;
		fr++;
		bl++;
		bc++;
		br++;	
	}
}
void sort_duplex(ICeiImage *pimg, long sensor_len_600dpi)
{
	long len = sensor_len_600dpi * pimg->xdpi() / 600;
	char *line = new char [len * 3 * 2];
	
	char *fl = pimg->img();
	char *fc = fl + len;
	char *fr = fc + len;
	char *bl = fr + len;
	char *bc = bl + len;
	char *br = bc + len;
	
	long hmax = pimg->height() * pimg->spp();
	for (long h=0; h<hmax; h++) {
		memcpy(line, fl, len * 3 * 2);
		sort_duplex(fl, fc, fr, bl, bc, br, line, len);
		fl += pimg->width();
		fc += pimg->width();
		fr += pimg->width();
		bl += pimg->width();
		bc += pimg->width();
		br += pimg->width();
	}
	delete [] line;
}
inline void sort_simplex(char *c, char *l, char *r, char *line, long len)
{
	for (long i=0; i<len; i++) {
		*l = line[i * 3];
		*c = line[i * 3 + 1];
		*r = line[i * 3 + 2];
		l++;
		c++;
		r++;
	}
}
void sort_simplex(ICeiImage *pimg, long sensor_len_600dpi)
{
	long len = sensor_len_600dpi * pimg->xdpi() / 600;
	char *line = new char [len * 3];
	char *l = pimg->img();
	char *c = l + len;
	char *r = c + len;
	long hmax = pimg->height() * pimg->spp();
	for (long h=0; h<hmax; h++) {
		memcpy(line, l, len * 3);
		sort_simplex(l, c, r, line, len);
		l += pimg->width();
		c += pimg->width();
		r += pimg->width();
	}
	delete [] line;
}
}
long sensor_len_600dpi()
{
	return 1728;
}
void sort_simplex(ICeiImage **ppInOut)
{
	ICeiImage *in = *ppInOut;
	sort_simplex(in, sensor_len_600dpi());
}
void sort_duplex(ICeiImage **ppInOut)
{
	ICeiImage *in = *ppInOut;
	sort_duplex(in, sensor_len_600dpi());
}
void sort_duplex_line(char *f, char *b, char *line, long xdpi)
{
	long value_sensor_len_600dpi = sensor_len_600dpi();

	long len = value_sensor_len_600dpi * xdpi / 600;
		
	char *fl = f;
	char *fc = fl + len;
	char *fr = fc + len;
	char *bl = b;
	char *bc = bl + len;
	char *br = bc + len;

	sort_duplex(fl, fc, fr, bl, bc, br, line, len);	
}