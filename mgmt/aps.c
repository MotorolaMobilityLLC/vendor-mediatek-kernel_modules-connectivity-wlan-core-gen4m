// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2021 MediaTek Inc.
 */

#include "precomp.h"
#include "gl_kal.h"

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */

/*
 * definition for AP selection algrithm
 */
#define BSS_FULL_SCORE                          (100)
#define CHNL_BSS_NUM_THRESOLD                   100
#define BSS_STA_CNT_THRESOLD                    30
#define SCORE_PER_AP                            1
#define ROAMING_NO_SWING_SCORE_STEP             100
/* MCS9 at BW 160 requires rssi at least -48dbm */
#define BEST_RSSI                               -48
/* MCS7 at 20BW, MCS5 at 40BW, MCS4 at 80BW, MCS3 at 160BW */
#define GOOD_RSSI_FOR_HT_VHT                    -64
/* Link speed 1Mbps need at least rssi -94dbm for 2.4G */
#define MINIMUM_RSSI_2G4                        -94
/* Link speed 6Mbps need at least rssi -86dbm for 5G */
#define MINIMUM_RSSI_5G                         -86
#if (CFG_SUPPORT_WIFI_6G == 1)
/* Link speed 6Mbps need at least rssi -86dbm for 6G */
#define MINIMUM_RSSI_6G                         -86
#endif

/* level of rssi range on StatusBar */
#define RSSI_MAX_LEVEL                          -55
#define RSSI_SECOND_LEVEL                       -66

/* Real Rssi of a Bss may range in current_rssi - 5 dbm
 *to current_rssi + 5 dbm
 */
#define RSSI_DIFF_BIG_STEP			15 /* dbm */
#define RSSI_DIFF_MED_STEP			10 /* dbm */
#define RSSI_DIFF_SML_STEP			5 /* dbm */
#define LOW_RSSI_FOR_5G_BAND                    -70 /* dbm */
#define HIGH_RSSI_FOR_5G_BAND                   -60 /* dbm */

#define CHNL_DWELL_TIME_DEFAULT  100
#define CHNL_DWELL_TIME_ONLINE   50

#define WEIGHT_IDX_CHNL_UTIL                    0
#define WEIGHT_IDX_RSSI                         2
#define WEIGHT_IDX_SCN_MISS_CNT                 2
#define WEIGHT_IDX_PROBE_RSP                    1
#define WEIGHT_IDX_CLIENT_CNT                   0
#define WEIGHT_IDX_AP_NUM                       0
#define WEIGHT_IDX_5G_BAND                      2
#define WEIGHT_IDX_BAND_WIDTH                   1
#define WEIGHT_IDX_STBC                         1
#define WEIGHT_IDX_DEAUTH_LAST                  1
#define WEIGHT_IDX_BLOCK_LIST                   2
#define WEIGHT_IDX_SAA                          0
#define WEIGHT_IDX_CHNL_IDLE                    1
#define WEIGHT_IDX_OPCHNL                       0
#define WEIGHT_IDX_TPUT                         1
#define WEIGHT_IDX_PREFERENCE                   2

#define WEIGHT_IDX_CHNL_UTIL_PER                0
#define WEIGHT_IDX_RSSI_PER                     4
#define WEIGHT_IDX_SCN_MISS_CNT_PER             4
#define WEIGHT_IDX_PROBE_RSP_PER                1
#define WEIGHT_IDX_CLIENT_CNT_PER               1
#define WEIGHT_IDX_AP_NUM_PER                   6
#define WEIGHT_IDX_5G_BAND_PER                  4
#define WEIGHT_IDX_BAND_WIDTH_PER               1
#define WEIGHT_IDX_STBC_PER                     1
#define WEIGHT_IDX_DEAUTH_LAST_PER              1
#define WEIGHT_IDX_BLOCK_LIST_PER               4
#define WEIGHT_IDX_SAA_PER                      1
#define WEIGHT_IDX_CHNL_IDLE_PER                6
#define WEIGHT_IDX_OPCHNL_PER                   6
#define WEIGHT_IDX_TPUT_PER                     2
#define WEIGHT_IDX_PREFERENCE_PER               2

#define ROAM_SCORE_DELTA                        5
#define BSS_MATCH_BSSID_SCORE			(30000)
#define BSS_MATCH_BSSID_HINT_SCORE		(20000)

#define APS_AMSDU_HT_3K                         (3839)
#define APS_AMSDU_HT_8K                         (7935)
#define APS_AMSDU_VHT_HE_EHT_3K                 (3895)
#define APS_AMSDU_VHT_HE_EHT_8K                 (7991)
#define APS_AMSDU_VHT_HE_EHT_11K                (11454)

#define WEIGHT_GBAND_COEX_DOWNGRADE		70 /* 0~100 */
#define CU_6G_INDEX_OFFSET			256
#define WEIGHT_MCC_DOWNGRADE			70 /* 0~100 */

/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */

struct WEIGHT_CONFIG {
	uint8_t ucChnlUtilWeight;
	uint8_t ucSnrWeight;
	uint8_t ucRssiWeight;
	uint8_t ucProbeRespWeight;
	uint8_t ucClientCntWeight;
	uint8_t ucApNumWeight;
	uint8_t ucBandWeight;
	uint8_t ucBandWidthWeight;
	uint8_t ucStbcWeight;
	uint8_t ucLastDeauthWeight;
	uint8_t ucBlockListWeight;
	uint8_t ucSaaWeight;
	uint8_t ucChnlIdleWeight;
	uint8_t ucOpchnlWeight;
	uint8_t ucTputWeight;
	uint8_t ucPreferenceWeight;
};

/*******************************************************************************
 *                            P U B L I C   D A T A
 *******************************************************************************
 */

/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */

enum ENUM_APS_REPLACE_REASON {
	/* reason to not replace */
	APS_LOW_SCORE = 0,
	APS_UNMATCH_BSSID,
	APS_UNMATCH_BSSID_HINT,
	APS_WORSE_RSSI,
	APS_LAST_DEAUTH,

	/* reason to replace */
	APS_FIRST_CANDIDATE,
	APS_HIGH_SCORE,
	APS_MATCH_BSSID,
	APS_MATCH_BSSID_HINT,
	APS_BETTER_RSSI,
	APS_NOT_LAST_DEAUTH,

	/* don't add after this */
	APS_REPLACE_REASON_NUM,
	APS_NEED_REPLACE = APS_FIRST_CANDIDATE,
};

/* mapping with ENUM_APS_REPLACE_REASON */
static const char * const apucReplaceReasonStr[APS_REPLACE_REASON_NUM] = {
	"LOW SCORE",
	"UNMATCH BSSID",
	"UNMATCH BSSID_HINT",
	"WORSE RSSI",
	"LAST DEAUTH",
	"FIRST CANDIDATE",
	"HIGH SCORE",
	"MATCH BSSID",
	"MATCH BSSID_HINT",
	"BETTER RSSI",
	"NOT LAST DEAUTH",
};

static const char * const apucLinkPlanStr[MLO_LINK_PLAN_NUM] = {
	"2G",
	"5G",
	"2G_5G",
	"5G_5G",
	"2G_5G_5G",
#if (CFG_SUPPORT_WIFI_6G == 1)
	"6G",
	"2G_6G",
	"5G_6G",
	"6G_6G",
	"2G_5G_6G",
	"2G_6G_6G",
#endif
};

struct WEIGHT_CONFIG gasMtkWeightConfig[ROAM_TYPE_NUM] = {
	[ROAM_TYPE_RCPI] = {
		.ucChnlUtilWeight = WEIGHT_IDX_CHNL_UTIL,
		.ucRssiWeight = WEIGHT_IDX_RSSI,
		.ucProbeRespWeight = WEIGHT_IDX_PROBE_RSP,
		.ucClientCntWeight = WEIGHT_IDX_CLIENT_CNT,
		.ucApNumWeight = WEIGHT_IDX_AP_NUM,
		.ucBandWeight = WEIGHT_IDX_5G_BAND,
		.ucBandWidthWeight = WEIGHT_IDX_BAND_WIDTH,
		.ucStbcWeight = WEIGHT_IDX_STBC,
		.ucLastDeauthWeight = WEIGHT_IDX_DEAUTH_LAST,
		.ucBlockListWeight = WEIGHT_IDX_BLOCK_LIST,
		.ucSaaWeight = WEIGHT_IDX_SAA,
		.ucChnlIdleWeight = WEIGHT_IDX_CHNL_IDLE,
		.ucOpchnlWeight = WEIGHT_IDX_OPCHNL,
		.ucTputWeight = WEIGHT_IDX_TPUT,
		.ucPreferenceWeight = WEIGHT_IDX_PREFERENCE
	}
#if CFG_SUPPORT_ROAMING
	, [ROAM_TYPE_PER] = {
		.ucChnlUtilWeight = WEIGHT_IDX_CHNL_UTIL_PER,
		.ucRssiWeight = WEIGHT_IDX_RSSI_PER,
		.ucProbeRespWeight = WEIGHT_IDX_PROBE_RSP_PER,
		.ucClientCntWeight = WEIGHT_IDX_CLIENT_CNT_PER,
		.ucApNumWeight = WEIGHT_IDX_AP_NUM_PER,
		.ucBandWeight = WEIGHT_IDX_5G_BAND_PER,
		.ucBandWidthWeight = WEIGHT_IDX_BAND_WIDTH_PER,
		.ucStbcWeight = WEIGHT_IDX_STBC_PER,
		.ucLastDeauthWeight = WEIGHT_IDX_DEAUTH_LAST_PER,
		.ucBlockListWeight = WEIGHT_IDX_BLOCK_LIST_PER,
		.ucSaaWeight = WEIGHT_IDX_SAA_PER,
		.ucChnlIdleWeight = WEIGHT_IDX_CHNL_IDLE_PER,
		.ucOpchnlWeight = WEIGHT_IDX_OPCHNL_PER,
		.ucTputWeight = WEIGHT_IDX_TPUT_PER,
		.ucPreferenceWeight = WEIGHT_IDX_PREFERENCE_PER
	}
#endif
};

static uint8_t aucBaSizeTranslate[8] = {
	[0] = 0,
	[1] = 2,
	[2] = 4,
	[3] = 6,
	[4] = 8,
	[5] = 16,
	[6] = 32,
	[7] = 64
};

#if (CFG_SUPPORT_AVOID_DESENSE == 1)
const struct WFA_DESENSE_CHANNEL_LIST desenseChList[BAND_NUM] = {
	[BAND_5G]  = {120, 157},
#if (CFG_SUPPORT_WIFI_6G == 1)
	[BAND_6G]  = {13,  53},
#endif
};
#endif

const uint16_t mpduLen[CW_320_2MHZ + 1] = {
	[CW_20_40MHZ]  = 40,
	[CW_80MHZ] = 80,
	[CW_160MHZ] = 160,
	[CW_80P80MHZ] = 160,
	[CW_320_1MHZ]  = 320,
	[CW_320_2MHZ]  = 320
};

#define PERCENTAGE(_val, _base) (_val * 100 / _base)

enum ENUM_BAND g_aeLinkPlan[MLO_LINK_PLAN_NUM][APS_LINK_MAX] = {
	{BAND_2G4, BAND_NULL, BAND_NULL},
	{BAND_5G, BAND_NULL, BAND_NULL},
	{BAND_2G4, BAND_5G, BAND_NULL},
	{BAND_5G, BAND_5G, BAND_NULL},
	{BAND_2G4, BAND_5G, BAND_5G},
#if (CFG_SUPPORT_WIFI_6G == 1)
	{BAND_6G, BAND_NULL, BAND_NULL},
	{BAND_2G4, BAND_6G, BAND_NULL},
	{BAND_5G, BAND_6G, BAND_NULL},
	{BAND_6G, BAND_6G, BAND_NULL},
	{BAND_2G4, BAND_5G, BAND_6G},
	{BAND_2G4, BAND_6G, BAND_6G},
#endif
};

/* Minimum SNR required to achieve a certain bitrate. */
struct minsnr_bitrate_entry {
	int minsnr;
	unsigned int bitrate; /* in Mbps */
};

/* VHT needs to be enabled in order to achieve MCS8 and MCS9 rates. */
static const int vht_mcs = 8;

static const struct minsnr_bitrate_entry vht20_table[] = {
	{ 0, 0 },
	{ 2, 6500 },   /* HT20 MCS0 */
	{ 5, 13000 },  /* HT20 MCS1 */
	{ 9, 19500 },  /* HT20 MCS2 */
	{ 11, 26000 }, /* HT20 MCS3 */
	{ 15, 39000 }, /* HT20 MCS4 */
	{ 18, 52000 }, /* HT20 MCS5 */
	{ 20, 58500 }, /* HT20 MCS6 */
	{ 25, 65000 }, /* HT20 MCS7 */
	{ 29, 78000 }, /* VHT20 MCS8 */
	{ -1, 78000 }  /* SNR > 29 */
};

static const struct minsnr_bitrate_entry vht40_table[] = {
	{ 0, 0 },
	{ 5, 13500 },   /* HT40 MCS0 */
	{ 8, 27000 },   /* HT40 MCS1 */
	{ 12, 40500 },  /* HT40 MCS2 */
	{ 14, 54000 },  /* HT40 MCS3 */
	{ 18, 81000 },  /* HT40 MCS4 */
	{ 21, 108000 }, /* HT40 MCS5 */
	{ 23, 121500 }, /* HT40 MCS6 */
	{ 28, 135000 }, /* HT40 MCS7 */
	{ 32, 162000 }, /* VHT40 MCS8 */
	{ 34, 180000 }, /* VHT40 MCS9 */
	{ -1, 180000 }  /* SNR > 34 */
};

static const struct minsnr_bitrate_entry vht80_table[] = {
	{ 0, 0 },
	{ 8, 29300 },   /* VHT80 MCS0 */
	{ 11, 58500 },  /* VHT80 MCS1 */
	{ 15, 87800 },  /* VHT80 MCS2 */
	{ 17, 117000 }, /* VHT80 MCS3 */
	{ 21, 175500 }, /* VHT80 MCS4 */
	{ 24, 234000 }, /* VHT80 MCS5 */
	{ 26, 263300 }, /* VHT80 MCS6 */
	{ 31, 292500 }, /* VHT80 MCS7 */
	{ 35, 351000 }, /* VHT80 MCS8 */
	{ 37, 390000 }, /* VHT80 MCS9 */
	{ -1, 390000 }  /* SNR > 37 */
};


static const struct minsnr_bitrate_entry vht160_table[] = {
	{ 0, 0 },
	{ 11, 58500 },  /* VHT160 MCS0 */
	{ 14, 117000 }, /* VHT160 MCS1 */
	{ 18, 175500 }, /* VHT160 MCS2 */
	{ 20, 234000 }, /* VHT160 MCS3 */
	{ 24, 351000 }, /* VHT160 MCS4 */
	{ 27, 468000 }, /* VHT160 MCS5 */
	{ 29, 526500 }, /* VHT160 MCS6 */
	{ 34, 585000 }, /* VHT160 MCS7 */
	{ 38, 702000 }, /* VHT160 MCS8 */
	{ 40, 780000 }, /* VHT160 MCS9 */
	{ -1, 780000 }  /* SNR > 37 */
};

/* EHT needs to be enabled in order to achieve MCS12 and MCS13 rates. */
#define EHT_MCS 12

static const struct minsnr_bitrate_entry he20_table[] = {
	{ 0, 0 },
	{ 2, 8600 },    /* HE20 MCS0 */
	{ 5, 17200 },   /* HE20 MCS1 */
	{ 9, 25800 },   /* HE20 MCS2 */
	{ 11, 34400 },  /* HE20 MCS3 */
	{ 15, 51600 },  /* HE20 MCS4 */
	{ 18, 68800 },  /* HE20 MCS5 */
	{ 20, 77400 },  /* HE20 MCS6 */
	{ 25, 86000 },  /* HE20 MCS7 */
	{ 29, 103200 }, /* HE20 MCS8 */
	{ 31, 114700 }, /* HE20 MCS9 */
	{ 34, 129000 }, /* HE20 MCS10 */
	{ 36, 143400 }, /* HE20 MCS11 */
	{ 39, 154900 }, /* EHT20 MCS12 */
	{ 42, 172100 }, /* EHT20 MCS13 */
	{ -1, 172100 }  /* SNR > 42 */
};

static const struct minsnr_bitrate_entry he40_table[] = {
	{ 0, 0 },
	{ 5, 17200 },   /* HE40 MCS0 */
	{ 8, 34400 },   /* HE40 MCS1 */
	{ 12, 51600 },  /* HE40 MCS2 */
	{ 14, 68800 },  /* HE40 MCS3 */
	{ 18, 103200 }, /* HE40 MCS4 */
	{ 21, 137600 }, /* HE40 MCS5 */
	{ 23, 154900 }, /* HE40 MCS6 */
	{ 28, 172100 }, /* HE40 MCS7 */
	{ 32, 206500 }, /* HE40 MCS8 */
	{ 34, 229400 }, /* HE40 MCS9 */
	{ 37, 258100 }, /* HE40 MCS10 */
	{ 39, 286800 }, /* HE40 MCS11 */
	{ 42, 309500 }, /* EHT40 MCS12 */
	{ 45, 344100 }, /* EHT40 MCS13 */
	{ -1, 344100 }  /* SNR > 45 */
};

