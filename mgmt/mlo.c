/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

/*! \file   "mlo.c"
 *  \brief
 */

#include "precomp.h"
#if (CFG_SUPPORT_802_11BE == 1)

void beReqGenerateMLIE(
	struct ADAPTER *prAdapter,
	struct MSDU_INFO *prMsduInfo,
	u_int8_t type,
	struct APPEND_VAR_IE_ENTRY ieArrayTable[],
	uint16_t txIENums)
{
	struct IE_MULTI_LINK_CONTROL* mloCommonIe;
	ASSERT(prAdapter);
	ASSERT(prMsduInfo);

	mloCommonIe = beReqGenerateMultiLinkCommonInfo(prAdapter, prMsduInfo);
	beReqGenerateMultiLinkSTAInfo(prAdapter, prMsduInfo, mloCommonIe, type, ieArrayTable, txIENums);
}
struct IE_MULTI_LINK_CONTROL *beReqGenerateMultiLinkCommonInfo(
	struct ADAPTER *prAdapter,
	struct MSDU_INFO *prMsduInfo)
{
	uint8_t *cp;
	struct STA_RECORD *prStaRec;
	struct MLD_STA_RECORD *prMldStarec;
	struct BSS_INFO *prBssInfo;
	struct MLD_BSS_INFO *prMldBssInfo;
	struct IE_MULTI_LINK_CONTROL *prMultiLinkControlIE;

	prStaRec = cnmGetStaRecByIndex(prAdapter, prMsduInfo->ucStaRecIndex);
	prMldStarec = mldStarecGetByStarec(prAdapter, prStaRec);
	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);
	prMldBssInfo = mldBssGetByBss(prAdapter, prBssInfo);

	if (!prMldStarec || !prMldBssInfo)
		return 0;

	prMultiLinkControlIE = (struct IE_MULTI_LINK_CONTROL *)
		(((uint8_t *)prMsduInfo->prPacket) + prMsduInfo->u2FrameLength);

	prMultiLinkControlIE->ucId = ELEM_ID_RESERVED;
	prMultiLinkControlIE->ucExtId = ELEM_EXT_ID_MLD;

	/* filling control field */
	BE_SET_ML_CTRL_TYPE(prMultiLinkControlIE->u2Ctrl, ML_ELEMENT_TYPE_BASIC);
	BE_SET_ML_CTRL_PRESENCE_MLD_MAC(prMultiLinkControlIE->u2Ctrl);
	//BE_SET_ML_CTRL_PRESENCE_MLD_CAP(prMultiLinkControlIE->u2Ctrl);

	/* filling common info field*/
	cp = prMultiLinkControlIE->aucCommonInfo;

	if (BE_IS_ML_CTRL_PRESENCE_MLD_MAC(prMultiLinkControlIE->u2Ctrl)) {
		kalMemCopy(cp, prMldBssInfo->aucOwnMldAddr, MAC_ADDR_LEN);
		//u4OverallLen += 6;
		cp += 6;
	}

	if (BE_IS_ML_CTRL_PRESENCE_MLD_CAP(prMultiLinkControlIE->u2Ctrl)) {
		//TODO: fill in MLD cap
		//WLAN_SET_FIELD_16(cp, MLD_CAP);
		//u4OverallLen += 2;
		//cp += 2;
	}

	//DBGLOG(RLM, INFO, "u4OverallLen total: %d\n", u4OverallLen);
	prMultiLinkControlIE->ucLength =
		(cp - prMultiLinkControlIE->aucCommonInfo)
		+ OFFSET_OF(struct IE_MULTI_LINK_CONTROL, aucCommonInfo[0])
		- ELEM_HDR_LEN;

	prMsduInfo->u2FrameLength += IE_SIZE(prMultiLinkControlIE);

	DBGLOG(RLM, INFO, "Dump ML common IE\n");
	DBGLOG_MEM8(RLM, INFO, prMultiLinkControlIE, IE_SIZE(prMultiLinkControlIE));

	return prMultiLinkControlIE;
}

