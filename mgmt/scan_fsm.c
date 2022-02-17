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
/*
** Id: //Department/DaVinci/BRANCHES/MT6620_WIFI_DRIVER_V2_3/mgmt/scan_fsm.c#2
*/

/*! \file   "scan_fsm.c"
*    \brief  This file defines the state transition function for SCAN FSM.
*
*    The SCAN FSM is part of SCAN MODULE and responsible for performing basic SCAN
*    behavior as metioned in IEEE 802.11 2007 11.1.3.1 & 11.1.3.2 .
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
uint8_t g_aucScanChannelNum[SCN_SCAN_DONE_PRINT_BUFFER_LENGTH];
uint8_t g_aucScanChannelIdleTime[SCN_SCAN_DONE_PRINT_BUFFER_LENGTH];
uint8_t g_aucScanChannelMDRDY[SCN_SCAN_DONE_PRINT_BUFFER_LENGTH];
uint8_t g_aucScanChannelBeacon[SCN_SCAN_DONE_PRINT_BUFFER_LENGTH];

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/
static uint8_t *apucDebugScanState[SCAN_STATE_NUM] = {
	(uint8_t *) DISP_STRING("IDLE"),
	(uint8_t *) DISP_STRING("SCANNING"),
};

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
/*----------------------------------------------------------------------------*/
/*!
* \brief
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
void scnFsmSteps(IN struct ADAPTER *prAdapter, IN enum ENUM_SCAN_STATE eNextState)
{
	struct SCAN_INFO *prScanInfo;
	struct SCAN_PARAM *prScanParam;
	struct MSG_HDR *prMsgHdr;

	u_int8_t fgIsTransition = (u_int8_t) FALSE;

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prScanParam = &prScanInfo->rScanParam;

	do {
		DBGLOG(SCN, STATE, "[SCAN]TRANSITION: [%s] -> [%s]\n",
			apucDebugScanState[prScanInfo->eCurrentState], apucDebugScanState[eNextState]);

		/* NOTE(Kevin): This is the only place to change the eCurrentState(except initial) */
		prScanInfo->eCurrentState = eNextState;

		fgIsTransition = (u_int8_t) FALSE;

		switch (prScanInfo->eCurrentState) {
		case SCAN_STATE_IDLE:
			/* check for pending scanning requests */
			if (!LINK_IS_EMPTY(&(prScanInfo->rPendingMsgList))) {
				/* load next message from pending list as scan parameters */
				LINK_REMOVE_HEAD(&(prScanInfo->rPendingMsgList), prMsgHdr, struct MSG_HDR *);

				if (prMsgHdr->eMsgId == MID_AIS_SCN_SCAN_REQ
				    || prMsgHdr->eMsgId == MID_BOW_SCN_SCAN_REQ
				    || prMsgHdr->eMsgId == MID_P2P_SCN_SCAN_REQ
				    || prMsgHdr->eMsgId == MID_RLM_SCN_SCAN_REQ) {
					scnFsmHandleScanMsg(prAdapter, (struct MSG_SCN_SCAN_REQ *) prMsgHdr);

					eNextState = SCAN_STATE_SCANNING;
					fgIsTransition = TRUE;
				} else if (prMsgHdr->eMsgId == MID_AIS_SCN_SCAN_REQ_V2
					   || prMsgHdr->eMsgId == MID_BOW_SCN_SCAN_REQ_V2
					   || prMsgHdr->eMsgId == MID_P2P_SCN_SCAN_REQ_V2
					   || prMsgHdr->eMsgId == MID_RLM_SCN_SCAN_REQ_V2) {
					scnFsmHandleScanMsgV2(prAdapter, (struct MSG_SCN_SCAN_REQ_V2 *) prMsgHdr);

					eNextState = SCAN_STATE_SCANNING;
					fgIsTransition = TRUE;
				} else {
					/* should not happen */
					ASSERT(0);
				}

				/* switch to next state */
				cnmMemFree(prAdapter, prMsgHdr);
			}
			break;

		case SCAN_STATE_SCANNING:
			if (prScanParam->fgIsScanV2 == FALSE)
				scnSendScanReq(prAdapter);
			else
				scnSendScanReqV2(prAdapter);
			break;

		default:
			ASSERT(0);
			break;

		}
	} while (fgIsTransition);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief        Generate CMD_ID_SCAN_REQ command
 *
 * Because CMD_ID_SCAN_REQ is deprecated,
 * wrap this command to CMD_ID_SCAN_REQ_V2
 *
 * \param[in] prAdapter   adapter
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void scnSendScanReq(IN struct ADAPTER *prAdapter)
{
	DBGLOG(SCN, WARN,
		"CMD_ID_SCAN_REQ is deprecated, use CMD_ID_SCAN_REQ_V2\n");
	scnSendScanReqV2(prAdapter);
}

