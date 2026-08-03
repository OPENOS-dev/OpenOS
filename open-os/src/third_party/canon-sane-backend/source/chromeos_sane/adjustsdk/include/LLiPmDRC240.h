/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#pragma once
#include "DRC240FilterInfo.h"

#define LLIPM_VERSION "1.5.4.0"

namespace Cei
{
	namespace LLiPm
	{
		namespace DRC240
		{
			long getOffset(long lResolution);
			void setOffset(long lOffset);
			long getDummyPixelCount(long lResolution, int nSensorVer);
			RTN FilterSimplex(CImg& imgDst, CImg& imgSrc, FILTERSIMPLEXINFO* lpInfo);
			RTN FilterSimplexFirst(CImg& imgDst, CImg& imgSrc, FILTERSIMPLEXINFO* lpInfo);
			RTN FilterSimplexMiddle(CImg& imgDst, CImg& imgSrc, FILTERSIMPLEXINFO* lpInfo);
			RTN FilterSimplexLast(CImg& imgDst, CImg& imgSrc, FILTERSIMPLEXINFO* lpInfo);
			RTN FilterDuplex(CImg& imgDstFront, CImg& imgDstBack, CImg& imgSrc, FILTERDUPLEXINFO* lpInfo);
			RTN FilterDuplex(CImg& imgDstFront, CImg& imgDstBack, CImg& imgSrcFront, CImg& imgSrcBack, FILTERDUPLEXINFO* lpInfo);
			RTN FilterDuplexFirst(CImg& imgDstFront, CImg& imgDstBack, CImg& imgSrc, FILTERDUPLEXINFO* lpInfo);
			RTN FilterDuplexMiddle(CImg& imgDstFront, CImg& imgDstBack, CImg& imgSrc, FILTERDUPLEXINFO* lpInfo);
			RTN FilterDuplexLast(CImg& imgDstFront, CImg& imgDstBack, CImg& imgSrc, FILTERDUPLEXINFO* lpInfo);
			RTN FilterDuplexFirst(CImg& imgDstFront, CImg& imgDstBack, CImg& imgSrcFront, CImg& imgSrcBack, FILTERDUPLEXINFO* lpInfo);
			RTN FilterDuplexMiddle(CImg& imgDstFront, CImg& imgDstBack, CImg& imgSrcFront, CImg& imgSrcBack, FILTERDUPLEXINFO* lpInfo);
			RTN FilterDuplexLast(CImg& imgDstFront, CImg& imgDstBack, CImg& imgSrcFront, CImg& imgSrcBack, FILTERDUPLEXINFO* lpInfo);
			RTN NormalFilterSimplex(CImg& imgDst, CImg& imgSrc, FILTERSIMPLEXINFO* lpInfo, bool bFront = true);
			RTN NormalFilterFolio(CImg& imgDst, CImg& imgSrc, FILTERDUPLEXINFO* lpInfo);
            void AdjustLightFirst(ADJUSTINFO* lpInfo, long lSensorId);
            RTN AdjustLightNext(CImg& image, ADJUSTINFO* lpInfo);
			RTN AdjustLightLast(CImg& imgDstFront, CImg& imgDstBack, CImg& imgSrc, ADJUSTINFO* lpInfo);
			RTN AdjustLightFix(CImg& imgWhite, CImg& imgBlack, ADJUSTINFO* lpInfo, SIDE Side, unsigned char* lpData = NULL, unsigned long ulSize = 0);
            void Clear();
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

