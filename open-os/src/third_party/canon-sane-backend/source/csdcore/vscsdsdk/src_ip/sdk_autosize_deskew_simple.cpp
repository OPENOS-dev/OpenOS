/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <stdio.h>
#include <errno.h>
#include <memory.h>
#include <math.h>
#include "ipsdk.h"
#include "sdk_image_util.h"
#include "ceilogwrite.h"
#include "global_apis.h"
namespace {
	unsigned int func_diff(unsigned int a, unsigned int b) {
		return (a > b) ? (a - b) : (b - a);
	}
	void get_document_boundary(unsigned char* p, int interval, int width, int height, int sync, unsigned int* ppos_top, unsigned int* ppos_bottom, unsigned int th1, unsigned int th2)
	{
		for (int y = 0; y < height; y++) {
			unsigned char* pbase = p;
			unsigned char* pcurrent = p + sync * y;
			for (int x = 0; x < width; x++) {
				unsigned int val = func_diff(*pbase, *pcurrent);
				if (val > th1) {
					if (ppos_top[x] == (unsigned int)(-1)) { ppos_top[x] = y; }
				}
				if (val > th2) {
					ppos_bottom[x] = y;
				}
				pbase += interval;
				pcurrent += interval;
			}
		}
	}
	void get_rect(int width, int height, unsigned int* ppos_top, unsigned int* ppos_bottom, CEISDK_RECT* rect)
	{
		unsigned int left = width;
		unsigned int right = 0;
		int x = 0;
		unsigned int top = height;
		unsigned int bottom = 0;
		while (x < width) {
			if (ppos_top[x] != (unsigned int)(-1)) {
				left = x;
				top = ppos_top[x];
				bottom = ppos_bottom[x];
				break;
			}
			x++;
		}
		x++;
		while (x < width) {
			if (ppos_top[x] != (unsigned int)(-1)) {
				right = x;
				top = top < ppos_top[x] ? top : ppos_top[x];
				bottom = ppos_bottom[x] < bottom ? bottom : ppos_bottom[x];
			}
			x++;
		}
		rect->left = left;
		rect->right = right;
		rect->top = top;
		rect->bottom = bottom;
	}

	CEISDK_POINT get_slant(unsigned int* pos, int width, int res) {
		CEISDK_POINT slant = { 0 };
		int interval = 1000 * res / 25400;
		while (interval > 0) {
			int sum = 0;
			int current = 0;
			int count = 0;
			while (current + interval < width) {
				if (pos[current] != (unsigned int)(-1) && pos[current + interval] != (unsigned int)(-1)) {
					int diff = (int)pos[current + interval] - (int)pos[current];
					if (abs(diff) > interval) {
						diff = -interval * interval / diff;
					}
					sum += diff;
					count++;
				}
				current += interval;
			}
			if (count) {
				slant.x = interval * count;
				slant.y = sum;
				return slant;
			}
			interval = interval / 2;
		}
		slant.x = 1;
		slant.y = 0;
		return slant;
	}
	void get_intercept_maxmin(int width, unsigned int* ppos, int coef_a, int coef_b, long long& intercept_max, long long& intercept_min)
	{
		for (int i = 0; i < width; i++) {
			if (ppos[i] == (unsigned int)(-1)) {
				continue;
			}
			long long tmp = (long long)ppos[i] - coef_b * (long long)i / coef_a;
			intercept_max = intercept_max > tmp ? intercept_max : tmp;
			intercept_min = intercept_min < tmp ? intercept_min : tmp;
		}
	}
	CEISDK_POINT cross(long sx, long sy, long long b0, long long b1)
	{
		CEISDK_POINT pos;

		double a0 = (double)(sy) / (double)(sx);
		double a1 = -1.0 / a0;

		double x = ((double)(b1) - (double)(b0)) / (a0 - a1);
		pos.x = (long)x;
		pos.y = (long)(a0 * x + b0);

		return pos;
	}
	void get_slanted_rect(int width, unsigned int* ppos_top, unsigned int* ppos_bottom, CEISDK_POINT slant, CEISDK_POINT* pos)
	{
		long long intercept_a_min = 2147483647L;
		long long intercept_a_max = (-2147483647L - 1);
		long long intercept_b_min = 2147483647L;
		long long intercept_b_max = (-2147483647L - 1);
		get_intercept_maxmin(width, ppos_top, slant.x, slant.y, intercept_a_max, intercept_a_min);
		get_intercept_maxmin(width, ppos_bottom, slant.x, slant.y, intercept_a_max, intercept_a_min);
		get_intercept_maxmin(width, ppos_top, slant.y, -slant.x, intercept_b_max, intercept_b_min);
		get_intercept_maxmin(width, ppos_bottom, slant.y, -slant.x, intercept_b_max, intercept_b_min);
		if (slant.y < 0) {
			pos[0] = cross(slant.x, slant.y, intercept_a_min, intercept_b_max);
			pos[1] = cross(slant.x, slant.y, intercept_a_min, intercept_b_min);
			pos[2] = cross(slant.x, slant.y, intercept_a_max, intercept_b_max);
			pos[3] = cross(slant.x, slant.y, intercept_a_max, intercept_b_min);
		}
		else
		{
			pos[0] = cross(slant.x, slant.y, intercept_a_min, intercept_b_min);
			pos[1] = cross(slant.x, slant.y, intercept_a_min, intercept_b_max);
			pos[2] = cross(slant.x, slant.y, intercept_a_max, intercept_b_min);
			pos[3] = cross(slant.x, slant.y, intercept_a_max, intercept_b_max);
		}
	}