static const struct minsnr_bitrate_entry he80_table[] = {
	{ 0, 0 },
	{ 8, 36000 },   /* HE80 MCS0 */
	{ 11, 72100 },  /* HE80 MCS1 */
	{ 15, 108100 }, /* HE80 MCS2 */
	{ 17, 144100 }, /* HE80 MCS3 */
	{ 21, 216200 }, /* HE80 MCS4 */
	{ 24, 288200 }, /* HE80 MCS5 */
	{ 26, 324300 }, /* HE80 MCS6 */
	{ 31, 360300 }, /* HE80 MCS7 */
	{ 35, 432400 }, /* HE80 MCS8 */
	{ 37, 480400 }, /* HE80 MCS9 */
	{ 40, 540400 }, /* HE80 MCS10 */
	{ 42, 600500 }, /* HE80 MCS11 */
	{ 45, 648500 }, /* EHT80 MCS12 */
	{ 48, 720600 }, /* EHT80 MCS13 */
	{ -1, 720600 }  /* SNR > 48 */
};


static const struct minsnr_bitrate_entry he160_table[] = {
	{ 0, 0 },
	{ 11, 72100 },   /* HE160 MCS0 */
	{ 14, 144100 },  /* HE160 MCS1 */
	{ 18, 216200 },  /* HE160 MCS2 */
	{ 20, 288200 },  /* HE160 MCS3 */
	{ 24, 432400 },  /* HE160 MCS4 */
	{ 27, 576500 },  /* HE160 MCS5 */
	{ 29, 648500 },  /* HE160 MCS6 */
	{ 34, 720600 },  /* HE160 MCS7 */
	{ 38, 864700 },  /* HE160 MCS8 */
	{ 40, 960800 },  /* HE160 MCS9 */
	{ 43, 1080900 }, /* HE160 MCS10 */
	{ 45, 1201000 }, /* HE160 MCS11 */
	{ 48, 1297100 }, /* EHT160 MCS12 */
	{ 51, 1441200 }, /* EHT160 MCS13 */
	{ -1, 1441200 }  /* SNR > 51 */
};

/* See IEEE P802.11be/D2.0, Table 36-86: EHT-MCSs for 4x996-tone RU, NSS,u = 1
 */
static const struct minsnr_bitrate_entry eht320_table[] = {
	{ 0, 0 },
	{ 14, 144100 },   /* EHT320 MCS0 */
	{ 17, 288200 },   /* EHT320 MCS1 */
	{ 21, 432400 },   /* EHT320 MCS2 */
	{ 23, 576500 },   /* EHT320 MCS3 */
	{ 27, 864700 },   /* EHT320 MCS4 */
	{ 30, 1152900 },  /* EHT320 MCS5 */
	{ 32, 1297100 },  /* EHT320 MCS6 */
	{ 37, 1441200 },  /* EHT320 MCS7 */
	{ 41, 1729400 },  /* EHT320 MCS8 */
	{ 43, 1921500 },  /* EHT320 MCS9 */
	{ 46, 2161800 },  /* EHT320 MCS10 */
	{ 48, 2401900 },  /* EHT320 MCS11 */
	{ 51, 2594100 },  /* EHT320 MCS12 */
	{ 54, 2882400 },  /* EHT320 MCS13 */
	{ -1, 2882400 }   /* SNR > 54 */
};

/*******************************************************************************
 *                                 M A C R O S
 *******************************************************************************
 */

#define MAC_ADDR_HASH(_addr) \
	(_addr[0] ^ _addr[1] ^ _addr[2] ^ _addr[3] ^ _addr[4] ^ _addr[5])
#define AP_HASH(_addr) \
	((uint8_t) (MAC_ADDR_HASH(_addr) & (AP_HASH_SIZE - 1)))

#define CALCULATE_SCORE_BY_DEAUTH(prBssDesc, eRoamType) \
	(gasMtkWeightConfig[eRoamType].ucLastDeauthWeight * \
	(prBssDesc->prBlock && prBssDesc->prBlock->fgDeauthLastTime ? 0 : \
	BSS_FULL_SCORE))

/*******************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */

static uint8_t apsSanityCheckBssDesc(struct ADAPTER *prAdapter,
	struct BSS_DESC *prBssDesc, enum ENUM_ROAMING_REASON eRoamReason,
	uint8_t ucBssIndex);

static uint32_t apsGetEstimatedTput(struct ADAPTER *ad, struct BSS_DESC *bss,
	uint8_t bidx);

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */

struct AP_COLLECTION *apsHashGet(struct ADAPTER *ad,
	uint8_t *addr, uint8_t bidx)
{
	struct AIS_SPECIFIC_BSS_INFO *s = aisGetAisSpecBssInfo(ad, bidx);
	struct AP_COLLECTION *a = NULL;

	a = s->arApHash[AP_HASH(addr)];

	while (a != NULL &&
	       UNEQUAL_MAC_ADDR(a->aucAddr, addr))
		a = a->hnext;
	return a;
}

void apsHashAdd(struct ADAPTER *ad, struct AP_COLLECTION *ap, uint8_t bidx)
{
	struct AIS_SPECIFIC_BSS_INFO *s = aisGetAisSpecBssInfo(ad, bidx);

	ap->hnext = s->arApHash[AP_HASH(ap->aucAddr)];
	s->arApHash[AP_HASH(ap->aucAddr)] = ap;
}

void apsHashDel(struct ADAPTER *ad, struct AP_COLLECTION *ap, uint8_t bidx)
{
	struct AIS_SPECIFIC_BSS_INFO *s = aisGetAisSpecBssInfo(ad, bidx);
	struct AP_COLLECTION *a = NULL;

	a = s->arApHash[AP_HASH(ap->aucAddr)];

	if (a == NULL)
		return;

	if (EQUAL_MAC_ADDR(a->aucAddr, ap->aucAddr)) {
		s->arApHash[AP_HASH(ap->aucAddr)] = a->hnext;
		return;
	}

	while (a->hnext != NULL &&
	       UNEQUAL_MAC_ADDR(a->hnext->aucAddr, ap->aucAddr)) {
		a = a->hnext;
	}
	if (a->hnext != NULL)
		a->hnext = a->hnext->hnext;
	else
		DBGLOG(APS, INFO, "Could not remove AP " MACSTR
			   " from hash table\n", MAC2STR(ap->aucAddr));
}

uint8_t apsCanFormMld(struct ADAPTER *ad,
	struct BSS_DESC *bss, uint8_t bidx)
{
#if (CFG_SUPPORT_802_11BE_MLO == 1)
	struct WIFI_VAR *prWifiVar = &ad->rWifiVar;

#if (CFG_SUPPORT_MLO_STA_NAN_FALLBACK == 1)
	if (nanIsConcurrency(ad))
		return FALSE;
#endif

	if (!mldIsMultiLinkEnabled(ad, NETWORK_TYPE_AIS, bidx) ||
	    !aisSecondLinkAvailable(ad, bidx))
		return FALSE;

	if (bss->fgIsEHTPresent == FALSE ||
	    (!prWifiVar->fgDisSecurityCheck &&
	     !rsnIsKeyMgmtForEht(ad, bss, bidx)))
		return FALSE;

	if (bss->rMlInfo.fgValid) {
		bss->rMlInfo.prBlock = aisQueryMldBlockList(ad, bss);
		return TRUE;
	}
#endif
	return FALSE;
}

uint8_t apsBssDescToLink(struct ADAPTER *ad,
	struct AP_COLLECTION *ap, struct BSS_DESC *bss, uint8_t bidx)
{
	return bss->eBand;
}

uint32_t apsAddBssDescToList(struct ADAPTER *ad, struct AP_COLLECTION *ap,
			struct BSS_DESC *bss, uint8_t bidx)
{
	uint8_t aidx = AIS_INDEX(ad, bidx);
	uint8_t l = apsBssDescToLink(ad, ap, bss, bidx);

	if (l >= BAND_NUM)
		return WLAN_STATUS_FAILURE;

	LINK_ENTRY_INITIALIZE(&bss->rLinkEntryEss[aidx]);
	LINK_INSERT_TAIL(&ap->arLinks[l], &bss->rLinkEntryEss[aidx]);
	ap->ucTotalCount++;

	return WLAN_STATUS_SUCCESS;
}

struct AP_COLLECTION *apsAddAp(struct ADAPTER *ad,
	struct BSS_DESC *bss, uint8_t bidx)
{
	struct AIS_SPECIFIC_BSS_INFO *s = aisGetAisSpecBssInfo(ad, bidx);
	struct LINK *ess = &s->rCurEssLink;
	struct AP_COLLECTION *ap;
	uint8_t i;

	ap = kalMemZAlloc(sizeof(*ap), VIR_MEM_TYPE);
	if (!ap) {
		DBGLOG(APS, WARN, "no resource for " MACSTR "\n",
			MAC2STR(bss->aucBSSID));
		return NULL;
	}

	COPY_MAC_ADDR(ap->aucAddr, bss->aucBSSID);

	ap->fgIsMld = apsCanFormMld(ad, bss, bidx);
#if (CFG_SUPPORT_802_11BE_MLO == 1)
	if (ap->fgIsMld)
		COPY_MAC_ADDR(ap->aucAddr, bss->rMlInfo.aucMldAddr);
	ap->prBlock = aisQueryMldBlockList(ad, bss);
#endif

	for (i = 0; i < BAND_NUM; i++)
		LINK_INITIALIZE(&ap->arLinks[i]);

	if (apsAddBssDescToList(ad, ap, bss, bidx)) {
		kalMemFree(ap, VIR_MEM_TYPE, sizeof(*ap));
		return NULL;
	}

	LINK_INSERT_TAIL(ess, &ap->rLinkEntry);
	apsHashAdd(ad, ap, bidx);

	ap->u4Index = ess->u4NumElem - 1;

	DBGLOG(APS, TRACE, "Add CAND[%d][" MACSTR "][MLD=%d]\n",
		ap->u4Index, MAC2STR(ap->aucAddr), ap->fgIsMld);

	return ap;
}

struct AP_COLLECTION *apsGetAp(struct ADAPTER *ad,
	struct BSS_DESC *bss, uint8_t bidx)
{
#if (CFG_SUPPORT_802_11BE_MLO == 1)
	if (apsCanFormMld(ad, bss, bidx))
		return apsHashGet(ad, bss->rMlInfo.aucMldAddr, bidx);
#endif

	return apsHashGet(ad, bss->aucBSSID, bidx);
}

void apsRemoveAp(struct ADAPTER *ad, struct AP_COLLECTION *ap, uint8_t bidx)
{
	DBGLOG(APS, TRACE,
		"Remove CAND[%d][" MACSTR
		"][MLD=%d] LinkNum=%d, TotalCount=%d\n",
		ap->u4Index, MAC2STR(ap->aucAddr), ap->fgIsMld,
		ap->ucLinkNum, ap->ucTotalCount);

	apsHashDel(ad, ap, bidx);
	kalMemFree(ap, VIR_MEM_TYPE, sizeof(*ap));
}

void apsResetEssApList(struct ADAPTER *ad, uint8_t bidx)
{
	struct AIS_SPECIFIC_BSS_INFO *s = aisGetAisSpecBssInfo(ad, bidx);
	struct LINK *ess = &s->rCurEssLink;
	struct AP_COLLECTION *ap;

	while (!LINK_IS_EMPTY(ess)) {
		LINK_REMOVE_HEAD(ess, ap, struct AP_COLLECTION *);
		apsRemoveAp(ad, ap, bidx);
		/* mem is freed, don't use ap after this point */
	}

	kalMemZero(&s->arApHash[0], sizeof(s->arApHash));
	DBGLOG(APS, INFO, "BssIndex:%d reset prCurEssLink done\n", bidx);
}

#if (CFG_EXT_ROAMING == 0) /* Common */
uint8_t apsIsBssQualify(struct ADAPTER *ad, struct BSS_DESC *bss,
	enum ENUM_ROAMING_REASON eRoamReason, uint32_t u4ConnectedApScore,
	uint32_t u4CandidateApScore, uint8_t bidx)
{
	uint16_t delta = 0;

	/* check min rcpi */
	if (bss->ucRCPI < RCPI_FOR_DONT_ROAM) {
		DBGLOG(APS, TRACE, MACSTR " low rssi %d\n",
			MAC2STR(bss->aucBSSID),
			RCPI_TO_dBm(bss->ucRCPI));
		return FALSE;
	}

	/* check min score */
	switch (eRoamReason) {
	case ROAMING_REASON_POOR_RCPI:
	case ROAMING_REASON_INACTIVE:
	case ROAMING_REASON_RETRY:
	{
		delta += ROAM_SCORE_DELTA;

		/* Minimum Roam Delta
		 * Absolute score value comparing to current AP
		 */
		if (u4CandidateApScore <
		    u4ConnectedApScore * (100 + delta) / 100) {
			DBGLOG(APS, TRACE, "BSS[" MACSTR
				"] (%d < %d*%d%%) reason=%d\n",
				MAC2STR(bss->aucBSSID),
				u4CandidateApScore, u4ConnectedApScore,
				100 + delta, eRoamReason);
			return FALSE;
		}

		break;
	}
	case ROAMING_REASON_BEACON_TIMEOUT:
	{
		/* DON'T compare score if roam with BTO */
		break;
	}
	case ROAMING_REASON_SAA_FAIL:
	{
		/* DON'T compare score if roam with emergency */
		break;
	}
	case ROAMING_REASON_BTM:
	{
		/* DON'T compare score if roam with DIS_IMMI_STATE_3 */
		if (aisGetBTMParam(ad, bidx)->ucDisImmiState ==
		    AIS_BTM_DIS_IMMI_STATE_3)
			break;

		if (u4CandidateApScore < u4ConnectedApScore) {
			DBGLOG(APS, TRACE, "BSS[" MACSTR
				"] (%d < %d) reason=%d\n",
				MAC2STR(bss->aucBSSID),
				u4CandidateApScore, u4ConnectedApScore,
				eRoamReason);
			return FALSE;
		}
		break;
	}
	default:
	{
		if (u4CandidateApScore < u4ConnectedApScore) {
			DBGLOG(APS, TRACE, "BSS[" MACSTR
				"] (%d < %d) reason=%d\n",
				MAC2STR(bss->aucBSSID),
				u4CandidateApScore, u4ConnectedApScore,
				eRoamReason);
			return FALSE;
		}
		break;
	}
	}
	return TRUE;
}
#endif

uint16_t apsGetAmsduByte(struct ADAPTER *ad,
	struct BSS_DESC *bss, uint8_t bidx)
{
	uint16_t bssAmsduLen = 0, amsduLen = 0;

#if (CFG_SUPPORT_802_11BE == 1)
	if (bss->fgIsEHTPresent == TRUE &&
	    (ad->rWifiVar.fgDisSecurityCheck ||
	     rsnIsKeyMgmtForEht(ad, bss, bidx))) {
		bssAmsduLen = (bss->u2MaximumMpdu &
			EHT_MAC_CAP_MAX_MPDU_LEN_MASK) & 0xffff;

		if (bssAmsduLen == EHT_MAC_CAP_MAX_MPDU_LEN_8K)
			amsduLen = APS_AMSDU_VHT_HE_EHT_8K;
		else if (bssAmsduLen == EHT_MAC_CAP_MAX_MPDU_LEN_11K)
			amsduLen = APS_AMSDU_VHT_HE_EHT_11K;
		else if (bssAmsduLen == EHT_MAC_CAP_MAX_MPDU_LEN_3K)
			amsduLen = APS_AMSDU_VHT_HE_EHT_3K;
		else {
			DBGLOG(APS, INFO,
			       "Unexpected EHT maximum mpdu length, %d\n",
			       bssAmsduLen);
			amsduLen = APS_AMSDU_VHT_HE_EHT_3K;
		}
		return amsduLen;
	}
#endif

#if (CFG_SUPPORT_WIFI_6G == 1)
	if (bss->eBand == BAND_6G) {
		bssAmsduLen = (bss->u2MaximumMpdu &
			HE_6G_CAP_INFO_MAX_MPDU_LEN_MASK) & 0xffff;

		if (bssAmsduLen == HE_6G_CAP_INFO_MAX_MPDU_LEN_8K)
			amsduLen = APS_AMSDU_VHT_HE_EHT_8K;
		else if (bssAmsduLen == HE_6G_CAP_INFO_MAX_MPDU_LEN_11K)
			amsduLen = APS_AMSDU_VHT_HE_EHT_11K;
		else if (bssAmsduLen == HE_6G_CAP_INFO_MAX_MPDU_LEN_3K)
			amsduLen = APS_AMSDU_VHT_HE_EHT_3K;
		else {
			DBGLOG(APS, INFO,
				"Unexpected HE maximum mpdu length\n");
			amsduLen = APS_AMSDU_VHT_HE_EHT_3K;
		}
		return amsduLen;
	}
#endif

