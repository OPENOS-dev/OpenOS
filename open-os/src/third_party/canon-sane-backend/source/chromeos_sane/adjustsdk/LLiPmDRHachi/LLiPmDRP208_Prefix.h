/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/

#ifndef LLiPmDRP208_Prifix
#define LLiPmDRP208_Prifix


#define DR_NAMESPACE		DRP208
#define DRFilterInfoHeader	"DRP208FilterInfo.h"
#define LLiPmDRHeader		"LLiPmDRP208.h"
#define SENSOR_FB_OFFSET				(-5250)	//表裏オフセット
#define LIGHT_ADJUST_BOW_TYPE

//白紙判定
#define DO_IS_BLANK_PAGE_IN_SCANNER					0 //白紙判定をスキャナ内で行うときは1にする
#define IS_BLANK_PAGE_MAX_LIMIT_DOT_COUNT			1931188 //ChieBusの設定値のまま
#define IS_BLANK_PAGE_PRODUCT_MAX_LIMIT_DOT_COUNT	37623   //ChieBusの設定値のまま
//4点検知
#define FOUR_POINTS_DETECTION_BY_DOUBLE_SIDE 1//両面画像を使って4点検知をすべきスキャナのときは1にする
#define SHADE_POSITION_TOP_OR_BOTTOM	1 //画像の先端に濃い影がつく場合は1を、後端につく場合(例,TakeZ)は0をセット

#define RATE_FOR_SHADING_NEWDT                              0 //ShadingNewDT処理で用いる明るさ変更パラメーター。明るさ比の1000倍をセット。ShadingNewDT処理を行わない場合は0をセット。
#define LIGHT_ADJUST_LIGHT_TARGET_RATE						95 //TakeZは50%、ChieBusは75%
#define LIGHT_ADJUST_LIGHT_TARGET_RATE_GRAY					95
//300dpi
#define LIGHT_ADJUST_FEEDER_REFERENCE_DARK_POWER			200 //参照点その１
#define LIGHT_ADJUST_FEEDER_REFERENCE_POWER_300				500 //TakeZは800、ChieBusは500
#define LIGHT_ADJUST_FEEDER_REFERENCE_POWER_RED_300			1500 //TakeZは800、ChieBusは1500
#define LIGHT_ADJUST_FEEDER_REFERENCE_POWER_BLUE_300		1500 //TakeZは800、ChieBusは1500
#define LIGHT_ADJUST_FEEDER_REFERENCE_POWER_GREEN_300		1500 //TakeZは800、ChieBusは1500
//600dpi
#define LIGHT_ADJUST_FEEDER_REFERENCE_POWER_600				1000 //TakeZは1600、ChieBusは1000
#define LIGHT_ADJUST_FEEDER_REFERENCE_POWER_RED_600			3000 //TakeZは1600、ChieBusは3000
#define LIGHT_ADJUST_FEEDER_REFERENCE_POWER_BLUE_600		3000 //TakeZは1600、ChieBusは5000
#define LIGHT_ADJUST_FEEDER_REFERENCE_POWER_GREEN_600		3000 //TakeZは1600、ChieBusは5000
#define LIGHT_ADJUST_OFFSET_ADJ_TARGET						(int)(96) //TakeZは(int)(96)、ChieBusは(int)(96)
#define LIGHT_ADJUST_GAIN_MAX								63	//設定可能なゲインレジスタの最大値
#define LIGHT_ADJUST_GAIN_MIN								0x00	//設定可能なゲインレジスタの最小値
#define LIGHT_ADJUST_OFFSET_MAX								0xff	//設定可能なオフセットの最大値
#define LIGHT_ADJUST_OFFSET_MIN								0x00	//設定可能なオフセットの最小値
#define LIGHT_ADJUST_LED_MAX								0x1fff	//設定可能なLED光量の最大値
#define LIGHT_ADJUST_LED_MIN								0x0		//設定可能なLED光量の最小値
#define LIGHT_ADJUST_TARGET_COLOR							75	//飽和光量の75%
#define LIGHT_ADJUST_TARGET_GRAY							25	//飽和光量の25%
#define LIGHT_ADJUST_INITIALIZE_GAIN						0x0		//ゲインの初期値
#define LIGHT_ADJUST_INITIALIZE_OFFSET						160	//オフセットの初期値
#define LIGHT_ADJUST_AVAILABLE_LAMP_POWER_300_RED			2500		//使用可能なレジスタ値 (R, 300dpi)		
#define LIGHT_ADJUST_AVAILABLE_LAMP_POWER_300_GREEN			3000	//使用可能なレジスタ値（G, 300dpi）
#define LIGHT_ADJUST_AVAILABLE_LAMP_POWER_300_BLUE			3000		//使用可能なレジスタ値（B, 300dpi）
#define LIGHT_ADJUST_AVAILABLE_LAMP_POWER_600_RED			5000		//使用可能なレジスタ値（R, 600dpi）
#define LIGHT_ADJUST_AVAILABLE_LAMP_POWER_600_GREEN			6000	//使用可能なレジスタ値（G, 600dpi）
#define LIGHT_ADJUST_AVAILABLE_LAMP_POWER_600_BLUE			6000		//使用可能なレジスタ値（B, 600dpi）

