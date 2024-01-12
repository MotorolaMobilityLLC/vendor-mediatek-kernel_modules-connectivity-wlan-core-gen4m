/******************************************************************************
 *
 * This file is provided under a dual license.  When you use or
 * distribute this software, you may choose to be licensed under
 * version 2 of the GNU General Public License ("GPLv2 License")
 * or BSD License.
 *
 * GPLv2 License
 *
 * Copyright(C) 2017 MediaTek Inc.
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
 * Copyright(C) 2017 MediaTek Inc. All rights reserved.
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
/*! \file   "twt.c"
*   \brief  Functions for processing TWT related elements and frames.
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

static void twtFillTWTElement(
	struct _IE_TWT_T *prTWTBuf,
	uint8_t ucTWTFlowId,
	struct _TWT_PARAMS_T *prTWTParams);

static void twtParseTWTElement(
	struct _IE_TWT_T *prTWTIE,
	struct _TWT_PARAMS_T *prTWTParams);

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/
/*----------------------------------------------------------------------------*/
/*!
* \brief Send TWT Setup frame (S1G action frame)
*
* \param[in] prAdapter ADAPTER structure
*            prStaRec station record structure
*
* \return none
*/
/*----------------------------------------------------------------------------*/
uint32_t twtSendSetupFrame(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	uint8_t ucTWTFlowId,
	struct _TWT_PARAMS_T *prTWTParams,
	PFN_TX_DONE_HANDLER pfTxDoneHandler)
{
	struct MSDU_INFO *prMsduInfo;
	struct _ACTION_TWT_SETUP_FRAME *prTxFrame;
	struct BSS_INFO *prBssInfo;
	uint16_t u2EstimatedFrameLen;
	struct _IE_TWT_T *prTWTBuf;

	ASSERT(prAdapter);
	ASSERT(prStaRec);
	ASSERT(prTWTParams);

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);

	ASSERT(prBssInfo);

	/* Calculate MSDU buffer length */
	u2EstimatedFrameLen = MAC_TX_RESERVED_FIELD
		+ sizeof(struct _ACTION_TWT_SETUP_FRAME);

	/* Alloc MSDU_INFO */
	prMsduInfo = (struct MSDU_INFO *)
			cnmMgtPktAlloc(prAdapter, u2EstimatedFrameLen);

	if (!prMsduInfo) {
		DBGLOG(TWT_REQUESTER, WARN,
			"No MSDU_INFO_T for sending TWT Setup Frame.\n");
		return WLAN_STATUS_RESOURCES;
	}

	kalMemZero(prMsduInfo->prPacket, u2EstimatedFrameLen);

	prTxFrame = prMsduInfo->prPacket;

	/* Fill frame ctrl */
	prTxFrame->u2FrameCtrl = MAC_FRAME_ACTION;

	COPY_MAC_ADDR(prTxFrame->aucDestAddr, prStaRec->aucMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucSrcAddr, prBssInfo->aucOwnMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucBSSID, prBssInfo->aucBSSID);

	/* Compose the frame body's frame */
	prTxFrame->ucCategory = CATEGORY_S1G_ACTION;
	prTxFrame->ucAction = ACTION_S1G_TWT_SETUP;

	prTWTBuf = &(prTxFrame->rTWT);
	twtFillTWTElement(prTWTBuf, ucTWTFlowId, prTWTParams);

	/* Update information of MSDU_INFO_T */
	TX_SET_MMPDU(prAdapter,
			prMsduInfo,
			prBssInfo->ucBssIndex,
			prStaRec->ucIndex,
			WLAN_MAC_MGMT_HEADER_LEN,
			sizeof(struct _ACTION_TWT_SETUP_FRAME),
			pfTxDoneHandler, MSDU_RATE_MODE_AUTO);

	/* Enqueue the frame to send this action frame */
	nicTxEnqueueMsdu(prAdapter, prMsduInfo);

	return WLAN_STATUS_SUCCESS;
}

uint32_t twtSendTeardownFrame(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	uint8_t ucTWTFlowId,
	PFN_TX_DONE_HANDLER pfTxDoneHandler)
{
	struct MSDU_INFO *prMsduInfo;
	struct _ACTION_TWT_TEARDOWN_FRAME *prTxFrame;
	struct BSS_INFO *prBssInfo;
	uint16_t u2EstimatedFrameLen;

	ASSERT(prAdapter);
	ASSERT(prStaRec);

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);

	ASSERT(prBssInfo);

	/* Calculate MSDU buffer length */
	u2EstimatedFrameLen = MAC_TX_RESERVED_FIELD +
		sizeof(struct _ACTION_TWT_TEARDOWN_FRAME);

	/* Alloc MSDU_INFO */
	prMsduInfo = (struct MSDU_INFO *) cnmMgtPktAlloc(
		prAdapter, u2EstimatedFrameLen);

	if (!prMsduInfo) {
		DBGLOG(TWT_REQUESTER, WARN,
			"No MSDU_INFO_T for sending TWT Teardown Frame.\n");
		return WLAN_STATUS_RESOURCES;
	}

	kalMemZero(prMsduInfo->prPacket, u2EstimatedFrameLen);

	prTxFrame = prMsduInfo->prPacket;

	/* Fill frame ctrl */
	prTxFrame->u2FrameCtrl = MAC_FRAME_ACTION;

	COPY_MAC_ADDR(prTxFrame->aucDestAddr, prStaRec->aucMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucSrcAddr, prBssInfo->aucOwnMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucBSSID, prBssInfo->aucBSSID);

	/* Compose the frame body's frame */
	prTxFrame->ucCategory = CATEGORY_S1G_ACTION;
	prTxFrame->ucAction = ACTION_S1G_TWT_TEARDOWN;
	prTxFrame->ucTWTFlow = ucTWTFlowId;

	/* Update information of MSDU_INFO_T */
	TX_SET_MMPDU(prAdapter,
			prMsduInfo,
			prBssInfo->ucBssIndex,
			prStaRec->ucIndex,
			WLAN_MAC_MGMT_HEADER_LEN,
			sizeof(struct _ACTION_TWT_TEARDOWN_FRAME),
			pfTxDoneHandler, MSDU_RATE_MODE_AUTO);

	/* Enqueue the frame to send this action frame */
	nicTxEnqueueMsdu(prAdapter, prMsduInfo);

	return WLAN_STATUS_SUCCESS;
}

