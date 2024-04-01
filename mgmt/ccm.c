// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2021 MediaTek Inc.
 */
#include "precomp.h"


#if CFG_SUPPORT_CCM
struct P2P_CCM_CSA_ENTRY {
	struct LINK_ENTRY rLinkEntry;
	struct BSS_INFO *prBssInfo;
	uint32_t u4TargetCh;
	enum ENUM_MBMC_BN eTargetHwBandIdx;
	enum ENUM_BAND eTargetBand;
};

static void __ccmChannelSwitchProducer(struct ADAPTER *prAdapter,
				       struct BSS_INFO *prTargetBss,
				       const char *pucSrcFunc);

static u_int8_t ccmIsSwitchChannelNeeded(struct ADAPTER *prAdapter,
			    struct BSS_INFO *prBssInfo,
			    uint32_t u4TargetCh,
			    enum ENUM_MBMC_BN eTargetHwBandIdx,
			    enum ENUM_BAND *eTargetBand);

void ccmInit(struct ADAPTER *prAdapter)
{
	LINK_INITIALIZE(&prAdapter->rCcmCheckCsList);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Prevent to access the old connection info.
 */
/*----------------------------------------------------------------------------*/
void ccmRemoveBssPendingEntry(struct ADAPTER *prAdapter,
			      struct BSS_INFO *prBssInfo)
{
	struct LINK *prCcmCheckCsList = &prAdapter->rCcmCheckCsList;
	struct P2P_CCM_CSA_ENTRY *prCcmCsaEntry;

	LINK_FOR_EACH_ENTRY(prCcmCsaEntry, prCcmCheckCsList, rLinkEntry,
			    struct P2P_CCM_CSA_ENTRY) {
		if (prCcmCsaEntry->prBssInfo != prBssInfo)
			continue;
		DBGLOG(CCM, INFO, "bss=%u free, remove pending entry");
		LINK_REMOVE_KNOWN_ENTRY(prCcmCheckCsList, prCcmCsaEntry);
		cnmMemFree(prAdapter, prCcmCsaEntry);
	}
}

static void ccmPendingEventTimeout(struct ADAPTER *prAdapter,
				   uintptr_t ulParamPtr)
{
	struct BSS_INFO *targetBss = (struct BSS_INFO *) ulParamPtr;

	prAdapter->fgIsCcmPending = FALSE;
	ccmChannelSwitchProducer(prAdapter, targetBss, __func__);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Check if CCM need to pending until ch grant timeout.
 *        Only pending when a GO want to CSA.
 *
 * \param[in] pvAdapter Pointer to the adapter descriptor.
 *            u4TargetCh Target BSS channel.
 *            eTargetHwBandIdx Target BSS HW band.
 *            eTargetBand Target BSS RF band.
 *
 * \return status
 */
/*----------------------------------------------------------------------------*/
void ccmPendingCheck(struct ADAPTER *prAdapter,
		     struct BSS_INFO *prTargetBss,
		     uint32_t u4GrantInterval)
{
	uint8_t i;
	struct BSS_INFO *bss;
	enum ENUM_BAND eTargetBand = prTargetBss->eBand;
	u_int8_t fgIsTargetMlo = FALSE;

	if (prAdapter->fgIsCcmPending) {
		DBGLOG(CCM, WARN,
		       "skip pending check due to previous check not done\n");
		return;
	}
	prAdapter->fgIsCcmPending = FALSE;

#if (CFG_SUPPORT_802_11BE_MLO == 1)
	fgIsTargetMlo = IS_MLD_BSSINFO_MULTI(
				mldBssGetByBss(prAdapter, prTargetBss));
#endif

	for (i = 0; i < MAX_BSSID_NUM && !prAdapter->fgIsCcmPending; ++i) {
		bss = GET_BSS_INFO_BY_INDEX(prAdapter, i);

		/* skip target bss itself */
		if (!fgIsTargetMlo && bss == prTargetBss)
			continue;
#if (CFG_SUPPORT_802_11BE_MLO == 1) || defined(CFG_SUPPORT_UNIFIED_COMMAND)
		else if (bss->ucGroupMldId == prTargetBss->ucGroupMldId)
			continue;
#endif

		if (IS_BSS_APGO(bss) && IS_BSS_ALIVE(prAdapter, bss) &&
		    !p2pFuncIsAPMode(prAdapter->rWifiVar.prP2PConnSettings[
				     bss->u4PrivateData]) &&
		    ccmIsSwitchChannelNeeded(prAdapter, bss,
					 prTargetBss->ucPrimaryChannel,
					 prTargetBss->eHwBandIdx,
					 &eTargetBand))
			prAdapter->fgIsCcmPending = TRUE;
	}

	if (prAdapter->fgIsCcmPending) {
		/* New connect ch req will make GO NoA for 4~5s,
		 * which may cause GC absent and not receive beacon.
		 */
		DBGLOG(CCM, INFO,
		       "pending CCM for %ums because a GO wants to CSA\n",
		       u4GrantInterval);
		cnmTimerInitTimer(prAdapter, &prAdapter->rCcmPendingTimer,
			(PFN_MGMT_TIMEOUT_FUNC) ccmPendingEventTimeout,
			(uintptr_t) prTargetBss);
		cnmTimerStartTimer(prAdapter, &prAdapter->rCcmPendingTimer,
				   u4GrantInterval);
	}
}

static void ccmGetOtherAliveBssHwBitmap(struct ADAPTER *prAdapter,
				 uint32_t *pau4Bitmap,
				 struct BSS_INFO *prBssInfo)
{
	struct BSS_INFO *bss;
	uint8_t i;

	for (i = 0; i < MAX_BSSID_NUM; ++i) {
		bss = GET_BSS_INFO_BY_INDEX(prAdapter, i);

		if (!IS_BSS_ALIVE(prAdapter, bss) || bss == prBssInfo)
			continue;

		pau4Bitmap[bss->eHwBandIdx] |= BIT(i);
	}
}

void ccmChannelSwitchProducer(struct ADAPTER *prAdapter,
			      struct BSS_INFO *prTargetBss,
			      const char *pucSrcFunc)
{
	if (prAdapter->fgIsCcmPending) {
		DBGLOG(CCM, INFO, "skip CCM due to pending");
		return;
	}

#if (CFG_SUPPORT_802_11BE_MLO == 1)
	if (IS_BSS_GC(prTargetBss) || IS_BSS_AIS(prTargetBss)) {
		struct BSS_INFO *bss;
		struct MLD_BSS_INFO *prMldBss = mldBssGetByBss(prAdapter,
							       prTargetBss);

		if (prMldBss) {
			/* MLO GC/STA only ch abort once */
			LINK_FOR_EACH_ENTRY(bss, &prMldBss->rBssList,
					    rLinkEntryMld, struct BSS_INFO)
				__ccmChannelSwitchProducer(prAdapter, bss,
							   pucSrcFunc);
		} else
			__ccmChannelSwitchProducer(prAdapter, prTargetBss,
						   pucSrcFunc);
	} else if (IS_BSS_APGO(prTargetBss))
#endif /* CFG_SUPPORT_802_11BE_MLO == 1 */
		__ccmChannelSwitchProducer(prAdapter, prTargetBss, pucSrcFunc);
}

static u_int8_t ccmIsSwitchChannelNeeded(struct ADAPTER *prAdapter,
			    struct BSS_INFO *prBssInfo,
			    uint32_t u4TargetCh,
			    enum ENUM_MBMC_BN eTargetHwBandIdx,
			    enum ENUM_BAND *eTargetBand)
{
	u_int8_t fgIsDfs = rlmDomainIsDfsChnls(prAdapter, u4TargetCh);
#if (CFG_SUPPORT_WIFI_6G == 1)
	uint32_t au4AliveBssBitmap[AA_HW_BAND_NUM] = { 0 };
	uint32_t freqList[MAX_5G_BAND_CHN_NUM + MAX_6G_BAND_CHN_NUM] = {};
	uint32_t u4FreqListNum;
	u_int8_t fgIs6gPscCh = IS_6G_PSC_CHANNEL(u4TargetCh);
#endif

	/* pass for MCC only */
	if (fgIsDfs ||
#if (CFG_SUPPORT_WIFI_6G == 1)
	    (*eTargetBand == BAND_6G && !fgIs6gPscCh) ||
#endif
	    prBssInfo->eHwBandIdx != eTargetHwBandIdx ||
	    prBssInfo->ucPrimaryChannel == u4TargetCh) {
		DBGLOG(CCM, INFO,
		       "do not trigger CSA, [%s]Bss%u, ch=%u, hwBand=%u, rfBand=%u [Target]ch=%u, hwBand=%u, rfBand=%u\n",
		       bssGetRoleTypeString(prAdapter, prBssInfo),
		       prBssInfo->ucBssIndex, prBssInfo->ucPrimaryChannel,
		       prBssInfo->eHwBandIdx, prBssInfo->eBand,
		       u4TargetCh, eTargetHwBandIdx, *eTargetBand);
		return FALSE;
	}

#if (CFG_SUPPORT_WIFI_6G == 1)
	ccmGetOtherAliveBssHwBitmap(prAdapter, au4AliveBssBitmap, prBssInfo);
	DBGLOG(P2P, INFO,
	       "alive BSS hw bitmap [bn0:bn1:bn2]=[0x%x:0x%x:0x%x]\n",
	       au4AliveBssBitmap[AA_HW_BAND_0], au4AliveBssBitmap[AA_HW_BAND_1],
	       au4AliveBssBitmap[AA_HW_BAND_2]);

	if ((prBssInfo->eBand == BAND_5G || prBssInfo->eBand == BAND_6G) &&
	    p2pFuncIsPreferWfdAa(prAdapter, au4AliveBssBitmap)) {
		u4FreqListNum = p2pFuncAppendAaFreq(prAdapter, prBssInfo,
						     freqList);
		if (u4FreqListNum > 0) {
			u4TargetCh = nicFreq2ChannelNum(freqList[0] * 1000);
			if (freqList[0] >= 2412 && freqList[0] <= 2484)
				*eTargetBand = BAND_2G4;
			else if (freqList[0] >= 5180 && freqList[0] <= 5900)
				*eTargetBand = BAND_5G;
			else if (freqList[0] >= 5955 && freqList[0] <= 7115)
				*eTargetBand = BAND_6G;
		}
	}

	if (*eTargetBand == BAND_6G &&
	    !rsnKeyMgmtSae(prBssInfo->u4RsnSelectedAKMSuite)) {
		DBGLOG(CCM, WARN, "Skip CSA to 6G if auth type not SAE\n");
		return FALSE;
	}
#endif /* CFG_SUPPORT_WIFI_6G == 1 */

	return TRUE;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Each time consume one entry in queue to check whether it needs to CS.
 *        Only trigger by __ccmChannelSwitchProducer,
 *        p2pRoleStateAbort_SWITCH_CHANNEL and itself.
 *
 * \param[in] pvAdapter Pointer to the adapter descriptor.
 *
 * \return status
 */
/*----------------------------------------------------------------------------*/
void ccmChannelSwitchConsumer(struct ADAPTER *prAdapter)
{
	struct LINK *prCcmCheckCsList = &prAdapter->rCcmCheckCsList;
	struct P2P_CCM_CSA_ENTRY *prCcmCsaEntry;
	struct BSS_INFO *bss;
	uint32_t u4TargetCh;
	enum ENUM_MBMC_BN eTargetHwBandIdx;
	enum ENUM_BAND eTargetBand;
	u_int8_t fgIsSwitching = FALSE;
	u_int8_t fgIsMlo = FALSE;
	int8_t i;

	if (prAdapter->rWifiVar.fgCsaInProgress) {
		/* csa start countdown till csadone */
		DBGLOG(CCM, INFO, "skip due to CSA still in progress\n");
		return;
	} else {
		/* start switching channel till ch granted */
		for (i = 0; i < MAX_BSSID_NUM; ++i) {
			bss = GET_BSS_INFO_BY_INDEX(prAdapter, i);

			if (IS_BSS_P2P(bss) && bss->fgIsSwitchingChnl)
				break;
			else if (IS_BSS_AIS(bss) && bss->fgIsAisSwitchingChnl)
				break;
		}

		if (i < MAX_BSSID_NUM) {
			DBGLOG(CCM, INFO,
			       "skip due to Bss%u channel switch still in progress\n",
			       i);
			return;
		}
	}

	if (!LINK_IS_EMPTY(prCcmCheckCsList)) {
		LINK_REMOVE_HEAD(prCcmCheckCsList, prCcmCsaEntry,
			struct P2P_CCM_CSA_ENTRY *);
		bss = prCcmCsaEntry->prBssInfo;
		u4TargetCh = prCcmCsaEntry->u4TargetCh;
		eTargetHwBandIdx = prCcmCsaEntry->eTargetHwBandIdx;
		eTargetBand = prCcmCsaEntry->eTargetBand;

		if (!IS_BSS_APGO(bss))
			return;

		cnmMemFree(prAdapter, prCcmCsaEntry);
	} else {
		DBGLOG(CCM, INFO,
		       "all BSS align new connection or CSA chnl done\n");
		return;
	}

#if (CFG_SUPPORT_802_11BE_MLO == 1)
	fgIsMlo = IS_MLD_BSSINFO_MULTI(mldBssGetByBss(prAdapter, bss));
#endif

	DBGLOG(CCM, INFO,
	       "checking [%s]bss=%u ch=%u, hwBand=%u, rfBand=%u, is_mlo=%u, [Target]ch=%u, hwBand=%u, rfBand=%u\n",
	       bssGetRoleTypeString(prAdapter, bss),
	       bss->ucBssIndex, bss->ucPrimaryChannel,
	       bss->eHwBandIdx, bss->eBand, fgIsMlo,
	       u4TargetCh, eTargetHwBandIdx, eTargetBand);

	if (p2pFuncIsAPMode(prAdapter->rWifiVar.prP2PConnSettings[
		bss->u4PrivateData]) && !fgIsMlo)
		fgIsSwitching = p2pFuncSwitchSapChannel(prAdapter,
					P2P_DEFAULT_SCENARIO);
	else if (ccmIsSwitchChannelNeeded(prAdapter, bss, u4TargetCh,
				      eTargetHwBandIdx, &eTargetBand)) {
		cnmIdcCsaReq(prAdapter, eTargetBand, u4TargetCh,
				     bss->u4PrivateData);
		fgIsSwitching = TRUE;
	}

	if (!fgIsSwitching)
		ccmChannelSwitchConsumer(prAdapter);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Produce the BSS CS check event, and consumer will check whether it
 *        needs to CS to SCC with prTargetBss.
 *
 * \param[in] pvAdapter Pointer to the adapter descriptor.
 * \param[in] prTargetBss The target BSS, MLO should call twice.
 * \param[in] pucSrcFunc Source function that trigger the CSA.
 *
 * \return status
 */
/*----------------------------------------------------------------------------*/
static void __ccmChannelSwitchProducer(struct ADAPTER *prAdapter,
				struct BSS_INFO *prTargetBss,
				const char *pucSrcFunc)
{
	struct BSS_INFO *bss;
	uint8_t i;
	struct LINK *prCcmCheckCsList = &prAdapter->rCcmCheckCsList;
	u_int8_t fgIsTargetMlo = FALSE;
	struct P2P_CCM_CSA_ENTRY *prCcmCsaEntry;
	struct P2P_CCM_CSA_ENTRY *prFirstSap = NULL;
	struct WIFI_VAR *prWifiVar = &prAdapter->rWifiVar;

	if (!prTargetBss) {
		DBGLOG(CCM, INFO, "null target Bss\n");
		return;
	}

	if (!prAdapter->fgIsP2PRegistered)
		return;

	if (prWifiVar->eP2pCcmMode == P2P_CCM_MODE_DISABLE) {
		DBGLOG(CCM, WARN, "CCM is disabled\n");
		return;
	}

#if (CFG_SUPPORT_802_11BE_MLO == 1)
	fgIsTargetMlo = IS_MLD_BSSINFO_MULTI(
				mldBssGetByBss(prAdapter, prTargetBss));
#endif

	DBGLOG(CCM, INFO,
	       "%s: bss=%u, role=%s, is_mlo=%u notify other GO/SAP to CSA\n",
	       pucSrcFunc, prTargetBss->ucBssIndex,
	       bssGetRoleTypeString(prAdapter, prTargetBss), fgIsTargetMlo);

	/* find the first SAP entry in list */
	LINK_FOR_EACH_ENTRY(prCcmCsaEntry, prCcmCheckCsList, rLinkEntry,
			    struct P2P_CCM_CSA_ENTRY) {
		if (p2pFuncIsAPMode(prAdapter->rWifiVar.prP2PConnSettings[
				prCcmCsaEntry->prBssInfo->u4PrivateData])) {
			prFirstSap = prCcmCsaEntry;
			DBGLOG(CCM, TRACE, "first SAP in list is bss=%u\n",
			       prFirstSap->prBssInfo->ucBssIndex);
			break;
		}
	}

	/* enqueue GO before the first SAP first */
	for (i = 0; i < MAX_BSSID_NUM; ++i) {
		bss = GET_BSS_INFO_BY_INDEX(prAdapter, i);

		if (!IS_BSS_APGO(bss) || !IS_BSS_ALIVE(prAdapter, bss)
		    || p2pFuncIsAPMode(prAdapter->rWifiVar.prP2PConnSettings[
					bss->u4PrivateData]))
			continue;

		/* skip target bss itself */
		if (!fgIsTargetMlo && bss == prTargetBss)
			continue;
#if (CFG_SUPPORT_802_11BE_MLO == 1) || defined(CFG_SUPPORT_UNIFIED_COMMAND)
		else if (bss->ucGroupMldId == prTargetBss->ucGroupMldId)
			continue;
#endif

		prCcmCsaEntry = cnmMemAlloc(prAdapter,
			RAM_TYPE_MSG, sizeof(struct P2P_CCM_CSA_ENTRY));
		if (!prCcmCsaEntry) {
			DBGLOG(CCM, ERROR, "Alloc mem fail\n");
			return;
		}

		prCcmCsaEntry->prBssInfo = bss;
		prCcmCsaEntry->u4TargetCh = prTargetBss->ucPrimaryChannel;
		prCcmCsaEntry->eTargetHwBandIdx = prTargetBss->eHwBandIdx;
		prCcmCsaEntry->eTargetBand = prTargetBss->eBand;

		if (prFirstSap) {
			LINK_INSERT_BEFORE(prCcmCheckCsList,
					   &prFirstSap->rLinkEntry,
					   &prCcmCsaEntry->rLinkEntry);
		} else {
			LINK_INSERT_TAIL(prCcmCheckCsList,
					 &prCcmCsaEntry->rLinkEntry);
		}

		DBGLOG(CCM, INFO, "insert GO bss=%u waiting to check\n",
		       bss->ucBssIndex);
	}

	/* SAP must do CSA last, enqueue SAP at the end */
	for (i = 0; i < MAX_BSSID_NUM; ++i) {
		bss = GET_BSS_INFO_BY_INDEX(prAdapter, i);

		if (!IS_BSS_APGO(bss) || !IS_BSS_ALIVE(prAdapter, bss)
		    || !p2pFuncIsAPMode(prAdapter->rWifiVar.prP2PConnSettings[
					bss->u4PrivateData]))
			continue;

		/* skip target bss itself */
		if (!fgIsTargetMlo && bss == prTargetBss)
			continue;
#if (CFG_SUPPORT_802_11BE_MLO == 1) || defined(CFG_SUPPORT_UNIFIED_COMMAND)
		else if (bss->ucGroupMldId == prTargetBss->ucGroupMldId)
			continue;
#endif

		prCcmCsaEntry = cnmMemAlloc(prAdapter,
			RAM_TYPE_MSG, sizeof(struct P2P_CCM_CSA_ENTRY));

		prCcmCsaEntry->prBssInfo = bss;
		prCcmCsaEntry->u4TargetCh = prTargetBss->ucPrimaryChannel;
		prCcmCsaEntry->eTargetHwBandIdx = prTargetBss->eHwBandIdx;
		prCcmCsaEntry->eTargetBand = prTargetBss->eBand;

		LINK_INSERT_TAIL(prCcmCheckCsList,
				 &prCcmCsaEntry->rLinkEntry);

		DBGLOG(CCM, INFO, "insert SAP bss=%u waiting to check\n",
		       bss->ucBssIndex);
	}

	ccmChannelSwitchConsumer(prAdapter);
}
#endif /* CFG_SUPPORT_CCM */
