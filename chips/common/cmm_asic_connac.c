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
/*! \file   connac.c
*    \brief  Internal driver stack will export the required procedures here for GLUE Layer.
*
*    This file contains all routines which are exported from MediaTek 802.11 Wireless
*    LAN driver stack to GLUE Layer.
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
*                            P U B L I C   D A T A
********************************************************************************
*/

/*
* Common Default Setting
* TODO: This table can be modified by command or cfg file.
*/
UINT_8 arAcQIdx2GroupId[MAC_TXQ_NUM] = {
	GROUP0_INDEX,    /* MAC_TXQ_AC0_INDEX */
	GROUP1_INDEX,    /* MAC_TXQ_AC1_INDEX */
	GROUP2_INDEX,    /* MAC_TXQ_AC2_INDEX */
	GROUP4_INDEX,    /* MAC_TXQ_AC3_INDEX */

	GROUP0_INDEX,    /* MAC_TXQ_AC10_INDEX */
	GROUP1_INDEX,    /* MAC_TXQ_AC11_INDEX */
	GROUP2_INDEX,    /* MAC_TXQ_AC12_INDEX */
	GROUP4_INDEX,    /* MAC_TXQ_AC13_INDEX */

	GROUP0_INDEX,    /* MAC_TXQ_AC20_INDEX */
	GROUP1_INDEX,    /* MAC_TXQ_AC21_INDEX */
	GROUP2_INDEX,    /* MAC_TXQ_AC22_INDEX */
	GROUP4_INDEX,    /* MAC_TXQ_AC23_INDEX */

	GROUP0_INDEX,    /* MAC_TXQ_AC30_INDEX */
	GROUP1_INDEX,    /* MAC_TXQ_AC31_INDEX */
	GROUP2_INDEX,    /* MAC_TXQ_AC32_INDEX */
	GROUP4_INDEX,    /* MAC_TXQ_AC33_INDEX */

	GROUP5_INDEX,    /* MAC_TXQ_ALTX_0_INDEX */
	GROUP5_INDEX,    /* MAC_TXQ_BMC_0_INDEX */
	GROUP5_INDEX,    /* MAC_TXQ_BCN_0_INDEX */
	GROUP5_INDEX,    /* MAC_TXQ_PSMP_0_INDEX */

	GROUP5_INDEX,    /* Reserved */
	GROUP5_INDEX,    /* Reserved */
	GROUP5_INDEX,    /* Reserved */
	GROUP5_INDEX,    /* Reserved */

	GROUP5_INDEX,    /* Reserved */
	GROUP5_INDEX,    /* Reserved */
};

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/
#if defined(_HIF_USB)
#define USB_DMA_SHDL_GROUP_DEF_MIN_QUOTA 0x3
#define USB_DMA_SHDL_GROUP_DEF_MAX_QUOTA 0x1FF
#define USB_ACCESS_RETRY_LIMIT           1
#endif /* _HIF_USB */

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/
VOID asicCapInit(IN P_ADAPTER_T prAdapter)
{
	P_GLUE_INFO_T prGlueInfo;
	struct mt66xx_chip_info *prChipInfo;

	ASSERT(prAdapter);

	prGlueInfo = prAdapter->prGlueInfo;
	prChipInfo = prAdapter->chip_info;

	prChipInfo->u2HifTxdSize = 0;
	prChipInfo->u2TxInitCmdPort = 0;
	prChipInfo->u2TxFwDlPort = 0;
	prChipInfo->fillHifTxDesc = NULL;
	prChipInfo->u4ExtraTxByteCount = 0;

	switch (prGlueInfo->u4InfType) {
#if defined(_HIF_PCIE)
	case MT_DEV_INF_PCIE:
		prChipInfo->u2TxInitCmdPort = TX_RING_FWDL_IDX_3;
		prChipInfo->u2TxFwDlPort = TX_RING_FWDL_IDX_3;
		prChipInfo->ucPacketFormat = TXD_PKT_FORMAT_TXD;
		prChipInfo->u4HifDmaShdlBaseAddr = PCIE_HIF_DMASHDL_BASE;

		HAL_MCR_WR(prAdapter, CONN_HIF_ON_IRQ_ENA, BIT(0));
		asicPcieDmaShdlInit(prAdapter);
		break;
#endif /* _HIF_PCIE */
#if defined(_HIF_USB)
	case MT_DEV_INF_USB:
		prChipInfo->u2HifTxdSize = USB_HIF_TXD_LEN;
		prChipInfo->fillHifTxDesc = fillUsbHifTxDesc;
		prChipInfo->u2TxInitCmdPort = USB_DATA_BULK_OUT_EP8;
		prChipInfo->u2TxFwDlPort = USB_DATA_BULK_OUT_EP4;
		prChipInfo->ucPacketFormat = TXD_PKT_FORMAT_TXD_PAYLOAD;
		prChipInfo->u4ExtraTxByteCount = EXTRA_TXD_SIZE_FOR_TX_BYTE_COUNT;
		prChipInfo->u4HifDmaShdlBaseAddr = USB_HIF_DMASHDL_BASE;
		asicUsbDmaShdlInit(prAdapter);
		asicUdmaTxTimeoutEnable(prAdapter);
		asicUdmaRxFlush(prAdapter, FALSE);
		asicPdmaHifReset(prAdapter, TRUE);
		break;
#endif /* _HIF_USB */
#if defined(_HIF_SDIO)
	case MT_DEV_INF_SDIO:
		prChipInfo->ucPacketFormat = TXD_PKT_FORMAT_TXD_PAYLOAD;
		prChipInfo->u4ExtraTxByteCount = EXTRA_TXD_SIZE_FOR_TX_BYTE_COUNT;
		break;
#endif /* _HIF_SDIO */
	default:
		break;
	}
}

VOID asicEnableFWDownload(IN P_ADAPTER_T prAdapter, IN BOOL fgEnable)
{
	P_GLUE_INFO_T prGlueInfo;

	ASSERT(prAdapter);

	prGlueInfo = prAdapter->prGlueInfo;

	switch (prGlueInfo->u4InfType) {
#if defined(_HIF_PCIE)
	case MT_DEV_INF_PCIE:
	{
		WPDMA_GLO_CFG_STRUCT GloCfg;

		kalDevRegRead(prGlueInfo, WPDMA_GLO_CFG, &GloCfg.word);

		GloCfg.field_conn.bypass_dmashdl_txring3 = fgEnable;

		kalDevRegWrite(prGlueInfo, WPDMA_GLO_CFG, GloCfg.word);
	}
	break;
#endif /* _HIF_PCIE */

#if defined(_HIF_USB)
	case MT_DEV_INF_USB:
	{
		UINT_32 u4Value = 0;

		ASSERT(prAdapter);

		HAL_MCR_RD(prAdapter, CONNAC_UDMA_TX_QSEL, &u4Value);

		if (fgEnable)
			u4Value |= FW_DL_EN;
		else
			u4Value &= ~FW_DL_EN;

		HAL_MCR_WR(prAdapter, CONNAC_UDMA_TX_QSEL, u4Value);
	}
	break;
#endif /* _HIF_USB */

	default:
		break;
	}
}