uint32_t twtSendInfoFrame(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	uint8_t ucTWTFlowId,
	struct _NEXT_TWT_INFO_T *prNextTWTInfo,
	PFN_TX_DONE_HANDLER pfTxDoneHandler)
{
	struct MSDU_INFO *prMsduInfo;
	struct _ACTION_TWT_INFO_FRAME *prTxFrame;
	uint32_t u4Pos =
		OFFSET_OF(struct _ACTION_TWT_INFO_FRAME, aucNextTWT[0]);
	struct BSS_INFO *prBssInfo;
	uint16_t u2EstimatedFrameLen;

	ASSERT(prAdapter);
	ASSERT(prStaRec);

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);

	ASSERT(prBssInfo);

	/* Calculate MSDU buffer length */
	u2EstimatedFrameLen = MAC_TX_RESERVED_FIELD +
		sizeof(struct _ACTION_TWT_INFO_FRAME) +
		twtGetNextTWTByteCnt(prNextTWTInfo->ucNextTWTSize);

	/* Alloc MSDU_INFO */
	prMsduInfo = (struct MSDU_INFO *) cnmMgtPktAlloc(
		prAdapter, u2EstimatedFrameLen);

	if (!prMsduInfo) {
		DBGLOG(TWT_REQUESTER, WARN,
			"No MSDU_INFO_T for sending TWT Info Frame.\n");
		return WLAN_STATUS_RESOURCES;
	}

	kalMemZero(prMsduInfo->prPacket, u2EstimatedFrameLen);

	prTxFrame = prMsduInfo->prPacket;

	/* Fill frame ctrl */
	prTxFrame->u2FrameCtrl = MAC_FRAME_ACTION;

	COPY_MAC_ADDR(prTxFrame->aucDestAddr, prStaRec->aucMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucSrcAddr, prBssInfo->aucOwnMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucBSSID, prBssInfo->aucBSSID);

	/* Compose the frame body's frame */
	prTxFrame->ucCategory = CATEGORY_S1G_ACTION;
	prTxFrame->ucAction = ACTION_S1G_TWT_INFORMATION;
	prTxFrame->ucNextTWTCtrl = (ucTWTFlowId & TWT_INFO_FLOW_ID) |
		((prNextTWTInfo->ucNextTWTSize  <<
		TWT_INFO_NEXT_TWT_SIZE_OFFSET) & TWT_INFO_NEXT_TWT_SIZE);

	switch (prNextTWTInfo->ucNextTWTSize) {
	case NEXT_TWT_SUBFIELD_64_BITS:
	{
		uint64_t *pu8NextTWT =
			(uint64_t *)(((uint8_t *)prTxFrame) + u4Pos);
		*pu8NextTWT = CPU_TO_LE64(prNextTWTInfo->u8NextTWT);
		break;
	}

	case NEXT_TWT_SUBFIELD_32_BITS:
	{
		uint32_t *pu4NextTWT =
			(uint32_t *)(((uint8_t *)prTxFrame) + u4Pos);
		*pu4NextTWT = CPU_TO_LE32(
			(uint32_t)(prNextTWTInfo->u8NextTWT & 0xFFFFFFFF));

		DBGLOG(TWT_REQUESTER, WARN,
			"TWT Info Frame 0x%x 0x%x\n",
			*pu4NextTWT, prNextTWTInfo->u8NextTWT);

		break;
	}

	case NEXT_TWT_SUBFIELD_48_BITS:
	{
		uint8_t *pucMem = ((uint8_t *)prTxFrame) + u4Pos;
		/* little endian placement */
		*pucMem = prNextTWTInfo->u8NextTWT & 0xFF;
		*(pucMem + 1) = (prNextTWTInfo->u8NextTWT >> 8) & 0xFF;
		*(pucMem + 2) = (prNextTWTInfo->u8NextTWT >> 16) & 0xFF;
		*(pucMem + 3) = (prNextTWTInfo->u8NextTWT >> 24) & 0xFF;
		*(pucMem + 4) = (prNextTWTInfo->u8NextTWT >> 32) & 0xFF;
		*(pucMem + 5) = (prNextTWTInfo->u8NextTWT >> 40) & 0xFF;
		break;
	}

	default:
		break;
	}

	DBGLOG(TWT_REQUESTER, WARN,
			"TWT Info Frame %d %d %d 0x%x 0x%x 0x%x\n",
			u2EstimatedFrameLen,
			sizeof(struct _ACTION_TWT_INFO_FRAME),
			twtGetNextTWTByteCnt(
				prNextTWTInfo->ucNextTWTSize),
			prTxFrame->ucNextTWTCtrl,
			twtGetNextTWTByteCnt(
				prNextTWTInfo->ucNextTWTSize) == 0 ?
			0 : *((uint32_t *)(((uint8_t *)prTxFrame) + u4Pos)),
			prNextTWTInfo->u8NextTWT);

	/* Update information of MSDU_INFO_T */
	TX_SET_MMPDU(prAdapter,
			prMsduInfo,
			prBssInfo->ucBssIndex,
			prStaRec->ucIndex,
			WLAN_MAC_MGMT_HEADER_LEN,
			(sizeof(struct _ACTION_TWT_INFO_FRAME) +
				twtGetNextTWTByteCnt(
					prNextTWTInfo->ucNextTWTSize)),
			pfTxDoneHandler, MSDU_RATE_MODE_AUTO);

	/* Enqueue the frame to send this action frame */
	nicTxEnqueueMsdu(prAdapter, prMsduInfo);

	return WLAN_STATUS_SUCCESS;
}

static void twtFillTWTElement(
	struct _IE_TWT_T *prTWTBuf,
	uint8_t ucTWTFlowId,
	struct _TWT_PARAMS_T *prTWTParams)
{
	ASSERT(prTWTBuf);
	ASSERT(prTWTParams);

	/* Add TWT element */
	prTWTBuf->ucId = ELEM_ID_TWT;
	prTWTBuf->ucLength = sizeof(struct _IE_TWT_T) - ELEM_HDR_LEN;

	/* Request Type */
	prTWTBuf->u2ReqType |= SET_TWT_RT_REQUEST(prTWTParams->fgReq) |
		SET_TWT_RT_SETUP_CMD(prTWTParams->ucSetupCmd) |
		SET_TWT_RT_TRIGGER(prTWTParams->fgTrigger) |
		TWT_REQ_TYPE_IMPLICIT_LAST_BCAST_PARAM |
		SET_TWT_RT_FLOW_TYPE(prTWTParams->fgUnannounced) |
		SET_TWT_RT_FLOW_ID(ucTWTFlowId) |
		SET_TWT_RT_WAKE_INTVAL_EXP(prTWTParams->ucWakeIntvalExponent) |
		SET_TWT_RT_PROTECTION(prTWTParams->fgProtect);

	prTWTBuf->u8TWT = CPU_TO_LE64(prTWTParams->u8TWT);
	prTWTBuf->ucMinWakeDur = prTWTParams->ucMinWakeDur;
	prTWTBuf->u2WakeIntvalMantiss =
		CPU_TO_LE16(prTWTParams->u2WakeIntvalMantiss);
}

#if (CFG_SUPPORT_BTWT == 1)
static void btwtParseTWTElement(
	struct _IE_BTWT_T *prBTWTIE,
	struct _TWT_PARAMS_T *prTWTParams)
{
	uint16_t u2ReqType;
	uint64_t u8TargetWakeTime;
	uint64_t u8twt_interval = 0;
	uint64_t u8Mod = 0;
	uint64_t u8Temp;

	u2ReqType = LE16_TO_CPU(prBTWTIE->u2ReqType);
	prTWTParams->fgReq = GET_TWT_RT_REQUEST(u2ReqType);
	prTWTParams->ucSetupCmd = GET_TWT_RT_SETUP_CMD(u2ReqType);
	prTWTParams->fgTrigger = GET_TWT_RT_TRIGGER(u2ReqType);
	prTWTParams->fgUnannounced = GET_TWT_RT_FLOW_TYPE(u2ReqType);
	prTWTParams->ucWakeIntvalExponent =
		GET_TWT_RT_WAKE_INTVAL_EXP(u2ReqType);

