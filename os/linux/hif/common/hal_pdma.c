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
 *[File]             hif_pdma.c
 *[Version]          v1.0
 *[Revision Date]    2015-09-08
 *[Author]
 *[Description]
 *    The program provides PDMA HIF APIs
 *[Copyright]
 *    Copyright (C) 2015 MediaTek Incorporation. All Rights Reserved.
 ******************************************************************************/

/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */

/*******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 *******************************************************************************
 */
#include "precomp.h"

#include "hif_pdma.h"

#include <linux/mm.h>
#ifndef CONFIG_X86
#include <asm/memory.h>
#endif

#include "mt66xx_reg.h"
#include "gl_kal.h"
#include "host_csr.h"

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */

/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */

/*******************************************************************************
 *                            P U B L I C   D A T A
 *******************************************************************************
 */

/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */

/*******************************************************************************
 *                                 M A C R O S
 *******************************************************************************
 */

/*******************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */
static void halDumpMsduReportStats(IN struct ADAPTER *prAdapter);

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */
u_int8_t halIsDataRing(enum ENUM_WFDMA_RING_TYPE eType, uint32_t u4Idx)
{
	if (eType == TX_RING) {
		return (u4Idx == TX_RING_DATA0 ||
			u4Idx == TX_RING_DATA1 ||
			u4Idx == TX_RING_DATA_PRIO ||
			u4Idx == TX_RING_DATA_ALTX);
	} else if (eType == RX_RING) {
		return (u4Idx == RX_RING_DATA0 ||
			u4Idx == RX_RING_DATA1 ||
			u4Idx == RX_RING_DATA2);
	}
	return FALSE;
}

