/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

/*! \file   "mlo.c"
 *  \brief
 */

#include "precomp.h"
#if (CFG_SUPPORT_802_11BE_MLO == 1)

void beGenerateAuthMldIE(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	uint8_t ucBssIndex,
	struct SW_RFB *prRecvAuthSwRfb,
	struct MSDU_INFO *prMsduInfo,
	PFN_COMPOSE_AUTH_IE_FUNC pfnComposeIE)
{
	struct IE_MULTI_LINK_CONTROL *common = NULL;
	struct WLAN_AUTH_FRAME *auth;
	struct MSDU_INFO *msdu_sta;
	struct MLD_STA_RECORD *mld_starec;
	struct MLD_BSS_INFO *mld_bssinfo;
	struct LINK *links;
	struct STA_RECORD *starec;
	struct BSS_INFO *bss, *prBssInfo;
	uint32_t offset, len;

	ASSERT(prAdapter);
	ASSERT(prMsduInfo);

	prBssInfo= GET_BSS_INFO_BY_INDEX(prAdapter, ucBssIndex);
	mld_starec = mldStarecGetByStarec(prAdapter, prStaRec);
	mld_bssinfo = mldBssGetByBss(prAdapter, prBssInfo);
	offset = sortGetPayloadOffset(prAdapter, prMsduInfo);
	len = prMsduInfo->u2FrameLength;
	auth = (struct WLAN_AUTH_FRAME *) prMsduInfo->prPacket;

	if (!mld_starec && !mld_bssinfo)
		return;

	if (mld_starec && mld_starec->rStarecList.u4NumElem >= 1) {
		common = beGenerateMldCommonInfo(prAdapter, prMsduInfo);
		if (!common)
			return;

		links =  &mld_starec->rStarecList;
		LINK_FOR_EACH_ENTRY(starec, links, rLinkEntryMld,
			struct STA_RECORD) {
			bss = GET_BSS_INFO_BY_INDEX(prAdapter,
				starec->ucBssIndex);

			DBGLOG(INIT, INFO,
				"\tsta: %d, wlan_idx: %d, bss_idx: %d, mac: " MACSTR "\n",
				starec->ucIndex,
				starec->ucWlanIndex,
				starec->ucBssIndex,
				MAC2STR(bss->aucOwnMacAddr));

			if (starec == prStaRec) {
				beGenerateMldSTAInfo(prAdapter, common,
					prMsduInfo, offset, len,
					prMsduInfo, bss->ucBssIndex);
			} else {
				/* compose frame for each starec,
				 * dont care Header and some Fixed Fields
				 */
				msdu_sta = pfnComposeIE(prAdapter, starec,
					starec->ucBssIndex, prRecvAuthSwRfb,
					auth->u2AuthTransSeqNo,
					auth->u2StatusCode);
				if (msdu_sta == NULL) {
					DBGLOG(SAA, WARN,
						"No PKT_INFO_T for sending MLD STA.\n");
					continue;
				}

				beGenerateMldSTAInfo(prAdapter, common,
					prMsduInfo, offset, len,
					msdu_sta, bss->ucBssIndex);
				cnmMgtPktFree(prAdapter, msdu_sta);
			}
		}
	} else if (prRecvAuthSwRfb) {
		/* look for ml ie from recev auth frame */
		common = (struct IE_MULTI_LINK_CONTROL *)
		   kalFindIeExtIE(ELEM_ID_RESERVED, ELEM_EXT_ID_MLD,
		   prRecvAuthSwRfb->pvHeader + prRecvAuthSwRfb->u2HeaderLen,
		   prRecvAuthSwRfb->u2PacketLen - prRecvAuthSwRfb->u2HeaderLen);

		/* ml not found, skip it */
		if (!common)
			return;

		common = beGenerateMldCommonInfo(prAdapter, prMsduInfo);
		if (!common)
			return;

		links = &mld_bssinfo->rBssList;
		LINK_FOR_EACH_ENTRY(bss, links, rLinkEntryMld,
			struct BSS_INFO) {

			DBGLOG(INIT, INFO,
				"\tsta: bss_idx: %d, mac: " MACSTR "\n",
				bss->ucBssIndex,
				MAC2STR(bss->aucOwnMacAddr));

			if (bss == prBssInfo) {
				beGenerateMldSTAInfo(prAdapter, common,
					prMsduInfo, offset, len,
					prMsduInfo, bss->ucBssIndex);
			} else {
				/* compose frame for each starec,
				 * dont care Header and some Fixed Fields
				 */
				msdu_sta = pfnComposeIE(prAdapter, NULL,
					bss->ucBssIndex, prRecvAuthSwRfb,
					auth->u2AuthTransSeqNo,
					auth->u2StatusCode);
				if (msdu_sta == NULL) {
					DBGLOG(SAA, WARN,
						"No PKT_INFO_T for sending MLD STA.\n");
					continue;
				}

				beGenerateMldSTAInfo(prAdapter, common,
					prMsduInfo, offset, len,
					msdu_sta, bss->ucBssIndex);
				cnmMgtPktFree(prAdapter, msdu_sta);
			}
		}

	}

	if (common) {
		DBGLOG(SAA, INFO, "Dump total MLIE\n");
		DBGLOG_MEM8(SAA, INFO, common, IE_SIZE(common));
	}
}

