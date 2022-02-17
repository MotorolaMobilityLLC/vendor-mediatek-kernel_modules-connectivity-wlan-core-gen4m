/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

/*! \file   "mlo.c"
 *  \brief
 */

#include "precomp.h"
#if (CFG_SUPPORT_802_11BE == 1)

struct APPEND_VAR_IE_ENTRY txMloAssocReqIETable[] = {
		{0, assocCalculateConnIELen, assocGenerateConnIE}
		, /* supplicant connect IE including rsn */
#if CFG_SUPPORT_SPEC_MGMT || CFG_SUPPORT_802_11K
		{(ELEM_HDR_LEN + ELEM_MAX_LEN_POWER_CAP),
		 NULL, rlmReqGeneratePowerCapIE}
		,			/* 33 */
#endif
#if CFG_SUPPORT_SPEC_MGMT
		{(ELEM_HDR_LEN + ELEM_MAX_LEN_SUPPORTED_CHANNELS),
		 NULL, rlmReqGenerateSupportedChIE}
		,			/* 36 */
#endif
		{(ELEM_HDR_LEN + ELEM_MAX_LEN_HT_CAP), NULL, rlmReqGenerateHtCapIE}
		,			/* 45 */
#if CFG_SUPPORT_802_11R
		{(ELEM_HDR_LEN + 1), NULL, assocGenerateMDIE}, /* Element ID: 54 */
		{0, rsnCalculateFTIELen, rsnGenerateFTIE}, /* Element ID: 55 */
#endif
#if CFG_SUPPORT_802_11K
		{(ELEM_HDR_LEN + 5), NULL,
		 rrmGenerateRRMEnabledCapIE}, /* Element ID: 70 */
#endif
		{(ELEM_HDR_LEN + ELEM_MAX_LEN_EXT_CAP), NULL, rlmReqGenerateExtCapIE}
		,			/* 127 */
		{(ELEM_HDR_LEN + ELEM_MAX_LEN_WMM_INFO), NULL, mqmGenerateWmmInfoIE}
		,			/* 221 */
		{(ELEM_HDR_LEN + ELEM_MAX_LEN_RSN + 4), NULL, rsnGenerateRSNIE}
		,			/* 48 */
#if CFG_SUPPORT_802_11AC
		{(ELEM_HDR_LEN + ELEM_MAX_LEN_VHT_CAP), NULL, rlmReqGenerateVhtCapIE}
		,			/*191 */
		{(ELEM_HDR_LEN + ELEM_MAX_LEN_VHT_OP_MODE_NOTIFICATION), NULL,
			rlmReqGenerateVhtOpNotificationIE}
		,			/*199 */
#endif
#if (CFG_SUPPORT_802_11AX == 1)
		{(0), heRlmCalculateHeCapIELen, heRlmReqGenerateHeCapIE}
		,			/* 255, EXT 35 */
#endif
#if CFG_SUPPORT_MTK_SYNERGY
		{(ELEM_HDR_LEN + ELEM_MIN_LEN_MTK_OUI), NULL, rlmGenerateMTKOuiIE}
					/* 221 */
		,
#endif
};

void beReqGenerateMLIE(
	struct ADAPTER *prAdapter,
	struct MSDU_INFO *prMsduInfo)
{
	ASSERT(prAdapter);
	ASSERT(prMsduInfo);

	//TODO: if ucPhyTypeSet support 802.11BE, then...
	beReqGenerateMultiLinkCommonInfo(prAdapter, prMsduInfo);
	beReqGenerateMultiLinkSTAInfo(prAdapter, prMsduInfo);
}


void beReqGenerateMultiLinkCommonInfo(
	struct ADAPTER *prAdapter,
	struct MSDU_INFO *prMsduInfo)
{
	uint8_t *cp;
	uint32_t u4OverallLen;
	//struct STA_RECORD *prStaRec;
	struct _MULTI_LINK_CONTROL_IE *prMultiLinkControlIE;

	//TODO: get correct ML steRec
	//prStaRec = cnmGetStaRecByIndex(prAdapter, prMsduInfo->ucStaRecIndex);

	prMultiLinkControlIE = (struct _MULTI_LINK_CONTROL_IE *)
		(((uint8_t *)prMsduInfo->prPacket) + prMsduInfo->u2FrameLength);

	prMultiLinkControlIE->ucId = ELEM_ID_RESERVED;
	prMultiLinkControlIE->ucExtId = ELEM_EXT_ID_MLD;

	/* filling control field */
	BE_SET_ML_CTRL_TYPE(prMultiLinkControlIE->ctrl, ML_ELEMENT_TYPE_BASIC);
	BE_SET_ML_CTRL_PRESENCE_MLD_MAC(prMultiLinkControlIE->ctrl);
	BE_SET_ML_CTRL_PRESENCE_MLD_CAP(prMultiLinkControlIE->ctrl);