void beReqGenerateMultiLinkSTAInfo(
	struct ADAPTER *prAdapter,
	struct MSDU_INFO *prMsduInfo,
	struct IE_MULTI_LINK_CONTROL *prMultiLinkControlIE,
	u_int8_t type,
	struct APPEND_VAR_IE_ENTRY ieArrayTable[],
	uint16_t txIENums)
{
	struct IE_MULTI_LINK_INFO_STA_CONTROL *prMultiLinkSTAControlIE;
	const uint8_t *primary_IE;
	uint32_t primary_IE_start;
	uint32_t primary_IE_end;
	uint32_t i, t;
	struct MSDU_INFO *prMsduInfoSta;
	uint16_t u2EstimatedFrameLen;
	uint8_t primaryIEExist[ELEM_ID_MAX_NUM] = {FALSE};
	uint8_t noninheritanceIE[ELEM_ID_MAX_NUM] = {FALSE};
	struct STA_RECORD *prStaRec;
	struct MLD_STA_RECORD *prMldStarec;
	struct STA_RECORD *prCurrStarec;
	struct LINK *prStarecList;
	uint8_t *cp, *staCP, *pucBuffer;

	prStaRec = cnmGetStaRecByIndex(prAdapter, prMsduInfo->ucStaRecIndex);
	prMldStarec = mldStarecGetByStarec(prAdapter, prStaRec);
	prStarecList = &prMldStarec->rStarecList;
	primary_IE_end = prMsduInfo->u2FrameLength - IE_SIZE(prMultiLinkControlIE);

	if (!prMultiLinkControlIE || !prMldStarec)
		return;

	//handle noninheritance part: first, find all IE list in prmiary link's prMsduInfo
	if (type == TYPE_ASSOC) {
		primary_IE_start = OFFSET_OF(struct WLAN_ASSOC_REQ_FRAME, aucInfoElem[0]);
	} else if (type == TYPE_AUTH) {
		primary_IE_start = OFFSET_OF(struct WLAN_AUTH_FRAME, aucInfoElem[0]);
	}
	//TODO: check multiple ELEM_ID_RESERVED IEs, which include different EXT ID
	DBGLOG(RLM, INFO, "find eid in primary IE:\n");
	for (i = 0; i < ELEM_ID_MAX_NUM; i++) {
		primaryIEExist[i] = FALSE;
		noninheritanceIE[i] = FALSE;
		primary_IE = kalFindIeMatchMask(i,
					(uint8_t *)prMsduInfo->prPacket + primary_IE_start,
					(primary_IE_end - primary_IE_start),
					NULL, 0, 0, NULL);
		if (primary_IE) {
			DBGLOG(RLM, INFO, "%d\n", i);
			primaryIEExist[i] = TRUE;
			noninheritanceIE[i] = TRUE;
		}
	}

	LINK_FOR_EACH_ENTRY(prCurrStarec, prStarecList, rLinkEntryMld,
		struct STA_RECORD) {
		struct BSS_INFO *bss = GET_BSS_INFO_BY_INDEX(prAdapter,
				prCurrStarec->ucBssIndex);
			
		DBGLOG(INIT, INFO, "\tsta: %d, wlan_idx: %d, bss_idx: %d, mac: " MACSTR "\n",
			prCurrStarec->ucIndex,
			prCurrStarec->ucWlanIndex,
			prCurrStarec->ucBssIndex,
			bss->aucOwnMacAddr);

		/* Allocate a MSDU_INFO_T */
		u2EstimatedFrameLen = MAX_LEN_OF_MLIE;
		prMsduInfoSta = cnmMgtPktAlloc(prAdapter, u2EstimatedFrameLen);
		if (prMsduInfoSta == NULL) {
			DBGLOG(SAA, WARN,
				   "No PKT_INFO_T for sending MLD STA.\n");
		}
		prMsduInfoSta->ucBssIndex = prCurrStarec->ucBssIndex;
		prMsduInfoSta->ucStaRecIndex = prCurrStarec->ucIndex;

		prMultiLinkSTAControlIE = (struct IE_MULTI_LINK_INFO_STA_CONTROL *)
			(((uint8_t *)prMsduInfoSta->prPacket) + prMsduInfoSta->u2FrameLength);
		prMultiLinkSTAControlIE->ucSubID = SUB_IE_MLD_PER_STA_PROFILE;

		/* filling STA control field (fixed length) */
		BE_SET_ML_STA_CTRL_LINK_ID(prMultiLinkSTAControlIE->u2StaCtrl, prCurrStarec->ucLinkIndex);
		BE_SET_ML_STA_CTRL_COMPLETE_PROFILE(prMultiLinkSTAControlIE->u2StaCtrl);
		BE_SET_ML_STA_CTRL_PRESENCE_MAC(prMultiLinkSTAControlIE->u2StaCtrl);

		/* filling STA info field (varied length) */
		cp = prMultiLinkSTAControlIE->aucStaInfo;

		if (BE_IS_ML_STA_CTRL_PRESENCE_MAC(prMultiLinkSTAControlIE->u2StaCtrl)) {
			kalMemCopy(cp, bss->aucOwnMacAddr, MAC_ADDR_LEN);
			cp += 6;
		}

		prMultiLinkSTAControlIE->ucLength =
			(cp - prMultiLinkSTAControlIE->aucStaInfo)
			+ OFFSET_OF(struct IE_MULTI_LINK_INFO_STA_CONTROL, aucStaInfo[0])
			- ELEM_HDR_LEN;
		prMsduInfoSta->u2FrameLength += IE_SIZE(prMultiLinkSTAControlIE);

		prMultiLinkControlIE->ucLength += IE_SIZE(prMultiLinkSTAControlIE);

		DBGLOG(RLM, INFO, "Dump ML STA%d IE control part\n", prCurrStarec->ucIndex);
		DBGLOG_MEM8(RLM, INFO, prMultiLinkSTAControlIE, IE_SIZE(prMultiLinkSTAControlIE));

		/* filling STA profile field */
		staCP = cp;

		//loop each IE of primary link and genrate IE of 2nd link...
		for (t = 0; t < txIENums; t++) {
			uint32_t last_frame_index;
			uint32_t last_frame_len;
			struct IE_HDR *last_frame_IE;
			last_frame_index = prMsduInfoSta->u2FrameLength;

			if (ieArrayTable[t].pfnAppendIE)
				ieArrayTable[t].pfnAppendIE(prAdapter,
					prMsduInfoSta);

			last_frame_len = prMsduInfoSta->u2FrameLength - last_frame_index;
			cp += last_frame_len;

			if (last_frame_len == 0) { //non-inheritance case
				DBGLOG(RLM, INFO, "frame %d no IE\n", t);
			} else {
				last_frame_IE = (struct IE_HDR *)(cp - last_frame_len);
				DBGLOG(RLM, INFO, "frame %d ucID: %d\n", t, last_frame_IE->ucId);

				primary_IE = kalFindIeMatchMask(last_frame_IE->ucId,
							(uint8_t *)prMsduInfo->prPacket + primary_IE_start,
							(primary_IE_end - primary_IE_start),
							NULL, 0, 0, NULL);
				if (primary_IE) {
					DBGLOG_MEM8(RLM, INFO, primary_IE, last_frame_IE->ucLength + ELEM_HDR_LEN);
					DBGLOG_MEM8(RLM, INFO, cp - last_frame_len, last_frame_len);
				}
				/*
				   compare last_frame_cp and primary_IE
				   if, the same, subtract prMsduInfo->u2FrameLength to remove this IE
				   else, keep this IE and do nothing
				*/
				if (primary_IE && kalMemCmp(cp - last_frame_len, primary_IE, last_frame_len) == 0){
					prMsduInfoSta->u2FrameLength -= last_frame_len;
					cp -= last_frame_len;
					DBGLOG(RLM, INFO, "same IE, frame length of MSDU:  %d\n", prMsduInfoSta->u2FrameLength);
					if (primaryIEExist[last_frame_IE->ucId])
						noninheritanceIE[last_frame_IE->ucId] = FALSE;
				} else {
					DBGLOG(RLM, INFO, "different IE or STA specific IE, frame length of MSDU: %d\n", prMsduInfoSta->u2FrameLength);
					//do nothing
					if (primaryIEExist[last_frame_IE->ucId])
						noninheritanceIE[last_frame_IE->ucId] = FALSE;
				}
			}
		}

		DBGLOG(RLM, INFO, "noninheritanceIE:\n");
		for (i=0; i < ELEM_ID_MAX_NUM; i++) {
			if (noninheritanceIE[i] == TRUE)
				DBGLOG(RLM, INFO, "%d\n", i);
		}

		prMultiLinkSTAControlIE->ucLength += (cp - staCP);
		prMultiLinkControlIE->ucLength += (cp - staCP);

		/* copy each STA's prMsduInfoSta into MLIE's prMsduInfo */
		pucBuffer = (uint8_t *) ((unsigned long)
				 prMsduInfo->prPacket + (unsigned long)
				 prMsduInfo->u2FrameLength);
		kalMemCopy(pucBuffer, prMultiLinkSTAControlIE, IE_SIZE(prMultiLinkSTAControlIE));
		prMsduInfo->u2FrameLength += IE_SIZE(prMultiLinkSTAControlIE);

		DBGLOG(RLM, INFO, "Dump ML STA%d IE\n", prCurrStarec->ucIndex);
		DBGLOG_MEM8(RLM, INFO, prMultiLinkSTAControlIE, IE_SIZE(prMultiLinkSTAControlIE));

		cnmMgtPktFree(prAdapter, prMsduInfoSta);

	} /* end of rLinkEntryMld */

	DBGLOG(RLM, INFO, "Dump total MLIE\n");
	DBGLOG_MEM8(RLM, INFO, prMultiLinkControlIE, IE_SIZE(prMultiLinkControlIE));

}

