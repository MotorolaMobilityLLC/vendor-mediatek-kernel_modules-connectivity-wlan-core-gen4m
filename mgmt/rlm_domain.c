/******************************************************************************
 *
 * This file is provided under a dual license.  When you use or
 * distribute this software, you may choose to be licensed under
 * version 2 of the GNU General Public License ("GPLv2 License")
 * or BSD License.
 *
 * GPLv2 License
 *
 * Copyright(C) 2016 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 *
 * BSD LICENSE
 *
 * Copyright(C) 2016 MediaTek Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/
/*
 * Id: //Department/DaVinci/BRANCHES/MT6620_WIFI_DRIVER_V2_3/mgmt/rlm_domain.c#2
 */

/*! \file   "rlm_domain.c"
 *    \brief
 *
 */


/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */

/*******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 *******************************************************************************
 */
#include "precomp.h"
#include "rlm_txpwr_init.h"

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */
#if CFG_SUPPORT_DYNAMIC_PWR_LIMIT
/* dynamic tx power control */
char *g_au1TxPwrChlTypeLabel[] = {
	"NORMAL",
	"ALL",
	"RANGE",
	"2G4",
	"5G",
	"BANDEDGE_2G4",
	"BANDEDGE_5G",
	"5GBAND1",
	"5GBAND2",
	"5GBAND3",
	"5GBAND4",
};

char *g_au1TxPwrAppliedWayLabel[] = {
	"wifi on",
	"ioctl"
};

char *g_au1TxPwrOperationLabel[] = {
	"power level",
	"power offset"
};
#endif
/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */

/*******************************************************************************
 *                            P U B L I C   D A T A
 *******************************************************************************
 */
#if CFG_SUPPORT_DYNAMIC_PWR_LIMIT
/* dynamic tx power control */
char *g_au1TxPwrDefaultSetting[] = {
	"_SAR_PwrLevel;1;2;1;[2G4,,,,,,,,,][5G,,,,,,,,,]",
	"_SAR_PwrOffset;1;2;2;[2G4,,,,,,,,,][5G,,,,,,,,,]",
	"_G_Scenario;1;2;1;[ALL,1]",
	"_G_Scenario;2;2;1;[ALL,2]",
	"_G_Scenario;3;2;1;[ALL,3]",
	"_G_Scenario;4;2;1;[ALL,4]",
	"_G_Scenario;5;2;1;[ALL,5]",
};
#endif
/* The following country or domain shall be set from host driver.
 * And host driver should pass specified DOMAIN_INFO_ENTRY to MT6620 as
 * the channel list of being a STA to do scanning/searching AP or being an
 * AP to choose an adequate channel if auto-channel is set.
 */

/* Define mapping tables between country code and its channel set
 */
static const uint16_t g_u2CountryGroup0[] = {
	COUNTRY_CODE_AO, COUNTRY_CODE_BZ, COUNTRY_CODE_BJ, COUNTRY_CODE_BT,
	COUNTRY_CODE_BO, COUNTRY_CODE_BI, COUNTRY_CODE_CM, COUNTRY_CODE_CF,
	COUNTRY_CODE_TD, COUNTRY_CODE_KM, COUNTRY_CODE_CD, COUNTRY_CODE_CG,
	COUNTRY_CODE_CI, COUNTRY_CODE_DJ, COUNTRY_CODE_GQ, COUNTRY_CODE_ER,
	COUNTRY_CODE_FJ, COUNTRY_CODE_GA, COUNTRY_CODE_GM, COUNTRY_CODE_GN,
	COUNTRY_CODE_GW, COUNTRY_CODE_RKS, COUNTRY_CODE_KG, COUNTRY_CODE_LY,
	COUNTRY_CODE_MG, COUNTRY_CODE_ML, COUNTRY_CODE_NR, COUNTRY_CODE_NC,
	COUNTRY_CODE_ST, COUNTRY_CODE_SC, COUNTRY_CODE_SL, COUNTRY_CODE_SB,
	COUNTRY_CODE_SO, COUNTRY_CODE_SR, COUNTRY_CODE_SZ, COUNTRY_CODE_TJ,
	COUNTRY_CODE_TG, COUNTRY_CODE_TO, COUNTRY_CODE_TM, COUNTRY_CODE_TV,
	COUNTRY_CODE_VU, COUNTRY_CODE_YE
};

static const uint16_t g_u2CountryGroup1[] = {
	COUNTRY_CODE_AS, COUNTRY_CODE_AI, COUNTRY_CODE_BM, COUNTRY_CODE_KY,
	COUNTRY_CODE_GU, COUNTRY_CODE_FM, COUNTRY_CODE_PR, COUNTRY_CODE_US,
	COUNTRY_CODE_VI
};

static const uint16_t g_u2CountryGroup2[] = {
	COUNTRY_CODE_AR, COUNTRY_CODE_AU, COUNTRY_CODE_AZ, COUNTRY_CODE_BW,
	COUNTRY_CODE_CX, COUNTRY_CODE_CO, COUNTRY_CODE_CR, COUNTRY_CODE_EC,
	COUNTRY_CODE_GD, COUNTRY_CODE_GT, COUNTRY_CODE_HK, COUNTRY_CODE_KH,
	COUNTRY_CODE_KI, COUNTRY_CODE_KR, COUNTRY_CODE_LB, COUNTRY_CODE_LR,
	COUNTRY_CODE_MN, COUNTRY_CODE_AN, COUNTRY_CODE_NZ, COUNTRY_CODE_NI,
	COUNTRY_CODE_PW, COUNTRY_CODE_PY, COUNTRY_CODE_PE, COUNTRY_CODE_PH,
	COUNTRY_CODE_WS, COUNTRY_CODE_SG, COUNTRY_CODE_LK, COUNTRY_CODE_TH,
	COUNTRY_CODE_TT, COUNTRY_CODE_UY, COUNTRY_CODE_VN
};

static const uint16_t g_u2CountryGroup3[] = {
	COUNTRY_CODE_AW, COUNTRY_CODE_LA, COUNTRY_CODE_SA, COUNTRY_CODE_AE,
	COUNTRY_CODE_UG
};

static const uint16_t g_u2CountryGroup4[] = { COUNTRY_CODE_MM };

static const uint16_t g_u2CountryGroup5[] = {
	COUNTRY_CODE_AL, COUNTRY_CODE_DZ, COUNTRY_CODE_AD, COUNTRY_CODE_AT,
	COUNTRY_CODE_BY, COUNTRY_CODE_BE, COUNTRY_CODE_BA, COUNTRY_CODE_VG,
	COUNTRY_CODE_BG, COUNTRY_CODE_CV, COUNTRY_CODE_HR, COUNTRY_CODE_CY,
	COUNTRY_CODE_CZ, COUNTRY_CODE_DK, COUNTRY_CODE_EE, COUNTRY_CODE_ET,
	COUNTRY_CODE_FI, COUNTRY_CODE_FR, COUNTRY_CODE_GF, COUNTRY_CODE_PF,
	COUNTRY_CODE_TF, COUNTRY_CODE_GE, COUNTRY_CODE_DE, COUNTRY_CODE_GH,
	COUNTRY_CODE_GR, COUNTRY_CODE_GP, COUNTRY_CODE_HU, COUNTRY_CODE_IS,
	COUNTRY_CODE_IQ, COUNTRY_CODE_IE, COUNTRY_CODE_IT, COUNTRY_CODE_KE,
	COUNTRY_CODE_LV, COUNTRY_CODE_LS, COUNTRY_CODE_LI, COUNTRY_CODE_LT,
	COUNTRY_CODE_LU, COUNTRY_CODE_MK, COUNTRY_CODE_MT, COUNTRY_CODE_MQ,
	COUNTRY_CODE_MR, COUNTRY_CODE_MU, COUNTRY_CODE_YT, COUNTRY_CODE_MD,
	COUNTRY_CODE_MC, COUNTRY_CODE_ME, COUNTRY_CODE_MS, COUNTRY_CODE_NL,
	COUNTRY_CODE_NO, COUNTRY_CODE_OM, COUNTRY_CODE_PL, COUNTRY_CODE_PT,
	COUNTRY_CODE_RE, COUNTRY_CODE_RO, COUNTRY_CODE_MF, COUNTRY_CODE_SM,
	COUNTRY_CODE_SN, COUNTRY_CODE_RS, COUNTRY_CODE_SK, COUNTRY_CODE_SI,
	COUNTRY_CODE_ZA, COUNTRY_CODE_ES, COUNTRY_CODE_SE, COUNTRY_CODE_CH,
	COUNTRY_CODE_TR, COUNTRY_CODE_TC, COUNTRY_CODE_GB, COUNTRY_CODE_VA,
	COUNTRY_CODE_EU
};

static const uint16_t g_u2CountryGroup6[] = { COUNTRY_CODE_JP };

static const uint16_t g_u2CountryGroup7[] = {
	COUNTRY_CODE_AM, COUNTRY_CODE_IL, COUNTRY_CODE_KW, COUNTRY_CODE_MA,
	COUNTRY_CODE_NE, COUNTRY_CODE_TN,
};

static const uint16_t g_u2CountryGroup8[] = { COUNTRY_CODE_NP };

static const uint16_t g_u2CountryGroup9[] = { COUNTRY_CODE_AF };

static const uint16_t g_u2CountryGroup10[] = {
	COUNTRY_CODE_AG, COUNTRY_CODE_BS, COUNTRY_CODE_BH, COUNTRY_CODE_BB,
	COUNTRY_CODE_BN, COUNTRY_CODE_CL, COUNTRY_CODE_CN, COUNTRY_CODE_EG,
	COUNTRY_CODE_SV, COUNTRY_CODE_IN, COUNTRY_CODE_MY, COUNTRY_CODE_MV,
	COUNTRY_CODE_PA, COUNTRY_CODE_VE, COUNTRY_CODE_ZM
};

static const uint16_t g_u2CountryGroup11[] = {
	COUNTRY_CODE_JO, COUNTRY_CODE_PG
};

static const uint16_t g_u2CountryGroup12[] = {
	COUNTRY_CODE_BF, COUNTRY_CODE_GY, COUNTRY_CODE_HT, COUNTRY_CODE_HN,
	COUNTRY_CODE_JM, COUNTRY_CODE_MO, COUNTRY_CODE_MW, COUNTRY_CODE_PK,
	COUNTRY_CODE_QA, COUNTRY_CODE_RW, COUNTRY_CODE_KN, COUNTRY_CODE_TZ
};

static const uint16_t g_u2CountryGroup13[] = { COUNTRY_CODE_ID };

static const uint16_t g_u2CountryGroup14[] = { COUNTRY_CODE_NG };

static const uint16_t g_u2CountryGroup15[] = {
	COUNTRY_CODE_BD, COUNTRY_CODE_BR, COUNTRY_CODE_DM, COUNTRY_CODE_DO,
	COUNTRY_CODE_FK, COUNTRY_CODE_KZ, COUNTRY_CODE_MX, COUNTRY_CODE_MZ,
	COUNTRY_CODE_NA, COUNTRY_CODE_RU, COUNTRY_CODE_LC, COUNTRY_CODE_VC,
	COUNTRY_CODE_UA, COUNTRY_CODE_UZ, COUNTRY_CODE_ZW
};

static const uint16_t g_u2CountryGroup16[] = { COUNTRY_CODE_MP };

static const uint16_t g_u2CountryGroup17[] = { COUNTRY_CODE_TW };

static const uint16_t g_u2CountryGroup18[] = { COUNTRY_CODE_CA };

static const uint16_t g_u2CountryGroup19[] = {
	COUNTRY_CODE_CK, COUNTRY_CODE_CU, COUNTRY_CODE_TL, COUNTRY_CODE_FO,
	COUNTRY_CODE_GI, COUNTRY_CODE_GG, COUNTRY_CODE_IR, COUNTRY_CODE_IM,
	COUNTRY_CODE_JE, COUNTRY_CODE_KP, COUNTRY_CODE_MH, COUNTRY_CODE_NU,
	COUNTRY_CODE_NF, COUNTRY_CODE_PS, COUNTRY_CODE_PN, COUNTRY_CODE_PM,
	COUNTRY_CODE_SS, COUNTRY_CODE_SD, COUNTRY_CODE_SY
};

static const uint16_t g_u2CountryGroup20[] = {
	COUNTRY_CODE_DF
	/* When country code is not found and no matched NVRAM setting,
	 * the default group will be used.
	 */
};

#if (CFG_SUPPORT_SINGLE_SKU == 1)
struct mtk_regd_control g_mtk_regd_control = {
	.en = FALSE,
	.state = REGD_STATE_UNDEFINED
};

#if (CFG_SUPPORT_SINGLE_SKU_LOCAL_DB == 1)
const struct ieee80211_regdomain default_regdom_ww = {
	.n_reg_rules = 4,
	.alpha2 = "99",
	.reg_rules = {
	/* channels 1..13 */
	REG_RULE_LIGHT(2412-10, 2472+10, 40, 0),
	/* channels 14 */
	REG_RULE_LIGHT(2484-10, 2484+10, 20, 0),
	/* channel 36..64 */
	REG_RULE_LIGHT(5150-10, 5350+10, 80, 0),
	/* channel 100..165 */
	REG_RULE_LIGHT(5470-10, 5850+10, 80, 0),
	}
};
#endif

struct TX_PWR_LIMIT_SECTION {
	uint8_t ucSectionNum;
	const char *arSectionNames[TX_PWR_LIMIT_SECTION_NUM];
} gTx_Pwr_Limit_Section[] = {
	{5,
	 {"legacy", "ht20", "ht40", "vht20", "offset"}
	},
	{8,
	 {"cck", "ofdm", "ht20", "ht40", "vht20", "vht40", "vht80", "vht160"}
	},
};


const u8 gTx_Pwr_Limit_Element_Num[][TX_PWR_LIMIT_SECTION_NUM] = {
	{7, 6, 7, 7, 5},
	{POWER_LIMIT_SKU_CCK_NUM, POWER_LIMIT_SKU_OFDM_NUM,
	 POWER_LIMIT_SKU_HT20_NUM, POWER_LIMIT_SKU_HT40_NUM,
	 POWER_LIMIT_SKU_VHT20_NUM, POWER_LIMIT_SKU_VHT40_NUM,
	 POWER_LIMIT_SKU_VHT80_NUM, POWER_LIMIT_SKU_VHT160_NUM},
};

const char *gTx_Pwr_Limit_Element[]
	[TX_PWR_LIMIT_SECTION_NUM]
	[TX_PWR_LIMIT_ELEMENT_NUM] = {
	{
		{"cck1_2", "cck_5_11", "ofdm6_9", "ofdm12_18", "ofdm24_36",
		 "ofdm48", "ofdm54"},
		{"mcs0_8", "mcs1_2_9_10", "mcs3_4_11_12", "mcs5_13", "mcs6_14",
		 "mcs7_15"},
		{"mcs0_8", "mcs1_2_9_10", "mcs3_4_11_12", "mcs5_13", "mcs6_14",
		 "mcs7_15", "mcs32"},
		{"mcs0", "mcs1_2", "mcs3_4", "mcs5_6", "mcs7", "mcs8", "mcs9"},
		{"lg40", "lg80", "vht40", "vht80", "vht160nc"},
	},
	{
		{"c1", "c2", "c5", "c11"},
		{"o6", "o9", "o12", "o18",
		 "o24", "o36", "o48", "o54"},
		{"m0", "m1", "m2", "m3", "m4", "m5", "m6", "m7"},
		{"m0", "m1", "m2", "m3", "m4", "m5", "m6", "m7", "m32"},
		{"m0", "m1", "m2", "m3", "m4", "m5", "m6", "m7", "m8", "m9"},
		{"m0", "m1", "m2", "m3", "m4", "m5", "m6", "m7", "m8", "m9"},
		{"m0", "m1", "m2", "m3", "m4", "m5", "m6", "m7", "m8", "m9"},
		{"m0", "m1", "m2", "m3", "m4", "m5", "m6", "m7", "m8", "m9"},
	},
};

static const int8_t gTx_Pwr_Limit_2g_Ch[] = {
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
static const int8_t gTx_Pwr_Limit_5g_Ch[] = {
	36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62, 64, 100, 102,
	104, 106, 108, 110, 112, 114, 116, 118, 120, 122, 124, 126, 128, 132,
	134, 136, 138, 140, 142, 144, 149, 151, 153, 155, 157, 159, 161, 165};

#define TX_PWR_LIMIT_2G_CH_NUM (ARRAY_SIZE(gTx_Pwr_Limit_2g_Ch))
#define TX_PWR_LIMIT_5G_CH_NUM (ARRAY_SIZE(gTx_Pwr_Limit_5g_Ch))


#endif

struct DOMAIN_INFO_ENTRY arSupportedRegDomains[] = {
	{
		(uint16_t *) g_u2CountryGroup0,
		sizeof(g_u2CountryGroup0) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */

			{115, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_LOW_NA */
			{118, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_MID_NA */
			{121, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_WW_NA */
			{125, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_UPPER_NA */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup1,
		sizeof(g_u2CountryGroup1) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 11, FALSE}
			,			/* CH_SET_2G4_1_11 */

			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_5G, CHNL_SPAN_20, 100, 12, TRUE}
			,			/* CH_SET_UNII_WW_100_144 */
			{125, BAND_5G, CHNL_SPAN_20, 149, 5, FALSE}
			,			/* CH_SET_UNII_UPPER_149_165 */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup2,
		sizeof(g_u2CountryGroup2) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */

			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_5G, CHNL_SPAN_20, 100, 12, TRUE}
			,			/* CH_SET_UNII_WW_100_144 */
			{125, BAND_5G, CHNL_SPAN_20, 149, 5, FALSE}
			,			/* CH_SET_UNII_UPPER_149_165 */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup3,
		sizeof(g_u2CountryGroup3) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */

			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_5G, CHNL_SPAN_20, 100, 12, TRUE}
			,			/* CH_SET_UNII_WW_100_144 */
			{125, BAND_5G, CHNL_SPAN_20, 149, 4, FALSE}
			,			/* CH_SET_UNII_UPPER_149_161 */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup4,
		sizeof(g_u2CountryGroup4) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */

			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_5G, CHNL_SPAN_20, 100, 12, TRUE}
			,			/* CH_SET_UNII_WW_100_144 */
			{125, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_UPPER_NA */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup5,
		sizeof(g_u2CountryGroup5) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */

			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_5G, CHNL_SPAN_20, 100, 11, TRUE}
			,			/* CH_SET_UNII_WW_100_140 */
			{125, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_UPPER_NA */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup6,
		sizeof(g_u2CountryGroup6) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */
			{82, BAND_2G4, CHNL_SPAN_5, 14, 1, FALSE}
			,			/* CH_SET_2G4_14_14 */
			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_5G, CHNL_SPAN_20, 100, 11, TRUE}
			,			/* CH_SET_UNII_WW_100_140 */
			{125, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_UPPER_NA */
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup7,
		sizeof(g_u2CountryGroup7) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */

			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_WW_NA */
			{125, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_UPPER_NA */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup8,
		sizeof(g_u2CountryGroup8) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */

			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_WW_NA */
			{125, BAND_5G, CHNL_SPAN_20, 149, 4, FALSE}
			,			/* CH_SET_UNII_UPPER_149_161 */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup9,
		sizeof(g_u2CountryGroup9) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */

			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_MID_NA */
			{121, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_WW_NA */
			{125, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_UPPER_NA */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup10,
		sizeof(g_u2CountryGroup10) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */

			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_WW_NA */
			{125, BAND_5G, CHNL_SPAN_20, 149, 5, FALSE}
			,			/* CH_SET_UNII_UPPER_149_165 */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup11,
		sizeof(g_u2CountryGroup11) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */

			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_MID_NA */
			{121, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_WW_NA */
			{125, BAND_5G, CHNL_SPAN_20, 149, 5, FALSE}
			,			/* CH_SET_UNII_UPPER_149_165 */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup12,
		sizeof(g_u2CountryGroup12) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */

			{115, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_LOW_NA */
			{118, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_MID_NA */
			{121, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_WW_NA */
			{125, BAND_5G, CHNL_SPAN_20, 149, 5, FALSE}
			,			/* CH_SET_UNII_UPPER_149_165 */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup13,
		sizeof(g_u2CountryGroup13) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */

			{115, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_LOW_NA */
			{118, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_MID_NA */
			{121, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_WW_NA */
			{125, BAND_5G, CHNL_SPAN_20, 149, 4, FALSE}
			,			/* CH_SET_UNII_UPPER_149_161 */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup14,
		sizeof(g_u2CountryGroup14) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */

			{115, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_LOW_NA */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_5G, CHNL_SPAN_20, 100, 11, TRUE}
			,			/* CH_SET_UNII_WW_100_140 */
			{125, BAND_5G, CHNL_SPAN_20, 149, 5, FALSE}
			,			/* CH_SET_UNII_UPPER_149_165 */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup15,
		sizeof(g_u2CountryGroup15) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */

			{115, BAND_5G, CHNL_SPAN_20, 36, 4, TRUE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_5G, CHNL_SPAN_20, 100, 11, TRUE}
			,			/* CH_SET_UNII_WW_100_140 */
			{125, BAND_5G, CHNL_SPAN_20, 149, 5, FALSE}
			,			/* CH_SET_UNII_UPPER_149_165 */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup16,
		sizeof(g_u2CountryGroup16) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 11, FALSE}
			,			/* CH_SET_2G4_1_11 */

			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_5G, CHNL_SPAN_20, 100, 11, TRUE}
			,			/* CH_SET_UNII_WW_100_140 */
			{125, BAND_5G, CHNL_SPAN_20, 149, 5, FALSE}
			,			/* CH_SET_UNII_UPPER_149_165 */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup17,
		sizeof(g_u2CountryGroup17) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 11, FALSE}
			,			/* CH_SET_2G4_1_11 */

			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_5G, CHNL_SPAN_20, 100, 11, TRUE}
			,			/* CH_SET_UNII_WW_100_140 */
			{125, BAND_5G, CHNL_SPAN_20, 149, 5, FALSE}
			,			/* CH_SET_UNII_UPPER_149_165 */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup18,
		sizeof(g_u2CountryGroup18) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 11, FALSE}
			,			/* CH_SET_2G4_1_11 */
			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_5G, CHNL_SPAN_20, 100, 5, TRUE}
			,			/* CH_SET_UNII_WW_100_116 */
			{121, BAND_5G, CHNL_SPAN_20, 132, 4, TRUE}
			,			/* CH_SET_UNII_WW_132_144 */
			{125, BAND_5G, CHNL_SPAN_20, 149, 5, FALSE}
						/* CH_SET_UNII_UPPER_149_165 */
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup19,
		sizeof(g_u2CountryGroup19) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */
			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_5G, CHNL_SPAN_20, 100, 12, TRUE}
			,			/* CH_SET_UNII_WW_100_144 */
			{125, BAND_5G, CHNL_SPAN_20, 149, 5, FALSE}
			,			/* CH_SET_UNII_UPPER_149_165 */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		/* Note: Default group if no matched country code */
		(uint16_t *) g_u2CountryGroup20,
		sizeof(g_u2CountryGroup20) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */

			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_5G, CHNL_SPAN_20, 100, 12, TRUE}
			,			/* CH_SET_UNII_WW_100_144 */
			{125, BAND_5G, CHNL_SPAN_20, 149, 5, FALSE}
			,			/* CH_SET_UNII_UPPER_149_165 */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
};

static const uint16_t g_u2CountryGroup0_Passive[] = {
	COUNTRY_CODE_TW
};

struct DOMAIN_INFO_ENTRY arSupportedRegDomains_Passive[] = {
	{
		(uint16_t *) g_u2CountryGroup0_Passive,
		sizeof(g_u2CountryGroup0_Passive) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 0, FALSE}
			,			/* CH_SET_2G4_1_14_NA */
			{82, BAND_2G4, CHNL_SPAN_5, 14, 0, FALSE}
			,

			{115, BAND_5G, CHNL_SPAN_20, 36, 0, FALSE}
			,			/* CH_SET_UNII_LOW_NA */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_5G, CHNL_SPAN_20, 100, 11, TRUE}
			,			/* CH_SET_UNII_WW_100_140 */
			{125, BAND_5G, CHNL_SPAN_20, 149, 0, FALSE}
			,			/* CH_SET_UNII_UPPER_NA */
		}
	}
	,
	{
		/* Default passive scan channel table: ch52~64, ch100~144 */
		NULL,
		0,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 0, FALSE}
			,			/* CH_SET_2G4_1_14_NA */
			{82, BAND_2G4, CHNL_SPAN_5, 14, 0, FALSE}
			,

			{115, BAND_5G, CHNL_SPAN_20, 36, 0, FALSE}
			,			/* CH_SET_UNII_LOW_NA */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_5G, CHNL_SPAN_20, 100, 12, TRUE}
			,			/* CH_SET_UNII_WW_100_144 */
			{125, BAND_5G, CHNL_SPAN_20, 149, 0, FALSE}
			,			/* CH_SET_UNII_UPPER_NA */
		}
	}
};

