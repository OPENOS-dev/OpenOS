/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/

#pragma once
#include "IPBase.h"
#include "Dependencies.h"

#include <stdlib.h>
#include <math.h>

namespace Cei
{
	namespace LLiPm
	{
		namespace DR_NAMESPACE
		{
            void AdjustLightData_SetGain(ADJUSTINFO& info, bool front, unsigned char value);
            unsigned char AdjustLightData_GetGain(ADJUSTINFO& info, bool front);
            void AdjustLightData_SetOffset(ADJUSTINFO& info, bool front, unsigned char value);
            unsigned char AdjustLightData_GetOffset(ADJUSTINFO& info, bool front);
            
            void AdjustLightData_SetLEDCurrent(ADJUSTINFO& info, bool front, unsigned long value);
            
           inline double ConvertdB2Step(double dB) { return (double)pow((double)10, (double)dB / (double)20); }
            
            
			class CAdjustLight
			{
			public:
				enum {
					IGNORE_LAMPPOWER = 0x8000,
					SENSOR_SCAN_WHITE = 0x0001,
					SENSOR_SCAN_BLACK = 0x0,
				};
				typedef struct tagRGBPower
				{
					long Red;
					long RedOp;
					long Green;
					long GreenOp;
					long Blue;
					long BlueOp;
				} RGBPower, LPRGBPower;
			public:
				CAdjustLight(void);
				~CAdjustLight(void);
			private:
				unsigned char m_Itr;
                long m_lSensorVer;  // 0 based incremental version
			public:
				void AdjustLightFirst(ADJUSTINFO* lpInfo, long lSensorId);
				RTN AdjustLightNext(CImg& imgFront, CImg& imgBack, ADJUSTINFO* lpInfo);
				RTN AdjustLightCurve(CImg& White, CImg& Black, ADJUSTINFO* lpInfo, SIDE Side, unsigned char* lpData, unsigned long ulSize);	// �����̕␳
                long GetSensorVer() const { return m_lSensorVer; }
			private:
				void AdjustAnaproOffsetInit(ADJUSTINFO* lpInfo);											
				RTN AdjustAnaproOffset(CImg& imgFront, CImg& imgBack, ADJUSTINFO* lpInfo);					
				void AdjustLight_GetSensorDarkLevelInit(ADJUSTINFO* lpInfo);								// �Z���T�̈Ód�����x�����擾����
				RTN AdjustLight_GetSensorDarkLevel(CImg& imgFront, CImg& imgBack, ADJUSTINFO* lpInfo);		// �Z���T�̈Ód�����x�����擾����
				void AdjustLight_GetSensorSaturateLevelInit(ADJUSTINFO* lpInfo);							// �Z���T�̖O�a���x�����擾����
				RTN AdjustLight_GetSensorSaturateLevel(CImg& imgFront, CImg& imgBack, ADJUSTINFO* lpInfo);	// �Z���T�̖O�a���x�����擾����
				void AdjustLight_GetLightDarkLevelInit(ADJUSTINFO* lpInfo);									// LED������ɓ_�����A���ʂ�DarkLevel�Ƃ��Ďg�p����
				RTN AdjustLight_GetLightDarkLevel(CImg& imgFront, CImg& imgBack, ADJUSTINFO* lpInfo);		// LED������ɓ_�����A���ʂ�DarkLevel�Ƃ��Ďg�p����
				void AdjustLightInit(ADJUSTINFO* lpInfo);													// ���ʒ����l���쐬
				RTN AdjustLight(CImg& imgFront, CImg& imgBack, ADJUSTINFO* lpInfo);							// ���ʒ����l���쐬
				void AdjustAnaproGainInit(ADJUSTINFO* lpInfo);												// Gain ����
				RTN AdjustAnaproGain(CImg& imgFront, CImg& imgBack, ADJUSTINFO* lpInfo);					// Gain ����
				void AdjustDecideData(ADJUSTINFO* lpInfo);
			private:
				void AdjustLight_GetSensorReferenceLevelInit(ADJUSTINFO* lpInfo, long lPowerR, long lPowerG, long lPowerB);
				RTN AdjustLight_GetSensorReferenceLevel(CImg& imgFront, CImg& imgBack, ADJUSTINFO* lpInfo, long lPower);
			private:
				enum LightParams {
					Front_Red,
					Front_Green,
					Front_Blue,
					Back_Red,
					Back_Green,
					Back_Blue,
					MAX_LIGHTRES,
				};
				typedef struct tagLIGHTRESPONSELEVEL{
					long lLightRegister;			
					long lLightLevelMax;			
					long lLightLevelMin;			
				} LIGHTRESPONSELEVEL;
				LIGHTRESPONSELEVEL m_LightParams_Dark[MAX_LIGHTRES];
				LIGHTRESPONSELEVEL m_LightParams_Saturate[MAX_LIGHTRES];
				LIGHTRESPONSELEVEL m_LightParams_ReferenceDark[MAX_LIGHTRES];
				LIGHTRESPONSELEVEL m_LightParams_Reference[MAX_LIGHTRES];
				LIGHTRESPONSELEVEL m_LightParams_Target[MAX_LIGHTRES];

				
				CImg m_OneLineImageData_Dark[MAX_LIGHTRES];
				CImg m_OneLineImageData_Saturate[MAX_LIGHTRES];
				CImg m_OneLineImageData_ReferenceDark[MAX_LIGHTRES];
				CImg m_OneLineImageData_Reference[MAX_LIGHTRES];
			private:
				long m_lLampPowerForGetSensorLevelR;
				long m_lLampPowerForGetSensorLevelG;
				long m_lLampPowerForGetSensorLevelB;
				void AdjustLight_GetSensorLevelInit(ADJUSTINFO* lpInfo, long lLampPowerR, long lLampPowerG, long lLampPowerB);
				RTN AdjustLight_GetSensorLevel(LIGHTRESPONSELEVEL* pLightParams, CImg* pOneLineImg, CImg& imgFront, CImg& imgBack, ADJUSTINFO* lpInfo);
				void CorrectRegist(ADJUSTINFO* lpInfo, long & num, long & denom, bool bFront);

