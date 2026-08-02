/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include "Shading.h"
#include "ceilib.h"
#include <memory.h>
#include <assert.h>
#include <algorithm>
#include "ceiipsimd.h"

#include "CeiImgEdit.h"

using namespace Cei;
using namespace LLiPm;
using namespace DR_NAMESPACE;

#ifdef _WIN64
#undef SSE2
#endif

#ifdef __ARM_NEON__
#include <arm_neon.h>
#endif

CShading::CShading(void)
{
    memset(&m_avgPlaten, 0, sizeof(m_avgPlaten));
}

CShading::~CShading(void)
{
}

RTN CShading::IP(CImg& image)
{
	return Shading(image);
}

RTN CShading::IPFirst(CImg& image)
{
	return Shading(image);
}

RTN CShading::IPMiddle(CImg& image)
{
	return Shading(image);
}

RTN CShading::IPLast(CImg& image)
{
	return Shading(image);
}

RTN CShading::setInfo(CImg& image, void* lpInfo)
{
	if (lpInfo == 0) {
		return RTN_PAR;
	}
	SHADINGINFO* pShadingInfo = (SHADINGINFO*)lpInfo;
	if (pShadingInfo->ulSize != sizeof(SHADINGINFO)) {
		return RTN_PAR;
	}

	m_Info = *pShadingInfo;
	return RTN_OK;
}

