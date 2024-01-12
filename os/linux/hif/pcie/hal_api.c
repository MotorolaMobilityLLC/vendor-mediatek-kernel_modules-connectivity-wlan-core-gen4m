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
/******************************************************************************
*[File]             hif_api.c
*[Version]          v1.0
*[Revision Date]    2015-09-08
*[Author]
*[Description]
*    The program provides PCIE HIF APIs
*[Copyright]
*    Copyright (C) 2015 MediaTek Incorporation. All Rights Reserved.
******************************************************************************/

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/
#include "precomp.h"

#include "hif_pci.h"

#include <linux/mm.h>
#ifndef CONFIG_X86
#include <asm/memory.h>
#endif

#include "mt66xx_reg.h"

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/
#define RX_RESPONSE_TIMEOUT (15000)


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

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

/*----------------------------------------------------------------------------*/
/*!
* @brief Verify the CHIP ID
*
* @param prAdapter      a pointer to adapter private data structure.
*
*
* @retval TRUE          CHIP ID is the same as the setting compiled
* @retval FALSE         CHIP ID is different from the setting compiled
*/
/*----------------------------------------------------------------------------*/
BOOL halVerifyChipID(IN P_ADAPTER_T prAdapter)
{
	struct mt66xx_chip_info *prChipInfo;
	P_BUS_INFO prBusInfo;
	UINT_32 u4CIR = 0;

	ASSERT(prAdapter);

	prChipInfo = prAdapter->chip_info;
	prBusInfo = prChipInfo->bus_info;

	if (prAdapter->fgIsReadRevID || !prChipInfo->should_verify_chip_id)
		return TRUE;

	HAL_MCR_RD(prAdapter, prBusInfo->top_cfg_base + TOP_HW_CONTROL, &u4CIR);

	DBGLOG(INIT, INFO, "WCIR_CHIP_ID = 0x%x, chip_id = 0x%x\n",
		(u4CIR & WCIR_CHIP_ID), prChipInfo->chip_id);

	if ((u4CIR & WCIR_CHIP_ID) != prChipInfo->chip_id)
		return FALSE;

	HAL_MCR_RD(prAdapter, prBusInfo->top_cfg_base + TOP_HW_VERSION, &u4CIR);

	prAdapter->ucRevID = (UINT_8)(u4CIR & 0xF);
	prAdapter->fgIsReadRevID = TRUE;

	return TRUE;
}

