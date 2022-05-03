/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

/*! \file   "p2p_link.c"
 *  \brief
 */

#include "precomp.h"

#if (CFG_SUPPORT_802_11BE_MLO == 1)

struct MLD_BSS_INFO *prP2pMldBssInfo;

struct MLD_BSS_INFO *p2pGetMldBssInfo(
	IN struct ADAPTER *prAdapter,
	IN struct P2P_ROLE_FSM_INFO *prP2pRoleFsmInfo)
{
	if (prP2pMldBssInfo == NULL) {
		DBGLOG(INIT, TRACE, "\n");
		mldBssAlloc(prAdapter, &prP2pMldBssInfo);
	}

	return prP2pMldBssInfo;
}

int8_t p2pMldBssInit(struct ADAPTER *prAdapter)
{
	DBGLOG(INIT, TRACE, "\n");

	mldBssAlloc(prAdapter, &prP2pMldBssInfo);

	return 0;
}

void p2MldBssUninit(struct ADAPTER *prAdapter)
{
	mldBssFree(prAdapter, prP2pMldBssInfo);
	prP2pMldBssInfo = NULL;
	DBGLOG(INIT, TRACE, "\n");
}


#endif /* CFG_SUPPORT_802_11BE_MLO == 1 */


#if (CFG_SUPPORT_802_11BE_MLO == 1)
void p2pLinkInitGCRole(IN struct ADAPTER *prAdapter)
{
	struct GL_P2P_INFO *prP2pInfo = NULL;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct P2P_ROLE_FSM_INFO *fsm =
		(struct P2P_ROLE_FSM_INFO *) NULL;
	uint8_t i;

	if (!prAdapter ||
		p2pGetMode() != RUNNING_P2P_DEV_MODE)
		return;

	prGlueInfo = prAdapter->prGlueInfo;

	for (i = 0;
		i < prAdapter->rWifiVar.ucMldLinkMax;
		i++) {
		prP2pInfo = prGlueInfo->prP2PInfo[i];
		if (prP2pInfo == NULL)
			continue;
		DBGLOG(INIT, TRACE, "\n");
		p2pRoleFsmInitImpl(prAdapter,
			i, i != P2P_MAIN_LINK_INDEX);
		kal_init_completion(&prP2pInfo->rStopApComp);

		fsm = P2P_ROLE_INDEX_2_ROLE_FSM_INFO(
			prAdapter,
			i);
		if (!fsm)
			continue;

		wlanBindBssIdxToNetInterface(
			prGlueInfo,
			fsm->ucBssIndex,
			(void *) wlanGetP2pNetDev(
			prGlueInfo,
			P2P_MAIN_LINK_INDEX));

		kalP2PSetCipher(
			prGlueInfo,
			prGlueInfo->prP2PInfo
			[P2P_MAIN_LINK_INDEX]->u4CipherPairwise,
			i);
	}
}

void p2pLinkUninitGCRole(IN struct ADAPTER *prAdapter)
{
	struct GL_P2P_INFO *prP2pInfo = NULL;
	struct P2P_ROLE_FSM_INFO *fsm =
		(struct P2P_ROLE_FSM_INFO *) NULL;
	uint8_t i;

	if (!prAdapter ||
		p2pGetMode() != RUNNING_P2P_DEV_MODE)
		return;

	for (i = 0;
		i < prAdapter->rWifiVar.ucMldLinkMax;
		i++) {
		DBGLOG(INIT, TRACE, "\n");

		prP2pInfo =
			prAdapter->prGlueInfo->prP2PInfo[i];
		if (prP2pInfo == NULL)
			continue;

		fsm = P2P_ROLE_INDEX_2_ROLE_FSM_INFO(
			prAdapter,
				i);
		if (!fsm)
			continue;

		wlanBindBssIdxToNetInterface(
			prAdapter->prGlueInfo,
			fsm->ucBssIndex,
			(void *) prP2pInfo->prDevHandler);

		p2pRoleFsmUninit(prAdapter, i);
	}
}
#endif