	if (bss->u2MaximumMpdu) {
		bssAmsduLen = (bss->u2MaximumMpdu &
			VHT_CAP_INFO_MAX_MPDU_LEN_MASK) & 0xffff;
		if (bss->fgIsVHTPresent) {
			if (bssAmsduLen == VHT_CAP_INFO_MAX_MPDU_LEN_8K)
				amsduLen = APS_AMSDU_VHT_HE_EHT_8K;
			else if (bssAmsduLen ==
				VHT_CAP_INFO_MAX_MPDU_LEN_11K)
				amsduLen = APS_AMSDU_VHT_HE_EHT_11K;
			else if (bssAmsduLen ==
				VHT_CAP_INFO_MAX_MPDU_LEN_3K)
				amsduLen = APS_AMSDU_VHT_HE_EHT_3K;
			else {
				DBGLOG(APS, INFO,
					"Unexpected VHT maximum mpdu length\n");
				amsduLen = APS_AMSDU_VHT_HE_EHT_3K;
			}
		} else
			amsduLen = APS_AMSDU_HT_8K;
	} else {
		if (bss->fgIsVHTPresent)
			amsduLen = APS_AMSDU_VHT_HE_EHT_3K;
		else
			amsduLen = APS_AMSDU_HT_3K;
	}

	return amsduLen;
}

static unsigned int interpolate_rate(int snr, int snr0, int snr1,
				     int rate0, int rate1)
{
	return rate0 + (snr - snr0) * (rate1 - rate0) / (snr1 - snr0);
}

static unsigned int max_rate(const struct minsnr_bitrate_entry table[],
			     int snr, bool vht)
{
	const struct minsnr_bitrate_entry *prev, *entry = table;

	while ((entry->minsnr != -1) &&
	       (snr >= entry->minsnr) &&
	       (vht || entry - table <= vht_mcs))
		entry++;
	if (entry == table)
		return entry->bitrate;
	prev = entry - 1;
	if (entry->minsnr == -1 || (!vht && entry - table > vht_mcs))
		return prev->bitrate;
	return interpolate_rate(snr, prev->minsnr, entry->minsnr, prev->bitrate,
				entry->bitrate);
}

static unsigned int max_he_eht_rate(const struct minsnr_bitrate_entry table[],
				    int snr, bool eht)
{
	const struct minsnr_bitrate_entry *prev, *entry = table;

	while (entry->minsnr != -1 && snr >= entry->minsnr &&
	       (eht || entry - table <= EHT_MCS))
		entry++;
	if (entry == table)
		return 0;
	prev = entry - 1;
	if (entry->minsnr == -1 || (!eht && entry - table > EHT_MCS))
		return prev->bitrate;
	return interpolate_rate(snr, prev->minsnr, entry->minsnr,
				prev->bitrate, entry->bitrate);
}

uint32_t apsGetMaxRate(struct ADAPTER *ad, struct BSS_DESC *bss, uint8_t bidx)
{
	uint32_t rate = 5400; /* basic rate */
	/* TODO: get noise or snr when scan done */
	int32_t noise = bss->eBand == BAND_2G4 ? -89 : -92;
	int32_t snr = RCPI_TO_dBm(bss->ucRCPI) - noise;
	int32_t bw = MAX_BW_20MHZ, sta_bw = MAX_BW_20MHZ, ap_bw = MAX_BW_20MHZ;
	int32_t	gen = 0, sta_gen = 0, ap_gen = 0;
	struct WIFI_VAR *prWifiVar = &ad->rWifiVar;

	if (prWifiVar->ucStaHt)
		sta_gen = 4;
	if (bss->fgIsHTPresent)
		ap_gen = 4;

	if (prWifiVar->ucStaVht)
		sta_gen = 5;
	if (bss->fgIsVHTPresent)
		ap_gen = 5;

#if (CFG_SUPPORT_802_11AX == 1)
	if (prWifiVar->ucStaVht)
		sta_gen = 6;
	if (bss->fgIsHEPresent)
		ap_gen = 6;
#endif
#if (CFG_SUPPORT_802_11BE == 1)
	if (prWifiVar->ucStaEht)
		sta_gen = 7;
	if (bss->fgIsEHTPresent == TRUE &&
	    (prWifiVar->fgDisSecurityCheck ||
	     rsnIsKeyMgmtForEht(ad, bss, bidx)))
		ap_gen = 7;
#endif

	if (bss->eBand == BAND_2G4)
		sta_bw = prWifiVar->ucSta2gBandwidth;
	if (bss->eBand == BAND_5G)
		sta_bw = prWifiVar->ucSta5gBandwidth;
#if (CFG_SUPPORT_WIFI_6G == 1)
	if (bss->eBand == BAND_6G)
		sta_bw = prWifiVar->ucSta6gBandwidth;
#endif
	ap_bw = rlmGetBssOpBwByChannelWidth(bss->eSco, bss->eChannelWidth);

	/* get min wifi generation & bandwidth */
	gen = kal_min_t(int32_t, ap_gen, sta_gen);
	bw = kal_min_t(int32_t, ap_bw, sta_bw);

#if (CFG_SUPPORT_802_11BE == 1)
	if (gen == 7) {
		uint32_t boost = 30;

		if (bw == MAX_BW_320_2MHZ || bw == MAX_BW_320_1MHZ)
			rate = max_he_eht_rate(eht320_table, snr, TRUE);
		else if (bw == MAX_BW_160MHZ || bw == MAX_BW_80_80_MHZ)
			rate = max_he_eht_rate(he160_table, snr, TRUE);
		else if (bw == MAX_BW_80MHZ)
			rate = max_he_eht_rate(he80_table, snr, TRUE);
		else if (bw == MAX_BW_40MHZ)
			rate = max_he_eht_rate(he40_table, snr, TRUE);
		else
			rate = max_he_eht_rate(he20_table, snr, TRUE);
		rate += boost;

		goto done;
	}
#endif

#if (CFG_SUPPORT_802_11AX == 1)
	if (gen == 6) {
		uint32_t boost = 20;

		if (bw == MAX_BW_160MHZ || bw == MAX_BW_80_80_MHZ)
			rate = max_he_eht_rate(he160_table, snr, FALSE);
		else if (bw == MAX_BW_80MHZ)
			rate = max_he_eht_rate(he80_table, snr, FALSE);
		else if (bw == MAX_BW_40MHZ)
			rate = max_he_eht_rate(he40_table, snr, FALSE);
		else
			rate = max_he_eht_rate(he20_table, snr, FALSE);
		rate += boost;

		goto done;
	}
#endif

	if (gen == 5) {
		uint32_t boost = 10;

		if (bw == MAX_BW_160MHZ || bw == MAX_BW_80_80_MHZ)
			rate = max_rate(vht160_table, snr, TRUE) + boost;
		else if (bw == MAX_BW_80MHZ)
			rate = max_rate(vht80_table, snr, TRUE) + boost;
		else if (bw == MAX_BW_40MHZ)
			rate = max_rate(vht40_table, snr, TRUE) + boost;
		else
			rate = max_rate(vht20_table, snr, TRUE) + boost;
		rate += boost;

		goto done;
	}

	if (gen == 4) {
		if (bw == MAX_BW_40MHZ)
			rate = max_rate(vht40_table, snr, FALSE);
		else
			rate = max_rate(vht20_table, snr, FALSE);
		goto done;
	}

done:
	return rate;
}

void apsRecordCuInfo(struct ADAPTER *ad, struct BSS_DESC *bss,
	uint8_t bidx)
{
	struct APS_INFO *aps = aisGetApsInfo(ad, bidx);
	uint16_t u2CuOffset = 0;

	if (bss->eBand == BAND_2G4 || bss->eBand == BAND_5G)
		u2CuOffset = bss->ucChannelNum;
#if (CFG_SUPPORT_WIFI_6G == 1)
	else if (bss->eBand == BAND_6G)
		u2CuOffset = CU_6G_INDEX_OFFSET + bss->ucChannelNum;
#endif

	aps->arCuInfo[u2CuOffset].eBand = bss->eBand;
	aps->arCuInfo[u2CuOffset].u4TotalCount++;
	aps->arCuInfo[u2CuOffset].u4TotalCu += bss->ucChnlUtilization;
}

struct RF_CHANNEL_INFO apsGetRfChannelInfo(struct ADAPTER *prAdapter,
	struct BSS_DESC *prBssDesc, struct BSS_INFO *prBssInfo)
{
	struct RF_CHANNEL_INFO rRfChnlInfo = {0};

	if (!prBssDesc && !prBssInfo)
		return rRfChnlInfo;

	if (prBssDesc) {
		rRfChnlInfo.eBand = prBssDesc->eBand;
		rRfChnlInfo.ucChannelNum = prBssDesc->ucChannelNum;
		rRfChnlInfo.ucChnlBw = rlmGetBssOpBwByChannelWidth(
						prBssDesc->eSco,
						prBssDesc->eChannelWidth);
		rRfChnlInfo.eSco = prBssDesc->eSco;
	} else {
		rRfChnlInfo.eBand = prBssInfo->eBand;
		rRfChnlInfo.ucChannelNum = prBssInfo->ucPrimaryChannel;
		rRfChnlInfo.ucChnlBw = rlmGetBssOpBwByChannelWidth(
						prBssInfo->eBssSCO,
						prBssInfo->ucVhtChannelWidth);
		rRfChnlInfo.eSco = prBssInfo->eBssSCO;
	}

	rRfChnlInfo.u4CenterFreq1 = nicGetS1Freq(rRfChnlInfo.eBand,
						 rRfChnlInfo.ucChannelNum,
						 rRfChnlInfo.eSco,
						 rRfChnlInfo.ucChnlBw);

	return rRfChnlInfo;
}

void apsCheckConcurrent(struct ADAPTER *ad, struct BSS_DESC *bss,
	struct BSS_DESC *hint, uint8_t bidx)
{
	struct BSS_INFO *bssinfo;
	struct AIS_FSM_INFO *ais = aisGetAisFsmInfo(ad, bidx);
	uint32_t bmap = aisGetBssIndexBmap(aisGetAisFsmInfo(ad, bidx));
	struct CONNECTION_SETTINGS *conn = aisGetConnSettings(ad, bidx);
	uint8_t i;
#if CFG_SUPPORT_CCM
	struct RF_CHANNEL_INFO rExist, rCurr;

	rCurr = apsGetRfChannelInfo(ad, bss, NULL);
#endif

	/* SCC */
	for (i = 0; i < MAX_BSSID_NUM; i++) {
		/* Is connected BssInfo */
		if (BIT(i) & bmap)
			continue;

		bssinfo = GET_BSS_INFO_BY_INDEX(ad, i);
		if (!bssinfo || !IS_BSS_ALIVE(ad, bssinfo))
			continue;

		if (bss->eBand == bssinfo->eBand &&
		    bss->ucChannelNum == bssinfo->ucPrimaryChannel) {
			/* prefer higher band scc */
			if (hint && bss->eBand >= hint->eBand &&
			    !ad->rWifiVar.fgDisForceSCC &&
			    ais->ucReasonOfDisconnect !=
			    DISCONNECT_REASON_CODE_TEST_MODE)
				conn->eConnectionPolicy =
					CONNECT_BY_SSID_BEST_RSSI;
			bss->fgIsMCC = FALSE;
			return;
		}
	}

	/* MCC */
	for (i = 0; i < MAX_BSSID_NUM; i++) {
		/* Is connected BssInfo */
		if (BIT(i) & bmap)
			continue;

		bssinfo = GET_BSS_INFO_BY_INDEX(ad, i);
		if (!bssinfo || !IS_BSS_ALIVE(ad, bssinfo))
			continue;

		/* G band */
		if (bss->eBand == BAND_2G4 && bssinfo->eBand == BAND_2G4) {
			if (bss->ucChannelNum != bssinfo->ucPrimaryChannel) {
				bss->fgIsMCC = TRUE;
				return;
			}
		}

		/* A band */
		if (bss->eBand != BAND_2G4 && bssinfo->eBand != BAND_2G4) {
#if CFG_SUPPORT_CCM
			rExist = apsGetRfChannelInfo(ad, NULL, bssinfo);
			if (!ccmAAAvailableCheck(ad, &rCurr, &rExist)) {
				bss->fgIsMCC = TRUE;
				return;
			}
#else
			if (bss->eBand != bssinfo->eBand ||
			    bss->ucChannelNum != bssinfo->ucPrimaryChannel) {
				bss->fgIsMCC = TRUE;
				return;
			}
#endif
		}
	}

	bss->fgIsMCC = FALSE;
}

uint8_t apsGetCuInfo(struct ADAPTER *ad, struct BSS_DESC *bss, uint8_t bidx)
{
	struct APS_INFO *aps = aisGetApsInfo(ad, bidx);
	uint16_t u2CuOffset = 0;

	if (bss->eBand == BAND_2G4 || bss->eBand == BAND_5G)
		u2CuOffset = bss->ucChannelNum;
#if (CFG_SUPPORT_WIFI_6G == 1)
	else if (bss->eBand == BAND_6G)
		u2CuOffset = CU_6G_INDEX_OFFSET + bss->ucChannelNum;
#endif

	return aps->arCuInfo[u2CuOffset].u4TotalCount == 0 ? 0 :
	       aps->arCuInfo[u2CuOffset].u4TotalCu /
	       aps->arCuInfo[u2CuOffset].u4TotalCount;
}

static uint32_t apsGetEstimatedTput(struct ADAPTER *ad, struct BSS_DESC *bss,
	uint8_t bidx)
{
	struct APS_INFO *aps = aisGetApsInfo(ad, bidx);
	uint8_t fgIsGBandCoex = aps->fgIsGBandCoex, ucChannelCuInfo = 0;
	uint16_t amsduByte = apsGetAmsduByte(ad, bss, bidx);
	uint32_t baSize = 32, slot = 0, rcpi = 0, ppduDuration = 5;
	uint32_t airTime = 0, ideal = 0, tput = 0, est = 0;
	int32_t idle = 0, a = 0, b = 0, delta = 5;
	uint8_t *pucIEs = NULL;

	if (aps->ucConsiderEsp) {
		pucIEs = (uint8_t *) &bss->u4EspInfo[ESP_AC_BE];
		baSize = aucBaSizeTranslate[(uint8_t)((pucIEs[0] & 0xE0) >> 5)];
		airTime = pucIEs[1];
		if (bss->fgExistEspOutIE)
			airTime += bss->ucEspOutInfo[ESP_AC_BE];
		else
			airTime += pucIEs[1];
		airTime = airTime >> 1;
		if (pucIEs[2])
			ppduDuration = pucIEs[2];

		/* Unit: kbps */
		ideal = baSize * amsduByte * 8 / ppduDuration;
		rcpi = bss->ucRCPI;
		/* Consider the TxPwr only when the RCPI is sufficiently good */
		if (bss->fgExistTxPwr && bss->cTransmitPwr < 0 && rcpi > 100)
			rcpi = rcpi - (bss->cTransmitPwr > -10 ?
				bss->cTransmitPwr : -10);

		rcpi = rcpi > 220 ? 220 : rcpi;
		/* Adjust RCPI based on simultaneous equation */
		if (rcpi > 100) {
			/* RCPI from 220 to 100, peak(1) to breakpoint
			 * (1 - delta/100), y = ax + b, through 2 points
			 * (220, 1) (100, 1 - delta /100)
			 */
			a = (delta * 60000 / 100) / 120;
			b = 60000 - ((delta * 60000 / 100) * (220 / 120));
		} else if (rcpi >= 50) {
			/* RCPI from 100 to 50, breakpoint(1 - delta /100) to
			 * zero(0) y = ax + b, through 2 points
			 * (100, 1 - delta /100) (50, 0)
			 */
			a = (60000 - (delta * 60000 / 100)) / 50;
			b = (delta * 60000 / 100) - 60000;
		} else {
			/* RCPI less than 50, estimated tput will be zero */
			a = 0;
			b = 0;
		}
		tput = (uint32_t)((uint64_t)ideal *
			(uint64_t)(a * rcpi + b) / 60000);
		est = PERCENTAGE(airTime, 255) * tput / 100;

		if (bss->fgIsRWMValid && bss->u2ReducedWanMetrics < est)
			est = bss->u2ReducedWanMetrics;
	} else {
		if (bss->fgExistBssLoadIE) {
			airTime = 255 - bss->ucChnlUtilization;
		} else {
			ucChannelCuInfo = apsGetCuInfo(ad, bss, bidx);
			if (ucChannelCuInfo) {
				airTime = 255 - ucChannelCuInfo;
			} else {
				slot = scanGetChnlIdleSlot(ad,
					bss->eBand, bss->ucChannelNum);

				/* 90000 ms = 90ms dwell time to micro sec */
				idle = (slot * 9 * 100) / (90000);
				airTime  = kal_max_t(int32_t, idle, 50);

				/* nomalized to 0~255 */
				airTime = airTime * 255 / 100;
			}
		}

		tput = apsGetMaxRate(ad, bss, bidx); /* kbps */
		est = PERCENTAGE(airTime, 255) * tput / 100;
	}

#if (CFG_EXT_ROAMING == 1)
	if (fgIsGBandCoex && bss->eBand == BAND_2G4)
		est = (est * ad->rWifiVar.ucRBTCETPW / 100);
#else
	if (fgIsGBandCoex && bss->eBand == BAND_2G4)
		est = (est * WEIGHT_GBAND_COEX_DOWNGRADE / 100);
#endif

	if (bss->fgIsMCC)
		est = (est * WEIGHT_MCC_DOWNGRADE / 100);

	DBGLOG(APS, TRACE, "BSS["MACSTR
		"] EST:%d tput[%d] bw[%d] rssi[%d] CU[%d] airTime[%d] slot[%d] coex[%d] MCC[%d] TxPwr[%d] ideal[%d] ba[%d] amsdu[%d] a[%d] b[%d]\n",
		MAC2STR(bss->aucBSSID), est, tput,
		rlmGetBssOpBwByChannelWidth(bss->eSco, bss->eChannelWidth),
		RCPI_TO_dBm(bss->ucRCPI), ucChannelCuInfo, airTime, slot,
		fgIsGBandCoex, bss->fgIsMCC, bss->cTransmitPwr,
		ideal, baSize, amsduByte, a, b);

	return est;
}

