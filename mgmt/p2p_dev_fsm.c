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
#include "precomp.h"
#include "p2p_dev_state.h"
#if CFG_ENABLE_WIFI_DIRECT

#if 1
/*lint -save -e64 Type mismatch */
static PUINT_8 apucDebugP2pDevState[P2P_DEV_STATE_NUM] = {
	(PUINT_8) DISP_STRING("P2P_DEV_STATE_IDLE"),
	(PUINT_8) DISP_STRING("P2P_DEV_STATE_SCAN"),
	(PUINT_8) DISP_STRING("P2P_DEV_STATE_REQING_CHANNEL"),
	(PUINT_8) DISP_STRING("P2P_DEV_STATE_CHNL_ON_HAND"),
	(PUINT_8) DISP_STRING("P2P_DEV_STATE_NUM")
};

/*lint -restore */
#endif /* DBG */

UINT_8 p2pDevFsmInit(IN P_ADAPTER_T prAdapter)
{
	P_P2P_DEV_FSM_INFO_T prP2pDevFsmInfo = (P_P2P_DEV_FSM_INFO_T) NULL;
	P_P2P_CHNL_REQ_INFO_T prP2pChnlReqInfo = (P_P2P_CHNL_REQ_INFO_T) NULL;
	P_P2P_MGMT_TX_REQ_INFO_T prP2pMgmtTxReqInfo = (P_P2P_MGMT_TX_REQ_INFO_T) NULL;
	P_BSS_INFO_T prP2pBssInfo = (P_BSS_INFO_T) NULL;

	do {
		ASSERT_BREAK(prAdapter != NULL);

		prP2pDevFsmInfo = prAdapter->rWifiVar.prP2pDevFsmInfo;

		ASSERT_BREAK(prP2pDevFsmInfo != NULL);

		kalMemZero(prP2pDevFsmInfo, sizeof(P2P_DEV_FSM_INFO_T));

		prP2pDevFsmInfo->eCurrentState = P2P_DEV_STATE_IDLE;

		cnmTimerInitTimer(prAdapter,
				  &(prP2pDevFsmInfo->rP2pFsmTimeoutTimer),
				  (PFN_MGMT_TIMEOUT_FUNC) p2pDevFsmRunEventTimeout, (ULONG) prP2pDevFsmInfo);

		prP2pBssInfo = cnmGetBssInfoAndInit(prAdapter, NETWORK_TYPE_P2P, TRUE);

		if (prP2pBssInfo != NULL) {
			COPY_MAC_ADDR(prP2pBssInfo->aucOwnMacAddr, prAdapter->rMyMacAddr);
			prP2pBssInfo->aucOwnMacAddr[0] ^= 0x2;	/* change to local administrated address */

			prP2pDevFsmInfo->ucBssIndex = prP2pBssInfo->ucBssIndex;

			prP2pBssInfo->eCurrentOPMode = OP_MODE_P2P_DEVICE;
			prP2pBssInfo->ucConfigAdHocAPMode = AP_MODE_11G_P2P;
			prP2pBssInfo->u2HwDefaultFixedRateCode = RATE_OFDM_6M;

			prP2pBssInfo->eBand = BAND_2G4;
			if (prAdapter->rWifiVar.eDbdcMode == ENUM_DBDC_MODE_DISABLED)
				prP2pBssInfo->ucWmmQueSet = DBDC_5G_WMM_INDEX;
			else
				prP2pBssInfo->ucWmmQueSet = DBDC_2G_WMM_INDEX;

			prP2pBssInfo->ucPhyTypeSet = prAdapter->rWifiVar.ucAvailablePhyTypeSet & PHY_TYPE_SET_802_11GN;

			prP2pBssInfo->ucNonHTBasicPhyType = (UINT_8)
			    rNonHTApModeAttributes[prP2pBssInfo->ucConfigAdHocAPMode].ePhyTypeIndex;
			prP2pBssInfo->u2BSSBasicRateSet =
			    rNonHTApModeAttributes[prP2pBssInfo->ucConfigAdHocAPMode].u2BSSBasicRateSet;

			prP2pBssInfo->u2OperationalRateSet =
			    rNonHTPhyAttributes[prP2pBssInfo->ucNonHTBasicPhyType].u2SupportedRateSet;
			prP2pBssInfo->u4PrivateData = 0;/* TH3 Huang */

			rateGetDataRatesFromRateSet(prP2pBssInfo->u2OperationalRateSet,
					    prP2pBssInfo->u2BSSBasicRateSet,
					    prP2pBssInfo->aucAllSupportedRates, &prP2pBssInfo->ucAllSupportedRatesLen);
		}
		prP2pChnlReqInfo = &prP2pDevFsmInfo->rChnlReqInfo;
		LINK_INITIALIZE(&prP2pChnlReqInfo->rP2pChnlReqLink);

		prP2pMgmtTxReqInfo = &prP2pDevFsmInfo->rMgmtTxInfo;
		LINK_INITIALIZE(&prP2pMgmtTxReqInfo->rP2pTxReqLink);

		p2pDevFsmStateTransition(prAdapter, prP2pDevFsmInfo, P2P_DEV_STATE_IDLE);
	} while (FALSE);

	if (prP2pBssInfo)
		return prP2pBssInfo->ucBssIndex;
	else
		return prAdapter->ucP2PDevBssIdx + 1;

#if 0
	do {
		ASSERT_BREAK(prAdapter != NULL);

		prP2pFsmInfo = prAdapter->rWifiVar.prP2pFsmInfo;
		prP2pBssInfo = &(prAdapter->rWifiVar.arBssInfo[NETWORK_TYPE_P2P_INDEX]);

		ASSERT_BREAK(prP2pFsmInfo != NULL);

		LINK_INITIALIZE(&(prP2pFsmInfo->rMsgEventQueue));

		prP2pFsmInfo->eCurrentState = prP2pFsmInfo->ePreviousState = P2P_STATE_IDLE;
		prP2pFsmInfo->prTargetBss = NULL;

		cnmTimerInitTimer(prAdapter,
				  &(prP2pFsmInfo->rP2pFsmTimeoutTimer),
				  (PFN_MGMT_TIMEOUT_FUNC) p2pFsmRunEventFsmTimeout, (ULONG) prP2pFsmInfo);

		/* 4 <2> Initiate BSS_INFO_T - common part */
		BSS_INFO_INIT(prAdapter, NETWORK_TYPE_P2P_INDEX);

		/* 4 <2.1> Initiate BSS_INFO_T - Setup HW ID */
		prP2pBssInfo->ucConfigAdHocAPMode = AP_MODE_11G_P2P;
		prP2pBssInfo->ucHwDefaultFixedRateCode = RATE_OFDM_6M;

		prP2pBssInfo->ucNonHTBasicPhyType = (UINT_8)
		    rNonHTApModeAttributes[prP2pBssInfo->ucConfigAdHocAPMode].ePhyTypeIndex;
		prP2pBssInfo->u2BSSBasicRateSet =
		    rNonHTApModeAttributes[prP2pBssInfo->ucConfigAdHocAPMode].u2BSSBasicRateSet;

		prP2pBssInfo->u2OperationalRateSet =
		    rNonHTPhyAttributes[prP2pBssInfo->ucNonHTBasicPhyType].u2SupportedRateSet;

		rateGetDataRatesFromRateSet(prP2pBssInfo->u2OperationalRateSet,
					    prP2pBssInfo->u2BSSBasicRateSet,
					    prP2pBssInfo->aucAllSupportedRates, &prP2pBssInfo->ucAllSupportedRatesLen);

		prP2pBssInfo->prBeacon = cnmMgtPktAlloc(prAdapter,
							OFFSET_OF(WLAN_BEACON_FRAME_T, aucInfoElem[0]) + MAX_IE_LENGTH);

		if (prP2pBssInfo->prBeacon) {
			prP2pBssInfo->prBeacon->eSrc = TX_PACKET_MGMT;
			prP2pBssInfo->prBeacon->ucStaRecIndex = 0xFF;	/* NULL STA_REC */
			prP2pBssInfo->prBeacon->ucNetworkType = NETWORK_TYPE_P2P_INDEX;
		} else {
			/* Out of memory. */
			ASSERT(FALSE);
		}

		prP2pBssInfo->eCurrentOPMode = OP_MODE_NUM;

		prP2pBssInfo->rPmProfSetupInfo.ucBmpDeliveryAC = PM_UAPSD_ALL;
		prP2pBssInfo->rPmProfSetupInfo.ucBmpTriggerAC = PM_UAPSD_ALL;
		prP2pBssInfo->rPmProfSetupInfo.ucUapsdSp = WMM_MAX_SP_LENGTH_2;
		prP2pBssInfo->ucPrimaryChannel = P2P_DEFAULT_LISTEN_CHANNEL;
		prP2pBssInfo->eBand = BAND_2G4;
		prP2pBssInfo->eBssSCO = CHNL_EXT_SCN;

		if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucQoS))
			prP2pBssInfo->fgIsQBSS = TRUE;
		else
			prP2pBssInfo->fgIsQBSS = FALSE;

		SET_NET_PWR_STATE_IDLE(prAdapter, NETWORK_TYPE_P2P_INDEX);

		p2pFsmStateTransition(prAdapter, prP2pFsmInfo, P2P_STATE_IDLE);
	} while (FALSE);

	return;
