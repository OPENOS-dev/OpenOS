/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include "CeiImgEdit.h"
#include <assert.h>
#include <string.h>
#include <algorithm>
using namespace Cei;
using namespace LLiPm;

const char CImgEdit::BIT_TABLE[] = {(char)0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};


typedef unsigned char* (*PUTPIXEL)(unsigned char *pData, const RGBQUAD& color);

static unsigned char* FillColor_putPixel_Gray(unsigned char *pData, const RGBQUAD& color)
{
    *pData++ = color.rgbReserved;
    return pData;
}

static unsigned char* FillColor_putPixel_Color(unsigned char *pData, const RGBQUAD& color)
{
    *pData++ = color.rgbRed;
    *pData++ = color.rgbGreen;
    *pData++ = color.rgbBlue;
    return pData;
}

bool CImgEdit::FillColor(CImg& img, RGBQUAD fill, RECT& ignore)
{
    if (img.getRGBOrder() == LINE_ORDER)
    {
        return false;
    }
    
    ignore.top = std::max(0L, std::min(ignore.top, img.getHeight()));
    ignore.bottom = std::max(0L, std::min(ignore.bottom, img.getHeight()));
    ignore.left = std::max(0L, std::min(ignore.left, img.getWidth()));
    ignore.right = std::max(0L, std::min(ignore.right, img.getWidth()));
    
    PUTPIXEL putPixelProc;
    int pixelStep;
    if (img.getSpp() == 3) {
        putPixelProc = FillColor_putPixel_Color;
        pixelStep = 3;
    } else {
        putPixelProc = FillColor_putPixel_Gray;
        pixelStep = 1;
    }
    
    unsigned char* pData = img.getImagePtr();
    unsigned char* pFilledLine = NULL;
    unsigned char* pSyncLine = NULL;
    long lSync = img.getSync();
    long lHeight = 0;
    
    if (lHeight < ignore.top) {
        pFilledLine = pData;
        
        // 1ライン作成
        for (long i = 0; i < lSync; i += pixelStep) {
            pData = putPixelProc(pData, fill);
        }
        lHeight++;
        
        // 2ライン目以降は1ライン目をコピー
        while (lHeight < ignore.top) {
            memcpy(pData, pFilledLine, lSync);
            pData += lSync;
            lHeight++;
        }
    }
    
    pSyncLine = pData;
    while (lHeight < ignore.bottom) {
        pData = pSyncLine;
        
        for (long x = 0; x < ignore.left; x++) {
            pData = putPixelProc(pData, fill);
        }
        
        pData += (ignore.right - ignore.left) * pixelStep;
        
        for (long x = ignore.right; x < img.getWidth(); x++) {
            pData = putPixelProc(pData, fill);
        }
        
        pSyncLine += lSync;
        lHeight++;
    }
    
    if (lHeight < img.getHeight()) {
        pFilledLine = pSyncLine;
        
        for (long i = 0; i < lSync; i += pixelStep) {
            pData = putPixelProc(pData, fill);
        }
        lHeight++;
        
        while (lHeight < img.getHeight()) {
            memcpy(pData, pFilledLine, lSync);
            pData += lSync;
            lHeight++;
        }
    }
    return true;
}

bool CImgEdit::ToColor(CImg &img)
{
    int bpp = img.getBpp();
    
    switch (bpp)
    {
        case 24:
            return true;
        case 8:
            return GrayToColor(img);
        case 1:
            if (!BinaryToGray(img))
            {
                return false;
            }
            else
            {
                return GrayToColor(img);
            }
        default:
            return false;
    }
}

bool CImgEdit::ToGray(CImg &img)
{
    int bpp = img.getBpp();
    
    switch (bpp)
    {
        case 24:    return ColorToGray(img);
        case 8:     return true;
        case 1:     return BinaryToGray(img);
        default:    return false;
    }
}

bool CImgEdit::ToBinary(CImg &img)
{
    int bpp = img.getBpp();
    
    switch (bpp)
    {
        case 24:
            if (!ColorToGray(img))
            {
                return false;
            }
            else
            {
                return GrayToBinary(img);
            }
        case 8:
            return GrayToBinary(img);
        case 1:
            return true;
        default:
            return false;
    }
}

void CImgEdit::MemOr(unsigned char* lpDst, unsigned char* lpOr, long lWidth)
{
    while(lWidth--)
    {
        *lpDst++ |= *lpOr++;
    }
}

void CImgEdit::MemAnd(unsigned char* lpDst, unsigned char* lpAnd, long lWidth)
{
    while(lWidth--)
    {
        *lpDst++ &= *lpAnd++;
    }
}

