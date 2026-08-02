/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#pragma once

#if defined(DRC225_Build)
	#include "LLiPmDRC225_Prefix.h"
#elif defined(DRC240_Build)
    #include "LLiPmDRC240_Prefix.h"
#elif defined(DRP215_Build)
    #include "LLiPmDRP215_Prefix.h"
#elif defined(DRP208_Build)
    #include "LLiPmDRP208_Prefix.h"
#endif

//include
#include DRFilterInfoHeader