#if (CFG_SUPPORT_802_11BE_MLO == 1)
uint32_t p2pLinkProcessRxAuthReqFrame(
	IN struct ADAPTER *prAdapter,
	IN struct BSS_INFO *prBssInfo,
	IN struct STA_RECORD *prStaRec,
	IN struct SW_RFB *prSwRfb)
{
	struct WLAN_AUTH_FRAME *prAuthFrame = NULL;
	struct MULTI_LINK_INFO rMlInfo;
	struct MULTI_LINK_INFO *prMlInfo = &rMlInfo;
	uint8_t *pucIE;
	uint16_t u2IELength;
	const uint8_t *ml;
	uint16_t u2RxFrameCtrl;

	kalMemSet(prMlInfo, 0, sizeof(rMlInfo));

	if (!prSwRfb || !prBssInfo || !prStaRec || !prStaRec->fgIsInUse) {
		DBGLOG(AAA, WARN,
			"Invalid parameters swrfb=%p, bss=%p, starec=%p, used=%d, skip!\n",
			prSwRfb, prBssInfo, prStaRec,
			prStaRec ? prStaRec->fgIsInUse : -1);
		return WLAN_STATUS_FAILURE;
	}

	prAuthFrame = (struct WLAN_AUTH_FRAME *)prSwRfb->pvHeader;

	u2RxFrameCtrl = prAuthFrame->u2FrameCtrl;

	u2RxFrameCtrl &= MASK_FRAME_TYPE;

	if (u2RxFrameCtrl != MAC_FRAME_AUTH) {
		DBGLOG(AAA, WARN, "Incorrect frame type, failure!\n");
		return WLAN_STATUS_FAILURE;
	}

	/* sae auth frames are handled by hostapd, delay register until assoc */
	if (prAuthFrame->u2AuthAlgNum == AUTH_ALGORITHM_NUM_SAE) {
		DBGLOG(AAA, INFO, "auth_alg=SAE, handle ml ie when assoc\n");
		return WLAN_STATUS_SUCCESS;
	}

	u2IELength = prSwRfb->u2PacketLen -
		(uint16_t) OFFSET_OF(struct WLAN_AUTH_FRAME,
		aucInfoElem[0]);

	pucIE = prAuthFrame->aucInfoElem;

	ml = kalFindIeExtIE(ELEM_ID_RESERVED,
		ELEM_EXT_ID_MLD, pucIE, u2IELength);
	if (ml) {
		mldParseBasicMlIE(prMlInfo, ml, prAuthFrame->aucBSSID,
			u2RxFrameCtrl, "RxAuthReq");
	} else {
		DBGLOG(AAA, INFO, "no ml ie\n");
		return WLAN_STATUS_SUCCESS;
	}

	if (!prMlInfo->ucValid) {
		DBGLOG(AAA, INFO, "Invalid mld_info, reject!\n");
		return WLAN_STATUS_NOT_SUPPORTED;
	}

	/* assign correct id when assoc req is received*/
	mldStarecRegister(prAdapter, prStaRec,
		prMlInfo->aucMldAddr, MLD_GROUP_NONE);

	return WLAN_STATUS_SUCCESS;
}

