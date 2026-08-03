/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef _CEIIMGINFO_H
#define _CEIIMGINFO_H

#ifdef USE_WIN//_WIN32
#ifndef BYTE
typedef unsigned char BYTE;
#endif
#ifndef WORD
typedef unsigned short WORD;
#endif
#ifndef DWORD
typedef unsigned long DWORD;
#endif
#else //USE_WIN//_WIN32
#include <stddef.h>
#include "ceidef.h"
using namespace Cei;
#endif //USE_WIN//_WIN32


typedef struct tagCEIIMAGEINFO {
	size_t	cbSize;				// size of ViewSrcEx
	BYTE	*lpImage;			// ptr of Image buffer
	long	lXpos;				// start dot of image
	long	lYpos;				// start line of image
	long	lWidth;				// width of image (dot)
	long	lHeight;			// heigth of image (line)
	long	lSync;				// line bytes
	size_t	tImageSize;			// buffer size
	long	lBps;				// bits per sample
	long	lSpp;				// samples per pixel
	DWORD	dwRGBOrder;			// 
	long	lXResolution;		// resolution x
	long	lYResolution;		// resolution y
} CEIIMAGEINFO, *LPCEIIMAGEINFO;

#define CEIIMAGEINFO_V1_SIZE	CCSIZEOF_STRUCT(CEIIMAGEINFO, lYResolution)

#ifndef RGB_ORDER
#define RGB_ORDER
	#define PIXEL_ORDER			0
	#define LINE_ORDER			1
	#define PSEUDOCOLOR			2
#endif	// RGB_ORDER

#endif	// _CEIIMGINFO_H
