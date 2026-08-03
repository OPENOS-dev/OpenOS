/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include "ExtendBitData12To16.h"
#include "ceilib.h"
#include <memory.h>
#include <assert.h>

using namespace Cei;
using namespace LLiPm;
using namespace DR_NAMESPACE;

RTN CExtendBitData12To16::extend12To16WithInvert(CImg& img)
{
    CImg dst;
    if (!dst.createImg(img.getWidth(), img.getHeight(), 16, img.getSpp(), img.getRGBOrder(), img.getXResolution(), img.getYResolution()))
    {
        return RTN_PAR;
    }
    if (dst.isNull())
    {
        return RTN_NOMEM;
    }

    int spp = img.getSpp();

    int dstSync = dst.getSync();
    int srcSync = img.getSync();

    unsigned char* dstLine = dst.getImagePtr();
    unsigned char* srcLine = img.getImagePtr();
    
    for (int c = 0; c < spp; c++)
    {
        LPWORD dst =  reinterpret_cast<LPWORD>(dstLine);
        LPWORD src =  reinterpret_cast<LPWORD>(srcLine + srcSync);

        long n = srcSync / 2;
        while (n--) {
            *dst++ = *--src;
        }

        srcLine += srcSync;
        dstLine += dstSync;
    }
    
    img = dst;
    return RTN_OK;
}

CExtendBitData12To16::CExtendBitData12To16(void)
{
}

CExtendBitData12To16::~CExtendBitData12To16(void)
{
}

RTN CExtendBitData12To16::IP(CImg& image)
{
	return Extend12To16(image);
}

RTN CExtendBitData12To16::IPFirst(CImg& image)
{
	return Extend12To16(image);
}

RTN CExtendBitData12To16::IPMiddle(CImg& image)
{
	return Extend12To16(image);
}

RTN CExtendBitData12To16::IPLast(CImg& image)
{
	return Extend12To16(image);
}

RTN CExtendBitData12To16::setInfo(CImg& image, void* lpInfo)
{
	if (lpInfo == 0) {
		return RTN_PAR;
	}
	COLLECTARRAYINFO* pCollectArrayInfo = (COLLECTARRAYINFO*)lpInfo;
	if (pCollectArrayInfo->ulSize != sizeof(COLLECTARRAYINFO)) {
		return RTN_PAR;
	}

	m_Info = *pCollectArrayInfo;
	return RTN_OK;
}

RTN CExtendBitData12To16::Extend12To16(CImg& image)
{
	if (image.getBps() != 12) {
		return RTN_OK;
	}
//	assert(lpDst->lSpp == lpSrc->lSpp);
//	assert(lpDst->lWidth == lpSrc->lWidth);
//	assert(lpDst->lHeight == lpSrc->lHeight);
	Cei::IMAGEINFO Info = *(IMAGEINFO *)image;
	Info.lpImage = 0;
	Info.lBps = 16;
	if (image.getSpp()==3 && image.getRGBOrder()==PIXEL_ORDER) {
		Info.lSync = Info.lWidth * 2 * 3;	// Bps = 16だから2　Spp = 3だから3
		Info.tImageSize = Info.lSync * Info.lHeight;
	}
	else {
		Info.lSync = Info.lWidth * 2;		// Bps = 16だから2 Sppは1のはず
		Info.tImageSize = Info.lSync * Info.lSpp * Info.lHeight;	//カラーLINE_ORDER用にSppをかける
	}
	CImg imgDst;
	if (!imgDst.createImg(Info)) {
		return RTN_PAR;
	}
	if (imgDst.isNull()) {
		return RTN_NOMEM;
	}

	if (image.getSpp() == 3 && image.getRGBOrder() == PIXEL_ORDER) {
		unsigned char* pDstLineTop = imgDst.getImagePtr();
		unsigned char* pSrcLineTop = image.getImagePtr();
		long lLine = image.getHeight();
		while (lLine--) {
			Extend12To16BitAndArrayCollect((unsigned short*)pDstLineTop, pSrcLineTop, image.getWidth() * image.getSpp(), image.getXResolution(), m_Info.nSensorVer);
			pDstLineTop += imgDst.getSync();
			pSrcLineTop += image.getSync();
		}
	}
	else {
		unsigned char* pDstLineTop = imgDst.getImagePtr();
		unsigned char* pSrcLineTop = image.getImagePtr();
		long lLine = image.getHeight() * image.getSpp();
		while (lLine--) {
			Extend12To16BitAndArrayCollect((unsigned short*)pDstLineTop, pSrcLineTop, image.getWidth(), image.getXResolution(), m_Info.nSensorVer);
			pDstLineTop += imgDst.getSync();
			pSrcLineTop += image.getSync();
		}
	}
	image.attachImg(imgDst);
	return RTN_OK;
}

