/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include "StdAfx.h"
#include "ceiipsimd.h"
#ifdef _WIN64
#undef SSE2
#endif
#ifdef SSE2
#ifdef _WIN32
#include <intrin.h>
#else
#include <cpuid.h>
#endif
#endif
// ---- SSE2, SSE, MMX -----
const DWORD _MMX_FEATURE_BIT = 0x00800000;
const DWORD _SSE2_FEATURE_BIT = 0x04000000;
static bool _IsFeature(DWORD nFeature);
bool IsMMXFeatureAvailable()
{
	static bool bMMX = _IsFeature(_MMX_FEATURE_BIT);
	return(bMMX);
}
bool IsSSE2FeatureAvailable()
{
	static bool bSSE2 = _IsFeature(_SSE2_FEATURE_BIT);
	return(bSSE2);
}
bool IsNEONFeatureAvailable()
{
#ifdef __ARM_NEON__
	return false;
#else
	return false;
#endif
}
static bool _IsFeature(DWORD dwRequestFeature)
{
#ifdef SSE2
#ifdef _WIN32
	// This	bit	flag can get set on	calling	cpuid
	// with	register eax set to	1
	DWORD dwFeature	= 0;
	__try {
			_asm {
				mov	eax,1
				cpuid
				mov	dwFeature,edx
			}
	} __except ( EXCEPTION_EXECUTE_HANDLER)	{
			return false;
	}
	if ((dwRequestFeature == _MMX_FEATURE_BIT) &&
		(dwFeature & _MMX_FEATURE_BIT)) {
		__try {
			__asm {
				pxor mm0, mm0
				emms
			}
		} __except (EXCEPTION_EXECUTE_HANDLER) {
			return (0);
		}
		return(true);
	}
	else if ((dwRequestFeature == _SSE2_FEATURE_BIT) &&
		(dwFeature & _SSE2_FEATURE_BIT)) {
		__try {
			__asm {
				xorpd xmm0, xmm0
			}
		} __except (EXCEPTION_EXECUTE_HANDLER) {
			return (0);
		}
		return(true);
	}
#elif defined(__APPLE__)
	int hasSSE2 = 0;
	size_t length = sizeof(hasSSE2);
	int error = sysctlbyname("hw.optional.sse2", &hasSSE2, &length, NULL, 0);
	if (0 != error) {
		return false;
	}
	else {
		return true;
	}
#elif defined(__GNUC__)
    const int type = 1;
    int info[4] = {0};
#if defined(__LP64__)
    __asm__ __volatile__ ("pushq %%rbx   \n\t"
                          "cpuid         \n\t"
                          "movl %%ebx, %1\n\t"
                          "popq %%rbx    \n\t"
                          : "=a"(info[0]), "=r"(info[1]), "=c"(info[2]), "=d"(info[3])
                          : "a"(type)
                          : "cc" );
#else
    __asm__ __volatile__ ("pushl %%ebx   \n\t"
                          "cpuid         \n\t"
                          "movl %%ebx, %1\n\t"
                          "popl %%ebx    \n\t"
                          : "=a"(info[0]), "=r"(info[1]), "=c"(info[2]), "=d"(info[3])
                          : "a"(type)
                          : "cc" );
#endif
    return ((info[3] & 0x04000000) > 0);
#endif //_WIN32
#endif //SSE2
	return false;
}
#ifdef SSE2
void read_cpu_data(unsigned char * pbuf, int type)
{
	if (pbuf) {
#ifdef _WIN32
		__cpuid((int *)pbuf, type);
#else	/* Linux */
		unsigned int * p = (unsigned int *)pbuf;
		__get_cpuid(type, p, p + 1, p + 2, p + 3);
#endif
	}
	return;
}
#endif
bool IsInstructionSupportedCPU(unsigned int offset, unsigned int bit)
{
#ifdef SSE2
	unsigned int data[4] = {0};
	read_cpu_data((unsigned char*)data, 1);
	return (data[offset] & (0x00000001 << bit)) ? true : false;
#else
	return false;
#endif
}
bool IsIntelCPU() {
#ifdef SSE2
	unsigned int data[4] = {0};
	read_cpu_data((unsigned char*)data, 0);
	char vender[13] = {0};
	*((unsigned int *)(vender+0)) = data[1];
	*((unsigned int *)(vender+4)) = data[3];
	*((unsigned int *)(vender+8)) = data[2];
	if (strstr(vender, "Intel")) {
		return true;
	}
#endif
	return false;
}
bool IsMMXSupportedProc()	{ return IsInstructionSupportedCPU(3, 23); }
bool IsSSESupportedProc()	{ return IsInstructionSupportedCPU(3, 25); }
bool IsSSE2SupportedProc()	{ return IsInstructionSupportedCPU(3, 26); }
bool IsSSE3SupportedProc()	{ return IsInstructionSupportedCPU(2, 0); }
bool IsSSSE3SupportedProc()	{ return IsInstructionSupportedCPU(2, 9); }
bool IsSSE41SupportedProc()	{ return IsInstructionSupportedCPU(2, 19); }
bool IsSSE42SupportedProc()	{ return IsInstructionSupportedCPU(2, 20); }
bool IsAVXSupportedProc()	{
	if (IsIntelCPU()) {
		return IsInstructionSupportedCPU(3, 9);
	}
	return false;
}