uint32_t p2pLinkProcessRxAssocReqFrame(
	IN struct ADAPTER *prAdapter,
	IN struct BSS_INFO *prBssInfo,
	IN struct STA_RECORD *prStaRec,
	IN struct SW_RFB *prSwRfb)
{
	struct WLAN_ASSOC_REQ_FRAME *prFrame = NULL;
	struct MLD_STA_RECORD *prMldStarec = NULL;
	struct STA_RECORD *prCurr;
	struct LINK *prStarecList = NULL;
	struct MULTI_LINK_INFO rMlInfo;
	struct MULTI_LINK_INFO *prMlInfo = &rMlInfo;
	uint8_t *pucIE;
	uint16_t u2IELength;
	const uint8_t *ml;
	uint16_t u2RxFrameCtrl;
	uint8_t i;

	kalMemSet(prMlInfo, 0, sizeof(rMlInfo));

	if (!prAdapter || !prSwRfb || !prBssInfo) {
		DBGLOG(AAA, WARN, "Invalid parameters, ignore pkt!\n");
		return WLAN_STATUS_FAILURE;
	}

	prFrame = (struct WLAN_ASSOC_REQ_FRAME *)prSwRfb->pvHeader;

	u2RxFrameCtrl = prFrame->u2FrameCtrl;

	u2RxFrameCtrl &= MASK_FRAME_TYPE;

	if (u2RxFrameCtrl != MAC_FRAME_ASSOC_REQ &&
		u2RxFrameCtrl != MAC_FRAME_REASSOC_REQ) {
		DBGLOG(AAA, WARN, "Incorrect frame type, ignore pkt!\n");
		return WLAN_STATUS_FAILURE;
	}

	if (u2RxFrameCtrl == MAC_FRAME_REASSOC_REQ) {
		u2IELength = prSwRfb->u2PacketLen -
		    (uint16_t)
		    OFFSET_OF(struct WLAN_REASSOC_REQ_FRAME,
		    aucInfoElem[0]);
		pucIE = ((struct WLAN_REASSOC_REQ_FRAME *)
			(prSwRfb->pvHeader))->aucInfoElem;
	} else {
		u2IELength = prSwRfb->u2PacketLen -
			(uint16_t)
			OFFSET_OF(struct WLAN_ASSOC_REQ_FRAME,
			aucInfoElem[0]);
		pucIE = ((struct WLAN_ASSOC_REQ_FRAME *)
			(prSwRfb->pvHeader))->aucInfoElem;
	}

	ml = kalFindIeExtIE(ELEM_ID_RESERVED,
		ELEM_EXT_ID_MLD, pucIE, u2IELength);

	if (ml) {
		mldParseBasicMlIE(prMlInfo, ml, prFrame->aucBSSID,
			u2RxFrameCtrl, "RxAssocReq");
	} else {
		DBGLOG(AAA, INFO, "no ml ie\n");
		return WLAN_STATUS_SUCCESS;
	}

	/* sae auth frames are handled by hostapd, delay register until assoc */
	if (prStaRec->ucAuthAlgNum == AUTH_ALGORITHM_NUM_SAE)
		mldStarecRegister(prAdapter, prStaRec,
			prMlInfo->aucMldAddr, MLD_GROUP_NONE);

	prMldStarec = mldStarecGetByAddr(prAdapter, prStaRec->aucMldAddr);
	if (!prMlInfo->ucValid || !prMldStarec) {
		DBGLOG(AAA, WARN, "Incorrect ml valid=%d, mldStarec=%p!\n",
			prMlInfo->ucValid, prMldStarec);
		return WLAN_STATUS_FAILURE;
	}

	prStarecList = &prMldStarec->rStarecList;
	for (i = 0; i < prMlInfo->ucLinkNum; i++) {
		struct STA_PROFILE *prProfiles =
			&prMlInfo->rStaProfiles[i];
		uint8_t found = FALSE;

		DBGLOG(AAA, INFO,
			"%d/%d profile: %d, mac: " MACSTR "\n",
			i + 1, prMlInfo->ucLinkNum, prProfiles->ucLinkId,
			MAC2STR(prProfiles->aucLinkAddr));

		LINK_FOR_EACH_ENTRY(prCurr, prStarecList,
				rLinkEntryMld, struct STA_RECORD) {
			if (EQUAL_MAC_ADDR(prCurr->aucMacAddr,
				prProfiles->aucLinkAddr)) {
				struct BSS_INFO *bss =
					GET_BSS_INFO_BY_INDEX(prAdapter,
					prCurr->ucBssIndex);

				DBGLOG(AAA, INFO,
					"[%d] bss: %d, mac: " MACSTR
					", bssid: " MACSTR "\n",
					prProfiles->ucLinkId,
					bss->ucBssIndex,
					MAC2STR(bss->aucOwnMacAddr),
					MAC2STR(bss->aucBSSID));
				prCurr->ucLinkIndex = prProfiles->ucLinkId;
				found = TRUE;
				break;
			}
		}
		/* starec not found */
		if (!found) {
			struct BSS_INFO *bss =
				p2pGetLinkBssInfo(prAdapter,
				p2pGetDefaultRoleFsmInfo(prAdapter,
				IFTYPE_P2P_GO),
				prProfiles->ucLinkId);

			if (bss == NULL) {
				DBGLOG(AAA, WARN, "wrong link id(%d)\n",
					prProfiles->ucLinkId);
				continue;
			}

			DBGLOG(AAA, INFO,
				"[%d] bss: %d, mac: " MACSTR
				", bssid: " MACSTR "\n",
				prProfiles->ucLinkId,
				bss->ucBssIndex,
				MAC2STR(bss->aucOwnMacAddr),
				MAC2STR(bss->aucBSSID));

			prCurr = cnmStaRecAlloc(prAdapter,
				STA_TYPE_P2P_GC,
				bss->ucBssIndex,
				prProfiles->aucLinkAddr);
			if (!prCurr) {
				DBGLOG(AAA, WARN,
					"StaRec Full. (%d)\n", CFG_STA_REC_NUM);
				continue;
			}

			prCurr->u2BSSBasicRateSet = bss->u2BSSBasicRateSet;
			prCurr->u2DesiredNonHTRateSet = RATE_SET_ERP_P2P;
			prCurr->u2OperationalRateSet = RATE_SET_ERP_P2P;
			prCurr->ucPhyTypeSet = PHY_TYPE_SET_802_11GN;
			nicTxUpdateStaRecDefaultRate(prAdapter, prCurr);
			cnmStaRecChangeState(prAdapter, prCurr, STA_STATE_1);
			prCurr->eStaType = STA_TYPE_P2P_GC;
			prCurr->u2StatusCode = STATUS_CODE_SUCCESSFUL;
			prCurr->ucJoinFailureCount = 0;

			/* ml link info */
			mldStarecRegister(prAdapter, prCurr,
				prMlInfo->aucMldAddr, prProfiles->ucLinkId);
		}
	}

	/* make sure all links are assigned link id */
	LINK_FOR_EACH_ENTRY(prCurr, prStarecList,
			rLinkEntryMld, struct STA_RECORD) {
		if (prCurr->ucLinkIndex == MLD_GROUP_NONE) {
			DBGLOG(AAA, WARN, "sta%d " MACSTR " no link id\n",
				prCurr->ucWlanIndex,
				MAC2STR(prCurr->aucMacAddr));
			return WLAN_STATUS_FAILURE;
		}
	}

	return WLAN_STATUS_SUCCESS;
}

