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
/*! \file   cmm_asic_connac3x.c
*    \brief  Internal driver stack will export the required procedures here for
* GLUE Layer.
*
*    This file contains all routines which are exported from MediaTek 802.11
* Wireless
*    LAN driver stack to GLUE Layer.
*/

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

#if (CFG_SUPPORT_CONNAC3X == 1)

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/

#include "precomp.h"
#include "wlan_lib.h"

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/
#define CONNAC3X_HIF_DMASHDL_BASE 0x52000000

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

#define USB_ACCESS_RETRY_LIMIT           1

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/
void asicConnac3xCapInit(
	struct ADAPTER *prAdapter)
{
	struct GLUE_INFO *prGlueInfo;
	struct mt66xx_chip_info *prChipInfo;
	struct BUS_INFO *prBusInfo = NULL;
	uint32_t u4HostWpdamBase = 0;

	ASSERT(prAdapter);
	if (prAdapter->chip_info->is_support_wfdma1)
		u4HostWpdamBase = CONNAC3X_HOST_WPDMA_1_BASE;
	else
		u4HostWpdamBase = CONNAC3X_HOST_WPDMA_0_BASE;

	prGlueInfo = prAdapter->prGlueInfo;
	prChipInfo = prAdapter->chip_info;
	prBusInfo = prChipInfo->bus_info;

	prChipInfo->u2HifTxdSize = 0;
	prChipInfo->u2TxInitCmdPort = 0;
	prChipInfo->u2TxFwDlPort = 0;
	prChipInfo->fillHifTxDesc = NULL;
	prChipInfo->u2CmdTxHdrSize = sizeof(struct CONNAC3X_WIFI_CMD);
	prChipInfo->asicFillInitCmdTxd = asicConnac3xFillInitCmdTxd;
	prChipInfo->asicFillCmdTxd = asicConnac3xFillCmdTxd;
#ifdef CFG_SUPPORT_UNIFIED_COMMAND
	prChipInfo->u2UniCmdTxHdrSize = sizeof(struct CONNAC3X_WIFI_UNI_CMD);
	prChipInfo->asicFillUniCmdTxd = asicConnac3xFillUniCmdTxd;
#endif
	prChipInfo->u2RxSwPktBitMap = CONNAC3X_RX_STATUS_PKT_TYPE_SW_BITMAP;
	prChipInfo->u2RxSwPktEvent = CONNAC3X_RX_STATUS_PKT_TYPE_SW_EVENT;
	prChipInfo->u2RxSwPktFrame = CONNAC3X_RX_STATUS_PKT_TYPE_SW_FRAME;
	prChipInfo->u4ExtraTxByteCount = 0;
	asicConnac3xInitTxdHook(prAdapter, prChipInfo->prTxDescOps);
	asicConnac3xInitRxdHook(prAdapter, prChipInfo->prRxDescOps);
#if (CFG_SUPPORT_MSP == 1)
	prChipInfo->asicRxProcessRxvforMSP = asicConnac3xRxProcessRxvforMSP;
#endif /* CFG_SUPPORT_MSP == 1 */
	prChipInfo->asicRxGetRcpiValueFromRxv =
			asicConnac3xRxGetRcpiValueFromRxv;
#if (CFG_SUPPORT_PERF_IND == 1)
	prChipInfo->asicRxPerfIndProcessRXV = asicConnac3xRxPerfIndProcessRXV;
#endif
#if (CFG_CHIP_RESET_SUPPORT == 1) && (CFG_WMT_RESET_API_SUPPORT == 0)
	prChipInfo->rst_L0_notify_step2 = conn2_rst_L0_notify_step2;
#endif
#if CFG_SUPPORT_WIFI_SYSDVT
	prAdapter->u2TxTest = TX_TEST_UNLIMITIED;
	prAdapter->u2TxTestCount = 0;
	prAdapter->ucTxTestUP = TX_TEST_UP_UNDEF;
#endif /* CFG_SUPPORT_WIFI_SYSDVT */

#if (CFG_SUPPORT_802_11AX == 1)
	if (fgEfuseCtrlAxOn == 1) {
		prAdapter->fgEnShowHETrigger = FALSE;
		heRlmInitHeHtcACtrlOMAndUPH(prAdapter);
	}
#endif
#if (CFG_SUPPORT_802_11BE == 1)
	ehtRlmInit(prAdapter);
#endif

	switch (prGlueInfo->u4InfType) {
#if defined(_HIF_PCIE) || defined(_HIF_AXI)
	case MT_DEV_INF_PCIE:
	case MT_DEV_INF_AXI:

		prChipInfo->u2TxInitCmdPort =
			TX_RING_CMD_IDX_3; /* Ring17 for CMD */
		prChipInfo->u2TxFwDlPort =
			TX_RING_FWDL_IDX_4; /* Ring16 for FWDL */
		prChipInfo->ucPacketFormat = TXD_PKT_FORMAT_TXD;
		prChipInfo->u4HifDmaShdlBaseAddr = CONNAC3X_HIF_DMASHDL_BASE;

		HAL_MCR_WR(prAdapter,
				CONNAC3X_BN0_IRQ_ENA_ADDR,
				BIT(0));

		if (prChipInfo->is_support_asic_lp) {
			HAL_MCR_WR(prAdapter,
				CONNAC3X_WPDMA_MCU2HOST_SW_INT_MASK
					(u4HostWpdamBase),
				BITS(0, 15));
		}
		break;
#endif /* _HIF_PCIE */
#if defined(_HIF_USB)
	case MT_DEV_INF_USB:
		prChipInfo->u2HifTxdSize = USB_HIF_TXD_LEN;
		prChipInfo->fillHifTxDesc = fillUsbHifTxDesc;
		prChipInfo->u2TxInitCmdPort = USB_DATA_BULK_OUT_EP8;
		prChipInfo->u2TxFwDlPort = USB_DATA_BULK_OUT_EP4;
		if (prChipInfo->is_support_wacpu)
			prChipInfo->ucPacketFormat = TXD_PKT_FORMAT_TXD;
		else
			prChipInfo->ucPacketFormat =
						TXD_PKT_FORMAT_TXD_PAYLOAD;
		prChipInfo->u4ExtraTxByteCount =
				EXTRA_TXD_SIZE_FOR_TX_BYTE_COUNT;
		prChipInfo->u4HifDmaShdlBaseAddr = USB_HIF_DMASHDL_BASE;

#if (CFG_ENABLE_FW_DOWNLOAD == 1)
		prChipInfo->asicEnableFWDownload = asicConnac3xEnableUsbFWDL;
#endif /* CFG_ENABLE_FW_DOWNLOAD == 1 */
		break;
#endif /* _HIF_USB */
	default:
		break;
	}
}

static void asicConnac3xFillInitCmdTxdInfo(
	struct ADAPTER *prAdapter,
	struct WIFI_CMD_INFO *prCmdInfo,
	u_int8_t *pucSeqNum)
{
	struct INIT_HIF_TX_HEADER *prInitHifTxHeader;
	struct HW_MAC_CONNAC3X_TX_DESC *prInitHifTxHeaderPadding;
	uint32_t u4TxdLen = sizeof(struct HW_MAC_CONNAC3X_TX_DESC);

	prInitHifTxHeaderPadding =
		(struct HW_MAC_CONNAC3X_TX_DESC *) (prCmdInfo->pucInfoBuffer);
	prInitHifTxHeader = (struct INIT_HIF_TX_HEADER *)
		(prCmdInfo->pucInfoBuffer + u4TxdLen);

	HAL_MAC_CONNAC3X_TXD_SET_TX_BYTE_COUNT(prInitHifTxHeaderPadding,
		prCmdInfo->u2InfoBufLen);
	if (!prCmdInfo->ucCID)
		HAL_MAC_CONNAC3X_TXD_SET_PKT_FORMAT(prInitHifTxHeaderPadding,
						INIT_PKT_FT_PDA_FWDL)
	else
		HAL_MAC_CONNAC3X_TXD_SET_PKT_FORMAT(prInitHifTxHeaderPadding,
						INIT_PKT_FT_CMD)
	HAL_MAC_CONNAC3X_TXD_SET_HEADER_FORMAT(prInitHifTxHeaderPadding,
						HEADER_FORMAT_COMMAND);
	HAL_MAC_CONNAC3X_TXD_SET_QUEUE_INDEX(prInitHifTxHeaderPadding, 0x20);

	prInitHifTxHeader->rInitWifiCmd.ucCID = prCmdInfo->ucCID;
	prInitHifTxHeader->rInitWifiCmd.ucPktTypeID = prCmdInfo->ucPktTypeID;
	prInitHifTxHeader->rInitWifiCmd.ucSeqNum =
		nicIncreaseCmdSeqNum(prAdapter);
	prInitHifTxHeader->u2TxByteCount =
		prCmdInfo->u2InfoBufLen - u4TxdLen;

	if (pucSeqNum)
		*pucSeqNum = prInitHifTxHeader->rInitWifiCmd.ucSeqNum;

	DBGLOG(INIT, INFO, "TX CMD: ID[0x%02X] SEQ[%u] LEN[%u]\n",
			prInitHifTxHeader->rInitWifiCmd.ucCID,
			prInitHifTxHeader->rInitWifiCmd.ucSeqNum,
			prCmdInfo->u2InfoBufLen);
}