RTN CShading::Shading(CImg& image)
{
#if defined(LIGHT_ADJUST_CHIEBUS_TYPE) || defined(LIGHT_ADJUST_DOCAN_TYPE) || defined(LIGHT_ADJUST_HACHI_TYPE) || defined(LIGHT_ADJUST_BOW_TYPE) || defined(LIGHT_ADJUST_NEWDT_TYPE) || defined(LIGHT_ADJUST_VOYAJER_TYPE) || defined(LIGHT_ADJUST_DRF120_TYPE)
	if (m_Info.imgBlack.isNull() || m_Info.imgWhite.isNull()) {
		if (image.getBps() == 16) {
            mulImage(image, 16);
			return pack8OnUpperByteImage(image);
		}
		else if (image.getBps() == 8) {
			return RTN_PAR;
		}
	}

	if (image.getBps() == 8) {
		IMAGEINFO Info;
		memset(&Info, 0, sizeof(Info));
		Info.ulSize = sizeof(Info);
		Info.lWidth = image.getWidth();
		Info.lHeight = image.getHeight();
		Info.lBps = image.getBps();
		Info.lSpp = image.getSpp();
		Info.ulRGBOrder = PIXEL_ORDER;
		Info.lXResolution = image.getXResolution();
		Info.lYResolution = image.getYResolution();

		if (image.getSpp() == 3 && image.getRGBOrder() == LINE_ORDER) {
			Info.lSync = image.getWidth() * image.getSpp();
			Info.tImageSize = Info.lHeight * Info.lSync;
		}
		else {
			Info.lSync = image.getSync();
			Info.tImageSize = image.getImageSize();
		}

		CImg imgDst;
		if (!imgDst.createImg(Info)) {
			return RTN_PAR;
		}
		if (imgDst.isNull()) {
			return RTN_NOMEM;
		}

		RTN result;
		if (image.getSpp() == 3) {
			result = ShadingColor(imgDst, image);
		}
		else {
			result = ShadingGray(imgDst, image);
		}
		if (result != RTN_OK) {
			return result;
		}
		image.attachImg(imgDst);
	}
	else if (image.getBps() == 16) {
        IMAGEINFO Info;
		memset(&Info, 0, sizeof(Info));
		Info.ulSize = sizeof(Info);
		Info.lWidth = image.getWidth();
		Info.lHeight = image.getHeight();
		Info.lSync = image.getWidth() * image.getSpp();
		Info.lBps = 8;
		Info.lSpp = image.getSpp();
		Info.ulRGBOrder = PIXEL_ORDER;
		Info.lXResolution = image.getXResolution();
		Info.lYResolution = image.getYResolution();

		CImg imgDst;
		if (!imgDst.createImg(Info)) {
			return RTN_PAR;
		}
		if (imgDst.isNull()) {
			return RTN_NOMEM;
		}

		if (image.getSpp() == 3) {
			RTN result = ShadingColor(imgDst, image);
			if (result != RTN_OK) {
				return result;
			}
		}
		else {
			RTN result = ShadingGray(imgDst, image);
			if (result != RTN_OK) {
				return result;
			}
		}
		image.attachImg(imgDst);
	}
#elif defined(LIGHT_ADJUST_BUNZ_TYPE) || defined(LIGHT_ADJUST_VC_TYPE) || defined(LIGHT_ADJUST_CAROL_TYPE) || defined(LIGHT_ADJUST_MINERVA_TYPE) || defined(LIGHT_ADJUST_DASH_TYPE)
	CeiLogger::writeLog("CShading::Shading() does not need to work. The scanner has already done shading process.");
#else
#error LIGHT_ADJUST_XXX_TYPE is undefined.
#endif
	return RTN_OK;
}
RTN CShading::ShadingColor(CImg& imgDst, CImg& imgSrc)
{
	if (imgDst.getRGBOrder() != PIXEL_ORDER) {
		return RTN_PAR;
	}

	if (imgSrc.getBps() == 8) {
		unsigned char* pShadingWhitePtr = m_Info.imgWhite.getImagePtr();
		unsigned char* pShadingBlackPtr = m_Info.imgBlack.getImagePtr();
		long lShadingWhiteSync = m_Info.imgWhite.getSync();
		long lSrcSync = imgSrc.getSync();	
		long lDstSync = imgDst.getSync();
		if (imgSrc.getRGBOrder() == LINE_ORDER) {
			if (m_Info.imgWhite.getRGBOrder() != LINE_ORDER) {
				return RTN_PAR;
			}

			unsigned char* pDstLineTop = imgDst.getImagePtr();
			unsigned char* pSrcLineTop = imgSrc.getImagePtr();
			long lLine = imgDst.getHeight();
			long w = imgSrc.getWidth();
			if (IsSSE2FeatureAvailable()) {
				while (lLine--) {
					ShadingColorCore_OrderLine2Pixel_SIMD(
						pDstLineTop, 
						pSrcLineTop, 
						lSrcSync, 
						(unsigned short*)pShadingWhitePtr, 
						lShadingWhiteSync, 
						(unsigned short*)pShadingBlackPtr, 
						w);
					pSrcLineTop += lSrcSync * imgSrc.getSpp();
					pDstLineTop += lDstSync;
				}
			}
			else if (IsNEONFeatureAvailable()) {
				while (lLine--) {
					ShadingColorCore_OrderLine2Pixel_NEON(
						pDstLineTop, 
						pSrcLineTop, 
						lSrcSync, 
						(unsigned short*)pShadingWhitePtr, 
						lShadingWhiteSync, 
						(unsigned short*)pShadingBlackPtr, 
						w);
					pSrcLineTop += lSrcSync * imgSrc.getSpp();
					pDstLineTop += lDstSync;
				}
			}
			else {
				while (lLine--) {
					ShadingColorCore_OrderLine2Pixel_NonSIMD(
						pDstLineTop, 
						pSrcLineTop, 
						lSrcSync, 
						(unsigned short*)pShadingWhitePtr, 
						lShadingWhiteSync, 
						(unsigned short*)pShadingBlackPtr, 
						w);
					pSrcLineTop += lSrcSync * imgSrc.getSpp();
					pDstLineTop += lDstSync;
				}
			}
		}
		else if (imgSrc.getRGBOrder() == PIXEL_ORDER) {
           if (m_Info.imgWhite.getRGBOrder() != PIXEL_ORDER) {
				return RTN_PAR;
			}

			unsigned char* pDstLineTop = imgDst.getImagePtr();
			unsigned char* pSrcLineTop = imgSrc.getImagePtr();
			long lLine = imgDst.getHeight();
			while (lLine--) {
				unsigned char* psrc = pSrcLineTop;
				unsigned short* black = (unsigned short*) pShadingBlackPtr;
				unsigned short* white = (unsigned short*) pShadingWhitePtr;
				unsigned char* pdst = pDstLineTop;
				long w = imgSrc.getWidth() * imgSrc.getSpp();
				while (w--) {
					int iPix = (int)*psrc++;
					iPix -= *black++;
					iPix = std::max(iPix, 0);
					iPix = (iPix * *white++) >> 12;
					iPix = std::min(iPix, 0xff);
					*pdst++ = (unsigned char) iPix;
				}

				pSrcLineTop += lSrcSync;
				pDstLineTop += lDstSync;
			}
		}
		else {
			return RTN_PAR;
		}
	}
	else {
		unsigned char* pShadingWhitePtr = m_Info.imgWhite.getImagePtr();
		unsigned char* pShadingBlackPtr = m_Info.imgBlack.getImagePtr();
		long lShadingWhiteSync = m_Info.imgWhite.getSync();
		long lSrcSync = imgSrc.getSync();
		long lDstSync = imgDst.getSync();
		if (imgSrc.getRGBOrder() == LINE_ORDER) {
			if (m_Info.imgWhite.getRGBOrder() != LINE_ORDER) {
				return RTN_PAR;
			}
			unsigned char* pDstLineTop = imgDst.getImagePtr();
			unsigned char* pSrcLineTop = imgSrc.getImagePtr();
			long lLine = imgDst.getHeight();
			while (lLine--) {
				unsigned short* r = (unsigned short*) (pSrcLineTop + 0 * lSrcSync);
				unsigned short* g = (unsigned short*) (pSrcLineTop + 1 * lSrcSync);
				unsigned short* b = (unsigned short*) (pSrcLineTop + 2 * lSrcSync);
				unsigned short* red = (unsigned short*) (pShadingWhitePtr + 0 * lShadingWhiteSync);
				unsigned short* green = (unsigned short*) (pShadingWhitePtr + 1 * lShadingWhiteSync);
				unsigned short* blue = (unsigned short*) (pShadingWhitePtr + 2 * lShadingWhiteSync);
				unsigned short* black = (unsigned short*) pShadingBlackPtr;
				unsigned char* pdst = pDstLineTop;
				long w = imgSrc.getWidth();
				int iPix(0);

				while (w--) {
					// Red
					iPix = *r++;
					iPix -= *black;
					iPix = std::max(iPix, 0);
					iPix = (iPix * *red++) >> 16;
					iPix = std::min(iPix, 0xff);
					*pdst++ = static_cast<unsigned char>(iPix);

					// Green
					iPix = *g++;
					iPix -= *black;
					iPix = std::max(iPix, 0);
					iPix = (iPix * *green++) >> 16;
					iPix = std::min(iPix, 0xff);
					*pdst++ = static_cast<unsigned char>(iPix);

					// Blue
					iPix = *b++;
					iPix -= *black;
					iPix = std::max(iPix, 0);
					iPix = (iPix * *blue++) >> 16;
					iPix = std::min(iPix, 0xff);
					*pdst++ = static_cast<unsigned char>(iPix);

					black++;
					// 	*pdst++ = (BYTE) (*r++ >> 4);
					// 	*pdst++ = (BYTE) (*g++ >> 4);
					// 	*pdst++ = (BYTE) (*b++ >> 4);
				}
				pSrcLineTop += lSrcSync * imgSrc.getSpp();
				pDstLineTop += lDstSync;
			}
		}
		else if (imgSrc.getRGBOrder() == PIXEL_ORDER) {
			if (m_Info.imgWhite.getRGBOrder() != PIXEL_ORDER) {
				return RTN_PAR;
			}

			unsigned char* pDstLineTop = imgDst.getImagePtr();
			unsigned char* pSrcLineTop = imgSrc.getImagePtr();
			long lLine = imgDst.getHeight();
			while (lLine--) {
				unsigned short* psrc = (unsigned short*) pSrcLineTop;
				unsigned short* black = (unsigned short*) pShadingBlackPtr;
				unsigned short* white = (unsigned short*) pShadingWhitePtr;
				unsigned char* pdst = pDstLineTop;
				long w = imgSrc.getWidth() * imgSrc.getSpp();
				while (w--) {
					int iPix = (int) *psrc++;
					iPix -= *black++;
					iPix = std::max(iPix, 0);
					iPix = (iPix * *white++) >> 16;
					iPix = std::min(iPix, 0xff);
					*pdst++ = (unsigned char) iPix;
					// *pdst++ = (BYTE) (*psrc++ >> 4);
				}
				pSrcLineTop += lSrcSync;
				pDstLineTop += lDstSync;
			}
		}
		else {
			return RTN_PAR;
		}
	}
	return RTN_OK;
}