				RTN AdjustLight_DecideLightAdjustValue(ADJUSTINFO* lpInfo);


				LONG AdjustLight_DecideLightAdjustValue_GetTargetValueRate(ADJUSTINFO* lpInfo, int nIndex);
				LONG AdjustLight_DecideLightAdjustValue_ConvertTargetRate2TargetValue(ADJUSTINFO* lpInfo, LONG lTargetRate, int nIndex);
				LONG AdjustLight_DecideLightAdjustValue_DecideTargetLightLevel(ADJUSTINFO* lpInfo, LONG & lTargetValue, int nIndex);
				void AdjustLight_DecideLightAdjustValue_DecideTargetRegister(ADJUSTINFO* lpInfo, LONG & TargetValue, int nIndex);
				RTN DecideTargetRegister(ADJUSTINFO* lpInfo, int nIndex);
				RTN AdjustLight_DecideLightAdjustValue_CheckRegisterLimit(ADJUSTINFO* lpInfo, int nIndex);
				void AdjustLight_DecideLightAdjustValue_Finish(ADJUSTINFO* lpInfo, int nIndex);

			private:
				long GetTargetX(long x1, long y1, long x2, long y2, long targetx)
				{
					if (abs(y1 - y2) == 0) return 0;
					return (x1-x2) * targetx / (y1-y2) + (x1*y2 - x2*y1)/(y2-y1);
				}
				long GetTargetRegister(long reg1, long level1, long reg2, long level2, long targetlevel)
				{
					return GetTargetX(reg1, level1, reg2, level2, targetlevel);
				}
				RTN GetAvailableRegister(ADJUSTINFO* lpInfo, long & regR, long & regG, long & regB)
				{
					RTN rtn = RTN_OK;
					switch(lpInfo->lXResolution)
					{
					case 300:
						regR = LIGHT_ADJUST_AVAILABLE_LAMP_POWER_300_RED;
						regG = LIGHT_ADJUST_AVAILABLE_LAMP_POWER_300_GREEN;
						regB = LIGHT_ADJUST_AVAILABLE_LAMP_POWER_300_BLUE;
						break;
					case 600:
						regR = LIGHT_ADJUST_AVAILABLE_LAMP_POWER_600_RED;
						regG = LIGHT_ADJUST_AVAILABLE_LAMP_POWER_600_GREEN;
						regB = LIGHT_ADJUST_AVAILABLE_LAMP_POWER_600_BLUE;
						break;
                            
#if defined(LIGHT_ADJUST_EAGLE_TYPE) || defined(LIGHT_ADJUST_CHIEBUS_TYPE) || defined(LIGHT_ADJUST_DOCAN_TYPE) || defined(LIGHT_ADJUST_HACHI_TYPE) || defined(LIGHT_ADJUST_BOW_TYPE) || defined(LIGHT_ADJUST_NEWDT_TYPE) || defined(LIGHT_ADJUST_VOYAJER_TYPE) || defined(LIGHT_ADJUST_DRF120_TYPE) || defined(LIGHT_ADJUST_MINERVA_TYPE) || defined(LIGHT_ADJUST_DASH_TYPE)
#elif defined(LIGHT_ADJUST_BUNZ_TYPE) || defined(LIGHT_ADJUST_VC_TYPE) || defined(LIGHT_ADJUST_CAROL_TYPE)
                    
                    case 150:
                        regR = LIGHT_ADJUST_AVAILABLE_LAMP_POWER_150_RED;
                        regG = LIGHT_ADJUST_AVAILABLE_LAMP_POWER_150_GREEN;
                        regB = LIGHT_ADJUST_AVAILABLE_LAMP_POWER_150_BLUE;
                        break;
                    case 200:
                        regR = LIGHT_ADJUST_AVAILABLE_LAMP_POWER_200_RED;
                        regG = LIGHT_ADJUST_AVAILABLE_LAMP_POWER_200_GREEN;
                        regB = LIGHT_ADJUST_AVAILABLE_LAMP_POWER_200_BLUE;
                        break;
                    case 400:
                        regR = LIGHT_ADJUST_AVAILABLE_LAMP_POWER_400_RED;
                        regG = LIGHT_ADJUST_AVAILABLE_LAMP_POWER_400_GREEN;
                        regB = LIGHT_ADJUST_AVAILABLE_LAMP_POWER_400_BLUE;
                        break;
#else
#error LIGHT_ADJUST_XXX_TYPE is undefined.
#endif
                            
#if defined(LIGHT_ADJUST_EAGLE_TYPE) || defined(LIGHT_ADJUST_CHIEBUS_TYPE) || defined(LIGHT_ADJUST_DOCAN_TYPE) || defined(LIGHT_ADJUST_HACHI_TYPE) || defined(LIGHT_ADJUST_BOW_TYPE) || defined(LIGHT_ADJUST_NEWDT_TYPE) || defined(LIGHT_ADJUST_VOYAJER_TYPE) || defined(LIGHT_ADJUST_DRF120_TYPE) || defined(LIGHT_ADJUST_BUNZ_TYPE) || defined(LIGHT_ADJUST_VC_TYPE) || defined(LIGHT_ADJUST_MINERVA_TYPE) || defined(LIGHT_ADJUST_DASH_TYPE)
#elif defined(LIGHT_ADJUST_CAROL_TYPE)
                    
                    case 100:
                        regR = LIGHT_ADJUST_AVAILABLE_LAMP_POWER_100_RED;
                        regG = LIGHT_ADJUST_AVAILABLE_LAMP_POWER_100_GREEN;
                        regB = LIGHT_ADJUST_AVAILABLE_LAMP_POWER_100_BLUE;
                        break;
                    case 240:
                        regR = LIGHT_ADJUST_AVAILABLE_LAMP_POWER_240_RED;
                        regG = LIGHT_ADJUST_AVAILABLE_LAMP_POWER_240_GREEN;
                        regB = LIGHT_ADJUST_AVAILABLE_LAMP_POWER_240_BLUE;
                        break;
#else
#error LIGHT_ADJUST_XXX_TYPE is undefined.
#endif
                    default:
                        rtn = RTN_DEBUG;
						break;
					}
					return rtn;
				}
				RTN GetMulRate(ADJUSTINFO* lpInfo, BOOL & bNeedToCorrect, long & num, long & denom, bool bIsFront);

			private:
				inline int GetOffsetRegister_Feeder(int minData, int oldGainReg, int oldOffsetReg, int nOffsetAdjTarget);
				inline int GetGainRegister(int maxData, int nOldGainReg, int nGainAdjTarget);
			private:
				unsigned short GetMin(unsigned short *pStart, unsigned long size, unsigned long* pIndex = 0); 
				unsigned short GetMin(CImg& img, unsigned long* pIndex = 0);
				unsigned short GetMin(CImg& img, unsigned short &minR, unsigned short &minG, unsigned short &minB, unsigned long *pIndex = 0);
				unsigned short GetMax(unsigned short *pStart, unsigned long size, unsigned long *pIndex = 0);
				unsigned short GetMax(CImg& img, unsigned short &maxR, unsigned short &maxG, unsigned short &maxB, unsigned long *pIndex = 0);
				unsigned short GetMax(CImg& img, unsigned long *pIndex = 0);

			private:
				RTN ColorToRGB(CImg & src,	CImg & dstR, CImg & dstG, CImg & dstB);
			};
		}
	}
}