//ゲインターゲット
//(int)(0xfff * 0.90) //TakeZは(int)(0xfff * 0.90)、ChieBusは(int)(2730)
const unsigned long LIGHT_ADJUST_GAIN_ADJ_TARGET_LIST[] = {
	2730
};

//シェーディングターゲット {Gray, Red, Green, Blue}
#define SHADING_TARGET                                      {{4204, 4271, 4328, 4406}}

//スキャナの情報
#define NEW_SENSOR_VERISON_ID                               2   //センサー情報
//ダミー画素 {左、中、右}
const unsigned int DUMMY_PIXEL_300[][3] = {
    {0, 0, 0}
};
const unsigned int DUMMY_PIXEL_600[][3] = {
    {0, 0, 0}
};

#define SRGB_NAMESPACE	DR_NAMESPACE		// SRGBで用いるnamespace
#define GRC_NAMESPACE	DR_NAMESPACE		// GRCで用いるnamespace
#undef GRCFIRST                     // SRGBとGRCで、GRCを先にかける場合定義する

//色ずれ補正
#define SCANNER_LIGHTORDER							0	// スキャナの点灯順番(色ずれ補正に使用する)
#define COLORGAP_PARAM_150		sizeof(GAPPARAM),104,144,152,88,64,144,152,80,56,144,160,64,96
#define COLORGAP_PARAM_200		sizeof(GAPPARAM),96,136,144,88,64,136,144,80,56,144,160,64,96
#define COLORGAP_PARAM_240		sizeof(GAPPARAM),84,116,116,88,64,100,116,80,56,128,160,72,92
#define COLORGAP_PARAM_300		sizeof(GAPPARAM),84,116,116,88,64,100,116,80,56,128,160,72,92
#define COLORGAP_PARAM_400		sizeof(GAPPARAM),104,144,152,88,64,144,152,80,56,144,160,64,96
#define COLORGAP_PARAM_600		sizeof(GAPPARAM),84,116,116,88,64,100,108,80,48,128,160,72,92

// カラー白黒検知のオフセット
#define COLORORGRAY_THRESHOLD_OFFSET		{{ 0, 0.001, 0.035, 0.085, 0.150, 0.225, 0.310, 0.405 }}
// グレー白黒検知の中央値(2値化用スライス) デフォルトは明るさ128での2値化スライス
#define DETECT_GRAY_BINARIZE_SLICE				136
// グレー白黒検知の明るい側のスライス幅
#define HIGHER_GRAY_THRESHOLD_FROM_SLICE	{ 40, 48, 32, 32, 24, 24, 8, 0 }
// グレー白黒検知の暗い側のスライス幅
#define LOWER_GRAY_THRESHOLD_FROM_SLICE		{ 45, 54, 46, 32, 32, 16, 16, 0 }

#define GAMMA_FOR_COLORORGRAY(x)		(x <= 24 ? (1.333*x) : (391 * pow(x / (double)255, (double)1 / (double)2.2) - 103 + 0.5))
#define INVGAMMA_FOR_COLORORGRAY(x)		(x <= 32 ? (double)x/1.333 : (255 * pow((x + 103 - 0.5) / (double)391, 2.2)))

// 背景スムージング用のエッジ閾値
#define COLORSATURATION_EDGE_THRESHOLD			16

//エッジ強調
#define USE_EDGE_EMPHASIS_PARAMETER_V3								0	// Version3のエッジ強調パラメーターを使うか


#define DETECT4POINTS_ENABLED_MARGIN_MM		5000	// 4点検知で使用する画像につける上下マージンの最大値に指定がある場合は定義する。単位は[mm]
//#define FORCE_CORRECT_MIRRORIMAGE                   // CorrectUnusualScanningDirectionのMIRRORを強制的に行う

#endif