/*----------------------------------------------------------------------------*/
/*!
* \brief        Generate CMD_ID_SCAN_REQ_V2 command
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
void scnSendScanReqV2(IN struct ADAPTER *prAdapter)
{
	struct SCAN_INFO *prScanInfo;
	struct SCAN_PARAM *prScanParam;
	/* CMD_SCAN_REQ_V2 rCmdScanReq; */
	struct CMD_SCAN_REQ_V2 *prCmdScanReq;
	uint32_t i;

	ASSERT(prAdapter);

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prScanParam = &prScanInfo->rScanParam;

	prCmdScanReq = kalMemAlloc(sizeof(struct CMD_SCAN_REQ_V2), VIR_MEM_TYPE);
	if (!prCmdScanReq) {
		DBGLOG(SCN, ERROR, "alloc CmdScanReq V2 fail\n");
		return;
	}
	/* send command packet for scan */
	kalMemZero(prCmdScanReq, sizeof(struct CMD_SCAN_REQ_V2));

	prCmdScanReq->ucSeqNum = prScanParam->ucSeqNum;
	prCmdScanReq->ucBssIndex = prScanParam->ucBssIndex;
	prCmdScanReq->ucScanType = (uint8_t) prScanParam->eScanType;
	prCmdScanReq->ucSSIDType = prScanParam->ucSSIDType;

	for (i = 0; i < prScanParam->ucSSIDNum; i++) {
		COPY_SSID(prCmdScanReq->arSSID[i].aucSsid,
			  prCmdScanReq->arSSID[i].u4SsidLen,
			  prScanParam->aucSpecifiedSSID[i], prScanParam->ucSpecifiedSSIDLen[i]);
	}

	prCmdScanReq->u2ProbeDelayTime = (uint8_t) prScanParam->u2ProbeDelayTime;
	prCmdScanReq->ucChannelType = (uint8_t) prScanParam->eScanChannel;

	if (prScanParam->eScanChannel == SCAN_CHANNEL_SPECIFIED) {
		/* P2P would use:
		 * 1. Specified Listen Channel of passive scan for LISTEN state.
		 * 2. Specified Listen Channel of Target Device of active scan for SEARCH state. (Target != NULL)
		 */
		prCmdScanReq->ucChannelListNum = prScanParam->ucChannelListNum;

		for (i = 0; i < prCmdScanReq->ucChannelListNum; i++) {
			prCmdScanReq->arChannelList[i].ucBand = (uint8_t) prScanParam->arChnlInfoList[i].eBand;

			prCmdScanReq->arChannelList[i].ucChannelNum =
			    (uint8_t) prScanParam->arChnlInfoList[i].ucChannelNum;
		}
	}

	prCmdScanReq->u2ChannelDwellTime = prScanParam->u2ChannelDwellTime;
	prCmdScanReq->u2TimeoutValue = prScanParam->u2TimeoutValue;

	if (prScanParam->u2IELen <= MAX_IE_LENGTH)
		prCmdScanReq->u2IELen = prScanParam->u2IELen;
	else
		prCmdScanReq->u2IELen = MAX_IE_LENGTH;

	if (prScanParam->u2IELen)
		kalMemCopy(prCmdScanReq->aucIE, prScanParam->aucIE, sizeof(uint8_t) * prCmdScanReq->u2IELen);

	DBGLOG(SCN, INFO, "ScanReqV2: ScanType=%d, SSIDType=%d, Num=%d, ChannelType=%d, Num=%d",
		prCmdScanReq->ucScanType, prCmdScanReq->ucSSIDType, prScanParam->ucSSIDNum,
		prCmdScanReq->ucChannelType, prCmdScanReq->ucChannelListNum);

	wlanSendSetQueryCmd(prAdapter,
			    CMD_ID_SCAN_REQ_V2,
			    TRUE,
			    FALSE,
			    FALSE,
			    NULL,
			    NULL,
			    OFFSET_OF(struct CMD_SCAN_REQ_V2, aucIE) + prCmdScanReq->u2IELen,
			    (uint8_t *)prCmdScanReq, NULL, 0);

	kalMemFree(prCmdScanReq, VIR_MEM_TYPE, sizeof(struct CMD_SCAN_REQ_V2));

}

/*----------------------------------------------------------------------------*/
/*!
* \brief
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
void scnFsmMsgStart(IN struct ADAPTER *prAdapter, IN struct MSG_HDR *prMsgHdr)
{
	struct SCAN_INFO *prScanInfo;
	struct SCAN_PARAM *prScanParam;

	ASSERT(prMsgHdr);

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prScanParam = &prScanInfo->rScanParam;

	if (prScanInfo->eCurrentState == SCAN_STATE_IDLE) {
		if (prMsgHdr->eMsgId == MID_AIS_SCN_SCAN_REQ
		    || prMsgHdr->eMsgId == MID_BOW_SCN_SCAN_REQ
		    || prMsgHdr->eMsgId == MID_P2P_SCN_SCAN_REQ || prMsgHdr->eMsgId == MID_RLM_SCN_SCAN_REQ) {
			scnFsmHandleScanMsg(prAdapter, (struct MSG_SCN_SCAN_REQ *) prMsgHdr);
		} else if (prMsgHdr->eMsgId == MID_AIS_SCN_SCAN_REQ_V2
			   || prMsgHdr->eMsgId == MID_BOW_SCN_SCAN_REQ_V2
			   || prMsgHdr->eMsgId == MID_P2P_SCN_SCAN_REQ_V2
			   || prMsgHdr->eMsgId == MID_RLM_SCN_SCAN_REQ_V2) {
			scnFsmHandleScanMsgV2(prAdapter, (struct MSG_SCN_SCAN_REQ_V2 *) prMsgHdr);
		} else {
			/* should not deliver to this function */
			ASSERT(0);
		}

		cnmMemFree(prAdapter, prMsgHdr);
		scnFsmSteps(prAdapter, SCAN_STATE_SCANNING);
	} else {
		LINK_INSERT_TAIL(&prScanInfo->rPendingMsgList, &prMsgHdr->rLinkEntry);
	}
}