	prTWTParams->ucMinWakeDur = prBTWTIE->ucMinWakeDur;
	prTWTParams->u2WakeIntvalMantiss =
		LE16_TO_CPU(prBTWTIE->u2WakeIntvalMantiss);

	DBGLOG(TWT_REQUESTER, WARN,
		"BTWT cur TSF %x %x\nAP resp TWT %x\n",
		(prTWTParams->u8TWT & 0x00000000FFFFFFFF),
		((prTWTParams->u8TWT & 0xFFFFFFFF00000000) >> 32),
		prBTWTIE->u2TWT);

	u8TargetWakeTime = prTWTParams->u8TWT;
	u8twt_interval = (((u_int64_t)prTWTParams->u2WakeIntvalMantiss)
		<< prTWTParams->ucWakeIntvalExponent);
	u8Temp = u8TargetWakeTime + u8twt_interval;
	u8Mod = kal_mod64(u8Temp, u8twt_interval);
	prTWTParams->u8TWT = u8TargetWakeTime + (u8twt_interval - u8Mod);

	DBGLOG(TWT_REQUESTER, WARN,
		"BTWT twt %x %x\n",
		(prTWTParams->u8TWT & 0x00000000FFFFFFFF),
		((prTWTParams->u8TWT & 0xFFFFFFFF00000000) >> 32));
}

uint8_t btwtGetTxSetupFlowId(
	struct MSDU_INFO *prMsduInfo)
{
	uint8_t ucFlowId;
	struct _ACTION_BTWT_SETUP_FRAME *prTxFrame;

	ASSERT(prMsduInfo);

	prTxFrame = (struct _ACTION_BTWT_SETUP_FRAME *)(prMsduInfo->prPacket);
	ucFlowId = GET_BTWT_ID(prTxFrame->rTWT.u2BTWTInfo);

	return ucFlowId;
}

static void btwtFlagOnOff(
	struct _TWT_FLOW_T *prBtwtFlow,
	uint8_t ucBtwtOnOff)
{
	ASSERT(prBtwtFlow);

	prBtwtFlow->fgIsBTWT = ucBtwtOnOff;
}
#endif

static void twtParseTWTElement(
	struct _IE_TWT_T *prTWTIE,
	struct _TWT_PARAMS_T *prTWTParams)
{
	uint16_t u2ReqType;

	u2ReqType = LE16_TO_CPU(prTWTIE->u2ReqType);

	prTWTParams->fgReq = GET_TWT_RT_REQUEST(u2ReqType);
	prTWTParams->ucSetupCmd = GET_TWT_RT_SETUP_CMD(u2ReqType);
	prTWTParams->fgTrigger = GET_TWT_RT_TRIGGER(u2ReqType);
	prTWTParams->fgUnannounced = GET_TWT_RT_FLOW_TYPE(u2ReqType);

	prTWTParams->ucWakeIntvalExponent =
		GET_TWT_RT_WAKE_INTVAL_EXP(u2ReqType);

	prTWTParams->fgProtect = GET_TWT_RT_PROTECTION(u2ReqType);
	prTWTParams->u8TWT = LE64_TO_CPU(prTWTIE->u8TWT);
	prTWTParams->ucMinWakeDur = prTWTIE->ucMinWakeDur;

	prTWTParams->u2WakeIntvalMantiss =
		LE16_TO_CPU(prTWTIE->u2WakeIntvalMantiss);
}

uint8_t twtGetTxSetupFlowId(
	struct MSDU_INFO *prMsduInfo)
{
	uint8_t ucFlowId;
	struct _ACTION_TWT_SETUP_FRAME *prTxFrame;

	ASSERT(prMsduInfo);

	prTxFrame = (struct _ACTION_TWT_SETUP_FRAME *)(prMsduInfo->prPacket);
	ucFlowId = GET_TWT_RT_FLOW_ID(prTxFrame->rTWT.u2ReqType);

	return ucFlowId;
}

uint8_t twtGetTxTeardownFlowId(
	struct MSDU_INFO *prMsduInfo)
{
	uint8_t ucFlowId;
	struct _ACTION_TWT_TEARDOWN_FRAME *prTxFrame;

	ASSERT(prMsduInfo);

	prTxFrame = (struct _ACTION_TWT_TEARDOWN_FRAME *)(prMsduInfo->prPacket);
	ucFlowId = (prTxFrame->ucTWTFlow & TWT_TEARDOWN_FLOW_ID);

	return ucFlowId;
}

uint8_t twtGetTxInfoFlowId(
	struct MSDU_INFO *prMsduInfo)
{
	uint8_t ucFlowId;
	struct _ACTION_TWT_INFO_FRAME *prTxFrame;

	ASSERT(prMsduInfo);

	prTxFrame = (struct _ACTION_TWT_INFO_FRAME *)(prMsduInfo->prPacket);
	ucFlowId = GET_TWT_INFO_FLOW_ID(prTxFrame->ucNextTWTCtrl);

	return ucFlowId;
}

uint8_t twtGetRxSetupFlowId(
	struct _IE_TWT_T *prTWTIE)
{
	uint16_t u2ReqType;

	ASSERT(prTWTIE);

	u2ReqType = LE16_TO_CPU(prTWTIE->u2ReqType);

	return GET_TWT_RT_FLOW_ID(u2ReqType);
}

void twtProcessS1GAction(
	struct ADAPTER *prAdapter,
	struct SW_RFB *prSwRfb)
{
	struct WLAN_ACTION_FRAME *prRxFrame;
	struct _ACTION_TWT_SETUP_FRAME *prRxSetupFrame = NULL;
	struct _ACTION_TWT_TEARDOWN_FRAME *prRxTeardownFrame = NULL;
	struct _ACTION_TWT_INFO_FRAME *prRxInfoFrame = NULL;
	struct STA_RECORD *prStaRec;
	struct RX_DESC_OPS_T *prRxDescOps;

	uint8_t ucTWTFlowId;
	uint32_t u4Offset;
	uint16_t u2ReqType;

	ASSERT(prAdapter);
	ASSERT(prSwRfb);

	prRxDescOps = prAdapter->chip_info->prRxDescOps;
	ASSERT(prRxDescOps->nic_rxd_get_rx_byte_count);
	ASSERT(prRxDescOps->nic_rxd_get_pkt_type);
	ASSERT(prRxDescOps->nic_rxd_get_wlan_idx);

	prRxFrame = (struct WLAN_ACTION_FRAME *) prSwRfb->pvHeader;
	prStaRec = cnmGetStaRecByIndex(prAdapter, prSwRfb->ucStaRecIdx);
	if (!prStaRec) {
		DBGLOG(TWT_REQUESTER, WARN,
		"Received an S1G Action: wlanIdx[%d] w/o corresponding staRec\n"
		, prRxDescOps->nic_rxd_get_wlan_idx(prSwRfb->prRxStatus));
		return;
	}

