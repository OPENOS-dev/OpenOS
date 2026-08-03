/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include "NormalFilter.h"
#include "ceilib.h"
#include "CeiLogger.h"
#include <memory.h>
#include <assert.h>

using namespace Cei;
using namespace LLiPm;

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)


CNormalFilter::CNormalFilter(void) : m_nSensorVer(0)
{
}

CNormalFilter::~CNormalFilter(void)
{
}

void CNormalFilter::clear()
{
    
}

RTN CNormalFilter::setInfo(CImg& image, void* lpInfo)
{
	if (lpInfo == 0) {
		return RTN_PAR;
	}
	memset(&m_Info, 0, sizeof(m_Info));
	NORMALFILTERSIMPLEXINFO* pNormalFilterSimplexInfo = (NORMALFILTERSIMPLEXINFO*)lpInfo;
	NORMALFILTERDUPLEXINFO* pNormalFilterDuplexInfo = (NORMALFILTERDUPLEXINFO*)lpInfo;

	if (pNormalFilterSimplexInfo->ulSize == sizeof(NORMALFILTERSIMPLEXINFO)) {
		unsigned long size = pNormalFilterSimplexInfo->infoNormal.ulSize;
		m_Info.ulSize = sizeof(NORMALFILTERSIMPLEXINFO);
		m_Info.infoInputImg = pNormalFilterSimplexInfo->infoInputImg;
		m_Info.infoOutputImg = pNormalFilterSimplexInfo->infoOutputImg;
		memset(&m_Info.infoNormal[FRONT], 0, sizeof(NORMALFILTERINFO));
		memcpy(&m_Info.infoNormal[FRONT], &pNormalFilterSimplexInfo->infoNormal, size);
		
		return cehckInfoSEQError(FRONT);
	}
	else if (pNormalFilterDuplexInfo->ulSize == sizeof(NORMALFILTERDUPLEXINFO)) {
		m_Info.ulSize = sizeof(NORMALFILTERDUPLEXINFO);
		m_Info.infoInputImg = pNormalFilterDuplexInfo->infoInputImg;
		m_Info.infoOutputImg = pNormalFilterDuplexInfo->infoOutputImg;
		memset(&m_Info.infoNormal[FRONT], 0, sizeof(NORMALFILTERDUPLEXINFO));
		memcpy(&m_Info.infoNormal[FRONT], &pNormalFilterDuplexInfo->infoNormal[FRONT], pNormalFilterDuplexInfo->infoNormal[FRONT].ulSize);
		memset(&m_Info.infoNormal[BACK], 0, sizeof(NORMALFILTERDUPLEXINFO));
		memcpy(&m_Info.infoNormal[BACK], &pNormalFilterDuplexInfo->infoNormal[BACK], pNormalFilterDuplexInfo->infoNormal[BACK].ulSize);
	
		RTN result = cehckInfoSEQError(FRONT);
		if (result != RTN_OK) {
			return result;
		}
		return cehckInfoSEQError(BACK);
	}
	else {
		return RTN_PAR;
	}
}

RTN CNormalFilter::cehckInfoSEQError(SIDE side)
{
	return RTN_OK;
}
RTN CNormalFilter::execIP(CIPController<CIPBase>* pIP, CImg& image, void* lpInfo, IMAGESTATE imagestate)
{
	RTN result;
	switch (imagestate) {
	case IMAGESTATE_COMPLETE:
		result = pIP->IPComplete(image, lpInfo);
		break;
	case IMAGESTATE_FIRST:
		result = pIP->IPFirst(image, lpInfo);
		break;
	case IMAGESTATE_MIDDLE:
		result = pIP->IPMiddle(image, lpInfo);
		break;
	case IMAGESTATE_LAST:
		result = pIP->IPLast(image, lpInfo);
		break;
	default:
		result = RTN_DEBUG;
		break;
	}

	CeiLogger::writeLog("%30s.IP[%d] return %d", pIP->getPtr()->getName(), imagestate, result);

	return result;
}