/*----------------------------------------------------------------------------*/
/*!
* \brief
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
void scnFsmMsgAbort(IN struct ADAPTER *prAdapter, IN struct MSG_HDR *prMsgHdr)
{
	struct MSG_SCN_SCAN_CANCEL *prScanCancel;
	struct SCAN_INFO *prScanInfo;
	struct SCAN_PARAM *prScanParam;
	struct CMD_SCAN_CANCEL rCmdScanCancel;

	ASSERT(prMsgHdr);

	prScanCancel = (struct MSG_SCN_SCAN_CANCEL *) prMsgHdr;
	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prScanParam = &prScanInfo->rScanParam;

	if (prScanInfo->eCurrentState != SCAN_STATE_IDLE) {
		if (prScanCancel->ucSeqNum == prScanParam->ucSeqNum &&
		    prScanCancel->ucBssIndex == prScanParam->ucBssIndex) {
			/* send cancel message to firmware domain */
			rCmdScanCancel.ucSeqNum = prScanParam->ucSeqNum;
			rCmdScanCancel.ucIsExtChannel = (uint8_t) prScanCancel->fgIsChannelExt;

			wlanSendSetQueryCmd(prAdapter,
					    CMD_ID_SCAN_CANCEL,
					    TRUE,
					    FALSE,
					    FALSE,
					    NULL, NULL, sizeof(struct CMD_SCAN_CANCEL), (uint8_t *) &rCmdScanCancel, NULL, 0);

			/* generate scan-done event for caller */
			scnFsmGenerateScanDoneMsg(prAdapter,
						  prScanParam->ucSeqNum,
						  prScanParam->ucBssIndex, SCAN_STATUS_CANCELLED);

			/* switch to next pending scan */
			scnFsmSteps(prAdapter, SCAN_STATE_IDLE);
		} else {
			scnFsmRemovePendingMsg(prAdapter, prScanCancel->ucSeqNum, prScanCancel->ucBssIndex);
		}
	}

	cnmMemFree(prAdapter, prMsgHdr);
}