	switch (prRxFrame->ucAction) {
	case ACTION_S1G_TWT_SETUP:
		prRxSetupFrame =
			(struct _ACTION_TWT_SETUP_FRAME *) prSwRfb->pvHeader;
#if (CFG_SUPPORT_BTWT == 1)
		if (prStaRec->ucStaState != STA_STATE_3 ||
			prSwRfb->u2PacketLen <
				sizeof(struct _ACTION_BTWT_SETUP_FRAME)) {
			DBGLOG(TWT_REQUESTER, WARN,
				"improper TWT Setupframe %d\n",
				prStaRec->ucStaState);
			return;
		}
#else
		if (prStaRec->ucStaState != STA_STATE_3 ||
			prSwRfb->u2PacketLen <
				sizeof(struct _ACTION_TWT_SETUP_FRAME)) {
			DBGLOG(TWT_REQUESTER, WARN,
				"Received improper TWT Setup frame\n");
			return;
		}
#endif

#if (CFG_SUPPORT_BTWT == 1)
		if (GET_BTWT_CTRL_NEGO(
			((struct _IE_BTWT_T *)&(prRxSetupFrame->rTWT))->ucCtrl)
			== 3) {
			/* BTWT setup resp */
			ucTWTFlowId = GET_BTWT_ID(
			((struct _IE_BTWT_T *)&(prRxSetupFrame->rTWT))
				->u2BTWTInfo);
			DBGLOG(RLM, WARN, "BTWT params here\n");

	DBGLOG(RLM, WARN,
		"Rx BTWT params:\nReqType=%x\nTWT=%x\nMinWakeDur=%x\nWakeIntMantissa=%x\nBTWTInfo=%x\n",
		((struct _IE_BTWT_T *)&(prRxSetupFrame->rTWT))->u2ReqType,
		((struct _IE_BTWT_T *)&(prRxSetupFrame->rTWT))->u2TWT,
		((struct _IE_BTWT_T *)&(prRxSetupFrame->rTWT))->ucMinWakeDur,
		((struct _IE_BTWT_T *)&
			(prRxSetupFrame->rTWT))->u2WakeIntvalMantiss,
		((struct _IE_BTWT_T *)&(prRxSetupFrame->rTWT))->u2BTWTInfo);

			btwtParseTWTElement(
				((struct _IE_BTWT_T *)&(prRxSetupFrame->rTWT)),
			&(prStaRec->arTWTFlow[ucTWTFlowId].rTWTPeerParams));

			btwtFlagOnOff(&(prStaRec->arTWTFlow[ucTWTFlowId]),
				TRUE);
		} else {
#endif

#if (CFG_SUPPORT_802_11BE_ML_TWT == 1)
			if (GET_ML_TWT_CTRL_LINK_ID_BITMAP(
				((struct IE_ML_TWT_T *)&(prRxSetupFrame->rTWT))->ucCtrl)
				== 1) {
				/* We receive multi-link TWT response frame */
				/* Parse TWT element in the unit of link ID bitmap */
				/* Iterate all link IDs related STA_REC in MLD_GRP */
				if (mltwtParseTWTElement(
					prAdapter,
					prStaRec,
					(uint8_t *)&(prRxSetupFrame->rTWT),
					prSwRfb->u2PacketLen -
					sizeof(
					struct _ACTION_ML_TWT_SETUP_FRAME_PER_LINK_DISTINCT)
					) != WLAN_STATUS_SUCCESS) {
					DBGLOG(RLM, ERROR, "MLTWT setup resp\n");

					return;
				}

				/*
				* After manipulation of MLTWT setup response,
				* follow i-TWT/BTWT existening flow...
				*/
				ucTWTFlowId =
					twtGetRxSetupFlowId(&(prRxSetupFrame->rTWT));
			} else {
#endif

			/* Parse TWT element */
			ucTWTFlowId =
				twtGetRxSetupFlowId(&(prRxSetupFrame->rTWT));
			twtParseTWTElement(&(prRxSetupFrame->rTWT),
				&(prStaRec->arTWTFlow[ucTWTFlowId]
				.rTWTPeerParams));

#if (CFG_SUPPORT_802_11BE_ML_TWT == 1)
			}
#endif

#if (CFG_SUPPORT_BTWT == 1)
			btwtFlagOnOff(&(prStaRec->arTWTFlow[ucTWTFlowId]),
				FALSE);
		}
#endif

		/* Notify TWT Requester FSM upon reception of a TWT response */
		u2ReqType = prRxSetupFrame->rTWT.u2ReqType;
		if (!(u2ReqType & TWT_REQ_TYPE_TWT_REQUEST)) {
			twtReqFsmRunEventRxSetup(prAdapter,
				prSwRfb, prStaRec, ucTWTFlowId);
		}

		break;

	case ACTION_S1G_TWT_TEARDOWN:
		prRxTeardownFrame = (struct _ACTION_TWT_TEARDOWN_FRAME *)
			prSwRfb->pvHeader;
		if (prStaRec->ucStaState != STA_STATE_3 ||
			prSwRfb->u2PacketLen <
				sizeof(struct _ACTION_TWT_TEARDOWN_FRAME)) {
			DBGLOG(TWT_REQUESTER, WARN,
				"Received improper TWT Teardown frame\n");
			return;
		}

		ucTWTFlowId = prRxTeardownFrame->ucTWTFlow;

		/* Notify TWT Requester FSM */
		twtReqFsmRunEventRxTeardown(
			prAdapter, prSwRfb, prStaRec, ucTWTFlowId);

		break;

	case ACTION_S1G_TWT_INFORMATION:
	{
		uint8_t ucNextTWTSize = 0;
		uint8_t *pucMem;
		struct _NEXT_TWT_INFO_T rNextTWTInfo;

		prRxInfoFrame = (struct _ACTION_TWT_INFO_FRAME *)
			prSwRfb->pvHeader;
		if (prStaRec->ucStaState != STA_STATE_3 ||
			prSwRfb->u2PacketLen <
				sizeof(struct _ACTION_TWT_INFO_FRAME)) {
			DBGLOG(TWT_REQUESTER, WARN,
				"Received improper TWT Info frame\n");
			return;
		}

		ucTWTFlowId = GET_TWT_INFO_FLOW_ID(
			prRxInfoFrame->ucNextTWTCtrl);
		ucNextTWTSize = GET_TWT_INFO_NEXT_TWT_SIZE(
			prRxInfoFrame->ucNextTWTCtrl);

		u4Offset = OFFSET_OF(struct _ACTION_TWT_INFO_FRAME,
			aucNextTWT[0]);
		pucMem = ((uint8_t *)prRxInfoFrame) + u4Offset;

		if (ucNextTWTSize == NEXT_TWT_SUBFIELD_64_BITS &&
			prSwRfb->u2PacketLen >=
			(sizeof(struct _ACTION_TWT_INFO_FRAME) + 8)) {
			rNextTWTInfo.u8NextTWT =
				LE64_TO_CPU(*((uint64_t *)pucMem));
		} else if (ucNextTWTSize == NEXT_TWT_SUBFIELD_32_BITS &&
			prSwRfb->u2PacketLen >=
			(sizeof(struct _ACTION_TWT_INFO_FRAME) + 4)) {
			rNextTWTInfo.u8NextTWT =
				LE32_TO_CPU(*((uint32_t *)pucMem));
		} else if (ucNextTWTSize == NEXT_TWT_SUBFIELD_48_BITS &&
			prSwRfb->u2PacketLen >=
			(sizeof(struct _ACTION_TWT_INFO_FRAME) + 6)) {
			rNextTWTInfo.u8NextTWT =
				GET_48_BITS_NEXT_TWT_FROM_PKT(pucMem);
		} else {
			DBGLOG(TWT_REQUESTER, WARN,
				"TWT Info frame with imcorrect size\n");
			return;
		}

		rNextTWTInfo.ucNextTWTSize = ucNextTWTSize;

		/* Notify TWT Requester FSM */
		twtReqFsmRunEventRxInfoFrm(
			prAdapter, prSwRfb, prStaRec, ucTWTFlowId,
			&rNextTWTInfo);

		break;
	}
	default:
		break;
	}
}