WLAN_STATUS halRxWaitResponse(IN P_ADAPTER_T prAdapter, IN UINT_8 ucPortIdx,
	OUT PUINT_8 pucRspBuffer, IN UINT_32 u4MaxRespBufferLen, OUT PUINT_32 pu4Length)
{
	UINT_32 u4PktLen = 0, i = 0;
	WLAN_STATUS u4Status = WLAN_STATUS_SUCCESS;
	UINT_32 u4Time, u4Current;
	BOOLEAN fgStatus;

	DEBUGFUNC("nicRxWaitResponse");

	ASSERT(prAdapter);
	ASSERT(pucRspBuffer);
	ASSERT(ucPortIdx < 2);

	ucPortIdx = HIF_IMG_DL_STATUS_PORT_IDX;

	u4Time = (UINT_32) kalGetTimeTick();

	u4PktLen = u4MaxRespBufferLen;

	do {
		fgStatus = kalDevPortRead(prAdapter->prGlueInfo, ucPortIdx, u4PktLen,
			pucRspBuffer, HIF_RX_COALESCING_BUFFER_SIZE);

		if (!fgStatus) {
			/* timeout exceeding check */
			u4Current = (UINT_32) kalGetTimeTick();

			if ((u4Current > u4Time) && ((u4Current - u4Time) > RX_RESPONSE_TIMEOUT))
				return WLAN_STATUS_FAILURE;
			else if (u4Current < u4Time && ((u4Current + (0xFFFFFFFF - u4Time)) > RX_RESPONSE_TIMEOUT))
				return WLAN_STATUS_FAILURE;

			/* Response packet is not ready */
			kalUdelay(50);

			i++;
		} else {
			*pu4Length = u4PktLen;
			break;
		}
	} while (TRUE);

	return u4Status;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief enable global interrupt
*
* @param prAdapter pointer to the Adapter handler
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
VOID halDefaultEnableInterrupt(IN P_ADAPTER_T prAdapter)
{
	P_BUS_INFO prBusInfo = prAdapter->chip_info->bus_info;
	WPMDA_INT_MASK IntMask;

	prAdapter->fgIsIntEnable = TRUE;

	HAL_MCR_RD(prAdapter, WPDMA_INT_MSK, &IntMask.word);

	IntMask.field.rx_done_0 = 1;
	IntMask.field.rx_done_1 = 1;
	IntMask.field.tx_done = BIT(prBusInfo->tx_ring_fwdl_idx) |
		BIT(prBusInfo->tx_ring_cmd_idx) | BIT(prBusInfo->tx_ring_data_idx);
	IntMask.field.tx_coherent = 0;
	IntMask.field.rx_coherent = 0;
	IntMask.field.tx_dly_int = 0;
	IntMask.field.rx_dly_int = 0;
	IntMask.field.fw_clr_own = 1;

	HAL_MCR_WR(prAdapter, WPDMA_INT_MSK, IntMask.word);

	DBGLOG(HAL, TRACE, "%s [0x%08x]\n", __func__, IntMask.word);
}

/*----------------------------------------------------------------------------*/
/*!
* @brief enable global interrupt
*
* @param prAdapter pointer to the Adapter handler
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
VOID halEnableInterrupt(IN P_ADAPTER_T prAdapter)
{
	P_BUS_INFO prBusInfo;

	ASSERT(prAdapter);

	prBusInfo = prAdapter->chip_info->bus_info;

	if (prBusInfo->enableInterrupt)
		prBusInfo->enableInterrupt(prAdapter);
	else
		halDefaultEnableInterrupt(prAdapter);
}	/* end of nicEnableInterrupt() */

/*----------------------------------------------------------------------------*/
/*!
* @brief disable global interrupt
*
* @param prAdapter pointer to the Adapter handler
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
VOID halDisableInterrupt(IN P_ADAPTER_T prAdapter)
{
	P_GLUE_INFO_T prGlueInfo = NULL;
	WPMDA_INT_MASK IntMask;

	ASSERT(prAdapter);

	prGlueInfo = prAdapter->prGlueInfo;

	IntMask.word = 0;

	HAL_MCR_WR(prAdapter, WPDMA_INT_MSK, IntMask.word);
	HAL_MCR_RD(prAdapter, WPDMA_INT_MSK, &IntMask.word);

	prAdapter->fgIsIntEnable = FALSE;

	DBGLOG(HAL, TRACE, "%s\n", __func__);

}
/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is used to process the POWER OFF procedure.
*
* \param[in] pvAdapter Pointer to the Adapter structure.
*
* \return (none)
*/
/*----------------------------------------------------------------------------*/
BOOLEAN halSetDriverOwn(IN P_ADAPTER_T prAdapter)
{
	P_BUS_INFO prBusInfo;
	BOOLEAN fgStatus = TRUE;
	UINT_32 i, u4CurrTick, u4WriteTick, u4WriteTickTemp;
	BOOLEAN fgTimeout;
	BOOLEAN fgResult;
	BOOLEAN fgReady = FALSE;
	BOOLEAN fgDummyReq = FALSE;

	ASSERT(prAdapter);

	prBusInfo = prAdapter->chip_info->bus_info;

	GLUE_INC_REF_CNT(prAdapter->u4PwrCtrlBlockCnt);

	if (prAdapter->fgIsFwOwn == FALSE)
		return fgStatus;

	DBGLOG(INIT, INFO, "DRIVER OWN\n");

	u4WriteTick = 0;
	u4CurrTick = kalGetTimeTick();
	i = 0;

	/* PCIE need to do clear own, then could start polling status */
	HAL_LP_OWN_CLR(prAdapter, &fgResult);
	fgResult = FALSE;

	while (1) {

		if (!prBusInfo->fgCheckDriverOwnInt || test_bit(GLUE_FLAG_INT_BIT, &prAdapter->prGlueInfo->ulFlag))
			HAL_LP_OWN_RD(prAdapter, &fgResult);

		fgTimeout = ((kalGetTimeTick() - u4CurrTick) > LP_OWN_BACK_TOTAL_DELAY_MS) ? TRUE : FALSE;

		if (fgResult) {
			/* Check WPDMA FW own interrupt status and clear */
			/*
			*HAL_MCR_RD(prAdapter, WPDMA_INT_STA, &u4RegValue);
			*DBGLOG(INIT, INFO, "Already own back %x\n", u4RegValue);
			*/
			HAL_MCR_WR(prAdapter, WPDMA_INT_STA, WPDMA_FW_CLR_OWN_INT);
			prAdapter->fgIsFwOwn = FALSE;
			prAdapter->u4OwnFailedCount = 0;
			prAdapter->u4OwnFailedLogCount = 0;
			break;
		} else if ((i > LP_OWN_BACK_FAILED_RETRY_CNT) &&
			   (kalIsCardRemoved(prAdapter->prGlueInfo) || fgIsBusAccessFailed || fgTimeout
			    || wlanIsChipNoAck(prAdapter))) {

			if ((prAdapter->u4OwnFailedCount == 0) ||
			    CHECK_FOR_TIMEOUT(u4CurrTick, prAdapter->rLastOwnFailedLogTime,
					      MSEC_TO_SYSTIME(LP_OWN_BACK_FAILED_LOG_SKIP_MS))) {

				DBGLOG(INIT, ERROR,
				       "LP cannot be own back, Timeout[%u](%ums), BusAccessError[%u]",
				       fgTimeout, kalGetTimeTick() - u4CurrTick, fgIsBusAccessFailed);
				DBGLOG(INIT, ERROR,
				       "Resetting[%u], CardRemoved[%u] NoAck[%u] Cnt[%u]\n",
				       kalIsResetting(),
				       kalIsCardRemoved(prAdapter->prGlueInfo), wlanIsChipNoAck(prAdapter),
				       prAdapter->u4OwnFailedCount);

				DBGLOG(INIT, INFO,
				       "Skip LP own back failed log for next %ums\n", LP_OWN_BACK_FAILED_LOG_SKIP_MS);

				prAdapter->u4OwnFailedLogCount++;
				if (prAdapter->u4OwnFailedLogCount > LP_OWN_BACK_FAILED_RESET_CNT) {
					/* Trigger RESET */
#if CFG_CHIP_RESET_SUPPORT
					glGetRstReason(RST_DRV_OWN_FAIL);
					GL_RESET_TRIGGER(prAdapter, RST_FLAG_CHIP_RESET);
#endif
				}
				GET_CURRENT_SYSTIME(&prAdapter->rLastOwnFailedLogTime);
			}

			prAdapter->u4OwnFailedCount++;
			fgStatus = FALSE;
			break;
		}

		u4WriteTickTemp = kalGetTimeTick();
		if ((i == 0) || TIME_AFTER(u4WriteTickTemp, (u4WriteTick + LP_OWN_REQ_CLR_INTERVAL_MS))) {
			/* Driver get LP ownership per 200 ms, to avoid iteration time not accurate */
			HAL_LP_OWN_CLR(prAdapter, &fgResult);
			u4WriteTick = u4WriteTickTemp;
		}

		/* Delay for LP engine to complete its operation. */
		kalMsleep(LP_OWN_BACK_LOOP_DELAY_MS);
		i++;
	}

	/* For Low power Test */
	/* 1. Driver need to polling until CR4 ready, then could do normal Tx/Rx */
	/* 2. After CR4 readyy, send a dummy command to change data path to store-forward mode */
	if (prAdapter->fgIsFwDownloaded) {
		const UINT_32 ready_bits = prAdapter->chip_info->sw_ready_bits;

		HAL_WIFI_FUNC_READY_CHECK(prAdapter, WIFI_FUNC_DUMMY_REQ, &fgDummyReq);

		/* Wait CR4 ready */
		u4CurrTick = kalGetTimeTick();
		while (1) {
			/* kalMsleep(2); */
			HAL_WIFI_FUNC_READY_CHECK(prAdapter, ready_bits/*WIFI_FUNC_READY_BITS*/, &fgReady);

			if (fgReady) {
				break;
			} else if (kalIsCardRemoved(prAdapter->prGlueInfo) || fgIsBusAccessFailed || fgTimeout
			    || wlanIsChipNoAck(prAdapter)) {
				DBGLOG(INIT, INFO,
					"Skip waiting CR4 ready for next %ums\n", LP_OWN_BACK_FAILED_LOG_SKIP_MS);
				fgStatus = FALSE;
#if CFG_CHIP_RESET_SUPPORT
				glGetRstReason(RST_DRV_OWN_FAIL);
				GL_RESET_TRIGGER(prAdapter, RST_FLAG_CHIP_RESET);
#endif
				break;
			}
			/* Delay for CR4 to complete its operation. */
			kalMsleep(LP_OWN_BACK_LOOP_DELAY_MS);
		}

		/* Send dummy cmd and clear flag */
		if (fgDummyReq) {
			wlanSendDummyCmd(prAdapter, FALSE);
			HAL_CLEAR_DUMMY_REQ(prAdapter);
		}
	}

	return fgStatus;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is used to process the POWER ON procedure.
*
* \param[in] pvAdapter Pointer to the Adapter structure.
*
* \return (none)
*/
/*----------------------------------------------------------------------------*/
VOID halSetFWOwn(IN P_ADAPTER_T prAdapter, IN BOOLEAN fgEnableGlobalInt)
{
	BOOLEAN fgResult;

	ASSERT(prAdapter);

	ASSERT(prAdapter->u4PwrCtrlBlockCnt != 0);
	/* Decrease Block to Enter Low Power Semaphore count */
	GLUE_DEC_REF_CNT(prAdapter->u4PwrCtrlBlockCnt);
	if (!(prAdapter->fgWiFiInSleepyState && (prAdapter->u4PwrCtrlBlockCnt == 0)))
		return;

	if (prAdapter->fgIsFwOwn == TRUE)
		return;

	if (nicProcessIST(prAdapter) != WLAN_STATUS_NOT_INDICATING) {
		DBGLOG(INIT, STATE, "Skip FW OWN due to pending INT\n");
		/* pending interrupts */
		return;
	}


	if (fgEnableGlobalInt) {
		prAdapter->fgIsIntEnableWithLPOwnSet = TRUE;
	} else {
		HAL_LP_OWN_SET(prAdapter, &fgResult);

		prAdapter->fgIsFwOwn = TRUE;

		DBGLOG(INIT, INFO, "FW OWN\n");
	}
}

VOID halWakeUpWiFi(IN P_ADAPTER_T prAdapter)
{
	BOOLEAN fgResult;

#if CFG_SUPPORT_PMIC_SPI_CLOCK_SWITCH
	UINT_32 u4Value = 0;
	/*E1 PMIC clock workaround*/
	HAL_MCR_RD(prAdapter, TOP_CKGEN2_CR_PMIC_CK_MANUAL, &u4Value);

	if ((TOP_CKGEN2_CR_PMIC_CK_MANUAL_MASK & u4Value) == 0)
		HAL_MCR_WR(prAdapter, TOP_CKGEN2_CR_PMIC_CK_MANUAL, (TOP_CKGEN2_CR_PMIC_CK_MANUAL_MASK|u4Value));
	HAL_MCR_RD(prAdapter, TOP_CKGEN2_CR_PMIC_CK_MANUAL, &u4Value);
	DBGLOG(INIT, INFO, "PMIC SPI clock switch = %s\n",
		(TOP_CKGEN2_CR_PMIC_CK_MANUAL_MASK&u4Value)?"SUCCESS":"FAIL");
#endif

	ASSERT(prAdapter);

	HAL_LP_OWN_RD(prAdapter, &fgResult);

	if (fgResult)
		prAdapter->fgIsFwOwn = FALSE;
	else
		HAL_LP_OWN_CLR(prAdapter, &fgResult);
}

VOID halTxCancelSendingCmd(IN P_ADAPTER_T prAdapter, IN P_CMD_INFO_T prCmdInfo)
{
}

BOOLEAN halTxIsDataBufEnough(IN P_ADAPTER_T prAdapter, IN P_MSDU_INFO_T prMsduInfo)
{
	P_GL_HIF_INFO_T prHifInfo = NULL;
	P_RTMP_TX_RING prTxRing;

	prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	prTxRing = &prHifInfo->TxRing[TX_RING_DATA0_IDX_0];

	if ((halGetMsduTokenFreeCnt(prAdapter) > 0) && ((TX_RING_SIZE - prTxRing->u4UsedCnt) > 1))
		return TRUE;

	DBGLOG(HAL, TRACE, "Low Tx Data Resource Tok[%u] Ring[%u]\n", halGetMsduTokenFreeCnt(prAdapter),
		(TX_RING_SIZE - prTxRing->u4UsedCnt));
	return FALSE;

}

VOID halProcessTxInterrupt(IN P_ADAPTER_T prAdapter)
{
	P_BUS_INFO prBusInfo = prAdapter->chip_info->bus_info;
	P_GL_HIF_INFO_T prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	WPDMA_INT_STA_STRUCT rIntrStatus;

	rIntrStatus = (WPDMA_INT_STA_STRUCT)prHifInfo->u4IntStatus;

	if (rIntrStatus.field.tx_done & BIT(prBusInfo->tx_ring_fwdl_idx))
		halWpdmaProcessCmdDmaDone(prAdapter->prGlueInfo, TX_RING_FWDL_IDX_3);

	if (rIntrStatus.field.tx_done & BIT(prBusInfo->tx_ring_cmd_idx))
		halWpdmaProcessCmdDmaDone(prAdapter->prGlueInfo, TX_RING_CMD_IDX_2);

	if (rIntrStatus.field.tx_done & BIT(prBusInfo->tx_ring_data_idx)) {
		halWpdmaProcessDataDmaDone(prAdapter->prGlueInfo, TX_RING_DATA0_IDX_0);

		kalSetTxEvent2Hif(prAdapter->prGlueInfo);
	}
}

VOID halInitMsduTokenInfo(IN P_ADAPTER_T prAdapter)
{
	P_MSDU_TOKEN_INFO_T prTokenInfo = &prAdapter->prGlueInfo->rHifInfo.rTokenInfo;
	P_MSDU_TOKEN_ENTRY_T prToken;
	struct mt66xx_chip_info *prChipInfo = prAdapter->chip_info;
	UINT_32 u4Idx;
	UINT_32 u4TxHeadRoomSize;

	prTokenInfo->i4UsedCnt = 0;
	u4TxHeadRoomSize = NIC_TX_DESC_AND_PADDING_LENGTH + prChipInfo->txd_append_size;

	for (u4Idx = 0; u4Idx < HIF_TX_MSDU_TOKEN_NUM; u4Idx++) {
		prToken = &prTokenInfo->arToken[u4Idx];
		prToken->fgInUsed = FALSE;
		prToken->prMsduInfo = NULL;

#if HIF_TX_PREALLOC_DATA_BUFFER
		prToken->u4DmaLength = NIC_TX_MAX_SIZE_PER_FRAME + u4TxHeadRoomSize;
		prToken->prPacket = kalMemAlloc(prToken->u4DmaLength, PHY_MEM_TYPE);
		if (prToken->prPacket) {
			DBGLOG(HAL, TRACE, "Msdu Entry[0x%p] Tok[%u] Buf[0x%p] len[%u]\n", prToken,
				u4Idx, prToken->prPacket, prToken->u4DmaLength);
		} else {
			prTokenInfo->i4UsedCnt++;
			DBGLOG(HAL, WARN, "Msdu Token Memory alloc failed[%u]\n", u4Idx);
			continue;
		}
#else
		prToken->prPacket = NULL;
		prToken->u4DmaLength = 0;
#endif
		prToken->rDmaAddr = 0;
		prToken->rPktDmaAddr = 0;
		prToken->u4PktDmaLength = 0;
		prToken->u4Token = u4Idx;

		prTokenInfo->aprTokenStack[u4Idx] = prToken;
	}

	spin_lock_init(&prTokenInfo->rTokenLock);

	DBGLOG(HAL, INFO, "Msdu Token Init: Tot[%u] Used[%u]\n", HIF_TX_MSDU_TOKEN_NUM, prTokenInfo->i4UsedCnt);
}

VOID halUninitMsduTokenInfo(IN P_ADAPTER_T prAdapter)
{
	P_MSDU_TOKEN_INFO_T prTokenInfo = &prAdapter->prGlueInfo->rHifInfo.rTokenInfo;
	P_MSDU_TOKEN_ENTRY_T prToken;
	UINT_32 u4Idx;

	for (u4Idx = 0; u4Idx < HIF_TX_MSDU_TOKEN_NUM; u4Idx++) {
		prToken = &prTokenInfo->arToken[u4Idx];

		if (prToken->fgInUsed) {
			kalPciUnmapToDev(prAdapter->prGlueInfo, prToken->rPktDmaAddr, prToken->u4PktDmaLength);
			kalPciUnmapToDev(prAdapter->prGlueInfo, prToken->rDmaAddr, prToken->u4DmaLength);

			DBGLOG(HAL, TRACE, "Clear pending Tok[%u] Msdu[0x%p] Free[%u]\n", prToken->u4Token,
				prToken->prMsduInfo, halGetMsduTokenFreeCnt(prAdapter));

#if !HIF_TX_PREALLOC_DATA_BUFFER
			nicTxFreePacket(prAdapter, prToken->prMsduInfo, FALSE);
			nicTxReturnMsduInfo(prAdapter, prToken->prMsduInfo);
#endif
		}

#if HIF_TX_PREALLOC_DATA_BUFFER
		kalMemFree(prToken->prPacket, PHY_MEM_TYPE, prToken->u4DmaLength);
		prToken->prPacket = NULL;
#endif
	}

	prTokenInfo->i4UsedCnt = 0;

	DBGLOG(HAL, INFO, "Msdu Token Uninit: Tot[%u] Used[%u]\n", HIF_TX_MSDU_TOKEN_NUM, prTokenInfo->i4UsedCnt);
}

UINT_32 halGetMsduTokenFreeCnt(IN P_ADAPTER_T prAdapter)
{
	P_MSDU_TOKEN_INFO_T prTokenInfo = &prAdapter->prGlueInfo->rHifInfo.rTokenInfo;

	return (HIF_TX_MSDU_TOKEN_NUM - prTokenInfo->i4UsedCnt);
}

P_MSDU_TOKEN_ENTRY_T halGetMsduTokenEntry(IN P_ADAPTER_T prAdapter, UINT_32 u4TokenNum)
{
	P_MSDU_TOKEN_INFO_T prTokenInfo = &prAdapter->prGlueInfo->rHifInfo.rTokenInfo;

	return &prTokenInfo->arToken[u4TokenNum];
}

P_MSDU_TOKEN_ENTRY_T halAcquireMsduToken(IN P_ADAPTER_T prAdapter)
{
	P_MSDU_TOKEN_INFO_T prTokenInfo = &prAdapter->prGlueInfo->rHifInfo.rTokenInfo;
	P_MSDU_TOKEN_ENTRY_T prToken;
	ULONG flags = 0;

	if (!halGetMsduTokenFreeCnt(prAdapter)) {
		DBGLOG(HAL, INFO, "No more free MSDU token, Used[%u]\n", prTokenInfo->i4UsedCnt);
		return NULL;
	}

	spin_lock_irqsave(&prTokenInfo->rTokenLock, flags);

	prToken = prTokenInfo->aprTokenStack[prTokenInfo->i4UsedCnt];
	prToken->fgInUsed = TRUE;
	prTokenInfo->i4UsedCnt++;

	spin_unlock_irqrestore(&prTokenInfo->rTokenLock, flags);

	DBGLOG(HAL, TRACE, "Acquire Entry[0x%p] Tok[%u] Buf[%u] Len[%u]\n", prToken,
		prToken->u4Token, prToken->prPacket, prToken->u4DmaLength);

	return prToken;
}

VOID halReturnMsduToken(IN P_ADAPTER_T prAdapter, UINT_32 u4TokenNum)
{
	P_MSDU_TOKEN_INFO_T prTokenInfo = &prAdapter->prGlueInfo->rHifInfo.rTokenInfo;
	P_MSDU_TOKEN_ENTRY_T prToken;
	ULONG flags = 0;

	if (!prTokenInfo->i4UsedCnt) {
		DBGLOG(HAL, INFO, "MSDU token is full, Used[%u]\n", prTokenInfo->i4UsedCnt);
		return;
	}

	prToken = &prTokenInfo->arToken[u4TokenNum];

	spin_lock_irqsave(&prTokenInfo->rTokenLock, flags);

	prToken->fgInUsed = FALSE;
	prTokenInfo->i4UsedCnt--;
	prTokenInfo->aprTokenStack[prTokenInfo->i4UsedCnt] = prToken;

	spin_unlock_irqrestore(&prTokenInfo->rTokenLock, flags);
}

VOID halHifSwInfoInit(IN P_ADAPTER_T prAdapter)
{
	halWpdmaAllocRing(prAdapter->prGlueInfo);
	halWpdmaInitRing(prAdapter->prGlueInfo);
	halInitMsduTokenInfo(prAdapter);
}

VOID halRxProcessMsduReport(IN P_ADAPTER_T prAdapter, IN OUT P_SW_RFB_T prSwRfb)
{
	P_HW_MAC_MSDU_REPORT_T prMsduReport;
#if !HIF_TX_PREALLOC_DATA_BUFFER
	P_MSDU_INFO_T prMsduInfo;
#endif
	QUE_T rFreeQueue;
	P_QUE_T prFreeQueue;
	UINT_16 u2TokenCnt;
	UINT_32 u4Idx, u4Token;
	P_MSDU_TOKEN_ENTRY_T prTokenEntry;

	prFreeQueue = &rFreeQueue;
	QUEUE_INITIALIZE(prFreeQueue);

	prMsduReport = (P_HW_MAC_MSDU_REPORT_T)prSwRfb->pucRecvBuff;
	u2TokenCnt = prMsduReport->u2MsduCount;

	for (u4Idx = 0; u4Idx < u2TokenCnt; u4Idx++) {
		if (prMsduReport->u4Ver == 0)
			u4Token = prMsduReport->au4MsduToken[u4Idx >> 1].rFormatV1.u2MsduID[u4Idx & 1];
		else
			u4Token = prMsduReport->au4MsduToken[u4Idx].rFormatV2.u2MsduID;

		prTokenEntry = halGetMsduTokenEntry(prAdapter, u4Token);

#if HIF_TX_PREALLOC_DATA_BUFFER
		DBGLOG(HAL, TRACE, "MsduRpt: Cnt[%u] Tok[%u] Free[%u]\n", u2TokenCnt,
			u4Token, halGetMsduTokenFreeCnt(prAdapter));
#else
		prMsduInfo = prTokenEntry->prMsduInfo;
		prMsduInfo->prToken = NULL;
		if (!prMsduInfo->pfTxDoneHandler)
			QUEUE_INSERT_TAIL(prFreeQueue, (P_QUE_ENTRY_T) prMsduInfo);

		DBGLOG(HAL, TRACE, "MsduRpt: Cnt[%u] Tok[%u] Msdu[0x%p] TxDone[%u] Free[%u]\n", u2TokenCnt,
			u4Token, prMsduInfo, (prMsduInfo->pfTxDoneHandler ? TRUE : FALSE),
			halGetMsduTokenFreeCnt(prAdapter));
#endif
		kalPciUnmapToDev(prAdapter->prGlueInfo, prTokenEntry->rPktDmaAddr, prTokenEntry->u4PktDmaLength);
		kalPciUnmapToDev(prAdapter->prGlueInfo, prTokenEntry->rDmaAddr, prTokenEntry->u4DmaLength);

		halReturnMsduToken(prAdapter, u4Token);
	}

#if !HIF_TX_PREALLOC_DATA_BUFFER
	nicTxMsduDoneCb(prAdapter->prGlueInfo, prFreeQueue);
#endif

	/* Indicate Service Thread */
	if (wlanGetTxPendingFrameCount(prAdapter) > 0)
		kalSetEvent(prAdapter->prGlueInfo);

	kalSetTxEvent2Hif(prAdapter->prGlueInfo);
}

VOID halTxUpdateCutThroughDesc(P_GLUE_INFO_T prGlueInfo, P_MSDU_INFO_T prMsduInfo,
	P_MSDU_TOKEN_ENTRY_T prToken)
{
	P_GL_HIF_INFO_T prHifInfo = &prGlueInfo->rHifInfo;
	struct pci_dev *pdev = prHifInfo->pdev;
	struct mt66xx_chip_info *prChipInfo;
	struct TX_DESC_OPS_T *prTxDescOps;
	PUINT_8 pucBufferTxD;
	UINT_32 u4TxHeadRoomSize;
	dma_addr_t rDmaAddr;

	prChipInfo = prGlueInfo->prAdapter->chip_info;
	prTxDescOps = prChipInfo->prTxDescOps;
	pucBufferTxD = prToken->prPacket;
	u4TxHeadRoomSize = NIC_TX_DESC_AND_PADDING_LENGTH + prChipInfo->txd_append_size;

	rDmaAddr = pci_map_single(pdev, pucBufferTxD + u4TxHeadRoomSize, prMsduInfo->u2FrameLength,
		PCI_DMA_TODEVICE);
	if (pci_dma_mapping_error(pdev, rDmaAddr)) {
		DBGLOG(HAL, ERROR, "pci_map_single() error!\n");
		return;
	}

	if (prTxDescOps->fillHifAppend)
		prTxDescOps->fillHifAppend(prGlueInfo->prAdapter, prMsduInfo, prToken->u4Token,
					   rDmaAddr, 0, TRUE, prToken->prPacket);

	prToken->rPktDmaAddr = rDmaAddr;
	prToken->u4PktDmaLength = prMsduInfo->u2FrameLength;
}

UINT_32 halTxGetPageCount(IN P_ADAPTER_T prAdapter, IN UINT_32 u4FrameLength, IN BOOLEAN fgIncludeDesc)
{
	return 1;
}

WLAN_STATUS halTxPollingResource(IN P_ADAPTER_T prAdapter, IN UINT_8 ucTC)
{
	return WLAN_STATUS_SUCCESS;
}

VOID halSerHifReset(IN P_ADAPTER_T prAdapter)
{
}

VOID halRxPCIeReceiveRFBs(IN P_ADAPTER_T prAdapter, UINT_32 u4Port)
{
	P_RX_CTRL_T prRxCtrl;
	P_SW_RFB_T prSwRfb = (P_SW_RFB_T) NULL;
	PUINT_8 pucBuf = NULL;
	P_HW_MAC_RX_DESC_T prRxStatus;
	BOOLEAN fgStatus;
	UINT_32 u4RxCnt;

	KAL_SPIN_LOCK_DECLARATION();

	DEBUGFUNC("nicRxPCIeReceiveRFBs");

	ASSERT(prAdapter);

	prRxCtrl = &prAdapter->rRxCtrl;
	ASSERT(prRxCtrl);

	u4RxCnt = halWpdmaGetRxDmaDoneCnt(prAdapter->prGlueInfo, u4Port);

	while (u4RxCnt--) {
		KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_RX_FREE_QUE);
		QUEUE_REMOVE_HEAD(&prRxCtrl->rFreeSwRfbList, prSwRfb, P_SW_RFB_T);
		KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_RX_FREE_QUE);

		if (!prSwRfb) {
			DBGLOG(RX, WARN, "No More RFB for P[%u]\n", u4Port);
			break;
		}

		if (u4Port == RX_RING_DATA_IDX_0) {
			fgStatus = kalDevReadData(prAdapter->prGlueInfo, u4Port, prSwRfb);
		} else {
			pucBuf = prSwRfb->pucRecvBuff;
			ASSERT(pucBuf);

			fgStatus = kalDevPortRead(prAdapter->prGlueInfo, u4Port, CFG_RX_MAX_PKT_SIZE,
				pucBuf, CFG_RX_MAX_PKT_SIZE);
		}
		if (!fgStatus) {
			KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_RX_FREE_QUE);
			QUEUE_INSERT_TAIL(&prRxCtrl->rFreeSwRfbList, &prSwRfb->rQueEntry);
			KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_RX_FREE_QUE);

			continue;
		}

		prRxStatus = prSwRfb->prRxStatus;
		ASSERT(prRxStatus);

		prSwRfb->ucPacketType = (UINT_8) HAL_RX_STATUS_GET_PKT_TYPE(prRxStatus);
		DBGLOG(RX, TRACE, "ucPacketType = %d\n", prSwRfb->ucPacketType);

		if (prSwRfb->ucPacketType == RX_PKT_TYPE_MSDU_REPORT) {
			nicRxProcessMsduReport(prAdapter, prSwRfb);

			continue;
		}

		prSwRfb->ucStaRecIdx =
			secGetStaIdxByWlanIdx(prAdapter, (UINT_8)HAL_RX_STATUS_GET_WLAN_IDX(prRxStatus));

		KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_RX_QUE);
		QUEUE_INSERT_TAIL(&prRxCtrl->rReceivedRfbList, &prSwRfb->rQueEntry);
		RX_INC_CNT(prRxCtrl, RX_MPDU_TOTAL_COUNT);
		KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_RX_QUE);
	}

}

