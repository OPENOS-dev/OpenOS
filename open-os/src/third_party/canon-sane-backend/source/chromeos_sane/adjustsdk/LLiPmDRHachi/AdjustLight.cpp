/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include "AdjustLight.h"
#include "AdjustLightCurve.h"
#include "ceilib.h"
#include <memory.h>
#include <assert.h>
#include <algorithm>
#include "DRHachiLogger.h"

using namespace Cei;
using namespace LLiPm;
using namespace DR_NAMESPACE;

namespace DecideTargetRegister_for_BunZGrayProc
{
	double GetRate(long reg_ref1, long reg_ref2, long light_ref1, long light_ref2, long light_dark/*offset*/, long lLightTagetRate, long lGainAdjTarget)
	{
		long x1 = reg_ref1;
		long x2 = reg_ref2;

		long y1 = light_ref1 - light_dark;
		long y2 = light_ref2 - light_dark;

		long y3 = (lGainAdjTarget * lLightTagetRate / 100 - light_dark) / 3;

		double b = 0.0;
		if ((x1 - x2) != 0)
		{
			b = (double)(((double)y2*(double)x1) - ((double)y1*(double)x2)) / (double)(x1 - x2);
		}

		double num = y3 - b;

		double denom = y2 - b;

		double dRet = 0.0;

		if (denom != 0.0)
		{
			dRet = num / denom;
		}

		return dRet;		
	}
}


#if defined(LIGHT_ADJUST_CHIEBUS_TYPE)
void DR_NAMESPACE::AdjustLightData_SetGain(ADJUSTINFO &info, bool front, unsigned char value)
{
	if (front)
	{
        info.FrontAdjustInfo.Gain1 = value;
    } else {
        info.BackAdjustInfo.Gain1 = value;
    }
}

unsigned char DR_NAMESPACE::AdjustLightData_GetGain(ADJUSTINFO &info, bool front)
{
	if (front) {
        return info.FrontAdjustInfo.Gain1;
    } else {
        return info.BackAdjustInfo.Gain1;
    }
}

void DR_NAMESPACE::AdjustLightData_SetOffset(ADJUSTINFO &info, bool front, unsigned char value)
{
	if (front) {
        info.FrontAdjustInfo.Offset1 = value;
    } else {
        info.BackAdjustInfo.Offset1 = value;
    }
}

unsigned char DR_NAMESPACE::AdjustLightData_GetOffset(ADJUSTINFO &info, bool front)
{
	if (front) {
        return info.FrontAdjustInfo.Offset1;
    } else {
        return info.BackAdjustInfo.Offset1;
    }
}

void DR_NAMESPACE::AdjustLightData_SetLEDCurrent(ADJUSTINFO& info, bool front, unsigned long value)
{
}

#elif defined(LIGHT_ADJUST_EAGLE_TYPE) || defined(LIGHT_ADJUST_DOCAN_TYPE) || defined(LIGHT_ADJUST_HACHI_TYPE) || defined(LIGHT_ADJUST_BOW_TYPE) || defined(LIGHT_ADJUST_NEWDT_TYPE) || defined(LIGHT_ADJUST_VOYAJER_TYPE) || defined(LIGHT_ADJUST_DRF120_TYPE)
void DR_NAMESPACE::AdjustLightData_SetGain(ADJUSTINFO &info, bool front, unsigned char value)
{
	if (front)
	{
        info.FrontAdjustInfo.Gain1 = value;
        info.FrontAdjustInfo.Gain2 = value;
        info.FrontAdjustInfo.Gain3 = value;
    } else {
        info.BackAdjustInfo.Gain1 = value;
        info.BackAdjustInfo.Gain2 = value;
        info.BackAdjustInfo.Gain3 = value;
    }
}

unsigned char DR_NAMESPACE::AdjustLightData_GetGain(ADJUSTINFO &info, bool front)
{
	if (front) {
        return info.FrontAdjustInfo.Gain1;
    } else {
        return info.BackAdjustInfo.Gain1;
    }
}

void DR_NAMESPACE::AdjustLightData_SetOffset(ADJUSTINFO &info, bool front, unsigned char value)
{
	 if (front) {
        info.FrontAdjustInfo.Offset1 = value;
        info.FrontAdjustInfo.Offset2 = value;
        info.FrontAdjustInfo.Offset3 = value;
    } else {
        info.BackAdjustInfo.Offset1 = value;
        info.BackAdjustInfo.Offset2 = value;
        info.BackAdjustInfo.Offset3 = value;
    }
}

unsigned char DR_NAMESPACE::AdjustLightData_GetOffset(ADJUSTINFO &info, bool front)
{
	if (front) {
        return info.FrontAdjustInfo.Offset1;
    } else {
        return info.BackAdjustInfo.Offset1;
    }
}

void DR_NAMESPACE::AdjustLightData_SetLEDCurrent(ADJUSTINFO& info, bool front, unsigned long value)
{
}

#elif defined(LIGHT_ADJUST_BUNZ_TYPE) || defined(LIGHT_ADJUST_VC_TYPE) || defined(LIGHT_ADJUST_MINERVA_TYPE) || defined(LIGHT_ADJUST_DASH_TYPE)
void DR_NAMESPACE::AdjustLightData_SetGain(ADJUSTINFO &info, bool front, unsigned char value)
{
    if (front) {
        info.FrontAdjustInfo.Gain1 = value;
    } else {
        info.BackAdjustInfo.Gain1 = value;
    }
}

unsigned char DR_NAMESPACE::AdjustLightData_GetGain(ADJUSTINFO &info, bool front)
{
    if (front) {
        return info.FrontAdjustInfo.Gain1;
    } else {
        return info.BackAdjustInfo.Gain1;
    }
}

void DR_NAMESPACE::AdjustLightData_SetOffset(ADJUSTINFO &info, bool front, unsigned char value)
{
	 if (front) {
        info.FrontAdjustInfo.Offset1 = value;
        info.FrontAdjustInfo.Offset2 = value;
        info.FrontAdjustInfo.Offset3 = value;
        info.FrontAdjustInfo.Reserved2 = value;
    } else {
        info.BackAdjustInfo.Offset1 = value;
        info.BackAdjustInfo.Offset2 = value;
        info.BackAdjustInfo.Offset3 = value;
        info.BackAdjustInfo.Reserved2 = value;
    }
}

unsigned char DR_NAMESPACE::AdjustLightData_GetOffset(ADJUSTINFO &info, bool front)
{
    if (front) {
        return info.FrontAdjustInfo.Offset1;
    } else {
        return info.BackAdjustInfo.Offset1;
    }
}

void DR_NAMESPACE::AdjustLightData_SetLEDCurrent(ADJUSTINFO& info, bool front, unsigned long value)
{
    if (front) {
        info.FrontAdjustInfo.Reserved3 = value;
    } else {
        info.BackAdjustInfo.Reserved3 = value;
    }
}
 
#elif defined(LIGHT_ADJUST_CAROL_TYPE)
void DR_NAMESPACE::AdjustLightData_SetGain(ADJUSTINFO &info, bool front, unsigned char value)
{
	
}

unsigned char DR_NAMESPACE::AdjustLightData_GetGain(ADJUSTINFO &info, bool front)
{
	
	return 0;
}

void DR_NAMESPACE::AdjustLightData_SetOffset(ADJUSTINFO &info, bool front, unsigned char value)
{
	if (front) {
        info.FrontAdjustInfo.Offset1 = value;
        info.FrontAdjustInfo.Offset2 = value;
        info.FrontAdjustInfo.Offset3 = value;
        info.FrontAdjustInfo.Offset4 = value;
        info.FrontAdjustInfo.Offset5 = value;
        info.FrontAdjustInfo.Offset6 = value;
	} else {
        info.BackAdjustInfo.Offset1 = value;
        info.BackAdjustInfo.Offset2 = value;
        info.BackAdjustInfo.Offset3 = value;
        info.BackAdjustInfo.Offset4 = value;
        info.BackAdjustInfo.Offset5 = value;
        info.BackAdjustInfo.Offset6 = value;
	}
}

unsigned char DR_NAMESPACE::AdjustLightData_GetOffset(ADJUSTINFO &info, bool front)
{
	 if (front) {
        return info.FrontAdjustInfo.Offset1;
    } else {
        return info.BackAdjustInfo.Offset1;
    }
}

void DR_NAMESPACE::AdjustLightData_SetLEDCurrent(ADJUSTINFO& info, bool front, unsigned long value)
{
    if (front) {
        info.FrontAdjustInfo.LEDCurrent = value;
    } else {
        info.BackAdjustInfo.LEDCurrent = value;
    }
}

#else
#error LIGHT_ADJUST_XXX_TYPE is undefined.
#endif

CAdjustLight::CAdjustLight(void) : m_Itr(0), m_lSensorVer(0)
{
}

CAdjustLight::~CAdjustLight(void)
{
}

void CAdjustLight::AdjustLightFirst(ADJUSTINFO* lpInfo, long lSensorId)
{
	m_Itr = 0;
    if (lSensorId == NEW_SENSOR_VERISON_ID) { m_lSensorVer = 1; }
	memset(&m_LightParams_Dark, 0, sizeof(m_LightParams_Dark));
	memset(&m_LightParams_Saturate, 0, sizeof(m_LightParams_Saturate));
	memset(&m_LightParams_ReferenceDark, 0, sizeof(m_LightParams_ReferenceDark));
	memset(&m_LightParams_Reference, 0, sizeof(m_LightParams_Reference));
	memset(&m_LightParams_Target, 0, sizeof(m_LightParams_Target));
	AdjustAnaproOffsetInit(lpInfo);
	lpInfo->bUse = false;
}