#if (CFG_SUPPORT_BTWT == 1)
void btwtFillTWTElement(
	struct _IE_BTWT_T *prTWTBuf,
	uint8_t ucTWTFlowId,
	struct _TWT_PARAMS_T *prTWTParams)
{
	ASSERT(prTWTBuf);
	ASSERT(prTWTParams);

	/* Add TWT element */
	prTWTBuf->ucId = ELEM_ID_TWT;
	prTWTBuf->ucLength = sizeof(struct _IE_BTWT_T) - ELEM_HDR_LEN;

	/* Request Type */
	prTWTBuf->u2ReqType |= SET_TWT_RT_REQUEST(prTWTParams->fgReq) |
		SET_TWT_RT_SETUP_CMD(prTWTParams->ucSetupCmd) |
		SET_TWT_RT_TRIGGER(prTWTParams->fgTrigger) |
		BTWT_REQ_TYPE_LAST_BCAST_PARAM |
		SET_TWT_RT_FLOW_TYPE(prTWTParams->fgUnannounced) |
		SET_BTWT_RECOMMENDATION(0) |
		SET_TWT_RT_WAKE_INTVAL_EXP(prTWTParams->ucWakeIntvalExponent) |
		SET_BTWT_RESERVED(0);

	prTWTBuf->ucCtrl |= SET_BTWT_CTRL_NEGO(3);
	prTWTBuf->u2BTWTInfo |= SET_BTWT_ID(ucTWTFlowId);

	prTWTBuf->u2TWT = CPU_TO_LE16((uint16_t)prTWTParams->u8TWT);
	prTWTBuf->ucMinWakeDur = prTWTParams->ucMinWakeDur;
	prTWTBuf->u2WakeIntvalMantiss =
		CPU_TO_LE16(prTWTParams->u2WakeIntvalMantiss);
	DBGLOG(TWT_REQUESTER, WARN,
		"btwtSendSetupFrame frame %x, %x, %x, %x, %x, %x, %x, %x\n",
		prTWTBuf->ucId,
		prTWTBuf->ucLength,
		prTWTBuf->ucCtrl,
		prTWTBuf->u2ReqType,
		prTWTBuf->u2TWT,
		prTWTBuf->ucMinWakeDur,
		prTWTBuf->u2WakeIntvalMantiss,
		prTWTBuf->u2BTWTInfo);

}

uint32_t btwtSendSetupFrame(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	u_int8_t ucTWTFlowId,
	struct _TWT_PARAMS_T *prTWTParams,
	PFN_TX_DONE_HANDLER pfTxDoneHandler)
{
	struct MSDU_INFO *prMsduInfo;
	struct _ACTION_BTWT_SETUP_FRAME *prTxFrame;
	struct BSS_INFO *prBssInfo;
	uint16_t u2EstimatedFrameLen;
	struct _IE_BTWT_T *prBTWTBuf;

	ASSERT(prAdapter);
	ASSERT(prStaRec);
	ASSERT(prTWTParams);

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);

	ASSERT(prBssInfo);
	DBGLOG(TWT_REQUESTER, WARN,
		"btwtSendSetupFrame begin\n");

	/* Calculate MSDU buffer length */
	u2EstimatedFrameLen = MAC_TX_RESERVED_FIELD
		+ sizeof(struct _ACTION_BTWT_SETUP_FRAME);

	/* Alloc MSDU_INFO */
	prMsduInfo = (struct MSDU_INFO *)
			cnmMgtPktAlloc(prAdapter, u2EstimatedFrameLen);

	if (!prMsduInfo) {
		DBGLOG(TWT_REQUESTER, WARN,
			"No MSDU_INFO_T for sending TWT Setup Frame.\n");
		return WLAN_STATUS_RESOURCES;
	}

	kalMemZero(prMsduInfo->prPacket, u2EstimatedFrameLen);

	prTxFrame = prMsduInfo->prPacket;

	/* Fill frame ctrl */
	prTxFrame->u2FrameCtrl = MAC_FRAME_ACTION;

	COPY_MAC_ADDR(prTxFrame->aucDestAddr, prStaRec->aucMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucSrcAddr, prBssInfo->aucOwnMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucBSSID, prBssInfo->aucBSSID);

	/* Compose the frame body's frame */
	prTxFrame->ucCategory = CATEGORY_S1G_ACTION;
	prTxFrame->ucAction = ACTION_S1G_TWT_SETUP;

	prBTWTBuf = &(prTxFrame->rTWT);
	btwtFillTWTElement(prBTWTBuf, ucTWTFlowId, prTWTParams);
	DBGLOG(TWT_REQUESTER, WARN,
		"btwtSendSetupFrame flowid %d\n", ucTWTFlowId);

	/* Update information of MSDU_INFO_T */
	TX_SET_MMPDU(prAdapter,
			prMsduInfo,
			prBssInfo->ucBssIndex,
			prStaRec->ucIndex,
			WLAN_MAC_MGMT_HEADER_LEN,
			sizeof(struct _ACTION_BTWT_SETUP_FRAME),
			pfTxDoneHandler, MSDU_RATE_MODE_AUTO);

	/* Enqueue the frame to send this action frame */
	nicTxEnqueueMsdu(prAdapter, prMsduInfo);
	DBGLOG(TWT_REQUESTER, WARN,
		"btwtSendSetupFrame end\n");

	return WLAN_STATUS_SUCCESS;
}