#endif
}				/* p2pDevFsmInit */

VOID p2pDevFsmUninit(IN P_ADAPTER_T prAdapter)
{
	P_P2P_DEV_FSM_INFO_T prP2pDevFsmInfo = (P_P2P_DEV_FSM_INFO_T) NULL;
	P_BSS_INFO_T prP2pBssInfo = (P_BSS_INFO_T) NULL;

	do {
		ASSERT_BREAK(prAdapter != NULL);

		prP2pDevFsmInfo = prAdapter->rWifiVar.prP2pDevFsmInfo;

		ASSERT_BREAK(prP2pDevFsmInfo != NULL);

		prP2pBssInfo = prAdapter->aprBssInfo[prP2pDevFsmInfo->ucBssIndex];

		cnmTimerStopTimer(prAdapter, &(prP2pDevFsmInfo->rP2pFsmTimeoutTimer));

		/* Abort device FSM */
		p2pDevFsmStateTransition(prAdapter, prP2pDevFsmInfo, P2P_DEV_STATE_IDLE);
		p2pDevFsmRunEventAbort(prAdapter, prP2pDevFsmInfo);

		SET_NET_PWR_STATE_IDLE(prAdapter, prP2pBssInfo->ucBssIndex);

		/* Clear CmdQue */
		kalClearMgmtFramesByBssIdx(prAdapter->prGlueInfo, prP2pBssInfo->ucBssIndex);
		kalClearSecurityFramesByBssIdx(prAdapter->prGlueInfo, prP2pBssInfo->ucBssIndex);
		/* Clear PendingCmdQue */
		wlanReleasePendingCMDbyBssIdx(prAdapter, prP2pBssInfo->ucBssIndex);
		/* Clear PendingTxMsdu */
		nicFreePendingTxMsduInfoByBssIdx(prAdapter, prP2pBssInfo->ucBssIndex);

		/* Deactivate BSS. */
		UNSET_NET_ACTIVE(prAdapter, prP2pBssInfo->ucBssIndex);

		nicDeactivateNetwork(prAdapter, prP2pBssInfo->ucBssIndex);

		cnmFreeBssInfo(prAdapter, prP2pBssInfo);
	} while (FALSE);

#if 0
	P_P2P_FSM_INFO_T prP2pFsmInfo = (P_P2P_FSM_INFO_T) NULL;
	P_BSS_INFO_T prP2pBssInfo = (P_BSS_INFO_T) NULL;

	do {
		ASSERT_BREAK(prAdapter != NULL);

		DEBUGFUNC("p2pFsmUninit()");
		DBGLOG(P2P, INFO, "->p2pFsmUninit()\n");

		prP2pFsmInfo = prAdapter->rWifiVar.prP2pFsmInfo;
		prP2pBssInfo = &(prAdapter->rWifiVar.arBssInfo[NETWORK_TYPE_P2P_INDEX]);

		p2pFuncSwitchOPMode(prAdapter, prP2pBssInfo, OP_MODE_P2P_DEVICE, TRUE);

		p2pFsmRunEventAbort(prAdapter, prP2pFsmInfo);

		p2pStateAbort_IDLE(prAdapter, prP2pFsmInfo, P2P_STATE_NUM);

		UNSET_NET_ACTIVE(prAdapter, NETWORK_TYPE_P2P_INDEX);

		wlanAcquirePowerControl(prAdapter);

		/* Release all pending CMD queue. */
		DBGLOG(P2P, TRACE,
		       "p2pFsmUninit: wlanProcessCommandQueue, num of element:%d\n",
		       prAdapter->prGlueInfo->rCmdQueue.u4NumElem);
		wlanProcessCommandQueue(prAdapter, &prAdapter->prGlueInfo->rCmdQueue);

		wlanReleasePowerControl(prAdapter);

		/* Release pending mgmt frame,
		 * mgmt frame may be pending by CMD without resource.
		 */
		kalClearMgmtFramesByBssIdx(prAdapter->prGlueInfo, NETWORK_TYPE_P2P_INDEX);

		/* Clear PendingCmdQue */
		wlanReleasePendingCMDbyBssIdx(prAdapter, NETWORK_TYPE_P2P_INDEX);

		if (prP2pBssInfo->prBeacon) {
			cnmMgtPktFree(prAdapter, prP2pBssInfo->prBeacon);
			prP2pBssInfo->prBeacon = NULL;
		}
	} while (FALSE);

	return;
#endif
}				/* p2pDevFsmUninit */

