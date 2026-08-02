/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include "Dependencies.h"
#include "DRHachiFilter.h"
#include "ceilib.h"
#include "DRHachiLogger.h"
#include <memory.h>
#include <assert.h>
#include <algorithm>
#include <CeiLogger.h>
using namespace Cei;
using namespace LLiPm;
using namespace DR_NAMESPACE;


#define USE_EXEC_IP	RTN result = RTN_OK;

#define EXEC_SIMPLEX_IP(func, img, side)	\
	result = func(img, side, imagestate);	\
	if (result != RTN_OK) {	\
		return result;	\
	}

#define EXEC_DUPLEX_IP(func, imgF, imgB)	\
	result = func(imgF, imgB, imagestate);	\
	if (result != RTN_OK) {	\
		return result;	\
	}

#define EXEC_FOLIO_IP(func, img)	\
	result = func(img, imagestate);	\
	if (result != RTN_OK) {	\
		return result;	\
	}

CSpecialFilter::CSpecialFilter(void)
: m_lOffset(SENSOR_FB_OFFSET)
{
}

CSpecialFilter::~CSpecialFilter(void)
{
}

void CSpecialFilter::clear()
{
    m_CollectArrayForSimplex[0].clear();
    m_CollectArrayForSimplex[1].clear();
    m_ExtendBitData12To16.clear();
    m_CollectArrayForDuplex.clear();
    //m_Detect4PointsDuplex.clear();
    //m_CutOffset[0].clear();
    //m_CutOffset[1].clear();
    //m_Detect4Points[0].clear();
    //m_Detect4Points[1].clear();
    //m_CorrectDir[0].clear();
    //m_CorrectDir[1].clear();
    //m_MakePage[0].clear();
    //m_MakePage[1].clear();
    //m_ColorGapCorrect[0].clear();
    //m_ColorGapCorrect[1].clear();
    //m_CutOut[0].clear();
    //m_CutOut[1].clear();
    //m_RmvShadow[0].clear();
    //m_RmvShadow[1].clear();
    //m_SRGBConversion[0].clear();
    //m_SRGBConversion[1].clear();
    //m_ColorSaturate[0].clear();
    //m_ColorSaturate[1].clear();
    //m_DetectResolution[0].clear();
    //m_DetectResolution[1].clear();
    //m_FixPage[0].clear();
    //m_FixPage[1].clear();
    CNormalFilter::clear();
}

RTN CSpecialFilter::IP(CImg& image)
{
	return DRHachiFilter(image, IMAGESTATE_COMPLETE);
}

RTN CSpecialFilter::IPFirst(CImg& image)
{
	return DRHachiFilter(image, IMAGESTATE_FIRST);
}

RTN CSpecialFilter::IPMiddle(CImg& image)
{
	return DRHachiFilter(image, IMAGESTATE_MIDDLE);
}

RTN CSpecialFilter::IPLast(CImg& image)
{
	return DRHachiFilter(image, IMAGESTATE_LAST);
}