void beGenerateAssocMldIE(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	struct MSDU_INFO *prMsduInfo,
	PFN_COMPOSE_ASSOC_IE_FUNC pfnComposeIE)
{
	struct IE_MULTI_LINK_CONTROL* common;
	struct STA_RECORD *starec;
	struct MSDU_INFO *msdu_sta;
	struct MLD_STA_RECORD *mld_starec;
	struct LINK *links;
	uint32_t offset, len;

	ASSERT(prAdapter);
	ASSERT(prMsduInfo);

	mld_starec = mldStarecGetByStarec(prAdapter, prStaRec);
	offset = sortGetPayloadOffset(prAdapter, prMsduInfo);
	len = prMsduInfo->u2FrameLength;

	if (!mld_starec)
		return;

	common = beGenerateMldCommonInfo(prAdapter, prMsduInfo);
	if (!common)
		return;

	links = &mld_starec->rStarecList;
	LINK_FOR_EACH_ENTRY(starec, links, rLinkEntryMld,
		struct STA_RECORD) {
		struct BSS_INFO *bss = GET_BSS_INFO_BY_INDEX(prAdapter,
				starec->ucBssIndex);

		DBGLOG(INIT, INFO,
			"\tsta: %d, wlan_idx: %d, bss_idx: %d, mac: " MACSTR "\n",
			starec->ucIndex,
			starec->ucWlanIndex,
			starec->ucBssIndex,
			MAC2STR(bss->aucOwnMacAddr));

		if (starec == prStaRec) {
			beGenerateMldSTAInfo(prAdapter, common,
				prMsduInfo, offset, len,
				prMsduInfo, bss->ucBssIndex);
		} else {
			/* compose frame for each starec */
			msdu_sta = pfnComposeIE(prAdapter, starec);
			if (msdu_sta  == NULL) {
				DBGLOG(SAA, WARN,
					"No PKT_INFO_T for sending MLD STA.\n");
				continue;
			}

			beGenerateMldSTAInfo(prAdapter, common,
				prMsduInfo, offset, len,
				msdu_sta, bss->ucBssIndex);
			cnmMgtPktFree(prAdapter, msdu_sta);
		}
	}

	DBGLOG(RLM, INFO, "Dump total MLIE\n");
	DBGLOG_MEM8(RLM, INFO, common, IE_SIZE(common));
}

void beGenerateBeaconMldIE(
	struct ADAPTER *prAdapter,
	uint8_t ucBssIndex,
	uint8_t ucComplete,
	struct MSDU_INFO *prMsduInfo,
	PFN_COMPOSE_BEACON_IE_FUNC pfnComposeIE)
{
	struct IE_MULTI_LINK_CONTROL* common;
	struct MSDU_INFO *msdu_sta;
	struct MLD_BSS_INFO *mld_bssinfo;
	struct LINK *links;
	struct BSS_INFO *bss;
	uint32_t offset, len;

	ASSERT(prAdapter);
	ASSERT(prMsduInfo);

	bss = GET_BSS_INFO_BY_INDEX(prAdapter, ucBssIndex);
	mld_bssinfo = mldBssGetByBss(prAdapter, bss);
	offset = sortGetPayloadOffset(prAdapter, prMsduInfo);
	len = prMsduInfo->u2FrameLength;

	if (!mld_bssinfo)
		return;

	common = beGenerateMldCommonInfo(prAdapter, prMsduInfo);
	if (!common)
		return;

	if (!ucComplete)
		goto DONE;

	links =  &mld_bssinfo->rBssList;
	LINK_FOR_EACH_ENTRY(bss, links, rLinkEntryMld, struct BSS_INFO) {

		DBGLOG(INIT, INFO,
			"\tsta: bss_idx: %d, mac: " MACSTR "\n",
			bss->ucBssIndex,
			MAC2STR(bss->aucOwnMacAddr));

		/* compose frame for each starec,
		 * dont care Header and some Fixed Fields
		 */
		msdu_sta = pfnComposeIE(prAdapter, bss->ucBssIndex);
		if (msdu_sta == NULL) {
			DBGLOG(SAA, WARN,
				"No PKT_INFO_T for sending MLD STA.\n");
			continue;
		}

		beGenerateMldSTAInfo(prAdapter, common,
			prMsduInfo, offset, len,
			msdu_sta, bss->ucBssIndex);
	}

DONE:
	DBGLOG(SAA, INFO, "Dump total MLIE\n");
	DBGLOG_MEM8(SAA, INFO, common, IE_SIZE(common));
}

