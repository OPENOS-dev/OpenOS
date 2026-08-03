/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/

// IMAGEINFO.h: CImageInfo クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#ifndef _CIMGINF_H_INCLUDED
#define _CIMGINF_H_INCLUDED

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ceitbl.h"
#include "CeiImgInf.h"
#ifdef USE_WIN//_WIN32
#include <windows.h>
#include <tchar.h>
#endif //USE_WIN//_WIN32

#ifndef PACKBYTE
#define PACKBYTE(a)		(((a)+7)>>3)
#endif
#ifndef PACKING32
#define PACKING32(a)		(((a)+31) & ~0x1f)
#endif
#ifndef PACKING16
#define PACKING16(a)		(((a) + 0xf) & ~0xf)
#endif
#ifndef PACKING8
#define PACKING8(a)			(((a)+7) & ~0x07)
#endif
#ifndef PACKING4
#define PACKING4(a)			(((a)+3) & ~0x03)
#endif
#ifndef PACKING2
#define PACKING2(a)			(((a)+1) & ~0x01)
#endif
#ifndef PACKING
#define PACKING(a,b)		(((a)+(b)-1) / (b) * (b))
#endif

// 小数点第一位を四捨五入する割り算
static
ULONG RoundDiv(ULONG ulNum, ULONG ulDenom)
{
/*
(n + d/2) / d)が四捨五入となるための証明
nおよびdは、正の整数とする。

d:偶数のとき
	d	=	2C
	n	=	Ad + B
とする。
(式)	=	int( int(Ad + B + 2C/2 ) / d )
	=	A + int( int(B+C) / d )
	=	A + int( (B+C) / d )
よって、
( (B+C) / d ) < 1	のとき切り捨てとなる。
B/d < 1 - C/d = 1 - 1/2 = 1/2
すなわち、B/d < 1/2のとき、切り捨てとなる。(四捨五入)

d:奇数のとき
	d	=	2C + 1
	n	=	Ad + B
(式)	=	int( int( Ad + B + (2C + 1) / 2 ) / d )
	=	A + int ( (B+C) / d + int( 1 / 2) / d )
	=	A + int ( (B+C) / d )
(B+C) / d < 1	のとき切り捨てとなる。
B + C < d
B < C + 1
B,Cは整数なので、
B <= C	のとき、切り捨てとなる。
B > C	のとき、切り上げとなる。
これよりC < d/2 < C+1なので四捨五入。
*/
	return (ULONG)((ulNum + ulDenom/2) / ulDenom);
}

class CImageInfo  
{
protected :
	CEIIMAGEINFO * m_Image;					// 画像情報
	BOOL m_bAllocStructYes;					// 画像情報を確保したフラグ
	BOOL m_bAllocBuffYes;					// 画像バッファを確保したフラグ
	BOOL m_bAllocTypeVirtualYes;			// 画像バッファの確保に使った関数　true : VirtualAlloc, false : new
public :
	BOOL m_bWhite0;							// 白 == 0
	
protected :
	CEIIMAGEINFO * CreateCeiImageInfoStruct();

	// 初期化
	void Init();
	CEIIMAGEINFO * Init(long lWidth, long lSync, long lHeight, long lBps, long lSpp, DWORD dwRGBOrder);
	CEIIMAGEINFO * Init(long lWidth, long lSync, long lHeight, long lBps, long lSpp, DWORD dwRGBOrder, BYTE *lpBuff, size_t tBuffSize);
	CEIIMAGEINFO * Init(CEIIMAGEINFO l, BOOL bImageDataCopy=TRUE);

public :
	// 構築＆消滅
	CImageInfo();
	CImageInfo(CEIIMAGEINFO lView);
	CImageInfo(CEIIMAGEINFO * lpView);
	CImageInfo(CImageInfo & cImg);
	CImageInfo(long lWidth, long lHeight, long lBps, long lSpp, DWORD dwRGBOrder=LINE_ORDER);
	CImageInfo(long lWidth, long lSync, long lHeight, long lBps, long lSpp, DWORD dwRGBOrder=LINE_ORDER);
	CImageInfo(long lWidth, long lSync, long lHeight, long lBps, long lSpp, DWORD dwRGBOrder, LPBYTE lpBuff, size_t tSize);
	CImageInfo(long lWidth, long lSync, long lHeight, long lBps, long lSpp, DWORD dwRGBOrder, LPBYTE lpBuff, size_t tSize, BOOL bMemoryAttach, BOOL bDeleteMethod_VirtualFree);
	virtual ~CImageInfo();

	// メモリ確保
	virtual LPBYTE	AllocateBuffer(SIZE_T tSize);
	virtual void ReleaseImageBuffer();
#ifdef USE_WIN//_WIN32
	virtual LPBYTE AtachImageBuffer(LPBYTE lpImageBuffer, BOOL bUseVirtualAlloc=true);
#else //USE_WIN//_WIN32
#endif //USE_WIN//_WIN32
	virtual LPBYTE DetachImageBuffer();

	// 画像のコピー作成
	CImageInfo * CreateCopy();
	CImageInfo * CreateSameSize();

	// 画像情報変更
	virtual LPBYTE SetPtr(LPBYTE lpNewPtr);
	virtual long SetWidth(long lNewWidth);
	virtual long SetHeight(long lNewHeight);
	virtual long SetSync(long lSync);
	virtual long SetBps(long lBps);
	virtual size_t SetSize(size_t tSize);
	virtual DWORD SetRGBOrder(DWORD dwRGBOrder);
	virtual void SetResolution(long lXResolution, long lYResolution = -1);