RTN CShading::ShadingGray(CImg& imgDst, CImg& imgSrc)
{
	if (imgSrc.getBps() == 8) {
		// 8�r�b�g�摜�ցA16�r�b�g�V�F�[�f�B���O�f�[�^��K�p����
		if (m_Info.imgBlack.getBps() != 16 || m_Info.imgWhite.getBps() != 16) {
			return RTN_PAR;
		}

		unsigned char* pDstLineTop = imgDst.getImagePtr();
		unsigned char* pSrcLineTop = imgSrc.getImagePtr();
		long lLine = imgSrc.getHeight();

		unsigned char* pDst;
		unsigned char* pSrc;
		unsigned short* pBlack;
		unsigned short* pWhite;
		while (lLine--) {
			pDst = pDstLineTop;
			pSrc = pSrcLineTop;
			pBlack = (unsigned short*)m_Info.imgBlack.getImagePtr();
			pWhite = (unsigned short*)m_Info.imgWhite.getImagePtr();

			// 1���C���V�F�[�f�B���O���āA���ʂ�8�r�b�g�֕ϊ�����
			long w = std::min(
						std::min(
							static_cast<unsigned long>(imgSrc.getWidth()), 
							static_cast<unsigned long>(m_Info.imgBlack.getImageSize()/sizeof(unsigned short))
							), 
						static_cast<unsigned long>(m_Info.imgWhite.getImageSize()/sizeof(unsigned short)));

			if (IsSSE2FeatureAvailable()) {
				ShadingGrayCore_SIMD(pDst, pSrc, pWhite, pBlack, w);
			}
			else if (IsNEONFeatureAvailable()) {
				ShadingGrayCore_NEON(pDst, pSrc, pWhite, pBlack, w);
			}
			else {
				ShadingGrayCore_NonSIMD(pDst, pSrc, pWhite, pBlack, w);
			}

			pDstLineTop += imgDst.getSync();
			pSrcLineTop += imgSrc.getSync();
		}
	}
	else {
		unsigned char* pDstLineTop = imgDst.getImagePtr();
		unsigned char* pSrcLineTop = imgSrc.getImagePtr();
		long lLine = imgSrc.getHeight();

		unsigned char* pDst;
		unsigned short* pSrc;
		unsigned short* pBlack;
		unsigned short* pWhite;
		while (lLine--) {
			pDst = pDstLineTop;
			pSrc = (unsigned short*)pSrcLineTop;
			pBlack = (unsigned short*)m_Info.imgBlack.getImagePtr();
			pWhite = (unsigned short*)m_Info.imgWhite.getImagePtr();

			// 1���C���V�F�[�f�B���O���āA���ʂ�8�r�b�g�֕ϊ�����
			long w = std::min(
						std::min(
							static_cast<unsigned long>(imgSrc.getWidth()), 
							static_cast<unsigned long>(m_Info.imgBlack.getImageSize()/sizeof(unsigned short))
							), 
						static_cast<unsigned long>(m_Info.imgWhite.getImageSize()/sizeof(unsigned short)));

			while (w--) {
				int iPix = (int) *pSrc++;
				iPix -= *pBlack++;
				iPix = std::max(iPix, 0);
				iPix = (iPix * *pWhite++) >> 16;
				iPix = std::min(iPix, 0xff);
				*pDst++ = (unsigned char) iPix;
			}
			pDstLineTop += imgDst.getSync();
			pSrcLineTop += imgSrc.getSync();
		}
	}
	return RTN_OK;
}

