/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include "CollectArray.h"
#include "ceilib.h"
#include <memory.h>
#include <assert.h>

using namespace Cei;
using namespace LLiPm;
using namespace DR_NAMESPACE;

CCollectArray::CCollectArray(void)
{
}

CCollectArray::~CCollectArray(void)
{
}

void CCollectArray::ArrayCollection(unsigned char* pSrc, unsigned char* pDst, long lWidth, long lXResolution, int nSensorVer)
{
#if defined(LIGHT_ADJUST_VOYAJER_TYPE)
    const unsigned int unSensor600Table[][3] = {{ 26, 1648, 54},
                                                {  0, 1648, 80}};
    const unsigned int unSensor300Table[][3] = {{ 13, 824,  27},
                                                {  0, 824,  40}};
    const bool bPara1Avail[] = { false, true, true};
    const bool bPara2Avail[] = { true, true, true};
    const bool bPara3Avail[] = { true, true, false};
    const unsigned int unDst600ParaOffset[][3] = {{ 5104 - 1, 3402 - 1, 1674 - 1},
                                                  { 5104 - 1, 3376 - 1, 1648 - 1}};
    const unsigned int unDst300ParaOffset[][3] = {{ 2552 - 1, 1701 - 1, 837 - 1},
                                                  { 2552 - 1, 1688 - 1, 824 - 1}};
#elif defined(LIGHT_ADJUST_NEWDT_TYPE)
	const unsigned int unSensor600Table[][3] = { { 26, 1648, 54},
												{ 26, 1648, 54} };
	const unsigned int unSensor300Table[][3] = { { 13, 824,  27},
												{ 13, 824,  27} };
	const bool bPara1Avail[] = { false, true, true };
	const bool bPara2Avail[] = { true, true, true };
	const bool bPara3Avail[] = { true, true, false };
	const unsigned int unDst600ParaOffset[][3] = { { 5104 - 1, 3402 - 1, 1674 - 1},
												  { 5104 - 1, 3376 - 1, 1648 - 1} };
	const unsigned int unDst300ParaOffset[][3] = { { 2552 - 1, 1701 - 1, 837 - 1},
												  { 2552 - 1, 1688 - 1, 824 - 1} };
#else
    const unsigned int unSensor600Table[][4] = {{ 22, 1382, 446, 22}};
    const unsigned int unSensor300Table[][4] = {{ 11, 691, 223, 11}};
	const bool bPara1Avail[] = { false, true, true, true};
	const bool bPara2Avail[] = { true, true, false, false};
	const bool bPara3Avail[] = { true, true, true, false};
    const unsigned int unDst600ParaOffset[][4] = {{ 5104 - 1, 3254 - 1, 1850 - 1}};
    const unsigned int unDst300ParaOffset[][4] = {{ 2552 - 1, 1627 - 1, 925 - 1}};
#endif

	if (lXResolution == 300) {
		//�o�Ă��鏇�Ԃ́A2,1,3�̏���
		unsigned char* pDstPara1 = pDst + unDst300ParaOffset[nSensorVer][0];
		unsigned char* pDstPara2 = pDst + unDst300ParaOffset[nSensorVer][1];
		unsigned char* pDstPara3 = pDst + unDst300ParaOffset[nSensorVer][2];
		#define PARA_BLOCK_PROC_SIMP300(i)                                      \
			for (unsigned int x=0; x<unSensor300Table[nSensorVer][i]; x++) {    \
				if (bPara2Avail[i]) *pDstPara2-- = pSrc[0];                     \
				if (bPara1Avail[i]) *pDstPara1-- = pSrc[1];                     \
				if (bPara3Avail[i]) *pDstPara3-- = pSrc[2];                     \
				pSrc += 3;                                                      \
			}
		PARA_BLOCK_PROC_SIMP300(0);
		PARA_BLOCK_PROC_SIMP300(1);
		PARA_BLOCK_PROC_SIMP300(2);
#if defined(LIGHT_ADJUST_VOYAJER_TYPE) || defined(LIGHT_ADJUST_NEWDT_TYPE)
#else
        PARA_BLOCK_PROC_SIMP300(3);
#endif
	}
	else if (lXResolution == 600) {
		unsigned char* pDstPara1 = pDst + unDst600ParaOffset[nSensorVer][0];
		unsigned char* pDstPara2 = pDst + unDst600ParaOffset[nSensorVer][1];
		unsigned char* pDstPara3 = pDst + unDst600ParaOffset[nSensorVer][2];
		#define PARA_BLOCK_PROC_SIMP600(i)                                      \
			for (unsigned int x=0; x<unSensor600Table[nSensorVer][i]; x++) {    \
				if (bPara2Avail[i]) *pDstPara2-- = pSrc[0];                     \
				if (bPara1Avail[i]) *pDstPara1-- = pSrc[1];                     \
				if (bPara3Avail[i]) *pDstPara3-- = pSrc[2];                     \
				pSrc += 3;                                                      \
			}
		PARA_BLOCK_PROC_SIMP600(0);
		PARA_BLOCK_PROC_SIMP600(1);
		PARA_BLOCK_PROC_SIMP600(2);
#if defined(LIGHT_ADJUST_VOYAJER_TYPE) || defined(LIGHT_ADJUST_NEWDT_TYPE)
#else
		PARA_BLOCK_PROC_SIMP600(3);
#endif
	}
	else {
		return;
	}
}

