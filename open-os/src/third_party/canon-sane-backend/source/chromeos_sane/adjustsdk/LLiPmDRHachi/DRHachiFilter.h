/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#pragma once
#include "NormalFilter.h"
#include "AdjustLight.h"
#include "CollectArrayForSimplex.h"
#include "CollectArrayForDuplex.h"
#include "ExtendBitData12To16.h"
#include "Shading.h"
#include "MakePage.h"

#if defined(JPEG_IMPORT) || defined(JPEG_IMPORT_SEPARATED)
#   include "CollectArrayForJpeg.h"
#   define CCollectForSimplex   CCollectArrayForJpeg(false)
#   define CCollectForDuplex    CCollectArrayForJpeg(true)
#   define CCollectIPTYPE       CIPController<CCollectArray>::IPCOMPONLY
#else
#   define CCollectForSimplex   CCollectArrayForSimplex
#   define CCollectForDuplex    CCollectArrayForDuplex
#   define CCollectIPTYPE       CIPController<CCollectArray>::IPANY
#endif


#include DRFilterInfoHeader

namespace Cei
{
	namespace LLiPm
	{
		namespace DR_NAMESPACE
		{
			class CSpecialFilter : public CNormalFilter
			{
			public:
				CSpecialFilter(void);
				~CSpecialFilter(void);
				const char* const getName(void) const {return "SpecialFilter";}
                void clear();
			protected:
				RTN IP(CImg& image);
				RTN IPFirst(CImg& image);
				RTN IPMiddle(CImg& image);
				RTN IPLast(CImg& image);
				RTN setInfo(CImg& image, void* lpInfo);
				RTN setInfoFirst(CImg& image, void* lpInfo) {return setInfo(image, lpInfo);}
				RTN setInfoMiddle(CImg& image, void* lpInfo) {return setInfo(image, lpInfo);}
				RTN setInfoLast(CImg& image, void* lpInfo) {return setInfo(image, lpInfo);}
				FILTERDUPLEXINFO m_Info;
				CImg m_imgBack;
			private:
				RTN checkParamError(SIDE side);
			public:
				void getBackImage(CImg& image);
                void setBackImage(CImg& image);
				void AdjustLightFirst(ADJUSTINFO* lpInfo, long lSensorId = 0);
				RTN AdjustLightNext(CImg& img, ADJUSTINFO* lpInfo);
				RTN AdjustLightLast(CImg& imgDstFront, CImg& imgDstBack, CImg& imgSrc, ADJUSTINFO* lpInfo);
				RTN makeShadingData(CImg& imgWhite, CImg& imgBlack, ADJUSTINFO* lpInfo, SIDE Side, unsigned char* lpData, unsigned long ulSize);
			protected:
				RTN DRHachiFilter(CImg& image, IMAGESTATE imagestate);
			private:
				RTN execCollectArrayForSimplex(CImg& image, SIDE side, IMAGESTATE imagestate);
				RTN execExtendBitData12To16(CImg& image, SIDE side, IMAGESTATE imagestate);
				RTN execCollectArrayForDuplex(CImg& imgFront, CImg& imgBack, IMAGESTATE imagestate);
				RTN execMackOneLineImage(CImg& image);			
			protected:
				CAdjustLight m_AdjustLight;
				CIPController<CCollectArray> m_CollectArrayForSimplex[2];
				CIPController<CExtendBitData12To16> m_ExtendBitData12To16;
				CIPController<CCollectArray> m_CollectArrayForDuplex;		
				CIPController<CShading> m_Shading[2];
				CIPController<CMakePage> m_MakePage[2];
			public:
				inline void setOffset(long lOffset) {m_lOffset = lOffset;}
				inline long getOffset(long lResolution) {return m_lOffset * lResolution / 25400;}
			private:
				long m_lOffset;
			public:
				inline long getDummyPixelCount(long lResolution, int nSensorVer) {
					DUMMYPIXELS dummy = getDummyPixels(lResolution, nSensorVer);
					return dummy.lLeft + dummy.lMiddle + dummy.lRight;
				}
			private:
				DUMMYPIXELS getDummyPixels(long lResolution, int nSensorVer);
				long getMaxWidthWithoutDummyPixels(long lMaxWidth, long lResolution, int nSensorVer);
			};
		}
	}
}