struct IE_MULTI_LINK_CONTROL *beGenerateMldCommonInfo(
	struct ADAPTER *prAdapter,
	struct MSDU_INFO *prMsduInfo)
{
	uint8_t *cp;
	struct MLD_BSS_INFO *mld_bssinfo;
	struct BSS_INFO *bss;
	struct IE_MULTI_LINK_CONTROL *common;
	struct WLAN_MAC_MGMT_HEADER *mgmt;
	uint16_t frame_ctrl;
	uint16_t present;

	bss = GET_BSS_INFO_BY_INDEX(prAdapter, prMsduInfo->ucBssIndex);
	mld_bssinfo = mldBssGetByBss(prAdapter, bss);
	mgmt = (struct WLAN_MAC_MGMT_HEADER *)(prMsduInfo->prPacket);
	frame_ctrl = mgmt->u2FrameCtrl & MASK_FRAME_TYPE;

	if (!mld_bssinfo)
		return 0;

	common = (struct IE_MULTI_LINK_CONTROL *)
		(((uint8_t *)prMsduInfo->prPacket) + prMsduInfo->u2FrameLength);

	common->ucId = ELEM_ID_RESERVED;
	common->ucExtId = ELEM_EXT_ID_MLD;

	/* filling control field */
	BE_SET_ML_CTRL_TYPE(common->u2Ctrl, ML_ELEMENT_TYPE_BASIC);
	present = ML_CTRL_MLD_MAC_ADDR_PRESENT |
		  ML_CTRL_MLD_CAPA_PRESENT;

	if (frame_ctrl == MAC_FRAME_PROBE_RSP ||
	    frame_ctrl == MAC_FRAME_BEACON) {
		present |= (ML_CTRL_LINK_ID_INFO_PRESENT |
			    ML_CTRL_BSS_PARA_CHANGE_COUNT_PRESENT);
	}

	BE_SET_ML_CTRL_PRESENCE(common->u2Ctrl, present);

	/* filling common info field*/
	cp = common->aucCommonInfo;

	if (BE_IS_ML_CTRL_PRESENCE_MLD_MAC(common->u2Ctrl)) {
		COPY_MAC_ADDR(cp, mld_bssinfo->aucOwnMldAddr);
		cp += MAC_ADDR_LEN;
	}

	if (BE_IS_ML_CTRL_PRESENCE_LINK_ID(common->u2Ctrl))
		*cp++ = bss->ucLinkIndex;

	if (BE_IS_ML_CTRL_PRESENCE_BSS_PARA_CHANGE_COUNT(common->u2Ctrl))
		*cp++ = 0;

	if (BE_IS_ML_CTRL_PRESENCE_MLD_CAP(common->u2Ctrl)) {
		WLAN_SET_FIELD_16(cp, MLD_LINK_MAX);
		cp += 2;
	}

	common->ucLength = cp - (uint8_t *) common - ELEM_HDR_LEN;
	prMsduInfo->u2FrameLength += IE_SIZE(common);

	DBGLOG(RLM, INFO, "Dump ML common IE\n");
	DBGLOG_MEM8(RLM, INFO, common, IE_SIZE(common));

	return common;
}

uint8_t beMldNoNeedIE(uint8_t *pucBuf)
{
	/* assume vendor ie are the same amoung links */
	if (IE_ID(pucBuf) == ELEM_ID_VENDOR ||
	    IE_ID(pucBuf) == ELEM_ID_RNR ||
	    IE_ID(pucBuf) == ELEM_ID_NEIGHBOR_REPORT ||
	    IE_ID(pucBuf) == ELEM_ID_MBSSID ||
	    (IE_ID(pucBuf) == ELEM_ID_RESERVED &&
	     IE_ID_EXT(pucBuf) == ELEM_EXT_ID_MLD))
		return TRUE;

	return FALSE;
}