void CCollectArray::ArrayCollection(unsigned char* pSrc, unsigned char* pDst, long lWidth, DUMMYPIXELS& dummy)
{
	assert(lWidth % 3 == 0);	

	long para = lWidth / 3;

	long Available1 = para - dummy.lLeft;
	long Available2 = para - dummy.lMiddle;
	long Available3 = para - dummy.lRight;

	
	pDst += lWidth - dummy.lLeft - dummy.lMiddle - dummy.lRight - 1;
	ArrayCollectPara(pDst, pSrc + 1 + dummy.lLeft*3,		3, Available3);
	pDst -= Available3;
	ArrayCollectPara(pDst, pSrc,							3, Available2);
	pDst -= Available2;
	ArrayCollectPara(pDst, pSrc + 2,						3, Available1);
}

void CCollectArray::Separate(unsigned char* lpDst1, unsigned char* lpDst2, unsigned char* pSrc, long lWidth, long lXResolution, int nSensorVer)
{
#if defined(LIGHT_ADJUST_DOCAN_TYPE) || defined(LIGHT_ADJUST_VOYAJER_TYPE)
	// TakeZ, Docan

#if defined(LIGHT_ADJUST_VOYAJER_TYPE)
    const unsigned int unSensor600Table[][3] = {{ 26, 1648, 54},
                                                {  0, 1648, 80}};
    const unsigned int unSensor300Table[][3] = {{ 13, 824,  27},
                                                {  0, 824,  40}};
    const bool bPara1Avail[] = { false, true, true};
    const bool bPara2Avail[] = { true, true, true};
    const bool bPara3Avail[] = { true, true, false};
    const unsigned int unDst600ParaOffset[][3] = {{ 5104 - 1, 3402 - 1, 1674 - 1},
                                                  { 5104 - 1, 3376 - 1, 1648 - 1}};
    const unsigned int unDst300ParaOffset[][3] = {{ 2552 - 1, 1701 - 1, 837 - 1},
                                                  { 2552 - 1, 1688 - 1, 824 - 1}};
#else
    const unsigned int unSensor600Table[][4] = {{ 22, 1382, 446, 22}};
    const unsigned int unSensor300Table[][4] = {{ 11, 691, 223, 11}};
	const bool bPara1Avail[] = { false, true, true, true};
	const bool bPara2Avail[] = { true, true, false, false};
	const bool bPara3Avail[] = { true, true, true, false};
    const unsigned int unDst600ParaOffset[][4] = {{ 5104 - 1, 3254 - 1, 1850 - 1}};
    const unsigned int unDst300ParaOffset[][4] = {{ 2552 - 1, 1627 - 1, 925 - 1}};
#endif
	if (lXResolution == 300) {
		#define DECLARE_PARA_BLOCK_PROC_POINTER(res)                                            \
			unsigned char* pDstParaFront_1 = lpDst1 + unDst##res##ParaOffset[nSensorVer][0];    \
			unsigned char* pDstParaFront_2 = lpDst1 + unDst##res##ParaOffset[nSensorVer][1];    \
			unsigned char* pDstParaFront_3 = lpDst1 + unDst##res##ParaOffset[nSensorVer][2];    \
			unsigned char* pDstParaBack_1 = lpDst2 + unDst##res##ParaOffset[nSensorVer][0];     \
			unsigned char* pDstParaBack_2 = lpDst2 + unDst##res##ParaOffset[nSensorVer][1];     \
			unsigned char* pDstParaBack_3 = lpDst2 + unDst##res##ParaOffset[nSensorVer][2];
		#define DECLARE_PARA_BLOCK_PROC_ENDPOINTER(res)
		#define TEST_PARA_BLOCK_PROC_ENDPOINTER
		#define PARA_BLOCK_PROC_DUP_LOOP(res, i, para, fb, srcoffset)	{   \
			unsigned char* p = pSrc + srcoffset;                            \
			if (bPara##para##Avail[i]) {                                    \
				unsigned int x = unSensor##res##Table[nSensorVer][i];       \
				unsigned char* pEnd4 = p + ((x / 4) * 24);                  \
				while (pEnd4 != p) {                                        \
					*(pDstPara##fb##_##para - 1) = p[6];                    \
					*(pDstPara##fb##_##para) = p[0];                        \
					*(pDstPara##fb##_##para - 3) = p[18];                   \
					*(pDstPara##fb##_##para - 2) = p[12];                   \
					p += 24;                                                \
					pDstPara##fb##_##para -= 4;                             \
				}                                                           \
				x &= 0x3;                                                   \
				while (x--) {                                               \
					*pDstPara##fb##_##para = p[0];                          \
					pDstPara##fb##_##para -= 1;                             \
					p += 6;                                                 \
				}                                                           \
			}                                                               \
		}

		#define PARA_BLOCK_PROC_DUP(res, i)			{               \
			PARA_BLOCK_PROC_DUP_LOOP(res, i, 1, Back,	0);         \
			PARA_BLOCK_PROC_DUP_LOOP(res, i, 3, Back,	1);         \
			PARA_BLOCK_PROC_DUP_LOOP(res, i, 2, Front,	2);         \
			PARA_BLOCK_PROC_DUP_LOOP(res, i, 2, Back,	3);         \
			PARA_BLOCK_PROC_DUP_LOOP(res, i, 1, Front,	4);         \
			PARA_BLOCK_PROC_DUP_LOOP(res, i, 3, Front,	5);         \
			pSrc += unSensor##res##Table[nSensorVer][i] * 6;        \
		}
		DECLARE_PARA_BLOCK_PROC_POINTER(300);
		DECLARE_PARA_BLOCK_PROC_ENDPOINTER(300);
		PARA_BLOCK_PROC_DUP(300, 0);
		PARA_BLOCK_PROC_DUP(300, 1);
		PARA_BLOCK_PROC_DUP(300, 2);
#if defined(LIGHT_ADJUST_VOYAJER_TYPE)
#else
		PARA_BLOCK_PROC_DUP(300, 3);
#endif
		TEST_PARA_BLOCK_PROC_ENDPOINTER;
	}
	else if (lXResolution == 600) {
		DECLARE_PARA_BLOCK_PROC_POINTER(600);
		DECLARE_PARA_BLOCK_PROC_ENDPOINTER(600);
		PARA_BLOCK_PROC_DUP(600, 0);
		PARA_BLOCK_PROC_DUP(600, 1);
		PARA_BLOCK_PROC_DUP(600, 2);
#if defined(LIGHT_ADJUST_VOYAJER_TYPE)
#else
		PARA_BLOCK_PROC_DUP(600, 3);
#endif
		TEST_PARA_BLOCK_PROC_ENDPOINTER;
	}
#elif defined(LIGHT_ADJUST_CHIEBUS_TYPE) || defined(LIGHT_ADJUST_HACHI_TYPE)
	// ChieBus

	long lLoopCount = lWidth / 2;
	long lSrcPos = 0;
	for (long l=0; l<lLoopCount; l++)
	{
		lpDst1[lLoopCount - l - 1] = pSrc[lSrcPos];
		lSrcPos++;
		lpDst2[l] = pSrc[lSrcPos];
		lSrcPos++;
	}
#elif defined(LIGHT_ADJUST_EAGLE_TYPE) || defined(LIGHT_ADJUST_BOW_TYPE)
	// Bow

	long lLoopCount = lWidth / 2;
	long lSrcPos = 0;
	for (long l=0; l<lLoopCount; l++)
	{
		lpDst1[l] = pSrc[lSrcPos];
		lSrcPos++;
		lpDst2[lLoopCount - l - 1] = pSrc[lSrcPos];
		lSrcPos++;
	}
#elif defined(LIGHT_ADJUST_NEWDT_TYPE)
	// NewDT
	if (nSensorVer == 0) {
		long lCopyCount = (lWidth / 2);
		memcpy(lpDst1, pSrc, lCopyCount);
		memcpy(lpDst2, &pSrc[lCopyCount], lCopyCount);
	}
else if (nSensorVer == 1) {
	const unsigned int unSensor600Table[][3] = { { 26, 1648, 54},
												{  0, 1648, 80} };
	const unsigned int unSensor300Table[][3] = { { 13, 824,  27},
												{  0, 824,  40} };
	const bool bPara1Avail[] = { false, true, true };
	const bool bPara2Avail[] = { true, true, true };
	const bool bPara3Avail[] = { true, true, false };
	const unsigned int unDst600ParaOffset[][3] = { { 5104 - 1, 3402 - 1, 1674 - 1},
												{ 5104 - 1, 3376 - 1, 1648 - 1} };
	const unsigned int unDst300ParaOffset[][3] = { { 2552 - 1, 1701 - 1, 837 - 1},
												{ 2552 - 1, 1688 - 1, 824 - 1} };
	if (lXResolution == 300) {
		
#define DECLARE_PARA_BLOCK_PROC_POINTER(res)                                            \
				unsigned char* pDstParaFront_1 = lpDst1 + unDst##res##ParaOffset[nSensorVer][0];    \
				unsigned char* pDstParaFront_2 = lpDst1 + unDst##res##ParaOffset[nSensorVer][1];    \
				unsigned char* pDstParaFront_3 = lpDst1 + unDst##res##ParaOffset[nSensorVer][2];    \
				unsigned char* pDstParaBack_1 = lpDst2 + unDst##res##ParaOffset[nSensorVer][0];     \
				unsigned char* pDstParaBack_2 = lpDst2 + unDst##res##ParaOffset[nSensorVer][1];     \
				unsigned char* pDstParaBack_3 = lpDst2 + unDst##res##ParaOffset[nSensorVer][2];
#define DECLARE_PARA_BLOCK_PROC_ENDPOINTER(res)
#define TEST_PARA_BLOCK_PROC_ENDPOINTER
#define PARA_BLOCK_PROC_DUP_LOOP(res, i, para, fb, srcoffset)	{   \
				unsigned char* p = pSrc + srcoffset;                            \
				if (bPara##para##Avail[i]) {                                    \
					unsigned int x = unSensor##res##Table[nSensorVer][i];       \
					unsigned char* pEnd4 = p + ((x / 4) * 24);                  \
					while (pEnd4 != p) {                                        \
						*(pDstPara##fb##_##para - 1) = p[6];                    \
						*(pDstPara##fb##_##para) = p[0];                        \
						*(pDstPara##fb##_##para - 3) = p[18];                   \
						*(pDstPara##fb##_##para - 2) = p[12];                   \
						p += 24;                                                \
						pDstPara##fb##_##para -= 4;                             \
					}                                                           \
					x &= 0x3;                                                   \
					while (x--) {                                               \
						*pDstPara##fb##_##para = p[0];                          \
						pDstPara##fb##_##para -= 1;                             \
						p += 6;                                                 \
					}                                                           \
				}                                                               \
			}

#define PARA_BLOCK_PROC_DUP(res, i)			{               \
					PARA_BLOCK_PROC_DUP_LOOP(res, i, 1, Back,	0);         \
					PARA_BLOCK_PROC_DUP_LOOP(res, i, 3, Back,	1);         \
					PARA_BLOCK_PROC_DUP_LOOP(res, i, 2, Front,	2);         \
					PARA_BLOCK_PROC_DUP_LOOP(res, i, 2, Back,	3);         \
					PARA_BLOCK_PROC_DUP_LOOP(res, i, 1, Front,	4);         \
					PARA_BLOCK_PROC_DUP_LOOP(res, i, 3, Front,	5);         \
					pSrc += unSensor##res##Table[nSensorVer][i] * 6;        \
				}
			DECLARE_PARA_BLOCK_PROC_POINTER(300);
			DECLARE_PARA_BLOCK_PROC_ENDPOINTER(300);
			PARA_BLOCK_PROC_DUP(300, 0);
			PARA_BLOCK_PROC_DUP(300, 1);
			PARA_BLOCK_PROC_DUP(300, 2);
			TEST_PARA_BLOCK_PROC_ENDPOINTER;
		}
		else if (lXResolution == 600) {
			DECLARE_PARA_BLOCK_PROC_POINTER(600);
			DECLARE_PARA_BLOCK_PROC_ENDPOINTER(600);
			PARA_BLOCK_PROC_DUP(600, 0);
			PARA_BLOCK_PROC_DUP(600, 1);
			PARA_BLOCK_PROC_DUP(600, 2);
			TEST_PARA_BLOCK_PROC_ENDPOINTER;
		}
	}
	else {
		assert(false);
	}
#elif defined(LIGHT_ADJUST_DRF120_TYPE)
	// CAPRICORN
#elif defined(LIGHT_ADJUST_BUNZ_TYPE) || defined(LIGHT_ADJUST_VC_TYPE) || defined(LIGHT_ADJUST_CAROL_TYPE) || defined(LIGHT_ADJUST_MINERVA_TYPE) || defined(LIGHT_ADJUST_DASH_TYPE)
    assert(false);
#else
#error LIGHT_ADJUST_XXX_TYPE is undefined.
#endif
}

void CCollectArray::ArrayCollectPara(unsigned char* pDst, unsigned char* pSrc, long paranum, long AvailBits)
{
	for (int i = 0; i < AvailBits; i++) {
		*pDst-- = *pSrc;
		pSrc += paranum;
	}
}

void CCollectArray::ArrayCollectPara(unsigned short* pDst, unsigned short* pSrc, long paranum, long AvailBits)
{
	for (int i = 0; i < AvailBits; i++) {
		*pDst-- = *pSrc;
		pSrc += paranum;
	}
}

void CCollectArray::Extend12To16BitAndArrayCollect(unsigned short* lpDst, unsigned char* lpSrc, long lWidth, int xRes, int nSensorVer)
{
	if(!lpSrc || !lpDst)
		return;

	unsigned short* lpTmp = new unsigned short[lWidth];
	memset(lpTmp, 0, lWidth * sizeof(unsigned short));

#if defined(LIGHT_ADJUST_DOCAN_TYPE)
	Extend12To16BitCore(lpTmp, lpSrc, lWidth);

	long para = lWidth/3;
    const unsigned int* dummy = xRes == 300 ? &DUMMY_PIXEL_300[nSensorVer][0] : &DUMMY_PIXEL_600[nSensorVer][0];

	long Available1 = para - dummy[0];
	long Available2 = para - dummy[1];
	long Available3 = para - dummy[2];

	lpDst += lWidth - dummy[0] - dummy[1] - dummy[2] - 1;
	ArrayCollectPara(lpDst, lpTmp+ 1 + dummy[0]*3, 3, Available3);
	lpDst -= Available3;
	ArrayCollectPara(lpDst, lpTmp, 3, Available2);
	lpDst -= Available2;
	ArrayCollectPara(lpDst, lpTmp + 2 ,3, Available1);

#elif defined(LIGHT_ADJUST_VOYAJER_TYPE)
    Extend12To16BitCore(lpTmp, lpSrc, lWidth);
    
    const unsigned int unSensor600Table[][3] = {{ 26, 1648, 54},
                                                {  0, 1648, 80}};
    const unsigned int unSensor300Table[][3] = {{ 13, 824,  27},
                                                {  0, 824,  40}};
    const bool bPara1Avail[] = { false, true, true};
    const bool bPara2Avail[] = { true, true, true};
    const bool bPara3Avail[] = { true, true, false};
    const unsigned int unDst600ParaOffset[][3] = {{ 5104 - 1, 3402 - 1, 1674 - 1},
                                                  { 5104 - 1, 3376 - 1, 1648 - 1}};
    const unsigned int unDst300ParaOffset[][3] = {{ 2552 - 1, 1701 - 1, 837 - 1},
                                                  { 2552 - 1, 1688 - 1, 824 - 1}};
    
    const unsigned int* dummy = NULL;

    unsigned short* lpDat = lpTmp;
    if (xRes == 300)
    {
        
        LPWORD pDstPara1 = lpDst + unDst300ParaOffset[nSensorVer][0];
        LPWORD pDstPara2 = lpDst + unDst300ParaOffset[nSensorVer][1];
        LPWORD pDstPara3 = lpDst + unDst300ParaOffset[nSensorVer][2];
#define PARA_BLOCK_PROC_SIMP300_2(i)                                \
        for (UINT x=0; x<unSensor300Table[nSensorVer][i]; x++) {    \
            if (bPara2Avail[i]) *pDstPara2-- = lpDat[0];            \
            if (bPara1Avail[i]) *pDstPara1-- = lpDat[1];            \
            if (bPara3Avail[i]) *pDstPara3-- = lpDat[2];            \
            lpDat += 3;                                             \
        }
        PARA_BLOCK_PROC_SIMP300_2(0);
        PARA_BLOCK_PROC_SIMP300_2(1);
        PARA_BLOCK_PROC_SIMP300_2(2);

        dummy = &DUMMY_PIXEL_300[nSensorVer][0];
    }
    else
    {
        
        LPWORD pDstPara1 = lpDst + unDst600ParaOffset[nSensorVer][0];
        LPWORD pDstPara2 = lpDst + unDst600ParaOffset[nSensorVer][1];
        LPWORD pDstPara3 = lpDst + unDst600ParaOffset[nSensorVer][2];
#define PARA_BLOCK_PROC_SIMP600_2(i)                                \
        for (UINT x=0; x<unSensor600Table[nSensorVer][i]; x++) {    \
            if (bPara2Avail[i]) *pDstPara2-- = lpDat[0];            \
            if (bPara1Avail[i]) *pDstPara1-- = lpDat[1];            \
            if (bPara3Avail[i]) *pDstPara3-- = lpDat[2];            \
            lpDat += 3;                                             \
        }
        PARA_BLOCK_PROC_SIMP600_2(0);
        PARA_BLOCK_PROC_SIMP600_2(1);
        PARA_BLOCK_PROC_SIMP600_2(2);
        
        dummy = &DUMMY_PIXEL_600[nSensorVer][0];
    }
    
    int nWidthWODummy = lWidth - dummy[0] - dummy[1] - dummy[2];
    for (int x = 0; x < nWidthWODummy / 2; x++)
    {
        WORD tmp = lpDst[x];
        lpDst[x] = lpDst[nWidthWODummy - x - 1];
        lpDst[nWidthWODummy - x - 1] = tmp;
    }

#elif defined(LIGHT_ADJUST_CHIEBUS_TYPE) || defined(LIGHT_ADJUST_HACHI_TYPE)
	Extend12To16BitCore(lpTmp, lpSrc, lWidth);
    
	for (long l=0; l<lWidth; l++)
	{
		lpDst[l] = lpTmp[lWidth - l - 1];
	}
    
#elif defined(LIGHT_ADJUST_EAGLE_TYPE) || defined(LIGHT_ADJUST_BOW_TYPE)
	Extend12To16BitCore(lpTmp, lpSrc, lWidth);
    
	for (long l=0; l<lWidth; l++)
	{
		lpDst[l] = lpTmp[l];
	}


#elif defined(LIGHT_ADJUST_NEWDT_TYPE)
	if (nSensorVer == 0) {
		Extend12To16BitCore(lpDst, lpSrc, lWidth);
	}
	else if (nSensorVer == 1) {
		Extend12To16BitCore(lpTmp, lpSrc, lWidth);

		const unsigned int unSensor600Table[][3] = {{ 26, 1648, 54},
													{  0, 1648, 80}};
		const unsigned int unSensor300Table[][3] = {{ 13, 824,  27},
													{  0, 824,  40}};
		const bool bPara1Avail[] = { false, true, true};
		const bool bPara2Avail[] = { true, true, true};
		const bool bPara3Avail[] = { true, true, false};
		const unsigned int unDst600ParaOffset[][3] = {{ 5104 - 1, 3402 - 1, 1674 - 1},
													{ 5104 - 1, 3376 - 1, 1648 - 1}};
		const unsigned int unDst300ParaOffset[][3] = {{ 2552 - 1, 1701 - 1, 837 - 1},
													{ 2552 - 1, 1688 - 1, 824 - 1}};
		
		const unsigned int* dummy = NULL;

		unsigned short* lpDat = lpTmp;
		if (xRes == 300)
		{
			
			LPWORD pDstPara1 = lpDst + unDst300ParaOffset[nSensorVer][0];
			LPWORD pDstPara2 = lpDst + unDst300ParaOffset[nSensorVer][1];
			LPWORD pDstPara3 = lpDst + unDst300ParaOffset[nSensorVer][2];
	#define PARA_BLOCK_PROC_SIMP300_2(i)                                \
			for (UINT x=0; x<unSensor300Table[nSensorVer][i]; x++) {    \
				if (bPara2Avail[i]) *pDstPara2-- = lpDat[0];            \
				if (bPara1Avail[i]) *pDstPara1-- = lpDat[1];            \
				if (bPara3Avail[i]) *pDstPara3-- = lpDat[2];            \
				lpDat += 3;                                             \
			}
			PARA_BLOCK_PROC_SIMP300_2(0);
			PARA_BLOCK_PROC_SIMP300_2(1);
			PARA_BLOCK_PROC_SIMP300_2(2);

			dummy = &DUMMY_PIXEL_300[nSensorVer][0];
		}
		else
		{
			
			LPWORD pDstPara1 = lpDst + unDst600ParaOffset[nSensorVer][0];
			LPWORD pDstPara2 = lpDst + unDst600ParaOffset[nSensorVer][1];
			LPWORD pDstPara3 = lpDst + unDst600ParaOffset[nSensorVer][2];
	#define PARA_BLOCK_PROC_SIMP600_2(i)                                \
			for (UINT x=0; x<unSensor600Table[nSensorVer][i]; x++) {    \
				if (bPara2Avail[i]) *pDstPara2-- = lpDat[0];            \
				if (bPara1Avail[i]) *pDstPara1-- = lpDat[1];            \
				if (bPara3Avail[i]) *pDstPara3-- = lpDat[2];            \
				lpDat += 3;                                             \
			}
			PARA_BLOCK_PROC_SIMP600_2(0);
			PARA_BLOCK_PROC_SIMP600_2(1);
			PARA_BLOCK_PROC_SIMP600_2(2);
			
			dummy = &DUMMY_PIXEL_600[nSensorVer][0];
		}
	}
#elif defined(LIGHT_ADJUST_DRF120_TYPE)
	Extend12To16BitCore(lpTmp, lpSrc, lWidth);
   
	for (long l=0; l<lWidth; l++)
	{
		lpDst[l] = lpTmp[lWidth - l - 1];
	}

#elif defined(LIGHT_ADJUST_BUNZ_TYPE) || defined(LIGHT_ADJUST_VC_TYPE) || defined(LIGHT_ADJUST_CAROL_TYPE) || defined(LIGHT_ADJUST_MINERVA_TYPE) || defined(LIGHT_ADJUST_DASH_TYPE)
    Extend8To16BitCore(lpDst, lpSrc, lWidth);
#else
#error LIGHT_ADJUST_XXX_TYPE is undefined.
#endif

	delete [] lpTmp;
}

void CCollectArray::Extend12To16BitAndSeparate(unsigned short* lpDst1, unsigned short* lpDst2, unsigned char* lpSrc, long lWidth, int xRes, int nSensorVer)
{
	if(!lpSrc || !lpDst1 || !lpDst2) return;

	unsigned short* lpTmp = new unsigned short[lWidth];
	memset(lpTmp, 0, lWidth * sizeof(unsigned short));

#if defined(LIGHT_ADJUST_DOCAN_TYPE)
	Extend12To16BitCore(lpTmp, lpSrc, lWidth);

	long SimpleWidth = lWidth / 2;
	long para = SimpleWidth / 3;
    const unsigned int* dummy = xRes == 300 ? &DUMMY_PIXEL_300[nSensorVer][0] : &DUMMY_PIXEL_600[nSensorVer][0];
    
	long Available1 = para - dummy[0];
	long Available2 = para - dummy[1];
	long Available3 = para - dummy[2];

	
	lpDst2 += SimpleWidth - dummy[0] - dummy[1] - dummy[2] - 1;
	ArrayCollectPara(lpDst2, lpTmp + dummy[0]*6 			, 6, Available3);
	lpDst2 -= Available3;
	ArrayCollectPara(lpDst2, lpTmp + 3						, 6, Available2);
	lpDst2 -= Available2;
	ArrayCollectPara(lpDst2, lpTmp + 1						, 6, Available1);

	lpDst1 += SimpleWidth - dummy[0] - dummy[1] - dummy[2] - 1;
	ArrayCollectPara(lpDst1, lpTmp + 4 + dummy[0]*6		, 6, Available3);
	lpDst1 -= Available3;
	ArrayCollectPara(lpDst1, lpTmp + 2						, 6, Available2);
	lpDst1 -= Available2;
	ArrayCollectPara(lpDst1, lpTmp + 5						 , 6, Available1);
#elif defined(LIGHT_ADJUST_VOYAJER_TYPE)
    Extend12To16BitCore(lpTmp, lpSrc, lWidth);
    
    UINT unSensor600Table[][3] = {{26, 1648, 54},
                                  { 0, 1648, 80}};
    UINT unSensor300Table[][3] = {{13, 824, 27},
                                  { 0, 824, 40}};
    const bool bPara1Avail[] = { false, true, true};
    const bool bPara2Avail[] = { true, true, true};
    const bool bPara3Avail[] = { true, true, false};
    UINT unDst600ParaOffset[][3] = {{ 5104 - 1, 3402 - 1, 1674 - 1},
                                    { 5104 - 1, 3376 - 1, 1648 - 1}};
    UINT unDst300ParaOffset[][3] = {{ 2552 - 1, 1701 - 1, 837 - 1},
                                    { 2552 - 1, 1688 - 1, 824 - 1}};
    
    const unsigned int* dummy;

    unsigned short* lpDat = lpTmp;
    if (xRes == 300)
    {
         #define DECLARE_PARA_BLOCK_PROC_POINTER(res)                                \
        LPWORD pDstParaFront1 = lpDst1 + unDst##res##ParaOffset[nSensorVer][0];     \
        LPWORD pDstParaFront2 = lpDst1 + unDst##res##ParaOffset[nSensorVer][1];     \
        LPWORD pDstParaFront3 = lpDst1 + unDst##res##ParaOffset[nSensorVer][2];     \
        LPWORD pDstParaBack1 = lpDst2 + unDst##res##ParaOffset[nSensorVer][0];      \
        LPWORD pDstParaBack2 = lpDst2 + unDst##res##ParaOffset[nSensorVer][1];      \
        LPWORD pDstParaBack3 = lpDst2 + unDst##res##ParaOffset[nSensorVer][2];

        #define PARA_BLOCK_PROC_DUP_LOOP(res, i, para, fb, srcoffset) { \
            LPWORD p = lpDat + srcoffset;                               \
            if (bPara##para##Avail[i]) {                                \
                UINT x = unSensor##res##Table[nSensorVer][i];           \
                LPWORD pEnd4 = p + ((x / 4) * 24);                      \
                while (pEnd4 != p) {                                    \
                    DWORD w = p[0] << 16 | p[6];                        \
                    *(LPDWORD)(pDstPara##fb##para - 1) = w;             \
                    w = p[12] << 16 | p[18];                            \
                    *(LPDWORD)(pDstPara##fb##para - 3) = w;             \
                    p += 24;                                            \
                    pDstPara##fb##para -= 4;                            \
                }                                                       \
                x &= 0x3;                                               \
                while (x--) {                                           \
                    *pDstPara##fb##para = p[0];                         \
                    pDstPara##fb##para--;                               \
                    p += 6;                                             \
                }                                                       \
            }                                                           \
        }
            
        #define PARA_BLOCK_PROC_DUP(res, i) {                       \
            PARA_BLOCK_PROC_DUP_LOOP(res, i, 1, Back,	0);         \
            PARA_BLOCK_PROC_DUP_LOOP(res, i, 3, Back,	1);         \
            PARA_BLOCK_PROC_DUP_LOOP(res, i, 2, Front,	2);         \
            PARA_BLOCK_PROC_DUP_LOOP(res, i, 2, Back,	3);         \
            PARA_BLOCK_PROC_DUP_LOOP(res, i, 1, Front,	4);         \
            PARA_BLOCK_PROC_DUP_LOOP(res, i, 3, Front,	5);         \
            lpDat += unSensor##res##Table[nSensorVer][i] * 6;       \
        }
            
        DECLARE_PARA_BLOCK_PROC_POINTER(300);
        PARA_BLOCK_PROC_DUP(300, 0);
        PARA_BLOCK_PROC_DUP(300, 1);
        PARA_BLOCK_PROC_DUP(300, 2);
        //PARA_BLOCK_PROC_DUP(300, 3);

        dummy = &DUMMY_PIXEL_300[nSensorVer][0];
    }
    else
    {
        DECLARE_PARA_BLOCK_PROC_POINTER(600);
        PARA_BLOCK_PROC_DUP(600, 0);
        PARA_BLOCK_PROC_DUP(600, 1);
        PARA_BLOCK_PROC_DUP(600, 2);
        //PARA_BLOCK_PROC_DUP(600, 3);

        dummy = &DUMMY_PIXEL_600[nSensorVer][0];
    }

    int nWidthWODummy = lWidth / 2 - dummy[0] - dummy[1] - dummy[2];
    for (int x = 0; x < nWidthWODummy / 2; x++)
    {
        WORD tmp = lpDst1[x];
        lpDst1[x] = lpDst1[nWidthWODummy - x - 1];
        lpDst1[nWidthWODummy - x - 1] = tmp;
    }

#elif defined(LIGHT_ADJUST_CHIEBUS_TYPE) || defined(LIGHT_ADJUST_HACHI_TYPE)
	Extend12To16BitCore(lpTmp, lpSrc, lWidth);

	long lLoopCount = lWidth / 2;
	long lSrcPos = 0;

	for (long l=0; l<lLoopCount; l++)
	{
		lpDst1[lLoopCount - l - 1] = lpTmp[lSrcPos];
		lSrcPos++;
		lpDst2[l] = lpTmp[lSrcPos];
		lSrcPos++;
	}
#elif defined(LIGHT_ADJUST_EAGLE_TYPE) || defined(LIGHT_ADJUST_BOW_TYPE)
	Extend12To16BitCore(lpTmp, lpSrc, lWidth);

	long lLoopCount = lWidth / 2;
	long lSrcPos = 0;

	for (long l=0; l<lLoopCount; l++)
	{
		lpDst1[l] = lpTmp[lSrcPos];
		lSrcPos++;
		lpDst2[lLoopCount - l - 1] = lpTmp[lSrcPos];
		lSrcPos++;
	}
#elif defined(LIGHT_ADJUST_NEWDT_TYPE)
	Extend12To16BitCore(lpTmp, lpSrc, lWidth);
	if (nSensorVer == 0) {
		long lCopyCount = (lWidth / 2);
		long lCopySize = lCopyCount * sizeof(unsigned short);
		memcpy(lpDst1, lpTmp, lCopySize);
		memcpy(lpDst2, &lpTmp[lCopyCount], lCopySize);
	}
	else if (nSensorVer == 1) {
		UINT unSensor600Table[][3] = { {26, 1648, 54},
									{ 0, 1648, 80} };
		UINT unSensor300Table[][3] = { {13, 824, 27},
									{ 0, 824, 40} };
		const bool bPara1Avail[] = { false, true, true };
		const bool bPara2Avail[] = { true, true, true };
		const bool bPara3Avail[] = { true, true, false };
		UINT unDst600ParaOffset[][3] = { { 5104 - 1, 3402 - 1, 1674 - 1},
										{ 5104 - 1, 3376 - 1, 1648 - 1} };
		UINT unDst300ParaOffset[][3] = { { 2552 - 1, 1701 - 1, 837 - 1},
										{ 2552 - 1, 1688 - 1, 824 - 1} };

		const unsigned int* dummy;

		unsigned short* lpDat = lpTmp;
		if (xRes == 300)
		{
			
#define DECLARE_PARA_BLOCK_PROC_POINTER(res)                                \
			LPWORD pDstParaFront1 = lpDst1 + unDst##res##ParaOffset[nSensorVer][0];     \
			LPWORD pDstParaFront2 = lpDst1 + unDst##res##ParaOffset[nSensorVer][1];     \
			LPWORD pDstParaFront3 = lpDst1 + unDst##res##ParaOffset[nSensorVer][2];     \
			LPWORD pDstParaBack1 = lpDst2 + unDst##res##ParaOffset[nSensorVer][0];      \
			LPWORD pDstParaBack2 = lpDst2 + unDst##res##ParaOffset[nSensorVer][1];      \
			LPWORD pDstParaBack3 = lpDst2 + unDst##res##ParaOffset[nSensorVer][2];

#define PARA_BLOCK_PROC_DUP_LOOP(res, i, para, fb, srcoffset) { \
				LPWORD p = lpDat + srcoffset;                               \
				if (bPara##para##Avail[i]) {                                \
					UINT x = unSensor##res##Table[nSensorVer][i];           \
					LPWORD pEnd4 = p + ((x / 4) * 24);                      \
					while (pEnd4 != p) {                                    \
						DWORD w = p[0] << 16 | p[6];                        \
						*(LPDWORD)(pDstPara##fb##para - 1) = w;             \
						w = p[12] << 16 | p[18];                            \
						*(LPDWORD)(pDstPara##fb##para - 3) = w;             \
						p += 24;                                            \
						pDstPara##fb##para -= 4;                            \
					}                                                       \
					x &= 0x3;                                               \
					while (x--) {                                           \
						*pDstPara##fb##para = p[0];                         \
						pDstPara##fb##para--;                               \
						p += 6;                                             \
					}                                                       \
				}                                                           \
			}

#define PARA_BLOCK_PROC_DUP(res, i) {                       \
				PARA_BLOCK_PROC_DUP_LOOP(res, i, 1, Back,	0);         \
				PARA_BLOCK_PROC_DUP_LOOP(res, i, 3, Back,	1);         \
				PARA_BLOCK_PROC_DUP_LOOP(res, i, 2, Front,	2);         \
				PARA_BLOCK_PROC_DUP_LOOP(res, i, 2, Back,	3);         \
				PARA_BLOCK_PROC_DUP_LOOP(res, i, 1, Front,	4);         \
				PARA_BLOCK_PROC_DUP_LOOP(res, i, 3, Front,	5);         \
				lpDat += unSensor##res##Table[nSensorVer][i] * 6;       \
			}

			DECLARE_PARA_BLOCK_PROC_POINTER(300);
			PARA_BLOCK_PROC_DUP(300, 0);
			PARA_BLOCK_PROC_DUP(300, 1);
			PARA_BLOCK_PROC_DUP(300, 2);
			//PARA_BLOCK_PROC_DUP(300, 3);

			dummy = &DUMMY_PIXEL_300[nSensorVer][0];
		}
		else
		{
			DECLARE_PARA_BLOCK_PROC_POINTER(600);
			PARA_BLOCK_PROC_DUP(600, 0);
			PARA_BLOCK_PROC_DUP(600, 1);
			PARA_BLOCK_PROC_DUP(600, 2);
			//PARA_BLOCK_PROC_DUP(600, 3);

			dummy = &DUMMY_PIXEL_600[nSensorVer][0];
		}
	}
	else {
		assert(false);
	}
#elif defined(LIGHT_ADJUST_DRF120_TYPE)
	Extend12To16BitCore(lpTmp, lpSrc, lWidth);

	long lLoopCount = lWidth / 2;
	long lSrcPos = 0;

	for (long l=0; l<lLoopCount; l++)
	{
		lpDst1[l] = lpTmp[lSrcPos];
		lSrcPos++;
		lpDst2[lLoopCount - l - 1] = lpTmp[lSrcPos];
		lSrcPos++;
	}

#elif defined(LIGHT_ADJUST_BUNZ_TYPE) || defined(LIGHT_ADJUST_VC_TYPE) || defined(LIGHT_ADJUST_CAROL_TYPE) || defined(LIGHT_ADJUST_MINERVA_TYPE) || defined(LIGHT_ADJUST_DASH_TYPE)
    Extend8To16BitCore(lpTmp, lpSrc, lWidth);
    
    long lCopyCount = (lWidth / 2);
    long lCopySize = lCopyCount * sizeof(unsigned short);
    memcpy(lpDst1, lpTmp, lCopySize);
    memcpy(lpDst2, &lpTmp[lCopyCount], lCopySize);
#else
#error LIGHT_ADJUST_XXX_TYPE is undefined.
#endif

	delete [] lpTmp;
}

void CCollectArray::Extend12To16BitCore(unsigned short* lpDst, unsigned char* lpSrc, long lWidth)
{
	if(!lpSrc || !lpDst)
		return;

#define UNPACK12_0 *lpDst++ = MAKEWORD(lpSrc[0], lpSrc[1]) & 0xfff, lpSrc++;
#define UNPACK12_1 *lpDst++ = MAKEWORD(lpSrc[0], lpSrc[1]) >> 4, lpSrc+=2;
#define UNPACK12_2 *lpDst++ = MAKEWORD(lpSrc[0], lpSrc[1]) & 0xfff, lpSrc++;
#define UNPACK12_3 *lpDst++ = MAKEWORD(lpSrc[0], lpSrc[1]) >> 4, lpSrc+=2;

	while(lWidth > 3) {
		lWidth -= 4;
		UNPACK12_0;
		UNPACK12_1;
		UNPACK12_2;
		UNPACK12_3;
	}
	if((lWidth) && (lWidth--)) {
		UNPACK12_0;
	}
	if((lWidth) && (lWidth--))
		UNPACK12_1;
	if((lWidth) && (lWidth--))
		UNPACK12_2;
}


void CCollectArray::Extend8To16BitCore(unsigned short* lpDst, unsigned char* lpSrc, long lWidth)
{
    while (lWidth-- > 0) {
        *lpDst = *lpSrc;
        lpDst++;
        lpSrc++;
    }
}