static void asicConnac3xFillCmdTxdInfo(
	struct ADAPTER *prAdapter,
	struct WIFI_CMD_INFO *prCmdInfo,
	u_int8_t *pucSeqNum)
{
	struct CONNAC3X_WIFI_CMD *prWifiCmd;
	uint32_t u4TxdLen = sizeof(struct HW_MAC_CONNAC3X_TX_DESC);

	prWifiCmd = (struct CONNAC3X_WIFI_CMD *)prCmdInfo->pucInfoBuffer;

	HAL_MAC_CONNAC3X_TXD_SET_TX_BYTE_COUNT(
		(struct HW_MAC_CONNAC3X_TX_DESC *)prWifiCmd,
		prCmdInfo->u2InfoBufLen);
	HAL_MAC_CONNAC3X_TXD_SET_PKT_FORMAT(
		(struct HW_MAC_CONNAC3X_TX_DESC *)prWifiCmd,
		INIT_PKT_FT_CMD);
	HAL_MAC_CONNAC3X_TXD_SET_HEADER_FORMAT(
		(struct HW_MAC_CONNAC3X_TX_DESC *)prWifiCmd,
		HEADER_FORMAT_COMMAND);
	HAL_MAC_CONNAC3X_TXD_SET_QUEUE_INDEX(
		(struct HW_MAC_CONNAC3X_TX_DESC *)prWifiCmd, 0x20);

	prWifiCmd->ucCID = prCmdInfo->ucCID;
	prWifiCmd->ucExtenCID = prCmdInfo->ucExtCID;
	prWifiCmd->ucPktTypeID = prCmdInfo->ucPktTypeID;
	prWifiCmd->ucSetQuery = prCmdInfo->ucSetQuery;
	prWifiCmd->ucSeqNum = nicIncreaseCmdSeqNum(prAdapter);
	prWifiCmd->ucS2DIndex = prCmdInfo->ucS2DIndex;
	prWifiCmd->u2Length = prCmdInfo->u2InfoBufLen - u4TxdLen;

	if (pucSeqNum)
		*pucSeqNum = prWifiCmd->ucSeqNum;

	if (aucDebugModule[DBG_TX_IDX] & DBG_CLASS_TRACE)
		DBGLOG(TX, TRACE,
			"TX CMD: ID[0x%02X] SEQ[%u] SET[%u] LEN[%u]\n",
			prWifiCmd->ucCID, prWifiCmd->ucSeqNum,
			prWifiCmd->ucSetQuery, prCmdInfo->u2InfoBufLen);
	else
		DBGLOG_LIMITED(TX, INFO,
			"TX CMD: ID[0x%02X] SEQ[%u] SET[%u] LEN[%u]\n",
			prWifiCmd->ucCID, prWifiCmd->ucSeqNum,
			prWifiCmd->ucSetQuery, prCmdInfo->u2InfoBufLen);
}

#ifdef CFG_SUPPORT_UNIFIED_COMMAND
static void asicConnac3xFillUniCmdTxdInfo(
	struct ADAPTER *prAdapter,
	struct WIFI_UNI_CMD_INFO *prCmdInfo,
	u_int8_t *pucSeqNum)
{
	struct CONNAC3X_WIFI_UNI_CMD *prWifiCmd;
	uint32_t u4TxdLen = sizeof(struct HW_MAC_CONNAC3X_TX_DESC);

	prWifiCmd = (struct CONNAC3X_WIFI_UNI_CMD *)prCmdInfo->pucInfoBuffer;

	HAL_MAC_CONNAC3X_TXD_SET_TX_BYTE_COUNT(
		(struct HW_MAC_CONNAC3X_TX_DESC *)prWifiCmd,
		prCmdInfo->u2InfoBufLen);
	HAL_MAC_CONNAC3X_TXD_SET_PKT_FORMAT(
		(struct HW_MAC_CONNAC3X_TX_DESC *)prWifiCmd,
		INIT_PKT_FT_CMD);
	HAL_MAC_CONNAC3X_TXD_SET_HEADER_FORMAT(
		(struct HW_MAC_CONNAC3X_TX_DESC *)prWifiCmd,
		HEADER_FORMAT_COMMAND);
	HAL_MAC_CONNAC3X_TXD_SET_QUEUE_INDEX(
		(struct HW_MAC_CONNAC3X_TX_DESC *)prWifiCmd, 0x20);

	prWifiCmd->u2CID = prCmdInfo->u2CID;
	prWifiCmd->ucPktTypeID = prCmdInfo->ucPktTypeID;
	prWifiCmd->ucSeqNum = nicIncreaseCmdSeqNum(prAdapter);
	prWifiCmd->ucS2DIndex = prCmdInfo->ucS2DIndex;
	prWifiCmd->u2Length = prCmdInfo->u2InfoBufLen - u4TxdLen;
	prWifiCmd->ucOption = prCmdInfo->ucOption;

	if (pucSeqNum)
		*pucSeqNum = prWifiCmd->ucSeqNum;

	DBGLOG(TX, TRACE, "TX CMD: ID[0x%04X] SEQ[%u] OPT[0x%02X] LEN[%u]\n",
			prWifiCmd->u2CID, prWifiCmd->ucSeqNum,
			prWifiCmd->ucOption, prCmdInfo->u2InfoBufLen);
}

#endif

void asicConnac3xFillInitCmdTxd(
	struct ADAPTER *prAdapter,
	struct WIFI_CMD_INFO *prCmdInfo,
	u_int16_t *pu2BufInfoLen,
	u_int8_t *pucSeqNum, OUT void **pCmdBuf)
{
	struct INIT_HIF_TX_HEADER *prInitHifTxHeader;
	uint32_t u4TxdLen = sizeof(struct HW_MAC_CONNAC3X_TX_DESC);

	/* We don't need to append TXD while sending fw frames. */
	if (!prCmdInfo->ucCID && pCmdBuf)
		*pCmdBuf = prCmdInfo->pucInfoBuffer;
	else {
		prInitHifTxHeader = (struct INIT_HIF_TX_HEADER *)
			(prCmdInfo->pucInfoBuffer + u4TxdLen);
		asicConnac3xFillInitCmdTxdInfo(
			prAdapter,
			prCmdInfo,
			pucSeqNum);
		if (pCmdBuf)
			*pCmdBuf =
				&prInitHifTxHeader->rInitWifiCmd.aucBuffer[0];
	}
}

void asicConnac3xWfdmaDummyCrRead(
	struct ADAPTER *prAdapter,
	u_int8_t *pfgResult)
{
	u_int32_t u4RegValue = 0;

	HAL_MCR_RD(prAdapter,
		CONNAC3X_WFDMA_DUMMY_CR,
		&u4RegValue);
	*pfgResult = (u4RegValue &
		CONNAC3X_WFDMA_NEED_REINIT_BIT)
		== 0 ? TRUE : FALSE;
}


void asicConnac3xWfdmaDummyCrWrite(
	struct ADAPTER *prAdapter)
{
	u_int32_t u4RegValue = 0;

	HAL_MCR_RD(prAdapter,
		CONNAC3X_WFDMA_DUMMY_CR,
		&u4RegValue);
	u4RegValue |= CONNAC3X_WFDMA_NEED_REINIT_BIT;

	HAL_MCR_WR(prAdapter,
		CONNAC3X_WFDMA_DUMMY_CR,
		u4RegValue);
}

static void asicConnac3xWfdmaReInitImpl(struct ADAPTER *prAdapter)
{
#if defined(_HIF_PCIE) || defined(_HIF_AXI)
#if CFG_MTK_WIFI_WFDMA_BK_RS
	{
		struct BUS_INFO *prBusInfo;
		struct GL_HIF_INFO *prHifInfo;
		uint32_t u4Idx;

		prBusInfo = prAdapter->chip_info->bus_info;

		DBGLOG(INIT, TRACE,
			"WFDMA reinit after bk/sr(deep sleep)\n");
		prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
		for (u4Idx = 0; u4Idx < NUM_OF_TX_RING; u4Idx++) {
			prHifInfo->TxRing[u4Idx].TxSwUsedIdx = 0;
			prHifInfo->TxRing[u4Idx].u4UsedCnt = 0;
			prHifInfo->TxRing[u4Idx].TxCpuIdx = 0;
		}

		if (halWpdmaGetRxDmaDoneCnt(prAdapter->prGlueInfo,
					    RX_RING_EVT_IDX_1)) {
			prAdapter->u4NoMoreRfb |=
				BIT(RX_RING_EVT_IDX_1);
		}
	}
#else /* CFG_MTK_WIFI_WFDMA_BK_RS */
	DBGLOG(INIT, INFO, "WFDMA reinit due to deep sleep\n");
	halWpdmaInitRing(prAdapter->prGlueInfo, true);
#endif /* CFG_MTK_WIFI_WFDMA_BK_RS */
#elif defined(_HIF_USB)
	{
		struct mt66xx_chip_info *prChipInfo = NULL;

		prChipInfo = prAdapter->chip_info;
		if (prChipInfo->is_support_asic_lp &&
		    prChipInfo->asicUsbInit)
			prChipInfo->asicUsbInit(prAdapter, prChipInfo);

		if (prChipInfo->is_support_wacpu) {
			/* command packet forward to TX ring 17 (WMCPU) or
			 *	  TX ring 20 (WACPU)
			 */
			asicConnac2xEnableUsbCmdTxRing(prAdapter,
				CONNAC2X_USB_CMDPKT2WA);
		}
	}
#endif
}

void asicConnac3xWfdmaReInit(
	struct ADAPTER *prAdapter)
{
	u_int8_t fgResult;

	/*WFDMA re-init flow after chip deep sleep*/
	asicConnac3xWfdmaDummyCrRead(prAdapter, &fgResult);
	if (fgResult) {
		asicConnac3xWfdmaReInitImpl(prAdapter);
		asicConnac3xWfdmaDummyCrWrite(prAdapter);
	}
}