void beGenerateMldSTAInfo(
	struct ADAPTER *prAdapter,
	struct IE_MULTI_LINK_CONTROL *prMultiLinkControlIE,
	struct MSDU_INFO *prMsduInfo,
	uint32_t u4BeginOffset,
	uint32_t u4PrimaryLength,
	struct MSDU_INFO *prMsduInfoSta,
	uint8_t ucBssIndex)
{
	struct IE_MULTI_LINK_INFO_STA_CONTROL *sta_ctrl;
	struct IE_NON_INHERITANCE *non_inh;
	uint32_t i;
	struct STA_RECORD *starec;
	struct BSS_INFO *bss;
	struct WLAN_MAC_MGMT_HEADER *mgmt;
	uint8_t link;
	uint16_t frame_ctrl, control = 0, u2Offset = 0, u2IEsBufLen;
	uint8_t *cp, *pucBuf, *pos;
	const uint8_t *primary, *start, *end;
	uint8_t neid_arr[ELEM_ID_MAX_NUM], neid = 0;
	uint8_t nexid_arr[ELEM_ID_MAX_NUM], nexid = 0;

	bss = GET_BSS_INFO_BY_INDEX(prAdapter, ucBssIndex);
	starec = cnmGetStaRecByIndex(prAdapter, prMsduInfoSta->ucStaRecIndex);
	pos = (uint8_t *) prMsduInfo->prPacket + prMsduInfo->u2FrameLength;
	sta_ctrl = (struct IE_MULTI_LINK_INFO_STA_CONTROL *) pos;
	link = starec ? starec->ucLinkIndex : bss->ucLinkIndex;
	mgmt = (struct WLAN_MAC_MGMT_HEADER *)(prMsduInfo->prPacket);
	frame_ctrl = mgmt->u2FrameCtrl & MASK_FRAME_TYPE;

	sta_ctrl->ucSubID = SUB_IE_MLD_PER_STA_PROFILE;
	sta_ctrl->ucLength = 0;

	/* filling STA control field (fixed length) */
  	BE_SET_ML_STA_CTRL_LINK_ID(control, link);

	if (frame_ctrl != MAC_FRAME_PROBE_RSP &&
	    frame_ctrl != MAC_FRAME_BEACON) {
		control |= ML_STA_CTRL_COMPLETE_PROFILE;

		if (IS_BSS_APGO(bss))
			control |= (ML_STA_CTRL_BCN_INTV_PRESENT |
				    ML_STA_CTRL_DTIM_INFO_PRESENT);
	}

	/*
	 * meaningful if NSTR Link Pair Present is 1
	 * Bitmap subfield: 0 = 1 octet, 1 = 2 octets
	 */
	control |= (ML_STA_CTRL_MAC_ADDR_PRESENT |
		    ML_STA_CTRL_NSTR_LINK_PAIR_PRESENT);

	BE_SET_ML_STA_CTRL_FIELD(sta_ctrl->u2StaCtrl, control);

	/* filling STA info field (varied length) */
	cp = sta_ctrl->aucStaInfo;
	if (BE_IS_ML_STA_CTRL_PRESENCE_MAC(sta_ctrl->u2StaCtrl)) {
		COPY_MAC_ADDR(cp, bss->aucOwnMacAddr);
		cp += MAC_ADDR_LEN;
	}

	if (BE_IS_ML_STA_CTRL_PRESENCE_BCN_INTV(sta_ctrl->u2StaCtrl)) {
		WLAN_SET_FIELD_16(cp, bss->u2BeaconInterval);
		cp += 2;
	}

	if (BE_IS_ML_STA_CTRL_PRESENCE_DTIM(sta_ctrl->u2StaCtrl)) {
		*cp++ = bss->ucDTIMCount;
		*cp++ = bss->ucDTIMPeriod;
	}

	if (BE_IS_ML_STA_CTRL_PRESENCE_NSTR(sta_ctrl->u2StaCtrl)) {
		/* 1 octet for nstr bmp */
		*cp++ = 0;
	}

	sta_ctrl->ucLength = cp - pos - ELEM_HDR_LEN;
	prMultiLinkControlIE->ucLength += cp - pos;
	prMsduInfo->u2FrameLength += cp - pos;

	/* primary can skip sta info */
	if (prMsduInfoSta == prMsduInfo ||
	    !BE_IS_ML_STA_CTRL_COMPLETE(sta_ctrl->u2StaCtrl))
		goto DONE;

	/* start to fill STA profile field */
	pos = cp;

	/* handle inheritance ie */
	start = (uint8_t *) prMsduInfo->prPacket + u4BeginOffset;
	end = (uint8_t *) prMsduInfo->prPacket + u4PrimaryLength;
	pucBuf = (uint8_t *)prMsduInfoSta->prPacket + u4BeginOffset;
	u2IEsBufLen = prMsduInfoSta->u2FrameLength - u4BeginOffset;

	IE_FOR_EACH(pucBuf, u2IEsBufLen, u2Offset) {
		if (beMldNoNeedIE(pucBuf))
			continue;

		primary = kalFindIeExtIE(IE_ID(pucBuf),	IE_ID_EXT(pucBuf),
			start, end - start);
		if (!primary) {
			/* primary not found, copy it */
			kalMemCopy(cp, pucBuf, IE_SIZE(pucBuf));
			cp += IE_SIZE(pucBuf);
		} else if (kalMemCmp(pucBuf, primary, IE_SIZE(primary))) {
			/* same ie, copy and move to next primary */
			kalMemCopy(cp, pucBuf, IE_SIZE(pucBuf));
			cp += IE_SIZE(pucBuf);
			start = primary + IE_SIZE(primary);
		} else {
			/* found same primary, move to next primary */
			start = primary + IE_SIZE(primary);
		}
	}

	/* handle noninheritance ie */
	non_inh = (struct IE_NON_INHERITANCE *) cp;
	start = (uint8_t *) prMsduInfoSta->prPacket + u4BeginOffset;
	end = (uint8_t *) prMsduInfoSta->prPacket +
				prMsduInfoSta->u2FrameLength;
	pucBuf = (uint8_t *)prMsduInfo->prPacket + u4BeginOffset;
	u2IEsBufLen = u4PrimaryLength - u4BeginOffset;
	neid = 0;
	nexid = 0;

	IE_FOR_EACH(pucBuf, u2IEsBufLen, u2Offset) {
		if (beMldNoNeedIE(pucBuf))
			continue;

		primary = kalFindIeExtIE(IE_ID(pucBuf),	IE_ID_EXT(pucBuf),
			start, end - start);
		if (!primary) {
			/* noninheritance */
			if (IE_ID(pucBuf) == ELEM_ID_RESERVED)
				nexid_arr[nexid++] = IE_ID_EXT(pucBuf);
			else
				neid_arr[neid++] = IE_ID(pucBuf);
		} else {
			start = primary + IE_SIZE(primary);
		}
	}

	if (neid > 0 || nexid > 0) {
		non_inh->ucId = ELEM_ID_RESERVED;
		non_inh->ucLength = ELEM_HDR_LEN + 3 + neid + nexid;
		non_inh->ucExtId = ELEM_EXT_ID_NON_INHERITANCE;
		cp = non_inh->aucList;
		*cp++ = neid;
		for (i = 0; i < neid; i++)
			*cp++ = neid_arr[i];
		*cp++ = nexid;
		for (i = 0; i < nexid; i++)
			*cp++ = nexid_arr[i];
	}

	sta_ctrl->ucLength += cp - pos;
	prMultiLinkControlIE->ucLength += cp - pos;
	prMsduInfo->u2FrameLength += cp - pos;

DONE:
	DBGLOG(RLM, INFO, "Dump ML Link%d IE\n", link);
	DBGLOG_MEM8(RLM, INFO, sta_ctrl, IE_SIZE(sta_ctrl));
}