void CShading::ShadingColorCore_OrderLine2Pixel_SIMD(unsigned char* lpDst, unsigned char* lpSrc, long lSync, unsigned short* lpWhite, long lShadSync, unsigned short* lpBlack, long w)
{
#ifndef _WIN64
#ifdef SSE2
	while (w > 7) {
		__m128i ret[3];

		// red - green - blue
		for (int i = 0; i < 3; i++) {
			unsigned char* psrc = lpSrc + i * lSync;
			unsigned short* white = (unsigned short*)((unsigned char*)lpWhite + i * lShadSync);
			unsigned short* black = lpBlack;

			__m64 pix = *(__m64 *)psrc;								// HGFEDCBA
			__m64 pixH = _mm_unpackhi_pi8(pix, _mm_setzero_si64());	// 0H0G0F0E
			__m64 pixL = _mm_unpacklo_pi8(pix, _mm_setzero_si64());	// 0D0C0B0A

			__m64 b;
			b = *(__m64 *)(black + 0);				// ���f�[�^�ǂݍ��݁i4��f�j
			pixL = _mm_subs_pu16(pixL, b);			// 4���[�h�̖O�a�����Z�i0������0�ɒ���t���j
			b = *(__m64 *)(black + 4);				// ���f�[�^�ǂݍ��݁i4��f�j
			pixH = _mm_subs_pu16(pixH, b);			// 4���[�h�̖O�a�����Z�i0������0�ɒ���t���j

			__m64 wh, tmp;
			wh = *(__m64 *)(white + 0);				// ���f�[�^�ǂݍ��݁i4��f)
			tmp = _mm_slli_pi16(pixL, 4);			// x16 ���V�t�g4
			pixL = _mm_mulhi_pu16(tmp, wh);			// *white / 65536
			wh = *(__m64 *)(white + 4);				// ���f�[�^�ǂݍ��݁i4��f)
			tmp = _mm_slli_pi16(pixH, 4);			// x16 ���V�t�g4
			pixH = _mm_mulhi_pu16(tmp, wh);			// *white / 65536

			tmp = _mm_packs_pu16(pixL, pixH);		// 4���[�h�~2��8�o�C�g�Ƀp�b�N����B256�ȏ��255�ɒ���t��

            ret[i] = _mm_movpi64_epi64(tmp);
		}

__m128i rg = _mm_unpacklo_epi8(ret[0], ret[1]);
		__m128i gb = _mm_unpacklo_epi8(ret[1], ret[2]);
		ret[0] = _mm_srli_epi64(ret[0], 8);
		__m128i br = _mm_unpacklo_epi8(ret[2], ret[0]);
		*(unsigned short*)(lpDst+ 0) = _mm_extract_epi16(rg, 0);
		*(unsigned short*)(lpDst+ 2) = _mm_extract_epi16(br, 0);
		*(unsigned short*)(lpDst+ 4) = _mm_extract_epi16(gb, 1);
		*(unsigned short*)(lpDst+ 6) = _mm_extract_epi16(rg, 2);
		*(unsigned short*)(lpDst+ 8) = _mm_extract_epi16(br, 2);
		*(unsigned short*)(lpDst+ 10) = _mm_extract_epi16(gb, 3);
		*(unsigned short*)(lpDst+ 12) = _mm_extract_epi16(rg, 4);
		*(unsigned short*)(lpDst+ 14) = _mm_extract_epi16(br, 4);
		*(unsigned short*)(lpDst+ 16) = _mm_extract_epi16(gb, 5);
		*(unsigned short*)(lpDst+ 18) = _mm_extract_epi16(rg, 6);
		*(unsigned short*)(lpDst+ 20) = _mm_extract_epi16(br, 6);
		*(unsigned short*)(lpDst+ 22) = _mm_extract_epi16(gb, 7);

		lpDst += 24;
		lpSrc += 8;
		lpBlack += 8;
		lpWhite += 8;
		w -= 8;
	}
	_mm_empty();

	lpDst += 3 * (w & ~0x7);
	lpSrc += w & ~0x7;
	lpWhite += w & ~0x7;
	lpBlack += w & ~0x7;
	w &= 0x7;
	ShadingColorCore_OrderLine2Pixel_NonSIMD(lpDst, lpSrc, lSync, lpWhite, lShadSync, lpBlack, w);
#else //SSE2
	assert(false);
#endif //SSE2
#endif
}

void CShading::ShadingColorCore_OrderLine2Pixel_NEON(unsigned char* lpDst, unsigned char* lpSrc, long lSync, unsigned short* lpWhite, long lShadSync, unsigned short* lpBlack, long w)
{
#ifdef __ARM_NEON__
	uint8x8x3_t pixes;
	uint16x8_t pix;
	
	while (w > 7) {
		for (int i = 0; i < 3; i++) {
			unsigned char* psrc = lpSrc + i * lSync;
			unsigned short* white = (unsigned short*)((unsigned char*)lpWhite + i * lShadSync);
			unsigned short* black = lpBlack;
			pix = vmovl_u8(vld1_u8((uint8_t *)psrc));
			pix = vqsubq_u16(pix, vld1q_u16((uint16_t *)black));
			
			pix = vqshlq_n_u16(pix, 3);

			pix = (uint16x8_t)vqdmulhq_s16((int16x8_t)pix, vld1q_s16((int16_t *)white));
			
			// 1byte pack
			pixes.val[i]= vqmovn_u16(pix);
			
		}
		// output
		vst3_u8((uint8_t *)(lpDst), pixes);	

		lpDst += 24;
		w -= 8;
		lpWhite += 8;
		lpBlack += 8;
		lpSrc += 8;
	}

#else //__ARM_NEON__
#endif //__ARM_NEON__
	return ShadingColorCore_OrderLine2Pixel_NonSIMD(lpDst, lpSrc, lSync, lpWhite, lShadSync, lpBlack, w);
}