void asicConnac3xFillCmdTxd(
	struct ADAPTER *prAdapter,
	struct WIFI_CMD_INFO *prCmdInfo,
	u_int8_t *pucSeqNum,
	void **pCmdBuf)
{
	struct CONNAC3X_WIFI_CMD *prWifiCmd;

	/* 2. Setup common CMD Info Packet */
	prWifiCmd = (struct CONNAC3X_WIFI_CMD *)prCmdInfo->pucInfoBuffer;
	asicConnac3xFillCmdTxdInfo(prAdapter, prCmdInfo, pucSeqNum);
	if (pCmdBuf)
		*pCmdBuf = &prWifiCmd->aucBuffer[0];
}

#ifdef CFG_SUPPORT_UNIFIED_COMMAND
void asicConnac3xFillUniCmdTxd(
	struct ADAPTER *prAdapter,
	struct WIFI_UNI_CMD_INFO *prCmdInfo,
	u_int8_t *pucSeqNum,
	void **pCmdBuf)
{
	struct CONNAC3X_WIFI_UNI_CMD *prWifiCmd;

	/* 2. Setup common CMD Info Packet */
	prWifiCmd = (struct CONNAC3X_WIFI_UNI_CMD *)prCmdInfo->pucInfoBuffer;
	asicConnac3xFillUniCmdTxdInfo(prAdapter, prCmdInfo, pucSeqNum);
	if (pCmdBuf)
		*pCmdBuf = &prWifiCmd->aucBuffer[0];
}
#endif

#if defined(_HIF_PCIE) || defined(_HIF_AXI)
uint32_t asicConnac3xWfdmaCfgAddrGet(
	struct GLUE_INFO *prGlueInfo,
	u_int8_t ucDmaIdx)
{
	struct BUS_INFO *prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;

	if (ucDmaIdx == 0)
		return CONNAC3X_WPDMA_GLO_CFG(prBusInfo->host_dma0_base);
	else
		return CONNAC3X_WPDMA_GLO_CFG(prBusInfo->host_dma1_base);
}

uint32_t asicConnac3xWfdmaIntRstDtxPtrAddrGet(
	struct GLUE_INFO *prGlueInfo,
	u_int8_t ucDmaIdx)
{
	struct BUS_INFO *prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;

	if (ucDmaIdx == 0)
		return CONNAC3X_WPDMA_RST_DTX_PTR(prBusInfo->host_dma0_base);
	else
		return CONNAC3X_WPDMA_RST_DTX_PTR(prBusInfo->host_dma1_base);
}

uint32_t asicConnac3xWfdmaIntRstDrxPtrAddrGet(
	struct GLUE_INFO *prGlueInfo,
	u_int8_t ucDmaIdx)
{
	struct BUS_INFO *prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;

	if (ucDmaIdx == 0)
		return CONNAC3X_WPDMA_RST_DRX_PTR(prBusInfo->host_dma0_base);
	else
		return CONNAC3X_WPDMA_RST_DRX_PTR(prBusInfo->host_dma1_base);
}

uint32_t asicConnac3xWfdmaHifRstAddrGet(
	struct GLUE_INFO *prGlueInfo,
	u_int8_t ucDmaIdx)
{
	struct BUS_INFO *prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;

	if (ucDmaIdx == 0)
		return CONNAC3X_WPDMA_HIF_RST(prBusInfo->host_dma0_base);
	else
		return CONNAC3X_WPDMA_HIF_RST(prBusInfo->host_dma1_base);
}

void asicConnac3xWfdmaStop(struct GLUE_INFO *prGlueInfo, u_int8_t enable)
{
	struct ADAPTER *prAdapter = prGlueInfo->prAdapter;
	union WPDMA_GLO_CFG_STRUCT GloCfg;
	uint32_t u4DmaCfgCr;

	u4DmaCfgCr = asicConnac3xWfdmaCfgAddrGet(prGlueInfo, 0);
	HAL_MCR_RD(prAdapter, u4DmaCfgCr, &GloCfg.word);

	if (enable == TRUE) {
		GloCfg.field_conn3x.tx_dma_en = 0;
		GloCfg.field_conn3x.rx_dma_en = 0;
	} else {
		GloCfg.field_conn3x.tx_dma_en = 1;
		GloCfg.field_conn3x.rx_dma_en = 1;
	}

	HAL_MCR_WR(prAdapter, u4DmaCfgCr, GloCfg.word);
}

u_int8_t asicConnac3xWfdmaPollingAllIdle(struct GLUE_INFO *prGlueInfo)
{
	if (!asicConnac3xWfdmaWaitIdle(prGlueInfo, 0, 100, 1000)) {
		DBGLOG(HAL, WARN,
		       "Polling PDMA idle Timeout!!\n");
		return FALSE;
	}

	return TRUE;
}

void asicConnac3xWfdmaControl(
	struct GLUE_INFO *prGlueInfo,
	u_int8_t ucDmaIdx,
	u_int8_t enable)
{
	struct ADAPTER *prAdapter = prGlueInfo->prAdapter;
	union WPDMA_GLO_CFG_STRUCT GloCfg;
	uint32_t u4DmaCfgCr;
	uint32_t u4DmaRstDtxPtrCr;
	uint32_t u4DmaRstDrxPtrCr;

	ASSERT(ucDmaIdx < CONNAC3X_MAX_WFDMA_COUNT);
	u4DmaCfgCr = asicConnac3xWfdmaCfgAddrGet(prGlueInfo, ucDmaIdx);
	u4DmaRstDtxPtrCr =
		asicConnac3xWfdmaIntRstDtxPtrAddrGet(prGlueInfo, ucDmaIdx);
	u4DmaRstDrxPtrCr =
		asicConnac3xWfdmaIntRstDrxPtrAddrGet(prGlueInfo, ucDmaIdx);

	HAL_MCR_RD(prAdapter, u4DmaCfgCr, &GloCfg.word);
	if (enable == TRUE) {
		GloCfg.field_conn3x.pdma_bt_size = 3;
		GloCfg.field_conn3x.tx_wb_ddone = 1;
		GloCfg.field_conn3x.fifo_little_endian = 1;
		GloCfg.field_conn3x.clk_gate_dis = 1;
		GloCfg.field_conn3x.omit_tx_info = 1;
		if (ucDmaIdx == 1)
			GloCfg.field_conn3x.omit_rx_info = 1;
		GloCfg.field_conn3x.csr_disp_base_ptr_chain_en = 1;
		GloCfg.field_conn3x.omit_rx_info_pfet2 = 1;
	} else {
		GloCfg.field_conn3x.tx_dma_en = 0;
		GloCfg.field_conn3x.rx_dma_en = 0;
		GloCfg.field_conn3x.csr_disp_base_ptr_chain_en = 0;
		GloCfg.field_conn3x.omit_tx_info = 0;
		GloCfg.field_conn3x.omit_rx_info = 0;
		GloCfg.field_conn3x.omit_rx_info_pfet2 = 0;
	}
	HAL_MCR_WR(prAdapter, u4DmaCfgCr, GloCfg.word);

	if (!enable) {
		asicConnac3xWfdmaWaitIdle(prGlueInfo, ucDmaIdx, 100, 1000);
		/* Reset DMA Index */
		HAL_MCR_WR(prAdapter, u4DmaRstDtxPtrCr, 0xFFFFFFFF);
		HAL_MCR_WR(prAdapter, u4DmaRstDrxPtrCr, 0xFFFFFFFF);
	}
}

u_int8_t asicConnac3xWfdmaWaitIdle(
	struct GLUE_INFO *prGlueInfo,
	u_int8_t index,
	uint32_t round,
	uint32_t wait_us)
{
	uint32_t i = 0;
	uint32_t u4RegAddr = 0;
	union WPDMA_GLO_CFG_STRUCT GloCfg;
	struct BUS_INFO *prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;
	struct ADAPTER *prAdapter = prGlueInfo->prAdapter;

	if (index == 0)
		u4RegAddr = prBusInfo->host_dma0_base;
	else if (index == 1)
		u4RegAddr = prBusInfo->host_dma1_base;
	else {
		DBGLOG(HAL, ERROR, "Unknown wfdma index(=%d)\n", index);
		return FALSE;
	}

	do {
		HAL_MCR_RD(prAdapter, u4RegAddr, &GloCfg.word);
		if ((GloCfg.field.TxDMABusy == 0) &&
		    (GloCfg.field.RxDMABusy == 0)) {
			DBGLOG(HAL, TRACE, "==>  DMAIdle, GloCfg=0x%x\n",
			       GloCfg.word);
			return TRUE;
		}
		kalUdelay(wait_us);
	} while ((i++) < round);

	DBGLOG(HAL, INFO, "==>  DMABusy, GloCfg=0x%x\n", GloCfg.word);

	return FALSE;
}

void asicConnac3xWfdmaTxRingExtCtrl(
	struct GLUE_INFO *prGlueInfo,
	struct RTMP_TX_RING *tx_ring,
	u_int32_t index)
{
	struct BUS_INFO *prBusInfo;
	uint32_t ext_offset = 0;
	struct ADAPTER *prAdapter = prGlueInfo->prAdapter;
	struct mt66xx_chip_info *prChipInfo;

	prChipInfo = prGlueInfo->prAdapter->chip_info;
	prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;

	if (index == TX_RING_CMD_IDX_3)
		ext_offset = prBusInfo->tx_ring_cmd_idx * 4;
	else if (index == TX_RING_FWDL_IDX_4)
		ext_offset = prBusInfo->tx_ring_fwdl_idx * 4;
	else if (prChipInfo->is_support_wacpu) {
		if (index == TX_RING_DATA0_IDX_0)
			ext_offset = prBusInfo->tx_ring0_data_idx * 4;
		if (index == TX_RING_DATA1_IDX_1)
			ext_offset = prBusInfo->tx_ring1_data_idx * 4;
		if (index == TX_RING_WA_CMD_IDX_5)
			ext_offset = prBusInfo->tx_ring_wa_cmd_idx * 4;
	} else
		ext_offset = index * 4;

	tx_ring->hw_desc_base_ext =
		prBusInfo->host_tx_ring_ext_ctrl_base + ext_offset;
	HAL_MCR_WR(prAdapter, tx_ring->hw_desc_base_ext,
		   CONNAC3X_TX_RING_DISP_MAX_CNT);
}

