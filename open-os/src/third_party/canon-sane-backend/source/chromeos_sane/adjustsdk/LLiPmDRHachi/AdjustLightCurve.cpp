/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/

#include "AdjustLightCurve.h"
#include "ceilib.h"
#include <memory.h>
#include <assert.h>
#include <algorithm>

using namespace Cei;
using namespace LLiPm;
using namespace DR_NAMESPACE;

//
// CLightCurve
//
PUINT CLightCurve::GetPtr(int iIndex)
{
	assert(CCeiRaster::GetSize() > (iIndex * sizeof(UINT)));
	PUINT pBuffer = (PUINT)CCeiRaster::GetPtr();
	return pBuffer + iIndex;
}
bool CLightCurve::SetLcInfomation(LCAINFOHEADER &info)
{
	*(LCAINFOHEADER *)this = info;
	wDataBits = sizeof(UINT) * 8;
	dwSync = dwWidth * sizeof(UINT);
	dwDataSize = dwWidth * sizeof(UINT) * wSpp;
	if (!CCeiRaster::SetSize(dwWidth * wSpp * sizeof(UINT))) return false;
	return true;
}
void CLightCurve::SetAt(int iIndex, UINT data)
{
	*GetPtr(iIndex) = data;
}

//
// CLightCurveAdjustData
//
CLightCurveAdjustData::CLightCurveAdjustData()
: m_pCurve(0), m_nNumberOfData(0)
{
}

CLightCurveAdjustData::~CLightCurveAdjustData()
{
	Release();
}

RTN CLightCurveAdjustData::SetSize(DWORD dwSize)
{
	Release();

	m_pCurve = new(std::nothrow) CLightCurve [dwSize];
	if (m_pCurve == NULL) return RTN_NOMEM;

	m_nNumberOfData = dwSize;
	return RTN_OK;
}

void CLightCurveAdjustData::Release() {
	if (m_pCurve) {
		delete [] m_pCurve;
		m_nNumberOfData = 0;
	}
}

int CLightCurveAdjustData::CheckTimeStamp(unsigned char* lpData, unsigned long ulSize, DWORD dwTimeStamp)
{
	enum {
		CTS_ERROR_OK = 0,
		CTS_ERROR_NOT_CORRESPOND = 1,
		CTS_ERROR_ERROR = -1,
	};

	if (lpData == 0 || ulSize != LIGHTCURVEDATASIZE) {
		return RTN_PAR;
	}
	unsigned char* ptr = lpData;

	LCAFILEHEADER hdr;
	memcpy(&hdr, ptr, sizeof(hdr));

	if (hdr.dwTimestamp != dwTimeStamp) return CTS_ERROR_NOT_CORRESPOND;
	return CTS_ERROR_OK;
}

