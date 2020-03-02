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
/*! \file   cmm_asic_connac2x.c
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

#if (CFG_SUPPORT_CONNAC2X == 1)

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/
#include "coda/mt7915/wf_wfdma_host_dma0.h"
#include "coda/mt7915/wf_wfdma_host_dma1.h"


#include "precomp.h"

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/
#define CONNAC2X_HIF_DMASHDL_BASE 0x52000000

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/
void asicConnac2xCapInit(
	struct ADAPTER *prAdapter)
{
	struct GLUE_INFO *prGlueInfo;
	struct mt66xx_chip_info *prChipInfo;

	ASSERT(prAdapter);

	prGlueInfo = prAdapter->prGlueInfo;
	prChipInfo = prAdapter->chip_info;

	prChipInfo->u2HifTxdSize = 0;
	prChipInfo->u2TxInitCmdPort = 0;
	prChipInfo->u2TxFwDlPort = 0;
	prChipInfo->u2CmdTxHdrSize = sizeof(struct CONNAC2X_WIFI_CMD);
	prChipInfo->asicFillInitCmdTxd = asicConnac2xFillInitCmdTxd;
	prChipInfo->asicFillCmdTxd = asicConnac2xFillCmdTxd;
	prChipInfo->u2RxSwPktBitMap = CONNAC2X_RX_STATUS_PKT_TYPE_SW_BITMAP;
	prChipInfo->u2RxSwPktEvent = CONNAC2X_RX_STATUS_PKT_TYPE_SW_EVENT;
	prChipInfo->u2RxSwPktFrame = CONNAC2X_RX_STATUS_PKT_TYPE_SW_FRAME;
	asicConnac2xInitTxdHook(prChipInfo->prTxDescOps);
	asicConnac2xInitRxdHook(prChipInfo->prRxDescOps);
#if (CFG_SUPPORT_MSP == 1)
	prChipInfo->asicRxProcessRxvforMSP = asicConnac2xRxProcessRxvforMSP;
#endif /* CFG_SUPPORT_MSP == 1 */
	prChipInfo->asicRxGetRcpiValueFromRxv =
			asicConnac2xRxGetRcpiValueFromRxv;
#if CFG_SUPPORT_WIFI_SYSDVT
	prAdapter->u2TxTest = TX_TEST_UNLIMITIED;
	prAdapter->u2TxTestCount = 0;
	prAdapter->ucTxTestUP = TX_TEST_UP_UNDEF;