void CShading::ShadingColorCore_OrderLine2Pixel_NonSIMD(unsigned char* lpDst, unsigned char* lpSrc, long lSync, unsigned short* lpWhite, long lShadSync, unsigned short* lpBlack, long w)
{
	while (w--) {
		int iPix;
		// RED
		iPix = (int)lpSrc[0 * lSync];
		iPix -= *lpBlack;
		iPix = std::max(iPix, 0);
		iPix = (iPix * *(unsigned short*)((unsigned char*)lpWhite + 0 * lShadSync)) >> 12;
		iPix = std::min(iPix, 0xff);
		*lpDst++ = (unsigned char) iPix;

		// GREEN
		iPix = (int)lpSrc[1 * lSync];
		iPix -= *lpBlack;
		iPix = std::max(iPix, 0);
		iPix = (iPix * *(unsigned short*)((unsigned char*)lpWhite + 1 * lShadSync)) >> 12;
		iPix = std::min(iPix, 0xff);
		*lpDst++ = (unsigned char) iPix;

		// BLUE
		iPix = (int)lpSrc[2 * lSync];
		iPix -= *lpBlack;
		iPix = std::max(iPix, 0);
		iPix = (iPix * *(unsigned short*)((unsigned char*)lpWhite + 2 * lShadSync)) >> 12;
		iPix = std::min(iPix, 0xff);
		*lpDst++ = (unsigned char) iPix;

		++lpWhite;
		++lpBlack;
		++lpSrc;
	}

	lpDst += w * 3;
	lpSrc += w;
	lpWhite += w;
	lpBlack += w;
	w = 0;
}

void CShading::ShadingGrayCore_SIMD(unsigned char* lpDst, unsigned char* lpSrc, unsigned short* lpWhite, unsigned short* lpBlack, long w)
{
#ifdef SSE2
	while (w > 7) {
		// ���͉摜8��f��ǂݍ���ŁAWORD�ɃA���p�b�N���ApixH, pixL�ɂ��ꂼ��4��f���U�蕪����
		__m64 pix = *(__m64 *)lpSrc;							// HGFEDCBA
		__m64 pixH = _mm_unpackhi_pi8(pix, _mm_setzero_si64());	// 0H0G0F0E
		__m64 pixL = _mm_unpacklo_pi8(pix, _mm_setzero_si64());	// 0D0C0B0A

		// ���␳
		__m64 b;
		b = *(__m64 *)(lpBlack + 0);				// ���f�[�^�ǂݍ��݁i4��f�j
		pixL = _mm_subs_pu16(pixL, b);			// 4���[�h�̖O�a�����Z�i0������0�ɒ���t���j
		b = *(__m64 *)(lpBlack + 4);				// ���f�[�^�ǂݍ��݁i4��f�j
		pixH = _mm_subs_pu16(pixH, b);			// 4���[�h�̖O�a�����Z�i0������0�ɒ���t���j

		
		__m64 wh, tmp;
		wh = *(__m64 *)(lpWhite + 0);				// ���f�[�^�ǂݍ��݁i4��f)
		tmp = _mm_slli_pi16(pixL, 4);			// x16 ���V�t�g4
		pixL = _mm_mulhi_pu16(tmp, wh);			// *white / 65536
		wh = *(__m64 *)(lpWhite + 4);				// ���f�[�^�ǂݍ��݁i4��f)
		tmp = _mm_slli_pi16(pixH, 4);			// x16 ���V�t�g4
		pixH = _mm_mulhi_pu16(tmp, wh);			// *white / 65536

		pix = _mm_packs_pu16(pixL, pixH);		// 4���[�h�~2��8�o�C�g�Ƀp�b�N����B256�ȏ��255�ɒ���t��
		*(__m64 *)lpDst = pix;

		lpDst += 8;
		w -= 8;
		lpWhite += 8;
		lpBlack += 8;
		lpSrc += 8;
	}
	_mm_empty();

	
	lpDst += w & ~0x7;
	lpSrc += w & ~0x7;
	lpWhite += w & ~0x7;
	lpBlack += w & ~0x7;
	w &= 0x7;
	ShadingGrayCore_NonSIMD(lpDst, lpSrc, lpWhite, lpBlack, w);
#else //SSE2
	assert(false);
#endif //SSE2
}

void CShading::ShadingGrayCore_NEON(unsigned char* lpDst, unsigned char* lpSrc, unsigned short* lpWhite, unsigned short* lpBlack, long w)
{
#ifdef __ARM_NEON__
	uint16x8_t pix;
	while (w > 7) {
		pix = vmovl_u8(vld1_u8((uint8_t *)lpSrc));
		
		pix = vqsubq_u16(pix, vld1q_u16((uint16_t *)lpBlack));
		
		pix = vqshlq_n_u16(pix, 3);

		pix = (uint16x8_t)vqdmulhq_s16((int16x8_t)pix, vld1q_s16((int16_t *)lpWhite));
		
		// 1byte pack & output
		vst1_u8((uint8_t *)lpDst, vqmovn_u16(pix));

		lpDst += 8;
		w -= 8;
		lpWhite += 8;
		lpBlack += 8;
		lpSrc += 8;
	}

	
#else //__ARM_NEON__
#endif //__ARM_NEON__
	ShadingGrayCore_NonSIMD(lpDst, lpSrc, lpWhite, lpBlack, w);
}

void CShading::ShadingGrayCore_NonSIMD(unsigned char* lpDst, unsigned char* lpSrc, unsigned short* lpWhite, unsigned short* lpBlack, long w)
{
	while (w--) {
		int iPix = (int) *lpSrc++;
		iPix -= *lpBlack++;
		iPix = std::max(iPix, 0);
		iPix = (iPix * *lpWhite++) >> 12;
		iPix = std::min(iPix, 0xff);
		*lpDst++ = (unsigned char)iPix;
	}
	lpDst += w;
	lpSrc += w;
	lpWhite += w;
	lpBlack += w;
	w = 0;
}

void CShading::mulImage(CImg& img, long mul)
{
	if (img.getBps() == 16) {
		short* p = (short*)img.getImagePtr();
		long lLen = img.getImageSize() / sizeof(unsigned short);
		while (lLen--) {
			*p = (short)std::min((long)*p * mul, (long)0xffff);
			++p;
		}
	}
	else {
	}
}

