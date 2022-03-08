/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

/*
** Id: @(#) eht_rlm.c@@
*/

/*! \file   "eht_rlm.c"
*    \brief This file contains EHT Phy processing routines.
*
*/

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/

#include "precomp.h"

#if (CFG_SUPPORT_802_11BE == 1)
#include "eht_rlm.h"

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/


/*******************************************************************************
*                   F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

uint32_t ehtRlmCalculateCapIELen(
	struct ADAPTER *prAdapter,
	uint8_t ucBssIndex,
	struct STA_RECORD *prStaRec)
{
	enum ENUM_BAND eHePhyCapBand = BAND_5G;
	struct BSS_INFO *prBssInfo;
	uint8_t ucMaxBw;
	uint32_t u4OverallLen;

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, ucBssIndex);
	u4OverallLen = OFFSET_OF(struct IE_EHT_CAP, aucVarInfo[0]);
#if (CFG_SUPPORT_WIFI_6G == 1)
	if (prBssInfo->eBand == BAND_6G)
		eHePhyCapBand = BAND_6G;
#endif
	ucMaxBw = cnmGetBssBandBw(prAdapter, prBssInfo, eHePhyCapBand);

	if (ucMaxBw == MAX_BW_20MHZ) {
		u4OverallLen += sizeof(struct EHT_SUPPORTED_MCS_BW20_FIELD);
        } else {
		if (ucMaxBw >= MAX_BW_40MHZ) {
			u4OverallLen += sizeof(
				struct EHT_SUPPORTED_MCS_BW80_160_320_FIELD);
		}
		if (ucMaxBw >= MAX_BW_160MHZ) {
			u4OverallLen += sizeof(
				struct EHT_SUPPORTED_MCS_BW80_160_320_FIELD);
		}
		if (ucMaxBw >= MAX_BW_320MHZ) {
			u4OverallLen += sizeof(
				struct EHT_SUPPORTED_MCS_BW80_160_320_FIELD);
		}
        }

	// TODO: add variable IE length for EHT PPE threshold

	return u4OverallLen;
}

uint32_t ehtRlmCalculateOpIELen(
	struct ADAPTER *prAdapter,
	uint8_t ucBssIndex,
	struct STA_RECORD *prStaRec)
{
	/* struct BSS_INFO *prBssInfo = (struct BSS_INFO *) NULL; */
	uint32_t u4OverallLen = OFFSET_OF(struct IE_EHT_OP, aucVarInfo[0]);

	return u4OverallLen;
}

static void ehtRlmFillBW80MCSMap(
	struct ADAPTER *prAdapter,
	struct BSS_INFO *prBssInfo,
	uint8_t *prEhtSupportedMcsSet)
{
	uint8_t ucMcsMap, ucSupportedNss;
	struct EHT_SUPPORTED_MCS_BW80_160_320_FIELD *_prEhtSupportedMcsSet
			= (struct EHT_SUPPORTED_MCS_BW80_160_320_FIELD *)
				prEhtSupportedMcsSet;

	kalMemZero((void *) prEhtSupportedMcsSet,
		sizeof(struct EHT_SUPPORTED_MCS_BW80_160_320_FIELD));
	ucSupportedNss = wlanGetSupportNss(prAdapter, prBssInfo->ucBssIndex);
	ucMcsMap = ucSupportedNss + (ucSupportedNss << 4);
	_prEhtSupportedMcsSet->eht_mcs_0_9 = ucMcsMap;
	_prEhtSupportedMcsSet->eht_mcs_10_11 = ucMcsMap;
	_prEhtSupportedMcsSet->eht_mcs_12_13 = ucMcsMap;
}

static void ehtRlmFillBW20MCSMap(
	struct ADAPTER *prAdapter,
	struct BSS_INFO *prBssInfo,
	uint8_t *prEhtSupportedMcsSet)
{
	uint8_t ucMcsMap, ucSupportedNss;
	struct EHT_SUPPORTED_MCS_BW20_FIELD *_prEhtSupportedMcsSet =
		(struct EHT_SUPPORTED_MCS_BW20_FIELD *) prEhtSupportedMcsSet;

	kalMemZero((void *) prEhtSupportedMcsSet,
		sizeof(struct EHT_SUPPORTED_MCS_BW20_FIELD));
	ucSupportedNss = wlanGetSupportNss(prAdapter, prBssInfo->ucBssIndex);

	ucMcsMap = ucSupportedNss + (ucSupportedNss << 4);
	_prEhtSupportedMcsSet->eht_bw20_mcs_0_7 = ucMcsMap;
	_prEhtSupportedMcsSet->eht_bw20_mcs_8_9 = ucMcsMap;
	_prEhtSupportedMcsSet->eht_bw20_mcs_10_11 = ucMcsMap;
	_prEhtSupportedMcsSet->eht_bw20_mcs_12_13 = ucMcsMap;
}

