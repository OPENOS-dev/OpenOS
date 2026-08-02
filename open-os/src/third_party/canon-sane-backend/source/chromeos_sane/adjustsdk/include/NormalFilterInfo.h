/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#pragma once
#include "CeiImg.h"

namespace Cei
{
	namespace LLiPm
	{
		typedef struct tagDETECTCOLORORGRAYINFO {
			unsigned long ulSize;
			long lSensitivityOfAutoColor;
			long lIntensityOfAutoColor;

			// V2
			enum DETECTMODE {
				COLOR_GRAY_BIN,
				COLOR_GRAY,
				COLOR_BIN
			} Mode;
			long lLength;						//INPUT 実際に読み込んだ原稿のサイズ
			long lTopMargin, lBottomMargin;		//INPUT スキャナが読み取った画像が原稿に対してつけたマージン量

			// V3
			long lSliceOfGrayBinary;
			long lIntensityOfGrayBinary;
		} DETECTCOLORORGRAYINFO, *LPDETECTCOLORORGRAYINFO;

#define DETECTCOLORORGRAYINFO_V1_SIZE		CCSIZEOF_STRUCT(DETECTCOLORORGRAYINFO, lIntensityOfAutoColor)
#define DETECTCOLORORGRAYINFO_V2_SIZE		CCSIZEOF_STRUCT(DETECTCOLORORGRAYINFO, lBottomMargin)
#define DETECTCOLORORGRAYINFO_V3_SIZE		CCSIZEOF_STRUCT(DETECTCOLORORGRAYINFO, lIntensityOfGrayBinary)

		typedef struct tagDROPOUTCOLORINFO {
			unsigned long ulSize;
			enum {
				DROP_NO,
				DROP_RED,
				DROP_GREEN,
				DROP_BLUE,
				DROP_CUSTOM
			} Type;
			LPCOLORREF lpColor;
			LPCOLORREF lpNewColors;
			int nColorCount;
			int nRange;
		} DROPOUTCOLORINFO, *LPDROPOUTCOLORINFO;

		typedef struct tagEMPHASISCOLORINFO {
			unsigned long ulSize;
			enum {
				EMPH_NO,
				EMPH_RED,
				EMPH_GREEN,
				EMPH_BLUE,
				EMPH_CUSTOM
			} Type;
			LPCOLORREF lpColor;
			LPCOLORREF lpNewColors;
			int nColorCount;
			int nRange;
		} EMPHASISCOLORINFO, *LPEMPHASISCOLORINFO;

		typedef struct tagERASEBACKPAGEINFO {
			unsigned long ulSize;			// struct size
			unsigned long ulCoefficient;	// erase level (0 - 6)
			unsigned long ulThreshold;		// no erase level (0 or 30)
		} ERASEBACKPAGEINFO, *LPERASEBACKPAGEINFO;

		typedef struct tagGRCINFO {
			unsigned long ulSize;		// struct size
			unsigned char ucBrightness;	// brightness (0:Use custom gamma, 1- 255:Use inner gamma with this brightness)
			unsigned char ucContrast;	// contrast (0:Use custom gamma, 1- 255:Use inner gamma with this brightness)
			struct {
				unsigned char tableGrayGamma[256];
				unsigned char tableRedGamma[256];
				unsigned char tableGreenGamma[256];
				unsigned char tableBlueGamma[256];
			};
		} GRCINFO, *LPGRCINFO;

		typedef struct tagEMPHASISEDGEINFO {
			unsigned long ulSize;
			unsigned long ulEmphasisLevel;
		} EMPHASISEDGEINFO, *LPEMPHASISEDGEINFO;

		typedef struct tagGRAYTOBINARYINFO {
			unsigned long ulSize;
			enum {
				BLM_SIMPLE = 0,
				BLM_ED = 1,
				BLM_ADAPTREGION = 8,
				BLM_SIMPLE_DB = 13,
				BLM_ACTIVETHRESHOLD = 129,		/* 別関数を呼ぶのでこれは例外 */
			} Method;
			unsigned long ulBrightness;
			unsigned long ulContrast;
		} GRAYTOBINARYINFO, *LPGRAYTOBINARYINFO;

		typedef struct tagROTATE90XINFO {
			unsigned long ulSize;
			long lDegree;
		} ROTATE90XINFO, *LPROTATE90XINFO;