void asicConnac3xWfdmaRxRingExtCtrl(
	struct GLUE_INFO *prGlueInfo,
	struct RTMP_RX_RING *rx_ring,
	u_int32_t index)
{
	struct ADAPTER *prAdapter;
	struct mt66xx_chip_info *prChipInfo;
	struct BUS_INFO *prBusInfo;
	uint32_t ext_offset;

	prAdapter = prGlueInfo->prAdapter;
	prChipInfo = prAdapter->chip_info;
	prBusInfo = prChipInfo->bus_info;

	switch (index) {
	case RX_RING_EVT_IDX_1:
		ext_offset = 0;
		break;
	case RX_RING_DATA_IDX_0:
		ext_offset = (index + 2) * 4;
		break;
	case RX_RING_DATA1_IDX_2:
	case RX_RING_TXDONE0_IDX_3:
	case RX_RING_TXDONE1_IDX_4:
		ext_offset = (index + 1) * 4;
		break;
	case RX_RING_WAEVT0_IDX_5:
	case RX_RING_WAEVT1_IDX_6:
		ext_offset = (index - 4) * 4;
		break;
	default:
		DBGLOG(RX, ERROR, "Error index=%d\n", index);
		return;
	}

	rx_ring->hw_desc_base_ext =
		prBusInfo->host_rx_ring_ext_ctrl_base + ext_offset;

	HAL_MCR_WR(prAdapter, rx_ring->hw_desc_base_ext,
		   CONNAC3X_RX_RING_DISP_MAX_CNT);
}

void asicConnac3xEnablePlatformIRQ(IN struct ADAPTER *prAdapter)
{
	struct GL_HIF_INFO *prHifInfo = NULL;

	ASSERT(prAdapter);

	prAdapter->fgIsIntEnable = TRUE;

	prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	enable_irq(prHifInfo->u4IrqId);
}

void asicConnac3xDisablePlatformIRQ(IN struct ADAPTER *prAdapter)
{
	struct GL_HIF_INFO *prHifInfo = NULL;

	ASSERT(prAdapter);

	prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	disable_irq_nosync(prHifInfo->u4IrqId);

	prAdapter->fgIsIntEnable = FALSE;
}

#if defined(_HIF_AXI)
void asicConnac3xDisablePlatformSwIRQ(IN struct ADAPTER *prAdapter)
{
	struct GL_HIF_INFO *prHifInfo = NULL;

	ASSERT(prAdapter);

	prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	disable_irq_nosync(prHifInfo->u4IrqId_1);
}
#endif

void asicConnac3xDisableExtInterrupt(
	struct ADAPTER *prAdapter)
{
	struct GLUE_INFO *prGlueInfo = NULL;
	union WPDMA_INT_MASK IntMask;

	ASSERT(prAdapter);

	prGlueInfo = prAdapter->prGlueInfo;

	IntMask.word = 0;

	HAL_MCR_WR(prAdapter,
		CONNAC3X_WPDMA_EXT_INT_MASK(CONNAC3X_HOST_EXT_CONN_HIF_WRAP),
		IntMask.word);
	HAL_MCR_RD(prAdapter,
		CONNAC3X_WPDMA_EXT_INT_MASK(CONNAC3X_HOST_EXT_CONN_HIF_WRAP),
		&IntMask.word);

	prAdapter->fgIsIntEnable = FALSE;

	DBGLOG(HAL, TRACE, "%s\n", __func__);

}

void asicConnac3xLowPowerOwnRead(
	struct ADAPTER *prAdapter,
	u_int8_t *pfgResult)
{
	struct mt66xx_chip_info *prChipInfo;

	prChipInfo = prAdapter->chip_info;

	if (prChipInfo->is_support_asic_lp) {
		u_int32_t u4RegValue;

		HAL_MCR_RD(prAdapter,
				CONNAC3X_BN0_LPCTL_ADDR,
				&u4RegValue);
		*pfgResult = (u4RegValue &
				PCIE_LPCR_AP_HOST_OWNER_STATE_SYNC)
				== 0 ? TRUE : FALSE;
	} else
		*pfgResult = TRUE;
}

void asicConnac3xLowPowerOwnSet(
	struct ADAPTER *prAdapter,
	u_int8_t *pfgResult)
{
	struct mt66xx_chip_info *prChipInfo;

	prChipInfo = prAdapter->chip_info;

	if (prChipInfo->is_support_asic_lp) {
		u_int32_t u4RegValue = 0;

		HAL_MCR_WR(prAdapter,
				CONNAC3X_BN0_LPCTL_ADDR,
				PCIE_LPCR_HOST_SET_OWN);
		HAL_MCR_RD(prAdapter,
				CONNAC3X_BN0_LPCTL_ADDR,
				&u4RegValue);
		*pfgResult = (u4RegValue &
			PCIE_LPCR_AP_HOST_OWNER_STATE_SYNC) == 0x4;
	} else
		*pfgResult = TRUE;
}

void asicConnac3xLowPowerOwnClear(
	struct ADAPTER *prAdapter,
	u_int8_t *pfgResult)
{
	struct mt66xx_chip_info *prChipInfo;

	prChipInfo = prAdapter->chip_info;

	if (prChipInfo->is_support_asic_lp) {
		u_int32_t u4RegValue = 0;

		HAL_MCR_WR(prAdapter,
			CONNAC3X_BN0_LPCTL_ADDR,
			PCIE_LPCR_HOST_CLR_OWN);
		HAL_MCR_RD(prAdapter,
			CONNAC3X_BN0_LPCTL_ADDR,
			&u4RegValue);
		*pfgResult = (u4RegValue &
				PCIE_LPCR_AP_HOST_OWNER_STATE_SYNC) == 0;
	} else
		*pfgResult = TRUE;
}

void asicConnac3xProcessSoftwareInterrupt(
	struct ADAPTER *prAdapter)
{
	struct GLUE_INFO *prGlueInfo;
	struct GL_HIF_INFO *prHifInfo;
	struct ERR_RECOVERY_CTRL_T *prErrRecoveryCtrl;
	uint32_t u4Status = 0;
	uint32_t u4HostWpdamBase = 0;

	if (prAdapter->prGlueInfo == NULL) {
		DBGLOG(HAL, ERROR, "prGlueInfo is NULL\n");
		return;
	}

	prGlueInfo = prAdapter->prGlueInfo;
	prHifInfo = &prGlueInfo->rHifInfo;
	prErrRecoveryCtrl = &prHifInfo->rErrRecoveryCtl;

	if (prAdapter->chip_info->is_support_wfdma1)
		u4HostWpdamBase = CONNAC3X_HOST_WPDMA_1_BASE;
	else
		u4HostWpdamBase = CONNAC3X_HOST_WPDMA_0_BASE;

	kalDevRegRead(prGlueInfo,
		CONNAC3X_WPDMA_MCU2HOST_SW_INT_STA(u4HostWpdamBase),
		&u4Status);

	prErrRecoveryCtrl->u4BackupStatus = u4Status;
	if (u4Status & ERROR_DETECT_MASK) {
		prErrRecoveryCtrl->u4Status = u4Status;
		kalDevRegWrite(prGlueInfo,
			CONNAC3X_WPDMA_MCU2HOST_SW_INT_STA(u4HostWpdamBase),
			u4Status);
		halHwRecoveryFromError(prAdapter);
	} else {
		kalDevRegWrite(prGlueInfo,
			CONNAC3X_WPDMA_MCU2HOST_SW_INT_STA(u4HostWpdamBase),
			u4Status);
		DBGLOG(HAL, TRACE, "undefined SER status[0x%x].\n", u4Status);
	}
}

void asicConnac3xSoftwareInterruptMcu(
	struct ADAPTER *prAdapter, u_int32_t intrBitMask)
{
	struct GLUE_INFO *prGlueInfo;
	uint32_t u4McuWpdamBase = 0;

	if (prAdapter == NULL || prAdapter->prGlueInfo == NULL) {
		DBGLOG(HAL, ERROR, "prAdapter or prGlueInfo is NULL\n");
		return;
	}

	prGlueInfo = prAdapter->prGlueInfo;
	if (prAdapter->chip_info->is_support_wfdma1)
		u4McuWpdamBase = CONNAC3X_MCU_WPDMA_1_BASE;
	else
		u4McuWpdamBase = CONNAC3X_MCU_WPDMA_0_BASE;
	kalDevRegWrite(prGlueInfo,
		CONNAC3X_WPDMA_HOST2MCU_SW_INT_SET(u4McuWpdamBase),
		intrBitMask);
}


void asicConnac3xHifRst(
	struct GLUE_INFO *prGlueInfo)
{
	uint32_t u4HifRstCr;

	u4HifRstCr = asicConnac3xWfdmaHifRstAddrGet(prGlueInfo, 0);
	/* Reset dmashdl and wpdma */
	kalDevRegWrite(prGlueInfo, u4HifRstCr, 0x00000000);
	kalDevRegWrite(prGlueInfo, u4HifRstCr, 0x00000030);

	u4HifRstCr = asicConnac3xWfdmaHifRstAddrGet(prGlueInfo, 1);
	/* Reset dmashdl and wpdma */
	kalDevRegWrite(prGlueInfo, u4HifRstCr, 0x00000000);
	kalDevRegWrite(prGlueInfo, u4HifRstCr, 0x00000030);
}
#endif /* _HIF_PCIE */

