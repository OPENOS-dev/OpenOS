/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#pragma once


#ifndef CCSIZEOF_STRUCT
#define CCSIZEOF_STRUCT(structname, member)  (((int)((LPBYTE)(&((structname*)0)->member) - ((LPBYTE)((structname*)0)))) + sizeof(((structname*)0)->member))
#endif

namespace Cei
{
    namespace LLiPm {
        typedef enum tagRTN {
            RTN_OK = 0,
            RTN_NOSPT,
            RTN_PAR,
            RTN_NOMEM,
            RTN_DEBUG,
            RTN_SEQ,
        } RTN;
        
        typedef enum tagSIDE {
            FRONT = 0,
            BACK,
        } SIDE;
        
        enum ColorMode {
            BINARY,
            GRAY,
            COLOR
        };
    }
}


#ifndef _WIN32

namespace Cei
{
	typedef unsigned long ULONG;
	typedef unsigned short USHORT;
	typedef unsigned char UCHAR;

	#ifndef NULL
	#ifdef __cplusplus
	#define NULL    0
	#else
	#define NULL    ((void *)0)
	#endif
	#endif

	#ifndef FALSE
	#define FALSE               0
	#endif
	#ifndef TRUE
	#define TRUE                1
	#endif

	#define far
	#define near

	#undef FAR
	#undef  NEAR
	#define FAR                 far
	#define NEAR                near
	#ifndef CONST
	#define CONST               const
	#endif

    #ifndef IN
    #define IN
    #endif
    #ifndef OUT
    #define OUT
    #endif
    
    typedef char                CHAR;
	typedef short               SHORT;
	typedef long                LONG;
	typedef unsigned int        DWORD;
	typedef int                 BOOL;
	typedef unsigned char       BYTE;
	typedef unsigned short      WORD;
    typedef long long           LONG64;
	typedef float               FLOAT;

	typedef FLOAT               *PFLOAT;
	typedef BOOL near           *PBOOL;
	typedef BOOL far            *LPBOOL;
	typedef BYTE near           *PBYTE;
	typedef BYTE far            *LPBYTE;
	typedef int near            *PINT;
	typedef int far             *LPINT;
	typedef WORD near           *PWORD;
	typedef WORD far            *LPWORD;
	typedef long far            *LPLONG;
	typedef DWORD near          *PDWORD;
	typedef DWORD far           *LPDWORD;
	typedef void far            *LPVOID;
	typedef CONST void far      *LPCVOID;

	typedef int                 INT;
	typedef unsigned int        UINT;
    typedef unsigned int        UINT32;
    typedef unsigned int        *PUINT;

	typedef long long int LONGLONG;
	typedef unsigned long long int ULONGLONG;

	typedef unsigned long ULONG_PTR, *PULONG_PTR;
	typedef ULONG_PTR SIZE_T, *PSIZE_T;
	typedef ULONG_PTR DWORD_PTR, *PDWORD_PTR;


	typedef void* HANDLE;
	typedef HANDLE NEAR         *SPHANDLE;
	typedef HANDLE FAR          *LPHANDLE;

	typedef LONG HRESULT;

	typedef DWORD   COLORREF;
	typedef DWORD   *LPCOLORREF;

	typedef struct tagPOINT
	{
		LONG  x;
		LONG  y;
	} POINT, *PPOINT, NEAR *NPPOINT, FAR *LPPOINT;

	typedef struct tagSIZE
	{
		long	cx;
		long	cy;
	} SIZE, *PSIZE;

	typedef struct tagRECT
	{
		long left;
		long top;
		long right;
		long bottom;
	} RECT, *PRECT, NEAR *NPRECT, FAR *LPRECT;

	typedef struct tagRGBQUAD {
			BYTE    rgbBlue;
			BYTE    rgbGreen;
			BYTE    rgbRed;
			BYTE    rgbReserved;
	} RGBQUAD;
	typedef RGBQUAD FAR* LPRGBQUAD;

	#pragma pack(push, 2)
	typedef struct tagRGBTRIPLE {
			BYTE    rgbtBlue;
			BYTE    rgbtGreen;
			BYTE    rgbtRed;
	} RGBTRIPLE;
	#pragma pack(pop)
}

#else

#define NOMINMAX 
#include <windows.h>

#endif