VOID
p2pDevFsmStateTransition(IN P_ADAPTER_T prAdapter,
			 IN P_P2P_DEV_FSM_INFO_T prP2pDevFsmInfo, IN ENUM_P2P_DEV_STATE_T eNextState)
{
	BOOLEAN fgIsLeaveState = (BOOLEAN) FALSE;

	ASSERT(prP2pDevFsmInfo);
	if (!prP2pDevFsmInfo) {
		DBGLOG(P2P, ERROR, "prP2pDevFsmInfo is NULL!\n");
		return;
	}

	ASSERT(prP2pDevFsmInfo->ucBssIndex == prAdapter->ucP2PDevBssIdx);
	if (prP2pDevFsmInfo->ucBssIndex != prAdapter->ucP2PDevBssIdx) {
		DBGLOG(P2P, ERROR,
			"prP2pDevFsmInfo->ucBssIndex %s should be prAdapter->ucP2PDevBssIdx(%d)!\n",
			prP2pDevFsmInfo->ucBssIndex, prAdapter->ucP2PDevBssIdx);
		return;
	}

	do {
		if (!IS_BSS_ACTIVE(prAdapter->aprBssInfo[prP2pDevFsmInfo->ucBssIndex])) {
			if (!cnmP2PIsPermitted(prAdapter))
				return;

			SET_NET_ACTIVE(prAdapter, prP2pDevFsmInfo->ucBssIndex);
			nicActivateNetwork(prAdapter, prP2pDevFsmInfo->ucBssIndex);
		}

		fgIsLeaveState = fgIsLeaveState ? FALSE : TRUE;

		if (!fgIsLeaveState) {
			DBGLOG(P2P, STATE, "[P2P_DEV]TRANSITION: [%s] -> [%s]\n",
			       apucDebugP2pDevState[prP2pDevFsmInfo->eCurrentState], apucDebugP2pDevState[eNextState]);

			/* Transition into current state. */
			prP2pDevFsmInfo->eCurrentState = eNextState;
		}

		switch (prP2pDevFsmInfo->eCurrentState) {
		case P2P_DEV_STATE_IDLE:
			if (!fgIsLeaveState) {
				fgIsLeaveState = p2pDevStateInit_IDLE(prAdapter,
								      &prP2pDevFsmInfo->rChnlReqInfo, &eNextState);
			} else {
				p2pDevStateAbort_IDLE(prAdapter);
			}
			break;
		case P2P_DEV_STATE_SCAN:
			if (!fgIsLeaveState) {
				p2pDevStateInit_SCAN(prAdapter,
						     prP2pDevFsmInfo->ucBssIndex, &prP2pDevFsmInfo->rScanReqInfo);
			} else {
				p2pDevStateAbort_SCAN(prAdapter, prP2pDevFsmInfo);
			}
			break;
		case P2P_DEV_STATE_REQING_CHANNEL:
			if (!fgIsLeaveState) {
				fgIsLeaveState = p2pDevStateInit_REQING_CHANNEL(prAdapter,
										prP2pDevFsmInfo->ucBssIndex,
										&(prP2pDevFsmInfo->rChnlReqInfo),
										&eNextState);
			} else {
				p2pDevStateAbort_REQING_CHANNEL(prAdapter,
								&(prP2pDevFsmInfo->rChnlReqInfo), eNextState);
			}
			break;
		case P2P_DEV_STATE_CHNL_ON_HAND:
			if (!fgIsLeaveState) {
				p2pDevStateInit_CHNL_ON_HAND(prAdapter,
							     prAdapter->aprBssInfo[prP2pDevFsmInfo->ucBssIndex],
							     prP2pDevFsmInfo, &(prP2pDevFsmInfo->rChnlReqInfo));
			} else {
				p2pDevStateAbort_CHNL_ON_HAND(prAdapter,
							      prAdapter->aprBssInfo[prP2pDevFsmInfo->ucBssIndex],
							      prP2pDevFsmInfo, &(prP2pDevFsmInfo->rChnlReqInfo),
							      eNextState);
			}
			break;
		case P2P_DEV_STATE_OFF_CHNL_TX:
			if (!fgIsLeaveState) {
				fgIsLeaveState = p2pDevStateInit_OFF_CHNL_TX(prAdapter,
									     prP2pDevFsmInfo,
									     &(prP2pDevFsmInfo->rChnlReqInfo),
									     &(prP2pDevFsmInfo->rMgmtTxInfo),
									     &eNextState);
			} else {
				p2pDevStateAbort_OFF_CHNL_TX(prAdapter,
							     &(prP2pDevFsmInfo->rMgmtTxInfo),
							     &(prP2pDevFsmInfo->rChnlReqInfo), eNextState);
			}
			break;
		default:
			/* Unexpected state. */
			ASSERT(FALSE);
			break;
		}
	} while (fgIsLeaveState);
}				/* p2pDevFsmStateTransition */