RTN CShading::pack8OnUpperByteImage(CImg& img)
{
	if (img.getBps() == 16) {
		long lLen = img.getHeight();
		while (lLen--) {
			unsigned char* pDst = img.getImagePtr() + lLen * img.getSync();
			unsigned char* pSrc = pDst + 1;
			long w = img.getWidth();
			while (w--) {
				*pDst++ = *pSrc;
				pSrc += 2;
			}
		}

		IMAGEINFO Info = *((IMAGEINFO*)img);
		Info.lBps = 8;
		Info.lpImage = 0;
		Info.lSync = PACKING8(img.getWidth() * img.getBps() * img.getSpp()) / 8;
		Info.tImageSize = Info.lSync * Info.lHeight;

		long lOldSync = img.getSync();
		long lNewSync = Info.lSync;
		if (lOldSync == lNewSync) {
			return RTN_OK;
		}

		CImg imgDst;
		if (!imgDst.createImg(Info)) {
			return RTN_PAR;
		}
		if (imgDst.isNull()) {
			return RTN_NOMEM;
		}

		unsigned char* pDst = imgDst.getImagePtr();
		unsigned char* pSrc = img.getImagePtr();
		lLen = img.getHeight();
		while (lLen--) {
			memcpy(pDst, pSrc, lNewSync);
			pDst += lNewSync;
			pSrc += lOldSync;
		}
		img.attachImg(imgDst);
	}
	return RTN_OK;
}

RTN CShading::makeShadingData(CImg& imgWhite, CImg& imgBlack, CImg& imgWhiteOrg, CImg& imgBlackOrg, int nSensorVer)
{
	assert(imgWhite.getBps() == 16);
	assert(imgBlack.getBps() == 16);

	
	unsigned short* pw = (unsigned short*)imgWhite.getImagePtr();
	unsigned short* pb = (unsigned short*)imgBlack.getImagePtr();
	if (pw && pb) {
		assert(imgWhite.getImageSize() == imgBlack.getImageSize());
		long lSize = imgWhite.getImageSize() / sizeof(unsigned short);
		while (lSize--) {
			if (*pw > *pb) {
				*pw++ -= *pb++;
			}
			else {
				*pw++ = 0;
				pb++;
			}
		}
	}

    const int target[][4] = SHADING_TARGET;
    int targetGray = target[nSensorVer][0];
    int targetRed = target[nSensorVer][1];
    int targetGreen = target[nSensorVer][2];
    int targetBlue = target[nSensorVer][3];

    if (imgWhite.getSpp() == 3) {
		if (imgWhite.getRGBOrder() == LINE_ORDER) {
			unsigned char*  p = imgWhite.getImagePtr();
			long lWidth = imgWhite.getWidth();
			makeWhiteDataLine((unsigned short*)p, lWidth, targetRed);
			p += imgWhite.getSync();
			makeWhiteDataLine((unsigned short*)p, lWidth, targetGreen);
			p += imgWhite.getSync();
			makeWhiteDataLine((unsigned short*)p, lWidth, targetBlue);
		}
		else {
			unsigned char* p = imgWhite.getImagePtr();
			long lWidth = imgWhite.getWidth();
			makeWhiteDataLineColor((unsigned short*)p, lWidth, targetRed, targetGreen, targetBlue);
		}
	}
	else {
		unsigned char* p = imgWhite.getImagePtr();
		long lWidth = imgWhite.getWidth();
		makeWhiteDataLine((unsigned short*)p, lWidth, targetGray);
	}

	int div = 16;
	{
		if (div != 0) {
			if (imgBlack.getBps() == 16) {
				unsigned short* p = (unsigned short*)imgBlack.getImagePtr();
				long len = imgBlack.getImageSize() / sizeof(unsigned short);
				while (len--) {
					*p /= div;
					++p;
				}
			}
			else {
			}
		}
	}

    
    RTN result = RTN_OK;
    
    SHADING_AVARAGE avgShdWhite = {0};
	result = makeShadingAvarage16(imgWhite, avgShdWhite);
	if (result != RTN_OK) {
		return result;
	}
    SHADING_AVARAGE avgShdBlack = {0};
	result = makeShadingAvarage16(imgBlack, avgShdBlack);
	if (result != RTN_OK) {
		return result;
	}
    SHADING_AVARAGE avgShdWhiteOrg = {0};
	result = makeShadingAvarage16(imgWhiteOrg, avgShdWhiteOrg);
	if (result != RTN_OK) {
		return result;
	}
	m_avgPlaten = calcPlatenColor(avgShdWhite, avgShdBlack, avgShdWhiteOrg);

    if (RATE_FOR_SHADING_NEWDT > 0)
    {
        m_avgPlaten.wGray = std::min(255L, (long)m_avgPlaten.wGray * (long)RATE_FOR_SHADING_NEWDT / 1000L);
        m_avgPlaten.wRed = std::min(255L, (long)m_avgPlaten.wRed * (long)RATE_FOR_SHADING_NEWDT / 1000L);
        m_avgPlaten.wGreen = std::min(255L, (long)m_avgPlaten.wGreen * (long)RATE_FOR_SHADING_NEWDT / 1000L);
        m_avgPlaten.wBlue = std::min(255L, (long)m_avgPlaten.wBlue * (long)RATE_FOR_SHADING_NEWDT / 1000L);
    }
    
    return RTN_OK;
}

RTN CShading::makeShadingData(CImg& imgWhiteOrg)
{
	RTN result = RTN_OK;
    
    if (imgWhiteOrg.getBps() == 8)
    {
        result = makeShadingAvarage8(imgWhiteOrg, m_avgPlaten);
    }
    else
    {
        result = makeShadingAvarage16(imgWhiteOrg, m_avgPlaten);
    }
    
    return result;
}