/*----------------------------------------------------------------------------*/
/*!
* @brief Read frames from the data port for PCIE
*        I/F, fill RFB and put each frame into the rReceivedRFBList queue.
*
* @param prAdapter      Pointer to the Adapter structure.
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
VOID halProcessRxInterrupt(IN P_ADAPTER_T prAdapter)
{
	P_GL_HIF_INFO_T prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	WPDMA_INT_STA_STRUCT rIntrStatus;

	rIntrStatus = (WPDMA_INT_STA_STRUCT)prHifInfo->u4IntStatus;

	if (rIntrStatus.field.rx_done_1)
		halRxPCIeReceiveRFBs(prAdapter, RX_RING_EVT_IDX_1);

	if (rIntrStatus.field.rx_done_0)
		halRxPCIeReceiveRFBs(prAdapter, RX_RING_DATA_IDX_0);
}

static INT_32 halWpdmaAllocRingDesc(P_GLUE_INFO_T prGlueInfo, RTMP_DMABUF *pDescRing, INT_32 size)
{
	P_GL_HIF_INFO_T prHifInfo = &prGlueInfo->rHifInfo;
	struct pci_dev *pdev = prHifInfo->pdev;

	pDescRing->AllocSize = size;

	pDescRing->AllocVa = (PVOID) pci_alloc_consistent(pdev, pDescRing->AllocSize, &pDescRing->AllocPa);

	if (pDescRing->AllocVa == NULL) {
		DBGLOG(HAL, ERROR, "Failed to allocate a big buffer\n");
		return -1;
	}

	/* Zero init this memory block */
	kalMemZero(pDescRing->AllocVa, size);

	return 0;
}