int8_t mldBssRegister(struct ADAPTER *prAdapter,
	struct MLD_BSS_INFO *prMldBssInfo,
	struct BSS_INFO *prBssInfo)
{
	struct LINK *prBssList = NULL;

	if (!prMldBssInfo)
		return -EINVAL;

	prBssList = &prMldBssInfo->rBssList;

	DBGLOG(INIT, INFO, "prMldBssInfo: %d, prBssInfo: %d\n",
		prMldBssInfo->ucGroupMldId, prBssInfo->ucBssIndex);

	if (prBssList->u4NumElem == 0) {
		DBGLOG(INIT, INFO, "prMldBssInfo: %d, macAddr: " MACSTR "\n",
			prMldBssInfo->ucGroupMldId,
			MAC2STR(prBssInfo->aucOwnMacAddr));
		COPY_MAC_ADDR(prMldBssInfo->aucOwnMldAddr,
			prBssInfo->aucOwnMacAddr);
	}

	prBssInfo->ucGroupMldId = prMldBssInfo->ucGroupMldId;
	LINK_INSERT_TAIL(prBssList, &prBssInfo->rLinkEntryMld);

	return 0;
}

void mldBssUnregister(struct ADAPTER *prAdapter,
	struct MLD_BSS_INFO *prMldBssInfo,
	struct BSS_INFO *prBss)
{
	struct BSS_INFO *prCurrBssInfo;
	struct LINK *prBssList = NULL;

