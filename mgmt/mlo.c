/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

/*! \file   "mlo.c"
 *  \brief
 */

#include "precomp.h"
#if (CFG_SUPPORT_802_11BE_MLO == 1)

#define MAX_DUP_IE_COUNT 64

#define IS_MLD_BSSINFO_VALID(__prMldBssInfo) \
	(__prMldBssInfo && __prMldBssInfo->rBssList.u4NumElem > 1)

#define IS_MLD_STAREC_VALID(__prMldStaRec) \
	(__prMldStaRec && __prMldStaRec->rStarecList.u4NumElem > 1)

uint8_t beSanityCheckMld(struct ADAPTER *prAdapter, uint8_t *pucPacket,
	uint16_t u2PacketLen, struct STA_RECORD *prStaRec, uint8_t ucBssIndex)
{
	struct BSS_INFO *bss;
	struct STA_RECORD *starec;
	struct MLD_STA_RECORD *mld_starec;
	struct MLD_BSS_INFO *mld_bssinfo;
	struct MULTI_LINK_INFO parse, *info = &parse;
	struct WLAN_MAC_MGMT_HEADER *mgmt;
	const uint8_t *ml;
	struct LINK *links;
	uint16_t frame_ctrl;
	uint32_t offset;
	uint8_t i;

	bss = GET_BSS_INFO_BY_INDEX(prAdapter, ucBssIndex);
	offset = sortGetPayloadOffset(prAdapter, pucPacket);
	mgmt = (struct WLAN_MAC_MGMT_HEADER *)(pucPacket);
	frame_ctrl = mgmt->u2FrameCtrl & MASK_FRAME_TYPE;

	kalMemSet(info, 0, sizeof(*info));
	/* look for ml ie from frame payload */
	ml = kalFindIeExtIE(ELEM_ID_RESERVED, ELEM_EXT_ID_MLD,
		pucPacket + offset, u2PacketLen - offset);
	if (ml)
		beParseMldElement(info, ml, bss->aucOwnMacAddr, "Sanity Check");

	if (IS_BSS_APGO(bss)) {
		/* ap mode, check auth/assoc req */
		mld_starec = mldStarecGetByStarec(prAdapter, prStaRec);
		mld_bssinfo = mldBssGetByBss(prAdapter, bss);
		if (IS_MLD_BSSINFO_VALID(mld_bssinfo)) {
			links =  &mld_bssinfo->rBssList;

			/* reject if sta has unexpected link info */
			if (info->ucValid &&
			   ((frame_ctrl == MAC_FRAME_AUTH &&
					info->ucLinkNum != 0) ||
			   (frame_ctrl != MAC_FRAME_AUTH &&
				(info->ucLinkNum <= 1 ||
				 info->ucLinkNum > links->u4NumElem)))) {
				DBGLOG(ML, ERROR,
					"STA wrong ML ie (valid=%d, num=%d)\n",
					info->ucValid,
					info->ucLinkNum);
				return FALSE;
			}

			/* auth doesn't need to check link info */
			if (!info->ucValid || frame_ctrl == MAC_FRAME_AUTH)
				return TRUE;

			if (!prStaRec) {
				DBGLOG(ML, ERROR, "no starec\n");
				return FALSE;
			}

			if (!mld_starec) {
				/* SAE auth is handled by hostapd. mld_starec
				 * can be null, so bypass sanity check.
				 */
				if (prStaRec->ucAuthAlgNum !=
						AUTH_ALGORITHM_NUM_SAE) {
					DBGLOG(ML, ERROR,
						"starec(%d, %d) without mld_starec\n",
						prStaRec->ucIndex,
						prStaRec->ucWlanIndex);
					return FALSE;
				}
			} else if (UNEQUAL_MAC_ADDR(info->aucMldAddr,
				mld_starec->aucPeerMldAddr)) {
				DBGLOG(ML, ERROR,
					"STA wrong ML ie (addr=" MACSTR
					", num=%d) instead of " MACSTR "\n",
					MAC2STR(info->aucMldAddr),
					info->ucLinkNum,
					MAC2STR(mld_starec->aucPeerMldAddr));
				return FALSE;
			}

			/* link not matched, skip it */
			for (i = 0; i < info->ucLinkNum; i++) {
				struct STA_PROFILE *profile =
					&info->rStaProfiles[i];
				uint8_t count = 0;

				LINK_FOR_EACH_ENTRY(bss, links,
					rLinkEntryMld, struct BSS_INFO) {
					if (count >= MLD_LINK_MAX) {
						DBGLOG(ML, ERROR,
							"too many links!!!\n");
						return FALSE;
					}

					if (profile->ucLinkId ==
						bss->ucLinkIndex &&
					    EQUAL_MAC_ADDR(profile->aucLinkAddr,
						bss->aucOwnMacAddr))
						break;
					count++;
				}
				if (bss == NULL || !profile->ucComplete) {
					DBGLOG(ML, ERROR,
					   "STA wrong link (id=%d, addr=" MACSTR
					   "complete=%d)\n",
					   profile->ucLinkId,
					   MAC2STR(profile->aucLinkAddr),
					   profile->ucComplete);
					return FALSE;
				}
			}
		} else {
			if (info->ucValid) {
				DBGLOG(ML, ERROR, "STA shouldn't have ML ie\n");
				return FALSE;
			}
		}
	} else {
		/* sta mode, check auth/assoc resp */
		mld_starec = mldStarecGetByStarec(prAdapter, prStaRec);
		if (IS_MLD_STAREC_VALID(mld_starec)) {
			links =  &mld_starec->rStarecList;
			/* sta already send ml ie, expected ml ie in resp */
			if (!info->ucValid ||
			   (frame_ctrl == MAC_FRAME_AUTH &&
					info->ucLinkNum > 0) ||
			   (frame_ctrl != MAC_FRAME_AUTH &&
				info->ucLinkNum != links->u4NumElem) ||
			   UNEQUAL_MAC_ADDR(info->aucMldAddr,
				mld_starec->aucPeerMldAddr)) {
				DBGLOG(ML, ERROR,
					"AP wrong ML ie (addr=" MACSTR
					", valid=%d, num=%d)\n",
					MAC2STR(info->aucMldAddr),
					info->ucValid,
					info->ucLinkNum);
				return FALSE;
			}

			/* link not matched, skip it */
			for (i = 0; i < info->ucLinkNum; i++) {
				struct STA_PROFILE *profile =
					&info->rStaProfiles[i];
				uint8_t count = 0;

				LINK_FOR_EACH_ENTRY(starec, links,
					rLinkEntryMld, struct STA_RECORD) {
					if (count >= MLD_LINK_MAX) {
						DBGLOG(ML, ERROR,
							"too many links!!!\n");
						return FALSE;
					}

					if (profile->ucLinkId ==
						starec->ucLinkIndex &&
					    EQUAL_MAC_ADDR(profile->aucLinkAddr,
						starec->aucMacAddr))
						break;
					count++;
				}
				if (starec == NULL || !profile->ucComplete) {
					DBGLOG(ML, ERROR,
					   "AP Wrong link (id=%d, addr=" MACSTR
					   "complete=%d)\n",
					   profile->ucLinkId,
					   MAC2STR(profile->aucLinkAddr),
					   profile->ucComplete);
					return FALSE;
				}
			}
		} else {
			if (info->ucValid) {
				DBGLOG(ML, ERROR, "AP should not have ML ie\n");
				return FALSE;
			}
		}
	}

	return TRUE;
}

uint32_t beCalculateMldIELen(
	struct ADAPTER *prAdapter,
	uint8_t ucBssIndex,
	struct STA_RECORD *prStaRec)
{
	/* header: 2, extid: 1, common control: 2, common info: 11
	 * (len:1, mac:6, link id:1, bss change count: 1, mld cap: 2)
	 */
	return 16;
}

void beGenerateMldIE(struct ADAPTER *prAdapter,
	struct MSDU_INFO *prMsduInfo)
{
	struct MLD_STA_RECORD *mld_starec;
	struct MLD_BSS_INFO *mld_bssinfo;
	struct STA_RECORD *prStaRec;
	struct BSS_INFO *bss;
	struct WLAN_MAC_MGMT_HEADER *mgmt;
	uint8_t ucBssIndex;
	uint16_t frame_ctrl;

	ucBssIndex = prMsduInfo->ucBssIndex;
	prStaRec = cnmGetStaRecByIndex(prAdapter, prMsduInfo->ucStaRecIndex);
	bss = GET_BSS_INFO_BY_INDEX(prAdapter, ucBssIndex);
	mld_starec = mldStarecGetByStarec(prAdapter, prStaRec);
	mld_bssinfo = mldBssGetByBss(prAdapter, bss);
	mgmt = (struct WLAN_MAC_MGMT_HEADER *)(prMsduInfo->prPacket);
	frame_ctrl = mgmt->u2FrameCtrl & MASK_FRAME_TYPE;

	if (frame_ctrl == MAC_FRAME_PROBE_RSP ||
	    frame_ctrl == MAC_FRAME_BEACON) {
		beGenerateMldBasicInfo(prAdapter, prMsduInfo);
	} else if (IS_MLD_STAREC_VALID(mld_starec) ||
		  (IS_MLD_BSSINFO_VALID(mld_bssinfo) && mld_starec)) {
		DBGLOG(ML, INFO,  "Start MLO");
		beGenerateMldBasicInfo(prAdapter, prMsduInfo);
	}
}