static INT_32 halWpdmaFreeRingDesc(P_GLUE_INFO_T prGlueInfo, RTMP_DMABUF *pDescRing)
{
	P_GL_HIF_INFO_T prHifInfo = &prGlueInfo->rHifInfo;
	struct pci_dev *pdev = prHifInfo->pdev;

	if (pDescRing->AllocVa)
		pci_free_consistent(pdev, pDescRing->AllocSize, pDescRing->AllocVa, pDescRing->AllocPa);

	kalMemZero(pDescRing, sizeof(RTMP_DMABUF));

	return TRUE;
}

static PVOID halWpdmaAllocRxPacketBuff(IN PVOID pPciDev, IN ULONG Length,
	IN BOOLEAN Cached, OUT PPVOID VirtualAddress, OUT dma_addr_t *phy_addr)
{
	struct sk_buff *pkt;

	/* pkt = __dev_alloc_skb(Length, GFP_DMA | GFP_ATOMIC); */
	pkt = dev_alloc_skb(Length);

	if (pkt == NULL)
		DBGLOG(HAL, ERROR, "can't allocate rx %ld size packet\n", Length);

	if (pkt) {
		*VirtualAddress = (PVOID) pkt->data;
		*phy_addr = pci_map_single(pPciDev, *VirtualAddress, Length, PCI_DMA_FROMDEVICE);
	} else {
		*VirtualAddress = (PVOID) NULL;
		*phy_addr = (dma_addr_t) 0;
	}

	return (PVOID) pkt;
}

VOID halWpdmaAllocRxRing(P_GLUE_INFO_T prGlueInfo, UINT_32 u4Num, UINT_32 u4Size,
	UINT_32 u4DescSize, UINT_32 u4BufSize)
{
	dma_addr_t RingBasePa;
	PVOID RingBaseVa;
	INT_32 index;
	RXD_STRUCT *pRxD;
	RTMP_DMABUF *pDmaBuf;
	RTMP_DMACB *dma_cb;
	PVOID pPacket;
	P_GL_HIF_INFO_T prHifInfo = &prGlueInfo->rHifInfo;

	/* Alloc RxRingDesc memory except Tx ring allocated eariler */
	halWpdmaAllocRingDesc(prGlueInfo, &prHifInfo->RxDescRing[u4Num], u4Size * u4DescSize);
	if (prHifInfo->RxDescRing[u4Num].AllocVa == NULL) {
		DBGLOG(HAL, ERROR, "\n\n\nRxDescRing[%p] allocation failed!!\n\n\n");
		return;
	}

	DBGLOG(HAL, INFO, "RxDescRing[%p]: total %d bytes allocated\n",
		prHifInfo->RxDescRing[u4Num].AllocVa, (INT_32) prHifInfo->RxDescRing[u4Num].AllocSize);

	/* Initialize Rx Ring and associated buffer memory */
	RingBasePa = prHifInfo->RxDescRing[u4Num].AllocPa;
	RingBaseVa = prHifInfo->RxDescRing[u4Num].AllocVa;

	prHifInfo->RxRing[u4Num].u4BufSize = u4BufSize;
	prHifInfo->RxRing[u4Num].u4RingSize = u4Size;
	prHifInfo->RxRing[u4Num].fgRxSegPkt = FALSE;

	for (index = 0; index < u4Size; index++) {
		dma_cb = &prHifInfo->RxRing[u4Num].Cell[index];
		/* Init RX Ring Size, Va, Pa variables */
		dma_cb->AllocSize = u4DescSize;
		dma_cb->AllocVa = RingBaseVa;
		dma_cb->AllocPa = RingBasePa;

		/* Offset to next ring descriptor address */
		RingBasePa += u4DescSize;
		RingBaseVa = (PUCHAR) RingBaseVa + u4DescSize;

		/* Setup Rx associated Buffer size & allocate share memory */
		pDmaBuf = &dma_cb->DmaBuf;
		pDmaBuf->AllocSize = u4BufSize;
		pPacket = halWpdmaAllocRxPacketBuff(prHifInfo->pdev, pDmaBuf->AllocSize,
			FALSE, &pDmaBuf->AllocVa, &pDmaBuf->AllocPa);

		/* keep allocated rx packet */
		dma_cb->pPacket = pPacket;
		if (pDmaBuf->AllocVa == NULL) {
			DBGLOG(HAL, ERROR, "\n\n\nFailed to allocate RxRing buffer index[%u]\n\n\n", index);
			return;
		}

		/* Zero init this memory block */
		kalMemZero(pDmaBuf->AllocVa, pDmaBuf->AllocSize);

		/* Write RxD buffer address & allocated buffer length */
		pRxD = (RXD_STRUCT *) dma_cb->AllocVa;
		pRxD->SDPtr0 = pDmaBuf->AllocPa & DMA_LOWER_32BITS_MASK;
		pRxD->SDPtr1 = ((UINT_64)pDmaBuf->AllocPa >> DMA_BITS_OFFSET) & DMA_HIGHER_4BITS_MASK;
		pRxD->SDLen0 = u4BufSize;
		pRxD->DMADONE = 0;
	}

	DBGLOG(HAL, TRACE, "Rx[%d] Ring: total %d entry allocated\n", u4Num, index);
}

VOID halHifRst(P_GLUE_INFO_T prGlueInfo)
{
	/* Reset dmashdl and wpdma */
	kalDevRegWrite(prGlueInfo, CONN_HIF_RST, 0x00000000);
	kalDevRegWrite(prGlueInfo, CONN_HIF_RST, 0x00000030);
}

VOID halWpdmaAllocRing(P_GLUE_INFO_T prGlueInfo)
{
	dma_addr_t RingBasePa;
	PVOID RingBaseVa;
	INT_32 index, num;
	TXD_STRUCT *pTxD;
	RTMP_TX_RING *pTxRing;
	RTMP_DMABUF *pDmaBuf;
	RTMP_DMACB *dma_cb;
	P_GL_HIF_INFO_T prHifInfo = &prGlueInfo->rHifInfo;

	do {
		/*
		 *   Allocate all ring descriptors, include TxD, RxD, MgmtD.
		 *   Although each size is different, to prevent cacheline and alignment
		 *   issue, I intentional set them all to 64 bytes
		 */
		for (num = 0; num < NUM_OF_TX_RING; num++) {
			dma_addr_t BufBasePa;
			PVOID BufBaseVa;

			/*
			 *  Allocate Tx ring descriptor's memory
			 */
			halWpdmaAllocRingDesc(prGlueInfo, &prHifInfo->TxDescRing[num], TX_RING_SIZE * TXD_SIZE);
			if (prHifInfo->TxDescRing[num].AllocVa == NULL)
				break;

			pDmaBuf = &prHifInfo->TxDescRing[num];
			DBGLOG(HAL, TRACE, "TxDescRing[%p]: total %d bytes allocated\n",
			       pDmaBuf->AllocVa, (INT_32) pDmaBuf->AllocSize);

			/* Save PA & VA for further operation */
			RingBasePa = pDmaBuf->AllocPa;
			RingBaseVa = pDmaBuf->AllocVa;

			/*
			  * Allocate all 1st TXBuf's memory for this TxRing
			 */
			halWpdmaAllocRingDesc(prGlueInfo, &prHifInfo->TxBufSpace[num],
						 TX_RING_SIZE * TX_DMA_1ST_BUFFER_SIZE);
			if (prHifInfo->TxBufSpace[num].AllocVa == NULL) {
				DBGLOG(HAL, ERROR, "Failed to allocate a big buffer\n");
				break;
			}

			/* Zero init this memory block */
			kalMemZero(prHifInfo->TxBufSpace[num].AllocVa, prHifInfo->TxBufSpace[num].AllocSize);

			/* Save PA & VA for further operation */
			BufBasePa = prHifInfo->TxBufSpace[num].AllocPa;
			BufBaseVa = prHifInfo->TxBufSpace[num].AllocVa;

			/*
			 * Initialize Tx Ring Descriptor and associated buffer memory
			 */
			pTxRing = &prHifInfo->TxRing[num];
			for (index = 0; index < TX_RING_SIZE; index++) {
				dma_cb = &pTxRing->Cell[index];
				dma_cb->pPacket = NULL;
				dma_cb->pBuffer = NULL;
				/* Init Tx Ring Size, Va, Pa variables */
				dma_cb->AllocSize = TXD_SIZE;
				dma_cb->AllocVa = RingBaseVa;
				dma_cb->AllocPa = RingBasePa;

				/* Setup Tx Buffer size & address. only 802.11 header will store in this space */
				pDmaBuf = &dma_cb->DmaBuf;
				pDmaBuf->AllocSize = TX_DMA_1ST_BUFFER_SIZE;
				pDmaBuf->AllocVa = BufBaseVa;
				pDmaBuf->AllocPa = BufBasePa;

				/* link the pre-allocated TxBuf to TXD */
				pTxD = (TXD_STRUCT *) dma_cb->AllocVa;
				pTxD->SDPtr0 = BufBasePa;
				/* advance to next ring descriptor address */
				pTxD->DMADONE = 1;

				RingBasePa += TXD_SIZE;
				RingBaseVa = (PUCHAR) RingBaseVa + TXD_SIZE;

				/* advance to next TxBuf address */
				BufBasePa += TX_DMA_1ST_BUFFER_SIZE;
				BufBaseVa = (PUCHAR) BufBaseVa + TX_DMA_1ST_BUFFER_SIZE;
			}

			DBGLOG(HAL, TRACE, "TxRing[%d]: total %d entry allocated\n", num, index);
		}

		/* Data Rx path */
		halWpdmaAllocRxRing(prGlueInfo, RX_RING_DATA_IDX_0, RX_RING0_SIZE, RXD_SIZE, CFG_RX_MAX_PKT_SIZE);
		/* Event Rx path */
		halWpdmaAllocRxRing(prGlueInfo, RX_RING_EVT_IDX_1, RX_RING1_SIZE, RXD_SIZE, RX_BUFFER_AGGRESIZE);
	} while (FALSE);

	/* Initialize all transmit related software queues */

	/* Init TX rings index pointer */
	for (index = 0; index < NUM_OF_TX_RING; index++) {
		prHifInfo->TxRing[index].TxSwUsedIdx = 0;
		prHifInfo->TxRing[index].TxCpuIdx = 0;
	}

}

