/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include "CollectArrayForSimplex.h"
#include "ceilib.h"
#include <memory.h>
#include <assert.h>

using namespace Cei;
using namespace LLiPm;
using namespace DR_NAMESPACE;

CCollectArrayForSimplex::CCollectArrayForSimplex(void)
{
}

CCollectArrayForSimplex::~CCollectArrayForSimplex(void)
{
}

RTN CCollectArrayForSimplex::IP(CImg& image)
{
	return CollectArray(image, m_Info);
}

RTN CCollectArrayForSimplex::IPFirst(CImg& image)
{
	return CollectArray(image, m_Info);
}

RTN CCollectArrayForSimplex::IPMiddle(CImg& image)
{
	return CollectArray(image, m_Info);
}

RTN CCollectArrayForSimplex::IPLast(CImg& image)
{
	return CollectArray(image, m_Info);
}


RTN CCollectArrayForSimplex::setInfo(CImg& image, void* lpInfo)
{
	if (lpInfo == 0) {
		return RTN_PAR;
	}
	COLLECTARRAYINFO* pCollectArrayInfo = (COLLECTARRAYINFO*)lpInfo;
	if (pCollectArrayInfo->ulSize != sizeof(COLLECTARRAYINFO)) {
		return RTN_PAR;
	}
#if defined(LIGHT_ADJUST_DRF120_TYPE)

#else
	if (image.getRGBOrder() == PIXEL_ORDER) {
		return RTN_PAR;
	}
#endif
	m_Info = *pCollectArrayInfo;
	return RTN_OK;
}

