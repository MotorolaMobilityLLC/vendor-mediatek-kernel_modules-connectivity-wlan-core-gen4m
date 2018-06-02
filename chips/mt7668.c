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
/*! \file   mt7668.c
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

#include "mt7668.h"

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

ECO_INFO_T mt7668_eco_table[] = {
	/* HW version,  ROM version,    Factory version */
	{0x00, 0x00, 0xA},	/* E1 */
	{0x10, 0x01, 0xB},	/* E2 */
	{0x00, 0x00, 0x0}	/* End of table */
};

#if defined(_HIF_PCIE)
PCIE_CHIP_CR_MAPPING mt7668_bus2chip_cr_mapping[] = {
	/* chip addr, bus addr, range */
	{0x82060000, 0x00008000, 0x00000450}, /* WF_PLE */
	{0x82068000, 0x0000c000, 0x00000450}, /* WF_PSE */
	{0x8206c000, 0x0000e000, 0x00000300}, /* PP */
	{0x820d0000, 0x00020000, 0x00000200}, /* WF_AON */
	{0x820f0000, 0x00020200, 0x00000400}, /* WF_CFG */
	{0x820f0800, 0x00020600, 0x00000200}, /* WF_CFGOFF */
	{0x820f1000, 0x00020800, 0x00000200}, /* WF_TRB */
	{0x820f2000, 0x00020a00, 0x00000200}, /* WF_AGG */
	{0x820f3000, 0x00020c00, 0x00000400}, /* WF_ARB */
	{0x820f4000, 0x00021000, 0x00000200}, /* WF_TMAC */
	{0x820f5000, 0x00021200, 0x00000400}, /* WF_RMAC */
	{0x820f6000, 0x00021600, 0x00000200}, /* WF_SEC */
	{0x820f7000, 0x00021800, 0x00000200}, /* WF_DMA */

	{0x820f8000, 0x00022000, 0x00001000}, /* WF_PF */
	{0x820f9000, 0x00023000, 0x00000400}, /* WF_WTBLON */
	{0x820f9800, 0x00023400, 0x00000200}, /* WF_WTBLOFF */

	{0x820fa000, 0x00024000, 0x00000200}, /* WF_ETBF */
	{0x820fb000, 0x00024200, 0x00000400}, /* WF_LPON */
	{0x820fc000, 0x00024600, 0x00000200}, /* WF_INT */
	{0x820fd000, 0x00024800, 0x00000400}, /* WF_MIB */

	{0x820fe000, 0x00025000, 0x00002000}, /* WF_MU */

	{0x820e0000, 0x00030000, 0x00010000}, /* WF_WTBL */

	{0x80020000, 0x00000000, 0x00002000}, /* TOP_CFG */
	{0x80000000, 0x00002000, 0x00002000}, /* MCU_CFG */
	{0x50000000, 0x00004000, 0x00004000}, /* PDMA_CFG */
	{0xA0000000, 0x00008000, 0x00008000}, /* PSE_CFG */
	{0x82070000, 0x00010000, 0x00010000}, /* WF_PHY */

	{0x0, 0x0, 0x0}
};
#endif /* _HIF_PCIE */

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