RTN CSpecialFilter::setInfo(CImg& image, void* lpInfo)
{
	if (lpInfo == 0) {
		CeiLogger::writeLog("CSpecialFilter::setInfo return RTN_PAR. (lpInfo == 0)");
		return RTN_PAR;
	}
	memset(&m_Info, 0, sizeof(m_Info));
	FILTERSIMPLEXINFO* pFilterSimplexInfo = (FILTERSIMPLEXINFO*)lpInfo;
	FILTERDUPLEXINFO* pFilterDuplexInfo = (FILTERDUPLEXINFO*)lpInfo;

	RTN rtn;

	if (pFilterSimplexInfo->ulSize == sizeof(FILTERSIMPLEXINFO)) {
		DRHachiLogger::writeFILTERSIMPLEXINFO(*pFilterSimplexInfo);
		DRHachiLogger::writeIMAGEINFO(image);

		m_Info.ulSize = sizeof(FILTERSIMPLEXINFO);
		m_Info.infoInputImg = pFilterSimplexInfo->infoInputImg;
		m_Info.infoOutputImg = pFilterSimplexInfo->infoOutputImg;
		m_Info.infoSpecial[FRONT] = pFilterSimplexInfo->infoSpecial;
		m_Info.infoSpecial[BACK] = pFilterSimplexInfo->infoSpecial;
		m_Info.infoNormal[FRONT] = pFilterSimplexInfo->infoNormal;
		m_Info.infoNormal[BACK] = pFilterSimplexInfo->infoNormal;
		m_Info.bPutImageOnSide = false;

		NORMALFILTERDUPLEXINFO Info;
		Info.ulSize = sizeof(NORMALFILTERDUPLEXINFO);
		Info.infoInputImg = pFilterSimplexInfo->infoInputImg;
		Info.infoOutputImg = pFilterSimplexInfo->infoOutputImg;
		Info.infoNormal[FRONT] = pFilterSimplexInfo->infoNormal;
		Info.infoNormal[BACK] = pFilterSimplexInfo->infoNormal;
		Info.bPutImageOnSide = false;
		rtn = CNormalFilter::setInfo(image, &Info);
		if (rtn != RTN_OK) {
			return rtn;
		}
		rtn = checkParamError(FRONT);
		if (rtn != RTN_OK) {
			return rtn;
		}
	}
	else if (pFilterDuplexInfo->ulSize == sizeof(FILTERDUPLEXINFO)) {
		DRHachiLogger::writeFILTERDUPLEXINFO(*pFilterDuplexInfo);
		DRHachiLogger::writeIMAGEINFO(image);

		m_Info.ulSize = sizeof(FILTERDUPLEXINFO);
		m_Info.infoInputImg = pFilterDuplexInfo->infoInputImg;
		m_Info.infoOutputImg = pFilterDuplexInfo->infoOutputImg;
		m_Info.infoSpecial[FRONT] = pFilterDuplexInfo->infoSpecial[FRONT];
		m_Info.infoSpecial[BACK] = pFilterDuplexInfo->infoSpecial[BACK];
		m_Info.infoNormal[FRONT] = pFilterDuplexInfo->infoNormal[FRONT];
		m_Info.infoNormal[BACK] = pFilterDuplexInfo->infoNormal[BACK];
		m_Info.bPutImageOnSide = pFilterDuplexInfo->bPutImageOnSide;

		NORMALFILTERDUPLEXINFO Info;
		Info.ulSize = sizeof(NORMALFILTERDUPLEXINFO);
		Info.infoInputImg = pFilterDuplexInfo->infoInputImg;
		Info.infoOutputImg = pFilterDuplexInfo->infoOutputImg;
		Info.infoNormal[FRONT] = pFilterDuplexInfo->infoNormal[FRONT];
		Info.infoNormal[BACK] = pFilterDuplexInfo->infoNormal[BACK];
		Info.bPutImageOnSide = pFilterDuplexInfo->bPutImageOnSide;
		rtn = CNormalFilter::setInfo(image, &Info);
		if (rtn != RTN_OK) {
			return rtn;
		}
		rtn = checkParamError(FRONT);
		if (rtn != RTN_OK) {
			return rtn;
		}
		rtn = checkParamError(BACK);
		if (rtn != RTN_OK) {
			return rtn;
		}
	}
	else {
		CeiLogger::writeLog("CSpecialFilter::setInfo return RTN_PAR. (%d is not match info sizes.)", pFilterDuplexInfo->ulSize);
		return RTN_PAR;
	}

	return rtn;
}

RTN CSpecialFilter::checkParamError(SIDE side)
{
	return RTN_OK;
}

void CSpecialFilter::getBackImage(CImg& image)
{
	image.attachImg(m_imgBack);
}

void CSpecialFilter::setBackImage(CImg& image)
{
    m_imgBack.attachImg(image);
}
/*
RTN CSpecialFilter::NormalFilterSimplex(CImg& image, FILTERSIMPLEXINFO* lpInfo, bool bFront)
{
	RTN result = setInfo(image, lpInfo);
	if (result != RTN_OK) {
		return result;
	}
	SIDE side = bFront ? FRONT : BACK;
	return execNormalFilter(image, side, IMAGESTATE_COMPLETE);
}
*/
/*
RTN CSpecialFilter::NormalFilterFolio(CImg& image, FILTERDUPLEXINFO* lpInfo)
{
	RTN result = setInfo(image, lpInfo);
	if (result != RTN_OK) {
		return result;
	}
	result = execNormalFolioFilter(image, IMAGESTATE_COMPLETE);
	return result;
}
*/
void CSpecialFilter::AdjustLightFirst(ADJUSTINFO* lpInfo, long lSensorId)
{
	DRHachiLogger::writeADJUSTINFO(*lpInfo);

	m_AdjustLight.AdjustLightFirst(lpInfo, lSensorId);
    
    m_nSensorVer = m_AdjustLight.GetSensorVer();
}