uint16_t apsUpdateEssApList(struct ADAPTER *ad,
	enum ENUM_ROAMING_REASON reason, uint8_t bidx)
{
	struct APS_INFO *aps = aisGetApsInfo(ad, bidx);
	struct AP_COLLECTION *ap;
	struct BSS_DESC *bss = NULL, *hint = NULL;
	struct LINK *scan_result = &ad->rWifiVar.rScanInfo.rBSSDescList;
	struct CONNECTION_SETTINGS *conn = aisGetConnSettings(ad, bidx);
	uint16_t count = 0;

	kalMemZero(aps->arCuInfo, sizeof(aps->arCuInfo));
	aps->ucConsiderEsp = TRUE;

	if (conn->eConnectionPolicy == CONNECT_BY_BSSID_HINT) {
		struct PARAM_SSID ssid = {0};

		COPY_SSID(ssid.aucSsid, ssid.u4SsidLen,
			  conn->aucSSID, conn->ucSSIDLen);
		hint = scanSearchBssDescByBssidAndSsid(ad,
			conn->aucBSSIDHint, TRUE, &ssid);
	}

#if (CFG_EXT_ROAMING == 1)
	if (reason == ROAMING_REASON_BT_COEX)
		aps->fgIsGBandCoex = TRUE;
#endif /* CFG_EXT_ROAMING == 1 */

	LINK_FOR_EACH_ENTRY(bss, scan_result, rLinkEntry,
		struct BSS_DESC) {
		if (bss->ucChannelNum > 233)
			continue;
		if (!EQUAL_SSID(conn->aucSSID,
			conn->ucSSIDLen,
			bss->aucSSID, bss->ucSSIDLen) ||
			bss->eBSSType != BSS_TYPE_INFRASTRUCTURE)
			continue;

		bss->prBlock = aisQueryBlockList(ad, bss);
#if CFG_SUPPORT_802_11K
		/* update neighbor report entry */
		bss->prNeighbor = aisGetNeighborAPEntry(
			ad, bss, bidx);
#endif

		/* sanity check */
		if (!apsSanityCheckBssDesc(ad, bss, reason, bidx))
			continue;

		/* add bss to ap collection */
		ap = apsGetAp(ad, bss, bidx);
		if (ap) {
			if (!apsAddBssDescToList(ad, ap, bss, bidx))
				count++;
		} else {
			ap = apsAddAp(ad, bss, bidx);
			if (ap)
				count++;
		}

		/* handle cu/esp */
		if (bss->fgExistBssLoadIE)
			apsRecordCuInfo(ad, bss, bidx);
		if (!bss->fgExistEspIE)
			aps->ucConsiderEsp = FALSE;

		apsCheckConcurrent(ad, bss, hint, bidx);
	}

	DBGLOG(APS, INFO,
		"Find %s in %d BSSes, result %d, Using %s estimated tput, policy %d\n",
		conn->aucSSID, scan_result->u4NumElem, count,
		aps->ucConsiderEsp ? "ESP" : "LEGACY",
		conn->eConnectionPolicy);
	return count;
}

static enum ROAM_TYPE roamReasonToType(enum ENUM_ROAMING_REASON type)
{
	enum ROAM_TYPE ret = ROAM_TYPE_RCPI;

	if (type >= ROAMING_REASON_NUM)
		return ret;
#if CFG_SUPPORT_ROAMING
	if (type == ROAMING_REASON_TX_ERR)
		ret = ROAM_TYPE_PER;
#endif
	return ret;
}

#if (CFG_EXT_ROAMING == 0) /* Common part */
/* Channel Utilization: weight index will be */
static uint16_t apsCalculateScoreByChnlInfo(
	struct AIS_SPECIFIC_BSS_INFO *prAisSpecificBssInfo, uint8_t ucChannel,
	enum ROAM_TYPE eRoamType)
{
	struct ESS_CHNL_INFO *prEssChnlInfo = &prAisSpecificBssInfo->
		arCurEssChnlInfo[0];
	uint8_t i = 0;
	uint16_t u2Score = 0;
	uint8_t weight = 0;

	if (eRoamType >= ROAM_TYPE_NUM) {
		DBGLOG(APS, WARN, "Invalid roam type %d!\n", eRoamType);
		return 0;
	}

	weight = gasMtkWeightConfig[eRoamType].ucApNumWeight;

	for (; i < prAisSpecificBssInfo->ucCurEssChnlInfoNum; i++) {
		if (ucChannel == prEssChnlInfo[i].ucChannel) {
#if 0	/* currently, we don't take channel utilization into account */
			/* the channel utilization max value is 255.
			 *great utilization means little weight value.
			 * the step of weight value is 2.6
			 */
			u2Score = mtk_weight_config[eRoamType].
				ucChnlUtilWeight * (BSS_FULL_SCORE -
				(prEssChnlInfo[i].ucUtilization * 10 / 26));
#endif
			/* if AP num on this channel is greater than 100,
			 * the weight will be 0.
			 * otherwise, the weight value decrease 1
			 * if AP number increase 1
			 */
			if (prEssChnlInfo[i].ucApNum <= CHNL_BSS_NUM_THRESOLD)
				u2Score += weight *
				(BSS_FULL_SCORE - prEssChnlInfo[i].ucApNum *
					SCORE_PER_AP);
			DBGLOG(APS, TRACE, "channel %d, AP num %d\n",
				ucChannel, prEssChnlInfo[i].ucApNum);
			break;
		}
	}
	return u2Score;
}

static uint16_t apsCalculateScoreByBW(struct ADAPTER *prAdapter,
	struct BSS_DESC *prBssDesc, enum ROAM_TYPE eRoamType,
	uint8_t ucBssIndex)
{
	uint16_t u2Score = 0;

	if (eRoamType >= ROAM_TYPE_NUM) {
		DBGLOG(APS, WARN, "Invalid roam type %d!\n", eRoamType);
		return 0;
	}

	switch (prBssDesc->eChannelWidth) {
	case CW_20_40MHZ:
		u2Score = 40;
		break;
	case CW_80MHZ:
		u2Score = 60;
		break;
	case CW_160MHZ:
	case CW_80P80MHZ:
		u2Score = 80;
		break;
	case CW_320_1MHZ:
	case CW_320_2MHZ:
		u2Score = 100;
		break;
	default:
		break;
	}

	return u2Score * gasMtkWeightConfig[eRoamType].ucBandWidthWeight;
}

static uint16_t apsCalculateScoreByBand(struct ADAPTER *prAdapter,
	struct BSS_DESC *prBssDesc, int8_t cRssi, enum ROAM_TYPE eRoamType)
{
	uint16_t u2Score = 0;

	if (eRoamType >= ROAM_TYPE_NUM) {
		DBGLOG(APS, WARN, "Invalid roam type %d!\n", eRoamType);
		return 0;
	}

	switch (prBssDesc->eBand) {
	case BAND_2G4:
		u2Score = 0;
		break;
	case BAND_5G:
		if (prAdapter->fgEnable5GBand && cRssi > LOW_RSSI_FOR_5G_BAND)
			u2Score = 80;
		break;
#if (CFG_SUPPORT_WIFI_6G == 1)
	case BAND_6G:
		if (prAdapter->fgIsHwSupport6G && cRssi > LOW_RSSI_FOR_5G_BAND)
			u2Score = BSS_FULL_SCORE;
		break;
#endif
	default:
		break;
	}

	return u2Score * gasMtkWeightConfig[eRoamType].ucBandWeight;
}

static uint16_t apsCalculateScoreByClientCnt(struct BSS_DESC *prBssDesc,
			enum ROAM_TYPE eRoamType)
{
	uint16_t u2Score = 0;
	uint16_t u2StaCnt = 0;
#define BSS_STA_CNT_NORMAL_SCORE 50
#define BSS_STA_CNT_GOOD_THRESOLD 10

	if (eRoamType >= ROAM_TYPE_NUM) {
		DBGLOG(APS, WARN, "Invalid roam type %d!\n", eRoamType);
		return 0;
	}

	DBGLOG(APS, TRACE, "Exist bss load %d, sta cnt %d\n",
			prBssDesc->fgExistBssLoadIE, prBssDesc->u2StaCnt);

	if (!prBssDesc->fgExistBssLoadIE) {
		u2Score = BSS_STA_CNT_NORMAL_SCORE;
		return u2Score *
		gasMtkWeightConfig[eRoamType].ucClientCntWeight;
	}

	u2StaCnt = prBssDesc->u2StaCnt;
	if (u2StaCnt > BSS_STA_CNT_THRESOLD)
		u2Score = 0;
	else if (u2StaCnt < BSS_STA_CNT_GOOD_THRESOLD)
		u2Score = BSS_FULL_SCORE - u2StaCnt;
	else
		u2Score = BSS_STA_CNT_NORMAL_SCORE;

	return u2Score * gasMtkWeightConfig[eRoamType].ucClientCntWeight;
}

static uint16_t apsCalculateScoreByStbc(struct BSS_DESC *prBssDesc,
	enum ROAM_TYPE eRoamType)
{
	uint16_t u2Score = 0;

	if (eRoamType >= ROAM_TYPE_NUM) {
		DBGLOG(APS, WARN, "Invalid roam type %d!\n", eRoamType);
		return 0;
	}

	if (prBssDesc->fgMultiAnttenaAndSTBC)
		u2Score = BSS_FULL_SCORE;

#if (CFG_SUPPORT_WIFI_6G == 1)
	/* assume stbc is supported because 6g AP doesn't carry ht cap */
	if (prBssDesc->eBand == BAND_6G)
		u2Score = BSS_FULL_SCORE;
#endif

	u2Score *= gasMtkWeightConfig[eRoamType].ucStbcWeight;

	return u2Score;
}

static uint16_t apsCalculateScoreByRssi(struct BSS_DESC *prBssDesc,
	enum ROAM_TYPE eRoamType)
{
	uint16_t u2Score = 0;
	int8_t cRssi = RCPI_TO_dBm(prBssDesc->ucRCPI);

	if (eRoamType >= ROAM_TYPE_NUM) {
		DBGLOG(APS, WARN, "Invalid roam type %d!\n", eRoamType);
		return 0;
	}

	if (cRssi >= BEST_RSSI)
		u2Score = 100;
	else if (prBssDesc->eBand == BAND_5G && cRssi >= GOOD_RSSI_FOR_HT_VHT)
		u2Score = 100;
#if (CFG_SUPPORT_WIFI_6G == 1)
	else if (prBssDesc->eBand == BAND_6G && cRssi >= GOOD_RSSI_FOR_HT_VHT)
		u2Score = 100;
#endif
	else if (prBssDesc->eBand == BAND_2G4 && cRssi < MINIMUM_RSSI_2G4)
		u2Score = 0;
	else if (prBssDesc->eBand == BAND_5G && cRssi < MINIMUM_RSSI_5G)
		u2Score = 0;
#if (CFG_SUPPORT_WIFI_6G == 1)
	else if (prBssDesc->eBand == BAND_6G && cRssi < MINIMUM_RSSI_6G)
		u2Score = 0;
#endif
	else if (cRssi <= -98)
		u2Score = 0;
	else
		u2Score = (cRssi + 98) * 2;

	u2Score *= gasMtkWeightConfig[eRoamType].ucRssiWeight;

	return u2Score;
}

static uint16_t apsCalculateScoreBySaa(struct ADAPTER *prAdapter,
	struct BSS_DESC *prBssDesc, enum ROAM_TYPE eRoamType)
{
	uint16_t u2Score = 0;
	struct STA_RECORD *prStaRec = (struct STA_RECORD *) NULL;

	if (eRoamType >= ROAM_TYPE_NUM) {
		DBGLOG(APS, WARN, "Invalid roam type %d!\n", eRoamType);
		return 0;
	}

	prStaRec = cnmGetStaRecByAddress(prAdapter, NETWORK_TYPE_AIS,
		prBssDesc->aucSrcAddr);
	if (prStaRec)
		u2Score = gasMtkWeightConfig[eRoamType].ucSaaWeight *
		(prStaRec->ucTxAuthAssocRetryCount ? 0 : BSS_FULL_SCORE);
	else
		u2Score = gasMtkWeightConfig[eRoamType].ucSaaWeight *
		BSS_FULL_SCORE;

	return u2Score;
}

static uint16_t apsCalculateScoreByIdleTime(struct ADAPTER *prAdapter,
	uint8_t ucChannel, enum ROAM_TYPE eRoamType,
	struct BSS_DESC *prBssDesc, uint8_t ucBssIndex,
	enum ENUM_BAND eBand)
{
	struct SCAN_INFO *info;
	struct SCAN_PARAM *param;
	struct BSS_INFO *bss;
	int32_t score, rssi, cu = 0, cuRatio, dwell;
	uint32_t rssiFactor, cuFactor, rssiWeight, cuWeight;
	uint32_t slot = 0, idle;
	uint8_t i;

	rssi = RCPI_TO_dBm(prBssDesc->ucRCPI);
	rssiWeight = 65;
	cuWeight = 35;
	if (rssi >= -55)
		rssiFactor = 100;
	else if (rssi < -55 && rssi >= -60)
		rssiFactor = 90 + 2 * (60 + rssi);
	else if (rssi < -60 && rssi >= -70)
		rssiFactor = 60 + 3 * (70 + rssi);
	else if (rssi < -70 && rssi >= -80)
		rssiFactor = 20 + 4 * (80 + rssi);
	else if (rssi < -80 && rssi >= -90)
		rssiFactor = 2 * (90 + rssi);
	else
		rssiFactor = 0;
	if (eRoamType >= ROAM_TYPE_NUM) {
		DBGLOG(APS, WARN, "Invalid roam type %d!\n", eRoamType);
		return 0;
	}
	if (prBssDesc->eBand >= BAND_NUM) {
		DBGLOG(APS, WARN, "Invalid Band %d\n", prBssDesc->eBand);
		return 0;
	}
	if (prBssDesc->fgExistBssLoadIE) {
		cu = prBssDesc->ucChnlUtilization;
	} else {
		bss = aisGetAisBssInfo(prAdapter, ucBssIndex);
		info = &(prAdapter->rWifiVar.rScanInfo);
		param = &(info->rScanParam);

		if (param->u2ChannelDwellTime > 0)
			dwell = param->u2ChannelDwellTime;
		else if (bss->eConnectionState == MEDIA_STATE_CONNECTED)
			dwell = CHNL_DWELL_TIME_ONLINE;
		else
			dwell = CHNL_DWELL_TIME_DEFAULT;

		for (i = 0; i < info->ucSparseChannelArrayValidNum; i++) {
			if (prBssDesc->ucChannelNum == info->aucChannelNum[i] &&
					eBand == info->aeChannelBand[i]) {
				slot = info->au2ChannelIdleTime[i];
				idle = (slot * 9 * 100) / (dwell * 1000);
#if CFG_SUPPORT_ROAMING
				if (eRoamType == ROAM_TYPE_PER) {
					score = idle > BSS_FULL_SCORE ?
						BSS_FULL_SCORE : idle;
					goto done;
				}
#endif
				cu = 255 - idle * 255 / 100;
				break;
			}
		}
	}

	cuRatio = cu * 100 / 255;
	if (prBssDesc->eBand == BAND_2G4) {
		if (cuRatio < 10)
			cuFactor = 100;
		else if (cuRatio < 70 && cuRatio >= 10)
			cuFactor = 111 - (13 * cuRatio / 10);
		else
			cuFactor = 20;
	} else {
		if (cuRatio < 30)
			cuFactor = 100;
		else if (cuRatio < 80 && cuRatio >= 30)
			cuFactor = 148 - (16 * cuRatio / 10);
		else
			cuFactor = 20;
	}

	score = (rssiFactor * rssiWeight + cuFactor * cuWeight) >> 6;

	DBGLOG(APS, TRACE,
		MACSTR
		" Band[%s],chl[%d],slt[%d],ld[%d] idle Score %d,rssi[%d],cu[%d],cuR[%d],rf[%d],rw[%d],cf[%d],cw[%d]\n",
		MAC2STR(prBssDesc->aucBSSID),
		apucBandStr[prBssDesc->eBand],
		prBssDesc->ucChannelNum, slot,
		prBssDesc->fgExistBssLoadIE, score, rssi, cu, cuRatio,
		rssiFactor, rssiWeight, cuFactor, cuWeight);
#if CFG_SUPPORT_ROAMING
done:
#endif
	return score * gasMtkWeightConfig[eRoamType].ucChnlIdleWeight;

}

uint16_t apsCalculateScoreByBlockList(struct ADAPTER *prAdapter,
	    struct BSS_DESC *prBssDesc, enum ROAM_TYPE eRoamType)
{
	uint16_t u2Score = 0;

	if (eRoamType >= ROAM_TYPE_NUM) {
		DBGLOG(APS, WARN, "Invalid roam type %d!\n", eRoamType);
		return 0;
	}