RTN CLightCurveAdjustData::LoadData(unsigned char* lpData, unsigned long ulSize)
{
	if (lpData == 0 || ulSize != LIGHTCURVEDATASIZE) {
		return RTN_PAR;
	}
	unsigned char* ptr = lpData;

	
	LCAFILEHEADER hdr;
	hdr.dwTimestamp = MAKELONG(MAKEWORD(ptr[0], ptr[1]), MAKEWORD(ptr[2], ptr[3]));
	hdr.dwVersion = MAKELONG(MAKEWORD(ptr[4], ptr[5]), MAKEWORD(ptr[6], ptr[7]));
	hdr.dwNumData = MAKELONG(MAKEWORD(ptr[8], ptr[9]), MAKEWORD(ptr[10], ptr[11]));
	ptr += 12;
	if (hdr.dwTimestamp == 0xffffffff) {
		return RTN_PAR;	
	}

	RTN result = SetSize(hdr.dwNumData);
	if (result != RTN_OK) {
		return result;
	}


	for (DWORD i=0; i<hdr.dwNumData; i++) {

		
		WORD wInfoSize = MAKEWORD(ptr[0], ptr[1]);

		
		if (wInfoSize == 0) {
			return RTN_PAR;
		}

		
		LCAINFOHEADER info = {0};
		info.wSize = MAKEWORD(ptr[0], ptr[1]);
		info.wSide = MAKEWORD(ptr[2], ptr[3]);
		info.wSpp = MAKEWORD(ptr[4], ptr[5]);
		info.wDataBits = MAKEWORD(ptr[6], ptr[7]);
		info.dwDenom = MAKELONG(MAKEWORD(ptr[8], ptr[9]), MAKEWORD(ptr[10], ptr[11]));
		info.dwWidth = MAKELONG(MAKEWORD(ptr[12], ptr[13]), MAKEWORD(ptr[14], ptr[15]));
		info.dwSync = MAKELONG(MAKEWORD(ptr[16], ptr[17]), MAKEWORD(ptr[18], ptr[19]));
		info.dwDataSize = MAKELONG(MAKEWORD(ptr[20], ptr[21]), MAKEWORD(ptr[22], ptr[23]));
		info.wResolution = MAKEWORD(ptr[24], ptr[25]);
		info.wOrder = MAKEWORD(ptr[26], ptr[27]);
		info.dwFlags = MAKELONG(MAKEWORD(ptr[28], ptr[29]), MAKEWORD(ptr[30], ptr[31]));
		if (wInfoSize > 32) {
			info.dDropout = ptr[32];
			info.dEmphasis = ptr[33];
		}
		ptr += wInfoSize;

		CCeiRaster temp;
		if (!temp.SetSize(info.dwDataSize)) return RTN_PAR;
		if (temp.GetPtr() == 0) return RTN_NOMEM;

		memcpy(temp.GetPtr(), ptr, info.dwDataSize);
		ptr += info.dwDataSize;

		CLightCurve &lc = m_pCurve[i];
		if (!lc.SetLcInfomation(info)) return RTN_PAR;

		for (DWORD iPix=0; iPix<info.dwWidth; iPix++) {
			if (info.wSpp == 3) {
				switch (info.wDataBits) {
				case 8 : 
					lc.SetAt(iPix + info.dwWidth * 0, (UINT)GetByte(temp.GetPtr() + info.dwSync * 0, iPix));	// r
					lc.SetAt(iPix + info.dwWidth * 1, (UINT)GetByte(temp.GetPtr() + info.dwSync * 1, iPix));	// g
					lc.SetAt(iPix + info.dwWidth * 2, (UINT)GetByte(temp.GetPtr() + info.dwSync * 2, iPix));	// b
					break;
				case 16 : 
					lc.SetAt(iPix + info.dwWidth * 0, (UINT)GetWord(temp.GetPtr() + info.dwSync * 0, iPix));	// r
					lc.SetAt(iPix + info.dwWidth * 1, (UINT)GetWord(temp.GetPtr() + info.dwSync * 1, iPix));	// g
					lc.SetAt(iPix + info.dwWidth * 2, (UINT)GetWord(temp.GetPtr() + info.dwSync * 2, iPix));	// b
					break;
				case 32 :
					lc.SetAt(iPix + info.dwWidth * 0, (UINT)GetDWord(temp.GetPtr() + info.dwSync * 0, iPix));	// r
					lc.SetAt(iPix + info.dwWidth * 1, (UINT)GetDWord(temp.GetPtr() + info.dwSync * 1, iPix));	// g
					lc.SetAt(iPix + info.dwWidth * 2, (UINT)GetDWord(temp.GetPtr() + info.dwSync * 2, iPix));	// b
					break;
				default :
					return RTN_PAR;
				}
			}
			else {
				switch (info.wDataBits) {
				case 8 : 
					lc.SetAt(iPix, (UINT)GetByte(temp.GetPtr(), iPix));
					break;
				case 16 : 
					lc.SetAt(iPix, (UINT)GetWord(temp.GetPtr(), iPix));
					break;
				case 32 :
					lc.SetAt(iPix, (UINT)GetDWord(temp.GetPtr(), iPix));
					break;
				default :
					return RTN_PAR;
				}
			}
		}
	}

	return RTN_OK;
}

WORD CLightCurveAdjustData::LightCurveAdjPixel(WORD w, WORD b, UINT num, UINT denom)
{
	if (denom == 0) return w;

	DWORD dw = (w > b) ? (w - b) : 0;
	dw = num * dw;
	dw = (dw + denom / 2) / denom;
	dw = dw + b;
	dw = (WORD)std::min(dw, (DWORD)0xffff); // dw = min(dw, 4095);
	return (WORD)dw;
}