RTN CSpecialFilter::AdjustLightNext(CImg& image, ADJUSTINFO* lpInfo)
{
	RTN result;
	CImg imgDstFront, imgDstBack;
	result = AdjustLightLast(imgDstFront, imgDstBack, image, lpInfo);
	if (result != RTN_OK) {
		return result;
	}

	if (lpInfo->bDuplex) {
		return m_AdjustLight.AdjustLightNext(imgDstFront, imgDstBack, lpInfo);
	}
	else {
		//AdjustLightNextは、片面判定をlpInfo->bDuplexで行う。
		//片面処理時はimgBackは使わない。
		return m_AdjustLight.AdjustLightNext(imgDstFront, imgDstBack, lpInfo);
	}
}

RTN CSpecialFilter::AdjustLightLast(CImg& imgDstFront, CImg& imgDstBack, CImg& imgSrc, ADJUSTINFO* lpInfo)
{
	DRHachiLogger::writeADJUSTINFO(*lpInfo);

	//AdjustLightLastって名前だけど画像を並び替えて１ラインにしてるだけ

	if (lpInfo->bDuplex) {
		m_Info.infoInputImg = *(IMAGEINFO *)imgSrc;
		m_Info.infoInputImg.lpImage = 0;
		m_Info.infoInputImg.lWidth /= 2;
		m_Info.infoInputImg.lSync /= 2;
		//以下の画処理でlXResolutionとlWidthを使うだけ

		RTN Result;
		Result = execCollectArrayForDuplex(imgSrc, imgDstBack, IMAGESTATE_COMPLETE);
		if (Result != RTN_OK) {
			return Result;
		}
		Result = execExtendBitData12To16(imgSrc, FRONT, IMAGESTATE_COMPLETE);
		if (Result != RTN_OK) {
			return Result;
		}
		Result = execExtendBitData12To16(imgDstBack, FRONT, IMAGESTATE_COMPLETE);
		if (Result != RTN_OK) {
			return Result;
		}
		Result = execMackOneLineImage(imgSrc);
		if (Result != RTN_OK) {
			return Result;
		}
		Result = execMackOneLineImage(imgDstBack);
		if (Result != RTN_OK) {
			return Result;
		}

		imgDstFront.attachImg(imgSrc);
		return RTN_OK;
	}
	else {
		m_Info.infoInputImg = *(IMAGEINFO *)imgSrc;
		m_Info.infoInputImg.lpImage = 0;
		//以下の画処理でlXResolutionとlWidthを使うだけ

		RTN Result;
        Result = execCollectArrayForSimplex(imgSrc, FRONT, IMAGESTATE_COMPLETE);
		if (Result != RTN_OK) {
			return Result;
		}
        
		Result = execExtendBitData12To16(imgSrc, FRONT, IMAGESTATE_COMPLETE);
		if (Result != RTN_OK) {
			return Result;
		}

		Result = execMackOneLineImage(imgSrc);
		if (Result != RTN_OK) {
			return Result;
		}

		imgDstFront.attachImg(imgSrc);
		return RTN_OK;
	}
}

RTN CSpecialFilter::makeShadingData(CImg& imgWhite, CImg& imgBlack, ADJUSTINFO* lpInfo, SIDE Side, unsigned char* lpData, unsigned long ulSize)
{
	CImg imgWhiteOrg = imgWhite;
	CImg imgBlackOrg = imgBlack;

    if (ulSize > 0) {
        //導光体補正データが読み込めているなら補正
        if (RTN_OK != m_AdjustLight.AdjustLightCurve(imgWhite, imgBlack, lpInfo, Side, lpData, ulSize)) {
            CeiLogger::writeLog("AdjustLightCurve data is not loaded.");
        }
    }
    
	if (m_Shading[Side].isNull()) {
		m_Shading[Side].set(new CShading, CIPController<CShading>::IPANY);
	}
    
    RTN rtn;
#if defined(LIGHT_ADJUST_EAGLE_TYPE)
    CExtendBitData12To16::extend12To16WithInvert(imgWhite);
    CExtendBitData12To16::extend12To16WithInvert(imgBlack);
	imgWhiteOrg = imgWhite;
	imgBlackOrg = imgBlack;
	rtn = m_Shading[Side].getPtr()->makeShadingData(imgWhite, imgBlack, imgWhiteOrg, imgBlackOrg, m_nSensorVer);
#elif defined(LIGHT_ADJUST_CHIEBUS_TYPE) || defined(LIGHT_ADJUST_DOCAN_TYPE) || defined(LIGHT_ADJUST_HACHI_TYPE) || defined(LIGHT_ADJUST_BOW_TYPE) || defined(LIGHT_ADJUST_VOYAJER_TYPE)
	rtn = m_Shading[Side].getPtr()->makeShadingData(imgWhite, imgBlack, imgWhiteOrg, imgBlackOrg, m_nSensorVer);
#elif defined(LIGHT_ADJUST_NEWDT_TYPE)|| defined(LIGHT_ADJUST_DRF120_TYPE)
    if (imgWhite.getBps() == 8)
    {
        rtn = m_Shading[Side].getPtr()->makeShadingData(imgWhite);
    }
    else
    {
        rtn = m_Shading[Side].getPtr()->makeShadingData(imgWhite, imgBlack, imgWhiteOrg, imgBlackOrg, m_nSensorVer);
		if (rtn == RTN_OK) {
			if (m_nSensorVer != 1) {
				// 既存のバージョンではスキャナーにデータを送るため、エンディアンを変更する
				m_Shading[Side].getPtr()->formatShadingData9(imgWhite, imgBlack, imgWhiteOrg);
			}
		}
    }
#elif defined(LIGHT_ADJUST_EAGLE_TYPE) || defined(LIGHT_ADJUST_BUNZ_TYPE) || defined(LIGHT_ADJUST_VC_TYPE) || defined(LIGHT_ADJUST_CAROL_TYPE) || defined(LIGHT_ADJUST_MINERVA_TYPE) || defined(LIGHT_ADJUST_DASH_TYPE)
	rtn = m_Shading[Side].getPtr()->makeShadingData(imgWhiteOrg);
#else
#error LIGHT_ADJUST_XXX_TYPE is undefined.
#endif
    return rtn;
}