#if (CFG_SUPPORT_HOST_OFFLOAD == 1)
void fillConnac3xNicTxDescAppendWithSdo(
	struct ADAPTER *prAdapter,
	struct MSDU_INFO *prMsduInfo,
	uint8_t *prTxDescBuffer)
{
	struct mt66xx_chip_info *prChipInfo = prAdapter->chip_info;
	union HW_MAC_TX_DESC_APPEND *prHwTxDescAppend;

	/* Fill TxD append */
	prHwTxDescAppend = (union HW_MAC_TX_DESC_APPEND *)prTxDescBuffer;
	kalMemZero(prHwTxDescAppend, prChipInfo->txd_append_size);
	prHwTxDescAppend->CR4_APPEND.u2PktFlags = 0;
	prHwTxDescAppend->CR4_APPEND.ucBssIndex =
		prMsduInfo->ucBssIndex;
	prHwTxDescAppend->CR4_APPEND.ucWtblIndex =
		prMsduInfo->ucWlanIndex;
}

void fillConnac3xTxDescAppendBySdo(
	struct ADAPTER *prAdapter,
	struct MSDU_INFO *prMsduInfo,
	uint16_t u4MsduId,
	phys_addr_t rDmaAddr, IN uint32_t u4Idx,
	u_int8_t fgIsLast,
	uint8_t *pucBuffer)
{
	union HW_MAC_TX_DESC_APPEND *prHwTxDescAppend;

	prHwTxDescAppend = (union HW_MAC_TX_DESC_APPEND *)
		(pucBuffer + NIC_TX_DESC_LONG_FORMAT_LENGTH);

	prHwTxDescAppend->CR4_APPEND.u2MsduToken = u4MsduId;
	prHwTxDescAppend->CR4_APPEND.ucBufNum = 2;
	prHwTxDescAppend->CR4_APPEND.au4BufPtr[0] = rDmaAddr;
	prHwTxDescAppend->CR4_APPEND.au2BufLen[0] = SDO_HIF_TXD_SIZE;
	prHwTxDescAppend->CR4_APPEND.au4BufPtr[1] = rDmaAddr + SDO_HIF_TXD_SIZE;
	prHwTxDescAppend->CR4_APPEND.au2BufLen[1] = SDO_PARTIAL_PAYLOAD_SIZE;
}

void fillConnac3xTxDescAppendByMawdSdo(
	struct ADAPTER *prAdapter,
	struct MSDU_INFO *prMsduInfo,
	uint16_t u4MsduId,
	dma_addr_t rDmaAddr,
	uint32_t u4Idx,
	u_int8_t fgIsLast,
	uint8_t *pucBuffer)
{
	struct mt66xx_chip_info *prChipInfo;
	struct GL_HIF_INFO *prHifInfo;
	struct RTMP_TX_RING *prTxRing;
	struct RTMP_DMACB *pTxCell;
	union HW_MAC_TX_DESC_APPEND *prHwTxDescAppend;
	uint8_t *pucData;
	struct BUS_INFO *prBusInfo;
	union mawd_l2tbl rL2Tbl = {0};
	uint16_t u2EtherTypeLen = 0, u2Port = TX_RING_DATA0_IDX_0;
	uint8_t ucType = 0;
	uint32_t u4NumIPv4 = 0, u4NumIPv6 = 0;
	uint8_t pucIPv4Addr[IPV4_ADDR_LEN * CFG_PF_ARP_NS_MAX_NUM * 2];
	uint8_t pucIPv6Addr[IPV6_ADDR_LEN * CFG_PF_ARP_NS_MAX_NUM];
	struct net_device *prDev = wlanGetNetDev(prAdapter->prGlueInfo, 0);
	uint8_t ucIpVer, u4TxDSize;

	kalGetIPv4Address(prDev, CFG_PF_ARP_NS_MAX_NUM, pucIPv4Addr,
			  &u4NumIPv4);
	kalGetIPv6Address(prDev, CFG_PF_ARP_NS_MAX_NUM, pucIPv6Addr,
			  &u4NumIPv6);

	prChipInfo = prAdapter->chip_info;
	prBusInfo = prChipInfo->bus_info;
	prHifInfo = &prAdapter->prGlueInfo->rHifInfo;

	u2Port = halTxRingDataSelect(prAdapter, prMsduInfo);

	u4TxDSize = NIC_TX_DESC_LONG_FORMAT_LENGTH +
		prChipInfo->hif_txd_append_size;
	pucData = pucBuffer + u4TxDSize;
	ucIpVer = (pucData[ETH_HLEN] & IP_VERSION_MASK) >>
		      IP_VERSION_OFFSET;
	WLAN_GET_FIELD_BE16(&pucData[ETHER_HEADER_LEN - ETHER_TYPE_LEN],
			    &u2EtherTypeLen);

	DBGLOG(HAL, INFO, "EtherType [0x%08x]\n", u2EtherTypeLen);
	if (ucIpVer == IP_VERSION_4) {
		DBGLOG(HAL, INFO, "is IPV4[0x%08x]\n", u2EtherTypeLen);
		ucType = 1;
	}
	if (ucIpVer == IP_VERSION_6) {
		DBGLOG(HAL, INFO, "is IPV6[0x%08x]\n", u2EtherTypeLen);
		ucType = 2;
	}
	if (ucType) {
		if (ucType == 1) {
			for (u4Idx = 0; u4Idx < 4; u4Idx++)
				rL2Tbl.sram.key_ip[u4Idx] =
					pucData[ETH_HLEN + 12 + u4Idx];
		} else if (ucType == 2) {
			for (u4Idx = 0; u4Idx < 16; u4Idx++)
				rL2Tbl.sram.key_ip[u4Idx] =
					pucData[ETH_HLEN + 8 + u4Idx];
		}
		for (u4Idx = 0; u4Idx < MAC_ADDR_LEN; u4Idx++) {
			rL2Tbl.sram.d_mac[u4Idx] = pucData[u4Idx];
			rL2Tbl.sram.s_mac[u4Idx] =
				pucData[MAC_ADDR_LEN + u4Idx];
		}
		rL2Tbl.sram.wlan_id = prMsduInfo->ucWlanIndex;
		rL2Tbl.sram.bss_id = prMsduInfo->ucBssIndex;
		prHifInfo->u4MawdL2TblCnt = 2;
		halMawdUpdateL2Tbl(prAdapter->prGlueInfo, rL2Tbl, ucType - 1);
	}

	prTxRing = &prHifInfo->MawdTxRing[u2Port];
	pTxCell = &prTxRing->Cell[prTxRing->TxCpuIdx];

	kalMemZero(pTxCell->AllocVa, u4TxDSize);

	kalMemCopy(pTxCell->AllocVa, pucBuffer, u4TxDSize);

	prHwTxDescAppend = (union HW_MAC_TX_DESC_APPEND *)
		(pTxCell->AllocVa + NIC_TX_DESC_LONG_FORMAT_LENGTH);

	prHwTxDescAppend->CR4_APPEND.u2PktFlags = 0;
	if (ucType)
		prHwTxDescAppend->CR4_APPEND.u2PktFlags |= BIT(13) | BIT(15);

	prHwTxDescAppend->CR4_APPEND.ucBssIndex = prMsduInfo->ucBssIndex;
	prHwTxDescAppend->CR4_APPEND.ucWtblIndex = prMsduInfo->ucWlanIndex;

	prHwTxDescAppend->CR4_APPEND.u2MsduToken = u4MsduId;
	prHwTxDescAppend->CR4_APPEND.ucBufNum = 2;
	prHwTxDescAppend->CR4_APPEND.au4BufPtr[0] = rDmaAddr;
	/* eth header */
	prHwTxDescAppend->CR4_APPEND.au2BufLen[0] = ETH_HLEN;
	prHwTxDescAppend->CR4_APPEND.au4BufPtr[1] = (rDmaAddr + ETH_HLEN);
	prHwTxDescAppend->CR4_APPEND.au2BufLen[1] =
		prMsduInfo->u2FrameLength - ETH_HLEN;
}

void fillConnac3xTxDescTxByteCountWithSdo(
	struct ADAPTER *prAdapter,
	struct MSDU_INFO *prMsduInfo,
	void *prTxDesc)
{
	struct mt66xx_chip_info *prChipInfo;
	uint32_t u4TxByteCount = NIC_TX_DESC_LONG_FORMAT_LENGTH;

	prChipInfo = prAdapter->chip_info;
	u4TxByteCount += prMsduInfo->u2FrameLength;

	if (prMsduInfo->ucPacketType == TX_PACKET_TYPE_DATA)
		u4TxByteCount += prChipInfo->u4ExtraTxByteCount;

	HAL_MAC_CONNAC3X_TXD_SET_HIF_VERSION(
		(struct HW_MAC_CONNAC3X_TX_DESC *)prTxDesc, 1);

	/* Calculate Tx byte count */
	HAL_MAC_CONNAC3X_TXD_SET_TX_BYTE_COUNT(
		(struct HW_MAC_CONNAC3X_TX_DESC *)prTxDesc, u4TxByteCount);
}
#endif /* CFG_SUPPORT_HOST_OFFLOAD == 1 */

void fillConnac3xTxDescTxByteCount(
	struct ADAPTER *prAdapter,
	struct MSDU_INFO *prMsduInfo,
	void *prTxDesc)
{
	struct mt66xx_chip_info *prChipInfo;
	uint32_t u4TxByteCount = NIC_TX_DESC_LONG_FORMAT_LENGTH;

	ASSERT(prAdapter);
	ASSERT(prMsduInfo);
	ASSERT(prTxDesc);

	prChipInfo = prAdapter->chip_info;
	u4TxByteCount += prMsduInfo->u2FrameLength;

	if (prMsduInfo->ucPacketType == TX_PACKET_TYPE_DATA)
		u4TxByteCount += prChipInfo->u4ExtraTxByteCount;

	/* Calculate Tx byte count */
	HAL_MAC_CONNAC3X_TXD_SET_TX_BYTE_COUNT(
		(struct HW_MAC_CONNAC3X_TX_DESC *)prTxDesc, u4TxByteCount);
}

