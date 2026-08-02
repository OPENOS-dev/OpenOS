/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#if !defined(AFX_CeiLogger_H__128203F4_1814_40DC_A8E9_473268B7EAF0__INCLUDED_)
#define AFX_CeiLogger_H__128203F4_1814_40DC_A8E9_473268B7EAF0__INCLUDED_

#pragma once
#include <stdarg.h>
#include <stdio.h>
#define WRITE_TRACE
namespace Cei
{
	class CeiLogger 
	{
    public:
		CeiLogger() {}
		~CeiLogger() {}
		static void writeLog(const char* format, ...);
	};
}

#endif // !defined(AFX_CeiLogger_H__128203F4_1814_40DC_A8E9_473268B7EAF0__INCLUDED_)