uint8_t halRingDataSelectByWmmIndex(
	IN struct ADAPTER *prAdapter,
	IN uint8_t ucWmmIndex)
{
	struct BUS_INFO *bus_info;
	uint16_t u2Port = TX_RING_DATA0;

	bus_info = prAdapter->chip_info->bus_info;
	if (bus_info->tx_ring0_data_idx != bus_info->tx_ring1_data_idx) {
		u2Port = (ucWmmIndex % 2) ?
			TX_RING_DATA1 : TX_RING_DATA0;
	}
	return u2Port;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief Decide TxRingData number by MsduInfo
 *
 * @param prGlueInfo
 *
 * @param prMsduInfo
 *
 * @return TxRingData number
 */
/*----------------------------------------------------------------------------*/
uint8_t halTxRingDataSelect(IN struct ADAPTER *prAdapter,
	IN struct MSDU_INFO *prMsduInfo)
{
	struct BUS_INFO *bus_info;

	ASSERT(prAdapter);

	bus_info = prAdapter->chip_info->bus_info;

	if ((prMsduInfo->fgMgmtUseDataQ) &&
	    (prMsduInfo->ucControlFlag & MSDU_CONTROL_FLAG_FORCE_TX))
		return TX_RING_DATA_ALTX;

	if (bus_info->tx_ring2_data_idx &&
			nicTxIsPrioPackets(prAdapter, prMsduInfo))
		return TX_RING_DATA_PRIO;

	return halRingDataSelectByWmmIndex(prAdapter, prMsduInfo->ucWmmQueSet);
}


/*----------------------------------------------------------------------------*/
/*!
 * @brief check is timeout or not
 *
 * @param u4StartTime start time
 *
 * @param u4Timeout timeout value
 *
 * @return is timeout
 */
/*----------------------------------------------------------------------------*/
static inline bool halIsTimeout(uint32_t u4StartTime, uint32_t u4Timeout)
{
	uint32_t u4CurTime = kalGetTimeTick();
	uint32_t u4Time = 0;

	if (u4CurTime >= u4StartTime)
		u4Time = u4CurTime - u4StartTime;
	else
		u4Time = u4CurTime + (0xFFFFFFFF - u4StartTime);

	return u4Time > u4Timeout;
}

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
u_int8_t halVerifyChipID(IN struct ADAPTER *prAdapter)
{
	struct mt66xx_chip_info *prChipInfo;
	struct BUS_INFO *prBusInfo;
	uint32_t u4CIR = 0;

	ASSERT(prAdapter);

	prChipInfo = prAdapter->chip_info;
	prBusInfo = prChipInfo->bus_info;

	if (prAdapter->fgIsReadRevID || !prChipInfo->should_verify_chip_id)
		return TRUE;

	HAL_MCR_RD(prAdapter, prBusInfo->top_cfg_base + TOP_HW_CONTROL, &u4CIR);

	DBGLOG(INIT, INFO, "WCIR_CHIP_ID = 0x%x, chip_id = 0x%x\n",
	       (uint32_t)(u4CIR & WCIR_CHIP_ID), prChipInfo->chip_id);

	if ((u4CIR & WCIR_CHIP_ID) != prChipInfo->chip_id)
		return FALSE;

	HAL_MCR_RD(prAdapter, prBusInfo->top_cfg_base + TOP_HW_VERSION, &u4CIR);

	prAdapter->ucRevID = (uint8_t)(u4CIR & 0xF);
	prAdapter->fgIsReadRevID = TRUE;

	return TRUE;
}

uint32_t halRxWaitResponse(IN struct ADAPTER *prAdapter, IN uint8_t ucPortIdx,
	OUT uint8_t *pucRspBuffer, IN uint32_t u4MaxRespBufferLen,
	OUT uint32_t *pu4Length, IN uint32_t u4WaitingInterval,
	IN uint32_t u4TimeoutValue)
{
	struct GLUE_INFO *prGlueInfo;
	uint32_t u4PktLen = 0, u4Time;
	u_int8_t fgStatus;
	struct mt66xx_chip_info *prChipInfo;
	u_int8_t	ucNewPort;
#if CFG_SUPPORT_HOST_RX_WM_EVENT_FROM_PSE
	struct BUS_INFO *prBusInfo;
#endif

	DEBUGFUNC("nicRxWaitResponse");

	ASSERT(prAdapter);
	prGlueInfo = prAdapter->prGlueInfo;
	prChipInfo = prAdapter->chip_info;
	ASSERT(prGlueInfo);
	ASSERT(pucRspBuffer);

	ASSERT(ucPortIdx < 2);
	ucPortIdx = HIF_IMG_DL_STATUS_PORT_IDX;

	u4Time = kalGetTimeTick();
	u4PktLen = u4MaxRespBufferLen;

#if CFG_SUPPORT_HOST_RX_WM_EVENT_FROM_PSE
	prBusInfo = prAdapter->chip_info->bus_info;
	if (prBusInfo->checkPortForRxEventFromPse != NULL) {
		ucNewPort = prBusInfo->checkPortForRxEventFromPse(prAdapter,
			ucPortIdx);
	} else {
		ucNewPort = ucPortIdx;
	}
#else
	ucNewPort = ucPortIdx;
#endif

	do {
		if (wlanIsChipNoAck(prAdapter)) {
			DBGLOG(HAL, ERROR, "Chip No Ack\n");
			return WLAN_STATUS_FAILURE;
		}

		fgStatus = kalDevPortRead(
			prGlueInfo, ucNewPort, u4PktLen,
			pucRspBuffer, HIF_RX_COALESCING_BUFFER_SIZE);

		if (fgStatus) {
			*pu4Length = u4PktLen;
			break;
		}

		if (halIsTimeout(u4Time, u4TimeoutValue)) {
#if IS_ENABLED(CFG_SUPPORT_CONNAC1X)
			uint32_t u4Value = 0;

			kalDevRegRead(prGlueInfo, CONN_HIF_ON_DBGCR01,
				      &u4Value);
			DBGLOG(HAL, ERROR, "CONN_HIF_ON_DBGCR01[0x%x]\n",
			       u4Value);
#endif
			return WLAN_STATUS_FAILURE;
		}

		/* Response packet is not ready */
		/* use sleep waiting instead of busy waiting */
		kalUsleep(u4WaitingInterval);
	} while (TRUE);

	return WLAN_STATUS_SUCCESS;
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
void halEnableInterrupt(IN struct ADAPTER *prAdapter)
{
	struct BUS_INFO *prBusInfo = NULL;

	ASSERT(prAdapter);

	prBusInfo = prAdapter->chip_info->bus_info;

	GLUE_SET_REF_CNT(1, prAdapter->fgIsIntEnable);

	if (prBusInfo->enableInterrupt)
		prBusInfo->enableInterrupt(prAdapter);
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief disable global interrupt
 *
 * @param prAdapter pointer to the Adapter handler
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
void halDisableInterrupt(IN struct ADAPTER *prAdapter)
{
	struct BUS_INFO *prBusInfo;

	ASSERT(prAdapter);

	prBusInfo = prAdapter->chip_info->bus_info;

	if (prBusInfo->disableInterrupt)
		prBusInfo->disableInterrupt(prAdapter);

	GLUE_SET_REF_CNT(0, prAdapter->fgIsIntEnable);
}

static u_int8_t halDriverOwnCheckCR4(struct ADAPTER *prAdapter)
{
	struct mt66xx_chip_info *prChipInfo;
	uint32_t u4CurrTick;
	uint32_t ready_bits;
	u_int8_t fgStatus = TRUE;
	u_int8_t fgReady = FALSE;
	u_int8_t fgDummyReq = FALSE;
	bool fgTimeout;

	ASSERT(prAdapter);

	prChipInfo = prAdapter->chip_info;
	ready_bits = prChipInfo->sw_ready_bits;

	HAL_WIFI_FUNC_READY_CHECK(prAdapter,
				  WIFI_FUNC_DUMMY_REQ, &fgDummyReq);

	u4CurrTick = kalGetTimeTick();
	/* Wait CR4 ready */
	while (1) {
		fgTimeout = halIsTimeout(u4CurrTick,
					 LP_OWN_BACK_TOTAL_DELAY_MS);
		HAL_WIFI_FUNC_READY_CHECK(prAdapter, ready_bits, &fgReady);

		if (fgReady) {
			break;
		} else if (kalIsCardRemoved(prAdapter->prGlueInfo) ||
			   fgIsBusAccessFailed || fgTimeout
			   || wlanIsChipNoAck(prAdapter)) {
			DBGLOG(INIT, INFO,
			       "Skip waiting CR4 ready for next %ums\n",
			       LP_OWN_BACK_FAILED_LOG_SKIP_MS);
			fgStatus = FALSE;
			GL_DEFAULT_RESET_TRIGGER(prAdapter, RST_DRV_OWN_FAIL);
			break;
		}
		/* Delay for CR4 to complete its operation. */
		kalUsleep_range(LP_OWN_BACK_LOOP_DELAY_MIN_US,
				LP_OWN_BACK_LOOP_DELAY_MAX_US);
	}

	/* Send dummy cmd and clear flag */
	if (fgDummyReq) {
		wlanSendDummyCmd(prAdapter, FALSE);
		HAL_CLEAR_DUMMY_REQ(prAdapter);
	}

	return fgStatus;
}

static void halDriverOwnTimeout(struct ADAPTER *prAdapter,
				uint32_t u4CurrTick, u_int8_t fgTimeout)
{
	struct mt66xx_chip_info *prChipInfo;
	struct CHIP_DBG_OPS *prChipDbgOps;
	uint32_t u4DrvOwnTimeoutMs = LP_OWN_BACK_FAILED_LOG_SKIP_MS;

	if (prAdapter->u4CasanLoadType == 1)
		u4DrvOwnTimeoutMs = LP_OWN_BACK_FAILED_LOG_SKIP_CASAN_MS;

	DBGLOG(INIT, INFO,
		   "Driver own timeout %u ms\n",
		   u4DrvOwnTimeoutMs);

	prChipInfo = prAdapter->chip_info;
	prChipDbgOps = prChipInfo->prDebugOps;

	if ((prAdapter->u4OwnFailedCount == 0) ||
	    CHECK_FOR_TIMEOUT(u4CurrTick, prAdapter->rLastOwnFailedLogTime,
			      MSEC_TO_SYSTIME(u4DrvOwnTimeoutMs))
		) {
		DBGLOG(INIT, ERROR,
		       "LP cannot be own back, Timeout[%u](%ums), BusAccessError[%u]",
		       fgTimeout,
		       kalGetTimeTick() - u4CurrTick,
		       fgIsBusAccessFailed);
		DBGLOG(INIT, ERROR,
		       "Resetting[%u], CardRemoved[%u] NoAck[%u] Cnt[%u]\n",
		       kalIsResetting(),
		       kalIsCardRemoved(prAdapter->prGlueInfo),
		       wlanIsChipNoAck(prAdapter),
		       prAdapter->u4OwnFailedCount);
		DBGLOG(INIT, INFO,
		       "Skip LP own back failed log for next %ums\n",
		       u4DrvOwnTimeoutMs);
		if (prChipDbgOps->dumpwfsyscpupcr)
			prChipDbgOps->dumpwfsyscpupcr(prAdapter);

		prAdapter->u4OwnFailedLogCount++;
		if (prAdapter->u4OwnFailedLogCount >
		    LP_OWN_BACK_FAILED_RESET_CNT) {
			if (IS_MOBILE_SEGMENT && in_interrupt()) {
				DBGLOG(INIT, INFO,
					"Skip reset in tasklet\n");
			} else {
				if (prChipDbgOps->showCsrInfo)
					prChipDbgOps->showCsrInfo(
							prAdapter);
				if (prChipDbgOps->dumpBusHangCr)
					prChipDbgOps->dumpBusHangCr(
							prAdapter);
				GL_DEFAULT_RESET_TRIGGER(prAdapter,
						RST_DRV_OWN_FAIL);
			}
		}
		GET_CURRENT_SYSTIME(&prAdapter->rLastOwnFailedLogTime);
	}

	prAdapter->u4OwnFailedCount++;
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
u_int8_t halSetDriverOwn(IN struct ADAPTER *prAdapter)
{
	struct mt66xx_chip_info *prChipInfo;
	struct BUS_INFO *prBusInfo;
	struct GL_HIF_INFO *prHifInfo;
	struct WIFI_VAR *prWifiVar;
	u_int8_t fgStatus = TRUE;
	uint32_t i, u4CurrTick;
	u_int8_t fgTimeout;
	u_int8_t fgResult;
	u_int8_t fgIsDriverOwnTimeout = FALSE;

	KAL_TIME_INTERVAL_DECLARATION();

	ASSERT(prAdapter);

	prChipInfo = prAdapter->chip_info;
	prBusInfo = prChipInfo->bus_info;
	prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	prWifiVar = &prAdapter->rWifiVar;

	KAL_HIF_OWN_LOCK(prAdapter);

	GLUE_INC_REF_CNT(prAdapter->u4PwrCtrlBlockCnt);

	if (prAdapter->fgIsFwOwn == FALSE)
		goto end;

	DBGLOG(INIT, TRACE, "DRIVER OWN Start\n");
	KAL_REC_TIME_START();

	u4CurrTick = kalGetTimeTick();
	i = 0;

	/* PCIE/AXI need to do clear own, then could start polling status */
	HAL_LP_OWN_CLR(prAdapter, &fgResult);
	fgResult = FALSE;

	while (1) {
		/* Delay for LP engine to complete its operation. */
#if CFG_SUPPORT_RX_WORK
		kalUsleep_range(LP_OWN_BACK_LOOP_DELAY_MIN_US,
				LP_OWN_BACK_LOOP_DELAY_MAX_US);
#else /* !CFG_SUPPORT_RX_WORK */
		kalUdelay(LP_OWN_BACK_LOOP_DELAY_MAX_US);
#endif /* !CFG_SUPPORT_RX_WORK */

		if (!prBusInfo->fgCheckDriverOwnInt ||
		    test_bit(GLUE_FLAG_INT_BIT, &prAdapter->prGlueInfo->ulFlag))
			HAL_LP_OWN_RD(prAdapter, &fgResult);

		fgTimeout = ((kalGetTimeTick() - u4CurrTick) >
			     LP_OWN_BACK_TOTAL_DELAY_MS) ? TRUE : FALSE;

		if (fgResult) {
			/* Check WPDMA FW own interrupt status and clear */
			if (prBusInfo->fgCheckDriverOwnInt)
				HAL_MCR_WR(prAdapter,
					prBusInfo->fw_own_clear_addr,
					prBusInfo->fw_own_clear_bit);
			prAdapter->fgIsFwOwn = FALSE;
			prAdapter->u4OwnFailedCount = 0;
			prAdapter->u4OwnFailedLogCount = 0;
			break;
		} else if (wlanIsChipNoAck(prAdapter)) {
			DBGLOG(INIT, INFO,
			"Driver own return due to chip reset and chip no response.\n");
#if (CFG_SUPPORT_DEBUG_SOP == 1)
			prChipInfo->prDebugOps->show_debug_sop_info(prAdapter,
			  SLAVENORESP);
#endif
			fgStatus = FALSE;
			break;
		} else if ((i > LP_OWN_BACK_FAILED_RETRY_CNT) &&
			   (kalIsCardRemoved(prAdapter->prGlueInfo) ||
			    fgIsBusAccessFailed || fgTimeout)) {
			fgIsDriverOwnTimeout = TRUE;
			fgStatus = FALSE;
			break;
		}

		i++;
	}

	if (fgIsDriverOwnTimeout) {
#if !CFG_SUPPORT_RX_WORK
		if (HAL_IS_TX_DIRECT(prAdapter) || HAL_IS_RX_DIRECT(prAdapter))
			goto end;
		else
#endif /* !CFG_SUPPORT_RX_WORK */
			halDriverOwnTimeout(prAdapter, u4CurrTick, fgTimeout);
	}

#if !CFG_CONTROL_ASPM_BY_FW
#if CFG_SUPPORT_PCIE_ASPM
	glBusConfigASPM(prHifInfo->pdev, DISABLE_ASPM_L1);
#endif
#endif

	/* For Low power Test */
	/* 1. Driver need to polling until CR4 ready,
	 *    then could do normal Tx/Rx
	 * 2. After CR4 ready, send a dummy command to change data path
	 *    to store-forward mode
	 */
	if (prAdapter->fgIsFwDownloaded && prChipInfo->is_support_cr4)
		fgStatus &= halDriverOwnCheckCR4(prAdapter);

	if (fgStatus) {
		if (prAdapter->fgIsFwDownloaded) {
			/*WFDMA re-init flow after chip deep sleep*/
			if (prChipInfo->asicWfdmaReInit)
				prChipInfo->asicWfdmaReInit(prAdapter);
		}
		/* Check consys enter sleep mode DummyReg(0x0F) */
		if (prBusInfo->checkDummyReg)
			prBusInfo->checkDummyReg(prAdapter->prGlueInfo);
	}

	KAL_REC_TIME_END();
	DBGLOG(INIT, INFO,
		"DRIVER OWN Done[%lu us]\n", KAL_GET_TIME_INTERVAL());

end:
	KAL_HIF_OWN_UNLOCK(prAdapter);

#if !CFG_SUPPORT_RX_WORK
	if (fgIsDriverOwnTimeout) {
		if (HAL_IS_TX_DIRECT(prAdapter) ||
				HAL_IS_RX_DIRECT(prAdapter))
			halDriverOwnTimeout(prAdapter,
					u4CurrTick, fgTimeout);
	}
#endif /* !CFG_SUPPORT_RX_WORK */

#if (CFG_SUPPORT_HOST_OFFLOAD == 1)
	if (IS_FEATURE_ENABLED(prWifiVar->fgEnableMawd))
		halMawdWakeup(prAdapter->prGlueInfo);
#endif /* CFG_SUPPORT_HOST_OFFLOAD == 1 */

	return fgStatus;
}

static uint32_t halGetWfdmaRxCnt(struct ADAPTER *prAdapter)
{
	uint32_t u4RxCnt = 0, u4Idx;

	for (u4Idx = 0; u4Idx < NUM_OF_RX_RING; u4Idx++) {
		u4RxCnt += halWpdmaGetRxDmaDoneCnt(
			prAdapter->prGlueInfo, u4Idx);
	}

	return u4RxCnt;
}

#if CFG_SUPPORT_DISABLE_DATA_DDONE_INTR
void halDataDmaDoneManualUpdate(struct ADAPTER *prAdapter)
{
	uint32_t u4Idx;

	for (u4Idx = 0; u4Idx < NUM_OF_TX_RING; u4Idx++) {
		if (!halIsDataRing(TX_RING, u4Idx))
			continue;

		halWpdmaProcessDataDmaDone(
				prAdapter->prGlueInfo, u4Idx);
	}
}
#endif /* CFG_SUPPORT_DISABLE_DATA_DDONE_INTR */

/*----------------------------------------------------------------------------*/
/*!
 * \brief This routine is used to process the POWER ON procedure.
 *
 * \param[in] pvAdapter Pointer to the Adapter structure.
 *
 * \return (none)
 */
/*----------------------------------------------------------------------------*/
void halSetFWOwn(IN struct ADAPTER *prAdapter, IN u_int8_t fgEnableGlobalInt)
{
	struct mt66xx_chip_info *prChipInfo;
	struct BUS_INFO *prBusInfo;
	struct GL_HIF_INFO *prHifInfo;
	struct WIFI_VAR *prWifiVar;
	u_int8_t fgResult;

	ASSERT(prAdapter);

	prChipInfo = prAdapter->chip_info;
	prBusInfo = prAdapter->chip_info->bus_info;
	prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	prWifiVar = &prAdapter->rWifiVar;

	KAL_HIF_OWN_LOCK(prAdapter);

	/* Decrease Block to Enter Low Power Semaphore count */
	GLUE_DEC_REF_CNT(prAdapter->u4PwrCtrlBlockCnt);

#if CFG_CE_ASSERT_DUMP
	/* During core dump, can't issue fw own, will result
	 * driver own fail (MCU can't process it).
	 */
	if (prAdapter->fgN9AssertDumpOngoing == TRUE)
		goto unlock;
#endif

	if (p2pFuncNeedForceSleep(prAdapter))
		DBGLOG(INIT, LOUD, "SAP: Skip fgWiFiInSleepyState check\n");
	else if (!prAdapter->fgWiFiInSleepyState)
		goto unlock;

	if (GLUE_GET_REF_CNT(prAdapter->u4PwrCtrlBlockCnt) != 0)
		goto unlock;

	if (prAdapter->fgIsFwOwn == TRUE)
		goto unlock;

	if (prHifInfo->fgIsPowerOn && halGetWfdmaRxCnt(prAdapter)) {
		DBGLOG_LIMITED(INIT, STATE, "Skip FW OWN due to pending INT\n");
		/* pending interrupts */
		goto unlock;
	}

	if (fgEnableGlobalInt) {
		prAdapter->fgIsIntEnableWithLPOwnSet = TRUE;
	} else {
		/* Write sleep mode magic num to dummy reg */
		if (prBusInfo->setDummyReg)
			prBusInfo->setDummyReg(prAdapter->prGlueInfo);

#if CFG_SUPPORT_DISABLE_DATA_DDONE_INTR
		/*
		 * fw may reset data dma done counter when fw own
		 * so we need manually update it before it
		 */
		halDataDmaDoneManualUpdate(prAdapter);
#endif /* CFG_SUPPORT_DISABLE_DATA_DDONE_INTR */

#if !CFG_CONTROL_ASPM_BY_FW
#if CFG_SUPPORT_PCIE_ASPM
		glBusConfigASPML1SS(prHifInfo->pdev,
			PCI_L1PM_CTR1_ASPM_L12_EN |
			PCI_L1PM_CTR1_ASPM_L11_EN);
		glBusConfigASPM(prHifInfo->pdev,
			ENABLE_ASPM_L1);
#endif
#endif

#if (CFG_SUPPORT_HOST_OFFLOAD == 1)
	if (IS_FEATURE_ENABLED(prWifiVar->fgEnableMawd))
		halMawdSleep(prAdapter->prGlueInfo);
#endif /* CFG_SUPPORT_HOST_OFFLOAD == 1 */

		HAL_LP_OWN_SET(prAdapter, &fgResult);

		prAdapter->fgIsFwOwn = TRUE;
		prHifInfo->fgIsBackupIntSta = false;

		DBGLOG(INIT, INFO, "FW OWN:%u, IntSta:0x%08x\n",
		       fgResult, prHifInfo->u4WakeupIntSta);
	}

unlock:
	KAL_HIF_OWN_UNLOCK(prAdapter);
}

void halWakeUpWiFi(IN struct ADAPTER *prAdapter)
{
	struct BUS_INFO *prBusInfo;

	ASSERT(prAdapter);

	prBusInfo = prAdapter->chip_info->bus_info;
	if (prBusInfo->wakeUpWiFi)
		prBusInfo->wakeUpWiFi(prAdapter);
}

void halTxCancelSendingCmd(IN struct ADAPTER *prAdapter,
	IN struct CMD_INFO *prCmdInfo)
{
}

u_int8_t halTxIsCmdBufEnough(IN struct ADAPTER *prAdapter)
{
	struct mt66xx_chip_info *prChipInfo;
	struct GL_HIF_INFO *prHifInfo;
	struct BUS_INFO *prBusInfo;
	struct SW_WFDMA_INFO *prSwWfdmaInfo;
	struct RTMP_TX_RING *prTxRing;
	uint16_t u2Port = TX_RING_CMD;

	prChipInfo = prAdapter->chip_info;
	prBusInfo = prChipInfo->bus_info;
	prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	prSwWfdmaInfo = &prBusInfo->rSwWfdmaInfo;

	if (prSwWfdmaInfo->fgIsEnSwWfdma)
		return TRUE;

#if (CFG_SUPPORT_CONNAC2X == 1)
	if (prChipInfo->is_support_wacpu)
		u2Port = TX_RING_WA_CMD;
#endif /* CFG_SUPPORT_CONNAC2X == 1 */

	/* Port idx sanity */
	if (u2Port >= NUM_OF_TX_RING) {
		DBGLOG(HAL, ERROR, "Invalid Port[%u]\n", u2Port);
		return FALSE;
	}

	prTxRing = &prHifInfo->TxRing[u2Port];

	if (prTxRing->u4UsedCnt + 1 < prTxRing->u4RingSize)
		return TRUE;

	halWpdmaProcessCmdDmaDone(prAdapter->prGlueInfo, u2Port);
	DBGLOG(HAL, INFO, "Force recycle port %d DMA resource UsedCnt[%d].\n",
	       u2Port, prTxRing->u4UsedCnt);

	if (prTxRing->u4UsedCnt + 1 < prTxRing->u4RingSize)
		return TRUE;

	return FALSE;
}

u_int8_t halTxIsDataBufEnough(IN struct ADAPTER *prAdapter,
	IN struct MSDU_INFO *prMsduInfo)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct RTMP_TX_RING *prTxRing;
	uint16_t u2Port;
#if CFG_SUPPORT_DISABLE_DATA_DDONE_INTR
	uint8_t ucTryCnt = 2; /* May need to try one more time */
#else
	uint8_t ucTryCnt = 1;
#endif /* CFG_SUPPORT_DISABLE_DATA_DDONE_INTR */

	u2Port = halTxRingDataSelect(prAdapter, prMsduInfo);
	prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	prTxRing = &prHifInfo->TxRing[u2Port];

	while (TRUE) {
		if ((prHifInfo->u4TxDataQLen[u2Port] <
		     halGetMsduTokenFreeCnt(prAdapter)) &&
		    (prTxRing->u4UsedCnt + prHifInfo->u4TxDataQLen[u2Port] + 1
		     < prTxRing->u4RingSize))
			return TRUE;

		ucTryCnt--;
		if (!ucTryCnt)
			break;

		/* Try to update from HAL */
		halWpdmaProcessDataDmaDone(
				prAdapter->prGlueInfo, u2Port);
	}

	DBGLOG(HAL, TRACE,
		"Low Tx Data Resource Tok[%u] Ring%d[%u] List[%u]\n",
		halGetMsduTokenFreeCnt(prAdapter),
		u2Port,
		(prTxRing->u4RingSize - prTxRing->u4UsedCnt),
		prHifInfo->u4TxDataQLen[u2Port]);
	kalTraceEvent("Low T[%u]Ring%d[%u]L[%u] id=0x%04x sn=%d",
		halGetMsduTokenFreeCnt(prAdapter),
		u2Port,
		(prTxRing->u4RingSize - prTxRing->u4UsedCnt),
		prHifInfo->u4TxDataQLen[u2Port],
		GLUE_GET_PKT_IP_ID(prMsduInfo->prPacket),
		GLUE_GET_PKT_SEQ_NO(prMsduInfo->prPacket));
	return FALSE;
}

static void halDefaultProcessTxInterrupt(IN struct ADAPTER *prAdapter)
{
	struct BUS_INFO *prBusInfo = prAdapter->chip_info->bus_info;
	struct GL_HIF_INFO *prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	union WPDMA_INT_STA_STRUCT rIntrStatus;
	bool fgIsSetHifTxEvent = false;

	rIntrStatus = (union WPDMA_INT_STA_STRUCT)prHifInfo->u4IntStatus;

	if (rIntrStatus.field.tx_done & BIT(prBusInfo->tx_ring_fwdl_idx))
		halWpdmaProcessCmdDmaDone(prAdapter->prGlueInfo,
			TX_RING_FWDL);

	if (rIntrStatus.field.tx_done & BIT(prBusInfo->tx_ring_cmd_idx))
		halWpdmaProcessCmdDmaDone(prAdapter->prGlueInfo,
			TX_RING_CMD);

	if (rIntrStatus.field.tx_done & BIT(prBusInfo->tx_ring0_data_idx)) {
		halWpdmaProcessDataDmaDone(prAdapter->prGlueInfo,
			TX_RING_DATA0);
		fgIsSetHifTxEvent = true;
	}

	if (prBusInfo->tx_ring0_data_idx != prBusInfo->tx_ring1_data_idx &&
		rIntrStatus.field.tx_done & BIT(prBusInfo->tx_ring1_data_idx)) {
		halWpdmaProcessDataDmaDone(prAdapter->prGlueInfo,
			TX_RING_DATA1);
		fgIsSetHifTxEvent = true;
	}

	if (prBusInfo->tx_ring2_data_idx &&
		rIntrStatus.field.tx_done & BIT(prBusInfo->tx_ring2_data_idx)) {
		halWpdmaProcessDataDmaDone(prAdapter->prGlueInfo,
			TX_RING_DATA_PRIO);
		fgIsSetHifTxEvent = true;
	}

#if CFG_SUPPORT_MULTITHREAD
	if (fgIsSetHifTxEvent)
		kalSetTxEvent2Hif(prAdapter->prGlueInfo);
#endif
}


void halProcessTxInterrupt(IN struct ADAPTER *prAdapter)
{
	struct BUS_INFO *prBusInfo = prAdapter->chip_info->bus_info;

	if (prBusInfo->processTxInterrupt)
		prBusInfo->processTxInterrupt(prAdapter);
	else
		halDefaultProcessTxInterrupt(prAdapter);
}


void halInitMsduTokenInfo(IN struct ADAPTER *prAdapter)
{
	struct GL_HIF_INFO *prHifInfo;
	struct HIF_MEM_OPS *prMemOps;
	struct MSDU_TOKEN_INFO *prTokenInfo;
	struct MSDU_TOKEN_ENTRY *prToken;
	struct mt66xx_chip_info *prChipInfo;
	uint32_t u4Idx;
	uint32_t u4TxHeadRoomSize;

	prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	prMemOps = &prHifInfo->rMemOps;
	prTokenInfo = &prHifInfo->rTokenInfo;
	prChipInfo = prAdapter->chip_info;

	prTokenInfo->u4UsedCnt = 0;
	u4TxHeadRoomSize = NIC_TX_DESC_AND_PADDING_LENGTH +
		prChipInfo->txd_append_size;

	for (u4Idx = 0; u4Idx < HIF_TX_MSDU_TOKEN_NUM; u4Idx++) {
		prToken = &prTokenInfo->arToken[u4Idx];
		prToken->fgInUsed = FALSE;
		prToken->prMsduInfo = NULL;

#if HIF_TX_PREALLOC_DATA_BUFFER
		prToken->u4DmaLength = NIC_TX_MAX_SIZE_PER_FRAME +
			u4TxHeadRoomSize;
		if (prMemOps->allocTxDataBuf)
			prMemOps->allocTxDataBuf(prToken, u4Idx);

		if (prToken->prPacket) {
			/* DBGLOG(HAL, TRACE,
				"Msdu Entry[0x%p] Tok[%u] Buf[0x%p] len[%u]\n",
				prToken, u4Idx, prToken->prPacket,
				prToken->u4DmaLength);
			*/
		} else {
			prTokenInfo->u4UsedCnt++;
			DBGLOG(HAL, WARN,
				"Msdu Token Memory alloc failed[%u]\n",
				u4Idx);
			continue;
		}
#else
		prToken->prPacket = NULL;
		prToken->u4DmaLength = 0;
		prToken->rDmaAddr = 0;
#endif
		prToken->rPktDmaAddr = 0;
		prToken->u4PktDmaLength = 0;
		prToken->u4Token = u4Idx;
		prToken->u4CpuIdx = TX_RING_DATA_SIZE;

		prTokenInfo->aprTokenStack[u4Idx] = prToken;
	}

#if (CFG_TX_HIF_CREDIT_FEATURE == 1)
	prTokenInfo->fgEnAdjustCtrl = false;
	prTokenInfo->u4MinBssTxCredit = HIF_DEFAULT_MIN_BSS_TX_CREDIT;
	prTokenInfo->u4MaxBssTxCredit = HIF_DEFAULT_MAX_BSS_TX_CREDIT;
#if (CFG_WFD_SCC_BALANCE_SUPPORT == 1)
	prTokenInfo->u4MinBssBalanceTxCredit =
		HIF_DEFAULT_MIN_BSS_BALANCE_TX_CREDIT;
	prTokenInfo->u4MaxBssBalanceTxCredit =
		HIF_DEFAULT_MAX_BSS_BALANCE_TX_CREDIT;
	prTokenInfo->u4WFDBssBalanceState = WFD_BSS_BALANCE_NO_LIMIT_STATE;
#endif
	for (u4Idx = 0; u4Idx < MAX_BSSID_NUM; u4Idx++) {
		prTokenInfo->u4TxBssCnt[u4Idx] = 0;
		prTokenInfo->u4LastTxBssCnt[u4Idx] = 0;
		prTokenInfo->u4TxCredit[u4Idx] = prTokenInfo->u4MinBssTxCredit;
	}
#else
	prTokenInfo->u4MaxBssFreeCnt = HIF_TX_MSDU_TOKEN_NUM;
	for (u4Idx = 0; u4Idx < MAX_BSSID_NUM; u4Idx++)
		prTokenInfo->u4TxBssCnt[u4Idx] = 0;
#endif

	spin_lock_init(&prTokenInfo->rTokenLock);

	DBGLOG(HAL, INFO, "Msdu Token Init: Tot[%u] Used[%u]\n",
		HIF_TX_MSDU_TOKEN_NUM, prTokenInfo->u4UsedCnt);
}

void halUninitMsduTokenInfo(IN struct ADAPTER *prAdapter)
{
	struct GL_HIF_INFO *prHifInfo;
	struct HIF_MEM_OPS *prMemOps;
	struct MSDU_TOKEN_INFO *prTokenInfo;
	struct MSDU_TOKEN_ENTRY *prToken;
	uint32_t u4Idx;

	prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	prMemOps = &prHifInfo->rMemOps;
	prTokenInfo = &prHifInfo->rTokenInfo;

	for (u4Idx = 0; u4Idx < HIF_TX_MSDU_TOKEN_NUM; u4Idx++) {
		prToken = &prTokenInfo->arToken[u4Idx];

		if (prToken->fgInUsed) {
			if (prMemOps->unmapTxBuf) {
				prMemOps->unmapTxBuf(
					prHifInfo, prToken->rPktDmaAddr,
					prToken->u4PktDmaLength);
				prMemOps->unmapTxBuf(
					prHifInfo, prToken->rDmaAddr,
					prToken->u4DmaLength);
			}

			log_dbg(HAL, TRACE, "Clear pending Tok[%u] Msdu[0x%p] Free[%u]\n",
				prToken->u4Token, prToken->prMsduInfo,
				halGetMsduTokenFreeCnt(prAdapter));

#if !HIF_TX_PREALLOC_DATA_BUFFER
			nicTxFreePacket(prAdapter, prToken->prMsduInfo, FALSE);
			nicTxReturnMsduInfo(prAdapter, prToken->prMsduInfo);
#endif
		}

#if HIF_TX_PREALLOC_DATA_BUFFER
		if (prMemOps->freeBuf)
			prMemOps->freeBuf(prToken->prPacket,
					  prToken->u4DmaLength);
		prToken->prPacket = NULL;
#endif
	}

	prTokenInfo->u4UsedCnt = 0;

#if (CFG_TX_HIF_CREDIT_FEATURE == 1)
	prTokenInfo->u4MinBssTxCredit = HIF_DEFAULT_MIN_BSS_TX_CREDIT;
	prTokenInfo->u4MaxBssTxCredit = HIF_DEFAULT_MAX_BSS_TX_CREDIT;
#if (CFG_WFD_SCC_BALANCE_SUPPORT == 1)
	prTokenInfo->u4MinBssBalanceTxCredit =
		HIF_DEFAULT_MIN_BSS_BALANCE_TX_CREDIT;
	prTokenInfo->u4MaxBssBalanceTxCredit =
		HIF_DEFAULT_MAX_BSS_BALANCE_TX_CREDIT;
	prTokenInfo->u4WFDBssBalanceState = WFD_BSS_BALANCE_NO_LIMIT_STATE;
#endif
	for (u4Idx = 0; u4Idx < MAX_BSSID_NUM; u4Idx++) {
		prTokenInfo->u4TxBssCnt[u4Idx] = 0;
		prTokenInfo->u4LastTxBssCnt[u4Idx] = 0;
		prTokenInfo->u4TxCredit[u4Idx] = prTokenInfo->u4MinBssTxCredit;
	}
#else
	prTokenInfo->u4MaxBssFreeCnt = HIF_DEFAULT_BSS_FREE_CNT;
	for (u4Idx = 0; u4Idx < MAX_BSSID_NUM; u4Idx++)
		prTokenInfo->u4TxBssCnt[u4Idx] = 0;
#endif

	DBGLOG(HAL, INFO, "Msdu Token Uninit: Tot[%u] Used[%u]\n",
		HIF_TX_MSDU_TOKEN_NUM, prTokenInfo->u4UsedCnt);
}

uint32_t halGetMsduTokenFreeCnt(IN struct ADAPTER *prAdapter)
{
	struct PERF_MONITOR *prPerMonitor;
	struct MSDU_TOKEN_INFO *prTokenInfo =
		&prAdapter->prGlueInfo->rHifInfo.rTokenInfo;
	prPerMonitor = &prAdapter->rPerMonitor;
	prPerMonitor->u4UsedCnt = prTokenInfo->u4UsedCnt;

	return HIF_TX_MSDU_TOKEN_NUM - prTokenInfo->u4UsedCnt;
}

struct MSDU_TOKEN_ENTRY *halGetMsduTokenEntry(IN struct ADAPTER *prAdapter,
	uint32_t u4TokenNum)
{
	struct MSDU_TOKEN_INFO *prTokenInfo =
		&prAdapter->prGlueInfo->rHifInfo.rTokenInfo;

	return &prTokenInfo->arToken[u4TokenNum];
}

struct MSDU_TOKEN_ENTRY *halAcquireMsduToken(IN struct ADAPTER *prAdapter,
					     uint8_t ucBssIndex)
{
#if CFG_SUPPORT_PCIE_ASPM
	struct GL_HIF_INFO *prHifInfo;
	struct BUS_INFO *prBusInfo = NULL;
#endif

	struct MSDU_TOKEN_INFO *prTokenInfo =
		&prAdapter->prGlueInfo->rHifInfo.rTokenInfo;
	struct MSDU_TOKEN_ENTRY *prToken;
	unsigned long flags = 0;

#if CFG_SUPPORT_PCIE_ASPM
	prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	prBusInfo = prAdapter->chip_info->bus_info;
#endif

	if (!halGetMsduTokenFreeCnt(prAdapter)) {
		DBGLOG(HAL, INFO, "No more free MSDU token, Used[%u]\n",
			prTokenInfo->u4UsedCnt);
		return NULL;
	}

	spin_lock_irqsave(&prTokenInfo->rTokenLock, flags);

	prToken = prTokenInfo->aprTokenStack[prTokenInfo->u4UsedCnt];
	ktime_get_ts64(&prToken->rTs);
	prToken->fgInUsed = TRUE;

#if defined(_HIF_PCIE)
#if CFG_SUPPORT_PCIE_ASPM
	if (prBusInfo->updatePcieAspm)
		prBusInfo->updatePcieAspm(prAdapter->prGlueInfo, FALSE);
#endif
#endif

	prTokenInfo->u4UsedCnt++;

	if (ucBssIndex < MAX_BSSID_NUM) {
		prToken->ucBssIndex = ucBssIndex;
		prTokenInfo->u4TxBssCnt[ucBssIndex]++;
#if (CFG_TX_HIF_CREDIT_FEATURE == 1)
		prTokenInfo->u4LastTxBssCnt[ucBssIndex]++;
#endif
	}

	spin_unlock_irqrestore(&prTokenInfo->rTokenLock, flags);

	DBGLOG(HAL, LOUD,
		       "Acquire Entry[0x%p] Tok[%u] Buf[%p] Len[%u]\n",
		       prToken, prToken->u4Token,
		       prToken->prPacket, prToken->u4DmaLength);

	return prToken;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief Reset all msdu token. Return used msdu & re-init token.
 *
 * @param prAdapter      a pointer to adapter private data structure.
 *
 */
/*----------------------------------------------------------------------------*/

static void halResetMsduToken(IN struct ADAPTER *prAdapter)
{
	struct GL_HIF_INFO *prHifInfo;
	struct HIF_MEM_OPS *prMemOps;
	struct MSDU_TOKEN_INFO *prTokenInfo;
	struct MSDU_TOKEN_ENTRY *prToken;
	uint32_t u4Idx = 0;

	prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	prMemOps = &prHifInfo->rMemOps;
	prTokenInfo = &prHifInfo->rTokenInfo;

	for (u4Idx = 0; u4Idx < HIF_TX_MSDU_TOKEN_NUM; u4Idx++) {
		prToken = &prTokenInfo->arToken[u4Idx];
		if (prToken->fgInUsed) {
			if (prMemOps->unmapTxBuf) {
				prMemOps->unmapTxBuf(
					prHifInfo, prToken->rPktDmaAddr,
					prToken->u4PktDmaLength);
				prMemOps->unmapTxBuf(
					prHifInfo, prToken->rDmaAddr,
					prToken->u4DmaLength);
				prToken->rPktDmaAddr = 0;
				prToken->u4PktDmaLength = 0;
				prToken->rDmaAddr = 0;
			}

#if !HIF_TX_PREALLOC_DATA_BUFFER
			nicTxFreePacket(prAdapter, prToken->prMsduInfo, FALSE);
			nicTxReturnMsduInfo(prAdapter, prToken->prMsduInfo);
#endif
		}

		prToken->fgInUsed = FALSE;
		prTokenInfo->aprTokenStack[u4Idx] = prToken;
	}
	prTokenInfo->u4UsedCnt = 0;
	for (u4Idx = 0; u4Idx < MAX_BSSID_NUM; u4Idx++) {
		prTokenInfo->u4TxBssCnt[u4Idx] = 0;
#if (CFG_TX_HIF_CREDIT_FEATURE == 1)
		prTokenInfo->u4LastTxBssCnt[u4Idx] = 0;
#endif
	}
}

void halReturnMsduToken(IN struct ADAPTER *prAdapter, uint32_t u4TokenNum)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
#if CFG_SUPPORT_PCIE_ASPM
	struct BUS_INFO *prBusInfo = NULL;
#endif

	struct MSDU_TOKEN_INFO *prTokenInfo =
		&prAdapter->prGlueInfo->rHifInfo.rTokenInfo;
	struct MSDU_TOKEN_ENTRY *prToken;
	unsigned long flags = 0;

#if CFG_SUPPORT_PCIE_ASPM
	prBusInfo = prAdapter->chip_info->bus_info;
#endif

	prHifInfo = &prAdapter->prGlueInfo->rHifInfo;

	if (!prTokenInfo->u4UsedCnt) {
		DBGLOG(HAL, WARN, "MSDU token is full, Used[%u]\n",
			prTokenInfo->u4UsedCnt);
		return;
	}

	prToken = &prTokenInfo->arToken[u4TokenNum];
	if (!prToken->fgInUsed) {
		DBGLOG(HAL, ERROR, "Return unuse token[%u]\n", u4TokenNum);
		return;
	}

	spin_lock_irqsave(&prTokenInfo->rTokenLock, flags);

	if (prToken->ucBssIndex < MAX_BSSID_NUM) {
		if (prTokenInfo->u4TxBssCnt[prToken->ucBssIndex] == 0)
			DBGLOG(HAL, ERROR, "TxBssCnt is zero[%u]\n",
			       prToken->ucBssIndex);
		else
			prTokenInfo->u4TxBssCnt[prToken->ucBssIndex]--;
	}
	prToken->ucBssIndex = MAX_BSSID_NUM;

	prToken->fgInUsed = FALSE;
	prTokenInfo->u4UsedCnt--;
	prTokenInfo->aprTokenStack[prTokenInfo->u4UsedCnt] = prToken;

#if defined(_HIF_PCIE)
#if CFG_SUPPORT_PCIE_ASPM
	if (prTokenInfo->u4UsedCnt == 0 && prBusInfo->updatePcieAspm)
		prBusInfo->updatePcieAspm(prAdapter->prGlueInfo, TRUE);
#endif
#endif

	spin_unlock_irqrestore(&prTokenInfo->rTokenLock, flags);
}


/*----------------------------------------------------------------------------*/
/*!
 * @brief Return all timeout msdu token.
 *
 * @param prAdapter      a pointer to adapter private data structure.
 *
 */
/*----------------------------------------------------------------------------*/
void halReturnTimeoutMsduToken(struct ADAPTER *prAdapter)
{
	struct MSDU_TOKEN_INFO *prTokenInfo;
	struct MSDU_TOKEN_ENTRY *prToken;
	struct timespec64 rNowTs, rTime;
	struct timespec64 rTimeout;
	uint32_t u4Idx = 0;

	ASSERT(prAdapter);
	ASSERT(prAdapter->prGlueInfo);

	prTokenInfo = &prAdapter->prGlueInfo->rHifInfo.rTokenInfo;

	rTimeout.tv_sec = HIF_MSDU_REPORT_RETURN_TIMEOUT;
	KAL_GET_TIME_OF_USEC_OR_NSEC(rTimeout) = 0;
	ktime_get_ts64(&rNowTs);

	for (u4Idx = 0; u4Idx < HIF_TX_MSDU_TOKEN_NUM; u4Idx++) {
		prToken = &prTokenInfo->arToken[u4Idx];
		if (!prToken->fgInUsed)
			continue;

		/* Ignore now time < token time */
		if (halTimeCompare(&rNowTs, &prToken->rTs) < 0)
			continue;

		rTime.tv_sec = rNowTs.tv_sec - prToken->rTs.tv_sec;
		KAL_GET_TIME_OF_USEC_OR_NSEC(rTime) =
			KAL_GET_TIME_OF_USEC_OR_NSEC(rNowTs);
		if (KAL_GET_TIME_OF_USEC_OR_NSEC(prToken->rTs) >
			KAL_GET_TIME_OF_USEC_OR_NSEC(rNowTs)) {
			rTime.tv_sec -= 1;
			KAL_GET_TIME_OF_USEC_OR_NSEC(rTime) +=
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
				SEC_TO_NSEC(1);
#else
				SEC_TO_USEC(1);
#endif
		}
		KAL_GET_TIME_OF_USEC_OR_NSEC(rTime) -=
			KAL_GET_TIME_OF_USEC_OR_NSEC(prToken->rTs);

		/* Return token to free stack */
		if (halTimeCompare(&rTime, &rTimeout) >= 0) {
			DBGLOG(HAL, INFO,
			       "Free TokenId[%u] timeout[sec:%ld, nsec:%ld]\n",
			       u4Idx, rTime.tv_sec,
			       KAL_GET_TIME_OF_USEC_OR_NSEC(rTime));
			halReturnMsduToken(prAdapter, u4Idx);
		}
	}
}

#if (CFG_SUPPORT_TX_DATA_DELAY == 1)
#if KERNEL_VERSION(4, 15, 0) <= CFG80211_VERSION_CODE
void halTxDelayTimeout(struct timer_list *timer)
#else
void halTxDelayTimeout(unsigned long arg)
#endif
{
#if (KERNEL_VERSION(4, 15, 0) <= CFG80211_VERSION_CODE)
	struct GL_HIF_INFO *prHif = from_timer(prHif, timer, rTxDelayTimer);
	struct GLUE_INFO *prGlueInfo =
		(struct GLUE_INFO *)prHif->rTxDelayTimerData;
#else
	struct GLUE_INFO *prGlueInfo = (struct GLUE_INFO *)arg;
#endif
	struct ADAPTER *prAdapter = NULL;
	struct GL_HIF_INFO *prHifInfo;

	prAdapter = prGlueInfo->prAdapter;
	prHifInfo = &prGlueInfo->rHifInfo;

	DBGLOG(HAL, TRACE, "Tx Delay timeout\n");

	KAL_SET_BIT(HIF_TX_DATA_DELAY_TIMEOUT_BIT,
		    prHifInfo->ulTxDataTimeout);
	KAL_CLR_BIT(HIF_TX_DATA_DELAY_TIMER_RUNNING_BIT,
		    prHifInfo->ulTxDataTimeout);

	kalSetTxEvent2Hif(prGlueInfo);
}

void halStartTxDelayTimer(IN struct ADAPTER *prAdapter)
{
	struct GLUE_INFO *prGlueInfo;
	struct GL_HIF_INFO *prHifInfo;
	uint32_t u4Timeout = prAdapter->rWifiVar.u4TxDataDelayTimeout;

	prGlueInfo = prAdapter->prGlueInfo;
	prHifInfo = &prGlueInfo->rHifInfo;

	if (KAL_TEST_BIT(HIF_TX_DATA_DELAY_TIMER_RUNNING_BIT,
			 prHifInfo->ulTxDataTimeout))
		return;

	mod_timer(&prHifInfo->rTxDelayTimer,
		  jiffies + u4Timeout * HZ / MSEC_PER_SEC);
	KAL_SET_BIT(HIF_TX_DATA_DELAY_TIMER_RUNNING_BIT,
		    prHifInfo->ulTxDataTimeout);

	DBGLOG(HAL, TRACE, "Start Delay Timer\n");
}
#endif /* (CFG_SUPPORT_TX_DATA_DELAY == 1) */

bool halHifSwInfoInit(IN struct ADAPTER *prAdapter)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct HIF_MEM_OPS *prMemOps = NULL;
	struct BUS_INFO *prBusInfo = NULL;
	struct mt66xx_chip_info *prChipInfo;
	struct SW_WFDMA_INFO *prSwWfdmaInfo;
	struct WIFI_VAR *prWifiVar;
	uint32_t u4Idx;

	prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	prMemOps = &prHifInfo->rMemOps;
	prChipInfo = prAdapter->chip_info;
	prBusInfo = prChipInfo->bus_info;
	prSwWfdmaInfo = &prBusInfo->rSwWfdmaInfo;
	prWifiVar = &prAdapter->rWifiVar;

#if (CFG_SUPPORT_HOST_OFFLOAD == 1)
	if (IS_FEATURE_ENABLED(prWifiVar->fgEnableMawdTx))
		halMawdAllocTxRing(prAdapter->prGlueInfo, TRUE);

	if (IS_FEATURE_ENABLED(prWifiVar->fgEnableRro)) {
		halRroAllocMem(prAdapter->prGlueInfo);
		halRroAllocRcbList(prAdapter->prGlueInfo);
		if (IS_FEATURE_ENABLED(prWifiVar->fgEnableMawd))
			halMawdAllocRxBlkRing(prAdapter->prGlueInfo, TRUE);
	}
#endif /* CFG_SUPPORT_HOST_OFFLOAD == 1 */

	if (prBusInfo->DmaShdlInit)
		prBusInfo->DmaShdlInit(prAdapter);

	if (!halWpdmaAllocRing(prAdapter->prGlueInfo, true))
		return false;

	halWpdmaInitRing(prAdapter->prGlueInfo, true);
	halInitMsduTokenInfo(prAdapter);
	/* Initialize wfdma reInit handshake parameters */
	if ((prChipInfo->asicWfdmaReInit)
	    && (prChipInfo->asicWfdmaReInit_handshakeInit))
		prChipInfo->asicWfdmaReInit_handshakeInit(prAdapter);

#if defined(_HIF_PCIE) || defined(_HIF_AXI)
	prAdapter->ucSerState = SER_IDLE_DONE;
	prHifInfo->rErrRecoveryCtl.eErrRecovState = ERR_RECOV_STOP_IDLE;
	prHifInfo->rErrRecoveryCtl.u4Status = 0;

#if (KERNEL_VERSION(4, 15, 0) <= CFG80211_VERSION_CODE)
	timer_setup(&prHifInfo->rSerTimer, halHwRecoveryTimeout, 0);
	prHifInfo->rSerTimerData = (unsigned long)prAdapter->prGlueInfo;
#else
	init_timer(&prHifInfo->rSerTimer);
	prHifInfo->rSerTimer.function = halHwRecoveryTimeout;
	prHifInfo->rSerTimer.data = (unsigned long)prAdapter->prGlueInfo;
#endif
	prHifInfo->rSerTimer.expires =
		jiffies + HIF_SER_TIMEOUT * HZ / MSEC_PER_SEC;

#if (CFG_SUPPORT_TX_DATA_DELAY == 1)
#if (KERNEL_VERSION(4, 15, 0) <= CFG80211_VERSION_CODE)
	timer_setup(&prHifInfo->rTxDelayTimer, halTxDelayTimeout, 0);
	prHifInfo->rTxDelayTimerData = (unsigned long)prAdapter->prGlueInfo;
#else
	init_timer(&prHifInfo->rTxDelayTimer);
	prHifInfo->rTxDelayTimer.function = halTxDelayTimeout;
	prHifInfo->rTxDelayTimer.data = (unsigned long)prAdapter->prGlueInfo;
#endif
	prHifInfo->rTxDelayTimer.expires =
		jiffies + prAdapter->rWifiVar.u4TxDataDelayTimeout *
		HZ / MSEC_PER_SEC;
	prHifInfo->ulTxDataTimeout = 0;
#endif /* CFG_SUPPORT_TX_DATA_DELAY == 1 */

	INIT_LIST_HEAD(&prHifInfo->rTxCmdQ);
	INIT_LIST_HEAD(&prHifInfo->rTxCmdFreeList);
	spin_lock_init(&prHifInfo->rTxCmdQLock);

	for (u4Idx = 0; u4Idx < TX_RING_CMD_SIZE; u4Idx++) {
		struct TX_CMD_REQ *prCmdReq;

		prCmdReq = kalMemAlloc(sizeof(struct TX_CMD_REQ),
				       VIR_MEM_TYPE);
		if (!prCmdReq) {
			DBGLOG(HAL, ERROR, "alloc cmd req fail[%u]\n", u4Idx);
			break;
		}
		list_add_tail(&prCmdReq->list, &prHifInfo->rTxCmdFreeList);
	}

	for (u4Idx = 0; u4Idx < NUM_OF_TX_RING; u4Idx++) {
		INIT_LIST_HEAD(&prHifInfo->rTxDataQ[u4Idx]);
		prHifInfo->u4TxDataQLen[u4Idx] = 0;
		spin_lock_init(&prHifInfo->rTxDataQLock[u4Idx]);
	}
#endif

#if (CFG_ENABLE_HOST_BUS_TIMEOUT == 1)
	DBGLOG(HAL, INFO, "Enable Host CSR timeout mechanism.\n");
	HAL_MCR_WR(prAdapter, HOST_CSR_BUS_TIMOUT_CTRL_ADDR, 0x80EFFFFF);
#endif

	prHifInfo->fgIsPowerOn = true;

#if defined(_HIF_PCIE)
#if CFG_SUPPORT_PCIE_ASPM
	prHifInfo->eCurPcieState = PCIE_STATE_NUM;
	prHifInfo->eNextPcieState = PCIE_STATE_NUM;
#endif
#endif

	if (prBusInfo->setupMcuEmiAddr)
		prBusInfo->setupMcuEmiAddr(prAdapter);

#if defined(_HIF_PCIE) || defined(_HIF_AXI)
	if (prSwWfdmaInfo->rOps.init)
		prSwWfdmaInfo->rOps.init(prAdapter->prGlueInfo);
#endif

	return true;
}

void halHifSwInfoUnInit(IN struct GLUE_INFO *prGlueInfo)
{
#if defined(_HIF_PCIE) || defined(_HIF_AXI)
	struct mt66xx_chip_info *prChipInfo;
	struct BUS_INFO *prBusInfo = NULL;
	struct GL_HIF_INFO *prHifInfo = &prGlueInfo->rHifInfo;
	struct HIF_MEM_OPS *prMemOps = NULL;
	struct SW_WFDMA_INFO *prSwWfdmaInfo;
	struct WIFI_VAR *prWifiVar;
	struct list_head *prCur, *prNext;
	struct TX_CMD_REQ *prTxCmdReq;
	struct TX_DATA_REQ *prTxDataReq;
	uint32_t u4Idx;
	unsigned long flags;

	prMemOps = &prHifInfo->rMemOps;
	prChipInfo = prGlueInfo->prAdapter->chip_info;
	prBusInfo = prChipInfo->bus_info;
	prSwWfdmaInfo = &prBusInfo->rSwWfdmaInfo;
	prWifiVar = &prGlueInfo->prAdapter->rWifiVar;

	del_timer_sync(&prHifInfo->rSerTimer);
#if (CFG_SUPPORT_TX_DATA_DELAY == 1)
	del_timer_sync(&prHifInfo->rTxDelayTimer);
#endif

	halUninitMsduTokenInfo(prGlueInfo->prAdapter);
	halWpdmaFreeRing(prGlueInfo);

#if (CFG_SUPPORT_HOST_OFFLOAD == 1)
	if (IS_FEATURE_ENABLED(prWifiVar->fgEnableRro))
		halRroUninit(prGlueInfo);
#endif /* CFG_SUPPORT_HOST_OFFLOAD == 1 */

	spin_lock_irqsave(&prHifInfo->rTxCmdQLock, flags);
	list_for_each_safe(prCur, prNext, &prHifInfo->rTxCmdQ) {
		prTxCmdReq = list_entry(prCur, struct TX_CMD_REQ, list);
		list_del(prCur);
		list_add_tail(&prTxCmdReq->list, &prHifInfo->rTxCmdFreeList);
	}

	list_for_each_safe(prCur, prNext, &prHifInfo->rTxCmdFreeList) {
		prTxCmdReq = list_entry(prCur, struct TX_CMD_REQ, list);
		list_del(prCur);
		kalMemFree(prTxCmdReq, VIR_MEM_TYPE, sizeof(struct TX_CMD_REQ));
	}
	spin_unlock_irqrestore(&prHifInfo->rTxCmdQLock, flags);

	for (u4Idx = 0; u4Idx < NUM_OF_TX_RING; u4Idx++) {
		spin_lock_irqsave(&prHifInfo->rTxDataQLock[u4Idx], flags);
		list_for_each_safe(prCur, prNext, &prHifInfo->rTxDataQ[u4Idx]) {
			prTxDataReq = list_entry(
				prCur, struct TX_DATA_REQ, list);
			list_del(prCur);
			prHifInfo->u4TxDataQLen[u4Idx]--;
		}
		spin_unlock_irqrestore(&prHifInfo->rTxDataQLock[u4Idx], flags);
	}

	if (prSwWfdmaInfo->rOps.uninit)
		prSwWfdmaInfo->rOps.uninit(prGlueInfo);
#endif
}

u_int8_t halProcessToken(IN struct ADAPTER *prAdapter,
	IN uint32_t u4Token,
	IN struct QUE *prFreeQueue)
{
	struct GL_HIF_INFO *prHifInfo;
	struct MSDU_TOKEN_ENTRY *prTokenEntry;
#if !HIF_TX_PREALLOC_DATA_BUFFER
	struct MSDU_INFO *prMsduInfo;
#endif
	struct HIF_MEM_OPS *prMemOps;
	struct RTMP_DMACB *prTxCell;
	struct RTMP_TX_RING *prTxRing;

	prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	prMemOps = &prHifInfo->rMemOps;
	prTokenEntry = halGetMsduTokenEntry(prAdapter, u4Token);

	if (!prTokenEntry->fgInUsed) {
		DBGLOG(HAL, WARN, "Skip unused token[%d]\n",
			u4Token);
		return FALSE;
	}

#if HIF_TX_PREALLOC_DATA_BUFFER
	DBGLOG(HAL, LOUD, "MsduRpt: Tok[%u] Free[%u]\n",
		u4Token,
		halGetMsduTokenFreeCnt(prAdapter));
#else
	prMsduInfo = prTokenEntry->prMsduInfo;
	prMsduInfo->prToken = NULL;
	if (!prMsduInfo->pfTxDoneHandler)
		QUEUE_INSERT_TAIL(prFreeQueue,
			(struct QUE_ENTRY *) prMsduInfo);

	DBGLOG(HAL, LOUD,
		       "MsduRpt: Tok[%u] Msdu[0x%p] TxDone[%u] Free[%u]\n",
		       u4Token, prMsduInfo,
		       (prMsduInfo->pfTxDoneHandler ? TRUE : FALSE),
		       halGetMsduTokenFreeCnt(prAdapter));
#endif

	if (prMemOps->unmapTxBuf) {
		prMemOps->unmapTxBuf(prHifInfo,
				     prTokenEntry->rPktDmaAddr,
				     prTokenEntry->u4PktDmaLength);
		prMemOps->unmapTxBuf(prHifInfo,
				     prTokenEntry->rDmaAddr,
				     prTokenEntry->u4DmaLength);
	}

	if (prTokenEntry->u4CpuIdx < TX_RING_DATA_SIZE) {
		prTxRing = &prHifInfo->TxRing[prTokenEntry->u2Port];
		prTxCell = &prTxRing->Cell[prTokenEntry->u4CpuIdx];
		prTxCell->prToken = NULL;
	}
	prTokenEntry->u4CpuIdx = TX_RING_DATA_SIZE;
	halReturnMsduToken(prAdapter, u4Token);
	GLUE_INC_REF_CNT(prAdapter->rHifStats.u4DataMsduRptCount);

	return TRUE;
}

#if CFG_SUPPORT_TX_LATENCY_STATS
static void halAddConnsysLatencyCount(IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex, IN uint32_t u4ConnsysLatency)
{
	uint32_t *pMaxConnsysDelay = prAdapter->rWifiVar.au4ConnsysTxDelayMax;
	uint32_t *pConnsysDelay =
		prAdapter->rMsduReportStats.rCounting.au4ConnsysLatency
							[ucBssIndex];
	uint8_t i;

	for (i = 0; i < LATENCY_STATS_MAX_SLOTS; i++) {
		if (u4ConnsysLatency <= *pMaxConnsysDelay++) {
			GLUE_INC_REF_CNT(pConnsysDelay[i]);
			break;
		}
	}
}

static void halAddTxFailConnsysLatencyCount(IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex, IN uint32_t u4ConnsysLatency)
{
	uint32_t *pMaxFailConnsysDelay =
		prAdapter->rWifiVar.au4ConnsysTxFailDelayMax;
	uint32_t *pFailConnsysDelay =
		prAdapter->rMsduReportStats.rCounting.au4FailConnsysLatency
							[ucBssIndex];
	uint8_t i;

	for (i = 0; i < LATENCY_STATS_MAX_SLOTS; i++) {
		if (u4ConnsysLatency <= *pMaxFailConnsysDelay++) {
			GLUE_INC_REF_CNT(pFailConnsysDelay[i]);
			break;
		}
	}
}

static void halAddMacLatencyCount(IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex, IN uint32_t u4MacLatency)
{
	uint32_t *pMaxMacDelay = prAdapter->rWifiVar.au4MacTxDelayMax;
	uint32_t *pMacDelay =
		prAdapter->rMsduReportStats.rCounting.au4MacLatency
							[ucBssIndex];
	uint8_t i;

	for (i = 0; i < LATENCY_STATS_MAX_SLOTS; i++) {
		if (u4MacLatency <= *pMaxMacDelay++) {
			GLUE_INC_REF_CNT(pMacDelay[i]);
			break;
		}
	}
}
#endif

void halMsduReportStats(IN struct ADAPTER *prAdapter, IN uint32_t u4Token,
	IN uint32_t u4MacLatency, IN uint32_t u4Stat)
{
#if CFG_SUPPORT_TX_LATENCY_STATS
	struct TX_LATENCY_REPORT_STATS *report = &prAdapter->rMsduReportStats;
	struct TX_LATENCY_STATS *stats = &prAdapter->rMsduReportStats.rCounting;
	struct MSDU_TOKEN_ENTRY *prTokenEntry;
	struct WIFI_VAR *prWifiVar = NULL;
	uint32_t u4ConnsysLatency;
	struct timespec64 rNowTs;
	uint8_t ucBssIndex;

	if (u4Token >= HIF_TX_MSDU_TOKEN_NUM)
		return;

	prTokenEntry = halGetMsduTokenEntry(prAdapter, u4Token);
	prWifiVar = &prAdapter->rWifiVar;
	report->fgTxLatencyEnabled = 1;
	ucBssIndex = prTokenEntry->ucBssIndex;

	/*
	 * Driver latency counted in wlanTxLifetimeTagPacket,
	 * since MSDU info freed on passed to DMA.
	 */

	ktime_get_ts64(&rNowTs);
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
	if (rNowTs.tv_nsec < prTokenEntry->rTs.tv_nsec) {
		rNowTs.tv_sec -= 1;
		rNowTs.tv_nsec += NSEC_PER_SEC;
	}
	u4ConnsysLatency =
		(rNowTs.tv_sec - prTokenEntry->rTs.tv_sec) * MSEC_PER_SEC +
		(rNowTs.tv_nsec - prTokenEntry->rTs.tv_nsec) / NSEC_PER_MSEC;
#else
	if (rNowTs.tv_usec < prTokenEntry->rTs.tv_usec) {
		rNowTs.tv_sec -= 1;
		rNowTs.tv_usec += USEC_PER_SEC;
	}
	u4ConnsysLatency =
		(rNowTs.tv_sec - prTokenEntry->rTs.tv_sec) * MSEC_PER_SEC +
		(rNowTs.tv_usec - prTokenEntry->rTs.tv_usec) / USEC_PER_MSEC;
#endif

	halAddMacLatencyCount(prAdapter, ucBssIndex, u4MacLatency);

	if (unlikely(u4Stat)) {
		uint32_t lim = prWifiVar->u4ContinuousTxFailThreshold;

		GLUE_INC_REF_CNT(stats->u4TxFail);
		GLUE_INC_REF_CNT(report->u4ContinuousTxFail);
		halAddTxFailConnsysLatencyCount(prAdapter, ucBssIndex,
						u4ConnsysLatency);
		if (lim && report->u4ContinuousTxFail >= lim &&
		    report->u4ContinuousTxFail % lim == 0) {
			char uevent[64];

			kalSnprintf(uevent, sizeof(uevent),
				"abnormaltrx=DIR:TX,event:AbDrop,Count:%u",
				report->u4ContinuousTxFail);
			kalSendUevent(uevent);
		}
	} else {
		halAddConnsysLatencyCount(prAdapter, ucBssIndex,
					u4ConnsysLatency);
		report->u4ContinuousTxFail = 0;
	}

	if (prWifiVar->fgPacketLatencyLog)
		DBGLOG(HAL, INFO, "Latency C: %u M: %u; tok=%u",
			u4ConnsysLatency, u4MacLatency, u4Token);
#endif
}

static void halMsduV3ReportDelayStats(IN struct ADAPTER *prAdapter,
		IN uint32_t u4Token, IN union HW_MAC_MSDU_TOKEN_T *msduToken)
{
#if CFG_SUPPORT_TX_LATENCY_STATS
	halMsduReportStats(prAdapter, u4Token,
			msduToken->rFormatV3.rP0.u4TxCnt,
			msduToken->rFormatV3.rP0.u4Stat);
#endif
}

static void halDefaultProcessMsduReport(IN struct ADAPTER *prAdapter,
	IN OUT struct SW_RFB *prSwRfb,
	IN OUT struct QUE *prFreeQueue)
{
	struct GL_HIF_INFO *prHifInfo;
	struct HIF_MEM_OPS *prMemOps;
	struct HW_MAC_MSDU_REPORT *prMsduReport;
#if !HIF_TX_PREALLOC_DATA_BUFFER
	struct MSDU_INFO *prMsduInfo;
#endif
	uint16_t u2TokenCnt, u2TotalTokenCnt;
	uint32_t u4Idx, u4Token;
	uint8_t ucVer;

	ASSERT(prAdapter);
	ASSERT(prAdapter->prGlueInfo);

	prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	prMemOps = &prHifInfo->rMemOps;

	prMsduReport = (struct HW_MAC_MSDU_REPORT *)prSwRfb->pucRecvBuff;
	ucVer = prMsduReport->DW1.field.u4Ver;
	if (ucVer == TFD_EVT_VER_3)
		u2TotalTokenCnt = prMsduReport->DW0.field_v3.u2MsduCount;
	else
		u2TotalTokenCnt = prMsduReport->DW0.field.u2MsduCount;

	u4Idx = u2TokenCnt = 0;
	while (u2TokenCnt < u2TotalTokenCnt) {
		/* Format version of this tx done event.
		 *	0: MT7615
		 *	1: MT7622, CONNAC (X18/P18/MT7663)
		 *	2: MT7915 E1/MT6885
		 *      3: MT7915 E2/MT7961
		 */
		if (ucVer == TFD_EVT_VER_0) {
			u4Token = prMsduReport->au4MsduToken[u4Idx >> 1].
				rFormatV0.u2MsduID[u4Idx & 1];
		} else if (ucVer == TFD_EVT_VER_1) {
			u4Token = prMsduReport->au4MsduToken[u4Idx].
				rFormatV1.u2MsduID;
		} else if (ucVer == TFD_EVT_VER_2) {
			u4Token = prMsduReport->au4MsduToken[u4Idx].
				rFormatV2.u2MsduID;
		} else { /* TFD_EVT_VER_3 */
			if (!prMsduReport->au4MsduToken[u4Idx].
				rFormatV3.rP0.u4Pair) {
				u4Token = prMsduReport->au4MsduToken[u4Idx].
						rFormatV3.rP0.u4MsduID;

				halMsduV3ReportDelayStats(prAdapter, u4Token,
					&prMsduReport->au4MsduToken[u4Idx]);
			} else {
				u4Idx++;
				continue;
			}
		}
		u4Idx++;
		u2TokenCnt++;

		if (u4Token >= HIF_TX_MSDU_TOKEN_NUM) {
			DBGLOG(HAL, ERROR, "Error MSDU report[%u]\n", u4Token);
			DBGLOG_MEM32(HAL, ERROR, prMsduReport, 64);
			prAdapter->u4HifDbgFlag |= DEG_HIF_DEFAULT_DUMP;
			halPrintHifDbgInfo(prAdapter);
			continue;
		}

		halProcessToken(prAdapter, u4Token, prFreeQueue);
	}
}

void halRxProcessMsduReport(IN struct ADAPTER *prAdapter,
	IN OUT struct SW_RFB *prSwRfb)
{
	struct RX_DESC_OPS_T *prRxDescOps;
	struct QUE rFreeQueue;

	prRxDescOps = prAdapter->chip_info->prRxDescOps;
	QUEUE_INITIALIZE(&rFreeQueue);

	if (prRxDescOps->nic_rxd_handle_host_rpt)
		prRxDescOps->nic_rxd_handle_host_rpt(
			prAdapter, prSwRfb, &rFreeQueue);
	else
		halDefaultProcessMsduReport(
			prAdapter, prSwRfb, &rFreeQueue);

#if !HIF_TX_PREALLOC_DATA_BUFFER
	nicTxMsduDoneCb(prAdapter->prGlueInfo, &rFreeQueue);
#endif

	/* Indicate Service Thread */
	if (!HAL_IS_TX_DIRECT(prAdapter) &&
		wlanGetTxPendingFrameCount(prAdapter) > 0)
		kalSetEvent(prAdapter->prGlueInfo);
#if CFG_SUPPORT_MULTITHREAD
	kalSetTxEvent2Hif(prAdapter->prGlueInfo);
#endif
}

void halTxUpdateCutThroughDesc(struct GLUE_INFO *prGlueInfo,
			       struct MSDU_INFO *prMsduInfo,
			       struct MSDU_TOKEN_ENTRY *prFillToken,
			       struct MSDU_TOKEN_ENTRY *prDataToken,
			       uint32_t u4Idx, bool fgIsLast)
{
	struct GL_HIF_INFO *prHifInfo;
	struct HIF_MEM_OPS *prMemOps;
	struct mt66xx_chip_info *prChipInfo;
	struct TX_DESC_OPS_T *prTxDescOps;
	uint8_t *pucBufferTxD;
	uint32_t u4TxHeadRoomSize;
	phys_addr_t rPhyAddr = 0;

	prHifInfo = &prGlueInfo->rHifInfo;
	prMemOps = &prHifInfo->rMemOps;
	prChipInfo = prGlueInfo->prAdapter->chip_info;
	prTxDescOps = prChipInfo->prTxDescOps;
	pucBufferTxD = prDataToken->prPacket;
	u4TxHeadRoomSize = NIC_TX_DESC_AND_PADDING_LENGTH +
		prChipInfo->txd_append_size;

	if (prMemOps->mapTxBuf) {
		rPhyAddr = prMemOps->mapTxBuf(
			prHifInfo, pucBufferTxD, u4TxHeadRoomSize,
			prMsduInfo->u2FrameLength);
	} else {
		if (prDataToken->rDmaAddr)
			rPhyAddr = prDataToken->rDmaAddr + u4TxHeadRoomSize;
	}

	if (!rPhyAddr) {
		DBGLOG(HAL, ERROR, "Get address error!\n");
		return;
	}

	if (prTxDescOps->fillHifAppend)
		prTxDescOps->fillHifAppend(prGlueInfo->prAdapter,
			prMsduInfo, prDataToken->u4Token,
			rPhyAddr, u4Idx, fgIsLast, prFillToken->prPacket);

	prDataToken->rPktDmaAddr = rPhyAddr;
	prDataToken->u4PktDmaLength = prMsduInfo->u2FrameLength;
}

static uint32_t halTxGetPageCount(IN struct ADAPTER *prAdapter,
	IN uint32_t u4FrameLength, IN u_int8_t fgIncludeDesc)
{
	return 1;
}

uint32_t halTxGetDataPageCount(IN struct ADAPTER *prAdapter,
	IN uint32_t u4FrameLength, IN u_int8_t fgIncludeDesc)
{
	return halTxGetPageCount(prAdapter, u4FrameLength, fgIncludeDesc);
}

uint32_t halTxGetCmdPageCount(IN struct ADAPTER *prAdapter,
	IN uint32_t u4FrameLength, IN u_int8_t fgIncludeDesc)
{
	return halTxGetPageCount(prAdapter, u4FrameLength, fgIncludeDesc);
}

uint32_t halTxPollingResource(IN struct ADAPTER *prAdapter, IN uint8_t ucTC)
{
	nicTxReleaseResource(prAdapter, ucTC, 1, TRUE, FALSE);
	return WLAN_STATUS_SUCCESS;
}

void halSerHifReset(IN struct ADAPTER *prAdapter)
{
}

u_int8_t halRxInsertRecvRfbList(
	struct ADAPTER *prAdapter,
	struct QUE *prReceivedRfbList,
	struct SW_RFB *prSwRfb)
{
	struct GLUE_INFO *prGlueInfo;
	struct RX_CTRL *prRxCtrl;
	u_int8_t fgRet = TRUE;

	prGlueInfo = prAdapter->prGlueInfo;
	prRxCtrl = &prAdapter->rRxCtrl;

	if (HAL_IS_RX_DIRECT(prAdapter) &&
	    prSwRfb->ucPacketType == RX_PKT_TYPE_RX_DATA) {
#if CFG_SUPPORT_RX_NAPI
		/* If RxDirectNapi and RxFifo available, run NAPI mode
		 * Otherwise, goto default RX-direct policy
		 */
		if (prGlueInfo->prRxDirectNapi) {
			if (KAL_FIFO_IN(&prGlueInfo->rRxKfifoQ, prSwRfb)) {
				RX_INC_CNT(prRxCtrl, RX_NAPI_FIFO_IN_COUNT);
				RX_INC_CNT(prRxCtrl, RX_NAPI_SCHEDULE_COUNT);
				kal_napi_schedule(prGlueInfo->prRxDirectNapi);
			} else {
				/* should not enter here */
				RX_INC_CNT(prRxCtrl,
					   RX_NAPI_FIFO_ABN_FULL_COUNT);
				nicRxProcessPacketType(prAdapter, prSwRfb);
				fgRet = FALSE;
			}
		} else
#endif /* CFG_SUPPORT_RX_NAPI */
		{
			/* Rx Direct without NAPI */
			RX_INC_CNT(prRxCtrl, RX_NAPI_FIFO_ABNORMAL_COUNT);
			nicRxProcessPacketType(prAdapter, prSwRfb);
		}
	} else
		QUEUE_INSERT_TAIL(prReceivedRfbList, &prSwRfb->rQueEntry);

	return fgRet;
}

void halRxReceiveRFBs(IN struct ADAPTER *prAdapter, uint32_t u4Port,
	uint8_t fgRxData)
{
	struct GLUE_INFO *prGlueInfo;
	struct mt66xx_chip_info *prChipInfo;
	struct RX_CTRL *prRxCtrl;
	struct SW_RFB *prSwRfb = (struct SW_RFB *) NULL;
	uint8_t *pucBuf = NULL;
	void *prRxStatus;
	u_int8_t fgStatus;
	uint32_t u4RxCnt;
	uint32_t u4RxLoopCnt, u4RxSuccessCnt = 0;
	struct RX_DESC_OPS_T *prRxDescOps;
	struct RTMP_RX_RING *prRxRing;
	struct GL_HIF_INFO *prHifInfo;
	uint32_t u4MsduReportCnt = 0;
	struct QUE *prFreeSwRfbList = NULL, *prReceivedRfbList = NULL;
	struct QUE rFreeSwRfbList, rReceivedRfbList;
	struct HIF_STATS *prHifStats;
	static int32_t ai4PortLock[RX_RING_MAX];
	u_int8_t fgRet = TRUE;

	KAL_SPIN_LOCK_DECLARATION();

	/* Port idx sanity */
	if (u4Port >= RX_RING_MAX) {
		DBGLOG(RX, ERROR, "Invalid P[%u]\n", u4Port);
		return;
	}

	if (GLUE_INC_REF_CNT(ai4PortLock[u4Port]) > 1) {
		/* Single user allowed per port read */
		DBGLOG_LIMITED(RX, WARN, "Single user only P[%u] [%d]\n",
			u4Port,
			GLUE_GET_REF_CNT(ai4PortLock[u4Port]));
		goto end;
	}

	DEBUGFUNC("nicRxPCIeReceiveRFBs");

	ASSERT(prAdapter);
	prGlueInfo = prAdapter->prGlueInfo;
	prHifInfo = &prGlueInfo->rHifInfo;
	prChipInfo = prAdapter->chip_info;
	prRxDescOps = prChipInfo->prRxDescOps;

	prRxCtrl = &prAdapter->rRxCtrl;
	ASSERT(prRxCtrl);
	ASSERT(prRxDescOps->nic_rxd_get_rx_byte_count);
	ASSERT(prRxDescOps->nic_rxd_get_pkt_type);
	ASSERT(prRxDescOps->nic_rxd_get_wlan_idx);
#if DBG
	ASSERT(prRxDescOps->nic_rxd_get_sec_mode);
#endif /* DBG */

	if (!prRxCtrl->rFreeSwRfbList.u4NumElem) {
		DBGLOG_LIMITED(RX, WARN, "No More RFB for P[%u], Ind=%u\n",
				u4Port, prRxCtrl->rIndicatedRfbList.u4NumElem);
		KAL_SET_BIT(u4Port, prAdapter->ulNoMoreRfb);
		goto end;
	}

	u4RxCnt = halWpdmaGetRxDmaDoneCnt(prGlueInfo, u4Port);

	DBGLOG(RX, TEMP, "halRxReceiveRFBs: u4RxCnt:%d\n", u4RxCnt);

	if (!u4RxCnt) {
		/* No data in DMA, return directly */
		KAL_CLR_BIT(u4Port, prAdapter->ulNoMoreRfb);
		goto end;
	}

	prRxRing = &prHifInfo->RxRing[u4Port];
	prHifStats = &prAdapter->rHifStats;
	prChipInfo = prAdapter->chip_info;

	prFreeSwRfbList = &rFreeSwRfbList;
	prReceivedRfbList = &rReceivedRfbList;
	QUEUE_INITIALIZE(prFreeSwRfbList);
	QUEUE_INITIALIZE(prReceivedRfbList);

	KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_RX_FREE_QUE);
	QUEUE_MOVE_ALL(prFreeSwRfbList, &prRxCtrl->rFreeSwRfbList);
	if (prFreeSwRfbList->u4NumElem < u4RxCnt) {
		DBGLOG_LIMITED(RX, WARN,
			"No More RFB for P[%u], RxCnt:%u, RfbCnt:%u, Ind:%u\n",
			u4Port, u4RxCnt,
			prFreeSwRfbList->u4NumElem,
			prRxCtrl->rIndicatedRfbList.u4NumElem);
	}
	KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_RX_FREE_QUE);

	prHifStats->u4RxDataRegCnt++;
	kalDevRegRead(prAdapter->prGlueInfo, prRxRing->hw_cidx_addr,
		      &prRxRing->RxCpuIdx);

	u4RxLoopCnt = u4RxCnt;
	while (u4RxLoopCnt--) {
#if CFG_SUPPORT_RX_NAPI
		/* if fifo exhausted, stop deQ and schedule NAPI */
		if (prGlueInfo->prRxDirectNapi &&
			KAL_FIFO_IS_FULL(&prGlueInfo->rRxKfifoQ)) {
			RX_INC_CNT(prRxCtrl, RX_NAPI_FIFO_FULL_COUNT);
			RX_INC_CNT(prRxCtrl, RX_NAPI_SCHEDULE_COUNT);
			kal_napi_schedule(prGlueInfo->prRxDirectNapi);
			break;
		}
#endif /* CFG_SUPPORT_RX_NAPI */
		QUEUE_REMOVE_HEAD(prFreeSwRfbList, prSwRfb, struct SW_RFB *);
		if (!prSwRfb)
			break;

		if (fgRxData) {
			fgStatus = kalDevReadData(prGlueInfo, u4Port, prSwRfb);
		} else {
			pucBuf = prSwRfb->pucRecvBuff;
			ASSERT(pucBuf);

			fgStatus = kalDevPortRead(prGlueInfo,
				u4Port, CFG_RX_MAX_PKT_SIZE,
				pucBuf, CFG_RX_MAX_PKT_SIZE);
		}
		if (!fgStatus) {
			QUEUE_INSERT_TAIL(prFreeSwRfbList, &prSwRfb->rQueEntry);
			DBGLOG(RX, TEMP, "fgStatus:%d\n", fgStatus);
			continue;
		}

		u4RxSuccessCnt++;
		prRxStatus = prSwRfb->prRxStatus;
		ASSERT(prRxStatus);

#if (CFG_DUMP_RXD == 1)
		DBGLOG(HAL, INFO, "Dump RXD: \n");
		DBGLOG_MEM8(HAL, INFO, prRxStatus, prChipInfo->rxd_size);
#endif

		prSwRfb->ucPacketType =
			prRxDescOps->nic_rxd_get_pkt_type(prRxStatus);
#if DBG
		DBGLOG_LIMITED(RX, LOUD, "ucPacketType = %u, ucSecMode = %u\n",
				  prSwRfb->ucPacketType,
				  prRxDescOps->nic_rxd_get_sec_mode(
					prRxStatus));
#endif /* DBG */

		if (prSwRfb->ucPacketType == RX_PKT_TYPE_MSDU_REPORT) {
			nicRxProcessMsduReport(prAdapter, prSwRfb);
			u4MsduReportCnt++;
			QUEUE_INSERT_TAIL(prFreeSwRfbList, &prSwRfb->rQueEntry);
			continue;
		} else if (prSwRfb->ucPacketType == RX_PKT_TYPE_RX_REPORT) {
			nicRxProcessRxReport(prAdapter, prSwRfb);
			QUEUE_INSERT_TAIL(prFreeSwRfbList, &prSwRfb->rQueEntry);
			continue;
		}

		GLUE_RX_SET_PKT_INT_TIME(prSwRfb->pvPacket,
					 prGlueInfo->u8HifIntTime);
		GLUE_RX_SET_PKT_RX_TIME(prSwRfb->pvPacket, sched_clock());

		prSwRfb->ucStaRecIdx =
			secGetStaIdxByWlanIdx(
				prAdapter,
				prRxDescOps->nic_rxd_get_wlan_idx(prRxStatus));

		fgRet = halRxInsertRecvRfbList(
			prAdapter, prReceivedRfbList, prSwRfb);
		if (!fgRet)
			break;

		RX_INC_CNT(prRxCtrl, RX_MPDU_TOTAL_COUNT);
		DBGLOG(RX, TEMP, "Recv p=%p total:%lu\n",
			prSwRfb, RX_GET_CNT(prRxCtrl, RX_MPDU_TOTAL_COUNT));
		kalTraceEvent("Recv p=%p total:%lu",
			prSwRfb, RX_GET_CNT(prRxCtrl, RX_MPDU_TOTAL_COUNT));
	}

	kalDevRegWrite(prGlueInfo, prRxRing->hw_cidx_addr, prRxRing->RxCpuIdx);

	KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_RX_FREE_QUE);
	QUEUE_CONCATENATE_QUEUES(&prRxCtrl->rFreeSwRfbList,
		prFreeSwRfbList);
	KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_RX_FREE_QUE);

	KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_RX_QUE);
	QUEUE_CONCATENATE_QUEUES(&prRxCtrl->rReceivedRfbList,
		prReceivedRfbList);
	KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_RX_QUE);

	prRxRing->u4PendingCnt = u4RxCnt - u4RxSuccessCnt;

	if (prRxRing->u4PendingCnt == 0) {
		/* clear no more rfb port bit */
		KAL_CLR_BIT(u4Port, prAdapter->ulNoMoreRfb);
	} else
		KAL_SET_BIT(u4Port, prAdapter->ulNoMoreRfb);

	if (u4MsduReportCnt > 0)
		DBGLOG(RX, TEMP, "Recv %d msdu reports\n", u4MsduReportCnt);