	long detect_4points_simple_core(unsigned char* p, int interval, int width, int height, int sync, int res, CEISDK_POINT* pos, CEISDK_RECT * rect, unsigned int th1, unsigned int th2)
	{
		unsigned int* ppos_top = new unsigned int[width];
		unsigned int* ppos_bottom = new unsigned int[width];
		if (!ppos_top || !ppos_bottom) {
			if (ppos_top) delete [] ppos_top;
			if (ppos_bottom) delete[] ppos_bottom;
			return -1;
		}
		memset(ppos_top, 0xff, sizeof(unsigned int) * width);
		memset(ppos_bottom, 0xff, sizeof(unsigned int) * width);

		get_document_boundary(p, interval, width, height, sync, ppos_top, ppos_bottom, th1, th2);

		CEISDK_POINT slant = get_slant(ppos_top, width, res);


		CEISDK_RECT result_rect = { 0 };
		if (!rect) { rect = &result_rect; }
		get_rect(width, height, ppos_top, ppos_bottom, rect);

		if (pos) {
			if (slant.x && slant.y) {
				get_slanted_rect(width, ppos_top, ppos_bottom, slant, pos);
			}
			else {
				pos[0].x = pos[2].x = rect->left;
				pos[1].x = pos[3].x = rect->right;
				pos[0].y = pos[1].y = rect->top;
				pos[2].y = pos[3].y = rect->bottom;
			}
		}


		if (ppos_top) delete[] ppos_top;
		if (ppos_bottom) delete[] ppos_bottom;

		return 0;
	}