uint8_t beGenerateExternalAuthMldIE(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	uint8_t *pucBuf)
{
	struct IE_MULTI_LINK_CONTROL *common = NULL;
	struct MLD_STA_RECORD *mld_starec;
	struct MSDU_INFO *prMsduInfo;

	mld_starec = mldStarecGetByStarec(prAdapter, prStaRec);
	if (IS_MLD_STAREC_VALID(mld_starec)) {
		/* Allocate a MSDU_INFO_T */
		prMsduInfo = cnmMgtPktAlloc(prAdapter, MAX_LEN_OF_MLIE);
		if (prMsduInfo == NULL) {
			DBGLOG(SAA, WARN,
			       "No PKT_INFO_T for sending external auth.\n");
			return 0;
		}

		prMsduInfo->u2FrameLength = 0;
		common = beGenerateMldBasicInfo(prAdapter, prMsduInfo);
		if (!common) {
			cnmMgtPktFree(prAdapter, prMsduInfo);
			return 0;
		}

		/* copy ml ie to output buffer */
		kalMemCopy(pucBuf, prMsduInfo->prPacket,
			prMsduInfo->u2FrameLength);
		cnmMgtPktFree(prAdapter, prMsduInfo);

		return prMsduInfo->u2FrameLength;
	}

	return 0;
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
	uint8_t count = 0;

	ASSERT(prAdapter);
	ASSERT(prMsduInfo);

	mld_starec = mldStarecGetByStarec(prAdapter, prStaRec);
	offset = sortMsduPayloadOffset(prAdapter, prMsduInfo);
	len = prMsduInfo->u2FrameLength;

	if (!IS_MLD_STAREC_VALID(mld_starec))
		return;

	common = beGenerateMldBasicInfo(prAdapter, prMsduInfo);
	if (!common)
		return;

	links = &mld_starec->rStarecList;
	LINK_FOR_EACH_ENTRY(starec, links, rLinkEntryMld,
		struct STA_RECORD) {
		struct BSS_INFO *bss = GET_BSS_INFO_BY_INDEX(prAdapter,
				starec->ucBssIndex);

		if (count >= MLD_LINK_MAX) {
			DBGLOG(ML, ERROR, "too many links!!!\n");
			return;
		}

		DBGLOG(ML, INFO,
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
				DBGLOG(ML, WARN,
					"No PKT_INFO_T for sending MLD STA.\n");
				continue;
			}

			beGenerateMldSTAInfo(prAdapter, common,
				prMsduInfo, offset, len,
				msdu_sta, bss->ucBssIndex);
			cnmMgtPktFree(prAdapter, msdu_sta);
		}

		count++;
	}

	DBGLOG(ML, INFO, "Dump total MLIE\n");
	DBGLOG_MEM8(ML, INFO, common, IE_SIZE(common));
}

struct IE_MULTI_LINK_CONTROL *beGenerateMldBasicInfo(
	struct ADAPTER *prAdapter,
	struct MSDU_INFO *prMsduInfo)
{
	uint8_t *cp;
	struct MLD_BSS_INFO *mld_bssinfo;
	struct BSS_INFO *bss;
	struct IE_MULTI_LINK_CONTROL *common;
	uint16_t present;

	bss = GET_BSS_INFO_BY_INDEX(prAdapter, prMsduInfo->ucBssIndex);
	mld_bssinfo = mldBssGetByBss(prAdapter, bss);

	if (!IS_MLD_BSSINFO_VALID(mld_bssinfo))
		return 0;

	common = (struct IE_MULTI_LINK_CONTROL *)
		(((uint8_t *)prMsduInfo->prPacket) + prMsduInfo->u2FrameLength);

	common->ucId = ELEM_ID_RESERVED;
	common->ucExtId = ELEM_EXT_ID_MLD;

	/* filling control field */
	BE_SET_ML_CTRL_TYPE(common->u2Ctrl, ML_ELEMENT_TYPE_BASIC);
	present = ML_CTRL_MLD_CAPA_PRESENT;

	if (IS_BSS_APGO(bss)) {
		present |= (ML_CTRL_LINK_ID_INFO_PRESENT |
			    ML_CTRL_BSS_PARA_CHANGE_COUNT_PRESENT);
	}

	BE_SET_ML_CTRL_PRESENCE(common->u2Ctrl, present);

	/* filling common info field*/
	cp = common->aucCommonInfo;
	cp++; /* reserve for common info length */

	COPY_MAC_ADDR(cp, mld_bssinfo->aucOwnMldAddr);
	cp += MAC_ADDR_LEN;

	if (BE_IS_ML_CTRL_PRESENCE_LINK_ID(common->u2Ctrl))
		*cp++ = bss->ucLinkIndex;

	if (BE_IS_ML_CTRL_PRESENCE_BSS_PARA_CHANGE_COUNT(common->u2Ctrl))
		*cp++ = 0;

	if (BE_IS_ML_CTRL_PRESENCE_MLD_CAP(common->u2Ctrl)) {
		/* Set to the maximum number of affiliated STAs in the non-AP
		 * MLD that support simultaneous transmission or reception of
		 * frames minus 1. For an AP MLD, set to the number of
		 * affiliated APs minus 1
		 */
		WLAN_SET_FIELD_16(cp, prAdapter->rWifiVar.ucMldLinkMax - 1);
		cp += 2;
	}

	/* update common info length, ie length, frame length */
	*common->aucCommonInfo = cp - common->aucCommonInfo;
	common->ucLength = cp - (uint8_t *) common - ELEM_HDR_LEN;
	prMsduInfo->u2FrameLength += IE_SIZE(common);

	DBGLOG(ML, INFO, "Dump ML common IE\n");
	DBGLOG_MEM8(ML, INFO, common, IE_SIZE(common));

	return common;
}

void beGenerateMlProbeReqIE(uint8_t *pucIE,
	uint32_t *u4IELength, uint8_t ucMldId)
{
	struct IE_MULTI_LINK_CONTROL *common;

	common = (struct IE_MULTI_LINK_CONTROL *)pucIE;

	common->ucId = ELEM_ID_RESERVED;
	common->ucExtId = ELEM_EXT_ID_MLD;

	/* EID 1byte + ML Ctrl 2byte + CommonInfo 2byte */
	common->ucLength = 5;

	/* filling control field */
	BE_SET_ML_CTRL_TYPE(common->u2Ctrl, ML_ELEMENT_TYPE_PROBE_REQ);
	BE_SET_ML_CTRL_PRESENCE(common->u2Ctrl, MLD_ID_PRESENT);

	/* Common Info Length = 2 */
	*common->aucCommonInfo = 2;
	/* Assign MLD ID*/
	*(common->aucCommonInfo + 1) = ucMldId;

	*u4IELength += common->ucLength;
}