static void ehtRlmFillCapIE(
	struct ADAPTER *prAdapter,
	struct BSS_INFO *prBssInfo,
	struct MSDU_INFO *prMsduInfo)
{
	enum ENUM_BAND eHePhyCapBand = BAND_5G;
	struct IE_EHT_CAP *prEhtCap;
	struct EHT_PHY_CAP_INFO eht_phy_cap;
	uint32_t phy_cap_1 = 0;
	uint32_t phy_cap_2 = 0;
	uint32_t u4OverallLen = OFFSET_OF(struct IE_EHT_CAP, aucVarInfo[0]);
	uint8_t eht_mcs15_mru = EHT_MCS15_MRU_106_or_52_w_26_tone;
	uint8_t eht_bw = 0, ucSupportedNss = 0;

	ASSERT(prAdapter);
	ASSERT(prBssInfo);
	ASSERT(prMsduInfo);

	prEhtCap = (struct IE_EHT_CAP *)
		(((uint8_t *)prMsduInfo->prPacket)+prMsduInfo->u2FrameLength);

	prEhtCap->ucId = ELEM_ID_RESERVED;
	prEhtCap->ucExtId = ELEM_EXT_ID_EHT_CAPS;

	ucSupportedNss = wlanGetSupportNss(prAdapter, prBssInfo->ucBssIndex);

	/* MAC capabilities */
	EHT_RESET_MAC_CAP(prEhtCap->ucEhtMacCap);

	/* NSEP: not support for the time being */
	/* SET_EHT_MAC_CAP_NSEP_PRI_ACCESS(prEhtCap->ucEhtMacCap); */
	/* OM_CTRL: default support for both STA and AP; */
	SET_EHT_MAC_CAP_OM_CTRL(prEhtCap->ucEhtMacCap);

	/* TXOP_SHARING: not support */
	/* SET_EHT_MAC_CAP_TXOP_SHARING(prEhtCap->ucEhtMacCap); */

	/* PHY capabilities */
	EHT_RESET_PHY_CAP(prEhtCap->ucEhtPhyCap);

#if (CFG_SUPPORT_WIFI_6G == 1)
	if (prBssInfo->eBand == BAND_6G) {
		if (cnmGetBssBandBw(prAdapter, prBssInfo, BAND_6G)
			>= MAX_BW_320MHZ)
			phy_cap_1 |= DOT11BE_PHY_CAP_320M_6G;
		eHePhyCapBand = BAND_6G;
	}
#endif

	eht_bw = cnmGetBssBandBw(prAdapter, prBssInfo, eHePhyCapBand);
	if (!IS_BSS_APGO(prBssInfo)) {
		phy_cap_2 |= DOT11BE_PHY_CAP_PARTIAL_BW_DL_MU_MIMO;

		if (eht_bw == MAX_BW_20MHZ)
			phy_cap_1 &= ~DOT11BE_PHY_CAP_242_TONE_RU_WT_20M;
		else
			phy_cap_1 |= DOT11BE_PHY_CAP_242_TONE_RU_WT_20M;
	} else {
		/* phy_cap_2 |= DOT11BE_PHY_CAP_PPE_THRLD_PRESENT; */
	}