RTN CLightCurveAdjustData::AdjustData(IMAGEINFO& Black, IMAGEINFO& White, SIDE Side, int nDropout, int nEmphasis)
{

	assert(!(nDropout && nEmphasis));
	assert(0 <= nDropout && nDropout <= 3);
	assert(0 <= nEmphasis && nEmphasis <= 3);
	

	CLightCurve *plc = NULL;
	for (DWORD i=0;i<m_nNumberOfData; i++) {

		if (m_pCurve[i].wSide != Side) continue;
		if (nDropout) {		
			if (m_pCurve[i].wSpp != 3) continue;
			if (White.lSpp != 1) continue;
			
		}
		else if (nEmphasis) {	
			if (m_pCurve[i].wSpp != White.lSpp) continue;
			if (!m_pCurve[i].dEmphasis) continue;
		}
		else {
			
			if (m_pCurve[i].wSpp != White.lSpp) continue;
			if (m_pCurve[i].dDropout) continue;
			if (m_pCurve[i].dEmphasis) continue;
		}

		if (m_pCurve[i].wResolution != White.lXResolution) continue;
		if (!(m_pCurve[i].dwFlags & LCA_DATA_FILTERED)) continue;

		plc = m_pCurve + i;
		break;
	}

	if (plc == 0) return RTN_PAR;		// cannot find

	#define GET_COLOR_DATA(img, x, col)	\
				(((img).ulRGBOrder == LINE_ORDER) ?							\
				((img).lpImage + (col) * (img).lSync + (img).lBps * (x) / 8) :	\
				((img).lpImage + ((img).lBps * ((x) * 3 + (col))) / 8))
	#define GETR(img, x)	GET_COLOR_DATA(img, x, 0)
	#define GETG(img, x)	GET_COLOR_DATA(img, x, 1)
	#define GETB(img, x)	GET_COLOR_DATA(img, x, 2)

	if (White.lSpp == 3) {
		IMAGEINFO lc;
		lc.ulRGBOrder = (unsigned long)plc->wOrder;
		lc.lpImage = (unsigned char*)plc->GetPtr();
		lc.lSync = plc->dwSync;
		lc.lBps = sizeof(unsigned int) * 8;

		DWORD w = std::min(plc->dwWidth, (DWORD)White.lWidth);
		int offset = 0;
		for (DWORD x=0; x<w; x++) {
			WORD &wr = *(WORD *)GETR(White, x);
			WORD &br = *(WORD *)GETR(Black, x);
			UINT &cr = *(UINT *)GETR(lc, x + offset);
			wr = LightCurveAdjPixel(wr, br, cr, plc->dwDenom);
			WORD &wg = *(WORD *)GETG(White, x);
			WORD &bg = *(WORD *)GETG(Black, x);
			UINT &cg = *(UINT *)GETG(lc, x + offset);
			wg = LightCurveAdjPixel(wg, bg, cg, plc->dwDenom);
			WORD &wb = *(WORD *)GETB(White, x);
			WORD &bb = *(WORD *)GETB(Black, x);
			UINT &cb = *(UINT *)GETB(lc, x + offset);
			wb = LightCurveAdjPixel(wb, bb, cb, plc->dwDenom);
		}
	}
	else if (nDropout) {
		IMAGEINFO lc;
		lc.ulRGBOrder = (DWORD)plc->wOrder;
		lc.lpImage = (BYTE *)plc->GetPtr();
		lc.lSync = plc->dwSync;
		lc.lBps = sizeof(UINT) * 8;
		int nColorIndex = 
			((nDropout == 1) ? 0 // red
			: ((nDropout == 2) ? 1	// green
			: 2));					// blue


		DWORD w = std::min(plc->dwWidth, (DWORD)White.lWidth);
		LPWORD pw = (LPWORD)White.lpImage;
		LPWORD pb = (LPWORD)Black.lpImage;
		int offset = 0;
		for (DWORD x=0; x<w; x++) {
			UINT &cr = *(UINT *)GET_COLOR_DATA(lc, x + offset, nColorIndex);
			*pw = LightCurveAdjPixel(*pw, *pb, cr, plc->dwDenom);
			++pw;
			++pb;
		}
	}
	else {
		DWORD w = std::min(plc->dwWidth, (DWORD)White.lWidth);
		LPWORD pw = (LPWORD)White.lpImage;
		LPWORD pb = (LPWORD)Black.lpImage;
		UINT* plcd = plc->GetPtr();
		for (DWORD i=0; i<w; i++) {
			*pw = LightCurveAdjPixel(*pw, *pb, *plcd, plc->dwDenom);
			++pw;
			++pb;
			++plcd;
		}
	}
	return RTN_OK;
}

BYTE CLightCurveAdjustData::GetByte(LPVOID p, int iIndex) {
	return *(((LPBYTE)p) + iIndex);	
}
WORD CLightCurveAdjustData::GetWord(LPVOID p, int iIndex) {
	LPWORD pData = ((LPWORD)p) + iIndex;
	LPBYTE ptr = (LPBYTE)pData;
	return MAKEWORD(*(ptr), *(ptr+1));
}
DWORD CLightCurveAdjustData::GetDWord(LPVOID p, int iIndex) {
	LPDWORD pData = ((LPDWORD)p) + iIndex;
	LPBYTE ptr = (LPBYTE)pData;
	return MAKELONG(MAKEWORD(*(ptr), *(ptr+1)), MAKEWORD(*(ptr+2), *(ptr+3)));
}