uint32_t beCalculateRnrIELen(
	struct ADAPTER *prAdapter,
	uint8_t ucBssIndex,
	struct STA_RECORD *prStaRec)
{
	struct BSS_INFO *bss;
	struct MLD_BSS_INFO *mld_bssinfo;

	bss = GET_BSS_INFO_BY_INDEX(prAdapter, ucBssIndex);
	mld_bssinfo = mldBssGetByBss(prAdapter, bss);

	if (!mld_bssinfo || mld_bssinfo->rBssList.u4NumElem <= 1)
		return 0;

	/* 10: Neighbor AP TBTT Offset + BSSID + MLD Para */
	return sizeof(struct IE_RNR) + mld_bssinfo->rBssList.u4NumElem *
		(sizeof(struct NEIGHBOR_AP_INFO_FIELD) + 10);
}

void beGenerateRnrIE(struct ADAPTER *prAdapter,
	struct MSDU_INFO *prMsduInfo)
{
	struct BSS_INFO *bss;
	struct MLD_BSS_INFO *mld_bssinfo;
	struct LINK *links;
	struct IE_RNR *rnr;
	struct NEIGHBOR_AP_INFO_FIELD *info;
	uint8_t *cp;

	bss = GET_BSS_INFO_BY_INDEX(prAdapter, prMsduInfo->ucBssIndex);
	mld_bssinfo = mldBssGetByBss(prAdapter, bss);