	if (eht_bw >= MAX_BW_40MHZ) {
		eht_mcs15_mru |= EHT_MCS15_MRU_484_w_242_tone_80M;
		/* set 3 to support AP NSS 4 */
		SET_DOT11BE_PHY_CAP_BFEE_SS_LE_EQ_80M(phy_cap_1, 3);
		/* set 1 to support 2 TX NSS */
		SET_DOT11BE_PHY_CAP_SOUND_DIM_NUM_LE_EQ_80M(
			phy_cap_1, ucSupportedNss - 1);
	}
	if (eht_bw >= MAX_BW_160MHZ) {
		eht_mcs15_mru |= EHT_MCS15_MRU_996_to_242_tone_160M;
		/* set 3 to support AP NSS 4 */
		SET_DOT11BE_PHY_CAP_BFEE_160M(phy_cap_1, 3);
		/* set 1 to support TX NSS 2 */
		SET_DOT11BE_PHY_CAP_SOUND_DIM_NUM_160M(
			phy_cap_1, ucSupportedNss - 1);
	}
	if (eht_bw >= MAX_BW_320MHZ) {
		eht_mcs15_mru |= EHT_MCS15_MRU_3x996_tone_320M;
		/* set 3 to support AP NSS 4 */
		SET_DOT11BE_PHY_CAP_BFEE_320M(phy_cap_1, 3);
		/* set 1 to support TX NSS 2 */
		SET_DOT11BE_PHY_CAP_SOUND_DIM_NUM_320M(
			phy_cap_1, ucSupportedNss - 1);
	}

	phy_cap_1 |= DOT11BE_PHY_CAP_NDP_4X_EHT_LTF_3DOT2US_GI;
	/* phy_cap_1 &= ~DOT11BE_PHY_CAP_PARTIAL_BW_UL_MU_MIMO; */
	phy_cap_1 |= DOT11BE_PHY_CAP_SU_BFER;
	phy_cap_1 |= DOT11BE_PHY_CAP_SU_BFEE;
	phy_cap_1 |= DOT11BE_PHY_CAP_NG16_SU_FEEDBACK;
	phy_cap_1 |= DOT11BE_PHY_CAP_NG16_MU_FEEDBACK;
	phy_cap_1 |= DOT11BE_PHY_CAP_CODEBOOK_7_5_MU_FEEDBACK;
	phy_cap_1 |= DOT11BE_PHY_CAP_TRIGED_SU_BF_FEEDBACK;
	phy_cap_1 |= DOT11BE_PHY_CAP_TRIGED_MU_BF_PARTIAL_BW_FEEDBACK;
	phy_cap_1 |= DOT11BE_PHY_CAP_TRIGED_CQI_FEEDBACK;
	phy_cap_2 |= DOT11BE_PHY_CAP_PARTIAL_BW_DL_MU_MIMO;
	/* phy_cap_2 &= ~DOT11BE_PHY_CAP_PSR_BASED_SR; */
	/* phy_cap_2 &= ~DOT11BE_PHY_CAP_POWER_BOOST_FACTOR; */
	phy_cap_2 |= DOT11BE_PHY_CAP_EHT_MU_PPDU_4X_EHT_LTF_DOT8US_GI;
	SET_DOT11BE_PHY_CAP_MAX_NC(phy_cap_2, ucSupportedNss - 1);
        phy_cap_2 |= DOT11BE_PHY_CAP_NON_TRIGED_CQI_FEEDBACK;
	phy_cap_2 |= DOT11BE_PHY_CAP_TX_1024QAM_4096QAM_LE_242_TONE_RU;
	phy_cap_2 |= DOT11BE_PHY_CAP_RX_1024QAM_4096QAM_LE_242_TONE_RU;
	/* phy_cap_2 |= DOT11BE_PHY_CAP_PPE_THRLD_PRESENT; */
	phy_cap_2 |= DOT11BE_PHY_CAP_EHT_MU_PPDU_4X_EHT_LTF_DOT8US_GI;
	SET_DOT11BE_PHY_CAP_MAX_EHT_LTF_NUM(phy_cap_2, 0x0B);
	phy_cap_2 |= DOT11BE_PHY_CAP_EHT_DUP_6G;
	phy_cap_2 |= DOT11BE_PHY_CAP_20M_RX_NDP_W_WIDER_BW;
	/* phy_cap_2 &= ~DOT11BE_PHY_CAP_NON_OFDMA_UL_MU_MIMO_80M; */
	/* phy_cap_2 &= ~DOT11BE_PHY_CAP_NON_OFDMA_UL_MU_MIMO_160M; */
	/* phy_cap_2 &= ~DOT11BE_PHY_CAP_NON_OFDMA_UL_MU_MIMO_320M; */
	/* phy_cap_2 &= ~DOT11BE_PHY_CAP_MU_BFER_80M; */
	/* phy_cap_2 &= ~DOT11BE_PHY_CAP_MU_BFER_160M; */
	/* phy_cap_2 &= ~DOT11BE_PHY_CAP_MU_BFER_320M; */