VOID halWpdmaFreeRing(P_GLUE_INFO_T prGlueInfo)
{
	int index, num, j;
	RTMP_TX_RING *pTxRing;
	TXD_STRUCT *pTxD;
	PVOID pPacket, pBuffer;
	RTMP_DMACB *dma_cb;
	P_GL_HIF_INFO_T prHifInfo = &prGlueInfo->rHifInfo;
	struct pci_dev *pdev = prHifInfo->pdev;

	/* Free Tx Ring Packet */
	for (index = 0; index < NUM_OF_TX_RING; index++) {
		pTxRing = &prHifInfo->TxRing[index];

		for (j = 0; j < TX_RING_SIZE; j++) {
			pTxD = (TXD_STRUCT *) (pTxRing->Cell[j].AllocVa);

			pPacket = pTxRing->Cell[j].pPacket;
			pBuffer = pTxRing->Cell[j].pBuffer;

			if (pPacket || pBuffer)
				pci_unmap_single(pdev, pTxD->SDPtr0, pTxD->SDLen0, PCI_DMA_TODEVICE);

			pTxRing->Cell[j].pPacket = NULL;

			if (pBuffer)
				kalMemFree(pBuffer, PHY_MEM_TYPE, 0);

			pTxRing->Cell[j].pBuffer = NULL;
		}
	}

	for (j = 0; j < NUM_OF_RX_RING; j++) {
		for (index = prHifInfo->RxRing[j].u4RingSize - 1; index >= 0; index--) {
			dma_cb = &prHifInfo->RxRing[j].Cell[index];
			if ((dma_cb->DmaBuf.AllocVa) && (dma_cb->pPacket)) {
				pci_unmap_single(pdev, dma_cb->DmaBuf.AllocPa,
					dma_cb->DmaBuf.AllocSize, PCI_DMA_FROMDEVICE);

				kalPacketFree(prGlueInfo, dma_cb->pPacket);
			}
		}
		kalMemZero(prHifInfo->RxRing[j].Cell, prHifInfo->RxRing[j].u4RingSize * sizeof(RTMP_DMACB));

		halWpdmaFreeRingDesc(prGlueInfo, &prHifInfo->RxDescRing[j]);
	}

	for (num = 0; num < NUM_OF_TX_RING; num++) {
		halWpdmaFreeRingDesc(prGlueInfo, &prHifInfo->TxBufSpace[num]);

		halWpdmaFreeRingDesc(prGlueInfo, &prHifInfo->TxDescRing[num]);
	}
}

/*----------------------------------------------------------------------------*/
/*!
* @brief enable firmware download.
*
* @param[in] fgEnable 1 for fw download, 0 for normal data operation.
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
VOID halEnableFWDownload(IN P_ADAPTER_T prAdapter, IN BOOL fgEnable)
{
	struct mt66xx_chip_info *prChipInfo;

	ASSERT(prAdapter);

	prChipInfo = prAdapter->chip_info;

	if (prChipInfo->asicEnableFWDownload)
		prChipInfo->asicEnableFWDownload(prAdapter, fgEnable);
}

static BOOLEAN halWpdmaWaitIdle(P_GLUE_INFO_T prGlueInfo, INT_32 round, INT_32 wait_us)
{
	INT_32 i = 0;
	WPDMA_GLO_CFG_STRUCT GloCfg;

	do {
		kalDevRegRead(prGlueInfo, WPDMA_GLO_CFG, &GloCfg.word);
		if ((GloCfg.field.TxDMABusy == 0) && (GloCfg.field.RxDMABusy == 0)) {
			DBGLOG(HAL, TRACE, "==>  DMAIdle, GloCfg=0x%x\n", GloCfg.word);
			return TRUE;
		}
		kalUdelay(wait_us);
	} while ((i++) < round);

	DBGLOG(HAL, INFO, "==>  DMABusy, GloCfg=0x%x\n", GloCfg.word);

	return FALSE;
}

VOID halWpdmaInitRing(P_GLUE_INFO_T prGlueInfo)
{
	P_GL_HIF_INFO_T prHifInfo;
	P_BUS_INFO prBusInfo;
	UINT_32 i = 0;

	ASSERT(prGlueInfo);

	prHifInfo = &prGlueInfo->rHifInfo;
	prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;

	/* Set DMA global configuration except TX_DMA_EN and RX_DMA_EN bits */
	prBusInfo->pdmaSetup(prGlueInfo, FALSE);

	halWpdmaWaitIdle(prGlueInfo, 100, 1000);

	/* Reset DMA Index */
	kalDevRegWrite(prGlueInfo, WPDMA_RST_PTR, 0xFFFFFFFF);

	halWpdmaInitTxRing(prGlueInfo);
	for (i = 0; i < NUM_OF_TX_RING; i++)
		spin_lock_init((spinlock_t *) (&prHifInfo->TxRingLock[i]));

	/* Init RX Ring0 Base/Size/Index pointer CSR */
	halWpdmaInitRxRing(prGlueInfo);
	for (i = 0; i < NUM_OF_RX_RING; i++)
		spin_lock_init((spinlock_t *) (&prHifInfo->RxRingLock[i]));

	prBusInfo->pdmaSetup(prGlueInfo, TRUE);
}

VOID halWpdmaInitTxRing(IN P_GLUE_INFO_T prGlueInfo)
{
	P_GL_HIF_INFO_T prHifInfo = NULL;
	P_RTMP_TX_RING prTxRing = NULL;
	P_BUS_INFO prBusInfo = NULL;
	UINT_32 i = 0, offset = 0, phy_addr = 0;
	UINT_32 phy_addr_ext = 0, ext_offset = 0;

	prHifInfo = &prGlueInfo->rHifInfo;
	prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;

	/* reset all TX Ring register */
	for (i = 0; i < NUM_OF_TX_RING; i++) {
		prTxRing = &prHifInfo->TxRing[i];
		if (i == TX_RING_CMD_IDX_2)
			offset = prBusInfo->tx_ring_cmd_idx * MT_RINGREG_DIFF;
		else
			offset = i * MT_RINGREG_DIFF;
		phy_addr = (UINT_32)(prHifInfo->TxRing[i].Cell[0].AllocPa & DMA_LOWER_32BITS_MASK);
		phy_addr_ext = (UINT_32)(((UINT_64)prHifInfo->TxRing[i].Cell[0].AllocPa >> DMA_BITS_OFFSET)
					 & DMA_HIGHER_4BITS_MASK);
		ext_offset = i * MT_RINGREG_EXT_DIFF;
		prTxRing->TxSwUsedIdx = 0;
		prTxRing->u4UsedCnt = 0;
		prTxRing->TxCpuIdx = 0;
		prTxRing->hw_desc_base = MT_TX_RING_BASE + offset;
		prTxRing->hw_desc_base_ext = MT_TX_RING_BASE_EXT + ext_offset;
		prTxRing->hw_cidx_addr = MT_TX_RING_CIDX + offset;
		prTxRing->hw_didx_addr = MT_TX_RING_DIDX + offset;
		prTxRing->hw_cnt_addr = MT_TX_RING_CNT + offset;
		kalDevRegWrite(prGlueInfo, prTxRing->hw_desc_base, phy_addr);
		kalDevRegWrite(prGlueInfo, prTxRing->hw_desc_base_ext, phy_addr_ext);
		kalDevRegWrite(prGlueInfo, prTxRing->hw_cidx_addr, prTxRing->TxCpuIdx);
		kalDevRegWrite(prGlueInfo, prTxRing->hw_cnt_addr, TX_RING_SIZE);

		DBGLOG(HAL, INFO, "-->TX_RING_%d[0x%x]: Base=0x%x, Cnt=%d!\n",
		       i, prHifInfo->TxRing[i].hw_desc_base, phy_addr, TX_RING_SIZE);
	}
}

VOID halWpdmaInitRxRing(IN P_GLUE_INFO_T prGlueInfo)
{
	P_GL_HIF_INFO_T prHifInfo = NULL;
	P_RTMP_RX_RING prRxRing = NULL;
	UINT_32 i = 0, offset = 0, phy_addr = 0;
	UINT_32 phy_addr_ext = 0, ext_offset = 0;

	ASSERT(prGlueInfo);
	prHifInfo = &prGlueInfo->rHifInfo;

	/* reset all RX Ring register */
	for (i = 0; i < NUM_OF_RX_RING; i++) {
		prRxRing = &prHifInfo->RxRing[i];
		offset = i * MT_RINGREG_DIFF;
		phy_addr = (UINT_32)(prRxRing->Cell[0].AllocPa & DMA_LOWER_32BITS_MASK);
		phy_addr_ext = (UINT_32)(((UINT_64)prRxRing->Cell[0].AllocPa >> DMA_BITS_OFFSET)
					 & DMA_HIGHER_4BITS_MASK);
		ext_offset = i * MT_RINGREG_EXT_DIFF;
		prRxRing->RxSwReadIdx = 0;
		prRxRing->RxCpuIdx = prRxRing->u4RingSize - 1;
		prRxRing->hw_desc_base = MT_RX_RING_BASE + offset;
		prRxRing->hw_desc_base_ext = MT_RX_RING_BASE_EXT + ext_offset;
		prRxRing->hw_cidx_addr = MT_RX_RING_CIDX + offset;
		prRxRing->hw_didx_addr = MT_RX_RING_DIDX + offset;
		prRxRing->hw_cnt_addr = MT_RX_RING_CNT + offset;
		kalDevRegWrite(prGlueInfo, prRxRing->hw_desc_base, phy_addr);
		kalDevRegWrite(prGlueInfo, prRxRing->hw_desc_base_ext, phy_addr_ext);
		kalDevRegWrite(prGlueInfo, prRxRing->hw_cidx_addr, prRxRing->RxCpuIdx);
		kalDevRegWrite(prGlueInfo, prRxRing->hw_cnt_addr, prRxRing->u4RingSize);

		DBGLOG(HAL, INFO, "-->RX_RING_%d[0x%x]: Base=0x%x, Cnt=%d\n",
		       i, prRxRing->hw_desc_base, phy_addr, prRxRing->u4RingSize);
	}
}

