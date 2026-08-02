/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/

#pragma once
#include "DRP208FilterInfo.h"

namespace Cei
{
	namespace LLiPm
	{
		namespace DRP208
		{
			long getOffset(long lResolution);
			void setOffset(long lOffset);
			long getDummyPixelCount(long lResolution, int nSensorVer);
			int setOcrPath(const char* lpFilePath);
			RTN FilterSimplex(CImg& imgDst, CImg& imgSrc, FILTERSIMPLEXINFO* lpInfo);
			RTN FilterSimplexFirst(CImg& imgDst, CImg& imgSrc, FILTERSIMPLEXINFO* lpInfo);
			RTN FilterSimplexMiddle(CImg& imgDst, CImg& imgSrc, FILTERSIMPLEXINFO* lpInfo);
			RTN FilterSimplexLast(CImg& imgDst, CImg& imgSrc, FILTERSIMPLEXINFO* lpInfo);
			RTN FilterDuplex(CImg& imgDstFront, CImg& imgDstBack, CImg& imgSrc, FILTERDUPLEXINFO* lpInfo);
			RTN FilterDuplexFirst(CImg& imgDstFront, CImg& imgDstBack, CImg& imgSrc, FILTERDUPLEXINFO* lpInfo);
			RTN FilterDuplexMiddle(CImg& imgDstFront, CImg& imgDstBack, CImg& imgSrc, FILTERDUPLEXINFO* lpInfo);
			RTN FilterDuplexLast(CImg& imgDstFront, CImg& imgDstBack, CImg& imgSrc, FILTERDUPLEXINFO* lpInfo);
			RTN NormalFilterSimplex(CImg& imgDst, CImg& imgSrc, FILTERSIMPLEXINFO* lpInfo, bool bFront = true);
			RTN NormalFilterFolio(CImg& imgDst, CImg& imgSrc, FILTERDUPLEXINFO* lpInfo);
			RTN DropOutColor(CImg& img, DROPOUTCOLORINFO* lpInfo);
			RTN EmphasisColor(CImg& img, EMPHASISCOLORINFO* lpInfo);
			RTN EraseBackPage(CImg& img, ERASEBACKPAGEINFO* lpInfo);
			RTN ColorToGray(CImg& img);
			RTN GRC(CImg& img, GRCINFO* lpInfo, ColorMode mode, bool bBinED);
			RTN EmphasisEdge(CImg& img, EMPHASISEDGEINFO* lpInfo);
			RTN GrayToBinary(CImg& img, GRAYTOBINARYINFO* lpInfo);
			RTN Inverse(CImg& img);
			RTN PutImageOnSide(CImg& imgDst, CImg& imgSrcLeft, CImg& imgSrcRight);
			RTN TextImageDirection(CImg& img, TEXTIMAGEDIRECTIONINFO* lpInfo);
			RTN Rotate90x(CImg& img, ROTATE90XINFO* lpInfo);
			RTN IsBlankPage(CImg& img, BLANKPAGEINFO* lpInfo);
			void AdjustLightFirst(ADJUSTINFO* lpInfo, long lSensorId);
			RTN AdjustLightNext(CImg& image, ADJUSTINFO* lpInfo);
			RTN AdjustLightLast(CImg& imgDstFront, CImg& imgDstBack, CImg& imgSrc, ADJUSTINFO* lpInfo);
			RTN AdjustLightFix(CImg& imgWhite, CImg& imgBlack, ADJUSTINFO* lpInfo, SIDE Side, unsigned char* lpData, unsigned long ulSize);

            void* createLLiPm();
            void deleteLLiPm(void* llipm);
			RTN FilterSimplex(void* llipm, CImg& imgDst, CImg& imgSrc, FILTERSIMPLEXINFO* lpInfo);
			RTN FilterSimplexFirst(void* llipm, CImg& imgDst, CImg& imgSrc, FILTERSIMPLEXINFO* lpInfo);
			RTN FilterSimplexMiddle(void* llipm, CImg& imgDst, CImg& imgSrc, FILTERSIMPLEXINFO* lpInfo);
			RTN FilterSimplexLast(void* llipm, CImg& imgDst, CImg& imgSrc, FILTERSIMPLEXINFO* lpInfo);
			RTN FilterDuplex(void* llipm, CImg& imgDstFront, CImg& imgDstBack, CImg& imgSrc, FILTERDUPLEXINFO* lpInfo);
			RTN FilterDuplexFirst(void* llipm, CImg& imgDstFront, CImg& imgDstBack, CImg& imgSrc, FILTERDUPLEXINFO* lpInfo);
			RTN FilterDuplexMiddle(void* llipm, CImg& imgDstFront, CImg& imgDstBack, CImg& imgSrc, FILTERDUPLEXINFO* lpInfo);
			RTN FilterDuplexLast(void* llipm, CImg& imgDstFront, CImg& imgDstBack, CImg& imgSrc, FILTERDUPLEXINFO* lpInfo);
			RTN AdjustLightFix(void* llipm, CImg& imgWhite, CImg& imgBlack, ADJUSTINFO* lpInfo, SIDE Side, unsigned char* lpData = NULL, unsigned long ulSize = 0);
        }
	}
}