VOID p2pDevFsmRunEventAbort(IN P_ADAPTER_T prAdapter, IN P_P2P_DEV_FSM_INFO_T prP2pDevFsmInfo)
{
	do {
		ASSERT_BREAK((prAdapter != NULL) && (prP2pDevFsmInfo != NULL));

		if (prP2pDevFsmInfo->eCurrentState != P2P_DEV_STATE_IDLE) {
			/* Get into IDLE state. */
			p2pDevFsmStateTransition(prAdapter, prP2pDevFsmInfo, P2P_DEV_STATE_IDLE);
		}

		/* Abort IDLE. */
		p2pDevStateAbort_IDLE(prAdapter);
	} while (FALSE);
}				/* p2pDevFsmRunEventAbort */

VOID p2pDevFsmRunEventTimeout(IN P_ADAPTER_T prAdapter, IN ULONG ulParamPtr)
{
	P_P2P_DEV_FSM_INFO_T prP2pDevFsmInfo = (P_P2P_DEV_FSM_INFO_T) ulParamPtr;

	do {
		ASSERT_BREAK((prAdapter != NULL) && (prP2pDevFsmInfo != NULL));

		switch (prP2pDevFsmInfo->eCurrentState) {
		case P2P_DEV_STATE_IDLE:
			/* TODO: IDLE timeout for low power mode. */
			break;
		case P2P_DEV_STATE_CHNL_ON_HAND:
			switch (prAdapter->prP2pInfo->eConnState) {
			case P2P_CNN_GO_NEG_REQ:
			case P2P_CNN_GO_NEG_RESP:
			case P2P_CNN_INVITATION_REQ:
			case P2P_CNN_DEV_DISC_REQ:
			case P2P_CNN_PROV_DISC_REQ:
				DBGLOG(P2P, INFO, "P2P: re-enter CHNL_ON_HAND with state: %d\n",
				       prAdapter->prP2pInfo->eConnState);
				p2pDevFsmStateTransition(prAdapter, prP2pDevFsmInfo,
							 P2P_DEV_STATE_CHNL_ON_HAND);
				break;
			case P2P_CNN_NORMAL:
			case P2P_CNN_GO_NEG_CONF:
			case P2P_CNN_INVITATION_RESP:
			case P2P_CNN_DEV_DISC_RESP:
			case P2P_CNN_PROV_DISC_RES:
			default:
				p2pDevFsmStateTransition(prAdapter, prP2pDevFsmInfo,
							 P2P_DEV_STATE_IDLE);
				break;
			}
			break;
		default:
			ASSERT(FALSE);
			DBGLOG(P2P, ERROR,
			       "Current P2P Dev State %d is unexpected for FSM timeout event.\n",
			       prP2pDevFsmInfo->eCurrentState);
			break;
		}
	} while (FALSE);
}				/* p2pDevFsmRunEventTimeout */

/*================ Message Event =================*/
VOID p2pDevFsmRunEventScanRequest(IN P_ADAPTER_T prAdapter, IN P_MSG_HDR_T prMsgHdr)
{
	P_MSG_P2P_SCAN_REQUEST_T prP2pScanReqMsg = (P_MSG_P2P_SCAN_REQUEST_T) NULL;
	P_P2P_DEV_FSM_INFO_T prP2pDevFsmInfo = (P_P2P_DEV_FSM_INFO_T) NULL;
	P_P2P_SCAN_REQ_INFO_T prScanReqInfo = (P_P2P_SCAN_REQ_INFO_T) NULL;
	UINT_32 u4ChnlListSize = 0;
	P_P2P_SSID_STRUCT_T prP2pSsidStruct = (P_P2P_SSID_STRUCT_T) NULL;

	do {
		ASSERT_BREAK((prAdapter != NULL) && (prMsgHdr != NULL));

		prP2pDevFsmInfo = prAdapter->rWifiVar.prP2pDevFsmInfo;

		if (prP2pDevFsmInfo == NULL)
			break;

		if (prP2pDevFsmInfo->eCurrentState != P2P_DEV_STATE_IDLE)
			p2pDevFsmRunEventAbort(prAdapter, prP2pDevFsmInfo);

		prP2pScanReqMsg = (P_MSG_P2P_SCAN_REQUEST_T) prMsgHdr;
		prScanReqInfo = &(prP2pDevFsmInfo->rScanReqInfo);

		DBGLOG(P2P, TRACE, "p2pDevFsmRunEventScanRequest\n");

		/* Do we need to be in IDLE state? */
		/* p2pDevFsmRunEventAbort(prAdapter, prP2pDevFsmInfo); */

		ASSERT(prScanReqInfo->fgIsScanRequest == FALSE);

		prScanReqInfo->fgIsAbort = TRUE;
		prScanReqInfo->eScanType = prP2pScanReqMsg->eScanType;
		prScanReqInfo->u2PassiveDewellTime = 0;

		if (prP2pScanReqMsg->u4NumChannel) {
			prScanReqInfo->eChannelSet = SCAN_CHANNEL_SPECIFIED;

			/* Channel List */
			prScanReqInfo->ucNumChannelList = prP2pScanReqMsg->u4NumChannel;
			DBGLOG(P2P, TRACE, "Scan Request Channel List Number: %d\n", prScanReqInfo->ucNumChannelList);
			if (prScanReqInfo->ucNumChannelList > MAXIMUM_OPERATION_CHANNEL_LIST) {
				DBGLOG(P2P, TRACE,
				       "Channel List Number Overloaded: %d, change to: %d\n",
				       prScanReqInfo->ucNumChannelList, MAXIMUM_OPERATION_CHANNEL_LIST);
				prScanReqInfo->ucNumChannelList = MAXIMUM_OPERATION_CHANNEL_LIST;
			}

			u4ChnlListSize = sizeof(RF_CHANNEL_INFO_T) * prScanReqInfo->ucNumChannelList;
			kalMemCopy(prScanReqInfo->arScanChannelList,
				   prP2pScanReqMsg->arChannelListInfo, u4ChnlListSize);
		} else {
			/* If channel number is ZERO.
			 * It means do a FULL channel scan.
			 */
			prScanReqInfo->eChannelSet = SCAN_CHANNEL_FULL;
		}

		/* SSID */
		prP2pSsidStruct = prP2pScanReqMsg->prSSID;
		for (prScanReqInfo->ucSsidNum = 0;
		     prScanReqInfo->ucSsidNum < prP2pScanReqMsg->i4SsidNum; prScanReqInfo->ucSsidNum++) {
			kalMemCopy(prScanReqInfo->arSsidStruct[prScanReqInfo->ucSsidNum].aucSsid,
				   prP2pSsidStruct->aucSsid, prP2pSsidStruct->ucSsidLen);

			prScanReqInfo->arSsidStruct[prScanReqInfo->ucSsidNum].ucSsidLen = prP2pSsidStruct->ucSsidLen;

			prP2pSsidStruct++;
		}

		/* IE Buffer */
		kalMemCopy(prScanReqInfo->aucIEBuf, prP2pScanReqMsg->pucIEBuf, prP2pScanReqMsg->u4IELen);

		prScanReqInfo->u4BufLength = prP2pScanReqMsg->u4IELen;

		p2pDevFsmStateTransition(prAdapter, prP2pDevFsmInfo, P2P_DEV_STATE_SCAN);
	} while (FALSE);

	if (prMsgHdr)
		cnmMemFree(prAdapter, prMsgHdr);
}				/* p2pDevFsmRunEventScanRequest */