	// 点描画関数
	void PSET2(long x,long y, BOOL bBit);
	BOOL PGET2(long x,long y);
	void PSET8(long x,long y, BYTE byData);
	BYTE PGET8(long x,long y);
	void PSET16(long x,long y, WORD wData);
	WORD PGET16(long x,long y);
	void PSET24(long x, long y, COLORREF dwRGB);

	// 線描画関数
	void VertLine(long lXPos, long lStartY, long lLength, DWORD dwColor);
	void HorzLine(long lYPos, long lStartX, long lWidth, DWORD dwColor);

	// 行,列データ取得
	LPBYTE GetVLineData(LPBYTE lpBuff, long x);
	LPBYTE GetHLineData(LPBYTE lpBuff, long y);
	LPBYTE GetRectData(LPBYTE lpBuff, long x, long y, long lWidth, long lSync, long lHeight);
	LPBYTE GetRectData(LPBYTE lpBuff, RECT rEct, long lSync);

	// 画像貼り付け関数
	void PutImage1(long x, long y, CImageInfo& dstImg);
	void PutImage8(long x, long y, CImageInfo& dstImg);
	void PutImage16(long x, long y, CImageInfo& dstImg);
	void PutImage24(long x, long y, CImageInfo& dstImg);
	void PutImage(long x, long y, CImageInfo& dstImg);

	// 画像移動関数
	void Shift(int nShift);
	void ShiftLeft(int nShift);
	void ShiftRight(int nShift);

	// 画像切り取り関数
	void CutOffH(long lStartLine, long lEndLine);
	void CutOffV(long lStartPos, long lEnPos);

	// パッキング解消関数
	void SetPack1(long lOffsetBytes = 0);
	void SetPackN(int nPack, long lOffsetBytes = 0);
	void CrearUnusedArea(DWORD dwFill = 0);
	
	// 画像(色)反転
	void Reverse();
	BOOL SetBlackIs(BOOL bBlack);
	BOOL SetWhiteIs(BOOL bWhite);

	// 画像反転
#ifdef USE_WIN//_WIN32
	BOOL MirrorUpDown();
#else //USE_WIN//_WIN32
#endif //USE_WIN//_WIN32
	BOOL MirrorLeftRight();

	// 画像のデータ取得
	BYTE RectAve(LPRECT lprEct);
	DWORD RectSum(LPRECT lprEct, DWORD * dwDot);

	// トリミング
	void Trim(double dFrame);
	void Trim(RECT rEct);

	// ファイル保存
#ifdef USE_WIN//_WIN32
#ifdef UNICODE
	BOOL SaveBmp(LPTSTR szFname);
#else
	BOOL SaveBmp(LPSTR szFname);
#endif
#endif //USE_WIN//_WIN32

	// メンバへのアクセス
	size_t GetSize() {
		return m_Image->tImageSize;
	};
	LPBYTE GetPtr() {
		return m_Image->lpImage;
	};
	LPBYTE GetPtr(long lX, long lY) {
		if( Spp() == 3 && RGBOrder() == PIXEL_ORDER )
			return GetPtr() + RGBSync()*lY + Bps()*Spp()*lX/8;
		else
			return GetPtr() + RGBSync()*lY + Bps()*lX/8;
	};
	long BitCount() {
		return Bps() * Spp();
	};
	long Xpos() {
		return m_Image->lXpos;
	};
	long Ypos() {
		return m_Image->lYpos;
	};
	long Height(){
		return m_Image->lHeight;
	};
	long Width() {
		return m_Image->lWidth;
	};
	long Sync() {
		return m_Image->lSync;
	};
	long RGBSync() {
		if(RGBOrder()==LINE_ORDER)
			return Sync() * Spp();
		else return Sync();
	};
	BYTE * Image() {
			return m_Image->lpImage;
	};
	size_t ImageSize() {
		return m_Image->tImageSize;
	};
	long Bps() {
		return m_Image->lBps;
	};
	long Spp() {
		return m_Image->lSpp;
	};
	DWORD RGBOrder() {
		return m_Image->dwRGBOrder;
	};
	long Resolution() {
		return XResolution();
	};
	long XResolution() {
		return m_Image->lXResolution;
	};
	long YResolution() {
		return m_Image->lYResolution;
	};

	long HSync() {
		return Sync();
	};
	long VSync() {
		return Height();
	};
	BOOL IsReverse() {
		return m_bWhite0;
	};

	operator long () {
		return static_cast<long>(GetSize());
	};
	operator BYTE * () {
		return GetPtr();
	};
	operator CEIIMAGEINFO * () {
		return m_Image;
	};
	operator CEIIMAGEINFO () {
		return *m_Image;
	};
	CImageInfo &operator = (CEIIMAGEINFO iInf) {
		CreateCeiImageInfoStruct();
		*m_Image = iInf;
		return *this;
	};
	operator RECT() {
		RECT rEct;
		rEct.left = Xpos();
		rEct.top = Ypos();
		rEct.right = Width();
		rEct.bottom = Height();
		return rEct;
	};
};


// Compare helpers
bool operator == (CImageInfo& i1, CImageInfo& i2);
bool operator != (CImageInfo& i1, CImageInfo& i2);

#endif // !defined(_CIMGINF_H_INCLUDED)