struct SUBBAND_CHANNEL g_rRlmSubBand[] = {

	{BAND_2G4_LOWER_BOUND, BAND_2G4_UPPER_BOUND, 1, 0}
	,			/* 2.4G */
	{UNII1_LOWER_BOUND, UNII1_UPPER_BOUND, 2, 0}
	,			/* ch36,38,40,..,48 */
	{UNII2A_LOWER_BOUND, UNII2A_UPPER_BOUND, 2, 0}
	,			/* ch52,54,56,..,64 */
	{UNII2C_LOWER_BOUND, UNII2C_UPPER_BOUND, 2, 0}
	,			/* ch100,102,104,...,144 */
	{UNII3_LOWER_BOUND, UNII3_UPPER_BOUND, 2, 0}
				/* ch149,151,153,....,165 */
};

/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */

/*******************************************************************************
 *                                 M A C R O S
 *******************************************************************************
 */

/*******************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */

/*----------------------------------------------------------------------------*/
/*!
 * \brief
 *
 * \param[in/out]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
struct DOMAIN_INFO_ENTRY *rlmDomainGetDomainInfo(struct ADAPTER *prAdapter)
{
#define REG_DOMAIN_DEF_IDX             20 /* Default regulatory domain */
#define REG_DOMAIN_GROUP_NUM \
	(sizeof(arSupportedRegDomains) / sizeof(struct DOMAIN_INFO_ENTRY))

	struct DOMAIN_INFO_ENTRY *prDomainInfo;
	struct REG_INFO *prRegInfo;
	uint16_t u2TargetCountryCode;
	uint16_t i, j;

	ASSERT(prAdapter);

	if (prAdapter->prDomainInfo)
		return prAdapter->prDomainInfo;

	prRegInfo = &prAdapter->prGlueInfo->rRegInfo;

	DBGLOG(RLM, TRACE, "eRegChannelListMap=%d, u2CountryCode=0x%04x\n",
			   prRegInfo->eRegChannelListMap,
			   prAdapter->rWifiVar.rConnSettings.u2CountryCode);

	/*
	 * Domain info can be specified by given idx of arSupportedRegDomains
	 * table, customized, or searched by country code,
	 * only one is set among these three methods in NVRAM.
	 */
	if (prRegInfo->eRegChannelListMap == REG_CH_MAP_TBL_IDX &&
	    prRegInfo->ucRegChannelListIndex < REG_DOMAIN_GROUP_NUM) {
		/* by given table idx */
		DBGLOG(RLM, TRACE, "ucRegChannelListIndex=%d\n",
		       prRegInfo->ucRegChannelListIndex);
		prDomainInfo = &arSupportedRegDomains
					[prRegInfo->ucRegChannelListIndex];
	} else if (prRegInfo->eRegChannelListMap == REG_CH_MAP_CUSTOMIZED) {
		/* by customized */
		prDomainInfo = &prRegInfo->rDomainInfo;
	} else {
		/* by country code */
		u2TargetCountryCode =
				prAdapter->rWifiVar.rConnSettings.u2CountryCode;

		for (i = 0; i < REG_DOMAIN_GROUP_NUM; i++) {
			prDomainInfo = &arSupportedRegDomains[i];

			if ((prDomainInfo->u4CountryNum &&
			     prDomainInfo->pu2CountryGroup) ||
			    prDomainInfo->u4CountryNum == 0) {
				for (j = 0;
				     j < prDomainInfo->u4CountryNum;
				     j++) {
					if (prDomainInfo->pu2CountryGroup[j] ==
							u2TargetCountryCode)
						break;
				}
				if (j < prDomainInfo->u4CountryNum)
					break;	/* Found */
			}
		}

		/* If no matched country code,
		 * use the default regulatory domain
		 */
		if (i >= REG_DOMAIN_GROUP_NUM) {
			DBGLOG(RLM, INFO,
			       "No matched country code, use the default regulatory domain\n");
			prDomainInfo = &arSupportedRegDomains
							[REG_DOMAIN_DEF_IDX];
		}
	}

	prAdapter->prDomainInfo = prDomainInfo;
	return prDomainInfo;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Retrieve the supported channel list of specified band
 *
 * \param[in/out] eSpecificBand:   BAND_2G4, BAND_5G or BAND_NULL
 *                                 (both 2.4G and 5G)
 *                fgNoDfs:         whether to exculde DFS channels
 *                ucMaxChannelNum: max array size
 *                pucNumOfChannel: pointer to returned channel number
 *                paucChannelList: pointer to returned channel list array
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void
rlmDomainGetChnlList_V2(struct ADAPTER *prAdapter,
			enum ENUM_BAND eSpecificBand, u_int8_t fgNoDfs,
			uint8_t ucMaxChannelNum, uint8_t *pucNumOfChannel,
			struct RF_CHANNEL_INFO *paucChannelList)
{
#if (CFG_SUPPORT_SINGLE_SKU == 1)
	enum ENUM_BAND band;
	uint8_t max_count, i, ucNum;
	struct CMD_DOMAIN_CHANNEL *prCh;

	if (eSpecificBand == BAND_2G4) {
		i = 0;
		max_count = rlmDomainGetActiveChannelCount(KAL_BAND_2GHZ);
	} else if (eSpecificBand == BAND_5G) {
		i = rlmDomainGetActiveChannelCount(KAL_BAND_2GHZ);
		max_count = rlmDomainGetActiveChannelCount(KAL_BAND_5GHZ) +
			rlmDomainGetActiveChannelCount(KAL_BAND_2GHZ);
	} else {
		i = 0;
		max_count = rlmDomainGetActiveChannelCount(KAL_BAND_5GHZ) +
			rlmDomainGetActiveChannelCount(KAL_BAND_2GHZ);
	}

	ucNum = 0;
	for (; i < max_count; i++) {
		prCh = rlmDomainGetActiveChannels() + i;
		if (fgNoDfs && (prCh->eFlags & IEEE80211_CHAN_RADAR))
			continue; /*not match*/

		if (i < rlmDomainGetActiveChannelCount(KAL_BAND_2GHZ))
			band = BAND_2G4;
		else
			band = BAND_5G;

		paucChannelList[ucNum].eBand = band;
		paucChannelList[ucNum].ucChannelNum = prCh->u2ChNum;

		ucNum++;
		if (ucMaxChannelNum == ucNum)
			break;
	}

	*pucNumOfChannel = ucNum;
#else
	*pucNumOfChannel = 0;
#endif
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Retrieve the supported channel list of specified band
 *
 * \param[in/out] eSpecificBand:   BAND_2G4, BAND_5G or BAND_NULL
 *                                 (both 2.4G and 5G)
 *                fgNoDfs:         whether to exculde DFS channels
 *                ucMaxChannelNum: max array size
 *                pucNumOfChannel: pointer to returned channel number
 *                paucChannelList: pointer to returned channel list array
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void
rlmDomainGetChnlList(struct ADAPTER *prAdapter,
		     enum ENUM_BAND eSpecificBand, u_int8_t fgNoDfs,
		     uint8_t ucMaxChannelNum, uint8_t *pucNumOfChannel,
		     struct RF_CHANNEL_INFO *paucChannelList)
{
	uint8_t i, j, ucNum;
	struct DOMAIN_SUBBAND_INFO *prSubband;
	struct DOMAIN_INFO_ENTRY *prDomainInfo;

	ASSERT(prAdapter);
	ASSERT(paucChannelList);
	ASSERT(pucNumOfChannel);

	if (regd_is_single_sku_en())
		return rlmDomainGetChnlList_V2(prAdapter, eSpecificBand,
					       fgNoDfs, ucMaxChannelNum,
					       pucNumOfChannel,
					       paucChannelList);

	/* If no matched country code, the final one will be used */
	prDomainInfo = rlmDomainGetDomainInfo(prAdapter);
	ASSERT(prDomainInfo);

	ucNum = 0;
	for (i = 0; i < MAX_SUBBAND_NUM; i++) {
		prSubband = &prDomainInfo->rSubBand[i];

		if (prSubband->ucBand == BAND_NULL ||
		    prSubband->ucBand >= BAND_NUM ||
		    (prSubband->ucBand == BAND_5G &&
		     !prAdapter->fgEnable5GBand))
			continue;

		/* repoert to upper layer only non-DFS channel
		 * for ap mode usage
		 */
		if (fgNoDfs == TRUE && prSubband->fgDfs == TRUE)
			continue;

		if (eSpecificBand == BAND_NULL ||
		    prSubband->ucBand == eSpecificBand) {
			for (j = 0; j < prSubband->ucNumChannels; j++) {
				if (ucNum >= ucMaxChannelNum)
					break;
				paucChannelList[ucNum].eBand =
							prSubband->ucBand;
				paucChannelList[ucNum].ucChannelNum =
				    prSubband->ucFirstChannelNum +
				    j * prSubband->ucChannelSpan;
				ucNum++;
			}
		}
	}

	*pucNumOfChannel = ucNum;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Retrieve DFS channels from 5G band
 *
 * \param[in/out] ucMaxChannelNum: max array size
 *                pucNumOfChannel: pointer to returned channel number
 *                paucChannelList: pointer to returned channel list array
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void rlmDomainGetDfsChnls(struct ADAPTER *prAdapter,
			  uint8_t ucMaxChannelNum, uint8_t *pucNumOfChannel,
			  struct RF_CHANNEL_INFO *paucChannelList)
{
	uint8_t i, j, ucNum;
	struct DOMAIN_SUBBAND_INFO *prSubband;
	struct DOMAIN_INFO_ENTRY *prDomainInfo;

	ASSERT(prAdapter);
	ASSERT(paucChannelList);
	ASSERT(pucNumOfChannel);

	prDomainInfo = rlmDomainGetDomainInfo(prAdapter);
	ASSERT(prDomainInfo);

	ucNum = 0;
	for (i = 0; i < MAX_SUBBAND_NUM; i++) {
		prSubband = &prDomainInfo->rSubBand[i];

		if (prSubband->ucBand == BAND_5G) {
			if (!prAdapter->fgEnable5GBand)
				continue;

			if (prSubband->fgDfs == TRUE) {
				for (j = 0; j < prSubband->ucNumChannels; j++) {
					if (ucNum >= ucMaxChannelNum)
						break;
					paucChannelList[ucNum].eBand =
					    prSubband->ucBand;
					paucChannelList[ucNum].ucChannelNum =
					    prSubband->ucFirstChannelNum + j *
					    prSubband->ucChannelSpan;
					ucNum++;
				}
			}
		}
	}