	SET_DOT11BE_PHY_CAP_COMMON_NOMINAL_PKT_PAD(phy_cap_2,
		COMMON_NOMINAL_PAD_16_US);

	SET_DOT11BE_PHY_CAP_MCS_15(phy_cap_2, eht_mcs15_mru);

	DBGLOG(RLM, INFO,
		"[%d] eht_bw=%d, phy_cap_1=0x%x, phy_cap_2=0x%x\n",
		prBssInfo->ucBssIndex,
		eht_bw, phy_cap_1, phy_cap_2);

	eht_phy_cap.phy_capinfo_1 = (phy_cap_1);
	eht_phy_cap.phy_capinfo_2 = (phy_cap_2);

	memcpy(prEhtCap->ucEhtPhyCap, &phy_cap_1,
		sizeof(phy_cap_1));
	memcpy(prEhtCap->ucEhtPhyCap + 4, &phy_cap_2,
		sizeof(phy_cap_2));

	/* Set EHT MCS MAP & NSS */
	if (eht_bw == MAX_BW_20MHZ) {
		uint8_t *prEhtSupportedBw20McsSet = NULL;

		prEhtSupportedBw20McsSet =
			(((uint8_t *) prEhtCap) + u4OverallLen);
		ehtRlmFillBW20MCSMap(
			prAdapter, prBssInfo, prEhtSupportedBw20McsSet);
		u4OverallLen += sizeof(struct EHT_SUPPORTED_MCS_BW20_FIELD);
	} else {
		uint8_t *prEhtSupportedBw80McsSet = NULL;

		if (eht_bw >= MAX_BW_40MHZ) {
			prEhtSupportedBw80McsSet =
				(((uint8_t *) prEhtCap) + u4OverallLen);
			ehtRlmFillBW80MCSMap(
				prAdapter, prBssInfo, prEhtSupportedBw80McsSet);
			u4OverallLen += sizeof(
				struct EHT_SUPPORTED_MCS_BW80_160_320_FIELD);
		}
		if (eht_bw >= MAX_BW_160MHZ) {
			prEhtSupportedBw80McsSet =
				(((uint8_t *) prEhtCap) + u4OverallLen);
			ehtRlmFillBW80MCSMap(
				prAdapter, prBssInfo, prEhtSupportedBw80McsSet);
			u4OverallLen += sizeof(
				struct EHT_SUPPORTED_MCS_BW80_160_320_FIELD);
		}
		if (eht_bw >= MAX_BW_320MHZ) {
			prEhtSupportedBw80McsSet =
				(((uint8_t *) prEhtCap) + u4OverallLen);
			ehtRlmFillBW80MCSMap(
				prAdapter, prBssInfo, prEhtSupportedBw80McsSet);
			u4OverallLen += sizeof(
				struct EHT_SUPPORTED_MCS_BW80_160_320_FIELD);
		}
	}
	/* Set EHT PPE Thresholds */
	if (phy_cap_2 & DOT11BE_PHY_CAP_PPE_THRLD_PRESENT) {
		// TODO: add EHT PPE threshold
	}

	prEhtCap->ucLength = u4OverallLen - ELEM_HDR_LEN;
	prMsduInfo->u2FrameLength += IE_SIZE(prEhtCap);

	DBGLOG_MEM8(RLM, INFO, prEhtCap, IE_SIZE(prEhtCap));
}

void ehtRlmReqGenerateCapIE(
	struct ADAPTER *prAdapter,
	struct MSDU_INFO *prMsduInfo)
{
	struct BSS_INFO *prBssInfo;
	struct STA_RECORD *prStaRec;

	ASSERT(prAdapter);
	ASSERT(prMsduInfo);

	prBssInfo = prAdapter->aprBssInfo[prMsduInfo->ucBssIndex];
	if (!prBssInfo)
		return;

	prStaRec = cnmGetStaRecByIndex(prAdapter, prMsduInfo->ucStaRecIndex);

	if ((prAdapter->rWifiVar.ucAvailablePhyTypeSet & PHY_TYPE_SET_802_11BE)
	    && (!prStaRec || (prStaRec->ucPhyTypeSet & PHY_TYPE_SET_802_11BE)))
		ehtRlmFillCapIE(prAdapter, prBssInfo, prMsduInfo);
}

void ehtRlmRspGenerateCapIE(
	struct ADAPTER *prAdapter,
	struct MSDU_INFO *prMsduInfo)
{
	struct BSS_INFO *prBssInfo;
	struct STA_RECORD *prStaRec;
	uint8_t ucPhyTypeSet;