RTN CAdjustLight::AdjustLightNext(CImg& imgFront, CImg& imgBack, ADJUSTINFO* lpInfo)
{
	RTN Result;

	m_Itr++;
	switch (m_Itr) {
	case 1:
		Result = AdjustAnaproOffset(imgFront, imgBack, lpInfo);
		AdjustLight_GetSensorDarkLevelInit(lpInfo);
		lpInfo->bUse = false;
		break;
	case 2:
		Result = AdjustLight_GetSensorDarkLevel(imgFront, imgBack, lpInfo);
		AdjustLight_GetSensorSaturateLevelInit(lpInfo);
		lpInfo->bUse = false;
		break;
	case 3:
		Result = AdjustLight_GetSensorSaturateLevel(imgFront, imgBack, lpInfo);
		AdjustLight_GetLightDarkLevelInit(lpInfo);
		lpInfo->bUse = false;
		break;
	case 4:
		Result = AdjustLight_GetLightDarkLevel(imgFront, imgBack, lpInfo);
		AdjustLightInit(lpInfo);
		lpInfo->bUse = false;
		break;
	case 5:
		Result = AdjustLight(imgFront, imgBack, lpInfo);
		AdjustAnaproGainInit(lpInfo);
#if defined(LIGHT_ADJUST_CHIEBUS_TYPE) || defined(LIGHT_ADJUST_DOCAN_TYPE) || defined(LIGHT_ADJUST_HACHI_TYPE) || defined(LIGHT_ADJUST_BOW_TYPE) || defined(LIGHT_ADJUST_NEWDT_TYPE) || defined(LIGHT_ADJUST_VOYAJER_TYPE) || defined(LIGHT_ADJUST_DRF120_TYPE) || defined(LIGHT_ADJUST_BUNZ_TYPE) || defined(LIGHT_ADJUST_VC_TYPE) || defined(LIGHT_ADJUST_MINERVA_TYPE) || defined(LIGHT_ADJUST_DASH_TYPE)
		lpInfo->bUse = false;
#elif defined(LIGHT_ADJUST_EAGLE_TYPE) || defined(LIGHT_ADJUST_CAROL_TYPE)
		lpInfo->ScanInfo.MainWindowID = 0xf3;
		lpInfo->ScanInfo.SubWindowID = 0xf3;
		lpInfo->bUse = true;
#else
#error LIGHT_ADJUST_XXX_TYPE is undefined.
#endif
		break;
	case 6:
		Result = AdjustAnaproGain(imgFront,imgBack, lpInfo);
		lpInfo->ScanInfo.MainWindowID = 0xff;
		lpInfo->ScanInfo.SubWindowID = 0xff;
		lpInfo->bUse = false;
		break;
	case 7:
#if defined(LIGHT_ADJUST_CHIEBUS_TYPE) || defined(LIGHT_ADJUST_DOCAN_TYPE) || defined(LIGHT_ADJUST_HACHI_TYPE) || defined(LIGHT_ADJUST_BOW_TYPE) || defined(LIGHT_ADJUST_NEWDT_TYPE) || defined(LIGHT_ADJUST_VOYAJER_TYPE) || defined(LIGHT_ADJUST_DRF120_TYPE)
		Result = AdjustAnaproOffset(imgFront, imgBack, lpInfo);
		AdjustDecideData(lpInfo);
#elif defined(LIGHT_ADJUST_EAGLE_TYPE) || defined(LIGHT_ADJUST_BUNZ_TYPE) || defined(LIGHT_ADJUST_VC_TYPE) || defined(LIGHT_ADJUST_CAROL_TYPE) || defined(LIGHT_ADJUST_VC_TYPE) || defined(LIGHT_ADJUST_MINERVA_TYPE) || defined(LIGHT_ADJUST_DASH_TYPE)
		Result = AdjustAnaproOffset(imgFront, imgBack, lpInfo);
		AdjustDecideData(lpInfo);
		lpInfo->ScanInfo.MainWindowID = 0xf3;
		lpInfo->ScanInfo.SubWindowID = 0xf3;
#else
#error LIGHT_ADJUST_XXX_TYPE is undefined.
#endif
        lpInfo->bUse = true;
		break;
	default:
		Result = RTN_DEBUG;
		break;
	}
	return Result;
}

RTN CAdjustLight::AdjustLightCurve(CImg& White, CImg& Black, ADJUSTINFO* lpInfo, SIDE Side, unsigned char* lpData, unsigned long ulSize)
{
	CLightCurveAdjustData adjustData;
	RTN result = adjustData.LoadData(lpData, ulSize);
	if (result != RTN_OK) {
		return result;
	}
	int nDropout = 0, nEmphasis = 0;

	if (Side == FRONT) {
		switch (lpInfo->FrontLightSorce) {
		case ADJUSTINFO::DROPOUT_RED:
			nDropout = 1;
			break;
		case ADJUSTINFO::DROPOUT_GREEN:
			nDropout = 2;
			break;
		case ADJUSTINFO::DROPOUT_BLUE:
			nDropout = 3;
			break;
		case ADJUSTINFO::EMPHASIS_RED:
			nEmphasis = 1;
			break;
		case ADJUSTINFO::EMPHASIS_GREEN:
			nEmphasis = 2;
			break;
		case ADJUSTINFO::EMPHASIS_BLUE:
			nEmphasis = 3;
			break;
		case ADJUSTINFO::LIGHT_NORMAL:
		default:
			break;
		}
	}
	else {
		switch (lpInfo->BackLightSorce) {
		case ADJUSTINFO::DROPOUT_RED:
			nDropout = 1;
			break;
		case ADJUSTINFO::DROPOUT_GREEN:
			nDropout = 2;
			break;
		case ADJUSTINFO::DROPOUT_BLUE:
			nDropout = 3;
			break;
		case ADJUSTINFO::EMPHASIS_RED:
			nEmphasis = 1;
			break;
		case ADJUSTINFO::EMPHASIS_GREEN:
			nEmphasis = 2;
			break;
		case ADJUSTINFO::EMPHASIS_BLUE:
			nEmphasis = 3;
			break;
		case ADJUSTINFO::LIGHT_NORMAL:
		default:
			break;
		}
	}
	IMAGEINFO infoBlack = *(IMAGEINFO *)Black;
	IMAGEINFO infoWhite = *(IMAGEINFO*)White;
	return adjustData.AdjustData(infoBlack, infoWhite, Side, nDropout, nEmphasis);
}

void CAdjustLight::AdjustAnaproOffsetInit(ADJUSTINFO* lpInfo)
{
    memset(&lpInfo->FrontAdjustInfo, 0, sizeof(lpInfo->FrontAdjustInfo));
    AdjustLightData_SetGain(*lpInfo, true, LIGHT_ADJUST_INITIALIZE_GAIN);
    AdjustLightData_SetOffset(*lpInfo, true, LIGHT_ADJUST_INITIALIZE_OFFSET);
    
    memset(&lpInfo->BackAdjustInfo, 0, sizeof(lpInfo->BackAdjustInfo));
    AdjustLightData_SetGain(*lpInfo, false, LIGHT_ADJUST_INITIALIZE_GAIN);
    AdjustLightData_SetOffset(*lpInfo, false, LIGHT_ADJUST_INITIALIZE_OFFSET);

	lpInfo->ScanInfo.MainWindowID = 0xff;
	lpInfo->ScanInfo.SubWindowID = 0xff;
}

RTN CAdjustLight::AdjustAnaproOffset(CImg& imgFront, CImg& imgBack, ADJUSTINFO* lpInfo)
{
    {
        // Front
        int minData = GetMin(imgFront);
        int nGain = AdjustLightData_GetGain(*lpInfo, true);
        int nOffset = AdjustLightData_GetOffset(*lpInfo, true);
        
        int newOffset = GetOffsetRegister_Feeder(minData, nGain, nOffset, LIGHT_ADJUST_OFFSET_ADJ_TARGET);
        
        unsigned char ulOffset = std::max(std::min(LIGHT_ADJUST_OFFSET_MAX, newOffset), LIGHT_ADJUST_OFFSET_MIN);
        AdjustLightData_SetOffset(*lpInfo, true, ulOffset);
    }
	if (lpInfo->bDuplex) {
		// Back
		int minData = GetMin(imgBack);
        int nGain = AdjustLightData_GetGain(*lpInfo, false);
        int nOffset = AdjustLightData_GetOffset(*lpInfo, false);
        
		int newOffset = GetOffsetRegister_Feeder(minData, nGain, nOffset, LIGHT_ADJUST_OFFSET_ADJ_TARGET);
        
		unsigned char ulOffset = std::max(std::min(LIGHT_ADJUST_OFFSET_MAX, newOffset), LIGHT_ADJUST_OFFSET_MIN);
        AdjustLightData_SetOffset(*lpInfo, false, ulOffset);
	}
	return RTN_OK;
}

void CAdjustLight::AdjustLight_GetSensorDarkLevelInit(ADJUSTINFO* lpInfo)
{
    
#if defined(LIGHT_ADJUST_EAGLE_TYPE) || defined(LIGHT_ADJUST_CHIEBUS_TYPE) || defined(LIGHT_ADJUST_DOCAN_TYPE) || defined(LIGHT_ADJUST_HACHI_TYPE) || defined(LIGHT_ADJUST_BOW_TYPE) || defined(LIGHT_ADJUST_NEWDT_TYPE) || defined(LIGHT_ADJUST_DRF120_TYPE) || defined(LIGHT_ADJUST_MINERVA_TYPE) || defined(LIGHT_ADJUST_DASH_TYPE)
    
#elif defined(LIGHT_ADJUST_BUNZ_TYPE) || defined(LIGHT_ADJUST_CAROL_TYPE)
    if (lpInfo->ScanMode != COLOR) {
        AdjustLightData_SetLEDCurrent(*lpInfo, true, 4);    
        if (lpInfo->bDuplex) {
            AdjustLightData_SetLEDCurrent(*lpInfo, false, 4);
        }
    }
#elif defined(LIGHT_ADJUST_VOYAJER_TYPE)
    if (lpInfo->ScanMode != COLOR)
    {
        long value;
        if (m_lSensorVer == 1) {
            value = 3;      
        }
        else {
            value = 4;      
        }
        
        AdjustLightData_SetLEDCurrent(*lpInfo, true, value);
        if (lpInfo->bDuplex) {
            AdjustLightData_SetLEDCurrent(*lpInfo, false, value);
        }
    }
    else
    {
        if (m_lSensorVer == 1) {
            long value = 6;  

            AdjustLightData_SetLEDCurrent(*lpInfo, true, value);
            if (lpInfo->bDuplex) {
                AdjustLightData_SetLEDCurrent(*lpInfo, false, value);
            }
        }
    }
#elif defined(LIGHT_ADJUST_VC_TYPE)
    if (lpInfo->ScanMode != COLOR)
    {
        long value;
        if (m_lSensorVer == 1) {
            value = 6;  
        }
        else {
            value = 4;  
        }

        AdjustLightData_SetLEDCurrent(*lpInfo, true, value);
        if (lpInfo->bDuplex) {
            AdjustLightData_SetLEDCurrent(*lpInfo, false, value);
        }
    }
#else
#error LIGHT_ADJUST_XXX_TYPE is undefined.
#endif

	AdjustLight_GetSensorLevelInit(lpInfo, LIGHT_ADJUST_LED_MIN, LIGHT_ADJUST_LED_MIN, LIGHT_ADJUST_LED_MIN);
	lpInfo->ScanInfo.MainWindowID = 0xfe;
	lpInfo->ScanInfo.SubWindowID = 0xfe;
}
RTN CAdjustLight::AdjustLight_GetSensorDarkLevel(CImg& imgFront, CImg& imgBack, ADJUSTINFO* lpInfo)
{
	if (imgFront.getSpp() != 3 || (lpInfo->bDuplex && imgBack.getSpp() != 3)) {	
		return RTN_PAR;
	}

	return AdjustLight_GetSensorLevel(m_LightParams_Dark, m_OneLineImageData_Dark, imgFront, imgBack, lpInfo);
}

void CAdjustLight::AdjustLight_GetSensorSaturateLevelInit(ADJUSTINFO* lpInfo)
{
	AdjustLight_GetSensorLevelInit(lpInfo, LIGHT_ADJUST_LED_MAX, LIGHT_ADJUST_LED_MAX, LIGHT_ADJUST_LED_MAX);
	lpInfo->ScanInfo.MainWindowID = 0xfe;
	lpInfo->ScanInfo.SubWindowID = 0xfe;
}
RTN CAdjustLight::AdjustLight_GetSensorSaturateLevel(CImg& imgFront, CImg& imgBack, ADJUSTINFO* lpInfo)
{
	if (imgFront.getSpp() != 3 || (lpInfo->bDuplex && imgBack.getSpp() != 3)) {	
		return RTN_PAR;
	}

	return AdjustLight_GetSensorLevel(m_LightParams_Saturate, m_OneLineImageData_Saturate, imgFront, imgBack, lpInfo);
}