VOID p2pDevFsmRunEventScanAbort(IN P_ADAPTER_T prAdapter, IN P_MSG_HDR_T prMsgHdr)
{
	P_P2P_DEV_FSM_INFO_T prP2pDevFsmInfo = (P_P2P_DEV_FSM_INFO_T) NULL;

	do {
		ASSERT_BREAK(prAdapter != NULL);

		DBGLOG(P2P, TRACE, "p2pDevFsmRunEventScanAbort\n");

		prP2pDevFsmInfo = prAdapter->rWifiVar.prP2pDevFsmInfo;

		if (prP2pDevFsmInfo->eCurrentState == P2P_DEV_STATE_SCAN) {
			P_P2P_SCAN_REQ_INFO_T prScanReqInfo = &(prP2pDevFsmInfo->rScanReqInfo);

			prScanReqInfo->fgIsAbort = TRUE;

			p2pDevFsmStateTransition(prAdapter, prP2pDevFsmInfo, P2P_DEV_STATE_IDLE);
		}

	} while (FALSE);

	if (prMsgHdr)
		cnmMemFree(prAdapter, prMsgHdr);

}				/* p2pDevFsmRunEventScanAbort */

VOID
p2pDevFsmRunEventScanDone(IN P_ADAPTER_T prAdapter, IN P_MSG_HDR_T prMsgHdr, IN P_P2P_DEV_FSM_INFO_T prP2pDevFsmInfo)
{
	P_MSG_SCN_SCAN_DONE prScanDoneMsg = (P_MSG_SCN_SCAN_DONE) prMsgHdr;
	P_P2P_SCAN_REQ_INFO_T prP2pScanReqInfo = (P_P2P_SCAN_REQ_INFO_T) NULL;

	do {
		ASSERT_BREAK((prAdapter != NULL) && (prMsgHdr != NULL) && (prP2pDevFsmInfo != NULL));

		if (!prP2pDevFsmInfo) {
			DBGLOG(P2P, ERROR, "prP2pDevFsmInfo is null, maybe remove p2p already\n");
			break;
		}

		prP2pScanReqInfo = &(prP2pDevFsmInfo->rScanReqInfo);

		if (prScanDoneMsg->ucSeqNum != prP2pScanReqInfo->ucSeqNumOfScnMsg) {
			DBGLOG(P2P, TRACE,
			       "P2P Scan Done SeqNum:%d  <->   P2P Dev FSM Scan SeqNum:%d",
			       prScanDoneMsg->ucSeqNum, prP2pScanReqInfo->ucSeqNumOfScnMsg);
			break;
		}

		ASSERT_BREAK(prScanDoneMsg->ucBssIndex == prP2pDevFsmInfo->ucBssIndex);

		prP2pScanReqInfo->fgIsAbort = FALSE;
		prP2pScanReqInfo->fgIsScanRequest = FALSE;

		p2pDevFsmStateTransition(prAdapter, prP2pDevFsmInfo, P2P_DEV_STATE_IDLE);
	} while (FALSE);

	if (prMsgHdr)
		cnmMemFree(prAdapter, prMsgHdr);
}				/* p2pDevFsmRunEventScanDone */