VOID fillNicTxDescAppend(IN P_ADAPTER_T prAdapter, IN P_MSDU_INFO_T prMsduInfo,
	OUT PUINT_8 prTxDescBuffer)
{
	struct mt66xx_chip_info *prChipInfo = prAdapter->chip_info;
	P_HW_MAC_TX_DESC_APPEND_T prHwTxDescAppend;

	/* Fill TxD append */
	prHwTxDescAppend = (P_HW_MAC_TX_DESC_APPEND_T)prTxDescBuffer;
	kalMemZero(prHwTxDescAppend, prChipInfo->txd_append_size);
}

VOID fillNicTxDescAppendWithCR4(IN P_ADAPTER_T prAdapter, IN P_MSDU_INFO_T prMsduInfo,
	OUT PUINT_8 prTxDescBuffer)
{
	struct mt66xx_chip_info *prChipInfo = prAdapter->chip_info;
	P_HW_MAC_TX_DESC_APPEND_T prHwTxDescAppend;

	/* Fill TxD append */
	prHwTxDescAppend = (P_HW_MAC_TX_DESC_APPEND_T)prTxDescBuffer;
	kalMemZero(prHwTxDescAppend, prChipInfo->txd_append_size);
	prHwTxDescAppend->CR4_APPEND.u2PktFlags = HIT_PKT_FLAGS_CT_WITH_TXD;
	prHwTxDescAppend->CR4_APPEND.ucBssIndex = prMsduInfo->ucBssIndex;
}

VOID fillTxDescAppendByHost(IN P_ADAPTER_T prAdapter, IN P_MSDU_INFO_T prMsduInfo, IN UINT_16 u4MsduId,
			    IN dma_addr_t rDmaAddr, IN UINT_32 u4Idx, IN BOOLEAN fgIsLast,
			    OUT PUINT_8 pucBuffer)
{
	P_HW_MAC_TX_DESC_APPEND_T prHwTxDescAppend;
	P_TXD_PTR_LEN_T prPtrLen;

	prHwTxDescAppend = (P_HW_MAC_TX_DESC_APPEND_T) (pucBuffer + NIC_TX_DESC_LONG_FORMAT_LENGTH);
	prHwTxDescAppend->CONNAC_APPEND.au2MsduId[u4Idx] = u4MsduId | TXD_MSDU_ID_VLD;
	prPtrLen = &prHwTxDescAppend->CONNAC_APPEND.arPtrLen[u4Idx >> 1];
	if ((u4Idx & 1) == 0) {
		prPtrLen->u4Ptr0 = rDmaAddr;
		prPtrLen->u2Len0 = prMsduInfo->u2FrameLength;
		if (fgIsLast)
			prPtrLen->u2Len0 |= TXD_LEN_AL | TXD_LEN_ML;
	} else {
		prPtrLen->u4Ptr1 = rDmaAddr;
		prPtrLen->u2Len1 = prMsduInfo->u2FrameLength;
		if (fgIsLast)
			prPtrLen->u2Len1 |= TXD_LEN_AL | TXD_LEN_ML;
	}
}

VOID fillTxDescAppendByHostV2(IN P_ADAPTER_T prAdapter, IN P_MSDU_INFO_T prMsduInfo, IN UINT_16 u4MsduId,
			      IN dma_addr_t rDmaAddr, IN UINT_32 u4Idx, IN BOOLEAN fgIsLast,
			      OUT PUINT_8 pucBuffer)
{
	P_HW_MAC_TX_DESC_APPEND_T prHwTxDescAppend;
	P_TXD_PTR_LEN_T prPtrLen;

	prHwTxDescAppend = (P_HW_MAC_TX_DESC_APPEND_T) (pucBuffer + NIC_TX_DESC_LONG_FORMAT_LENGTH);
	prHwTxDescAppend->CONNAC_APPEND.au2MsduId[u4Idx] = u4MsduId | TXD_MSDU_ID_VLD;
	prPtrLen = &prHwTxDescAppend->CONNAC_APPEND.arPtrLen[u4Idx >> 1];
	if ((u4Idx & 1) == 0) {
		prPtrLen->u4Ptr0 = rDmaAddr;
		prPtrLen->u2Len0 = (prMsduInfo->u2FrameLength & TXD_LEN_MASK_V2)
			| ((rDmaAddr >> TXD_ADDR2_OFFSET) & TXD_ADDR2_MASK);
		if (fgIsLast)
			prPtrLen->u2Len0 |= TXD_LEN_ML_V2;
	} else {
		prPtrLen->u4Ptr1 = rDmaAddr;
		prPtrLen->u2Len1 = (prMsduInfo->u2FrameLength & TXD_LEN_MASK_V2)
			| ((rDmaAddr >> TXD_ADDR2_OFFSET) & TXD_ADDR2_MASK);
		if (fgIsLast)
			prPtrLen->u2Len1 |= TXD_LEN_ML_V2;
	}
}

VOID fillTxDescAppendByCR4(IN P_ADAPTER_T prAdapter, IN P_MSDU_INFO_T prMsduInfo, IN UINT_16 u4MsduId,
			   IN dma_addr_t rDmaAddr, IN UINT_32 u4Idx, IN BOOLEAN fgIsLast,
			   OUT PUINT_8 pucBuffer)
{
	P_HW_MAC_TX_DESC_APPEND_T prHwTxDescAppend;

	prHwTxDescAppend = (P_HW_MAC_TX_DESC_APPEND_T) (pucBuffer + NIC_TX_DESC_LONG_FORMAT_LENGTH);
	prHwTxDescAppend->CR4_APPEND.u2MsduToken = u4MsduId;
	prHwTxDescAppend->CR4_APPEND.ucBufNum = u4Idx + 1;
	prHwTxDescAppend->CR4_APPEND.au4BufPtr[u4Idx] = rDmaAddr;
	prHwTxDescAppend->CR4_APPEND.au2BufLen[u4Idx] = prMsduInfo->u2FrameLength;
}