		typedef struct tagBLANKPAGEINFO {
			unsigned long ulSize;
			long lBlankSkipSlice;	//画像のエッジリミット値
			bool bBlankPage;		//ブランクスキップの結果（true:白紙画像，false:黒紙画像）
		} BLANKPAGEINFO, *LPBLANKPAGEINFO;

		typedef struct tagTEXTIMAGEDIRECTIONINFO {
			unsigned long ulSize;
			enum TID_LANG {
				TID_JAPANESE	= 0x00000001,
				TID_ENGLISH		= 0x00000002,
				TID_CHINESE		= 0x00000004,
				TID_KOREAN		= 0x00100000,
				TID_FRENCH		= 0x00000008,
				TID_ITALIAN		= 0x00000010,
				TID_GERMAN		= 0x00000020,
				TID_SPANISH		= 0x00000040,
				TID_DUTCH		= 0x00000080,
				TID_PORTUGUESE	= 0x00000100,
				TID_ALBANIAN	= 0x00000200,
				TID_CATALAN		= 0x00000400,
				TID_DANISH		= 0x00000800,
				TID_FINNISH		= 0x00001000,
				TID_ICELANDIC	= 0x00002000,
				TID_NORWEGIAN	= 0x00004000,
				TID_SWEDISH		= 0x00008000,
				TID_FAROESE		= 0x00010000,
			};
			int tidlang;
			int tidcodepage;
			int nDeg;
		} TEXTIMAGEDIRECTIONINFO, *LPTEXTIMAGEDIRECTIONINFO;

		typedef struct tagEPUBFILTERINFO {
			unsigned long ulSize;
			unsigned long ulLevel;	// level (1 - 6)
		} EPUBFILTERINFO, *LPEPUBFILTERINFO;

		typedef struct tagBINIPFILTERINFO {
			unsigned long ulSize;
            typedef enum tagTYPE {
                BINFLT_NONE = 0,
                BINFLT_ISOLATE = 0x01,
                BINFLT_NOTCH = 0x02,
            } TYPE;
			int type; // TYPE flags
		} BINIPFILTERINFO, *LPBINIPFILTERINFO;
        
		typedef struct tagSKEWCORRECTIONINFO {
			unsigned long ulSize;
			typedef enum tagSKEWCORRECTTYPE {
				SKEWCORRECT_PAPER = 0,
				SKEWCORRECT_CONTENTS,
			} SKEWCORRECTTYPE;
			SKEWCORRECTTYPE type;
            
            // v2
            typedef enum tagFILLCOLOR {
                COLOR_SHADING = 0,
                COLOR_WHITE,
                COLOR_BLACK,
            } FILLCOLOR;
            FILLCOLOR color;
		} SKEWCORRECTIONINFO;
        
#define SKEWCORRECTIONINFO_V1_SIZE		CCSIZEOF_STRUCT(SKEWCORRECTIONINFO, type)
#define SKEWCORRECTIONINFO_V2_SIZE		CCSIZEOF_STRUCT(SKEWCORRECTIONINFO, color)

		typedef struct tagNORMALFILTERINFO {
			unsigned long ulSize;
			bool bReduceMoire;
			DETECTCOLORORGRAYINFO* pDetectColorOrGrayInfo;
			DROPOUTCOLORINFO* pDropOutColorInfo;
			EMPHASISCOLORINFO* pEmphasisColorInfo;
			ERASEBACKPAGEINFO* pEraseBackPageInfo;
			GRCINFO* pGRCInfo;
			EMPHASISEDGEINFO* pEmphasisEdgeInfo;
			bool bAutoSize;
			bool bSkewCorrection;
			TEXTIMAGEDIRECTIONINFO* pTextImageDirectionInfo;
			ROTATE90XINFO* pRotate90xInfo;
			GRAYTOBINARYINFO* pGrayToBinaryInfo;
			bool bInverse;
			BLANKPAGEINFO* pBlankPageInfo;

			//V2
			long bDetectResolution;
			long bColorSaturationInfo;

			//V3
			EPUBFILTERINFO* pEPubFilterInfo;

			//V4
			SKEWCORRECTIONINFO* pSkewCorrectionInfo;	// NULLの場合は斜行補正の設定はデフォルトが使用されます
			long bPhotoMode;
            
            //V5
            BINIPFILTERINFO* pBinIPFilterInfo;
		} NORMALFILTERINFO, *LPNORMALFILTERINFO;

#define NORMALFILTERINFO_V1_SIZE		CCSIZEOF_STRUCT(NORMALFILTERINFO, pBlankPageInfo)
#define NORMALFILTERINFO_V2_SIZE		CCSIZEOF_STRUCT(NORMALFILTERINFO, bColorSaturationInfo)
#define NORMALFILTERINFO_V3_SIZE		CCSIZEOF_STRUCT(NORMALFILTERINFO, pEPubFilterInfo)
#define NORMALFILTERINFO_V4_SIZE		CCSIZEOF_STRUCT(NORMALFILTERINFO, bPhotoMode)
#define NORMALFILTERINFO_V5_SIZE		CCSIZEOF_STRUCT(NORMALFILTERINFO, pBinIPFilterInfo)