RTN CSpecialFilter::DRHachiFilter(CImg& image, IMAGESTATE imagestate)
{
	return RTN_OK;
}
RTN CSpecialFilter::execCollectArrayForSimplex(CImg& image, SIDE side, IMAGESTATE imagestate)
{
	IP_TRY_BADALLOC

	if (!image.isNull()) {	// Append + image.isNullの場合はチェックしたくないため
		if ((image.getBps() != 8 && image.getBps() != 12) ||
			(image.getSpp() != 3 && image.getSpp() != 1)) {
				return RTN_PAR;
		}
	}
	if (m_CollectArrayForSimplex[side].isNull()) {
        m_CollectArrayForSimplex[side].set(new CCollectForSimplex, CCollectIPTYPE);
    }

	COLLECTARRAYINFO infoCollectArray;
	infoCollectArray.ulSize = sizeof(COLLECTARRAYINFO);
	IMAGEINFO* infoInputImg = &m_Info.infoInputImg;
	infoCollectArray.DummyPixels = getDummyPixels(infoInputImg->lXResolution, m_nSensorVer);
	infoCollectArray.lScannerAvailableWidth = getMaxWidthWithoutDummyPixels(infoInputImg->lWidth, infoInputImg->lXResolution, m_nSensorVer);
    infoCollectArray.lInputHeight = infoInputImg->lHeight;
#if defined(DRF120_Build)
	infoCollectArray.bflatbed = m_Info.infoSpecial[side].bFlatbed?1:0;
#endif
    infoCollectArray.nSensorVer = m_nSensorVer;
	return execIP((CIPController<CIPBase>*)&m_CollectArrayForSimplex[side], image, &infoCollectArray, imagestate);

	IP_CATCH_BADALLOC
}

RTN CSpecialFilter::execExtendBitData12To16(CImg& image, SIDE side, IMAGESTATE imagestate)
{
	IP_TRY_BADALLOC

	if (m_ExtendBitData12To16.isNull()) {
		m_ExtendBitData12To16.set(new CExtendBitData12To16, CIPController<CExtendBitData12To16>::IPANY);
	}
	COLLECTARRAYINFO infoCollectArray;
	infoCollectArray.ulSize = sizeof(COLLECTARRAYINFO);
	IMAGEINFO* infoInputImg = &m_Info.infoInputImg;
	infoCollectArray.DummyPixels = getDummyPixels(infoInputImg->lXResolution, m_nSensorVer);
	infoCollectArray.lScannerAvailableWidth = getMaxWidthWithoutDummyPixels(infoInputImg->lWidth, infoInputImg->lXResolution, m_nSensorVer);
    infoCollectArray.nSensorVer = m_nSensorVer;
    return execIP((CIPController<CIPBase>*)&m_ExtendBitData12To16, image, &infoCollectArray, imagestate);

	IP_CATCH_BADALLOC
}