	if (!prBssDesc->prBlock)
		u2Score = 100;
	else if (rsnApOverload(prBssDesc->prBlock->u2AuthStatus,
		prBssDesc->prBlock->u2DeauthReason) ||
		 prBssDesc->prBlock->ucCount >= 10)
		u2Score = 0;
	else
		u2Score = 100 - prBssDesc->prBlock->ucCount * 10;

	return u2Score * gasMtkWeightConfig[eRoamType].ucBlockListWeight;
}

uint16_t apsCalculateScoreByTput(struct ADAPTER *prAdapter,
	    struct BSS_DESC *prBssDesc, enum ROAM_TYPE eRoamType)
{
	uint16_t u2Score = 0;

	if (eRoamType >= ROAM_TYPE_NUM) {
		DBGLOG(APS, WARN, "Invalid roam type %d!\n", eRoamType);
		return 0;
	}

#if CFG_SUPPORT_MBO
	if (prBssDesc->fgExistEspIE)
		u2Score = (prBssDesc->u4EspInfo[ESP_AC_BE] >> 8) & 0xff;
#endif

	return u2Score * gasMtkWeightConfig[eRoamType].ucTputWeight;
}

uint16_t apsCalculateScoreByPreference(struct ADAPTER *prAdapter,
	    struct BSS_DESC *prBssDesc, enum ENUM_ROAMING_REASON eRoamReason)
{
	enum ROAM_TYPE eRoamType = roamReasonToType(eRoamReason);

	if (eRoamType >= ROAM_TYPE_NUM) {
		DBGLOG(APS, WARN, "Invalid roam type %d!\n", eRoamType);
		return 0;
	}

#if CFG_SUPPORT_ROAMING
#if CFG_SUPPORT_802_11K
	if (prBssDesc->prNeighbor)
		return (prBssDesc->prNeighbor->ucPreference + 100) *
		       gasMtkWeightConfig[eRoamType].ucPreferenceWeight;
#endif
#endif
	return 100 * gasMtkWeightConfig[eRoamType].ucPreferenceWeight;
}

uint16_t apsCalculateApScore(struct ADAPTER *prAdapter,
	struct BSS_DESC *prBssDesc, enum ENUM_ROAMING_REASON eRoamReason,
	uint8_t ucBssIndex)
{
	struct AIS_SPECIFIC_BSS_INFO *prAisSpecificBssInfo = NULL;
	struct APS_INFO *aps = aisGetApsInfo(prAdapter, ucBssIndex);
	uint8_t fgIsGBandCoex = aps->fgIsGBandCoex;
	uint16_t u2ScoreStaCnt = 0;
	uint16_t u2ScoreBandwidth = 0;
	uint16_t u2ScoreSTBC = 0;
	uint16_t u2ScoreChnlInfo = 0;
	uint16_t u2ScoreSnrRssi = 0;
	uint16_t u2ScoreDeauth = 0;
	uint16_t u2ScoreBand = 0;
	uint16_t u2ScoreSaa = 0;
	uint16_t u2ScoreIdleTime = 0;
	uint16_t u2ScoreTotal = 0;
	uint16_t u2BlockListScore = 0;
	uint16_t u2PreferenceScore = 0;
	uint16_t u2TputScore = 0;
#if (CFG_SUPPORT_AVOID_DESENSE == 1)
	uint8_t fgBssInDenseRange =
		IS_CHANNEL_IN_DESENSE_RANGE(prAdapter,
		prBssDesc->ucChannelNum,
		prBssDesc->eBand);
	char extra[16] = {0};
#else
	char *extra = "";
#endif
	int8_t cRssi = -128;
	enum ROAM_TYPE eRoamType = roamReasonToType(eRoamReason);

	prAisSpecificBssInfo = aisGetAisSpecBssInfo(prAdapter, ucBssIndex);
	cRssi = RCPI_TO_dBm(prBssDesc->ucRCPI);

	if (eRoamType >= ROAM_TYPE_NUM) {
		DBGLOG(APS, WARN, "Invalid roam type %d!\n", eRoamType);
		return 0;
	}
	if (prBssDesc->eBand >= BAND_NUM) {
		DBGLOG(APS, WARN, "Invalid Band %d\n", prBssDesc->eBand);
		return 0;
	}
	u2ScoreBandwidth = apsCalculateScoreByBW(prAdapter,
		prBssDesc, eRoamType, ucBssIndex);
	u2ScoreStaCnt = apsCalculateScoreByClientCnt(prBssDesc, eRoamType);
	u2ScoreSTBC = apsCalculateScoreByStbc(prBssDesc, eRoamType);
	u2ScoreChnlInfo = apsCalculateScoreByChnlInfo(prAisSpecificBssInfo,
				prBssDesc->ucChannelNum, eRoamType);
	u2ScoreSnrRssi = apsCalculateScoreByRssi(prBssDesc, eRoamType);
	u2ScoreDeauth = CALCULATE_SCORE_BY_DEAUTH(prBssDesc, eRoamType);
	u2ScoreBand = apsCalculateScoreByBand(prAdapter, prBssDesc,
		cRssi, eRoamType);
	u2ScoreSaa = apsCalculateScoreBySaa(prAdapter, prBssDesc, eRoamType);
	u2ScoreIdleTime = apsCalculateScoreByIdleTime(prAdapter,
		prBssDesc->ucChannelNum, eRoamType, prBssDesc, ucBssIndex,
		prBssDesc->eBand);
	u2BlockListScore =
	       apsCalculateScoreByBlockList(prAdapter, prBssDesc, eRoamType);
	u2PreferenceScore =
	      apsCalculateScoreByPreference(prAdapter, prBssDesc, eRoamReason);

	u2TputScore = apsCalculateScoreByTput(prAdapter, prBssDesc, eRoamType);

	u2ScoreTotal = u2ScoreBandwidth + u2ScoreChnlInfo +
		u2ScoreDeauth + u2ScoreSnrRssi + u2ScoreStaCnt + u2ScoreSTBC +
		u2ScoreBand + u2BlockListScore + u2ScoreSaa +
		u2ScoreIdleTime + u2TputScore;

	/* Adjust 2.4G AP's score if BT coex */
	if (prBssDesc->eBand == BAND_2G4 && fgIsGBandCoex)
		u2ScoreTotal = u2ScoreTotal * WEIGHT_GBAND_COEX_DOWNGRADE / 100;

	if (prBssDesc->fgIsMCC)
		u2ScoreTotal = (u2ScoreTotal * WEIGHT_MCC_DOWNGRADE / 100);

#if (CFG_SUPPORT_AVOID_DESENSE == 1)
	if (fgBssInDenseRange)
		u2ScoreTotal /= 4;
	kalSnprintf(extra, sizeof(extra), ", DESENSE[%d]", fgBssInDenseRange);
#endif

#define TEMP_LOG_TEMPLATE\
		"BSS["MACSTR"] Score:%d Band[%s],cRSSI[%d],GBandCoex[%d]"\
		",MCC[%d],DE[%d],RSSI[%d],BD[%d],BL[%d],SAA[%d]"\
		",BW[%d],SC[%d],ST[%d],CI[%d],IT[%d],CU[%d,%d],PF[%d]"\
		",TPUT[%d]%s\n"

	DBGLOG(APS, TRACE,
		TEMP_LOG_TEMPLATE,
		MAC2STR(prBssDesc->aucBSSID),
		u2ScoreTotal, apucBandStr[prBssDesc->eBand],
		cRssi, fgIsGBandCoex, prBssDesc->fgIsMCC, u2ScoreDeauth,
		u2ScoreSnrRssi, u2ScoreBand, u2BlockListScore,
		u2ScoreSaa, u2ScoreBandwidth, u2ScoreStaCnt,
		u2ScoreSTBC, u2ScoreChnlInfo, u2ScoreIdleTime,
		prBssDesc->fgExistBssLoadIE,
		prBssDesc->ucChnlUtilization,
		u2PreferenceScore,
		u2TputScore, extra);

#undef TEMP_LOG_TEMPLATE

	return u2ScoreTotal;
}
#endif /* CFG_EXT_ROAMING  */

uint8_t apsSanityCheckBssDesc(struct ADAPTER *prAdapter,
	struct BSS_DESC *prBssDesc, enum ENUM_ROAMING_REASON eRoamReason,
	uint8_t ucBssIndex)
{
	struct AIS_FSM_INFO *ais;
	struct BSS_INFO *prAisBssInfo;
	struct CONNECTION_SETTINGS *conn;
	struct APS_INFO *prApsInfo;
#if CFG_SUPPORT_MBO
	struct PARAM_BSS_DISALLOWED_LIST *disallow;
	struct BSS_TRANSITION_MGT_PARAM *prBtmParam;
	uint32_t i;
#endif

	ais = aisGetAisFsmInfo(prAdapter, ucBssIndex);
	prAisBssInfo = aisGetAisBssInfo(prAdapter, ucBssIndex);
	conn = aisGetConnSettings(prAdapter, ucBssIndex);
	prApsInfo = aisGetApsInfo(prAdapter, ucBssIndex);

	if (ais == NULL) {
		DBGLOG(APS, WARN, "ais is NULL\n");
		return FALSE;
	}

	if (aisQueryCusBlocklist(prAdapter, ucBssIndex, prBssDesc))
		return FALSE;

	if (!prBssDesc->fgIsInUse) {
		DBGLOG(APS, WARN, MACSTR" is not in use\n",
			MAC2STR(prBssDesc->aucBSSID));
		return FALSE;
	}

	if ((prBssDesc->eBand == BAND_2G4 &&
		prAdapter->rWifiVar.ucDisallowBand2G) ||
	    (prBssDesc->eBand == BAND_5G &&
		prAdapter->rWifiVar.ucDisallowBand5G)
#if (CFG_SUPPORT_WIFI_6G == 1)
	 || (prBssDesc->eBand == BAND_6G &&
		prAdapter->rWifiVar.ucDisallowBand6G)
#endif
	) {
		DBGLOG(APS, WARN, MACSTR" Band[%s] is not allowed\n",
			MAC2STR(prBssDesc->aucBSSID),
			apucBandStr[prBssDesc->eBand]);
		return FALSE;
	}

#if (CFG_SUPPORT_802_11BE_MLO == 1)
	if (ais->ucMlProbeEnable &&
	    (!prBssDesc->rMlInfo.fgValid ||
	     UNEQUAL_MAC_ADDR(prBssDesc->rMlInfo.aucMldAddr,
			      ais->prMlProbeBssDesc->rMlInfo.aucMldAddr))) {
		DBGLOG(APS, WARN, MACSTR" is not the target of MLO scan.\n",

			MAC2STR(prBssDesc->aucBSSID));
		return FALSE;
	}

	if (prBssDesc->rMlInfo.fgValid &&
		!(BIT(prBssDesc->rMlInfo.ucLinkIndex) & conn->u2LinkIdBitmap)) {
		DBGLOG(APS, WARN, MACSTR" LinkID[%d] is not allowed [%d]\n",
			MAC2STR(prBssDesc->aucBSSID),
			prBssDesc->rMlInfo.ucLinkIndex,
			conn->u2LinkIdBitmap);
		return FALSE;
	}

	if (mldIsMultiLinkEnabled(prAdapter, NETWORK_TYPE_AIS, ucBssIndex) &&
	    prBssDesc->rMlInfo.u2ApRemovalTimer) {
		DBGLOG(APS, WARN, MACSTR " is being removed.\n",
			MAC2STR(prBssDesc->aucBSSID));
		return FALSE;
	}
#endif

	if (prBssDesc->eBSSType != BSS_TYPE_INFRASTRUCTURE) {
		DBGLOG(APS, WARN, MACSTR" is not infrastructure\n",
			MAC2STR(prBssDesc->aucBSSID));
		return FALSE;
	}

	if (prBssDesc->prBlock) {
		if (prBssDesc->prBlock->fgIsInFWKBlocklist) {
			DBGLOG(APS, WARN, MACSTR" in FWK blocklist\n",
				MAC2STR(prBssDesc->aucBSSID));
			return FALSE;
		}

		if (prBssDesc->prBlock->fgDeauthLastTime
#if (CFG_EXT_ROAMING == 0)
		    && (prApsInfo->u4EssApNum <= 1 ||
			prBssDesc->prBlock->ucDeauthCount >= 2 ||
			prAdapter->rWifiVar.u4SwTestMode !=
						ENUM_SW_TEST_MODE_NONE)
#endif
		    ) {
			DBGLOG(APS, WARN, MACSTR " is sending deauth [%d].\n",
				MAC2STR(prBssDesc->aucBSSID),
				prBssDesc->prBlock->ucDeauthCount);
			return FALSE;
		}

		if (prBssDesc->prBlock->ucCount >=
		    prAdapter->rWifiVar.ucAisBssTrialLimit) {
			DBGLOG(APS, WARN,
				MACSTR
				" Skip AP that add to blocklist count %d >= %d\n",
				MAC2STR(prBssDesc->aucBSSID),
				prBssDesc->prBlock->ucCount,
				prAdapter->rWifiVar.ucAisBssTrialLimit);
			return FALSE;
		}
	}

	/* Restrict STAs other than wlan0 */
	if (ais->ucAisIndex != AIS_DEFAULT_INDEX) {
		struct AIS_FSM_INFO *tempAis;
		struct BSS_DESC *tempBssDesc;
		uint8_t i, j;
		uint8_t ucExistALinks = 0, ucExistGLinks = 0;
		struct RF_CHANNEL_INFO rCurrRfChnlInfo, rExistRfChnlInfo;

		/* Disallow to pick a bss that is connecting */
		if (prBssDesc->fgIsConnecting) {
			DBGLOG(APS, INFO,
				MACSTR " is connecting by other WLAN",
				MAC2STR(prBssDesc->aucBSSID));
			return FALSE;
		}

		/* Disallow to pick a bss that already connected */
		if (IS_AIS_CONN_BSSDESC(ais, prBssDesc)) {
			DBGLOG(APS, INFO,
				MACSTR " already connected by wlan0",
				MAC2STR(prBssDesc->aucBSSID));
			return FALSE;
		}

		for (i = 0; i < KAL_AIS_NUM; i++) {
			if (i == ais->ucAisIndex)
				continue;

			tempAis = aisFsmGetInstance(prAdapter, i);
			for (j = 0; j < MLD_LINK_MAX; j++) {
				tempBssDesc = aisGetLinkBssDesc(tempAis, j);
				if (tempBssDesc) {
					if (tempBssDesc->eBand == BAND_2G4) {
						ucExistGLinks++;
					} else {
						ucExistALinks++;
						rExistRfChnlInfo =
							apsGetRfChannelInfo(
								prAdapter,
								tempBssDesc,
								NULL);
					}
				}
			}
		}

		rCurrRfChnlInfo =
			apsGetRfChannelInfo(prAdapter, prBssDesc, NULL);

		/* If band not fully used */
		if (ucExistGLinks < 1 || ucExistALinks < ENUM_BAND_NUM - 1) {
			/* G band used, skip 2.4G */
			if (ucExistGLinks >= 1 &&
			    prBssDesc->eBand == BAND_2G4) {
				DBGLOG(APS, INFO,
					MACSTR " can't use G band",
					MAC2STR(prBssDesc->aucBSSID));
				return FALSE;
			}

			/* A band is fully used, only allow 2.4G */
			if (ucExistALinks >= ENUM_BAND_NUM - 1 &&
			    prBssDesc->eBand != BAND_2G4) {
				DBGLOG(APS, INFO,
					MACSTR " can't use A band",
					MAC2STR(prBssDesc->aucBSSID));
				return FALSE;
			}

#if CFG_SUPPORT_CCM
			/* A+A */
			if (prBssDesc->eBand != BAND_2G4 &&
			    ucExistALinks == 1 &&
			    !ccmAAAvailableCheck(prAdapter,
						 &rCurrRfChnlInfo,
						 &rExistRfChnlInfo)) {
				DBGLOG(APS, INFO, MACSTR
					" Band[%d] Chnl[%d] ChnlBw[%d] Freq[%d] can't form A+A DBDC with Band[%d] Chnl[%d] ChnlBw[%d] Freq[%d]",
					MAC2STR(prBssDesc->aucBSSID),
					rCurrRfChnlInfo.eBand,
					rCurrRfChnlInfo.ucChannelNum,
					rCurrRfChnlInfo.ucChnlBw,
					rCurrRfChnlInfo.u4CenterFreq1,
					rExistRfChnlInfo.eBand,
					rExistRfChnlInfo.ucChannelNum,
					rExistRfChnlInfo.ucChnlBw,
					rExistRfChnlInfo.u4CenterFreq1);
				return FALSE;
			}
#endif
		}
	}

#if CFG_SUPPORT_NCHO
	if (prAdapter->rNchoInfo.fgNCHOEnabled) {
		if (!(BIT(prBssDesc->eBand) &
			prAdapter->rNchoInfo.ucRoamBand)) {
			DBGLOG(APS, WARN,
				MACSTR" band(%s) is not in NCHO roam band\n",
				MAC2STR(prBssDesc->aucBSSID),
				apucBandStr[prBssDesc->eBand]);
			return FALSE;
		}
	}
#endif

	if (!(prBssDesc->ucPhyTypeSet &
		(prAdapter->rWifiVar.ucAvailablePhyTypeSet))) {
		DBGLOG(APS, WARN,
			MACSTR" ignore unsupported ucPhyTypeSet = %x\n",
			MAC2STR(prBssDesc->aucBSSID),
			prBssDesc->ucPhyTypeSet);
		return FALSE;
	}

	if (prBssDesc->fgIsUnknownBssBasicRate) {
		DBGLOG(APS, WARN, MACSTR" unknown bss basic rate\n",
			MAC2STR(prBssDesc->aucBSSID));
		return FALSE;
	}

	if (!rlmDomainIsLegalChannel(prAdapter, prBssDesc->eBand,
		prBssDesc->ucChannelNum)) {
		DBGLOG(APS, WARN, MACSTR" band %d channel %d is not legal\n",
			MAC2STR(prBssDesc->aucBSSID), prBssDesc->eBand,
			prBssDesc->ucChannelNum);
		return FALSE;
	}

	if (CHECK_FOR_TIMEOUT(kalGetTimeTick(), prBssDesc->rUpdateTime,
		SEC_TO_SYSTIME(wlanWfdEnabled(prAdapter) ?
		SCN_BSS_DESC_STALE_SEC_WFD : SCN_BSS_DESC_STALE_SEC))) {
		DBGLOG(APS, WARN, MACSTR " description is too old.\n",
			MAC2STR(prBssDesc->aucBSSID));
		return FALSE;
	}

	/* BTO case */
	if (prBssDesc->fgIsInBTO) {
		log_dbg(APS, WARN, MACSTR " is in BTO.\n",
			MAC2STR(prBssDesc->aucBSSID));
		return FALSE;
	}

	if (!rsnPerformPolicySelection(prAdapter, prBssDesc,
		ucBssIndex)) {
		DBGLOG(APS, WARN, MACSTR " rsn policy select fail.\n",
			MAC2STR(prBssDesc->aucBSSID));
		return FALSE;
	}

	if (aisGetAisSpecBssInfo(prAdapter,
		ucBssIndex)->fgCounterMeasure) {
		DBGLOG(APS, WARN, MACSTR " Skip in counter measure period.\n",
			MAC2STR(prBssDesc->aucBSSID));
		return FALSE;
	}

#if CFG_SUPPORT_MBO
	prBtmParam = aisGetBTMParam(prAdapter, ucBssIndex);
	disallow = &prAdapter->rWifiVar.rBssDisallowedList;
	for (i = 0; i < disallow->u4NumBssDisallowed; ++i) {
		uint32_t index = i * MAC_ADDR_LEN;

		if (EQUAL_MAC_ADDR(prBssDesc->aucBSSID,
				&disallow->aucList[index])) {
			DBGLOG(APS, WARN, MACSTR" disallowed list\n",
				MAC2STR(prBssDesc->aucBSSID));
			return FALSE;
		}
	}

	if (prBssDesc->fgIsDisallowed) {
		DBGLOG(APS, WARN, MACSTR" disallowed\n",
			MAC2STR(prBssDesc->aucBSSID));
		return FALSE;
	}

	if (prBssDesc->prBlock && prBssDesc->prBlock->fgDisallowed &&
	    (eRoamReason != ROAMING_REASON_BTM ||
	     prBtmParam->ucDisImmiState == AIS_BTM_DIS_IMMI_STATE_3)) {
		DBGLOG(APS, WARN, MACSTR" disallowed delay, rssi %d(%d)\n",
			MAC2STR(prBssDesc->aucBSSID),
			RCPI_TO_dBm(prBssDesc->ucRCPI),
			prBssDesc->prBlock->i4RssiThreshold);
		return FALSE;
	}

	if (eRoamReason == ROAMING_REASON_BTM) {
		if (aisCheckNeighborApValidity(prAdapter, ucBssIndex)) {
			uint8_t ucRequestMode = prBtmParam->ucRequestMode;

			if (prBssDesc->prNeighbor &&
			    prBssDesc->prNeighbor->fgPrefPresence &&
			    !prBssDesc->prNeighbor->ucPreference) {
				DBGLOG(APS, WARN,
				     MACSTR " preference is 0, skip it\n",
				     MAC2STR(prBssDesc->aucBSSID));
				return FALSE;
			}

			if ((ucRequestMode & WNM_BSS_TM_REQ_ABRIDGED) &&
			    !prBssDesc->prNeighbor &&
			    !IS_AIS_CONN_BSSDESC(ais, prBssDesc)) {
				DBGLOG(APS, WARN,
				     MACSTR " not in candidate list, skip it\n",
				     MAC2STR(prBssDesc->aucBSSID));
				return FALSE;
			}
		}
	}
#endif

	return TRUE;
}