VOID fillTxDescTxByteCount(IN P_ADAPTER_T prAdapter, IN P_MSDU_INFO_T prMsduInfo,
			   P_HW_MAC_TX_DESC_T prTxDesc)
{
	struct mt66xx_chip_info *prChipInfo;
	UINT_32 u4TxByteCount = NIC_TX_DESC_LONG_FORMAT_LENGTH;

	ASSERT(prAdapter);
	ASSERT(prMsduInfo);
	ASSERT(prTxDesc);

	prChipInfo = prAdapter->chip_info;
	u4TxByteCount += prMsduInfo->u2FrameLength;

	if (prMsduInfo->ucPacketType == TX_PACKET_TYPE_DATA)
		u4TxByteCount += prChipInfo->u4ExtraTxByteCount;

	/* Calculate Tx byte count */
	HAL_MAC_TX_DESC_SET_TX_BYTE_COUNT(prTxDesc, u4TxByteCount);
}

VOID fillTxDescTxByteCountWithCR4(IN P_ADAPTER_T prAdapter, IN P_MSDU_INFO_T prMsduInfo,
				  P_HW_MAC_TX_DESC_T prTxDesc)
{
	struct mt66xx_chip_info *prChipInfo;
	UINT_32 u4TxByteCount = NIC_TX_DESC_LONG_FORMAT_LENGTH;

	ASSERT(prAdapter);
	ASSERT(prMsduInfo);
	ASSERT(prTxDesc);

	prChipInfo = prAdapter->chip_info;
	u4TxByteCount += prMsduInfo->u2FrameLength;

	if (prMsduInfo->ucPacketType == TX_PACKET_TYPE_DATA)
		u4TxByteCount += prChipInfo->txd_append_size;

	/* Calculate Tx byte count */
	HAL_MAC_TX_DESC_SET_TX_BYTE_COUNT(prTxDesc, u4TxByteCount);
}

#if defined(_HIF_PCIE)
VOID asicPcieDmaShdlInit(IN P_ADAPTER_T prAdapter)
{
	UINT_32 u4BaseAddr, u4MacVal;
	struct mt66xx_chip_info *prChipInfo;

	ASSERT(prAdapter);

	prChipInfo = prAdapter->chip_info;
	u4BaseAddr = prChipInfo->u4HifDmaShdlBaseAddr;

	HAL_MCR_RD(prAdapter, CONN_HIF_DMASHDL_PACKET_MAX_SIZE(u4BaseAddr), &u4MacVal);
	u4MacVal &= ~(PLE_PKT_MAX_SIZE_MASK | PSE_PKT_MAX_SIZE_MASK);
	u4MacVal |= PLE_PKT_MAX_SIZE_NUM(0x1);
	u4MacVal |= PSE_PKT_MAX_SIZE_NUM(0x8);
	HAL_MCR_WR(prAdapter, CONN_HIF_DMASHDL_PACKET_MAX_SIZE(u4BaseAddr), u4MacVal);

	u4MacVal = (CONN_HIF_DMASHDL_TOP_REFILL_CONTROL_GROUP1_REFILL_DISABLE_MASK |
			CONN_HIF_DMASHDL_TOP_REFILL_CONTROL_GROUP2_REFILL_DISABLE_MASK |
			CONN_HIF_DMASHDL_TOP_REFILL_CONTROL_GROUP3_REFILL_DISABLE_MASK |
			CONN_HIF_DMASHDL_TOP_REFILL_CONTROL_GROUP4_REFILL_DISABLE_MASK |
			CONN_HIF_DMASHDL_TOP_REFILL_CONTROL_GROUP5_REFILL_DISABLE_MASK |
			CONN_HIF_DMASHDL_TOP_REFILL_CONTROL_GROUP6_REFILL_DISABLE_MASK |
			CONN_HIF_DMASHDL_TOP_REFILL_CONTROL_GROUP7_REFILL_DISABLE_MASK |
			CONN_HIF_DMASHDL_TOP_REFILL_CONTROL_GROUP8_REFILL_DISABLE_MASK |
			CONN_HIF_DMASHDL_TOP_REFILL_CONTROL_GROUP9_REFILL_DISABLE_MASK |
			CONN_HIF_DMASHDL_TOP_REFILL_CONTROL_GROUP10_REFILL_DISABLE_MASK |
			CONN_HIF_DMASHDL_TOP_REFILL_CONTROL_GROUP11_REFILL_DISABLE_MASK |
			CONN_HIF_DMASHDL_TOP_REFILL_CONTROL_GROUP12_REFILL_DISABLE_MASK |
			CONN_HIF_DMASHDL_TOP_REFILL_CONTROL_GROUP13_REFILL_DISABLE_MASK |
			CONN_HIF_DMASHDL_TOP_REFILL_CONTROL_GROUP14_REFILL_DISABLE_MASK |
			CONN_HIF_DMASHDL_TOP_REFILL_CONTROL_GROUP15_REFILL_DISABLE_MASK);
	HAL_MCR_WR(prAdapter, CONN_HIF_DMASHDL_REFILL_CONTROL(u4BaseAddr), u4MacVal);

	u4MacVal = DMASHDL_MIN_QUOTA_NUM(0x3);
	u4MacVal |= DMASHDL_MAX_QUOTA_NUM(0xfff);
	HAL_MCR_WR(prAdapter, CONN_HIF_DMASHDL_GROUP0_CTRL(u4BaseAddr), u4MacVal);

	u4MacVal = 0;
	HAL_MCR_WR(prAdapter, CONN_HIF_DMASHDL_GROUP1_CTRL(u4BaseAddr), u4MacVal);
	HAL_MCR_WR(prAdapter, CONN_HIF_DMASHDL_GROUP2_CTRL(u4BaseAddr), u4MacVal);
	HAL_MCR_WR(prAdapter, CONN_HIF_DMASHDL_GROUP3_CTRL(u4BaseAddr), u4MacVal);
	HAL_MCR_WR(prAdapter, CONN_HIF_DMASHDL_GROUP4_CTRL(u4BaseAddr), u4MacVal);
	HAL_MCR_WR(prAdapter, CONN_HIF_DMASHDL_GROUP5_CTRL(u4BaseAddr), u4MacVal);
	HAL_MCR_WR(prAdapter, CONN_HIF_DMASHDL_GROUP6_CTRL(u4BaseAddr), u4MacVal);
	HAL_MCR_WR(prAdapter, CONN_HIF_DMASHDL_GROUP7_CTRL(u4BaseAddr), u4MacVal);
	HAL_MCR_WR(prAdapter, CONN_HIF_DMASHDL_GROUP8_CTRL(u4BaseAddr), u4MacVal);
	HAL_MCR_WR(prAdapter, CONN_HIF_DMASHDL_GROUP9_CTRL(u4BaseAddr), u4MacVal);
	HAL_MCR_WR(prAdapter, CONN_HIF_DMASHDL_GROUP10_CTRL(u4BaseAddr), u4MacVal);
	HAL_MCR_WR(prAdapter, CONN_HIF_DMASHDL_GROUP11_CTRL(u4BaseAddr), u4MacVal);
	HAL_MCR_WR(prAdapter, CONN_HIF_DMASHDL_GROUP12_CTRL(u4BaseAddr), u4MacVal);
	HAL_MCR_WR(prAdapter, CONN_HIF_DMASHDL_GROUP13_CTRL(u4BaseAddr), u4MacVal);
	HAL_MCR_WR(prAdapter, CONN_HIF_DMASHDL_GROUP14_CTRL(u4BaseAddr), u4MacVal);
}

