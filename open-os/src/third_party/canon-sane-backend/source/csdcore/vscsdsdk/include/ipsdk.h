/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef __IP_SDK_IP_API_UTIL_HEADER_DEFINED__
#define __IP_SDK_IP_API_UTIL_HEADER_DEFINED__

#include "image_interface.h"

long ceisdk_read_bmp_file(const char *pfilename, ICeiImage **ppOut);
long ceisdk_write_bmp_file(const char *pfilename, ICeiImage *pIn);
long ceisdk_read_jpeg_file(const char *pfilename, ICeiImage **ppOut);
long ceisdk_write_jpeg_file(const char *pfilename, ICeiImage *pIn);
long ceisdk_jpeg_decomp(ICeiImage **ppInOut, void *plib=NULL);
long ceisdk_change_sync(ICeiImage **ppInOut, long a=1 /*a=1 bytesync,2 bytessync,3 bytessync,4 bytessync,....*/);
void ceisdk_mirror(ICeiImage *pimg);
void ceisdk_mirror_lineorder(ICeiImage* pimg);
long ceisdk_invert_bw(ICeiImage *pIn);
long ceisdk_invert(ICeiImage *pIn);
long ceisdk_get_gamma_table_color(unsigned char *gamma, long gamma_size, long brightness, long contrast);
long ceisdk_get_gamma_table_gray(unsigned char *gamma, long gamma_size, long brightness, long contrast);
long ceisdk_get_gamma_table_bw(unsigned char *gamma, long gamma_size, long brightness, long contrast);
long ceisdk_gamma(ICeiImage *pin, unsigned char *gamma, long gamma_size);
typedef long(*LPFN_CEISDK_GAMMA_TABLE)(unsigned char *gamma, long gamma_size, long brightness, long contrast);
long ceisdk_gamma(ICeiImage *pin, LPFN_CEISDK_GAMMA_TABLE lpfn, long brightness, long contrast);
long ceisdk_gamma(ICeiImage *pin, unsigned char *gamma_r, unsigned char *gamma_g, unsigned char *gamma_b, long gamma_size);
#ifdef _WIN32
typedef POINT CEISDK_POINT;
typedef RECT CEISDK_RECT;
#else
typedef struct tagCEISDK_POINT
{
	long x, y;
}CEISDK_POINT, * LPCEISDK_POINT;
typedef struct tagCEISDK_RECT
{
	long    left;
	long    top;
	long    right;
	long    bottom;
}CEISDK_RECT, * LPCEISDK_RECT;
#endif
long ceisdk_detect_4points_simple_front(ICeiImage* pIn/*in*/, CEISDK_POINT* pos/*out. pos[4]*/);
long ceisdk_detect_4points_simple_back(ICeiImage* pIn/*in*/, CEISDK_POINT* pos/*out. pos[4]*/);
long ceisdk_autosize_simple(ICeiImage** ppInOut, CEISDK_POINT* pos);
long ceisdk_deskew_simple(ICeiImage* pIn, CEISDK_POINT* pos);
long ceisdk_autosize_deskew_simple(ICeiImage** ppInOut, CEISDK_POINT* pos);
void ceisdk_to_pixelorder_simple(ICeiImage** ppInOut);
void ceisdk_cut_offset_simple(ICeiImage** ppInOut, long offset);
void ceisdk_cut_offset_simple(ICeiImage** ppfront, ICeiImage** ppback, long offset);
void ceisdk_cutout_simple(ICeiImage** ppInOut, long x, long y, long w, long h);
class ICeiJpeg : public IUnknown
{
public:
	virtual long run(ICeiImage **ppInOut) = 0;
};
ICeiJpeg *jpeg_comp(long quality);
ICeiJpeg *jpeg_decmp();
#ifndef _CEISDKJPEGIMGINFO_H
#define _CEISDKJPEGIMGINFO_H
typedef struct tagCEICSDSDKJPEGIMAGEINFO {
	long struct_size;
	unsigned char *lpImage;
	long lWidth;
	long lHeight;
	long lSync;
	size_t	tImageSize;
	long lBps;
	long lSpp;
	long lXResolution;
	long lYResolution;
} CEICSDSDKJPEGIMAGEINFO, *LPCEICSDSDKJPEGIMAGEINFO;
#endif
// must call free(dst.lpImage) after ceisdk_jpeg_comp and ceisdk_jpeg_decomp;
long ceisdk_jpeg_comp(LPCEICSDSDKJPEGIMAGEINFO dst, LPCEICSDSDKJPEGIMAGEINFO src, int quality = 90, char *comment = (char*)"", void *plib = NULL);
long ceisdk_jpeg_decomp(LPCEICSDSDKJPEGIMAGEINFO dst, LPCEICSDSDKJPEGIMAGEINFO src, void *plib = NULL);
long read_jpeg_file(char *pfilename, ICeiImage **ppOut, void *plib = NULL);
class CCeiBitMapImg
{
public:
	CCeiBitMapImg() :width(0), height(0), sync(0), xdpi(0), ydpi(0), bpp(1), pimg(NULL) {}
	~CCeiBitMapImg()
	{
		if (pimg) delete[]pimg;
		pimg = NULL;
	}
	long width, height, sync, xdpi, ydpi, bpp;
	unsigned char *pimg;
};
long read_bmp_file(const char *pfilename, CCeiBitMapImg *pbm);
long write_bmp_file(const char *pfilename, CCeiBitMapImg *pbm);
long read_bmp_info(const char *pfilename, CCeiBitMapImg *pbm);
long read_bmp_file(const char *pfilename, ICeiImage **ppOut);
long write_bmp_file(const char *pfilename, ICeiImage *pIn);
void write_debug_bmp_file(const char * pfilestr, ICeiImage * pIn);
class ICeiImgAccessor : public IUnknown
{
public:
	virtual char * get(long x, long y) = 0;
	virtual void set(long x, long y, char *pixel) = 0;
};
ICeiImgAccessor *create_image_accessor(ICeiImage *pin, long threshold=128);
long cutout_internal(ICeiImage **ppInOut, long x, long y, long w, long h);
long resolution_convert_internal(ICeiImage **ppInOut, long dst_w, long dst_h, long dst_dpi);
long resolution_convert_internal(ICeiImage **ppInOut, long dst_w, long dst_h, long dst_xdpi, long dst_ydpi);
long color2gray_internal(ICeiImage **ppInOut);
long gray2binary_internal(ICeiImage **ppInOut, long threshold = 128);
long gray2binary_internal2(ICeiImage **ppInOut, long threshold = 128);
#endif