uint32_t btwtSendTeardownFrame(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	u_int8_t ucTWTFlowId,
	PFN_TX_DONE_HANDLER pfTxDoneHandler)
{
	struct MSDU_INFO *prMsduInfo;
	struct _ACTION_TWT_TEARDOWN_FRAME *prTxFrame;
	struct BSS_INFO *prBssInfo;
	uint16_t u2EstimatedFrameLen;

	ASSERT(prAdapter);
	ASSERT(prStaRec);

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);

	ASSERT(prBssInfo);

	/* Calculate MSDU buffer length */
	u2EstimatedFrameLen = MAC_TX_RESERVED_FIELD +
		sizeof(struct _ACTION_TWT_TEARDOWN_FRAME);

	/* Alloc MSDU_INFO */
	prMsduInfo = (struct MSDU_INFO *) cnmMgtPktAlloc(
		prAdapter, u2EstimatedFrameLen);

	if (!prMsduInfo) {
		DBGLOG(TWT_REQUESTER, WARN,
			"No MSDU_INFO_T for sending TWT Teardown Frame.\n");
		return WLAN_STATUS_RESOURCES;
	}

	kalMemZero(prMsduInfo->prPacket, u2EstimatedFrameLen);

	prTxFrame = prMsduInfo->prPacket;

	/* Fill frame ctrl */
	prTxFrame->u2FrameCtrl = MAC_FRAME_ACTION;

	COPY_MAC_ADDR(prTxFrame->aucDestAddr, prStaRec->aucMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucSrcAddr, prBssInfo->aucOwnMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucBSSID, prBssInfo->aucBSSID);

	/* Compose the frame body's frame */
	prTxFrame->ucCategory = CATEGORY_S1G_ACTION;
	prTxFrame->ucAction = ACTION_S1G_TWT_TEARDOWN;
	prTxFrame->ucTWTFlow = ucTWTFlowId;
	prTxFrame->ucTWTFlow |= SET_TWT_TEARDOWN_NEGO(3);

	/* Update information of MSDU_INFO_T */
	TX_SET_MMPDU(prAdapter,
			prMsduInfo,
			prBssInfo->ucBssIndex,
			prStaRec->ucIndex,
			WLAN_MAC_MGMT_HEADER_LEN,
			sizeof(struct _ACTION_TWT_TEARDOWN_FRAME),
			pfTxDoneHandler, MSDU_RATE_MODE_AUTO);

	/* Enqueue the frame to send this action frame */
	nicTxEnqueueMsdu(prAdapter, prMsduInfo);

	return WLAN_STATUS_SUCCESS;

}
#endif

#if (CFG_SUPPORT_802_11BE_ML_TWT == 1)
uint32_t mltwtParseTWTElement(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	uint8_t *pucIE,
	uint16_t u2IELength)
{
	struct MLD_BSS_INFO *prMldBssInfo = NULL;
	struct BSS_INFO *prBssInfo = NULL;
	struct STA_RECORD *prStaRecOfAP = NULL;
	struct IE_ML_TWT_T *prMLTWTIE = NULL;
	uint16_t u2Offset;
	uint8_t ucLinkID;
	uint8_t ucTWTFlowId;
	uint16_t u2LinkIdBitMap;

	/* Get MLD_BSS_INFO in MLO connection */
	ASSERT(prAdapter);

	ASSERT(prStaRec);

	prBssInfo = GET_BSS_INFO_BY_INDEX(
					prAdapter, prStaRec->ucBssIndex);

	ASSERT(prBssInfo);

	prMldBssInfo = mldBssGetByBss(prAdapter, prBssInfo);

	if (!prMldBssInfo) {
		DBGLOG(TWT_REQUESTER, ERROR,
			"ML Invalid MLD_BSS_INFO\n");

		return WLAN_STATUS_FAILURE;
	}

	IE_FOR_EACH(pucIE, u2IELength, u2Offset)
	{
		DBGLOG(TWT_REQUESTER, WARN,
			"u2IELength %d u2Offset %d\n",
			u2IELength, u2Offset);

	switch (IE_ID(pucIE))
	{
	case ELEM_ID_TWT:
		prMLTWTIE = (struct IE_ML_TWT_T *)pucIE;

		ucTWTFlowId = twtGetRxSetupFlowId(
					(struct _IE_TWT_T *)prMLTWTIE);

		u2LinkIdBitMap = prMLTWTIE->u2LinkIdBitmap;

		/* Iterate each link ID */
		for (ucLinkID = 0;
			ucLinkID < ML_TWT_LINK_ID_BITMAP_COUNT;
			ucLinkID++) {
			if ((u2LinkIdBitMap & BIT(ucLinkID))
				== BIT(ucLinkID)) {
				/* Get the BSS INFO of ucLinkID */
				prBssInfo = mldGetBssInfoByLinkID(
								prAdapter,
								prMldBssInfo,
								ucLinkID,
								TRUE);

				if (!prBssInfo) {
					DBGLOG(TWT_REQUESTER, ERROR,
						"MLTWT %d no BSS_INFO of link ID %d\n",
						ucTWTFlowId,
						ucLinkID);

					return WLAN_STATUS_FAILURE;
				}

				prStaRecOfAP = prBssInfo->prStaRecOfAP;

				if (!prStaRecOfAP) {
					DBGLOG(TWT_REQUESTER, ERROR,
						"TWT Flow %d no STA_REC of link ID %d\n",
						ucTWTFlowId,
						ucLinkID);

					return WLAN_STATUS_FAILURE;
				}

				twtParseTWTElement(
					(struct _IE_TWT_T *)pucIE,
					&(prStaRecOfAP->arTWTFlow[ucTWTFlowId]
					.rTWTPeerParams));

				prStaRecOfAP->arTWTFlow[ucTWTFlowId].fgIsMLTWT = TRUE;
			}
		}

		break;

	default:
		break;
	}
	}

#if 0
    if (kal_mod64(u2IELength, IE_ML_TWT_LENGTH) == 0) {
		/*
		* The frame contains multiple complete TWT elements,
		* ex.
		* {TWT Element[0]|TWT Element[1]|TWT Element[2]}
		* , where TWT Element = {tag|length|CTRL|TWT param}
		*/
    } else {
		/*
		* The frame contains TWT element combinatory with 
		* multiple TWT parameters,
		* ex.
		* {tag|length|CTRL|TWT param[0]|TWT param[1]|TWT param[2]}
		*/
    }
#endif

	return WLAN_STATUS_SUCCESS;
}

/*
* All MLTWT links sharing the same TWT parameter
*/
uint32_t mltwtFillTWTElementAllInOne(
	struct ADAPTER *prAdapter,
	struct BSS_INFO *prBssInfo,
	struct IE_ML_TWT_T *prMLTWTBuf,
	uint8_t ucTWTFlowId,
	struct _TWT_PARAMS_T *prTWTParams)
{
	struct MLD_BSS_INFO *prMldBssInfo = NULL;
	struct LINK *prBssList = NULL;
	struct BSS_INFO *prCurrBssInfo = NULL;
	struct STA_RECORD *prStaRecOfAP = NULL;

	ASSERT(prAdapter);
	ASSERT(prBssInfo);
	ASSERT(prMLTWTBuf);
	ASSERT(prTWTParams);

	/* Add TWT element */
	prMLTWTBuf->ucId = ELEM_ID_TWT;
	prMLTWTBuf->ucLength = sizeof(struct IE_ML_TWT_T) - ELEM_HDR_LEN;