/*----------------------------------------------------------------------------*/
/*!
* \brief            Scan Message Parsing (Legacy)
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
void scnFsmHandleScanMsg(IN struct ADAPTER *prAdapter, IN struct MSG_SCN_SCAN_REQ *prScanReqMsg)
{
	struct SCAN_INFO *prScanInfo;
	struct SCAN_PARAM *prScanParam;
	uint32_t i;

	ASSERT(prAdapter);
	ASSERT(prScanReqMsg);

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prScanParam = &prScanInfo->rScanParam;

	prScanParam->eScanType = prScanReqMsg->eScanType;
	prScanParam->ucBssIndex = prScanReqMsg->ucBssIndex;
	prScanParam->ucSSIDType = prScanReqMsg->ucSSIDType;
	if (prScanParam->ucSSIDType & (SCAN_REQ_SSID_SPECIFIED | SCAN_REQ_SSID_P2P_WILDCARD)) {
		prScanParam->ucSSIDNum = 1;

		COPY_SSID(prScanParam->aucSpecifiedSSID[0],
			  prScanParam->ucSpecifiedSSIDLen[0], prScanReqMsg->aucSSID, prScanReqMsg->ucSSIDLength);

		/* reset SSID length to zero for rest array entries */
		for (i = 1; i < SCN_SSID_MAX_NUM; i++)
			prScanParam->ucSpecifiedSSIDLen[i] = 0;
	} else {
		prScanParam->ucSSIDNum = 0;

		for (i = 0; i < SCN_SSID_MAX_NUM; i++)
			prScanParam->ucSpecifiedSSIDLen[i] = 0;
	}

	prScanParam->u2ProbeDelayTime = 0;
	prScanParam->eScanChannel = prScanReqMsg->eScanChannel;
	if (prScanParam->eScanChannel == SCAN_CHANNEL_SPECIFIED) {
		if (prScanReqMsg->ucChannelListNum <= MAXIMUM_OPERATION_CHANNEL_LIST)
			prScanParam->ucChannelListNum = prScanReqMsg->ucChannelListNum;
		else
			prScanParam->ucChannelListNum = MAXIMUM_OPERATION_CHANNEL_LIST;

		kalMemCopy(prScanParam->arChnlInfoList,
			   prScanReqMsg->arChnlInfoList, sizeof(struct RF_CHANNEL_INFO) * prScanParam->ucChannelListNum);
	}

	if (prScanReqMsg->u2IELen <= MAX_IE_LENGTH)
		prScanParam->u2IELen = prScanReqMsg->u2IELen;
	else
		prScanParam->u2IELen = MAX_IE_LENGTH;

	if (prScanParam->u2IELen)
		kalMemCopy(prScanParam->aucIE, prScanReqMsg->aucIE, prScanParam->u2IELen);

	prScanParam->u2ChannelDwellTime = prScanReqMsg->u2ChannelDwellTime;
	prScanParam->u2TimeoutValue = prScanReqMsg->u2TimeoutValue;
	prScanParam->ucSeqNum = prScanReqMsg->ucSeqNum;

	if (prScanReqMsg->rMsgHdr.eMsgId == MID_RLM_SCN_SCAN_REQ)
		prScanParam->fgIsObssScan = TRUE;
	else
		prScanParam->fgIsObssScan = FALSE;

	prScanParam->fgIsScanV2 = FALSE;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief            Scan Message Parsing - V2 with multiple SSID support
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
void scnFsmHandleScanMsgV2(IN struct ADAPTER *prAdapter, IN struct MSG_SCN_SCAN_REQ_V2 *prScanReqMsg)
{
	struct SCAN_INFO *prScanInfo;
	struct SCAN_PARAM *prScanParam;
	uint32_t i;

	ASSERT(prAdapter);
	ASSERT(prScanReqMsg);
	ASSERT(prScanReqMsg->ucSSIDNum <= SCN_SSID_MAX_NUM);

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prScanParam = &prScanInfo->rScanParam;

	prScanParam->eScanType = prScanReqMsg->eScanType;
	prScanParam->ucBssIndex = prScanReqMsg->ucBssIndex;
	prScanParam->ucSSIDType = prScanReqMsg->ucSSIDType;
	prScanParam->ucSSIDNum = prScanReqMsg->ucSSIDNum;

	for (i = 0; i < prScanReqMsg->ucSSIDNum; i++) {
		COPY_SSID(prScanParam->aucSpecifiedSSID[i],
			  prScanParam->ucSpecifiedSSIDLen[i],
			  prScanReqMsg->prSsid[i].aucSsid, (uint8_t) prScanReqMsg->prSsid[i].u4SsidLen);
	}

	prScanParam->u2ProbeDelayTime = prScanReqMsg->u2ProbeDelay;
	prScanParam->eScanChannel = prScanReqMsg->eScanChannel;
	if (prScanParam->eScanChannel == SCAN_CHANNEL_SPECIFIED) {
		if (prScanReqMsg->ucChannelListNum <= MAXIMUM_OPERATION_CHANNEL_LIST)
			prScanParam->ucChannelListNum = prScanReqMsg->ucChannelListNum;
		else
			prScanParam->ucChannelListNum = MAXIMUM_OPERATION_CHANNEL_LIST;

		kalMemCopy(prScanParam->arChnlInfoList,
			   prScanReqMsg->arChnlInfoList, sizeof(struct RF_CHANNEL_INFO) * prScanParam->ucChannelListNum);
	}

	if (prScanReqMsg->u2IELen <= MAX_IE_LENGTH)
		prScanParam->u2IELen = prScanReqMsg->u2IELen;
	else
		prScanParam->u2IELen = MAX_IE_LENGTH;

	if (prScanParam->u2IELen)
		kalMemCopy(prScanParam->aucIE, prScanReqMsg->aucIE, prScanParam->u2IELen);

	prScanParam->u2ChannelDwellTime = prScanReqMsg->u2ChannelDwellTime;
	prScanParam->u2TimeoutValue = prScanReqMsg->u2TimeoutValue;
	prScanParam->ucSeqNum = prScanReqMsg->ucSeqNum;

	if (prScanReqMsg->rMsgHdr.eMsgId == MID_RLM_SCN_SCAN_REQ_V2)
		prScanParam->fgIsObssScan = TRUE;
	else
		prScanParam->fgIsObssScan = FALSE;

	prScanParam->fgIsScanV2 = TRUE;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief            Remove pending scan request
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
void scnFsmRemovePendingMsg(IN struct ADAPTER *prAdapter, IN uint8_t ucSeqNum, IN uint8_t ucBssIndex)
{
	struct SCAN_INFO *prScanInfo;
	struct SCAN_PARAM *prScanParam;
	struct MSG_HDR *prPendingMsgHdr, *prPendingMsgHdrNext, *prRemoveMsgHdr = NULL;
	struct LINK_ENTRY *prRemoveLinkEntry = NULL;
	u_int8_t fgIsRemovingScan = FALSE;

	ASSERT(prAdapter);

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prScanParam = &prScanInfo->rScanParam;

	/* traverse through rPendingMsgList for removal */
	LINK_FOR_EACH_ENTRY_SAFE(prPendingMsgHdr,
				 prPendingMsgHdrNext, &(prScanInfo->rPendingMsgList), rLinkEntry, struct MSG_HDR) {
		if (prPendingMsgHdr->eMsgId == MID_AIS_SCN_SCAN_REQ
		    || prPendingMsgHdr->eMsgId == MID_BOW_SCN_SCAN_REQ
		    || prPendingMsgHdr->eMsgId == MID_P2P_SCN_SCAN_REQ
		    || prPendingMsgHdr->eMsgId == MID_RLM_SCN_SCAN_REQ) {
			struct MSG_SCN_SCAN_REQ *prScanReqMsg = (struct MSG_SCN_SCAN_REQ *) prPendingMsgHdr;

			if (ucSeqNum == prScanReqMsg->ucSeqNum && ucBssIndex == prScanReqMsg->ucBssIndex) {
				prRemoveLinkEntry = &(prScanReqMsg->rMsgHdr.rLinkEntry);
				prRemoveMsgHdr = prPendingMsgHdr;
				fgIsRemovingScan = TRUE;
			}
		} else if (prPendingMsgHdr->eMsgId == MID_AIS_SCN_SCAN_REQ_V2
			   || prPendingMsgHdr->eMsgId == MID_BOW_SCN_SCAN_REQ_V2
			   || prPendingMsgHdr->eMsgId == MID_P2P_SCN_SCAN_REQ_V2
			   || prPendingMsgHdr->eMsgId == MID_RLM_SCN_SCAN_REQ_V2) {
			struct MSG_SCN_SCAN_REQ_V2 *prScanReqMsgV2 = (struct MSG_SCN_SCAN_REQ_V2 *) prPendingMsgHdr;

			if (ucSeqNum == prScanReqMsgV2->ucSeqNum && ucBssIndex == prScanReqMsgV2->ucBssIndex) {
				prRemoveLinkEntry = &(prScanReqMsgV2->rMsgHdr.rLinkEntry);
				prRemoveMsgHdr = prPendingMsgHdr;
				fgIsRemovingScan = TRUE;
			}
		}

		if (prRemoveLinkEntry) {
			if (fgIsRemovingScan == TRUE) {
				/* generate scan-done event for caller */
				scnFsmGenerateScanDoneMsg(prAdapter, ucSeqNum, ucBssIndex, SCAN_STATUS_CANCELLED);
			}

			/* remove from pending list */
			LINK_REMOVE_KNOWN_ENTRY(&(prScanInfo->rPendingMsgList), prRemoveLinkEntry);
			cnmMemFree(prAdapter, prRemoveMsgHdr);

			break;
		}
	}
}

/*----------------------------------------------------------------------------*/
/*!
* \brief
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
void scnEventScanDone(IN struct ADAPTER *prAdapter, IN struct EVENT_SCAN_DONE *prScanDone, u_int8_t fgIsNewVersion)
{
	struct SCAN_INFO *prScanInfo;
	struct SCAN_PARAM *prScanParam;
	uint32_t	u4ChCnt;
	uint32_t	u4PrintfIdx = 0;

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prScanParam = &prScanInfo->rScanParam;

	kalMemZero(g_aucScanChannelNum, SCN_SCAN_DONE_PRINT_BUFFER_LENGTH);
	kalMemZero(g_aucScanChannelIdleTime, SCN_SCAN_DONE_PRINT_BUFFER_LENGTH);
	kalMemZero(g_aucScanChannelMDRDY, SCN_SCAN_DONE_PRINT_BUFFER_LENGTH);
	kalMemZero(g_aucScanChannelBeacon, SCN_SCAN_DONE_PRINT_BUFFER_LENGTH);

	if (fgIsNewVersion) {
		DBGLOG(SCN, INFO,
		       "scnEventScanDone Version%u!size of ScanDone%zu,ucCompleteChanCount[%u],ucCurrentState%u, u4ScanDurBcnCnt[%u]\n",
		       prScanDone->ucScanDoneVersion, sizeof(struct EVENT_SCAN_DONE), prScanDone->ucCompleteChanCount,
		       prScanDone->ucCurrentState, prScanDone->u4ScanDurBcnCnt);

		if (prScanDone->ucCurrentState != FW_SCAN_STATE_SCAN_DONE) {
			DBGLOG(SCN, INFO,
			       "FW Scan timeout!generate ScanDone event at State%d complete chan count%d ucChannelListNum%d\n",
			       prScanDone->ucCurrentState, prScanDone->ucCompleteChanCount,
			       prScanParam->ucChannelListNum);

		} else {
			DBGLOG(SCN, INFO, " scnEventScanDone at FW_SCAN_STATE_SCAN_DONE state\n");
		}
	} else {
		DBGLOG(SCN, INFO, "Old scnEventScanDone Version\n");
	}

	/* buffer empty channel information */
	if (prScanParam->eScanChannel == SCAN_CHANNEL_FULL || prScanParam->eScanChannel == SCAN_CHANNEL_2G4) {
		if (prScanDone->ucSparseChannelValid) {
			prScanInfo->fgIsSparseChannelValid = TRUE;
			prScanInfo->rSparseChannel.eBand = (enum ENUM_BAND) prScanDone->rSparseChannel.ucBand;
			prScanInfo->rSparseChannel.ucChannelNum = prScanDone->rSparseChannel.ucChannelNum;
			prScanInfo->ucSparseChannelArrayValidNum = prScanDone->ucSparseChannelArrayValidNum;
			DBGLOG(SCN, INFO, "Country Code = %c%c, Detected_Channel_Num = %d\n",
				((prAdapter->rWifiVar.rConnSettings.u2CountryCode & 0xff00) >> 8),
				(prAdapter->rWifiVar.rConnSettings.u2CountryCode & 0x00ff),
				prScanInfo->ucSparseChannelArrayValidNum);

			for (u4ChCnt = 0; u4ChCnt < prScanInfo->ucSparseChannelArrayValidNum; u4ChCnt++) {
				prScanInfo->aucChannelNum[u4ChCnt] = prScanDone->aucChannelNum[u4ChCnt];
				prScanInfo->au2ChannelIdleTime[u4ChCnt] = prScanDone->au2ChannelIdleTime[u4ChCnt];
				prScanInfo->aucChannelMDRDYCnt[u4ChCnt] = prScanDone->aucChannelMDRDYCnt[u4ChCnt];
				prScanInfo->aucChannelBAndPCnt[u4ChCnt] = prScanDone->aucChannelBAndPCnt[u4ChCnt];

				if (u4PrintfIdx % 10 == 0 && u4PrintfIdx != 0) {
					DBGFWLOG(SCN, INFO, "Channel  : %s\n", g_aucScanChannelNum);
					DBGFWLOG(SCN, INFO, "IdleTime : %s\n", g_aucScanChannelIdleTime);
					DBGFWLOG(SCN, INFO, "MdrdyCnt : %s\n", g_aucScanChannelMDRDY);
					DBGFWLOG(SCN, INFO, "BAndPCnt : %s\n", g_aucScanChannelBeacon);
					DBGFWLOG(SCN, INFO,
						"==================================================================================\n");
					kalMemZero(g_aucScanChannelNum, SCN_SCAN_DONE_PRINT_BUFFER_LENGTH);
					kalMemZero(g_aucScanChannelIdleTime, SCN_SCAN_DONE_PRINT_BUFFER_LENGTH);
					kalMemZero(g_aucScanChannelMDRDY, SCN_SCAN_DONE_PRINT_BUFFER_LENGTH);
					kalMemZero(g_aucScanChannelBeacon, SCN_SCAN_DONE_PRINT_BUFFER_LENGTH);
					u4PrintfIdx = 0;
				}
				kalSprintf(g_aucScanChannelNum + u4PrintfIdx*7, "%7d",
					prScanInfo->aucChannelNum[u4ChCnt]);
				kalSprintf(g_aucScanChannelIdleTime + u4PrintfIdx*7, "%7d",
					prScanInfo->au2ChannelIdleTime[u4ChCnt]);
				kalSprintf(g_aucScanChannelMDRDY + u4PrintfIdx*7, "%7d",
					prScanInfo->aucChannelMDRDYCnt[u4ChCnt]);
				kalSprintf(g_aucScanChannelBeacon + u4PrintfIdx*7, "%7d",
					prScanInfo->aucChannelBAndPCnt[u4ChCnt]);
				u4PrintfIdx++;
			}

			DBGFWLOG(SCN, INFO, "Channel  : %s\n", g_aucScanChannelNum);
			DBGFWLOG(SCN, INFO, "IdleTime : %s\n", g_aucScanChannelIdleTime);
			DBGFWLOG(SCN, INFO, "MdrdyCnt : %s\n", g_aucScanChannelMDRDY);
			DBGFWLOG(SCN, INFO, "BAndPCnt : %s\n", g_aucScanChannelBeacon);
		} else {
			prScanInfo->fgIsSparseChannelValid = FALSE;
		}
	}

	if (prScanInfo->eCurrentState == SCAN_STATE_SCANNING && prScanDone->ucSeqNum == prScanParam->ucSeqNum) {
		/* generate scan-done event for caller */
		scnFsmGenerateScanDoneMsg(prAdapter, prScanParam->ucSeqNum, prScanParam->ucBssIndex, SCAN_STATUS_DONE);

		/* switch to next pending scan */
		scnFsmSteps(prAdapter, SCAN_STATE_IDLE);
	} else {
		DBGLOG(SCN, INFO, "Unexpected SCAN-DONE event: SeqNum = %d, Current State = %d\n",
		       prScanDone->ucSeqNum, prScanInfo->eCurrentState);
	}
}				/* end of scnEventScanDone */