uint8_t beDupSkipIE(uint8_t *pucBuf)
{
	if (IE_ID(pucBuf) == ELEM_ID_RNR ||
	    IE_ID(pucBuf) == ELEM_ID_NEIGHBOR_REPORT ||
	    IE_ID(pucBuf) == ELEM_ID_MBSSID ||
	    (IE_ID(pucBuf) == ELEM_ID_RESERVED &&
	     IE_ID_EXT(pucBuf) == ELEM_EXT_ID_MLD) ||
	    (IE_ID(pucBuf) == ELEM_ID_RESERVED &&
	     IE_ID_EXT(pucBuf) == ELEM_EXT_ID_NON_INHERITANCE))
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
	struct IE_MULTI_LINK_STA_CONTROL *sta_ctrl;
	struct IE_NON_INHERITANCE *non_inh;
	struct STA_RECORD *starec;
	struct BSS_INFO *bss;
	struct WLAN_MAC_MGMT_HEADER *mgmt;
	uint8_t i, link, *cp, *pucBuf, *pos;
	uint16_t frame_ctrl, control = 0, u2Offset = 0, u2IEsBufLen, u2CapInfo;
	const uint8_t *primary, *start, *end;
	uint8_t neid_arr[ELEM_ID_MAX_NUM], neid = 0;
	uint8_t nexid_arr[ELEM_ID_MAX_NUM], nexid = 0;

	bss = GET_BSS_INFO_BY_INDEX(prAdapter, ucBssIndex);
	starec = cnmGetStaRecByIndex(prAdapter, prMsduInfoSta->ucStaRecIndex);
	pos = (uint8_t *) prMsduInfo->prPacket + prMsduInfo->u2FrameLength;
	sta_ctrl = (struct IE_MULTI_LINK_STA_CONTROL *) pos;
	link = starec ? starec->ucLinkIndex : bss->ucLinkIndex;
	mgmt = (struct WLAN_MAC_MGMT_HEADER *)(prMsduInfo->prPacket);
	frame_ctrl = mgmt->u2FrameCtrl & MASK_FRAME_TYPE;

	/* Only Management frames belonging to subtypes (Re)Association Request,
	 * (Re)Association Response, or Probe Response that is an ML probe
	 * response can carry complete profile of a reported STA
	 */
	if (frame_ctrl != MAC_FRAME_ASSOC_REQ &&
	    frame_ctrl != MAC_FRAME_ASSOC_RSP &&
	    frame_ctrl != MAC_FRAME_REASSOC_RSP &&
	    frame_ctrl != MAC_FRAME_REASSOC_RSP &&
	    frame_ctrl != MAC_FRAME_PROBE_RSP)
		return;

	if (frame_ctrl != MAC_FRAME_PROBE_RSP) {
		control |= ML_STA_CTRL_COMPLETE_PROFILE;

		if (IS_BSS_APGO(bss))
			control |= (ML_STA_CTRL_BCN_INTV_PRESENT |
				    ML_STA_CTRL_DTIM_INFO_PRESENT);
	}

	sta_ctrl->ucSubID = SUB_IE_MLD_PER_STA_PROFILE;
	sta_ctrl->ucLength = 0;

	/* filling STA control field (fixed length) */
	BE_SET_ML_STA_CTRL_LINK_ID(control, link);

	/*
	 * meaningful if NSTR Link Pair Present is 1
	 * Bitmap subfield: 0 = 1 octet, 1 = 2 octets
	 */
	control |= (ML_STA_CTRL_MAC_ADDR_PRESENT |
		    ML_STA_CTRL_NSTR_LINK_PAIR_PRESENT);

	BE_SET_ML_STA_CTRL_FIELD(sta_ctrl->u2StaCtrl, control);

	/* filling STA info field (varied length) */
	cp = sta_ctrl->aucStaInfo;
	cp++; /* reserved for sta info length */

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

	*sta_ctrl->aucStaInfo = cp - sta_ctrl->aucStaInfo;
	sta_ctrl->ucLength = cp - pos - ELEM_HDR_LEN;
	prMultiLinkControlIE->ucLength += cp - pos;
	prMsduInfo->u2FrameLength += cp - pos;

	/* primary can skip sta info */
	if (prMsduInfoSta == prMsduInfo ||
	    !BE_IS_ML_STA_CTRL_COMPLETE(sta_ctrl->u2StaCtrl))
		goto DONE;

	/* start to fill STA profile field */
	pos = cp;

	/* Fill the Capability Information field. */
	u2CapInfo = assocBuildCapabilityInfo(prAdapter, starec);
	WLAN_SET_FIELD_16(cp, u2CapInfo);
	cp += 2;

	/* handle inheritance ie */
	start = (uint8_t *) prMsduInfo->prPacket + u4BeginOffset;
	end = (uint8_t *) prMsduInfo->prPacket + u4PrimaryLength;
	pucBuf = (uint8_t *)prMsduInfoSta->prPacket + u4BeginOffset;
	u2IEsBufLen = prMsduInfoSta->u2FrameLength - u4BeginOffset;

	IE_FOR_EACH(pucBuf, u2IEsBufLen, u2Offset) {
		if (beDupSkipIE(pucBuf))
			continue;

		/* always search all for vendor ie*/
		if (IE_ID(pucBuf) == ELEM_ID_VENDOR) {
			uint32_t oui;

			WLAN_GET_FIELD_BE24(WFA_IE(pucBuf)->aucOui, &oui);
			primary = kalFindVendorIe(
				oui,
				WFA_IE(pucBuf)->ucOuiType,
				prMsduInfo->prPacket + u4BeginOffset,
				u4PrimaryLength - u4BeginOffset);
		} else {
			primary = kalFindIeExtIE(
				IE_ID(pucBuf), IE_ID_EXT(pucBuf),
				start, end - start);
		}

		if (!primary) {
			/* primary not found, copy it */
			kalMemCopy(cp, pucBuf, IE_SIZE(pucBuf));
			cp += IE_SIZE(pucBuf);
		} else {
			if (kalMemCmp(pucBuf, primary, IE_SIZE(primary))) {
				/* different ie, copy and move to next primary */
				kalMemCopy(cp, pucBuf, IE_SIZE(pucBuf));
				cp += IE_SIZE(pucBuf);
				if (IE_ID(pucBuf) != ELEM_ID_VENDOR)
					start = primary + IE_SIZE(primary);
			} else {
				/* found same primary, move to next primary */
				if (IE_ID(pucBuf) != ELEM_ID_VENDOR)
					start = primary + IE_SIZE(primary);
			}
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
		if (beDupSkipIE(pucBuf))
			continue;

		/* always search all for vendor ie*/
		if (IE_ID(pucBuf) == ELEM_ID_VENDOR) {
			uint32_t oui;

			WLAN_GET_FIELD_BE24(WFA_IE(pucBuf)->aucOui, &oui);
			primary = kalFindVendorIe(
				oui,
				WFA_IE(pucBuf)->ucOuiType,
				prMsduInfoSta->prPacket + u4BeginOffset,
				prMsduInfoSta->u2FrameLength - u4BeginOffset);
		} else {
			primary = kalFindIeExtIE(
				IE_ID(pucBuf), IE_ID_EXT(pucBuf),
				start, end - start);
		}

		if (!primary) {
			/* noninheritance */
			if (IE_ID(pucBuf) == ELEM_ID_RESERVED)
				nexid_arr[nexid++] = IE_ID_EXT(pucBuf);
			else
				neid_arr[neid++] = IE_ID(pucBuf);
		} else {
			if (IE_ID(pucBuf) != ELEM_ID_VENDOR)
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
	DBGLOG(ML, INFO, "Dump ML Link%d IE\n", link);
	DBGLOG_MEM8(ML, INFO, sta_ctrl, IE_SIZE(sta_ctrl));
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
	uint8_t count = 0;

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
		if (count >= MLD_LINK_MAX) {
			DBGLOG(ML, ERROR, "too many links!!!\n");
			return;
		}

		info = (struct NEIGHBOR_AP_INFO_FIELD *) cp;

		/* count is default 0. no need to set,
		 * 10: Neighbor AP TBTT Offset + BSSID + MLD Para
		 */
		info->u2TbttInfoHdr = 10 << TBTT_INFO_HDR_LENGTH_OFFSET;
		info->ucOpClass =
			rlmGetOpClassForChannel(
				bss->ucPrimaryChannel,
				bss->eBand);
		info->ucChannelNum = bss->ucPrimaryChannel;

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

		count++;
	}

	prMsduInfo->u2FrameLength += IE_SIZE(rnr);
}

void beParseMldElement(IN struct MULTI_LINK_INFO *prMlInfo,
	IN const uint8_t *pucIE, IN const uint8_t *paucBssId,
	IN const char* pucDesc)
{
	const uint8_t *pos, *end;
	uint8_t ucMlCtrlType, ucMlCtrlPreBmp;
	struct IE_MULTI_LINK_CONTROL *prMlInfoIe;

	DBGLOG(ML, INFO, "%s ML IE, IE_LEN = %d\n", pucDesc, IE_LEN(pucIE));
	dumpMemory8((uint8_t *)pucIE, IE_SIZE(pucIE));

	end = pucIE + IE_SIZE(pucIE);
	prMlInfoIe = (struct IE_MULTI_LINK_CONTROL *)pucIE;
	pos = prMlInfoIe->aucCommonInfo;

	/* ML control bits[4,15] is presence bitmap */
	ucMlCtrlPreBmp = ((prMlInfoIe->u2Ctrl & ML_CTRL_PRE_BMP_MASK)
				>> ML_CTRL_PRE_BMP_SHIFT);
	/* ML control bits[0,2] is type */
	ucMlCtrlType = (prMlInfoIe->u2Ctrl & ML_CTRL_TYPE_MASK);

	/* It shall be Basic variant ML element*/
	if (ucMlCtrlType != ML_ELEMENT_TYPE_BASIC) {
		DBGLOG(ML, WARN, "invalid ML control type:%d", ucMlCtrlType);
		return;
	}

	prMlInfo->ucMlCtrlPreBmp = ucMlCtrlPreBmp;
	prMlInfo->ucCommonInfoLength = *pos++;

	DBGLOG(ML, INFO, "\tML common Info Len = %d",
		prMlInfo->ucCommonInfoLength);

	/* Check ML control that which common info exist */
	COPY_MAC_ADDR(prMlInfo->aucMldAddr, pos);
	DBGLOG(ML, INFO, "\tML common Info MAC addr = "MACSTR"",
		MAC2STR(prMlInfo->aucMldAddr));
	pos += MAC_ADDR_LEN;

	if (ucMlCtrlPreBmp & ML_CTRL_LINK_ID_INFO_PRESENT) {
		prMlInfo->ucLinkId = *pos;
		DBGLOG(ML, INFO, "\tML common Info LinkID = %d ("MACSTR")",
			*pos, MAC2STR(paucBssId));
		pos += 1;
	}
	if (ucMlCtrlPreBmp & ML_CTRL_BSS_PARA_CHANGE_COUNT_PRESENT) {
		prMlInfo->ucBssParaChangeCount = *pos;
		DBGLOG(ML, INFO, "\tML common Info BssParaChangeCount = %d",
			*pos);
		pos += 1;
	}
	if (ucMlCtrlPreBmp & ML_CTRL_MEDIUM_SYN_DELAY_INFO_PRESENT) {
		/* todo: handle 2byte MEDIUM_SYN_DELAY_INFO_PRESENT */
 		kalMemCopy(&prMlInfo->u2MediumSynDelayInfo, pos, 2);
		DBGLOG(ML, INFO, "\tML common Info MediumSynDelayInfo = 0x%x",
			prMlInfo->u2MediumSynDelayInfo);
		pos += 2;
	}
	if (ucMlCtrlPreBmp & ML_CTRL_EML_CAPA_PRESENT) {
		kalMemCopy(&prMlInfo->u2EmlCap, pos, 2);
		DBGLOG(ML, INFO, "\tML common Info EML capa = 0x%x",
			prMlInfo->u2EmlCap);
		pos += 2;
	}
	if (ucMlCtrlPreBmp & ML_CTRL_MLD_CAPA_PRESENT) {
		kalMemCopy(&prMlInfo->u2MldCap, pos, 2);
		DBGLOG(ML, INFO, "\tML common Info MLD capa = 0x%x",
			prMlInfo->u2MldCap);
		pos += 2;
	}

	if (pos - prMlInfoIe->aucCommonInfo !=
			prMlInfo->ucCommonInfoLength) {
		prMlInfo->ucValid = FALSE;
		DBGLOG(ML, INFO,
			"invalid ML control len: real %d != expected %d",
			pos - prMlInfoIe->aucCommonInfo,
			prMlInfo->ucCommonInfoLength);
		return;
	} else {
		prMlInfo->ucValid = TRUE;
	}

	/* pos point to link info, recusive parse it */
	while (pos < end) {
		struct IE_MULTI_LINK_STA_CONTROL *prIePerStaProfile =
			(struct IE_MULTI_LINK_STA_CONTROL *)pos;
		const uint8_t *tail = pos + IE_SIZE(pos);
		struct STA_PROFILE *prStaProfile;
		uint8_t ucLinkId, ucStaInfoLen;
		uint16_t u2StaControl;

		if (prIePerStaProfile->ucSubID != SUB_IE_MLD_PER_STA_PROFILE ||
		    prMlInfo->ucLinkNum >= MLD_LINK_MAX)
			break;

		u2StaControl = prIePerStaProfile->u2StaCtrl;
		ucLinkId = (u2StaControl & ML_STA_CTRL_LINK_ID_MASK);

		prStaProfile = &prMlInfo->rStaProfiles[prMlInfo->ucLinkNum++];
		prStaProfile->ucLinkId = ucLinkId;
		prStaProfile->u2StaCtrl = u2StaControl;
		prStaProfile->ucComplete =
			u2StaControl & ML_STA_CTRL_COMPLETE_PROFILE;

		DBGLOG(ML, INFO, "\tLinkID=%d Ctrl=0x%x(%s) Total=%d",
			ucLinkId, u2StaControl,
			prStaProfile->ucComplete ? "COMPLETE" : "PARTIAL",
			prMlInfo->ucLinkNum);

		pos = prIePerStaProfile->aucStaInfo;
		ucStaInfoLen = *pos++;

		if (u2StaControl & ML_STA_CTRL_MAC_ADDR_PRESENT) {
 			COPY_MAC_ADDR(prStaProfile->aucLinkAddr, pos);
			DBGLOG(ML, INFO, "\tLinkID=%d, LinkAddr="MACSTR"",
				ucLinkId, MAC2STR(prStaProfile->aucLinkAddr));
			pos += MAC_ADDR_LEN;
		}
		if (u2StaControl & ML_STA_CTRL_BCN_INTV_PRESENT) {
			kalMemCopy(&prStaProfile->u2BcnIntv, pos, 2);
			DBGLOG(ML, INFO, "\tLinkID=%d, BCN_INTV = %d",
				ucLinkId, prStaProfile->u2BcnIntv);
			pos += 2;
		}
		if (u2StaControl & ML_STA_CTRL_DTIM_INFO_PRESENT) {
			kalMemCopy(&prStaProfile->u2DtimInfo, pos, 2);
			DBGLOG(ML, INFO, "\tLinkID=%d, DTIM_INFO = 0x%x",
				ucLinkId, prStaProfile->u2DtimInfo);
			pos += 2;
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
				prStaProfile->u2NstrBmp = *pos;
				DBGLOG(ML, INFO, "\tLinkID=%d, NSTR_BMP0=0x%x",
					ucLinkId, prStaProfile->u2NstrBmp);
				pos += 1;
			} else {
				kalMemCopy(&prStaProfile->u2NstrBmp, pos, 2);
				DBGLOG(ML, INFO, "\tLinkID=%d, NSTR_BMP1=0x%x",
					ucLinkId, prStaProfile->u2NstrBmp);
				pos += 2;
			}
		}

		if (ucStaInfoLen != pos - prIePerStaProfile->aucStaInfo) {
			DBGLOG(ML, WARN,
				"invalid STA info len: real %d != expected %d",
				pos - prIePerStaProfile->aucStaInfo,
				ucStaInfoLen);
			prMlInfo->ucLinkNum--;
		} else {
			WLAN_GET_FIELD_16(pos, &prStaProfile->u2CapInfo);
			pos += 2;

			/* (tail - pos) is length of STA Profile
			 * copy STA profile in Per-STA profile subelement.
			 */
			prStaProfile->ucIEbufLen = 0;
			if (tail - pos < sizeof(prStaProfile->aucIEbuf)) {
				DBGLOG(ML, INFO, "\tcopy sta profile len=%d\n",
					tail - pos);
				kalMemCopy(prStaProfile->aucIEbuf,
					pos, tail - pos);
				prStaProfile->ucIEbufLen = tail - pos;
			} else {
				DBGLOG(ML, WARN,
					"sta profile ie len too long %d!!\n",
					tail - pos);
			}
		}

		/* point to next Per-STA profile*/
		pos = tail;
	}
}

uint8_t beSameElement(uint8_t *ie1, uint8_t *ie2)
{
	if (!ie1 || !ie2)
		return FALSE;

	if (IE_ID(ie1) == IE_ID(ie2)) {
		if (IE_ID(ie1) == ELEM_ID_RESERVED) {
			if (IE_ID_EXT(ie1) == IE_ID_EXT(ie2))
				return TRUE;
		} else if (IE_ID(ie1) == ELEM_ID_VENDOR) {
			if (!kalMemCmp(WFA_IE(ie1)->aucOui,
				WFA_IE(ie2)->aucOui, 3) &&
			    WFA_IE(ie1)->ucOuiType == WFA_IE(ie2)->ucOuiType)
				return TRUE;
		} else {
			return TRUE;
		}
	}

	return FALSE;
}

uint8_t beNonInheritElement(uint8_t *ie, struct IE_NON_INHERITANCE *ninh_elem)
{
	uint8_t i, *cp;
	uint8_t *ninh = NULL, ninh_num = 0;
	uint8_t	*ninh_ext = NULL, ninh_ext_num = 0;

	if (!ninh_elem)
		return FALSE;

	cp = ninh_elem->aucList;
	ninh_num = *cp++;
	if (ninh_num > 0) {
		ninh = cp;
		cp += ninh_num;
	}
	ninh_ext_num = *cp++;
	if (ninh_ext_num > 0) {
		ninh_ext = cp;
		cp += ninh_ext_num;
	}

	if (IE_ID(ie) == ELEM_ID_RESERVED) {
		for (i = 0; i < ninh_ext_num; i++)
			if (ninh_ext[i] == IE_ID_EXT(ie))
				return TRUE;
	} else {
		for (i = 0; i < ninh_num; i++)
			if (ninh[i] == IE_ID(ie))
				return TRUE;
	}

	return FALSE;
}

int beParseProfile(uint8_t *ie, uint32_t len, uint8_t *prof,
	uint32_t prof_len, uint8_t *out[], uint8_t *out_num,
	int max_num, uint8_t fgParseMbss)
{
	struct IE_NON_TX_CAP *cap = NULL;
	struct IE_MBSSID_INDEX *idx = NULL;
	struct IE_NON_INHERITANCE *ninh = NULL;
    	uint8_t **prof_ies;
	uint8_t ie_count = 0, profile_count = 0;
	int8_t i, need_profile, need_add;
	uint16_t offset = 0;
	int ret = 0;

	prof_ies = kalMemAlloc(sizeof(uint8_t *) * max_num, VIR_MEM_TYPE);
	if (!prof_ies) {
		DBGLOG(ML, INFO, "no mem\n");
		ret = -1;
		goto done;
	}
	kalMemSet(prof_ies, 0, sizeof(uint8_t *) * max_num);

	/* Check profile IE for MBSSID index, and Nontransmitted cap */
	IE_FOR_EACH(prof, prof_len, offset) {
		switch (IE_ID(prof)) {
		case ELEM_ID_NON_TX_CAP:
			cap = NON_TX_CAP_IE(prof);
			break;
		case ELEM_ID_MBSSID_INDEX:
			idx = MBSSID_INDEX_IE(prof);
			break;
		case ELEM_ID_RESERVED:
			if (IE_ID_EXT(prof) == ELEM_EXT_ID_NON_INHERITANCE) {
				ninh = NON_INHERITANCE_IE(prof);
				break;
			}
			/* fallthrough */
		default:
			prof_ies[profile_count++] = prof;
			break;
		}
	}

	/*not a valid profile*/
	if (fgParseMbss &&
	   (cap == NULL || idx == NULL || idx->ucBSSIDIndex == 0)) {
		DBGLOG(ML, INFO, "mbss but with cap=%p, idx=%p\n", cap, idx);
		ret = -1;
	        goto done;
	}
	IE_FOR_EACH(ie, len, offset) {
		need_profile = FALSE;
		need_add = FALSE;

		if (beDupSkipIE(ie))
			continue;

		if (fgParseMbss) { /* mbss fast path */
			switch (IE_ID(ie)) {
			case ELEM_ID_TIM:
			case ELEM_ID_DS_PARAM_SET:
			case ELEM_ID_IBSS_PARAM_SET:
			case ELEM_ID_COUNTRY_INFO:
			case ELEM_ID_CH_SW_ANNOUNCEMENT:
			case ELEM_ID_EX_CH_SW_ANNOUNCEMENT:
			case ELEM_ID_WIDE_BAND_CHANNEL_SWITCH:
			case ELEM_ID_TX_PWR_ENVELOPE:
			case ELEM_ID_SUP_OPERATING_CLASS:
			case ELEM_ID_IBSS_DFS:
			case ELEM_ID_ERP_INFO:
			case ELEM_ID_HT_CAP:
			case ELEM_ID_HT_OP:
			case ELEM_ID_VHT_CAP:
			case ELEM_ID_VHT_OP:
			case ELEM_ID_S1G_CAP:
			case ELEM_ID_S1G_OP:
				need_add = TRUE;
				break;
			case ELEM_ID_RESERVED:
				/* Check Element ID Extension */
				switch (IE_ID_EXT(ie)) {
					case ELEM_EXT_ID_HE_CAP:
					case ELEM_EXT_ID_HE_OP:
					case ELEM_EXT_ID_HE_6G_BAND_CAP:
					case ELEM_EXT_ID_SR_PARAM:
					case ELEM_EXT_ID_BSS_COLOR_CHANGE:
					case ELEM_EXT_ID_EHT_OP:
					case ELEM_EXT_ID_EHT_CAPS:
						need_add = TRUE;
						break;
					default:
						need_profile = TRUE;
						break;
				}
				break;
			default:
				need_profile = TRUE;
				break;
			}
		} else {
			need_profile = TRUE;
		}

		if (need_add) {
			out[ie_count++] = ie;
		} else if (need_profile) {
			/*check if profile has same ie*/
			for (i = 0; i < profile_count; i++) {
				if (beSameElement(prof_ies[i], ie))
					break;
			}

			if (i < profile_count) {
				/*found ie in profile, use profile version*/
				out[ie_count++] = prof_ies[i];
				prof_ies[i] = NULL;
			} else { /*not found, use ie from transmitted beacon*/
				if (!beNonInheritElement(ie, ninh))
					out[ie_count++] = ie;
			}
		}

		if (ie_count == max_num) {
			ret = -1;
			goto done;
		}
	}

	/*check if the ie is in profile but not in transmitted beacon,
	should keep this ie */
	for (i = 0; i < profile_count && ie_count < max_num; i++) {
		if (prof_ies[i] != NULL && IE_ID(prof_ies[i])) {
		    out[ie_count++] = prof_ies[i];
		}
	}
done:
	kalMemFree(prof_ies, VIR_MEM_TYPE, sizeof(uint8_t *) * max_num);
	*out_num = ie_count;
	return ret;
}

int beDupMbssNonTxProfileImpl(struct ADAPTER *prAdapter,
	struct SW_RFB *prSrc, uint8_t *pucProf, uint8_t u2ProfLen,
	struct SW_RFB *prDst)
{
	int padding;
	uint8_t new_bssid[MAC_ADDR_LEN], lsb;
	struct WLAN_BEACON_FRAME *mgmt;
	struct IE_MBSSID_INDEX *idx = NULL;
	struct IE_MBSSID *mbss = NULL;
	struct IE_NON_TX_CAP *cap = NULL;
	uint8_t i, ie_count, *ie, *ies[MAX_DUP_IE_COUNT], *pos;
	size_t len;

	padding = sortGetPayloadOffset(prAdapter, prSrc->pvHeader);
	mgmt = (struct WLAN_BEACON_FRAME *)prSrc->pvHeader;
	ie = prSrc->pvHeader + padding;
	len = prSrc->u2PacketLen - padding;

	if (beParseProfile(ie, len, pucProf, u2ProfLen,
		ies, &ie_count, MAX_DUP_IE_COUNT, TRUE) < 0)
		return -1;

	mbss = (struct IE_MBSSID *) kalFindIeExtIE(ELEM_ID_MBSSID, 0,
		ie, len);
	idx = (struct IE_MBSSID_INDEX *) kalFindIeExtIE(ELEM_ID_MBSSID_INDEX, 0,
		pucProf, u2ProfLen);
	cap = (struct IE_NON_TX_CAP *) kalFindIeExtIE(ELEM_ID_NON_TX_CAP, 0,
		pucProf, u2ProfLen);

	/* calculate new BSSID */
	COPY_MAC_ADDR(new_bssid, &mgmt->aucBSSID[0]);
	lsb = new_bssid[5] & ((1 << mbss->ucMaxBSSIDIndicator) - 1);
	new_bssid[5] &= ~((1 << mbss->ucMaxBSSIDIndicator) - 1);
	new_bssid[5] |= (lsb + idx->ucBSSIDIndex) &
	              ((1 << mbss->ucMaxBSSIDIndicator) - 1);

	DBGLOG(ML, INFO, "MBSS new mac: "MACSTR "\n", MAC2STR(new_bssid));

	/* compose RXD, mac header, payload(fixed field)*/
	nicRxCopyRFB(prAdapter, prDst, prSrc);
	pos = (uint8_t *)prDst->pvHeader;
	mgmt = (struct WLAN_BEACON_FRAME *)prDst->pvHeader;
	COPY_MAC_ADDR(mgmt->aucSrcAddr, new_bssid);
	COPY_MAC_ADDR(mgmt->aucBSSID, new_bssid);
	mgmt->u2CapInfo = cap->u2Cap;

	/* compose IE */
	for (i = 0; i < ie_count; i++) {
		len = IE_SIZE(ies[i]);
		kalMemCopy(pos + padding, ies[i], len);

		/* replace dtim count and dtim period of the MBSS to the TIM */
		if (IE_ID(ies[i]) == ELEM_ID_TIM && IE_LEN(idx) == 3) {
			struct IE_TIM *tmp = (struct IE_TIM *)(pos + padding);

			tmp->ucDTIMCount = idx->ucDtimCount;
			tmp->ucDTIMPeriod = idx->ucDtimPeriod;
		}
		padding += len;
	}

	prDst->u2PacketLen = padding;
	prDst->u2RxByteCount = ((uint8_t *)prDst->pvHeader) +
		padding - prDst->pucRecvBuff;

	return 0;
}

struct SW_RFB * beDupMbssNonTxProfile(struct ADAPTER *prAdapter,
	struct SW_RFB *prSrc)
{
	struct QUE tmp, *que = &tmp;
	uint8_t *pucSubIE, *pucIE;
	uint16_t u2IELen, u2SubIElen, u2SubOffset;
	struct IE_MBSSID *mbss;
	struct SW_RFB *rfb;
	uint8_t ret;
	int offset = sortGetPayloadOffset(prAdapter, prSrc->pvHeader);

	if (offset < 0)
		return NULL;

	QUEUE_INITIALIZE(que);
	pucIE = prSrc->pvHeader + offset;
	u2IELen = prSrc->u2PacketLen - offset;
	IE_FOR_EACH(pucIE, u2IELen, offset) {
		if (IE_ID(pucIE) != ELEM_ID_MBSSID)
			continue;

		mbss = (struct IE_MBSSID *)pucIE;
		pucSubIE = mbss->ucSubelements;
		u2SubIElen = IE_SIZE(mbss) - sizeof(struct IE_MBSSID);
		IE_FOR_EACH(pucSubIE, u2SubIElen, u2SubOffset) {
			if (IE_ID(pucSubIE) != NON_TX_BSSID_PROFILE)
				continue;

			rfb = nicRxAcquireRFB(prAdapter, 1);
			if (!rfb)
				break;

			ret = beDupMbssNonTxProfileImpl(prAdapter,
				prSrc, pucSubIE + 2, IE_LEN(pucSubIE), rfb);
			if (ret == WLAN_STATUS_SUCCESS) {
				QUEUE_INSERT_TAIL(que, &rfb->rQueEntry);
			} else
				nicRxReturnRFB(prAdapter, rfb);
		}
	}

	return (struct SW_RFB *) QUEUE_GET_HEAD(que);
}

uint8_t * beHandleRnrMldParam(IN uint8_t *ie,
			      IN struct MULTI_LINK_INFO *prMlInfo)
{
	struct NEIGHBOR_AP_INFO_FIELD *prNeighborAPInfoField =
		(struct NEIGHBOR_AP_INFO_FIELD *)ie;
	uint8_t i, j;
	uint8_t ucMldParamOffset, ucMldId, ucMldLinkId, ucBssParamChangeCount;
	uint16_t u2TbttInfoCount, u2TbttInfoLength;
	uint32_t u4MldParam = 0;
	uint8_t band;
	struct STA_PROFILE *prProfile;

	/* get channel number for this neighborAPInfo */
	scanOpClassToBand(prNeighborAPInfoField->ucOpClass, &band);
	u2TbttInfoCount = ((prNeighborAPInfoField->u2TbttInfoHdr &
				TBTT_INFO_HDR_COUNT)
				>> TBTT_INFO_HDR_COUNT_OFFSET)
				+ 1;
	u2TbttInfoLength = (prNeighborAPInfoField->u2TbttInfoHdr &
				TBTT_INFO_HDR_LENGTH)
				>> TBTT_INFO_HDR_LENGTH_OFFSET;

	DBGLOG(ML, INFO, "dump RNR AP info field\n");
	dumpMemory8(ie, 4 + u2TbttInfoCount * u2TbttInfoLength);

	for (i = 0; i < u2TbttInfoCount; i++) {
		j = i * u2TbttInfoLength;

		switch (u2TbttInfoLength) {
			/* 10: Neighbor AP TBTT Offset + BSSID + MLD Parameter
			 */
			case 10:
				ucMldParamOffset = 7;
				break;
			/* 16: Neighbor AP TBTT Offset + BSSID + Short SSID +
			 * BSS parameters + 20MHz PSD + MLD Parameter */
			case 16 ... 255:
				ucMldParamOffset = 13;
				break;
			default:
			/* only handle neighbor AP info that MLD parameter
			 * and BSSID both exist*/
				continue;
		}

		DBGLOG(ML, INFO, "RnrIe[%x][" MACSTR "]\n", i,
			MAC2STR(&prNeighborAPInfoField->aucTbttInfoSet[j + 1]));

		/* Directly copy 4 bytes content, but MLD param is only 3 bytes
		 * actually. We will only use 3 bytes content.
		 */
		kalMemCopy(&u4MldParam, &prNeighborAPInfoField->
			aucTbttInfoSet[j + ucMldParamOffset],
			sizeof(u4MldParam));
		ucMldId = (u4MldParam & MLD_PARAM_MLD_ID_MASK);
		ucMldLinkId = (u4MldParam & MLD_PARAM_LINK_ID_MASK) >>
			MLD_PARAM_LINK_ID_SHIFT;
		ucBssParamChangeCount =
			(u4MldParam & MLD_PARAM_BSS_PARAM_CHANGE_COUNT_MASK) >>
			MLD_PARAM_BSS_PARAM_CHANGE_COUNT_SHIFT;

		DBGLOG(ML, INFO,
			"MldId=%d, MldLinkId=%d, BssParChangeCount=%d\n",
			ucMldId, ucMldLinkId, ucBssParamChangeCount);

		for (j = 0; j < prMlInfo->ucLinkNum; j++) {
			prProfile = &prMlInfo->rStaProfiles[j];
			if (prProfile->ucLinkId == ucMldLinkId)
				break;
		}

		if (j >= prMlInfo->ucLinkNum) {
			DBGLOG(ML, WARN, "invalid link_id: %d", ucMldLinkId);
			continue;
		}

		switch (band) {
			case KAL_BAND_2GHZ:
				prProfile->rChnlInfo.eBand = BAND_2G4;
				break;
			case KAL_BAND_5GHZ:
				prProfile->rChnlInfo.eBand = BAND_5G;
				break;
#if (CFG_SUPPORT_WIFI_6G == 1)
			case KAL_BAND_6GHZ:
				prProfile->rChnlInfo.eBand = BAND_6G;
				break;
#endif
			default:
				DBGLOG(ML, WARN, "unsupported band: %d\n",
					band);
			break;
		}

		prProfile->rChnlInfo.ucChannelNum =
			prNeighborAPInfoField->ucChannelNum;

		prProfile->rChnlInfo.ucChnlBw =
			rlmOpClassToBandwidth(prNeighborAPInfoField->ucOpClass);

		prProfile->rChnlInfo.u4CenterFreq1 = 0;
		prProfile->rChnlInfo.u4CenterFreq2 = 0;
		DBGLOG(ML, INFO,
			"link_id:%d, op:%d, rfband:%d, ch:%d, bw:%d, s1:%d, s2:%d\n",
			prProfile->ucLinkId,
			prNeighborAPInfoField->ucOpClass,
			prProfile->rChnlInfo.eBand,
			prProfile->rChnlInfo.ucChannelNum,
			prProfile->rChnlInfo.ucChnlBw,
			prProfile->rChnlInfo.u4CenterFreq1,
			prProfile->rChnlInfo.u4CenterFreq2);
	}

	return ie + 4 + u2TbttInfoCount * u2TbttInfoLength;
}

uint32_t beDupMldStaProfileImpl(struct ADAPTER *prAdapter, struct SW_RFB *prDst,
	struct SW_RFB *prSrc, struct STA_PROFILE *prSta,
	struct STA_RECORD *prStaRec)
{
	int padding;
	struct WLAN_MAC_MGMT_HEADER *mgmt;
	uint8_t i, ie_count, *ie, *ies[MAX_DUP_IE_COUNT], *pos;
	size_t len;
	uint16_t fctrl;

	padding = sortGetPayloadOffset(prAdapter, prSrc->pvHeader);
	mgmt = (struct WLAN_MAC_MGMT_HEADER *)prSrc->pvHeader;
	fctrl = mgmt->u2FrameCtrl & MASK_FRAME_TYPE;
	ie = prSrc->pvHeader + padding;
	len = prSrc->u2PacketLen - padding;

	if (beParseProfile(ie, len, prSta->aucIEbuf, prSta->ucIEbufLen,
		ies, &ie_count, MAX_DUP_IE_COUNT, FALSE) < 0)
		return WLAN_STATUS_INVALID_PACKET;

	/* compose RXD, mac header, payload(fixed field)*/
	nicRxCopyRFB(prAdapter, prDst, prSrc);
	pos = (uint8_t *)prDst->pvHeader;
	mgmt = (struct WLAN_MAC_MGMT_HEADER *)prDst->pvHeader;
	COPY_MAC_ADDR(mgmt->aucSrcAddr, prSta->aucLinkAddr);

	DBGLOG(ML, INFO, "header total %d ies\n", ie_count);
	dumpMemory8(pos, padding);

	/* compose IE */
	for (i = 0; i < ie_count; i++) {
		dumpMemory8(ies[i], IE_SIZE(ies[i]));
		kalMemCopy(pos + padding, ies[i], IE_SIZE(ies[i]));
		padding += IE_SIZE(ies[i]);
	}

	DBGLOG(ML, INFO, "total len=%d\n", padding);

	/* copy common info for scan result, don't include per sta profile other
	 * scanProcessBeaconAndProbeResp will enter infinite loop
	 */
	if (fctrl == MAC_FRAME_PROBE_RSP || fctrl == MAC_FRAME_BEACON) {
		struct IE_MULTI_LINK_CONTROL *src, *dst;
		uint8_t offset = 0;

		src = (struct IE_MULTI_LINK_CONTROL *)
			kalFindIeExtIE(ELEM_ID_RESERVED,
			ELEM_EXT_ID_MLD, ie, len);
		dst = (struct IE_MULTI_LINK_CONTROL *)(pos + padding);

		DBGLOG(ML, INFO, "src=%p dst=%p\n", src, dst);

		/* fixed field, 1:common info len, 6:mac addr*/
		kalMemCopy(dst, src, sizeof(*src) + 1 + MAC_ADDR_LEN);
		offset += 1 + MAC_ADDR_LEN;

		if (BE_IS_ML_CTRL_PRESENCE_LINK_ID(src->u2Ctrl)) {
			kalMemCopy(dst->aucCommonInfo + offset,
				&prSta->ucLinkId, 1);
			offset += 1;
		}
		if (BE_IS_ML_CTRL_PRESENCE_BSS_PARA_CHANGE_COUNT(src->u2Ctrl)) {
			kalMemCopy(dst->aucCommonInfo + offset,
				src->aucCommonInfo + offset, 1);
			offset += 1;
		}
		if (BE_IS_ML_CTRL_PRESENCE_MEDIUM_SYN_DELAY_INFO(src->u2Ctrl)) {
			kalMemCopy(dst->aucCommonInfo + offset,
				src->aucCommonInfo + offset, 2);
			offset += 2;
		}
		if (BE_IS_ML_CTRL_PRESENCE_EML_CAP(src->u2Ctrl)) {
			kalMemCopy(dst->aucCommonInfo + offset,
				src->aucCommonInfo + offset, 2);
			offset += 2;

		}
		if (BE_IS_ML_CTRL_PRESENCE_MLD_CAP(src->u2Ctrl)) {
			kalMemCopy(dst->aucCommonInfo + offset,
				src->aucCommonInfo + offset, 2);
			offset += 2;
		}

		dst->ucLength = offset + sizeof(struct IE_MULTI_LINK_CONTROL) -
				ELEM_HDR_LEN;

		DBGLOG(ML, INFO, "src ml len=%d\n", IE_SIZE(src));
		dumpMemory8((uint8_t *)src, IE_SIZE(src));
 		DBGLOG(ML, INFO, "dst ml len=%d\n", IE_SIZE(dst));
		dumpMemory8((uint8_t *)dst, IE_SIZE(dst));

		padding += IE_SIZE(dst);
	}

	prDst->u2PacketLen = padding;
	prDst->u2RxByteCount = ((uint8_t *)prDst->pvHeader) +
		padding - prDst->pucRecvBuff;
	prDst->ucChnlNum = prSta->rChnlInfo.ucChannelNum;
	prDst->eRfBand = prSta->rChnlInfo.eBand;

	if (prStaRec) {
		struct BSS_INFO *bss =
			GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);

		if (fctrl == MAC_FRAME_ASSOC_REQ ||
		    fctrl == MAC_FRAME_REASSOC_REQ)
			COPY_MAC_ADDR(mgmt->aucBSSID, bss->aucOwnMacAddr);
		else if (fctrl == MAC_FRAME_ASSOC_RSP ||
			 fctrl == MAC_FRAME_REASSOC_RSP)
			COPY_MAC_ADDR(mgmt->aucBSSID, bss->aucBSSID);
		COPY_MAC_ADDR(mgmt->aucDestAddr, bss->aucOwnMacAddr);
		prDst->ucWlanIdx = prStaRec->ucWlanIndex;
		prDst->ucStaRecIdx = prStaRec->ucIndex;
		prDst->prStaRec = prStaRec;
	}

	DBGLOG(ML, INFO, "dump duplicated SwRFB, len=%d, chnl=%d, band=%d\n",
		padding, prDst->ucChnlNum, prDst->eRfBand);
	dumpMemory8(pos, padding);

	return WLAN_STATUS_SUCCESS;
}