	*pucNumOfChannel = ucNum;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief
 *
 * @param[in]
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
void rlmDomainSendCmd(struct ADAPTER *prAdapter)
{
	if (!regd_is_single_sku_en())
		rlmDomainSendPassiveScanInfoCmd(prAdapter);
	rlmDomainSendDomainInfoCmd(prAdapter);
#if CFG_SUPPORT_PWR_LIMIT_COUNTRY
	rlmDomainSendPwrLimitCmd(prAdapter);
#endif
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief
 *
 * @param[in]
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
void rlmDomainSendDomainInfoCmd_V2(struct ADAPTER *prAdapter)
{
#if (CFG_SUPPORT_SINGLE_SKU == 1)
	u8 max_channel_count = 0;
	u32 buff_max_size, buff_valid_size;
	struct CMD_SET_DOMAIN_INFO_V2 *prCmd;
	struct CMD_DOMAIN_ACTIVE_CHANNEL_LIST *prChs;
	struct wiphy *pWiphy;


	pWiphy = priv_to_wiphy(prAdapter->prGlueInfo);
	if (pWiphy->bands[KAL_BAND_2GHZ] != NULL)
		max_channel_count += pWiphy->bands[KAL_BAND_2GHZ]->n_channels;
	if (pWiphy->bands[KAL_BAND_5GHZ] != NULL)
		max_channel_count += pWiphy->bands[KAL_BAND_5GHZ]->n_channels;

	if (max_channel_count == 0) {
		DBGLOG(RLM, ERROR, "%s, invalid channel count.\n", __func__);
		ASSERT(0);
	}


	buff_max_size = sizeof(struct CMD_SET_DOMAIN_INFO_V2) +
		max_channel_count * sizeof(struct CMD_DOMAIN_CHANNEL);

	prCmd = cnmMemAlloc(prAdapter, RAM_TYPE_BUF, buff_max_size);
	prChs = &(prCmd->arActiveChannels);


	/*
	 * Fill in the active channels
	 */
	rlmExtractChannelInfo(max_channel_count, prChs);

	prCmd->u4CountryCode = rlmDomainGetCountryCode();
	prCmd->uc2G4Bandwidth = prAdapter->rWifiVar.rConnSettings.
							uc2G4BandwidthMode;
	prCmd->uc5GBandwidth = prAdapter->rWifiVar.rConnSettings.
							uc5GBandwidthMode;
	prCmd->aucPadding[0] = 0;
	prCmd->aucPadding[1] = 0;

	buff_valid_size = sizeof(struct CMD_SET_DOMAIN_INFO_V2) +
		(prChs->u1ActiveChNum2g + prChs->u1ActiveChNum5g) *
		sizeof(struct CMD_DOMAIN_CHANNEL);

	DBGLOG(RLM, INFO,
	       "rlmDomainSendDomainInfoCmd_V2(), buff_valid_size = 0x%x\n",
	       buff_valid_size);


	/* Set domain info to chip */
	wlanSendSetQueryCmd(prAdapter, /* prAdapter */
			    CMD_ID_SET_DOMAIN_INFO, /* ucCID */
			    TRUE,  /* fgSetQuery */
			    FALSE, /* fgNeedResp */
			    FALSE, /* fgIsOid */
			    NULL,  /* pfCmdDoneHandler */
			    NULL,  /* pfCmdTimeoutHandler */
			    buff_valid_size,
			    (uint8_t *) prCmd, /* pucInfoBuffer */
			    NULL,  /* pvSetQueryBuffer */
			    0      /* u4SetQueryBufferLen */
	    );

	cnmMemFree(prAdapter, prCmd);
#endif
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief
 *
 * @param[in]
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
void rlmDomainSendDomainInfoCmd(struct ADAPTER *prAdapter)
{
	struct DOMAIN_INFO_ENTRY *prDomainInfo;
	struct CMD_SET_DOMAIN_INFO *prCmd;
	struct DOMAIN_SUBBAND_INFO *prSubBand;
	uint8_t i;

	if (regd_is_single_sku_en())
		return rlmDomainSendDomainInfoCmd_V2(prAdapter);


	prDomainInfo = rlmDomainGetDomainInfo(prAdapter);
	ASSERT(prDomainInfo);

	prCmd = cnmMemAlloc(prAdapter, RAM_TYPE_BUF,
			    sizeof(struct CMD_SET_DOMAIN_INFO));
	if (!prCmd) {
		DBGLOG(RLM, ERROR, "Alloc cmd buffer failed\n");
		return;
	}
	kalMemZero(prCmd, sizeof(struct CMD_SET_DOMAIN_INFO));

	prCmd->u2CountryCode = prAdapter->rWifiVar.rConnSettings.u2CountryCode;
	prCmd->u2IsSetPassiveScan = 0;
	prCmd->uc2G4Bandwidth = prAdapter->rWifiVar.rConnSettings.
							uc2G4BandwidthMode;
	prCmd->uc5GBandwidth = prAdapter->rWifiVar.rConnSettings.
							uc5GBandwidthMode;
	prCmd->aucReserved[0] = 0;
	prCmd->aucReserved[1] = 0;

	for (i = 0; i < MAX_SUBBAND_NUM; i++) {
		prSubBand = &prDomainInfo->rSubBand[i];

		prCmd->rSubBand[i].ucRegClass = prSubBand->ucRegClass;
		prCmd->rSubBand[i].ucBand = prSubBand->ucBand;

		if (prSubBand->ucBand != BAND_NULL && prSubBand->ucBand
								< BAND_NUM) {
			prCmd->rSubBand[i].ucChannelSpan
						= prSubBand->ucChannelSpan;
			prCmd->rSubBand[i].ucFirstChannelNum
						= prSubBand->ucFirstChannelNum;
			prCmd->rSubBand[i].ucNumChannels
						= prSubBand->ucNumChannels;
		}
	}

	/* Set domain info to chip */
	wlanSendSetQueryCmd(prAdapter, /* prAdapter */
		CMD_ID_SET_DOMAIN_INFO, /* ucCID */
		TRUE,  /* fgSetQuery */
		FALSE, /* fgNeedResp */
		FALSE, /* fgIsOid */
		NULL,  /* pfCmdDoneHandler */
		NULL,  /* pfCmdTimeoutHandler */
		sizeof(struct CMD_SET_DOMAIN_INFO), /* u4SetQueryInfoLen */
		(uint8_t *) prCmd, /* pucInfoBuffer */
		NULL,  /* pvSetQueryBuffer */
		0      /* u4SetQueryBufferLen */
	    );

	cnmMemFree(prAdapter, prCmd);
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief
 *
 * @param[in]
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
void rlmDomainSendPassiveScanInfoCmd(struct ADAPTER *prAdapter)
{
#define REG_DOMAIN_PASSIVE_DEF_IDX	1
#define REG_DOMAIN_PASSIVE_GROUP_NUM \
	(sizeof(arSupportedRegDomains_Passive)	\
	 / sizeof(struct DOMAIN_INFO_ENTRY))

	struct DOMAIN_INFO_ENTRY *prDomainInfo;
	struct CMD_SET_DOMAIN_INFO *prCmd;
	struct DOMAIN_SUBBAND_INFO *prSubBand;
	uint16_t u2TargetCountryCode;
	uint8_t i, j;

	prCmd = cnmMemAlloc(prAdapter, RAM_TYPE_BUF,
			    sizeof(struct CMD_SET_DOMAIN_INFO));
	if (!prCmd) {
		DBGLOG(RLM, ERROR, "Alloc cmd buffer failed\n");
		return;
	}
	kalMemZero(prCmd, sizeof(struct CMD_SET_DOMAIN_INFO));

	prCmd->u2CountryCode = prAdapter->rWifiVar.rConnSettings.u2CountryCode;
	prCmd->u2IsSetPassiveScan = 1;
	prCmd->uc2G4Bandwidth = prAdapter->rWifiVar.rConnSettings.
							uc2G4BandwidthMode;
	prCmd->uc5GBandwidth = prAdapter->rWifiVar.rConnSettings.
							uc5GBandwidthMode;
	prCmd->aucReserved[0] = 0;
	prCmd->aucReserved[1] = 0;

	DBGLOG(RLM, TRACE, "u2CountryCode=0x%04x\n",
	       prAdapter->rWifiVar.rConnSettings.u2CountryCode);

	u2TargetCountryCode = prAdapter->rWifiVar.rConnSettings.u2CountryCode;

	for (i = 0; i < REG_DOMAIN_PASSIVE_GROUP_NUM; i++) {
		prDomainInfo = &arSupportedRegDomains_Passive[i];

		for (j = 0; j < prDomainInfo->u4CountryNum; j++) {
			if (prDomainInfo->pu2CountryGroup[j] ==
						u2TargetCountryCode)
				break;
		}
		if (j < prDomainInfo->u4CountryNum)
			break;	/* Found */
	}

	if (i >= REG_DOMAIN_PASSIVE_GROUP_NUM)
		prDomainInfo = &arSupportedRegDomains_Passive
					[REG_DOMAIN_PASSIVE_DEF_IDX];

	for (i = 0; i < MAX_SUBBAND_NUM; i++) {
		prSubBand = &prDomainInfo->rSubBand[i];

		prCmd->rSubBand[i].ucRegClass = prSubBand->ucRegClass;
		prCmd->rSubBand[i].ucBand = prSubBand->ucBand;

		if (prSubBand->ucBand != BAND_NULL && prSubBand->ucBand
		    < BAND_NUM) {
			prCmd->rSubBand[i].ucChannelSpan =
						prSubBand->ucChannelSpan;
			prCmd->rSubBand[i].ucFirstChannelNum =
						prSubBand->ucFirstChannelNum;
			prCmd->rSubBand[i].ucNumChannels =
						prSubBand->ucNumChannels;
		}
	}

	/* Set passive scan channel info to chip */
	wlanSendSetQueryCmd(prAdapter, /* prAdapter */
		CMD_ID_SET_DOMAIN_INFO, /* ucCID */
		TRUE,  /* fgSetQuery */
		FALSE, /* fgNeedResp */
		FALSE, /* fgIsOid */
		NULL,  /* pfCmdDoneHandler */
		NULL,  /* pfCmdTimeoutHandler */
		sizeof(struct CMD_SET_DOMAIN_INFO), /* u4SetQueryInfoLen */
		(uint8_t *) prCmd, /* pucInfoBuffer */
		NULL,  /* pvSetQueryBuffer */
		0      /* u4SetQueryBufferLen */
	    );

	cnmMemFree(prAdapter, prCmd);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief
 *
 * \param[in/out]
 *
 * \return TRUE  Legal channel
 *         FALSE Illegal channel for current regulatory domain
 */
/*----------------------------------------------------------------------------*/
u_int8_t rlmDomainIsLegalChannel_V2(struct ADAPTER *prAdapter,
				    enum ENUM_BAND eBand,
				    uint8_t ucChannel)
{
#if (CFG_SUPPORT_SINGLE_SKU == 1)
	uint8_t idx, start_idx, end_idx;
	struct CMD_DOMAIN_CHANNEL *prCh;

	if (eBand == BAND_2G4) {
		start_idx = 0;
		end_idx = rlmDomainGetActiveChannelCount(KAL_BAND_2GHZ);
	} else {
		start_idx = rlmDomainGetActiveChannelCount(KAL_BAND_2GHZ);
		end_idx = rlmDomainGetActiveChannelCount(KAL_BAND_2GHZ) +
				rlmDomainGetActiveChannelCount(KAL_BAND_5GHZ);
	}

	for (idx = start_idx; idx < end_idx; idx++) {
		prCh = rlmDomainGetActiveChannels() + idx;

		if (prCh->u2ChNum == ucChannel)
			return TRUE;
	}

	return FALSE;
#else
	return FALSE;
#endif
}

u_int8_t rlmDomainIsLegalChannel(struct ADAPTER *prAdapter,
				 enum ENUM_BAND eBand, uint8_t ucChannel)
{
	uint8_t i, j;
	struct DOMAIN_SUBBAND_INFO *prSubband;
	struct DOMAIN_INFO_ENTRY *prDomainInfo;

	if (regd_is_single_sku_en())
		return rlmDomainIsLegalChannel_V2(prAdapter, eBand, ucChannel);


	prDomainInfo = rlmDomainGetDomainInfo(prAdapter);
	ASSERT(prDomainInfo);

	for (i = 0; i < MAX_SUBBAND_NUM; i++) {
		prSubband = &prDomainInfo->rSubBand[i];

		if (prSubband->ucBand == BAND_5G && !prAdapter->fgEnable5GBand)
			continue;

		if (prSubband->ucBand == eBand) {
			for (j = 0; j < prSubband->ucNumChannels; j++) {
				if ((prSubband->ucFirstChannelNum + j *
				    prSubband->ucChannelSpan) == ucChannel) {
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

u_int8_t rlmDomainIsLegalDfsChannel(struct ADAPTER *prAdapter,
		enum ENUM_BAND eBand, uint8_t ucChannel)
{
	uint8_t i, j;
	struct DOMAIN_SUBBAND_INFO *prSubband;
	struct DOMAIN_INFO_ENTRY *prDomainInfo;

	prDomainInfo = rlmDomainGetDomainInfo(prAdapter);
	ASSERT(prDomainInfo);

	for (i = 0; i < MAX_SUBBAND_NUM; i++) {
		prSubband = &prDomainInfo->rSubBand[i];

		if (prSubband->ucBand == BAND_5G
			&& !prAdapter->fgEnable5GBand)
			continue;

		if (prSubband->ucBand == eBand
			&& prSubband->fgDfs == TRUE) {
			for (j = 0; j < prSubband->ucNumChannels; j++) {
				if ((prSubband->ucFirstChannelNum + j *
					prSubband->ucChannelSpan)
					== ucChannel) {
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief
 *
 * \param[in/out]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/

uint32_t rlmDomainSupOperatingClassIeFill(uint8_t *pBuf)
{
	/*
	 *  The Country element should only be included for
	 *  Status Code 0 (Successful).
	 */
	uint32_t u4IeLen;
	uint8_t aucClass[12] = { 0x01, 0x02, 0x03, 0x05, 0x16, 0x17, 0x19, 0x1b,
		0x1c, 0x1e, 0x20, 0x21
	};

	/*
	 * The Supported Operating Classes element is used by a STA to
	 * advertise the operating classes that it is capable of operating
	 * with in this country.
	 * The Country element (see 8.4.2.10) allows a STA to configure its
	 * PHY and MAC for operation when the operating triplet of Operating
	 * Extension Identifier, Operating Class, and Coverage Class fields
	 * is present.
	 */
	SUP_OPERATING_CLASS_IE(pBuf)->ucId = ELEM_ID_SUP_OPERATING_CLASS;
	SUP_OPERATING_CLASS_IE(pBuf)->ucLength = 1 + sizeof(aucClass);
	SUP_OPERATING_CLASS_IE(pBuf)->ucCur = 0x0c;	/* 0x51 */
	kalMemCopy(SUP_OPERATING_CLASS_IE(pBuf)->ucSup, aucClass,
		   sizeof(aucClass));
	u4IeLen = (SUP_OPERATING_CLASS_IE(pBuf)->ucLength + 2);
	pBuf += u4IeLen;

	COUNTRY_IE(pBuf)->ucId = ELEM_ID_COUNTRY_INFO;
	COUNTRY_IE(pBuf)->ucLength = 6;
	COUNTRY_IE(pBuf)->aucCountryStr[0] = 0x55;
	COUNTRY_IE(pBuf)->aucCountryStr[1] = 0x53;
	COUNTRY_IE(pBuf)->aucCountryStr[2] = 0x20;
	COUNTRY_IE(pBuf)->arCountryStr[0].ucFirstChnlNum = 1;
	COUNTRY_IE(pBuf)->arCountryStr[0].ucNumOfChnl = 11;
	COUNTRY_IE(pBuf)->arCountryStr[0].cMaxTxPwrLv = 0x1e;
	u4IeLen += (COUNTRY_IE(pBuf)->ucLength + 2);

	return u4IeLen;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief
 *
 * @param[in]
 *
 * @return (fgValid) : 0 -> inValid, 1 -> Valid
 */
/*----------------------------------------------------------------------------*/
u_int8_t rlmDomainCheckChannelEntryValid(struct ADAPTER *prAdapter,
					 uint8_t ucCentralCh)
{
	u_int8_t fgValid = FALSE;
	uint8_t ucTemp = 0xff;
	uint8_t i;
	/*Check Power limit table channel efficient or not */

	/* CH50 is not located in any FCC subbands
	 * but it's a valid central channel for 160C
	 */
	if (ucCentralCh == 50) {
		fgValid = TRUE;
		return fgValid;
	}

	for (i = POWER_LIMIT_2G4; i < POWER_LIMIT_SUBAND_NUM; i++) {
		if ((ucCentralCh >= g_rRlmSubBand[i].ucStartCh) &&
				    (ucCentralCh <= g_rRlmSubBand[i].ucEndCh))
			ucTemp = (ucCentralCh - g_rRlmSubBand[i].ucStartCh) %
				 g_rRlmSubBand[i].ucInterval;
	}

	if (ucTemp == 0)
		fgValid = TRUE;
	return fgValid;

}

/*----------------------------------------------------------------------------*/
/*!
 * \brief
 *
 * \param[in]
 *
 * \return
 */
/*----------------------------------------------------------------------------*/
uint8_t rlmDomainGetCenterChannel(enum ENUM_BAND eBand, uint8_t ucPriChannel,
				  enum ENUM_CHNL_EXT eExtend)
{
	uint8_t ucCenterChannel;

	if (eExtend == CHNL_EXT_SCA)
		ucCenterChannel = ucPriChannel + 2;
	else if (eExtend == CHNL_EXT_SCB)
		ucCenterChannel = ucPriChannel - 2;
	else
		ucCenterChannel = ucPriChannel;

	return ucCenterChannel;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief
 *
 * \param[in]
 *
 * \return
 */
/*----------------------------------------------------------------------------*/
u_int8_t
rlmDomainIsValidRfSetting(struct ADAPTER *prAdapter,
			  enum ENUM_BAND eBand,
			  uint8_t ucPriChannel,
			  enum ENUM_CHNL_EXT eExtend,
			  enum ENUM_CHANNEL_WIDTH eChannelWidth,
			  uint8_t ucChannelS1, uint8_t ucChannelS2)
{
	uint8_t ucCenterCh = 0;
	uint8_t  ucUpperChannel;
	uint8_t  ucLowerChannel;
	u_int8_t fgValidChannel = TRUE;
	u_int8_t fgUpperChannel = TRUE;
	u_int8_t fgLowerChannel = TRUE;
	u_int8_t fgValidBW = TRUE;
	u_int8_t fgValidRfSetting = TRUE;
	uint32_t u4PrimaryOffset;

	/*DBG msg for Channel InValid */
	if (eChannelWidth == CW_20_40MHZ) {
		ucCenterCh = rlmDomainGetCenterChannel(eBand, ucPriChannel,
						       eExtend);

		/* Check Central Channel Valid or Not */
		fgValidChannel = rlmDomainCheckChannelEntryValid(prAdapter,
								 ucCenterCh);
		if (fgValidChannel == FALSE)
			DBGLOG(RLM, WARN, "Rf20: CentralCh=%d\n", ucCenterCh);

		/* Check Upper Channel and Lower Channel */
		switch (eExtend) {
		case CHNL_EXT_SCA:
			ucUpperChannel = ucPriChannel + 4;
			ucLowerChannel = ucPriChannel;
			break;
		case CHNL_EXT_SCB:
			ucUpperChannel = ucPriChannel;
			ucLowerChannel = ucPriChannel - 4;
			break;
		default:
			ucUpperChannel = ucPriChannel;
			ucLowerChannel = ucPriChannel;
			break;
		}

		fgUpperChannel = rlmDomainCheckChannelEntryValid(prAdapter,
								ucUpperChannel);
		if (fgUpperChannel == FALSE)
			DBGLOG(RLM, WARN, "Rf20: UpperCh=%d\n", ucUpperChannel);

		fgLowerChannel = rlmDomainCheckChannelEntryValid(prAdapter,
								ucLowerChannel);
		if (fgLowerChannel == FALSE)
			DBGLOG(RLM, WARN, "Rf20: LowerCh=%d\n", ucLowerChannel);

	} else if ((eChannelWidth == CW_80MHZ) ||
		   (eChannelWidth == CW_160MHZ)) {
		ucCenterCh = ucChannelS1;

		/* Check Central Channel Valid or Not */
		if (eChannelWidth != CW_160MHZ) {
			/* BW not check , ex: primary 36 and
			 * central channel 50 will fail the check
			 */
			fgValidChannel =
				rlmDomainCheckChannelEntryValid(prAdapter,
								ucCenterCh);
		}

		if (fgValidChannel == FALSE)
			DBGLOG(RLM, WARN, "Rf80/160C: CentralCh=%d\n",
			       ucCenterCh);
	} else if (eChannelWidth == CW_80P80MHZ) {
		ucCenterCh = ucChannelS1;

		fgValidChannel = rlmDomainCheckChannelEntryValid(prAdapter,
								 ucCenterCh);

		if (fgValidChannel == FALSE)
			DBGLOG(RLM, WARN, "Rf160NC: CentralCh1=%d\n",
			       ucCenterCh);

		ucCenterCh = ucChannelS2;

		fgValidChannel = rlmDomainCheckChannelEntryValid(prAdapter,
								 ucCenterCh);

		if (fgValidChannel == FALSE)
			DBGLOG(RLM, WARN, "Rf160NC: CentralCh2=%d\n",
			       ucCenterCh);

		/* Check Central Channel Valid or Not */
	} else {
		DBGLOG(RLM, ERROR, "Wrong BW =%d\n", eChannelWidth);
		fgValidChannel = FALSE;
	}

	/* Check BW Setting Correct or Not */
	if (eBand == BAND_2G4) {
		if (eChannelWidth != CW_20_40MHZ) {
			fgValidBW = FALSE;
			DBGLOG(RLM, WARN, "Rf: B=%d, W=%d\n",
			       eBand, eChannelWidth);
		}
	} else {
		if ((eChannelWidth == CW_80MHZ) ||
				(eChannelWidth == CW_80P80MHZ)) {
			u4PrimaryOffset = CAL_CH_OFFSET_80M(ucPriChannel,
							    ucChannelS1);
			if (u4PrimaryOffset > 4) {
				fgValidBW = FALSE;
				DBGLOG(RLM, WARN, "Rf: PriOffSet=%d, W=%d\n",
				       u4PrimaryOffset, eChannelWidth);
			}
			if (ucPriChannel == 165) {
				fgValidBW = FALSE;
				DBGLOG(RLM, WARN,
				       "Rf: PriOffSet=%d, W=%d C=%d\n",
				       u4PrimaryOffset, eChannelWidth,
				       ucPriChannel);
			}
		} else if (eChannelWidth == CW_160MHZ) {
			u4PrimaryOffset = CAL_CH_OFFSET_160M(ucPriChannel,
							     ucCenterCh);
			if (u4PrimaryOffset > 8) {
				fgValidBW = FALSE;
				DBGLOG(RLM, WARN,
				       "Rf: PriOffSet=%d, W=%d\n",
				       u4PrimaryOffset, eChannelWidth);
			}
		}
	}

	if ((fgValidBW == FALSE) || (fgValidChannel == FALSE) ||
	    (fgUpperChannel == FALSE) || (fgLowerChannel == FALSE))
		fgValidRfSetting = FALSE;

	return fgValidRfSetting;

}

#if (CFG_SUPPORT_SINGLE_SKU == 1)
/*
 * This function coverts country code from alphabet chars to u32,
 * the caller need to pass country code chars and do size check
 */
uint32_t rlmDomainAlpha2ToU32(uint8_t *pcAlpha2, uint8_t ucAlpha2Size)
{
	uint8_t ucIdx;
	uint32_t u4CountryCode = 0;

	if (ucAlpha2Size > TX_PWR_LIMIT_COUNTRY_STR_MAX_LEN) {
		DBGLOG(RLM, ERROR, "alpha2 size %d is invalid!(max: %d)\n",
			ucAlpha2Size, TX_PWR_LIMIT_COUNTRY_STR_MAX_LEN);
		ucAlpha2Size = TX_PWR_LIMIT_COUNTRY_STR_MAX_LEN;
	}

	for (ucIdx = 0; ucIdx < ucAlpha2Size; ucIdx++)
		u4CountryCode |= (pcAlpha2[ucIdx] << (ucIdx * 8));

	return u4CountryCode;
}

uint8_t rlmDomainTxPwrLimitGetTableVersion(
	uint8_t *pucBuf, uint32_t u4BufLen)
{
#define TX_PWR_LIMIT_VERSION_STR_LEN 7
#define TX_PWR_LIMIT_MAX_VERSION 1
	uint32_t u4TmpPos = 0;
	uint8_t ucVersion = 0;

	while (u4TmpPos < u4BufLen && pucBuf[u4TmpPos] != '<')
		u4TmpPos++;

	if (u4TmpPos >= (u4BufLen - TX_PWR_LIMIT_VERSION_STR_LEN))
		return ucVersion;

	if (kalStrnCmp(&pucBuf[u4TmpPos + 1], "Ver:", 4) == 0) {
		ucVersion = (pucBuf[u4TmpPos + 5] - '0') * 10 +
			(pucBuf[u4TmpPos + 6] - '0');
	}

	if (ucVersion > TX_PWR_LIMIT_MAX_VERSION)
		ucVersion = 0;

	return ucVersion;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Search the tx power limit setting range of the specified in the text
 *        file
 *
 * \param[IN] u4CountryCode The u32 type of the specified country.
 * \param[IN] pucBuf The content of the text file.
 * \param[IN] u4cBufLen End boundary of the text file.
 * \param[OUT] pu4CountryStart Store the start position of the desired country
 *             settings.
 * \param[OUT] pu4CountryEnd Store the end position of the desired country
 *             settings.
 *
 * \retval TRUE Success.
 * \retval FALSE Failure.
 */
/*----------------------------------------------------------------------------*/
u_int8_t rlmDomainTxPwrLimitGetCountryRange(
	uint32_t u4CountryCode, uint8_t *pucBuf, uint32_t u4BufLen,
	uint32_t *pu4CountryStart, uint32_t *pu4CountryEnd)
{
	uint32_t u4TmpPos = 0;
	char pcrCountryStr[TX_PWR_LIMIT_COUNTRY_STR_MAX_LEN + 1] = {0};
	uint8_t cIdx = 0;

	while (1) {
		while (u4TmpPos < u4BufLen && pucBuf[u4TmpPos] != '[')
			u4TmpPos++;

		u4TmpPos++; /* skip the '[' char */

		cIdx = 0;
		while ((u4TmpPos < u4BufLen) &&
			   (cIdx < TX_PWR_LIMIT_COUNTRY_STR_MAX_LEN) &&
			   (pucBuf[u4TmpPos] != ']')) {
			pcrCountryStr[cIdx++] = pucBuf[u4TmpPos];
			u4TmpPos++;
		}

		u4TmpPos++; /* skip the ']' char */

		if ((u4TmpPos >= u4BufLen) ||
		    (cIdx > TX_PWR_LIMIT_COUNTRY_STR_MAX_LEN))
			return FALSE;

		if (u4CountryCode ==
			rlmDomainAlpha2ToU32(pcrCountryStr, cIdx)) {
			DBGLOG(RLM, INFO,
				"Found TxPwrLimit table for CountryCode \"%s\"\n",
				pcrCountryStr);
			*pu4CountryStart = u4TmpPos;
			/* the location after char ']' */
			break;
		}
	}

	while (u4TmpPos < u4BufLen && pucBuf[u4TmpPos] != '[')
		u4TmpPos++;

	*pu4CountryEnd = u4TmpPos;

	return TRUE;
}

u_int8_t rlmDomainTxPwrLimitSearchSection(const char *pSectionName,
	uint8_t *pucBuf, uint32_t *pu4Pos, uint32_t u4BufEnd)
{
	uint32_t u4TmpPos = *pu4Pos;
	uint8_t uSectionNameLen = kalStrLen(pSectionName);

	while (1) {
		while (u4TmpPos < u4BufEnd && pucBuf[u4TmpPos] != '<')
			u4TmpPos++;

		u4TmpPos++; /* skip char '<' */

		if (u4TmpPos + uSectionNameLen >= u4BufEnd)
			return FALSE;

		if (kalStrnCmp(&pucBuf[u4TmpPos],
				pSectionName, uSectionNameLen) == 0) {

			/* Go to the end of section header line */
			while ((u4TmpPos < u4BufEnd) &&
				   (pucBuf[u4TmpPos] != '\n'))
				u4TmpPos++;

			*pu4Pos = u4TmpPos;

			break;
		}
	}

	return TRUE;
}

u_int8_t rlmDomainTxPwrLimitSectionEnd(uint8_t *pucBuf,
	const char *pSectionName, uint32_t *pu4Pos, uint32_t u4BufEnd)
{
	uint32_t u4TmpPos = *pu4Pos;
	char cTmpChar = 0;
	uint8_t uSectionNameLen = kalStrLen(pSectionName);

	while (u4TmpPos < u4BufEnd) {
		cTmpChar = pucBuf[u4TmpPos];

		/* skip blank lines */
		if (cTmpChar == ' ' || cTmpChar == '\t' ||
			cTmpChar == '\n' || cTmpChar == '\r') {
			u4TmpPos++;
			continue;
		}

		break;
	}

	/* 2 means '/' and '>' */
	if (u4TmpPos + uSectionNameLen + 2 >= u4BufEnd) {
		*pu4Pos = u4BufEnd;
		return FALSE;
	}

	if (pucBuf[u4TmpPos] != '<')
		return FALSE;

	if (pucBuf[u4TmpPos + 1] != '/' ||
		pucBuf[u4TmpPos + 2 + uSectionNameLen] != '>' ||
		kalStrnCmp(&pucBuf[u4TmpPos + 2],
			pSectionName, uSectionNameLen)) {

		*pu4Pos = u4TmpPos + uSectionNameLen + 2;
		return FALSE;
	}

	/* 3 means go to the location after '>' */
	*pu4Pos = u4TmpPos + uSectionNameLen + 3;
	return TRUE;
}

int8_t rlmDomainTxPwrLimitGetChIdx(
	struct TX_PWR_LIMIT_DATA *pTxPwrLimit, uint8_t ucChannel)
{
	int8_t cIdx = 0;

	for (cIdx = 0; cIdx < pTxPwrLimit->ucChNum; cIdx++)
		if (ucChannel ==
			pTxPwrLimit->rChannelTxPwrLimit[cIdx].ucChannel)
			return cIdx;

	DBGLOG(RLM, ERROR,
		"Can't find idx of channel %d in TxPwrLimit data\n",
		ucChannel);

	return -1;
}

u_int8_t rlmDomainTxPwrLimitLoadChannelSetting(
	uint8_t ucVersion, uint8_t *pucBuf, uint32_t *pu4Pos, uint32_t u4BufEnd,
	struct TX_PWR_LIMIT_DATA *pTxPwrLimit, uint8_t ucSectionIdx)
{
	uint32_t u4TmpPos = *pu4Pos;
	char cTmpChar = 0;
	struct CHANNEL_TX_PWR_LIMIT *prChTxPwrLimit = NULL;
	u_int8_t bNeg = FALSE;
	int8_t cLimitValue = 0, cChIdx = 0;
	uint8_t ucIdx = 0, ucChannel = 0;
	uint8_t ucElementNum =
		gTx_Pwr_Limit_Element_Num[ucVersion][ucSectionIdx];

	/* skip blank lines */
	while (u4TmpPos < u4BufEnd) {
		cTmpChar = pucBuf[u4TmpPos];

		if (cTmpChar == ' ' || cTmpChar == '\t' ||
			cTmpChar == '\n' || cTmpChar == '\r') {
			u4TmpPos++;
			continue;
		}

		break;
	}

	/* current is at the location of 'c',
	 * check remaining buf length for 'chxxx'
	 */
	if (u4TmpPos + 5 >= u4BufEnd) {
		DBGLOG(RLM, ERROR,
			"Invalid location of ch setting: %u/%u\n",
			u4TmpPos, u4BufEnd);
		return FALSE;
	}

	if (pucBuf[u4TmpPos] == 'c' && pucBuf[u4TmpPos + 1] == 'h') {
		ucChannel = (pucBuf[u4TmpPos + 2] - '0') * 100 +
					(pucBuf[u4TmpPos + 3] - '0') * 10 +
					(pucBuf[u4TmpPos + 4] - '0');
	} else { /* invalid format */
		*pu4Pos = u4TmpPos;
		DBGLOG(RLM, ERROR,
			"Invalid ch setting starting chars: %c%c\n",
			pucBuf[u4TmpPos], pucBuf[u4TmpPos + 1]);

		/* goto next line */
		while (*pu4Pos < u4BufEnd && pucBuf[*pu4Pos] != '\n')
			(*pu4Pos)++;

		return TRUE;
	}

	cChIdx = rlmDomainTxPwrLimitGetChIdx(pTxPwrLimit, ucChannel);

	if (cChIdx == -1) {
		*pu4Pos = u4TmpPos;
		DBGLOG(RLM, ERROR, "Invalid ch %u %c%c%c\n", ucChannel,
			pucBuf[u4TmpPos + 2],
			pucBuf[u4TmpPos + 3], pucBuf[u4TmpPos + 4]);

		/* goto next line */
		while (*pu4Pos < u4BufEnd && pucBuf[*pu4Pos] != '\n')
			(*pu4Pos)++;

		return TRUE;
	}

	u4TmpPos += 5;

	prChTxPwrLimit = &pTxPwrLimit->rChannelTxPwrLimit[cChIdx];

	/* read the channel TxPwrLimit settings */
	for (ucIdx = 0; ucIdx < ucElementNum; ucIdx++) {

		/* skip blank and comma */
		while (u4TmpPos < u4BufEnd) {
			cTmpChar = pucBuf[u4TmpPos];

			if ((cTmpChar == ' ') ||
				(cTmpChar == '\t') ||
				(cTmpChar == ',')) {
				u4TmpPos++;
				continue;
			}
			break;
		}

		if (u4TmpPos >= u4BufEnd) {
			*pu4Pos = u4BufEnd;
			DBGLOG(RLM, ERROR,
				"Invalid location of ch tx pwr limit val: %u/%u\n",
				u4TmpPos, u4BufEnd);
			return FALSE;
		}

		bNeg = FALSE;

		cTmpChar = pucBuf[u4TmpPos];

		if (cTmpChar == '-') {
			bNeg = TRUE;
			u4TmpPos++;
		} else if (cTmpChar == 'x') {
			prChTxPwrLimit->rTxPwrLimitValue[ucSectionIdx][ucIdx] =
				TX_PWR_LIMIT_MAX_VAL;
			u4TmpPos++;
			continue;
		}

		cLimitValue = 0;
		while (u4TmpPos < u4BufEnd) {
			cTmpChar = pucBuf[u4TmpPos];

			if (cTmpChar < '0' || cTmpChar > '9')
				break;

			cLimitValue = (cLimitValue * 10) + (cTmpChar - '0');
			u4TmpPos++;
		}

		if (bNeg)
			cLimitValue = -cLimitValue;

		prChTxPwrLimit->rTxPwrLimitValue[ucSectionIdx][ucIdx] =
			cLimitValue;
	}

	*pu4Pos = u4TmpPos;
	return TRUE;
}

void rlmDomainTxPwrLimitRemoveComments(
	uint8_t *pucBuf, uint32_t u4BufLen)
{
	uint32_t u4TmpPos = 0;
	char cTmpChar = 0;

	while (u4TmpPos < u4BufLen) {
		cTmpChar = pucBuf[u4TmpPos];

		if (cTmpChar == '#') {
			while (cTmpChar != '\n') {
				pucBuf[u4TmpPos] = ' ';

				u4TmpPos++;
				if (u4TmpPos >= u4BufLen)
					break;

				cTmpChar = pucBuf[u4TmpPos];
			}
		}
		u4TmpPos++;
	}
}

u_int8_t rlmDomainTxPwrLimitLoad(
	struct ADAPTER *prAdapter, uint8_t *pucBuf, uint32_t u4BufLen,
	uint8_t ucVersion, uint32_t u4CountryCode,
	struct TX_PWR_LIMIT_DATA *pTxPwrLimitData)
{
	uint8_t uSecIdx = 0;
	uint8_t ucSecNum = gTx_Pwr_Limit_Section[ucVersion].ucSectionNum;
	uint32_t u4CountryStart = 0, u4CountryEnd = 0, u4Pos = 0;
	struct TX_PWR_LIMIT_SECTION *prSection =
		&gTx_Pwr_Limit_Section[ucVersion];
	uint8_t *prFileName = prAdapter->chip_info->prTxPwrLimitFile;

	if (!rlmDomainTxPwrLimitGetCountryRange(u4CountryCode, pucBuf,
		u4BufLen, &u4CountryStart, &u4CountryEnd)) {
		DBGLOG(RLM, ERROR, "Can't find specified table in %s\n",
			prFileName);
		return FALSE;
	}
	u4Pos = u4CountryStart;

	for (uSecIdx = 0; uSecIdx < ucSecNum; uSecIdx++) {
		const uint8_t *pSecName =
			prSection->arSectionNames[uSecIdx];
		if (!rlmDomainTxPwrLimitSearchSection(
				pSecName, pucBuf, &u4Pos,
				u4CountryEnd)) {
			DBGLOG(RLM, ERROR,
				"Can't find specified section %s in %s\n",
				pSecName,
				prFileName);
			continue;
		}

		DBGLOG(RLM, INFO, "Find specified section %s in %s\n",
			pSecName,
			prFileName);

		while (!rlmDomainTxPwrLimitSectionEnd(pucBuf,
			pSecName,
			&u4Pos, u4CountryEnd) &&
			u4Pos < u4CountryEnd) {
			if (!rlmDomainTxPwrLimitLoadChannelSetting(
				ucVersion, pucBuf, &u4Pos, u4CountryEnd,
				pTxPwrLimitData, uSecIdx))
				return FALSE;
		}
	}

	DBGLOG(RLM, INFO, "Load %s finished\n", prFileName);
	return TRUE;
}

void rlmDomainTxPwrLimitSetChValues(
	uint8_t ucVersion,
	struct CMD_CHANNEL_POWER_LIMIT_V2 *pCmd,
	struct CHANNEL_TX_PWR_LIMIT *pChTxPwrLimit)
{
	uint8_t section = 0, e = 0;
	uint8_t ucElementNum = 0;

	pCmd->tx_pwr_dsss_cck = pChTxPwrLimit->rTxPwrLimitValue[0][0];
	pCmd->tx_pwr_dsss_bpsk = pChTxPwrLimit->rTxPwrLimitValue[0][1];

	/* 6M, 9M */
	pCmd->tx_pwr_ofdm_bpsk = pChTxPwrLimit->rTxPwrLimitValue[0][2];
	/* 12M, 18M */
	pCmd->tx_pwr_ofdm_qpsk = pChTxPwrLimit->rTxPwrLimitValue[0][3];
	/* 24M, 36M */
	pCmd->tx_pwr_ofdm_16qam = pChTxPwrLimit->rTxPwrLimitValue[0][4];
	pCmd->tx_pwr_ofdm_48m = pChTxPwrLimit->rTxPwrLimitValue[0][5];
	pCmd->tx_pwr_ofdm_54m = pChTxPwrLimit->rTxPwrLimitValue[0][6];

	/* MCS0*/
	pCmd->tx_pwr_ht20_bpsk = pChTxPwrLimit->rTxPwrLimitValue[1][0];
	/* MCS1, MCS2*/
	pCmd->tx_pwr_ht20_qpsk = pChTxPwrLimit->rTxPwrLimitValue[1][1];
	/* MCS3, MCS4*/
	pCmd->tx_pwr_ht20_16qam = pChTxPwrLimit->rTxPwrLimitValue[1][2];
	/* MCS5*/
	pCmd->tx_pwr_ht20_mcs5 = pChTxPwrLimit->rTxPwrLimitValue[1][3];
	/* MCS6*/
	pCmd->tx_pwr_ht20_mcs6 = pChTxPwrLimit->rTxPwrLimitValue[1][4];
	/* MCS7*/
	pCmd->tx_pwr_ht20_mcs7 = pChTxPwrLimit->rTxPwrLimitValue[1][5];

	/* MCS0*/
	pCmd->tx_pwr_ht40_bpsk = pChTxPwrLimit->rTxPwrLimitValue[2][0];
	/* MCS1, MCS2*/
	pCmd->tx_pwr_ht40_qpsk = pChTxPwrLimit->rTxPwrLimitValue[2][1];
	/* MCS3, MCS4*/
	pCmd->tx_pwr_ht40_16qam = pChTxPwrLimit->rTxPwrLimitValue[2][2];
	/* MCS5*/
	pCmd->tx_pwr_ht40_mcs5 = pChTxPwrLimit->rTxPwrLimitValue[2][3];
	/* MCS6*/
	pCmd->tx_pwr_ht40_mcs6 = pChTxPwrLimit->rTxPwrLimitValue[2][4];
	/* MCS7*/
	pCmd->tx_pwr_ht40_mcs7 = pChTxPwrLimit->rTxPwrLimitValue[2][5];
	/* MCS32*/
	pCmd->tx_pwr_ht40_mcs32 = pChTxPwrLimit->rTxPwrLimitValue[2][6];

	/* MCS0*/
	pCmd->tx_pwr_vht20_bpsk = pChTxPwrLimit->rTxPwrLimitValue[3][0];
	/* MCS1, MCS2*/
	pCmd->tx_pwr_vht20_qpsk = pChTxPwrLimit->rTxPwrLimitValue[3][1];
	/* MCS3, MCS4*/
	pCmd->tx_pwr_vht20_16qam = pChTxPwrLimit->rTxPwrLimitValue[3][2];
	/* MCS5, MCS6*/
	pCmd->tx_pwr_vht20_64qam = pChTxPwrLimit->rTxPwrLimitValue[3][3];
	pCmd->tx_pwr_vht20_mcs7 = pChTxPwrLimit->rTxPwrLimitValue[3][4];
	pCmd->tx_pwr_vht20_mcs8 = pChTxPwrLimit->rTxPwrLimitValue[3][5];
	pCmd->tx_pwr_vht20_mcs9 = pChTxPwrLimit->rTxPwrLimitValue[3][6];

	pCmd->tx_pwr_vht_40 = pChTxPwrLimit->rTxPwrLimitValue[4][2];
	pCmd->tx_pwr_vht_80 = pChTxPwrLimit->rTxPwrLimitValue[4][3];
	pCmd->tx_pwr_vht_160c = pChTxPwrLimit->rTxPwrLimitValue[4][5];
	pCmd->tx_pwr_vht_160nc = pChTxPwrLimit->rTxPwrLimitValue[4][4];
	pCmd->tx_pwr_lg_40 = pChTxPwrLimit->rTxPwrLimitValue[4][0];
	pCmd->tx_pwr_lg_80 = pChTxPwrLimit->rTxPwrLimitValue[4][1];


	DBGLOG(RLM, TRACE, "ch %d\n", pCmd->ucCentralCh);
	for (section = 0; section < TX_PWR_LIMIT_SECTION_NUM; section++) {
		struct TX_PWR_LIMIT_SECTION *pSection =
			&gTx_Pwr_Limit_Section[ucVersion];
		ucElementNum = gTx_Pwr_Limit_Element_Num[ucVersion][section];
		for (e = 0; e < ucElementNum; e++)
			DBGLOG(RLM, TRACE, "TxPwrLimit[%s][%s]= %d\n",
				pSection->arSectionNames[section],
				gTx_Pwr_Limit_Element[ucVersion][section][e],
				pChTxPwrLimit->rTxPwrLimitValue[section][e]);
	}
}

void rlmDomainTxPwrLimitPerRateSetChValues(
	uint8_t ucVersion,
	struct CMD_TXPOWER_CHANNEL_POWER_LIMIT_PER_RATE *pCmd,
	struct CHANNEL_TX_PWR_LIMIT *pChTxPwrLimit)
{
	uint8_t section = 0, e = 0, count = 0;
	uint8_t ucElementNum = 0;

	for (section = 0; section < TX_PWR_LIMIT_SECTION_NUM; section++) {
		ucElementNum = gTx_Pwr_Limit_Element_Num[ucVersion][section];
		for (e = 0; e < ucElementNum; e++) {
			pCmd->aucTxPwrLimit.i1PwrLimit[count] =
				pChTxPwrLimit->rTxPwrLimitValue[section][e];
			count++;
		}
	}

	DBGLOG(RLM, TRACE, "ch %d\n", pCmd->u1CentralCh);
	count = 0;
	for (section = 0; section < TX_PWR_LIMIT_SECTION_NUM; section++) {
		struct TX_PWR_LIMIT_SECTION *pSection =
			&gTx_Pwr_Limit_Section[ucVersion];
		ucElementNum = gTx_Pwr_Limit_Element_Num[ucVersion][section];
		for (e = 0; e < ucElementNum; e++) {
			DBGLOG(RLM, TRACE, "TxPwrLimit[%s][%s]= %d\n",
				pSection->arSectionNames[section],
				gTx_Pwr_Limit_Element[ucVersion][section][e],
				pCmd->aucTxPwrLimit.i1PwrLimit[count]);
			count++;
		}
	}
}

void rlmDomainTxPwrLimitSetValues(
	uint8_t ucVersion,
	struct CMD_SET_COUNTRY_CHANNEL_POWER_LIMIT_V2 *pSetCmd,
	struct TX_PWR_LIMIT_DATA *pTxPwrLimit)
{
	uint8_t ucIdx = 0;
	int8_t cChIdx = 0;
	struct CMD_CHANNEL_POWER_LIMIT_V2 *pCmd = NULL;
	struct CHANNEL_TX_PWR_LIMIT *pChTxPwrLimit = NULL;

	if (!pSetCmd || !pTxPwrLimit) {
		DBGLOG(RLM, ERROR, "Invalid TxPwrLimit request\n");
		return;
	}

	for (ucIdx = 0; ucIdx < pSetCmd->ucNum; ucIdx++) {
		pCmd = &(pSetCmd->rChannelPowerLimit[ucIdx]);
		cChIdx = rlmDomainTxPwrLimitGetChIdx(pTxPwrLimit,
			pCmd->ucCentralCh);
		if (cChIdx == -1) {
			DBGLOG(RLM, ERROR,
				"Invalid ch idx found while assigning values\n");
			continue;
		}
		pChTxPwrLimit = &pTxPwrLimit->rChannelTxPwrLimit[cChIdx];
		rlmDomainTxPwrLimitSetChValues(ucVersion, pCmd, pChTxPwrLimit);
	}
}

void rlmDomainTxPwrLimitPerRateSetValues(
	uint8_t ucVersion,
	struct CMD_SET_TXPOWER_COUNTRY_TX_POWER_LIMIT_PER_RATE *pSetCmd,
	struct TX_PWR_LIMIT_DATA *pTxPwrLimit)
{
	uint8_t ucIdx = 0;
	int8_t cChIdx = 0;
	struct CMD_TXPOWER_CHANNEL_POWER_LIMIT_PER_RATE *pChPwrLimit = NULL;
	struct CHANNEL_TX_PWR_LIMIT *pChTxPwrLimit = NULL;

	for (ucIdx = 0; ucIdx < pSetCmd->ucNum; ucIdx++) {
		pChPwrLimit = &(pSetCmd->rChannelPowerLimit[ucIdx]);
		cChIdx = rlmDomainTxPwrLimitGetChIdx(pTxPwrLimit,
			pChPwrLimit->u1CentralCh);

		if (cChIdx == -1) {
			DBGLOG(RLM, ERROR,
				"Invalid ch idx found while assigning values\n");
			continue;
		}
		pChTxPwrLimit = &pTxPwrLimit->rChannelTxPwrLimit[cChIdx];
		rlmDomainTxPwrLimitPerRateSetChValues(ucVersion,
			pChPwrLimit, pChTxPwrLimit);
	}
}

u_int8_t rlmDomainTxPwrLimitLoadFromFile(
	struct ADAPTER *prAdapter,
	uint8_t *pucConfigBuf, uint32_t *pu4ConfigReadLen)
{
#define TXPWRLIMIT_FILE_LEN 64
	u_int8_t bRet = TRUE;
	uint8_t *prFileName = prAdapter->chip_info->prTxPwrLimitFile;
	uint8_t aucPath[4][TXPWRLIMIT_FILE_LEN];

	kalMemZero(aucPath, sizeof(aucPath));
	kalSnprintf(aucPath[0], TXPWRLIMIT_FILE_LEN, "%s", prFileName);
	kalSnprintf(aucPath[1], TXPWRLIMIT_FILE_LEN,
		"/data/misc/%s", prFileName);
	kalSnprintf(aucPath[2], TXPWRLIMIT_FILE_LEN,
		"/data/misc/wifi/%s", prFileName);
	kalSnprintf(aucPath[3], TXPWRLIMIT_FILE_LEN,
		"/storage/sdcard0/%s", prFileName);

	kalMemZero(pucConfigBuf, WLAN_TX_PWR_LIMIT_FILE_BUF_SIZE);
	*pu4ConfigReadLen = 0;

	if (wlanGetFileContent(
			prAdapter,
			aucPath[0],
			pucConfigBuf,
			WLAN_TX_PWR_LIMIT_FILE_BUF_SIZE,
			pu4ConfigReadLen, TRUE) == 0) {
		/* ToDo:: Nothing */
	} else if (wlanGetFileContent(
				prAdapter,
				aucPath[1],
				pucConfigBuf,
				WLAN_TX_PWR_LIMIT_FILE_BUF_SIZE,
				pu4ConfigReadLen, FALSE) == 0) {
		/* ToDo:: Nothing */
	} else if (wlanGetFileContent(
				prAdapter,
				aucPath[2],
				pucConfigBuf,
				WLAN_TX_PWR_LIMIT_FILE_BUF_SIZE,
				pu4ConfigReadLen, FALSE) == 0) {
		/* ToDo:: Nothing */
	} else if (wlanGetFileContent(
				prAdapter,
				aucPath[3],
				pucConfigBuf,
				WLAN_TX_PWR_LIMIT_FILE_BUF_SIZE,
				pu4ConfigReadLen, FALSE) == 0) {
		/* ToDo:: Nothing */
	} else {
		bRet = FALSE;
		goto error;
	}

	if (pucConfigBuf[0] == '\0' || *pu4ConfigReadLen == 0) {
		bRet = FALSE;
		goto error;
	}

error:

	return bRet;
}

u_int8_t rlmDomainGetTxPwrLimit(
	uint32_t country_code,
	uint8_t *pucVersion,
	struct GLUE_INFO *prGlueInfo,
	struct TX_PWR_LIMIT_DATA *pTxPwrLimitData)
{
	u_int8_t bRet = TRUE;
	uint8_t *pucConfigBuf = NULL;
	uint32_t u4ConfigReadLen = 0;

	pucConfigBuf = (uint8_t *) kalMemAlloc(
		WLAN_TX_PWR_LIMIT_FILE_BUF_SIZE, VIR_MEM_TYPE);

	if (!pucConfigBuf)
		return FALSE;

	bRet = rlmDomainTxPwrLimitLoadFromFile(prGlueInfo->prAdapter,
		pucConfigBuf, &u4ConfigReadLen);

	rlmDomainTxPwrLimitRemoveComments(pucConfigBuf, u4ConfigReadLen);
	*pucVersion = rlmDomainTxPwrLimitGetTableVersion(pucConfigBuf,
		u4ConfigReadLen);

	if (!rlmDomainTxPwrLimitLoad(prGlueInfo->prAdapter,
		pucConfigBuf, u4ConfigReadLen, *pucVersion,
		country_code, pTxPwrLimitData)) {
		bRet = FALSE;
		goto error;
	}

error:

	kalMemFree(pucConfigBuf,
		VIR_MEM_TYPE, WLAN_TX_PWR_LIMIT_FILE_BUF_SIZE);

	return bRet;
}

#endif

#if CFG_SUPPORT_PWR_LIMIT_COUNTRY

/*----------------------------------------------------------------------------*/
/*!
 * @brief Check if power limit setting is in the range [MIN_TX_POWER,
 *        MAX_TX_POWER]
 *
 * @param[in]
 *
 * @return (fgValid) : 0 -> inValid, 1 -> Valid
 */
/*----------------------------------------------------------------------------*/
u_int8_t
rlmDomainCheckPowerLimitValid(struct ADAPTER *prAdapter,
			      struct COUNTRY_POWER_LIMIT_TABLE_CONFIGURATION
						rPowerLimitTableConfiguration,
			      uint8_t ucPwrLimitNum)
{
	uint8_t i;
	u_int8_t fgValid = TRUE;
	int8_t *prPwrLimit;

	prPwrLimit = &rPowerLimitTableConfiguration.aucPwrLimit[0];

	for (i = 0; i < ucPwrLimitNum; i++, prPwrLimit++) {
		if (*prPwrLimit > MAX_TX_POWER || *prPwrLimit < MIN_TX_POWER) {
			fgValid = FALSE;
			break;	/*Find out Wrong Power limit */
		}
	}
	return fgValid;

}

/*----------------------------------------------------------------------------*/
/*!
 * @brief 1.Check if power limit configuration table valid(channel intervel)
 *	2.Check if power limit configuration/default table entry repeat
 *
 * @param[in]
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
void rlmDomainCheckCountryPowerLimitTable(struct ADAPTER *prAdapter)
{
#define PwrLmtConf g_rRlmPowerLimitConfiguration
	uint8_t i, j;
	uint16_t u2CountryCodeTable, u2CountryCodeCheck;
	u_int8_t fgChannelValid = FALSE;
	u_int8_t fgPowerLimitValid = FALSE;
	u_int8_t fgEntryRepetetion = FALSE;
	u_int8_t fgTableValid = TRUE;

	/*1.Configuration Table Check */
	for (i = 0; i < sizeof(PwrLmtConf) /
	     sizeof(struct COUNTRY_POWER_LIMIT_TABLE_CONFIGURATION); i++) {
		/*Table Country Code */
		WLAN_GET_FIELD_BE16(&PwrLmtConf[i].aucCountryCode[0],
				    &u2CountryCodeTable);

		/*<1>Repetition Entry Check */
		for (j = i + 1; j < sizeof(PwrLmtConf) /
		     sizeof(struct COUNTRY_POWER_LIMIT_TABLE_CONFIGURATION);
		     j++) {

			WLAN_GET_FIELD_BE16(&PwrLmtConf[j].aucCountryCode[0],
					    &u2CountryCodeCheck);
			if (((PwrLmtConf[i].ucCentralCh) ==
			     PwrLmtConf[j].ucCentralCh)
			    && (u2CountryCodeTable == u2CountryCodeCheck)) {
				fgEntryRepetetion = TRUE;
				DBGLOG(RLM, LOUD,
				       "Domain: Configuration Repetition CC=%c%c, Ch=%d\n",
				       PwrLmtConf[i].aucCountryCode[0],
				       PwrLmtConf[i].aucCountryCode[1],
				       PwrLmtConf[i].ucCentralCh);
			}
		}

		/*<2>Channel Number Interval Check */
		fgChannelValid =
		    rlmDomainCheckChannelEntryValid(prAdapter,
						    PwrLmtConf[i].ucCentralCh);

		/*<3>Power Limit Range Check */
		fgPowerLimitValid =
		    rlmDomainCheckPowerLimitValid(prAdapter,
						  PwrLmtConf[i],
						  PWR_LIMIT_NUM);

		if (fgChannelValid == FALSE || fgPowerLimitValid == FALSE) {
			fgTableValid = FALSE;
			DBGLOG(RLM, LOUD,
				"Domain: CC=%c%c, Ch=%d, Limit: %d,%d,%d,%d,%d,%d,%d,%d,%d, Valid:%d,%d\n",
				PwrLmtConf[i].aucCountryCode[0],
				PwrLmtConf[i].aucCountryCode[1],
				PwrLmtConf[i].ucCentralCh,
				PwrLmtConf[i].aucPwrLimit[PWR_LIMIT_CCK],
				PwrLmtConf[i].aucPwrLimit[PWR_LIMIT_20M_L],
				PwrLmtConf[i].aucPwrLimit[PWR_LIMIT_20M_H],
				PwrLmtConf[i].aucPwrLimit[PWR_LIMIT_40M_L],
				PwrLmtConf[i].aucPwrLimit[PWR_LIMIT_40M_H],
				PwrLmtConf[i].aucPwrLimit[PWR_LIMIT_80M_L],
				PwrLmtConf[i].aucPwrLimit[PWR_LIMIT_80M_H],
				PwrLmtConf[i].aucPwrLimit[PWR_LIMIT_160M_L],
				PwrLmtConf[i].aucPwrLimit[PWR_LIMIT_160M_H],
				fgChannelValid,
				fgPowerLimitValid);
		}

		if (u2CountryCodeTable == COUNTRY_CODE_NULL) {
			DBGLOG(RLM, LOUD, "Domain: Full search down\n");
			break;	/*End of country table entry */
		}

	}

	if (fgEntryRepetetion == FALSE)
		DBGLOG(RLM, TRACE,
		       "Domain: Configuration Table no Repetiton.\n");

	/*Configuration Table no error */
	if (fgTableValid == TRUE)
		prAdapter->fgIsPowerLimitTableValid = TRUE;
	else
		prAdapter->fgIsPowerLimitTableValid = FALSE;

	/*2.Default Table Repetition Entry Check */
	fgEntryRepetetion = FALSE;
	for (i = 0; i < sizeof(g_rRlmPowerLimitDefault) /
	     sizeof(struct COUNTRY_POWER_LIMIT_TABLE_DEFAULT); i++) {

		WLAN_GET_FIELD_BE16(&g_rRlmPowerLimitDefault[i].
							aucCountryCode[0],
				    &u2CountryCodeTable);

		for (j = i + 1; j < sizeof(g_rRlmPowerLimitDefault) /
		     sizeof(struct COUNTRY_POWER_LIMIT_TABLE_DEFAULT); j++) {
			WLAN_GET_FIELD_BE16(&g_rRlmPowerLimitDefault[j].
							aucCountryCode[0],
					    &u2CountryCodeCheck);
			if (u2CountryCodeTable == u2CountryCodeCheck) {
				fgEntryRepetetion = TRUE;
				DBGLOG(RLM, LOUD,
				       "Domain: Default Repetition CC=%c%c\n",
				       g_rRlmPowerLimitDefault[j].
							aucCountryCode[0],
				       g_rRlmPowerLimitDefault[j].
							aucCountryCode[1]);
			}
		}
	}
	if (fgEntryRepetetion == FALSE)
		DBGLOG(RLM, TRACE, "Domain: Default Table no Repetiton.\n");
#undef PwrLmtConf
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief
 *
 * @param[in]
 *
 * @return (u2TableIndex) : if  0xFFFF -> No Table Match
 */
/*----------------------------------------------------------------------------*/
uint16_t rlmDomainPwrLimitDefaultTableDecision(struct ADAPTER *prAdapter,
					       uint16_t u2CountryCode)
{

	uint16_t i;
	uint16_t u2CountryCodeTable = COUNTRY_CODE_NULL;
	uint16_t u2TableIndex = POWER_LIMIT_TABLE_NULL;	/* No Table Match */

	/*Default Table Index */
	for (i = 0; i < sizeof(g_rRlmPowerLimitDefault) /
	     sizeof(struct COUNTRY_POWER_LIMIT_TABLE_DEFAULT); i++) {

		WLAN_GET_FIELD_BE16(&g_rRlmPowerLimitDefault[i].
						aucCountryCode[0],
				    &u2CountryCodeTable);

		if (u2CountryCodeTable == u2CountryCode) {
			u2TableIndex = i;
			break;	/*match country code */
		} else if (u2CountryCodeTable == COUNTRY_CODE_NULL) {
			u2TableIndex = i;
			break;	/*find last one country- Default */
		}
	}

	return u2TableIndex;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief Fill power limit CMD by Power Limit Default Table(regulation)
 *
 * @param[in]
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
void
rlmDomainBuildCmdByDefaultTable(struct CMD_SET_COUNTRY_CHANNEL_POWER_LIMIT
				*prCmd,
				uint16_t u2DefaultTableIndex)
{
	uint8_t i, k;
	struct COUNTRY_POWER_LIMIT_TABLE_DEFAULT *prPwrLimitSubBand;
	struct CMD_CHANNEL_POWER_LIMIT *prCmdPwrLimit;

	prCmdPwrLimit = &prCmd->rChannelPowerLimit[0];
	prPwrLimitSubBand = &g_rRlmPowerLimitDefault[u2DefaultTableIndex];

	/*Build power limit cmd by default table information */

	for (i = POWER_LIMIT_2G4; i < POWER_LIMIT_SUBAND_NUM; i++) {
		if (prPwrLimitSubBand->aucPwrLimitSubBand[i] <= MAX_TX_POWER) {
			for (k = g_rRlmSubBand[i].ucStartCh;
			     k <= g_rRlmSubBand[i].ucEndCh;
			     k += g_rRlmSubBand[i].ucInterval) {
				if ((prPwrLimitSubBand->ucPwrUnit
							& BIT(i)) == 0) {
					prCmdPwrLimit->ucCentralCh = k;
					kalMemSet(&prCmdPwrLimit->cPwrLimitCCK,
						  prPwrLimitSubBand->
							aucPwrLimitSubBand[i],
						  PWR_LIMIT_NUM);
				} else {
					/* ex: 40MHz power limit(mW\MHz)
					 *	= 20MHz power limit(mW\MHz) * 2
					 * ---> 40MHz power limit(dBm)
					 *	= 20MHz power limit(dBm) + 6;
					 */
					prCmdPwrLimit->ucCentralCh = k;
					/* BW20 */
					prCmdPwrLimit->cPwrLimitCCK =
							prPwrLimitSubBand->
							  aucPwrLimitSubBand[i];
					prCmdPwrLimit->cPwrLimit20L =
							prPwrLimitSubBand->
							  aucPwrLimitSubBand[i];
					prCmdPwrLimit->cPwrLimit20H =
							prPwrLimitSubBand->
							  aucPwrLimitSubBand[i];

					/* BW40 */
					if (prPwrLimitSubBand->
						aucPwrLimitSubBand[i] + 6 >
								MAX_TX_POWER) {
						prCmdPwrLimit->cPwrLimit40L =
								   MAX_TX_POWER;
						prCmdPwrLimit->cPwrLimit40H =
								   MAX_TX_POWER;
					} else {
						prCmdPwrLimit->cPwrLimit40L =
							prPwrLimitSubBand->
							aucPwrLimitSubBand[i]
							+ 6;
						prCmdPwrLimit->cPwrLimit40H =
							prPwrLimitSubBand->
							aucPwrLimitSubBand[i]
							+ 6;
					}

					/* BW80 */
					if (prPwrLimitSubBand->
						aucPwrLimitSubBand[i] + 12 >
								MAX_TX_POWER) {
						prCmdPwrLimit->cPwrLimit80L =
								MAX_TX_POWER;
						prCmdPwrLimit->cPwrLimit80H =
								MAX_TX_POWER;
					} else {
						prCmdPwrLimit->cPwrLimit80L =
							prPwrLimitSubBand->
							aucPwrLimitSubBand[i]
							+ 12;
						prCmdPwrLimit->cPwrLimit80H =
							prPwrLimitSubBand->
							aucPwrLimitSubBand[i]
							+ 12;
					}

					/* BW160 */
					if (prPwrLimitSubBand->
						aucPwrLimitSubBand[i] + 18 >
								MAX_TX_POWER) {
						prCmdPwrLimit->cPwrLimit160L =
								MAX_TX_POWER;
						prCmdPwrLimit->cPwrLimit160H =
								MAX_TX_POWER;
					} else {
						prCmdPwrLimit->cPwrLimit160L =
							prPwrLimitSubBand->
							aucPwrLimitSubBand[i]
							+ 18;
						prCmdPwrLimit->cPwrLimit160H =
							prPwrLimitSubBand->
							aucPwrLimitSubBand[i]
							+ 18;
					}

				}
				/* save to power limit array per
				 * subband channel
				 */
				prCmdPwrLimit++;
				prCmd->ucNum++;
			}
		}
	}
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief Fill power limit CMD by Power Limit Configurartion Table
 * (Bandedge and Customization)
 *
 * @param[in]
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
void rlmDomainBuildCmdByConfigTable(struct ADAPTER *prAdapter,
			struct CMD_SET_COUNTRY_CHANNEL_POWER_LIMIT *prCmd)
{
#define PwrLmtConf g_rRlmPowerLimitConfiguration
	uint8_t i, k;
	uint16_t u2CountryCodeTable = COUNTRY_CODE_NULL;
	struct CMD_CHANNEL_POWER_LIMIT *prCmdPwrLimit;
	u_int8_t fgChannelValid;

	/*Build power limit cmd by configuration table information */

	for (i = 0; i < sizeof(PwrLmtConf) /
			sizeof(struct COUNTRY_POWER_LIMIT_TABLE_CONFIGURATION);
	     i++) {

		WLAN_GET_FIELD_BE16(&PwrLmtConf[i].aucCountryCode[0],
				    &u2CountryCodeTable);

		fgChannelValid =
		    rlmDomainCheckChannelEntryValid(prAdapter,
						    PwrLmtConf[i].ucCentralCh);

		if (u2CountryCodeTable == COUNTRY_CODE_NULL) {
			break;	/*end of configuration table */
		} else if ((u2CountryCodeTable == prCmd->u2CountryCode)
				&& (fgChannelValid == TRUE)) {

			prCmdPwrLimit = &prCmd->rChannelPowerLimit[0];

			if (prCmd->ucNum != 0) {
				for (k = 0; k < prCmd->ucNum; k++) {
					if (prCmdPwrLimit->ucCentralCh ==
						PwrLmtConf[i].ucCentralCh) {

						/* Cmd setting (Default table
						 * information) and Conf table
						 * has repetition channel entry,
						 * ex : Default table (ex: 2.4G,
						 *      limit = 20dBm) -->
						 *      ch1~14 limit =20dBm,
						 * Conf table (ex: ch1, limit =
						 *      22dBm) --> ch 1 = 22 dBm
						 * Cmd final setting --> ch1 =
						 *      22dBm, ch2~14 = 20dBm
						 */
						kalMemCopy(&prCmdPwrLimit->
								cPwrLimitCCK,
							   &PwrLmtConf[i].
								aucPwrLimit,
							   PWR_LIMIT_NUM);

						DBGLOG(RLM, LOUD,
						       "Domain: CC=%c%c,ReplaceCh=%d,Limit=%d,%d,%d,%d,%d,%d,%d,%d,%d,Fg=%d\n",
						       ((prCmd->u2CountryCode &
								0xff00) >> 8),
						       (prCmd->u2CountryCode &
									0x00ff),
						       prCmdPwrLimit->
								ucCentralCh,
						       prCmdPwrLimit->
								cPwrLimitCCK,
						       prCmdPwrLimit->
								cPwrLimit20L,
						       prCmdPwrLimit->
								cPwrLimit20H,
						       prCmdPwrLimit->
								cPwrLimit40L,
						       prCmdPwrLimit->
								cPwrLimit40H,
						       prCmdPwrLimit->
								cPwrLimit80L,
						       prCmdPwrLimit->
								cPwrLimit80H,
						       prCmdPwrLimit->
								cPwrLimit160L,
						       prCmdPwrLimit->
								cPwrLimit160H,
						       prCmdPwrLimit->ucFlag);

						break;
					}
					/* To search next entry in
					 * rChannelPowerLimit[k]
					 */
					prCmdPwrLimit++;
				}
				if (k == prCmd->ucNum) {

					/* Full search cmd(Default table
					 * setting) no match channel,
					 *  ex : Default table (ex: 2.4G, limit
					 *       =20dBm) -->ch1~14 limit =20dBm,
					 *  Configuration table(ex: ch36, limit
					 *       =22dBm) -->ch 36 = 22 dBm
					 *  Cmd final setting -->
					 *       ch1~14 = 20dBm, ch36 = 22dBm
					 */
					prCmdPwrLimit->ucCentralCh =
						PwrLmtConf[i].ucCentralCh;
					kalMemCopy(&prCmdPwrLimit->cPwrLimitCCK,
					      &PwrLmtConf[i].aucPwrLimit,
					      PWR_LIMIT_NUM);
					/* Add this channel setting in
					 * rChannelPowerLimit[k]
					 */
					prCmd->ucNum++;

					DBGLOG(RLM, LOUD,
					       "Domain: CC=%c%c,AddCh=%d,Limit=%d,%d,%d,%d,%d,%d,%d,%d,%d,Fg=%d\n",
					       ((prCmd->u2CountryCode & 0xff00)
									>> 8),
					       (prCmd->u2CountryCode & 0x00ff),
					       prCmdPwrLimit->ucCentralCh,
					       prCmdPwrLimit->cPwrLimitCCK,
					       prCmdPwrLimit->cPwrLimit20L,
					       prCmdPwrLimit->cPwrLimit20H,
					       prCmdPwrLimit->cPwrLimit40L,
					       prCmdPwrLimit->cPwrLimit40H,
					       prCmdPwrLimit->cPwrLimit80L,
					       prCmdPwrLimit->cPwrLimit80H,
					       prCmdPwrLimit->cPwrLimit160L,
					       prCmdPwrLimit->cPwrLimit160H,
					       prCmdPwrLimit->ucFlag);

				}
			} else {

				/* Default table power limit value are max on
				 * all subbands --> cmd table no channel entry
				 *  ex : Default table (ex: 2.4G, limit = 63dBm)
				 *  --> no channel entry in cmd,
				 *  Configuration table(ex: ch36, limit = 22dBm)
				 *  --> ch 36 = 22 dBm
				 *  Cmd final setting -->  ch36 = 22dBm
				 */
				prCmdPwrLimit->ucCentralCh =
						PwrLmtConf[i].ucCentralCh;
				kalMemCopy(&prCmdPwrLimit->cPwrLimitCCK,
					   &PwrLmtConf[i].aucPwrLimit,
					   PWR_LIMIT_NUM);
				/* Add this channel setting in
				 * rChannelPowerLimit[k]
				 */
				prCmd->ucNum++;

				DBGLOG(RLM, LOUD,
				       "Domain: Default table power limit value are max on all subbands.\n");
				DBGLOG(RLM, LOUD,
				       "Domain: CC=%c%c,AddCh=%d,Limit=%d,%d,%d,%d,%d,%d,%d,%d,%d,Fg=%d\n",
				       ((prCmd->u2CountryCode & 0xff00) >> 8),
				       (prCmd->u2CountryCode & 0x00ff),
				       prCmdPwrLimit->ucCentralCh,
				       prCmdPwrLimit->cPwrLimitCCK,
				       prCmdPwrLimit->cPwrLimit20L,
				       prCmdPwrLimit->cPwrLimit20H,
				       prCmdPwrLimit->cPwrLimit40L,
				       prCmdPwrLimit->cPwrLimit40H,
				       prCmdPwrLimit->cPwrLimit80L,
				       prCmdPwrLimit->cPwrLimit80H,
				       prCmdPwrLimit->cPwrLimit160L,
				       prCmdPwrLimit->cPwrLimit160H,
				       prCmdPwrLimit->ucFlag);
			}
		}
	}
#undef PwrLmtConf
}

struct TX_PWR_LIMIT_DATA *
rlmDomainInitTxPwrLimitData(struct ADAPTER *prAdapter)
{
	uint8_t ch_cnt = 0;
	uint8_t ch_idx = 0;
	uint8_t band_idx = 0;
	uint8_t ch_num = 0;
	const int8_t *prChannelList = NULL;
	struct TX_PWR_LIMIT_DATA *pTxPwrLimitData = NULL;
	struct CHANNEL_TX_PWR_LIMIT *prChTxPwrLimit = NULL;

	pTxPwrLimitData =
		(struct TX_PWR_LIMIT_DATA *)
		kalMemAlloc(sizeof(struct TX_PWR_LIMIT_DATA),
			VIR_MEM_TYPE);

	if (!pTxPwrLimitData) {
		DBGLOG(RLM, ERROR,
			"Alloc buffer for TxPwrLimit main struct failed\n");
		return NULL;
	}

	pTxPwrLimitData->ucChNum =
		TX_PWR_LIMIT_2G_CH_NUM + TX_PWR_LIMIT_5G_CH_NUM;

	pTxPwrLimitData->rChannelTxPwrLimit =
		(struct CHANNEL_TX_PWR_LIMIT *)
		kalMemAlloc(sizeof(struct CHANNEL_TX_PWR_LIMIT) *
			(pTxPwrLimitData->ucChNum), VIR_MEM_TYPE);

	if (!pTxPwrLimitData->rChannelTxPwrLimit) {
		DBGLOG(RLM, ERROR,
			"Alloc buffer for TxPwrLimit ch values failed\n");

		kalMemFree(pTxPwrLimitData, VIR_MEM_TYPE,
			sizeof(struct TX_PWR_LIMIT_DATA));
		return NULL;
	}

	for (ch_idx = 0; ch_idx < pTxPwrLimitData->ucChNum; ch_idx++) {
		prChTxPwrLimit =
			&(pTxPwrLimitData->rChannelTxPwrLimit[ch_idx]);
		kalMemSet(prChTxPwrLimit->rTxPwrLimitValue,
			MAX_TX_POWER,
			sizeof(prChTxPwrLimit->rTxPwrLimitValue));
	}

	ch_cnt = 0;
	for (band_idx = 0; band_idx < KAL_NUM_BANDS; band_idx++) {
		if (band_idx != KAL_BAND_2GHZ && band_idx != KAL_BAND_5GHZ)
			continue;

		prChannelList = (band_idx == KAL_BAND_2GHZ) ?
			gTx_Pwr_Limit_2g_Ch : gTx_Pwr_Limit_5g_Ch;

		ch_num =
			(band_idx == KAL_BAND_2GHZ) ?
				TX_PWR_LIMIT_2G_CH_NUM :
				TX_PWR_LIMIT_5G_CH_NUM;

		for (ch_idx = 0; ch_idx < ch_num; ch_idx++) {
			pTxPwrLimitData->rChannelTxPwrLimit[ch_cnt].ucChannel =
				prChannelList[ch_idx];
			++ch_cnt;
		}
	}

	return pTxPwrLimitData;
}

void
rlmDomainSendTxPwrLimitCmd(struct ADAPTER *prAdapter,
	uint8_t ucVersion,
	struct TX_PWR_LIMIT_DATA *pTxPwrLimitData)
{
	uint8_t ch_cnt = 0;
	uint8_t ch_idx = 0;
	uint8_t band_idx = 0;
	const int8_t *prChannelList = NULL;
	uint32_t rStatus;
	uint32_t u4SetQueryInfoLen;
	uint32_t u4SetCmdTableMaxSize[KAL_NUM_BANDS] = {0};
	struct CMD_SET_COUNTRY_CHANNEL_POWER_LIMIT_V2
		*prCmd[KAL_NUM_BANDS] = {0};
	struct CMD_CHANNEL_POWER_LIMIT_V2 *prCmdChPwrLimitV2 = NULL;
	uint32_t u4SetCountryCmdSize =
		sizeof(struct CMD_SET_COUNTRY_CHANNEL_POWER_LIMIT_V2);
	uint32_t u4ChPwrLimitV2Size = sizeof(struct CMD_CHANNEL_POWER_LIMIT_V2);
	const uint8_t ucCmdBatchSize =
		prAdapter->chip_info->ucTxPwrLimitBatchSize;

	for (band_idx = 0; band_idx < KAL_NUM_BANDS; band_idx++) {
		if (band_idx != KAL_BAND_2GHZ && band_idx != KAL_BAND_5GHZ)
			continue;

		prChannelList = (band_idx == KAL_BAND_2GHZ) ?
			gTx_Pwr_Limit_2g_Ch : gTx_Pwr_Limit_5g_Ch;

		ch_cnt = (band_idx == KAL_BAND_2GHZ) ? TX_PWR_LIMIT_2G_CH_NUM :
			TX_PWR_LIMIT_5G_CH_NUM;

		if (!ch_cnt)
			continue;

		u4SetCmdTableMaxSize[band_idx] = u4SetCountryCmdSize +
			ch_cnt * u4ChPwrLimitV2Size;

		prCmd[band_idx] = cnmMemAlloc(prAdapter,
			RAM_TYPE_BUF, u4SetCmdTableMaxSize[band_idx]);

		if (!prCmd[band_idx]) {
			DBGLOG(RLM, ERROR, "Domain: no buf to send cmd\n");
			goto error;
		}

		/*initialize tw pwr table*/
		kalMemSet(prCmd[band_idx], MAX_TX_POWER,
			u4SetCmdTableMaxSize[band_idx]);

		prCmd[band_idx]->ucNum = ch_cnt;
		prCmd[band_idx]->eband =
			(band_idx == KAL_BAND_2GHZ) ?
				BAND_2G4 : BAND_5G;
		prCmd[band_idx]->countryCode = rlmDomainGetCountryCode();

		DBGLOG(RLM, INFO,
			"%s, active n_channels=%d, band=%d\n",
			__func__, ch_cnt, prCmd[band_idx]->eband);

		for (ch_idx = 0; ch_idx < ch_cnt; ch_idx++) {
			prCmdChPwrLimitV2 =
				&(prCmd[band_idx]->rChannelPowerLimit[ch_idx]);
			prCmdChPwrLimitV2->ucCentralCh =
				prChannelList[ch_idx];
		}
	}

	rlmDomainTxPwrLimitSetValues(ucVersion,
		prCmd[KAL_BAND_2GHZ], pTxPwrLimitData);
	rlmDomainTxPwrLimitSetValues(ucVersion,
		prCmd[KAL_BAND_5GHZ], pTxPwrLimitData);

	for (band_idx = 0; band_idx < KAL_NUM_BANDS; band_idx++) {
		uint8_t ucRemainChNum, i, ucTempChNum, prCmdBatchNum;
		uint32_t u4BufSize = 0;
		struct CMD_SET_COUNTRY_CHANNEL_POWER_LIMIT_V2 *prTempCmd = NULL;
		enum ENUM_BAND eBand = (band_idx == KAL_BAND_2GHZ) ?
				BAND_2G4 : BAND_5G;
		uint16_t u2ChIdx = 0;

		if (!prCmd[band_idx])
			continue;

		ucRemainChNum = prCmd[band_idx]->ucNum;
		prCmdBatchNum = (ucRemainChNum +
			ucCmdBatchSize - 1) /
			ucCmdBatchSize;

		for (i = 0; i < prCmdBatchNum; i++) {
			if (i == prCmdBatchNum - 1)
				ucTempChNum = ucRemainChNum;
			else
				ucTempChNum = ucCmdBatchSize;

			u4BufSize = u4SetCountryCmdSize +
				ucTempChNum * u4ChPwrLimitV2Size;

			prTempCmd =
				cnmMemAlloc(prAdapter,
					RAM_TYPE_BUF, u4BufSize);

			if (!prTempCmd) {
				DBGLOG(RLM, ERROR,
					"Domain: no buf to send cmd\n");
				goto error;
			}

			/*copy partial tx pwr limit*/
			prTempCmd->ucNum = ucTempChNum;
			prTempCmd->eband = eBand;
			prTempCmd->countryCode = rlmDomainGetCountryCode();
			u2ChIdx = i * ucCmdBatchSize;
			kalMemCopy(&prTempCmd->rChannelPowerLimit[0],
				&prCmd[band_idx]->rChannelPowerLimit[u2ChIdx],
				ucTempChNum * u4ChPwrLimitV2Size);

			u4SetQueryInfoLen = u4BufSize;
			/* Update tx max. power info to chip */
			rStatus = wlanSendSetQueryCmd(prAdapter,
				CMD_ID_SET_COUNTRY_POWER_LIMIT,
				TRUE,
				FALSE,
				FALSE,
				NULL,
				NULL,
				u4SetQueryInfoLen,
				(uint8_t *) prTempCmd,
				NULL,
				0);

			cnmMemFree(prAdapter, prTempCmd);

			ucRemainChNum -= ucTempChNum;
		}
	}

error:
	for (band_idx = 0; band_idx < KAL_NUM_BANDS; band_idx++) {
		if (prCmd[band_idx])
			cnmMemFree(prAdapter, prCmd[band_idx]);
	}
}

void
rlmDomainSendTxPwrLimitPerRateCmd(struct ADAPTER *prAdapter,
	uint8_t ucVersion,
	struct TX_PWR_LIMIT_DATA *pTxPwrLimitData)
{
	uint8_t ch_cnt = 0;
	uint8_t ch_idx = 0;
	uint8_t band_idx = 0;
	const int8_t *prChannelList = NULL;
	uint32_t rStatus;
	uint32_t u4SetQueryInfoLen;
	uint32_t u4SetCmdTableMaxSize[KAL_NUM_BANDS] = {0};
	struct CMD_SET_TXPOWER_COUNTRY_TX_POWER_LIMIT_PER_RATE
		*prCmd[KAL_NUM_BANDS] = {0};
	struct CMD_TXPOWER_CHANNEL_POWER_LIMIT_PER_RATE *prChPwrLimit = NULL;
	uint32_t u4SetCountryTxPwrLimitCmdSize =
		sizeof(struct CMD_SET_TXPOWER_COUNTRY_TX_POWER_LIMIT_PER_RATE);
	uint32_t u4ChPwrLimitSize =
		sizeof(struct CMD_TXPOWER_CHANNEL_POWER_LIMIT_PER_RATE);
	const uint8_t ucCmdBatchSize =
		prAdapter->chip_info->ucTxPwrLimitBatchSize;

	for (band_idx = 0; band_idx < KAL_NUM_BANDS; band_idx++) {
		if (band_idx != KAL_BAND_2GHZ && band_idx != KAL_BAND_5GHZ)
			continue;

		prChannelList = (band_idx == KAL_BAND_2GHZ) ?
			gTx_Pwr_Limit_2g_Ch : gTx_Pwr_Limit_5g_Ch;

		ch_cnt = (band_idx == KAL_BAND_2GHZ) ? TX_PWR_LIMIT_2G_CH_NUM :
			TX_PWR_LIMIT_5G_CH_NUM;

		if (!ch_cnt)
			continue;

		u4SetCmdTableMaxSize[band_idx] = u4SetCountryTxPwrLimitCmdSize +
			ch_cnt * u4ChPwrLimitSize;

		prCmd[band_idx] = cnmMemAlloc(prAdapter,
			RAM_TYPE_BUF, u4SetCmdTableMaxSize[band_idx]);

		if (!prCmd[band_idx]) {
			DBGLOG(RLM, ERROR, "Domain: no buf to send cmd\n");
			goto error;
		}

		/*initialize tw pwr table*/
		kalMemSet(prCmd[band_idx], MAX_TX_POWER,
			u4SetCmdTableMaxSize[band_idx]);

		prCmd[band_idx]->ucNum = ch_cnt;
		prCmd[band_idx]->eBand =
			(band_idx == KAL_BAND_2GHZ) ?
				BAND_2G4 : BAND_5G;
		prCmd[band_idx]->u4CountryCode = rlmDomainGetCountryCode();

		DBGLOG(RLM, INFO,
			"%s, active n_channels=%d, band=%d\n",
			__func__, ch_cnt, prCmd[band_idx]->eBand);

		for (ch_idx = 0; ch_idx < ch_cnt; ch_idx++) {
			prChPwrLimit =
				&(prCmd[band_idx]->
				  rChannelPowerLimit[ch_idx]);
			prChPwrLimit->u1CentralCh =	prChannelList[ch_idx];
		}
	}

	rlmDomainTxPwrLimitPerRateSetValues(ucVersion,
		prCmd[KAL_BAND_2GHZ], pTxPwrLimitData);
	rlmDomainTxPwrLimitPerRateSetValues(ucVersion,
		prCmd[KAL_BAND_5GHZ], pTxPwrLimitData);

	for (band_idx = 0; band_idx < KAL_NUM_BANDS; band_idx++) {
		uint8_t ucRemainChNum, i, ucTempChNum, prCmdBatchNum;
		uint32_t u4BufSize = 0;
		struct CMD_SET_TXPOWER_COUNTRY_TX_POWER_LIMIT_PER_RATE
			*prTempCmd = NULL;
		enum ENUM_BAND eBand = (band_idx == KAL_BAND_2GHZ) ?
				BAND_2G4 : BAND_5G;
		uint16_t u2ChIdx = 0;
		u_int8_t bCmdFinished = FALSE;

		if (!prCmd[band_idx])
			continue;

		ucRemainChNum = prCmd[band_idx]->ucNum;
		prCmdBatchNum = (ucRemainChNum +
			ucCmdBatchSize - 1) /
			ucCmdBatchSize;

		for (i = 0; i < prCmdBatchNum; i++) {
			if (i == prCmdBatchNum - 1) {
				ucTempChNum = ucRemainChNum;
				bCmdFinished = TRUE;
			} else {
				ucTempChNum = ucCmdBatchSize;
			}

			u4BufSize = u4SetCountryTxPwrLimitCmdSize +
				ucTempChNum * u4ChPwrLimitSize;

			prTempCmd =
				cnmMemAlloc(prAdapter,
					RAM_TYPE_BUF, u4BufSize);

			if (!prTempCmd) {
				DBGLOG(RLM, ERROR,
					"Domain: no buf to send cmd\n");
				goto error;
			}

			/*copy partial tx pwr limit*/
			prTempCmd->ucNum = ucTempChNum;
			prTempCmd->eBand = eBand;
			prTempCmd->u4CountryCode =
				rlmDomainGetCountryCode();
			prTempCmd->bCmdFinished = bCmdFinished;
			u2ChIdx = i * ucCmdBatchSize;
			kalMemCopy(
				&prTempCmd->rChannelPowerLimit[0],
				&prCmd[band_idx]->
				 rChannelPowerLimit[u2ChIdx],
				ucTempChNum * u4ChPwrLimitSize);

			u4SetQueryInfoLen = u4BufSize;
			/* Update tx max. power info to chip */
			rStatus = wlanSendSetQueryCmd(prAdapter,
				CMD_ID_SET_COUNTRY_POWER_LIMIT_PER_RATE,
				TRUE,
				FALSE,
				FALSE,
				NULL,
				NULL,
				u4SetQueryInfoLen,
				(uint8_t *) prTempCmd,
				NULL,
				0);

			cnmMemFree(prAdapter, prTempCmd);

			ucRemainChNum -= ucTempChNum;
		}
	}

error:
	for (band_idx = 0; band_idx < KAL_NUM_BANDS; band_idx++)
		if (prCmd[band_idx])
			cnmMemFree(prAdapter, prCmd[band_idx]);
}

void rlmDomainSendPwrLimitCmd_V2(struct ADAPTER *prAdapter)
{
#if (CFG_SUPPORT_SINGLE_SKU == 1)
	uint8_t ucVersion = 0;
	struct TX_PWR_LIMIT_DATA *pTxPwrLimitData = NULL;

	DBGLOG(RLM, INFO, "rlmDomainSendPwrLimitCmd()\n");
	pTxPwrLimitData = rlmDomainInitTxPwrLimitData(prAdapter);

	if (!pTxPwrLimitData) {
		DBGLOG(RLM, ERROR,
			"Init TxPwrLimitData failed\n");
		goto error;
	}

	/*
	 * Get Max Tx Power from MT_TxPwrLimit.dat
	 */
	rlmDomainGetTxPwrLimit(rlmDomainGetCountryCode(),
		&ucVersion,
		prAdapter->prGlueInfo,
		pTxPwrLimitData);

	/* Prepare to send CMD to FW */
	if (ucVersion == 0)
		rlmDomainSendTxPwrLimitCmd(prAdapter,
			ucVersion, pTxPwrLimitData);
	else if (ucVersion == 1)
		rlmDomainSendTxPwrLimitPerRateCmd(prAdapter,
			ucVersion, pTxPwrLimitData);
	else
		DBGLOG(RLM, WARN, "Unsupported TxPwrLimit dat version %u\n",
			ucVersion);

error:
	if (pTxPwrLimitData && pTxPwrLimitData->rChannelTxPwrLimit)
		kalMemFree(pTxPwrLimitData->rChannelTxPwrLimit, VIR_MEM_TYPE,
			sizeof(struct CHANNEL_TX_PWR_LIMIT) *
			pTxPwrLimitData->ucChNum);

	if (pTxPwrLimitData)
		kalMemFree(pTxPwrLimitData, VIR_MEM_TYPE,
			sizeof(struct TX_PWR_LIMIT_DATA));
#endif
}

#if CFG_SUPPORT_DYNAMIC_PWR_LIMIT
/* dynamic tx power control: begin ********************************************/
uint32_t txPwrParseNumber(char **pcContent, char *delim, uint8_t *op,
			  int8_t *value) {
	u_int8_t fgIsNegtive = FALSE;
	char *pcTmp = NULL;
	char *result = NULL;

	if ((!pcContent) || (*pcContent == NULL))
		return -1;

	if (**pcContent == '-') {
		fgIsNegtive = TRUE;
		*pcContent = *pcContent + 1;
	}
	pcTmp = *pcContent;

	result = kalStrSep(pcContent, delim);
	if (result == NULL) {
		return -1;
	} else if ((result != NULL) && (kalStrLen(result) == 0)) {
		if (fgIsNegtive)
			return -1;
		*value = 0;
		*op = 0;
	} else {
		if (kalkStrtou8(pcTmp, 0, value) != 0) {
			DBGLOG(RLM, ERROR,
			       "parse number error: invalid number [%s]\n",
			       pcTmp);
			return -1;
		}
		if (fgIsNegtive)
			*op = 2;
		else
			*op = 1;
	}

	return 0;
}

void txPwrOperate(enum ENUM_TX_POWER_CTRL_TYPE eCtrlType,
		  int8_t *operand1, int8_t *operand2)
{
	switch (eCtrlType) {
	case PWR_CTRL_TYPE_WIFION_POWER_LEVEL:
	case PWR_CTRL_TYPE_IOCTL_POWER_LEVEL:
		if (*operand1 > *operand2)
			*operand1 = *operand2;
		break;
	case PWR_CTRL_TYPE_WIFION_POWER_OFFSET:
	case PWR_CTRL_TYPE_IOCTL_POWER_OFFSET:
		*operand1 += *operand2;
		break;
	default:
		break;
	}

	if (*operand1 > MAX_TX_POWER)
		*operand1 = MAX_TX_POWER;
	else if (*operand1 < MIN_TX_POWER)
		*operand1 = MIN_TX_POWER;
}

uint32_t txPwrArbitrator(enum ENUM_TX_POWER_CTRL_TYPE eCtrlType,
			 struct CMD_CHANNEL_POWER_LIMIT *prCmdPwrLimit,
			 struct TX_PWR_CTRL_CHANNEL_SETTING *prChlSetting)
{
	if (prChlSetting->op[0] != PWR_CTRL_TYPE_NO_ACTION) {
		txPwrOperate(eCtrlType, &prCmdPwrLimit->cPwrLimitCCK,
			     &prChlSetting->i8PwrLimit[0]);
	}
	if (prChlSetting->op[1] != PWR_CTRL_TYPE_NO_ACTION) {
		txPwrOperate(eCtrlType, &prCmdPwrLimit->cPwrLimit20L,
			     &prChlSetting->i8PwrLimit[1]);
	}
	if (prChlSetting->op[2] != PWR_CTRL_TYPE_NO_ACTION) {
		txPwrOperate(eCtrlType, &prCmdPwrLimit->cPwrLimit20H,
			     &prChlSetting->i8PwrLimit[2]);
	}
	if (prChlSetting->op[3] != PWR_CTRL_TYPE_NO_ACTION) {
		txPwrOperate(eCtrlType, &prCmdPwrLimit->cPwrLimit40L,
			     &prChlSetting->i8PwrLimit[3]);
	}
	if (prChlSetting->op[4] != PWR_CTRL_TYPE_NO_ACTION) {
		txPwrOperate(eCtrlType, &prCmdPwrLimit->cPwrLimit40H,
			     &prChlSetting->i8PwrLimit[4]);
	}
	if (prChlSetting->op[5] != PWR_CTRL_TYPE_NO_ACTION) {
		txPwrOperate(eCtrlType, &prCmdPwrLimit->cPwrLimit80L,
			     &prChlSetting->i8PwrLimit[5]);
	}
	if (prChlSetting->op[6] != PWR_CTRL_TYPE_NO_ACTION) {
		txPwrOperate(eCtrlType, &prCmdPwrLimit->cPwrLimit80H,
			     &prChlSetting->i8PwrLimit[6]);
	}
	if (prChlSetting->op[7] != PWR_CTRL_TYPE_NO_ACTION) {
		txPwrOperate(eCtrlType, &prCmdPwrLimit->cPwrLimit160L,
			     &prChlSetting->i8PwrLimit[7]);
	}
	if (prChlSetting->op[8] != PWR_CTRL_TYPE_NO_ACTION) {
		txPwrOperate(eCtrlType, &prCmdPwrLimit->cPwrLimit160H,
			     &prChlSetting->i8PwrLimit[8]);
	}

	return 0;
}

uint32_t txPwrApplyOneSetting(struct CMD_SET_COUNTRY_CHANNEL_POWER_LIMIT *prCmd,
			      struct TX_PWR_CTRL_ELEMENT *prCurElement,
			      uint8_t *bandedgeParam)
{
	struct CMD_CHANNEL_POWER_LIMIT *prCmdPwrLimit;
	struct TX_PWR_CTRL_CHANNEL_SETTING *prChlSetting;
	uint8_t i, j, channel, channel2, channel3;
	u_int8_t fgDoArbitrator;

	prCmdPwrLimit = &prCmd->rChannelPowerLimit[0];
	for (i = 0; i < prCmd->ucNum; i++) {
		channel = prCmdPwrLimit->ucCentralCh;
		for (j = 0; j < prCurElement->settingCount; j++) {
			prChlSetting = &prCurElement->rChlSettingList[j];
			channel2 = prChlSetting->channelParam[0];
			channel3 = prChlSetting->channelParam[1];
			fgDoArbitrator = FALSE;
			switch (prChlSetting->eChnlType) {
				case PWR_CTRL_CHNL_TYPE_NORMAL: {
					if (channel == channel2)
						fgDoArbitrator = TRUE;
					break;
				}
				case PWR_CTRL_CHNL_TYPE_ALL: {
					fgDoArbitrator = TRUE;
					break;
				}
				case PWR_CTRL_CHNL_TYPE_RANGE: {
					if ((channel >= channel2) &&
					    (channel <= channel3))
						fgDoArbitrator = TRUE;
					break;
				}
				case PWR_CTRL_CHNL_TYPE_2G4: {
					if (channel <= 14)
						fgDoArbitrator = TRUE;
					break;
				}
				case PWR_CTRL_CHNL_TYPE_5G: {
					if (channel > 14)
						fgDoArbitrator = TRUE;
					break;
				}
				case PWR_CTRL_CHNL_TYPE_BANDEDGE_2G4: {
					if ((channel == *bandedgeParam) ||
					    (channel == *(bandedgeParam + 1)))
						fgDoArbitrator = TRUE;
					break;
				}
				case PWR_CTRL_CHNL_TYPE_BANDEDGE_5G: {
					if ((channel == *(bandedgeParam + 2)) ||
					    (channel == *(bandedgeParam + 3)))
						fgDoArbitrator = TRUE;
					break;
				}
				case PWR_CTRL_CHNL_TYPE_5G_BAND1: {
					if ((channel >= 30) && (channel <= 50))
						fgDoArbitrator = TRUE;
					break;
				}
				case PWR_CTRL_CHNL_TYPE_5G_BAND2: {
					if ((channel >= 51) && (channel <= 70))
						fgDoArbitrator = TRUE;
					break;
				}
				case PWR_CTRL_CHNL_TYPE_5G_BAND3: {
					if ((channel >= 71) && (channel <= 145))
						fgDoArbitrator = TRUE;
					break;
				}
				case PWR_CTRL_CHNL_TYPE_5G_BAND4: {
					if ((channel >= 146) &&
					    (channel <= 170))
						fgDoArbitrator = TRUE;
					break;
				}
			}
			if (fgDoArbitrator)
				txPwrArbitrator(prCurElement->eCtrlType,
						prCmdPwrLimit, prChlSetting);
		}

		prCmdPwrLimit++;
	}

	return 0;
}

uint32_t txPwrCtrlApplySettings(struct ADAPTER *prAdapter,
			struct CMD_SET_COUNTRY_CHANNEL_POWER_LIMIT *prCmd,
			uint8_t *bandedgeParam)
{
	struct LINK_ENTRY *prCur, *prNext;
	struct TX_PWR_CTRL_ELEMENT *element = NULL;
	struct LINK *aryprlist[2] = {
		&prAdapter->rTxPwr_DefaultList,
		&prAdapter->rTxPwr_DynamicList
	};
	int32_t i;

	/* show the tx power ctrl applied list */
	txPwrCtrlShowList(prAdapter, 1, "applied list");

	for (i = 0; i < ARRAY_SIZE(aryprlist); i++) {
		LINK_FOR_EACH_SAFE(prCur, prNext, aryprlist[i]) {
			element = LINK_ENTRY(prCur,
					struct TX_PWR_CTRL_ELEMENT, node);
			if (element->fgApplied == TRUE)
				txPwrApplyOneSetting(
					prCmd, element, bandedgeParam);
		}
	}

	return 0;
}

char *txPwrGetString(char **pcContent, char *delim)
{
	char *result = NULL;

	if (pcContent == NULL)
		return NULL;

	result = kalStrSep(pcContent, delim);
	if ((pcContent == NULL) || (result == NULL) ||
	    ((result != NULL) && (kalStrLen(result) == 0)))
		return NULL;

	return result;
}

struct TX_PWR_CTRL_ELEMENT *txPwrCtrlStringToStruct(char *pcContent,
						    u_int8_t fgSkipHeader)
{
	struct TX_PWR_CTRL_ELEMENT *prCurElement = NULL;
	struct TX_PWR_CTRL_CHANNEL_SETTING *prTmpSetting;
	char acTmpName[MAX_TX_PWR_CTRL_ELEMENT_NAME_SIZE];
	char *pcContCur = NULL, *pcContCur2 = NULL, *pcContEnd = NULL;
	char *pcContTmp = NULL, *pcContNext = NULL, *pcContOld = NULL;
	char carySeperator[2] = { 0, 0 };
	uint32_t u4MemSize = sizeof(struct TX_PWR_CTRL_ELEMENT);
	uint32_t copySize = 0;
	uint8_t i, j, op, ucSettingCount = 0;
	uint8_t value, value2, count = 0;
	uint8_t ucAppliedWay, ucOperation = 0;
	uint8_t ucCommaCount;

	if (!pcContent) {
		DBGLOG(RLM, ERROR, "pcContent is null\n");
		return NULL;
	}

	pcContCur = pcContent;
	pcContEnd = pcContent + kalStrLen(pcContent);

	if (fgSkipHeader == TRUE)
		goto skipLabel;

	/* insert elenemt into prTxPwrCtrlList */
	/* parse scenario name */
	kalMemZero(acTmpName, MAX_TX_PWR_CTRL_ELEMENT_NAME_SIZE);
	pcContOld = pcContCur;
	pcContTmp = txPwrGetString(&pcContCur, ";");
	if (!pcContTmp) {
		DBGLOG(RLM, ERROR, "parse scenario name error: %s\n",
		       pcContOld);
		return NULL;
	}
	copySize = kalStrLen(pcContTmp);
	if (copySize >= MAX_TX_PWR_CTRL_ELEMENT_NAME_SIZE)
		copySize = MAX_TX_PWR_CTRL_ELEMENT_NAME_SIZE - 1;
	kalMemCopy(acTmpName, pcContTmp, copySize);
	acTmpName[copySize] = 0;

	/* parese scenario sub index */
	pcContOld = pcContCur;
	if (txPwrParseNumber(&pcContCur, ";", &op, &value)) {
		DBGLOG(RLM, ERROR, "parse scenario sub index error: %s\n",
		       pcContOld);
		return NULL;
	}
	if ((op != 1) || (value < 0)) {
		DBGLOG(RLM, ERROR,
		       "parse scenario sub index error: op=%u, val=%d\n",
		       op, value);
		return NULL;
	}

	/* parese scenario applied way */
	pcContOld = pcContCur;
	if (txPwrParseNumber(&pcContCur, ";", &op, &ucAppliedWay)) {
		DBGLOG(RLM, ERROR, "parse applied way error: %s\n",
		       pcContOld);
		return NULL;
	}
	if ((ucAppliedWay < PWR_CTRL_TYPE_APPLIED_WAY_WIFION) ||
	    (ucAppliedWay > PWR_CTRL_TYPE_APPLIED_WAY_IOCTL)) {
		DBGLOG(RLM, ERROR,
		       "parse applied way error: value=%u\n",
		       ucAppliedWay);
		return NULL;
	}

	/* parese scenario applied type */
	pcContOld = pcContCur;
	if (txPwrParseNumber(&pcContCur, ";", &op, &ucOperation)) {
		DBGLOG(RLM, ERROR, "parse operation error: %s\n",
		       pcContOld);
		return NULL;
	}
	if ((ucOperation < PWR_CTRL_TYPE_OPERATION_POWER_LEVEL) ||
	    (ucOperation > PWR_CTRL_TYPE_OPERATION_POWER_OFFSET)) {
		DBGLOG(RLM, ERROR,
		       "parse operation error: value=%u\n",
		       ucOperation);
		return NULL;
	}

	switch (ucAppliedWay) {
	case PWR_CTRL_TYPE_APPLIED_WAY_WIFION:
		if (ucOperation == PWR_CTRL_TYPE_OPERATION_POWER_LEVEL)
			value2 = PWR_CTRL_TYPE_WIFION_POWER_LEVEL;
		else
			value2 = PWR_CTRL_TYPE_WIFION_POWER_OFFSET;
		break;
	case PWR_CTRL_TYPE_APPLIED_WAY_IOCTL:
		if (ucOperation == PWR_CTRL_TYPE_OPERATION_POWER_LEVEL)
			value2 = PWR_CTRL_TYPE_IOCTL_POWER_LEVEL;
		else
			value2 = PWR_CTRL_TYPE_IOCTL_POWER_OFFSET;
		break;
	}

skipLabel:
	/* decide how many channel setting */
	pcContOld = pcContCur;
	while (pcContCur <= pcContEnd) {
		if ((*pcContCur) == '[')
			ucSettingCount++;
		pcContCur++;
	}

	if (ucSettingCount == 0) {
		DBGLOG(RLM, ERROR,
		       "power ctrl channel setting is empty\n");
		return NULL;
	}

	/* allocate memory for control element */
	u4MemSize += (ucSettingCount == 1) ? 0 : (ucSettingCount - 1) *
			sizeof(struct TX_PWR_CTRL_CHANNEL_SETTING);
	prCurElement = (struct TX_PWR_CTRL_ELEMENT *)kalMemAlloc(
					u4MemSize, VIR_MEM_TYPE);
	if (!prCurElement) {
		DBGLOG(RLM, ERROR,
		       "alloc power ctrl element failed\n");
		return NULL;
	}

	/* assign values into control element */
	kalMemZero(prCurElement, u4MemSize);
	if (fgSkipHeader == FALSE) {
		kalMemCopy(prCurElement->name, acTmpName, copySize);
		prCurElement->index = (uint8_t)value;
		prCurElement->eCtrlType = (enum ENUM_TX_POWER_CTRL_TYPE)value2;
		if (prCurElement->eCtrlType <=
		    PWR_CTRL_TYPE_WIFION_POWER_OFFSET)
			prCurElement->fgApplied = TRUE;
	}
	prCurElement->settingCount = ucSettingCount;

	/* parse channel setting list */
	pcContCur = pcContOld + 1; /* skip '[' */

	for (i = 0; i < ucSettingCount; i++) {
		if (pcContCur >= pcContEnd) {
			DBGLOG(RLM, ERROR,
			       "parse error: out of bound\n");
			goto clearLabel;
		}

		prTmpSetting = &prCurElement->rChlSettingList[i];

		/* verify there is ] symbol */
		pcContNext = kalStrChr(pcContCur, ']');
		if (!pcContNext) {
			DBGLOG(RLM, ERROR,
			       "parse error: miss symbol ']', %s\n",
			       pcContCur);
			goto clearLabel;
		}

		/* verify this setting has 9 segment */
		pcContTmp = pcContCur;
		count = 0;
		while (pcContTmp < pcContNext) {
			if (*pcContTmp == ',')
				count++;
			pcContTmp++;
		}
		if ((count != 9) && (count != 1)) {
			DBGLOG(RLM, ERROR,
			       "parse error: not 9 segments, %s\n",
			       pcContCur);
			goto clearLabel;
		} else {
			ucCommaCount = count;
			if (ucCommaCount == 9)
				carySeperator[0] = ',';
			else
				carySeperator[0] = ']';
		}

		/* parse channel setting type */
		pcContOld = pcContCur;
		pcContTmp = txPwrGetString(&pcContCur, ",");
		if (!pcContTmp) {
			DBGLOG(RLM, ERROR,
			       "parse channel setting type error, %s\n",
			       pcContOld);
			goto clearLabel;
		/* "ALL" */
		} else if (kalStrCmp(pcContTmp,
				     PWR_CTRL_CHNL_TYPE_KEY_ALL) == 0)
			prTmpSetting->eChnlType =
				PWR_CTRL_CHNL_TYPE_ALL;
		/* "2G4" */
		else if (kalStrCmp(pcContTmp,
				   PWR_CTRL_CHNL_TYPE_KEY_2G4) == 0)
			prTmpSetting->eChnlType =
				PWR_CTRL_CHNL_TYPE_2G4;
		/* "5G" */
		else if (kalStrCmp(pcContTmp,
				   PWR_CTRL_CHNL_TYPE_KEY_5G) == 0)
			prTmpSetting->eChnlType =
				PWR_CTRL_CHNL_TYPE_5G;
		/* "BANDEDGE2G4" */
		else if (kalStrCmp(pcContTmp,
				   PWR_CTRL_CHNL_TYPE_KEY_BANDEDGE_2G4) == 0)
			prTmpSetting->eChnlType =
				PWR_CTRL_CHNL_TYPE_BANDEDGE_2G4;
		/* "BANDEDGE5G" */
		else if (kalStrCmp(pcContTmp,
				   PWR_CTRL_CHNL_TYPE_KEY_BANDEDGE_5G) == 0)
			prTmpSetting->eChnlType =
				PWR_CTRL_CHNL_TYPE_BANDEDGE_5G;
		/* "5GBAND1" */
		else if (kalStrCmp(pcContTmp,
				   PWR_CTRL_CHNL_TYPE_KEY_5G_BAND1) == 0)
			prTmpSetting->eChnlType =
				PWR_CTRL_CHNL_TYPE_5G_BAND1;
		/* "5GBAND2" */
		else if (kalStrCmp(pcContTmp,
				   PWR_CTRL_CHNL_TYPE_KEY_5G_BAND2) == 0)
			prTmpSetting->eChnlType =
				PWR_CTRL_CHNL_TYPE_5G_BAND2;
		/* "5GBAND3" */
		else if (kalStrCmp(pcContTmp,
				   PWR_CTRL_CHNL_TYPE_KEY_5G_BAND3) == 0)
			prTmpSetting->eChnlType =
				PWR_CTRL_CHNL_TYPE_5G_BAND3;
		/* "5GBAND4" */
		else if (kalStrCmp(pcContTmp,
				   PWR_CTRL_CHNL_TYPE_KEY_5G_BAND4) == 0)
			prTmpSetting->eChnlType =
				PWR_CTRL_CHNL_TYPE_5G_BAND4;
		else {
			pcContCur2 = pcContOld;
			pcContTmp = txPwrGetString(&pcContCur2, "-");
			if (pcContCur2 == NULL) { /* case: normal channel */
				if (kalkStrtou8(pcContOld, 0, &value) != 0) {
					DBGLOG(RLM, ERROR,
					       "parse channel error: %s\n",
					       pcContOld);
					goto clearLabel;
				}
				prTmpSetting->channelParam[0] = value;
				prTmpSetting->eChnlType =
						PWR_CTRL_CHNL_TYPE_NORMAL;
			} else { /* case: channel range */
				if (kalkStrtou8(pcContTmp, 0, &value) != 0) {
					DBGLOG(RLM, ERROR,
					       "parse first channel error, %s\n",
					       pcContTmp);
					goto clearLabel;
				}
				if (kalkStrtou8(pcContCur2, 0, &value2) != 0) {
					DBGLOG(RLM, ERROR,
					       "parse second channel error, %s\n",
					       pcContCur2);
					goto clearLabel;
				}
				prTmpSetting->channelParam[0] = value;
				prTmpSetting->channelParam[1] =	value2;
				prTmpSetting->eChnlType =
						PWR_CTRL_CHNL_TYPE_RANGE;
			}
		}

		/* parse cck setting */
		pcContOld = pcContCur;
		if (txPwrParseNumber(&pcContCur, carySeperator, &op, &value)) {
			DBGLOG(RLM, ERROR, "parse CCK error, %s\n",
			       pcContOld);
			goto clearLabel;
		}
		prTmpSetting->op[0] = (enum ENUM_TX_POWER_CTRL_VALUE_SIGN)op;
		prTmpSetting->i8PwrLimit[0] = (op != 2) ? value : (0 - value);
		if ((prTmpSetting->op[0] == PWR_CTRL_TYPE_POSITIVE) &&
		    (ucOperation == PWR_CTRL_TYPE_OPERATION_POWER_OFFSET)) {
			DBGLOG(RLM, ERROR,
				"parse CCK error, Power_Offset value cannot be positive: %u\n",
				value);
			goto clearLabel;
		}
		if (ucCommaCount == 1) {
			for (j = 1; j < 9; j++) {
				prTmpSetting->op[j] = op;
				prTmpSetting->i8PwrLimit[j] = (op != 2) ?
							value : (0 - value);
			}
			goto skipLabel2;
		}

		/* parse cPwrLimit20L setting */
		pcContOld = pcContCur;
		if (txPwrParseNumber(&pcContCur, ",", &op, &value)) {
			DBGLOG(RLM, ERROR, "parse HT20L error, %s\n",
			       pcContOld);
			goto clearLabel;
		}
		prTmpSetting->op[1] = (enum ENUM_TX_POWER_CTRL_VALUE_SIGN)op;
		prTmpSetting->i8PwrLimit[1] = (op != 2) ? value : (0 - value);
		if ((prTmpSetting->op[1] == PWR_CTRL_TYPE_POSITIVE) &&
		    (ucOperation == PWR_CTRL_TYPE_OPERATION_POWER_OFFSET)) {
			DBGLOG(RLM, ERROR,
				"parse HT20L error, Power_Offset value cannot be positive: %u\n",
				value);
			goto clearLabel;
		}

		/* parse cPwrLimit20H setting */
		pcContOld = pcContCur;
		if (txPwrParseNumber(&pcContCur, ",", &op, &value)) {
			DBGLOG(RLM, ERROR, "parse HT20H error, %s\n",
			       pcContOld);
			goto clearLabel;
		}
		prTmpSetting->op[2] = (enum ENUM_TX_POWER_CTRL_VALUE_SIGN)op;
		prTmpSetting->i8PwrLimit[2] = (op != 2) ? value : (0 - value);
		if ((prTmpSetting->op[2] == PWR_CTRL_TYPE_POSITIVE) &&
		    (ucOperation == PWR_CTRL_TYPE_OPERATION_POWER_OFFSET)) {
			DBGLOG(RLM, ERROR,
				"parse HT20H error, Power_Offset value cannot be positive: %u\n",
				value);
			goto clearLabel;
		}

		/* parse cPwrLimit40L setting */
		pcContOld = pcContCur;
		if (txPwrParseNumber(&pcContCur, ",", &op, &value)) {
			DBGLOG(RLM, ERROR, "parse HT40L error, %s\n",
			       pcContOld);
			goto clearLabel;
		}
		prTmpSetting->op[3] = (enum ENUM_TX_POWER_CTRL_VALUE_SIGN)op;
		prTmpSetting->i8PwrLimit[3] = (op != 2) ? value : (0 - value);
		if ((prTmpSetting->op[3] == PWR_CTRL_TYPE_POSITIVE) &&
		    (ucOperation == PWR_CTRL_TYPE_OPERATION_POWER_OFFSET)) {
			DBGLOG(RLM, ERROR,
				"parse HT40L error, Power_Offset value cannot be positive: %u\n",
				value);
			goto clearLabel;
		}

		/* parse cPwrLimit40H setting */
		pcContOld = pcContCur;
		if (txPwrParseNumber(&pcContCur, ",", &op, &value)) {
			DBGLOG(RLM, ERROR, "parse HT40H error, %s\n",
			       pcContOld);
			goto clearLabel;
		}
		prTmpSetting->op[4] = (enum ENUM_TX_POWER_CTRL_VALUE_SIGN)op;
		prTmpSetting->i8PwrLimit[4] = (op != 2) ? value : (0 - value);
		if ((prTmpSetting->op[4] == PWR_CTRL_TYPE_POSITIVE) &&
		    (ucOperation == PWR_CTRL_TYPE_OPERATION_POWER_OFFSET)) {
			DBGLOG(RLM, ERROR,
				"parse HT40H error, Power_Offset value cannot be positive: %u\n",
				value);
			goto clearLabel;
		}

		/* parse cPwrLimit80L setting */
		pcContOld = pcContCur;
		if (txPwrParseNumber(&pcContCur, ",", &op, &value)) {
			DBGLOG(RLM, ERROR,
			       "parse HT80L error, %s\n",
			       pcContOld);
			goto clearLabel;
		}
		prTmpSetting->op[5] = (enum ENUM_TX_POWER_CTRL_VALUE_SIGN)op;
		prTmpSetting->i8PwrLimit[5] = (op != 2) ? value : (0 - value);
		if ((prTmpSetting->op[5] == PWR_CTRL_TYPE_POSITIVE) &&
		    (ucOperation == PWR_CTRL_TYPE_OPERATION_POWER_OFFSET)) {
			DBGLOG(RLM, ERROR,
				"parse HT80L error, Power_Offset value cannot be positive: %u\n",
				value);
			goto clearLabel;
		}

		/* parse cPwrLimit80H setting */
		pcContOld = pcContCur;
		if (txPwrParseNumber(&pcContCur, ",", &op, &value)) {
			DBGLOG(RLM, ERROR,
			       "parse HT80H error, %s\n",
			       pcContOld);
			goto clearLabel;
		}
		prTmpSetting->op[6] = (enum ENUM_TX_POWER_CTRL_VALUE_SIGN)op;
		prTmpSetting->i8PwrLimit[6] = (op != 2) ? value : (0 - value);
		if ((prTmpSetting->op[6] == PWR_CTRL_TYPE_POSITIVE) &&
		    (ucOperation == PWR_CTRL_TYPE_OPERATION_POWER_OFFSET)) {
			DBGLOG(RLM, ERROR,
				"parse HT80H error, Power_Offset value cannot be positive: %u\n",
				value);
			goto clearLabel;
		}

		/* parse cPwrLimit160L setting */
		pcContOld = pcContCur;
		if (txPwrParseNumber(&pcContCur, ",", &op, &value)) {
			DBGLOG(RLM, ERROR,
			       "parse HT160L error, %s\n",
			       pcContOld);
			goto clearLabel;
		}
		prTmpSetting->op[7] = (enum ENUM_TX_POWER_CTRL_VALUE_SIGN)op;
		prTmpSetting->i8PwrLimit[7] = (op != 2) ? value : (0 - value);
		if ((prTmpSetting->op[7] == PWR_CTRL_TYPE_POSITIVE) &&
		    (ucOperation == PWR_CTRL_TYPE_OPERATION_POWER_OFFSET)) {
			DBGLOG(RLM, ERROR,
				"parse HT160L error, Power_Offset value cannot be positive: %u\n",
				value);
			goto clearLabel;
		}

		/* parse cPwrLimit160H setting */
		pcContOld = pcContCur;
		if (txPwrParseNumber(&pcContCur, "]", &op, &value)) {
			DBGLOG(RLM, ERROR,
			       "parse HT160H error, %s\n",
			       pcContOld);
			goto clearLabel;
		}
		prTmpSetting->op[8] = (enum ENUM_TX_POWER_CTRL_VALUE_SIGN)op;
		prTmpSetting->i8PwrLimit[8] = (op != 2) ? value : (0 - value);
		if ((prTmpSetting->op[8] == PWR_CTRL_TYPE_POSITIVE) &&
		    (ucOperation == PWR_CTRL_TYPE_OPERATION_POWER_OFFSET)) {
			DBGLOG(RLM, ERROR,
				"parse HT160H error, Power_Offset value cannot be positive: %u\n",
				value);
			goto clearLabel;
		}

skipLabel2:
		pcContCur = pcContNext + 2;
	}

	return prCurElement;

clearLabel:
	if (prCurElement != NULL)
		kalMemFree(prCurElement, VIR_MEM_TYPE, u4MemSize);

	return NULL;
}

/* filterType: 0:no filter, 1:fgEnable is TRUE */
int txPwrCtrlListSize(struct ADAPTER *prAdapter, uint8_t filterType)
{
	struct LINK_ENTRY *prCur, *prNext;
	struct TX_PWR_CTRL_ELEMENT *prCurElement = NULL;
	struct LINK *aryprlist[2] = {
		&prAdapter->rTxPwr_DefaultList,
		&prAdapter->rTxPwr_DynamicList
	};
	int i, count = 0;

	for (i = 0; i < ARRAY_SIZE(aryprlist); i++) {
		LINK_FOR_EACH_SAFE(prCur, prNext, aryprlist[i]) {
			prCurElement = LINK_ENTRY(prCur,
				struct TX_PWR_CTRL_ELEMENT, node);
			if ((filterType == 1) &&
			    (prCurElement->fgApplied != TRUE))
				continue;
			count++;
		}
	}

	return count;
}

/* filterType: 0:no filter, 1:fgApplied is TRUE */
void txPwrCtrlShowList(struct ADAPTER *prAdapter, uint8_t filterType,
		       char *message)
{
	struct LINK_ENTRY *prCur, *prNext;
	struct TX_PWR_CTRL_ELEMENT *prCurElement = NULL;
	struct TX_PWR_CTRL_CHANNEL_SETTING *prChlSettingList;
	struct LINK *aryprlist[2] = {
		&prAdapter->rTxPwr_DefaultList,
		&prAdapter->rTxPwr_DynamicList
	};
	uint8_t ucAppliedWay, ucOperation;
	int i, j, count = 0;

	if (filterType == 1)
		DBGLOG(RLM, INFO, "Tx Power Ctrl List=[%s], Size=[%d]",
		       message, txPwrCtrlListSize(prAdapter, filterType));
	else
		DBGLOG(RLM, TRACE, "Tx Power Ctrl List=[%s], Size=[%d]",
		       message, txPwrCtrlListSize(prAdapter, filterType));

	for (i = 0; i < ARRAY_SIZE(aryprlist); i++) {
		LINK_FOR_EACH_SAFE(prCur, prNext, aryprlist[i]) {
			prCurElement = LINK_ENTRY(prCur,
					struct TX_PWR_CTRL_ELEMENT, node);
			if ((filterType == 1) &&
			    (prCurElement->fgApplied != TRUE))
				continue;

			switch (prCurElement->eCtrlType) {
			case PWR_CTRL_TYPE_WIFION_POWER_LEVEL:
				ucAppliedWay =
					PWR_CTRL_TYPE_APPLIED_WAY_WIFION;
				ucOperation =
					PWR_CTRL_TYPE_OPERATION_POWER_LEVEL;
				break;
			case PWR_CTRL_TYPE_WIFION_POWER_OFFSET:
				ucAppliedWay =
					PWR_CTRL_TYPE_APPLIED_WAY_WIFION;
				ucOperation =
					PWR_CTRL_TYPE_OPERATION_POWER_OFFSET;
				break;
			case PWR_CTRL_TYPE_IOCTL_POWER_LEVEL:
				ucAppliedWay =
					PWR_CTRL_TYPE_APPLIED_WAY_IOCTL;
				ucOperation =
					PWR_CTRL_TYPE_OPERATION_POWER_LEVEL;
				break;
			case PWR_CTRL_TYPE_IOCTL_POWER_OFFSET:
				ucAppliedWay =
					PWR_CTRL_TYPE_APPLIED_WAY_IOCTL;
				ucOperation =
					PWR_CTRL_TYPE_OPERATION_POWER_OFFSET;
				break;
			default:
				ucAppliedWay = 0;
				ucOperation = 0;
				break;
			}

			DBGLOG(RLM, TRACE,
			       "Tx Power Ctrl Element-%u: name=[%s], index=[%u], appliedWay=[%u:%s], operation=[%u:%s], ChlSettingCount=[%u]\n",
			       ++count, prCurElement->name, prCurElement->index,
			       ucAppliedWay,
			       g_au1TxPwrAppliedWayLabel[ucAppliedWay - 1],
			       ucOperation,
			       g_au1TxPwrOperationLabel[ucOperation - 1],
			       prCurElement->settingCount);
			prChlSettingList = &(prCurElement->rChlSettingList[0]);
			for (j = 0; j < prCurElement->settingCount; j++) {
				DBGLOG(RLM, TRACE,
				       "              Setting-%u:[%s:%u,%u],[%u,%d],[%u,%d],[%u,%d],[%u,%d],[%u,%d],[%u,%d],[%u,%d],[%u,%d],[%u,%d]\n",
				       (j + 1),
				       g_au1TxPwrChlTypeLabel[
						prChlSettingList->eChnlType],
				       prChlSettingList->channelParam[0],
				       prChlSettingList->channelParam[1],
				       prChlSettingList->op[0],
				       prChlSettingList->i8PwrLimit[0],
				       prChlSettingList->op[1],
				       prChlSettingList->i8PwrLimit[1],
				       prChlSettingList->op[2],
				       prChlSettingList->i8PwrLimit[2],
				       prChlSettingList->op[3],
				       prChlSettingList->i8PwrLimit[3],
				       prChlSettingList->op[4],
				       prChlSettingList->i8PwrLimit[4],
				       prChlSettingList->op[5],
				       prChlSettingList->i8PwrLimit[5],
				       prChlSettingList->op[6],
				       prChlSettingList->i8PwrLimit[6],
				       prChlSettingList->op[7],
				       prChlSettingList->i8PwrLimit[7],
				       prChlSettingList->op[8],
				       prChlSettingList->i8PwrLimit[8]
				);
				prChlSettingList++;
			}
		}
	}
}

/* This function used to delete element by specifying name or index
 * if index is 0, deletion only according name
 * if index >= 1, deletion according name and index
 */
void _txPwrCtrlDeleteElement(struct ADAPTER *prAdapter,
			     uint8_t *name,
			     uint32_t index,
			     struct LINK *prTxPwrCtrlList)
{
	struct LINK_ENTRY *prCur, *prNext;
	struct TX_PWR_CTRL_ELEMENT *prCurElement = NULL;
	uint32_t u4MemSize = sizeof(struct TX_PWR_CTRL_ELEMENT);
	uint32_t u4MemSize2;
	uint32_t u4SettingSize = sizeof(struct TX_PWR_CTRL_CHANNEL_SETTING);
	uint8_t ucSettingCount;
	u_int8_t fgFind;

	LINK_FOR_EACH_SAFE(prCur, prNext, prTxPwrCtrlList) {
		fgFind = FALSE;
		prCurElement = LINK_ENTRY(prCur, struct TX_PWR_CTRL_ELEMENT,
					  node);
		if (kalStrCmp(prCurElement->name, name) == 0) {
			if (index == 0)
				fgFind = TRUE;
			else if (prCurElement->index == index)
				fgFind = TRUE;
			if (fgFind) {
				linkDel(prCur);
				if (prCurElement != NULL) {
					ucSettingCount =
						prCurElement->settingCount;
						u4MemSize2 = u4MemSize +
						((ucSettingCount == 1) ? 0 :
						(ucSettingCount - 1) *
						u4SettingSize);
					kalMemFree(prCurElement, VIR_MEM_TYPE,
						u4MemSize2);
				}
			}
		}
	}
}

void txPwrCtrlDeleteElement(struct ADAPTER *prAdapter,
			    uint8_t *name,
			    uint32_t index,
			    enum ENUM_TX_POWER_CTRL_LIST_TYPE eListType)
{
	if ((eListType == PWR_CTRL_TYPE_ALL_LIST) ||
	    (eListType == PWR_CTRL_TYPE_DEFAULT_LIST))
		_txPwrCtrlDeleteElement(prAdapter, name, index,
					&prAdapter->rTxPwr_DefaultList);

	if ((eListType == PWR_CTRL_TYPE_ALL_LIST) ||
	    (eListType == PWR_CTRL_TYPE_DYNAMIC_LIST))
		_txPwrCtrlDeleteElement(prAdapter, name, index,
					&prAdapter->rTxPwr_DynamicList);
}

struct TX_PWR_CTRL_ELEMENT *_txPwrCtrlFindElement(struct ADAPTER *prAdapter,
				 uint8_t *name, uint32_t index,
				 u_int8_t fgCheckIsApplied,
				 struct LINK *prTxPwrCtrlList)
{
	struct LINK_ENTRY *prCur, *prNext;
	struct TX_PWR_CTRL_ELEMENT *prCurElement = NULL;

	LINK_FOR_EACH_SAFE(prCur, prNext, prTxPwrCtrlList) {
		u_int8_t fgFind = FALSE;

		prCurElement = LINK_ENTRY(
			prCur, struct TX_PWR_CTRL_ELEMENT, node);
		if (kalStrCmp(prCurElement->name, name) == 0) {
			if ((!fgCheckIsApplied) ||
			    (fgCheckIsApplied &&
			    (prCurElement->fgApplied == TRUE))) {
				if (index == 0)
					fgFind = TRUE;
				else if (prCurElement->index == index)
					fgFind = TRUE;
			}
			if (fgFind)
				return prCurElement;
		}
	}
	return NULL;
}

struct TX_PWR_CTRL_ELEMENT *txPwrCtrlFindElement(struct ADAPTER *prAdapter,
				 uint8_t *name, uint32_t index,
				 u_int8_t fgCheckIsApplied,
				 enum ENUM_TX_POWER_CTRL_LIST_TYPE eListType)
{
	struct LINK *prTxPwrCtrlList = NULL;

	if (eListType == PWR_CTRL_TYPE_DEFAULT_LIST)
		prTxPwrCtrlList = &prAdapter->rTxPwr_DefaultList;
	if (eListType == PWR_CTRL_TYPE_DYNAMIC_LIST)
		prTxPwrCtrlList = &prAdapter->rTxPwr_DynamicList;
	if (prTxPwrCtrlList == NULL)
		return NULL;

	return _txPwrCtrlFindElement(prAdapter, name, index, fgCheckIsApplied,
				     prTxPwrCtrlList);
}

void txPwrCtrlAddElement(struct ADAPTER *prAdapter,
			 struct TX_PWR_CTRL_ELEMENT *prElement)
{
	struct LINK_ENTRY *prNode = &prElement->node;

	switch (prElement->eCtrlType) {
	case PWR_CTRL_TYPE_WIFION_POWER_LEVEL:
		linkAdd(prNode, &prAdapter->rTxPwr_DefaultList);
		break;
	case PWR_CTRL_TYPE_WIFION_POWER_OFFSET:
		linkAddTail(prNode, &prAdapter->rTxPwr_DefaultList);
		break;
	case PWR_CTRL_TYPE_IOCTL_POWER_LEVEL:
		linkAdd(prNode, &prAdapter->rTxPwr_DynamicList);
		break;
	case PWR_CTRL_TYPE_IOCTL_POWER_OFFSET:
		linkAddTail(prNode, &prAdapter->rTxPwr_DynamicList);
		break;
	}
}

void txPwrCtrlFileBufToList(struct ADAPTER *prAdapter, uint8_t *pucFileBuf)
{
	struct TX_PWR_CTRL_ELEMENT *prNewElement;
	char *oneLine;

	if (pucFileBuf == NULL)
		return;

	while ((oneLine = kalStrSep((char **)(&pucFileBuf), "\r\n"))
	       != NULL) {
		/* skip comment line and empty line */
		if ((oneLine[0] == '#') || (oneLine[0] == 0))
			continue;

		prNewElement = txPwrCtrlStringToStruct(oneLine, FALSE);
		if (prNewElement != NULL) {
			/* delete duplicated element
			 * by checking name and index
			 */
			txPwrCtrlDeleteElement(prAdapter,
				prNewElement->name, prNewElement->index,
				PWR_CTRL_TYPE_ALL_LIST);

			/* append to rTxPwr_List */
			txPwrCtrlAddElement(prAdapter, prNewElement);
		}
	}

	/* show the tx power ctrl list */
	txPwrCtrlShowList(prAdapter, 0, "config list, after loading cfg file");
}

void txPwrCtrlGlobalVariableToList(struct ADAPTER *prAdapter)
{
	struct TX_PWR_CTRL_ELEMENT *pcElement;
	char *ptr;
	int32_t i, u4MemSize;

	for (i = 0; i < ARRAY_SIZE(g_au1TxPwrDefaultSetting); i++) {
		/* skip empty line */
		if (g_au1TxPwrDefaultSetting[i][0] == 0)
			continue;
		u4MemSize = kalStrLen(g_au1TxPwrDefaultSetting[i]) + 1;
		ptr = (char *)kalMemAlloc(u4MemSize, VIR_MEM_TYPE);
		if (ptr == NULL) {
			DBGLOG(RLM, ERROR, "kalMemAlloc fail: %d\n", u4MemSize);
			continue;
		}
		kalMemCopy(ptr, g_au1TxPwrDefaultSetting[i], u4MemSize);
		*(ptr + u4MemSize - 1) = 0;
		pcElement = txPwrCtrlStringToStruct(ptr, FALSE);
		kalMemFree(ptr, VIR_MEM_TYPE, u4MemSize);
		if (pcElement != NULL) {
			/* delete duplicated element
			 * by checking name and index
			 */
			txPwrCtrlDeleteElement(prAdapter,
				pcElement->name, pcElement->index,
				PWR_CTRL_TYPE_ALL_LIST);

			/* append to rTxPwr_List */
			txPwrCtrlAddElement(prAdapter, pcElement);
		}
	}

	/* show the tx power ctrl cfg list */
	txPwrCtrlShowList(prAdapter, 0,
			  "config list, after loadding global variables");
}

void txPwrCtrlCfgFileToList(struct ADAPTER *prAdapter)
{
	uint8_t *pucConfigBuf;
	uint32_t u4ConfigReadLen;

	pucConfigBuf = (uint8_t *)kalMemAlloc(WLAN_CFG_FILE_BUF_SIZE,
					      VIR_MEM_TYPE);
	kalMemZero(pucConfigBuf, WLAN_CFG_FILE_BUF_SIZE);
	if (pucConfigBuf) {
		if (kalRequestFirmware("txpowerctrl.cfg", pucConfigBuf,
		    WLAN_CFG_FILE_BUF_SIZE, &u4ConfigReadLen,
		    prAdapter->prGlueInfo->prDev) == 0) {
			/* ToDo:: Nothing */
		} else if (kalReadToFile("/data/misc/wifi/txpowerctrl.cfg",
			   pucConfigBuf, WLAN_CFG_FILE_BUF_SIZE,
			   &u4ConfigReadLen) == 0) {
			/* ToDo:: Nothing */
		} else if (kalReadToFile("/storage/sdcard0/txpowerctrl.cfg",
			   pucConfigBuf, WLAN_CFG_FILE_BUF_SIZE,
			   &u4ConfigReadLen) == 0) {
			/* ToDo:: Nothing */
		}

		if (pucConfigBuf[0] != '\0' && u4ConfigReadLen > 0)
			txPwrCtrlFileBufToList(prAdapter, pucConfigBuf);
		else
			DBGLOG(RLM, INFO,
			       "no txpowerctrl.cfg or file is empty\n");

		kalMemFree(pucConfigBuf, VIR_MEM_TYPE, WLAN_CFG_FILE_BUF_SIZE);
	}
}

void txPwrCtrlLoadConfig(struct ADAPTER *prAdapter)
{
	/* 1. add records from global tx power ctrl setting into cfg list */
	txPwrCtrlGlobalVariableToList(prAdapter);

	/* 2. update cfg list by txpowerctrl.cfg */
	txPwrCtrlCfgFileToList(prAdapter);

#if CFG_SUPPORT_PWR_LIMIT_COUNTRY
	/* 3. send setting to firmware */
	rlmDomainSendPwrLimitCmd(prAdapter);
#endif
}

void txPwrCtrlInit(struct ADAPTER *prAdapter)
{
	LINK_INITIALIZE(&prAdapter->rTxPwr_DefaultList);
	LINK_INITIALIZE(&prAdapter->rTxPwr_DynamicList);
}

void txPwrCtrlUninit(struct ADAPTER *prAdapter)
{
	struct LINK_ENTRY *prCur, *prNext;
	struct TX_PWR_CTRL_ELEMENT *prCurElement = NULL;
	struct LINK *aryprlist[2] = {
		&prAdapter->rTxPwr_DefaultList,
		&prAdapter->rTxPwr_DynamicList
	};
	uint32_t u4MemSize = sizeof(struct TX_PWR_CTRL_ELEMENT);
	uint32_t u4MemSize2;
	uint32_t u4SettingSize = sizeof(struct TX_PWR_CTRL_CHANNEL_SETTING);
	uint8_t ucSettingCount;
	int32_t i;

	for (i = 0; i < ARRAY_SIZE(aryprlist); i++) {
		LINK_FOR_EACH_SAFE(prCur, prNext, aryprlist[i]) {
			prCurElement = LINK_ENTRY(prCur,
					struct TX_PWR_CTRL_ELEMENT, node);
			linkDel(prCur);
			if (prCurElement) {
				ucSettingCount = prCurElement->settingCount;
					u4MemSize2 = u4MemSize +
					((ucSettingCount <= 1) ? 0 :
					(ucSettingCount - 1) * u4SettingSize);
				kalMemFree(prCurElement,
					VIR_MEM_TYPE, u4MemSize2);
			}
		}
	}
}
/* dynamic tx power control: end **********************************************/
#endif /* CFG_SUPPORT_DYNAMIC_PWR_LIMIT */

void rlmDomainSendPwrLimitCmd(struct ADAPTER *prAdapter)
{
	struct CMD_SET_COUNTRY_CHANNEL_POWER_LIMIT *prCmd = NULL;
	uint32_t rStatus;
	uint8_t i;
	uint16_t u2DefaultTableIndex;
	uint32_t u4SetCmdTableMaxSize;
	uint32_t u4SetQueryInfoLen;
	struct CMD_CHANNEL_POWER_LIMIT *prCmdPwrLimit;	/* for print usage */
	uint8_t bandedgeParam[4] = { 0, 0, 0, 0 };
	struct DOMAIN_INFO_ENTRY *prDomainInfo;
	/* TODO : 5G band edge */
	prDomainInfo = rlmDomainGetDomainInfo(prAdapter);
	if (prDomainInfo) {
		bandedgeParam[0] = prDomainInfo->rSubBand[0].ucFirstChannelNum;
		bandedgeParam[1] = bandedgeParam[0] +
			prDomainInfo->rSubBand[0].ucNumChannels - 1;
	}

	if (regd_is_single_sku_en())
		return rlmDomainSendPwrLimitCmd_V2(prAdapter);


	u4SetCmdTableMaxSize =
	    sizeof(struct CMD_SET_COUNTRY_CHANNEL_POWER_LIMIT) +
	    MAX_CMD_SUPPORT_CHANNEL_NUM *
	    sizeof(struct CMD_CHANNEL_POWER_LIMIT);

	prCmd = cnmMemAlloc(prAdapter, RAM_TYPE_BUF, u4SetCmdTableMaxSize);
	if (!prCmd) {
		DBGLOG(RLM, ERROR, "Domain: Alloc cmd buffer failed\n");
		return;
	}
	kalMemZero(prCmd, u4SetCmdTableMaxSize);

	u2DefaultTableIndex =
	    rlmDomainPwrLimitDefaultTableDecision(prAdapter,
		prAdapter->rWifiVar.rConnSettings.u2CountryCode);

	if (u2DefaultTableIndex != POWER_LIMIT_TABLE_NULL) {

		WLAN_GET_FIELD_BE16(&g_rRlmPowerLimitDefault
				    [u2DefaultTableIndex]
				    .aucCountryCode[0],
				    &prCmd->u2CountryCode);

		/* Initialize channel number */
		prCmd->ucNum = 0;

		if (prCmd->u2CountryCode != COUNTRY_CODE_NULL) {
			/*<1>Command - default table information,
			 *fill all subband
			 */
			rlmDomainBuildCmdByDefaultTable(prCmd,
				u2DefaultTableIndex);

			/*<2>Command - configuration table information,
			 * replace specified channel
			 */
			rlmDomainBuildCmdByConfigTable(prAdapter, prCmd);
		}
	}

	DBGLOG(RLM, INFO,
	       "Domain: ValidCC=%c%c, PwrLimitCC=%c%c, PwrLimitChNum=%d\n",
	       (prAdapter->rWifiVar.rConnSettings.u2CountryCode & 0xff00) >> 8,
	       (prAdapter->rWifiVar.rConnSettings.u2CountryCode & 0x00ff),
	       ((prCmd->u2CountryCode & 0xff00) >> 8),
	       (prCmd->u2CountryCode & 0x00ff),
	       prCmd->ucNum);

	prCmdPwrLimit = &prCmd->rChannelPowerLimit[0];

	for (i = 0; i < prCmd->ucNum; i++) {
		DBGLOG(RLM, TRACE,
			"Old Domain: Ch=%d,Limit=%d,%d,%d,%d,%d,%d,%d,%d,%d,Fg=%d\n",
			prCmdPwrLimit->ucCentralCh,
			prCmdPwrLimit->cPwrLimitCCK,
			prCmdPwrLimit->cPwrLimit20L,
			prCmdPwrLimit->cPwrLimit20H,
			prCmdPwrLimit->cPwrLimit40L,
			prCmdPwrLimit->cPwrLimit40H,
			prCmdPwrLimit->cPwrLimit80L,
			prCmdPwrLimit->cPwrLimit80H,
			prCmdPwrLimit->cPwrLimit160L,
			prCmdPwrLimit->cPwrLimit160H,
			prCmdPwrLimit->ucFlag);
		prCmdPwrLimit++;
	}
#if CFG_SUPPORT_DYNAMIC_PWR_LIMIT
	/* apply each setting into country channel power table */
	txPwrCtrlApplySettings(prAdapter, prCmd, bandedgeParam);
#endif
	/* show tx power table after applying setting */
	prCmdPwrLimit = &prCmd->rChannelPowerLimit[0];
	for (i = 0; i < prCmd->ucNum; i++) {
		DBGLOG(RLM, TRACE,
		       "New Domain: Idx=%d,Ch=%d,Limit=%d,%d,%d,%d,%d,%d,%d,%d,%d,Fg=%d\n",
		       i, prCmdPwrLimit->ucCentralCh,
		       prCmdPwrLimit->cPwrLimitCCK,
		       prCmdPwrLimit->cPwrLimit20L,
		       prCmdPwrLimit->cPwrLimit20H,
		       prCmdPwrLimit->cPwrLimit40L,
		       prCmdPwrLimit->cPwrLimit40H,
		       prCmdPwrLimit->cPwrLimit80L,
		       prCmdPwrLimit->cPwrLimit80H,
		       prCmdPwrLimit->cPwrLimit160L,
		       prCmdPwrLimit->cPwrLimit160H,
		       prCmdPwrLimit->ucFlag);

		prCmdPwrLimit++;
	}

	u4SetQueryInfoLen =
		(sizeof(struct CMD_SET_COUNTRY_CHANNEL_POWER_LIMIT) +
		(prCmd->ucNum) * sizeof(struct CMD_CHANNEL_POWER_LIMIT));

	/* Update domain info to chip */
	if (prCmd->ucNum <= MAX_CMD_SUPPORT_CHANNEL_NUM) {
		rStatus = wlanSendSetQueryCmd(prAdapter,	/* prAdapter */
				CMD_ID_SET_COUNTRY_POWER_LIMIT,	/* ucCID */
				TRUE,	/* fgSetQuery */
				FALSE,	/* fgNeedResp */
				FALSE,	/* fgIsOid */
				NULL,	/* pfCmdDoneHandler */
				NULL,	/* pfCmdTimeoutHandler */
				u4SetQueryInfoLen,	/* u4SetQueryInfoLen */
				(uint8_t *) prCmd,	/* pucInfoBuffer */
				NULL,	/* pvSetQueryBuffer */
				0	/* u4SetQueryBufferLen */
		    );
	} else {
		DBGLOG(RLM, ERROR, "Domain: illegal power limit table\n");
	}

	/* ASSERT(rStatus == WLAN_STATUS_PENDING); */

	cnmMemFree(prAdapter, prCmd);

}
#endif
u_int8_t regd_is_single_sku_en(void)
{
#if (CFG_SUPPORT_SINGLE_SKU == 1)
	return g_mtk_regd_control.en;
#else
	return FALSE;
#endif
}

#if (CFG_SUPPORT_SINGLE_SKU == 1)
u_int8_t rlmDomainIsCtrlStateEqualTo(enum regd_state state)
{
	return (g_mtk_regd_control.state == state) ? TRUE : FALSE;
}

enum regd_state rlmDomainGetCtrlState(void)
{
	return g_mtk_regd_control.state;
}


void rlmDomainResetActiveChannel(void)
{
	g_mtk_regd_control.n_channel_active_2g = 0;
	g_mtk_regd_control.n_channel_active_5g = 0;
}

void rlmDomainAddActiveChannel(u8 band)

{
	if (band == KAL_BAND_2GHZ)
		g_mtk_regd_control.n_channel_active_2g += 1;
	else if (band == KAL_BAND_5GHZ)
		g_mtk_regd_control.n_channel_active_5g += 1;
}

u8 rlmDomainGetActiveChannelCount(u8 band)
{
	if (band == KAL_BAND_2GHZ)
		return g_mtk_regd_control.n_channel_active_2g;
	else if (band == KAL_BAND_5GHZ)
		return g_mtk_regd_control.n_channel_active_5g;
	else
		return 0;
}

struct CMD_DOMAIN_CHANNEL *rlmDomainGetActiveChannels(void)
{
	return g_mtk_regd_control.channels;
}

void rlmDomainSetDefaultCountryCode(void)
{
	g_mtk_regd_control.alpha2 = COUNTRY_CODE_WW;
}

void rlmDomainResetCtrlInfo(u_int8_t force)
{
	if ((g_mtk_regd_control.state == REGD_STATE_UNDEFINED) ||
	    (force == TRUE)) {
		memset(&g_mtk_regd_control, 0, sizeof(struct mtk_regd_control));

		g_mtk_regd_control.state = REGD_STATE_INIT;

		rlmDomainSetDefaultCountryCode();

#if (CFG_SUPPORT_SINGLE_SKU_LOCAL_DB == 1)
		g_mtk_regd_control.flag |= REGD_CTRL_FLAG_SUPPORT_LOCAL_REGD_DB;
#endif
	}
}

u_int8_t rlmDomainIsUsingLocalRegDomainDataBase(void)
{
#if (CFG_SUPPORT_SINGLE_SKU_LOCAL_DB == 1)
	return (g_mtk_regd_control.flag & REGD_CTRL_FLAG_SUPPORT_LOCAL_REGD_DB)
		? TRUE : FALSE;
#else
	return FALSE;
#endif
}

bool rlmDomainIsSameCountryCode(char *alpha2, u8 size_of_alpha2)
{
	u8 idx;
	u32 alpha2_hex = 0;

	for (idx = 0; idx < size_of_alpha2; idx++)
		alpha2_hex |= (alpha2[idx] << (idx * 8));

	return (rlmDomainGetCountryCode() == alpha2_hex) ? TRUE : FALSE;
}

void rlmDomainSetCountryCode(char *alpha2, u8 size_of_alpha2)
{
	u8 max;
	u8 buf_size;

	buf_size = sizeof(g_mtk_regd_control.alpha2);
	max = (buf_size < size_of_alpha2) ? buf_size : size_of_alpha2;

	g_mtk_regd_control.alpha2 = rlmDomainAlpha2ToU32(alpha2, max);
}
void rlmDomainSetDfsRegion(enum nl80211_dfs_regions dfs_region)
{
	g_mtk_regd_control.dfs_region = dfs_region;
}

enum nl80211_dfs_regions rlmDomainGetDfsRegion(void)
{
	return g_mtk_regd_control.dfs_region;
}

void rlmDomainSetTempCountryCode(char *alpha2, u8 size_of_alpha2)
{
	u8 idx, max;
	u8 buf_size;

	buf_size = sizeof(g_mtk_regd_control.tmp_alpha2);
	max = (buf_size < size_of_alpha2) ? buf_size : size_of_alpha2;

	g_mtk_regd_control.tmp_alpha2 = 0;

	for (idx = 0; idx < max; idx++)
		g_mtk_regd_control.tmp_alpha2 |= (alpha2[idx] << (idx * 8));

}

enum regd_state rlmDomainStateTransition(enum regd_state request_state,
					 struct regulatory_request *pRequest)
{
	enum regd_state next_state, old_state;
	bool the_same = 0;

	old_state = g_mtk_regd_control.state;
	next_state = REGD_STATE_INVALID;

	if (old_state == REGD_STATE_INVALID)
		DBGLOG(RLM, ERROR,
		       "%s(): invalid state. trasntion is not allowed.\n",
		       __func__);

	switch (request_state) {
	case REGD_STATE_SET_WW_CORE:
		if ((old_state == REGD_STATE_SET_WW_CORE) ||
		    (old_state == REGD_STATE_INIT) ||
		    (old_state == REGD_STATE_SET_COUNTRY_USER) ||
		    (old_state == REGD_STATE_SET_COUNTRY_IE))
			next_state = request_state;
		break;
	case REGD_STATE_SET_COUNTRY_USER:
		/* Allow user to set multiple times */
		if ((old_state == REGD_STATE_SET_WW_CORE) ||
		    (old_state == REGD_STATE_INIT) ||
		    (old_state == REGD_STATE_SET_COUNTRY_USER) ||
		    (old_state == REGD_STATE_SET_COUNTRY_IE))
			next_state = request_state;
		else
			DBGLOG(RLM, ERROR, "Invalid old state = %d\n",
			       old_state);
		break;
	case REGD_STATE_SET_COUNTRY_DRIVER:
		if (old_state == REGD_STATE_SET_COUNTRY_USER) {
			/*
			 * Error.
			 * Mixing using set_country_by_user and
			 * set_country_by_driver is not allowed.
			 */
			break;
		}

		next_state = request_state;
		break;
	case REGD_STATE_SET_COUNTRY_IE:
		next_state = request_state;
		break;
	default:
		break;
	}

	if (next_state == REGD_STATE_INVALID) {
		DBGLOG(RLM, ERROR,
		       "%s():  ERROR. trasntion to invalid state. o=%x, r=%x, s=%x\n",
		       __func__, old_state, request_state, the_same);
	} else
		DBGLOG(RLM, INFO, "%s():  trasntion to state = %x (old = %x)\n",
		__func__, next_state, g_mtk_regd_control.state);

	g_mtk_regd_control.state = next_state;

	return g_mtk_regd_control.state;
}

/**
 * rlmDomainChannelFlagString - Transform channel flags to readable string
 *
 * @ flags: the ieee80211_channel->flags for a channel
 * @ buf: string buffer to put the transformed string
 * @ buf_size: size of the buf
 **/
void rlmDomainChannelFlagString(u32 flags, char *buf, size_t buf_size)
{
	int32_t buf_written = 0;

	if (!flags || !buf || !buf_size)
		return;

	if (flags & IEEE80211_CHAN_DISABLED) {
		LOGBUF(buf, ((int32_t)buf_size), buf_written, "DISABLED ");
		/* If DISABLED, don't need to check other flags */
		return;
	}
	if (flags & IEEE80211_CHAN_PASSIVE_FLAG)
		LOGBUF(buf, ((int32_t)buf_size), buf_written,
		       IEEE80211_CHAN_PASSIVE_STR " ");
	if (flags & IEEE80211_CHAN_RADAR)
		LOGBUF(buf, ((int32_t)buf_size), buf_written, "RADAR ");
	if (flags & IEEE80211_CHAN_NO_HT40PLUS)
		LOGBUF(buf, ((int32_t)buf_size), buf_written, "NO_HT40PLUS ");
	if (flags & IEEE80211_CHAN_NO_HT40MINUS)
		LOGBUF(buf, ((int32_t)buf_size), buf_written, "NO_HT40MINUS ");
	if (flags & IEEE80211_CHAN_NO_80MHZ)
		LOGBUF(buf, ((int32_t)buf_size), buf_written, "NO_80MHZ ");
	if (flags & IEEE80211_CHAN_NO_160MHZ)
		LOGBUF(buf, ((int32_t)buf_size), buf_written, "NO_160MHZ ");
}

void rlmDomainParsingChannel(IN struct wiphy *pWiphy)
{
	u32 band_idx, ch_idx;
	u32 ch_count;
	struct ieee80211_supported_band *sband;
	struct ieee80211_channel *chan;
	struct CMD_DOMAIN_CHANNEL *pCh;
	char chan_flag_string[64] = {0};


	if (!pWiphy) {
		DBGLOG(RLM, ERROR, "%s():  ERROR. pWiphy = NULL.\n", __func__);
		ASSERT(0);
		return;
	}


	/*
	 * Ready to parse the channel for bands
	 */

	rlmDomainResetActiveChannel();

	ch_count = 0;
	for (band_idx = 0; band_idx < KAL_NUM_BANDS; band_idx++) {
		sband = pWiphy->bands[band_idx];
		if (!sband)
			continue;

		for (ch_idx = 0; ch_idx < sband->n_channels; ch_idx++) {
			chan = &sband->channels[ch_idx];
			pCh = (rlmDomainGetActiveChannels() + ch_count);
			/* Parse flags and get readable string */
			rlmDomainChannelFlagString(chan->flags,
						   chan_flag_string,
						   sizeof(chan_flag_string));

			if (chan->flags & IEEE80211_CHAN_DISABLED) {
				DBGLOG(RLM, INFO,
				       "channels[%d][%d]: ch%d (freq = %d) flags=0x%x [ %s]\n",
				    band_idx, ch_idx, chan->hw_value,
				    chan->center_freq, chan->flags,
				    chan_flag_string);
				continue;
			}

			/* Allowable channel */
			if (ch_count == MAX_SUPPORTED_CH_COUNT) {
				DBGLOG(RLM, ERROR,
				       "%s(): no buffer to store channel information.\n",
				       __func__);
				break;
			}

			rlmDomainAddActiveChannel(band_idx);

			DBGLOG(RLM, INFO,
			       "channels[%d][%d]: ch%d (freq = %d) flgs=0x%x [%s]\n",
				band_idx, ch_idx, chan->hw_value,
				chan->center_freq, chan->flags,
				chan_flag_string);

			pCh->u2ChNum = chan->hw_value;
			pCh->eFlags = chan->flags;

			ch_count += 1;
		}

	}
}
void rlmExtractChannelInfo(u32 max_ch_count,
			   struct CMD_DOMAIN_ACTIVE_CHANNEL_LIST *prBuff)
{
	u32 ch_count, idx;
	struct CMD_DOMAIN_CHANNEL *pCh;

	prBuff->u1ActiveChNum2g = rlmDomainGetActiveChannelCount(KAL_BAND_2GHZ);
	prBuff->u1ActiveChNum5g = rlmDomainGetActiveChannelCount(KAL_BAND_5GHZ);
	ch_count = prBuff->u1ActiveChNum2g + prBuff->u1ActiveChNum5g;

	if (ch_count > max_ch_count) {
		ch_count = max_ch_count;
		DBGLOG(RLM, WARN,
		       "%s(); active channel list is not a complete one.\n",
		       __func__);
	}

	for (idx = 0; idx < ch_count; idx++) {
		pCh = &(prBuff->arChannels[idx]);

		pCh->u2ChNum = (rlmDomainGetActiveChannels() + idx)->u2ChNum;
		pCh->eFlags = (rlmDomainGetActiveChannels() + idx)->eFlags;
	}

}

const struct ieee80211_regdomain
*rlmDomainSearchRegdomainFromLocalDataBase(char *alpha2)
{
#if (CFG_SUPPORT_SINGLE_SKU_LOCAL_DB == 1)
	u8 idx;
	const struct mtk_regdomain *prRegd;

	idx = 0;
	while (g_prRegRuleTable[idx]) {
		prRegd = g_prRegRuleTable[idx];

		if ((prRegd->country_code[0] == alpha2[0]) &&
			(prRegd->country_code[1] == alpha2[1]) &&
			(prRegd->country_code[2] == alpha2[2]) &&
			(prRegd->country_code[3] == alpha2[3]))
			return prRegd->prRegdRules;

		idx++;
	}

	DBGLOG(RLM, ERROR,
	       "%s(): Error, Cannot find the correct RegDomain. country = %s\n",
	       __func__, alpha2);
	DBGLOG(RLM, INFO, "    Set as default WW.\n");

	return &default_regdom_ww; /*default world wide*/
#else
	return NULL;
#endif
}


const struct ieee80211_regdomain *rlmDomainGetLocalDefaultRegd(void)
{
#if (CFG_SUPPORT_SINGLE_SKU_LOCAL_DB == 1)
	return &default_regdom_ww;
#else
	return NULL;
#endif
}
struct GLUE_INFO *rlmDomainGetGlueInfo(void)
{
	return g_mtk_regd_control.pGlueInfo;
}

bool rlmDomainIsEfuseUsed(void)
{
	return g_mtk_regd_control.isEfuseCountryCodeUsed;
}

uint8_t rlmDomainGetChannelBw(uint8_t channelNum)
{
	uint32_t ch_idx = 0, start_idx = 0, end_idx = 0;
	uint8_t channelBw = MAX_BW_80_80_MHZ;
	struct CMD_DOMAIN_CHANNEL *pCh;

	end_idx = rlmDomainGetActiveChannelCount(KAL_BAND_2GHZ)
			+ rlmDomainGetActiveChannelCount(KAL_BAND_5GHZ);

	for (ch_idx = start_idx; ch_idx < end_idx; ch_idx++) {
		pCh = (rlmDomainGetActiveChannels() + ch_idx);

		if (pCh->u2ChNum != channelNum)
			continue;

		/* Max BW */
		if ((pCh->eFlags & IEEE80211_CHAN_NO_160MHZ)
						== IEEE80211_CHAN_NO_160MHZ)
			channelBw = MAX_BW_80MHZ;
		if ((pCh->eFlags & IEEE80211_CHAN_NO_80MHZ)
						== IEEE80211_CHAN_NO_80MHZ)
			channelBw = MAX_BW_40MHZ;
		if ((pCh->eFlags & IEEE80211_CHAN_NO_HT40)
						== IEEE80211_CHAN_NO_HT40)
			channelBw = MAX_BW_20MHZ;
	}

	DBGLOG(RLM, INFO, "ch=%d, BW=%d\n", channelNum, channelBw);
	return channelBw;
}
#endif

uint32_t rlmDomainExtractSingleSkuInfoFromFirmware(IN struct ADAPTER *prAdapter,
						   IN uint8_t *pucEventBuf)
{
#if (CFG_SUPPORT_SINGLE_SKU == 1)
	struct SINGLE_SKU_INFO *prSkuInfo =
			(struct SINGLE_SKU_INFO *) pucEventBuf;

	g_mtk_regd_control.en = TRUE;

	if (prSkuInfo->isEfuseValid) {
		if (!rlmDomainIsUsingLocalRegDomainDataBase()) {

			DBGLOG(RLM, ERROR,
			       "%s(): Error. In efuse mode, must use local data base.\n",
			       __func__);

			ASSERT(0);
			/* force using local db if getting
			 * country code from efuse
			 */
			return WLAN_STATUS_NOT_SUPPORTED;
		}

		rlmDomainSetCountryCode((char *) &prSkuInfo->u4EfuseCountryCode,
					sizeof(prSkuInfo->u4EfuseCountryCode));
		g_mtk_regd_control.isEfuseCountryCodeUsed = TRUE;

	}
#endif

	return WLAN_STATUS_SUCCESS;
}

void rlmDomainSendInfoToFirmware(IN struct ADAPTER *prAdapter)
{
#if (CFG_SUPPORT_SINGLE_SKU == 1)
	struct regulatory_request request;
	struct regulatory_request *prReq = NULL;

	if (!regd_is_single_sku_en())
		return; /*not support single sku*/

	if (g_mtk_regd_control.isEfuseCountryCodeUsed) {
		request.initiator = NL80211_REGDOM_SET_BY_DRIVER;
		prReq = &request;
	}

	g_mtk_regd_control.pGlueInfo = prAdapter->prGlueInfo;
	mtk_reg_notify(priv_to_wiphy(prAdapter->prGlueInfo), prReq);
#endif
}

enum ENUM_CHNL_EXT rlmSelectSecondaryChannelType(struct ADAPTER *prAdapter,
						 enum ENUM_BAND band,
						 u8 primary_ch)
{
#if (CFG_SUPPORT_SINGLE_SKU == 1)
	u8 below_ch, above_ch;

	below_ch = primary_ch - CHNL_SPAN_20;
	above_ch = primary_ch + CHNL_SPAN_20;

	if (rlmDomainIsLegalChannel(prAdapter, band, above_ch))
		return CHNL_EXT_SCA;

	if (rlmDomainIsLegalChannel(prAdapter, band, below_ch))
		return CHNL_EXT_SCB;

#endif

	return CHNL_EXT_SCN;
}

void rlmDomainOidSetCountry(IN struct ADAPTER *prAdapter, char *country,
			    u8 size_of_country)
{
#if (CFG_SUPPORT_SINGLE_SKU == 1)
	struct regulatory_request request;

	if (rlmDomainIsUsingLocalRegDomainDataBase()) {
		rlmDomainSetTempCountryCode(country, size_of_country);
		request.initiator = NL80211_REGDOM_SET_BY_DRIVER;
		mtk_reg_notify(priv_to_wiphy(prAdapter->prGlueInfo), &request);
	} else {
		DBGLOG(RLM, INFO,
		       "%s(): Using driver hint to query CRDA getting regd.\n",
		       __func__);
		regulatory_hint(priv_to_wiphy(prAdapter->prGlueInfo), country);
	}
#endif
}

u32 rlmDomainGetCountryCode(void)
{
#if (CFG_SUPPORT_SINGLE_SKU == 1)
	return g_mtk_regd_control.alpha2;
#else
	return 0;
#endif
}

u32 rlmDomainGetTempCountryCode(void)
{
#if (CFG_SUPPORT_SINGLE_SKU == 1)
	return g_mtk_regd_control.tmp_alpha2;
#else
	return 0;
#endif
}

void rlmDomainAssert(u_int8_t cond)
{
	/* bypass this check because single sku is not enable */
	if (!regd_is_single_sku_en())
		return;

	if (!cond) {
		WARN_ON(1);
		DBGLOG(RLM, ERROR, "[WARNING!!] RLM unexpected case.\n");
	}

}


