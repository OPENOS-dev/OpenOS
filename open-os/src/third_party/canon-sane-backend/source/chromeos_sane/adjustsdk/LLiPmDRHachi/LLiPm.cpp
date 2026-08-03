/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include "DRHachiFilter.h"
#include LLiPmDRHeader

namespace Cei {
	namespace LLiPm {
		namespace DR_NAMESPACE {
			static CSpecialFilter SpecialFilter;
		}
	}
}

long Cei::LLiPm::DR_NAMESPACE::getOffset(long lResolution)
{
	return Cei::LLiPm::DR_NAMESPACE::SpecialFilter.getOffset(lResolution);
}

void Cei::LLiPm::DR_NAMESPACE::setOffset(long lOffset)
{
	Cei::LLiPm::DR_NAMESPACE::SpecialFilter.setOffset(lOffset);
}

#if defined(CR50_Build) || defined(DRC240_Build)  || defined(DRM140_Build) || defined(DRM160_Build) || defined(DRM260_Build) || defined(DR6030C_Build) || defined(DRX10C_Build) || defined(DRM1060_Build)
Cei::LLiPm::RTN Cei::LLiPm::DR_NAMESPACE::FilterDuplex(Cei::LLiPm::CImg& imgDstFront, Cei::LLiPm::CImg& imgDstBack, Cei::LLiPm::CImg& imgSrcFront, Cei::LLiPm::CImg& imgSrcBack, Cei::LLiPm::DR_NAMESPACE::FILTERDUPLEXINFO* lpInfo)
{
	if (!lpInfo) {
		return RTN_PAR;
	}
	Cei::LLiPm::DR_NAMESPACE::SpecialFilter.setBackImage(imgSrcBack);
	Cei::LLiPm::RTN result = Cei::LLiPm::DR_NAMESPACE::SpecialFilter.IPInterface(imgSrcFront, lpInfo);
	if (result != RTN_OK) {
		return result;
	}
	imgDstFront.attachImg(imgSrcFront);
	Cei::LLiPm::DR_NAMESPACE::SpecialFilter.getBackImage(imgDstBack);
	return result;
}
#endif
#if defined(JPEG_IMPORT_SEPARATED)
Cei::LLiPm::RTN Cei::LLiPm::DR_NAMESPACE::FilterDuplex(Cei::LLiPm::CImg& imgDstFront, Cei::LLiPm::CImg& imgDstBack, Cei::LLiPm::CImg& imgSrcFront, Cei::LLiPm::CImg& imgSrcBack, Cei::LLiPm::DR_NAMESPACE::FILTERDUPLEXINFO* lpInfo)
{
	if (!lpInfo) {
		return RTN_PAR;
	}
	Cei::LLiPm::DR_NAMESPACE::SpecialFilter.setBackImage(imgSrcBack);
	Cei::LLiPm::RTN result = Cei::LLiPm::DR_NAMESPACE::SpecialFilter.IPInterface(imgSrcFront, lpInfo);
	if (result != RTN_OK) {
		return result;
	}
	imgDstFront.attachImg(imgSrcFront);
	Cei::LLiPm::DR_NAMESPACE::SpecialFilter.getBackImage(imgDstBack);
	return result;
}

Cei::LLiPm::RTN Cei::LLiPm::DR_NAMESPACE::FilterDuplexFirst(Cei::LLiPm::CImg& imgDstFront, Cei::LLiPm::CImg& imgDstBack, Cei::LLiPm::CImg& imgSrcFront, Cei::LLiPm::CImg& imgSrcBack, Cei::LLiPm::DR_NAMESPACE::FILTERDUPLEXINFO* lpInfo)
{
	if (!lpInfo) {
		return RTN_PAR;
	}
	Cei::LLiPm::DR_NAMESPACE::SpecialFilter.setBackImage(imgSrcBack);
	Cei::LLiPm::RTN result = Cei::LLiPm::DR_NAMESPACE::SpecialFilter.IPFirstInterface(imgSrcFront, lpInfo);
	if (result != RTN_OK) {
		return result;
	}
	imgDstFront.attachImg(imgSrcFront);
	Cei::LLiPm::DR_NAMESPACE::SpecialFilter.getBackImage(imgDstBack);
	return result;
}

Cei::LLiPm::RTN Cei::LLiPm::DR_NAMESPACE::FilterDuplexMiddle(Cei::LLiPm::CImg& imgDstFront, Cei::LLiPm::CImg& imgDstBack, Cei::LLiPm::CImg& imgSrcFront, Cei::LLiPm::CImg& imgSrcBack, Cei::LLiPm::DR_NAMESPACE::FILTERDUPLEXINFO* lpInfo)
{
	if (!lpInfo) {
		return RTN_PAR;
	}
	Cei::LLiPm::DR_NAMESPACE::SpecialFilter.setBackImage(imgSrcBack);
	Cei::LLiPm::RTN result = Cei::LLiPm::DR_NAMESPACE::SpecialFilter.IPMiddleInterface(imgSrcFront, lpInfo);
	if (result != RTN_OK) {
		return result;
	}
	imgDstFront.attachImg(imgSrcFront);
	Cei::LLiPm::DR_NAMESPACE::SpecialFilter.getBackImage(imgDstBack);
	return result;
}

