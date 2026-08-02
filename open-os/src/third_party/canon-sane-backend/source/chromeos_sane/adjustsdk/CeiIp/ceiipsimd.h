/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/

#ifndef _CEIIPSIMD_H_INCLUDED
#define _CEIIPSIMD_H_INCLUDED

// ---- SSE2, SSE, MMX -----
#define USE_SIMD
#ifdef USE_SIMD

#ifdef _WIN32
#   if _MSC_VER >= 1400
#       define SSE2
#       define SSE
#       define MMX
#       include <excpt.h>	
#   endif
#else
#   if defined(__i386__)
#       define SSE2
#       define SSE
#       define MMX
#       include <sys/sysctl.h>
#   endif
#endif //_WIN32

#ifdef SSE2
#   include <emmintrin.h>
#endif
#ifdef SSE
#   include <xmmintrin.h>
#endif
#ifdef MMX
#   include <mmintrin.h>
#endif

#endif //USE_SIMD

bool IsMMXFeatureAvailable();
bool IsSSE2FeatureAvailable();
bool IsNEONFeatureAvailable();

bool IsMMXSupportedProc();
bool IsSSESupportedProc();
bool IsSSE2SupportedProc();
bool IsSSE3SupportedProc();
bool IsSSSE3SupportedProc();
bool IsSSE41SupportedProc();
bool IsSSE42SupportedProc();
bool IsAVXSupportedProc();

#endif	/* _CEIIPSIMD_H_INCLUDED */