end:
	GLUE_DEC_REF_CNT(ai4PortLock[u4Port]);
}

static void halDefaultProcessRxInterrupt(IN struct ADAPTER *prAdapter)
{
	struct GL_HIF_INFO *prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	union WPDMA_INT_STA_STRUCT rIntrStatus;

	rIntrStatus = (union WPDMA_INT_STA_STRUCT)prHifInfo->u4IntStatus;
	prAdapter->prGlueInfo->u8HifIntTime = sched_clock();

	if (rIntrStatus.field.rx_done_1 ||
	    (KAL_TEST_BIT(RX_RING_EVT, prAdapter->ulNoMoreRfb)))
		halRxReceiveRFBs(prAdapter, RX_RING_EVT, FALSE);

	if (rIntrStatus.field.rx_done_0 ||
	    (KAL_TEST_BIT(RX_RING_DATA0, prAdapter->ulNoMoreRfb)))
		halRxReceiveRFBs(prAdapter, RX_RING_DATA0, TRUE);
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
void halProcessRxInterrupt(IN struct ADAPTER *prAdapter)
{
	struct BUS_INFO *prBusInfo = prAdapter->chip_info->bus_info;

	if (prBusInfo->processRxInterrupt)
		prBusInfo->processRxInterrupt(prAdapter);
	else
		halDefaultProcessRxInterrupt(prAdapter);
}

static int32_t halWpdmaFreeRingDesc(struct GLUE_INFO *prGlueInfo,
				    struct RTMP_DMABUF *prDescRing)
{
	struct GL_HIF_INFO *prHifInfo;
	struct HIF_MEM_OPS *prMemOps;

	ASSERT(prGlueInfo);

	prHifInfo = &prGlueInfo->rHifInfo;
	prMemOps = &prHifInfo->rMemOps;

	if (prMemOps->freeDesc)
		prMemOps->freeDesc(prHifInfo, prDescRing);

	return TRUE;
}

bool halWpdmaAllocTxRing(struct GLUE_INFO *prGlueInfo, uint32_t u4Num,
			 uint32_t u4Size, uint32_t u4DescSize, bool fgAllocMem)
{
	struct GL_HIF_INFO *prHifInfo;
	struct HIF_MEM_OPS *prMemOps;
	struct RTMP_TX_RING *pTxRing;
	struct RTMP_DMABUF *prTxDesc;
	struct RTMP_DMACB *prTxCell;
	struct TXD_STRUCT *pTxD;
	phys_addr_t RingBasePa;
	void *RingBaseVa;
	uint32_t u4Idx;

	ASSERT(prGlueInfo);
	prHifInfo = &prGlueInfo->rHifInfo;
	prMemOps = &prHifInfo->rMemOps;
	prTxDesc = &prHifInfo->TxDescRing[u4Num];

	/* Don't re-alloc memory when second time call alloc ring */
	prTxDesc->AllocSize = u4Size * u4DescSize;
	if (fgAllocMem && prMemOps->allocTxDesc)
		prMemOps->allocTxDesc(prHifInfo, prTxDesc, u4Num);

	if (prTxDesc->AllocVa == NULL) {
		DBGLOG(HAL, ERROR, "TxDescRing[%d] allocation failed\n", u4Num);
		return false;
	}

	DBGLOG(HAL, TRACE, "TxDescRing[%p]: total %lu bytes allocated\n",
	       prTxDesc->AllocVa, prTxDesc->AllocSize);

	/* Save PA & VA for further operation */
	RingBasePa = prTxDesc->AllocPa;
	RingBaseVa = prTxDesc->AllocVa;

	/*
	 * Initialize Tx Ring Descriptor and associated buffer memory
	 */
	pTxRing = &prHifInfo->TxRing[u4Num];
	pTxRing->u4RingSize = u4Size;
	for (u4Idx = 0; u4Idx < u4Size; u4Idx++) {
		prTxCell = &pTxRing->Cell[u4Idx];
		prTxCell->pPacket = NULL;
		prTxCell->pBuffer = NULL;

		/* Init Tx Ring Size, Va, Pa variables */
		prTxCell->AllocSize = u4DescSize;
		prTxCell->AllocVa = RingBaseVa;
		prTxCell->AllocPa = RingBasePa;
		prTxCell->prToken = NULL;

		RingBasePa += u4DescSize;
		RingBaseVa += u4DescSize;

		if (fgAllocMem && prMemOps->allocTxCmdBuf) {
			bool ret;

			ret = prMemOps->allocTxCmdBuf(&prTxCell->DmaBuf,
						u4Num, u4Idx);
			if (ret == false) {
				DBGLOG(HAL, ERROR,
					"TxRing[%u] TxCmd[%u] alloc failed\n",
					u4Num, u4Idx);
				return false;
			}
		}

		pTxD = (struct TXD_STRUCT *)prTxCell->AllocVa;
		pTxD->DMADONE = 1;
	}

	DBGLOG(HAL, TRACE, "TxRing[%d]: total %d entry allocated\n",
	       u4Num, u4Idx);

	return true;
}

void halWpdmaGetRxBuf(
	struct GLUE_INFO *prGlueInfo,
	struct RTMP_DMACB *prRxCell,
	struct RTMP_DMABUF *pDmaBuf,
	uint32_t u4Num,
	uint32_t u4Idx,
	bool fgAllocMem)
{
	struct ADAPTER *prAdapter;
	struct mt66xx_chip_info *prChipInfo;
	struct GL_HIF_INFO *prHifInfo;
	struct HIF_MEM_OPS *prMemOps;
	struct WIFI_VAR *prWifiVar;

	prAdapter = prGlueInfo->prAdapter;
	prChipInfo = prAdapter->chip_info;
	prHifInfo = &prGlueInfo->rHifInfo;
	prMemOps = &prHifInfo->rMemOps;
	prWifiVar = &prAdapter->rWifiVar;

#if (CFG_SUPPORT_HOST_OFFLOAD == 1)
	/* rro need realloc memory when SER */
	if ((IS_FEATURE_ENABLED(prWifiVar->fgEnableRro)) &&
	    halIsDataRing(RX_RING, u4Num)) {
		struct RX_CTRL_BLK *prRcb;

		prRcb = halRroGetFreeRcbBlk(prHifInfo, pDmaBuf, u4Num);
		if (prRcb)
			prRxCell->pPacket = prRcb->prSkb;
		return;
	}
#endif /* CFG_SUPPORT_HOST_OFFLOAD == 1 */

	if (fgAllocMem && prMemOps->allocRxBuf)
		prRxCell->pPacket = prMemOps->allocRxBuf(
			prHifInfo, pDmaBuf, u4Num, u4Idx);
}

bool halWpdmaAllocRxRing(struct GLUE_INFO *prGlueInfo, uint32_t u4Num,
			 uint32_t u4Size, uint32_t u4DescSize,
			 uint32_t u4BufSize, bool fgAllocMem)
{
	struct GL_HIF_INFO *prHifInfo;
	struct HIF_MEM_OPS *prMemOps;
	struct RTMP_RX_RING *pRxRing;
	struct RTMP_DMABUF *prRxDesc;
	struct RTMP_DMABUF *pDmaBuf;
	struct RTMP_DMACB *prRxCell;
	struct RXD_STRUCT *pRxD;
	phys_addr_t RingBasePa;
	void *RingBaseVa;
	uint32_t u4Idx;

	ASSERT(prGlueInfo);
	prHifInfo = &prGlueInfo->rHifInfo;
	prMemOps = &prHifInfo->rMemOps;
	prRxDesc = &prHifInfo->RxDescRing[u4Num];

	/* Don't re-alloc memory when second time call alloc ring */
	prRxDesc->AllocSize = u4Size * u4DescSize;
	if (fgAllocMem && prMemOps->allocRxDesc)
		prMemOps->allocRxDesc(prHifInfo, prRxDesc, u4Num);

	if (prRxDesc->AllocVa == NULL) {
		DBGLOG(HAL, ERROR, "RxDescRing allocation failed!!\n");
		return false;
	}

	DBGLOG(HAL, TRACE, "RxDescRing[%p]: total %lu bytes allocated\n",
		prRxDesc->AllocVa, prRxDesc->AllocSize);

	/* Initialize Rx Ring and associated buffer memory */
	RingBasePa = prRxDesc->AllocPa;
	RingBaseVa = prRxDesc->AllocVa;

	pRxRing = &prHifInfo->RxRing[u4Num];
	pRxRing->u4BufSize = u4BufSize;
	pRxRing->u4RingSize = u4Size;
	pRxRing->fgRxSegPkt = FALSE;
	pRxRing->pvPacket = NULL;
	pRxRing->u4PacketLen = 0;
	pRxRing->u4MagicCnt = 0;

	for (u4Idx = 0; u4Idx < u4Size; u4Idx++) {
		/* Init RX Ring Size, Va, Pa variables */
		prRxCell = &pRxRing->Cell[u4Idx];
		prRxCell->AllocSize = u4DescSize;
		prRxCell->AllocVa = RingBaseVa;
		prRxCell->AllocPa = RingBasePa;
		prRxCell->prToken = NULL;

		/* Offset to next ring descriptor address */
		RingBasePa += u4DescSize;
		RingBaseVa += u4DescSize;

		/* Setup Rx associated Buffer size & allocate share memory */
		pDmaBuf = &prRxCell->DmaBuf;
		pDmaBuf->AllocSize = u4BufSize;

		halWpdmaGetRxBuf(prGlueInfo, prRxCell, pDmaBuf,
				 u4Num, u4Idx, fgAllocMem);
		if (pDmaBuf->AllocVa == NULL) {
			log_dbg(HAL, ERROR, "\nFailed to allocate RxRing buffer idx[%u]\n",
				u4Idx);
			return false;
		}

		/* Write RxD buffer address & allocated buffer length */
		pRxD = (struct RXD_STRUCT *)prRxCell->AllocVa;
		pRxD->SDPtr0 = ((uint64_t)pDmaBuf->AllocPa) &
			DMA_LOWER_32BITS_MASK;
#ifdef CONFIG_PHYS_ADDR_T_64BIT
		pRxD->SDPtr1 = (((uint64_t)pDmaBuf->AllocPa >>
			DMA_BITS_OFFSET) & DMA_HIGHER_4BITS_MASK);
#else
		pRxD->SDPtr1 = 0;
#endif
		pRxD->SDLen0 = u4BufSize;
		pRxD->DMADONE = 0;
		pRxD->MagicCnt = 0;
	}

	DBGLOG(HAL, TRACE, "Rx[%d] Ring: total %d entry allocated\n",
	       u4Num, u4Idx);

	return true;
}

static void halDefaultHifRst(struct GLUE_INFO *prGlueInfo)
{
	/* Reset Conn HIF logic */
	kalDevRegWrite(prGlueInfo, CONN_HIF_RST, 0x00000020);
	kalDevRegWrite(prGlueInfo, CONN_HIF_RST, 0x00000030);
}

void halHifRst(struct GLUE_INFO *prGlueInfo)
{
	struct ADAPTER *prAdapter;
	struct BUS_INFO *prBusInfo;

	prAdapter = prGlueInfo->prAdapter;
	prBusInfo = prAdapter->chip_info->bus_info;

	/* Reset dmashdl and wpdma */
	if (prBusInfo->hifRst)
		prBusInfo->hifRst(prGlueInfo);
	else
		halDefaultHifRst(prGlueInfo);
}

bool halWpdmaAllocRing(struct GLUE_INFO *prGlueInfo, bool fgAllocMem)
{
	struct GL_HIF_INFO *prHifInfo;
	struct BUS_INFO *prBusInfo = NULL;
	int32_t u4Num, u4Index, u4Size;

	ASSERT(prGlueInfo);
	prHifInfo = &prGlueInfo->rHifInfo;
	prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;

	/*
	 *   Allocate all ring descriptors, include TxD, RxD, MgmtD.
	 *   Although each size is different, to prevent cacheline and alignment
	 *   issue, I intentional set them all to 64 bytes
	 */
	for (u4Num = 0; u4Num < NUM_OF_TX_RING; u4Num++) {
		if (u4Num == TX_RING_DATA1 &&
				!prBusInfo->tx_ring1_data_idx)
			continue;
		else if (u4Num == TX_RING_DATA_PRIO &&
				!prBusInfo->tx_ring2_data_idx)
			continue;
		else if (u4Num == TX_RING_DATA_ALTX &&
				!prBusInfo->tx_ring3_data_idx)
			continue;
		u4Size = halIsDataRing(TX_RING, u4Num) ?
			TX_RING_DATA_SIZE : TX_RING_CMD_SIZE;
		if (!halWpdmaAllocTxRing(prGlueInfo, u4Num, u4Size,
					 TXD_SIZE, fgAllocMem)) {
			DBGLOG(HAL, ERROR, "AllocTxRing[%d] fail\n", u4Num);
			return false;
		}
	}

	/* Data Rx path */
	if (!halWpdmaAllocRxRing(prGlueInfo, RX_RING_DATA0,
				 RX_RING0_SIZE, RXD_SIZE,
				 CFG_RX_MAX_MPDU_SIZE, fgAllocMem)) {
		DBGLOG(HAL, ERROR, "AllocRxRing[0] fail\n");
		return false;
	}

	/* Event Rx path */
	if (!halWpdmaAllocRxRing(prGlueInfo, RX_RING_EVT,
			 RX_RING1_SIZE, RXD_SIZE,
			 RX_BUFFER_AGGRESIZE, fgAllocMem)) {
		DBGLOG(HAL, ERROR, "AllocRxRing[1] fail\n");
		return false;
	}

	if (prBusInfo->wfdmaAllocRxRing) {
		if (!prBusInfo->wfdmaAllocRxRing(prGlueInfo, fgAllocMem)) {
			DBGLOG(HAL, ERROR, "wfdmaAllocRxRing fail\n");
			return false;
		}
	}

	/* Initialize all transmit related software queues */

	/* Init TX rings index pointer */
	for (u4Index = 0; u4Index < NUM_OF_TX_RING; u4Index++) {
		prHifInfo->TxRing[u4Index].TxSwUsedIdx = 0;
		prHifInfo->TxRing[u4Index].TxCpuIdx = 0;
#if CFG_SUPPORT_RX_WORK
		mutex_init(&prHifInfo->TxRing[u4Index].rTxDmaQMutex);
#else /* CFG_SUPPORT_RX_WORK */
		spin_lock_init(&prHifInfo->TxRing[u4Index].rTxDmaQLock);
#endif /* CFG_SUPPORT_RX_WORK */
	}

	return true;
}

void halWpdmaFreeRing(struct GLUE_INFO *prGlueInfo)
{
	struct mt66xx_chip_info *prChipInfo;
	struct GL_HIF_INFO *prHifInfo;
	struct HIF_MEM_OPS *prMemOps;
	struct RTMP_TX_RING *pTxRing;
	struct RTMP_RX_RING *pRxRing;
	struct TXD_STRUCT *pTxD;
	struct RTMP_DMACB *prDmaCb;
	void *pPacket, *pBuffer;
	uint32_t i, j;

	prChipInfo = prGlueInfo->prAdapter->chip_info;
	prHifInfo = &prGlueInfo->rHifInfo;
	prMemOps = &prHifInfo->rMemOps;

	/* Free Tx Ring Packet */
	for (i = 0; i < NUM_OF_TX_RING; i++) {
		pTxRing = &prHifInfo->TxRing[i];
		for (j = 0; j < pTxRing->u4RingSize; j++) {
			pTxD = (struct TXD_STRUCT *) (pTxRing->Cell[j].AllocVa);

			pPacket = pTxRing->Cell[j].pPacket;
			pBuffer = pTxRing->Cell[j].pBuffer;
			if (prMemOps->unmapTxBuf && pPacket)
				prMemOps->unmapTxBuf(
					prHifInfo, pTxRing->Cell[j].PacketPa,
					pTxD->SDLen0);
			pTxRing->Cell[j].pPacket = NULL;

			if (prMemOps->freeBuf && pBuffer)
				prMemOps->freeBuf(pBuffer, pTxD->SDLen0);
			pTxRing->Cell[j].pBuffer = NULL;
		}

		halWpdmaFreeRingDesc(prGlueInfo, &prHifInfo->TxDescRing[i]);
	}

	for (i = 0; i < NUM_OF_RX_RING; i++) {
		pRxRing = &prHifInfo->RxRing[i];
		for (j = 0; j < pRxRing->u4RingSize; j++) {
			prDmaCb = &pRxRing->Cell[j];
			if (prMemOps->unmapRxBuf && prDmaCb->DmaBuf.AllocVa) {
				prMemOps->unmapRxBuf(
					prHifInfo,
					prDmaCb->DmaBuf.AllocPa,
					prDmaCb->DmaBuf.AllocSize);
			}
			prDmaCb->DmaBuf.AllocVa = NULL;
			if (prMemOps->freePacket && prDmaCb->pPacket) {
				prMemOps->freePacket(
					prHifInfo, prDmaCb->pPacket, i);
			}
			prDmaCb->pPacket = NULL;
		}

		halWpdmaFreeRingDesc(prGlueInfo, &prHifInfo->RxDescRing[i]);
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
void halEnableFWDownload(IN struct ADAPTER *prAdapter, IN u_int8_t fgEnable)
{
	struct mt66xx_chip_info *prChipInfo;

	ASSERT(prAdapter);

	prChipInfo = prAdapter->chip_info;

	if (prChipInfo->asicEnableFWDownload)
		prChipInfo->asicEnableFWDownload(prAdapter, fgEnable);
}

u_int8_t halWpdmaWaitIdle(struct GLUE_INFO *prGlueInfo,
	int32_t round, int32_t wait_us)
{
	int32_t i = 0;
	union WPDMA_GLO_CFG_STRUCT GloCfg = {0};

	do {
		kalDevRegRead(prGlueInfo, WPDMA_GLO_CFG, &GloCfg.word);
		if ((GloCfg.field.TxDMABusy == 0) &&
		(GloCfg.field.RxDMABusy == 0)) {
			DBGLOG(HAL, TRACE,
				"==>  DMAIdle, GloCfg=0x%x\n", GloCfg.word);
			return TRUE;
		}
		kalUdelay(wait_us);
	} while ((i++) < round);

	DBGLOG(HAL, INFO, "==>  DMABusy, GloCfg=0x%x\n", GloCfg.word);

	return FALSE;
}

void halWpdmaInitRing(struct GLUE_INFO *prGlueInfo, bool fgResetHif)
{
	struct GL_HIF_INFO *prHifInfo;
	struct mt66xx_chip_info *prChipInfo;
	struct BUS_INFO *prBusInfo;
	struct WIFI_VAR *prWifiVar;

	ASSERT(prGlueInfo);

	prHifInfo = &prGlueInfo->rHifInfo;
	prChipInfo = prGlueInfo->prAdapter->chip_info;
	prBusInfo = prChipInfo->bus_info;
	prWifiVar = &prGlueInfo->prAdapter->rWifiVar;

	/* Set DMA global configuration except TX_DMA_EN and RX_DMA_EN bits */
	if (prBusInfo->pdmaSetup)
		prBusInfo->pdmaSetup(prGlueInfo, FALSE, fgResetHif);

	halWpdmaInitTxRing(prGlueInfo, fgResetHif);

	/* Init RX Ring0 Base/Size/Index pointer CSR */
	halWpdmaInitRxRing(prGlueInfo);

#if (CFG_SUPPORT_HOST_OFFLOAD == 1)
	if (IS_FEATURE_ENABLED(prWifiVar->fgEnableMawdTx))
		halMawdInitTxRing(prGlueInfo);

	if (IS_FEATURE_ENABLED(prWifiVar->fgEnableRro)) {
		halRroInit(prGlueInfo);
		if (IS_FEATURE_ENABLED(prWifiVar->fgEnableMawd))
			halMawdInitRxBlkRing(prGlueInfo);
	}
#endif /* CFG_SUPPORT_HOST_OFFLOAD == 1 */

	if (prBusInfo->wfdmaManualPrefetch)
		prBusInfo->wfdmaManualPrefetch(prGlueInfo);

	if (prBusInfo->pdmaSetup)
		prBusInfo->pdmaSetup(prGlueInfo, TRUE, fgResetHif);

	/* Write sleep mode magic num to dummy reg */
	if (prBusInfo->setDummyReg)
		prBusInfo->setDummyReg(prGlueInfo);
}

void halWpdmaInitTxRing(IN struct GLUE_INFO *prGlueInfo, bool fgResetHif)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct BUS_INFO *prBusInfo = NULL;
	struct mt66xx_chip_info *prChipInfo;
	struct RTMP_TX_RING *prTxRing = NULL;
	struct RTMP_DMACB *prTxCell;
	struct SW_WFDMA_INFO *prSwWfdmaInfo;
	uint32_t i = 0, offset = 0, phy_addr = 0, idx;

	prHifInfo = &prGlueInfo->rHifInfo;
	prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;
	prChipInfo = prGlueInfo->prAdapter->chip_info;
	prSwWfdmaInfo = &prBusInfo->rSwWfdmaInfo;

	/* reset all TX Ring register */
	for (i = 0; i < NUM_OF_TX_RING; i++) {
		prTxRing = &prHifInfo->TxRing[i];
		prTxCell = &prTxRing->Cell[0];

		idx = i;
		if (i == TX_RING_DATA0)
			idx = prBusInfo->tx_ring0_data_idx;
		else if (i == TX_RING_DATA1) {
			if (!prBusInfo->tx_ring1_data_idx)
				continue;
			idx = prBusInfo->tx_ring1_data_idx;
		} else if (i == TX_RING_DATA_PRIO) {
			if (!prBusInfo->tx_ring2_data_idx)
				continue;
			idx = prBusInfo->tx_ring2_data_idx;
		} else if (i == TX_RING_DATA_ALTX) {
			if (!prBusInfo->tx_ring3_data_idx)
				continue;
			idx = prBusInfo->tx_ring3_data_idx;
		} else if (i == TX_RING_CMD) {
			if (prSwWfdmaInfo->fgIsEnSwWfdma && !fgResetHif)
				continue;
			idx = prBusInfo->tx_ring_cmd_idx;
		} else if (i == TX_RING_FWDL)
			idx = prBusInfo->tx_ring_fwdl_idx;
		else if (i == TX_RING_WA_CMD)
			idx = prBusInfo->tx_ring_wa_cmd_idx;

		offset = idx * MT_RINGREG_DIFF;
		phy_addr = ((uint64_t)prTxCell->AllocPa) &
			DMA_LOWER_32BITS_MASK;
		prTxRing->TxSwUsedIdx = 0;
		prTxRing->u4UsedCnt = 0;
		prTxRing->TxCpuIdx = 0;

		prTxRing->hw_desc_base =
			prBusInfo->host_tx_ring_base + offset;

		prTxRing->hw_cidx_addr =
			prBusInfo->host_tx_ring_cidx_addr + offset;
		prTxRing->hw_cidx_mask = BITS(0, 12);
		prTxRing->hw_cidx_shift = 0;
		prTxRing->hw_didx_addr =
			prBusInfo->host_tx_ring_didx_addr + offset;
		prTxRing->hw_didx_mask = BITS(0, 12);
		prTxRing->hw_didx_shift = 0;
		prTxRing->hw_cnt_addr =
			prBusInfo->host_tx_ring_cnt_addr + offset;
		prTxRing->hw_cnt_mask = BITS(0, 12);
		prTxRing->hw_cnt_shift = 0;

		kalDevRegWrite(prGlueInfo, prTxRing->hw_desc_base, phy_addr);
		kalDevRegWrite(prGlueInfo, prTxRing->hw_cidx_addr,
			prTxRing->TxCpuIdx);
		kalDevRegWrite(prGlueInfo, prTxRing->hw_cnt_addr,
			prTxRing->u4RingSize);

		if (prBusInfo->tx_ring_ext_ctrl)
			prBusInfo->tx_ring_ext_ctrl(prGlueInfo, prTxRing, i);

		DBGLOG(HAL, TRACE, "-->TX_RING_%d[0x%x]: Base=0x%x, Cnt=%d!\n",
			i, prHifInfo->TxRing[i].hw_desc_base,
			phy_addr, prTxRing->u4RingSize);
	}
}

uint8_t halSetRxRingHwAddr(
	struct RTMP_RX_RING *prRxRing,
	struct BUS_INFO *prBusInfo,
	uint32_t u4SwRingIdx)
{
	uint32_t offset = 0;

	offset = u4SwRingIdx * MT_RINGREG_DIFF;
	prRxRing->hw_desc_base = prBusInfo->host_rx_ring_base + offset;
	prRxRing->hw_cidx_addr = prBusInfo->host_rx_ring_cidx_addr + offset;
	prRxRing->hw_cidx_mask = BITS(0, 12);
	prRxRing->hw_cidx_shift = 0;
	prRxRing->hw_didx_addr = prBusInfo->host_rx_ring_didx_addr + offset;
	prRxRing->hw_didx_mask = BITS(0, 12);
	prRxRing->hw_didx_shift = 0;
	prRxRing->hw_cnt_addr = prBusInfo->host_rx_ring_cnt_addr + offset;
	prRxRing->hw_cnt_mask = BITS(0, 12);
	prRxRing->hw_cnt_shift = 0;

	return TRUE;
}

void halWpdmaInitRxRing(IN struct GLUE_INFO *prGlueInfo)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct RTMP_RX_RING *prRxRing = NULL;
	uint32_t i = 0, phy_addr = 0;
	struct BUS_INFO *prBusInfo = NULL;
	struct mt66xx_chip_info *prChipInfo;
	uint8_t rv;
	struct RXD_STRUCT *pRxD;
	uint32_t j = 0;

	ASSERT(prGlueInfo);
	prHifInfo = &prGlueInfo->rHifInfo;
	prChipInfo = prGlueInfo->prAdapter->chip_info;
	prBusInfo = prChipInfo->bus_info;

	/* reset all RX Ring register */
	for (i = 0; i < NUM_OF_RX_RING; i++) {
		prRxRing = &prHifInfo->RxRing[i];
		if (prBusInfo->setRxRingHwAddr)
			rv = prBusInfo->setRxRingHwAddr(prRxRing, prBusInfo, i);
		else
			rv = halSetRxRingHwAddr(prRxRing, prBusInfo, i);

		if (!rv)
			break;
		phy_addr = ((uint64_t)prRxRing->Cell[0].AllocPa &
			DMA_LOWER_32BITS_MASK);
		prRxRing->RxCpuIdx = prRxRing->u4RingSize - 1;
		kalDevRegWrite(prGlueInfo, prRxRing->hw_desc_base, phy_addr);
		kalDevRegWrite(prGlueInfo, prRxRing->hw_cidx_addr,
			prRxRing->RxCpuIdx);
		kalDevRegWrite(prGlueInfo, prRxRing->hw_cnt_addr,
			prRxRing->u4RingSize);
		/* Reset DMADONE bit */
		for (j = 0; j < prRxRing->u4RingSize; j++) {
			pRxD = (struct RXD_STRUCT *) prRxRing->Cell[j].AllocVa;
			if (pRxD->DMADONE != 0) {
			/* There was a packet in rx_ring before WFDMA reinit!
			 * Clear done bit can prevent WFDMA coherence issue but
			 * the packe will loss after WFDMA reinit
			 */
				DBGLOG(HAL, WARN,
				"-->RX_RING_%d[%d]: DMADONE !=0\n", i, j);
				pRxD->DMADONE = 0;
			}
		}

		if (prBusInfo->rx_ring_ext_ctrl)
			prBusInfo->rx_ring_ext_ctrl(prGlueInfo, prRxRing, i);

		prRxRing->fgIsDumpLog = false;

		DBGLOG(HAL, TRACE, "-->RX_RING_%d[0x%x]: Base=0x%x, Cnt=%d\n",
			i, prRxRing->hw_desc_base,
			phy_addr, prRxRing->u4RingSize);
	}
}

void halWpdmaProcessCmdDmaDone(IN struct GLUE_INFO *prGlueInfo,
	IN uint16_t u2Port)
{
	struct mt66xx_chip_info *prChipInfo;
	struct BUS_INFO *prBusInfo;
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct HIF_MEM_OPS *prMemOps;
	struct SW_WFDMA_INFO *prSwWfdmaInfo;
	struct RTMP_TX_RING *prTxRing;
	struct TXD_STRUCT *pTxD;
	phys_addr_t PacketPa = 0;
	void *pBuffer = NULL;
	uint32_t u4SwIdx, u4DmaIdx = 0;

	KAL_HIF_TXRING_LOCK_DECLARATION();

	ASSERT(prGlueInfo);

	prChipInfo = prGlueInfo->prAdapter->chip_info;
	prBusInfo = prChipInfo->bus_info;
	prHifInfo = &prGlueInfo->rHifInfo;
	prMemOps = &prHifInfo->rMemOps;
	prSwWfdmaInfo = &prBusInfo->rSwWfdmaInfo;
	prTxRing = &prHifInfo->TxRing[u2Port];

	if (prTxRing->fgStopRecycleDmad)
		return;

	KAL_HIF_TXRING_LOCK(prTxRing);

	if (prSwWfdmaInfo->fgIsEnSwWfdma) {
		if (prSwWfdmaInfo->rOps.getDidx)
			prSwWfdmaInfo->rOps.getDidx(prGlueInfo, &u4DmaIdx);
		else
			DBGLOG(HAL, ERROR, "SwWfdma ops unsupported!");
	} else
		kalDevRegRead(prGlueInfo, prTxRing->hw_didx_addr, &u4DmaIdx);

	u4SwIdx = prTxRing->TxSwUsedIdx;

	while (u4SwIdx != u4DmaIdx) {
		pBuffer = prTxRing->Cell[u4SwIdx].pBuffer;
		PacketPa = prTxRing->Cell[u4SwIdx].PacketPa;
		pTxD = (struct TXD_STRUCT *) prTxRing->Cell[u4SwIdx].AllocVa;

		if (pTxD->DMADONE == 0)
			break;

		log_dbg(HAL, TRACE, "DMA done: port[%u] dma[%u] idx[%u] done[%u] pkt[0x%p] used[%u]\n",
			u2Port, u4DmaIdx, u4SwIdx, pTxD->DMADONE,
			prTxRing->Cell[u4SwIdx].pPacket, prTxRing->u4UsedCnt);

		if (prMemOps->unmapTxBuf && PacketPa)
			prMemOps->unmapTxBuf(prHifInfo, PacketPa, pTxD->SDLen0);

		pTxD->DMADONE = 0;
		if (prMemOps->freeBuf && pBuffer)
			prMemOps->freeBuf(pBuffer, 0);
		prTxRing->Cell[u4SwIdx].pBuffer = NULL;
		prTxRing->Cell[u4SwIdx].pPacket = NULL;
		prTxRing->u4UsedCnt--;

		GLUE_INC_REF_CNT(
			prGlueInfo->prAdapter->rHifStats.u4CmdTxdoneCount);

		INC_RING_INDEX(u4SwIdx, prTxRing->u4RingSize);
	}

	prTxRing->TxSwUsedIdx = u4SwIdx;

	KAL_HIF_TXRING_UNLOCK(prTxRing);

#if CFG_SUPPORT_MULTITHREAD
	if (!QUEUE_IS_EMPTY(&prGlueInfo->prAdapter->rTxCmdQueue))
		kalSetTxCmdEvent2Hif(prGlueInfo);
#endif

}

void halWpdmaProcessDataDmaDone(IN struct GLUE_INFO *prGlueInfo,
	IN uint16_t u2Port)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	uint32_t u4SwIdx, u4DmaIdx = 0, u4Diff = 0;
	struct RTMP_TX_RING *prTxRing;

#if !CFG_TX_DIRECT_VIA_HIF_THREAD
	KAL_HIF_TXRING_LOCK_DECLARATION();
#endif /* !CFG_TX_DIRECT_VIA_HIF_THREAD */

	ASSERT(prGlueInfo);

	prHifInfo = &prGlueInfo->rHifInfo;
	prTxRing = &prHifInfo->TxRing[u2Port];

#if !CFG_TX_DIRECT_VIA_HIF_THREAD
	KAL_HIF_TXRING_LOCK(prTxRing);
#endif /* !CFG_TX_DIRECT_VIA_HIF_THREAD */

	HAL_GET_RING_DIDX(prGlueInfo, prTxRing, &u4DmaIdx);
	u4SwIdx = prTxRing->TxSwUsedIdx;

	if (u4DmaIdx > u4SwIdx) {
		u4Diff = u4DmaIdx - u4SwIdx;
		prTxRing->u4UsedCnt -= u4Diff;
	} else if (u4DmaIdx < u4SwIdx) {
		u4Diff = (prTxRing->u4RingSize + u4DmaIdx) - u4SwIdx;
		prTxRing->u4UsedCnt -= u4Diff;
	} else {
		/* DMA index == SW used index */
		if (prTxRing->u4UsedCnt == prTxRing->u4RingSize) {
			u4Diff = prTxRing->u4RingSize;
			prTxRing->u4UsedCnt = 0;
		}
	}

	DBGLOG_LIMITED(HAL, TRACE,
		"DMA done: port[%u] dma[%u] idx[%u] used[%u]\n", u2Port,
		u4DmaIdx, u4SwIdx, prTxRing->u4UsedCnt);

	GLUE_ADD_REF_CNT(u4Diff,
			prGlueInfo->prAdapter->rHifStats.u4DataTxdoneCount);

	prTxRing->TxSwUsedIdx = u4DmaIdx;

#if !CFG_TX_DIRECT_VIA_HIF_THREAD
	KAL_HIF_TXRING_UNLOCK(prTxRing);
#endif /* !CFG_TX_DIRECT_VIA_HIF_THREAD */
}