/*----------------------------------------------------------------------------*/
/*!
* \brief
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
void
scnFsmGenerateScanDoneMsg(IN struct ADAPTER *prAdapter,
			  IN uint8_t ucSeqNum, IN uint8_t ucBssIndex, IN enum ENUM_SCAN_STATUS eScanStatus)
{
	struct SCAN_INFO *prScanInfo;
	struct SCAN_PARAM *prScanParam;
	struct MSG_SCN_SCAN_DONE *prScanDoneMsg;

	ASSERT(prAdapter);

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prScanParam = &prScanInfo->rScanParam;

	prScanDoneMsg = (struct MSG_SCN_SCAN_DONE *) cnmMemAlloc(prAdapter, RAM_TYPE_MSG, sizeof(struct MSG_SCN_SCAN_DONE));
	if (!prScanDoneMsg) {
		ASSERT(0);	/* Can't indicate SCAN FSM Complete */
		return;
	}

	if (prScanParam->fgIsObssScan == TRUE) {
		prScanDoneMsg->rMsgHdr.eMsgId = MID_SCN_RLM_SCAN_DONE;
	} else {
		switch (GET_BSS_INFO_BY_INDEX(prAdapter, ucBssIndex)->eNetworkType) {
		case NETWORK_TYPE_AIS:
			prScanDoneMsg->rMsgHdr.eMsgId = MID_SCN_AIS_SCAN_DONE;
			break;

		case NETWORK_TYPE_P2P:
			prScanDoneMsg->rMsgHdr.eMsgId = MID_SCN_P2P_SCAN_DONE;
			break;

		case NETWORK_TYPE_BOW:
			prScanDoneMsg->rMsgHdr.eMsgId = MID_SCN_BOW_SCAN_DONE;
			break;

		default:
			DBGLOG(SCN, LOUD,
			       "Unexpected Network Type: %d\n",
			       GET_BSS_INFO_BY_INDEX(prAdapter, ucBssIndex)->eNetworkType);
			ASSERT(0);
			break;
		}
	}

	prScanDoneMsg->ucSeqNum = ucSeqNum;
	prScanDoneMsg->ucBssIndex = ucBssIndex;
	prScanDoneMsg->eScanStatus = eScanStatus;

	mboxSendMsg(prAdapter, MBOX_ID_0, (struct MSG_HDR *) prScanDoneMsg, MSG_SEND_METHOD_BUF);

}				/* end of scnFsmGenerateScanDoneMsg() */