	ASSERT(prAdapter);
	ASSERT(prMsduInfo);

	prBssInfo = prAdapter->aprBssInfo[prMsduInfo->ucBssIndex];
	if (!prBssInfo)
		return;

	if (!IS_BSS_ACTIVE(prBssInfo))
		return;

	prStaRec = cnmGetStaRecByIndex(prAdapter, prMsduInfo->ucStaRecIndex);

	/* Decide PHY type set source */
	if (prStaRec) {
		/* Get PHY type set from target STA */
		ucPhyTypeSet = prStaRec->ucPhyTypeSet;
	} else {
		/* Get PHY type set from current BSS */
		ucPhyTypeSet = prBssInfo->ucPhyTypeSet;
	}

	if (RLM_NET_IS_11BE(prBssInfo) &&
	    (ucPhyTypeSet & PHY_TYPE_SET_802_11BE))
		ehtRlmFillCapIE(prAdapter, prBssInfo, prMsduInfo);
}


uint8_t ehtRlmGetEhtOpBwByBssOpBw(uint8_t ucBssOpBw)
{
	uint8_t ucEhtOpBw = EHT_MAX_BW_20;

	switch (ucBssOpBw) {
	case MAX_BW_20MHZ:
		ucEhtOpBw = EHT_MAX_BW_20;
		break;

	case MAX_BW_40MHZ:
		ucEhtOpBw = EHT_MAX_BW_40;
		break;

	case MAX_BW_80MHZ:
		ucEhtOpBw = EHT_MAX_BW_80;
		break;

	case MAX_BW_160MHZ:
		ucEhtOpBw = EHT_MAX_BW_160;
		break;

	case MAX_BW_320MHZ:
		ucEhtOpBw = EHT_MAX_BW_320;
		break;

	default:
		DBGLOG(RLM, WARN, "unexpected Bss OP BW: %d\n",
		       ucBssOpBw);

		ucEhtOpBw = EHT_MAX_BW_20;
		break;
	}

	return ucEhtOpBw;
}

static void ehtRlmFillOpIE(
	struct ADAPTER *prAdapter,
	struct BSS_INFO *prBssInfo,
	struct MSDU_INFO *prMsduInfo)
{
	struct IE_EHT_OP*prEhtOp;
	uint32_t u4OverallLen = OFFSET_OF(struct IE_EHT_OP, aucVarInfo[0]);
	uint8_t eht_bw = 0;

	ASSERT(prAdapter);
	ASSERT(prBssInfo);
	ASSERT(prMsduInfo);

	prEhtOp = (struct IE_EHT_OP *)
		(((uint8_t *)prMsduInfo->prPacket)+prMsduInfo->u2FrameLength);

	prEhtOp->ucId = ELEM_ID_RESERVED;
	prEhtOp->ucExtId = ELEM_EXT_ID_EHT_OP;

	/* MAC capabilities */
	EHT_RESET_OP(prEhtOp->ucEhtOpParams);

	eht_bw = cnmGetBssBandBw(prAdapter, prBssInfo,
		prBssInfo->eBand);

	/* EHT Operation Information */
	EHT_SET_OP_CHAN_WIDTH(prEhtOp->ucEhtOpParams,
		ehtRlmGetEhtOpBwByBssOpBw(eht_bw));

	DBGLOG(RLM, INFO, "EHT channel width: %d\n",
		EHT_GET_OP_CHAN_WIDTH(prEhtOp->ucEhtOpParams));

	prEhtOp->ucLength = u4OverallLen - ELEM_HDR_LEN;

	prMsduInfo->u2FrameLength += IE_SIZE(prEhtOp);
}

void ehtRlmRspGenerateOpIE(
	struct ADAPTER *prAdapter,
	struct MSDU_INFO *prMsduInfo)
{
	struct BSS_INFO *prBssInfo;
	struct STA_RECORD *prStaRec;
	uint8_t ucPhyTypeSet;

	ASSERT(prAdapter);
	ASSERT(prMsduInfo);

	prBssInfo = prAdapter->aprBssInfo[prMsduInfo->ucBssIndex];
	if (!prBssInfo)
		return;

	if (!IS_BSS_ACTIVE(prBssInfo))
		return;

	prStaRec = cnmGetStaRecByIndex(prAdapter, prMsduInfo->ucStaRecIndex);