Cei::LLiPm::RTN Cei::LLiPm::DR_NAMESPACE::FilterDuplexLast(Cei::LLiPm::CImg& imgDstFront, Cei::LLiPm::CImg& imgDstBack, Cei::LLiPm::CImg& imgSrcFront, Cei::LLiPm::CImg& imgSrcBack, Cei::LLiPm::DR_NAMESPACE::FILTERDUPLEXINFO* lpInfo)
{
	if (!lpInfo) {
		return RTN_PAR;
	}
	Cei::LLiPm::DR_NAMESPACE::SpecialFilter.setBackImage(imgSrcBack);
	Cei::LLiPm::RTN result = Cei::LLiPm::DR_NAMESPACE::SpecialFilter.IPLastInterface(imgSrcFront, lpInfo);
	if (result != RTN_OK) {
		return result;
	}
	imgDstFront.attachImg(imgSrcFront);
	Cei::LLiPm::DR_NAMESPACE::SpecialFilter.getBackImage(imgDstBack);
	return result;
}
#endif

void Cei::LLiPm::DR_NAMESPACE::AdjustLightFirst(Cei::LLiPm::DR_NAMESPACE::ADJUSTINFO* lpInfo, long lSensorId)
{
	Cei::LLiPm::DR_NAMESPACE::SpecialFilter.AdjustLightFirst(lpInfo, lSensorId);
}

Cei::LLiPm::RTN Cei::LLiPm::DR_NAMESPACE::AdjustLightNext(Cei::LLiPm::CImg& image, Cei::LLiPm::DR_NAMESPACE::ADJUSTINFO* lpInfo)
{
	return Cei::LLiPm::DR_NAMESPACE::SpecialFilter.AdjustLightNext(image, lpInfo);
}

Cei::LLiPm::RTN Cei::LLiPm::DR_NAMESPACE::AdjustLightLast(Cei::LLiPm::CImg& imgDstFront, Cei::LLiPm::CImg& imgDstBack, Cei::LLiPm::CImg& imgSrc, Cei::LLiPm::DR_NAMESPACE::ADJUSTINFO* lpInfo)
{
	return Cei::LLiPm::DR_NAMESPACE::SpecialFilter.AdjustLightLast(imgDstFront, imgDstBack, imgSrc, lpInfo);
}

Cei::LLiPm::RTN Cei::LLiPm::DR_NAMESPACE::AdjustLightFix(Cei::LLiPm::CImg& imgWhite, Cei::LLiPm::CImg& imgBlack, Cei::LLiPm::DR_NAMESPACE::ADJUSTINFO* lpInfo, Cei::LLiPm::SIDE Side, unsigned char* lpData, unsigned long ulSize)
{
	return AdjustLightFix(&Cei::LLiPm::DR_NAMESPACE::SpecialFilter, imgWhite, imgBlack, lpInfo, Side, lpData, ulSize);
}

#if defined(DRC225_Build) || defined(DRC240_Build)
void Cei::LLiPm::DR_NAMESPACE::Clear()
{
	return Cei::LLiPm::DR_NAMESPACE::SpecialFilter.clear();
}
#endif


void* Cei::LLiPm::DR_NAMESPACE::createLLiPm()
{
    try {
        return new CSpecialFilter;
    } catch (const std::exception&) {
        return NULL;
    }
}

void Cei::LLiPm::DR_NAMESPACE::deleteLLiPm(void* llipm)
{
    if (llipm != NULL) {
        delete (CSpecialFilter*)llipm;
    }
}

Cei::LLiPm::RTN Cei::LLiPm::DR_NAMESPACE::FilterSimplex(void* llipm, CImg& imgDst, CImg& imgSrc, FILTERSIMPLEXINFO* lpInfo)
{
    CSpecialFilter* filter = (CSpecialFilter*)llipm;
	if (!lpInfo) {
		return RTN_PAR;
	}
	Cei::LLiPm::RTN result = filter->IPInterface(imgSrc, lpInfo);
	if (result != RTN_OK) {
		return result;
	}
	imgDst.attachImg(imgSrc);
	return result;
}

Cei::LLiPm::RTN Cei::LLiPm::DR_NAMESPACE::FilterSimplexFirst(void* llipm, CImg& imgDst, CImg& imgSrc, FILTERSIMPLEXINFO* lpInfo)
{
    CSpecialFilter* filter = (CSpecialFilter*)llipm;
	if (!lpInfo) {
		return RTN_PAR;
	}
	Cei::LLiPm::RTN result = filter->IPFirstInterface(imgSrc, lpInfo);
	if (result != RTN_OK) {
		return result;
	}
	imgDst.attachImg(imgSrc);
	return result;
}