uint32_t halWpdmaGetRxDmaDoneCnt(IN struct GLUE_INFO *prGlueInfo,
	IN uint8_t ucRingNum)
{
	struct mt66xx_chip_info *prChipInfo;
	struct RTMP_RX_RING *prRxRing;
	struct GL_HIF_INFO *prHifInfo;
	uint32_t u4MaxCnt = 0, u4CpuIdx = 0, u4DmaIdx = 0, u4RxPktCnt;

	prHifInfo = &prGlueInfo->rHifInfo;
	prChipInfo = prGlueInfo->prAdapter->chip_info;

	prRxRing = &prHifInfo->RxRing[ucRingNum];
	HAL_GET_RING_DIDX(prGlueInfo, prRxRing, &prRxRing->RxDmaIdx);
	u4MaxCnt = prRxRing->u4RingSize;
	u4CpuIdx = prRxRing->RxCpuIdx;
	u4DmaIdx = prRxRing->RxDmaIdx;

	if (u4MaxCnt == 0 || u4MaxCnt > RX_RING_SIZE)
		return 0;

	if (u4CpuIdx > u4DmaIdx)
		u4RxPktCnt = u4MaxCnt + u4DmaIdx - u4CpuIdx - 1;
	else if (u4CpuIdx < u4DmaIdx)
		u4RxPktCnt = u4DmaIdx - u4CpuIdx - 1;
	else
		u4RxPktCnt = u4MaxCnt - 1;

	return u4RxPktCnt;
}

