/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#pragma once
#include <ceidef.h>
#include <string.h>	//This header is for memmove(), memcpy(), memset()

#include "CeiLogger.h"

#ifndef PACKBYTE
#define PACKBYTE(a)		(((a)+7)>>3)
#endif
#ifndef PACKING32
#define PACKING32(a)		(((a)+31) & ~0x1f)
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

#define NO_CODE_HERE

#define _T(str) str

#ifndef _WIN32

namespace Cei {
	#define MAKEWORD(a, b)      ((WORD)(((BYTE)(a)) | ((WORD)((BYTE)(b))) << 8))
	#define MAKELONG(a, b)      ((DWORD)(((WORD)(a)) | ((DWORD)((WORD)(b))) << 16))
	#define LOWORD(l)           ((WORD)(l))
	#define HIWORD(l)           ((WORD)(((DWORD)(l)>>16) & 0xFFFF))
	#define LOBYTE(w)           ((BYTE)(w))
	#define HIBYTE(w)           ((BYTE)(((WORD)(w)>> 8) & 0xFF))
	#define LOBIT(b)            ((BYTE)((b) & 0x0F))
	#define HIBIT(b)            ((BYTE)(((BYTE)(b)>> 4) & 0x0F))

	#define RGB(r,g,b)          ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))
	#define GetRValue(rgb)      (LOBYTE(rgb))
	#define GetGValue(rgb)      (LOBYTE(((WORD)(rgb)) >> 8))
	#define GetBValue(rgb)      (LOBYTE((rgb)>>16))

	#ifndef MoveMemory
	#define MoveMemory(Destination,Source,Length) memmove((Destination),(Source),(Length))
	#endif	//MoveMemory
	#ifndef CopyMemory
	#define CopyMemory(Destination,Source,Length) memcpy((Destination),(Source),(Length))
	#endif	//CopyMemory
	#ifndef FillMemory
	#define FillMemory(Destination,Length,Fill) memset((Destination),(Fill),(Length))
	#endif	//FillMemory
	#ifndef ZeroMemory
	#define ZeroMemory(Destination,Length) memset((Destination),0,(Length))
	#endif	//ZeroMemory

	#ifndef Int32x32To64
	#define Int32x32To64( a, b ) (LONGLONG)((LONGLONG)(LONG)(a) * (LONG)(b))
	#endif	//Int32x32To64
	#ifndef UInt32x32To64
	#define UInt32x32To64( a, b ) (ULONGLONG)((ULONGLONG)(DWORD)(a) * (DWORD)(b))
	#endif	//UInt32x32To64

    // WIN User API
	inline bool IsRectEmpty(const RECT* lpRect)
	{
		return ((lpRect->right - lpRect->left) <= 0 || ((lpRect->bottom - lpRect->top) <= 0));
	}
	inline bool SetRect(RECT* lpRect, int xLeft, int yTop, int xRight, int yBottom)
	{
		lpRect->left = xLeft;
		lpRect->top = yTop;
		lpRect->right = xRight;
		lpRect->bottom = yBottom;
		return true;
	}
}

#else

#define NOMINMAX
#include <windows.h>

#endif
