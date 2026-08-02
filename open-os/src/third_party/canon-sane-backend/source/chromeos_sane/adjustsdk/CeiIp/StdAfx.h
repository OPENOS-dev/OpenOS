/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#pragma once


#include <ceilib.h>

#include "ceidbg.h"
#include "ceitbl.h"

//#include <WinError.h>

#include <assert.h>
#include <algorithm>
#include <memory>

using namespace std;



#if _WIN32_WCE > 0x500
#ifndef DOUBLE
#define DOUBLE(value) (((value)==0)? 0.0 : (double)(value))
#endif
#else
#define DOUBLE(value)		(double)(value)
#endif

#ifndef IN
#define IN
#endif
#ifndef OUT
#define OUT
#endif
