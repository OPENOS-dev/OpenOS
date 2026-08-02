/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#pragma once
#include "NormalFilterInfo.h"

namespace Cei
{
	namespace LLiPm
	{
		namespace DRC240
		{
			static const unsigned long LIGHTCURVEDATASIZE = 512 * 1024;		//導光体補正データのサイズ
			static const unsigned long LIGHTCURVEDATAADDRESS = 0x10080000;	//導光体補正データの先頭アドレス

			typedef struct tagADJUSTINFO {
				unsigned long ulSize;
				bool bDuplex;						//入力	両面or片面
				long lXResolution;					//入力	スキャナにセットしたX方向解像度（300or600）
				ColorMode ScanMode;					//入力	シェーディング後にスキャンするときのモード
				enum {
					LIGHT_NORMAL,
					DROPOUT_RED,
					DROPOUT_GREEN,
					DROPOUT_BLUE,
					EMPHASIS_RED,
					EMPHASIS_GREEN,
					EMPHASIS_BLUE,
				} FrontLightSorce, BackLightSorce;			//入力	スキャンするときのスキャナに対するドロップアウト、色強調設定
				struct {
					unsigned char Gain1;
					unsigned char Gain2;
					unsigned char Gain3;
					unsigned char Reserved1;
					unsigned char Offset1;
					unsigned char Offset2;
					unsigned char Offset3;
					unsigned char Reserved2;
					unsigned short RedLED;
					unsigned short GreenLED;
					unsigned short BlueLED;
					unsigned short Reserved3;
					unsigned short Reserved4;
					unsigned short Reserved5;
				} FrontAdjustInfo, BackAdjustInfo;	//入出力	前の補正スキャンで使用した設定値、次の補正スキャンで使用する設定値
				struct {
					unsigned char MainWindowID;
					unsigned char SubWindowID;
				} ScanInfo;							//出力	補正スキャンの設定
				bool bUse;							//出力	設定値調整が完了したらtrueになる
			} ADJUSTINFO, *LPADJUSTINFO;

            typedef struct tagFILTERINFO {
				unsigned long ulSize;
                enum {
                    EXECUTE_ONLY_CORRECTARRAY,
                    DISABLED_CORRECTARRAY,
                } mode;
            } FILTERINFO;
            
			typedef struct tagSHADINGINFO {
				unsigned long ulSize;
				CImg imgWhite;
				CImg imgBlack;
			} SHADINGINFO, *LPSHADINGINFO;

			typedef struct tagSRGBCONVERSIONINFO {
				unsigned long ulSize;
				enum {
					SRGB_FEEDER,
					SRGB_FLATBED,
					SRGB_CUSTOM
				} Type;
				long Matrix[3][3];
			} SRGBCONVERSIONINFO, *LPSRGBCONVERSIONINFO;

			typedef struct tagDETECT4POINTSINFO {
				unsigned long ulSize;
				POINT LeftTop;			//OUTPUT 4点 左上
				POINT RightTop;			//OUTPUT 4点 右上
				POINT LeftBottom;		//OUTPUT 4点 左下
				POINT RightBottom;		//OUTPUT 4点 右下
				RECT rectDoc;			//OUTPUT サイズ検知結果
				POINT vectorSlant;		//OUTPUT 傾き検知結果(ベクトル)
				SIZE Size;				//INPUT 実際に読み込んだ原稿のサイズ
				long  lTopMargin, lBottomMargin, lLeftMargin, lRightMargin;	//INPUT スキャナが読み取った画像が原稿に対してつけたマージン量
                
				// V2
				long  lTopTrim, lBottomTrim, lLeftTrim, lRightTrim;	//INPUT 検知結果から４辺を削る量（単位は入力画像の解像度）
                
                // LLiPm Extension Version
                long lForceBaseLine; //INPUT BaseLineを外部から強制的に設定する　0:自動判定 0x01:影に合わせる 0x7:影と逆に合わせる
                
                // V3
                bool bCarrierSheetMode;
			} DETECT4POINTSINFO, *LPDETECT4POINTSINFO;
            
#define DETECT4POINTSINFO_V1_SIZE		CCSIZEOF_STRUCT(DETECT4POINTSINFO, lRightMargin)
#define DETECT4POINTSINFO_V2_SIZE		CCSIZEOF_STRUCT(DETECT4POINTSINFO, lRightTrim)
#define DETECT4POINTSINFO_VEX_SIZE		CCSIZEOF_STRUCT(DETECT4POINTSINFO, lForceBaseLine)
#define DETECT4POINTSINFO_V3_SIZE		CCSIZEOF_STRUCT(DETECT4POINTSINFO, bCarrierSheetMode)

			typedef struct tagFEEDINGDIRECTIONINFO {
				unsigned long ulSize;				//INPUT 
				unsigned long ulDirection;			//INPUT 
				long lLength;						//INPUT 実際に読み込んだ原稿のサイズ
				long lTopMargin, lBottomMargin;		//INPUT スキャナが読み取った画像が原稿に対してつけたマージン量
			} FEEDINGDIRECTIONINFO, *LPFEEDINGDIRECTIONINFO;

			typedef struct tagREMOVESHADOWINFO {
				unsigned long ulSize;				//INPUT 
				long lLength;						//INPUT 実際に読み込んだ原稿のサイズ
				long lScannerTopMargin, lScannerBottomMargin;		//INPUT スキャナが読み取った画像が原稿に対してつけたマージン量
			} REMOVESHADOWINFO, *LPREMOVESHADOWINFO;
            
            typedef struct tagCOLORGAPINFO {
                unsigned long ulSize;
            } COLORGAPINFO, *LPCOLORGAPINFO;

			typedef struct tagSPECIALFILTERINFO {
				unsigned long ulSize;
				SHADINGINFO *pShadingInfo;
				SRGBCONVERSIONINFO *pSRGBConversionInfo;
				DETECT4POINTSINFO* pDetect4PointsInfo;
				FEEDINGDIRECTIONINFO* pFeedingDirectionInfo;
				REMOVESHADOWINFO* pRemoveShadowInfo;
				bool bCardScan;
                COLORGAPINFO* pColorGapInfo;
                FILTERINFO* pFilterInfo;                // NULL : default, !NULL : obey FILTERINFO::mode
			} SPECIALFILTERINFO, *LPSPECIALFILTERINFO;

			typedef struct tagFILTERSIMPLEXINFO : public NORMALFILTERSIMPLEXINFO {
				SPECIALFILTERINFO infoSpecial;
			} FILTERSIMPLEXINFO, *LPFILTERSIMPLEXINFO;

			typedef struct tagFILTERDUPLEXINFO : public NORMALFILTERDUPLEXINFO {
				SPECIALFILTERINFO infoSpecial[2];
			} FILTERDUPLEXINFO, *LPFILTERDUPLEXINFO;
		}
	}
}