void CAdjustLight::AdjustLight_GetLightDarkLevelInit(ADJUSTINFO* lpInfo)
{
	long lDarkRefPower = LIGHT_ADJUST_FEEDER_REFERENCE_DARK_POWER;
	AdjustLight_GetSensorLevelInit(lpInfo, lDarkRefPower, lDarkRefPower, lDarkRefPower);
	lpInfo->ScanInfo.MainWindowID = 0xfe;
	lpInfo->ScanInfo.SubWindowID = 0xfe;
}
RTN CAdjustLight::AdjustLight_GetLightDarkLevel(CImg& imgFront, CImg& imgBack, ADJUSTINFO* lpInfo)
{
	if (imgFront.getSpp() != 3 || (lpInfo->bDuplex && imgBack.getSpp() != 3)) {	
		return RTN_PAR;
	}

	return AdjustLight_GetSensorLevel(m_LightParams_ReferenceDark, m_OneLineImageData_ReferenceDark, imgFront, imgBack, lpInfo);
}

void CAdjustLight::AdjustLightInit(ADJUSTINFO* lpInfo)
{

	long inner_Reference_R = 0;
	long inner_Reference_G = 0;
	long inner_Reference_B = 0;
    
    switch (lpInfo->lXResolution) {
        case 300:
            inner_Reference_R = LIGHT_ADJUST_FEEDER_REFERENCE_POWER_RED_300;
            inner_Reference_G = LIGHT_ADJUST_FEEDER_REFERENCE_POWER_GREEN_300;
            inner_Reference_B = LIGHT_ADJUST_FEEDER_REFERENCE_POWER_BLUE_300;
            break;
        case 600:
            inner_Reference_R = LIGHT_ADJUST_FEEDER_REFERENCE_POWER_RED_600;
            inner_Reference_G = LIGHT_ADJUST_FEEDER_REFERENCE_POWER_GREEN_600;
            inner_Reference_B = LIGHT_ADJUST_FEEDER_REFERENCE_POWER_BLUE_600;
            break;
            
#if defined(LIGHT_ADJUST_EAGLE_TYPE) || defined(LIGHT_ADJUST_CHIEBUS_TYPE) || defined(LIGHT_ADJUST_DOCAN_TYPE) || defined(LIGHT_ADJUST_HACHI_TYPE) || defined(LIGHT_ADJUST_BOW_TYPE) || defined(LIGHT_ADJUST_NEWDT_TYPE) || defined(LIGHT_ADJUST_VOYAJER_TYPE) || defined(LIGHT_ADJUST_DRF120_TYPE) || defined(LIGHT_ADJUST_MINERVA_TYPE) || defined(LIGHT_ADJUST_DASH_TYPE)
#elif defined(LIGHT_ADJUST_BUNZ_TYPE) || defined(LIGHT_ADJUST_VC_TYPE) || defined(LIGHT_ADJUST_CAROL_TYPE)
        case 150:
            inner_Reference_R = LIGHT_ADJUST_FEEDER_REFERENCE_POWER_RED_150;
            inner_Reference_G = LIGHT_ADJUST_FEEDER_REFERENCE_POWER_GREEN_150;
            inner_Reference_B = LIGHT_ADJUST_FEEDER_REFERENCE_POWER_BLUE_150;
            break;
        case 200:
            inner_Reference_R = LIGHT_ADJUST_FEEDER_REFERENCE_POWER_RED_200;
            inner_Reference_G = LIGHT_ADJUST_FEEDER_REFERENCE_POWER_GREEN_200;
            inner_Reference_B = LIGHT_ADJUST_FEEDER_REFERENCE_POWER_BLUE_200;
            break;
        case 400:
            inner_Reference_R = LIGHT_ADJUST_FEEDER_REFERENCE_POWER_RED_400;
            inner_Reference_G = LIGHT_ADJUST_FEEDER_REFERENCE_POWER_GREEN_400;
            inner_Reference_B = LIGHT_ADJUST_FEEDER_REFERENCE_POWER_BLUE_400;
        break;
#else
#error LIGHT_ADJUST_XXX_TYPE is undefined.
#endif

#if defined(LIGHT_ADJUST_EAGLE_TYPE) || defined(LIGHT_ADJUST_CHIEBUS_TYPE) || defined(LIGHT_ADJUST_DOCAN_TYPE) || defined(LIGHT_ADJUST_HACHI_TYPE) || defined(LIGHT_ADJUST_BOW_TYPE) || defined(LIGHT_ADJUST_NEWDT_TYPE) || defined(LIGHT_ADJUST_VOYAJER_TYPE) || defined(LIGHT_ADJUST_DRF120_TYPE) || defined(LIGHT_ADJUST_BUNZ_TYPE) || defined(LIGHT_ADJUST_VC_TYPE) || defined(LIGHT_ADJUST_MINERVA_TYPE) || defined(LIGHT_ADJUST_DASH_TYPE)
#elif defined(LIGHT_ADJUST_CAROL_TYPE)
        case 100:
            inner_Reference_R = LIGHT_ADJUST_FEEDER_REFERENCE_POWER_RED_100;
            inner_Reference_G = LIGHT_ADJUST_FEEDER_REFERENCE_POWER_GREEN_100;
            inner_Reference_B = LIGHT_ADJUST_FEEDER_REFERENCE_POWER_BLUE_100;
            break;
        case 240:
            inner_Reference_R = LIGHT_ADJUST_FEEDER_REFERENCE_POWER_RED_240;
            inner_Reference_G = LIGHT_ADJUST_FEEDER_REFERENCE_POWER_GREEN_240;
            inner_Reference_B = LIGHT_ADJUST_FEEDER_REFERENCE_POWER_BLUE_240;
            break;
#else
#error LIGHT_ADJUST_XXX_TYPE is undefined.
#endif
    }
	lpInfo->FrontAdjustInfo.RedLED = (unsigned short)inner_Reference_R;
	lpInfo->FrontAdjustInfo.GreenLED = (unsigned short)inner_Reference_G;
	lpInfo->FrontAdjustInfo.BlueLED = (unsigned short)inner_Reference_B;
	lpInfo->BackAdjustInfo.RedLED = (unsigned short)inner_Reference_R;
	lpInfo->BackAdjustInfo.GreenLED = (unsigned short)inner_Reference_G;
	lpInfo->BackAdjustInfo.BlueLED = (unsigned short)inner_Reference_B;
	AdjustLight_GetSensorReferenceLevelInit(lpInfo, inner_Reference_R, inner_Reference_G, inner_Reference_B);

	lpInfo->ScanInfo.MainWindowID = 0xfe;
	lpInfo->ScanInfo.SubWindowID = 0xfe;
}
RTN CAdjustLight::AdjustLight(CImg& imgFront, CImg& imgBack, ADJUSTINFO* lpInfo)
{
	if (imgFront.getSpp() != 3 || (lpInfo->bDuplex && imgBack.getSpp() != 3)) {	
		return RTN_PAR;
	}

	RTN Result = AdjustLight_GetSensorReferenceLevel(imgFront, imgBack, lpInfo, -1);
	if (Result != RTN_OK) {
		return Result;
	}
	return AdjustLight_DecideLightAdjustValue(lpInfo);
}

void CAdjustLight::AdjustAnaproGainInit(ADJUSTINFO* lpInfo)
{
    AdjustLightData_SetLEDCurrent(*lpInfo, true, 0);
    AdjustLightData_SetLEDCurrent(*lpInfo, false, 0);
	lpInfo->ScanInfo.MainWindowID = 0xfe;
	lpInfo->ScanInfo.SubWindowID = 0xfe;
}

RTN CAdjustLight::AdjustAnaproGain(CImg& imgFront, CImg& imgBack, ADJUSTINFO* lpInfo)
{
#if defined(LIGHT_ADJUST_CHIEBUS_TYPE) || defined(LIGHT_ADJUST_DOCAN_TYPE) || defined(LIGHT_ADJUST_HACHI_TYPE) || defined(LIGHT_ADJUST_BOW_TYPE) || defined(LIGHT_ADJUST_NEWDT_TYPE) || defined(LIGHT_ADJUST_VOYAJER_TYPE) || defined(LIGHT_ADJUST_DRF120_TYPE) || defined(LIGHT_ADJUST_BUNZ_TYPE) || defined(LIGHT_ADJUST_VC_TYPE)
    int nGainAdjTarget = LIGHT_ADJUST_GAIN_ADJ_TARGET_LIST[m_lSensorVer];
	{
        //FRONT
        int maxData = GetMax(imgFront);
        int nGainRegister = GetGainRegister(maxData, lpInfo->FrontAdjustInfo.Gain1, nGainAdjTarget);	// Gain��1,2,3�œ����Ƃ����O��
        AdjustLightData_SetGain(*lpInfo, true, std::max(std::min(nGainRegister, LIGHT_ADJUST_GAIN_MAX), LIGHT_ADJUST_GAIN_MIN));
	}
	if (lpInfo->bDuplex) {
		//BACK
		int maxData = GetMax(imgBack);
		int nGainRegister = GetGainRegister(maxData, lpInfo->BackAdjustInfo.Gain1, nGainAdjTarget);	// Gain��1,2,3�œ����Ƃ����O��
        AdjustLightData_SetGain(*lpInfo, false, std::max(std::min(nGainRegister, LIGHT_ADJUST_GAIN_MAX), LIGHT_ADJUST_GAIN_MIN));
	}
	return RTN_OK;
#elif defined(LIGHT_ADJUST_EAGLE_TYPE) || defined(LIGHT_ADJUST_CAROL_TYPE) || defined(LIGHT_ADJUST_MINERVA_TYPE) || defined(LIGHT_ADJUST_DASH_TYPE)
	return RTN_OK;
#else
#error LIGHT_ADJUST_XXX_TYPE is undefined.
#endif
}
void CAdjustLight::AdjustLight_GetSensorReferenceLevelInit(ADJUSTINFO* lpInfo, long lPowerR, long lPowerG, long lPowerB)
{
	if (lPowerR < 0) {
        lPowerR = -1;
	}
	if (lPowerG < 0) {
        lPowerG = -1;
	}
	if (lPowerB < 0) {
        lPowerB = -1;
	}
	AdjustLight_GetSensorLevelInit(lpInfo, lPowerR, lPowerG, lPowerB);
}
RTN CAdjustLight::AdjustLight_GetSensorReferenceLevel(CImg& imgFront, CImg& imgBack, ADJUSTINFO* lpInfo, long lPower)
{
	return AdjustLight_GetSensorLevel(m_LightParams_Reference, m_OneLineImageData_Reference, imgFront, imgBack, lpInfo);
}

void CAdjustLight::AdjustLight_GetSensorLevelInit(ADJUSTINFO* lpInfo, long lLampPowerR, long lLampPowerG, long lLampPowerB)
{
	m_lLampPowerForGetSensorLevelR = lLampPowerR;
	m_lLampPowerForGetSensorLevelG = lLampPowerG;
	m_lLampPowerForGetSensorLevelB = lLampPowerB;

    lpInfo->FrontAdjustInfo.RedLED = (unsigned short)m_lLampPowerForGetSensorLevelR;
    lpInfo->FrontAdjustInfo.GreenLED = (unsigned short)m_lLampPowerForGetSensorLevelG;
    lpInfo->FrontAdjustInfo.BlueLED = (unsigned short)m_lLampPowerForGetSensorLevelB;
    lpInfo->BackAdjustInfo.RedLED = (unsigned short)m_lLampPowerForGetSensorLevelR;
    lpInfo->BackAdjustInfo.GreenLED = (unsigned short)m_lLampPowerForGetSensorLevelG;
    lpInfo->BackAdjustInfo.BlueLED = (unsigned short)m_lLampPowerForGetSensorLevelB;
}

