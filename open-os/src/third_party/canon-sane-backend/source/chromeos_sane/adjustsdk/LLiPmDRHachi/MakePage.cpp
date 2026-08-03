/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include "MakePage.h"

#if defined(JPEG_EXPORT)
#include "CeiImgJpg.h"
#endif

#include <assert.h>
#include <string.h>


using namespace Cei;
using namespace LLiPm;

CMakePage::CMakePage(void)
{
}

CMakePage::~CMakePage(void)
{
}

RTN CMakePage::setInfo(CImg& image, void* lpInfo)
{
	if (lpInfo == 0) {
		return RTN_PAR;
	}
	MAKEPAGEINFO* pFixPage = (MAKEPAGEINFO*)lpInfo;
    
	m_Info = *pFixPage;
	return RTN_OK;
}

RTN CMakePage::MakePage(CImg& image)
{
    if (m_Info.RGBOrder == PIXEL_ORDER)
    {
        return toPixelOrder(image);
    }
    else if (m_Info.RGBOrder == LINE_ORDER)
    {
        return toLineOrder(image);
    }
    else if (IS_JPEG_ORDER(m_Info.RGBOrder))
    {
        return toJpegOrder(image);
    }

    return RTN_NOSPT;
}

RTN CMakePage::toPixelOrder(CImg& image)
{
    if (image.getRGBOrder() == PIXEL_ORDER) { return RTN_OK; }
    if (IS_JPEG_ORDER(image.getRGBOrder())) { return RTN_NOSPT; }
    if (image.getSpp() == 1)
    {
        IMAGEINFO* info = image;
        info->ulRGBOrder = PIXEL_ORDER;
        return RTN_OK;
    }

    long sync = image.getSync();
    long height = image.getHeight();

    long bufsize = sync * 3;
    unsigned char* line = new(std::nothrow) unsigned char[bufsize];
    if (line == NULL)
    {
        return RTN_NOMEM;
    }

    unsigned char* img = image.getImagePtr();
    unsigned char* rline = line;
    unsigned char* gline = rline + sync;
    unsigned char* bline = gline + sync;
    
    while (height--)
    {
        memcpy(line, img, bufsize);
        unsigned char* r = rline;
        unsigned char* g = gline;
        unsigned char* b = bline;
        
        long width = image.getWidth();
        unsigned char* p = img;
        
        while (width--)
        {
            *p++ = *r++;
            *p++ = *g++;
            *p++ = *b++;
        }
        
        img += bufsize;
    }
    
    delete[] line;
    
    IMAGEINFO* info = image;
    info->lSync = bufsize;
    info->ulRGBOrder = PIXEL_ORDER;
    return RTN_OK;
}

RTN CMakePage::toLineOrder(CImg& image)
{
    if (image.getRGBOrder() == LINE_ORDER) { return RTN_OK; }
    if (IS_JPEG_ORDER(image.getRGBOrder())) { return RTN_NOSPT; }
    if (image.getSpp() == 1)
    {
        IMAGEINFO* info = image;
        info->ulRGBOrder = LINE_ORDER;
        return RTN_OK;
    }
    
    long sync = image.getSync() / 3;
    long height = image.getHeight();
    
    long bufsize = sync * 3;
    unsigned char* line = new(std::nothrow) unsigned char[bufsize];
    if (line == NULL)
    {
        return RTN_NOMEM;
    }
    
    unsigned char* img = image.getImagePtr();
    
    while (height--)
    {
        memcpy(line, img, bufsize);
        unsigned char* p = line;
        
        long width = image.getWidth();
        unsigned char* r = img;
        unsigned char* g = r + sync;
        unsigned char* b = g + sync;
        
        while (width--)
        {
            *r++ = *p++;
            *g++ = *p++;
            *b++ = *p++;
        }
        
        img += bufsize;
    }
    
    delete[] line;
    
    IMAGEINFO* info = image;
    info->lSync = sync;
    info->ulRGBOrder = LINE_ORDER;
    return RTN_OK;
}

RTN CMakePage::toJpegOrder(CImg& image)
{
    if (IS_JPEG_ORDER(image.getRGBOrder()))
    {
        return RTN_OK;
    }
    
#if defined(JPEG_EXPORT)
    int quality = GET_JPEG_QUALITY(image.getRGBOrder());
    bool ret = CImgJpg::Compress(image, quality);
    return ret ? RTN_OK : RTN_PAR;
#else
    return RTN_NOSPT;
#endif
}