enum ENUM_CMD_TX_RESULT halWpdmaWriteCmd(IN struct GLUE_INFO *prGlueInfo,
		      IN struct CMD_INFO *prCmdInfo, IN uint8_t ucTC)
{
	struct mt66xx_chip_info *prChipInfo;
	struct BUS_INFO *prBusInfo;
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct HIF_MEM_OPS *prMemOps;
	struct SW_WFDMA_INFO *prSwWfdmaInfo;
	struct RTMP_TX_RING *prTxRing;
	struct RTMP_DMACB *pTxCell;
	struct TXD_STRUCT *pTxD;
	uint16_t u2Port = TX_RING_CMD;
	uint32_t u4TotalLen;
	void *pucSrc = NULL;
	enum ENUM_CMD_TX_RESULT ret = CMD_TX_RESULT_SUCCESS;

	KAL_HIF_TXRING_LOCK_DECLARATION();

	ASSERT(prGlueInfo);

	prChipInfo = prGlueInfo->prAdapter->chip_info;
	prBusInfo = prChipInfo->bus_info;
	prHifInfo = &prGlueInfo->rHifInfo;
	prMemOps = &prHifInfo->rMemOps;
	prSwWfdmaInfo = &prBusInfo->rSwWfdmaInfo;

#if (CFG_SUPPORT_CONNAC2X == 1)
	if (prChipInfo->is_support_wacpu)
		u2Port = TX_RING_WA_CMD;
#endif /* CFG_SUPPORT_CONNAC2X == 1 */
	prTxRing = &prHifInfo->TxRing[u2Port];

#if !CFG_SUPPORT_RX_WORK
	KAL_HIF_BH_DISABLE(prGlueInfo);
#endif /* !CFG_SUPPORT_RX_WORK */

	KAL_HIF_TXRING_LOCK(prTxRing);

	u4TotalLen = prCmdInfo->u4TxdLen + prCmdInfo->u4TxpLen;
#if (CFG_SUPPORT_CONNAC2X == 1 || CFG_SUPPORT_CONNAC3X == 1)
	if (u4TotalLen > prChipInfo->cmd_max_pkt_size) {
		DBGLOG(HAL, ERROR,
			"type: %u, cid: 0x%x, seq: %u, txd: %u, txp: %u\n",
				prCmdInfo->eCmdType,
				prCmdInfo->ucCID,
				prCmdInfo->ucCmdSeqNum,
				prCmdInfo->u4TxdLen,
				prCmdInfo->u4TxpLen);
		if (prCmdInfo->u4TxdLen)
			DBGLOG_MEM32(HAL, ERROR, prCmdInfo->pucTxd,
				prCmdInfo->u4TxdLen);
		if (prCmdInfo->u4TxpLen)
			DBGLOG_MEM32(HAL, ERROR, prCmdInfo->pucTxp,
				prCmdInfo->u4TxpLen);
		ret = FALSE;
		goto unlock;
	}
#endif /* CFG_SUPPORT_CONNAC2X == 1 */
	if (prMemOps->allocRuntimeMem)
		pucSrc = prMemOps->allocRuntimeMem(u4TotalLen);

	if (prSwWfdmaInfo->fgIsEnSwWfdma) {
		if (prSwWfdmaInfo->rOps.getCidx)
			prSwWfdmaInfo->rOps.
				getCidx(prGlueInfo, &prTxRing->TxCpuIdx);
		else
			DBGLOG(HAL, ERROR, "SwWfdma ops unsupported!");
	} else
		kalDevRegRead(prGlueInfo, prTxRing->hw_cidx_addr,
			      &prTxRing->TxCpuIdx);

	if (prTxRing->TxCpuIdx >= prTxRing->u4RingSize) {
		DBGLOG(HAL, ERROR, "Error TxCpuIdx[%u]\n", prTxRing->TxCpuIdx);
		if (prMemOps->freeBuf)
			prMemOps->freeBuf(pucSrc, u4TotalLen);
		ret = CMD_TX_RESULT_FAILED;
		goto unlock;
	}

	pTxCell = &prTxRing->Cell[prTxRing->TxCpuIdx];
	pTxD = (struct TXD_STRUCT *)pTxCell->AllocVa;
	pTxCell->pPacket = (void *)prCmdInfo;
	pTxCell->pBuffer = pucSrc;

	if (prMemOps->copyCmd &&
	    !prMemOps->copyCmd(prHifInfo, pTxCell, pucSrc,
			       prCmdInfo->pucTxd, prCmdInfo->u4TxdLen,
			       prCmdInfo->pucTxp, prCmdInfo->u4TxpLen)) {
		if (prMemOps->freeBuf)
			prMemOps->freeBuf(pucSrc, u4TotalLen);
		ASSERT(0);
		ret = CMD_TX_RESULT_FAILED;
		goto unlock;
	}

	pTxD->SDPtr0 = (uint64_t)pTxCell->PacketPa & DMA_LOWER_32BITS_MASK;
#ifdef CONFIG_PHYS_ADDR_T_64BIT
	pTxD->SDPtr0Ext = ((uint64_t)pTxCell->PacketPa >> DMA_BITS_OFFSET) &
		DMA_HIGHER_4BITS_MASK;
#else
	pTxD->SDPtr0Ext = 0;
#endif
	pTxD->SDLen0 = u4TotalLen;
	pTxD->SDPtr1 = 0;
	pTxD->SDLen1 = 0;
	pTxD->LastSec0 = 1;
	pTxD->LastSec1 = 0;
	pTxD->Burst = 0;
	pTxD->DMADONE = 0;

#if (CFG_DUMP_TXDMAD == 1)
	DBGLOG(HAL, INFO, "Dump CMD TXDMAD: \n");
	DBGLOG_MEM8(HAL, INFO, (uint8_t *)pTxD, sizeof(struct TXD_STRUCT));
#endif
#if (CFG_DUMP_TXD == 1)
	DBGLOG(HAL, INFO, "Dump CMD TXD: \n");
	DBGLOG_MEM8(HAL, INFO, prCmdInfo->pucTxd, prCmdInfo->u4TxdLen);
#endif

	/* Increase TX_CTX_IDX, but write to register later. */
	INC_RING_INDEX(prTxRing->TxCpuIdx, prTxRing->u4RingSize);

	prTxRing->u4UsedCnt++;

	if (prSwWfdmaInfo->fgIsEnSwWfdma) {
		if (prSwWfdmaInfo->rOps.setCidx)
			prSwWfdmaInfo->rOps.
				setCidx(prGlueInfo, prTxRing->TxCpuIdx);
		else
			DBGLOG(HAL, ERROR, "SwWfdma ops unsupported!");
	} else
		kalDevRegWrite(prGlueInfo, prTxRing->hw_cidx_addr,
			       prTxRing->TxCpuIdx);

	GLUE_INC_REF_CNT(prGlueInfo->prAdapter->rHifStats.u4CmdTxCount);

	DBGLOG(HAL, TRACE,
	       "%s: CmdInfo[0x%p], TxD[0x%p/%u] TxP[0x%p/%u] CPU idx[%u] Used[%u]\n",
	       __func__, prCmdInfo, prCmdInfo->pucTxd, prCmdInfo->u4TxdLen,
	       prCmdInfo->pucTxp, prCmdInfo->u4TxpLen,
	       prTxRing->TxCpuIdx, prTxRing->u4UsedCnt);
	DBGLOG_MEM32(HAL, TRACE, prCmdInfo->pucTxd, prCmdInfo->u4TxdLen);

	if (u2Port == TX_RING_CMD
#if (CFG_SUPPORT_CONNAC2X == 1)
			|| u2Port == TX_RING_WA_CMD
#endif /* CFG_SUPPORT_CONNAC2 == 1 */
		)
		nicTxReleaseResource_PSE(prGlueInfo->prAdapter,
			TC4_INDEX,
			halTxGetCmdPageCount(prGlueInfo->prAdapter,
				pTxD->SDLen0,
				TRUE),
			TRUE);
unlock:
	KAL_HIF_TXRING_UNLOCK(prTxRing);

#if !CFG_SUPPORT_RX_WORK
	KAL_HIF_BH_ENABLE(prGlueInfo);
#endif /* !CFG_SUPPORT_RX_WORK */

	return ret;
}

static bool halWpdmaFillTxRing(struct GLUE_INFO *prGlueInfo,
			       struct MSDU_TOKEN_ENTRY *prToken)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct mt66xx_chip_info *prChipInfo;
	struct WIFI_VAR *prWifiVar;
	struct RTMP_TX_RING *prTxRing;
	struct RTMP_DMACB *pTxCell;
	struct TXD_STRUCT *pTxD;
	uint16_t u2Port = TX_RING_DATA0;

	ASSERT(prGlueInfo);

	prHifInfo = &prGlueInfo->rHifInfo;
	prChipInfo = prGlueInfo->prAdapter->chip_info;
	prWifiVar = &prGlueInfo->prAdapter->rWifiVar;

	u2Port = halTxRingDataSelect(
		prGlueInfo->prAdapter, prToken->prMsduInfo);
	prTxRing = &prHifInfo->TxRing[u2Port];

	pTxCell = &prTxRing->Cell[prTxRing->TxCpuIdx];
	prToken->u4CpuIdx = prTxRing->TxCpuIdx;
	prToken->u2Port = u2Port;
	pTxCell->prToken = prToken;

	pTxD = (struct TXD_STRUCT *)pTxCell->AllocVa;
	pTxD->SDPtr0 = (uint64_t)prToken->rDmaAddr & DMA_LOWER_32BITS_MASK;
#ifdef CONFIG_PHYS_ADDR_T_64BIT
	pTxD->SDPtr0Ext = ((uint64_t)prToken->rDmaAddr >> DMA_BITS_OFFSET) &
		DMA_HIGHER_4BITS_MASK;
#else
	pTxD->SDPtr0Ext = 0;
#endif
	pTxD->SDLen0 = NIC_TX_DESC_AND_PADDING_LENGTH +
		prChipInfo->txd_append_size;
	if (prChipInfo->is_support_cr4)
		pTxD->SDLen0 += HIF_TX_PAYLOAD_LENGTH;
#if (CFG_SUPPORT_HOST_OFFLOAD == 1)
	if (IS_FEATURE_ENABLED(prWifiVar->fgEnableSdo))
		pTxD->SDLen0 += HIF_TX_PAYLOAD_LENGTH;
#endif /* CFG_SUPPORT_HOST_OFFLOAD == 1 */
	pTxD->SDPtr1 = 0;
	pTxD->SDLen1 = 0;
	pTxD->LastSec0 = 1;
	pTxD->LastSec1 = 0;
	pTxD->Burst = 0;
	pTxD->DMADONE = 0;

#if (CFG_DUMP_TXDMAD == 1)
	DBGLOG(HAL, INFO, "Dump TXDMAD: \n");
	dumpMemory8((uint8_t *)pTxD, sizeof(struct TXD_STRUCT));
#endif

	/* Increase TX_CTX_IDX, but write to register later. */
	INC_RING_INDEX(prTxRing->TxCpuIdx, prTxRing->u4RingSize);

	/* Update HW Tx DMA ring */
	prTxRing->u4UsedCnt++;

	DBGLOG_LIMITED(HAL, TRACE,
		"Tx Data:Ring%d CPU idx[0x%x] Used[%u]\n",
		u2Port, prTxRing->TxCpuIdx, prTxRing->u4UsedCnt);

	GLUE_INC_REF_CNT(prGlueInfo->prAdapter->rHifStats.u4DataTxCount);

	return TRUE;
}

static bool halFlushToken(struct GLUE_INFO *prGlueInfo,
			  struct MSDU_TOKEN_ENTRY *prToken)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct HIF_MEM_OPS *prMemOps;

	ASSERT(prGlueInfo);
	prHifInfo = &prGlueInfo->rHifInfo;
	prMemOps = &prHifInfo->rMemOps;

	if (prMemOps->mapTxBuf) {
		prToken->rDmaAddr = prMemOps->mapTxBuf(
			prHifInfo, prToken->prPacket, 0, prToken->u4DmaLength);
		if (!prToken->rDmaAddr)
			return false;
	}

	return true;
}

static bool halWpdmaWriteData(struct GLUE_INFO *prGlueInfo,
			      struct MSDU_INFO *prMsduInfo,
			      struct MSDU_TOKEN_ENTRY *prFillToken,
			      struct MSDU_TOKEN_ENTRY *prToken,
			      uint32_t u4Idx, uint32_t u4Num)
{
	struct ADAPTER *prAdapter;
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct mt66xx_chip_info *prChipInfo;
	bool fgIsLast = (u4Idx + 1) == u4Num;

	ASSERT(prGlueInfo);
	prHifInfo = &prGlueInfo->rHifInfo;
	prAdapter = prGlueInfo->prAdapter;
	prChipInfo = prAdapter->chip_info;

	/* Update Tx descriptor */
	halTxUpdateCutThroughDesc(prGlueInfo, prMsduInfo, prFillToken,
				  prToken, u4Idx, fgIsLast);

	/* Update token exclude FillToken */
	if (prToken != prFillToken) {
		if (!halFlushToken(prGlueInfo, prToken))
			return false;
	}

	/* Update FillToken */
	if (fgIsLast) {
		if (!halFlushToken(prGlueInfo, prFillToken))
			return false;

#if (CFG_SUPPORT_HOST_OFFLOAD == 1)
		if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.fgEnableMawdTx))
			halMawdFillTxRing(prGlueInfo, prFillToken);
		else
#endif /* CFG_SUPPORT_HOST_OFFLOAD == 1 */
			halWpdmaFillTxRing(prGlueInfo, prFillToken);
	}

	return true;
}

#if CFG_SUPPORT_TASKLET_FREE_MSDU
void halWpdmaFreeMsduTasklet(unsigned long data)
{
	struct GLUE_INFO *prGlueInfo = (struct GLUE_INFO *)data;

#if CFG_SUPPORT_TX_FREE_MSDU_WORK
	/* do schedule work */
	kalTxFreeMsduWorkSchedule(prGlueInfo);
#else /* CFG_SUPPORT_TX_FREE_MSDU_WORK */
	/* just run it */
	halWpdmaFreeMsduWork(prGlueInfo);
#endif /* CFG_SUPPORT_TX_FREE_MSDU_WORK */
}

void halWpdmaFreeMsduWork(struct GLUE_INFO *prGlueInfo)
{
	struct MSDU_INFO *prMsduInfo;
	struct QUE *prTxMsduRetQue = &prGlueInfo->rTxMsduRetQueue;

	QUEUE_INITIALIZE(prTxMsduRetQue);

	while (KAL_FIFO_OUT(&prGlueInfo->rTxMsduRetFifo, prMsduInfo)) {
		if (!prMsduInfo) {
			DBGLOG(RX, ERROR, "prMsduInfo null\n");
			break;
		}
		halWpdmaFreeMsdu(prGlueInfo, prMsduInfo, FALSE);
	}

	prMsduInfo = (struct MSDU_INFO *)QUEUE_GET_HEAD(
			prTxMsduRetQue);
	if (prMsduInfo) {
		nicTxFreeMsduInfoPacketEx(prGlueInfo->prAdapter,
				prMsduInfo, FALSE);
		nicTxReturnMsduInfo(prGlueInfo->prAdapter, prMsduInfo);
	}
}
#endif /* CFG_SUPPORT_TASKLET_FREE_MSDU */

void halWpdmaFreeMsdu(struct GLUE_INFO *prGlueInfo,
		      struct MSDU_INFO *prMsduInfo,
		      bool fgSetEvent)
{
	DBGLOG(HAL, LOUD, "Tx Data: Msdu[0x%p], TokFree[%u] TxDone[%u]\n",
		prMsduInfo, halGetMsduTokenFreeCnt(prGlueInfo->prAdapter),
		(prMsduInfo->pfTxDoneHandler ? TRUE : FALSE));

	nicTxReleaseResource_PSE(prGlueInfo->prAdapter, prMsduInfo->ucTC,
		halTxGetCmdPageCount(prGlueInfo->prAdapter,
		prMsduInfo->u2FrameLength, TRUE), TRUE);

#if HIF_TX_PREALLOC_DATA_BUFFER
	if (!prMsduInfo->pfTxDoneHandler) {
#if CFG_SUPPORT_TASKLET_FREE_MSDU
		/* reduce locks */
		QUEUE_INSERT_TAIL(&prGlueInfo->rTxMsduRetQueue,
				  (struct QUE_ENTRY *) prMsduInfo);
#else /* CFG_SUPPORT_TASKLET_FREE_MSDU */
		nicTxFreePacket(prGlueInfo->prAdapter, prMsduInfo, FALSE);
		nicTxReturnMsduInfo(prGlueInfo->prAdapter, prMsduInfo);
#endif /* CFG_SUPPORT_TASKLET_FREE_MSDU */
	}
#endif

	if (!HAL_IS_TX_DIRECT(prGlueInfo->prAdapter) &&
		fgSetEvent && wlanGetTxPendingFrameCount(prGlueInfo->prAdapter))
		kalSetEvent(prGlueInfo);
}

bool halWpdmaWriteMsdu(struct GLUE_INFO *prGlueInfo,
		       struct MSDU_INFO *prMsduInfo,
		       struct list_head *prCurList)
{
	struct ADAPTER *prAdapter;
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct HIF_MEM_OPS *prMemOps;
	struct MSDU_TOKEN_ENTRY *prToken = NULL;
	struct sk_buff *prSkb;
	uint8_t *pucSrc = NULL;
	uint32_t u4TotalLen = 0;
	uint32_t u4TxDescAppendSize = 0;
	uint16_t u2Port;

	ASSERT(prGlueInfo);
	ASSERT(prMsduInfo);

	prAdapter = prGlueInfo->prAdapter;
	prHifInfo = &prGlueInfo->rHifInfo;
	prMemOps = &prHifInfo->rMemOps;
	u2Port = halTxRingDataSelect(prAdapter, prMsduInfo);
	u4TxDescAppendSize = wlanGetTxdAppendSize(prAdapter);

	prSkb = (struct sk_buff *)prMsduInfo->prPacket;
	if (prSkb) {
		pucSrc = prSkb->data;
		u4TotalLen = prSkb->len;
	}

	if (pucSrc == NULL || u4TotalLen == 0 ||
	    prMsduInfo->u2FrameLength == 0) {
		DBGLOG(HAL, ERROR, "prSkb=0x%p, frameLen=%d\n",
		       prSkb, prMsduInfo->u2FrameLength);

		if (prCurList) {
			list_del(prCurList);
			prHifInfo->u4TxDataQLen[u2Port]--;
		}
		halWpdmaFreeMsdu(prGlueInfo, prMsduInfo, true);

		return false;
	}

	if (u4TotalLen <= (HIF_TX_MAX_SIZE_PER_FRAME + u4TxDescAppendSize)) {

		/* Acquire MSDU token */
		prToken = halAcquireMsduToken(prAdapter,
					      prMsduInfo->ucBssIndex);
		if (!prToken) {
			DBGLOG(HAL, ERROR, "Write MSDU acquire token fail\n");
			return false;
		}

		/* Use MsduInfo to select TxRing */
		prToken->prMsduInfo = prMsduInfo;
		prToken->ucWlanIndex = prMsduInfo->ucWlanIndex;

#if HIF_TX_PREALLOC_DATA_BUFFER
		if (prMemOps->copyTxData)
			prMemOps->copyTxData(prToken, pucSrc, u4TotalLen);
#else
		prToken->prPacket = pucSrc;
		prToken->u4DmaLength = u4TotalLen;
		prMsduInfo->prToken = prToken;
#endif

		if (!halWpdmaWriteData(prGlueInfo, prMsduInfo, prToken,
			prToken, 0, 1)) {
			halReturnMsduToken(prAdapter, prToken->u4Token);
			return false;
		}

	} else {
		DBGLOG(HAL, ERROR, "u4Len=%u, 0x%p, txd_append_size=%d\n",
			u4TotalLen, prSkb,
			prAdapter->chip_info->txd_append_size);

		DBGLOG(HAL, ERROR, "%u,%u,%u,%u,%u,%u,%u,%u\n",
			prMsduInfo->eSrc, prMsduInfo->ucUserPriority,
			prMsduInfo->ucTC, prMsduInfo->ucPacketType,
			prMsduInfo->ucStaRecIndex, prMsduInfo->ucBssIndex,
			prMsduInfo->ucWlanIndex, prMsduInfo->ucPacketFormat);
	}