VOID asicPdmaLoopBackConfig(P_GLUE_INFO_T prGlueInfo, BOOLEAN fgEnable)
{
	WPDMA_GLO_CFG_STRUCT GloCfg;
	UINT_32 word = 1;

	kalDevRegRead(prGlueInfo, WPDMA_GLO_CFG, &GloCfg.word);

	GloCfg.field_conn.bypass_dmashdl_txring3 = 1;
	GloCfg.field_conn.pdma_addr_ext_en = 0;
	GloCfg.field_conn.omit_rx_info = 1;
	GloCfg.field_conn.omit_tx_info = 1;
	GloCfg.field_conn.multi_dma_en = 0;
	GloCfg.field_conn.pdma_addr_ext_en = 0;
	GloCfg.field_conn.tx_dma_en = 1;
	GloCfg.field_conn.rx_dma_en = 1;
	GloCfg.field_conn.multi_dma_en = 0;

	kalDevRegWrite(prGlueInfo, WPDMA_FIFO_TEST_MOD, word);
	kalDevRegWrite(prGlueInfo, WPDMA_GLO_CFG, GloCfg.word);
}

VOID asicPdmaConfig(P_GLUE_INFO_T prGlueInfo, BOOLEAN fgEnable)
{
	P_BUS_INFO prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;
	WPDMA_GLO_CFG_STRUCT GloCfg;
	WPMDA_INT_MASK IntMask;

	kalDevRegRead(prGlueInfo, WPDMA_GLO_CFG, &GloCfg.word);
	kalDevRegRead(prGlueInfo, WPDMA_INT_MSK, &IntMask.word);

	if (fgEnable == TRUE) {
		GloCfg.field_conn.tx_dma_en = 1;
		GloCfg.field_conn.rx_dma_en = 1;
		GloCfg.field_conn.pdma_bt_size = 3;
		GloCfg.field_conn.pdma_addr_ext_en = (prBusInfo->u4DmaMask > 32) ? 1 : 0;
		GloCfg.field_conn.tx_wb_ddone = 1;
		GloCfg.field_conn.multi_dma_en = 3;
		GloCfg.field_conn.fifo_little_endian = 1;
		GloCfg.field_conn.clk_gate_dis = 1;

		IntMask.field.rx_done_0 = 1;
		IntMask.field.rx_done_1 = 1;
		IntMask.field.tx_done = BIT(prBusInfo->tx_ring_fwdl_idx) |
			BIT(prBusInfo->tx_ring_cmd_idx) | BIT(prBusInfo->tx_ring_data_idx);
		IntMask.field.tx_dly_int = 0;
		IntMask.field_conn.mcu2host_sw_int_ena = 1;
	} else {
		GloCfg.field_conn.tx_dma_en = 0;
		GloCfg.field_conn.rx_dma_en = 0;

		IntMask.field_conn.rx_done_0 = 0;
		IntMask.field_conn.rx_done_1 = 0;
		IntMask.field_conn.tx_done = 0;
		IntMask.field_conn.tx_dly_int = 0;
		IntMask.field_conn.mcu2host_sw_int_ena = 0;
	}

	kalDevRegWrite(prGlueInfo, WPDMA_INT_MSK, IntMask.word);
	kalDevRegWrite(prGlueInfo, WPDMA_GLO_CFG, GloCfg.word);
	kalDevRegWrite(prGlueInfo, MCU2HOST_SW_INT_ENA, ERROR_DETECT_MASK);
}

VOID asicEnableInterrupt(IN P_ADAPTER_T prAdapter)
{
	P_BUS_INFO prBusInfo = prAdapter->chip_info->bus_info;
	WPMDA_INT_MASK IntMask;

	prAdapter->fgIsIntEnable = TRUE;

	HAL_MCR_RD(prAdapter, WPDMA_INT_MSK, &IntMask.word);

	IntMask.field_conn.rx_done_0 = 1;
	IntMask.field_conn.rx_done_1 = 1;
	IntMask.field_conn.tx_done = BIT(prBusInfo->tx_ring_fwdl_idx) |
		BIT(prBusInfo->tx_ring_cmd_idx) | BIT(prBusInfo->tx_ring_data_idx);
	IntMask.field_conn.tx_coherent = 0;
	IntMask.field_conn.rx_coherent = 0;
	IntMask.field_conn.tx_dly_int = 0;
	IntMask.field_conn.rx_dly_int = 0;
	IntMask.field_conn.mcu2host_sw_int_ena = 1;

	HAL_MCR_WR(prAdapter, WPDMA_INT_MSK, IntMask.word);

	DBGLOG(HAL, TRACE, "%s [0x%08x]\n", __func__, IntMask.word);
}

VOID asicLowPowerOwnRead(IN P_ADAPTER_T prAdapter, OUT PBOOLEAN pfgResult)
{
	UINT_32 u4RegValue;

	HAL_MCR_RD(prAdapter, CONN_HIF_ON_LPCTL, &u4RegValue);
	*pfgResult = (u4RegValue & PCIE_LPCR_HOST_SET_OWN) == 0 ? TRUE : FALSE;
}

VOID asicLowPowerOwnSet(IN P_ADAPTER_T prAdapter, OUT PBOOLEAN pfgResult)
{
	UINT_32 u4RegValue;

	HAL_MCR_WR(prAdapter, CONN_HIF_ON_LPCTL, PCIE_LPCR_HOST_SET_OWN);
	HAL_MCR_RD(prAdapter, CONN_HIF_ON_LPCTL, &u4RegValue);
	*pfgResult = (u4RegValue & PCIE_LPCR_HOST_SET_OWN) == 1;
}