	/* Decide PHY type set source */
	if (prStaRec) {
		/* Get PHY type set from target STA */
		ucPhyTypeSet = prStaRec->ucPhyTypeSet;
	} else {
		/* Get PHY type set from current BSS */
		ucPhyTypeSet = prBssInfo->ucPhyTypeSet;
	}

	if (RLM_NET_IS_11BE(prBssInfo) &&
	    (ucPhyTypeSet & PHY_TYPE_SET_802_11BE))
		ehtRlmFillOpIE(prAdapter, prBssInfo, prMsduInfo);
}

static void ehtRlmRecMcsMap(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	struct IE_EHT_CAP *prEhtCap)
{
	enum ENUM_BAND eHePhyCapBand = BAND_5G;
	uint32_t u4McsMapOffset;
	struct BSS_INFO *prBssInfo;
	uint8_t ucMaxBw;

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);
#if (CFG_SUPPORT_WIFI_6G == 1)
	if (prBssInfo->eBand == BAND_6G)
		eHePhyCapBand = BAND_6G;
#endif
	ucMaxBw = cnmGetBssBandBw(prAdapter, prBssInfo, eHePhyCapBand);

	u4McsMapOffset = OFFSET_OF(struct IE_EHT_CAP, aucVarInfo[0]);

	kalMemZero((void *) prStaRec->aucMscMap20MHzSta,
		sizeof(struct EHT_SUPPORTED_MCS_BW20_FIELD));
	kalMemZero((void *) prStaRec->aucMscMap80MHz,
		sizeof(struct EHT_SUPPORTED_MCS_BW80_160_320_FIELD));
	kalMemZero((void *) prStaRec->aucMscMap160MHz,
		sizeof(struct EHT_SUPPORTED_MCS_BW80_160_320_FIELD));
	kalMemZero((void *) prStaRec->aucMscMap320MHz,
		sizeof(struct EHT_SUPPORTED_MCS_BW80_160_320_FIELD));
	if (ucMaxBw == MAX_BW_20MHZ) {
		memcpy(prStaRec->aucMscMap20MHzSta,
			((uint8_t *)prEhtCap) + u4McsMapOffset,
			sizeof(struct EHT_SUPPORTED_MCS_BW20_FIELD));
        } else {
		if (ucMaxBw >= MAX_BW_40MHZ) {
			memcpy(prStaRec->aucMscMap80MHz,
				((uint8_t *)prEhtCap) + u4McsMapOffset,
				sizeof(struct
					EHT_SUPPORTED_MCS_BW80_160_320_FIELD));
			u4McsMapOffset +=
				sizeof(struct
					EHT_SUPPORTED_MCS_BW80_160_320_FIELD);
		}
		if (ucMaxBw >= MAX_BW_160MHZ) {
			memcpy(prStaRec->aucMscMap160MHz,
				((uint8_t *)prEhtCap) + u4McsMapOffset,
				sizeof(struct
					EHT_SUPPORTED_MCS_BW80_160_320_FIELD));
			u4McsMapOffset +=
				sizeof(struct
					EHT_SUPPORTED_MCS_BW80_160_320_FIELD);
		}
		if (ucMaxBw >= MAX_BW_320MHZ) {
			memcpy(prStaRec->aucMscMap320MHz,
				((uint8_t *)prEhtCap) + u4McsMapOffset,
				sizeof(struct
					EHT_SUPPORTED_MCS_BW80_160_320_FIELD));
		}
        }
}

void ehtRlmRecCapInfo(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	uint8_t *pucIE)
{
	struct IE_EHT_CAP *prEhtCap = (struct IE_EHT_CAP *) pucIE;

	/* if payload not contain any aucVarInfo,
	 * IE size = sizeof(struct IE_EHT_CAP)
	 */
	if (IE_SIZE(prEhtCap) < (sizeof(struct IE_EHT_CAP))) {
		DBGLOG(SCN, WARN,
			"EHT_CAP IE_LEN err(%d)!\n", IE_LEN(prEhtCap));
		return;
	}

	memcpy(prStaRec->ucEhtMacCapInfo, prEhtCap->ucEhtMacCap,
		EHT_MAC_CAP_BYTE_NUM);
	memcpy(prStaRec->ucEhtPhyCapInfo, prEhtCap->ucEhtPhyCap,
		EHT_PHY_CAP_BYTE_NUM);
	ehtRlmRecMcsMap(prAdapter, prStaRec, prEhtCap);
}