VOID halWpdmaProcessCmdDmaDone(IN P_GLUE_INFO_T prGlueInfo, IN UINT_16 u2Port)
{
	P_GL_HIF_INFO_T prHifInfo = NULL;
	UINT_32 u4SwIdx, u4DmaIdx;
	P_RTMP_TX_RING prTxRing;
	TXD_STRUCT *pTxD;
	PVOID pBuffer = NULL;
	struct pci_dev *pdev;
	ULONG flags = 0;

	ASSERT(prGlueInfo);

	prHifInfo = &prGlueInfo->rHifInfo;
	pdev = prHifInfo->pdev;
	prTxRing = &prHifInfo->TxRing[u2Port];

	spin_lock_irqsave(&prHifInfo->TxRingLock[u2Port], flags);

	kalDevRegRead(prGlueInfo, prTxRing->hw_didx_addr, &u4DmaIdx);

	u4SwIdx = prTxRing->TxSwUsedIdx;

	do {
		pBuffer = prTxRing->Cell[u4SwIdx].pBuffer;
		pTxD = (TXD_STRUCT *) prTxRing->Cell[u4SwIdx].AllocVa;

		if (!pBuffer || (pTxD->DMADONE == 0))
			break;

		DBGLOG(HAL, TRACE, "DMA done: port[%u] dma[%u] idx[%u] done[%u] pkt[0x%p] used[%u]\n", u2Port,
			u4DmaIdx, u4SwIdx, pTxD->DMADONE, prTxRing->Cell[u4SwIdx].pPacket, prTxRing->u4UsedCnt);

		if (pTxD->SDPtr0)
			pci_unmap_single(pdev, pTxD->SDPtr0, pTxD->SDLen0, PCI_DMA_TODEVICE);

		if (pTxD->SDPtr1)
			pci_unmap_single(pdev, pTxD->SDPtr1, pTxD->SDLen1, PCI_DMA_TODEVICE);

		pTxD->DMADONE = 0;
		kalMemFree(prTxRing->Cell[u4SwIdx].pBuffer, PHY_MEM_TYPE, 0);
		prTxRing->Cell[u4SwIdx].pBuffer = NULL;
		prTxRing->Cell[u4SwIdx].pPacket = NULL;
		prTxRing->u4UsedCnt--;

		if (u2Port == TX_RING_CMD_IDX_2)
			nicTxReleaseResource_PSE(prGlueInfo->prAdapter, TC4_INDEX,
				nicTxGetPageCount(prGlueInfo->prAdapter, pTxD->SDLen0, TRUE), TRUE);

		INC_RING_INDEX(u4SwIdx, TX_RING_SIZE);
	} while (u4SwIdx != u4DmaIdx);

	prTxRing->TxSwUsedIdx = u4SwIdx;

	spin_unlock_irqrestore(&prHifInfo->TxRingLock[u2Port], flags);
}

VOID halWpdmaProcessDataDmaDone(IN P_GLUE_INFO_T prGlueInfo, IN UINT_16 u2Port)
{
	P_GL_HIF_INFO_T prHifInfo = NULL;
	UINT_32 u4SwIdx, u4DmaIdx;
	P_RTMP_TX_RING prTxRing;
	ULONG flags = 0;

	ASSERT(prGlueInfo);

	prHifInfo = &prGlueInfo->rHifInfo;
	prTxRing = &prHifInfo->TxRing[u2Port];

	kalDevRegRead(prGlueInfo, prTxRing->hw_didx_addr, &u4DmaIdx);
	u4SwIdx = prTxRing->TxSwUsedIdx;

	spin_lock_irqsave(&prHifInfo->TxRingLock[u2Port], flags);

	if (u4DmaIdx > u4SwIdx)
		prTxRing->u4UsedCnt -= u4DmaIdx - u4SwIdx;
	else if (u4DmaIdx < u4SwIdx)
		prTxRing->u4UsedCnt -= (TX_RING_SIZE + u4DmaIdx) - u4SwIdx;
	else {
		/* DMA index == SW used index */
		if (prTxRing->u4UsedCnt == TX_RING_SIZE)
			prTxRing->u4UsedCnt = 0;
	}

	spin_unlock_irqrestore(&prHifInfo->TxRingLock[u2Port], flags);

	DBGLOG(HAL, TRACE, "DMA done: port[%u] dma[%u] idx[%u] used[%u]\n", u2Port,
		u4DmaIdx, u4SwIdx, prTxRing->u4UsedCnt);

	prTxRing->TxSwUsedIdx = u4DmaIdx;
}

UINT_32 halWpdmaGetRxDmaDoneCnt(IN P_GLUE_INFO_T prGlueInfo, IN UINT_8 ucRingNum)
{
	P_RTMP_RX_RING prRxRing;
	P_GL_HIF_INFO_T prHifInfo;
	UINT_32 u4MaxCnt, u4CpuIdx, u4DmaIdx, u4RxPktCnt;

	prHifInfo = &prGlueInfo->rHifInfo;
	prRxRing = &prHifInfo->RxRing[ucRingNum];

	kalDevRegRead(prGlueInfo, prRxRing->hw_cnt_addr, &u4MaxCnt);
	kalDevRegRead(prGlueInfo, prRxRing->hw_cidx_addr, &u4CpuIdx);
	kalDevRegRead(prGlueInfo, prRxRing->hw_didx_addr, &u4DmaIdx);

	u4RxPktCnt = u4MaxCnt;

	if (u4CpuIdx > u4DmaIdx)
		u4RxPktCnt = u4MaxCnt + u4DmaIdx - u4CpuIdx - 1;
	else if (u4CpuIdx < u4DmaIdx)
		u4RxPktCnt = u4DmaIdx - u4CpuIdx - 1;

	return u4RxPktCnt;
}

BOOL halWpdmaWriteCmd(IN P_GLUE_INFO_T prGlueInfo, IN P_CMD_INFO_T prCmdInfo, IN UINT_8 ucTC)
{
	P_GL_HIF_INFO_T prHifInfo = NULL;
	ULONG flags = 0;
	UINT_32 SwIdx = 0;
	P_RTMP_TX_RING prTxRing;
	spinlock_t *prTxRingLock;
	TXD_STRUCT *pTxD;
	struct pci_dev *pdev = NULL;
	UINT_16 u2Port = TX_RING_CMD_IDX_2;
	UINT_32 u4TotalLen;
	PUINT_8 pucSrc = NULL;

	ASSERT(prGlueInfo);
	prHifInfo = &prGlueInfo->rHifInfo;

	pdev = prHifInfo->pdev;
	prTxRing = &prHifInfo->TxRing[u2Port];
	prTxRingLock = &prHifInfo->TxRingLock[u2Port];

	u4TotalLen = prCmdInfo->u4TxdLen + prCmdInfo->u4TxpLen;
	pucSrc = kalMemAlloc(u4TotalLen, PHY_MEM_TYPE);
	ASSERT(pucSrc);

	kalMemCopy(pucSrc, prCmdInfo->pucTxd, prCmdInfo->u4TxdLen);
	kalMemCopy(pucSrc + prCmdInfo->u4TxdLen, prCmdInfo->pucTxp, prCmdInfo->u4TxpLen);

	spin_lock_irqsave((spinlock_t *)prTxRingLock, flags);

	kalCheckAndResetTXReg(prGlueInfo, TX_RING_CMD_IDX_2);
	SwIdx = prTxRing->TxCpuIdx;
	pTxD = (TXD_STRUCT *) prTxRing->Cell[SwIdx].AllocVa;

	prTxRing->Cell[SwIdx].pPacket = (PVOID)prCmdInfo;
	prTxRing->Cell[SwIdx].pBuffer = pucSrc;
	prTxRing->Cell[SwIdx].PacketPa = pci_map_single(pdev, pucSrc, u4TotalLen, PCI_DMA_TODEVICE);
	if (pci_dma_mapping_error(pdev, prTxRing->Cell[SwIdx].PacketPa)) {
		DBGLOG(HAL, ERROR, "pci_map_single() error!\n");
		kalMemFree(pucSrc, PHY_MEM_TYPE, u4TotalLen);
		spin_unlock_irqrestore((spinlock_t *)prTxRingLock, flags);
		ASSERT(0);
		return FALSE;
	}

	pTxD->SDPtr0 = prTxRing->Cell[SwIdx].PacketPa & DMA_LOWER_32BITS_MASK;
	pTxD->SDPtr0Ext = ((UINT_64)prTxRing->Cell[SwIdx].PacketPa >> DMA_BITS_OFFSET) & DMA_HIGHER_4BITS_MASK;
	pTxD->SDLen0 = u4TotalLen;
	pTxD->SDPtr1 = 0;
	pTxD->SDLen1 = 0;
	pTxD->LastSec0 = 1;
	pTxD->LastSec1 = 0;
	pTxD->Burst = 0;
	pTxD->DMADONE = 0;

	/* Increase TX_CTX_IDX, but write to register later. */
	INC_RING_INDEX(prTxRing->TxCpuIdx, TX_RING_SIZE);

	prTxRing->u4UsedCnt++;
	kalDevRegWrite(prGlueInfo, prTxRing->hw_cidx_addr, prTxRing->TxCpuIdx);

	spin_unlock_irqrestore((spinlock_t *)prTxRingLock, flags);

	DBGLOG(HAL, TRACE, "%s: CmdInfo[0x%p], TxD[0x%p/%u] TxP[0x%p/%u] CPU idx[%u] Used[%u]\n", __func__,
		prCmdInfo, prCmdInfo->pucTxd, prCmdInfo->u4TxdLen, prCmdInfo->pucTxp, prCmdInfo->u4TxpLen,
		SwIdx, prTxRing->u4UsedCnt);

	return TRUE;
}