VOID asicLowPowerOwnClear(IN P_ADAPTER_T prAdapter, OUT PBOOLEAN pfgResult)
{
	UINT_32 u4RegValue;

	HAL_MCR_WR(prAdapter, CONN_HIF_ON_LPCTL, PCIE_LPCR_HOST_CLR_OWN);
	HAL_MCR_RD(prAdapter, CONN_HIF_ON_LPCTL, &u4RegValue);
	*pfgResult = (u4RegValue & PCIE_LPCR_HOST_SET_OWN) == 0;
}
#endif /* _HIF_PCIE */

#if defined(_HIF_USB)
/* DMS Scheduler Init */
VOID asicUsbDmaShdlGroupInit(IN P_ADAPTER_T prAdapter, UINT_32 u4RefillGroup)
{
	UINT_32 u4BaseAddr, u4MacVal = 0;
	struct mt66xx_chip_info *prChipInfo;
	UINT_32 u4CfgVal = 0;

	ASSERT(prAdapter);

	prChipInfo = prAdapter->chip_info;
	u4BaseAddr = prChipInfo->u4HifDmaShdlBaseAddr;

	HAL_MCR_RD(prAdapter, CONN_HIF_DMASHDL_PACKET_MAX_SIZE(u4BaseAddr), &u4MacVal);
	u4MacVal &= ~(PLE_PKT_MAX_SIZE_MASK | PSE_PKT_MAX_SIZE_MASK);
	u4MacVal |= PLE_PKT_MAX_SIZE_NUM(0x1);
	u4MacVal |= PSE_PKT_MAX_SIZE_NUM(0x8);
	HAL_MCR_WR(prAdapter, CONN_HIF_DMASHDL_PACKET_MAX_SIZE(u4BaseAddr), u4MacVal);

	u4RefillGroup |= (CONN_HIF_DMASHDL_TOP_REFILL_CONTROL_GROUP2_REFILL_PRIORITY_MASK
		| CONN_HIF_DMASHDL_TOP_REFILL_CONTROL_GROUP3_REFILL_PRIORITY_MASK);
	HAL_MCR_WR(prAdapter, CONN_HIF_DMASHDL_REFILL_CONTROL(u4BaseAddr), u4RefillGroup);

#if CFG_SUPPORT_CFG_FILE
	u4CfgVal = wlanCfgGetUint32(prAdapter, "DmaShdlGroup1MinQuota", USB_DMA_SHDL_GROUP_DEF_MIN_QUOTA);
	u4MacVal = DMASHDL_MIN_QUOTA_NUM(u4CfgVal);
	u4CfgVal = wlanCfgGetUint32(prAdapter, "DmaShdlGroup1MaxQuota", USB_DMA_SHDL_GROUP_DEF_MAX_QUOTA);
	u4MacVal |= DMASHDL_MAX_QUOTA_NUM(u4CfgVal);
#else /* CFG_SUPPORT_CFG_FILE */
	u4MacVal = DMASHDL_MIN_QUOTA_NUM(USB_DMA_SHDL_GROUP_DEF_MIN_QUOTA);
	u4MacVal |= DMASHDL_MAX_QUOTA_NUM(USB_DMA_SHDL_GROUP_DEF_MAX_QUOTA);
#endif /* !CFG_SUPPORT_CFG_FILE */
	HAL_MCR_WR(prAdapter, CONN_HIF_DMASHDL_GROUP1_CTRL(u4BaseAddr), u4MacVal);

#if CFG_SUPPORT_CFG_FILE
	u4CfgVal = wlanCfgGetUint32(prAdapter, "DmaShdlGroup0MinQuota", USB_DMA_SHDL_GROUP_DEF_MIN_QUOTA);
	u4MacVal = DMASHDL_MIN_QUOTA_NUM(u4CfgVal);
	u4CfgVal = wlanCfgGetUint32(prAdapter, "DmaShdlGroup0MaxQuota", USB_DMA_SHDL_GROUP_DEF_MAX_QUOTA);
	u4MacVal |= DMASHDL_MAX_QUOTA_NUM(u4CfgVal);
#else /* CFG_SUPPORT_CFG_FILE */
	u4MacVal = DMASHDL_MIN_QUOTA_NUM(USB_DMA_SHDL_GROUP_DEF_MIN_QUOTA);
	u4MacVal |= DMASHDL_MAX_QUOTA_NUM(USB_DMA_SHDL_GROUP_DEF_MAX_QUOTA);
#endif /* !CFG_SUPPORT_CFG_FILE */
	HAL_MCR_WR(prAdapter, CONN_HIF_DMASHDL_GROUP0_CTRL(u4BaseAddr), u4MacVal);

#if CFG_SUPPORT_CFG_FILE
	u4CfgVal = wlanCfgGetUint32(prAdapter, "DmaShdlGroup2MinQuota", USB_DMA_SHDL_GROUP_DEF_MIN_QUOTA);
	u4MacVal = DMASHDL_MIN_QUOTA_NUM(u4CfgVal);
	u4CfgVal = wlanCfgGetUint32(prAdapter, "DmaShdlGroup2MaxQuota", USB_DMA_SHDL_GROUP_DEF_MAX_QUOTA);
	u4MacVal |= DMASHDL_MAX_QUOTA_NUM(u4CfgVal);
#else /* CFG_SUPPORT_CFG_FILE */
	u4MacVal = DMASHDL_MIN_QUOTA_NUM(USB_DMA_SHDL_GROUP_DEF_MIN_QUOTA);
	u4MacVal |= DMASHDL_MAX_QUOTA_NUM(USB_DMA_SHDL_GROUP_DEF_MAX_QUOTA);
#endif /* !CFG_SUPPORT_CFG_FILE */
	HAL_MCR_WR(prAdapter, CONN_HIF_DMASHDL_GROUP2_CTRL(u4BaseAddr), u4MacVal);

#if CFG_SUPPORT_CFG_FILE
	u4CfgVal = wlanCfgGetUint32(prAdapter, "DmaShdlGroup3MinQuota", USB_DMA_SHDL_GROUP_DEF_MIN_QUOTA);
	u4MacVal = DMASHDL_MIN_QUOTA_NUM(u4CfgVal);
	u4CfgVal = wlanCfgGetUint32(prAdapter, "DmaShdlGroup3MaxQuota", USB_DMA_SHDL_GROUP_DEF_MAX_QUOTA);
	u4MacVal |= DMASHDL_MAX_QUOTA_NUM(u4CfgVal);
#else /* CFG_SUPPORT_CFG_FILE */
	u4MacVal = DMASHDL_MIN_QUOTA_NUM(USB_DMA_SHDL_GROUP_DEF_MIN_QUOTA);
	u4MacVal |= DMASHDL_MAX_QUOTA_NUM(USB_DMA_SHDL_GROUP_DEF_MAX_QUOTA);
#endif /* !CFG_SUPPORT_CFG_FILE */
	HAL_MCR_WR(prAdapter, CONN_HIF_DMASHDL_GROUP3_CTRL(u4BaseAddr), u4MacVal);

#if CFG_SUPPORT_CFG_FILE
	u4CfgVal = wlanCfgGetUint32(prAdapter, "DmaShdlGroup4MinQuota", USB_DMA_SHDL_GROUP_DEF_MIN_QUOTA);
	u4MacVal = DMASHDL_MIN_QUOTA_NUM(u4CfgVal);
	u4CfgVal = wlanCfgGetUint32(prAdapter, "DmaShdlGroup4MaxQuota", USB_DMA_SHDL_GROUP_DEF_MAX_QUOTA);
	u4MacVal |= DMASHDL_MAX_QUOTA_NUM(u4CfgVal);
#else /* CFG_SUPPORT_CFG_FILE */
	u4MacVal = DMASHDL_MIN_QUOTA_NUM(USB_DMA_SHDL_GROUP_DEF_MIN_QUOTA);
	u4MacVal |= DMASHDL_MAX_QUOTA_NUM(USB_DMA_SHDL_GROUP_DEF_MAX_QUOTA);
#endif /* !CFG_SUPPORT_CFG_FILE */
	HAL_MCR_WR(prAdapter, CONN_HIF_DMASHDL_GROUP4_CTRL(u4BaseAddr), u4MacVal);

	u4MacVal = ((arAcQIdx2GroupId[MAC_TXQ_AC0_INDEX] << CONN_HIF_DMASHDL_TOP_QUEUE_MAPPING0_QUEUE0_MAPPING) |
		(arAcQIdx2GroupId[MAC_TXQ_AC1_INDEX] << CONN_HIF_DMASHDL_TOP_QUEUE_MAPPING0_QUEUE1_MAPPING) |
		(arAcQIdx2GroupId[MAC_TXQ_AC2_INDEX] << CONN_HIF_DMASHDL_TOP_QUEUE_MAPPING0_QUEUE2_MAPPING) |
		(arAcQIdx2GroupId[MAC_TXQ_AC3_INDEX] << CONN_HIF_DMASHDL_TOP_QUEUE_MAPPING0_QUEUE3_MAPPING) |
		(arAcQIdx2GroupId[MAC_TXQ_AC10_INDEX] << CONN_HIF_DMASHDL_TOP_QUEUE_MAPPING0_QUEUE4_MAPPING) |
		(arAcQIdx2GroupId[MAC_TXQ_AC11_INDEX] << CONN_HIF_DMASHDL_TOP_QUEUE_MAPPING0_QUEUE5_MAPPING) |
		(arAcQIdx2GroupId[MAC_TXQ_AC12_INDEX] << CONN_HIF_DMASHDL_TOP_QUEUE_MAPPING0_QUEUE6_MAPPING) |
		(arAcQIdx2GroupId[MAC_TXQ_AC13_INDEX] << CONN_HIF_DMASHDL_TOP_QUEUE_MAPPING0_QUEUE7_MAPPING));
	HAL_MCR_WR(prAdapter, CONN_HIF_DMASHDL_Q_MAP0(u4BaseAddr), u4MacVal);

	u4MacVal = ((arAcQIdx2GroupId[MAC_TXQ_AC20_INDEX] << CONN_HIF_DMASHDL_TOP_QUEUE_MAPPING1_QUEUE8_MAPPING) |
		(arAcQIdx2GroupId[MAC_TXQ_AC21_INDEX] << CONN_HIF_DMASHDL_TOP_QUEUE_MAPPING1_QUEUE9_MAPPING) |
		(arAcQIdx2GroupId[MAC_TXQ_AC22_INDEX] << CONN_HIF_DMASHDL_TOP_QUEUE_MAPPING1_QUEUE10_MAPPING) |
		(arAcQIdx2GroupId[MAC_TXQ_AC23_INDEX] << CONN_HIF_DMASHDL_TOP_QUEUE_MAPPING1_QUEUE11_MAPPING) |
		(arAcQIdx2GroupId[MAC_TXQ_AC30_INDEX] << CONN_HIF_DMASHDL_TOP_QUEUE_MAPPING1_QUEUE12_MAPPING) |
		(arAcQIdx2GroupId[MAC_TXQ_AC31_INDEX] << CONN_HIF_DMASHDL_TOP_QUEUE_MAPPING1_QUEUE13_MAPPING) |
		(arAcQIdx2GroupId[MAC_TXQ_AC32_INDEX] << CONN_HIF_DMASHDL_TOP_QUEUE_MAPPING1_QUEUE14_MAPPING) |
		(arAcQIdx2GroupId[MAC_TXQ_AC33_INDEX] << CONN_HIF_DMASHDL_TOP_QUEUE_MAPPING1_QUEUE15_MAPPING));
	HAL_MCR_WR(prAdapter, CONN_HIF_DMASHDL_Q_MAP1(u4BaseAddr), u4MacVal);

	u4MacVal = ((arAcQIdx2GroupId[MAC_TXQ_ALTX_0_INDEX] << CONN_HIF_DMASHDL_TOP_QUEUE_MAPPING2_QUEUE16_MAPPING) |
		(arAcQIdx2GroupId[MAC_TXQ_BMC_0_INDEX] << CONN_HIF_DMASHDL_TOP_QUEUE_MAPPING2_QUEUE17_MAPPING)|
		(arAcQIdx2GroupId[MAC_TXQ_BCN_0_INDEX] << CONN_HIF_DMASHDL_TOP_QUEUE_MAPPING2_QUEUE18_MAPPING) |
		(arAcQIdx2GroupId[MAC_TXQ_PSMP_0_INDEX] << CONN_HIF_DMASHDL_TOP_QUEUE_MAPPING2_QUEUE19_MAPPING));
	HAL_MCR_WR(prAdapter, CONN_HIF_DMASHDL_Q_MAP2(u4BaseAddr), u4MacVal);
}