VOID p2pDevFsmRunEventChannelRequest(IN P_ADAPTER_T prAdapter, IN P_MSG_HDR_T prMsgHdr)
{
	P_P2P_DEV_FSM_INFO_T prP2pDevFsmInfo = (P_P2P_DEV_FSM_INFO_T) NULL;
	P_P2P_CHNL_REQ_INFO_T prChnlReqInfo = (P_P2P_CHNL_REQ_INFO_T) NULL;
	BOOLEAN fgIsChnlFound = FALSE;

	do {
		ASSERT_BREAK((prAdapter != NULL) && (prMsgHdr != NULL));

		prP2pDevFsmInfo = prAdapter->rWifiVar.prP2pDevFsmInfo;

		if (prP2pDevFsmInfo == NULL) {
			DBGLOG(P2P, WARN, "uninitialized p2p Dev fsm\n");
			break;
		}

		prChnlReqInfo = &(prP2pDevFsmInfo->rChnlReqInfo);

		DBGLOG(P2P, TRACE, "p2pDevFsmRunEventChannelRequest\n");

		/* printk("p2pDevFsmRunEventChannelRequest check cookie =%lld\n",prChnlReqInfo->u8Cookie); */

		if (!LINK_IS_EMPTY(&prChnlReqInfo->rP2pChnlReqLink)) {
			P_LINK_ENTRY_T prLinkEntry = (P_LINK_ENTRY_T) NULL;
			P_MSG_P2P_CHNL_REQUEST_T prP2pMsgChnlReq = (P_MSG_P2P_CHNL_REQUEST_T) NULL;

			LINK_FOR_EACH(prLinkEntry, &prChnlReqInfo->rP2pChnlReqLink) {
				prP2pMsgChnlReq =
				    (P_MSG_P2P_CHNL_REQUEST_T) LINK_ENTRY(prLinkEntry, MSG_HDR_T, rLinkEntry);

				if (prP2pMsgChnlReq->eChnlReqType == CH_REQ_TYPE_P2P_LISTEN) {
					LINK_REMOVE_KNOWN_ENTRY(&prChnlReqInfo->rP2pChnlReqLink, prLinkEntry);
					cnmMemFree(prAdapter, prP2pMsgChnlReq);
					/* DBGLOG(P2P, TRACE, */
					/* ("p2pDevFsmRunEventChannelAbort: Channel Abort, cookie found:%d\n", */
					/* prChnlAbortMsg->u8Cookie)); */
					fgIsChnlFound = TRUE;
					break;
				}
			}
		}

		/* Queue the channel request. */
		LINK_INSERT_TAIL(&(prChnlReqInfo->rP2pChnlReqLink), &(prMsgHdr->rLinkEntry));
		prMsgHdr = NULL;

		/* If channel is not requested, it may due to channel is released. */
		if ((!fgIsChnlFound) &&
		    (prChnlReqInfo->eChnlReqType == CH_REQ_TYPE_P2P_LISTEN) && (prChnlReqInfo->fgIsChannelRequested)) {
			ASSERT((prP2pDevFsmInfo->eCurrentState == P2P_DEV_STATE_REQING_CHANNEL) ||
			       (prP2pDevFsmInfo->eCurrentState == P2P_DEV_STATE_CHNL_ON_HAND));

			p2pDevFsmRunEventAbort(prAdapter, prP2pDevFsmInfo);

			break;
		}

		if (prP2pDevFsmInfo->eCurrentState == P2P_DEV_STATE_IDLE) {
			/* Re-enter IDLE state would trigger channel request. */
			p2pDevFsmStateTransition(prAdapter, prP2pDevFsmInfo, P2P_DEV_STATE_IDLE);
		}
	} while (FALSE);

	if (prMsgHdr)
		cnmMemFree(prAdapter, prMsgHdr);
}				/* p2pDevFsmRunEventChannelRequest */

VOID p2pDevFsmRunEventChannelAbort(IN P_ADAPTER_T prAdapter, IN P_MSG_HDR_T prMsgHdr)
{
	P_P2P_DEV_FSM_INFO_T prP2pDevFsmInfo = (P_P2P_DEV_FSM_INFO_T) NULL;
	P_MSG_P2P_CHNL_ABORT_T prChnlAbortMsg = (P_MSG_P2P_CHNL_ABORT_T) NULL;
	P_P2P_CHNL_REQ_INFO_T prChnlReqInfo = (P_P2P_CHNL_REQ_INFO_T) NULL;

	do {
		ASSERT_BREAK((prAdapter != NULL) && (prMsgHdr != NULL));

		prChnlAbortMsg = (P_MSG_P2P_CHNL_ABORT_T) prMsgHdr;

		prP2pDevFsmInfo = prAdapter->rWifiVar.prP2pDevFsmInfo;

		if (prP2pDevFsmInfo == NULL)
			break;

		prChnlReqInfo = &(prP2pDevFsmInfo->rChnlReqInfo);

		DBGLOG(P2P, TRACE, "p2pDevFsmRunEventChannelAbort\n");

		/* If channel is not requested, it may due to channel is released. */
		if ((prChnlAbortMsg->u8Cookie == prChnlReqInfo->u8Cookie) && (prChnlReqInfo->fgIsChannelRequested)) {
			ASSERT((prP2pDevFsmInfo->eCurrentState == P2P_DEV_STATE_REQING_CHANNEL) ||
			       (prP2pDevFsmInfo->eCurrentState == P2P_DEV_STATE_CHNL_ON_HAND));
			/*
			 * If cancel-roc cmd is called from Supplicant while driver is waiting
			 * for FW's channel grant event, roc event must be returned to Supplicant
			 * first to reset Supplicant's variables and then transition to idle state.
			 */
			if (prP2pDevFsmInfo->eCurrentState == P2P_DEV_STATE_REQING_CHANNEL) {
				kalP2PIndicateChannelReady(prAdapter->prGlueInfo,
							   prChnlReqInfo->u8Cookie,
							   prChnlReqInfo->ucReqChnlNum,
							   prChnlReqInfo->eBand,
							   prChnlReqInfo->eChnlSco,
							   prChnlReqInfo->u4MaxInterval);
				kalP2PIndicateChannelExpired(prAdapter->prGlueInfo,
								 prChnlReqInfo->u8Cookie,
								 prChnlReqInfo->ucReqChnlNum,
								 prChnlReqInfo->eBand,
								 prChnlReqInfo->eChnlSco);
			}
			p2pDevFsmRunEventAbort(prAdapter, prP2pDevFsmInfo);

			break;
		} else if (!LINK_IS_EMPTY(&prChnlReqInfo->rP2pChnlReqLink)) {
			P_LINK_ENTRY_T prLinkEntry = (P_LINK_ENTRY_T) NULL;
			P_MSG_P2P_CHNL_REQUEST_T prP2pMsgChnlReq = (P_MSG_P2P_CHNL_REQUEST_T) NULL;

			LINK_FOR_EACH(prLinkEntry, &prChnlReqInfo->rP2pChnlReqLink) {
				prP2pMsgChnlReq =
				    (P_MSG_P2P_CHNL_REQUEST_T) LINK_ENTRY(prLinkEntry, MSG_HDR_T, rLinkEntry);

				if (prP2pMsgChnlReq->u8Cookie == prChnlAbortMsg->u8Cookie) {
					LINK_REMOVE_KNOWN_ENTRY(&prChnlReqInfo->rP2pChnlReqLink, prLinkEntry);
					DBGLOG(P2P, TRACE,
						"p2pDevFsmRunEventChannelAbort: Channel Abort, cookie found:%d\n",
						prChnlAbortMsg->u8Cookie);
					kalP2PIndicateChannelReady(prAdapter->prGlueInfo,
							prP2pMsgChnlReq->u8Cookie,
							prP2pMsgChnlReq->rChannelInfo.ucChannelNum,
							prP2pMsgChnlReq->rChannelInfo.eBand,
							prP2pMsgChnlReq->eChnlSco,
							prP2pMsgChnlReq->u4Duration);
					kalP2PIndicateChannelExpired(prAdapter->prGlueInfo,
							prP2pMsgChnlReq->u8Cookie,
							prP2pMsgChnlReq->rChannelInfo.ucChannelNum,
							prP2pMsgChnlReq->rChannelInfo.eBand,
							prP2pMsgChnlReq->eChnlSco);
					cnmMemFree(prAdapter, prP2pMsgChnlReq);
					break;
				}
			}
		} else {
			DBGLOG(P2P, WARN,
			       "p2pDevFsmRunEventChannelAbort: Channel Abort Fail, cookie not found:%d\n",
			       prChnlAbortMsg->u8Cookie);
		}
	} while (FALSE);

	if (prMsgHdr)
		cnmMemFree(prAdapter, prMsgHdr);
}				/* p2pDevFsmRunEventChannelAbort */