RTN CAdjustLight::AdjustLight_GetSensorLevel(LIGHTRESPONSELEVEL* pLightParams, CImg* pOneLineImg, CImg& imgFront, CImg& imgBack, ADJUSTINFO* lpInfo)
{
	unsigned short wRMax, wGMax, wBMax, wRMin, wGMin, wBMin;
	{
		
		unsigned short wMax = GetMax(imgFront, wRMax, wGMax, wBMax);
		unsigned short wMin = GetMin(imgFront, wRMin, wGMin, wBMin);
		pLightParams[CAdjustLight::Front_Red].lLightRegister = m_lLampPowerForGetSensorLevelR;
		pLightParams[CAdjustLight::Front_Red].lLightLevelMax = wRMax;
		pLightParams[CAdjustLight::Front_Red].lLightLevelMin = wRMin;
		pLightParams[CAdjustLight::Front_Green].lLightRegister = m_lLampPowerForGetSensorLevelG;
		pLightParams[CAdjustLight::Front_Green].lLightLevelMax = wGMax;
		pLightParams[CAdjustLight::Front_Green].lLightLevelMin = wGMin;
		pLightParams[CAdjustLight::Front_Blue].lLightRegister = m_lLampPowerForGetSensorLevelB;
		pLightParams[CAdjustLight::Front_Blue].lLightLevelMax = wBMax;
		pLightParams[CAdjustLight::Front_Blue].lLightLevelMin = wBMin;
	}
	if (lpInfo->bDuplex) {
		unsigned short wMax = GetMax(imgBack, wRMax, wGMax, wBMax);
		unsigned short wMin = GetMin(imgBack, wRMin, wGMin, wBMin);
		pLightParams[CAdjustLight::Back_Red].lLightRegister = m_lLampPowerForGetSensorLevelR;
		pLightParams[CAdjustLight::Back_Red].lLightLevelMax = wRMax;
		pLightParams[CAdjustLight::Back_Red].lLightLevelMin = wRMin;
		pLightParams[CAdjustLight::Back_Green].lLightRegister = m_lLampPowerForGetSensorLevelG;
		pLightParams[CAdjustLight::Back_Green].lLightLevelMax = wGMax;
		pLightParams[CAdjustLight::Back_Green].lLightLevelMin = wGMin;
		pLightParams[CAdjustLight::Back_Blue].lLightRegister = m_lLampPowerForGetSensorLevelB;
		pLightParams[CAdjustLight::Back_Blue].lLightLevelMax = wBMax;
		pLightParams[CAdjustLight::Back_Blue].lLightLevelMin = wBMin;
	}
	
	{
		RTN rtn = ColorToRGB(	imgFront,
								pOneLineImg[CAdjustLight::Front_Red],
								pOneLineImg[CAdjustLight::Front_Green],
								pOneLineImg[CAdjustLight::Front_Blue]
								);
		if (rtn != RTN_OK) {
			return rtn;
		}
	}
	
	if (lpInfo->bDuplex) {
		RTN rtn = ColorToRGB(	imgBack,
								pOneLineImg[CAdjustLight::Back_Red],
								pOneLineImg[CAdjustLight::Back_Green],
								pOneLineImg[CAdjustLight::Back_Blue]
								);
		if (rtn != RTN_OK) {
			return rtn;
		}
	}
	return RTN_OK;
}

void CAdjustLight::AdjustDecideData(ADJUSTINFO* lpInfo)
{
	int nLightSorce[2] = {(int)lpInfo->FrontLightSorce, (int)lpInfo->BackLightSorce};
	unsigned short* pDstRed[2] = {&lpInfo->FrontAdjustInfo.RedLED, &lpInfo->BackAdjustInfo.RedLED};
	unsigned short* pDstGreen[2] = {&lpInfo->FrontAdjustInfo.GreenLED, &lpInfo->BackAdjustInfo.GreenLED};
	unsigned short* pDstBlue[2] = {&lpInfo->FrontAdjustInfo.BlueLED, &lpInfo->BackAdjustInfo.BlueLED};

	for (int i = 0; i < 2; i++)
	{
		bool bOffRed = false;
		bool bOffBlue = false;
		bool bOffGreen = false;

		switch (nLightSorce[i]) {
		case ADJUSTINFO::DROPOUT_RED:
			bOffBlue = true;
			bOffGreen = true;
			break;
		case ADJUSTINFO::DROPOUT_BLUE:
			bOffRed = true;
			bOffGreen = true;
			break;
		case ADJUSTINFO::DROPOUT_GREEN:
			bOffRed = true;
			bOffBlue = true;
			break;
		case ADJUSTINFO::EMPHASIS_RED:
			bOffRed = true;
			break;
		case ADJUSTINFO::EMPHASIS_BLUE:
			bOffBlue = true;
			break;
		case ADJUSTINFO::EMPHASIS_GREEN:
			bOffGreen = true;
			break;
		default:
			break;
		}

		if (bOffRed) {
			*(pDstRed[i]) = (unsigned short)0;
		}
		if (bOffGreen) {
			*(pDstGreen[i]) = (unsigned short)0;
		}
		if (bOffBlue) {
			*(pDstBlue[i]) = (unsigned short)0;
		}
	}
}

void CAdjustLight::CorrectRegist(ADJUSTINFO* lpInfo, long & num, long & denom, bool bFront)
{
	if (denom == 0) 
	{
		return;
	}

	if (bFront == true)
	{
		long lReg;

		lReg = m_LightParams_Target[CAdjustLight::Front_Red].lLightRegister;
		lReg = lReg * num / denom;
		m_LightParams_Target[CAdjustLight::Front_Red].lLightRegister = lReg;

		lReg = m_LightParams_Target[CAdjustLight::Front_Green].lLightRegister;
		lReg = lReg * num / denom;
		m_LightParams_Target[CAdjustLight::Front_Green].lLightRegister = lReg;

		lReg = m_LightParams_Target[CAdjustLight::Front_Blue].lLightRegister;
		lReg = lReg * num / denom;
		m_LightParams_Target[CAdjustLight::Front_Blue].lLightRegister = lReg;
	}
	else
	{
		long lReg;

		lReg = m_LightParams_Target[CAdjustLight::Back_Red].lLightRegister;
		lReg = lReg * num / denom;
		m_LightParams_Target[CAdjustLight::Back_Red].lLightRegister = lReg;

		lReg = m_LightParams_Target[CAdjustLight::Back_Green].lLightRegister;
		lReg = lReg * num / denom;
		m_LightParams_Target[CAdjustLight::Back_Green].lLightRegister = lReg;

		lReg = m_LightParams_Target[CAdjustLight::Back_Blue].lLightRegister;
		lReg = lReg * num / denom;
		m_LightParams_Target[CAdjustLight::Back_Blue].lLightRegister = lReg;
	}
}