Cei::LLiPm::RTN Cei::LLiPm::DR_NAMESPACE::FilterSimplexMiddle(void* llipm, CImg& imgDst, CImg& imgSrc, FILTERSIMPLEXINFO* lpInfo)
{
    CSpecialFilter* filter = (CSpecialFilter*)llipm;
	if (!lpInfo) {
		return RTN_PAR;
	}
	Cei::LLiPm::RTN result = filter->IPMiddleInterface(imgSrc, lpInfo);
	if (result != RTN_OK) {
		return result;
	}
	imgDst.attachImg(imgSrc);
	return result;
}

Cei::LLiPm::RTN Cei::LLiPm::DR_NAMESPACE::FilterSimplexLast(void* llipm, CImg& imgDst, CImg& imgSrc, FILTERSIMPLEXINFO* lpInfo)
{
    CSpecialFilter* filter = (CSpecialFilter*)llipm;
	if (!lpInfo) {
		return RTN_PAR;
	}
	Cei::LLiPm::RTN result = filter->IPLastInterface(imgSrc, lpInfo);
	if (result != RTN_OK) {
		return result;
	}
	imgDst.attachImg(imgSrc);
	return result;
}

Cei::LLiPm::RTN Cei::LLiPm::DR_NAMESPACE::FilterDuplex(void* llipm, CImg& imgDstFront, CImg& imgDstBack, CImg& imgSrc, FILTERDUPLEXINFO* lpInfo)
{
    CSpecialFilter* filter = (CSpecialFilter*)llipm;
	if (!lpInfo) {
		return RTN_PAR;
	}
    CImg imgBlank;
	filter->setBackImage(imgBlank);
	Cei::LLiPm::RTN result = filter->IPInterface(imgSrc, lpInfo);
	if (result != RTN_OK) {
		return result;
	}
	imgDstFront.attachImg(imgSrc);
	filter->getBackImage(imgDstBack);
	return result;
}

Cei::LLiPm::RTN Cei::LLiPm::DR_NAMESPACE::FilterDuplexFirst(void* llipm, CImg& imgDstFront, CImg& imgDstBack, CImg& imgSrc, FILTERDUPLEXINFO* lpInfo)
{
    CSpecialFilter* filter = (CSpecialFilter*)llipm;
	if (!lpInfo) {
		return RTN_PAR;
	}
    CImg imgBlank;
	filter->setBackImage(imgBlank);
	Cei::LLiPm::RTN result = filter->IPFirstInterface(imgSrc, lpInfo);
	if (result != RTN_OK) {
		return result;
	}
	imgDstFront.attachImg(imgSrc);
	filter->getBackImage(imgDstBack);
	return result;
}

Cei::LLiPm::RTN Cei::LLiPm::DR_NAMESPACE::FilterDuplexMiddle(void* llipm, CImg& imgDstFront, CImg& imgDstBack, CImg& imgSrc, FILTERDUPLEXINFO* lpInfo)
{
    CSpecialFilter* filter = (CSpecialFilter*)llipm;
	if (!lpInfo) {
		return RTN_PAR;
	}
    CImg imgBlank;
	filter->setBackImage(imgBlank);
	Cei::LLiPm::RTN result = filter->IPMiddleInterface(imgSrc, lpInfo);
	if (result != RTN_OK) {
		return result;
	}
	imgDstFront.attachImg(imgSrc);
	filter->getBackImage(imgDstBack);
	return result;
}

Cei::LLiPm::RTN Cei::LLiPm::DR_NAMESPACE::FilterDuplexLast(void* llipm, CImg& imgDstFront, CImg& imgDstBack, CImg& imgSrc, FILTERDUPLEXINFO* lpInfo)
{
    CSpecialFilter* filter = (CSpecialFilter*)llipm;
	if (!lpInfo) {
		return RTN_PAR;
	}
    CImg imgBlank;
	filter->setBackImage(imgBlank);
	Cei::LLiPm::RTN result = filter->IPLastInterface(imgSrc, lpInfo);
	if (result != RTN_OK) {
		return result;
	}
	imgDstFront.attachImg(imgSrc);
	filter->getBackImage(imgDstBack);
	return result;
}

Cei::LLiPm::RTN Cei::LLiPm::DR_NAMESPACE::AdjustLightFix(void* llipm, Cei::LLiPm::CImg& imgWhite, Cei::LLiPm::CImg& imgBlack, Cei::LLiPm::DR_NAMESPACE::ADJUSTINFO* lpInfo, Cei::LLiPm::SIDE Side, unsigned char* lpData, unsigned long ulSize)
{
    CSpecialFilter* filter = (CSpecialFilter*)llipm;
	return filter->makeShadingData(imgWhite, imgBlack, lpInfo, Side, lpData, ulSize);
}