struct SW_RFB * beDupScanMldStaProfile(struct ADAPTER *prAdapter,
				  struct SW_RFB *prSrc)
{
	struct QUE tmp, *que = &tmp;
	struct STA_PROFILE *sta;
	struct MULTI_LINK_INFO parse, *info = &parse;
	struct IE_RNR *rnr;
	struct WLAN_MAC_MGMT_HEADER *mgmt =
		(struct WLAN_MAC_MGMT_HEADER *)prSrc->pvHeader;
	struct SW_RFB *rfb;
	int offset = sortGetPayloadOffset(prAdapter, prSrc->pvHeader);
	uint8_t i, ret, *pos;
	const uint8_t *ml;

	if (offset < 0)
		return NULL;

	QUEUE_INITIALIZE(que);

	ml = kalFindIeExtIE(ELEM_ID_RESERVED,
		ELEM_EXT_ID_MLD, prSrc->pvHeader + offset,
		prSrc->u2PacketLen - offset);
	if (!ml)
		return NULL;

	/* parsing rnr & ml */
	kalMemSet(info, 0, sizeof(*info));
	beParseMldElement(info, ml, mgmt->aucBSSID, "Duplicate Scan SwRFB");

	if (info->ucLinkNum == 0) {
		DBGLOG(ML, INFO, "beacon but no per sta profile\n");
		return NULL;
	}