RTN CAdjustLight::AdjustLight_DecideLightAdjustValue(ADJUSTINFO* lpInfo)
{
	int LightSorce[2] = {lpInfo->FrontLightSorce, lpInfo->BackLightSorce};

	
	for (int i=0; i<(lpInfo->bDuplex ? 2 : 1); i++)
	{
		if ((lpInfo->ScanMode != COLOR) && (LightSorce[i] == ADJUSTINFO::LIGHT_NORMAL)) {
			
			RTN rtn = DecideTargetRegister(lpInfo, i);
			if (rtn != RTN_OK) {
				return rtn;
			}
		}
		else {
			LONG lTargetValueRate = AdjustLight_DecideLightAdjustValue_GetTargetValueRate(lpInfo, i);

		
			LONG lTargetValue = AdjustLight_DecideLightAdjustValue_ConvertTargetRate2TargetValue(lpInfo, lTargetValueRate, i);

			lTargetValue = AdjustLight_DecideLightAdjustValue_DecideTargetLightLevel(lpInfo, lTargetValue, i);
			
			AdjustLight_DecideLightAdjustValue_DecideTargetRegister(lpInfo, lTargetValue, i);
		}

		RTN rtn = AdjustLight_DecideLightAdjustValue_CheckRegisterLimit(lpInfo, i);
		if (rtn != RTN_OK) {
			return rtn;
		}

		AdjustLight_DecideLightAdjustValue_Finish(lpInfo, i);
	}

	return RTN_OK;
}
LONG CAdjustLight::AdjustLight_DecideLightAdjustValue_GetTargetValueRate(ADJUSTINFO* lpInfo, int nIndex)
{
	int LightSorce[2] = {lpInfo->FrontLightSorce, lpInfo->BackLightSorce};

	long lTargetValueRate = LIGHT_ADJUST_LIGHT_TARGET_RATE;

	if (lpInfo->ScanMode == GRAY)
	{
		if ((LightSorce[nIndex] == ADJUSTINFO::DROPOUT_RED) || (LightSorce[nIndex] == ADJUSTINFO::DROPOUT_GREEN) || (LightSorce[nIndex] == ADJUSTINFO::DROPOUT_BLUE))
		{
			
		}
		else if ((LightSorce[nIndex] == ADJUSTINFO::EMPHASIS_RED) || (LightSorce[nIndex] == ADJUSTINFO::EMPHASIS_GREEN) || (LightSorce[nIndex] == ADJUSTINFO::EMPHASIS_BLUE))
		{
			
			lTargetValueRate = lTargetValueRate / 2;
		}
		else
		{
#if defined(LIGHT_ADJUST_EAGLE_TYPE) || defined(LIGHT_ADJUST_CHIEBUS_TYPE) || defined(LIGHT_ADJUST_DOCAN_TYPE) || defined(LIGHT_ADJUST_HACHI_TYPE) || defined(LIGHT_ADJUST_BOW_TYPE) || defined(LIGHT_ADJUST_NEWDT_TYPE) || defined(LIGHT_ADJUST_VOYAJER_TYPE) || defined(LIGHT_ADJUST_DRF120_TYPE) || defined(LIGHT_ADJUST_BUNZ_TYPE) || defined(LIGHT_ADJUST_VC_TYPE) || defined(LIGHT_ADJUST_MINERVA_TYPE) || defined(LIGHT_ADJUST_DASH_TYPE)
			
			lTargetValueRate = lTargetValueRate / 3;
#elif defined(LIGHT_ADJUST_CAROL_TYPE)
			
			lTargetValueRate = lTargetValueRate * 1 / 2;
#else
#error LIGHT_ADJUST_XXX_TYPE is undefined.
#endif
		}
	}
	else
	{
		
	}

	return lTargetValueRate;
}
LONG CAdjustLight::AdjustLight_DecideLightAdjustValue_ConvertTargetRate2TargetValue(ADJUSTINFO* lpInfo, LONG lTargetRate, int nIndex)
{
	LONG lTargetValue = 0;

	if (nIndex == FRONT)
	{
		
#if defined(LIGHT_ADJUST_EAGLE_TYPE) || defined(LIGHT_ADJUST_CHIEBUS_TYPE) || defined(LIGHT_ADJUST_DOCAN_TYPE) || defined(LIGHT_ADJUST_HACHI_TYPE) || defined(LIGHT_ADJUST_BOW_TYPE) || defined(LIGHT_ADJUST_NEWDT_TYPE) || defined(LIGHT_ADJUST_VOYAJER_TYPE) || defined(LIGHT_ADJUST_DRF120_TYPE) || defined(LIGHT_ADJUST_BUNZ_TYPE) || defined(LIGHT_ADJUST_VC_TYPE) || defined(LIGHT_ADJUST_CAROL_TYPE) || defined(LIGHT_ADJUST_MINERVA_TYPE) || defined(LIGHT_ADJUST_DASH_TYPE)
		
		LONG lSaturation = 0;
		
		LONG lDark =0;
		lSaturation = std::min(m_LightParams_Saturate[CAdjustLight::Front_Red].lLightLevelMax, std::min(m_LightParams_Saturate[CAdjustLight::Front_Green].lLightLevelMax, m_LightParams_Saturate[CAdjustLight::Front_Blue].lLightLevelMax));
		lDark = std::max(m_LightParams_Dark[CAdjustLight::Front_Red].lLightLevelMin, std::max(m_LightParams_Dark[CAdjustLight::Front_Green].lLightLevelMin, m_LightParams_Dark[CAdjustLight::Front_Blue].lLightLevelMin));
		
		lTargetValue = lDark + (lSaturation - lDark) * lTargetRate / 100;
#elif 0
		
		LONG lSaturation = 0;
		LONG lDark =0;
		lSaturation = std::max(m_LightParams_Saturate[CAdjustLight::Front_Red].lLightLevelMax, std::max(m_LightParams_Saturate[CAdjustLight::Front_Green].lLightLevelMax, m_LightParams_Saturate[CAdjustLight::Front_Blue].lLightLevelMax));
		lDark = std::max(m_LightParams_Dark[CAdjustLight::Front_Red].lLightLevelMin, std::max(m_LightParams_Dark[CAdjustLight::Front_Green].lLightLevelMin, m_LightParams_Dark[CAdjustLight::Front_Blue].lLightLevelMin));
		
		lTargetValue = lDark + (lSaturation - lDark) * lTargetRate / 100;
#else
#error LIGHT_ADJUST_XXX_TYPE is undefined.
#endif
	}
	else if (nIndex == BACK)
	{
#if defined(LIGHT_ADJUST_EAGLE_TYPE) || defined(LIGHT_ADJUST_CHIEBUS_TYPE) || defined(LIGHT_ADJUST_DOCAN_TYPE) || defined(LIGHT_ADJUST_HACHI_TYPE) || defined(LIGHT_ADJUST_BOW_TYPE) || defined(LIGHT_ADJUST_NEWDT_TYPE) || defined(LIGHT_ADJUST_VOYAJER_TYPE) || defined(LIGHT_ADJUST_DRF120_TYPE) || defined(LIGHT_ADJUST_BUNZ_TYPE) || defined(LIGHT_ADJUST_VC_TYPE) || defined(LIGHT_ADJUST_CAROL_TYPE) || defined(LIGHT_ADJUST_MINERVA_TYPE) || defined(LIGHT_ADJUST_DASH_TYPE)
		
		LONG lSaturation = 0;
		LONG lDark =0;
		lSaturation = std::min(m_LightParams_Saturate[CAdjustLight::Back_Red].lLightLevelMax, std::min(m_LightParams_Saturate[CAdjustLight::Back_Green].lLightLevelMax, m_LightParams_Saturate[CAdjustLight::Back_Blue].lLightLevelMax));
		lDark = std::max(m_LightParams_Dark[CAdjustLight::Back_Red].lLightLevelMin, std::max(m_LightParams_Dark[CAdjustLight::Back_Green].lLightLevelMin, m_LightParams_Dark[CAdjustLight::Back_Blue].lLightLevelMin));
		
		lTargetValue = lDark + (lSaturation - lDark) * lTargetRate / 100;
#elif 0
		
		LONG lSaturation = 0;
		LONG lDark =0;
		lSaturation = std::max(m_LightParams_Saturate[CAdjustLight::Back_Red].lLightLevelMax, std::max(m_LightParams_Saturate[CAdjustLight::Back_Green].lLightLevelMax, m_LightParams_Saturate[CAdjustLight::Back_Blue].lLightLevelMax));
		lDark = std::max(m_LightParams_Dark[CAdjustLight::Back_Red].lLightLevelMin, std::max(m_LightParams_Dark[CAdjustLight::Back_Green].lLightLevelMin, m_LightParams_Dark[CAdjustLight::Back_Blue].lLightLevelMin));
		lTargetValue = lDark + (lSaturation - lDark) * lTargetRate / 100;
#else
#error LIGHT_ADJUST_XXX_TYPE is undefined.
#endif
	}

	return lTargetValue;
}
LONG CAdjustLight::AdjustLight_DecideLightAdjustValue_DecideTargetLightLevel(ADJUSTINFO* lpInfo, LONG & lTargetValue, int nIndex)
{
	int LightSorce[2] = {lpInfo->FrontLightSorce, lpInfo->BackLightSorce};

	LONG lDecidedTargetValue = lTargetValue;

#if defined(LIGHT_ADJUST_EAGLE_TYPE) || defined(LIGHT_ADJUST_CHIEBUS_TYPE) || defined(LIGHT_ADJUST_HACHI_TYPE) || defined(LIGHT_ADJUST_BOW_TYPE)
	{
		long lTargetMax = (long)((double)LIGHT_ADJUST_GAIN_ADJ_TARGET_LIST[m_lSensorVer]*(double)0.95);

		lDecidedTargetValue = std::min(lTargetValue, lTargetMax);
	}

#elif defined LIGHT_ADJUST_DOCAN_TYPE || defined(LIGHT_ADJUST_NEWDT_TYPE) || defined(LIGHT_ADJUST_VOYAJER_TYPE) || defined(LIGHT_ADJUST_DRF120_TYPE) || defined(LIGHT_ADJUST_MINERVA_TYPE) || defined(LIGHT_ADJUST_DASH_TYPE)
	{
		if (lpInfo->ScanMode == GRAY)
		{
			if ((LightSorce[nIndex] == ADJUSTINFO::DROPOUT_RED) || (LightSorce[nIndex] == ADJUSTINFO::DROPOUT_GREEN) || (LightSorce[nIndex] == ADJUSTINFO::DROPOUT_BLUE))//�h���b�v�A�E�g�ݒ�
			{
				long lTargetMax = LIGHT_ADJUST_GAIN_ADJ_TARGET_LIST[m_lSensorVer] * 85 / 100;

				lDecidedTargetValue = std::min(lTargetValue, lTargetMax);
			}
			else if ((LightSorce[nIndex] == ADJUSTINFO::EMPHASIS_RED) || (LightSorce[nIndex] == ADJUSTINFO::EMPHASIS_GREEN) || (LightSorce[nIndex] == ADJUSTINFO::EMPHASIS_BLUE))//�F�����ݒ�
			{
				lDecidedTargetValue = (LIGHT_ADJUST_GAIN_ADJ_TARGET_LIST[m_lSensorVer] / 2) * 85 / 100;
			}
			else 
			{
				lDecidedTargetValue = (LIGHT_ADJUST_GAIN_ADJ_TARGET_LIST[m_lSensorVer] / 3) * 85 / 100;
			}
		}
		else
		{
			long lTargetMax = LIGHT_ADJUST_GAIN_ADJ_TARGET_LIST[m_lSensorVer] * 85 / 100;

			lDecidedTargetValue = std::min(lTargetValue, lTargetMax);
		}	
	}
#elif defined(LIGHT_ADJUST_BUNZ_TYPE) || defined(LIGHT_ADJUST_VC_TYPE)
	{
		if (lpInfo->ScanMode == COLOR)
		{
			long lTargetMax = LIGHT_ADJUST_GAIN_ADJ_TARGET_LIST[m_lSensorVer] * 95 / 100;
			lDecidedTargetValue = std::min(lTargetValue, lTargetMax);
		}
		else if ((lpInfo->ScanMode == GRAY) && ((LightSorce[nIndex] == ADJUSTINFO::DROPOUT_RED) || (LightSorce[nIndex] == ADJUSTINFO::DROPOUT_GREEN) || (LightSorce[nIndex] == ADJUSTINFO::DROPOUT_BLUE)))
		{
			long lTargetMax = LIGHT_ADJUST_GAIN_ADJ_TARGET_LIST[m_lSensorVer] * 95 / 100;
			lDecidedTargetValue = std::min(lTargetValue, lTargetMax);
		}
		else if ((lpInfo->ScanMode == GRAY) && ((LightSorce[nIndex] == ADJUSTINFO::EMPHASIS_RED) || (LightSorce[nIndex] == ADJUSTINFO::EMPHASIS_GREEN) || (LightSorce[nIndex] == ADJUSTINFO::EMPHASIS_BLUE)))
		{
			long lTargetMax = LIGHT_ADJUST_GAIN_ADJ_TARGET_LIST[m_lSensorVer] * 50 / 100;
			lDecidedTargetValue = std::min(lTargetValue, lTargetMax);
		}
		else
		{
		}
	}
#elif defined(LIGHT_ADJUST_CAROL_TYPE)
	{
		if (lpInfo->ScanMode == COLOR)
		{
			long lTargetMax = LIGHT_ADJUST_GAIN_ADJ_TARGET_LIST[m_lSensorVer] * 100 / 100;
			lDecidedTargetValue = std::min(lTargetValue, lTargetMax);
		}
		else if ((lpInfo->ScanMode == GRAY) && ((LightSorce[nIndex] == ADJUSTINFO::DROPOUT_RED) || (LightSorce[nIndex] == ADJUSTINFO::DROPOUT_GREEN) || (LightSorce[nIndex] == ADJUSTINFO::DROPOUT_BLUE)))
		{
			long lTargetMax = LIGHT_ADJUST_GAIN_ADJ_TARGET_LIST[m_lSensorVer] * 100 / 100;
			lDecidedTargetValue = std::min(lTargetValue, lTargetMax);
		}
		else if ((lpInfo->ScanMode == GRAY) && ((LightSorce[nIndex] == ADJUSTINFO::EMPHASIS_RED) || (LightSorce[nIndex] == ADJUSTINFO::EMPHASIS_GREEN) || (LightSorce[nIndex] == ADJUSTINFO::EMPHASIS_BLUE)))
		{
			long lTargetMax = LIGHT_ADJUST_GAIN_ADJ_TARGET_LIST[m_lSensorVer] * 50 / 100;
			lDecidedTargetValue = std::min(lTargetValue, lTargetMax);
		}
		else
		{
		}
	}
#else
#error LIGHT_ADJUST_XXX_TYPE is undefined.
#endif
	
	return lDecidedTargetValue;
}
void CAdjustLight::AdjustLight_DecideLightAdjustValue_DecideTargetRegister(ADJUSTINFO* lpInfo, LONG & TargetValue, int nIndex)
{
	if (nIndex == FRONT)
	{
		m_LightParams_Target[CAdjustLight::Front_Red].lLightRegister = GetTargetRegister(	m_LightParams_ReferenceDark[CAdjustLight::Front_Red].lLightRegister,
																							m_LightParams_ReferenceDark[CAdjustLight::Front_Red].lLightLevelMax,
																							m_LightParams_Reference[CAdjustLight::Front_Red].lLightRegister,
																							m_LightParams_Reference[CAdjustLight::Front_Red].lLightLevelMax,
																							TargetValue);
		m_LightParams_Target[CAdjustLight::Front_Red].lLightLevelMax = TargetValue;

		m_LightParams_Target[CAdjustLight::Front_Green].lLightRegister = GetTargetRegister(	m_LightParams_ReferenceDark[CAdjustLight::Front_Green].lLightRegister,
																							m_LightParams_ReferenceDark[CAdjustLight::Front_Green].lLightLevelMax,
																							m_LightParams_Reference[CAdjustLight::Front_Green].lLightRegister,
																							m_LightParams_Reference[CAdjustLight::Front_Green].lLightLevelMax,
																							TargetValue);
		m_LightParams_Target[CAdjustLight::Front_Green].lLightLevelMax = TargetValue;

		m_LightParams_Target[CAdjustLight::Front_Blue].lLightRegister = GetTargetRegister(	m_LightParams_ReferenceDark[CAdjustLight::Front_Blue].lLightRegister,
																							m_LightParams_ReferenceDark[CAdjustLight::Front_Blue].lLightLevelMax,
																							m_LightParams_Reference[CAdjustLight::Front_Blue].lLightRegister,
																							m_LightParams_Reference[CAdjustLight::Front_Blue].lLightLevelMax,
																							TargetValue);
		m_LightParams_Target[CAdjustLight::Front_Blue].lLightLevelMax = TargetValue;
	}
	else if (nIndex == BACK)
	{
		m_LightParams_Target[CAdjustLight::Back_Red].lLightRegister = GetTargetRegister(	m_LightParams_ReferenceDark[CAdjustLight::Back_Red].lLightRegister,
																							m_LightParams_ReferenceDark[CAdjustLight::Back_Red].lLightLevelMax,
																							m_LightParams_Reference[CAdjustLight::Back_Red].lLightRegister,
																							m_LightParams_Reference[CAdjustLight::Back_Red].lLightLevelMax,
																							TargetValue);
		m_LightParams_Target[CAdjustLight::Back_Red].lLightLevelMax = TargetValue;

		m_LightParams_Target[CAdjustLight::Back_Green].lLightRegister = GetTargetRegister(	m_LightParams_ReferenceDark[CAdjustLight::Back_Green].lLightRegister,
																							m_LightParams_ReferenceDark[CAdjustLight::Back_Green].lLightLevelMax,
																							m_LightParams_Reference[CAdjustLight::Back_Green].lLightRegister,
																							m_LightParams_Reference[CAdjustLight::Back_Green].lLightLevelMax,
																							TargetValue);
		m_LightParams_Target[CAdjustLight::Back_Green].lLightLevelMax = TargetValue;

		m_LightParams_Target[CAdjustLight::Back_Blue].lLightRegister = GetTargetRegister(	m_LightParams_ReferenceDark[CAdjustLight::Back_Blue].lLightRegister,
																							m_LightParams_ReferenceDark[CAdjustLight::Back_Blue].lLightLevelMax,
																							m_LightParams_Reference[CAdjustLight::Back_Blue].lLightRegister,
																							m_LightParams_Reference[CAdjustLight::Back_Blue].lLightLevelMax,
																							TargetValue);
		m_LightParams_Target[CAdjustLight::Back_Blue].lLightLevelMax = TargetValue;
	}
}
RTN CAdjustLight::DecideTargetRegister(ADJUSTINFO* lpInfo, int nIndex)
{
	int LightSorce[2] = {lpInfo->FrontLightSorce, lpInfo->BackLightSorce};

	if ((lpInfo->ScanMode == COLOR) || (LightSorce[nIndex] != ADJUSTINFO::LIGHT_NORMAL)) {
		return RTN_OK;
	}

	if (nIndex==0)
	{
		LPWORD lpwRef2_R = (LPWORD)m_OneLineImageData_Reference[CAdjustLight::Front_Red].getImagePtr();
		LPWORD lpwRef2_G = (LPWORD)m_OneLineImageData_Reference[CAdjustLight::Front_Green].getImagePtr();
		LPWORD lpwRef2_B = (LPWORD)m_OneLineImageData_Reference[CAdjustLight::Front_Blue].getImagePtr();

		long reg_Ref2_R = m_LightParams_Reference[CAdjustLight::Front_Red].lLightRegister;
		long reg_Ref2_G = m_LightParams_Reference[CAdjustLight::Front_Green].lLightRegister;
		long reg_Ref2_B = m_LightParams_Reference[CAdjustLight::Front_Blue].lLightRegister;

		LPWORD lpwRef1_R = (LPWORD)m_OneLineImageData_ReferenceDark[CAdjustLight::Front_Red].getImagePtr();
		LPWORD lpwRef1_G = (LPWORD)m_OneLineImageData_ReferenceDark[CAdjustLight::Front_Green].getImagePtr();
		LPWORD lpwRef1_B = (LPWORD)m_OneLineImageData_ReferenceDark[CAdjustLight::Front_Blue].getImagePtr();

		long reg_Ref1_R = m_LightParams_ReferenceDark[CAdjustLight::Front_Red].lLightRegister;
		long reg_Ref1_G = m_LightParams_ReferenceDark[CAdjustLight::Front_Green].lLightRegister;
		long reg_Ref1_B = m_LightParams_ReferenceDark[CAdjustLight::Front_Blue].lLightRegister;

		LPWORD lpwDark_R = (LPWORD)m_OneLineImageData_Dark[CAdjustLight::Front_Red].getImagePtr();
		LPWORD lpwDark_G = (LPWORD)m_OneLineImageData_Dark[CAdjustLight::Front_Green].getImagePtr();
		LPWORD lpwDark_B = (LPWORD)m_OneLineImageData_Dark[CAdjustLight::Front_Blue].getImagePtr();

		LONG size = m_OneLineImageData_ReferenceDark[CAdjustLight::Front_Red].getImageSize() / sizeof(WORD);
		if (size == 0) {
			return RTN_DEBUG;
		}

		{
			double rate = DecideTargetRegister_for_BunZGrayProc::GetRate(reg_Ref1_R, reg_Ref2_R, lpwRef1_R[0], lpwRef2_R[0], lpwDark_R[0], LIGHT_ADJUST_LIGHT_TARGET_RATE_GRAY, LIGHT_ADJUST_GAIN_ADJ_TARGET_LIST[m_lSensorVer]);

			for (LONG l=0; l<size; l++)
			{
				double temp = 
					DecideTargetRegister_for_BunZGrayProc::GetRate(reg_Ref1_R, reg_Ref2_R, lpwRef1_R[l], lpwRef2_R[l], lpwDark_R[l], LIGHT_ADJUST_LIGHT_TARGET_RATE_GRAY, LIGHT_ADJUST_GAIN_ADJ_TARGET_LIST[m_lSensorVer]);

				rate = std::min(rate, temp);
			}

			LONG lTarget_Register = (LONG)((double)reg_Ref2_R * rate);

			m_LightParams_Target[CAdjustLight::Front_Red].lLightRegister = lTarget_Register;
		}

		//green
		{
			double dSum = 0;

			for (LONG l=0; l<size; l++)
			{
				if ((lpwRef2_G[l] - lpwDark_G[l]) != 0)
				{
					dSum += (double)(lpwRef2_R[l] - lpwDark_R[l]) / (double)(lpwRef2_G[l] - lpwDark_G[l]);
				}
			}

			double rate = (double)dSum / (double)size;

			LONG RReg = m_LightParams_Target[CAdjustLight::Front_Red].lLightRegister;

			m_LightParams_Target[CAdjustLight::Front_Green].lLightRegister = (LONG)((double)RReg * rate);
		}

		//blue
		{
			double dSum = 0;

			for (LONG l=0; l<size; l++)
			{
				if ((lpwRef2_B[l] - lpwDark_B[l]) != 0)
				{
					dSum += (double)(lpwRef2_R[l] - lpwDark_R[l]) / (double)(lpwRef2_B[l] - lpwDark_B[l]);
				}
			}

			double rate = (double)dSum / (double)size;

			LONG RReg = m_LightParams_Target[CAdjustLight::Front_Red].lLightRegister;

			m_LightParams_Target[CAdjustLight::Front_Blue].lLightRegister = (LONG)((double)RReg * rate);
		}
	}
	else if (nIndex == 1)
	{LPWORD lpwRef2_R = (LPWORD)m_OneLineImageData_Reference[CAdjustLight::Back_Red].getImagePtr();
		LPWORD lpwRef2_G = (LPWORD)m_OneLineImageData_Reference[CAdjustLight::Back_Green].getImagePtr();
		LPWORD lpwRef2_B = (LPWORD)m_OneLineImageData_Reference[CAdjustLight::Back_Blue].getImagePtr();

		long reg_Ref2_R = m_LightParams_Reference[CAdjustLight::Back_Red].lLightRegister;
		long reg_Ref2_G = m_LightParams_Reference[CAdjustLight::Back_Green].lLightRegister;
		long reg_Ref2_B = m_LightParams_Reference[CAdjustLight::Back_Blue].lLightRegister;

		LPWORD lpwRef1_R = (LPWORD)m_OneLineImageData_ReferenceDark[CAdjustLight::Back_Red].getImagePtr();
		LPWORD lpwRef1_G = (LPWORD)m_OneLineImageData_ReferenceDark[CAdjustLight::Back_Green].getImagePtr();
		LPWORD lpwRef1_B = (LPWORD)m_OneLineImageData_ReferenceDark[CAdjustLight::Back_Blue].getImagePtr();

		long reg_Ref1_R = m_LightParams_ReferenceDark[CAdjustLight::Back_Red].lLightRegister;
		long reg_Ref1_G = m_LightParams_ReferenceDark[CAdjustLight::Back_Green].lLightRegister;
		long reg_Ref1_B = m_LightParams_ReferenceDark[CAdjustLight::Back_Blue].lLightRegister;

		LPWORD lpwDark_R = (LPWORD)m_OneLineImageData_Dark[CAdjustLight::Back_Red].getImagePtr();
		LPWORD lpwDark_G = (LPWORD)m_OneLineImageData_Dark[CAdjustLight::Back_Green].getImagePtr();
		LPWORD lpwDark_B = (LPWORD)m_OneLineImageData_Dark[CAdjustLight::Back_Blue].getImagePtr();

		LONG size = m_OneLineImageData_ReferenceDark[CAdjustLight::Back_Red].getImageSize() / 2;
		if (size == 0) {
			return RTN_DEBUG;
		}

		{
			double rate = DecideTargetRegister_for_BunZGrayProc::GetRate(reg_Ref1_R, reg_Ref2_R, lpwRef1_R[0], lpwRef2_R[0], lpwDark_R[0], LIGHT_ADJUST_LIGHT_TARGET_RATE_GRAY, LIGHT_ADJUST_GAIN_ADJ_TARGET_LIST[m_lSensorVer]);

			for (LONG l=0; l<size; l++)
			{
				double temp = 
					DecideTargetRegister_for_BunZGrayProc::GetRate(reg_Ref1_R, reg_Ref2_R, lpwRef1_R[l], lpwRef2_R[l], lpwDark_R[l], LIGHT_ADJUST_LIGHT_TARGET_RATE_GRAY, LIGHT_ADJUST_GAIN_ADJ_TARGET_LIST[m_lSensorVer]);

				rate = std::min(rate, temp);
			}

			LONG lTarget_Register = (LONG)((double)reg_Ref2_R * rate);

			m_LightParams_Target[CAdjustLight::Back_Red].lLightRegister = lTarget_Register;
		}

		//green
		{
			double dSum = 0;

			for (LONG l=0; l<size; l++)
			{
				if ((lpwRef2_G[l] - lpwDark_G[l]) != 0)
				{
					dSum += (double)(lpwRef2_R[l] - lpwDark_R[l]) / (double)(lpwRef2_G[l] - lpwDark_G[l]);
				}
			}

			double rate = (double)dSum / (double)size;

			LONG RReg = m_LightParams_Target[CAdjustLight::Back_Red].lLightRegister;

			m_LightParams_Target[CAdjustLight::Back_Green].lLightRegister = (LONG)((double)RReg * rate);
		}

		//blue
		{
			double dSum = 0;

			for (LONG l=0; l<size; l++)
			{
				if ((lpwRef2_B[l] - lpwDark_B[l]) != 0)
				{
					dSum += (double)(lpwRef2_R[l] - lpwDark_R[l]) / (double)(lpwRef2_B[l] - lpwDark_B[l]);
				}		
			}

			double rate = (double)dSum / (double)size;

			LONG RReg = m_LightParams_Target[CAdjustLight::Back_Red].lLightRegister;

			m_LightParams_Target[CAdjustLight::Back_Blue].lLightRegister = (LONG)((double)RReg * rate);
		}
	}

	return RTN_OK;
}
RTN CAdjustLight::AdjustLight_DecideLightAdjustValue_CheckRegisterLimit(ADJUSTINFO* lpInfo, int nIndex)
{
	bool bFront = nIndex == 0 ? true : false;

	
	long num = 0, denom = 0;	
	BOOL bNeedToCorrect = FALSE;	

	RTN rtn = GetMulRate(lpInfo, bNeedToCorrect, num, denom, bFront);
	if (rtn != RTN_OK) {
		return rtn;
	}

	if (bNeedToCorrect == TRUE)
	{
		CorrectRegist(lpInfo, num, denom, bFront);
	}

	return rtn;
}
void CAdjustLight::AdjustLight_DecideLightAdjustValue_Finish(ADJUSTINFO* lpInfo, int nIndex)
{
	if (nIndex == FRONT)
	{
		lpInfo->FrontAdjustInfo.RedLED = m_LightParams_Target[CAdjustLight::Front_Red].lLightRegister;
		lpInfo->FrontAdjustInfo.GreenLED = m_LightParams_Target[CAdjustLight::Front_Green].lLightRegister;
		lpInfo->FrontAdjustInfo.BlueLED = m_LightParams_Target[CAdjustLight::Front_Blue].lLightRegister;
	}
	else if (nIndex == BACK)
	{
		lpInfo->BackAdjustInfo.RedLED = m_LightParams_Target[CAdjustLight::Back_Red].lLightRegister;
		lpInfo->BackAdjustInfo.GreenLED = m_LightParams_Target[CAdjustLight::Back_Green].lLightRegister;
		lpInfo->BackAdjustInfo.BlueLED = m_LightParams_Target[CAdjustLight::Back_Blue].lLightRegister;
	}
}