void ehtRlmRecOperation(
	struct ADAPTER *prAdapter,
	struct BSS_INFO *prBssInfo,
	uint8_t *pucIE)
{
	struct IE_EHT_OP *prEhtOp = (struct IE_EHT_OP *) pucIE;

	/* if payload not contain any aucVarInfo,
	 * IE size = sizeof(struct IE_EHT_OP)
	 */
	if (IE_SIZE(prEhtOp) < (sizeof(struct IE_EHT_OP))) {
		DBGLOG(SCN, WARN,
			"HE_OP IE_LEN err(%d)!\n", IE_LEN(prEhtOp));
		return;
	}

	// TODO: The format of EHT operation Information subfield is missing in spec D1.1
	prBssInfo->ucVhtChannelWidth =
		ehtRlmGetVhtOpBwByEhtOpBw(prEhtOp->ucEhtOpParams[0]);
	DBGLOG(RLM, INFO, "EHT channel width: %d\n",
		prBssInfo->ucVhtChannelWidth);

	memcpy(prBssInfo->ucEhtOpParams, prEhtOp->ucEhtOpParams,
		HE_OP_BYTE_NUM);
}
void ehtRlmInit(
	struct ADAPTER *prAdapter)
{
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief
 *
 * \param[in]
 *
 * \return ucVhtOpBw 0:20M/40Hz, 1:80MHz, 2:160MHz, 4:320MHz
 * Note: 80+80MHz is not used for EHT op BW
 *
 */
/*----------------------------------------------------------------------------*/
uint8_t ehtRlmGetVhtOpBwByEhtOpBw(uint8_t ucBssOpBw)
{
	uint8_t ucVhtOpBw =
		VHT_OP_CHANNEL_WIDTH_80; /*VHT default should support BW 80*/

	switch (ucBssOpBw) {
	case EHT_MAX_BW_20:
	case EHT_MAX_BW_40:
		ucVhtOpBw = VHT_OP_CHANNEL_WIDTH_20_40;
		break;

	case EHT_MAX_BW_80:
		ucVhtOpBw = VHT_OP_CHANNEL_WIDTH_80;
		break;

	case EHT_MAX_BW_160:
		ucVhtOpBw = VHT_OP_CHANNEL_WIDTH_160;
		break;

	case EHT_MAX_BW_320:
		ucVhtOpBw = VHT_OP_CHANNEL_WIDTH_320;
		break;

	default:
		DBGLOG(RLM, WARN, "%s: unexpected Bss OP BW: %d\n", __func__,
		       ucBssOpBw);
		break;
	}

	return ucVhtOpBw;
}

void ehtRlmInitHtcACtrlOM(struct ADAPTER *prAdapter)
{
	prAdapter->fgEhtHtcOM = TRUE;
	prAdapter->u4HeHtcOM = 0;
	EHT_SET_HTC_HE_VARIANT(prAdapter->u4HeHtcOM);
	EHT_SET_HTC_1ST_A_CTRL_ID(prAdapter->u4HeHtcOM, HTC_EHT_A_CTRL_OM);
	EHT_SET_HTC_EHT_OM_RX_NSS_EXT(prAdapter->u4HeHtcOM, 1);
	EHT_SET_HTC_EHT_OM_TX_NSTS_EXT(prAdapter->u4HeHtcOM, 1);
	EHT_SET_HTC_EHT_OM_CH_WIDTH_EXT(prAdapter->u4HeHtcOM, CH_BW_80);
	EHT_SET_HTC_2ND_A_CTRL_ID(prAdapter->u4HeHtcOM, HTC_HE_A_CTRL_OM);
	EHT_SET_HTC_HE_OM_RX_NSS(prAdapter->u4HeHtcOM, 1);
	EHT_SET_HTC_HE_OM_TX_NSTS(prAdapter->u4HeHtcOM, 1);
	EHT_SET_HTC_HE_OM_CH_WIDTH(prAdapter->u4HeHtcOM, CH_BW_80);
	EHT_SET_HTC_HE_OM_UL_MU_DISABLE(prAdapter->u4HeHtcOM, 0);
	EHT_SET_HTC_HE_OM_UL_MU_DATA_DISABLE(prAdapter->u4HeHtcOM, 0);
}

#endif /* CFG_SUPPORT_802_11BE == 1 */