VOID asicUsbDmaShdlInit(IN P_ADAPTER_T prAdapter)
{
	UINT_32 u4BaseAddr, u4MacVal;
	struct mt66xx_chip_info *prChipInfo;

	ASSERT(prAdapter);

	prChipInfo = prAdapter->chip_info;
	u4BaseAddr = prChipInfo->u4HifDmaShdlBaseAddr;

	/*
	* USB Endpoint OUT/DMA Scheduler Group Mapping (HW Define)
	* EP#4 / Group0  (DATA)
	* EP#5 / Group1  (DATA)
	* EP#6 / Group2  (DATA)
	* EP#7 / Group3  (DATA)
	* EP#9 / Group4  (DATA)
	* EP#8 / Group15 (CMD)
	*/
	arAcQIdx2GroupId[MAC_TXQ_AC3_INDEX] = GROUP3_INDEX;
	arAcQIdx2GroupId[MAC_TXQ_AC13_INDEX] = GROUP3_INDEX;
	arAcQIdx2GroupId[MAC_TXQ_AC23_INDEX] = GROUP3_INDEX;
	arAcQIdx2GroupId[MAC_TXQ_AC33_INDEX] = GROUP3_INDEX;
	arAcQIdx2GroupId[MAC_TXQ_ALTX_0_INDEX] = GROUP4_INDEX;
	arAcQIdx2GroupId[MAC_TXQ_BMC_0_INDEX] = GROUP4_INDEX;
	arAcQIdx2GroupId[MAC_TXQ_BCN_0_INDEX] = GROUP4_INDEX;
	arAcQIdx2GroupId[MAC_TXQ_PSMP_0_INDEX] = GROUP4_INDEX;

	/*
	* Enable refill control group 0, 1, 2, 3, 4.
	* Keep all group low refill priority to prevent low group starvation if we have high group.
	*/
	u4MacVal = (CONN_HIF_DMASHDL_TOP_REFILL_CONTROL_GROUP5_REFILL_DISABLE_MASK |
				CONN_HIF_DMASHDL_TOP_REFILL_CONTROL_GROUP6_REFILL_DISABLE_MASK |
				CONN_HIF_DMASHDL_TOP_REFILL_CONTROL_GROUP7_REFILL_DISABLE_MASK |
				CONN_HIF_DMASHDL_TOP_REFILL_CONTROL_GROUP8_REFILL_DISABLE_MASK |
				CONN_HIF_DMASHDL_TOP_REFILL_CONTROL_GROUP9_REFILL_DISABLE_MASK |
				CONN_HIF_DMASHDL_TOP_REFILL_CONTROL_GROUP10_REFILL_DISABLE_MASK |
				CONN_HIF_DMASHDL_TOP_REFILL_CONTROL_GROUP11_REFILL_DISABLE_MASK |
				CONN_HIF_DMASHDL_TOP_REFILL_CONTROL_GROUP12_REFILL_DISABLE_MASK |
				CONN_HIF_DMASHDL_TOP_REFILL_CONTROL_GROUP13_REFILL_DISABLE_MASK |
				CONN_HIF_DMASHDL_TOP_REFILL_CONTROL_GROUP14_REFILL_DISABLE_MASK |
				CONN_HIF_DMASHDL_TOP_REFILL_CONTROL_GROUP15_REFILL_DISABLE_MASK);

	asicUsbDmaShdlGroupInit(prAdapter, u4MacVal);

	/*
	* HIF Scheduler Setting
	* Group15(CMD) is highest priority.
	*/
	HAL_MCR_WR(prAdapter, CONN_HIF_DMASHDL_SHDL_SET0(u4BaseAddr), 0x6501234f);
	HAL_MCR_WR(prAdapter, CONN_HIF_DMASHDL_SHDL_SET1(u4BaseAddr), 0xedcba987);

	HAL_MCR_WR(prAdapter, CONN_HIF_DMASHDL_OPTIONAL_CONTROL(u4BaseAddr), 0x7004801c);
}

