/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#pragma once
#include "IPBase.h"
#include "Dependencies.h"

namespace Cei
{
	namespace LLiPm
	{
		namespace DR_NAMESPACE
		{
			typedef struct tagCOLLECTARRAYINFO {
				unsigned long ulSize;
				DUMMYPIXELS DummyPixels;
				long lScannerAvailableWidth;
                long lInputHeight;
				long bflatbed;
                int nSensorVer;
			} COLLECTARRAYINFO, *LPCOLLECTARRAYINFO;

			class CCollectArray : public CIPBase
			{
			public:
				CCollectArray(void);
				virtual ~CCollectArray(void);
			public:
				virtual void getBackImage(CImg& image) {}
                virtual void setBackImage(CImg& image) {}
			protected:
				static void ArrayCollection(unsigned char* pSrc, unsigned char* pDst, long lWidth, long lXResolution, int nSensorVer);
				static void ArrayCollection(unsigned char* pSrc, unsigned char* pDst, long lWidth, DUMMYPIXELS& dummy);
				static void Separate(unsigned char* lpDst1, unsigned char* lpDst2, unsigned char* pSrc, long lWidth, long lXResolution, int nSensorVer);
				static void ArrayCollectPara(unsigned char* pDst, unsigned char* pSrc, long paranum, long AvailBits);
				static void ArrayCollectPara(unsigned short* pDst, unsigned short* pSrc, long paranum, long AvailBits);
				static void Extend12To16BitAndArrayCollect(unsigned short* lpDst, unsigned char* lpSrc, long lWidth, int xRes, int nSensorVer);
				static void Extend12To16BitAndSeparate(unsigned short* lpDst1, unsigned short* lpDst2, unsigned char* lpSrc, long lWidth, int xRes, int nSensorVer);
				static void Extend12To16BitCore(unsigned short* lpDst, unsigned char* lpSrc, long lWidth);
                static void Extend8To16BitCore(unsigned short* lpDst, unsigned char* lpSrc, long lWidth);
			};
		}
	}
}