VOID
p2pDevFsmRunEventChnlGrant(IN P_ADAPTER_T prAdapter, IN P_MSG_HDR_T prMsgHdr, IN P_P2P_DEV_FSM_INFO_T prP2pDevFsmInfo)
{
	P_MSG_CH_GRANT_T prMsgChGrant = (P_MSG_CH_GRANT_T) NULL;
	P_P2P_CHNL_REQ_INFO_T prChnlReqInfo = (P_P2P_CHNL_REQ_INFO_T) NULL;

	do {
		ASSERT((prAdapter != NULL) && (prMsgHdr != NULL) && (prP2pDevFsmInfo != NULL));
		if ((prAdapter == NULL) || (prMsgHdr == NULL) || (prP2pDevFsmInfo == NULL))
			break;

		prMsgChGrant = (P_MSG_CH_GRANT_T) prMsgHdr;
		prChnlReqInfo = &(prP2pDevFsmInfo->rChnlReqInfo);

		if ((prMsgChGrant->ucTokenID != prChnlReqInfo->ucSeqNumOfChReq) ||
		    (!prChnlReqInfo->fgIsChannelRequested)) {
			break;
		}

		ASSERT(prMsgChGrant->ucPrimaryChannel == prChnlReqInfo->ucReqChnlNum);
		ASSERT(prMsgChGrant->eReqType == prChnlReqInfo->eChnlReqType);
		ASSERT(prMsgChGrant->u4GrantInterval == prChnlReqInfo->u4MaxInterval);
		prChnlReqInfo->u4MaxInterval = prMsgChGrant->u4GrantInterval;

		if (prMsgChGrant->eReqType == CH_REQ_TYPE_P2P_LISTEN) {
			p2pDevFsmStateTransition(prAdapter, prP2pDevFsmInfo, P2P_DEV_STATE_CHNL_ON_HAND);
		} else {
			ASSERT(prMsgChGrant->eReqType == CH_REQ_TYPE_OFFCHNL_TX);
			p2pDevFsmStateTransition(prAdapter, prP2pDevFsmInfo, P2P_DEV_STATE_OFF_CHNL_TX);
		}
	} while (FALSE);

	if (prMsgHdr)
		cnmMemFree(prAdapter, prMsgHdr);
}				/* p2pDevFsmRunEventChnlGrant */

VOID p2pDevFsmRunEventMgmtTx(IN P_ADAPTER_T prAdapter, IN P_MSG_HDR_T prMsgHdr)
{
	P_P2P_DEV_FSM_INFO_T prP2pDevFsmInfo = (P_P2P_DEV_FSM_INFO_T) NULL;
	P_MSG_P2P_MGMT_TX_REQUEST_T prMgmtTxMsg = (P_MSG_P2P_MGMT_TX_REQUEST_T) NULL;
	P_P2P_CHNL_REQ_INFO_T prP2pChnlReqInfo = (P_P2P_CHNL_REQ_INFO_T) NULL;
	P_P2P_MGMT_TX_REQ_INFO_T prP2pMgmtTxReqInfo = (P_P2P_MGMT_TX_REQ_INFO_T) NULL;

	prMgmtTxMsg = (P_MSG_P2P_MGMT_TX_REQUEST_T) prMsgHdr;

	if ((prMgmtTxMsg->ucBssIdx != prAdapter->ucP2PDevBssIdx) &&
		(IS_NET_ACTIVE(prAdapter, prMgmtTxMsg->ucBssIdx))) {
		DBGLOG(P2P, TRACE, " Role Interface\n");
		p2pFuncTxMgmtFrame(prAdapter,
				   prMgmtTxMsg->ucBssIdx,
				   prMgmtTxMsg->prMgmtMsduInfo, prMgmtTxMsg->fgNoneCckRate);
		goto error;
	}

	DBGLOG(P2P, TRACE, "p2pDevFsmRunEventMgmtTx with Device Interface\n");

	prMgmtTxMsg->ucBssIdx = prAdapter->ucP2PDevBssIdx;

	prP2pDevFsmInfo = prAdapter->rWifiVar.prP2pDevFsmInfo;

	if (prP2pDevFsmInfo == NULL) {
		DBGLOG(P2P, ERROR, "prP2pDevFsmInfo is NULL!\n");
		goto error;
	}

	prP2pChnlReqInfo = &(prP2pDevFsmInfo->rChnlReqInfo);
	prP2pMgmtTxReqInfo = &(prP2pDevFsmInfo->rMgmtTxInfo);

	if ((!prMgmtTxMsg->fgIsOffChannel) ||
	    ((prP2pDevFsmInfo->eCurrentState == P2P_DEV_STATE_OFF_CHNL_TX) &&
	     (LINK_IS_EMPTY(&prP2pMgmtTxReqInfo->rP2pTxReqLink)))) {
		p2pFuncTxMgmtFrame(prAdapter,
				   prP2pDevFsmInfo->ucBssIndex,
				   prMgmtTxMsg->prMgmtMsduInfo, prMgmtTxMsg->fgNoneCckRate);
	} else {
		P_P2P_OFF_CHNL_TX_REQ_INFO_T prOffChnlTxReq = (P_P2P_OFF_CHNL_TX_REQ_INFO_T) NULL;

		prOffChnlTxReq = cnmMemAlloc(prAdapter, RAM_TYPE_MSG, sizeof(P2P_OFF_CHNL_TX_REQ_INFO_T));

		if (prOffChnlTxReq == NULL) {
			DBGLOG(P2P, ERROR, "Can not serve TX request due to MSG buffer not enough\n");
			ASSERT(FALSE);
			goto error;
		}

		prOffChnlTxReq->prMgmtTxMsdu = prMgmtTxMsg->prMgmtMsduInfo;
		prOffChnlTxReq->fgNoneCckRate = prMgmtTxMsg->fgNoneCckRate;
		kalMemCopy(&prOffChnlTxReq->rChannelInfo, &prMgmtTxMsg->rChannelInfo,
			   sizeof(RF_CHANNEL_INFO_T));
		prOffChnlTxReq->eChnlExt = prMgmtTxMsg->eChnlExt;
		prOffChnlTxReq->fgIsWaitRsp = prMgmtTxMsg->fgIsWaitRsp;

		LINK_INSERT_TAIL(&prP2pMgmtTxReqInfo->rP2pTxReqLink, &prOffChnlTxReq->rLinkEntry);

		/* Channel Request if needed. */
		if (prP2pDevFsmInfo->eCurrentState != P2P_DEV_STATE_OFF_CHNL_TX) {
			P_MSG_P2P_CHNL_REQUEST_T prP2pMsgChnlReq = (P_MSG_P2P_CHNL_REQUEST_T) NULL;

			prP2pMsgChnlReq = cnmMemAlloc(prAdapter, RAM_TYPE_MSG, sizeof(MSG_P2P_CHNL_REQUEST_T));

			if (prP2pMsgChnlReq == NULL) {
				cnmMemFree(prAdapter, prOffChnlTxReq);
				ASSERT(FALSE);
				DBGLOG(P2P, ERROR, "Not enough MSG buffer for channel request\n");
				goto error;
			}

			prP2pMsgChnlReq->eChnlReqType = CH_REQ_TYPE_OFFCHNL_TX;

			/* Not used in TX OFFCHNL REQ fields. */
			prP2pMsgChnlReq->rMsgHdr.eMsgId = MID_MNY_P2P_CHNL_REQ;
			prP2pMsgChnlReq->u8Cookie = 0;
			prP2pMsgChnlReq->u4Duration = P2P_OFF_CHNL_TX_DEFAULT_TIME_MS;

			kalMemCopy(&prP2pMsgChnlReq->rChannelInfo,
				   &prMgmtTxMsg->rChannelInfo, sizeof(RF_CHANNEL_INFO_T));
			prP2pMsgChnlReq->eChnlSco = prMgmtTxMsg->eChnlExt;

			p2pDevFsmRunEventChannelRequest(prAdapter, (P_MSG_HDR_T) prP2pMsgChnlReq);
		}
	}

error:
	cnmMemFree(prAdapter, prMsgHdr);
}				/* p2pDevFsmRunEventMgmtTx */