BOOLEAN asicUsbSuspend(IN P_ADAPTER_T prAdapter, IN P_GLUE_INFO_T prGlueInfo)
{
	UINT_32 u4Value;
	UINT_32 count = 0;
	INT_32 ret = 0;
	P_BUS_INFO prBusInfo;

	DBGLOG(HAL, INFO, "%s ---->\n", __func__);
	prBusInfo = prAdapter->chip_info->bus_info;

	/* Disable PDMA TX */
	HAL_MCR_RD(prAdapter, PDMA_IF_MISC, &u4Value);
	u4Value &= ~PDMA_IF_MISC_TX_ENABLE_MASK;
	HAL_MCR_WR(prAdapter, PDMA_IF_MISC, u4Value);

	/* Set PDMA Debug flag */
	u4Value = 0x00000116;
	HAL_MCR_WR(prAdapter, PDMA_DEBUG_EN, u4Value);
	/* Polling PDMA_dmashdl_request done  */
	while (count < PDMA_TX_IDLE_WAIT_COUNT) {
		HAL_MCR_RD(prAdapter, PDMA_DEBUG_STATUS, &u4Value);
		DBGLOG(HAL, INFO, "%s: 0x%08x = 0x%08x\n", __func__, PDMA_DEBUG_STATUS, u4Value);
		if (!(u4Value & PDMA_DEBUG_DMASHDL_REQUEST_DONE_MASK) && (count >= 3))
			break;
		mdelay(1);
		count++;
	}

	if (count >= PDMA_TX_IDLE_WAIT_COUNT) {
		DBGLOG(HAL, ERROR, "%s:: 2.2 suspend fail, enable PDMA TX again.\n", __func__);
		/* Enable PDMA TX again */
		HAL_MCR_RD(prAdapter, PDMA_IF_MISC, &u4Value);
		u4Value |= PDMA_IF_MISC_TX_ENABLE_MASK;
		HAL_MCR_WR(prAdapter, PDMA_IF_MISC, u4Value);
		return FALSE;
	}

	u4Value = 0x00000101;
	HAL_MCR_WR(prAdapter, PDMA_DEBUG_EN, u4Value);
	count = 0;
	while (count < PDMA_TX_IDLE_WAIT_COUNT) {
		HAL_MCR_RD(prAdapter, PDMA_DEBUG_STATUS, &u4Value);
		DBGLOG(HAL, INFO, "%s:: 0x%08x = 0x%08x\n",
			__func__, PDMA_DEBUG_STATUS, u4Value);
		if ((u4Value == PDMA_DEBUG_TX_STATUS_MASK) && (count >= 3)) {
			DBGLOG(HAL, ERROR, "%s:: PDMA Tx idle~\n", __func__);
			break;
		}
		DBGLOG(HAL, ERROR, "%s:: PDMA Tx busy.....\n", __func__);
		count++;
	}

	if (count >= PDMA_TX_IDLE_WAIT_COUNT) {
		DBGLOG(HAL, ERROR, "%s:: 2.4 suspend fail, enable PDMA TX again.\n", __func__);
		/* Enable PDMA TX again */
		HAL_MCR_RD(prAdapter, PDMA_IF_MISC, &u4Value);
		u4Value |= PDMA_IF_MISC_TX_ENABLE_MASK;
		HAL_MCR_WR(prAdapter, PDMA_IF_MISC, u4Value);
		return FALSE;
	}

	prGlueInfo->rHifInfo.state = USB_STATE_SUSPEND;
	halDisableInterrupt(prGlueInfo->prAdapter);
	halTxCancelAllSending(prGlueInfo->prAdapter);

	ret = usb_control_msg(prGlueInfo->rHifInfo.udev,
	usb_sndctrlpipe(prGlueInfo->rHifInfo.udev, 0), VND_REQ_FEATURE_SET,
		DEVICE_VENDOR_REQUEST_OUT, FEATURE_SET_WVALUE_SUSPEND, 0, NULL, 0,
		VENDOR_TIMEOUT_MS);
	if (ret) {
		DBGLOG(HAL, ERROR, "%s:: VendorRequest FeatureSetResume ERROR: %x, enable PDMA TX again.\n",
			__func__, (unsigned int)ret);
		/* Enable PDMA TX again */
		HAL_MCR_RD(prAdapter, PDMA_IF_MISC, &u4Value);
		u4Value |= PDMA_IF_MISC_TX_ENABLE_MASK;
		HAL_MCR_WR(prAdapter, PDMA_IF_MISC, u4Value);
		DBGLOG(HAL, INFO, "%s <----\n", __func__);
		return FALSE;
	}
	DBGLOG(HAL, INFO, "%s <----\n", __func__);
	return TRUE;
}