		typedef struct tagNORMALFILTERSIMPLEXINFO {
			unsigned long ulSize;
			IMAGEINFO infoInputImg;		//画像メモリは使わない画像情報のみ
			IMAGEINFO infoOutputImg;	//画像メモリは使わない画像情報のみ
			NORMALFILTERINFO infoNormal;
		} NORMALFILTERSIMPLEXINFO, *LPNORMALFILTERSIMPLEXINFO;

#define NORMALFILTERSIMPLEXINFO_V1_SIZE		(sizeof(NORMALFILTERSIMPLEXINFO) - (sizeof(NORMALFILTERINFO) - (NORMALFILTERINFO_V1_SIZE)))
#define NORMALFILTERSIMPLEXINFO_V2_SIZE		(sizeof(NORMALFILTERSIMPLEXINFO) - (sizeof(NORMALFILTERINFO) - (NORMALFILTERINFO_V2_SIZE)))
#define NORMALFILTERSIMPLEXINFO_V3_SIZE		(sizeof(NORMALFILTERSIMPLEXINFO) - (sizeof(NORMALFILTERINFO) - (NORMALFILTERINFO_V3_SIZE)))
#define NORMALFILTERSIMPLEXINFO_V4_SIZE		(sizeof(NORMALFILTERSIMPLEXINFO) - (sizeof(NORMALFILTERINFO) - (NORMALFILTERINFO_V4_SIZE)))
#define NORMALFILTERSIMPLEXINFO_V5_SIZE		(sizeof(NORMALFILTERSIMPLEXINFO) - (sizeof(NORMALFILTERINFO) - (NORMALFILTERINFO_V5_SIZE)))


		typedef struct tagNORMALFILTERDUPLEXINFO {
			unsigned long ulSize;
			IMAGEINFO infoInputImg;		//画像メモリは使わない画像情報のみ
			IMAGEINFO infoOutputImg;	//画像メモリは使わない画像情報のみ
			NORMALFILTERINFO infoNormal[2];
			bool bPutImageOnSide;
		} NORMALFILTERDUPLEXINFO, *LPNORMALFILTERDUPLEXINFO;

#define NORMALFILTERDUPLEXINFO_V1_SIZE		(sizeof(NORMALFILTERDUPLEXINFO) - (sizeof(NORMALFILTERINFO) - (NORMALFILTERINFO_V1_SIZE)) * 2)
#define NORMALFILTERDUPLEXINFO_V2_SIZE		(sizeof(NORMALFILTERDUPLEXINFO) - (sizeof(NORMALFILTERINFO) - (NORMALFILTERINFO_V2_SIZE)) * 2)
#define NORMALFILTERDUPLEXINFO_V3_SIZE		(sizeof(NORMALFILTERDUPLEXINFO) - (sizeof(NORMALFILTERINFO) - (NORMALFILTERINFO_V3_SIZE)) * 2)
#define NORMALFILTERDUPLEXINFO_V4_SIZE		(sizeof(NORMALFILTERDUPLEXINFO) - (sizeof(NORMALFILTERINFO) - (NORMALFILTERINFO_V4_SIZE)) * 2)
#define NORMALFILTERDUPLEXINFO_V5_SIZE		(sizeof(NORMALFILTERDUPLEXINFO) - (sizeof(NORMALFILTERINFO) - (NORMALFILTERINFO_V5_SIZE)) * 2)
        
	}
}