	/* filling common info field*/
	cp = (uint8_t *) &MLCIE(prMultiLinkControlIE)->commonInfo[0];
	u4OverallLen = OFFSET_OF(struct _MULTI_LINK_CONTROL_IE, commonInfo[0]);

	if (BE_IS_ML_CTRL_PRESENCE_MLD_MAC(prMultiLinkControlIE->ctrl)) {
		//TODO: fill in MLD MAC
		kalMemCopy(cp, MLD_MAC_ADDR, MAC_ADDR_LEN);
		u4OverallLen += 6;
		cp += 6;
	}

	if (BE_IS_ML_CTRL_PRESENCE_MLD_CAP(prMultiLinkControlIE->ctrl)) {
		//TODO: fill in MLD cap
		WLAN_SET_FIELD_16(cp, MLD_CAP);
		u4OverallLen += 2;
		cp += 2;
	}

	DBGLOG(RLM, INFO, "u4OverallLen total: %d\n", u4OverallLen);
	prMultiLinkControlIE->ucLength = u4OverallLen - ELEM_HDR_LEN;

	DBGLOG_MEM8(RLM, INFO, cp - u4OverallLen, u4OverallLen);

	prMsduInfo->u2FrameLength += IE_SIZE(prMultiLinkControlIE);
}

void beReqGenerateMultiLinkSTAInfo(
	struct ADAPTER *prAdapter,
	struct MSDU_INFO *prMsduInfo)
{
	uint8_t *cp;
	uint32_t u4OverallLen;
	struct _MULTI_LINK_INFO_STA_CONTROL_IE *prMultiLinkSTAControlIE;
	uint32_t last_frame_index;
	uint32_t last_frame_len;
	const uint8_t *primary_IE;
	//uint8_t *pucBuffer;
	struct IE_HDR *last_frame_IE;
	uint32_t primary_IE_start;
	uint32_t i, t;
	uint16_t txMloAssocReqIENums;
	struct MSDU_INFO *prMsduInfoSta;
	uint16_t u2EstimatedFrameLen;
	uint8_t primaryIEExist[ELEM_ID_MAX_NUM] = {FALSE};
	uint8_t noninheritanceIE[ELEM_ID_MAX_NUM] = {FALSE};

	//TODO: loop multiple ML STAs including primary link
	//for (primary STA, STA1, STA2, ...) {

	prMultiLinkSTAControlIE = (struct _MULTI_LINK_INFO_STA_CONTROL_IE *)
		(((uint8_t *)prMsduInfo->prPacket) + prMsduInfo->u2FrameLength);

	prMultiLinkSTAControlIE->ucSubID = SUB_IE_MLD_PER_STA_PROFILE;
	/* filling STA control field */
	//TODO: fill in correct ML STA link ID
	BE_SET_ML_STA_CTRL_LINK_ID(prMultiLinkSTAControlIE->staCtrl, STA1_LINK_ID);
	BE_SET_ML_STA_CTRL_COMPLETE_PROFILE(prMultiLinkSTAControlIE->staCtrl);
	BE_SET_ML_STA_CTRL_PRESENCE_MAC(prMultiLinkSTAControlIE->staCtrl);

	/* filling STA info field */
	u4OverallLen = OFFSET_OF(struct _MULTI_LINK_INFO_STA_CONTROL_IE, staInfo[0]);
	cp = (uint8_t *) &MLSTAIE(prMultiLinkSTAControlIE)->staInfo[0];

	if (BE_IS_ML_STA_CTRL_PRESENCE_MAC(prMultiLinkSTAControlIE->staCtrl)) {
		//TODO: fill in correct ML STA MAC
		kalMemCopy(cp, MLD_STA1_MAC_ADDR, MAC_ADDR_LEN);
		u4OverallLen += 6;
		cp += 6;
	}

	DBGLOG(RLM, INFO, "u4OverallLen total: %d\n", u4OverallLen);
	prMultiLinkSTAControlIE->ucLength = u4OverallLen - ELEM_HDR_LEN;

	DBGLOG_MEM8(RLM, INFO, cp - u4OverallLen, u4OverallLen);

	prMsduInfo->u2FrameLength += IE_SIZE(prMultiLinkSTAControlIE);
	//} end of loop

	//TODO: loop multiple ML STAs except primary link
	//for (STA1, STA2, ...) {