BOOL halWpdmaWriteData(IN P_GLUE_INFO_T prGlueInfo, IN P_MSDU_INFO_T prMsduInfo)
{
	P_GL_HIF_INFO_T prHifInfo = NULL;
	ULONG flags = 0;
	UINT_32 SwIdx = 0;
	P_RTMP_TX_RING prTxRing;
	spinlock_t *prTxRingLock;
	TXD_STRUCT *pTxD;
	struct pci_dev *pdev = NULL;
	UINT_16 u2Port = TX_RING_DATA0_IDX_0;
	UINT_32 u4TotalLen;
	struct sk_buff *skb;
	PUINT_8 pucSrc;
	P_MSDU_TOKEN_ENTRY_T prToken;
	struct mt66xx_chip_info *prChipInfo;

	ASSERT(prGlueInfo);
	prHifInfo = &prGlueInfo->rHifInfo;
	prChipInfo = prGlueInfo->prAdapter->chip_info;

	pdev = prHifInfo->pdev;
	prTxRing = &prHifInfo->TxRing[u2Port];
	prTxRingLock = &prHifInfo->TxRingLock[u2Port];

	skb = (struct sk_buff *)prMsduInfo->prPacket;
	pucSrc = skb->data;
	u4TotalLen = skb->len;

	/* Acquire MSDU token */
	prToken = halAcquireMsduToken(prGlueInfo->prAdapter);

#if HIF_TX_PREALLOC_DATA_BUFFER
	kalMemCopy(prToken->prPacket, pucSrc, u4TotalLen);
#else
	prToken->prMsduInfo = prMsduInfo;
	prToken->prPacket = pucSrc;
	prToken->u4DmaLength = u4TotalLen;
	prMsduInfo->prToken = prToken;
#endif

	/* Update Tx descriptor */
	halTxUpdateCutThroughDesc(prGlueInfo, prMsduInfo, prToken);

	prToken->rDmaAddr = pci_map_single(pdev, prToken->prPacket, prToken->u4DmaLength, PCI_DMA_TODEVICE);
	if (pci_dma_mapping_error(pdev, prToken->rDmaAddr)) {
		DBGLOG(HAL, ERROR, "pci_map_single() error!\n");
		halReturnMsduToken(prGlueInfo->prAdapter, prToken->u4Token);
		ASSERT(0);
		return FALSE;
	}

	kalCheckAndResetTXReg(prGlueInfo, u2Port);
	SwIdx = prTxRing->TxCpuIdx;
	pTxD = (TXD_STRUCT *) prTxRing->Cell[SwIdx].AllocVa;

	pTxD->SDPtr0 = prToken->rDmaAddr & DMA_LOWER_32BITS_MASK;
	pTxD->SDPtr0Ext = ((UINT_64)prToken->rDmaAddr >> DMA_BITS_OFFSET) & DMA_HIGHER_4BITS_MASK;
	pTxD->SDLen0 = NIC_TX_DESC_AND_PADDING_LENGTH + prChipInfo->txd_append_size;
	if (prChipInfo->is_support_cr4)
		pTxD->SDLen0 += HIF_TX_PAYLOAD_LENGTH;
	pTxD->SDPtr1 = 0;
	pTxD->SDLen1 = 0;
	pTxD->LastSec0 = 1;
	pTxD->LastSec1 = 0;
	pTxD->Burst = 0;
	pTxD->DMADONE = 0;

	/* Increase TX_CTX_IDX, but write to register later. */
	INC_RING_INDEX(prTxRing->TxCpuIdx, TX_RING_SIZE);

	/* Update HW Tx DMA ring */
	spin_lock_irqsave((spinlock_t *)prTxRingLock, flags);

	prTxRing->u4UsedCnt++;
	kalDevRegWrite(prGlueInfo, prTxRing->hw_cidx_addr, prTxRing->TxCpuIdx);

	spin_unlock_irqrestore((spinlock_t *)prTxRingLock, flags);

	DBGLOG(HAL, TRACE, "Tx Data: Msdu[0x%p], Tok[%u] TokFree[%u] CPU idx[%u] Used[%u] TxDone[%u]\n",
		prMsduInfo, prToken->u4Token, halGetMsduTokenFreeCnt(prGlueInfo->prAdapter),
		SwIdx, prTxRing->u4UsedCnt, (prMsduInfo->pfTxDoneHandler ? TRUE : FALSE));

	DBGLOG_MEM32(HAL, TRACE, pucSrc, pTxD->SDLen0);

	nicTxReleaseResource_PSE(prGlueInfo->prAdapter, prMsduInfo->ucTC,
		nicTxGetPageCount(prGlueInfo->prAdapter, prMsduInfo->u2FrameLength, TRUE), TRUE);

#if HIF_TX_PREALLOC_DATA_BUFFER
	if (!prMsduInfo->pfTxDoneHandler) {
		nicTxFreePacket(prGlueInfo->prAdapter, prMsduInfo, FALSE);
		nicTxReturnMsduInfo(prGlueInfo->prAdapter, prMsduInfo);
	}
#endif

	if (wlanGetTxPendingFrameCount(prGlueInfo->prAdapter))
		kalSetEvent(prGlueInfo);

	return TRUE;
}

VOID halDumpTxRing(IN P_GLUE_INFO_T prGlueInfo, IN UINT_16 u2Port, IN UINT_32 u4Idx)
{
	P_GL_HIF_INFO_T prHifInfo = &prGlueInfo->rHifInfo;
	P_RTMP_TX_RING prTxRing;
	TXD_STRUCT *pTxD;

	prTxRing = &prHifInfo->TxRing[u2Port];

	pTxD = (TXD_STRUCT *) prTxRing->Cell[u4Idx].AllocVa;

	DBGLOG(SW4, INFO, "TX Ring[%u] Idx[%04u] SDP0[0x%08x] SDL0[%u] LS[%u] B[%u] DDONE[%u] SDP0_EXT[%u]\n",
	       u2Port, u4Idx, pTxD->SDPtr0, pTxD->SDLen0, pTxD->LastSec0,
	       pTxD->Burst, pTxD->DMADONE, pTxD->SDPtr0Ext);
}

UINT_32 halDumpHifStatus(IN P_ADAPTER_T prAdapter, IN PUINT_8 pucBuf, IN UINT_32 u4Max)
{
	P_GLUE_INFO_T prGlueInfo = prAdapter->prGlueInfo;
	P_GL_HIF_INFO_T prHifInfo = &prGlueInfo->rHifInfo;
	UINT_32 u4Idx, u4DmaIdx, u4CpuIdx, u4MaxCnt;
	UINT_32 u4Len = 0;
	P_RTMP_TX_RING prTxRing;
	P_RTMP_RX_RING prRxRing;

	LOGBUF(pucBuf, u4Max, u4Len, "\n------<Dump PCIe Status>------\n");

	for (u4Idx = 0; u4Idx < NUM_OF_TX_RING; u4Idx++) {
		prTxRing = &prHifInfo->TxRing[u4Idx];
		kalDevRegRead(prGlueInfo, prTxRing->hw_cnt_addr, &u4MaxCnt);
		kalDevRegRead(prGlueInfo, prTxRing->hw_cidx_addr, &u4CpuIdx);
		kalDevRegRead(prGlueInfo, prTxRing->hw_didx_addr, &u4DmaIdx);

		LOGBUF(pucBuf, u4Max, u4Len, "TX[%u] SZ[%04u] CPU[%04u/%04u] DMA[%04u/%04u] SW_UD[%04u] Used[%u]\n",
			u4Idx, u4MaxCnt, prTxRing->TxCpuIdx, u4CpuIdx, prTxRing->TxDmaIdx,
			u4DmaIdx, prTxRing->TxSwUsedIdx, prTxRing->u4UsedCnt);

		if (u4Idx == TX_RING_DATA0_IDX_0) {
			halDumpTxRing(prGlueInfo, u4Idx, prTxRing->TxCpuIdx);
			halDumpTxRing(prGlueInfo, u4Idx, u4CpuIdx);
			halDumpTxRing(prGlueInfo, u4Idx, u4DmaIdx);
			halDumpTxRing(prGlueInfo, u4Idx, prTxRing->TxSwUsedIdx);
		}
	}

	for (u4Idx = 0; u4Idx < NUM_OF_RX_RING; u4Idx++) {
		prRxRing = &prHifInfo->RxRing[u4Idx];

		kalDevRegRead(prGlueInfo, prRxRing->hw_cnt_addr, &u4MaxCnt);
		kalDevRegRead(prGlueInfo, prRxRing->hw_cidx_addr, &u4CpuIdx);
		kalDevRegRead(prGlueInfo, prRxRing->hw_didx_addr, &u4DmaIdx);

		LOGBUF(pucBuf, u4Max, u4Len, "RX[%u] SZ[%04u] CPU[%04u/%04u] DMA[%04u/%04u] SW_RD[%04u]\n", u4Idx,
			u4MaxCnt, prRxRing->RxCpuIdx, u4CpuIdx, prRxRing->RxDmaIdx, u4DmaIdx, prRxRing->RxSwReadIdx);
	}

	LOGBUF(pucBuf, u4Max, u4Len, "MSDU Tok: Free[%u] Used[%u]\n",
		halGetMsduTokenFreeCnt(prGlueInfo->prAdapter), prGlueInfo->rHifInfo.rTokenInfo.i4UsedCnt);
	LOGBUF(pucBuf, u4Max, u4Len, "Pending QLen Normal[%u] Sec[%u]\n",
		prGlueInfo->i4TxPendingFrameNum, prGlueInfo->i4TxPendingSecurityFrameNum);

	LOGBUF(pucBuf, u4Max, u4Len, "---------------------------------\n\n");

	return u4Len;
}


BOOLEAN halIsStaticMapBusAddr(IN UINT_32 u4Addr)
{
	if (u4Addr < MAX_PCIE_BUS_STATIC_MAP_ADDR)
		return TRUE;
	else
		return FALSE;
}

BOOLEAN halChipToStaticMapBusAddr(IN P_BUS_INFO prBusInfo, IN UINT_32 u4ChipAddr, OUT PUINT_32 pu4BusAddr)
{
	UINT_32 u4StartAddr, u4EndAddr, u4BusAddr;
	UINT_32 u4Idx = 0;

	if (halIsStaticMapBusAddr(u4ChipAddr)) {
		*pu4BusAddr = u4ChipAddr;
		return TRUE;
	}

	while (TRUE) {
		u4StartAddr = prBusInfo->bus2chip[u4Idx].u4ChipAddr;
		u4EndAddr = prBusInfo->bus2chip[u4Idx].u4ChipAddr + prBusInfo->bus2chip[u4Idx].u4Range;

		/* End of mapping table */
		if (u4EndAddr == 0x0)
			return FALSE;


		if ((u4ChipAddr >= u4StartAddr) && (u4ChipAddr <= u4EndAddr)) {
			u4BusAddr = (u4ChipAddr - u4StartAddr) + prBusInfo->bus2chip[u4Idx].u4BusAddr;
			break;
		}

		u4Idx++;
	}

	*pu4BusAddr = u4BusAddr;
	return TRUE;
}

BOOLEAN halGetDynamicMapReg(IN P_GLUE_INFO_T prGlueInfo, IN UINT_32 u4ChipAddr, OUT PUINT_32 pu4Value)
{
	P_BUS_INFO prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;
	P_GL_HIF_INFO_T prHifInfo = &prGlueInfo->rHifInfo;
	UINT_32 u4ReMapReg, u4BusAddr;
	ULONG flags;

	if (!halChipToStaticMapBusAddr(prBusInfo, MCU_CFG_PCIE_REMAP2, &u4ReMapReg))
		return FALSE;

	spin_lock_irqsave(&prHifInfo->rDynMapRegLock, flags);

	RTMP_IO_WRITE32(prHifInfo, u4ReMapReg, u4ChipAddr & PCIE_REMAP2_MASK);
	u4BusAddr = PCIE_REMAP2_BUS_ADDR + (u4ChipAddr & ~PCIE_REMAP2_MASK);
	RTMP_IO_READ32(prHifInfo, u4BusAddr, pu4Value);

	spin_unlock_irqrestore(&prHifInfo->rDynMapRegLock, flags);

	return TRUE;
}

BOOLEAN halSetDynamicMapReg(IN P_GLUE_INFO_T prGlueInfo, IN UINT_32 u4ChipAddr, IN UINT_32 u4Value)
{
	P_BUS_INFO prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;
	P_GL_HIF_INFO_T prHifInfo = &prGlueInfo->rHifInfo;
	UINT_32 u4ReMapReg, u4BusAddr;
	ULONG flags;

	if (!halChipToStaticMapBusAddr(prBusInfo, MCU_CFG_PCIE_REMAP2, &u4ReMapReg))
		return FALSE;

	spin_lock_irqsave(&prHifInfo->rDynMapRegLock, flags);

	RTMP_IO_WRITE32(prHifInfo, u4ReMapReg, u4ChipAddr & PCIE_REMAP2_MASK);
	u4BusAddr = PCIE_REMAP2_BUS_ADDR + (u4ChipAddr & ~PCIE_REMAP2_MASK);
	RTMP_IO_WRITE32(prHifInfo, u4BusAddr, u4Value);

	spin_unlock_irqrestore(&prHifInfo->rDynMapRegLock, flags);

	return TRUE;
}