/*----------------------------------------------------------------------------*/
/*!
* \brief        Query for most sparse channel
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
u_int8_t scnQuerySparseChannel(IN struct ADAPTER *prAdapter, enum ENUM_BAND *prSparseBand, uint8_t *pucSparseChannel)
{
	struct SCAN_INFO *prScanInfo;

	ASSERT(prAdapter);

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);

	if (prScanInfo->fgIsSparseChannelValid == TRUE) {
		if (prSparseBand)
			*prSparseBand = prScanInfo->rSparseChannel.eBand;

		if (pucSparseChannel)
			*pucSparseChannel = prScanInfo->rSparseChannel.ucChannelNum;

		return TRUE;
	} else {
		return FALSE;
	}
}

/*----------------------------------------------------------------------------*/
/*!
* \brief        Event handler for NLO done event
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
void scnEventNloDone(IN struct ADAPTER *prAdapter, IN struct EVENT_NLO_DONE *prNloDone)
{
	struct SCAN_INFO *prScanInfo;
	struct NLO_PARAM *prNloParam;
	struct SCAN_PARAM *prScanParam;

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prNloParam = &prScanInfo->rNloParam;
	prScanParam = &prNloParam->rScanParam;

	if (prScanInfo->fgNloScanning == TRUE && prNloDone->ucSeqNum == prScanParam->ucSeqNum) {

		DBGLOG(SCN, INFO, "scnEventNloDone reporting to uplayer\n");

		kalSchedScanResults(prAdapter->prGlueInfo);

		if (prNloParam->fgStopAfterIndication == TRUE)
			prScanInfo->fgNloScanning = FALSE;
	} else {
		DBGLOG(SCN, INFO, "Unexpected NLO-DONE event: SeqNum = %d, Current State = %d\n",
		       prNloDone->ucSeqNum, prScanInfo->eCurrentState);
	}
}

/*----------------------------------------------------------------------------*/
/*!
* \brief        OID handler for starting scheduled scan
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
u_int8_t
scnFsmSchedScanRequest(IN struct ADAPTER *prAdapter,
		       IN uint8_t ucSsidNum,
		       IN struct PARAM_SSID *prSsid, IN uint32_t u4IeLength, IN uint8_t *pucIe, IN uint16_t u2Interval)
{
	struct SCAN_INFO *prScanInfo;
	struct NLO_PARAM *prNloParam;
	struct SCAN_PARAM *prScanParam;
	struct CMD_NLO_REQ *prCmdNloReq;
	uint32_t i, j;

	ASSERT(prAdapter);

	DBGLOG(SCN, INFO, "scnFsmSchedScanRequest\n");

	if (prAdapter->prAisBssInfo == NULL)
		return FALSE;

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prNloParam = &prScanInfo->rNloParam;
	prScanParam = &prNloParam->rScanParam;

	/* ASSERT(prScanInfo->fgNloScanning == FALSE); */
	if (prScanInfo->fgNloScanning) {
		DBGLOG(SCN, INFO, "prScanInfo->fgNloScanning == FALSE  already scanning\n");
		return TRUE;
	}

	prScanInfo->fgNloScanning = TRUE;

	/* 1. load parameters */
	prScanParam->ucSeqNum++;
	prScanParam->ucBssIndex = prAdapter->prAisBssInfo->ucBssIndex;
	prNloParam->fgStopAfterIndication = FALSE;
	prNloParam->ucFastScanIteration = 0;

	if (u2Interval < SCAN_NLO_DEFAULT_INTERVAL) {
		u2Interval = SCAN_NLO_DEFAULT_INTERVAL;
		DBGLOG(SCN, INFO, "force interval to SCAN_NLO_DEFAULT_INTERVAL\n");
	}
	prAdapter->prAisBssInfo->fgIsPNOEnable = TRUE;

	if (!IS_NET_ACTIVE(prAdapter, prAdapter->prAisBssInfo->ucBssIndex)) {
		SET_NET_ACTIVE(prAdapter, prAdapter->prAisBssInfo->ucBssIndex);

		DBGLOG(SCN, INFO, "ACTIVE AIS from INACTIVE to enable PNO\n");
		/* sync with firmware */
		nicActivateNetwork(prAdapter, prAdapter->prAisBssInfo->ucBssIndex);
	}
	prNloParam->u2FastScanPeriod = u2Interval;
	prNloParam->u2SlowScanPeriod = u2Interval;

	if (ucSsidNum  > CFG_SCAN_SSID_MAX_NUM)
		prScanParam->ucSSIDNum = CFG_SCAN_SSID_MAX_NUM;
	else
		prScanParam->ucSSIDNum = ucSsidNum;

	if (ucSsidNum > CFG_SCAN_SSID_MATCH_MAX_NUM)
		prNloParam->ucMatchSSIDNum = CFG_SCAN_SSID_MATCH_MAX_NUM;
	else
		prNloParam->ucMatchSSIDNum = ucSsidNum;

	for (i = 0; i < prNloParam->ucMatchSSIDNum; i++) {
		if (i < CFG_SCAN_SSID_MAX_NUM) {
			COPY_SSID(prScanParam->aucSpecifiedSSID[i],
				  prScanParam->ucSpecifiedSSIDLen[i], prSsid[i].aucSsid, (uint8_t) prSsid[i].u4SsidLen);
		}

		COPY_SSID(prNloParam->aucMatchSSID[i],
			  prNloParam->ucMatchSSIDLen[i], prSsid[i].aucSsid, (uint8_t) prSsid[i].u4SsidLen);

		/*  for linux the Ciper,Auth Algo will be zero  */
		prNloParam->aucCipherAlgo[i] = 0;
		prNloParam->au2AuthAlgo[i] = 0;

		for (j = 0; j < SCN_NLO_NETWORK_CHANNEL_NUM; j++)
			prNloParam->aucChannelHint[i][j] = 0;
	}

	/* 2. prepare command for sending */
	prCmdNloReq = (struct CMD_NLO_REQ *) cnmMemAlloc(prAdapter, RAM_TYPE_BUF, sizeof(struct CMD_NLO_REQ) + u4IeLength);

	if (!prCmdNloReq) {
		ASSERT(0);	/* Can't initiate NLO operation */
		return FALSE;
	}

	/* 3. send command packet for NLO operation */
	kalMemZero(prCmdNloReq, sizeof(struct CMD_NLO_REQ));

	prCmdNloReq->ucSeqNum = prScanParam->ucSeqNum;
	prCmdNloReq->ucBssIndex = prScanParam->ucBssIndex;
	prCmdNloReq->fgStopAfterIndication = prNloParam->fgStopAfterIndication;
	prCmdNloReq->ucFastScanIteration = prNloParam->ucFastScanIteration;
	prCmdNloReq->u2FastScanPeriod = prNloParam->u2FastScanPeriod;
	prCmdNloReq->u2SlowScanPeriod = prNloParam->u2SlowScanPeriod;
	prCmdNloReq->ucEntryNum = prNloParam->ucMatchSSIDNum;