	if (!mld_bssinfo || mld_bssinfo->rBssList.u4NumElem <= 1)
		return;

	rnr = (struct IE_RNR *)	((uint8_t *)prMsduInfo->prPacket +
						prMsduInfo->u2FrameLength);
	rnr->ucId = ELEM_ID_RNR;
	rnr->ucLength = mld_bssinfo->rBssList.u4NumElem *
		(sizeof(struct NEIGHBOR_AP_INFO_FIELD) + 10);
	cp = rnr->aucInfoField;

	links = &mld_bssinfo->rBssList;
	LINK_FOR_EACH_ENTRY(bss, links, rLinkEntryMld,
		struct BSS_INFO) {
		info = (struct NEIGHBOR_AP_INFO_FIELD *) cp;

		/* count is default 0. no need to set,
		 * 10: Neighbor AP TBTT Offset + BSSID + MLD Para
		 */
		info->u2TbttInfoHdr = 10 << TBTT_INFO_HDR_LENGTH_OFFSET;
		info->ucOpClass = 0;
		info->ucChannelNum = 0;
		cp = info->aucTbttInfoSet;

		/* Neighbor AP TBTT Offset*/
		*cp++ = 0xff;

		/* BSSID */
		COPY_MAC_ADDR(cp, bss->aucOwnMacAddr);
		cp += MAC_ADDR_LEN;

		/* MLD Para */
		*cp++ = 0; /* MLD ID */
		*cp++ = bss->ucLinkIndex; /* Link ID */
		*cp++ = 0; /* BSS para change count */
	}

	prMsduInfo->u2FrameLength += IE_SIZE(rnr);
}

