/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef LLiPmDRHachi_Prifix
#define LLiPmDRHachi_Prifix


#define DR_NAMESPACE		DRP215
#define DRFilterInfoHeader	"DRP215FilterInfo.h"
#define LLiPmDRHeader		"LLiPmDRP215.h"
#define SENSOR_FB_OFFSET				(6250)	//�\���I�t�Z�b�g
#define LIGHT_ADJUST_HACHI_TYPE


#define DO_IS_BLANK_PAGE_IN_SCANNER					0 //����������X�L���i���ōs���Ƃ���1�ɂ���
#define IS_BLANK_PAGE_MAX_LIMIT_DOT_COUNT			1931188 //ChieBus�̐ݒ�l�̂܂�
#define IS_BLANK_PAGE_PRODUCT_MAX_LIMIT_DOT_COUNT	37623   //ChieBus�̐ݒ�l�̂܂�

#define FOUR_POINTS_DETECTION_BY_DOUBLE_SIDE 1//���ʉ摜���g����4�_���m�����ׂ��X�L���i�̂Ƃ���1�ɂ���
#define SHADE_POSITION_TOP_OR_BOTTOM	0 //�摜�̐�[�ɔZ���e�����ꍇ��1���A��[�ɂ��ꍇ(��,TakeZ)��0���Z�b�g

#define RATE_FOR_SHADING_NEWDT                              0 //ShadingNewDT�����ŗp���閾�邳�ύX�p�����[�^�[�B���邳���1000�{���Z�b�g�BShadingNewDT�������s��Ȃ��ꍇ��0���Z�b�g�B
#define LIGHT_ADJUST_LIGHT_TARGET_RATE						95 //TakeZ��50%�AChieBus��75%
#define LIGHT_ADJUST_LIGHT_TARGET_RATE_GRAY					95
//300dpi
#define LIGHT_ADJUST_FEEDER_REFERENCE_DARK_POWER			200 //�Q�Ɠ_���̂P
#define LIGHT_ADJUST_FEEDER_REFERENCE_POWER_300				500 //TakeZ��800�AChieBus��500
#define LIGHT_ADJUST_FEEDER_REFERENCE_POWER_RED_300			1500 //TakeZ��800�AChieBus��1500
#define LIGHT_ADJUST_FEEDER_REFERENCE_POWER_BLUE_300		1500 //TakeZ��800�AChieBus��1500
#define LIGHT_ADJUST_FEEDER_REFERENCE_POWER_GREEN_300		1500 //TakeZ��800�AChieBus��1500
//600dpi
#define LIGHT_ADJUST_FEEDER_REFERENCE_POWER_600				1000 //TakeZ��1600�AChieBus��1000
#define LIGHT_ADJUST_FEEDER_REFERENCE_POWER_RED_600			3500 //TakeZ��1600�AChieBus��3000
#define LIGHT_ADJUST_FEEDER_REFERENCE_POWER_BLUE_600		3500 //TakeZ��1600�AChieBus��5000
#define LIGHT_ADJUST_FEEDER_REFERENCE_POWER_GREEN_600		3500 //TakeZ��1600�AChieBus��5000
#define LIGHT_ADJUST_OFFSET_ADJ_TARGET						(int)(96) //TakeZ��(int)(96)�AChieBus��(int)(96)
#define LIGHT_ADJUST_GAIN_MAX								0xff	//�ݒ�\�ȃQ�C�����W�X�^�̍ő�l
#define LIGHT_ADJUST_GAIN_MIN								0x00	//�ݒ�\�ȃQ�C�����W�X�^�̍ŏ��l
#define LIGHT_ADJUST_OFFSET_MAX								0xff	//�ݒ�\�ȃI�t�Z�b�g�̍ő�l
#define LIGHT_ADJUST_OFFSET_MIN								0x00	//�ݒ�\�ȃI�t�Z�b�g�̍ŏ��l
#define LIGHT_ADJUST_LED_MAX								0x1fff	//�ݒ�\��LED���ʂ̍ő�l
#define LIGHT_ADJUST_LED_MIN								0x0		//�ݒ�\��LED���ʂ̍ŏ��l
#define LIGHT_ADJUST_TARGET_COLOR							75	//�O�a���ʂ�75%
#define LIGHT_ADJUST_TARGET_GRAY							25	//�O�a���ʂ�25%
#define LIGHT_ADJUST_INITIALIZE_GAIN						0x0		//�Q�C���̏����l
#define LIGHT_ADJUST_INITIALIZE_OFFSET						0x80	//�I�t�Z�b�g�̏����l
#define LIGHT_ADJUST_AVAILABLE_LAMP_POWER_300_RED			2500		//�g�p�\�ȃ��W�X�^�l (R, 300dpi)		
#define LIGHT_ADJUST_AVAILABLE_LAMP_POWER_300_GREEN			3000	//�g�p�\�ȃ��W�X�^�l�iG, 300dpi�j
#define LIGHT_ADJUST_AVAILABLE_LAMP_POWER_300_BLUE			3000		//�g�p�\�ȃ��W�X�^�l�iB, 300dpi�j
#define LIGHT_ADJUST_AVAILABLE_LAMP_POWER_600_RED			5000		//�g�p�\�ȃ��W�X�^�l�iR, 600dpi�j
#define LIGHT_ADJUST_AVAILABLE_LAMP_POWER_600_GREEN			6000	//�g�p�\�ȃ��W�X�^�l�iG, 600dpi�j
#define LIGHT_ADJUST_AVAILABLE_LAMP_POWER_600_BLUE			6000		//�g�p�\�ȃ��W�X�^�l�iB, 600dpi�j