uint8_t apsIntraNeedReplaceCandi(struct ADAPTER *ad,
	struct BSS_DESC *cand, struct BSS_DESC *curr,
	uint16_t cand_score, uint16_t curr_score,
	enum ENUM_ROAMING_REASON reason, uint8_t bidx)
{
	struct AIS_FSM_INFO *ais = aisGetAisFsmInfo(ad, bidx);

	if (!cand && curr && IS_AIS_CONN_BSSDESC(ais, curr))
		return TRUE;

	if (curr_score > cand_score)
		return TRUE;

	return FALSE;
}

uint8_t apsLinkPlanDecision(struct ADAPTER *prAdapter,
	struct AP_COLLECTION *prAp, enum ENUM_MLO_LINK_PLAN eLinkPlan,
	uint8_t ucBssIndex)
{
#if (CFG_SUPPORT_802_11BE_MLO == 1)
	uint32_t u4LinkPlanBmap = BIT(MLO_LINK_PLAN_2_5);

#if (CFG_SUPPORT_WIFI_6G == 1)
#if (CFG_MLO_LINK_PLAN_MODE == 0)
	u4LinkPlanBmap |= BIT(MLO_LINK_PLAN_2_6); /* 2+5/6 */
#else
	u4LinkPlanBmap |= BIT(MLO_LINK_PLAN_2_5_6); /* 2+5+6 */
#endif
#endif

#else /* CFG_SUPPORT_802_11BE_MLO */
	uint32_t u4LinkPlanBmap = BIT(MLO_LINK_PLAN_2) | BIT(MLO_LINK_PLAN_5);

#if (CFG_SUPPORT_WIFI_6G == 1)
	u4LinkPlanBmap |= BIT(MLO_LINK_PLAN_6);
#endif

#endif /* CFG_SUPPORT_802_11BE_MLO */

	return !!(u4LinkPlanBmap & BIT(eLinkPlan));
}

struct BSS_DESC *apsIntraUpdateCandi(struct ADAPTER *ad,
	struct AP_COLLECTION *ap, enum ENUM_BAND eBand, uint16_t min_score,
	enum ENUM_ROAMING_REASON reason, uint8_t ignore_policy, uint8_t bidx)
{
	struct AIS_FSM_INFO *ais = aisGetAisFsmInfo(ad, bidx);
	struct CONNECTION_SETTINGS *conn = aisGetConnSettings(ad, bidx);
	enum ENUM_PARAM_CONNECTION_POLICY policy = conn->eConnectionPolicy;
	struct LINK *link = &ap->arLinks[eBand];
	uint8_t aidx = AIS_INDEX(ad, bidx);
	struct BSS_DESC *bss, *cand = NULL;
	uint16_t score, goal_score = 0;
	uint8_t search_blk = ignore_policy;

try_again:
	LINK_FOR_EACH_ENTRY(bss, link, rLinkEntryEss[aidx], struct BSS_DESC) {
		/*
		 * Skip if
		 * 1. bssid is in driver's blocklist in the first round
		 * 2. already picked
		 */
		if ((!search_blk && bss->prBlock) || bss->fgPicked)
			continue;

		if (!ignore_policy) {
			if (policy == CONNECT_BY_BSSID) {
				if (bss->fgIsMatchBssid) {
					cand = bss;
					break;
				}
				continue;
			} else if (policy == CONNECT_BY_BSSID_HINT) {
				if (bss->fgIsMatchBssidHint) {
					cand = bss;
					break;
				}
#if (CFG_EXT_ROAMING == 1)
				/* didn't conn/scan bssid_hint yet */
				if (ais->ucConnTrialCount == 0 &&
				    ais->ucScanTrialCount == 0)
					continue;
#endif
			}

			/* Skip connected AP */
			if (reason != ROAMING_REASON_UPPER_LAYER_TRIGGER &&
			    reason != ROAMING_REASON_BTM &&
			    reason != ROAMING_REASON_BEACON_TIMEOUT &&
			    IS_AIS_CONN_BSSDESC(ais, bss)) {
				DBGLOG(APS, WARN, MACSTR" connected\n",
					MAC2STR(bss->aucBSSID));
				continue;
			}

			/* Skip generated AP */
			if (bss->fgDriverGen) {
				DBGLOG(APS, WARN, "BSS[" MACSTR
					"] is driver gen\n",
					MAC2STR(bss->aucBSSID));
				continue;
			}

			if (!apsIsBssQualify(ad, bss, reason, min_score,
				bss->u2Score, bidx))
				continue;
		}

		score = bss->u2Score;
		if (apsIntraNeedReplaceCandi(ad, cand, bss,
			goal_score, score, reason, bidx)) {
			cand = bss;
			goal_score = score;
		}
	}

	if (cand) {
		if (IS_AIS_CONN_BSSDESC(ais, cand) &&
		    !search_blk && link->u4NumElem > 1) {
			search_blk = TRUE;
			goto try_again;
		}

		cand->fgPicked = TRUE;
		goto done;
	}

	/* if No Candidate BSS is found, try BSSes which are in blocklist */
	if (!search_blk && link->u4NumElem > 1) {
		search_blk = TRUE;
		goto try_again;
	}
done:
	return cand;
}

void apsUpdateTotalScore(struct ADAPTER *prAdapter,
	struct BSS_DESC *arLinks[], uint8_t ucLinkNum,
	struct AP_SCORE_INFO *prScoreInfo, uint8_t ucBssidx)
{
	uint32_t u4TotalScore = 0;
	uint32_t u4TotalTput = 0;
#if (CFG_EXT_ROAMING == 0)
	uint8_t i;

	for (i = 0; i < ucLinkNum; i++) {
		u4TotalScore += arLinks[i]->u2Score;
		u4TotalTput += arLinks[i]->u4Tput;
	}
#else
	u4TotalScore = arLinks[0]->u2Score;
	u4TotalTput = arLinks[0]->u4Tput;

	if (ucLinkNum > 1) {
		u4TotalScore = u4TotalScore *
			(prAdapter->rWifiVar.ucRCMloTpPref + 100) / 100;
		u4TotalTput = u4TotalTput *
			(prAdapter->rWifiVar.ucRCMloTpPref + 100) / 100;
	}
#endif

	kalMemCopy(prScoreInfo->aprTarget, arLinks,
		sizeof(prScoreInfo->aprTarget));
	prScoreInfo->ucLinkNum = ucLinkNum;
	prScoreInfo->u4TotalScore = u4TotalScore;
	prScoreInfo->u4TotalTput = u4TotalTput;
	prScoreInfo->eMloMode = MLO_MODE_STR;
	prScoreInfo->ucMaxSimuLinks = ucLinkNum - 1;
}

uint32_t apsSortGetScore(struct BSS_DESC *candi)
{
	if (candi) {
		if (candi->fgIsMatchBssid || candi->fgIsMatchBssidHint)
			return UINT_MAX;
		else if (!candi->fgDriverGen)
			return candi->u4Tput;
	}
	return 0;
}

uint8_t apsSortTrimCandiByScore(struct ADAPTER *ad, struct BSS_DESC *candi[])
{
	struct BSS_DESC *bss;
	int i, j;
	uint8_t link_num = 0;

	/* insertion sort by score */
	for (i = 1; i < APS_LINK_MAX; i++) {
		uint32_t score = 0;

		bss = candi[i];
		score = apsSortGetScore(bss);

		/* Ensure that NULL will be place at the end */
		for (j = i - 1; j >= 0 && (!candi[j] ||
			apsSortGetScore(candi[j]) < score); j--)
			candi[j + 1] = candi[j];

		candi[j + 1] = bss;
	}

	/* ensure no null target */
	for (i = 0; i < APS_LINK_MAX; i++) {
		if (candi[i])
			link_num++;
	}

	return link_num;
}

const char *apsGetLinkPlanStr(enum ENUM_MLO_LINK_PLAN eLinkPlan)
{
	if (eLinkPlan < MLO_LINK_PLAN_NUM)
		return apucLinkPlanStr[eLinkPlan];

	return (uint8_t *) NULL;
}

enum ENUM_MLO_LINK_PLAN apsRfBandBmapToLinkPlan(
	uint8_t ucRfBandBmap, uint8_t ucLinkNum)
{
	switch (ucRfBandBmap) {
	case BIT(BAND_2G4):
		if (ucLinkNum == 1)
			return MLO_LINK_PLAN_2;
		break;
	case BIT(BAND_5G): /* 5 or 5+5 */
		if (ucLinkNum == 1)
			return MLO_LINK_PLAN_5;
		else if (ucLinkNum == 2)
			return MLO_LINK_PLAN_5_5;
		break;
	case BIT(BAND_2G4) | BIT(BAND_5G): /* 2+5 or 2+5+5 */
		if (ucLinkNum == 2)
			return MLO_LINK_PLAN_2_5;
		else if (ucLinkNum == 3)
			return MLO_LINK_PLAN_2_5_5;
		break;
#if (CFG_SUPPORT_WIFI_6G == 1)
	case BIT(BAND_6G): /* 6 or 6+6 */
		if (ucLinkNum == 1)
			return MLO_LINK_PLAN_6;
		else if (ucLinkNum == 2)
			return MLO_LINK_PLAN_6_6;
		break;
	case BIT(BAND_2G4) | BIT(BAND_6G): /* 2+6 or 2+6+6 */
		if (ucLinkNum == 2)
			return MLO_LINK_PLAN_2_6;
		else if (ucLinkNum == 3)
			return MLO_LINK_PLAN_2_6_6;
		break;
	case BIT(BAND_5G) | BIT(BAND_6G):
		if (ucLinkNum == 2)
			return MLO_LINK_PLAN_5_6;
		break;
	case BIT(BAND_2G4) | BIT(BAND_5G) | BIT(BAND_6G):
		if (ucLinkNum == 3)
			return MLO_LINK_PLAN_2_5_6;
		break;
#endif
	default:
		break;
	}

	return MLO_LINK_PLAN_NUM;
}

uint8_t apsLinksToRfBandBmap(
	struct BSS_DESC *aprLink[], uint8_t ucLinkNum)
{
	uint8_t ucRfBandBmap = 0;
	uint8_t i;

	for (i = 0; i < ucLinkNum; i++) {
		if (aprLink[i])
			ucRfBandBmap |= BIT(aprLink[i]->eBand);
	}

	return ucRfBandBmap;
}

enum ENUM_MLO_LINK_PLAN apsLinksToLinkPlan(
	struct BSS_DESC *aprLink[], uint8_t ucLinkNum)
{
	uint8_t ucRfBandBmap = apsLinksToRfBandBmap(aprLink, ucLinkNum);

	return apsRfBandBmapToLinkPlan(ucRfBandBmap, ucLinkNum);
}

uint8_t apsLinkPlanAllow(struct ADAPTER *ad, struct AP_COLLECTION *ap,
	enum ENUM_MLO_LINK_PLAN plan)
{
	uint8_t link_num = 0;

	/* link plan to link num */
	switch (plan) {
	case MLO_LINK_PLAN_2:
	case MLO_LINK_PLAN_5:
#if (CFG_SUPPORT_WIFI_6G == 1)
	case MLO_LINK_PLAN_6:
#endif
		link_num = 1;
		break;
	case MLO_LINK_PLAN_2_5:
	case MLO_LINK_PLAN_5_5:
#if (CFG_SUPPORT_WIFI_6G == 1)
	case MLO_LINK_PLAN_2_6:
	case MLO_LINK_PLAN_5_6:
	case MLO_LINK_PLAN_6_6:
#endif
		link_num = 2;
		break;
	case MLO_LINK_PLAN_2_5_5:
#if (CFG_SUPPORT_WIFI_6G == 1)
	case MLO_LINK_PLAN_2_5_6:
	case MLO_LINK_PLAN_2_6_6:
#endif
		link_num = 3;
		break;
	default:
		link_num = 0;
		break;
	}

	if (plan < 0 || plan >= MLO_LINK_PLAN_NUM)
		return FALSE;

#if (CFG_SUPPORT_802_11BE_MLO == 1)
	/* trim ap */
	if (link_num > ad->rWifiVar.ucStaMldLinkMax)
		return FALSE;

	/* disallow if over mld block limit*/
	if (ap->prBlock &&
	    (ap->prBlock->u4BlockBmap & BIT(plan)) &&
	    ap->prBlock->aucCount[plan] >= ad->rWifiVar.ucAisMldTrialLimit)
		return FALSE;
#endif

	return TRUE;
}

uint8_t apsIntraNeedReplace(struct ADAPTER *ad,
	struct AP_COLLECTION *ap, struct AP_SCORE_INFO *score_info,
	enum ENUM_ROAMING_REASON reason, uint8_t min_rfband_bmap, uint8_t bidx)
{
	uint8_t rfband_bmap = 0;
	int i;

	/* find matched link plan by final link combination */
	score_info->eLinkPlan = apsLinksToLinkPlan(score_info->aprTarget,
			score_info->ucLinkNum);
	if (!apsLinkPlanAllow(ad, ap, score_info->eLinkPlan))
		return FALSE;