void beParsingMldElement(IN struct MULTI_LINK_INFO *prMlInfo, IN uint8_t *pucIE)
{
	const uint8_t *pos, *end;
	uint8_t ucMlCtrlType, ucMlCtrlPreBmp;
	struct IE_MULTI_LINK_INFO *prMlInfoIe;

	log_dbg(SCN, INFO, "dump ML IE, IE_LEN = %d\n", IE_LEN(pucIE));
	dumpMemory8(pucIE, IE_SIZE(pucIE));

	end = pucIE + IE_SIZE(pucIE);
	prMlInfoIe = (struct IE_MULTI_LINK_INFO *)pucIE;
	pos = prMlInfoIe->aucMultiLinkVarIe;

	/* ML control bits[4,15] is presence bitmap */
	ucMlCtrlPreBmp = ((prMlInfoIe->u2MlCtrl & ML_CTRL_PRE_BMP_MASK)
				>> ML_CTRL_PRE_BMP_SHIFT);
	/* ML control bits[0,2] is type */
	ucMlCtrlType = (prMlInfoIe->u2MlCtrl & ML_CTRL_TYPE_MASK);

	/* It shall be Basic variant ML element*/
	if (ucMlCtrlType != ML_ELEMENT_TYPE_BASIC) {
		log_dbg(SCN, WARN, "invalid ML control type:%d", ucMlCtrlType);
		return;
	}

	prMlInfo->ucMlCtrlPreBmp = ucMlCtrlPreBmp;

	/* Check ML control that which common info exist */
	if (ucMlCtrlPreBmp & ML_CTRL_MLD_MAC_ADDR_PRESENT) {
		COPY_MAC_ADDR(prMlInfo->aucMldAddr, pos);
		log_dbg(SCN, INFO, "MLD common Info MAC addr = "MACSTR"",
			MAC2STR(prMlInfo->aucMldAddr));
		prMlInfo->ucValid = TRUE;
		pos += MAC_ADDR_LEN;
	}
	if (ucMlCtrlPreBmp & ML_CTRL_LINK_ID_INFO_PRESENT) {
		prMlInfo->ucLinkId = *pos;
		log_dbg(SCN, INFO, "ML common Info LinkID = %d",
			*pos);
		pos += 1;
	}
	if (ucMlCtrlPreBmp & ML_CTRL_BSS_PARA_CHANGE_COUNT_PRESENT) {
		prMlInfo->ucBssParaChangeCount = *pos;
		log_dbg(SCN, INFO, "ML common Info BssParaChangeCount = %d",
			*pos);
		pos += 1;
	}
	if (ucMlCtrlPreBmp & ML_CTRL_MEDIUM_SYN_DELAY_INFO_PRESENT) {
		/* todo: handle 2byte MEDIUM_SYN_DELAY_INFO_PRESENT */
 		kalMemCopy(&prMlInfo->u2MediumSynDelayInfo, pos, 2);
		log_dbg(SCN, INFO, "ML common Info MediumSynDelayInfo = 0x%x",
			prMlInfo->u2MediumSynDelayInfo);
		pos += 2;
	}
	if (ucMlCtrlPreBmp & ML_CTRL_EML_CAPA_PRESENT) {
		kalMemCopy(&prMlInfo->u2EmlCap, pos, 2);
		log_dbg(SCN, INFO, "ML common Info EML capa = 0x%x",
			prMlInfo->u2EmlCap);
		pos += 2;
	}
	if (ucMlCtrlPreBmp & ML_CTRL_MLD_CAPA_PRESENT) {
		kalMemCopy(&prMlInfo->u2MldCap, pos, 2);
		log_dbg(SCN, INFO, "ML common Info MLD capa = 0x%x",
			prMlInfo->u2MldCap);
		pos += 2;
	}

	/* pos point to link info, recusive parse it */
	while (pos < end) {
		struct IE_ML_PER_STA_PROFILE_INFO *prIePerStaProfile;
		struct STA_PROFILE *prStaProfile;
		uint8_t ucLinkId, ucPerStaProfLen, ucCurrLength;
		uint16_t u2StaControl;

		if (*pos != SUB_IE_MLD_PER_STA_PROFILE ||
		    prMlInfo->ucLinkNum >= MLD_LINK_MAX)
			break;

		ucCurrLength = 0;
		ucPerStaProfLen = IE_LEN(pos);
		/* 2 bytes offset: Subelement ID, IE length
		 * now pos point to STA Control in Per-STA profile subelement
		 */
		pos += 2;

		prIePerStaProfile = (struct IE_ML_PER_STA_PROFILE_INFO *) pos;
		u2StaControl = prIePerStaProfile->u2StaCtrl;

		ucLinkId = (u2StaControl & ML_STA_CTRL_LINK_ID_MASK);

		prStaProfile = &prMlInfo->rStaProfiles[prMlInfo->ucLinkNum++];
		prStaProfile->ucLinkId = ucLinkId;
		prStaProfile->u2StaCtrl = u2StaControl;

		log_dbg(SCN, INFO, "LinkID=%d Ctrl=0x%x(%s) Total=%d",
				ucLinkId, u2StaControl,
				(u2StaControl & ML_STA_CTRL_COMPLETE_PROFILE) ?
				"COMPLETE" : "PARTIAL",
				prMlInfo->ucLinkNum);

		/* 2 bytes is length of STA control,
		 * ucCurrLength will be length of STA control + STA Info,
		 * and keep pos point to STA Control.
		 */
		ucCurrLength += 2;
		if (u2StaControl & ML_STA_CTRL_MAC_ADDR_PRESENT) {
			COPY_MAC_ADDR(prStaProfile->aucLinkAddr,
					(pos + ucCurrLength));
			log_dbg(SCN, INFO, "LinkID=%d, LinkAddr="MACSTR"",
				ucLinkId, MAC2STR(prStaProfile->aucLinkAddr));
			ucCurrLength += MAC_ADDR_LEN;
		}
		if (u2StaControl & ML_STA_CTRL_BCN_INTV_PRESENT) {
			kalMemCopy(&prStaProfile->u2BcnIntv,
				pos + ucCurrLength, 2);
			log_dbg(SCN, INFO, "LinkID=%d, BCN_INTV = %d",
				ucLinkId, prStaProfile->u2BcnIntv);
			ucCurrLength += 2;
		}
		if (u2StaControl & ML_STA_CTRL_DTIM_INFO_PRESENT) {
			kalMemCopy(&prStaProfile->u2DtimInfo,
				pos + ucCurrLength, 2);
			log_dbg(SCN, INFO, "LinkID=%d, DTIM_INFO = 0x%x",
				ucLinkId, prStaProfile->u2DtimInfo);
			ucCurrLength += 2;
		}
		/* If the Complete Profile subfield = 1 and
		 * NSTR Link Pair Present = 1, then NSTR Indication Bitmap exist
		 * NSTR Bitmap Size = 1 if the length of the corresponding
		 * NSTR Indication Bitmap is 2 bytes, and = 0 if the
		 * length of the corresponding NSTR Indication Bitmap = 1 byte
		 */
		if ((u2StaControl & ML_STA_CTRL_COMPLETE_PROFILE) &&
			(u2StaControl & ML_STA_CTRL_NSTR_LINK_PAIR_PRESENT)) {
			if (((u2StaControl & ML_STA_CTRL_NSTR_BMP_SIZE) >>
				ML_STA_CTRL_NSTR_BMP_SIZE_SHIFT) == 0) {
				prStaProfile->u2NstrBmp = *(pos + ucCurrLength);
				log_dbg(SCN, INFO, "LinkID=%d, NSTR_BMP0=0x%x",
					ucLinkId, prStaProfile->u2NstrBmp);
				ucCurrLength += 1;
			} else {
				kalMemCopy(&prStaProfile->u2NstrBmp,
					pos + ucCurrLength, 2);
				log_dbg(SCN, INFO, "LinkID=%d, NSTR_BMP1=0x%x",
					ucLinkId, prStaProfile->u2NstrBmp);
				ucCurrLength += 2;
			}
		}
		/*(ucPerStaProfLen - ucCurrLength) is length of STA Profile
		 * copy STA profile in Per-STA profile subelement.
		 * STA profile contains Inheritance and Non-Inheritance element
		 * todo: handle Inheritance and Non-Inheritance element
		 */
		if (ucPerStaProfLen - ucCurrLength <
				sizeof(prStaProfile->aucIEbuf))
			kalMemCopy(prStaProfile->aucIEbuf, pos + ucCurrLength,
				ucPerStaProfLen - ucCurrLength);

		/* point to next Per-STA profile*/
		pos += ucPerStaProfLen;
	}
}