void CShading::formatShadingData9(CImg &imgWhite, CImg &imgBlack, CImg& imgWhiteOrg)
{
   {
        LPWORD p = (LPWORD)imgWhite.getImagePtr();
        int count = imgWhite.getWidth() * imgWhite.getSpp();
        
        for (int i=0; i<count; i++)
        {
            WORD w = p[i];
            w = w & 0x3fff;
            w = w >> 5;
            WORD b;
            LPBYTE lpw = (LPBYTE)&w;
            LPBYTE lpb = (LPBYTE)&b;
            lpb[0] = lpw[1];
            lpb[1] = lpw[0];
            
            p[i] = b;
        }
    }
    {
        LPWORD p = (LPWORD)imgBlack.getImagePtr();
        int count = imgBlack.getWidth() * imgBlack.getSpp();
        
        for (int i=0; i<count; i++)
        {
            WORD w = p[i];
            w = w << 2;
            WORD b;
            LPBYTE lpw = (LPBYTE)&w;
            LPBYTE lpb = (LPBYTE)&b;
            lpb[0] = lpw[1];
            lpb[1] = lpw[0];
            
            p[i] = b;
        }
    }
}

RTN CShading::fixPlatenImage(CImg& image)
{
	RECT ignore = {0, 0, 0, 0};
	return fixPlatenImage(image, ignore);
}

RTN CShading::fixPlatenImage(CImg& image, RECT ignore)
{
	if (image.isNull()) {
		return RTN_PAR;
	}
	if ((image.getSpp() == 3) != m_avgPlaten.isColor) {
		return RTN_PAR;
	}

	if (image.getRGBOrder() == LINE_ORDER) {
		if (!image.createImg(image.getWidth(), image.getHeight(), image.getBps(), image.getSpp(), PIXEL_ORDER, image.getXResolution(), image.getYResolution())) {
			return RTN_PAR;
		}
		if (image.isNull()) {
			return RTN_NOMEM;
		}
	}

    RGBQUAD fill = {static_cast<BYTE>(m_avgPlaten.wBlue), static_cast<BYTE>(m_avgPlaten.wGreen), static_cast<BYTE>(m_avgPlaten.wRed), static_cast<BYTE>(m_avgPlaten.wGray)};
    CImgEdit::FillColor(image, fill, ignore);
    
    return RTN_OK;
}


RTN CShading::makeShadingAvarage16(CImg& shadingdata, SHADING_AVARAGE& target)
{
	if (shadingdata.getBps() != 16) {
		return RTN_PAR;
	}
	if (shadingdata.getHeight() != 1) {
		return RTN_PAR;
	}

	LPBYTE lpData = shadingdata.getImagePtr();

	LONG width = shadingdata.getWidth();

	if (width == 0) {
		return RTN_PAR;	
	}

	if (shadingdata.getSpp() == 3 && shadingdata.getRGBOrder() == LINE_ORDER)
	{
        DWORD dwSum = 0;
        
		LPWORD lpRed = (LPWORD)lpData;
		LPWORD lpGreen = (LPWORD)(lpData + shadingdata.getSync()*1);
		LPWORD lpBlue = (LPWORD)(lpData + shadingdata.getSync()*2);

		{
			dwSum = 0;

			//�Ԃ̃��C���I�[�_
			for (LONG l=0; l<width; l++)
			{
				dwSum += *lpRed++;
			}
		}

		target.wRed = (WORD)((double)dwSum / (double)width);

		{
			dwSum = 0;

			//�΂̃��C���I�[�_
			for (LONG l=0; l<width; l++)
			{
				dwSum += *lpGreen++;
			}
		}

		target.wGreen = (WORD)((double)dwSum / (double)width);

		{
			dwSum = 0;

			//�̃��C���I�[�_
			for (LONG l=0; l<width; l++)
			{
				dwSum += *lpBlue++;
			}
		}

		target.wBlue = (WORD)((double)dwSum / (double)width);
	}
    else if (shadingdata.getSpp() == 3 && shadingdata.getRGBOrder() == PIXEL_ORDER)
	{
        DWORD dwSumR = 0;
        DWORD dwSumG = 0;
        DWORD dwSumB = 0;

        LPWORD lpSrcW = (LPWORD)lpData;
        for (LONG l=0; l<width; l++)
        {
            dwSumR += *lpSrcW++;
            dwSumG += *lpSrcW++;
            dwSumB += *lpSrcW++;
        }

		target.wRed = (WORD)((double)dwSumR / (double)width);
		target.wGreen = (WORD)((double)dwSumG / (double)width);
		target.wBlue = (WORD)((double)dwSumB / (double)width);
    }
	else if (shadingdata.getSpp() == 1)
	{
        DWORD dwSum = 0;
        
		LPWORD lpGray = (LPWORD)lpData;
		{
			dwSum = 0;

			for (LONG l=0; l<width; l++)
			{
				dwSum += *lpGray++;
			}
		}

		target.wGray = (WORD)((double)dwSum / (double)width);
	}
	else
	{
		return RTN_PAR;;
	}

	target.isColor = (shadingdata.getSpp() == 3);
	return RTN_OK;
}