	int get_distance(CEISDK_POINT a, CEISDK_POINT b) {
		return (int)(sqrt((double)(b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y)));
	}
}
unsigned int get_threshold_for_detect_4points(bool bIsFront, bool bIsTop)
{
	const char* section = "Detect4Points";
	char key[32] = { 0 };
	sprintf(key, "TH_%s_%s", bIsFront ? "F" : "B", bIsTop ? "T" : "B");
	return (unsigned int)ceisdk_get_private_profile_int(section, key, 20);
}
unsigned int get_background_color()
{
	return 0x00ffffff;		// white
}
long ceisdk_skew_collection(ICeiImage* pIn, ICeiImage* pOut, CEISDK_POINT ptBase, CEISDK_POINT slant)
{
	float length = sqrt(slant.x * slant.x + slant.y * slant.y);
	float slant_x = slant.x / length;
	float slant_y = slant.y / length;

	unsigned int bg_color = get_background_color();

	int spp = pOut->spp();
	// pixel order only
	for (int y = 0; y < pOut->height(); y++) {
		unsigned char* pd = (unsigned char*)pOut->img() + pOut->sync() * y;
		for (int x = 0; x < pOut->width(); x++) {
			float pos_x = ptBase.x + slant_x * x - slant_y * y;
			float pos_y = ptBase.y + slant_y * x + slant_x * y;
			bool bOut = false;
			if (pos_x < 0 || pos_x >= pIn->width() - 1) { bOut = true; }
			if (pos_y < 0 || pos_y >= pIn->height() - 1) { bOut = true; }
			if (bOut) {
				for (int i = 0; i < spp; i++) {
					*(pd + i) = ((bg_color >> (i * 8)) & 0x000000ff);
				}
			}
			else {
				unsigned char* psrc = (unsigned char*)pIn->img() + ((int)pos_y) * pIn->sync() + (int)(pos_x) * spp;
				for (int i = 0; i < spp; i++) {
					unsigned char s1_1 = *psrc;
					unsigned char s1_2 = *(psrc + spp);
					unsigned char s2_1 = *(psrc + pIn->sync());
					unsigned char s2_2 = *(psrc + pIn->sync() + spp);
					float pos_rest_x = pos_x - (float)((int)pos_x);
					float pos_rest_y = pos_y - (float)((int)pos_y);
					int tmp = s2_2 * pos_rest_x * pos_rest_y +
								s2_1 * (1 - pos_rest_x) * pos_rest_y +
								s1_2 * pos_rest_x * (1 - pos_rest_y) +
								s1_1 * (1 - pos_rest_x) * (1 - pos_rest_y);

					*(pd + i) = (tmp > 255 ? 255 : (tmp < 0 ? 0 : tmp));

					psrc += 1;
				}
			}
			pd += spp;
		}
	}
	return 0;
}
long ceisdk_detect_4points_simple(ICeiImage* pIn, CEISDK_POINT* pos/*==pos[4]*/, bool bIsFront)
{
	WriteLog("ceisdk_detect_4points_simple() start");
	unsigned int th1 = get_threshold_for_detect_4points(bIsFront, true);
	unsigned int th2 = get_threshold_for_detect_4points(bIsFront, false);
	long rtn = detect_4points_simple_core((unsigned char *)pIn->img(), 1, pIn->width(), pIn->height(), pIn->sync(), pIn->xdpi(), pos, nullptr, th1, th2);
	WriteLog("ceisdk_detect_4points_simple() end");
	return rtn;
}
long ceisdk_detect_4points_simple_front(ICeiImage* pIn/*in*/, CEISDK_POINT* pos/*out. pos[4]*/)
{
	return ceisdk_detect_4points_simple(pIn, pos, true);
}
long ceisdk_detect_4points_simple_back(ICeiImage* pIn/*in*/, CEISDK_POINT* pos/*out. pos[4]*/)
{
	return ceisdk_detect_4points_simple(pIn, pos, false);
}
long ceisdk_autosize_simple(ICeiImage**ppInOut, CEISDK_POINT* pos)
{
	WriteLog("ceisdk_autosize_simple() start");
	CEISDK_RECT rect = { 0 };

	ICeiImage* pimg = *ppInOut;
	rect.left = pos[0].x < pos[2].x ? pos[0].x : pos[2].x;
	rect.right = pos[1].x > pos[3].x ? pos[1].x : pos[3].x;
	rect.top = pos[0].y < pos[1].y ? pos[0].y : pos[1].y;
	rect.bottom = pos[2].y > pos[3].y ? pos[2].y : pos[3].y;
	rect.left = rect.left > 0 ? rect.left : 0;
	rect.right = rect.right < pimg->width() ? rect.right : (pimg->width() - 1);
	rect.top = rect.top > 0 ? rect.top : 0;
	rect.bottom = rect.bottom < pimg->height() ? rect.bottom : (pimg->height() - 1);

	ceisdk_cutout_simple(ppInOut, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
	WriteLog("ceisdk_autosize_simple() end");
	return 0;
}
long ceisdk_deskew_simple(ICeiImage*pIn, CEISDK_POINT* pos)
{
	WriteLog("ceisdk_deskew_simple() start");
	if (pos == NULL) return -1;

	CVSCSDSDKImage* pnew = create_vscsdsdk_image();
	if (pnew == NULL) return -1;

	pnew->width(pIn->width());
	pnew->height(pIn->height());
	pnew->spp(pIn->spp());
	pnew->bps(pIn->bps());
	pnew->xdpi(pIn->xdpi());
	pnew->ydpi(pIn->ydpi());
	pnew->sync(pIn->width() * pnew->spp());
	pnew->size(pnew->sync() * pnew->height());
	CEISDK_POINT slant;
	slant.x = pos[1].x - pos[0].x;
	slant.y = pos[1].y - pos[0].y;

	ceisdk_skew_collection(pIn, pnew, pos[0], slant);
	memcpy(pIn->img(), pnew->img(), pIn->sync() * pIn->height());

	pnew->Release();

	WriteLog("ceisdk_deskew_simple() end");
	return 0;
}
long ceisdk_autosize_deskew_simple(ICeiImage** ppInOut, CEISDK_POINT* pos)
{
	WriteLog("ceisdk_autosize_deskew_simple() start");
	if (pos == NULL) return -1;
	ICeiImage* pimg = *ppInOut;
	CVSCSDSDKImage* pnew = create_vscsdsdk_image();
	if (pnew == NULL) return -1;
	pnew->width(get_distance(pos[0], pos[1]));
	pnew->height(get_distance(pos[0], pos[2]));
	pnew->spp(pimg->spp());
	pnew->bps(pimg->bps());
	pnew->xdpi(pimg->xdpi());
	pnew->ydpi(pimg->ydpi());
	pnew->sync(pnew->width() * pnew->spp());
	pnew->size(pnew->sync() * pnew->height());
	CEISDK_POINT slant;
	slant.x = pos[1].x - pos[0].x;
	slant.y = pos[1].y - pos[0].y;
	ceisdk_skew_collection(pimg, pnew, pos[0], slant);
	pimg->Release();
	*ppInOut = pnew;
	WriteLog("ceisdk_autosize_deskew_simple() end");
	return 0;
}