	if (prCurList) {
		list_del(prCurList);
		prHifInfo->u4TxDataQLen[u2Port]--;
	}

	if (!HAL_IS_TX_DIRECT(prAdapter))
		if (prMsduInfo->pfHifTxMsduDoneCb)
			prMsduInfo->pfHifTxMsduDoneCb(prAdapter, prMsduInfo);
#if CFG_SUPPORT_TASKLET_FREE_MSDU
	if (prMsduInfo->pfTxDoneHandler == NULL &&
		KAL_FIFO_IN(&prGlueInfo->rTxMsduRetFifo, prMsduInfo))
		kalTxFreeMsduTaskSchedule(prGlueInfo);
	else
#endif /* CFG_SUPPORT_TASKLET_FREE_MSDU */
	{
		halWpdmaFreeMsdu(prGlueInfo, prMsduInfo, true);
	}

	return true;
}

bool halWpdmaWriteAmsdu(struct GLUE_INFO *prGlueInfo,
			struct list_head *prList,
			uint32_t u4Num, uint16_t u2Size)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct HIF_MEM_OPS *prMemOps;
	struct RTMP_TX_RING *prTxRing;
	struct list_head *prCur, *prNext;
	struct TX_DATA_REQ *prTxReq;
	struct MSDU_TOKEN_ENTRY *prFillToken = NULL, *prToken = NULL;
	struct MSDU_INFO *prMsduInfo;
	struct AMSDU_MAC_TX_DESC *prTxD = NULL;
	struct sk_buff *prSkb;
	uint8_t *pucSrc;
	uint32_t u4TotalLen, u4Idx, u4FreeToken, u4FreeRing;
	uint16_t u2Port;
	bool fgIsLast;

	ASSERT(prGlueInfo);

	prHifInfo = &prGlueInfo->rHifInfo;
	prMemOps = &prHifInfo->rMemOps;
	u4FreeToken = halGetMsduTokenFreeCnt(prGlueInfo->prAdapter);

	/* Peek head to select TxRing */
	prTxReq = list_entry(prList, struct TX_DATA_REQ, list);
	u2Port = halTxRingDataSelect(
		prGlueInfo->prAdapter, prTxReq->prMsduInfo);
	prTxRing = &prHifInfo->TxRing[u2Port];
	u4FreeRing = prTxRing->u4RingSize - prTxRing->u4UsedCnt;

	if ((u4FreeToken < u4Num) || (u4FreeRing <= 1)) {
		DBGLOG(HAL, WARN,
		       "Amsdu low tx res acquire[%u], tok[%u], ring%d[%u]\n",
		       u4Num, u4FreeToken, u2Port, u4FreeRing);
		return false;
	}

	prCur = prList;
	for (u4Idx = 0; u4Idx < u4Num; u4Idx++) {
		prTxReq = list_entry(prCur, struct TX_DATA_REQ, list);
		prMsduInfo = prTxReq->prMsduInfo;
		prSkb = (struct sk_buff *)prMsduInfo->prPacket;
		pucSrc = prSkb->data;
		u4TotalLen = prSkb->len;
		fgIsLast = (u4Idx == u4Num - 1);

		/* Acquire MSDU token */
		prToken = halAcquireMsduToken(prGlueInfo->prAdapter,
					      prMsduInfo->ucBssIndex);
		if (!prToken) {
			DBGLOG(HAL, ERROR, "Write AMSDU acquire token fail\n");
			return false;
		}

		/* Use MsduInfo to select TxRing */
		prToken->prMsduInfo = prMsduInfo;
		prToken->ucWlanIndex = prMsduInfo->ucWlanIndex;

#if HIF_TX_PREALLOC_DATA_BUFFER
		if (prMemOps->copyTxData)
			prMemOps->copyTxData(prToken, pucSrc, u4TotalLen);
#else
		prToken->prPacket = pucSrc;
		prToken->u4DmaLength = u4TotalLen;
		prMsduInfo->prToken = prToken;
#endif

		if (!prFillToken) {
			prFillToken = prToken;
			prTxD = (struct AMSDU_MAC_TX_DESC *)prToken->prPacket;
		}

		if (fgIsLast) {
			prTxD->u2TxByteCount = u2Size;
			prTxD->u4DW1 |= TXD_DW1_AMSDU_C;
		}

		if (!halWpdmaWriteData(prGlueInfo, prMsduInfo, prFillToken,
				       prToken, u4Idx, u4Num)) {
			halReturnMsduToken(prGlueInfo->prAdapter,
					   prToken->u4Token);
			return false;
		}
		prCur = prCur->next;
	}

	prCur = prList;
	for (u4Idx = 0; u4Idx < u4Num; u4Idx++) {
		prNext = prCur->next;
		prTxReq = list_entry(prCur, struct TX_DATA_REQ, list);
		prMsduInfo = prTxReq->prMsduInfo;

		list_del(prCur);
		prHifInfo->u4TxDataQLen[u2Port]--;

		if (!HAL_IS_TX_DIRECT(prAdapter))
			if (prMsduInfo->pfHifTxMsduDoneCb)
				prMsduInfo->pfHifTxMsduDoneCb(
						prGlueInfo->prAdapter,
						prMsduInfo);
#if CFG_SUPPORT_TASKLET_FREE_MSDU
		if (prMsduInfo->pfTxDoneHandler == NULL &&
			KAL_FIFO_IN(&prGlueInfo->rTxMsduRetFifo, prMsduInfo))
			tasklet_schedule(&prGlueInfo->rTxMsduRetTask);
		else {
#endif /* CFG_SUPPORT_TASKLET_FREE_MSDU */
			halWpdmaFreeMsdu(prGlueInfo, prMsduInfo, true);
#if CFG_SUPPORT_TASKLET_FREE_MSDU
		}
#endif /* CFG_SUPPORT_TASKLET_FREE_MSDU */
		prCur = prNext;
	}

	DBGLOG(HAL, LOUD, "Amsdu num:%d tx byte: %d\n", u4Num, u2Size);
	return true;
}

static u_int8_t halIsStaticMapBusAddr(struct mt66xx_chip_info *prChipInfo,
				      uint32_t u4Addr)
{
	if (u4Addr < prChipInfo->bus_info->max_static_map_addr)
		return TRUE;
	else
		return FALSE;
}

u_int8_t halChipToStaticMapBusAddr(struct mt66xx_chip_info *prChipInfo,
				   uint32_t u4ChipAddr,
				   uint32_t *pu4BusAddr)
{
	struct BUS_INFO *prBusInfo = prChipInfo->bus_info;
	uint32_t u4StartAddr, u4EndAddr, u4BusAddr;
	uint32_t u4Idx = 0;

	if (halIsStaticMapBusAddr(prChipInfo, u4ChipAddr)) {
		*pu4BusAddr = u4ChipAddr;
		return TRUE;
	}

	while (TRUE) {
		u4StartAddr = prBusInfo->bus2chip[u4Idx].u4ChipAddr;
		u4EndAddr = prBusInfo->bus2chip[u4Idx].u4ChipAddr +
			prBusInfo->bus2chip[u4Idx].u4Range;

		/* End of mapping table */
		if (u4EndAddr == 0x0)
			return FALSE;

		if ((u4ChipAddr >= u4StartAddr) && (u4ChipAddr < u4EndAddr)) {
			u4BusAddr = (u4ChipAddr - u4StartAddr) +
				prBusInfo->bus2chip[u4Idx].u4BusAddr;
			break;
		}

		u4Idx++;
	}

	*pu4BusAddr = u4BusAddr;
	return TRUE;
}

u_int8_t halGetDynamicMapReg(IN struct GLUE_INFO *prGlueInfo,
			     IN uint32_t u4ChipAddr, OUT uint32_t *pu4Value)
{
	struct GL_HIF_INFO *prHifInfo = &prGlueInfo->rHifInfo;
	struct mt66xx_chip_info *prChipInfo;
	uint32_t u4ReMapReg, u4BusAddr;

	prChipInfo = prGlueInfo->prAdapter->chip_info;

	if (!halChipToStaticMapBusAddr(prChipInfo, MCU_CFG_PCIE_REMAP2,
				       &u4ReMapReg))
		return FALSE;


	RTMP_IO_WRITE32(prHifInfo, u4ReMapReg, u4ChipAddr & PCIE_REMAP2_MASK);
	u4BusAddr = PCIE_REMAP2_BUS_ADDR + (u4ChipAddr & ~PCIE_REMAP2_MASK);
	RTMP_IO_READ32(prHifInfo, u4BusAddr, pu4Value);

	return TRUE;
}

u_int8_t halSetDynamicMapReg(IN struct GLUE_INFO *prGlueInfo,
			     IN uint32_t u4ChipAddr, IN uint32_t u4Value)
{
	struct GL_HIF_INFO *prHifInfo = &prGlueInfo->rHifInfo;
	struct mt66xx_chip_info *prChipInfo;
	uint32_t u4ReMapReg, u4BusAddr;

	prChipInfo = prGlueInfo->prAdapter->chip_info;

	if (!halChipToStaticMapBusAddr(prChipInfo, MCU_CFG_PCIE_REMAP2,
				       &u4ReMapReg))
		return FALSE;

	RTMP_IO_WRITE32(prHifInfo, u4ReMapReg, u4ChipAddr & PCIE_REMAP2_MASK);
	u4BusAddr = PCIE_REMAP2_BUS_ADDR + (u4ChipAddr & ~PCIE_REMAP2_MASK);
	RTMP_IO_WRITE32(prHifInfo, u4BusAddr, u4Value);

	return TRUE;
}

u_int8_t halIsPendingRx(IN struct ADAPTER *prAdapter)
{
	/* TODO: check pending Rx
	 * if previous Rx handling is break due to lack of SwRfb
	 */
	return FALSE;
}

uint32_t halGetValidCoalescingBufSize(IN struct ADAPTER *prAdapter)
{
	uint32_t u4BufSize;

	if (HIF_TX_COALESCING_BUFFER_SIZE > HIF_RX_COALESCING_BUFFER_SIZE)
		u4BufSize = HIF_TX_COALESCING_BUFFER_SIZE;
	else
		u4BufSize = HIF_RX_COALESCING_BUFFER_SIZE;

	return u4BufSize;
}

uint32_t halAllocateIOBuffer(IN struct ADAPTER *prAdapter)
{
	return WLAN_STATUS_SUCCESS;
}

uint32_t halReleaseIOBuffer(IN struct ADAPTER *prAdapter)
{
	return WLAN_STATUS_SUCCESS;
}

void halProcessAbnormalInterrupt(IN struct ADAPTER *prAdapter)
{
	prAdapter->u4HifDbgFlag |= DEG_HIF_DEFAULT_DUMP;
	halPrintHifDbgInfo(prAdapter);
	halSetDrvSer(prAdapter);
	GL_DEFAULT_RESET_TRIGGER(prAdapter, RST_PROCESS_ABNORMAL_INT);
}

static void halDefaultProcessSoftwareInterrupt(
	IN struct ADAPTER *prAdapter)
{
	struct GLUE_INFO *prGlueInfo;
	struct GL_HIF_INFO *prHifInfo;
	struct ERR_RECOVERY_CTRL_T *prErrRecoveryCtrl;
	uint32_t u4Status = 0;

	if (prAdapter->prGlueInfo == NULL) {
		DBGLOG(HAL, ERROR, "prGlueInfo is NULL\n");
		return;
	}

	prGlueInfo = prAdapter->prGlueInfo;
	prHifInfo = &prGlueInfo->rHifInfo;
	prErrRecoveryCtrl = &prHifInfo->rErrRecoveryCtl;

	kalDevRegRead(prGlueInfo, MCU2HOST_SW_INT_STA, &u4Status);
	DBGLOG(HAL, INFO, "SER status[0x%x].\n", u4Status);
	prErrRecoveryCtrl->u4BackupStatus = u4Status;
	if (u4Status & ERROR_DETECT_MASK) {
		prErrRecoveryCtrl->u4Status = u4Status;
		kalDevRegWrite(prGlueInfo, MCU2HOST_SW_INT_STA,
			ERROR_DETECT_MASK);
		halHwRecoveryFromError(prAdapter);
	}
}

void halProcessSoftwareInterrupt(IN struct ADAPTER *prAdapter)
{
	struct BUS_INFO *prBusInfo = NULL;

	if (prAdapter == NULL) {
		DBGLOG(HAL, ERROR, "prAdapter is NULL\n");
		return;
	}

	prBusInfo = prAdapter->chip_info->bus_info;

	if (prBusInfo->processSoftwareInterrupt)
		prBusInfo->processSoftwareInterrupt(prAdapter);
	else
		halDefaultProcessSoftwareInterrupt(prAdapter);
}
#if KERNEL_VERSION(4, 15, 0) <= LINUX_VERSION_CODE
void halHwRecoveryTimeout(struct timer_list *timer)
#else
void halHwRecoveryTimeout(unsigned long arg)
#endif
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 15, 0))
	struct GL_HIF_INFO *prHif = from_timer(prHif, timer, rSerTimer);
	struct GLUE_INFO *prGlueInfo = (struct GLUE_INFO *)prHif->rSerTimerData;
#else
	struct GLUE_INFO *prGlueInfo = (struct GLUE_INFO *)arg;
#endif
	struct ADAPTER *prAdapter = NULL;
	struct GL_HIF_INFO *prHifInfo;
	struct ERR_RECOVERY_CTRL_T *prErrRecoveryCtrl;

	ASSERT(prGlueInfo);
	prAdapter = prGlueInfo->prAdapter;
	ASSERT(prAdapter);

	prHifInfo = &prGlueInfo->rHifInfo;
	prErrRecoveryCtrl = &prHifInfo->rErrRecoveryCtl;

	DBGLOG(HAL, ERROR,
	       "SER timer Timeout. ErrState[%d] Status[0x%x] Backup[0x%x]\n",
	       prErrRecoveryCtrl->eErrRecovState,
	       prErrRecoveryCtrl->u4Status,
	       prErrRecoveryCtrl->u4BackupStatus);

#if CFG_CHIP_RESET_SUPPORT
	kalSetSerTimeoutEvent(prGlueInfo);
#endif
}

void halSetDrvSer(struct ADAPTER *prAdapter)
{
	struct BUS_INFO *prBusInfo = NULL;
	struct mt66xx_chip_info *prChipInfo;

	ASSERT(prAdapter);
	ASSERT(prAdapter->prGlueInfo);

	prChipInfo = prAdapter->chip_info;
	prBusInfo = prAdapter->chip_info->bus_info;

	DBGLOG(HAL, INFO, "Set Driver Ser\n");
	if (prBusInfo->softwareInterruptMcu)
		prBusInfo->softwareInterruptMcu(prAdapter,
				MCU_INT_DRIVER_SER);
	else
		kalDevRegWrite(prAdapter->prGlueInfo, HOST2MCU_SW_INT_SET,
				MCU_INT_DRIVER_SER);
}

static void halStartSerTimer(IN struct ADAPTER *prAdapter)
{
	struct GLUE_INFO *prGlueInfo;
	struct GL_HIF_INFO *prHifInfo;

	prGlueInfo = prAdapter->prGlueInfo;
	prHifInfo = &prGlueInfo->rHifInfo;
	mod_timer(&prHifInfo->rSerTimer,
		  jiffies + HIF_SER_TIMEOUT * HZ / MSEC_PER_SEC);
	DBGLOG(HAL, INFO, "Start SER timer\n");
}

void halHwRecoveryFromError(IN struct ADAPTER *prAdapter)
{
	struct GLUE_INFO *prGlueInfo;
	struct mt66xx_chip_info *prChipInfo;
	struct GL_HIF_INFO *prHifInfo;
	struct BUS_INFO *prBusInfo = NULL;
	struct SW_WFDMA_INFO *prSwWfdmaInfo;
	struct WIFI_VAR *prWifiVar;
	struct ERR_RECOVERY_CTRL_T *prErrRecoveryCtrl;
	uint32_t u4Status = 0;

	prGlueInfo = prAdapter->prGlueInfo;
	prHifInfo = &prGlueInfo->rHifInfo;
	prChipInfo = prAdapter->chip_info;
	prBusInfo = prChipInfo->bus_info;
	prSwWfdmaInfo = &prBusInfo->rSwWfdmaInfo;
	prWifiVar = &prAdapter->rWifiVar;
	prErrRecoveryCtrl = &prHifInfo->rErrRecoveryCtl;

	u4Status = prErrRecoveryCtrl->u4Status;
	prErrRecoveryCtrl->u4Status = 0;

	if (prAdapter->rWifiVar.eEnableSerL1 !=
	     FEATURE_OPT_SER_ENABLE) {
		if (prChipInfo->asicDumpSerDummyCR)
			prChipInfo->asicDumpSerDummyCR(prAdapter);

		DBGLOG(HAL, WARN,
		       "[SER][L1] Bypass L1 reset due to wifi.cfg\n");

		GL_DEFAULT_RESET_TRIGGER(prAdapter, RST_SER_L1_FAIL);

		return;
	}

	switch (prErrRecoveryCtrl->eErrRecovState) {
	case ERR_RECOV_STOP_IDLE:
		if (u4Status & ERROR_DETECT_STOP_PDMA) {
			if (prChipInfo->asicDumpSerDummyCR)
				prChipInfo->asicDumpSerDummyCR(prAdapter);
			halStartSerTimer(prAdapter);
			DBGLOG(HAL, INFO,
				"SER(E) Host stop PDMA tx/rx ring operation & receive\n");
			nicSerStopTxRx(prAdapter);
#if (CFG_SUPPORT_CONNAC2X == 1)
			/*get WFDMA HW data before Layer 1 SER*/
			/*event*/
			if (prChipInfo->is_support_wfdma1)
				halRxReceiveRFBs(
					prAdapter, RX_RING_EVT, FALSE);
#endif
#if CFG_SUPPORT_MULTITHREAD
			kalSetRxProcessEvent(prAdapter->prGlueInfo);
			DBGLOG(HAL, INFO,
				"SER(F) kalSetRxProcessEvent\n");
#else
			DBGLOG(HAL, INFO,
				"SER(F) nicRxProcessRFBs\n");
			nicRxProcessRFBs(prAdapter);
#endif

			DBGLOG(HAL, INFO,
				"SER(F) Host ACK PDMA tx/rx ring stop operation\n");

			if (prBusInfo->softwareInterruptMcu) {
				prBusInfo->softwareInterruptMcu(prAdapter,
					MCU_INT_PDMA0_STOP_DONE);
			} else {
				kalDevRegWrite(prGlueInfo, HOST2MCU_SW_INT_SET,
					MCU_INT_PDMA0_STOP_DONE);
			}

			/* re-call for change status to stop dma0 */
			prErrRecoveryCtrl->eErrRecovState =
				ERR_RECOV_STOP_PDMA0;
		} else {
			DBGLOG(HAL, ERROR, "SER CurStat=%u Event=%x\n",
			       prErrRecoveryCtrl->eErrRecovState, u4Status);
		}
		break;

	case ERR_RECOV_STOP_PDMA0:
		if (u4Status & ERROR_DETECT_RESET_DONE) {
			DBGLOG(HAL, INFO, "SER(L) Host re-initialize PDMA\n");

			if (prSwWfdmaInfo->rOps.backup)
				prSwWfdmaInfo->rOps.backup(prGlueInfo);

			if (prSwWfdmaInfo->rOps.reset)
				prSwWfdmaInfo->rOps.reset(prSwWfdmaInfo);

			if (prBusInfo->DmaShdlInit)
				prBusInfo->DmaShdlInit(prAdapter);

			if (prBusInfo->DmaShdlReInit)
				prBusInfo->DmaShdlReInit(prAdapter);
#if (CFG_SUPPORT_HOST_OFFLOAD == 1)
			DBGLOG(HAL, INFO, "SER(M) Reset Host Offload\n");
			if (IS_FEATURE_ENABLED(prWifiVar->fgEnableRro)) {
				halRroResetRcbList(prGlueInfo);
				halRroResetMem(prGlueInfo);
			}
			if (IS_FEATURE_ENABLED(prWifiVar->fgEnableMawd))
				halMawdReset(prGlueInfo);
#endif /* CFG_SUPPORT_HOST_OFFLOAD == 1 */

			/* only reset TXD & RXD */
			halWpdmaAllocRing(prAdapter->prGlueInfo, false);
			nicFreePendingTxMsduInfo(prAdapter, 0xFF,
					MSDU_REMOVE_BY_ALL);
			wlanClearPendingCommandQueue(prAdapter);
			halResetMsduToken(prAdapter);
			prAdapter->ulNoMoreRfb = 0;

			DBGLOG(HAL, INFO, "SER(M) Host enable PDMA\n");
			halWpdmaInitRing(prGlueInfo, false);

			/* reset SW value after InitRing */
			if (prChipInfo->asicWfdmaReInit)
				prChipInfo->asicWfdmaReInit(prAdapter);

			DBGLOG(HAL, INFO,
				"SER(N) Host interrupt MCU PDMA ring init done\n");
			prErrRecoveryCtrl->eErrRecovState =
				ERR_RECOV_RESET_PDMA0;
			if (prBusInfo->softwareInterruptMcu) {
				prBusInfo->softwareInterruptMcu(prAdapter,
					MCU_INT_PDMA0_INIT_DONE);
			} else {
				kalDevRegWrite(prGlueInfo, HOST2MCU_SW_INT_SET,
					MCU_INT_PDMA0_INIT_DONE);
			}
		} else {
			DBGLOG(HAL, ERROR, "SER CurStat=%u Event=%x\n",
			       prErrRecoveryCtrl->eErrRecovState, u4Status);
		}
		break;

	case ERR_RECOV_RESET_PDMA0:
		if (u4Status & ERROR_DETECT_RECOVERY_DONE) {
			DBGLOG(HAL, INFO,
				"SER(Q) Host interrupt MCU SER handle done\n");
			prErrRecoveryCtrl->eErrRecovState =
				ERR_RECOV_WAIT_MCU_NORMAL;
			if (prBusInfo->softwareInterruptMcu) {
				prBusInfo->softwareInterruptMcu(prAdapter,
					MCU_INT_PDMA0_RECOVERY_DONE);
			} else {
				kalDevRegWrite(prGlueInfo, HOST2MCU_SW_INT_SET,
					MCU_INT_PDMA0_RECOVERY_DONE);
			}
		} else {
			DBGLOG(HAL, ERROR, "SER CurStat=%u Event=%x\n",
			       prErrRecoveryCtrl->eErrRecovState, u4Status);
		}
		break;

	case ERR_RECOV_WAIT_MCU_NORMAL:
		if (u4Status & ERROR_DETECT_MCU_NORMAL_STATE) {
			del_timer_sync(&prHifInfo->rSerTimer);

			/* update Beacon frame if operating in AP mode. */
			DBGLOG(HAL, INFO, "SER(T) Host re-initialize BCN\n");
			nicSerReInitBeaconFrame(prAdapter);

			kalDevKickCmd(prAdapter->prGlueInfo);
			kalDevKickData(prAdapter->prGlueInfo);
			halRxReceiveRFBs(prAdapter, RX_RING_EVT, FALSE);
			halRxReceiveRFBs(prAdapter, RX_RING_DATA0, TRUE);
			nicSerStartTxRx(prAdapter);
#if CFG_SUPPORT_MULTITHREAD
			kalSetTxEvent2Hif(prAdapter->prGlueInfo);
#endif
			prErrRecoveryCtrl->eErrRecovState = ERR_RECOV_STOP_IDLE;

			if (prSwWfdmaInfo->rOps.restore)
				prSwWfdmaInfo->rOps.restore(prGlueInfo);
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

void halDeAggRxPktWorker(struct work_struct *work)
{

}

void halRxTasklet(unsigned long data)
{
	struct GLUE_INFO *prGlueInfo = (struct GLUE_INFO *)data;
	halRxWork(prGlueInfo);
}

void halRxWork(struct GLUE_INFO *prGlueInfo)
{
	bool fgEnInt = FALSE;

	if (!HAL_IS_RX_DIRECT(prGlueInfo->prAdapter)) {
		DBGLOG(INIT, ERROR,
		       "Valid in RX-direct mode only\n");
		return;
	}

	/* do nothing if wifi is not ready */
	if (prGlueInfo->fgRxTaskReady == FALSE) {
		DBGLOG_LIMITED(INIT, INFO,
		       "Not ready yet, ignore pending interrupt\n");
		return;
	}

	ACQUIRE_POWER_CONTROL_FROM_PM(prGlueInfo->prAdapter);

	fgEnInt = KAL_TEST_AND_CLEAR_BIT(
			GLUE_FLAG_RX_DIRECT_INT_BIT,
			prGlueInfo->ulFlag);

	if (prGlueInfo->ulFlag & GLUE_FLAG_HALT
		|| kalIsResetting()
		) {
		/* Should stop now... skip pending interrupt */
		DBGLOG_LIMITED(INIT, INFO,
		       "ignore pending interrupt\n");
	} else {
		/* DBGLOG(INIT, INFO, ("HIF Interrupt!\n")); */
		prGlueInfo->TaskIsrCnt++;
		wlanIST(prGlueInfo->prAdapter, FALSE);
	}

#if CFG_SUPPORT_RX_WORK
	RX_INC_CNT(&prGlueInfo->prAdapter->rRxCtrl, RX_WORK_COUNT);
#else /* CFG_SUPPORT_RX_WORK */
	RX_INC_CNT(&prGlueInfo->prAdapter->rRxCtrl, RX_TASKLET_COUNT);
#endif /* CFG_SUPPORT_RX_WORK */

	if (kalRxTaskletWorkDone(prGlueInfo, fgEnInt)) {
		/* interrupt is not enabled, keep int bit */
		KAL_SET_BIT(GLUE_FLAG_RX_DIRECT_INT_BIT,
			prGlueInfo->ulFlag);
	}

	RECLAIM_POWER_CONTROL_TO_PM(prGlueInfo->prAdapter, FALSE);
}

void halTxCompleteTasklet(unsigned long data)
{

}

/* Hif power off wifi */
uint32_t halHifPowerOffWifi(IN struct ADAPTER *prAdapter)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	uint32_t rStatus = WLAN_STATUS_SUCCESS, u4Retry = 0;
	struct BUS_INFO *prBusInfo = NULL;
	struct SW_WFDMA_INFO *prSwWfdmaInfo;
	struct WIFI_VAR *prWifiVar;

	prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	prBusInfo = prAdapter->chip_info->bus_info;
	prSwWfdmaInfo = &prBusInfo->rSwWfdmaInfo;
	prWifiVar = &prAdapter->rWifiVar;

	DBGLOG(INIT, INFO, "Power off Wi-Fi!\n");

	ACQUIRE_POWER_CONTROL_FROM_PM(prAdapter);

#if defined(_HIF_AXI)
	if (nicProcessISTWithSpecifiedCount(prAdapter, 5) !=
		WLAN_STATUS_NOT_INDICATING)
		DBGLOG(INIT, INFO,
		       "Handle pending interrupt\n");

	while (prHifInfo->rErrRecoveryCtl.eErrRecovState !=
	       ERR_RECOV_STOP_IDLE) {
		if (u4Retry >= HIF_SER_POWER_OFF_RETRY_COUNT)
			break;
		kalMsleep(HIF_SER_POWER_OFF_RETRY_TIME);
		u4Retry++;
		nicProcessISTWithSpecifiedCount(prAdapter, 1);
		DBGLOG(INIT, INFO, "process SER...\n");
	}
#endif

	if (prBusInfo->setDmaIntMask)
		prBusInfo->setDmaIntMask(prAdapter->prGlueInfo,
			BIT(DMA_INT_TYPE_MCU2HOST) | BIT(DMA_INT_TYPE_TRX),
			FALSE);

	nicDisableInterrupt(prAdapter);

	/* Power off Wi-Fi */
	wlanSendNicPowerCtrlCmd(prAdapter, TRUE);

	if (prSwWfdmaInfo->fgIsEnSwWfdma &&
	    prSwWfdmaInfo->rOps.processDmaDone) {
		u4Retry = 0;
		while (prSwWfdmaInfo->rOps.processDmaDone(
			       prAdapter->prGlueInfo)) {
			if (u4Retry >= SW_WFDMA_MAX_RETRY_COUNT) {
				if (prSwWfdmaInfo->rOps.dumpDebugLog)
					prSwWfdmaInfo->rOps.dumpDebugLog(
						prAdapter->prGlueInfo);
				break;
			}
			DBGLOG(INIT, INFO, "Try to sent cmd to fw\n");
			kalMsleep(SW_WFDMA_RETRY_TIME);
			u4Retry++;
		}
	}

	prHifInfo->fgIsPowerOn = false;

#if (CFG_SUPPORT_HOST_OFFLOAD == 1)
	if (IS_FEATURE_ENABLED(prWifiVar->fgEnableMawd))
		halMawdReset(prAdapter->prGlueInfo);
#endif /* CFG_SUPPORT_HOST_OFFLOAD == 1 */

	/* prAdapter->fgWiFiInSleepyState = TRUE; */

	/* Wait til RDY bit has been cleared */
	rStatus = wlanCheckWifiFunc(prAdapter, FALSE);

	RECLAIM_POWER_CONTROL_TO_PM(prAdapter, FALSE);

	if (prBusInfo->disableSwInterrupt)
		prBusInfo->disableSwInterrupt(prAdapter);

	return rStatus;
}

u_int8_t halIsTxResourceControlEn(IN struct ADAPTER *prAdapter)
{
	return FALSE;
}

void halTxResourceResetHwTQCounter(IN struct ADAPTER *prAdapter)
{
}

static uint32_t halGetHifTxPageSize(IN struct ADAPTER *prAdapter)
{
	return HIF_TX_PAGE_SIZE;
}

uint32_t halGetHifTxDataPageSize(IN struct ADAPTER *prAdapter)
{
	return halGetHifTxPageSize(prAdapter);
}

uint32_t halGetHifTxCmdPageSize(IN struct ADAPTER *prAdapter)
{
	return halGetHifTxPageSize(prAdapter);
}

/*----------------------------------------------------------------------------*/
/*!
* @brief Check if HIF state is READY for upper layer cfg80211
*
* @param prGlueInfo      Pointer to the GlueInfo structure.
*
* @return (TRUE: ready, FALSE: not ready)
*/
/*----------------------------------------------------------------------------*/
u_int8_t halIsHifStateReady(IN struct GLUE_INFO *prGlueInfo, uint8_t *pucState)
{
	struct mt66xx_hif_driver_data *data = get_platform_driver_data();
	struct mt66xx_chip_info *chip = data->chip_info;

	return chip->CSRBaseAddress != NULL;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief Check if HIF state is during supend process
*
* @param prAdapter      Pointer to the Adapter structure.
*
* @return (TRUE: suspend, reject the caller action. FALSE: not suspend)
*/
/*----------------------------------------------------------------------------*/
bool halIsHifStateSuspend(IN struct ADAPTER *prAdapter)
{
	/* PCIE owner should implement this function */

	return FALSE;
}

void halUpdateTxMaxQuota(IN struct ADAPTER *prAdapter)
{
	struct BUS_INFO *prBusInfo;
	uint32_t u4Ret;
	uint32_t u4Quota;
	bool fgRun;
	uint8_t ucWmmIndex;

	KAL_SPIN_LOCK_DECLARATION();

	prBusInfo = prAdapter->chip_info->bus_info;

	for (ucWmmIndex = 0; ucWmmIndex < prAdapter->ucWmmSetNum;
		ucWmmIndex++) {

		u4Ret = WLAN_STATUS_SUCCESS;
		KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_UPDATE_WMM_QUOTA);
		u4Quota = prAdapter->rWmmQuotaReqCS[ucWmmIndex].u4Quota;
		fgRun = prAdapter->rWmmQuotaReqCS[ucWmmIndex].fgRun;
		KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_UPDATE_WMM_QUOTA);

		if (fgRun) {
			if (prBusInfo->updateTxRingMaxQuota) {
				u4Ret = prBusInfo->updateTxRingMaxQuota(
						prAdapter, ucWmmIndex, u4Quota);
			} else {
				DBGLOG(HAL, TRACE,
				"updateTxRingMaxQuota not implemented\n");
				u4Ret = WLAN_STATUS_NOT_ACCEPTED;
			}
		}
		DBGLOG(HAL, TRACE,
			"WmmQuota,Run,%u,Wmm,%u,Quota,0x%x,ret=0x%x\n",
			fgRun, ucWmmIndex, u4Quota, u4Ret);

		if (u4Ret != WLAN_STATUS_PENDING) {
			KAL_ACQUIRE_SPIN_LOCK(prAdapter,
				SPIN_LOCK_UPDATE_WMM_QUOTA);
			prAdapter->rWmmQuotaReqCS[ucWmmIndex].fgRun
				= false;
			KAL_RELEASE_SPIN_LOCK(prAdapter,
				SPIN_LOCK_UPDATE_WMM_QUOTA);
		}
	}
}

void halEnableSlpProt(struct GLUE_INFO *prGlueInfo)
{
	uint32_t u4Val = 0;
	uint32_t u4WaitDelay = 20000;

	kalDevRegRead(prGlueInfo, CONN_HIF_PDMA_CSR_PDMA_SLP_PROT_ADDR, &u4Val);
	u4Val |= CONN_HIF_PDMA_CSR_PDMA_SLP_PROT_PDMA_AXI_SLPPROT_ENABLE_MASK;
	kalDevRegWrite(prGlueInfo, CONN_HIF_PDMA_CSR_PDMA_SLP_PROT_ADDR, u4Val);
	while (TRUE) {
		u4WaitDelay--;
		kalDevRegRead(prGlueInfo, CONN_HIF_PDMA_CSR_PDMA_SLP_PROT_ADDR,
			&u4Val);
		if (CONN_HIF_PDMA_CSR_PDMA_SLP_PROT_PDMA_AXI_SLPPROT_RDY_MASK &
				u4Val)
			break;
		if (u4WaitDelay == 0) {
			DBGLOG(HAL, ERROR, "wait for sleep protect timeout.\n");
			GL_DEFAULT_RESET_TRIGGER(prGlueInfo->prAdapter,
						 RST_SLP_PROT_TIMEOUT);
			break;
		}
		kalUdelay(1);
	}
}

void halDisableSlpProt(struct GLUE_INFO *prGlueInfo)
{
	uint32_t u4Val = 0;

	kalDevRegRead(prGlueInfo, CONN_HIF_PDMA_CSR_PDMA_SLP_PROT_ADDR, &u4Val);
	u4Val &= ~CONN_HIF_PDMA_CSR_PDMA_SLP_PROT_PDMA_AXI_SLPPROT_ENABLE_MASK;
	kalDevRegWrite(prGlueInfo, CONN_HIF_PDMA_CSR_PDMA_SLP_PROT_ADDR, u4Val);
}

#if CFG_MTK_MDDP_SUPPORT
void halNotifyMdCrash(IN struct ADAPTER *prAdapter)
{
	struct mt66xx_chip_info *prChipInfo;
	struct BUS_INFO *prBusInfo;

	if (!prAdapter) {
		DBGLOG(HAL, ERROR, "Null prAdapter.\n");
		return;
	}

	prChipInfo = prAdapter->chip_info;
	prBusInfo = prChipInfo->bus_info;

	DBGLOG(HAL, INFO, "halNotifyMdCrash.\n");

	if (prBusInfo->softwareInterruptMcu) {
		prBusInfo->softwareInterruptMcu(
			prAdapter, MCU_INT_NOTIFY_MD_CRASH);
	} else {
		kalDevRegWrite(prAdapter->prGlueInfo, HOST2MCU_SW_INT_SET,
			       MCU_INT_NOTIFY_MD_CRASH);
	}
}
#endif

#if (CFG_TX_HIF_CREDIT_FEATURE == 1)
uint32_t halGetBssTxCredit(struct ADAPTER *prAdapter, uint8_t ucBssIndex)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct MSDU_TOKEN_INFO *prTokenInfo;

	prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	prTokenInfo = &prHifInfo->rTokenInfo;

	if (!prTokenInfo->fgEnAdjustCtrl)
		return prTokenInfo->u4MaxBssTxCredit;

	if (prTokenInfo->u4TxBssCnt[ucBssIndex] >
	    prTokenInfo->u4TxCredit[ucBssIndex])
		return 0;

	return prTokenInfo->u4TxCredit[ucBssIndex] -
		prTokenInfo->u4TxBssCnt[ucBssIndex];
}

