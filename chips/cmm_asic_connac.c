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
	prChipInfo->u2ExtraTxByteCount = 0;

	switch (prGlueInfo->u4InfType) {
#if defined(_HIF_PCIE)
	case MT_DEV_INF_PCIE:
		prChipInfo->u2TxInitCmdPort = TX_RING_FWDL_IDX_3;
		prChipInfo->u2TxFwDlPort = TX_RING_FWDL_IDX_3;
		prChipInfo->ucPacketFormat = TXD_PKT_FORMAT_TXD;
		break;
#endif /* _HIF_PCIE */
#if defined(_HIF_USB)
	case MT_DEV_INF_USB:
		prChipInfo->u2HifTxdSize = USB_HIF_TXD_LEN;
		prChipInfo->fillHifTxDesc = fillUsbHifTxDesc;
		prChipInfo->u2TxInitCmdPort = USB_DATA_BULK_OUT_EP8;
		prChipInfo->u2TxFwDlPort = USB_DATA_BULK_OUT_EP4;
		prChipInfo->ucPacketFormat = TXD_PKT_FORMAT_TXD_PAYLOAD;
		prChipInfo->u2ExtraTxByteCount = EXTRA_TXD_SIZE_FOR_TX_BYTE_COUNT;
		break;
#endif /* _HIF_USB */
#if defined(_HIF_SDIO)
	case MT_DEV_INF_SDIO:
		prChipInfo->ucPacketFormat = TXD_PKT_FORMAT_TXD_PAYLOAD;
		prChipInfo->u2ExtraTxByteCount = EXTRA_TXD_SIZE_FOR_TX_BYTE_COUNT;
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

VOID fillTxDescAppendByHost(IN P_ADAPTER_T prAdapter, IN P_MSDU_INFO_T prMsduInfo, IN UINT_16 u4MsduId,
			    IN dma_addr_t rDmaAddr, OUT PUINT_8 pucBuffer)
{
	P_HW_MAC_TX_DESC_APPEND_T prHwTxDescAppend;

	prHwTxDescAppend = (P_HW_MAC_TX_DESC_APPEND_T) (pucBuffer + NIC_TX_DESC_LONG_FORMAT_LENGTH);
	prHwTxDescAppend->CONNAC_APPEND.au2MsduId[0] = u4MsduId | TXD_MSDU_ID_VLD;
	prHwTxDescAppend->CONNAC_APPEND.arPtrLen[0].u4Ptr0 = rDmaAddr;
	prHwTxDescAppend->CONNAC_APPEND.arPtrLen[0].u2Len0 = prMsduInfo->u2FrameLength | TXD_LEN_AL | TXD_LEN_ML;
}

VOID fillTxDescAppendByHostV2(IN P_ADAPTER_T prAdapter, IN P_MSDU_INFO_T prMsduInfo, IN UINT_16 u4MsduId,
			      IN dma_addr_t rDmaAddr, OUT PUINT_8 pucBuffer)
{
	P_HW_MAC_TX_DESC_APPEND_T prHwTxDescAppend;

	prHwTxDescAppend = (P_HW_MAC_TX_DESC_APPEND_T) (pucBuffer + NIC_TX_DESC_LONG_FORMAT_LENGTH);
	prHwTxDescAppend->CONNAC_APPEND.au2MsduId[0] = u4MsduId | TXD_MSDU_ID_VLD;
	prHwTxDescAppend->CONNAC_APPEND.arPtrLen[0].u4Ptr0 = rDmaAddr;
	prHwTxDescAppend->CONNAC_APPEND.arPtrLen[0].u2Len0 = (prMsduInfo->u2FrameLength & TXD_LEN_MASK_V2) |
		((rDmaAddr >> TXD_ADDR2_OFFSET) & TXD_ADDR2_MASK) | TXD_LEN_ML_V2;
}

VOID fillTxDescAppendByCR4(IN P_ADAPTER_T prAdapter, IN P_MSDU_INFO_T prMsduInfo, IN UINT_16 u4MsduId,
			   IN dma_addr_t rDmaAddr, OUT PUINT_8 pucBuffer)
{
	P_HW_MAC_TX_DESC_APPEND_T prHwTxDescAppend;

	prHwTxDescAppend = (P_HW_MAC_TX_DESC_APPEND_T) (pucBuffer + NIC_TX_DESC_LONG_FORMAT_LENGTH);
	prHwTxDescAppend->CR4_APPEND.u2MsduToken = u4MsduId;
	prHwTxDescAppend->CR4_APPEND.ucBufNum = 1;
	prHwTxDescAppend->CR4_APPEND.au4BufPtr[0] = rDmaAddr;
	prHwTxDescAppend->CR4_APPEND.au2BufLen[0] = prMsduInfo->u2FrameLength;
}

#if defined(_HIF_USB)
VOID fillUsbHifTxDesc(IN PUINT_8 * pDest, IN PUINT_16 pInfoBufLen)
{
	/*USB TX Descriptor (4 bytes)*/
	/*BIT[15:0] - TX Bytes Count (Not including USB TX Descriptor and 4-bytes zero padding.*/
	kalMemZero((PVOID)*pDest, sizeof(UINT_32));
	kalMemCopy((PVOID)*pDest, (PVOID) pInfoBufLen, sizeof(UINT_16));
}
#endif /* _HIF_USB */
