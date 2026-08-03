/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef _TYPES_FOR_CSDCORE_H_
#define _TYPES_FOR_CSDCORE_H_

#ifndef MAX_PATH
#define MAX_PATH (260)
#endif


typedef void		VOID;
typedef VOID*		LPVOID;

typedef int		BOOL;

typedef unsigned long 	DWORD;
typedef DWORD*		LPDWORD;

typedef unsigned char	BYTE;
typedef BYTE*			LPBYTE;

typedef unsigned short 	WORD;
typedef WORD*			LPWORD;

typedef long		LONG;
typedef LONG*		LPLONG;

typedef   int   		INT;
typedef unsigned int 	UINT;

typedef long INT32;
typedef unsigned long UINT32;

typedef long LPARAM;
typedef long WPARAM;

#ifdef _UNICODE
typedef  wchar_t* LPTSTR;
#else
typedef  char*    LPTSTR;
#endif

#ifdef _UNICODE
typedef  const wchar_t* LPCTSTR;
#else
typedef  const char*    LPCTSTR;
#endif

typedef wchar_t *  LPWSTR;
typedef char *     LPSTR;

typedef const wchar_t *  LPCWSTR;
typedef const char *  LPCSTR;

#ifdef _UNICODE
typedef wchar_t TCHAR;
#else
typedef char TCHAR;
#endif

#ifndef CONST
#define CONST const
#endif

typedef LPVOID		HWND;

typedef const char*	LPCTSTR; 
typedef int		HANDLE;

#ifndef _T
#define _T(x) x
#endif 

#ifndef WINAPI
#define WINAPI
#endif

#ifndef FAR
#define FAR
#endif

#ifndef APIENTRY
#define APIENTRY
#endif

#ifndef DLLAPI
#define DLLAPI
#endif

#ifndef STDMETHODCALLTYPE
#define STDMETHODCALLTYPE
#endif

#ifndef _HRESULT_DEFINED
#define _HRESULT_DEFINED
typedef int HRESULT;
#endif // !_HRESULT_DEFINED

#ifndef SUCCEEDED
#define SUCCEEDED(hr) ((HRESULT)(hr) >= 0)
#endif

#ifndef FAILED
#define FAILED(hr) ((HRESULT)(hr) < 0)
#endif

#ifndef CALLBACK
#define CALLBACK
#endif

#ifndef NULL
#define NULL 0
#endif

#ifndef FALSE
#define FALSE false
#endif

#ifndef TRUE
#define TRUE true
#endif

#ifndef S_OK
#define S_OK (0)
#endif

#ifndef E_INVALIDARG
#define E_INVALIDARG (0x80070057)
#endif

#ifndef E_OUTOFMEMORY
#define E_OUTOFMEMORY (0x8007000E)
#endif

#ifndef E_FAIL
#define E_FAIL (0x80004005)
#endif

#ifndef STDMETHODCALLTYPE
#define STDMETHODCALLTYPE
#endif

#endif //_TYPES_FOR_CSDCORE_H_
