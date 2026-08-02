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
		namespace DRP215
		{
			static const unsigned long LIGHTCURVEDATASIZE = 512 * 1024;
			static const unsigned long LIGHTCURVEDATAADDRESS = 0x10080000;

			typedef struct tagADJUSTINFO {
				unsigned long ulSize;
				bool bDuplex;						
				long lXResolution;					
				ColorMode ScanMode;					
				enum {
					LIGHT_NORMAL,
					DROPOUT_RED,
					DROPOUT_GREEN,
					DROPOUT_BLUE,
					EMPHASIS_RED,
					EMPHASIS_GREEN,
					EMPHASIS_BLUE,
				} FrontLightSorce, BackLightSorce;		
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
				} FrontAdjustInfo, BackAdjustInfo;
				struct {
					unsigned char MainWindowID;
					unsigned char SubWindowID;
				} ScanInfo;						
				bool bUse;						
			} ADJUSTINFO, *LPADJUSTINFO;

			typedef struct tagSHADINGINFO {
				unsigned long ulSize;
				CImg imgWhite;
				CImg imgBlack;
			} SHADINGINFO, *LPSHADINGINFO;

			typedef struct tagSPECIALFILTERINFO {
				unsigned long ulSize;
				SHADINGINFO *pShadingInfo;
				bool bCardScan;
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