RTN CShading::makeShadingAvarage8(CImg& shadingdata, SHADING_AVARAGE& target)
{
	if (shadingdata.getBps() != 8) {
		return RTN_PAR;
	}
	if (shadingdata.getHeight() != 1) {
		return RTN_PAR;
	}
    
	LPBYTE lpData = shadingdata.getImagePtr();
    
	//��
	LONG width = shadingdata.getWidth();
    
	if (width == 0) {
		return RTN_PAR;	//0����h�~
	}
    
	if (shadingdata.getSpp() == 3 && shadingdata.getRGBOrder() == LINE_ORDER)
	{
        DWORD dwSum = 0;
        
		LPBYTE lpRed = (LPBYTE)lpData;
		LPBYTE lpGreen = (LPBYTE)(lpData + shadingdata.getSync()*1);
		LPBYTE lpBlue = (LPBYTE)(lpData + shadingdata.getSync()*2);
        
		{
			dwSum = 0;
            
			//�Ԃ̃��C���I�[�_
			for (LONG l=0; l<width; l++)
			{
				dwSum += *lpRed++;
			}
		}
        
		target.wRed = std::min((WORD)255, (WORD)((double)dwSum / (double)width));
        
		{
			dwSum = 0;
            
			//�΂̃��C���I�[�_
			for (LONG l=0; l<width; l++)
			{
				dwSum += *lpGreen++;
			}
		}
        
		target.wGreen = std::min((WORD)255, (WORD)((double)dwSum / (double)width));
        
		{
			dwSum = 0;
            
			//�̃��C���I�[�_
			for (LONG l=0; l<width; l++)
			{
				dwSum += *lpBlue++;
			}
		}
        
		target.wBlue = std::min((WORD)255, (WORD)((double)dwSum / (double)width));
	}
    else if (shadingdata.getSpp() == 3 && shadingdata.getRGBOrder() == PIXEL_ORDER)
	{
        DWORD dwSumR = 0;
        DWORD dwSumG = 0;
        DWORD dwSumB = 0;
        
        LPBYTE lpSrcW = (LPBYTE)lpData;
        for (LONG l=0; l<width; l++)
        {
            dwSumR += *lpSrcW++;
            dwSumG += *lpSrcW++;
            dwSumB += *lpSrcW++;
        }
        
		target.wRed = std::min((WORD)255, (WORD)((double)dwSumR / (double)width));
		target.wGreen = std::min((WORD)255, (WORD)((double)dwSumG / (double)width));
		target.wBlue = std::min((WORD)255, (WORD)((double)dwSumB / (double)width));
    }
	else if (shadingdata.getSpp() == 1)
	{
        DWORD dwSum = 0;
        
		LPBYTE lpGray = (LPBYTE)lpData;
		{
			dwSum = 0;
            
			for (LONG l=0; l<width; l++)
			{
				dwSum += *lpGray++;
			}
		}
        
		target.wGray = std::min((WORD)255, (WORD)((double)dwSum / (double)width));
	}
	else
	{
		return RTN_PAR;;
	}
    
	target.isColor = (shadingdata.getSpp() == 3);
	return RTN_OK;
}

CShading::SHADING_AVARAGE CShading::calcPlatenColor(const SHADING_AVARAGE& avgWhite, const SHADING_AVARAGE& avgBlack, const SHADING_AVARAGE& avgWhiteOrg)
{
	assert(avgWhite.isColor == avgBlack.isColor);
	assert(avgBlack.isColor == avgWhiteOrg.isColor);
	assert(avgWhiteOrg.isColor == avgWhite.isColor);

	SHADING_AVARAGE result;
	result.isColor = avgWhite.isColor;

	if (result.isColor) {
		LONG lLevel;

		result.wRed	= (BYTE)(avgWhiteOrg.wRed >> 4);
		lLevel		= (LONG)((result.wRed - avgBlack.wRed) * avgWhite.wRed) / 4096;
		result.wRed	= (BYTE)std::min(lLevel, 255L);

		result.wGreen	= (BYTE)(avgWhiteOrg.wGreen >> 4);
		lLevel			= (LONG)((result.wGreen - avgBlack.wGreen) * avgWhite.wGreen) / 4096;
		result.wGreen	= (BYTE)std::min(lLevel, 255L);

		result.wBlue	= (BYTE)(avgWhiteOrg.wBlue >> 4);
		lLevel			= (LONG)((result.wBlue - avgBlack.wBlue) * avgWhite.wBlue) / 4096;
		result.wBlue	= (BYTE)std::min(lLevel, 255L);
	}
	else{
		result.wGray	= (BYTE)(avgWhiteOrg.wGray >> 4);
		LONG lLevel		= (LONG)((result.wGray - avgBlack.wGray) * avgWhite.wGray) / 4096;
		result.wGray	= (BYTE)std::min(lLevel, 255L);
	}
	return result;
}

void CShading::makeWhiteDataLine(unsigned short* p, long len, int tgt)
{
	if (len == 0 || p == 0) return;

	if (tgt == 0) {
		memset(p, 0, sizeof(unsigned short) * len);
	}
	else {
		while (len--) {
			if (!*p || (long)tgt >= 16 * (long)*p) {
				*p++ = 0xffff;
			}
			else {
				*p = (unsigned short) ((((long)tgt) << 12) / (long)*p);
				++p;
			}
		}
	}
}

void CShading::makeWhiteDataLineColor(unsigned short* p, long len, int tgtR, int tgtG, int tgtB)
{
	if (len == 0 || p == 0) return;

	while (len--) {
		
		// RED
		if (!*p || (long)tgtR >= 16 * (long)*p) {
			*p++ = 0xffff;
		}
		else {
			*p = (unsigned short) ((((long)tgtR) << 12) / (long)*p);
			++p;
		}
		// GREEN
		if (!*p || (long)tgtG >= 16 * (long)*p) {
			*p++ = 0xffff;
		}
		else {
			*p = (unsigned short) ((((long)tgtG) << 12) / (long)*p);
			++p;
		}
		// BLUE
		if (!*p || (long)tgtB >= 16 * (long)*p) {
			*p++ = 0xffff;
		}
		else {
			*p = (unsigned short) ((((long)tgtB) << 12) / (long)*p);
			++p;
		}
	}
}