#endif /* CFG_SUPPORT_WIFI_SYSDVT */
	switch (prGlueInfo->u4InfType) {
#if defined(_HIF_PCIE) || defined(_HIF_AXI)
	case MT_DEV_INF_PCIE:
	case MT_DEV_INF_AXI:

		prChipInfo->u2TxInitCmdPort =
			TX_RING_CMD_IDX_2; /* Ring17 for CMD */
		prChipInfo->u2TxFwDlPort =
			TX_RING_FWDL_IDX_3; /* Ring16 for FWDL */
		prChipInfo->ucPacketFormat = TXD_PKT_FORMAT_TXD;
		prChipInfo->u4HifDmaShdlBaseAddr = CONNAC2X_HIF_DMASHDL_BASE;
#if 0 /* merge next time 20181210 */
		prChipInfo->rx_event_port = WFDMA1_RX_RING_IDX_0;
#endif /* if 0 */
		HAL_MCR_WR(prAdapter,
				CONNAC2X_BN0_IRQ_ENA_ADDR,
				BIT(0));

		if (prChipInfo->is_support_asic_lp) {
			HAL_MCR_WR(prAdapter,
				CONNAC2X_WPDMA_MCU2HOST_SW_INT_MASK
					(CONNAC2X_HOST_WPDMA_1_BASE),
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
		prChipInfo->ucPacketFormat = TXD_PKT_FORMAT_TXD;
		prChipInfo->u4ExtraTxByteCount = 0;
		prChipInfo->u4HifDmaShdlBaseAddr = USB_HIF_DMASHDL_BASE;
#if (CFG_ENABLE_FW_DOWNLOAD == 1)
		prChipInfo->asicEnableFWDownload = asicConnac2xEnableUsbFWDL;
#endif /* CFG_ENABLE_FW_DOWNLOAD == 1 */
		if (prChipInfo->asicUsbInit)
			prChipInfo->asicUsbInit(prAdapter, prChipInfo);
		break;
#endif /* _HIF_USB */
	default:
		break;
	}
}

static void asicConnac2xFillInitCmdTxdInfo(
	struct ADAPTER *prAdapter,
	struct WIFI_CMD_INFO *prCmdInfo,
	u_int8_t *pucSeqNum)
{
	struct INIT_HIF_TX_HEADER *prInitHifTxHeader;
	struct HW_MAC_CONNAC2X_TX_DESC *prInitHifTxHeaderPadding;
	uint32_t u4TxdLen = sizeof(struct HW_MAC_CONNAC2X_TX_DESC);

	prInitHifTxHeaderPadding =
		(struct HW_MAC_CONNAC2X_TX_DESC *) (prCmdInfo->pucInfoBuffer);
	prInitHifTxHeader = (struct INIT_HIF_TX_HEADER *)
		(prCmdInfo->pucInfoBuffer + u4TxdLen);

	HAL_MAC_CONNAC2X_TXD_SET_TX_BYTE_COUNT(prInitHifTxHeaderPadding,
		prCmdInfo->u2InfoBufLen);
	if (!prCmdInfo->ucCID)
		HAL_MAC_CONNAC2X_TXD_SET_PKT_FORMAT(prInitHifTxHeaderPadding,
						INIT_PKT_FT_PDA_FWDL)
	else
		HAL_MAC_CONNAC2X_TXD_SET_PKT_FORMAT(prInitHifTxHeaderPadding,
						INIT_PKT_FT_CMD)
	HAL_MAC_CONNAC2X_TXD_SET_HEADER_FORMAT(prInitHifTxHeaderPadding,
						HEADER_FORMAT_COMMAND);
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


static void asicConnac2xFillCmdTxdInfo(
	struct ADAPTER *prAdapter,
	struct WIFI_CMD_INFO *prCmdInfo,
	u_int8_t *pucSeqNum)
{
	struct CONNAC2X_WIFI_CMD *prWifiCmd;
	uint32_t u4TxdLen = sizeof(struct HW_MAC_CONNAC2X_TX_DESC);

	prWifiCmd = (struct CONNAC2X_WIFI_CMD *)prCmdInfo->pucInfoBuffer;

	HAL_MAC_CONNAC2X_TXD_SET_TX_BYTE_COUNT(
		(struct HW_MAC_CONNAC2X_TX_DESC *)prWifiCmd,
		prCmdInfo->u2InfoBufLen);
	HAL_MAC_CONNAC2X_TXD_SET_PKT_FORMAT(
		(struct HW_MAC_CONNAC2X_TX_DESC *)prWifiCmd,
		INIT_PKT_FT_CMD);
	HAL_MAC_CONNAC2X_TXD_SET_HEADER_FORMAT(
		(struct HW_MAC_CONNAC2X_TX_DESC *)prWifiCmd,
		HEADER_FORMAT_COMMAND);

	prWifiCmd->ucCID = prCmdInfo->ucCID;
	prWifiCmd->ucExtenCID = prCmdInfo->ucExtCID;
	prWifiCmd->ucPktTypeID = prCmdInfo->ucPktTypeID;
	prWifiCmd->ucSetQuery = prCmdInfo->ucSetQuery;
	prWifiCmd->ucSeqNum = nicIncreaseCmdSeqNum(prAdapter);
	prWifiCmd->ucS2DIndex = prCmdInfo->ucS2DIndex;
	prWifiCmd->u2Length = prCmdInfo->u2InfoBufLen - u4TxdLen;

	if (pucSeqNum)
		*pucSeqNum = prWifiCmd->ucSeqNum;

	DBGLOG(INIT, INFO, "TX CMD: ID[0x%02X] SEQ[%u] SET[%u] LEN[%u]\n",
			prWifiCmd->ucCID, prWifiCmd->ucSeqNum,
			prWifiCmd->ucSetQuery, prCmdInfo->u2InfoBufLen);
}

void asicConnac2xFillInitCmdTxd(
	struct ADAPTER *prAdapter,
	struct WIFI_CMD_INFO *prCmdInfo,
	u_int16_t *pu2BufInfoLen,
	u_int8_t *pucSeqNum, OUT void **pCmdBuf)
{
	struct INIT_HIF_TX_HEADER *prInitHifTxHeader;
	uint32_t u4TxdLen = sizeof(struct HW_MAC_CONNAC2X_TX_DESC);

	/* We don't need to append TXD while sending fw frames. */
	if (!prCmdInfo->ucCID && pCmdBuf)
		*pCmdBuf = prCmdInfo->pucInfoBuffer;
	else {
		prInitHifTxHeader = (struct INIT_HIF_TX_HEADER *)
			(prCmdInfo->pucInfoBuffer + u4TxdLen);
		asicConnac2xFillInitCmdTxdInfo(
			prAdapter,
			prCmdInfo,
			pucSeqNum);
		if (pCmdBuf)
			*pCmdBuf =
				&prInitHifTxHeader->rInitWifiCmd.aucBuffer[0];
	}
}

void asicConnac2xFillCmdTxd(
	struct ADAPTER *prAdapter,
	struct WIFI_CMD_INFO *prCmdInfo,
	u_int8_t *pucSeqNum,
	void **pCmdBuf)
{
	struct CONNAC2X_WIFI_CMD *prWifiCmd;

	/* 2. Setup common CMD Info Packet */
	prWifiCmd = (struct CONNAC2X_WIFI_CMD *)prCmdInfo->pucInfoBuffer;
	asicConnac2xFillCmdTxdInfo(prAdapter, prCmdInfo, pucSeqNum);
	if (pCmdBuf)
		*pCmdBuf = &prWifiCmd->aucBuffer[0];
}

#if defined(_HIF_USB)
/*
 * tx_ring config
 * 1.1tx_ring_ext_ctrl
 * 7c025600[7:0]: 0x4 (ring 0 max_cnt for EP4)
 * 7c025604[7:0]: 0x4 (ring 1 max_cnt for EP5)
 * 7c025608[7:0]: 0x4 (ring 2 max_cnt for EP6)
 * 7c02560C[7:0]: 0x4 (ring 3 max_cnt for EP7)
 * 7c025610[7:0]: 0x4 (ring 4 max_cnt for EP9)
 * 7c025640[7:0]: 0x4 (ring 16 max_cnt for EP4/FWDL)
 * 7c025644[7:0]: 0x4 (ring 17 max_cnt for EP8/WMCPU)
 * 7c025650[7:0]: 0x4 (ring 20 max_cnt for EP8/WACPU)
 *
 * WFDMA_GLO_CFG Setting
 * 2.1 WFDMA_GLO_CFG: 7c025208[28][27]=2'b11;
 * 2.2 WFDMA_GLO_CFG: 7c025208[20]=1'b1;
 * 2.3 WFDMA_GLO_CFG: 7c025208[9]=1'b1;
 *
 * 3.	trx_dma_en:
 * 3.1 WFDMA_GLO_CFG: 7c025208[2][0]=1'b1;
 */
void asicConnac2xWfdmaInitForUSB(
	struct ADAPTER *prAdapter,
	struct mt66xx_chip_info *prChipInfo)
{
	uint32_t idx;
	uint32_t u4WfdmaAddr, u4WfdmaCr;

	u4WfdmaAddr =
		CONNAC2X_TX_RING_EXT_CTRL_BASE(CONNAC2X_HOST_WPDMA_1_BASE);
	/*
	 * HOST_DMA1_WPDMA_TX_RING0_EXT_CTRL ~ HOST_DMA1_WPDMA_TX_RING4_EXT_CTRL
	 */
	for (idx = 0; idx < USB_TX_EPOUT_NUM; idx++) {
		HAL_MCR_RD(prAdapter, u4WfdmaAddr + (idx*4), &u4WfdmaCr);
		u4WfdmaCr &= ~CONNAC2X_WFDMA_DISP_MAX_CNT_MASK;
		u4WfdmaCr |= CONNAC2X_TX_RING_DISP_MAX_CNT;
		HAL_MCR_WR(prAdapter, u4WfdmaAddr + (idx*4), u4WfdmaCr);
	}

	/* HOST_DMA1_WPDMA_TX_RING16_EXT_CTRL_ADDR */
	HAL_MCR_RD(prAdapter, u4WfdmaAddr + 0x40, &u4WfdmaCr);
	u4WfdmaCr &= ~CONNAC2X_WFDMA_DISP_MAX_CNT_MASK;
	u4WfdmaCr |= CONNAC2X_TX_RING_DISP_MAX_CNT;
	HAL_MCR_WR(prAdapter, u4WfdmaAddr + 0x40, u4WfdmaCr);

	/* HOST_DMA1_WPDMA_TX_RING17_EXT_CTRL_ADDR */
	HAL_MCR_RD(prAdapter, u4WfdmaAddr + 0x44, &u4WfdmaCr);
	u4WfdmaCr &= ~CONNAC2X_WFDMA_DISP_MAX_CNT_MASK;
	u4WfdmaCr |= CONNAC2X_TX_RING_DISP_MAX_CNT;
	HAL_MCR_WR(prAdapter, u4WfdmaAddr + 0x44, u4WfdmaCr);

	/* HOST_DMA1_WPDMA_TX_RING20_EXT_CTRL_ADDR */
	HAL_MCR_RD(prAdapter, u4WfdmaAddr + 0x50, &u4WfdmaCr);
	u4WfdmaCr &= ~CONNAC2X_WFDMA_DISP_MAX_CNT_MASK;
	u4WfdmaCr |= CONNAC2X_TX_RING_DISP_MAX_CNT;
	HAL_MCR_WR(prAdapter, u4WfdmaAddr + 0x50, u4WfdmaCr);

	u4WfdmaAddr = CONNAC2X_WPDMA_GLO_CFG(CONNAC2X_HOST_WPDMA_1_BASE);
	HAL_MCR_RD(prAdapter, u4WfdmaAddr, &u4WfdmaCr);
	u4WfdmaCr |=
		(CONNAC2X_WPDMA1_GLO_CFG_OMIT_TX_INFO |
		 CONNAC2X_WPDMA1_GLO_CFG_OMIT_RX_INFO |
		 CONNAC2X_WPDMA1_GLO_CFG_FW_DWLD_Bypass_dmashdl |
		 CONNAC2X_WPDMA1_GLO_CFG_RX_DMA_EN |
		 CONNAC2X_WPDMA1_GLO_CFG_TX_DMA_EN);
	HAL_MCR_WR(prAdapter, u4WfdmaAddr, u4WfdmaCr);

	/* Enable WFDMA0 RX for receiving data frame */
	u4WfdmaAddr = CONNAC2X_WPDMA_GLO_CFG(CONNAC2X_HOST_WPDMA_0_BASE);
	HAL_MCR_RD(prAdapter, u4WfdmaAddr, &u4WfdmaCr);
	u4WfdmaCr |=
		(CONNAC2X_WPDMA1_GLO_CFG_RX_DMA_EN);
	HAL_MCR_WR(prAdapter, u4WfdmaAddr, u4WfdmaCr);

	/* 7915U E1 Workaround. TODO: check chip version */
	/* Driver need to write rx ring cpu index for receiving data */
	HAL_MCR_WR(prAdapter,
		CONNAC2X_RX_RING_CIDX(CONNAC2X_HOST_WPDMA_0_BASE), 0x1);

	prChipInfo->is_support_dma_shdl = wlanCfgGetUint32(prAdapter,
				    "DmaShdlEnable",
				    FEATURE_DISABLED);
	if (!prChipInfo->is_support_dma_shdl) {
		/*
		 *	To disable 0x7C0252B0[6] DMASHDL
		 */
		u4WfdmaAddr = CONNAC2X_WPDMA_GLO_CFG_EXT0(
					CONNAC2X_HOST_WPDMA_1_BASE);
		HAL_MCR_RD(prAdapter, u4WfdmaAddr, &u4WfdmaCr);
		u4WfdmaCr &= ~CONNAC2X_WPDMA1_GLO_CFG_EXT0_TX_DMASHDL_EN;
		HAL_MCR_WR(prAdapter, u4WfdmaAddr, u4WfdmaCr);

		/*
		 *	[28]DMASHDL_BYPASS
		 *	DMASHDL host ask and quota control function bypass
		 *	0: Disable
		 *	1: Enable
		 */
		u4WfdmaAddr = CONNAC2X_HOST_DMASHDL_SW_CONTROL(
					CONNAC2X_HOST_DMASHDL);
		HAL_MCR_RD(prAdapter, u4WfdmaAddr, &u4WfdmaCr);
		u4WfdmaCr |= CONNAC2X_HIF_DMASHDL_BYPASS_EN;
		HAL_MCR_WR(prAdapter, u4WfdmaAddr, u4WfdmaCr);
	}

}

void asicConnac2xEnableUsbCmdTxRing(
	struct ADAPTER *prAdapter,
	u_int8_t ucDstRing)
{
	struct GLUE_INFO *prGlueInfo;
	struct BUS_INFO *prBusInfo;
	uint32_t u4Value = 0;
	uint32_t u4WfdmaValue = 0;
	uint32_t i = 0;

	ASSERT(prAdapter);

	prGlueInfo = prAdapter->prGlueInfo;
	prBusInfo = prAdapter->chip_info->bus_info;

	HAL_MCR_RD(prAdapter, CONNAC2X_WFDMA_HOST_CONFIG_ADDR, &u4WfdmaValue);
	u4WfdmaValue &= ~CONNAC2X_WFDMA_HOST_CONFIG_USB_CMDPKT_DST_MASK;
	u4WfdmaValue |= CONNAC2X_WFDMA_HOST_CONFIG_USB_CMDPKT_DST(ucDstRing);
	do {
		HAL_MCR_RD(prAdapter,
			CONNAC2X_WPDMA_GLO_CFG(CONNAC2X_HOST_WPDMA_1_BASE),
			&u4Value);
		if ((u4Value & CONNAC2X_WPDMA1_GLO_CFG_TX_DMA_BUSY) == 0) {
			u4Value &= ~CONNAC2X_WPDMA1_GLO_CFG_TX_DMA_EN;
			HAL_MCR_WR(prAdapter,
				CONNAC2X_WPDMA_GLO_CFG(
					CONNAC2X_HOST_WPDMA_1_BASE),
				u4Value);
			break;
		}
		kalUdelay(1000);
	} while ((i++) < 100);
	if (i > 100)
		DBGLOG(HAL, ERROR, "WFDMA1 TX keep busy....\n");
	else {
		HAL_MCR_WR(prAdapter,
				CONNAC2X_WFDMA_HOST_CONFIG_ADDR,
				u4WfdmaValue);
		HAL_MCR_RD(prAdapter,
			CONNAC2X_WPDMA_GLO_CFG(CONNAC2X_HOST_WPDMA_1_BASE),
			&u4Value);
		u4Value |= CONNAC2X_WPDMA1_GLO_CFG_TX_DMA_EN;
		HAL_MCR_WR(prAdapter,
			CONNAC2X_WPDMA_GLO_CFG(CONNAC2X_HOST_WPDMA_1_BASE),
			u4Value);
	}
}

#if CFG_ENABLE_FW_DOWNLOAD
void asicConnac2xEnableUsbFWDL(
	struct ADAPTER *prAdapter,
	u_int8_t fgEnable)
{
	struct GLUE_INFO *prGlueInfo;
	struct BUS_INFO *prBusInfo;
	uint32_t u4Value = 0;
	uint32_t u4WfdmaValue = 0;

	ASSERT(prAdapter);

	prGlueInfo = prAdapter->prGlueInfo;
	prBusInfo = prAdapter->chip_info->bus_info;

	HAL_MCR_RD(prAdapter, prBusInfo->u4UdmaTxQsel, &u4Value);
	HAL_MCR_RD(prAdapter, CONNAC2X_WFDMA_HOST_CONFIG_ADDR, &u4WfdmaValue);
	if (fgEnable) {
		u4Value |= FW_DL_EN;
		/* command packet forward to TX ring 17 (WMCPU) */
		asicConnac2xEnableUsbCmdTxRing(prAdapter,
			CONNAC2X_USB_CMDPKT2WM);
	} else {
		u4Value &= ~FW_DL_EN;
		/* command packet forward to TX ring 20 (WACPU) */
		asicConnac2xEnableUsbCmdTxRing(prAdapter,
			CONNAC2X_USB_CMDPKT2WA);
	}

	HAL_MCR_WR(prAdapter, prBusInfo->u4UdmaTxQsel, u4Value);
}
#endif /* CFG_ENABLE_FW_DOWNLOAD */

#endif /* _HIF_USB */

void fillConnac2xTxDescTxByteCount(
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
	HAL_MAC_CONNAC2X_TXD_SET_TX_BYTE_COUNT(
		(struct HW_MAC_CONNAC2X_TX_DESC *)prTxDesc, u4TxByteCount);
}

void fillConnac2xTxDescAppendWithWaCpu(
	struct ADAPTER *prAdapter,
	struct MSDU_INFO *prMsduInfo,
	uint8_t *prTxDescBuffer)
{
	struct mt66xx_chip_info *prChipInfo = prAdapter->chip_info;
	union HW_MAC_TX_DESC_APPEND *prHwTxDescAppend;

	/* Fill TxD append */
	prHwTxDescAppend = (union HW_MAC_TX_DESC_APPEND *)
			   prTxDescBuffer;
	kalMemZero(prHwTxDescAppend, prChipInfo->txd_append_size);
	prHwTxDescAppend->CR4_APPEND.u2PktFlags =
		HIF_PKT_FLAGS_CT_INFO_APPLY_TXD;
	prHwTxDescAppend->CR4_APPEND.ucBssIndex =
		prMsduInfo->ucBssIndex;
	prHwTxDescAppend->CR4_APPEND.ucWtblIndex =
		prMsduInfo->ucWlanIndex;
}

void fillConnac2xTxDescAppendByWaCpu(
	struct ADAPTER *prAdapter,
	struct MSDU_INFO *prMsduInfo,
	uint16_t u4MsduId,
	dma_addr_t rDmaAddr,
	uint32_t u4Idx,
	u_int8_t fgIsLast,
	uint8_t *pucBuffer)
{
	union HW_MAC_TX_DESC_APPEND *prHwTxDescAppend;

	prHwTxDescAppend = (union HW_MAC_TX_DESC_APPEND *)
		(pucBuffer + NIC_TX_DESC_LONG_FORMAT_LENGTH);
	prHwTxDescAppend->CR4_APPEND.u2MsduToken = u4MsduId;
	prHwTxDescAppend->CR4_APPEND.ucBufNum = u4Idx + 1;
	prHwTxDescAppend->CR4_APPEND.au4BufPtr[u4Idx] = rDmaAddr;
	prHwTxDescAppend->CR4_APPEND.au2BufLen[u4Idx] =
		prMsduInfo->u2FrameLength;
}

void fillTxDescTxByteCountWithWaCpu(
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
		u4TxByteCount += prChipInfo->txd_append_size;

	/* Calculate Tx byte count */
	HAL_MAC_CONNAC2X_TXD_SET_TX_BYTE_COUNT(
		(struct HW_MAC_CONNAC2X_TX_DESC *)prTxDesc, u4TxByteCount);
}

void asicConnac2xInitTxdHook(
	struct TX_DESC_OPS_T *prTxDescOps)
{
	ASSERT(prTxDescOps);
	prTxDescOps->nic_txd_long_format_op = nic_txd_v2_long_format_op;
	prTxDescOps->nic_txd_tid_op = nic_txd_v2_tid_op;
	prTxDescOps->nic_txd_queue_idx_op = nic_txd_v2_queue_idx_op;
#if (CFG_TCP_IP_CHKSUM_OFFLOAD == 1)
	prTxDescOps->nic_txd_chksum_op = nic_txd_v2_chksum_op;
#endif /* CFG_TCP_IP_CHKSUM_OFFLOAD == 1 */
	prTxDescOps->nic_txd_header_format_op = nic_txd_v2_header_format_op;
	prTxDescOps->nic_txd_fill_by_pkt_option = nic_txd_v2_fill_by_pkt_option;
	prTxDescOps->nic_txd_compose = nic_txd_v2_compose;
	prTxDescOps->nic_txd_compose_security_frame =
		nic_txd_v2_compose_security_frame;
	prTxDescOps->nic_txd_set_pkt_fixed_rate_option_full =
		nic_txd_v2_set_pkt_fixed_rate_option_full;
	prTxDescOps->nic_txd_set_pkt_fixed_rate_option =
		nic_txd_v2_set_pkt_fixed_rate_option;
	prTxDescOps->nic_txd_set_hw_amsdu_template =
		nic_txd_v2_set_hw_amsdu_template;
}

void asicConnac2xInitRxdHook(
	struct RX_DESC_OPS_T *prRxDescOps)
{
	ASSERT(prRxDescOps);
	prRxDescOps->nic_rxd_get_rx_byte_count = nic_rxd_v2_get_rx_byte_count;
	prRxDescOps->nic_rxd_get_pkt_type = nic_rxd_v2_get_packet_type;
	prRxDescOps->nic_rxd_get_wlan_idx = nic_rxd_v2_get_wlan_idx;
	prRxDescOps->nic_rxd_get_sec_mode = nic_rxd_v2_get_sec_mode;
	prRxDescOps->nic_rxd_get_sw_class_error_bit =
		nic_rxd_v2_get_sw_class_error_bit;
	prRxDescOps->nic_rxd_get_ch_num = nic_rxd_v2_get_ch_num;
	prRxDescOps->nic_rxd_get_rf_band = nic_rxd_v2_get_rf_band;
	prRxDescOps->nic_rxd_get_tcl = nic_rxd_v2_get_tcl;
	prRxDescOps->nic_rxd_get_ofld = nic_rxd_v2_get_ofld;
	prRxDescOps->nic_rxd_fill_rfb = nic_rxd_v2_fill_rfb;
	prRxDescOps->nic_rxd_sanity_check = nic_rxd_v2_sanity_check;
	prRxDescOps->nic_rxd_check_wakeup_reason =
		nic_rxd_v2_check_wakeup_reason;
}

#if (CFG_SUPPORT_MSP == 1)
void asicConnac2xRxProcessRxvforMSP(IN struct ADAPTER *prAdapter,
	  IN OUT struct SW_RFB *prRetSwRfb) {
	struct HW_MAC_RX_STS_GROUP_3_V2 *prGroup3;

	if (prRetSwRfb->ucStaRecIdx >= CFG_STA_REC_NUM) {
		DBGLOG(RX, WARN,
		"prRetSwRfb->ucStaRecIdx(%d) >= CFG_STA_REC_NUM(%d)\n",
			prRetSwRfb->ucStaRecIdx, CFG_STA_REC_NUM);
		return;
	}

	prGroup3 =
		(struct HW_MAC_RX_STS_GROUP_3_V2 *)prRetSwRfb->prRxStatusGroup3;
	if (prRetSwRfb->ucGroupVLD & BIT(RX_GROUP_VLD_3)) {
		prAdapter->arStaRec[
			prRetSwRfb->ucStaRecIdx].u4RxVector0 =
			CONNAC2X_HAL_RX_VECTOR_GET_RX_VECTOR(prGroup3, 0);
	} else {
		prAdapter->arStaRec[
			prRetSwRfb->ucStaRecIdx].u4RxVector0 = 0;
	}

	if (prRetSwRfb->ucGroupVLD & BIT(RX_GROUP_VLD_5)) {
		prAdapter->arStaRec[
			prRetSwRfb->ucStaRecIdx].u4RxVector1 =
			CONNAC2X_HAL_RX_VECTOR_GET_RX_VECTOR(
			prRetSwRfb->prRxStatusGroup5, 0);

		prAdapter->arStaRec[
			prRetSwRfb->ucStaRecIdx].u4RxVector2 =
			CONNAC2X_HAL_RX_VECTOR_GET_RX_VECTOR(
			prRetSwRfb->prRxStatusGroup5, 2);

		prAdapter->arStaRec[
			prRetSwRfb->ucStaRecIdx].u4RxVector3 =
			CONNAC2X_HAL_RX_VECTOR_GET_RX_VECTOR(
			prRetSwRfb->prRxStatusGroup5, 4);

		prAdapter->arStaRec[
			prRetSwRfb->ucStaRecIdx].u4RxVector4 =
			CONNAC2X_HAL_RX_VECTOR_GET_RX_VECTOR(
			prRetSwRfb->prRxStatusGroup5, 6);
	} else {
		prAdapter->arStaRec[
			prRetSwRfb->ucStaRecIdx].u4RxVector1 = 0;
		prAdapter->arStaRec[
			prRetSwRfb->ucStaRecIdx].u4RxVector2 = 0;
		prAdapter->arStaRec[
			prRetSwRfb->ucStaRecIdx].u4RxVector3 = 0;
		prAdapter->arStaRec[
			prRetSwRfb->ucStaRecIdx].u4RxVector4 = 0;
	}
}
#endif /* CFG_SUPPORT_MSP == 1 */

uint8_t asicConnac2xRxGetRcpiValueFromRxv(
	IN uint8_t ucRcpiMode,
	IN struct SW_RFB *prSwRfb)
{
#if 1
	/* TODO: need to read RCPI from WTBL */
	DBGLOG(RX, WARN,
		"asicConnac2xRxGetRcpiValueFromRxv:: TODO - read from WTBL?\n");
	return 0;
#else
	uint8_t ucRcpi0, ucRcpi1;
	uint8_t ucRcpiValue = 0;
	uint8_t ucRxNum;

	ASSERT(prSwRfb);

	if (ucRcpiMode >= RCPI_MODE_NUM) {
		DBGLOG(RX, WARN,
		"Rcpi Mode=%d is invalid for getting uint8_t value from RXV\n",
		ucRcpiMode);
		return 0;
	}

	ucRcpi0 = HAL_RX_STATUS_GET_RCPI0(
			  prSwRfb->prRxStatusGroup3);
	ucRcpi1 = HAL_RX_STATUS_GET_RCPI1(
			  prSwRfb->prRxStatusGroup3);
	ucRxNum = HAL_RX_STATUS_GET_RX_NUM(
			  prSwRfb->prRxStatusGroup3);

	if (ucRxNum == 0)
		ucRcpiValue =
			ucRcpi0; /*0:1R, BBP always report RCPI0 at 1R mode*/

	else if (ucRxNum == 1) {
		switch (ucRcpiMode) {
		case RCPI_MODE_WF0:
			ucRcpiValue = ucRcpi0;
			break;

		case RCPI_MODE_WF1:
			ucRcpiValue = ucRcpi1;
			break;

		case RCPI_MODE_WF2:
		case RCPI_MODE_WF3:
			DBGLOG(RX, WARN,
			"Rcpi Mode=%d is invalid for",
			       ucRcpiMode);
			DBGLOG(RX, WARN,
			" device with only 2 antenna, use default rcpi0\n");
			ucRcpiValue = ucRcpi0;
			break;

		case RCPI_MODE_AVG: /*Not recommended for CBW80+80*/
			if (ucRcpi0 <= RCPI_HIGH_BOUND &&
				ucRcpi1 <= RCPI_HIGH_BOUND)
				ucRcpiValue = (ucRcpi0 + ucRcpi1) / 2;
			else
				ucRcpiValue = ucRcpi0 <= RCPI_HIGH_BOUND ?
					(ucRcpi0) : (ucRcpi1);
			break;

		case RCPI_MODE_MAX:
			if (ucRcpi0 <= RCPI_HIGH_BOUND &&
				ucRcpi1 <= RCPI_HIGH_BOUND)
				ucRcpiValue =
					(ucRcpi0 > ucRcpi1) ?
					(ucRcpi0) : (ucRcpi1);
			else
				ucRcpiValue = ucRcpi0 <= RCPI_HIGH_BOUND ?
					(ucRcpi0) : (ucRcpi1);
			break;

		case RCPI_MODE_MIN:
			ucRcpiValue =
				(ucRcpi0 < ucRcpi1) ? (ucRcpi0) : (ucRcpi1);
			break;

		default:
			break;
		}
	} else {
		DBGLOG(RX, WARN,
		"RX_NUM = %d is invalid for getting uint8_t value from RXV\n",
		ucRxNum);
		return 0;
	}

	if (ucRcpiValue < RCPI_MEASUREMENT_NOT_AVAILABLE)
		return ucRcpiValue;

	DBGLOG(RX, ERROR,
	       "ucRcpiValue == RCPI_MEASUREMENT_NOT_AVAILABLE ??\n");
	return 0;
#endif
}

#endif /* CFG_SUPPORT_CONNAC2X == 1 */
