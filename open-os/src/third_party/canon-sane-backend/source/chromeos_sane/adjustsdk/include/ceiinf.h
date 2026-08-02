/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#pragma once
#include "ceidef.h"

namespace Cei
{
	typedef struct tagIMAGEINFO {
		unsigned long	ulSize;				// size of ViewSrcEx
		unsigned char*	lpImage;			// ptr of Image buffer
		long			lXpos;				// start dot of image
		long			lYpos;				// start line of image
		long			lWidth;				// width of image (dot)
		long			lHeight;			// heigth of image (line)
		long			lSync;				// line bytes
		unsigned long	tImageSize;			// buffer size
		long			lBps;				// bits per sample
		long			lSpp;				// samples per pixel
		unsigned long	ulRGBOrder;			// 
		long			lXResolution;		// resolution x
		long			lYResolution;		// resolution y
	} IMAGEINFO, *LPIMAGEINFO;

	#define CEIIMAGEINFO_V1_SIZE	CCSIZEOF_STRUCT(CEIIMAGEINFO, lYResolution)

#ifndef RGB_ORDER
#define RGB_ORDER
	#define PIXEL_ORDER			0
	#define LINE_ORDER			1
	#define PSEUDOCOLOR			2
    #define JPEG_ORDER          3
    
    #define IS_JPEG_ORDER(RGBOrder) (RGBOrder >= JPEG_ORDER)
    #define GET_JPEG_QUALITY(RGBOrder) ((RGBOrder - JPEG_ORDER) >> 2)
    #define SET_JPEG_QUALITY(quality) (JPEG_ORDER + (quality << 2))
#endif	// RGB_ORDER
}