static bool halIsHighCreditUsage(uint32_t u4Credit, uint32_t u4Used)
{
	return (u4Used * 100 / u4Credit) > HIF_TX_CREDIT_HIGH_USAGE;
}

static bool halIsLowCreditUsage(uint32_t u4Credit, uint32_t u4Used)
{
	return (u4Used * 100 / u4Credit) < HIF_TX_CREDIT_LOW_USAGE;
}

void halSetAdjustCtrlMode(struct ADAPTER *prAdapter,
	enum ENUM_WFD_ADJUST_CTRL_MODE u4Mode)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct MSDU_TOKEN_INFO *prTokenInfo;

	prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	prTokenInfo = &prHifInfo->rTokenInfo;

	if (prTokenInfo->u4EnAdjustCtrlMode != u4Mode)
		DBGLOG(HAL, INFO, "u4EnAdjustCtrlMode[%u].\n", u4Mode);

	prTokenInfo->u4EnAdjustCtrlMode = u4Mode;
}

void halSetAdjustCtrl(struct ADAPTER *prAdapter, bool fgEn)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct MSDU_TOKEN_INFO *prTokenInfo;

	prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	prTokenInfo = &prHifInfo->rTokenInfo;

	if (prTokenInfo->fgEnAdjustCtrl != fgEn)
		DBGLOG(HAL, INFO, "fgEnAdjustCtrl[%u].\n", fgEn);

	prTokenInfo->fgEnAdjustCtrl = fgEn;
}

#if (CFG_WFD_SCC_BALANCE_SUPPORT == 1)
int32_t halWFDBssBalanceGetBssCntCfg(struct ADAPTER *prAdapter)
{
	return prAdapter->rWifiVar.i4BssCount[0];
}

void halWFDBssBalanceSetBssCntCfg(struct ADAPTER *prAdapter,
	int32_t i4BssCntCfg, struct MSDU_TOKEN_INFO *prTokenInfo)
{
	uint32_t i = 0;

	if (i4BssCntCfg < 0)
		i4BssCntCfg = prTokenInfo->u4MinBssBalanceTxCredit;
	else if (i4BssCntCfg > prTokenInfo->u4MaxBssBalanceTxCredit)
		i4BssCntCfg = prTokenInfo->u4MaxBssBalanceTxCredit;
	else if (i4BssCntCfg < prTokenInfo->u4MinBssBalanceTxCredit)
		i4BssCntCfg = prTokenInfo->u4MinBssBalanceTxCredit;

	for (i = 0; i < MAX_BSSID_NUM; i++)
		prAdapter->rWifiVar.i4BssCount[i] = i4BssCntCfg;

	DBGLOG(REQ, INFO, "BssCntCfg ===> %ld", i4BssCntCfg);
}

void halWFDBssBalanceGetPreTxBW(struct ADAPTER *prAdapter,
	IN struct MSDU_TOKEN_INFO *prTokenInfo,
	OUT uint32_t *u4CurBitRate, OUT uint32_t *u4PredBitRate)
{
	uint32_t i = 0;
	*u4CurBitRate = 0;
	*u4PredBitRate = 0;

	for (i = 0; i < BSSID_NUM; i++)	{
		*u4CurBitRate += prTokenInfo->bitrate.au4CurrentBitrate[i];
		if (*u4PredBitRate < prTokenInfo->bitrate.au4PredictBitrate[i])
			*u4PredBitRate =
				prTokenInfo->bitrate.au4PredictBitrate[i];
	}

	DBGLOG(REQ, INFO, "CurR[%lu,%lu,%lu,%lu], PreR[%lu,%lu,%lu,%lu]",
		prTokenInfo->bitrate.au4CurrentBitrate[0],
		prTokenInfo->bitrate.au4CurrentBitrate[1],
		prTokenInfo->bitrate.au4CurrentBitrate[2],
		prTokenInfo->bitrate.au4CurrentBitrate[3],
		prTokenInfo->bitrate.au4PredictBitrate[0],
		prTokenInfo->bitrate.au4PredictBitrate[1],
		prTokenInfo->bitrate.au4PredictBitrate[2],
		prTokenInfo->bitrate.au4PredictBitrate[3]);
}

enum ENUM_WFD_BSS_BALANCE_STATE halWFDBssBalanceFsmHandleNoLimitState(
	struct ADAPTER *prAdapter, struct TX_LATENCY_STATS *report,
	struct MSDU_TOKEN_INFO *prTokenInfo)
{
	uint32_t u4ConsysAll = 0, u4Connsys[BSSID_NUM] = {0};
	uint32_t i = 0, j = 0;
	uint32_t u4CurBitRate = 0;
	uint32_t u4PredBitRate = 0;

	for (i = 0; i < BSSID_NUM; i++) {
		for (j = 0; j < LATENCY_STATS_MAX_SLOTS; j++) {
			u4ConsysAll += report->au4ConnsysLatency[i][j];
			u4Connsys[i] += report->au4ConnsysLatency[i][j];
		}
	}

	halWFDBssBalanceGetPreTxBW(prAdapter, prTokenInfo,
		&u4CurBitRate, &u4PredBitRate);

	DBGLOG(REQ, INFO,
		"C All: %lu, CurRate: %lu, PredRate: %lu",
		u4ConsysAll, u4CurBitRate, u4PredBitRate);


	/* C<10ms % < 80% */
	if (u4Connsys[0] < (u4ConsysAll*4/5) &&
		(u4CurBitRate > u4PredBitRate*5/10)) {
		DBGLOG(REQ, INFO,
			"C[0]:[%lu] > LowTH, FSM: [NO_LIMIT] -> [QUICK]",
			u4Connsys[0]);
		return WFD_BSS_BALANCE_QUICK_STATE;
	}

	DBGLOG(REQ, INFO,
		"C[0]:[%lu] < LowTH, FSM: [NO_LIMIT] -> [NO_LIMIT]",
		u4Connsys[0]);
	halWFDBssBalanceSetBssCntCfg(prAdapter,
		prTokenInfo->u4MaxBssBalanceTxCredit, prTokenInfo);
	return WFD_BSS_BALANCE_NO_LIMIT_STATE;
}

enum ENUM_WFD_BSS_BALANCE_STATE halWFDBssBalanceFsmHandleQuickState(
	struct ADAPTER *prAdapter, struct TX_LATENCY_STATS *report,
	struct MSDU_TOKEN_INFO *prTokenInfo)
{
	uint32_t u4ConsysAll = 0;
	uint32_t i = 0, j = 0;

	for (i = 0; i < BSSID_NUM; i++)
		for (j = 0; j < LATENCY_STATS_MAX_SLOTS; j++)
			u4ConsysAll += report->au4ConnsysLatency[i][j];

	DBGLOG(REQ, INFO, "C All: %lu", u4ConsysAll);

	/* set Credit = ring size * 1/2, then go to main state */
	halWFDBssBalanceSetBssCntCfg(prAdapter, TX_RING_SIZE/2, prTokenInfo);
	DBGLOG(REQ, INFO,
		"Cnt[%ld], FSM: [QUICK] -> [MAIN]",
		halWFDBssBalanceGetBssCntCfg(prAdapter));

	return WFD_BSS_BALANCE_MAIN_STATE;
}

enum ENUM_WFD_BSS_BALANCE_STATE halWFDBssBalanceFsmHandleMainState(
	struct ADAPTER *prAdapter, struct TX_LATENCY_STATS *report,
	struct MSDU_TOKEN_INFO *prTokenInfo)
{
	uint32_t u4ConsysAll = 0, u4Connsys[BSSID_NUM] = {0};
	uint32_t u4DriverAll = 0, u4Driver[BSSID_NUM] = {0};
	int32_t i4BssCntCfg = 0;
	uint32_t u4CurBitRate = 0;
	uint32_t u4PredBitRate = 0;
	uint32_t i = 0, j = 0;

	for (i = 0; i < BSSID_NUM; i++) {
		for (j = 0; j < LATENCY_STATS_MAX_SLOTS; j++) {
			u4ConsysAll += report->au4ConnsysLatency[i][j];
			u4DriverAll += report->au4DriverLatency[i][j];
			u4Connsys[i] += report->au4ConnsysLatency[i][j];
			u4Driver[i] += report->au4DriverLatency[i][j];
		}
	}

	halWFDBssBalanceGetPreTxBW(prAdapter, prTokenInfo,
		&u4CurBitRate, &u4PredBitRate);

	DBGLOG(REQ, INFO,
		"C All: %lu, D All: %lu, CurRate: %lu, PredRate: %lu",
		u4ConsysAll, u4DriverAll, u4CurBitRate, u4PredBitRate);

	/* if Actual TPUT <= Est TPUT*50% */
	if (u4CurBitRate < u4PredBitRate*5/10) {
		DBGLOG(REQ, INFO,
			"TPUT <= NoLimitTH, Cnt[%ld], FSM: [MAIN] -> [NO_LIMIT]",
			halWFDBssBalanceGetBssCntCfg(prAdapter));
		halWFDBssBalanceSetBssCntCfg(prAdapter,
			prTokenInfo->u4MaxBssBalanceTxCredit, prTokenInfo);
		return WFD_BSS_BALANCE_NO_LIMIT_STATE;
	}

	/* if C < 10ms % < 90%, Credit -= tx ring size * 1/8 */
	if (u4Connsys[0] < (u4ConsysAll*9/10)) {
		i4BssCntCfg = halWFDBssBalanceGetBssCntCfg(prAdapter);
		halWFDBssBalanceSetBssCntCfg(prAdapter,
			i4BssCntCfg - TX_RING_SIZE/8, prTokenInfo);
		DBGLOG(REQ, INFO,
			"C[0]:[%lu] > TH, Cnt[%ld], FSM: [MAIN] -> [MAIN]",
			u4Connsys[0],
			halWFDBssBalanceGetBssCntCfg(prAdapter));
		return WFD_BSS_BALANCE_MAIN_STATE;
	}

	/* if C < 10ms % > 90% & Actual TPUT < Estimate TPUT*80% */
	/* Credit += tx ring size * 1/8 */
	if ((u4Connsys[0] > (u4ConsysAll*9/10)) &&
		(u4CurBitRate < u4PredBitRate*8/10)) {
		i4BssCntCfg = halWFDBssBalanceGetBssCntCfg(prAdapter);
		halWFDBssBalanceSetBssCntCfg(prAdapter,
			i4BssCntCfg + TX_RING_SIZE/8, prTokenInfo);
		DBGLOG(REQ, INFO,
			"C[0]:[%lu] > TH and TPUT < TH, Cnt[%ld], FSM: [MAIN] -> [MAIN]",
			u4Connsys[0],
			halWFDBssBalanceGetBssCntCfg(prAdapter));
		return WFD_BSS_BALANCE_MAIN_STATE;
	}

	/* if C < 10ms % > 90%, go to step state */
	if ((u4Connsys[0] > (u4ConsysAll*9/10))
		&& (u4CurBitRate > u4PredBitRate*8/10)) {
		DBGLOG(REQ, INFO,
			"C[0]:[%lu] > TH, Cnt[%ld], FSM: [MAIN] -> [STEP]",
			u4Connsys[0],
			halWFDBssBalanceGetBssCntCfg(prAdapter));
		return WFD_BSS_BALANCE_STEP_STATE;
	}

	return WFD_BSS_BALANCE_MAIN_STATE;
}

enum ENUM_WFD_BSS_BALANCE_STATE halWFDBssBalanceFsmHandleStepState(
	struct ADAPTER *prAdapter, struct TX_LATENCY_STATS *report,
	struct MSDU_TOKEN_INFO *prTokenInfo)
{
	uint32_t u4ConsysAll = 0, u4Connsys[BSSID_NUM] = {0};
	uint32_t u4DriverAll = 0, u4Driver[BSSID_NUM] = {0};
	uint32_t u4CurBitRate = 0;
	uint32_t u4PredBitRate = 0;
	uint32_t i4BssCntCfg = 0;
	uint32_t i = 0, j = 0;

	for (i = 0; i < BSSID_NUM; i++) {
		for (j = 0; j < LATENCY_STATS_MAX_SLOTS; j++) {
			u4ConsysAll += report->au4ConnsysLatency[i][j];
			u4DriverAll += report->au4DriverLatency[i][j];
			u4Connsys[i] += report->au4ConnsysLatency[i][j];
			u4Driver[i] += report->au4DriverLatency[i][j];
		}
	}

	halWFDBssBalanceGetPreTxBW(prAdapter, prTokenInfo,
		&u4CurBitRate, &u4PredBitRate);

	DBGLOG(REQ, INFO,
		"C All: %lu, D All: %lu, CurRate: %lu, PreRate: %lu",
		u4ConsysAll, u4DriverAll, u4CurBitRate, u4PredBitRate);


	/* if Actual TPUT < Est TPUT*50% */
	if (u4CurBitRate <= u4PredBitRate*5/10) {
		DBGLOG(REQ, INFO,
			"TPUT <= NoLimitTH, Cnt[%ld], FSM: [STEP] -> [NO_LIMIT]",
			halWFDBssBalanceGetBssCntCfg(prAdapter));

		halWFDBssBalanceSetBssCntCfg(prAdapter,
			prTokenInfo->u4MaxBssBalanceTxCredit, prTokenInfo);

		return WFD_BSS_BALANCE_NO_LIMIT_STATE;
	}

	/* if C < 10ms < 90%, go to main state */
	if (u4Connsys[0] < (u4ConsysAll*9/10)) {
		DBGLOG(REQ, INFO,
			"C[0]:[%lu] > TH, Cnt[%ld], FSM: [STEP] -> [MAIN]",
			u4Connsys[0],
			halWFDBssBalanceGetBssCntCfg(prAdapter));

		return WFD_BSS_BALANCE_MAIN_STATE;
	}

	/* if Actual TPUT > Est TPUT * 95 */
	if (u4CurBitRate >= u4PredBitRate*95/100) {
		i4BssCntCfg = halWFDBssBalanceGetBssCntCfg(prAdapter);
		if ((u4Driver[0] + u4Driver[1])
			> (u4DriverAll*9/10)) {
			halWFDBssBalanceSetBssCntCfg(prAdapter,
				i4BssCntCfg - 10, prTokenInfo);
			DBGLOG(REQ, INFO,
				"D[0]+D[1]:[%lu] < HighTH, Cnt[%ld], FSM: [STEP] -> [STEP]",
				(u4Driver[0] + u4Driver[1]),
				halWFDBssBalanceGetBssCntCfg(prAdapter));
		} else {
			halWFDBssBalanceSetBssCntCfg(prAdapter,
				i4BssCntCfg + 10, prTokenInfo);
			DBGLOG(REQ, INFO,
				"D[0]+D[1]:[%lu] > HighTH, Cnt[%ld], FSM: [STEP] -> [STEP]",
				(u4Driver[0] + u4Driver[1]),
				halWFDBssBalanceGetBssCntCfg(prAdapter));
		}
		return WFD_BSS_BALANCE_STEP_STATE;
	}

	/* if Actual TPUT < Est TPUT * 95 */
	if (u4CurBitRate < u4PredBitRate*95/100) {
		i4BssCntCfg = halWFDBssBalanceGetBssCntCfg(prAdapter);
		halWFDBssBalanceSetBssCntCfg(prAdapter,
			i4BssCntCfg + 10, prTokenInfo);
		DBGLOG(REQ, INFO,
			"TPUT < HighTH, Cnt[%ld], FSM: [STEP] -> [STEP]",
			halWFDBssBalanceGetBssCntCfg(prAdapter));
		return WFD_BSS_BALANCE_STEP_STATE;
	}
#if 0
	/* if Actual TPUT > Est TPUT * 80 */
	if (u4CurBitRate > u4PredBitRate*8/10) {
		i4BssCntCfg = halWFDBssBalanceGetBssCntCfg(prAdapter);
		DBGLOG(REQ, INFO,
			"ENTER STEP STATE, sActual TPUT > Est TPUT * 90, goto STEP",
			halWFDBssBalanceGetBssCntCfg(prAdapter));
		return WFD_BSS_BALANCE_STEP_STATE;
	}
#endif

	return WFD_BSS_BALANCE_STEP_STATE;
}


void halWFDBssBalanceFsmSteps(IN struct ADAPTER *prAdapter,
	IN struct MSDU_TOKEN_INFO *prTokenInfo)
{
	enum ENUM_WFD_BSS_BALANCE_STATE u4NextState =
		WFD_BSS_BALANCE_NO_LIMIT_STATE;
	struct TX_LATENCY_REPORT_STATS *stats = &prAdapter->rMsduReportStats;
	struct TX_LATENCY_STATS *report = &stats->rDiff;

	if ((prAdapter->rWifiVar.u4WfdSccBalanceEnable == 0) ||
		prTokenInfo->u4EnAdjustCtrlMode != WFD_SCC_BALANCE_MODE) {
		if (u4NextState != WFD_BSS_BALANCE_NO_LIMIT_STATE) {
			halWFDBssBalanceSetBssCntCfg(prAdapter,
				prTokenInfo->u4MaxBssBalanceTxCredit,
				prTokenInfo);
			u4NextState = WFD_BSS_BALANCE_NO_LIMIT_STATE;
		}
		halSetAdjustCtrl(prAdapter, false);
		halSetAdjustCtrlMode(prAdapter, WFD_DEFAULT_MODE);
		goto END;
	}

	if (prAdapter->rWifiVar.u4WfdSccBalanceMode == 1) {
		halWFDBssBalanceSetBssCntCfg(prAdapter,
			prAdapter->rWifiVar.i4BssCount[0], prTokenInfo);
		u4NextState = WFD_BSS_BALANCE_FORCE_STATE;
		goto END;
	}

	switch (prTokenInfo->u4WFDBssBalanceState) {
	case WFD_BSS_BALANCE_NO_LIMIT_STATE:
		u4NextState =
			halWFDBssBalanceFsmHandleNoLimitState(prAdapter,
			report, prTokenInfo);
		break;
	case WFD_BSS_BALANCE_QUICK_STATE:
		u4NextState =
			halWFDBssBalanceFsmHandleQuickState(prAdapter,
			report, prTokenInfo);
		break;
	case WFD_BSS_BALANCE_MAIN_STATE:
		u4NextState =
			halWFDBssBalanceFsmHandleMainState(prAdapter,
			report, prTokenInfo);
		break;
	case WFD_BSS_BALANCE_STEP_STATE:
		u4NextState =
			halWFDBssBalanceFsmHandleStepState(prAdapter,
			report, prTokenInfo);
		break;
	case WFD_BSS_BALANCE_FORCE_STATE:
		u4NextState = WFD_BSS_BALANCE_FORCE_STATE;
		break;
	default:
		u4NextState = WFD_BSS_BALANCE_NO_LIMIT_STATE;
		break;
	}

END:
	prTokenInfo->u4WFDBssBalanceState = u4NextState;
}
#endif

void halAdjustBssTxCredit(struct ADAPTER *prAdapter, uint8_t ucBssIndex)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct MSDU_TOKEN_INFO *prTokenInfo;
	uint32_t u4Credit, u4Used, u4Delta = 0;

	prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	prTokenInfo = &prHifInfo->rTokenInfo;
	u4Credit = prTokenInfo->u4TxCredit[ucBssIndex];
	u4Used = prTokenInfo->u4TxBssCnt[ucBssIndex];

	if (!prTokenInfo->fgEnAdjustCtrl)
		return;

	if (prTokenInfo->u4LastTxBssCnt[ucBssIndex] > u4Used)
		u4Delta = prTokenInfo->u4LastTxBssCnt[ucBssIndex] - u4Used;

	if (u4Delta == 0)
		return;

#if (CFG_WFD_SCC_BALANCE_SUPPORT == 1)
	/* for SCC Balnance V1, set all bss the same bss count (credit) */
	if (prTokenInfo->u4EnAdjustCtrlMode == WFD_SCC_BALANCE_MODE &&
		prAdapter->rWifiVar.u4WfdSccBalanceEnable) {
		if (halWFDBssBalanceGetBssCntCfg(prAdapter))
			u4Credit = halWFDBssBalanceGetBssCntCfg(prAdapter);
	} else