RTN CAdjustLight::GetMulRate(ADJUSTINFO* lpInfo, BOOL & bNeedToCorrect, long & num, long & denom, bool bIsFront)
{
	
	long regR = 0, regG = 0, regB = 0;
	RTN rtn = GetAvailableRegister(lpInfo, regR, regG, regB);
	if (rtn != RTN_OK) {
		return rtn;
	}
#define CALC_MAX_MULRATE(current, available, maxcurrent, maxavailable) \
	{\
		/**/\
		if (available==0)\
		{\
			return RTN_DEBUG;\
		}\
		if (maxavailable==0)\
		{\
			return RTN_DEBUG;\
		}\
		double newrate = (double)current/(double)available;\
		double currentrate = (double)maxcurrent/(double)maxavailable;\
		if (newrate > currentrate)\
		{\
			maxavailable = available;\
			maxcurrent = current;\
		}\
	}
long lMaxCurrent = 0, lMaxAvailable = 0;

	if (bIsFront == true)
	{
		lMaxCurrent = m_LightParams_Target[CAdjustLight::Front_Red].lLightRegister;
		lMaxAvailable = regR;

		CALC_MAX_MULRATE(m_LightParams_Target[CAdjustLight::Front_Red].lLightRegister, regR, lMaxCurrent, lMaxAvailable);
		CALC_MAX_MULRATE(m_LightParams_Target[CAdjustLight::Front_Green].lLightRegister, regG, lMaxCurrent, lMaxAvailable);
		CALC_MAX_MULRATE(m_LightParams_Target[CAdjustLight::Front_Blue].lLightRegister, regB, lMaxCurrent, lMaxAvailable);
	}
	else
	{
		lMaxCurrent = m_LightParams_Target[CAdjustLight::Back_Red].lLightRegister;
		lMaxAvailable = regR;

		CALC_MAX_MULRATE(m_LightParams_Target[CAdjustLight::Back_Red].lLightRegister, regR, lMaxCurrent, lMaxAvailable);
		CALC_MAX_MULRATE(m_LightParams_Target[CAdjustLight::Back_Green].lLightRegister, regG, lMaxCurrent, lMaxAvailable);
		CALC_MAX_MULRATE(m_LightParams_Target[CAdjustLight::Back_Blue].lLightRegister, regB, lMaxCurrent, lMaxAvailable);
	}

	if (lMaxAvailable < lMaxCurrent)
	{
		bNeedToCorrect = TRUE;
	denom = lMaxCurrent;
		num = lMaxAvailable;
	}
	else
	{
		bNeedToCorrect = FALSE;
	}

	return rtn;
}