//�Q�C���^�[�Q�b�g
//(int)(0xfff * 0.90) //TakeZ��(int)(0xfff * 0.90)�AChieBus��(int)(2730)
const unsigned long LIGHT_ADJUST_GAIN_ADJ_TARGET_LIST[] = {
	2730,
	2730
};

//�V�F�[�f�B���O�^�[�Q�b�g {Gray, Red, Green, Blue}
#define SHADING_TARGET                                      {{4316, 4306, 4317, 4349}, \
                                                             {4316, 4306, 4317, 4349}}

//�X�L���i�̏��
#define NEW_SENSOR_VERISON_ID                               2   //�Z���T�[���
//�_�~�[��f {���A���A�E}
const unsigned int DUMMY_PIXEL_300[][3] = {
    {0, 0, 0}
};
const unsigned int DUMMY_PIXEL_600[][3] = {
    {0, 0, 0}
};

#define SRGB_NAMESPACE	DR_NAMESPACE		// SRGB�ŗp����namespace
#define GRC_NAMESPACE	DR_NAMESPACE		// GRC�ŗp����namespace
#define GRCFIRST                    // SRGB��GRC�ŁAGRC���ɂ�����ꍇ��`����

//�F����␳
#define SCANNER_LIGHTORDER							0	// �X�L���i�̓_������(�F����␳�Ɏg�p����)
#define COLORGAP_PARAM_150		sizeof(GAPPARAM),104,144,152,88,64,144,152,80,56,144,160,64,96
#define COLORGAP_PARAM_200		sizeof(GAPPARAM),96,136,144,88,64,136,144,80,56,144,160,64,96
#define COLORGAP_PARAM_240		sizeof(GAPPARAM),84,116,116,88,64,100,116,80,56,128,160,72,92
#define COLORGAP_PARAM_300		sizeof(GAPPARAM),84,116,116,88,64,100,116,80,56,128,160,72,92
#define COLORGAP_PARAM_400		sizeof(GAPPARAM),104,144,152,88,64,144,152,80,56,144,160,64,96
#define COLORGAP_PARAM_600		sizeof(GAPPARAM),84,116,116,88,64,100,108,80,48,128,160,72,92

// �J���[�������m�̃I�t�Z�b�g
#define COLORORGRAY_THRESHOLD_OFFSET		{{ 0, 0.001, 0.035, 0.085, 0.150, 0.225, 0.310, 0.405 }, \
                                             { 0, 0.001, 0.035, 0.085, 0.150, 0.225, 0.310, 0.405 }}
// �O���[�������m�̒����l(2�l���p�X���C�X) �f�t�H���g�͖��邳128�ł�2�l���X���C�X
#define DETECT_GRAY_BINARIZE_SLICE				136
// �O���[�������m�̖��邢���̃X���C�X��
#define HIGHER_GRAY_THRESHOLD_FROM_SLICE	{ 40, 48, 32, 32, 24, 24, 8, 0 }
// �O���[�������m�̈Â����̃X���C�X��
#define LOWER_GRAY_THRESHOLD_FROM_SLICE		{ 45, 54, 46, 32, 32, 16, 16, 0 }

#define GAMMA_FOR_COLORORGRAY(x)		(x <= 24 ? (1.333*x) : (391 * pow(x / (double)255, (double)1 / (double)2.2) - 103 + 0.5))
#define INVGAMMA_FOR_COLORORGRAY(x)		(x <= 32 ? (double)x/1.333 : (255 * pow((x + 103 - 0.5) / (double)391, 2.2)))

// �w�i�X���[�W���O�p�̃G�b�W臒l
#define COLORSATURATION_EDGE_THRESHOLD			16

//�G�b�W����
#define USE_EDGE_EMPHASIS_PARAMETER_V3								0	// Version3�̃G�b�W�����p�����[�^�[���g����

//#define DETECT4POINTS_ENABLED_MARGIN_MM		5000	// 4�_���m�Ŏg�p����摜�ɂ���㉺�}�[�W���̍ő�l�Ɏw�肪����ꍇ�͒�`����B�P�ʂ�[mm]
//#define FORCE_CORRECT_MIRRORIMAGE                   // CorrectUnusualScanningDirection��MIRROR�������I�ɍs��

#endif