	for (i = 0; i < score_info->ucLinkNum; i++) {
		struct BSS_DESC *bss = score_info->aprTarget[i];

		if (bss) {
			rfband_bmap |= BIT(bss->eBand);
			if (bss->fgIsMatchBssid)
				score_info->fgIsMatchBssid = TRUE;
			if (bss->fgIsMatchBssidHint)
				score_info->fgIsMatchBssidHint = TRUE;
		}
	}

#if (CFG_EXT_ROAMING == 1)
	if (reason == ROAMING_REASON_IDLE &&
	    rfband_bmap <= min_rfband_bmap)
		return FALSE;
#endif

#if (CFG_SINGLE_BAND_MLSR_56 == 1)
	if (mldNeedSingleBandMlsr56(ad, score_info->eLinkPlan) &&
	    score_info->ucLinkNum == 2) {
		score_info->eMloMode = MLO_MODE_SB_MLSR;
		score_info->ucMaxSimuLinks = 0;
		DBGLOG(APS, INFO, "Force to select MLSR 5+6\n");
		return TRUE;
	}
#endif

	if ((!score_info->fgIsMatchBssid && ap->fgIsMatchBssid) ||
	    (!score_info->fgIsMatchBssidHint && ap->fgIsMatchBssidHint))
		return FALSE;

	if ((score_info->fgIsMatchBssid && !ap->fgIsMatchBssid) ||
	    (score_info->fgIsMatchBssidHint && !ap->fgIsMatchBssidHint) ||
	    (score_info->u4TotalTput > ap->u4TotalTput) ||
	    (score_info->u4TotalTput == ap->u4TotalTput &&
	     score_info->ucLinkNum > ap->ucLinkNum))
		return TRUE;

	return FALSE;
}

void apsIntraSelectLinkPlan(struct ADAPTER *ad, struct AP_COLLECTION *ap,
	uint16_t min_score, uint8_t min_rfband_bmap,
	enum ENUM_ROAMING_REASON reason, uint8_t bidx)
{
	struct mt66xx_chip_info *prChipInfo = ad->chip_info;
	struct AIS_FSM_INFO *ais = aisGetAisFsmInfo(ad, bidx);
	uint8_t aidx = AIS_INDEX(ad, bidx);
	struct CONNECTION_SETTINGS *conn = aisGetConnSettings(ad, bidx);
	enum ENUM_PARAM_CONNECTION_POLICY policy = conn->eConnectionPolicy;
	uint32_t allow_plan_bitmap = 0, block_bmap = 0;
	enum ENUM_BAND *link_plan;
	enum ENUM_MLO_LINK_PLAN curr_plan;
	struct BSS_DESC *bss;
	int i, j, k;

	/* candi bss scoring */
	for (i = BAND_2G4; i < BAND_NUM; i++) {
		struct LINK *link = &ap->arLinks[i];

		LINK_FOR_EACH_ENTRY(bss, link,
			rLinkEntryEss[aidx], struct BSS_DESC) {

			bss->u2Score =
				apsCalculateApScore(ad, bss, reason, bidx);
			bss->u4Tput =
				apsGetEstimatedTput(ad, bss, bidx);
			bss->fgIsMatchBssid = FALSE;
			bss->fgIsMatchBssidHint = FALSE;

#if (CFG_SUPPORT_ROAMING_LOG == 1)
			roamingFsmLogCandi(ad, bss, reason, bidx);
#endif

			if (policy == CONNECT_BY_BSSID) {
				if (EQUAL_MAC_ADDR(bss->aucBSSID,
						   conn->aucBSSID)) {
					bss->fgIsMatchBssid = TRUE;
					bss->u2Score = BSS_MATCH_BSSID_SCORE;
				}
			} else if (policy == CONNECT_BY_BSSID_HINT) {
				uint8_t oce = FALSE;
				uint8_t desense = FALSE;
				uint8_t chnl = nicFreq2ChannelNum(
						conn->u4FreqInMHz * 1000);

#if CFG_SUPPORT_MBO
				oce = ad->rWifiVar.u4SwTestMode ==
					ENUM_SW_TEST_MODE_SIGMA_OCE;
#endif

#if (CFG_SUPPORT_AVOID_DESENSE == 1)
				desense = IS_CHANNEL_IN_DESENSE_RANGE(
					ad, bss->ucChannelNum, bss->eBand);
#endif

				if (EQUAL_MAC_ADDR(bss->aucBSSID,
					conn->aucBSSIDHint) &&
				    (chnl == 0 || chnl == bss->ucChannelNum)) {
					if (oce || desense) {
						DBGLOG(APS, INFO,
							"Do network selection even match bssid_hint (oce=%d, desense=%d)\n",
							oce, desense);
					} else {
						bss->fgIsMatchBssidHint = TRUE;
						bss->u2Score =
						     BSS_MATCH_BSSID_HINT_SCORE;
					}
				}
			}
		}
	}

	/* list all combination of allowed link plan */
	for (i = 0; i < MLO_LINK_PLAN_NUM; i++) {
		uint8_t allow;

		if (prChipInfo->apsLinkPlanDecision)
			allow = prChipInfo->apsLinkPlanDecision(ad, ap,
				i, bidx);
		else
			allow = apsLinkPlanDecision(ad, ap,
				i, bidx);

#if (CFG_SINGLE_BAND_MLSR_56 == 1)
		if (mldNeedSingleBandMlsr56(ad, i))
			allow = TRUE;
#endif

		/* skip if not found matched link plan */
		if (!allow)
			continue;

		link_plan = g_aeLinkPlan[i];

		/* Check if the link plan(1 or 2 or 3 links) is allowed */
		if (apsLinkPlanAllow(ad, ap, i))
			allow_plan_bitmap |= BIT(i);

		/* list and check all sub combinations of link plan */
		for (j = 0; j < APS_LINK_MAX; j++) {
			int k;

			if (link_plan[j] == BAND_NULL)
				break;
			/* 1 link */
			curr_plan = apsRfBandBmapToLinkPlan(
					BIT(link_plan[j]),
					1);
			/* Check if the 1-link link plan is allowed */
			if (apsLinkPlanAllow(ad, ap, curr_plan))
				allow_plan_bitmap |= BIT(curr_plan);

			for (k = j + 1; k < APS_LINK_MAX; k++) {
				if (link_plan[k] == BAND_NULL)
					break;
				/* 2 link */
				curr_plan = apsRfBandBmapToLinkPlan(
					BIT(link_plan[j]) | BIT(link_plan[k]),
					2);
				/* Check if the 2-link link plan is allowed */
				if (apsLinkPlanAllow(ad, ap, curr_plan))
					allow_plan_bitmap |= BIT(curr_plan);
			}
		}
	}

	DBGLOG(APS, TRACE, "CAND[%d]["MACSTR"] allow_plan = 0x%x\n",
			  ap->u4Index, MAC2STR(ap->aucAddr),
			  allow_plan_bitmap);

	/* select highest score link plan */
	for (i = 0; i < MLO_LINK_PLAN_NUM; i++) {
		struct AP_SCORE_INFO score_info = {0};
		struct BSS_DESC *candi[APS_LINK_MAX] = {0};
		uint8_t found = FALSE;
		uint32_t akm = 0;
		uint16_t best = 0;
		uint8_t link_num;

		/* skip disallowed plan */
		if (!(allow_plan_bitmap & BIT(i)))
			continue;

		/* reset picked flag */
		for (j = BAND_2G4; j < BAND_NUM; j++) {
			struct LINK *link = &ap->arLinks[j];

			LINK_FOR_EACH_ENTRY(bss, link,
				rLinkEntryEss[aidx], struct BSS_DESC)
				bss->fgPicked = FALSE;
		}

		link_plan = g_aeLinkPlan[i];
		for (j = 0; j < APS_LINK_MAX; j++) {
			if (link_plan[j] == BAND_NULL)
				continue;

			candi[j] = bss = apsIntraUpdateCandi(ad, ap,
			       link_plan[j], min_score, reason, FALSE, bidx);

			if (!bss)
				continue;

			found = TRUE;

			/* use akm of best ap as target akm */
			if (bss->u2Score > best) {
				best = bss->u2Score;
				akm = bss->u4RsnSelectedAKMSuite;
			}
		}

		/* skip if no candidate */
		if (!found)
			continue;

		/* use lower score to find links if already found one */
		for (j = 0; j < APS_LINK_MAX; j++) {
			if (link_plan[j] == BAND_NULL)
				continue;

			bss = candi[j];

			if (!bss)
				candi[j] = bss = apsIntraUpdateCandi(ad, ap,
				       link_plan[j], 0, reason, TRUE, bidx);

			if (bss && bss->u4RsnSelectedAKMSuite != akm) {
				DBGLOG(APS, INFO,
				       "Remove target akm 0x%x!=0x%x\n",
				       bss->u4RsnSelectedAKMSuite, akm);
				candi[j] = NULL;
			}
		}

		/* sort links by score */
		link_num = apsSortTrimCandiByScore(ad, candi);

		/* update total score/mode/max simu.. of link plan */
		if (prChipInfo->apsUpdateTotalScore)
			prChipInfo->apsUpdateTotalScore(ad,
				candi, link_num, &score_info, bidx);
		else
			apsUpdateTotalScore(ad,
				candi, link_num, &score_info, bidx);

		if (apsIntraNeedReplace(ad, ap, &score_info, reason,
			min_rfband_bmap, bidx)) {
			kalMemCopy(ap->aprTarget, score_info.aprTarget,
				sizeof(ap->aprTarget));
			ap->ucLinkNum = score_info.ucLinkNum;
			ap->u4TotalScore = score_info.u4TotalScore;
			ap->u4TotalTput = score_info.u4TotalTput;
			ap->eMloMode = score_info.eMloMode;
			ap->ucMaxSimuLinks = score_info.ucMaxSimuLinks;
			ap->eLinkPlan = score_info.eLinkPlan;
			ap->fgIsMatchBssid = score_info.fgIsMatchBssid;
			ap->fgIsMatchBssidHint = score_info.fgIsMatchBssidHint;

			DBGLOG(APS, TRACE,
				"CAND[%d] num[%d,%s] score[%d] tput[%d] mode[%d] simu[%d]\n",
				ap->u4Index, ap->ucLinkNum,
				apsGetLinkPlanStr(ap->eLinkPlan),
				ap->u4TotalScore, ap->u4TotalTput,
				ap->eMloMode, ap->ucMaxSimuLinks);
		}
	}

	if (ap->ucLinkNum == 0)
		return;

	for (i = 0, j = 0, k = 0; i < ap->ucLinkNum; i++) {
		struct BSS_DESC *cand = ap->aprTarget[i];
		uint8_t addr[MAC_ADDR_LEN] = {0};
		uint8_t *mld_addr = addr;

		if (cand->prBlock) {
			j++;

			if (cand->prBlock->fgDeauthLastTime)
				ap->fgIsLastDeauth = TRUE;
		}

		if (IS_AIS_CONN_BSSDESC(ais, cand))
			k++;

#if (CFG_SUPPORT_802_11BE_MLO == 1)
		mld_addr = cand->rMlInfo.aucMldAddr;
#endif

		DBGLOG(APS, INFO,
			"CAND[%d] BSS[" MACSTR "] band[%s] mld[" MACSTR
			"] score[%d] tput[%d] conn[%d] bssid[%d] bssid_hint[%d] blk[%d] deauth[%d] mode[%d] simu[%d]\n",
			ap->u4Index,
			MAC2STR(cand->aucBSSID),
			apucBandStr[cand->eBand],
			MAC2STR(mld_addr),
			cand->u2Score, cand->u4Tput,
			cand->fgIsConnected,
			cand->fgIsMatchBssid,
			cand->fgIsMatchBssidHint,
			cand->prBlock != NULL,
			ap->fgIsLastDeauth,
			ap->eMloMode, ap->ucMaxSimuLinks);
	}

	if (j == ap->ucLinkNum)
		ap->fgIsAllLinkInBlockList = TRUE;

	if (k == ap->ucLinkNum)
		ap->fgIsAllLinkConnected = TRUE;

#if (CFG_SUPPORT_802_11BE_MLO == 1)
	block_bmap = ap->prBlock ? ap->prBlock->u4BlockBmap : 0;
#endif

	DBGLOG(APS, INFO,
		"CAND[%d] num[%d,%s] score[%d] tput[%d] mode[%d] simu[%d] mld_blk[0x%x] %s%s%s%s\n",
		ap->u4Index, ap->ucLinkNum,
		apsGetLinkPlanStr(ap->eLinkPlan),
		ap->u4TotalScore, ap->u4TotalTput,
		ap->eMloMode, ap->ucMaxSimuLinks, block_bmap,
		ap->fgIsMatchBssid ? "(match_bssid)" : "",
		ap->fgIsMatchBssidHint ? "(match_bssid_hint)" : "",
		ap->fgIsAllLinkConnected ? "(connected)" : "",
		ap->fgIsAllLinkInBlockList ? "(in blocklist)" : "");
}

void apsIntraApSelection(struct ADAPTER *ad,
	enum ENUM_ROAMING_REASON reason, uint8_t bidx)
{
	struct AIS_SPECIFIC_BSS_INFO *s = aisGetAisSpecBssInfo(ad, bidx);
	struct AIS_FSM_INFO *ais = aisGetAisFsmInfo(ad, bidx);
	struct LINK *ess = &s->rCurEssLink;
	struct AP_COLLECTION *ap, *nap;
	struct BSS_DESC *bss, *currBss = NULL;
	struct STA_RECORD *sta;
	uint16_t min_score = 0;
	uint8_t min_rfband_bmap = 0;
	int i;

	/* minimum requirement */
	for (i = 0; i < MLD_LINK_MAX; i++) {
		bss = aisGetLinkBssDesc(ais, i);
		sta = aisGetLinkStaRec(ais, i);

		if (!bss || !sta)
			continue;

		bss->u2Score = apsCalculateApScore(ad, bss, reason, bidx);
		bss->u4Tput = apsGetEstimatedTput(ad, bss, bidx);
		min_rfband_bmap |= BIT(bss->eBand);

		DBGLOG(APS, INFO,
			"CURR[" MACSTR "] score[%d] tput[%d]\n",
			MAC2STR(bss->aucBSSID), bss->u2Score, bss->u4Tput);

		/* highest band: 6G > 5G > 2.4G */
		if (cnmStaRecIsActive(ad, sta) &&
		    (currBss == NULL || bss->eBand > currBss->eBand ||
		    (bss->eBand == currBss->eBand &&
		     bss->u2Score > currBss->u2Score))) {
			currBss = bss;
			min_score = bss->u2Score;
		}
	}

#if (CFG_SUPPORT_ROAMING_LOG == 1)
	roamingFsmLogCurr(ad, currBss, reason, bidx);
#endif

	if (currBss && !apsSanityCheckBssDesc(ad, currBss, reason, bidx))
		min_score = 0;

	LINK_FOR_EACH_ENTRY_SAFE(ap, nap,
			ess, rLinkEntry, struct AP_COLLECTION) {
		/* select best link plan for each ap collection */
		apsIntraSelectLinkPlan(ad, ap, min_score,
			min_rfband_bmap, reason, bidx);
	}
}

uint32_t apsCalculateFinalScore(struct ADAPTER *ad,
	struct AP_COLLECTION *ap, enum ENUM_ROAMING_REASON reason,
	uint8_t bidx)
{
	uint32_t score = 0;

#if (CFG_EXT_ROAMING == 1)
	/* Customization */
	score = ap->u4TotalTput;
#else
	/* Common */
	switch (reason) {
#if CFG_SUPPORT_ROAMING
	case ROAMING_REASON_BTM: {
		uint8_t i;

		if (!aisCheckNeighborApValidity(ad, bidx)) {
			score = ap->u4TotalTput;
		} else {
			for (i = 0; i < ap->ucLinkNum; i++) {
				uint32_t pref;

				if (!ap->aprTarget[i])
					continue;

				pref = apsCalculateScoreByPreference(
					ad, ap->aprTarget[i], reason);

				/* select highest pref as tput score */
				if (score == 0 || pref > score)
					score = pref;
			}
		}
	}
		break;
#endif
	default:
		score = ap->u4TotalTput;
		break;
	}

#endif

	return score;
}

static uint8_t apsNeedReplaceCandidateByRssi(struct ADAPTER *prAdapter,
	struct BSS_DESC *prCandBss, struct BSS_DESC *prCurrBss,
	enum ENUM_ROAMING_REASON eRoamReason)
{
	int8_t cCandRssi = RCPI_TO_dBm(prCandBss->ucRCPI);
	int8_t cCurrRssi = RCPI_TO_dBm(prCurrBss->ucRCPI);
	enum ENUM_BAND eCurrBand = prCurrBss->eBand;
	enum ENUM_BAND eCandBand = prCandBss->eBand;

#if CFG_SUPPORT_NCHO
	if (prAdapter->rNchoInfo.fgNCHOEnabled)
		return cCurrRssi >= cCandRssi ? TRUE : FALSE;
#endif

	/* 1.3 Hard connecting RSSI check */
	if ((eCurrBand == BAND_5G && cCurrRssi < MINIMUM_RSSI_5G) ||
#if (CFG_SUPPORT_WIFI_6G == 1)
	   (eCurrBand == BAND_6G && cCurrRssi < MINIMUM_RSSI_6G) ||
#endif
	   (eCurrBand == BAND_2G4 && cCurrRssi < MINIMUM_RSSI_2G4))
		return FALSE;
	else if ((eCandBand == BAND_5G && cCandRssi < MINIMUM_RSSI_5G) ||
#if (CFG_SUPPORT_WIFI_6G == 1)
	   (eCandBand == BAND_6G && cCandRssi < MINIMUM_RSSI_6G) ||
#endif
	   (eCandBand == BAND_2G4 && cCandRssi < MINIMUM_RSSI_2G4))
		return TRUE;