#endif

void p2pTargetBssDescResetConnecting(
	IN struct ADAPTER *prAdapter,
	IN struct P2P_ROLE_FSM_INFO *fsm)
{
	uint8_t i;

	for (i = 0; i < MLD_LINK_MAX; i++) {
		struct BSS_INFO *prBss =
			p2pGetLinkBssInfo(prAdapter,
			fsm, i);
		struct BSS_DESC *prBssDesc =
			p2pGetLinkBssDesc(fsm, i);

		if (prBss && prBssDesc) {
			prBssDesc->fgIsConnecting &=
				~BIT(prBss->ucBssIndex);
		}
	}
}

void p2pClearAllLink(IN struct P2P_ROLE_FSM_INFO *prP2pRoleFsmInfo)
{
	uint8_t i;

	for (i = 0; i < MLD_LINK_MAX; i++) {
		prP2pRoleFsmInfo->aprP2pLinkInfo[i]
			.prP2pTargetBssDesc = NULL;
		prP2pRoleFsmInfo->aprP2pLinkInfo[i]
			.prP2pTargetStaRec = NULL;
	}
}

void p2pFillLinkBssDesc(
	IN struct ADAPTER *prAdapter,
	IN struct P2P_ROLE_FSM_INFO *prP2pRoleFsmInfo,
	IN struct BSS_DESC_SET *prBssDescSet)
{
	uint8_t i;

	if (!prAdapter || !prBssDescSet)
		return;

	for (i = 0; i < prBssDescSet->ucLinkNum; i++) {
		p2pSetLinkBssDesc(
			prP2pRoleFsmInfo,
			prBssDescSet->aprBssDesc[i],
			i);
	}

}

void p2pDeactivateAllLink(
	IN struct ADAPTER *prAdapter,
	IN struct P2P_ROLE_FSM_INFO *prP2pRoleFsmInfo,
	IN uint8_t fgClearStaRec)
{
	uint8_t i;

	for (i = 0; i < MLD_LINK_MAX; i++) {
		struct BSS_INFO *bss =
			p2pGetLinkBssInfo(prAdapter,
			prP2pRoleFsmInfo, i);

		if (bss && IS_NET_ACTIVE(prAdapter, bss->ucBssIndex))
			nicDeactivateNetworkEx(prAdapter,
				NETWORK_ID(bss->ucBssIndex, i),
				fgClearStaRec);
	}
}

struct P2P_ROLE_FSM_INFO *p2pGetDefaultRoleFsmInfo(
	IN struct ADAPTER *prAdapter,
	IN enum ENUM_IFTYPE eIftype)

{
	return P2P_ROLE_INDEX_2_ROLE_FSM_INFO(prAdapter,
		P2P_MAIN_ROLE_INDEX);
}

struct BSS_INFO *p2pGetDefaultLinkBssInfo(
	IN struct ADAPTER *prAdapter,
	IN enum ENUM_IFTYPE eIftype)
{
	struct P2P_ROLE_FSM_INFO *fsm =
		p2pGetDefaultRoleFsmInfo(prAdapter, eIftype);

	if (fsm)
		return p2pGetLinkBssInfo(prAdapter, fsm,
			P2P_MAIN_LINK_INDEX);
	else
		return NULL;
}

struct STA_RECORD *p2pGetDefaultLinkStaRec(
	IN struct ADAPTER *prAdapter,
	IN enum ENUM_IFTYPE eIftype)
{
	struct P2P_ROLE_FSM_INFO *fsm =
		p2pGetDefaultRoleFsmInfo(prAdapter, eIftype);

	if (fsm)
		return p2pGetLinkStaRec(fsm,
			P2P_MAIN_LINK_INDEX);
	else
		return NULL;
}