	/* get channel info from rnr if exist*/
	rnr = (struct IE_RNR *) kalFindIeExtIE(ELEM_ID_RNR, 0,
		prSrc->pvHeader + offset, prSrc->u2PacketLen - offset);
	if (!rnr) {
		DBGLOG(ML, INFO, "beacon but no rnr\n");
		return NULL;
	}

	pos = rnr->aucInfoField;
	do {
		pos = beHandleRnrMldParam(pos, info);
	} while (pos < ((uint8_t *)rnr) + IE_SIZE(rnr));

	for (i = 0; i < info->ucLinkNum; i++) {
		sta = &info->rStaProfiles[i];

		/* skip if no complete info or already exist */
		if (!sta->ucComplete ||
		    EQUAL_MAC_ADDR(mgmt->aucBSSID, sta->aucLinkAddr) ||
		    scanSearchExistingBssDescWithSsid(
			prAdapter,
			BSS_TYPE_INFRASTRUCTURE,
			sta->aucLinkAddr,
			sta->aucLinkAddr,
			FALSE, NULL))
			continue;

		rfb = nicRxAcquireRFB(prAdapter, 1);
		if (!rfb)
			break;

		DBGLOG(ML, INFO, "Dup SwRfb for id=%d addr=" MACSTR "\n",
			sta->ucLinkId, MAC2STR(sta->aucLinkAddr));
		ret = beDupMldStaProfileImpl(prAdapter, rfb, prSrc,
			sta, NULL);
		if (ret == 0) {
			QUEUE_INSERT_TAIL(que, &rfb->rQueEntry);
		} else {
			nicRxReturnRFB(prAdapter, rfb);
		}
	}

