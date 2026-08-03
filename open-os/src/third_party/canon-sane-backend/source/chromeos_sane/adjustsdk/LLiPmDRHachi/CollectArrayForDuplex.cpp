/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include "CollectArrayForDuplex.h"
#include "ceilib.h"
#include <memory.h>
#include <assert.h>

using namespace Cei;
using namespace LLiPm;
using namespace DR_NAMESPACE;

CCollectArrayForDuplex::CCollectArrayForDuplex(void)
{
}

CCollectArrayForDuplex::~CCollectArrayForDuplex(void)
{
}

RTN CCollectArrayForDuplex::IP(CImg& image)
{
	return CollectArray(image, m_imgBack, m_Info);
}

RTN CCollectArrayForDuplex::IPFirst(CImg& image)
{
	return CollectArray(image, m_imgBack, m_Info);
}

RTN CCollectArrayForDuplex::IPMiddle(CImg& image)
{
	return CollectArray(image, m_imgBack, m_Info);
}

RTN CCollectArrayForDuplex::IPLast(CImg& image)
{
	return CollectArray(image, m_imgBack, m_Info);
}

RTN CCollectArrayForDuplex::setInfo(CImg& image, void* lpInfo)
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

void CCollectArrayForDuplex::getBackImage(CImg& image)
{
	image.attachImg(m_imgBack);
}