struct BSS_INFO *p2pGetLinkBssInfo(
	IN struct ADAPTER *prAdapter,
	IN struct P2P_ROLE_FSM_INFO *prP2pRoleFsmInfo,
	IN uint8_t ucLinkIdx)
{
	struct P2P_ROLE_FSM_INFO *fsm =
		(struct P2P_ROLE_FSM_INFO *) NULL;
	uint8_t ucLinkMax = 1;

#if (CFG_SUPPORT_802_11BE_MLO == 1)
	ucLinkMax = prAdapter->rWifiVar.ucMldLinkMax;
#endif

	if (!prP2pRoleFsmInfo)
		return NULL;

	if (ucLinkIdx >= ucLinkMax)
		ucLinkIdx = P2P_MAIN_LINK_INDEX;

	fsm = P2P_ROLE_INDEX_2_ROLE_FSM_INFO(prAdapter,
		ucLinkIdx);
	if (!fsm) {
		DBGLOG(P2P, ERROR, "p2p fsm shouldn't be NULL!\n");
		return NULL;
	}

	return GET_BSS_INFO_BY_INDEX(prAdapter, fsm->ucBssIndex);
}

void p2pGetLinkWmmQueSet(
	IN struct ADAPTER *prAdapter,
	IN struct BSS_INFO *prBssInfo)
{
	struct BSS_INFO *bss;

	/* main bss must assign wmm first */
	bss = p2pGetDefaultLinkBssInfo(prAdapter, IFTYPE_UNSPECIFIED);
	cnmWmmIndexDecision(prAdapter, bss);

#if (CFG_SUPPORT_802_11BE_MLO == 1) && (CFG_SUPPORT_CONNAC3X == 1)
	/* connac3 MLO all bss use the same wmm index as main bss use */
	prBssInfo->fgIsWmmInited = TRUE;
	prBssInfo->ucWmmQueSet = bss->ucWmmQueSet;
#else
	/* connac2 always assign different wmm index to bssinfo */
	cnmWmmIndexDecision(prAdapter, prBssInfo);
#endif

	DBGLOG(P2P, TRACE, "bss[%d] = %d\n",
		prBssInfo->ucBssIndex, prBssInfo->ucWmmQueSet);
}


void p2pSetLinkBssDesc(
	IN struct P2P_ROLE_FSM_INFO *prP2pRoleFsmInfo,
	IN struct BSS_DESC *prBssDesc,
	IN uint8_t ucLinkIdx)
{
	if (ucLinkIdx >= MLD_LINK_MAX)
		return;

	prP2pRoleFsmInfo->aprP2pLinkInfo[ucLinkIdx]
		.prP2pTargetBssDesc = prBssDesc;
}

struct BSS_DESC *p2pGetLinkBssDesc(
	IN struct P2P_ROLE_FSM_INFO *prP2pRoleFsmInfo,
	IN uint8_t ucLinkIdx)
{
	if (ucLinkIdx >= MLD_LINK_MAX)
		return NULL;

	return prP2pRoleFsmInfo->aprP2pLinkInfo[ucLinkIdx]
		.prP2pTargetBssDesc;
}

uint8_t p2pGetLinkNum(IN struct P2P_ROLE_FSM_INFO *prP2pRoleFsmInfo)
{
	uint8_t i, num = 0;

	for (i = 0; i < MLD_LINK_MAX; i++) {
		if (p2pGetLinkBssDesc(prP2pRoleFsmInfo, i))
			num++;
	}

	return num;
}

void p2pSetLinkStaRec(
	IN struct P2P_ROLE_FSM_INFO *prP2pRoleFsmInfo,
	IN struct STA_RECORD *prStaRec,
	IN uint8_t ucLinkIdx)
{
	if (ucLinkIdx >= MLD_LINK_MAX)
		return;
	prP2pRoleFsmInfo->aprP2pLinkInfo[ucLinkIdx]
		.prP2pTargetStaRec = prStaRec;
}

struct STA_RECORD *p2pGetLinkStaRec(
	IN struct P2P_ROLE_FSM_INFO *prP2pRoleFsmInfo,
	IN uint8_t ucLinkIdx)
{
	if (ucLinkIdx >= MLD_LINK_MAX)
		return NULL;

	return prP2pRoleFsmInfo->aprP2pLinkInfo[ucLinkIdx]
		.prP2pTargetStaRec;
}