	if (!prMldBssInfo)
		return;

	prBssList = &prMldBssInfo->rBssList;

	DBGLOG(INIT, INFO, "prMldBssInfo: %d, prBss: %d\n",
		prMldBssInfo->ucGroupMldId, prBss->ucBssIndex);

	LINK_FOR_EACH_ENTRY(prCurrBssInfo, prBssList,
			rLinkEntryMld,
			struct BSS_INFO) {
		if (!prCurrBssInfo)
			break;

		if (prBss != prCurrBssInfo)
			continue;

		LINK_REMOVE_KNOWN_ENTRY(prBssList,
			&prCurrBssInfo->rLinkEntryMld);
	}
}

int8_t mldBssAlloc(struct ADAPTER *prAdapter,
	struct MLD_BSS_INFO **pprMldBssInfo)
{
	struct MLD_BSS_INFO *prMldBssInfo = NULL;
	uint8_t i = 0;

	for (i = 0; i < ARRAY_SIZE(prAdapter->aprMldBssInfo); i++) {
		prMldBssInfo = &prAdapter->aprMldBssInfo[i];

		if (prMldBssInfo->fgIsInUse)
			continue;

		LINK_INITIALIZE(&prMldBssInfo->rBssList);
		prMldBssInfo->fgIsInUse = TRUE;
		prMldBssInfo->ucGroupMldId = i;
		prMldBssInfo->ucOmRemapIdx = i;
		prMldBssInfo->ucOmacIdx = INVALID_OMAC_IDX;

		*pprMldBssInfo = prMldBssInfo;
		break;
	}
	if (prMldBssInfo)
		DBGLOG(INIT, INFO, "ucGroupMldId: %d, ucOmRemapIdx: %d\n",
			prMldBssInfo->ucGroupMldId,
			prMldBssInfo->ucOmRemapIdx);
	return 0;
}