int CAdjustLight::GetOffsetRegister_Feeder(int minData, int oldGainReg, int oldOffsetReg, int nOffsetAdjTarget)
{

#if defined(LIGHT_ADJUST_DOCAN_TYPE)
	int newOffsetRegister = (minData - nOffsetAdjTarget) * (351 - oldGainReg) * 156 / (400 * (656 + 3 * oldGainReg)) + oldOffsetReg;
#elif defined(LIGHT_ADJUST_EAGLE_TYPE) || defined(LIGHT_ADJUST_CHIEBUS_TYPE) || defined(LIGHT_ADJUST_HACHI_TYPE) || defined(LIGHT_ADJUST_BOW_TYPE)
	
	int nDelta = 0;
	{
		double d1 = (double)(minData - nOffsetAdjTarget) / (double) 4096;
		double d2 = (double)1.98 / (double) 2.0;
		double num = d1 * (double)1980 * (double)(16 + 63 - oldGainReg);
		double denom = (double)1.9 * (double)80 * d2;
		nDelta = (int)(num / denom);
	}

	int newOffsetRegister = oldOffsetReg - nDelta;
#elif defined(LIGHT_ADJUST_NEWDT_TYPE) || defined(LIGHT_ADJUST_VOYAJER_TYPE) || defined(LIGHT_ADJUST_DRF120_TYPE)
   
    
    int nAnaproRange = 1350;
    double dAnaproTick = -2.4;
    int nDigitalRange = 4096;
    double dGainValue = (double)((2*208)/(33.3+255.0-oldGainReg))/(double)((2*208)/(33.3+255.0));
    double dOffsetChange = ((double)(minData-nOffsetAdjTarget)/(double)nDigitalRange) * ((double)(nAnaproRange/(double)(dGainValue*dAnaproTick)));
    int newOffsetRegister = (int)(oldOffsetReg - dOffsetChange);
    newOffsetRegister = std::min((int)LIGHT_ADJUST_OFFSET_MAX, std::max((int)LIGHT_ADJUST_OFFSET_MIN , newOffsetRegister));
#elif defined(LIGHT_ADJUST_BUNZ_TYPE)
    int nDelta = (int)((double)((double)1200 * ((double)minData - (double)nOffsetAdjTarget) )
                       / (double)((double)ConvertdB2Step((double)BUNZ_GAIN_DB[oldGainReg]) * (double)256 * (double)2.34));
    
    int newOffsetRegister = oldOffsetReg - nDelta;
#elif defined(LIGHT_ADJUST_VC_TYPE)
    int newOffsetRegister = 0;
    if (m_lSensorVer == 1)
    {
        int nAnaproRange = 1100;
        double dAnaproTick = 1.95;
        int nDigitalRange = 256;
        
        double dGainValue = ((double)3/(double)63)*(double)oldGainReg + 0.5;
        
        double dOffsetChange = ((double)(minData-nOffsetAdjTarget)/(double)nDigitalRange) * ((double)(nAnaproRange/(double)(dGainValue*dAnaproTick)));
        
        newOffsetRegister = (int)(oldOffsetReg - dOffsetChange);
    }
    else
    {
        int nDelta = (int)((double)((double)1200 * ((double)minData - (double)nOffsetAdjTarget) )
                           / (double)((double)ConvertdB2Step((double)BUNZ_GAIN_DB[oldGainReg]) * (double)256 * (double)2.34));
        
        newOffsetRegister = oldOffsetReg - nDelta;
    }
#elif defined(LIGHT_ADJUST_CAROL_TYPE) || defined(LIGHT_ADJUST_MINERVA_TYPE) || defined(LIGHT_ADJUST_DASH_TYPE)
	double dOffsetTick = 2.04;
	double dAnaproRange = 1200.0;
	double dMinData = minData;
	double dOffsetTarget = nOffsetAdjTarget;
	double dDigitalRange = 255.0;
	int nRegisterDelta = (int)(((double)dAnaproRange/(double)dOffsetTick) *((double)(dMinData-dOffsetTarget)/(double)dDigitalRange));
	int newOffsetRegister = oldOffsetReg - nRegisterDelta;
#else
#error LIGHT_ADJUST_XXX_TYPE is undefined.
#endif
	newOffsetRegister = std::min((int)LIGHT_ADJUST_OFFSET_MAX, std::max((int)LIGHT_ADJUST_OFFSET_MIN , newOffsetRegister));
	return newOffsetRegister;
}