struct P2P_CONNECTION_REQ_INFO *p2pGetConnReqInfo(
	IN struct ADAPTER *prAdapter,
	IN struct P2P_ROLE_FSM_INFO *prP2pRoleFsmInfo,
	IN uint8_t ucLinkIdx)
{
	struct P2P_ROLE_FSM_INFO *fsm =
		(struct P2P_ROLE_FSM_INFO *) NULL;

	if (!prAdapter || ucLinkIdx >= MLD_LINK_MAX)
		return NULL;

	fsm = P2P_ROLE_INDEX_2_ROLE_FSM_INFO(prAdapter,
		ucLinkIdx);
	if (!fsm) {
		DBGLOG(P2P, ERROR, "p2p fsm shouldn't be NULL!\n");
		return NULL;
	}

	return &(fsm->rConnReqInfo);
}

struct P2P_CHNL_REQ_INFO *p2pGetChnlReqInfo(
	IN struct ADAPTER *prAdapter,
	IN struct P2P_ROLE_FSM_INFO *prP2pRoleFsmInfo,
	IN uint8_t ucLinkIdx)
{
	struct P2P_ROLE_FSM_INFO *fsm =
		(struct P2P_ROLE_FSM_INFO *) NULL;

	if (!prAdapter || ucLinkIdx >= MLD_LINK_MAX)
		return NULL;

	fsm = P2P_ROLE_INDEX_2_ROLE_FSM_INFO(prAdapter,
		ucLinkIdx);
	if (!fsm) {
		DBGLOG(P2P, ERROR, "p2p fsm shouldn't be NULL!\n");
		return NULL;
	}

	return &(fsm->rChnlReqInfo);
}


void p2pLinkStaRecFree(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec)
{
#if (CFG_SUPPORT_802_11BE_MLO == 1)
	struct MLD_STA_RECORD *mld_starec;

	mld_starec = mldStarecGetByStarec(prAdapter, prStaRec);
	if (mld_starec) {
		uint16_t i;
		struct STA_RECORD *starec;

		for (i = 0; i < CFG_STA_REC_NUM; i++) {
			starec = (struct STA_RECORD *) &prAdapter->arStaRec[i];

			if (!starec ||
				!starec->fgIsInUse ||
				starec->ucMldStaIndex != mld_starec->ucIdx)
				continue;

			DBGLOG(INIT, INFO,
				"\tsta: %d, wid: %d, bss: %d\n",
				starec->ucIndex,
				starec->ucWlanIndex,
				starec->ucBssIndex);

			/* Change station state. */
			cnmStaRecChangeState(prAdapter,
				starec, STA_STATE_1);

			/* Reset Station Record Status. */
			p2pFuncResetStaRecStatus(prAdapter,
				starec);

			cnmStaRecFree(prAdapter,
				starec);
		}
	} else
#endif
	{
		/* Change station state. */
		cnmStaRecChangeState(prAdapter, prStaRec, STA_STATE_1);

		/* Reset Station Record Status. */
		p2pFuncResetStaRecStatus(prAdapter, prStaRec);

		cnmStaRecFree(prAdapter, prStaRec);
	}
}