#endif
	if (halIsLowCreditUsage(u4Credit, u4Used) &&
	    halIsHighCreditUsage(u4Credit, u4Delta)) {
		u4Credit += HIF_TX_CREDIT_STEP_COUNT;
	} else {
		if (u4Credit > HIF_TX_CREDIT_STEP_COUNT)
			u4Credit -= HIF_TX_CREDIT_STEP_COUNT;
	}

	if (prTokenInfo->u4EnAdjustCtrlMode == WFD_DEFAULT_MODE) {
		if (u4Credit > prTokenInfo->u4MaxBssTxCredit)
			u4Credit = prTokenInfo->u4MaxBssTxCredit;

		if (u4Credit < prTokenInfo->u4MinBssTxCredit)
			u4Credit = prTokenInfo->u4MinBssTxCredit;
	}


	if (u4Credit != prTokenInfo->u4TxCredit[ucBssIndex]) {
#if (CFG_WFD_SCC_BALANCE_SUPPORT == 1)
		DBGLOG(HAL, INFO,
		       "adjust tx credit Bss[%u], [%u]->[%u], used[%u], delta[%u]\n",
		       ucBssIndex, prTokenInfo->u4TxCredit[ucBssIndex],
		       u4Credit, u4Used, u4Delta);
#else
		DBGLOG(HAL, TRACE,
		       "adjust tx credit Bss[%u], [%u]->[%u], used[%u], delta[%u]\n",
		       ucBssIndex, prTokenInfo->u4TxCredit[ucBssIndex],
		       u4Credit, u4Used, u4Delta);
#endif
		prTokenInfo->u4TxCredit[ucBssIndex] = u4Credit;
	}

	prTokenInfo->u4LastTxBssCnt[ucBssIndex] =
		prTokenInfo->u4TxBssCnt[ucBssIndex];
}


u_int8_t halTxIsBssCreditCntFull(uint32_t u4TxCredit)
{
	if (u4TxCredit == 0)
		return TRUE;
	else
		return FALSE;
}

#endif

u_int8_t halTxIsBssCntFull(struct ADAPTER *prAdapter, uint8_t ucBssIndex)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct MSDU_TOKEN_INFO *prTokenInfo;
	uint8_t aucStrBuf[MAX_BSSID_NUM * 20];
	uint32_t u4Idx, u4Offset = 0;
	uint32_t u4DebugLevel = 0;

	ASSERT(prAdapter);
	ASSERT(prAdapter->prGlueInfo);

	prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	prTokenInfo = &prHifInfo->rTokenInfo;

	if (ucBssIndex >= MAX_BSSID_NUM ||
	    prTokenInfo->u4TxBssCnt[ucBssIndex] < prTokenInfo->u4MaxBssFreeCnt)
		return FALSE;

	wlanGetDriverDbgLevel(DBG_TX_IDX, &u4DebugLevel);

	if (u4DebugLevel & DBG_CLASS_TRACE) {
		kalMemZero(aucStrBuf, MAX_BSSID_NUM * 20);
		for (u4Idx = 0; u4Idx < MAX_BSSID_NUM; u4Idx++) {
			u4Offset += kalSprintf(
				aucStrBuf + u4Offset,
				u4Idx == 0 ? "%u" : ":%u",
				prTokenInfo->u4TxBssCnt[u4Idx]);
		}

		DBGLOG(HAL, TRACE, "Bss[%d] tx full, Cnt[%s]\n",
			ucBssIndex, aucStrBuf);
	}

	return TRUE;
}

static void halSetTxRingBssTokenCnt(struct ADAPTER *prAdapter, uint32_t u4Cnt)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct MSDU_TOKEN_INFO *prTokenInfo;

	ASSERT(prAdapter);
	ASSERT(prAdapter->prGlueInfo);

	prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	prTokenInfo = &prHifInfo->rTokenInfo;

	prTokenInfo->u4MaxBssFreeCnt = u4Cnt ? u4Cnt : HIF_TX_MSDU_TOKEN_NUM;

	DBGLOG(HAL, INFO, "SetTxRingBssTokenCnt=[%u].\n",
	       prTokenInfo->u4MaxBssFreeCnt);
}

#if (CFG_WFD_SCC_BALANCE_SUPPORT == 1)
enum ENUM_BAND halIsDualBandSccMode(IN struct ADAPTER *prAdapter)
{
	struct BSS_INFO *prBssInfo;
	uint32_t u4Idx, u4ActiveBssCnt[BAND_NUM] = {0};
	uint32_t u4AllActiveBssCnt = 0;
	uint8_t ucLastChNum[BAND_NUM] = {0};
	bool fgIsScc[BAND_NUM] = {0};
	enum ENUM_BAND u4SccBand = BAND_NULL;

	ASSERT(prAdapter);

	for (u4Idx = 0; u4Idx < MAX_BSSID_NUM; u4Idx++) {
		prBssInfo = prAdapter->aprBssInfo[u4Idx];

		if (IS_BSS_NOT_ALIVE(prAdapter, prBssInfo))
			continue;
		DBGLOG(HAL, INFO, "band:[%lu], channel:[%lu]\n",
			   prBssInfo->eBand, prBssInfo->ucPrimaryChannel);
		u4AllActiveBssCnt++;

		u4ActiveBssCnt[prBssInfo->eBand]++;
		if (ucLastChNum[prBssInfo->eBand] != 0 &&
			(ucLastChNum[prBssInfo->eBand] ==
			prBssInfo->ucPrimaryChannel) &&
			u4ActiveBssCnt[prBssInfo->eBand] == 2)
			fgIsScc[prBssInfo->eBand] = true;
		else
			fgIsScc[prBssInfo->eBand] = false;
		ucLastChNum[prBssInfo->eBand] = prBssInfo->ucPrimaryChannel;
	}

	for (u4Idx = 0; u4Idx < BAND_NUM; u4Idx++) {
		if (fgIsScc[u4Idx])
			u4SccBand = u4Idx;
	}
	DBGLOG(HAL, INFO, "ActiveBssCnt:[%lu], SccBand:[%lu]\n",
		   u4AllActiveBssCnt, u4SccBand);

	return u4SccBand;
}
#endif

void halUpdateBssTokenCnt(struct ADAPTER *prAdapter,
	uint8_t ucBssIndex)
{
	struct BSS_INFO *prBssInfo;
	struct WIFI_VAR *prWifiVar;

	prWifiVar = &prAdapter->rWifiVar;
	prBssInfo = prAdapter->aprBssInfo[ucBssIndex];

#if (CFG_TX_HIF_CREDIT_FEATURE == 1)
	if (prAdapter->rWifiVar.ucNSS == 1 && cnmIsMccMode(prAdapter)) {
		halSetAdjustCtrl(prAdapter, true);
		halSetAdjustCtrlMode(prAdapter, WFD_DEFAULT_MODE);
	} else if (prAdapter->ucAdjustCtrlBitmap) {
		halSetAdjustCtrl(prAdapter, true);
		halSetAdjustCtrlMode(prAdapter, WFD_DEFAULT_MODE);
	} else if (halIsDualBandSccMode(prAdapter)) {
#if (CFG_WFD_SCC_BALANCE_SUPPORT == 1)
		if (prAdapter->rWifiVar.u4WfdSccBalanceEnable == 1) {
			halSetAdjustCtrl(prAdapter, true);
			halSetAdjustCtrlMode(prAdapter, WFD_SCC_BALANCE_MODE);
		} else {
			halSetAdjustCtrl(prAdapter, false);
			halSetAdjustCtrlMode(prAdapter, WFD_DEFAULT_MODE);
		}
#else
		halSetAdjustCtrl(prAdapter, false);
		halSetAdjustCtrlMode(prAdapter, WFD_DEFAULT_MODE);
#endif
	} else {
		halSetAdjustCtrl(prAdapter, false);
		halSetAdjustCtrlMode(prAdapter, WFD_DEFAULT_MODE);
	}
#else
	if (prAdapter->rWifiVar.ucNSS == 1 && cnmIsMccMode(prAdapter))
		halSetTxRingBssTokenCnt(prAdapter, NIC_BSS_MCC_MODE_TOKEN_CNT);
	else if (prAdapter->ucAdjustCtrlBitmap)
		halSetTxRingBssTokenCnt(prAdapter, NIC_BSS_LOW_RATE_TOKEN_CNT);
	else
		halSetTxRingBssTokenCnt(prAdapter, HIF_TX_MSDU_TOKEN_NUM);
#endif
}

void halSetHifIntEvent(struct GLUE_INFO *pr, unsigned long ulBit)
{
	set_bit(ulBit, &pr->rHifInfo.ulIntFlag);
	kalSetDrvIntEvent(pr);
}

/**
 * diffTxDelayCounter - Counting diff from two 32-bit uint32_t array
 * @num: Number of items to count the diff
 * @diff: Returning the result
 * @value: Original value to be subtract
 * @remove: Subtract value to subtract
 */
static inline void diffTxDelayCounter(const size_t num, uint32_t *diff,
				      const uint32_t *value,
				      const uint32_t *remove)
{
	int i;

	for (i = 0; i < num; i++)
		*diff++ = *value++ - *remove++;
}

/**
 * composeTxDelayLog - Fill output log string to buffer
 * @buf: Base pointer of the buffer to be filled with
 * @pos: Position of the starting point to fill data in this round
 * @u4BufferSize: Buffer size of the caller provided buffer
 * @delayType: D/C/M/F for Dirver/Connsys/Mac/FailTx
 * @delayMax: Deliminators of TX delay latency
 * @delayValue: Counter of the measured delay MSDUs in each slot
 * @bss_num: number of report BSS groups, 1 or BSSID_NUM
 *
 * The buffer will be filled in the format like "D:[1:5:10:20]=[47:10:6:0:0]"
 * The former array is the value of max values for the statistics;
 * The latter array is the value of measured TX delay distribution.
 *
 * Return: The number of newly printed characters.
 */
static inline uint32_t composeTxDelayLog(char *buf, uint32_t pos,
		uint32_t u4BufferSize, const char *delayType,
		const uint32_t *delayMax, const uint32_t *delayValue,
		uint8_t bss_num)
{
	const uint32_t *delay;
	uint32_t delay_sum[LATENCY_STATS_MAX_SLOTS] = {0};
	int i;
	int b;

	u4BufferSize -= pos;
	buf += pos;
	pos = 0;

	pos += kalSnprintf(buf + pos, u4BufferSize - pos, "%s", delayType);

	delay = delayMax;
	for (i = 0; i < LATENCY_STATS_MAX_SLOTS-1; i++)
		pos += kalSnprintf(buf + pos, u4BufferSize - pos, "%s%u%s",
			i == 0 ? ":[" : ":", *delay++,
			i != LATENCY_STATS_MAX_SLOTS-2 ? "" : "]=");

	if (bss_num == 1) {
		delay = delayValue;
		for (b = 0; b < BSSID_NUM; b++)
			for (i = 0; i < LATENCY_STATS_MAX_SLOTS; i++)
				delay_sum[i] += *delay++;
	}

	delay = bss_num == BSSID_NUM ? delayValue : delay_sum;
	for (b = 0; b < bss_num; b++)
		for (i = 0; i < LATENCY_STATS_MAX_SLOTS; i++)
			pos += kalSnprintf(buf + pos, u4BufferSize - pos,
				"%s%u%s", b + i == 0 ? "[" : i == 0 ? "" : ":",
				*delay++, i == LATENCY_STATS_MAX_SLOTS - 1 ?
					b == bss_num - 1 ?  "] " : "," : "");

	return pos;
}

static void halDumpMsduReportStats(IN struct ADAPTER *prAdapter)
{
#if CFG_SUPPORT_TX_LATENCY_STATS
	static unsigned long next_update; /* in ms */
	unsigned long update_interval; /* in ms */
	struct TX_LATENCY_REPORT_STATS *stats = &prAdapter->rMsduReportStats;
	char *buf;
	uint32_t u4BufferSize = 512, pos = 0;
	struct WIFI_VAR *prWifiVar = &prAdapter->rWifiVar;
	struct TX_LATENCY_STATS rDiff = {0};
	struct TX_LATENCY_STATS *report = NULL;
	uint8_t report_num = 1; /* Default: sum up */

	if (!stats->fgTxLatencyEnabled || time_before(jiffies, next_update))
		return;

	buf = (char *)kalMemAlloc(u4BufferSize, VIR_MEM_TYPE);
	if (buf == NULL)
		return;
	kalMemZero(buf, u4BufferSize);

	/* By wifi.cfg first. If it is not set 1s by default; 100ms on more. */
	if (prWifiVar->u4MsduStatsUpdateInterval != 0)
		update_interval =
			prWifiVar->u4MsduStatsUpdateInterval * HZ / 1000;
	else {
		uint32_t u4DebugLevel = 0;

		wlanGetDriverDbgLevel(DBG_TX_IDX, &u4DebugLevel);
		update_interval = 1000 * HZ / 1000;
		if (u4DebugLevel & DBG_CLASS_TRACE)
			update_interval = 100 * HZ / 1000;
	}
	next_update = jiffies + update_interval;

	/* Set 'counting' to be reoprted by default.
	 * If KeepCounting is not set, replace the report with 'diff',
	 * in this case, use counting in report as counting base.
	 */
	report = &stats->rCounting;
	if (!prWifiVar->fgTxLatencyKeepCounting) {
		diffTxDelayCounter(BSSID_NUM * LATENCY_STATS_MAX_SLOTS,
			   rDiff.au4DriverLatency[0],
			   report->au4DriverLatency[0],
			   stats->rReported.au4DriverLatency[0]);
		diffTxDelayCounter(BSSID_NUM * LATENCY_STATS_MAX_SLOTS,
			   rDiff.au4ConnsysLatency[0],
			   report->au4ConnsysLatency[0],
			   stats->rReported.au4ConnsysLatency[0]);
		diffTxDelayCounter(BSSID_NUM * LATENCY_STATS_MAX_SLOTS,
			   rDiff.au4MacLatency[0],
			   report->au4MacLatency[0],
			   stats->rReported.au4MacLatency[0]);
		diffTxDelayCounter(BSSID_NUM * LATENCY_STATS_MAX_SLOTS,
			   rDiff.au4FailConnsysLatency[0],
			   report->au4FailConnsysLatency[0],
			   stats->rReported.au4FailConnsysLatency[0]);
		rDiff.u4TxFail = stats->rCounting.u4TxFail -
				 stats->rReported.u4TxFail;
		report = &rDiff;
	}
	stats->rReported = stats->rCounting;
#if (CFG_WFD_SCC_BALANCE_SUPPORT == 1)
	stats->rDiff = rDiff;
#endif

	if (prWifiVar->fgTxLatencyPerBss)
		report_num = BSSID_NUM;

	/* TX_Delay [%u:%u:%u:%u]=[%u:%u:%u:%u:%u,%u:%u:%u:%u:%u,...] */
	pos += kalSnprintf(buf + pos, u4BufferSize - pos, "TX_Delay ");
	pos += composeTxDelayLog(buf, pos, u4BufferSize, "D",
				 prWifiVar->au4DriverTxDelayMax,
				 report->au4DriverLatency[0], report_num);
	pos += composeTxDelayLog(buf, pos, u4BufferSize, "C",
				 prWifiVar->au4ConnsysTxDelayMax,
				 report->au4ConnsysLatency[0], report_num);
	pos += composeTxDelayLog(buf, pos, u4BufferSize, "M",
				 prWifiVar->au4MacTxDelayMax,
				 report->au4MacLatency[0], report_num);
	pos += composeTxDelayLog(buf, pos, u4BufferSize, "F",
				 prWifiVar->au4ConnsysTxFailDelayMax,
				 report->au4FailConnsysLatency[0], report_num);
	pos += kalSnprintf(buf + pos, u4BufferSize - pos, "Txfail:%u",
			report->u4TxFail);

	DBGLOG(HAL, INFO, "%s", buf);
	kalMemFree(buf, VIR_MEM_TYPE, u4BufferSize);
#endif
}

#if CFG_SUPPORT_LLS
#if (CFG_WFD_SCC_BALANCE_SUPPORT == 1)
void kalWFDBssBalanceGetPreTxBW(struct ADAPTER *prAdapter,
	OUT struct WFD_LLS_TX_BIT_RATE *outBitrate)
{
	struct GLUE_INFO *prGlueInfo;
	int32_t rStatus = -EOPNOTSUPP;
	union {
		struct CMD_GET_STATS_LLS cmd;
		struct WFD_LLS_TX_BIT_RATE bitrate;
	} query = {0};
	uint32_t u4QueryBufLen =
		sizeof(struct WFD_LLS_TX_BIT_RATE);
	uint32_t u4QueryInfoLen = sizeof(query.cmd);

	prGlueInfo = prAdapter->prGlueInfo;
	if (!prGlueInfo) {
		DBGLOG(REQ, INFO, "prGlueInfo null");
		return;
	}

	query.cmd.u4Tag = STATS_LLS_TAG_GET_WFD_PRED_TX_BITRATE;

	DBGLOG(REQ, TRACE, "cmd: u4Tag=%08x, args=%u/%u/%u/%u, len=%u",
			query.cmd.u4Tag, query.cmd.ucArg0, query.cmd.ucArg1,
			query.cmd.ucArg2, query.cmd.ucArg3, u4QueryInfoLen);


	rStatus = wlanSendSetQueryCmd(prAdapter,	/* prAdapter */
			    CMD_ID_GET_STATS_LLS,	/* ucCID */
			    FALSE,	/* fgSetQuery */
			    TRUE,	/* fgNeedResp */
			    FALSE,	/* fgIsOid */
			    nicCmdEventQueryLinkStats,    /* pfCmdDoneHandler */
			    nicOidCmdTimeoutCommon, /* pfCmdTimeoutHandler */
			    u4QueryInfoLen,    /* u4SetQueryInfoLen */
			    (uint8_t *)&query.cmd,  /* pucInfoBuffer */
			    (void *)outBitrate,       /* pvSetQueryBuffer */
			    u4QueryBufLen);   /* u4SetQueryBufferLen */

	if ((rStatus != WLAN_STATUS_SUCCESS &&
		rStatus != WLAN_STATUS_PENDING) ||
		(u4QueryInfoLen !=
		sizeof(struct WFD_LLS_TX_BIT_RATE))) {
		DBGLOG(REQ, WARN, "kalIoctl=%x, %u bytes",
				rStatus, u4QueryBufLen);
		rStatus = -EFAULT;
	}
}
#endif
#endif


void halDumpHifStats(IN struct ADAPTER *prAdapter)
{
	struct GLUE_INFO *prGlueInfo;
	struct HIF_STATS *prHifStats;
	struct GL_HIF_INFO *prHifInfo;
	struct RTMP_TX_RING *prTxRing;
	struct RTMP_RX_RING *prRxRing;
	struct RX_CTRL *prRxCtrl;
	struct TX_CTRL *prTxCtrl;
	uint8_t i = 0;
	uint32_t u4BufferSize = 512, pos = 0;
	char *buf;
#if (CFG_WFD_SCC_BALANCE_SUPPORT == 1)
	struct MSDU_TOKEN_INFO *prTokenInfo;
	struct TX_LATENCY_REPORT_STATS *prStats = &prAdapter->rMsduReportStats;
	struct TX_LATENCY_STATS *prLatencyReport;
#endif

	if (!prAdapter)
		return;

	prGlueInfo = prAdapter->prGlueInfo;
#if (CFG_WFD_SCC_BALANCE_SUPPORT == 1)
	prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	prTokenInfo = &prHifInfo->rTokenInfo;
#endif

#if (CFG_WFD_SCC_BALANCE_SUPPORT == 0)
	halDumpMsduReportStats(prAdapter);
#endif

#ifdef CFG_SUPPORT_SNIFFER_RADIOTAP
	if (prGlueInfo->fgIsEnableMon)
		return;
#endif
	prHifStats = &prAdapter->rHifStats;
	prHifInfo = &prGlueInfo->rHifInfo;
	prRxCtrl = &prAdapter->rRxCtrl;
	prTxCtrl = &prAdapter->rTxCtrl;

	if (time_before(jiffies, prHifStats->ulUpdatePeriod))
		return;

#if (CFG_WFD_SCC_BALANCE_SUPPORT == 1)
	halDumpMsduReportStats(prAdapter);
#endif

	buf = (char *) kalMemAlloc(u4BufferSize, VIR_MEM_TYPE);
	if (!buf)
		return;
	kalMemZero(buf, u4BufferSize);

	prHifStats->ulUpdatePeriod = jiffies +
			prAdapter->rWifiVar.u4PerfMonUpdatePeriod * HZ / 1000;

	pos += kalSnprintf(buf + pos, u4BufferSize - pos,
			"I[%u %u %u %u]",
			GLUE_GET_REF_CNT(prHifStats->u4HwIsrCount),
			GLUE_GET_REF_CNT(prHifStats->u4SwIsrCount),
			GLUE_GET_REF_CNT(prAdapter->fgIsIntEnable),
			GLUE_GET_REF_CNT(prGlueInfo->u4RxTaskScheduleCnt));
	pos += kalSnprintf(buf + pos, u4BufferSize - pos,
			" T[%u %u %u / %u %u %u %u]",
			GLUE_GET_REF_CNT(prHifStats->u4CmdInCount),
			GLUE_GET_REF_CNT(prHifStats->u4CmdTxCount),
			GLUE_GET_REF_CNT(prHifStats->u4CmdTxdoneCount),
			GLUE_GET_REF_CNT(prHifStats->u4DataInCount),
			GLUE_GET_REF_CNT(prHifStats->u4DataTxCount),
			GLUE_GET_REF_CNT(prHifStats->u4DataTxdoneCount),
			GLUE_GET_REF_CNT(prHifStats->u4DataMsduRptCount));
	pos += kalSnprintf(buf + pos, u4BufferSize - pos,
			" R[%u / %u]",
			GLUE_GET_REF_CNT(prHifStats->u4DataRxCount),
			GLUE_GET_REF_CNT(prHifStats->u4EventRxCount));
	for (i = 0; i < NUM_OF_TX_RING; ++i) {
		prTxRing = &prHifInfo->TxRing[i];
		pos += kalSnprintf(buf + pos, u4BufferSize - pos, "%s%u:%u%s",
				(i == 0) ? " T_Q:T_R[" : "",
				prHifInfo->u4TxDataQLen[i],
				prTxRing->u4UsedCnt,
				(i == NUM_OF_TX_RING - 1) ? "] " : " ");
	}
	for (i = 0; i < NUM_OF_RX_RING; ++i) {
		prRxRing = &prHifInfo->RxRing[i];
		pos += kalSnprintf(buf + pos, u4BufferSize - pos, "%s%u:%u%s",
				(i == 0) ? " R_R:R_N[" : "",
				prRxRing->u4PendingCnt,
				KAL_TEST_BIT(i, prAdapter->ulNoMoreRfb),
				(i == NUM_OF_RX_RING - 1) ? "]" : " ");
	}
	pos += kalSnprintf(buf + pos, u4BufferSize - pos,
			" Msdu[%u/%u] Tok[%u/%u] Rfb[%u/%u]",
			prTxCtrl->rFreeMsduInfoList.u4NumElem,
			CFG_TX_MAX_PKT_NUM,
			prHifInfo->rTokenInfo.u4UsedCnt,
			HIF_TX_MSDU_TOKEN_NUM,
			prRxCtrl->rFreeSwRfbList.u4NumElem,
			CFG_RX_MAX_PKT_NUM);
	pos += kalSnprintf(buf + pos, u4BufferSize - pos,
			" txreg[%u] rxreg[%u]",
			GLUE_GET_REF_CNT(prHifStats->u4TxDataRegCnt),
			GLUE_GET_REF_CNT(prHifStats->u4RxDataRegCnt)
			);
#if (CFG_SUPPORT_TX_DATA_DELAY == 1)
	pos += kalSnprintf(buf + pos, u4BufferSize - pos,
			" txdelay[0x%x]",
			prHifInfo->ulTxDataTimeout);
#endif /* CFG_SUPPORT_TX_DATA_DELAY == 1 */
	DBGLOG(HAL, INFO, "%s\n", buf);
	kalMemFree(buf, VIR_MEM_TYPE, u4BufferSize);

#if (CFG_WFD_SCC_BALANCE_SUPPORT == 1)
	if (prTokenInfo->u4EnAdjustCtrlMode == WFD_SCC_BALANCE_MODE &&
		prAdapter->rWifiVar.u4WfdSccBalanceEnable) {
#if CFG_SUPPORT_LLS
		kalWFDBssBalanceGetPreTxBW(prAdapter, &prTokenInfo->bitrate);
#endif
		prLatencyReport = &prStats->rDiff;
		DBGLOG(HAL, INFO,
			"WFD Credit: [%lu,%lu,%lu,%lu]\n",
			prTokenInfo->u4TxCredit[0], prTokenInfo->u4TxCredit[1],
			prTokenInfo->u4TxCredit[2], prTokenInfo->u4TxCredit[3]);
	}
	halWFDBssBalanceFsmSteps(prAdapter, prTokenInfo);
#endif
}

#if CFG_CHIP_RESET_SUPPORT
uint32_t halToggleWfsysRst(IN struct ADAPTER *prAdapter)
{
	if (!prAdapter) {
		DBGLOG(HAL, ERROR, "ADAPTER is NULL\n");
		return WLAN_STATUS_FAILURE;
	}

	/* TODO */

	return WLAN_STATUS_SUCCESS;
}
#endif /* CFG_CHIP_RESET_SUPPORT */

void halWpdmaStopRecycleDmad(IN struct GLUE_INFO *prGlueInfo,
				       IN uint16_t u2Port)
{
	struct GL_HIF_INFO *prHifInfo = &prGlueInfo->rHifInfo;
	struct RTMP_TX_RING *prTxRing = &prHifInfo->TxRing[u2Port];

	DBGLOG(HAL, INFO, "u2Port: %d\n", u2Port);

	prTxRing->fgStopRecycleDmad = TRUE;
}

uint32_t halSetSuspendFlagToFw(IN struct ADAPTER *prAdapter,
	IN u_int8_t fgSuspend)
{
	struct HOST_SUSPEND_NOTIFY_INFO *prNotifyInfo =
			&prAdapter->rHostSuspendInfo;
	uint32_t u4Value = 0;

	DBGLOG(HAL, TRACE,
		"fgSuspend: %d, type: %d, addr: 0x%x 0x%x, mask: 0x%x, shift: %d\n",
		fgSuspend,
		prNotifyInfo->eType,
		prNotifyInfo->u4SetAddr,
		prNotifyInfo->u4ClrAddr,
		prNotifyInfo->u4Mask,
		prNotifyInfo->u4Shift);

	if (prNotifyInfo->eType == ENUM_HOST_SUSPEND_ADDR_TYPE_EMI) {
#if CFG_MTK_ANDROID_EMI
		uint32_t u4SuspendFlag = (fgSuspend == TRUE) ?
			0x11111111 : 0x22222222;

		emi_mem_read(prAdapter->chip_info,
			prNotifyInfo->u4SetAddr,
			&u4Value,
			sizeof(u4Value));
		u4Value &= ~prNotifyInfo->u4Mask;
		u4Value |= ((u4SuspendFlag << prNotifyInfo->u4Shift) &
			prNotifyInfo->u4Mask);
		emi_mem_write(prAdapter->chip_info,
			prNotifyInfo->u4SetAddr,
			&u4SuspendFlag,
			sizeof(u4SuspendFlag));
#endif
	} else if (prNotifyInfo->eType ==
		   ENUM_HOST_SUSPEND_ADDR_TYPE_CONN_W_R_REG) {

		HAL_MCR_RD(prAdapter, prNotifyInfo->u4SetAddr, &u4Value);
		if (fgSuspend)
			u4Value |= prNotifyInfo->u4Mask;
		else
			u4Value &= ~prNotifyInfo->u4Mask;
		HAL_MCR_WR(prAdapter, prNotifyInfo->u4SetAddr, u4Value);
	} else if (prNotifyInfo->eType ==
		   ENUM_HOST_SUSPEND_ADDR_TYPE_CONN_SET_CLR_REG) {
		if (fgSuspend) {
			HAL_MCR_WR(prAdapter,
				prNotifyInfo->u4SetAddr,
				prNotifyInfo->u4Mask);
		} else {
			HAL_MCR_WR(prAdapter,
				prNotifyInfo->u4ClrAddr,
				prNotifyInfo->u4Mask);
		}
	}

	return WLAN_STATUS_SUCCESS;
}