UINT_8 asicUsbEventEpDetected(IN P_ADAPTER_T prAdapter)
{
	P_GL_HIF_INFO_T prHifInfo = NULL;
	P_GLUE_INFO_T prGlueInfo = NULL;
	INT_32 ret = 0;
	UINT_8 ucRetryCount = 0;
	BOOLEAN ucEp5Disable = FALSE;

	ASSERT(FALSE == 0);
	prGlueInfo = prAdapter->prGlueInfo;
	prHifInfo = &prGlueInfo->rHifInfo;

	if (prHifInfo->fgEventEpDetected == FALSE) {
		prHifInfo->fgEventEpDetected = TRUE;
		do {
			ret = mtk_usb_vendor_request(prGlueInfo, 0, DEVICE_VENDOR_REQUEST_IN, VND_REQ_EP5_IN_INFO,
					       0, 0, &ucEp5Disable, sizeof(ucEp5Disable));
			if (ret || ucRetryCount)
				DBGLOG(HAL, ERROR, "usb_control_msg() status: %x retry: %u\n",
					(unsigned int)ret, ucRetryCount);
			ucRetryCount++;
			if (ucRetryCount > USB_ACCESS_RETRY_LIMIT)
				break;
		} while (ret);

		if (ret) {
			kalSendAeeWarning(HIF_USB_ERR_TITLE_STR,
					  HIF_USB_ERR_DESC_STR "USB() reports error: %x retry: %u", ret, ucRetryCount);
			DBGLOG(HAL, ERROR, "usb_readl() reports error: %x retry: %u\n", ret, ucRetryCount);
		} else {
			DBGLOG(HAL, INFO, "%s: Get ucEp5Disable = %d\n", __func__, ucEp5Disable);
			if (ucEp5Disable)
				prHifInfo->eEventEpType = EVENT_EP_TYPE_DATA_EP;
		}
	}
	if (prHifInfo->eEventEpType == EVENT_EP_TYPE_DATA_EP)
		return USB_DATA_EP_IN;
	else
		return USB_EVENT_EP_IN;
}

VOID asicUdmaTxTimeoutEnable(IN P_ADAPTER_T prAdapter)
{
	P_BUS_INFO prBusInfo;
	UINT_32 u4Value;

	prBusInfo = prAdapter->chip_info->bus_info;
	HAL_MCR_RD(prAdapter, prBusInfo->u4UdmaWlCfg_1_Addr, &u4Value);
	u4Value &= ~UDMA_WLCFG_1_TX_TIMEOUT_LIMIT_MASK;
	u4Value |= UDMA_WLCFG_1_TX_TIMEOUT_LIMIT(prBusInfo->u4UdmaTxTimeout);
	HAL_MCR_WR(prAdapter, prBusInfo->u4UdmaWlCfg_1_Addr, u4Value);

	HAL_MCR_RD(prAdapter, prBusInfo->u4UdmaWlCfg_0_Addr, &u4Value);
	u4Value |= UDMA_WLCFG_0_TX_TIMEOUT_EN_MASK;
	HAL_MCR_WR(prAdapter, prBusInfo->u4UdmaWlCfg_0_Addr, u4Value);
}

VOID asicUdmaRxFlush(IN P_ADAPTER_T prAdapter, IN BOOLEAN bEnable)
{
	P_BUS_INFO prBusInfo;
	UINT_32 u4Value;

	prBusInfo = prAdapter->chip_info->bus_info;

	HAL_MCR_RD(prAdapter, prBusInfo->u4UdmaWlCfg_0_Addr, &u4Value);
	if (bEnable)
		u4Value |= UDMA_WLCFG_0_RX_FLUSH_MASK;
	else
		u4Value &= ~UDMA_WLCFG_0_RX_FLUSH_MASK;
	HAL_MCR_WR(prAdapter, prBusInfo->u4UdmaWlCfg_0_Addr, u4Value);
}

VOID asicPdmaHifReset(IN P_ADAPTER_T prAdapter, IN BOOLEAN bRelease)
{
	UINT_32 u4Value;

	HAL_MCR_RD(prAdapter, PDMA_HIF_RESET, &u4Value);
	if (bRelease)
		u4Value |= DPMA_HIF_LOGIC_RESET_MASK;
	else
		u4Value &= ~DPMA_HIF_LOGIC_RESET_MASK;
	HAL_MCR_WR(prAdapter, PDMA_HIF_RESET, u4Value);
}

VOID fillUsbHifTxDesc(IN PUINT_8 * pDest, IN PUINT_16 pInfoBufLen)
{
	/*USB TX Descriptor (4 bytes)*/
	/*BIT[15:0] - TX Bytes Count (Not including USB TX Descriptor and 4-bytes zero padding.*/
	kalMemZero((PVOID)*pDest, sizeof(UINT_32));
	kalMemCopy((PVOID)*pDest, (PVOID) pInfoBufLen, sizeof(UINT_16));
}
#endif /* _HIF_USB */