bool CImgEdit::ColorToGray(CImg &img)
{
    assert(img.getBpp() == 24);
    
    CImg imgDst;
    imgDst.createImg(img.getWidth(), img.getHeight(), 8, 1, PIXEL_ORDER, img.getXResolution(), img.getYResolution());
    if (imgDst.isNull())
    {
        return false;
    }

    unsigned char* src_top = img.getImagePtr();
    long src_sync = img.getSync();
    long src_rgbsync = img.getRGBSync();
    
    unsigned char* dst_top = imgDst.getImagePtr();
    long dst_sync = imgDst.getSync();

    if (img.getRGBOrder() == LINE_ORDER)
    {
        long lines = img.getHeight();
        while (lines--)
        {
            unsigned char* dst = dst_top;
            unsigned char* r = src_top;
            unsigned char* g = r + src_sync;
            unsigned char* b = g + src_sync;
            
            long bytes = img.getWidth();
            while (bytes--)
            {
                *dst++ = static_cast<unsigned char>((static_cast<unsigned long>(*r) * 38 + static_cast<unsigned long>(*g) * 76 + static_cast<unsigned long>(*b) * 14) / 128);
                r++;
                g++;
                b++;
            }
            src_top += src_rgbsync;
            dst_top += dst_sync;
        }
    }
    else
    {
        long lines = img.getHeight();
        while (lines--)
        {
            unsigned char* dst = dst_top;
            unsigned char* r = src_top;
            unsigned char* g = r + 1;
            unsigned char* b = g + 1;
            
            long bytes = img.getWidth();
            while (bytes--)
            {
                *dst++ = static_cast<unsigned char>((static_cast<unsigned long>(*r) * 38 + static_cast<unsigned long>(*g) * 76 + static_cast<unsigned long>(*b) * 14) / 128);
                r += 3;
                g += 3;
                b += 3;
            }
            src_top += src_rgbsync;
            dst_top += dst_sync;
        }
    }

    img.attachImg(imgDst);
    return true;
}

bool CImgEdit::GrayToColor(CImg& img)
{
    assert(img.getBpp() == 8);
    
    CImg imgDst;
    imgDst.createImg(img.getWidth(), img.getHeight(), 8, 3, PIXEL_ORDER, img.getXResolution(), img.getYResolution());
    if (imgDst.isNull())
    {
        return false;
    }
    
    unsigned char* src_top = img.getImagePtr();
    long src_sync = img.getSync();
    
    unsigned char* dst_top = imgDst.getImagePtr();
    long dst_sync = imgDst.getSync();
    
    long lines = img.getHeight();
    while (lines--)
    {
        unsigned char* src = src_top;
        unsigned char* dst = dst_top;

        long bytes = img.getWidth();
        while (bytes--)
        {
            *dst++ = *src;
            *dst++ = *src;
            *dst++ = *src;
            src++;
        }
        
        src_top += src_sync;
        dst_top += dst_sync;
    }
    
    img.attachImg(imgDst);
    return true;
}

bool CImgEdit::GrayToBinary(CImg& img)
{
    assert(img.getBpp() == 8);
    
    CImg imgDst;
    imgDst.createImg(img.getWidth(), img.getHeight(), 1, 1, PIXEL_ORDER, img.getXResolution(), img.getYResolution());
    if (imgDst.isNull())
    {
        return false;
    }
        
    unsigned char* src_top = img.getImagePtr();
    long src_sync = img.getSync();
    
    unsigned char* dst_top = imgDst.getImagePtr();
    long dst_sync = imgDst.getSync();
    
    long lines = img.getHeight();
    while (lines--)
    {
        unsigned char* src = src_top;
        unsigned char* dst = dst_top;
            
        long bits = img.getWidth();
        while (bits > 8)
        {
            unsigned char byte = 0;
            byte |= (*src++ & 0x80) ? 0x80 : 0;
            byte |= (*src++ & 0x80) ? 0x40 : 0;
            byte |= (*src++ & 0x80) ? 0x20 : 0;
            byte |= (*src++ & 0x80) ? 0x10 : 0;
            byte |= (*src++ & 0x80) ? 0x08 : 0;
            byte |= (*src++ & 0x80) ? 0x04 : 0;
            byte |= (*src++ & 0x80) ? 0x02 : 0;
            byte |= (*src++ & 0x80) ? 0x01 : 0;
            *dst++ = byte;
            bits -= 8;
        }
        
        if (bits)
        {
            unsigned char byte = 0;
            for (int i = 0; i < bits; i++)
            {
                byte |= (*src++ & 0x80) ? BIT_TABLE[i] : 0;
            }
            *dst = byte;
        }
        
        src_top += src_sync;
        dst_top += dst_sync;
    }
    
    img.attachImg(imgDst);
    return true;
}

bool CImgEdit::BinaryToGray(CImg &img)
{
    assert(img.getBpp() == 1);

    CImg imgDst;
    imgDst.createImg(img.getWidth(), img.getHeight(), 8, 1, PIXEL_ORDER, img.getXResolution(), img.getYResolution());
    if (imgDst.isNull())
    {
        return false;
    }

    unsigned char* src_top = img.getImagePtr();
    long src_sync = img.getSync();
    
    unsigned char* dst_top = imgDst.getImagePtr();
    long dst_sync = imgDst.getSync();
    
    long lines = img.getHeight();
    while (lines--)
    {
        unsigned char* src = src_top;
        unsigned char* dst = dst_top;
        
        long bits = img.getWidth();
        while (bits > 8)
        {
            unsigned char data = *src++;
            data & 0x80 ? *dst++ = 0xff : *dst++;
            data & 0x40 ? *dst++ = 0xff : *dst++;
            data & 0x20 ? *dst++ = 0xff : *dst++;
            data & 0x10 ? *dst++ = 0xff : *dst++;
            data & 0x08 ? *dst++ = 0xff : *dst++;
            data & 0x04 ? *dst++ = 0xff : *dst++;
            data & 0x02 ? *dst++ = 0xff : *dst++;
            data & 0x01 ? *dst++ = 0xff : *dst++;
            bits -= 8;
        }
        
        for (int i = 0; i < bits; i++)
        {
            unsigned char data = *src;
            data & BIT_TABLE[i] ? *dst++ = 0xff : *dst++;
        }

        src_top += src_sync;
        dst_top += dst_sync;
    }
    
    img.attachImg(imgDst);
    return true;
}