void p2pLinkAcquireChJoin(
	IN struct ADAPTER *prAdapter,
	IN struct P2P_ROLE_FSM_INFO *prP2pRoleFsmInfo,
	IN struct P2P_CHNL_REQ_INFO *prChnlReq)
{
	struct MSG_CH_REQ *prMsgChReq = NULL;
	struct MSG_CH_REQ *prSubReq = NULL;
	uint8_t ucReqChNum = 0;
	uint32_t u4MsgSz;
	uint8_t i = 0;
	uint8_t ucSeqNumOfChReq;

	if (!prAdapter || !prChnlReq)
		return;

	for (i = 0; i < MLD_LINK_MAX; i++) {
		struct BSS_INFO *prBss =
			p2pGetLinkBssInfo(prAdapter,
			prP2pRoleFsmInfo, i);
		struct BSS_DESC *prBssDesc =
			p2pGetLinkBssDesc(prP2pRoleFsmInfo, i);

		if (!prBss || !prBssDesc)
			continue;

		/* for secondary link */
		if (!IS_NET_ACTIVE(prAdapter, prBss->ucBssIndex))
			/* sync with firmware */
			nicActivateNetwork(prAdapter,
				NETWORK_ID(prBss->ucBssIndex, i));

		ucReqChNum++;
	}

	/* send message to CNM for acquiring channel */
	u4MsgSz = sizeof(struct MSG_CH_REQ) +
		sizeof(struct MSG_CH_REQ) * ucReqChNum;
	prMsgChReq = (struct MSG_CH_REQ *)cnmMemAlloc(prAdapter,
		RAM_TYPE_MSG,
		u4MsgSz);
	if (!prMsgChReq) {
		DBGLOG(P2P, ERROR, "Alloc CH req msg failed.\n");
		return;
	}
	kalMemZero(prMsgChReq, u4MsgSz);

	prChnlReq->ucChReqNum = ucReqChNum;

	prMsgChReq->ucExtraChReqNum = prChnlReq->ucChReqNum - 1;

	ucSeqNumOfChReq = ++prChnlReq->ucSeqNumOfChReq;

	for (i = 0; i < ucReqChNum; i++) {
		struct BSS_INFO *prBss =
			p2pGetLinkBssInfo(prAdapter,
			prP2pRoleFsmInfo, i);
		struct BSS_DESC *prBssDesc =
			p2pGetLinkBssDesc(prP2pRoleFsmInfo, i);
		struct P2P_CHNL_REQ_INFO *prChnlReqInfo =
			p2pGetChnlReqInfo(prAdapter,
			prP2pRoleFsmInfo, i);

		if (!prBss || !prBssDesc)
			continue;

		if (i == 0)
			prSubReq = prMsgChReq;
		else
			prSubReq = (struct MSG_CH_REQ *)
				&prMsgChReq->aucBuffer[i];

		p2pFuncReleaseCh(prAdapter,
			prBss->ucBssIndex,
			prChnlReq);

		prSubReq->rMsgHdr.eMsgId = MID_MNY_CNM_CH_REQ;
		prSubReq->ucBssIndex = prBss->ucBssIndex;
		prSubReq->ucTokenID = ucSeqNumOfChReq;
		prSubReq->eReqType = CH_REQ_TYPE_JOIN;
		prSubReq->u4MaxInterval = prChnlReqInfo->u4MaxInterval;
		prSubReq->ucPrimaryChannel = prChnlReqInfo->ucReqChnlNum;
		prSubReq->eRfSco = prChnlReqInfo->eChnlSco;
		prSubReq->eRfBand = prChnlReqInfo->eBand;
		prSubReq->eRfChannelWidth = prChnlReqInfo->eChannelWidth;
		prSubReq->ucRfCenterFreqSeg1 = prChnlReqInfo->ucCenterFreqS1;
		prSubReq->ucRfCenterFreqSeg2 = prChnlReqInfo->ucCenterFreqS2;
#if CFG_SUPPORT_DBDC
		if (ucReqChNum >= 2)
			prSubReq->eDBDCBand = ENUM_BAND_ALL;
		else
			prSubReq->eDBDCBand = ENUM_BAND_AUTO;
#endif /*CFG_SUPPORT_DBDC*/
	}

	DBGLOG(P2P, INFO,
	   "on band %u, tokenID: %d, cookie: 0x%llx, ucExtraChReqNum: %d.\n",
	   prMsgChReq->eDBDCBand,
	   prMsgChReq->ucTokenID,
	   prChnlReq->u8Cookie,
	   prMsgChReq->ucExtraChReqNum);

	mboxSendMsg(prAdapter, MBOX_ID_0,
			(struct MSG_HDR *)prMsgChReq,
			MSG_SEND_METHOD_BUF);

	prChnlReq->fgIsChannelRequested = TRUE;
}

uint32_t
p2pRoleFsmRunEventAAAComplete(
	IN struct ADAPTER *prAdapter,
	IN struct STA_RECORD *prStaRec,
	IN struct BSS_INFO *prP2pBssInfo)
{
#if (CFG_SUPPORT_802_11BE_MLO == 1)
	struct MLD_STA_RECORD *mld_starec;

	mld_starec = mldStarecGetByStarec(prAdapter, prStaRec);

	if (mld_starec) {
		struct LINK *links;
		struct STA_RECORD *starec;
		uint32_t rStatus = WLAN_STATUS_SUCCESS;

		links =  &mld_starec->rStarecList;
		LINK_FOR_EACH_ENTRY(starec,
			links, rLinkEntryMld,
			struct STA_RECORD) {
			struct BSS_INFO *bss =
				GET_BSS_INFO_BY_INDEX(
				prAdapter,
				starec->ucBssIndex);

			DBGLOG(INIT, INFO,
				"\tsta: %d, wid: %d, bss: %d, " MACSTR "\n",
				starec->ucIndex,
				starec->ucWlanIndex,
				starec->ucBssIndex,
				MAC2STR(bss->aucOwnMacAddr));

			rStatus =
			p2pRoleFsmRunEventAAACompleteImpl(
				prAdapter,
				starec,
				bss);
			if (rStatus != WLAN_STATUS_SUCCESS)
				return rStatus;
		}

		return rStatus;
	}
#endif

	return p2pRoleFsmRunEventAAACompleteImpl(
		prAdapter,
		prStaRec,
		prP2pBssInfo);
}