	return (struct SW_RFB *) QUEUE_GET_HEAD(que);
}

void beProcessBeaconAndProbeResp(
		struct ADAPTER *prAdapter, struct SW_RFB *prSrc)
{
	struct QUE tmp, *que = &tmp;
	struct SW_RFB *rfb;

	QUEUE_INITIALIZE(que);

#if CFG_SUPPORT_802_11V_MBSSID && !CFG_SUPPORT_802_11V_MBSSID_OFFLOAD
	rfb = beDupMbssNonTxProfile(prAdapter, prSrc);
	QUEUE_INSERT_TAIL_ALL(que, rfb);
#endif

	rfb = beDupScanMldStaProfile(prAdapter, prSrc);
	QUEUE_INSERT_TAIL_ALL(que, rfb);

	while(QUEUE_IS_NOT_EMPTY(que)) {
		QUEUE_REMOVE_HEAD(que, rfb, struct SW_RFB *);
		scanProcessBeaconAndProbeResp(prAdapter, rfb);
	}
}

struct SW_RFB * beDuplicateAssocSwRfb(struct ADAPTER *prAdapter,
	struct SW_RFB *prSrc, struct STA_RECORD *prStaRec)
{
	struct BSS_INFO *bss;
	struct STA_PROFILE *sta;
	struct MULTI_LINK_INFO parse, *info = &parse;
	struct SW_RFB *rfb = NULL;
	struct WLAN_MAC_MGMT_HEADER *mgmt =
		(struct WLAN_MAC_MGMT_HEADER *)prSrc->pvHeader;
	int offset = sortGetPayloadOffset(prAdapter, prSrc->pvHeader);
	const uint8_t *ml;
	uint8_t i;
	uint32_t ret;