RTN CCollectArrayForSimplex::CollectArray(CImg& image, COLLECTARRAYINFO& info)
{
	IP_TRY_BADALLOC

    bool isNotShading = true;
#if defined(LIGHT_ADJUST_EAGLE_TYPE) || defined(LIGHT_ADJUST_CHIEBUS_TYPE) || defined(LIGHT_ADJUST_HACHI_TYPE) || defined(LIGHT_ADJUST_BOW_TYPE) || defined(LIGHT_ADJUST_DOCAN_TYPE) || defined(LIGHT_ADJUST_NEWDT_TYPE) || defined(LIGHT_ADJUST_VOYAJER_TYPE) || defined(LIGHT_ADJUST_DRF120_TYPE)
    isNotShading = (image.getBps() == 8);
#elif defined(LIGHT_ADJUST_BUNZ_TYPE) || defined(LIGHT_ADJUST_VC_TYPE) || defined(LIGHT_ADJUST_CAROL_TYPE) || defined(LIGHT_ADJUST_MINERVA_TYPE) || defined(LIGHT_ADJUST_DASH_TYPE)
    isNotShading = IS_JPEG_ORDER(image.getRGBOrder());
#else
#error LIGHT_ADJUST_XXX_TYPE is undefined.
#endif
    
	if (isNotShading) {
		CImg imgDst;
		if (!imgDst.createImg(image)) {
			return RTN_PAR;
		}
		if (imgDst.isNull()) {
			return RTN_NOMEM;
		}

		unsigned char* pDstTop = imgDst.getImagePtr();
		unsigned char* pSrcTop = image.getImagePtr();
		long lLine = image.getHeight() * image.getSpp();		// スキャナからは、ラインオーダーでデータが送られてくるため、カラーなら、3*ライン回ループする
		long lWidth = image.getWidth();
		long lSrcSync = image.getSync();
		long lDstSync = imgDst.getSync();
#if defined(LIGHT_ADJUST_DOCAN_TYPE)
		// TakeZ, Docan
		while (lLine--) {
			ArrayCollection(pSrcTop, pDstTop, image.getWidth(), image.getXResolution(), info.nSensorVer);
			pSrcTop += image.getSync();
			pDstTop += imgDst.getSync();
		}
#elif defined(LIGHT_ADJUST_CHIEBUS_TYPE) || defined(LIGHT_ADJUST_HACHI_TYPE)
		// ChieBus
		for (long h=0; h<lLine; h++)
		{
			for (LONG w=0; w<lWidth; w++)
			{
				pDstTop[h*lDstSync + lWidth - 1 - w] = pSrcTop[h*lSrcSync + w];
			}
		}
#elif defined(LIGHT_ADJUST_EAGLE_TYPE) || defined(LIGHT_ADJUST_BOW_TYPE)
		// Bow
		for (long h=0; h<lLine; h++)
		{
			for (LONG w=0; w<lWidth; w++)
			{
				pDstTop[h*lDstSync + w] = pSrcTop[h*lSrcSync + w];
			}
		}
#elif defined(LIGHT_ADJUST_NEWDT_TYPE)
        // NewDT
		int nSensorVer = info.nSensorVer;
		if (nSensorVer == 0) {
			memcpy(pDstTop, pSrcTop, lLine * lWidth);
		}
		else if (nSensorVer == 1) {
			// 
			while (lLine--) {
				ArrayCollection(pSrcTop, pDstTop, image.getWidth(), image.getXResolution(), info.nSensorVer);
				pSrcTop += image.getSync();
				pDstTop += imgDst.getSync();
			}
		}
		else {
			assert(false);
		}
#elif defined(LIGHT_ADJUST_VOYAJER_TYPE)
        // Voyajer
        while (lLine--) {
            ArrayCollection(pSrcTop, pDstTop, image.getWidth(), image.getXResolution(), info.nSensorVer);

            for (int x = 0; x < info.lScannerAvailableWidth / 2; x++)
            {
                BYTE tmp = pDstTop[x];
                pDstTop[x] = pDstTop[info.lScannerAvailableWidth - x - 1];
                pDstTop[info.lScannerAvailableWidth - x - 1] = tmp;
            }

            pSrcTop += image.getSync();
            pDstTop += imgDst.getSync();
        }
#elif defined(LIGHT_ADJUST_DRF120_TYPE)
		// Capricorn
		if (info.bflatbed) {

			

			memcpy(pDstTop, pSrcTop, image.getHeight()*lDstSync);

		

		} else {
			if (image.getSpp()==1) {
				for (long h=0; h<lLine; h++)
				{
					for (LONG w=0; w<lWidth; w++)
					{
						pDstTop[h*lDstSync + lWidth - 1 - w] = pSrcTop[h*lSrcSync + w];
					}
				}
			} 
			else {
				for (long h=0; h<image.getHeight(); h++) 
				{
					for (long w=0; w<lWidth; w++) {
						pDstTop[h*lDstSync + (lWidth - w - 1)*3    ] = pSrcTop[h*lSrcSync + w*3];
						pDstTop[h*lDstSync + (lWidth - w - 1)*3 + 1] = pSrcTop[h*lSrcSync + w*3 + 1];
						pDstTop[h*lDstSync + (lWidth - w - 1)*3 + 2] = pSrcTop[h*lSrcSync + w*3 + 2];
					}
				}
			}			
		}
#elif defined(LIGHT_ADJUST_BUNZ_TYPE) || defined(LIGHT_ADJUST_VC_TYPE) || defined(LIGHT_ADJUST_CAROL_TYPE) || defined(LIGHT_ADJUST_MINERVA_TYPE) || defined(LIGHT_ADJUST_DASH_TYPE)
        assert(false);
#else
#error LIGHT_ADJUST_XXX_TYPE is undefined.
#endif
		image.attachImg(imgDst);
	} else {
		IMAGEINFO Info;
		Info = *((IMAGEINFO *)image);
		Info.lpImage = 0;
		Info.lWidth = image.getWidth();
		Info.lSync = Info.lWidth * 2;	// 16bit
        if (image.getRGBOrder() == PIXEL_ORDER) {
            Info.lSync *= image.getSpp();
            Info.tImageSize = Info.lSync * Info.lHeight;
        } else {
            Info.tImageSize = Info.lSync * Info.lSpp * Info.lHeight;
        }
		Info.lBps = 16;

		CImg imgDst;
		if (!imgDst.createImg(Info)) {
			return RTN_PAR;
		}
		if (imgDst.isNull()) {
			return RTN_NOMEM;
		}

		unsigned char* pDstTop = imgDst.getImagePtr();
		unsigned char* pSrcTop = image.getImagePtr();
		long lLine = image.getHeight();
        long lSync = image.getWidth();
        if (image.getRGBOrder() == LINE_ORDER) {
            lLine *= image.getSpp();
        }
        else {
            lSync *= image.getSpp();
        }
        
		while (lLine--) {
			Extend12To16BitAndArrayCollect((unsigned short*)pDstTop, pSrcTop, lSync, image.getXResolution(), info.nSensorVer);
			pDstTop += imgDst.getSync();
			pSrcTop += image.getSync();
		}
		image.attachImg(imgDst);
	}
	((IMAGEINFO *)image)->lWidth = info.lScannerAvailableWidth;

	return RTN_OK;

	IP_CATCH_BADALLOC
}