void asicConnac3xInitTxdHook(
	struct ADAPTER *prAdapter,
	struct TX_DESC_OPS_T *prTxDescOps)
{
	struct mt66xx_chip_info *prChipInfo;

	ASSERT(prTxDescOps);
	prChipInfo = prAdapter->chip_info;

	prTxDescOps->nic_txd_long_format_op = nic_txd_v3_long_format_op;
	prTxDescOps->nic_txd_tid_op = nic_txd_v3_tid_op;
	prTxDescOps->nic_txd_queue_idx_op = nic_txd_v3_queue_idx_op;
#if (CFG_TCP_IP_CHKSUM_OFFLOAD == 1)
	prTxDescOps->nic_txd_chksum_op = nic_txd_v3_chksum_op;
#endif /* CFG_TCP_IP_CHKSUM_OFFLOAD == 1 */
	prTxDescOps->nic_txd_header_format_op = nic_txd_v3_header_format_op;
	prTxDescOps->nic_txd_fill_by_pkt_option = nic_txd_v3_fill_by_pkt_option;
	prTxDescOps->nic_txd_compose = nic_txd_v3_compose;
	prTxDescOps->nic_txd_compose_security_frame =
		nic_txd_v3_compose_security_frame;
	prTxDescOps->nic_txd_set_pkt_fixed_rate_option_full =
		nic_txd_v3_set_pkt_fixed_rate_option_full;
	prTxDescOps->nic_txd_set_pkt_fixed_rate_option =
		nic_txd_v3_set_pkt_fixed_rate_option;
	prTxDescOps->nic_txd_set_hw_amsdu_template =
		nic_txd_v3_set_hw_amsdu_template;

#if (CFG_SUPPORT_HOST_OFFLOAD == 1)
	if (prChipInfo->is_support_sdo) {
		prChipInfo->txd_append_size =
			prChipInfo->hif_txd_append_size;
		prTxDescOps->fillNicAppend =
			fillConnac3xNicTxDescAppendWithSdo;
		prTxDescOps->fillHifAppend =
			fillConnac3xTxDescAppendBySdo;
		prTxDescOps->fillTxByteCount =
			fillConnac3xTxDescTxByteCountWithSdo;
	}

	if (prChipInfo->is_support_mawd_tx) {
		prChipInfo->txd_append_size =
			prChipInfo->hif_txd_append_size;
		prTxDescOps->fillNicAppend =
			fillConnac3xNicTxDescAppendWithSdo;
		prTxDescOps->fillHifAppend =
			fillConnac3xTxDescAppendByMawdSdo;
		prTxDescOps->fillTxByteCount =
			fillConnac3xTxDescTxByteCountWithSdo;
	}
#endif /* CFG_SUPPORT_HOST_OFFLOAD == 1 */
}

void asicConnac3xInitRxdHook(
	struct ADAPTER *prAdapter,
	struct RX_DESC_OPS_T *prRxDescOps)
{
	ASSERT(prRxDescOps);
	prRxDescOps->nic_rxd_get_rx_byte_count = nic_rxd_v3_get_rx_byte_count;
	prRxDescOps->nic_rxd_get_pkt_type = nic_rxd_v3_get_packet_type;
	prRxDescOps->nic_rxd_get_wlan_idx = nic_rxd_v3_get_wlan_idx;
	prRxDescOps->nic_rxd_get_sec_mode = nic_rxd_v3_get_sec_mode;
	prRxDescOps->nic_rxd_get_sw_class_error_bit =
		nic_rxd_v3_get_sw_class_error_bit;
	prRxDescOps->nic_rxd_get_ch_num = nic_rxd_v3_get_ch_num;
	prRxDescOps->nic_rxd_get_rf_band = nic_rxd_v3_get_rf_band;
	prRxDescOps->nic_rxd_get_tcl = nic_rxd_v3_get_tcl;
	prRxDescOps->nic_rxd_get_ofld = nic_rxd_v3_get_ofld;
	prRxDescOps->nic_rxd_get_HdrTrans = nic_rxd_v3_get_HdrTrans;
	prRxDescOps->nic_rxd_fill_rfb = nic_rxd_v3_fill_rfb;
	prRxDescOps->nic_rxd_sanity_check = nic_rxd_v3_sanity_check;
	prRxDescOps->nic_rxd_check_wakeup_reason =
		nic_rxd_v3_check_wakeup_reason;
#ifdef CFG_SUPPORT_SNIFFER_RADIOTAP
	prRxDescOps->nic_rxd_fill_radiotap = nic_rxd_v3_fill_radiotap;
#endif
	prRxDescOps->nic_rxd_handle_host_rpt =
		nic_rxd_v3_handle_host_rpt;
}

#if (CFG_SUPPORT_MSP == 1)
void asicConnac3xRxProcessRxvforMSP(IN struct ADAPTER *prAdapter,
	  IN OUT struct SW_RFB *prRetSwRfb) {
	struct HW_MAC_RX_STS_GROUP_3_V2 *prGroup3;

	if (prRetSwRfb->ucStaRecIdx >= CFG_STA_REC_NUM) {
		DBGLOG(RX, LOUD,
		"prRetSwRfb->ucStaRecIdx(%d) >= CFG_STA_REC_NUM(%d)\n",
			prRetSwRfb->ucStaRecIdx, CFG_STA_REC_NUM);
		return;
	}

	prGroup3 =
		(struct HW_MAC_RX_STS_GROUP_3_V2 *)prRetSwRfb->prRxStatusGroup3;

	prAdapter->arStaRec[prRetSwRfb->ucStaRecIdx].u4RxVector0 = 0;
	prAdapter->arStaRec[prRetSwRfb->ucStaRecIdx].u4RxVector1 = 0;
	prAdapter->arStaRec[prRetSwRfb->ucStaRecIdx].u4RxVector2 = 0;
	prAdapter->arStaRec[prRetSwRfb->ucStaRecIdx].u4RxVector3 = 0;
	prAdapter->arStaRec[prRetSwRfb->ucStaRecIdx].u4RxVector4 = 0;

	if (prRetSwRfb->ucGroupVLD & BIT(RX_GROUP_VLD_3)) {
		/* P-RXV0[0:31] in RXD Group3 */
		prAdapter->arStaRec[
		prRetSwRfb->ucStaRecIdx].u4RxVector0 =
		CONNAC3X_HAL_RX_VECTOR_GET_RX_VECTOR(prGroup3, 0);

		/* P-RXV0[32:63] in RXD Group3 */
		prAdapter->arStaRec[
		prRetSwRfb->ucStaRecIdx].u4RxVector1 =
		CONNAC3X_HAL_RX_VECTOR_GET_RX_VECTOR(prGroup3, 1);
	}
}
#endif /* CFG_SUPPORT_MSP == 1 */

uint8_t asicConnac3xRxGetRcpiValueFromRxv(
	IN uint8_t ucRcpiMode,
	IN struct SW_RFB *prSwRfb)
{
	uint8_t ucRcpi0, ucRcpi1, ucRcpi2, ucRcpi3;
	uint8_t ucRcpiValue = 0;
	/* falcon IP donot have this field 'ucRxNum' */
	/* uint8_t ucRxNum; */
	struct HW_MAC_RX_STS_GROUP_3_V2 *prGroup3 = NULL;

	ASSERT(prSwRfb);

	if (ucRcpiMode >= RCPI_MODE_NUM) {
		DBGLOG(RX, WARN,
		       "Rcpi Mode = %d is invalid for getting uint8_t value from RXV\n",
		       ucRcpiMode);
		return 0;
	}

	if ((prSwRfb->ucGroupVLD & BIT(RX_GROUP_VLD_3)) == 0) {
		DBGLOG(RX, WARN, "%s, RXD group 3 is not valid\n", __func__);
		return 0;
	}

	prGroup3 = (struct HW_MAC_RX_STS_GROUP_3_V2 *)
				prSwRfb->prRxStatusGroup3;

	ucRcpi0 = CONNAC3X_HAL_RX_VECTOR_GET_RCPI0_V2(prGroup3);
	ucRcpi1 = CONNAC3X_HAL_RX_VECTOR_GET_RCPI1_V2(prGroup3);
	ucRcpi2 = CONNAC3X_HAL_RX_VECTOR_GET_RCPI2_V2(prGroup3);
	ucRcpi3 = CONNAC3X_HAL_RX_VECTOR_GET_RCPI3_V2(prGroup3);

	/*If Rcpi is not available, set it to zero*/
	if (ucRcpi0 == RCPI_MEASUREMENT_NOT_AVAILABLE)
		ucRcpi0 = RCPI_LOW_BOUND;
	if (ucRcpi1 == RCPI_MEASUREMENT_NOT_AVAILABLE)
		ucRcpi1 = RCPI_LOW_BOUND;
	DBGLOG(RX, TRACE, "RCPI WF0:%d WF1:%d WF2:%d WF3:%d\n",
	       ucRcpi0, ucRcpi1, ucRcpi2, ucRcpi3);

	switch (ucRcpiMode) {
	case RCPI_MODE_WF0:
		ucRcpiValue = ucRcpi0;
		break;

	case RCPI_MODE_WF1:
		ucRcpiValue = ucRcpi1;
		break;

	case RCPI_MODE_WF2:
		ucRcpiValue = ucRcpi2;
		break;

	case RCPI_MODE_WF3:
		ucRcpiValue = ucRcpi3;
		break;

	case RCPI_MODE_AVG: /*Not recommended for CBW80+80*/
		ucRcpiValue = (ucRcpi0 + ucRcpi1) / 2;
		break;

	case RCPI_MODE_MAX:
		ucRcpiValue =
			(ucRcpi0 > ucRcpi1) ? (ucRcpi0) : (ucRcpi1);
		break;

	case RCPI_MODE_MIN:
		ucRcpiValue =
			(ucRcpi0 < ucRcpi1) ? (ucRcpi0) : (ucRcpi1);
		break;

	default:
		break;
	}

	return ucRcpiValue;
}