	/* Request Type */
	prMLTWTBuf->u2ReqType |= SET_TWT_RT_REQUEST(prTWTParams->fgReq) |
		SET_TWT_RT_SETUP_CMD(prTWTParams->ucSetupCmd) |
		SET_TWT_RT_TRIGGER(prTWTParams->fgTrigger) |
		TWT_REQ_TYPE_IMPLICIT_LAST_BCAST_PARAM |
		SET_TWT_RT_FLOW_TYPE(prTWTParams->fgUnannounced) |
		SET_TWT_RT_FLOW_ID(ucTWTFlowId) |
		SET_TWT_RT_WAKE_INTVAL_EXP(prTWTParams->ucWakeIntvalExponent) |
		SET_TWT_RT_PROTECTION(prTWTParams->fgProtect);

	prMLTWTBuf->u8TWT = CPU_TO_LE64(prTWTParams->u8TWT);
	prMLTWTBuf->ucMinWakeDur = prTWTParams->ucMinWakeDur;
	prMLTWTBuf->u2WakeIntvalMantiss =
		CPU_TO_LE16(prTWTParams->u2WakeIntvalMantiss);

	/* CTRL link ID bitmap manipulation */
	prMLTWTBuf->ucCtrl |= SET_ML_TWT_CTRL_LINK_ID_BITMAP(1);

	prMldBssInfo = mldBssGetByBss(prAdapter, prBssInfo);

	if (!prMldBssInfo) {
		DBGLOG(TWT_REQUESTER, ERROR, "ML Invalid MLD_BSS_INFO\n");

		return WLAN_STATUS_FAILURE;
	}

	prBssList = &prMldBssInfo->rBssList;

	LINK_FOR_EACH_ENTRY(prCurrBssInfo, prBssList,
			rLinkEntryMld,
			struct BSS_INFO) {
		if (!prCurrBssInfo)
			break;

		prStaRecOfAP = prCurrBssInfo->prStaRecOfAP;

		if (!prStaRecOfAP)
			break;

		prMLTWTBuf->u2LinkIdBitmap |= BIT(prStaRecOfAP->ucLinkIndex);
	}

	DBGLOG(TWT_REQUESTER, WARN,
		"Link ID bitmap %x\n", prMLTWTBuf->u2LinkIdBitmap);

	return WLAN_STATUS_SUCCESS;
}

/*
* All MLTWT links sharing the same TWT parameter
*/
uint32_t mltwtSendSetupFrameAllInOne(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	u_int8_t ucTWTFlowId,
	struct _TWT_PARAMS_T *prTWTParams,
	PFN_TX_DONE_HANDLER pfTxDoneHandler)
{
	struct MSDU_INFO *prMsduInfo;
	struct _ACTION_ML_TWT_SETUP_FRAME *prTxFrame;
	struct BSS_INFO *prBssInfo;
	uint16_t u2EstimatedFrameLen;
	struct IE_ML_TWT_T *prMLTWTBuf;

	ASSERT(prAdapter);
	ASSERT(prStaRec);
	ASSERT(prTWTParams);

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);

	ASSERT(prBssInfo);

	/* Calculate MSDU buffer length */
	u2EstimatedFrameLen = MAC_TX_RESERVED_FIELD
		+ sizeof(struct _ACTION_ML_TWT_SETUP_FRAME);

	/* Alloc MSDU_INFO */
	prMsduInfo = (struct MSDU_INFO *)
			cnmMgtPktAlloc(prAdapter, u2EstimatedFrameLen);

	if (!prMsduInfo) {
		DBGLOG(TWT_REQUESTER, WARN,
			"No MSDU_INFO_T for sending MLTWT Setup Frame.\n");
		return WLAN_STATUS_RESOURCES;
	}

	kalMemZero(prMsduInfo->prPacket, u2EstimatedFrameLen);

	prTxFrame = prMsduInfo->prPacket;

	/* Fill frame ctrl */
	prTxFrame->u2FrameCtrl = MAC_FRAME_ACTION;

	COPY_MAC_ADDR(prTxFrame->aucDestAddr, prStaRec->aucMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucSrcAddr, prBssInfo->aucOwnMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucBSSID, prBssInfo->aucBSSID);

	/* Compose the frame body's frame */
	prTxFrame->ucCategory = CATEGORY_S1G_ACTION;
	prTxFrame->ucAction = ACTION_S1G_TWT_SETUP;

	prMLTWTBuf = &(prTxFrame->rTWT);

	if (mltwtFillTWTElementAllInOne(
		prAdapter, prBssInfo,
		prMLTWTBuf, ucTWTFlowId,
		prTWTParams) == WLAN_STATUS_FAILURE) {
		DBGLOG(TWT_REQUESTER, ERROR, "MLTWT setup frame error\n");

		return WLAN_STATUS_FAILURE;
	}

	DBGLOG(TWT_REQUESTER, WARN,
		"MLTWT flowid %d\n", ucTWTFlowId);

	/* Update information of MSDU_INFO_T */
	TX_SET_MMPDU(prAdapter,
			prMsduInfo,
			prBssInfo->ucBssIndex,
			prStaRec->ucIndex,
			WLAN_MAC_MGMT_HEADER_LEN,
			sizeof(struct _ACTION_ML_TWT_SETUP_FRAME),
			pfTxDoneHandler, MSDU_RATE_MODE_AUTO);

	/* Enqueue the frame to send this action frame */
	nicTxEnqueueMsdu(prAdapter, prMsduInfo);

	return WLAN_STATUS_SUCCESS;
}

uint32_t mltwtGetLinkCount(
	struct ADAPTER *prAdapter,
	struct BSS_INFO *prBssInfo,
	uint8_t ucTWTFlowId)
{
	struct MLD_BSS_INFO *prMldBssInfo = NULL;
	struct LINK *prBssList = NULL;
	struct BSS_INFO *prCurrBssInfo = NULL;
	struct STA_RECORD *prStaRecOfAP = NULL;
	uint8_t ucMloLinkCount = 0;

	ASSERT(prAdapter);
	ASSERT(prBssInfo);

	prMldBssInfo = mldBssGetByBss(prAdapter, prBssInfo);

	if (!prMldBssInfo) {
		DBGLOG(TWT_REQUESTER, ERROR, "ML Invalid MLD_BSS_INFO\n");

		return 0;
	}

	prBssList = &prMldBssInfo->rBssList;

	LINK_FOR_EACH_ENTRY(prCurrBssInfo, prBssList,
			rLinkEntryMld,
			struct BSS_INFO) {
		if (!prCurrBssInfo)
			break;

		prStaRecOfAP = prCurrBssInfo->prStaRecOfAP;

		if (!prStaRecOfAP)
			break;

		if (prStaRecOfAP->arTWTFlow[ucTWTFlowId].fgIsMLTWT == TRUE)
			ucMloLinkCount++;
	}

	return ucMloLinkCount;
}