void mldBssFree(struct ADAPTER *prAdapter,
	struct MLD_BSS_INFO *prMldBssInfo)
{
	struct BSS_INFO *prCurrBssInfo;
	struct LINK *prBssList = NULL;

	if (!prMldBssInfo)
		return;

	prBssList = &prMldBssInfo->rBssList;

	DBGLOG(INIT, INFO, "ucGroupMldId: %d\n",
		prMldBssInfo->ucGroupMldId);

	LINK_FOR_EACH_ENTRY(prCurrBssInfo, prBssList, rLinkEntryMld,
			struct BSS_INFO) {
		if (!prCurrBssInfo)
			break;

		LINK_REMOVE_KNOWN_ENTRY(prBssList,
			&prCurrBssInfo->rLinkEntryMld);
	}
	prMldBssInfo->fgIsInUse = FALSE;
	prMldBssInfo->ucOmacIdx = INVALID_OMAC_IDX;
}

struct MLD_BSS_INFO *mldBssGetByBss(struct ADAPTER *prAdapter,
	struct BSS_INFO *prBssInfo)
{
	struct MLD_BSS_INFO *prMldBssInfo = NULL;

	if (prBssInfo->ucGroupMldId == MLD_GROUP_NONE)
		return NULL;

	prMldBssInfo = &prAdapter->aprMldBssInfo[prBssInfo->ucGroupMldId];

	if (prMldBssInfo->fgIsInUse)
		return prMldBssInfo;
	else
		return NULL;
}

struct MLD_BSS_INFO *mldBssGetByIdx(struct ADAPTER *prAdapter,
	uint8_t ucIdx)
{
	struct MLD_BSS_INFO *prMldBssInfo = NULL;

	if (ucIdx == MLD_GROUP_NONE)
		return NULL;

	prMldBssInfo = &prAdapter->aprMldBssInfo[ucIdx];

	if (prMldBssInfo->fgIsInUse)
		return prMldBssInfo;
	else
		return NULL;
}

int8_t mldBssInit(struct ADAPTER *prAdapter)
{
	DBGLOG(INIT, INFO, "\n");
	kalMemZero(prAdapter->aprMldBssInfo,
		sizeof(prAdapter->aprMldBssInfo));
	return 0;
}

void mldBssUninit(struct ADAPTER *prAdapter)
{
	DBGLOG(INIT, INFO, "\n");
}

void mldStarecDump(struct ADAPTER *prAdapter)
{
	uint8_t i = 0;

	for (i = 0; i < ARRAY_SIZE(prAdapter->aprMldStarec); i++) {
		struct MLD_STA_RECORD *prMldStarec = &prAdapter->aprMldStarec[i];
		struct STA_RECORD *prCurrStarec;
		struct LINK *prStarecList;

		if (!prMldStarec->fgIsInUse)
			continue;

		DBGLOG(INIT, INFO, "idx: %d, pri_mld: %d, sec_mld: %d, setup: %d, emlmr: %d, emlsr: %d, str: [0x%x,0x%x,0x%x], mac: " MACSTR "\n",
			prMldStarec->ucIdx,
			prMldStarec->u2PrimaryMldId,
			prMldStarec->u2SecondMldId,
			prMldStarec->u2SetupWlanId,
			prMldStarec->ucEmlmrBitmap,
			prMldStarec->ucEmlsrBitmap,
			prMldStarec->aucStrBitmap[0],
			prMldStarec->aucStrBitmap[1],
			prMldStarec->aucStrBitmap[2],
			prMldStarec->aucPeerMldAddr);

		prStarecList = &prMldStarec->rStarecList;
		LINK_FOR_EACH_ENTRY(prCurrStarec, prStarecList, rLinkEntryMld,
		    struct STA_RECORD) {
			DBGLOG(INIT, INFO, "\tsta: %d, wlan_idx: %d, bss_idx: %d, mac: " MACSTR "\n",
				prCurrStarec->ucIndex,
				prCurrStarec->ucWlanIndex,
				prCurrStarec->ucBssIndex,
				prCurrStarec->aucMacAddr);
		}
	}
}