WLAN_STATUS
p2pDevFsmRunEventMgmtFrameTxDone(IN P_ADAPTER_T prAdapter,
				 IN P_MSDU_INFO_T prMsduInfo, IN ENUM_TX_RESULT_CODE_T rTxDoneStatus)
{
	BOOLEAN fgIsSuccess = FALSE;
	P_P2P_DEV_FSM_INFO_T prP2pDevFsmInfo = (P_P2P_DEV_FSM_INFO_T) NULL;
	PUINT_64 pu8GlCookie = (PUINT_64) NULL;

	do {
		ASSERT_BREAK((prAdapter != NULL) && (prMsduInfo != NULL));

		if (!prMsduInfo->prPacket) {
			DBGLOG(P2P, WARN, "Freed Msdu, do not indicate to host\n");
			break;
		}

		pu8GlCookie =
			(PUINT_64) ((ULONG) prMsduInfo->prPacket +
				(ULONG) prMsduInfo->u2FrameLength + MAC_TX_RESERVED_FIELD);

		prP2pDevFsmInfo = prAdapter->rWifiVar.prP2pDevFsmInfo;

		if (prP2pDevFsmInfo->eCurrentState == P2P_DEV_STATE_OFF_CHNL_TX)
			p2pDevFsmStateTransition(prAdapter, prP2pDevFsmInfo, P2P_DEV_STATE_OFF_CHNL_TX);

		if (rTxDoneStatus != TX_RESULT_SUCCESS) {
			DBGLOG(P2P, INFO, "Mgmt Frame TX Fail, Status: %d. cookie: 0x%llx\n",
				rTxDoneStatus, *pu8GlCookie);
		} else {
			fgIsSuccess = TRUE;
			DBGLOG(P2P, INFO, "Mgmt Frame TX Done. cookie: 0x%llx\n", *pu8GlCookie);
		}

		kalP2PIndicateMgmtTxStatus(prAdapter->prGlueInfo, prMsduInfo, fgIsSuccess);
	} while (FALSE);

	return WLAN_STATUS_SUCCESS;
}				/* p2pDevFsmRunEventMgmtFrameTxDone */

VOID p2pDevFsmRunEventMgmtFrameRegister(IN P_ADAPTER_T prAdapter, IN P_MSG_HDR_T prMsgHdr)
{
	/* TODO: RX Filter Management. */

	if (prMsgHdr)
		cnmMemFree(prAdapter, prMsgHdr);
}				/* p2pDevFsmRunEventMgmtFrameRegister */

VOID p2pDevFsmRunEventActiveDevBss(IN P_ADAPTER_T prAdapter, IN P_MSG_HDR_T prMsgHdr)
{
	P_P2P_DEV_FSM_INFO_T prP2pDevFsmInfo = (P_P2P_DEV_FSM_INFO_T) NULL;

	do {
		ASSERT_BREAK((prAdapter != NULL) && (prMsgHdr != NULL));
		prP2pDevFsmInfo = prAdapter->rWifiVar.prP2pDevFsmInfo;

		if (prP2pDevFsmInfo->eCurrentState == P2P_DEV_STATE_IDLE) {
			/* Get into IDLE state to let BSS be active and do not Deactive. */
			p2pDevFsmStateTransition(prAdapter, prP2pDevFsmInfo, P2P_DEV_STATE_IDLE);
		}

	} while (FALSE);

	if (prMsgHdr)
		cnmMemFree(prAdapter, prMsgHdr);
}				/* p2pDevFsmRunEventActiveDevBss */


#endif /* RunEventWfdSettingUpdate */
