/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/


#pragma once
#include "IPBase.h"
#include "Dependencies.h"
#include "CeiRaster.h"

namespace Cei
{
	namespace LLiPm
	{
		namespace DR_NAMESPACE
		{

			#pragma pack(push, 2)

			typedef struct tagLCAFILEHEADER {
				DWORD dwTimestamp;	// The date of this data. time_t
				DWORD dwVersion;	// Data struct version
				DWORD dwNumData;	// the number of the pair of LCAINFOHEADER and data
			} LCAFILEHEADER, *LPLCAFILEHEADER;

			typedef struct tagLCAINFOHEADER {
				WORD wSize;		// size of this structure
				WORD wSide;		// Front is 0, Back is 1
				WORD wSpp;		// Color is 3, Gray is 1
				WORD wDataBits;	// 1 : The unit is BYTE. 
								// 2 : The unit is WORD. 
								// 4 : The unit is DWORD
				DWORD dwDenom;	// fixed denominator
								// pix' = pix * data / dwDenom
								//   pix is raw bytes data
								//   data is LCA data
				DWORD dwWidth;	// width
				DWORD dwSync;	// sync
				DWORD dwDataSize;	// width in bytes
				WORD wResolution;	// XResolution
				WORD wOrder;		// Color format   0:pixel order  1:line order
				DWORD dwFlags;		// Data flags
				// Version 2
				BYTE dDropout;	// Dropout Color None is 0, Red is 1, Green is 2, Blue is 3
				BYTE dEmphasis;	// Emphasis Color None is 0, Red is 1, Green is 2, Blue is 3
			} LCAINFOHEADER, *LPLCAINFOHEADER;

			// wSide
			#define SIDE_FRONT			0
			#define SIDE_BACK			1
			// dwFlags
			#define LCA_DATA_RAW			0x00
			#define LCA_DATA_FILTERED	0x01

			#pragma pack(pop)

			class CLightCurve : public CCeiRaster, public LCAINFOHEADER {
			public :
				inline PUINT GetPtr(int iIndex = 0);
				inline bool SetLcInfomation(LCAINFOHEADER &info);
				inline void SetAt(int iIndex, UINT data);
			};

			class CLightCurveAdjustData {
			protected :
				DWORD m_nNumberOfData;
			public :
				CLightCurve *m_pCurve;
				CLightCurveAdjustData();
				~CLightCurveAdjustData();
				inline RTN SetSize(DWORD dwSize);
				inline void Release();

				virtual int CheckTimeStamp(unsigned char* lpData, unsigned long ulSize, DWORD dwTimeStamp);
				virtual RTN LoadData(unsigned char* lpData, unsigned long ulSize);
				RTN AdjustData(IMAGEINFO &Black, IMAGEINFO &White, SIDE Side, int nDropout, int nEmphasis);
			private:
				WORD LightCurveAdjPixel(WORD w, WORD b, UINT num, UINT denom);

				// Get helper function
				inline BYTE GetByte(LPVOID p, int iIndex);
				inline WORD GetWord(LPVOID p, int iIndex);
				inline DWORD GetDWord(LPVOID p, int iIndex);
			};
		}
	}
}