#if (CFG_SUPPORT_PERF_IND == 1)
void asicConnac3xRxPerfIndProcessRXV(IN struct ADAPTER *prAdapter,
			       IN struct SW_RFB *prSwRfb,
			       IN uint8_t ucBssIndex)
{
	struct HW_MAC_RX_STS_GROUP_3 *prRxStatusGroup3;
	uint8_t ucRCPI0 = 0, ucRCPI1 = 0;

	ASSERT(prAdapter);
	ASSERT(prSwRfb);
	/* REMOVE DATA RATE Parsing Logic:Workaround only for 6885*/
	/* Since MT6885 can not get Rx Data Rate dur to RXV HW Bug*/

	if (ucBssIndex >= BSSID_NUM)
		return;

	/* can't parse radiotap info if no rx vector */
	if (((prSwRfb->ucGroupVLD & BIT(RX_GROUP_VLD_2)) == 0)
		|| ((prSwRfb->ucGroupVLD & BIT(RX_GROUP_VLD_3)) == 0)) {
		return;
	}

	prRxStatusGroup3 = prSwRfb->prRxStatusGroup3;

	/* RCPI */
	ucRCPI0 = HAL_RX_STATUS_GET_RCPI0(prRxStatusGroup3);
	ucRCPI1 = HAL_RX_STATUS_GET_RCPI1(prRxStatusGroup3);

	/* Record peak rate to Traffic Indicator*/
	prAdapter->prGlueInfo->PerfIndCache.
		ucCurRxRCPI0[ucBssIndex] = ucRCPI0;
	prAdapter->prGlueInfo->PerfIndCache.
		ucCurRxRCPI1[ucBssIndex] = ucRCPI1;

}
#endif

#if (CFG_CHIP_RESET_SUPPORT == 1) && (CFG_WMT_RESET_API_SUPPORT == 0)
u_int8_t conn2_rst_L0_notify_step2(void)
{
	if (glGetRstReason() == RST_BT_TRIGGER) {
		typedef void (*p_bt_fun_type) (void);
		p_bt_fun_type bt_func;
		char *bt_func_name = "WF_rst_L0_notify_BT_step2";

		bt_func = (p_bt_fun_type)(uintptr_t)
				GLUE_LOOKUP_FUN(bt_func_name);
		if (bt_func) {
			bt_func();
		} else {
			DBGLOG(INIT, WARN, "[SER][L0] %s does not exist\n",
							bt_func_name);
			return FALSE;
		}
	} else {
		/* if wifi trigger, then wait bt ready notify */
		DBGLOG(INIT, WARN, "[SER][L0] not support..\n");
	}

	return TRUE;
}
#endif

void asicConnac3xDmashdlSetPlePktMaxPage(struct ADAPTER *prAdapter,
				      uint16_t u2MaxPage)
{
	struct BUS_INFO *prBusInfo;
	struct DMASHDL_CFG *prCfg;
	uint32_t u4Val = 0;

	prBusInfo = prAdapter->chip_info->bus_info;
	prCfg = prBusInfo->prDmashdlCfg;

	HAL_MCR_RD(prAdapter, prCfg->rPlePacketMaxSize.u4Addr, &u4Val);

	u4Val &= ~prCfg->rPlePacketMaxSize.u4Mask;
	u4Val |= (u2MaxPage << prCfg->rPlePacketMaxSize.u4Shift) &
		prCfg->rPlePacketMaxSize.u4Mask;

	HAL_MCR_WR(prAdapter, prCfg->rPlePacketMaxSize.u4Addr, u4Val);
}

void asicConnac3xDmashdlSetPsePktMaxPage(struct ADAPTER *prAdapter,
				      uint16_t u2MaxPage)
{
	struct BUS_INFO *prBusInfo;
	struct DMASHDL_CFG *prCfg;
	uint32_t u4Val = 0;

	prBusInfo = prAdapter->chip_info->bus_info;
	prCfg = prBusInfo->prDmashdlCfg;

	HAL_MCR_RD(prAdapter, prCfg->rPsePacketMaxSize.u4Addr, &u4Val);

	u4Val &= ~prCfg->rPsePacketMaxSize.u4Mask;
	u4Val |= (u2MaxPage << prCfg->rPsePacketMaxSize.u4Shift) &
		 prCfg->rPsePacketMaxSize.u4Mask;

	HAL_MCR_WR(prAdapter, prCfg->rPsePacketMaxSize.u4Addr, u4Val);
}

void asicConnac3xDmashdlGetPktMaxPage(struct ADAPTER *prAdapter)
{
	struct BUS_INFO *prBusInfo;
	struct DMASHDL_CFG *prCfg;
	uint32_t u4Val = 0;
	uint32_t ple_pkt_max_sz;
	uint32_t pse_pkt_max_sz;

	prBusInfo = prAdapter->chip_info->bus_info;
	prCfg = prBusInfo->prDmashdlCfg;

	HAL_MCR_RD(prAdapter, prCfg->rPlePacketMaxSize.u4Addr, &u4Val);

	ple_pkt_max_sz = (u4Val & prCfg->rPlePacketMaxSize.u4Mask) >>
		prCfg->rPlePacketMaxSize.u4Shift;
	pse_pkt_max_sz = (u4Val & prCfg->rPsePacketMaxSize.u4Mask) >>
		prCfg->rPsePacketMaxSize.u4Shift;

	DBGLOG(HAL, INFO, "DMASHDL PLE_PACKET_MAX_SIZE (0x%08x): 0x%08x\n",
		prCfg->rPlePacketMaxSize.u4Addr, u4Val);
	DBGLOG(HAL, INFO, "PLE/PSE packet max size=0x%03x/0x%03x\n",
		ple_pkt_max_sz, pse_pkt_max_sz);

}
void asicConnac3xDmashdlSetRefill(struct ADAPTER *prAdapter, uint8_t ucGroup,
			       u_int8_t fgEnable)
{
	struct BUS_INFO *prBusInfo;
	struct DMASHDL_CFG *prCfg;
	uint32_t u4Mask;
	uint32_t u4Val = 0;

	prBusInfo = prAdapter->chip_info->bus_info;
	prCfg = prBusInfo->prDmashdlCfg;

	if (ucGroup >= prCfg->u4GroupNum)
		ASSERT(0);

	u4Mask = prCfg->rGroup0RefillDisable.u4Mask << ucGroup;

	HAL_MCR_RD(prAdapter, prCfg->rGroup0RefillDisable.u4Addr, &u4Val);

	if (fgEnable)
		u4Val &= ~u4Mask;
	else
		u4Val |= u4Mask;

	HAL_MCR_WR(prAdapter, prCfg->rGroup0RefillDisable.u4Addr, u4Val);
}

void asicConnac3xDmashdlGetRefill(struct ADAPTER *prAdapter)
{
	struct BUS_INFO *prBusInfo;
	struct DMASHDL_CFG *prCfg;
	uint32_t u4Val = 0;

	prBusInfo = prAdapter->chip_info->bus_info;
	prCfg = prBusInfo->prDmashdlCfg;

	HAL_MCR_RD(prAdapter, prCfg->rGroup0RefillDisable.u4Addr, &u4Val);
	DBGLOG(HAL, INFO, "DMASHDL ReFill Control (0x%08x): 0x%08x\n",
		prCfg->rGroup0RefillDisable.u4Addr, u4Val);
}

void asicConnac3xDmashdlSetMaxQuota(struct ADAPTER *prAdapter, uint8_t ucGroup,
				 uint16_t u2MaxQuota)
{
	struct BUS_INFO *prBusInfo;
	struct DMASHDL_CFG *prCfg;
	uint32_t u4Addr;
	uint32_t u4Val = 0;

	prBusInfo = prAdapter->chip_info->bus_info;
	prCfg = prBusInfo->prDmashdlCfg;

	if (ucGroup >= prCfg->u4GroupNum)
		ASSERT(0);

	u4Addr = prCfg->rGroup0ControlMaxQuota.u4Addr + (ucGroup << 2);

	HAL_MCR_RD(prAdapter, u4Addr, &u4Val);

	u4Val &= ~prCfg->rGroup0ControlMaxQuota.u4Mask;
	u4Val |= (u2MaxQuota << prCfg->rGroup0ControlMaxQuota.u4Shift) &
		prCfg->rGroup0ControlMaxQuota.u4Mask;

	HAL_MCR_WR(prAdapter, u4Addr, u4Val);
}

void asicConnac3xDmashdlSetMinQuota(struct ADAPTER *prAdapter, uint8_t ucGroup,
				 uint16_t u2MinQuota)
{
	struct BUS_INFO *prBusInfo;
	struct DMASHDL_CFG *prCfg;
	uint32_t u4Addr;
	uint32_t u4Val = 0;

	prBusInfo = prAdapter->chip_info->bus_info;
	prCfg = prBusInfo->prDmashdlCfg;

	if (ucGroup >= prCfg->u4GroupNum)
		ASSERT(0);

	u4Addr = prCfg->rGroup0ControlMinQuota.u4Addr + (ucGroup << 2);

	HAL_MCR_RD(prAdapter, u4Addr, &u4Val);

	u4Val &= ~prCfg->rGroup0ControlMinQuota.u4Mask;
	u4Val |= (u2MinQuota << prCfg->rGroup0ControlMinQuota.u4Shift) &
		prCfg->rGroup0ControlMinQuota.u4Mask;

	HAL_MCR_WR(prAdapter, u4Addr, u4Val);
}