int CAdjustLight::GetGainRegister(int maxData, int nOldGainReg, int nGainAdjTarget)
{
#if defined(LIGHT_ADJUST_DOCAN_TYPE)
	
	int newGainRegister = 
		(351*(656+3*nOldGainReg)*nGainAdjTarget - 656*maxData*(351-nOldGainReg))
			/ ((656+3*nOldGainReg)*nGainAdjTarget + 3*maxData*(351-nOldGainReg));
#elif defined(LIGHT_ADJUST_EAGLE_TYPE) || defined(LIGHT_ADJUST_CHIEBUS_TYPE) || defined(LIGHT_ADJUST_HACHI_TYPE) || defined(LIGHT_ADJUST_BOW_TYPE)
	

	int newGainRegister = (16 + 63) - (int)((double)((double) maxData / (double) nGainAdjTarget) * (double)(16 + 63 - nOldGainReg));

#elif defined(LIGHT_ADJUST_NEWDT_TYPE) || defined(LIGHT_ADJUST_VOYAJER_TYPE)
    double dOldGainValue = ((double)(2.0*208.0)/(double)(33.3+255.0-nOldGainReg))/((double)(2.0*208.0)/(double)(33.3+255.0));
    int newGainRegister = (int)(33.3+255-((33.3+255.0)*(double)maxData)/(double)(dOldGainValue*(double)nGainAdjTarget));
#elif defined(LIGHT_ADJUST_DRF120_TYPE)
    double dOldGainValue = ((double)(2.0*208.0)/(double)(33.3+255.0-nOldGainReg))/((double)(2.0*208.0)/(double)(33.3+255.0));
    int newGainRegister = (int)(33.3+255-((33.3+255.0)*(double)maxData)/(double)(dOldGainValue*(double)nGainAdjTarget));
#elif defined(LIGHT_ADJUST_BUNZ_TYPE)
    int newGainRegister = 0;
    int maxDataWithNoGain = (int)((double)maxData / (double)ConvertdB2Step(BUNZ_GAIN_DB[nOldGainReg]));
    
    int diff = abs(nGainAdjTarget - (int)((double)maxDataWithNoGain * ConvertdB2Step(BUNZ_GAIN_DB[0])));
    
    for (int i=0; i<BUNZ_GAIN_DB_COUNT; i++)
    {
        int val = (int)((double)maxDataWithNoGain * ConvertdB2Step(BUNZ_GAIN_DB[i]));
        
        if ((val < nGainAdjTarget) && (abs(val - nGainAdjTarget) < diff))
        {
            newGainRegister = i;
            diff = (abs(val - nGainAdjTarget));
        }
    }
#elif defined(LIGHT_ADJUST_VC_TYPE)
    int newGainRegister = 0;
    
    if (m_lSensorVer == 1)
    {
        double dOldGain = ((double)3/(double)63)*(double)nOldGainReg + 0.5;
        newGainRegister = (int)(((double)63/(double)3)*(((double)nGainAdjTarget*dOldGain)/(double)maxData-0.5));
        
   }
    else
    {
        int maxDataWithNoGain = (int)((double)maxData / (double)ConvertdB2Step(BUNZ_GAIN_DB[nOldGainReg]));
        
        int diff = abs(nGainAdjTarget - (int)((double)maxDataWithNoGain * ConvertdB2Step(BUNZ_GAIN_DB[0])));
        
        for (int i=0; i<BUNZ_GAIN_DB_COUNT; i++)
        {
            int val = (int)((double)maxDataWithNoGain * ConvertdB2Step(BUNZ_GAIN_DB[i]));
            
            if ((val < nGainAdjTarget) && (abs(val - nGainAdjTarget) < diff))
            {
                newGainRegister = i;
                diff = (abs(val - nGainAdjTarget));
            }
        }
    }
#elif defined(LIGHT_ADJUST_CAROL_TYPE)
	int newGainRegister = 0;
#elif defined(LIGHT_ADJUST_MINERVA_TYPE) || defined(LIGHT_ADJUST_DASH_TYPE)
	int newGainRegister = 0;
#else
#error LIGHT_ADJUST_XXX_TYPE is undefined.
#endif
	newGainRegister = std::min((int)LIGHT_ADJUST_GAIN_MAX, std::max((int)LIGHT_ADJUST_GAIN_MIN , newGainRegister));
	return newGainRegister;
}

unsigned short CAdjustLight::GetMin(unsigned short *pStart, unsigned long size, unsigned long* pIndex) {
	assert(size);
	unsigned short *pMin = pStart;
	unsigned short *p = pStart;
	while (size--) {
		if (*pMin > *p) pMin = p;
		++p;
	}
	if (pIndex) *pIndex = static_cast<unsigned long>(pMin - pStart);
	return *pMin;
}

unsigned short CAdjustLight::GetMin(CImg& img, unsigned long* pIndex) {
	if (img.getSpp() == 1 || img.getRGBOrder() == PIXEL_ORDER) {
		assert(img.getBps() == 16);
		unsigned short ret = GetMin((unsigned short*)img.getImagePtr(), img.getWidth() * img.getSpp(), pIndex);
		return ret;
	}
	else {
		unsigned short minR = GetMin((unsigned short*)(img.getImagePtr() + img.getSync() * 0), img.getWidth());
		unsigned short minG = GetMin((unsigned short*)(img.getImagePtr() + img.getSync() * 1), img.getWidth());
		unsigned short minB = GetMin((unsigned short*)(img.getImagePtr() + img.getSync() * 2), img.getWidth());
		unsigned short ret = std::min(minG, minB);
		ret = std::min(minR, ret);
		return ret;
	}
}

unsigned short CAdjustLight::GetMin(CImg &img, unsigned short &minR, unsigned short &minG, unsigned short &minB, unsigned long *pIndex) {
    if ((img.getSpp() == 3) && (img.getRGBOrder() == PIXEL_ORDER)) {
		assert(img.getBps() == 16);
        unsigned short *ptr = (unsigned short*)img.getImagePtr();
        LONG width = img.getWidth();
        LONG sync = img.getSync();
        LONG height = img.getHeight();
        
        minR=*ptr;
        minG=*(ptr+1);
        minB=*(ptr+2);
        
        for (LONG y=0; y<height; y++)
        {
            for (LONG x=0; x<width; x++)
            {
                minR = std::min(minR, ptr[y*sync + x*3 + 0]);
                minG = std::min(minG, ptr[y*sync + x*3 + 1]);
                minB = std::min(minB, ptr[y*sync + x*3 + 2]);
            }
        }
        
        unsigned short ret = std::min(minR, std::min(minG, minB));
        return ret;
    }
	if (img.getSpp() == 1 || img.getRGBOrder() == PIXEL_ORDER) {
		assert(img.getBps() == 16);
		unsigned short ret = GetMin((unsigned short*)img.getImagePtr(), img.getWidth() * img.getSpp(), pIndex);
		return ret;
	}
	else {
		minR = GetMin((unsigned short*)(img.getImagePtr() + img.getSync() * 0), img.getWidth());
		minG = GetMin((unsigned short*)(img.getImagePtr() + img.getSync() * 1), img.getWidth());
		minB = GetMin((unsigned short*)(img.getImagePtr() + img.getSync() * 2), img.getWidth());
		unsigned short ret = std::min(minG, minB);
		ret = std::min(minR, ret);
		return ret;
	}
}

unsigned short CAdjustLight::GetMax(unsigned short *pStart, unsigned long size, unsigned long *pIndex) {
	assert(size);
	unsigned short *pmax = pStart;
	unsigned short *p = pStart;
	while (size--) {
		if (*pmax < *p) pmax = p;
		++p;
	}
	if (pIndex) *pIndex = static_cast<unsigned long>(pmax - pStart);
	return *pmax;
}

unsigned short CAdjustLight::GetMax(CImg& img, unsigned short &maxR, unsigned short &maxG, unsigned short &maxB, unsigned long *pIndex) {
    if ((img.getSpp() == 3) && (img.getRGBOrder() == PIXEL_ORDER)) {
		assert(img.getBps() == 16);
        unsigned short *ptr = (unsigned short*)img.getImagePtr();
        long width = img.getWidth();
        long sync = img.getSync();
        long height = img.getHeight();
        
        maxR=0;
        maxG=0;
        maxB=0;
        
        for (long y=0; y<height; y++)
        {
            for (long x=0; x<width; x++)
            {
                maxR = std::max(maxR, ptr[y*sync + x*3 + 0]);
                maxG = std::max(maxG, ptr[y*sync + x*3 + 1]);
                maxB = std::max(maxB, ptr[y*sync + x*3 + 2]);
            }
        }
        
        unsigned short ret = std::max(maxR, std::max(maxG, maxB));
        return ret;
    }
	else if (img.getSpp() == 1 || img.getRGBOrder() == PIXEL_ORDER) {
		assert(img.getBps() == 16);
		unsigned short ret = GetMax((unsigned short*)img.getImagePtr(), img.getWidth() * img.getSpp(), pIndex);
		return ret;
	}
	else {
		maxR = GetMax((unsigned short*)(img.getImagePtr() + img.getSync() * 0), img.getWidth());
		maxG = GetMax((unsigned short*)(img.getImagePtr() + img.getSync() * 1), img.getWidth());
		maxB = GetMax((unsigned short*)(img.getImagePtr() + img.getSync() * 2), img.getWidth());
		unsigned short ret = std::max(maxG, maxB);
		ret = std::max(maxR, ret);
		return ret;
	}
}

unsigned short CAdjustLight::GetMax(CImg &img, unsigned long *pIndex) {

unsigned short ret = 0;
	if (img.getSpp() == 1 || img.getRGBOrder() == PIXEL_ORDER) {
		assert(img.getBps() == 16);
		ret = GetMax((unsigned short*)img.getImagePtr(), img.getWidth() * img.getSpp(), pIndex);
	}
	else {
		unsigned short maxR = GetMax((unsigned short*)(img.getImagePtr() + img.getSync() * 0), img.getWidth());
		unsigned short maxG = GetMax((unsigned short*)(img.getImagePtr() + img.getSync() * 1), img.getWidth());
		unsigned short maxB = GetMax((unsigned short*)(img.getImagePtr() + img.getSync() * 2), img.getWidth());
		ret = std::max(maxG, maxB);
		ret = std::max(maxR, ret);
	}
	return ret;
}

RTN CAdjustLight::ColorToRGB(CImg & src, CImg & dstR, CImg & dstG, CImg & dstB)
{
	
	{
		if (!dstR.createImg(src.getWidth(), 1, src.getBps(), 1, 0)) {
			return RTN_PAR;
		}
		if (!dstG.createImg(src.getWidth(), 1, src.getBps(), 1, 0)) {
			return RTN_PAR;
		}
		if (!dstB.createImg(src.getWidth(), 1, src.getBps(), 1, 0)) {
			return RTN_PAR;
		}
		if (dstR.isNull() || dstG.isNull() || dstB.isNull()) {
			return RTN_NOMEM;
		}
	}

	{
		LPWORD lpwSrc = (LPWORD)src.getImagePtr();
		LPWORD lpwRed = (LPWORD)dstR.getImagePtr();
		LPWORD lpwGreen = (LPWORD)dstG.getImagePtr();
		LPWORD lpwBlue = (LPWORD)dstB.getImagePtr();

		LONG width = src.getWidth();

		if (src.getRGBOrder() == LINE_ORDER) {
			if (src.getHeight() != 1) {
				return RTN_PAR;
			}

			long sync = src.getSync();

			LPBYTE lpb = (LPBYTE)lpwSrc;

			memcpy(lpwRed, lpb + sync * 0, dstR.getImageSize());
			memcpy(lpwGreen, lpb + sync * 1, dstG.getImageSize());
			memcpy(lpwBlue, lpb + sync * 2, dstB.getImageSize());
		}
		else if (src.getRGBOrder() == PIXEL_ORDER) {
			if (src.getHeight() != 1) {
				return RTN_PAR;
			}

			while(width--) {
				*lpwRed++ = *lpwSrc++;
				*lpwGreen++ = *lpwSrc++;
				*lpwBlue++ = *lpwSrc++;
			}
		}
		else {
			return RTN_PAR;
		}
	}

	return RTN_OK;
}