static struct MLD_STA_RECORD *mldStarecGetByAddr(struct ADAPTER *prAdapter,
	uint8_t aucMacAddr[])
{
	const uint8_t aucZeroMacAddr[] = NULL_MAC_ADDR;
	uint8_t i = 0;

	if (EQUAL_MAC_ADDR(aucZeroMacAddr, aucMacAddr))
		return NULL;

	for (i = 0; i < ARRAY_SIZE(prAdapter->aprMldStarec); i++) {
		struct MLD_STA_RECORD *prMldStarec =
			&prAdapter->aprMldStarec[i];

		if (prMldStarec->fgIsInUse &&
				EQUAL_MAC_ADDR(prMldStarec->aucPeerMldAddr,
				aucMacAddr))
			return prMldStarec;
	}

	return NULL;
}

int8_t mldStarecRegister(struct ADAPTER *prAdapter,
	struct STA_RECORD *prStarec)
{
	int8_t rStatus = 0;
	struct MLD_STA_RECORD *prMldStarec = NULL;
	struct LINK *prStarecList = NULL;
	const uint8_t aucZeroMacAddr[] = NULL_MAC_ADDR;

	if (EQUAL_MAC_ADDR(aucZeroMacAddr, prStarec->aucMldAddr))
		return -EINVAL;

	prMldStarec = mldStarecGetByAddr(prAdapter, prStarec->aucMldAddr);
	if (!prMldStarec) {
		rStatus = mldStarecAlloc(prAdapter, &prMldStarec, prStarec->aucMldAddr);
		if (rStatus)
			goto exit;
	}

	prStarecList = &prMldStarec->rStarecList;

	if (prStarecList->u4NumElem == 0)
		prMldStarec->u2PrimaryMldId = prStarec->ucWlanIndex;
	else if (prStarecList->u4NumElem == 1)
		prMldStarec->u2SecondMldId = prStarec->ucWlanIndex;

	prStarec->ucMldStaIndex = prMldStarec->ucIdx;
	LINK_INSERT_TAIL(prStarecList, &prStarec->rLinkEntryMld);

	DBGLOG(INIT, INFO, "prMldStarec: %d, prStarec: %d, pri_mld: %d, sec_mld: %d, mld_mac: " MACSTR "\n",
		prMldStarec->ucIdx,
		prStarec->ucIndex,
		prMldStarec->u2PrimaryMldId,
		prMldStarec->u2SecondMldId,
		MAC2STR(prMldStarec->aucPeerMldAddr));

exit:
	return rStatus;
}

void mldStarecUnregister(struct ADAPTER *prAdapter,
	struct STA_RECORD *prStarec)
{
	struct MLD_STA_RECORD *prMldStarec;
	struct LINK *prStarecList;
	struct STA_RECORD *prCurrStarec;

	if (!prStarec || prStarec->ucMldStaIndex == MLD_GROUP_NONE)
		return;

	prMldStarec = mldStarecGetByStarec(prAdapter,
		prStarec);

	if (!prMldStarec)
		return;

	nicUniCmdMldStaTeardown(prAdapter, prStarec);

	prStarecList = &prMldStarec->rStarecList;

	DBGLOG(INIT, INFO, "prMldStarec: %d, prStarec: %d\n",
		prMldStarec->ucIdx, prStarec->ucIndex);

	LINK_FOR_EACH_ENTRY(prCurrStarec, prStarecList, rLinkEntryMld,
	    struct STA_RECORD) {
		if (prStarec != prCurrStarec)
			continue;

		prCurrStarec->ucMldStaIndex = MLD_GROUP_NONE;
		LINK_REMOVE_KNOWN_ENTRY(prStarecList, &prCurrStarec->rLinkEntryMld);
		break;
	}