	//handle noninheritance part: first, find all IE list in prmiary link's prMsduInfo
	for (i = 0; i < ELEM_ID_MAX_NUM; i++) {
		primaryIEExist[i] = FALSE;
		noninheritanceIE[i] = FALSE;

		primary_IE_start = OFFSET_OF(struct WLAN_ASSOC_REQ_FRAME, aucInfoElem[0]);
		primary_IE = kalFindIeMatchMask(i,
					(uint8_t *)prMsduInfo->prPacket + primary_IE_start,
					(prMsduInfo->u2FrameLength - primary_IE_start),
					NULL, 0, 0, NULL);
		if (primary_IE) {
			DBGLOG(RLM, INFO, "find eid in primary IE: %d\n", i);
			primaryIEExist[i] = TRUE;
			noninheritanceIE[i] = TRUE;
		}
	}

	/* STA profile for STA1, STA2, ... */
	//loop each IE of primary link and genrate IE of 2nd link...
	txMloAssocReqIENums = sizeof(txMloAssocReqIETable) /
			sizeof(struct APPEND_VAR_IE_ENTRY);

	/* Allocate a MSDU_INFO_T */
	u2EstimatedFrameLen = MAX_LEN_OF_MLIE;
	prMsduInfoSta = cnmMgtPktAlloc(prAdapter, u2EstimatedFrameLen);
	if (prMsduInfoSta == NULL) {
		DBGLOG(SAA, WARN,
		       "No PKT_INFO_T for sending (Re)Assoc Request.\n");
	}
	/* 4 <3> Update information of MSDU_INFO_T */
	nicTxSetPktLifeTime(prMsduInfoSta, 100);
	nicTxSetPktRetryLimit(prMsduInfoSta, TX_DESC_TX_COUNT_NO_LIMIT);
	nicTxSetForceRts(prMsduInfoSta, TRUE);

	//TODO: fill in correct ML starec's ucBssIndex and ucIndex
	TX_SET_MMPDU(prAdapter,
		     prMsduInfoSta,
		     1,  //prStaRec->ucBssIndex,
		     0, //prStaRec->ucIndex,
		     0, //no header
		     0, //no payload
		     saaFsmRunEventTxDone, MSDU_RATE_MODE_AUTO);

	last_frame_index = 0;
	for (t = 0; t < txMloAssocReqIENums; t++) {
		DBGLOG(RLM, INFO, "last_frame_index:  %d\n", last_frame_index);
		if (txMloAssocReqIETable[t].pfnAppendIE)
			txMloAssocReqIETable[t].pfnAppendIE(prAdapter,
							 prMsduInfoSta);
		/* Append non-wfa vendor specific ies for AIS mode */
		assoc_build_nonwfa_vend_ie(prAdapter, prMsduInfoSta);

		last_frame_len = prMsduInfoSta->u2FrameLength - last_frame_index;
		cp += last_frame_len;

		if (last_frame_len == 0) { //non-inheritance case

		} else {
			last_frame_IE = (struct IE_HDR *)(cp - last_frame_len);
			DBGLOG(RLM, INFO, "last_frame_IE ucID: %d\n", last_frame_IE->ucId);

			primary_IE_start = OFFSET_OF(struct WLAN_ASSOC_REQ_FRAME, aucInfoElem[0]);
			primary_IE = kalFindIeMatchMask(last_frame_IE->ucId,
						(uint8_t *)prMsduInfo->prPacket + primary_IE_start,
						(prMsduInfo->u2FrameLength - primary_IE_start),
						NULL, 0, 0, NULL);
			if (primary_IE) {
				DBGLOG_MEM8(RLM, INFO, primary_IE, last_frame_IE->ucLength + ELEM_HDR_LEN);
				DBGLOG_MEM8(RLM, INFO, cp - last_frame_len, last_frame_len);
			}
			//compare last_frame_cp and primary_IE
			//if, the same, subtract prMsduInfo->u2FrameLength to remove this IE
			//else, keep this IE and do nothing
			if (primary_IE && kalMemCmp(cp - last_frame_len, primary_IE, last_frame_len) == 0){
				prMsduInfoSta->u2FrameLength -= last_frame_len;
				cp -= last_frame_len;
				DBGLOG(RLM, INFO, "same IE, frame length of MSDU:  %d\n", prMsduInfoSta->u2FrameLength);
				if (primaryIEExist[last_frame_IE->ucId])
					noninheritanceIE[last_frame_IE->ucId] = FALSE;
			} else {
				u4OverallLen += last_frame_len;
				DBGLOG(RLM, INFO, "different IE, frame length of MSDU: %d\n", prMsduInfoSta->u2FrameLength);
				//do nothing
				if (primaryIEExist[last_frame_IE->ucId])
					noninheritanceIE[last_frame_IE->ucId] = FALSE;
			}
		}
		last_frame_index = prMsduInfoSta->u2FrameLength;
	}

	//} end of loop

	DBGLOG(RLM, INFO, "u4OverallLen total: %d\n", u4OverallLen);
	DBGLOG_MEM8(RLM, INFO, cp - u4OverallLen, u4OverallLen);

}

#endif /* CFG_SUPPORT_802_11BE == 1 */
