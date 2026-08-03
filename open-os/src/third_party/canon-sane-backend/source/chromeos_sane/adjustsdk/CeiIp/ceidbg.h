/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/

#ifndef __CEIDBG_H_INCLUDED__
#define __CEIDBG_H_INCLUDED__

#ifdef _WIN32
#ifdef _WIN32_WCE

#define _ASSERT(expr) ((void)0)
#define _ASSERTE(expr) ((void)0)

#define _RPT0(rptno, msg)
#define _RPT1(rptno, msg, arg1)
#define _RPT2(rptno, msg, arg1, arg2)
#define _RPT3(rptno, msg, arg1, arg2, arg3)
#define _RPT4(rptno, msg, arg1, arg2, arg3, arg4)

#define _RPTF0(rptno, msg)
#define _RPTF1(rptno, msg, arg1)
#define _RPTF2(rptno, msg, arg1, arg2)
#define _RPTF3(rptno, msg, arg1, arg2, arg3)
#define _RPTF4(rptno, msg, arg1, arg2, arg3, arg4)

#else	// _WIN32_WCE

#include <crtdbg.h>

#endif	// !_WIN32_WCE
#else //_WIN32

#include <assert.h>

#ifndef _ASSERT
#define _ASSERT(expr) assert(expr)
#endif	//_ASSERT
#ifndef _ASSERTE
#define _ASSERTE(expr) assert(expr)
#endif	//_ASSERTE

#ifdef _RPT0
#define _RPT0(rptno, msg)
#endif
#ifdef _RPT1
#define _RPT1(rptno, msg, arg1)
#endif
#ifdef _RPT2
#define _RPT2(rptno, msg, arg1, arg2)
#endif
#ifdef _RPT3
#define _RPT3(rptno, msg, arg1, arg2, arg3)
#endif
#ifdef _RPT4
#define _RPT4(rptno, msg, arg1, arg2, arg3, arg4)
#endif

#ifdef _RPTF0
#define _RPTF0(rptno, msg)
#endif
#ifdef _RPTF1
#define _RPTF1(rptno, msg, arg1)
#endif
#ifdef _RPTF2
#define _RPTF2(rptno, msg, arg1, arg2)
#endif
#ifdef _RPTF3
#define _RPTF3(rptno, msg, arg1, arg2, arg3)
#endif
#ifdef _RPTF4
#define _RPTF4(rptno, msg, arg1, arg2, arg3, arg4)
#endif

#endif //_WIN32

#endif	// !__CEIDBG_H_INCLUDED__