void asicConnac3xDmashdlGetGroupControl(struct ADAPTER *prAdapter,
					uint8_t ucGroup)
{
	struct BUS_INFO *prBusInfo;
	struct DMASHDL_CFG *prCfg;
	uint32_t u4Addr;
	uint32_t u4Val = 0;
	uint32_t max_quota;
	uint32_t min_quota;

	prBusInfo = prAdapter->chip_info->bus_info;
	prCfg = prBusInfo->prDmashdlCfg;

	u4Addr = prCfg->rGroup0ControlMaxQuota.u4Addr + (ucGroup << 2);

	HAL_MCR_RD(prAdapter, u4Addr, &u4Val);

	max_quota = GET_DMASHDL_MAX_QUOTA_NUM(u4Val);
	min_quota = GET_DMASHDL_MIN_QUOTA_NUM(u4Val);
	DBGLOG(HAL, INFO, "\tDMASHDL Group%d control(0x%08x): 0x%08x\n",
		ucGroup, u4Addr, u4Val);
	DBGLOG(HAL, INFO, "\tmax/min quota = 0x%03x/ 0x%03x\n",
		max_quota, min_quota);

}
void asicConnac3xDmashdlSetQueueMapping(struct ADAPTER *prAdapter,
					uint8_t ucQueue,
					uint8_t ucGroup)
{
	struct BUS_INFO *prBusInfo;
	struct DMASHDL_CFG *prCfg;
	uint32_t u4Addr, u4Mask, u4Shft;
	uint32_t u4Val = 0;

	prBusInfo = prAdapter->chip_info->bus_info;
	prCfg = prBusInfo->prDmashdlCfg;

	if (ucQueue >= 32)
		ASSERT(0);

	if (ucGroup >= prCfg->u4GroupNum)
		ASSERT(0);

	u4Addr = prCfg->rQueueMapping0Queue0.u4Addr + ((ucQueue >> 3) << 2);
	u4Mask = prCfg->rQueueMapping0Queue0.u4Mask << ((ucQueue % 8) << 2);
	u4Shft = (ucQueue % 8) << 2;

	HAL_MCR_RD(prAdapter, u4Addr, &u4Val);

	u4Val &= ~u4Mask;
	u4Val |= (ucGroup << u4Shft) & u4Mask;

	HAL_MCR_WR(prAdapter, u4Addr, u4Val);
}

void asicConnac3xDmashdlSetSlotArbiter(struct ADAPTER *prAdapter,
				    u_int8_t fgEnable)
{
	struct BUS_INFO *prBusInfo;
	struct DMASHDL_CFG *prCfg;
	uint32_t u4Val = 0;

	prBusInfo = prAdapter->chip_info->bus_info;
	prCfg = prBusInfo->prDmashdlCfg;

	HAL_MCR_RD(prAdapter, prCfg->rPageSettingGroupSeqOrderType.u4Addr,
		   &u4Val);

	if (fgEnable)
		u4Val |= prCfg->rPageSettingGroupSeqOrderType.u4Mask;
	else
		u4Val &= ~prCfg->rPageSettingGroupSeqOrderType.u4Mask;

	HAL_MCR_WR(prAdapter, prCfg->rPageSettingGroupSeqOrderType.u4Addr,
		   u4Val);
}

void asicConnac3xDmashdlSetUserDefinedPriority(struct ADAPTER *prAdapter,
					    uint8_t ucPriority, uint8_t ucGroup)
{
	struct BUS_INFO *prBusInfo;
	struct DMASHDL_CFG *prCfg;
	uint32_t u4Addr, u4Mask, u4Shft;
	uint32_t u4Val = 0;

	prBusInfo = prAdapter->chip_info->bus_info;
	prCfg = prBusInfo->prDmashdlCfg;

	ASSERT(ucPriority < 16);
	ASSERT(ucGroup < prCfg->u4GroupNum);

	u4Addr = prCfg->rSchdulerSetting0Priority0Group.u4Addr +
		((ucPriority >> 3) << 2);
	u4Mask = prCfg->rSchdulerSetting0Priority0Group.u4Mask <<
		((ucPriority % 8) << 2);
	u4Shft = (ucPriority % 8) << 2;

	HAL_MCR_RD(prAdapter, u4Addr, &u4Val);

	u4Val &= ~u4Mask;
	u4Val |= (ucGroup << u4Shft) & u4Mask;

	HAL_MCR_WR(prAdapter, u4Addr, u4Val);
}

uint32_t asicConnac3xDmashdlGetRsvCount(struct ADAPTER *prAdapter,
					uint8_t ucGroup)
{
	struct BUS_INFO *prBusInfo;
	struct DMASHDL_CFG *prCfg;
	uint32_t u4Addr;
	uint32_t u4Val = 0;
	uint32_t rsv_cnt = 0;

	prBusInfo = prAdapter->chip_info->bus_info;
	prCfg = prBusInfo->prDmashdlCfg;

	u4Addr = prCfg->rStatusRdGp0RsvCnt.u4Addr + (ucGroup << 2);

	HAL_MCR_RD(prAdapter, u4Addr, &u4Val);

	rsv_cnt = (u4Val & prCfg->rStatusRdGp0RsvCnt.u4Mask) >>
		prCfg->rStatusRdGp0RsvCnt.u4Shift;

	DBGLOG(HAL, INFO, "\tDMASHDL Status_RD_GP%d(0x%08x): 0x%08x\n",
		ucGroup, u4Addr, u4Val);
	DBGLOG(HAL, TRACE, "\trsv_cnt = 0x%03x\n", rsv_cnt);
	return rsv_cnt;
}

uint32_t asicConnac3xDmashdlGetSrcCount(struct ADAPTER *prAdapter,
					uint8_t ucGroup)
{
	struct BUS_INFO *prBusInfo;
	struct DMASHDL_CFG *prCfg;
	uint32_t u4Addr;
	uint32_t u4Val = 0;
	uint32_t src_cnt = 0;

	prBusInfo = prAdapter->chip_info->bus_info;
	prCfg = prBusInfo->prDmashdlCfg;

	u4Addr = prCfg->rStatusRdGp0SrcCnt.u4Addr + (ucGroup << 2);

	HAL_MCR_RD(prAdapter, u4Addr, &u4Val);

	src_cnt = (u4Val & prCfg->rStatusRdGp0SrcCnt.u4Mask) >>
		prCfg->rStatusRdGp0SrcCnt.u4Shift;

	DBGLOG(HAL, TRACE, "\tsrc_cnt = 0x%03x\n", src_cnt);
	return src_cnt;
}

void asicConnac3xDmashdlGetPKTCount(struct ADAPTER *prAdapter, uint8_t ucGroup)
{
	struct BUS_INFO *prBusInfo;
	struct DMASHDL_CFG *prCfg;
	uint32_t u4Addr;
	uint32_t u4Val = 0;
	uint32_t pktin_cnt = 0;
	uint32_t ask_cnt = 0;

	prBusInfo = prAdapter->chip_info->bus_info;
	prCfg = prBusInfo->prDmashdlCfg;

	if ((ucGroup & 0x1) == 0)
		u4Addr = prCfg->rRdGroupPktCnt0.u4Addr + (ucGroup << 1);
	else
		u4Addr = prCfg->rRdGroupPktCnt0.u4Addr + ((ucGroup-1) << 1);

	HAL_MCR_RD(prAdapter, u4Addr, &u4Val);
	DBGLOG(HAL, INFO, "\tDMASHDL RD_group_pkt_cnt_%d(0x%08x): 0x%08x\n",
		ucGroup / 2, u4Addr, u4Val);
	if ((ucGroup & 0x1) == 0) {
		pktin_cnt = GET_EVEN_GROUP_PKT_IN_CNT(u4Val);
		ask_cnt = GET_EVEN_GROUP_ASK_CNT(u4Val);
	} else {
		pktin_cnt = GET_ODD_GROUP_PKT_IN_CNT(u4Val);
		ask_cnt = GET_ODD_GROUP_ASK_CNT(u4Val);
	}
	DBGLOG(HAL, INFO, "\tpktin_cnt = 0x%02x, ask_cnt = 0x%02x",
		pktin_cnt, ask_cnt);
}

void asicConnac3xDmashdlSetOptionalControl(struct ADAPTER *prAdapter,
		uint16_t u2HifAckCntTh, uint16_t u2HifGupActMap)
{
	struct BUS_INFO *prBusInfo;
	struct DMASHDL_CFG *prCfg;
	uint32_t u4Addr, u4Val = 0;

	prBusInfo = prAdapter->chip_info->bus_info;
	prCfg = prBusInfo->prDmashdlCfg;

	u4Addr = prCfg->rOptionalControlCrHifAckCntTh.u4Addr;

	HAL_MCR_RD(prAdapter, u4Addr, &u4Val);

	u4Val &= ~prCfg->rOptionalControlCrHifAckCntTh.u4Mask;
	u4Val |= (u2HifAckCntTh <<
		  prCfg->rOptionalControlCrHifAckCntTh.u4Shift);

	u4Val &= ~prCfg->rOptionalControlCrHifGupActMap.u4Mask;
	u4Val |= (u2HifGupActMap <<
		  prCfg->rOptionalControlCrHifGupActMap.u4Shift);

	HAL_MCR_WR(prAdapter, u4Addr, u4Val);
}

u_int8_t asicConnac3xSwIntHandler(struct ADAPTER *prAdapter)
{
	return TRUE;
}

#endif /* CFG_SUPPORT_CONNAC3X == 1 */