	bss = GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);
	ml = kalFindIeExtIE(ELEM_ID_RESERVED, ELEM_EXT_ID_MLD,
		prSrc->pvHeader + offset, prSrc->u2PacketLen - offset);
	if (!ml) {
		DBGLOG(ML, INFO, "AA but no ml ie\n");
		goto fail;
	}

	kalMemSet(info, 0, sizeof(*info));
	beParseMldElement(info, ml, mgmt->aucBSSID, "Duplicate ASSOC SwRFB");

	for (i = 0; i < info->ucLinkNum; i++) {
		sta = &info->rStaProfiles[i];
		if (sta->ucLinkId == prStaRec->ucLinkIndex &&
		    EQUAL_MAC_ADDR(sta->aucLinkAddr, prStaRec->aucMacAddr)) {
			break;
		}
	}

	if (i >= info->ucLinkNum) {
		DBGLOG(ML, INFO, "AA but no matched id\n");
		goto fail;
	}

	/* skip if no complete info */
	if (!sta->ucComplete) {
		DBGLOG(ML, INFO, "not complete\n");
		goto fail;
	}

	sta->rChnlInfo.ucChannelNum = bss->ucPrimaryChannel;
	sta->rChnlInfo.eBand = bss->eBand;

	rfb = nicRxAcquireRFB(prAdapter, 1);
	if (!rfb) {
		DBGLOG(ML, INFO, "no rfb\n");
		goto fail;
	}

	ret = beDupMldStaProfileImpl(prAdapter, rfb, prSrc,
		sta, prStaRec);
	if (ret == WLAN_STATUS_SUCCESS)
		return rfb;
fail:
	nicRxReturnRFB(prAdapter, rfb);
	return NULL;
}

void mldBssDump(struct ADAPTER *prAdapter)
{
	struct MLD_BSS_INFO *prMldBssInfo;
	uint8_t i = 0;

	DBGLOG(ML, INFO, "========== START ==========\n");

	for (i = 0; i < ARRAY_SIZE(prAdapter->aprMldBssInfo); i++) {
		struct BSS_INFO *prBssInfo;
		struct LINK *prBssList;

		prMldBssInfo = &prAdapter->aprMldBssInfo[i];

		if (!prMldBssInfo->fgIsInUse)
			continue;

		DBGLOG(ML, INFO, "[%d] om:%d, group:%d, om_remap:%d, mld_mac:" MACSTR "\n",
			i,
			prMldBssInfo->ucOmacIdx,
			prMldBssInfo->ucGroupMldId,
			prMldBssInfo->ucOmRemapIdx,
			prMldBssInfo->aucOwnMldAddr);

		DBGLOG(ML, INFO, "\tBss list:\n");
		prBssList = &prMldBssInfo->rBssList;
		LINK_FOR_EACH_ENTRY(prBssInfo, prBssList, rLinkEntryMld,
		    struct BSS_INFO)
			cnmDumpBssInfo(prAdapter, prBssInfo->ucBssIndex);

		DBGLOG(ML, INFO, "\n");
	}

	DBGLOG(ML, INFO, "========== END ==========\n");
}

void mldBssUpdateMldAddr(
	IN struct ADAPTER *prAdapter,
	IN struct MLD_BSS_INFO *prMldBssInfo,
	IN const uint8_t *paucBssId)
{
	if (!prAdapter || !prMldBssInfo)
		return;

	DBGLOG(ML, INFO,
		"prMldBssInfo: %d, macAddr: " MACSTR "\n",
		prMldBssInfo->ucGroupMldId, paucBssId);

	COPY_MAC_ADDR(prMldBssInfo->aucOwnMldAddr, paucBssId);
}