RTN CCollectArrayForDuplex::CollectArray(CImg& image, CImg& imgBack, COLLECTARRAYINFO& info)
{
	IP_TRY_BADALLOC

	IMAGEINFO Info = *((IMAGEINFO *)image);

	Info.lpImage = 0;

	Info.lWidth = image.getWidth() / 2;

	Info.lSync = Info.lWidth * 2;	// 16bit

    if (image.getRGBOrder() == PIXEL_ORDER) {
        Info.lSync *= image.getSpp();
        Info.tImageSize = Info.lSync * Info.lHeight;
    } else {
        Info.tImageSize = Info.lSync * Info.lSpp * Info.lHeight;
    }

    bool isNotShading = true;
#if defined(LIGHT_ADJUST_EAGLE_TYPE) || defined(LIGHT_ADJUST_CHIEBUS_TYPE) || defined(LIGHT_ADJUST_HACHI_TYPE) || defined(LIGHT_ADJUST_BOW_TYPE) || defined(LIGHT_ADJUST_DOCAN_TYPE) || defined(LIGHT_ADJUST_NEWDT_TYPE) || defined(LIGHT_ADJUST_VOYAJER_TYPE) || defined(LIGHT_ADJUST_DRF120_TYPE)
    isNotShading = (Info.lBps == 8);
#elif defined(LIGHT_ADJUST_BUNZ_TYPE) || defined(LIGHT_ADJUST_VC_TYPE) || defined(LIGHT_ADJUST_CAROL_TYPE) || defined(LIGHT_ADJUST_MINERVA_TYPE) || defined(LIGHT_ADJUST_DASH_TYPE)
    isNotShading = false;
#else
#error LIGHT_ADJUST_XXX_TYPE is undefined.
#endif
    
	if (isNotShading) {
		if ((image.getSync() % 2) != 0) {
			return RTN_PAR;
		}
		if (image.getRGBOrder() == PIXEL_ORDER) {
			Info.lSync = image.getSync() / 2;
			Info.tImageSize = Info.lSync * Info.lHeight;
		} else {
			Info.lSync = image.getSync() / 2;
			Info.tImageSize = Info.lSync * Info.lSpp * Info.lHeight;
		}
		CImg imgDstFront, imgDstBack;
		if (!imgDstFront.createImg(Info)) {
			return RTN_PAR;
		}
		if (!imgDstBack.createImg(Info)) {
			return RTN_PAR;
		}
		if (imgDstFront.isNull() || imgDstBack.isNull()) {
			return RTN_NOMEM;
		}

		unsigned char* pDstTop1 = imgDstFront.getImagePtr();
		unsigned char* pDstTop2 = imgDstBack.getImagePtr();
		unsigned char* pSrcTop = image.getImagePtr();
		long lLine = image.getHeight();
#if defined(LIGHT_ADJUST_EAGLE_TYPE) || defined(LIGHT_ADJUST_CHIEBUS_TYPE) || defined(LIGHT_ADJUST_HACHI_TYPE) || defined(LIGHT_ADJUST_BOW_TYPE) || defined(LIGHT_ADJUST_DOCAN_TYPE) || defined(LIGHT_ADJUST_NEWDT_TYPE) || defined(LIGHT_ADJUST_BUNZ_TYPE) || defined(LIGHT_ADJUST_VC_TYPE) || defined(LIGHT_ADJUST_CAROL_TYPE) || defined(LIGHT_ADJUST_MINERVA_TYPE) || defined(LIGHT_ADJUST_DASH_TYPE)
		
		if (image.getRGBOrder() == LINE_ORDER) {
			lLine *= image.getSpp();
		}
		while (lLine--) {
			Separate(pDstTop1, pDstTop2, pSrcTop, image.getWidth(), image.getXResolution(), info.nSensorVer);
			pDstTop1 += imgDstFront.getSync();
			pDstTop2 += imgDstBack.getSync();
			pSrcTop += image.getSync();
		}
#elif defined(LIGHT_ADJUST_VOYAJER_TYPE)
        
        if (image.getRGBOrder() == LINE_ORDER) {
            lLine *= image.getSpp();
        }
        while (lLine--) {
            Separate(pDstTop1, pDstTop2, pSrcTop, image.getWidth(), image.getXResolution(), info.nSensorVer);

            for (int x = 0; x < info.lScannerAvailableWidth / 2; x++)
            {
                int tmp = pDstTop1[x];
                pDstTop1[x] = pDstTop1[info.lScannerAvailableWidth - x - 1];
                pDstTop1[info.lScannerAvailableWidth - x - 1] = tmp;
            }

            pDstTop1 += imgDstFront.getSync();
            pDstTop2 += imgDstBack.getSync();
            pSrcTop += image.getSync();
        }
#elif defined(LIGHT_ADJUST_DRF120_TYPE)
		unsigned char* pSrcTop2=pSrcTop+Info.lWidth*Info.lSpp;
		if (image.getSpp()==1) {

			while (lLine--) {
				for (long w=0; w<Info.lWidth; w++) {
					pDstTop1[w]=pSrcTop[Info.lWidth-w];
				}					
				memcpy(pDstTop2, pSrcTop2, Info.lSync);
				pDstTop1 += imgDstFront.getSync();
				pDstTop2 += imgDstBack.getSync();
				pSrcTop += image.getSync();
				pSrcTop2 += image.getSync();
			}
		} else {
			while (lLine--) {
				

				for (long w=0; w<Info.lWidth; w++) {
					pDstTop1[w*3]  =pSrcTop[(Info.lWidth-1-w)*3];
					pDstTop1[w*3+1]=pSrcTop[(Info.lWidth-1-w)*3+1];
					pDstTop1[w*3+2]=pSrcTop[(Info.lWidth-1-w)*3+2];
				}
				
				memcpy(pDstTop2, pSrcTop2, Info.lSync);

				pDstTop1 += imgDstFront.getSync();
				pDstTop2 += imgDstBack.getSync();
				pSrcTop += image.getSync();
				pSrcTop2 += image.getSync();
			}
		}

#else
#error LIGHT_ADJUST_XXX_TYPE is undefined.
#endif
		image.attachImg(imgDstFront);
		imgBack.attachImg(imgDstBack);
	}
	else {
		Info.lBps = 16;

		CImg imgDstFront, imgDstBack;
		if (!imgDstFront.createImg(Info)) {
			return RTN_PAR;
		}
		if (!imgDstBack.createImg(Info)) {
			return RTN_PAR;
		}
		if (imgDstFront.isNull() || imgDstBack.isNull()) {
			return RTN_NOMEM;
		}

		unsigned char* pDstTop1 = imgDstFront.getImagePtr();
		unsigned char* pDstTop2 = imgDstBack.getImagePtr();
		unsigned char* pSrcTop = image.getImagePtr();
		long lLine = image.getHeight();
        long lWidth = image.getWidth();
        if (image.getRGBOrder() == LINE_ORDER) {
            lLine *= image.getSpp();
        }
        else {
            lWidth *= image.getSpp();
        }

		while (lLine--) {
			Extend12To16BitAndSeparate((unsigned short*)pDstTop1, (unsigned short*)pDstTop2, pSrcTop, lWidth, image.getXResolution(), info.nSensorVer);
			pDstTop1 += imgDstFront.getSync();
			pDstTop2 += imgDstBack.getSync();
			pSrcTop += image.getSync();
		}

		image.attachImg(imgDstFront);
		imgBack.attachImg(imgDstBack);
	}
	// TODO SetWidthを作ったほうがよいのでは？
	((IMAGEINFO *)image)->lWidth = info.lScannerAvailableWidth;
	((IMAGEINFO *)imgBack)->lWidth = info.lScannerAvailableWidth;

	return RTN_OK;

	IP_CATCH_BADALLOC
}