#ifdef LINUX
	prCmdNloReq->ucFlag = SCAN_NLO_CHECK_SSID_ONLY;
	DBGLOG(SCN, INFO, "LINUX only check SSID for PNO SCAN\n");
#endif
	for (i = 0; i < prNloParam->ucMatchSSIDNum; i++) {
		COPY_SSID(prCmdNloReq->arNetworkList[i].aucSSID,
			  prCmdNloReq->arNetworkList[i].ucSSIDLength,
			  prNloParam->aucMatchSSID[i], prNloParam->ucMatchSSIDLen[i]);

		prCmdNloReq->arNetworkList[i].ucCipherAlgo = prNloParam->aucCipherAlgo[i];
		prCmdNloReq->arNetworkList[i].u2AuthAlgo = prNloParam->au2AuthAlgo[i];
		DBGLOG(SCN, INFO, "prCmdNloReq->arNetworkList[i].aucSSID %s\n", prCmdNloReq->arNetworkList[i].aucSSID);
		DBGLOG(SCN, INFO,
		       "prCmdNloReq->arNetworkList[i].ucSSIDLength %d\n", prCmdNloReq->arNetworkList[i].ucSSIDLength);
		DBGLOG(SCN, INFO,
		       "prCmdNloReq->arNetworkList[i].ucCipherAlgo %d\n", prCmdNloReq->arNetworkList[i].ucCipherAlgo);
		DBGLOG(SCN, INFO,
		       "prCmdNloReq->arNetworkList[i].u2AuthAlgo %d\n", prCmdNloReq->arNetworkList[i].u2AuthAlgo);

		for (j = 0; j < SCN_NLO_NETWORK_CHANNEL_NUM; j++)
			prCmdNloReq->arNetworkList[i].ucNumChannelHint[j] = prNloParam->aucChannelHint[i][j];
	}

	if (u4IeLength <= MAX_IE_LENGTH)
		prCmdNloReq->u2IELen = prScanParam->u2IELen;
	else
		prCmdNloReq->u2IELen = MAX_IE_LENGTH;

	if (u4IeLength) {
		kalMemCopy(prScanParam->aucIE, pucIe, prCmdNloReq->u2IELen);
		kalMemCopy(prCmdNloReq->aucIE, pucIe, prCmdNloReq->u2IELen);
	}

	wlanSendSetQueryCmd(prAdapter,
			    CMD_ID_SET_NLO_REQ,
			    TRUE,
			    FALSE,
			    TRUE,
			    nicCmdEventSetCommon,
			    nicOidCmdTimeoutCommon,
			    sizeof(struct CMD_NLO_REQ) + prCmdNloReq->u2IELen, (uint8_t *) prCmdNloReq, NULL, 0);

	cnmMemFree(prAdapter, (void *) prCmdNloReq);

	return TRUE;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief        OID handler for stopping scheduled scan
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
u_int8_t scnFsmSchedScanStopRequest(IN struct ADAPTER *prAdapter)
{
	struct SCAN_INFO *prScanInfo;
	struct NLO_PARAM *prNloParam;
	struct SCAN_PARAM *prScanParam;
	struct CMD_NLO_CANCEL rCmdNloCancel;
	uint32_t rStatus;

	ASSERT(prAdapter);
	DBGLOG(SCN, INFO, "scnFsmSchedScanStopRequest\n");

	if (prAdapter->prAisBssInfo == NULL)
		return FALSE;

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prNloParam = &prScanInfo->rNloParam;
	prScanParam = &prNloParam->rScanParam;

	if (prAdapter->prAisBssInfo->fgIsNetRequestInActive && prAdapter->prAisBssInfo->fgIsPNOEnable) {
		UNSET_NET_ACTIVE(prAdapter, prAdapter->prAisBssInfo->ucBssIndex);

		DBGLOG(SCN, INFO, "INACTIVE  AIS from ACTIVE to DISABLE PNO\n");
		/* sync with firmware */
		nicDeactivateNetwork(prAdapter, prAdapter->prAisBssInfo->ucBssIndex);
	} else {
		DBGLOG(SCN, INFO,
		       "fgIsNetRequestInActive %d, fgIsPNOEnable %d\n",
		       prAdapter->prAisBssInfo->fgIsNetRequestInActive, prAdapter->prAisBssInfo->fgIsPNOEnable);
	}

	prAdapter->prAisBssInfo->fgIsPNOEnable = FALSE;

	/* send cancel message to firmware domain */
	rCmdNloCancel.ucSeqNum = prScanParam->ucSeqNum;

	rStatus = wlanSendSetQueryCmd(prAdapter, CMD_ID_SET_NLO_CANCEL, TRUE, FALSE, TRUE, nicCmdEventSetStopSchedScan,
					  /* nicCmdEventSetCommon, */
				      nicOidCmdTimeoutCommon,
				      sizeof(struct CMD_NLO_CANCEL), (uint8_t *) &rCmdNloCancel, NULL, 0);

	prScanInfo->fgNloScanning = FALSE;
	if (rStatus != WLAN_STATUS_FAILURE)
		return TRUE;
	else
		return FALSE;
}
