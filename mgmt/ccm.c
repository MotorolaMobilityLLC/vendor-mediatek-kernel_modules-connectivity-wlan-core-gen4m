// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2021 MediaTek Inc.
 */
#include "precomp.h"


#if CFG_SUPPORT_CCM && CFG_ENABLE_WIFI_DIRECT
struct P2P_CCM_CSA_ENTRY {
	struct LINK_ENTRY rLinkEntry;
	struct BSS_INFO *prBssInfo;
	uint32_t u4TargetCh;
	enum ENUM_MBMC_BN eTargetHwBandIdx;
	enum ENUM_BAND eTargetBand;
};

void ccmInit(struct ADAPTER *prAdapter)
{
	LINK_INITIALIZE(&prAdapter->rCcmCheckCsList);
}

u_int8_t ccmGoSwitchChannel(struct ADAPTER *prAdapter,
			    struct BSS_INFO *prBssInfo,
			    uint32_t u4TargetCh,
			    enum ENUM_MBMC_BN eTargetHwBandIdx,
			    enum ENUM_BAND eTargetBand)
{
	u_int8_t fgIsDfs = rlmDomainIsDfsChnls(prAdapter, u4TargetCh);

	if (fgIsDfs ||
	    /* only allow MCC, this will skip MLO 2nd link, either. */
	    prBssInfo->eHwBandIdx != eTargetHwBandIdx ||
	    prBssInfo->ucPrimaryChannel == u4TargetCh) {
		DBGLOG(CCM, INFO,
		       "do not need CSA, isDfs=%u [BSS%u] ch=%u, hwBand=%u [Target] ch=%u, hwBand=%u, rfBand=%u",
		       fgIsDfs, prBssInfo->ucBssIndex,
		       prBssInfo->ucPrimaryChannel, prBssInfo->eHwBandIdx,
		       u4TargetCh, eTargetHwBandIdx, eTargetBand);
		return FALSE;
	}

#if (CFG_SUPPORT_WIFI_6G == 1)
	if (eTargetBand == BAND_6G &&
	    !rsnKeyMgmtSae(prBssInfo->u4RsnSelectedAKMSuite)) {
		DBGLOG(CCM, INFO, "Skip CSA to 6G if auth type not SAE");
		return FALSE;
	}
#endif

	cnmIdcCsaReq(prAdapter, eTargetBand, u4TargetCh,
			     prBssInfo->u4PrivateData);
	return TRUE;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Each time consume one entry in queue to check whether it needs to CS.
 *        Only trigger by ccmChannelSwitchProducer,
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

	if (prAdapter->rWifiVar.fgCsaInProgress) {
		DBGLOG(CCM, INFO, "skip due to CSA still in progress");
		return;
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
		       "all BSS align new connection or CSA chnl done");
		return;
	}

	if (p2pFuncIsAPMode(prAdapter->rWifiVar.prP2PConnSettings[
			bss->u4PrivateData]))
		fgIsSwitching = p2pFuncSwitchSapChannel(prAdapter);
	else
		fgIsSwitching = ccmGoSwitchChannel(prAdapter, bss,
			       u4TargetCh, eTargetHwBandIdx, eTargetBand);

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
void ccmChannelSwitchProducer(struct ADAPTER *prAdapter,
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
		DBGLOG(CCM, INFO, "null target Bss");
		return;
	}

	if (!prAdapter->fgIsP2PRegistered)
		return;

	if (prWifiVar->eP2pCcmMode == P2P_CCM_MODE_DISABLE) {
		DBGLOG(CCM, WARN, "CCM is disabled");
		return;
	}

#if (CFG_SUPPORT_802_11BE_MLO == 1)
	fgIsTargetMlo = IS_MLD_BSSINFO_MULTI(
				mldBssGetByBss(prAdapter, prTargetBss));
#endif

	DBGLOG(CCM, INFO,
	       "%s: bss=%u, role=%s, is_mlo=%u notify other GO/SAP to CSA",
	       pucSrcFunc, prTargetBss->ucBssIndex,
	       bssGetRoleTypeString(prAdapter, prTargetBss), fgIsTargetMlo);

	/* find the first SAP entry in list */
	LINK_FOR_EACH_ENTRY(prCcmCsaEntry, prCcmCheckCsList, rLinkEntry,
			    struct P2P_CCM_CSA_ENTRY) {
		if (p2pFuncIsAPMode(prAdapter->rWifiVar.prP2PConnSettings[
				prCcmCsaEntry->prBssInfo->u4PrivateData])) {
			prFirstSap = prCcmCsaEntry;
			DBGLOG(CCM, TRACE, "first SAP in list is bss=%u",
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
			DBGLOG(CCM, ERROR, "Alloc mem fail");
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

		DBGLOG(CCM, INFO,
		       "insert GO bss=%u waiting for CSA, Target: ch=%u, hwBand=%u, rfBand=%u",
		       bss->ucBssIndex, prTargetBss->ucPrimaryChannel,
		       prTargetBss->eHwBandIdx, prTargetBss->eBand);
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

		DBGLOG(CCM, INFO,
		       "insert GO bss=%u waiting for CSA, Target: ch=%u, hwBand=%u, rfBand=%u",
		       bss->ucBssIndex, prTargetBss->ucPrimaryChannel,
		       prTargetBss->eHwBandIdx, prTargetBss->eBand);
	}

	ccmChannelSwitchConsumer(prAdapter);
}

#endif /* CFG_SUPPORT_CCM && CFG_ENABLE_WIFI_DIRECT */