uint32_t
p2pRoleFsmRunEventAAASuccess(
	IN struct ADAPTER *prAdapter,
	IN struct STA_RECORD *prStaRec,
	IN struct BSS_INFO *prP2pBssInfo)
{
	return p2pRoleFsmRunEventAAASuccessImpl(
		prAdapter,
		prStaRec,
		prP2pBssInfo);
}

void p2pRoleFsmRunEventAAATxFail(
	IN struct ADAPTER *prAdapter,
	IN struct STA_RECORD *prStaRec,
	IN struct BSS_INFO *prP2pBssInfo)
{
	p2pRoleFsmRunEventAAATxFailImpl(
		prAdapter,
		prStaRec,
		prP2pBssInfo);
}

uint16_t bssAssignAssocID(IN struct STA_RECORD *prStaRec)
{
#if (CFG_SUPPORT_802_11BE_MLO == 1)
	if (prStaRec->ucMldStaIndex != MLD_GROUP_NONE)
		return prStaRec->ucMldStaIndex + 1;
#endif

	return prStaRec->ucIndex + 1;
}

#if (CFG_SUPPORT_802_11BE_MLO == 1)
void p2pScanFillSecondaryLink(struct ADAPTER *prAdapter,
	struct BSS_DESC_SET *prBssDescSet)
{
	struct LINK *prBSSDescList =
		&prAdapter->rWifiVar.rScanInfo.rBSSDescList;
	struct BSS_DESC *prBssDesc = NULL;
	struct BSS_DESC *prMainBssDesc = prBssDescSet->prMainBssDesc;
	uint8_t i, j;

	if (!prMainBssDesc || !prMainBssDesc->rMlInfo.fgValid) {
		DBGLOG(P2P, INFO, "no need secondary link");
		return;
	}

	DBGLOG(P2P, INFO,
		"Main " MACSTR " valid=%d mld_addr="MACSTR"\n",
		MAC2STR(prMainBssDesc->aucBSSID),
		prMainBssDesc->rMlInfo.fgValid,
		MAC2STR(prMainBssDesc->rMlInfo.aucMldAddr));

	/* setup secondary link */
	LINK_FOR_EACH_ENTRY(prBssDesc, prBSSDescList, rLinkEntry,
		struct BSS_DESC) {

		/* break if reach the limit num of links */
		if (prBssDescSet->ucLinkNum >= MLD_LINK_MAX)
			break;

		if (!prBssDesc->rMlInfo.fgValid ||
		    EQUAL_MAC_ADDR(prMainBssDesc->aucBSSID,
				 prBssDesc->aucBSSID) ||
		    !EQUAL_MAC_ADDR(prMainBssDesc->rMlInfo.aucMldAddr,
				 prBssDesc->rMlInfo.aucMldAddr)) {
			DBGLOG(P2P, LOUD,
				"Skip " MACSTR " valid=%d mld_addr="MACSTR"\n",
				MAC2STR(prBssDesc->aucBSSID),
				prBssDesc->rMlInfo.fgValid,
				MAC2STR(prBssDesc->rMlInfo.aucMldAddr));
			continue;
		}

		DBGLOG(P2P, LOUD,
			"Add " MACSTR " valid=%d mld_addr="MACSTR"\n",
			MAC2STR(prBssDesc->aucBSSID),
			prBssDesc->rMlInfo.fgValid,
			MAC2STR(prBssDesc->rMlInfo.aucMldAddr));

		/* Record same Mld list */
		prBssDescSet->aprBssDesc[prBssDescSet->ucLinkNum] = prBssDesc;
		prBssDescSet->ucLinkNum++;
	}

	for (i = 0; i < prBssDescSet->ucLinkNum - 1; i++) {
		for (j = i + 1; j < prBssDescSet->ucLinkNum; j++) {
			if (prBssDescSet->aprBssDesc[j]
				->rMlInfo.ucLinkIndex <
				prBssDescSet->aprBssDesc[i]
				->rMlInfo.ucLinkIndex) {
				prBssDesc = prBssDescSet->aprBssDesc[j];
				prBssDescSet->aprBssDesc[j] =
					prBssDescSet->aprBssDesc[i];
				prBssDescSet->aprBssDesc[i] =
					prBssDesc;
			}
		}
	}

	/* first bss desc is main bss */
	prBssDescSet->prMainBssDesc = prBssDescSet->aprBssDesc[0];
	DBGLOG(P2P, INFO, "Total %d link(s)\n", prBssDescSet->ucLinkNum);
}
#endif