BOOLEAN halIsPendingRx(IN P_ADAPTER_T prAdapter)
{
	/* TODO: check pending Rx if previous Rx handling is break due to lack of SwRfb */
	return FALSE;
}

UINT_32 halGetValidCoalescingBufSize(IN P_ADAPTER_T prAdapter)
{
	UINT_32 u4BufSize;

	if (HIF_TX_COALESCING_BUFFER_SIZE > HIF_RX_COALESCING_BUFFER_SIZE)
		u4BufSize = HIF_TX_COALESCING_BUFFER_SIZE;
	else
		u4BufSize = HIF_RX_COALESCING_BUFFER_SIZE;

	return u4BufSize;
}

WLAN_STATUS halAllocateIOBuffer(IN P_ADAPTER_T prAdapter)
{
	return WLAN_STATUS_SUCCESS;
}

WLAN_STATUS halReleaseIOBuffer(IN P_ADAPTER_T prAdapter)
{
	return WLAN_STATUS_SUCCESS;
}

VOID halProcessAbnormalInterrupt(IN P_ADAPTER_T prAdapter)
{

}

VOID halProcessSoftwareInterrupt(IN P_ADAPTER_T prAdapter)
{
	P_GLUE_INFO_T prGlueInfo;
	P_GL_HIF_INFO_T prHifInfo;
	struct ERR_RECOVERY_CTRL_T *prErrRecoveryCtrl;
	UINT_32 u4Status = 0;
	ULONG flags;

	if (prAdapter == NULL || prAdapter->prGlueInfo == NULL) {
		DBGLOG(HAL, ERROR, "prAdapter or prGlueInfo is NULL\n");
		return;
	}

	prGlueInfo = prAdapter->prGlueInfo;
	prHifInfo = &prGlueInfo->rHifInfo;
	prErrRecoveryCtrl = &prHifInfo->rErrRecoveryCtl;

	spin_lock_irqsave(&prHifInfo->rSerLock, flags);

	kalDevRegRead(prGlueInfo, MCU2HOST_SW_INT_STA, &u4Status);
	if (u4Status & ERROR_DETECT_MASK) {
		prErrRecoveryCtrl->u4Status = u4Status;
		kalDevRegWrite(prGlueInfo, MCU2HOST_SW_INT_STA, ERROR_DETECT_MASK);
		halHwRecoveryFromError(prAdapter);
	}

	spin_unlock_irqrestore(&prHifInfo->rSerLock, flags);
}

static VOID halHwRecoveryTimeout(unsigned long arg)
{
	P_GLUE_INFO_T prGlueInfo = (P_GLUE_INFO_T) arg;

	ASSERT(prGlueInfo);

	DBGLOG(HAL, ERROR, "SER timer Timeout\n");
#if CFG_CHIP_RESET_SUPPORT
	GL_RESET_TRIGGER(prAdapter, RST_FLAG_CHIP_RESET);
#endif
}

VOID halInitSerTimer(IN P_ADAPTER_T prAdapter)
{
	P_GLUE_INFO_T prGlueInfo;
	P_GL_HIF_INFO_T prHifInfo;

	prGlueInfo = prAdapter->prGlueInfo;
	prHifInfo = &prGlueInfo->rHifInfo;

	init_timer(&prHifInfo->rSerTimer);
	prHifInfo->rSerTimer.function = halHwRecoveryTimeout;
	prHifInfo->rSerTimer.data = (unsigned long)prGlueInfo;
	prHifInfo->rSerTimer.expires = jiffies + HIF_SER_TIMEOUT * HZ / MSEC_PER_SEC;
	add_timer(&prHifInfo->rSerTimer);

	DBGLOG(HAL, INFO, "Start SER timer\n");
}

VOID halHwRecoveryFromError(IN P_ADAPTER_T prAdapter)
{
	P_GLUE_INFO_T prGlueInfo;
	P_GL_HIF_INFO_T prHifInfo;
	P_BUS_INFO prBusInfo = NULL;
	struct ERR_RECOVERY_CTRL_T *prErrRecoveryCtrl;
	UINT_32 u4Status = 0;

	prGlueInfo = prAdapter->prGlueInfo;
	prHifInfo = &prGlueInfo->rHifInfo;
	prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;
	prErrRecoveryCtrl = &prHifInfo->rErrRecoveryCtl;

	u4Status = prErrRecoveryCtrl->u4Status;
	prErrRecoveryCtrl->u4Status = 0;

	switch (prErrRecoveryCtrl->eErrRecovState) {
	case ERR_RECOV_STOP_IDLE:
	case ERR_RECOV_EVENT_REENTRY:
		if (u4Status & ERROR_DETECT_STOP_PDMA) {
			if (!prHifInfo->fgIsErrRecovery) {
				prHifInfo->fgIsErrRecovery = TRUE;
				halInitSerTimer(prAdapter);
			}

			DBGLOG(HAL, INFO, "SER(E) Host stop PDMA tx/rx ring operation\n");
			nicSerStopTxRx(prAdapter);

			DBGLOG(HAL, INFO, "SER(F) Host ACK PDMA tx/rx ring stop operation\n");
			kalDevRegWrite(prGlueInfo, HOST2MCU_SW_INT_SET, MCU_INT_PDMA0_STOP_DONE);

			/* re-call for change status to stop dma0 */
			prErrRecoveryCtrl->eErrRecovState = ERR_RECOV_STOP_IDLE_DONE;
			halHwRecoveryFromError(prAdapter);
		} else {
			DBGLOG(HAL, ERROR, "SER CurStat=%u Event=%x\n",
			       prErrRecoveryCtrl->eErrRecovState, u4Status);
		}
		break;

	case ERR_RECOV_STOP_PDMA0:
		if (u4Status & ERROR_DETECT_RESET_DONE) {
			DBGLOG(HAL, INFO, "SER(L) Host re-initialize PDMA\n");
			halWpdmaInitTxRing(prGlueInfo);
			halWpdmaInitRxRing(prGlueInfo);

			DBGLOG(HAL, INFO, "SER(M) Host enable PDMA\n");
			prBusInfo->pdmaSetup(prGlueInfo, TRUE);
			halWpdmaWaitIdle(prGlueInfo, 100, 1000);
			kalDevRegWrite(prGlueInfo, WPDMA_RST_PTR, 0xFFFFFFFF);
			kalDevRegWrite(prGlueInfo, WPDMA_PAUSE_TX_Q, 0);

			DBGLOG(HAL, INFO, "SER(N) Host interrupt N9 PDMA ring init done\n");
			prErrRecoveryCtrl->eErrRecovState = ERR_RECOV_RESET_PDMA0;
			kalDevRegWrite(prGlueInfo, HOST2MCU_SW_INT_SET, MCU_INT_PDMA0_INIT_DONE);
		} else {
			DBGLOG(HAL, ERROR, "SER CurStat=%u Event=%x\n",
			       prErrRecoveryCtrl->eErrRecovState, u4Status);
		}
		break;

	case ERR_RECOV_RESET_PDMA0:
		if (u4Status & ERROR_DETECT_RECOVERY_DONE) {
			DBGLOG(HAL, INFO, "SER(Q) Host interrupt N9 SER handle done\n");
			prErrRecoveryCtrl->eErrRecovState = ERR_RECOV_WAIT_N9_NORMAL;
			kalDevRegWrite(prGlueInfo, HOST2MCU_SW_INT_SET, MCU_INT_PDMA0_RECOVERY_DONE);
		} else {
			DBGLOG(HAL, ERROR, "SER CurStat=%u Event=%x\n",
			       prErrRecoveryCtrl->eErrRecovState, u4Status);
		}
		break;

	case ERR_RECOV_STOP_IDLE_DONE:
		prErrRecoveryCtrl->eErrRecovState = ERR_RECOV_STOP_PDMA0;
		break;

	case ERR_RECOV_WAIT_N9_NORMAL:
		if (u4Status & ERROR_DETECT_N9_NORMAL_STATE) {
			del_timer(&prHifInfo->rSerTimer);

			/* update Beacon frame if operating in AP mode. */
			DBGLOG(HAL, INFO, "SER(T) Host re-initialize BCN\n");
			nicSerReInitBeaconFrame(prAdapter);

			kalDevKickCmd(prAdapter->prGlueInfo);
			kalDevKickData(prAdapter->prGlueInfo);
			prHifInfo->fgIsErrRecovery = FALSE;
			nicSerStartTxRx(prAdapter);
			prErrRecoveryCtrl->eErrRecovState = ERR_RECOV_STOP_IDLE;
		} else if (u4Status & ERROR_DETECT_STOP_PDMA) {
			DBGLOG(HAL, ERROR, "SER re-entry CurStat=%u Event=%x\n",
			       prErrRecoveryCtrl->eErrRecovState, u4Status);
			prErrRecoveryCtrl->eErrRecovState = ERR_RECOV_EVENT_REENTRY;
			halHwRecoveryFromError(prAdapter);
		} else {
			DBGLOG(HAL, ERROR, "SER CurStat=%u Event=%x\n",
			       prErrRecoveryCtrl->eErrRecovState, u4Status);
		}
		break;

	default:
		DBGLOG(HAL, ERROR, "SER CurStat=%u Event=%x!!!\n",
		       prErrRecoveryCtrl->eErrRecovState, u4Status);
		break;
	}
}

VOID halDeAggRxPktWorker(struct work_struct *work)
{

}

VOID halRxTasklet(unsigned long data)
{

}

VOID halTxCompleteTasklet(unsigned long data)
{

}

/* Hif power off wifi */
WLAN_STATUS halHifPowerOffWifi(IN P_ADAPTER_T prAdapter)
{
	WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;

	DBGLOG(INIT, INFO, "Power off Wi-Fi!\n");

	nicDisableInterrupt(prAdapter);
	ACQUIRE_POWER_CONTROL_FROM_PM(prAdapter);

	/* Power off Wi-Fi */
	wlanSendNicPowerCtrlCmd(prAdapter, TRUE);

	wlanClearPendingInterrupt(prAdapter);

	/* prAdapter->fgWiFiInSleepyState = TRUE; */
	RECLAIM_POWER_CONTROL_TO_PM(prAdapter, FALSE);

	rStatus = wlanCheckWifiFunc(prAdapter, FALSE);

	return rStatus;
}

VOID halPrintHifDbgInfo(IN P_ADAPTER_T prAdapter)
{

}

BOOLEAN halIsTxResourceControlEn(IN P_ADAPTER_T prAdapter)
{
	return FALSE;
}

VOID halTxResourceResetHwTQCounter(IN P_ADAPTER_T prAdapter)
{
}

UINT_32 halGetHifTxPageSize(IN P_ADAPTER_T prAdapter)
{
	return HIF_TX_PAGE_SIZE;
}