void mldBssDump(struct ADAPTER *prAdapter)
{
	struct MLD_BSS_INFO *prMldBssInfo;
	uint8_t i = 0;

	DBGLOG(INIT, INFO, "========== START ==========\n");

	for (i = 0; i < ARRAY_SIZE(prAdapter->aprMldBssInfo); i++) {
		struct BSS_INFO *prBssInfo;
		struct LINK *prBssList;

		prMldBssInfo = &prAdapter->aprMldBssInfo[i];

		if (!prMldBssInfo->fgIsInUse)
			continue;

		DBGLOG(INIT, INFO, "[%d] om:%d, group:%d, om_remap:%d, mld_mac:" MACSTR "\n",
			i,
			prMldBssInfo->ucOmacIdx,
			prMldBssInfo->ucGroupMldId,
			prMldBssInfo->ucOmRemapIdx,
			prMldBssInfo->aucOwnMldAddr);

		DBGLOG(INIT, INFO, "\tBss list:\n");
		prBssList = &prMldBssInfo->rBssList;
		LINK_FOR_EACH_ENTRY(prBssInfo, prBssList, rLinkEntryMld,
		    struct BSS_INFO)
			cnmDumpBssInfo(prAdapter, prBssInfo->ucBssIndex);

		DBGLOG(INIT, INFO, "\n");
	}

	DBGLOG(INIT, INFO, "========== END ==========\n");
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
	uint8_t i = 0, j = 0;

	DBGLOG(INIT, INFO, "========== START ==========\n");

	for (i = 0; i < ARRAY_SIZE(prAdapter->aprMldStarec); i++) {
		struct MLD_STA_RECORD *prMldStarec = &prAdapter->aprMldStarec[i];
		struct STA_RECORD *prCurrStarec;
		struct LINK *prStarecList;

		if (!prMldStarec->fgIsInUse)
			continue;

		DBGLOG(INIT, INFO, "[%d] pri:%d, sec:%d, setup:%d, emlmr:%d, emlsr:%d, str:[0x%x,0x%x,0x%x], mac:" MACSTR "\n",
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

		DBGLOG(INIT, INFO, "\tRX pkt count:\n");
		for (j = 0; j < MLD_LINK_MAX; j++)
			DBGLOG(INIT, INFO, "\t\tband%d:0x%llx\n",
				j, prMldStarec->aucRxPktCnt[j]);

		DBGLOG(INIT, INFO, "\tSta list:\n");
		prStarecList = &prMldStarec->rStarecList;
		LINK_FOR_EACH_ENTRY(prCurrStarec, prStarecList, rLinkEntryMld,
		    struct STA_RECORD)
			cnmDumpStaRec(prAdapter, prCurrStarec->ucIndex);

		DBGLOG(INIT, INFO, "\n");
	}
	DBGLOG(INIT, INFO, "========== END ==========\n");
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

void mldStarecLogRxData(struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	uint8_t ucHwBandIdx)
{
	struct MLD_STA_RECORD *mld_sta = mldStarecGetByStarec(prAdapter,
		prStaRec);

	if (!mld_sta)
		return;

	if (ucHwBandIdx >= MLD_LINK_MAX)
		return;

	mld_sta->aucRxPktCnt[ucHwBandIdx]++;
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

#endif /* CFG_SUPPORT_802_11BE_MLO == 1 */
