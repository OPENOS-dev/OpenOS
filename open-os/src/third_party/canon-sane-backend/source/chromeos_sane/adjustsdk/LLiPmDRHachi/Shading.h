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
			class CShading : public CIPBase
			{
			public:
				CShading(void);
				~CShading(void);
				const char* const getName(void) const {return "Shading";}
			protected:
				RTN IP(CImg& image);
				RTN IPFirst(CImg& image);
				RTN IPMiddle(CImg& image);
				RTN IPLast(CImg& image);
				RTN setInfo(CImg& image, void* lpInfo);
				RTN setInfoFirst(CImg& image, void* lpInfo) {return setInfo(image, lpInfo);}
				RTN setInfoMiddle(CImg& image, void* lpInfo) {return RTN_OK;}
				RTN setInfoLast(CImg& image, void* lpInfo) {return RTN_OK;}
				SHADINGINFO m_Info;
			private:
				RTN Shading(CImg& image);
				RTN ShadingColor(CImg& imgDst, CImg& imgSrc);
				RTN ShadingGray(CImg& imgDst, CImg& imgSrc);
				void ShadingColorCore_OrderLine2Pixel_SIMD(unsigned char* lpDst, unsigned char* lpSrc, long lSync, unsigned short* lpWhite, long lShadSync, unsigned short* lpBlack, long w);
				void ShadingColorCore_OrderLine2Pixel_NEON(unsigned char* lpDst, unsigned char* lpSrc, long lSync, unsigned short* lpWhite, long lShadSync, unsigned short* lpBlack, long w);
				void ShadingColorCore_OrderLine2Pixel_NonSIMD(unsigned char* lpDst, unsigned char* lpSrc, long lSync, unsigned short* lpWhite, long lShadSync, unsigned short* lpBlack, long w);
				void ShadingGrayCore_SIMD(unsigned char* lpDst, unsigned char* lpSrc, unsigned short* lpWhite, unsigned short* lpBlack, long w);
				void ShadingGrayCore_NEON(unsigned char* lpDst, unsigned char* lpSrc, unsigned short* lpWhite, unsigned short* lpBlack, long w);
				void ShadingGrayCore_NonSIMD(unsigned char* lpDst, unsigned char* lpSrc, unsigned short* lpWhite, unsigned short* lpBlack, long w);
				void mulImage(CImg& img, long mul);
				RTN pack8OnUpperByteImage(CImg& image);
			public:

				//補正スキャンの結果から導光体補正などをしてシェーディングデータを作る
				RTN makeShadingData(CImg& imgWhite, CImg& imgBlack, CImg& imgWhiteOrg, CImg& imgBlackOrg, int nSensorVer);
                RTN makeShadingData(CImg& imgWhiteOrg);
                void formatShadingData9(CImg& imgWhite, CImg& imgBlack, CImg& imgWhiteOrg);

				//プラテンの画像を取得する
				RTN fixPlatenImage(CImg& image);
				RTN fixPlatenImage(CImg& image, RECT ignore);
			private:
				typedef struct tagSHADING_AVARAGE
				{
					WORD wGray;
					WORD wRed;
					WORD wGreen;
					WORD wBlue;
					bool isColor;
				} SHADING_AVARAGE, *LPSHADING_AVARAGE;
				SHADING_AVARAGE m_avgPlaten;

				RTN makeShadingAvarage16(CImg& shadingdata, SHADING_AVARAGE& target);
				RTN makeShadingAvarage8(CImg& shadingdata, SHADING_AVARAGE& target);
				SHADING_AVARAGE calcPlatenColor(const SHADING_AVARAGE& avgWhite, const SHADING_AVARAGE& avgBlack, const SHADING_AVARAGE& avgWhiteOrg);
			private:
				void makeWhiteDataLine(unsigned short* p, long len, int tgt);
				void makeWhiteDataLineColor(unsigned short* p, long len, int tgtR, int tgtG, int tgtB);
			};
		}
	}
}