RTN CSpecialFilter::execCollectArrayForDuplex(CImg& imgFront, CImg& imgBack, IMAGESTATE imagestate)
{
	IP_TRY_BADALLOC

	if (!imgFront.isNull()) {	// Append + image.isNullの場合はチェックしたくないため
		if ((imgFront.getBps() != 8 && imgFront.getBps() != 12) ||
			(imgFront.getSpp() != 3 && imgFront.getSpp() != 1))
        {
				return RTN_PAR;
		}
	}

	if (m_CollectArrayForDuplex.isNull()) {
        m_CollectArrayForDuplex.set(new CCollectForDuplex, CCollectIPTYPE);
	}
	COLLECTARRAYINFO infoCollectArray;
	infoCollectArray.ulSize = sizeof(COLLECTARRAYINFO);
	IMAGEINFO* infoInputImg = &m_Info.infoInputImg;
	infoCollectArray.DummyPixels = getDummyPixels(infoInputImg->lXResolution, m_nSensorVer);
	infoCollectArray.lScannerAvailableWidth = getMaxWidthWithoutDummyPixels(infoInputImg->lWidth, infoInputImg->lXResolution, m_nSensorVer);
    infoCollectArray.lInputHeight = infoInputImg->lHeight;
    infoCollectArray.nSensorVer = m_nSensorVer;
    
    m_CollectArrayForDuplex.getPtr()->setBackImage(imgBack);
	RTN result = execIP((CIPController<CIPBase>*)&m_CollectArrayForDuplex, imgFront, &infoCollectArray, imagestate);
	m_CollectArrayForDuplex.getPtr()->getBackImage(imgBack);
    // TODO infoに裏面の設定を入れるようにする。
	return result;

	IP_CATCH_BADALLOC
}
RTN CSpecialFilter::execMackOneLineImage(CImg& image)
{
	if (image.getBps() == 16) {
		long lSync = image.getSync();
		if (image.getSpp() == 3 && image.getRGBOrder() == LINE_ORDER) {
			lSync *= image.getSpp();
		}

		unsigned short* pTop = (unsigned short*)image.getImagePtr();
		long w = lSync / 2;
		while (w--) {
			long line = image.getHeight();
			unsigned short* p = pTop;
			long sum = 0;
			while (line--) {
				sum += *p;
				p = (unsigned short*)(((unsigned char*)p) + lSync);
			}
			*pTop++ = static_cast<unsigned short>(sum / image.getHeight());
		}
	}
    else if (image.getBps() == 8) {
		long lSync = image.getSync();
		if (image.getSpp() == 3 && image.getRGBOrder() == LINE_ORDER) {
			lSync *= image.getSpp();
		}
        
		unsigned char* pTop = image.getImagePtr();
		long w = lSync;
		while (w--) {
			long line = image.getHeight();
			unsigned char* p = pTop;
			long sum = 0;
			while (line--) {
				sum += *p;
				p += lSync;
			}
			*pTop++ = static_cast<unsigned char>(sum / image.getHeight());
		}
        
    }

	long lAvailWidth = getMaxWidthWithoutDummyPixels(m_Info.infoInputImg.lWidth, m_Info.infoInputImg.lXResolution, m_nSensorVer);
	long lNewWidth = image.getWidth();
	if (lNewWidth > lAvailWidth) {
		lNewWidth = lAvailWidth;
	}
	CImg imgDst;
	if (!imgDst.createImg(image.getXPos(), image.getYPos(), lNewWidth, 1, image.getSync(), image.getBps(), image.getSpp(), image.getRGBOrder(), image.getXResolution(), image.getYResolution())) {
		return RTN_PAR;
	}
	if (imgDst.isNull()) {
		return RTN_NOMEM;
	}
	memcpy(imgDst.getImagePtr(), image.getImagePtr(), imgDst.getImageSize());
	image.attachImg(imgDst);
	return RTN_OK;
}
DUMMYPIXELS CSpecialFilter::getDummyPixels(long lResolution, int nSensorVer)
{
    DUMMYPIXELS dummy;
	if (lResolution == 600) {
        dummy.lLeft = DUMMY_PIXEL_600[nSensorVer][0];
        dummy.lMiddle = DUMMY_PIXEL_600[nSensorVer][1];
        dummy.lRight = DUMMY_PIXEL_600[nSensorVer][2];
    }
	else {
        dummy.lLeft = DUMMY_PIXEL_300[nSensorVer][0];
        dummy.lMiddle = DUMMY_PIXEL_300[nSensorVer][1];
        dummy.lRight = DUMMY_PIXEL_300[nSensorVer][2];
    }
    return dummy;
}

//ダミー画素分を引く
long CSpecialFilter::getMaxWidthWithoutDummyPixels(long lMaxWidth, long lResolution, int nSensorVer)
{
	return lMaxWidth - getDummyPixelCount(lResolution, nSensorVer);
}