	/* 1.4 prefer to select 5G Bss if Rssi of a 5G band BSS is good */
	if (eCandBand != eCurrBand) {
		switch (eCandBand) {
		case BAND_2G4:
			/* Current AP is 2.4G, replace candidate AP if target
			 * AP is good
			 */
			if (eCurrBand == BAND_5G
#if (CFG_SUPPORT_WIFI_6G == 1)
			 || eCurrBand == BAND_6G
#endif
			) {
				if (cCurrRssi >= GOOD_RSSI_FOR_HT_VHT)
					return TRUE;

				if (cCurrRssi < LOW_RSSI_FOR_5G_BAND &&
				   (cCandRssi > cCurrRssi + RSSI_DIFF_BIG_STEP))
					return FALSE;

				if (cCandRssi - cCurrRssi >= RSSI_DIFF_BIG_STEP)
					return FALSE;

				if (cCurrRssi - cCandRssi >= RSSI_DIFF_SML_STEP)
					return TRUE;
			}
			break;
		case BAND_5G:
			/* Candidate AP is 5G, don't replace it if it's
			 * good enough.
			 */
			if (eCurrBand == BAND_2G4) {
				if (cCandRssi >= GOOD_RSSI_FOR_HT_VHT)
					return FALSE;

				if (cCandRssi < LOW_RSSI_FOR_5G_BAND &&
				   (cCurrRssi > cCandRssi + RSSI_DIFF_BIG_STEP))
					return TRUE;

				if (cCandRssi - cCurrRssi >= RSSI_DIFF_SML_STEP)
					return FALSE;

				if (cCurrRssi - cCandRssi >= RSSI_DIFF_BIG_STEP)
					return TRUE;
			}
#if (CFG_SUPPORT_WIFI_6G == 1)
			else if (eCurrBand == BAND_6G) {
				/* Target AP is 6G, replace candidate AP
				 * if target AP is good
				 */
				if (cCurrRssi >= GOOD_RSSI_FOR_HT_VHT)
					return TRUE;

				if (cCurrRssi < LOW_RSSI_FOR_5G_BAND &&
				   (cCandRssi > cCurrRssi + RSSI_DIFF_MED_STEP))
					return FALSE;

				if (cCandRssi - cCurrRssi >= RSSI_DIFF_MED_STEP)
					return FALSE;

				if (cCurrRssi - cCandRssi >= RSSI_DIFF_SML_STEP)
					return TRUE;
			}
#endif
			break;
#if (CFG_SUPPORT_WIFI_6G == 1)
		case BAND_6G:
			/* Candidate AP is 6G, don't replace it if
			 * it's good enough.
			 */
			if (eCurrBand == BAND_2G4) {
				if (cCandRssi >= GOOD_RSSI_FOR_HT_VHT)
					return FALSE;

				if (cCandRssi < LOW_RSSI_FOR_5G_BAND &&
				   (cCurrRssi > cCandRssi + RSSI_DIFF_BIG_STEP))
					return TRUE;

				if (cCandRssi - cCurrRssi >= RSSI_DIFF_SML_STEP)
					return FALSE;

				if (cCurrRssi - cCandRssi >= RSSI_DIFF_BIG_STEP)
					return TRUE;
			} else if (eCurrBand == BAND_5G) {
				if (cCandRssi >= GOOD_RSSI_FOR_HT_VHT)
					return FALSE;

				if (cCandRssi < LOW_RSSI_FOR_5G_BAND &&
				   (cCurrRssi > cCandRssi + RSSI_DIFF_MED_STEP))
					return TRUE;

				if (cCandRssi - cCurrRssi >= RSSI_DIFF_SML_STEP)
					return FALSE;

				if (cCurrRssi - cCandRssi >= RSSI_DIFF_MED_STEP)
					return TRUE;
			}
			break;
#endif
		default:
			break;
		}
	} else {
		if (cCandRssi - cCurrRssi >= RSSI_DIFF_MED_STEP)
			return FALSE;
		if (cCurrRssi - cCandRssi >= RSSI_DIFF_MED_STEP)
			return TRUE;
	}

	return FALSE;
}

static uint8_t apsNeedReplaceByRssi(struct ADAPTER *ad,
	struct AP_COLLECTION *cand, struct AP_COLLECTION *curr,
	enum ENUM_ROAMING_REASON reason)
{
	uint8_t i, j;

	for (i = 0; i < curr->ucLinkNum; i++) {
		uint8_t better_rssi = TRUE;

		if (!curr->aprTarget[i])
			continue;

		for (j = 0; j < cand->ucLinkNum; j++) {
			if (!cand->aprTarget[j])
				continue;

			if (!apsNeedReplaceCandidateByRssi(ad,
				cand->aprTarget[j],
				curr->aprTarget[i],
				reason)) {
				better_rssi = FALSE;
				break;
			}
		}

		if (better_rssi)
			return TRUE;
	}

	return FALSE;
}

enum ENUM_APS_REPLACE_REASON apsInterNeedReplace(struct ADAPTER *ad,
	enum ENUM_PARAM_CONNECTION_POLICY policy,
	struct AP_COLLECTION *cand, struct AP_COLLECTION *curr,
	uint32_t cand_score, uint32_t curr_score,
	enum ENUM_ROAMING_REASON reason, uint8_t bidx)
{
	if (policy == CONNECT_BY_BSSID) {
		if (curr->fgIsMatchBssid)
			return APS_MATCH_BSSID;
		else
			return APS_UNMATCH_BSSID;
	} else if (policy == CONNECT_BY_BSSID_HINT) {
		if (curr->fgIsMatchBssidHint)
			return APS_MATCH_BSSID_HINT;
		if (cand && cand->fgIsMatchBssidHint)
			return APS_UNMATCH_BSSID_HINT;
	}

	/*
	 *  If there is no candidate AP,
	 *  then select normal or last deauth AP as candidate AP.
	 */
	if (!cand)
		return APS_FIRST_CANDIDATE;

	/*
	 *  If there is a more suitable AP than the last deauth AP in ESS,
	 *  choose it as the candidate AP.
	 */
	if (curr->fgIsLastDeauth)
		return APS_LAST_DEAUTH;
	else if (cand->fgIsLastDeauth)
		return APS_NOT_LAST_DEAUTH;

#if (CFG_TC10_FEATURE == 0)
	if (apsNeedReplaceByRssi(ad, cand, curr, reason))
		return APS_BETTER_RSSI;
	if (apsNeedReplaceByRssi(ad, curr, cand, reason))
		return APS_WORSE_RSSI;
#endif

	return curr_score > cand_score ? APS_HIGH_SCORE : APS_LOW_SCORE;
}

struct BSS_DESC *apsFillBssDescSet(struct ADAPTER *ad,
	struct AP_COLLECTION *ap, struct BSS_DESC_SET *set, uint8_t bidx)
{
	struct mt66xx_chip_info *prChipInfo = ad->chip_info;
	struct AIS_FSM_INFO *ais;
	struct CONNECTION_SETTINGS *conn = aisGetConnSettings(ad, bidx);
	enum ENUM_PARAM_CONNECTION_POLICY policy = conn->eConnectionPolicy;
	uint8_t i;

	ais = aisGetAisFsmInfo(ad, bidx);

	if (!set)
		return ap ? ap->aprTarget[0] : NULL;

	kalMemSet(set, 0, sizeof(*set));
	if (!ap)
		goto done;

	for (i = 0; i < ap->ucLinkNum && set->ucLinkNum < MLD_LINK_MAX; i++) {
		if (!ap->aprTarget[i])
			continue;

		set->aprBssDesc[set->ucLinkNum++] = ap->aprTarget[i];
		set->ucRfBandBmap |= BIT(ap->aprTarget[i]->eBand);
	}

	if (policy == CONNECT_BY_BSSID)
		goto done;

	if (prChipInfo->apsFillBssDescSet)
		prChipInfo->apsFillBssDescSet(ad, set, bidx);

#if (CFG_SUPPORT_802_11BE_MLO == 1)
	/* pick by special requirement
	 * and avoid picking the poor-quality link.
	 */
	for (i = 1; i < set->ucLinkNum; i++) {
		uint8_t *found = NULL;
		struct BSS_DESC *bss = set->aprBssDesc[i];

		if (IS_AIS_CONN_BSSDESC(ais, set->aprBssDesc[0]) &&
		    !(IS_AIS_CONN_BSSDESC(ais, bss))) {
			set->aprBssDesc[i] = set->aprBssDesc[0];
			set->aprBssDesc[0] = bss;
			found = "connected_link";
		} else if (!IS_AIS_CONN_BSSDESC(ais, bss) &&
		    bss->ucJoinFailureCount <
		    set->aprBssDesc[0]->ucJoinFailureCount) {
			set->aprBssDesc[i] = set->aprBssDesc[0];
			set->aprBssDesc[0] = bss;
			found = "bad_main_link";
		} else if (IS_FEATURE_ENABLED(ad->rWifiVar.ucStaPreferMldAddr)
			   && EQUAL_MAC_ADDR(bss->aucBSSID, ap->aucAddr)) {
			set->aprBssDesc[i] = set->aprBssDesc[0];
			set->aprBssDesc[0] = bss;
			found = "mld_addr";
		} else if (bss->rMlInfo.ucLinkIndex ==
			   ad->rWifiVar.ucStaMldMainLinkIdx) {
			set->aprBssDesc[i] = set->aprBssDesc[0];
			set->aprBssDesc[0] = bss;
			found = "link_id";
		}

		if (found != NULL) {
			DBGLOG(APS, INFO, MACSTR
				" link_id=%d max_links=%d Setup for %s\n",
				MAC2STR(bss->aucBSSID),
				bss->rMlInfo.ucLinkIndex,
				bss->rMlInfo.ucMaxSimuLinks,
				found);
			goto done;
		}
	}
#endif

done:
	/* first bss desc is main bss */
	set->prMainBssDesc = set->aprBssDesc[0];
	if (ap) {
		set->fgIsMatchBssid = ap->fgIsMatchBssid;
		set->fgIsMatchBssidHint = ap->fgIsMatchBssidHint;
		set->fgIsAllLinkInBlockList = ap->fgIsAllLinkInBlockList;
		set->fgIsAllLinkConnected = ap->fgIsAllLinkConnected;
		set->eMloMode = ap->eMloMode;
		set->ucMaxSimuLinks = ap->ucMaxSimuLinks;
	}
	DBGLOG(APS, INFO, "Total %d link(s)\n", set->ucLinkNum);
	return set->prMainBssDesc;
}

struct BSS_DESC *apsInterApSelection(struct ADAPTER *ad,
	struct BSS_DESC_SET *set,
	enum ENUM_ROAMING_REASON reason, uint8_t bidx)
{
	struct CONNECTION_SETTINGS *conn = aisGetConnSettings(ad, bidx);
	enum ENUM_PARAM_CONNECTION_POLICY policy = conn->eConnectionPolicy;
	struct AIS_SPECIFIC_BSS_INFO *s = aisGetAisSpecBssInfo(ad, bidx);
	struct LINK *ess = &s->rCurEssLink;
	struct AP_COLLECTION *ap, *cand = NULL;
	uint32_t best = 0, score = 0;
	uint8_t try_blockList = FALSE;
	enum ENUM_APS_REPLACE_REASON replace_reason;

try_again:
	LINK_FOR_EACH_ENTRY(ap, ess, rLinkEntry, struct AP_COLLECTION) {
		if (ap->ucLinkNum == 0)
			continue;

		if (!try_blockList && ap->fgIsAllLinkInBlockList)
			continue;

		score = apsCalculateFinalScore(ad, ap, reason, bidx);
		replace_reason = apsInterNeedReplace(ad, policy,
			cand, ap, best, score, reason, bidx);
		if (replace_reason >= APS_NEED_REPLACE) {
			best = score;
			cand = ap;
		}

		DBGLOG(APS, INFO,
			"%s CAND[%d] %s[" MACSTR
			"] num[%d,%s] score[%d] (%s)\n",
			replace_reason >= APS_FIRST_CANDIDATE ? "--->" : "<---",
			ap->u4Index, ap->fgIsMld ? "MLD" : "BSS",
			MAC2STR(ap->aucAddr), ap->ucLinkNum,
			apsGetLinkPlanStr(ap->eLinkPlan), score,
			replace_reason < APS_REPLACE_REASON_NUM ?
			apucReplaceReasonStr[replace_reason] :
			"UNKNOWN");

		/* early leave for specific reasons */
		if (replace_reason == APS_MATCH_BSSID ||
		    replace_reason == APS_MATCH_BSSID_HINT)
			goto done;
	}

	if (!try_blockList && (!cand || cand->fgIsAllLinkConnected)) {
		try_blockList = TRUE;
		DBGLOG(APS, INFO, "No ap collection found, try blocklist\n");
		goto try_again;
	}

done:
	return apsFillBssDescSet(ad, cand, set, bidx);
}

void apsGetConnectionPolicyStr(struct ADAPTER *ad, char *buf,
	uint16_t buf_size, uint8_t bidx)
{
	struct CONNECTION_SETTINGS *conn = aisGetConnSettings(ad, bidx);

	if (conn->eConnectionPolicy == CONNECT_BY_BSSID)
		kalSnprintf(buf, buf_size,
			" policy=BSSID " MACSTR,
			MAC2STR(conn->aucBSSID));
	else if (conn->eConnectionPolicy == CONNECT_BY_BSSID_HINT)
		kalSnprintf(buf, buf_size,
			" policy=BSSID_HINT " MACSTR,
			MAC2STR(conn->aucBSSIDHint));
	else if (conn->eConnectionPolicy == CONNECT_BY_SSID_BEST_RSSI)
		kalSnprintf(buf, buf_size,
			" policy=SSID_BEST_RSSI");
	else if (conn->eConnectionPolicy ==
		CONNECT_BY_SSID_GOOD_RSSI_MIN_CH_LOAD)
		kalSnprintf(buf, buf_size,
			" policy=SSID_GOOD_RSSI_MIN_CH_LOAD");
	else if (conn->eConnectionPolicy == CONNECT_BY_SSID_ANY)
		kalSnprintf(buf, buf_size,
			" policy=SSID_ANY");
	else
		kalSnprintf(buf, buf_size,
			" policy=OTHER");
}

struct BSS_DESC *apsSearchBssDescByScore(struct ADAPTER *ad,
	enum ENUM_ROAMING_REASON reason,
	uint8_t bidx, struct BSS_DESC_SET *set)
{
	struct AIS_SPECIFIC_BSS_INFO *s = aisGetAisSpecBssInfo(ad, bidx);
	struct LINK *ess = &s->rCurEssLink;
	struct CONNECTION_SETTINGS *conn = aisGetConnSettings(ad, bidx);
	struct BSS_DESC *cand = NULL;
	uint16_t count = 0;
	char buf[100] = {0};


	if (reason >= ROAMING_REASON_NUM) {
		DBGLOG(APS, ERROR, "reason %d!\n", reason);
		return NULL;
	}

	DBGLOG(APS, INFO, "ConnectionPolicy = %d, reason = %d\n",
		conn->eConnectionPolicy, reason);

	aisClearCusBlocklist(ad, bidx, FALSE);
	aisRemoveDeauthBlocklist(ad, FALSE,
		ad->rWifiVar.u2AisDeauthBlocklistTimeout);
	aisRemoveTimeoutBlocklist(ad, ad->rWifiVar.u2AisBlocklistTimeout);
#if (CFG_SUPPORT_802_11BE_MLO == 1)
	aisRemoveTimeoutMldBlocklist(ad, ad->rWifiVar.u2AisMldBlocklistTimeout);
#endif

#if CFG_SUPPORT_802_11K
	/* check before using neighbor report */
	aisCheckNeighborApValidity(ad, bidx);
#endif

	apsGetConnectionPolicyStr(ad, buf, sizeof(buf), bidx);
	count = apsUpdateEssApList(ad, reason, bidx);
	apsIntraApSelection(ad, reason, bidx);
	cand = apsInterApSelection(ad, set, reason, bidx);

	if (cand) {
		if (cand->eBand < 0 || cand->eBand >= BAND_NUM) {
			DBGLOG(APS, WARN, "Invalid Band %d\n", cand->eBand);
		} else {
			DBGLOG(APS, INFO,
				"Selected " MACSTR
				", RSSI[%d] Band[%s] when find %s%s in %d(%d) BSSes.\n",
				MAC2STR(cand->aucBSSID),
				RCPI_TO_dBm(cand->ucRCPI),
				apucBandStr[cand->eBand], HIDE(conn->aucSSID),
				buf, count, ess->u4NumElem);
			goto done;
		}
	}

	DBGLOG(APS, INFO, "Selected None when find %s%s in %d(%d) BSSes.\n",
		HIDE(conn->aucSSID), buf, count, ess->u4NumElem);

done:
#if (CFG_SUPPORT_ROAMING_LOG == 1)
	roamingFsmLogResult(ad, bidx, cand);
#endif
	apsResetEssApList(ad, bidx);
	return cand;
}