/*
* MLTWT per link distinct TWT parameter
*/
uint32_t mltwtFillTWTElementPerLinkDistinct(
	struct ADAPTER *prAdapter,
	struct BSS_INFO *prBssInfo,
	uint8_t *pucIE,
	uint16_t u2IELength,
	uint8_t ucTWTFlowId)
{
	struct MLD_BSS_INFO *prMldBssInfo = NULL;
	struct LINK *prBssList = NULL;
	struct BSS_INFO *prCurrBssInfo = NULL;
	struct STA_RECORD *prStaRecOfAP = NULL;
	uint16_t u2Offset = 0;
	struct _TWT_FLOW_T *prTWTFlow = NULL;
	struct _TWT_PARAMS_T *prTWTParams = NULL;
	struct IE_ML_TWT_T *prMLTWTBuf = NULL;

	ASSERT(prAdapter);
	ASSERT(prBssInfo);
	ASSERT(pucIE);

	prMldBssInfo = mldBssGetByBss(prAdapter, prBssInfo);

	if (!prMldBssInfo) {
		DBGLOG(TWT_REQUESTER, ERROR, "ML Invalid MLD_BSS_INFO\n");

		return WLAN_STATUS_FAILURE;
	}

	prBssList = &prMldBssInfo->rBssList;

	LINK_FOR_EACH_ENTRY(prCurrBssInfo, prBssList,
			rLinkEntryMld,
			struct BSS_INFO) {
		if (!prCurrBssInfo)
			break;

		prStaRecOfAP = prCurrBssInfo->prStaRecOfAP;

		if (!prStaRecOfAP)
			break;

		prTWTFlow = &(prStaRecOfAP->arTWTFlow[ucTWTFlowId]);

		if ((prTWTFlow->fgIsMLTWT == TRUE)
			&& (u2Offset < u2IELength))
		{
			prMLTWTBuf = (struct IE_ML_TWT_T *)(pucIE + u2Offset);

			prTWTParams = &(prTWTFlow->rTWTParams);

			/* Add TWT element */
			prMLTWTBuf->ucId = ELEM_ID_TWT;
			prMLTWTBuf->ucLength = sizeof(struct IE_ML_TWT_T) - ELEM_HDR_LEN;

			/* CTRL link ID bitmap manipulation */
			prMLTWTBuf->ucCtrl |= SET_ML_TWT_CTRL_LINK_ID_BITMAP(1);

			/* Request Type */
			prMLTWTBuf->u2ReqType |= SET_TWT_RT_REQUEST(prTWTParams->fgReq) |
				SET_TWT_RT_SETUP_CMD(prTWTParams->ucSetupCmd) |
				SET_TWT_RT_TRIGGER(prTWTParams->fgTrigger) |
				TWT_REQ_TYPE_IMPLICIT_LAST_BCAST_PARAM |
				SET_TWT_RT_FLOW_TYPE(prTWTParams->fgUnannounced) |
				SET_TWT_RT_FLOW_ID(ucTWTFlowId) |
				SET_TWT_RT_WAKE_INTVAL_EXP(prTWTParams->ucWakeIntvalExponent) |
				SET_TWT_RT_PROTECTION(prTWTParams->fgProtect);

			prMLTWTBuf->u8TWT = CPU_TO_LE64(prTWTParams->u8TWT);
			prMLTWTBuf->ucMinWakeDur = prTWTParams->ucMinWakeDur;
			prMLTWTBuf->u2WakeIntvalMantiss =
				CPU_TO_LE16(prTWTParams->u2WakeIntvalMantiss);

			/* Link ID bitmap manipulation */
			prMLTWTBuf->u2LinkIdBitmap = BIT(prStaRecOfAP->ucLinkIndex);

			/* Accumutate offset in the unit of (struct IE_ML_TWT_T) */
			u2Offset += sizeof(struct IE_ML_TWT_T);
		}
	}

	return WLAN_STATUS_SUCCESS;
}

/*
* MLTWT per link distinct TWT parameter
*/
uint32_t mltwtSendSetupFramePerLinkDistinct(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	u_int8_t ucTWTFlowId,
	struct _TWT_PARAMS_T *prTWTParams,
	PFN_TX_DONE_HANDLER pfTxDoneHandler)
{
	struct MSDU_INFO *prMsduInfo;
	struct _ACTION_ML_TWT_SETUP_FRAME_PER_LINK_DISTINCT *prTxFrame;
	struct BSS_INFO *prBssInfo;
	uint16_t u2EstimatedFrameLen;
	uint8_t ucMloLinkCount = 0;

	ASSERT(prAdapter);
	ASSERT(prStaRec);
	ASSERT(prTWTParams);

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);

	ASSERT(prBssInfo);

	ucMloLinkCount = mltwtGetLinkCount(
						prAdapter,
						prBssInfo,
						ucTWTFlowId);

	if (ucMloLinkCount == 0) {
		DBGLOG(TWT_REQUESTER, ERROR,
			"MLTWT %d no link.\n",
			ucTWTFlowId);

		return WLAN_STATUS_FAILURE;
	}

	/* Calculate MSDU buffer length */
	u2EstimatedFrameLen = MAC_TX_RESERVED_FIELD
		+ OFFSET_OF(
			struct _ACTION_ML_TWT_SETUP_FRAME_PER_LINK_DISTINCT,
			rTWT[0])
		+ (ucMloLinkCount * sizeof(struct IE_ML_TWT_T));

	/* Alloc MSDU_INFO */
	prMsduInfo = (struct MSDU_INFO *)
			cnmMgtPktAlloc(prAdapter, u2EstimatedFrameLen);

	if (!prMsduInfo) {
		DBGLOG(TWT_REQUESTER, ERROR,
			"No MSDU_INFO_T for sending MLTWT Setup Frame.\n");

		return WLAN_STATUS_RESOURCES;
	}

	kalMemZero(prMsduInfo->prPacket, u2EstimatedFrameLen);

	prTxFrame = prMsduInfo->prPacket;

	/* Fill frame ctrl */
	prTxFrame->u2FrameCtrl = MAC_FRAME_ACTION;

	COPY_MAC_ADDR(prTxFrame->aucDestAddr, prStaRec->aucMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucSrcAddr, prBssInfo->aucOwnMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucBSSID, prBssInfo->aucBSSID);

	/* Compose the frame body's frame */
	prTxFrame->ucCategory = CATEGORY_S1G_ACTION;
	prTxFrame->ucAction = ACTION_S1G_TWT_SETUP;

	/* Use the 1st struct _IE_ML_TWT_T fo rmanipulation */
	if (mltwtFillTWTElementPerLinkDistinct(
			prAdapter, prBssInfo, prTxFrame->rTWT,
			(ucMloLinkCount * sizeof(struct IE_ML_TWT_T)),
			ucTWTFlowId) == WLAN_STATUS_FAILURE) {
		DBGLOG(TWT_REQUESTER, ERROR, "MLTWT setup frame error\n");

		return WLAN_STATUS_FAILURE;
	}

	DBGLOG(TWT_REQUESTER, WARN,
		"MLTWT flowid %d\n", ucTWTFlowId);

	/* Update information of MSDU_INFO_T */
	TX_SET_MMPDU(prAdapter,
			prMsduInfo,
			prBssInfo->ucBssIndex,
			prStaRec->ucIndex,
			WLAN_MAC_MGMT_HEADER_LEN,
			u2EstimatedFrameLen,
			pfTxDoneHandler, MSDU_RATE_MODE_AUTO);

	/* Enqueue the frame to send this action frame */
	nicTxEnqueueMsdu(prAdapter, prMsduInfo);

	return WLAN_STATUS_SUCCESS;
}
#endif