void mldBssUpdateMldAddrByMainBss(
	IN struct ADAPTER *prAdapter,
	IN struct MLD_BSS_INFO *prMldBssInfo)
{
	struct BSS_INFO *prBssInfo = NULL;

	if (!prAdapter || !prMldBssInfo ||
	    LINK_IS_EMPTY(&prMldBssInfo->rBssList))
		return;

	prBssInfo = LINK_PEEK_HEAD(&(prMldBssInfo->rBssList),
					struct BSS_INFO,
					rLinkEntryMld);
	mldBssUpdateMldAddr(prAdapter,
		prMldBssInfo, prBssInfo->aucOwnMacAddr);
}

int8_t mldBssRegister(struct ADAPTER *prAdapter,
	struct MLD_BSS_INFO *prMldBssInfo,
	struct BSS_INFO *prBssInfo)
{
	struct LINK *prBssList = NULL;

	if (!prMldBssInfo)
		return -EINVAL;

	prBssList = &prMldBssInfo->rBssList;

	DBGLOG(ML, INFO, "prMldBssInfo: %d, prBssInfo: %d\n",
		prMldBssInfo->ucGroupMldId, prBssInfo->ucBssIndex);

	prBssInfo->ucGroupMldId = prMldBssInfo->ucGroupMldId;
	LINK_INSERT_TAIL(prBssList, &prBssInfo->rLinkEntryMld);

	mldBssUpdateMldAddrByMainBss(prAdapter, prMldBssInfo);

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

	DBGLOG(ML, INFO, "prMldBssInfo: %d, prBss: %d\n",
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
		DBGLOG(ML, INFO, "ucGroupMldId: %d, ucOmRemapIdx: %d\n",
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

	DBGLOG(ML, INFO, "ucGroupMldId: %d\n",
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

	if (!prBssInfo || prBssInfo->ucGroupMldId == MLD_GROUP_NONE)
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
	DBGLOG(ML, INFO, "\n");
	kalMemZero(prAdapter->aprMldBssInfo,
		sizeof(prAdapter->aprMldBssInfo));
	return 0;
}

void mldBssUninit(struct ADAPTER *prAdapter)
{
	DBGLOG(ML, INFO, "\n");
}

void mldStarecDump(struct ADAPTER *prAdapter)
{
	uint8_t i = 0, j = 0;

	DBGLOG(ML, INFO, "========== START ==========\n");

	for (i = 0; i < ARRAY_SIZE(prAdapter->aprMldStarec); i++) {
		struct MLD_STA_RECORD *prMldStarec = &prAdapter->aprMldStarec[i];
		struct STA_RECORD *prCurrStarec;
		struct LINK *prStarecList;

		if (!prMldStarec->fgIsInUse)
			continue;


		DBGLOG(ML, INFO, "[%d] pri:%d, sec:%d, setup:%d, emlmr:%d, emlsr:%d, str:[0x%x,0x%x,0x%x], mac:" MACSTR "\n",
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

		DBGLOG(ML, INFO, "\tRX pkt count:\n");
		for (j = 0; j < ARRAY_SIZE(prMldStarec->aucRxPktCnt); j++)
			DBGLOG(ML, INFO, "\t\tband%d:0x%llx\n",
				j, prMldStarec->aucRxPktCnt[j]);

		DBGLOG(ML, INFO, "\tSta list:\n");
		prStarecList = &prMldStarec->rStarecList;
		LINK_FOR_EACH_ENTRY(prCurrStarec, prStarecList, rLinkEntryMld,
		    struct STA_RECORD)
			cnmDumpStaRec(prAdapter, prCurrStarec->ucIndex);

		DBGLOG(ML, INFO, "\n");
	}
	DBGLOG(ML, INFO, "========== END ==========\n");
}

struct MLD_STA_RECORD *mldStarecGetByAddr(struct ADAPTER *prAdapter,
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
	struct STA_RECORD *prCurrStarec;
	struct LINK *prStarecList = NULL;
	const uint8_t aucZeroMacAddr[] = NULL_MAC_ADDR;

	if (!prStarec->fgIsInUse) {
		DBGLOG(ML, WARN, "starec(idx=%d, widx=%d) not in use",
			prStarec->ucIndex, prStarec->ucWlanIndex);
		return -EINVAL;
	}

	if (EQUAL_MAC_ADDR(aucZeroMacAddr, prStarec->aucMldAddr))
		return -EINVAL;

	prMldStarec = mldStarecGetByAddr(prAdapter, prStarec->aucMldAddr);
	if (!prMldStarec) {
		rStatus = mldStarecAlloc(prAdapter, &prMldStarec, prStarec->aucMldAddr);
		if (rStatus)
			goto exit;
	}

	prStarecList = &prMldStarec->rStarecList;

	LINK_FOR_EACH_ENTRY(prCurrStarec, prStarecList, rLinkEntryMld,
	    struct STA_RECORD) {
		if (prStarec == prCurrStarec) {
			DBGLOG(ML, WARN,
				"starec(%d) already in mld_starec(id=%d, "
				MACSTR ")\n",
				prStarec->ucIndex, prMldStarec->ucIdx,
				MAC2STR(prMldStarec->aucPeerMldAddr));
			rStatus = -EINVAL;
			goto exit;
		}
	}

	if (LINK_IS_EMPTY(prStarecList))
		prMldStarec->u2PrimaryMldId = prStarec->ucWlanIndex;
	else if (prStarecList->u4NumElem == 1)
		prMldStarec->u2SecondMldId = prStarec->ucWlanIndex;

	prStarec->ucMldStaIndex = prMldStarec->ucIdx;
	LINK_INSERT_TAIL(prStarecList, &prStarec->rLinkEntryMld);

	DBGLOG(ML, INFO,
		"prMldStarec: %d, prStarec: %d, widx: %d, bss: %d, pri_mld: %d, sec_mld: %d, mld_mac: "
		MACSTR "\n",
		prMldStarec->ucIdx,
		prStarec->ucIndex,
		prStarec->ucWlanIndex,
		prStarec->ucBssIndex,
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

#ifdef CFG_SUPPORT_UNIFIED_COMMAND
	nicUniCmdMldStaTeardown(prAdapter, prStarec);
#endif

	prStarecList = &prMldStarec->rStarecList;

	DBGLOG(ML, INFO, "prMldStarec: %d, prStarec: %d\n",
		prMldStarec->ucIdx, prStarec->ucIndex);

	LINK_FOR_EACH_ENTRY(prCurrStarec, prStarecList, rLinkEntryMld,
	    struct STA_RECORD) {
		if (prStarec != prCurrStarec)
			continue;

		prCurrStarec->ucMldStaIndex = MLD_GROUP_NONE;
		LINK_REMOVE_KNOWN_ENTRY(prStarecList, &prCurrStarec->rLinkEntryMld);
		break;
	}

	if (LINK_IS_EMPTY(prStarecList))
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
#if defined(BELLWETHER) || defined(MT7990)
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
		DBGLOG(ML, INFO, "ucIdx: %d, aucMacAddr: " MACSTR ", str[0x%x,0x%x,0x%x]\n",
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
	DBGLOG(ML, INFO, "prMldStarec: %d\n",
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

	DBGLOG(ML, INFO, "prMldStarec: %d, ucIdx: %d\n",
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

	if (ucHwBandIdx >= ENUM_BAND_NUM)
		return;

	mld_sta->aucRxPktCnt[ucHwBandIdx]++;
}

int8_t mldStarecInit(struct ADAPTER *prAdapter)
{
	DBGLOG(ML, INFO, "\n");
	kalMemZero(prAdapter->aprMldStarec, sizeof(prAdapter->aprMldStarec));
	return 0;
}

void mldStarecUninit(struct ADAPTER *prAdapter)
{
	DBGLOG(ML, INFO, "\n");
}

struct BSS_INFO *mldGetBssInfoByLinkID(
		struct ADAPTER *prAdapter,
		struct MLD_BSS_INFO *prMldBssInfo,
		uint8_t ucLinkIndex,
		uint8_t fgPeerSta
		)
{
	struct BSS_INFO *prCurrBssInfo = NULL;
	struct LINK *prBssList = NULL;
	struct STA_RECORD *prStaRecOfAP = NULL;

	if ((!prAdapter) || (!prMldBssInfo))
		return NULL;

	prBssList = &prMldBssInfo->rBssList;

	LINK_FOR_EACH_ENTRY(prCurrBssInfo, prBssList,
			rLinkEntryMld,
			struct BSS_INFO) {

		if (fgPeerSta == TRUE) {
			/* Match with peer STA(AP)'s link ID */
			prStaRecOfAP = prCurrBssInfo->prStaRecOfAP;

			if ((prStaRecOfAP) &&
				(prStaRecOfAP->ucLinkIndex == ucLinkIndex))
				 return prCurrBssInfo;
		} else {
			/* Match with local STA(SAP)'s link ID */
			if (prCurrBssInfo->ucLinkIndex == ucLinkIndex)
				  return prCurrBssInfo;
		}
	}

	return NULL;
}

uint8_t mldIsMultiLinkFormed(struct ADAPTER *prAdapter,
			     struct STA_RECORD *prStaRec)
{
	struct MLD_STA_RECORD *mld_starec;

	mld_starec = mldStarecGetByStarec(prAdapter, prStaRec);
	return IS_MLD_STAREC_VALID(mld_starec);
}

#endif /* CFG_SUPPORT_802_11BE_MLO == 1 */