VOID
mt7668ConstructFirmwarePrio(P_GLUE_INFO_T prGlueInfo, PPUINT_8 apucNameTable,
PPUINT_8 apucName, PUINT_8 pucNameIdx, UINT_8 ucMaxNameIdx)
{
	struct mt66xx_chip_info *prChipInfo = prGlueInfo->prAdapter->chip_info;
	UINT_32 chip_id = prChipInfo->chip_id;
	UINT_8 sub_idx = 0;

	for (sub_idx = 0; apucNameTable[sub_idx]; sub_idx++) {
		if (((*pucNameIdx) + 3) < ucMaxNameIdx) {
			/* Type 1. WIFI_RAM_CODE_MTxxxx.bin */
			snprintf(*(apucName + (*pucNameIdx)), CFG_FW_NAME_MAX_LEN, "%s%x.bin",
					apucNameTable[sub_idx], chip_id);
			(*pucNameIdx) += 1;

			/* Type 2. WIFI_RAM_CODE_MTxxxx */
			snprintf(*(apucName + (*pucNameIdx)), CFG_FW_NAME_MAX_LEN, "%s%x",
					apucNameTable[sub_idx], chip_id);
			(*pucNameIdx) += 1;

			/* Type 3. WIFI_RAM_CODE_MTxxxx_Ex.bin */
			snprintf(*(apucName + (*pucNameIdx)), CFG_FW_NAME_MAX_LEN, "%s%x_E%u.bin",
					apucNameTable[sub_idx], chip_id,
					wlanGetEcoVersion(prGlueInfo->prAdapter));
			(*pucNameIdx) += 1;

			/* Type 4. WIFI_RAM_CODE_MTxxxx_Ex */
			snprintf(*(apucName + (*pucNameIdx)), CFG_FW_NAME_MAX_LEN, "%s%x_E%u",
					apucNameTable[sub_idx], chip_id,
					wlanGetEcoVersion(prGlueInfo->prAdapter));
			(*pucNameIdx) += 1;
		} else {
			/* the table is not large enough */
			DBGLOG(INIT, ERROR, "kalFirmwareImageMapping >> file name array is not enough.\n");
			ASSERT(0);
		}
	}
}

VOID mt7668CapInit(IN P_ADAPTER_T prAdapter)
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

	switch (prGlueInfo->u4InfType) {
#if defined(_HIF_PCIE)
	case MT_DEV_INF_PCIE:
		prChipInfo->u2TxInitCmdPort = TX_RING_FWDL_IDX_3;
		prChipInfo->u2TxFwDlPort = TX_RING_FWDL_IDX_3;
		break;
#endif /* _HIF_PCIE */
#if defined(_HIF_USB)
	case MT_DEV_INF_USB:
		prChipInfo->u2TxInitCmdPort = USB_DATA_BULK_OUT_EP8;
		prChipInfo->u2TxFwDlPort = USB_DATA_BULK_OUT_EP8;
		break;
#endif /* _HIF_USB */
	default:
		break;
	}
}

BUS_INFO mt7668_bus_info = {
#if defined(_HIF_PCIE)
	.top_cfg_base = MT7668_TOP_CFG_BASE,
	.is_pcie_32dw_read = MT7668_IS_PCIE_32DW_READ, /* Litien */
	.bus2chip = mt7668_bus2chip_cr_mapping,
	.tx_ring_fwdl_idx = 3,
	.tx_ring_cmd_idx = 2,
	.tx_ring_data_idx = 0,
#endif /* _HIF_PCIE */
#if defined(_HIF_USB)
	.u4UdmaWlCfg_0_Addr = UDMA_WLCFG_0,
	.u4UdmaWlCfg_1_Addr = UDMA_WLCFG_1,
#endif /* _HIF_USB */
};

struct firmware_download_operations mt7668_fw_dl_ops = {
	.tailer_format = HARVARD_TAILER_FORMAT,
	.constructFirmwarePrio = mt7668ConstructFirmwarePrio,
	.downloadFirmware = wlanHarvardFormatDownload,
	.getFwInfo = wlanGetHarvardFwInfo,
};

/* Litien code refine to support multi chip */
struct mt66xx_chip_info mt66xx_chip_info_mt7668 = {
	.bus_info = &mt7668_bus_info,
	.fw_dl_ops = &mt7668_fw_dl_ops,

	.chip_id = MT7668_CHIP_ID,
	.sw_sync0 = MT7668_SW_SYNC0,
	.sw_ready_bits = WIFI_FUNC_READY_BITS,
	.sw_ready_bit_offset = MT7668_SW_SYNC0_RDY_OFFSET,
	.patch_addr = MT7668_PATCH_START_ADDR,
	.is_support_cr4 = TRUE,
	.txd_append_size = MT7668_TX_DESC_APPEND_LENGTH,
	.eco_info = mt7668_eco_table,

	.asicCapInit = mt7668CapInit,
	.asicEnableFWDownload = NULL,
	.fillTxDescAppend = fillTxDescAppendByCR4,
};

struct mt66xx_hif_driver_data mt66xx_driver_data_mt7668 = {
	.chip_info = &mt66xx_chip_info_mt7668,
};