	if (prStarecList->u4NumElem == 0)
		mldStarecFree(prAdapter, prMldStarec);
}

int8_t mldStarecAlloc(struct ADAPTER *prAdapter,
	struct MLD_STA_RECORD **pprMldStarec,
	uint8_t *aucMacAddr)
{
	struct MLD_STA_RECORD *prMldStarec = NULL;
	uint8_t i = 0;

	for (i = 0; i < ARRAY_SIZE(prAdapter->aprMldStarec); i++) {
		prMldStarec = &prAdapter->aprMldStarec[i];

		if (prMldStarec->fgIsInUse)
			continue;

		LINK_INITIALIZE(&prMldStarec->rStarecList);
		prMldStarec->fgIsInUse = TRUE;
		prMldStarec->ucIdx = i;

		/* TODO */
		prMldStarec->fgNSEP = FALSE;
		prMldStarec->ucEmlmrBitmap = 0;
		prMldStarec->ucEmlsrBitmap = 0;
#ifdef BELLWETHER
		prMldStarec->aucStrBitmap[0] = BIT(2);
		prMldStarec->aucStrBitmap[1] = BIT(1);
		prMldStarec->aucStrBitmap[2] = BIT(0);
#else
		prMldStarec->aucStrBitmap[0] = BIT(1);
		prMldStarec->aucStrBitmap[1] = BIT(0);
		prMldStarec->aucStrBitmap[2] = BIT(2);
#endif

		COPY_MAC_ADDR(prMldStarec->aucPeerMldAddr, aucMacAddr);

		*pprMldStarec = prMldStarec;
		break;
	}

	if (prMldStarec)
		DBGLOG(INIT, INFO, "ucIdx: %d, aucMacAddr: " MACSTR ", str[0x%x,0x%x,0x%x]\n",
			prMldStarec->ucIdx,
			MAC2STR(prMldStarec->aucPeerMldAddr),
			prMldStarec->aucStrBitmap[0],
			prMldStarec->aucStrBitmap[1],
			prMldStarec->aucStrBitmap[2]);

	return 0;
}

void mldStarecFree(struct ADAPTER *prAdapter,
	struct MLD_STA_RECORD *prMldStarec)
{
	DBGLOG(INIT, INFO, "prMldStarec: %d\n",
			prMldStarec->ucIdx);

	kalMemZero(prMldStarec, sizeof(struct MLD_STA_RECORD));
	prMldStarec->fgIsInUse = FALSE;
}

struct MLD_STA_RECORD *mldStarecGetByStarec(struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec)
{
	struct MLD_STA_RECORD *prMldStarec;

	if (!prAdapter || !prStaRec)
		return NULL;

	if (prStaRec->ucMldStaIndex == MLD_GROUP_NONE ||
			prStaRec->ucMldStaIndex >
			ARRAY_SIZE(prAdapter->aprMldStarec))
		return NULL;

	prMldStarec = &prAdapter->aprMldStarec[prStaRec->ucMldStaIndex];
	if (!prMldStarec->fgIsInUse)
		return NULL;

	return prMldStarec;
}

int8_t mldStarecSetSetupIdx(struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec)
{
	struct MLD_STA_RECORD *prMldStarec = mldStarecGetByStarec(prAdapter,
		prStaRec);

	if (!prAdapter || !prMldStarec)
		return -EINVAL;

	DBGLOG(INIT, INFO, "prMldStarec: %d, ucIdx: %d\n",
			prMldStarec->ucIdx, prStaRec->ucWlanIndex);

	prMldStarec->u2SetupWlanId = prStaRec->ucWlanIndex;

	return 0;
}

int8_t mldStarecInit(struct ADAPTER *prAdapter)
{
	DBGLOG(INIT, INFO, "\n");
	kalMemZero(prAdapter->aprMldStarec, sizeof(prAdapter->aprMldStarec));
	return 0;
}

void mldStarecUninit(struct ADAPTER *prAdapter)
{
	DBGLOG(INIT, INFO, "\n");
}

#endif /* CFG_SUPPORT_802_11BE == 1 */
