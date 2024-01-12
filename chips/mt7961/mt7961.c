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
/*! \file   mt7961.c
*    \brief  Internal driver stack will export
*    the required procedures here for GLUE Layer.
*
*    This file contains all routines which are exported
     from MediaTek 802.11 Wireless LAN driver stack to GLUE Layer.
*/

#ifdef MT7961

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/

#include "coda/mt7961/wf_wfdma_host_dma0.h"
#include "coda/mt7961/wf_wfdma_host_dma1.h"

#include "precomp.h"

#include "mt7961.h"

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

#define CONN_MCU_CONFG_BASE                0x88000000
#define CONN_MCU_CONFG_COM_REG0_ADDR       (CONN_MCU_CONFG_BASE + 0x200)

#define PATCH_SEMAPHORE_COMM_REG 0
#define PATCH_SEMAPHORE_COMM_REG_PATCH_DONE 1	/* bit0 is for patch. */

#define SW_WORKAROUND_FOR_WFDMA_ISSUE_HWITS00009838 1

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


/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/


struct ECO_INFO mt7961_eco_table[] = {
	/* HW version,  ROM version,    Factory version */
	{0x00, 0x00, 0x0}	/* End of table */
};

#if defined(_HIF_PCIE) || defined(_HIF_AXI)
struct PCIE_CHIP_CR_MAPPING mt7961_bus2chip_cr_mapping[] = {
	/* chip addr, bus addr, range */
	{0x54000000, 0x02000, 0x1000}, /* WFDMA PCIE0 MCU DMA0 */
	{0x55000000, 0x03000, 0x1000}, /* WFDMA PCIE0 MCU DMA1 */
	{0x56000000, 0x04000, 0x1000}, /* WFDMA reserved */
	{0x57000000, 0x05000, 0x1000}, /* WFDMA MCU wrap CR */
	{0x58000000, 0x06000, 0x1000}, /* WFDMA PCIE1 MCU DMA0 (MEM_DMA) */
	{0x59000000, 0x07000, 0x1000}, /* WFDMA PCIE1 MCU DMA1 */
	{0x820c0000, 0x08000, 0x4000}, /* WF_UMAC_TOP (PLE) */
	{0x820c8000, 0x0c000, 0x2000}, /* WF_UMAC_TOP (PSE) */
	{0x820cc000, 0x0e000, 0x2000}, /* WF_UMAC_TOP (PP) */
	{0x820e0000, 0x20000, 0x0400}, /* WF_LMAC_TOP BN0 (WF_CFG) */
	{0x820e1000, 0x20400, 0x0200}, /* WF_LMAC_TOP BN0 (WF_TRB) */
	{0x820e2000, 0x20800, 0x0400}, /* WF_LMAC_TOP BN0 (WF_AGG) */
	{0x820e3000, 0x20c00, 0x0400}, /* WF_LMAC_TOP BN0 (WF_ARB) */
	{0x820e4000, 0x21000, 0x0400}, /* WF_LMAC_TOP BN0 (WF_TMAC) */
	{0x820e5000, 0x21400, 0x0800}, /* WF_LMAC_TOP BN0 (WF_RMAC) */
	{0x820ce000, 0x21c00, 0x0200}, /* WF_LMAC_TOP (WF_SEC) */
	{0x820e7000, 0x21e00, 0x0200}, /* WF_LMAC_TOP BN0 (WF_DMA) */
	{0x820cf000, 0x22000, 0x1000}, /* WF_LMAC_TOP (WF_PF) */
	{0x820e9000, 0x23400, 0x0200}, /* WF_LMAC_TOP BN0 (WF_WTBLOFF) */
	{0x820ea000, 0x24000, 0x0200}, /* WF_LMAC_TOP BN0 (WF_ETBF) */
	{0x820eb000, 0x24200, 0x0400}, /* WF_LMAC_TOP BN0 (WF_LPON) */
	{0x820ec000, 0x24600, 0x0200}, /* WF_LMAC_TOP BN0 (WF_INT) */
	{0x820ed000, 0x24800, 0x0800}, /* WF_LMAC_TOP BN0 (WF_MIB) */
	{0x820ca000, 0x26000, 0x2000}, /* WF_LMAC_TOP BN0 (WF_MUCOP) */
	{0x820d0000, 0x30000, 0x10000}, /* WF_LMAC_TOP (WF_WTBLON) */
	{0x40000000, 0x70000, 0x10000}, /* WF_UMAC_SYSRAM */
	{0x00400000, 0x80000, 0x10000}, /* WF_MCU_SYSRAM */
	{0x00410000, 0x90000, 0x10000}, /* WF_MCU_SYSRAM (configure register) */
	{0x820f0000, 0xa0000, 0x0400}, /* WF_LMAC_TOP BN1 (WF_CFG) */
	{0x820f1000, 0xa0600, 0x0200}, /* WF_LMAC_TOP BN1 (WF_TRB) */
	{0x820f2000, 0xa0800, 0x0400}, /* WF_LMAC_TOP BN1 (WF_AGG) */
	{0x820f3000, 0xa0c00, 0x0400}, /* WF_LMAC_TOP BN1 (WF_ARB) */
	{0x820f4000, 0xa1000, 0x0400}, /* WF_LMAC_TOP BN1 (WF_TMAC) */
	{0x820f5000, 0xa1400, 0x0800}, /* WF_LMAC_TOP BN1 (WF_RMAC) */
	{0x820f7000, 0xa1e00, 0x0200}, /* WF_LMAC_TOP BN1 (WF_DMA) */
	{0x820f9000, 0xa3400, 0x0200}, /* WF_LMAC_TOP BN1 (WF_WTBLOFF) */
	{0x820fa000, 0xa4000, 0x0200}, /* WF_LMAC_TOP BN1 (WF_ETBF) */
	{0x820fb000, 0xa4200, 0x0400}, /* WF_LMAC_TOP BN1 (WF_LPON) */
	{0x820fc000, 0xa4600, 0x0200}, /* WF_LMAC_TOP BN1 (WF_INT) */
	{0x820fd000, 0xa4800, 0x0800}, /* WF_LMAC_TOP BN1 (WF_MIB) */
	{0x820cc000, 0xa5000, 0x2000}, /* WF_LMAC_TOP BN1 (WF_MUCOP) */
	{0x820c4000, 0xa8000, 0x4000}, /* WF_LMAC_TOP BN1 (WF_MUCOP) */
	{0x820b0000, 0xae000, 0x1000}, /* [APB2] WFSYS_ON */
	{0x80020000, 0xb0000, 0x10000}, /* WF_TOP_MISC_OFF */
	{0x81020000, 0xc0000, 0x10000}, /* WF_TOP_MISC_ON */
	{0x7c020000, 0xd0000, 0x10000}, /* CONN_INFRA, wfdma */
	{0x7c060000, 0xe0000, 0x10000}, /* CONN_INFRA, conn_host_csr_top */
	{0x7c000000, 0xf0000, 0x10000}, /* CONN_INFRA */
};
#endif /*_HIF_PCIE || _HIF_AXI */

void mt7961Connac2xProcessTxInterrupt(IN struct ADAPTER *prAdapter)
{
	struct GL_HIF_INFO *prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	union WPDMA_INT_STA_STRUCT rIntrStatus;

	rIntrStatus = (union WPDMA_INT_STA_STRUCT)prHifInfo->u4IntStatus;
	if (rIntrStatus.field_conn2x_ext.wfdma1_tx_done_16)
		halWpdmaProcessCmdDmaDone(prAdapter->prGlueInfo,
			TX_RING_FWDL_IDX_3);

	if (rIntrStatus.field_conn2x_ext.wfdma1_tx_done_17)
		halWpdmaProcessCmdDmaDone(prAdapter->prGlueInfo,
			TX_RING_CMD_IDX_2);

	if (rIntrStatus.field_conn2x_ext.wfdma1_tx_done_0) {
		halWpdmaProcessDataDmaDone(prAdapter->prGlueInfo,
			TX_RING_DATA0_IDX_0);

		kalSetTxEvent2Hif(prAdapter->prGlueInfo);
	}

}

void mt7961Connac2xProcessRxInterrupt(
	struct ADAPTER *prAdapter)
{
	struct GL_HIF_INFO *prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	union WPDMA_INT_STA_STRUCT rIntrStatus;

	rIntrStatus = (union WPDMA_INT_STA_STRUCT)prHifInfo->u4IntStatus;
	if (rIntrStatus.field_conn2x_ext.wfdma1_rx_done_0)
		halRxReceiveRFBs(prAdapter, WFDMA1_RX_RING_IDX_0, FALSE);

	if (rIntrStatus.field_conn2x_ext.wfdma0_rx_done_0)
		halRxReceiveRFBs(prAdapter, RX_RING_DATA_IDX_0, TRUE);

	if (rIntrStatus.field_conn2x_ext.wfdma0_rx_done_1)
		halRxReceiveRFBs(prAdapter, RX_RING_EVT_IDX_1, TRUE);

	if (rIntrStatus.field_conn2x_ext.wfdma0_rx_done_2)
		halRxReceiveRFBs(prAdapter, WFDMA0_RX_RING_IDX_2, TRUE);

	if (rIntrStatus.field_conn2x_ext.wfdma0_rx_done_3)
		halRxReceiveRFBs(prAdapter, WFDMA0_RX_RING_IDX_3, TRUE);
}

void mt7961Connac2xWfdmaManualPrefetch(
	struct GLUE_INFO *prGlueInfo)
{
	struct ADAPTER *prAdapter = prGlueInfo->prAdapter;
	u_int32_t val = 0;

	HAL_MCR_RD(prAdapter, WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_ADDR, &val);
	/* disable prefetch offset calculation auto-mode */
	val &=
	~WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_CSR_DISP_BASE_PTR_CHAIN_EN_MASK;
	HAL_MCR_WR(prAdapter, WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_ADDR, val);

	HAL_MCR_RD(prAdapter, WF_WFDMA_HOST_DMA1_WPDMA_GLO_CFG_ADDR, &val);
	/* disable prefetch offset calculation auto-mode */
	val &=
	~WF_WFDMA_HOST_DMA1_WPDMA_GLO_CFG_CSR_DISP_BASE_PTR_CHAIN_EN_MASK;
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA1_WPDMA_GLO_CFG_ADDR, val);

	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA0_WPDMA_RX_RING0_EXT_CTRL_ADDR, 0x00000004);
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA0_WPDMA_RX_RING1_EXT_CTRL_ADDR, 0x00400004);
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA0_WPDMA_RX_RING2_EXT_CTRL_ADDR, 0x00800004);
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA0_WPDMA_RX_RING3_EXT_CTRL_ADDR, 0x00c00004);

#if (SW_WORKAROUND_FOR_WFDMA_ISSUE_HWITS00009838 == 1)
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA0_WPDMA_RX_RING4_EXT_CTRL_ADDR, 0x01000004);
#endif

	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA1_WPDMA_TX_RING0_EXT_CTRL_ADDR, 0x01400004);
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA1_WPDMA_TX_RING1_EXT_CTRL_ADDR, 0x01800004);
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA1_WPDMA_TX_RING2_EXT_CTRL_ADDR, 0x01c00004);

	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA1_WPDMA_TX_RING3_EXT_CTRL_ADDR, 0x02000004);
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA1_WPDMA_TX_RING4_EXT_CTRL_ADDR, 0x02400004);
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA1_WPDMA_TX_RING5_EXT_CTRL_ADDR, 0x02800004);
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA1_WPDMA_TX_RING6_EXT_CTRL_ADDR, 0x02c00004);

#if (SW_WORKAROUND_FOR_WFDMA_ISSUE_HWITS00009838 == 1)
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA1_WPDMA_TX_RING7_EXT_CTRL_ADDR, 0x03000004);
#endif

	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA1_WPDMA_TX_RING16_EXT_CTRL_ADDR, 0x03400004);
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA1_WPDMA_TX_RING17_EXT_CTRL_ADDR, 0x03800004);

#if (SW_WORKAROUND_FOR_WFDMA_ISSUE_HWITS00009838 == 1)
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA1_WPDMA_TX_RING18_EXT_CTRL_ADDR, 0x03c00004);
#endif

	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA1_WPDMA_RX_RING0_EXT_CTRL_ADDR, 0x04800004);
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA1_WPDMA_RX_RING1_EXT_CTRL_ADDR, 0x04C00004);

#if (SW_WORKAROUND_FOR_WFDMA_ISSUE_HWITS00009838 == 1)
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA1_WPDMA_RX_RING2_EXT_CTRL_ADDR, 0x05000004);
#endif

	/* reset dma idx */
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA0_WPDMA_RST_DTX_PTR_ADDR, 0xFFFFFFFF);
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA1_WPDMA_RST_DTX_PTR_ADDR, 0xFFFFFFFF);
}


#if defined(_HIF_USB)
uint16_t wlanHarrierUsbRxByteCount(
	struct ADAPTER *prAdapter,
	struct BUS_INFO *prBusInfo,
	uint8_t *pRXD)
{
	uint16_t u2RxByteCount;
	uint8_t ucPacketType;
	uint8_t ucHdrTrans;

	ucHdrTrans = NIC_RX_FLAG_IS_HEADER_TRAN(pRXD);
	ucPacketType = NIC_RX_GET_PKT_TYPE(pRXD);
	u2RxByteCount = NIC_RX_GET_RX_BYTE_CNT(pRXD);
	if ((ucPacketType == RX_PKT_TYPE_RX_DATA) && ucHdrTrans)
		u2RxByteCount = ALIGN_8(u2RxByteCount)
			+ prBusInfo->u4RxPaddingRxInfo;
	else
		u2RxByteCount = ALIGN_4(u2RxByteCount)
			+ prBusInfo->u4RxPaddingCSO;

	return u2RxByteCount;
}
#endif /* defined(_HIF_USB) */

struct BUS_INFO mt7961_bus_info = {
#if defined(_HIF_PCIE) || defined(_HIF_AXI)
	.top_cfg_base = MT7961_TOP_CFG_BASE,

	/* host_dma0 for TXP */
	.host_dma0_base = CONNAC2X_HOST_WPDMA_0_BASE,
	/* host_dma1 for TXD and host cmd to WX_CPU */
	.host_dma1_base = CONNAC2X_HOST_WPDMA_1_BASE,
	.host_ext_conn_hif_wrap_base = CONNAC2X_HOST_EXT_CONN_HIF_WRAP,
	.host_int_status_addr =
		CONNAC2X_WPDMA_EXT_INT_STA(CONNAC2X_HOST_EXT_CONN_HIF_WRAP),

	.host_int_txdone_bits = (CONNAC2X_EXT_WFDMA1_TX_DONE_INT0
				| CONNAC2X_EXT_WFDMA1_TX_DONE_INT1
				| CONNAC2X_EXT_WFDMA1_TX_DONE_INT2
				| CONNAC2X_EXT_WFDMA1_TX_DONE_INT3
				| CONNAC2X_EXT_WFDMA1_TX_DONE_INT4
				| CONNAC2X_EXT_WFDMA1_TX_DONE_INT5
				| CONNAC2X_EXT_WFDMA1_TX_DONE_INT6
				| CONNAC2X_EXT_WFDMA1_TX_DONE_INT16
				| CONNAC2X_EXT_WFDMA1_TX_DONE_INT17),
	.host_int_rxdone_bits = (CONNAC2X_EXT_WFDMA1_RX_DONE_INT0
				| CONNAC2X_EXT_WFDMA0_RX_DONE_INT0
				| CONNAC2X_EXT_WFDMA0_RX_DONE_INT1
				| CONNAC2X_EXT_WFDMA0_RX_DONE_INT2
				| CONNAC2X_EXT_WFDMA0_RX_DONE_INT3
				),

	.host_tx_ring_base =
		CONNAC2X_TX_RING_BASE(CONNAC2X_HOST_WPDMA_1_BASE),
	.host_tx_ring_ext_ctrl_base =
		CONNAC2X_TX_RING_EXT_CTRL_BASE(CONNAC2X_HOST_WPDMA_1_BASE),
	.host_tx_ring_cidx_addr =
		CONNAC2X_TX_RING_CIDX(CONNAC2X_HOST_WPDMA_1_BASE),
	.host_tx_ring_didx_addr =
		CONNAC2X_TX_RING_DIDX(CONNAC2X_HOST_WPDMA_1_BASE),
	.host_tx_ring_cnt_addr =
		CONNAC2X_TX_RING_CNT(CONNAC2X_HOST_WPDMA_1_BASE),

	.host_rx_ring_base =
		CONNAC2X_RX_RING_BASE(CONNAC2X_HOST_WPDMA_0_BASE),
	.host_rx_ring_ext_ctrl_base =
		CONNAC2X_RX_RING_EXT_CTRL_BASE(CONNAC2X_HOST_WPDMA_0_BASE),
	.host_rx_ring_cidx_addr =
		CONNAC2X_RX_RING_CIDX(CONNAC2X_HOST_WPDMA_0_BASE),
	.host_rx_ring_didx_addr =
		CONNAC2X_RX_RING_DIDX(CONNAC2X_HOST_WPDMA_0_BASE),
	.host_rx_ring_cnt_addr =
		CONNAC2X_RX_RING_CNT(CONNAC2X_HOST_WPDMA_0_BASE),

	.host_wfdma1_rx_ring_base =
		CONNAC2X_WFDMA1_RX_RING_BASE(CONNAC2X_HOST_WPDMA_1_BASE),
	.host_wfdma1_rx_ring_cidx_addr =
		CONNAC2X_WFDMA1_RX_RING_CIDX(CONNAC2X_HOST_WPDMA_1_BASE),
	.host_wfdma1_rx_ring_didx_addr =
		CONNAC2X_WFDMA1_RX_RING_DIDX(CONNAC2X_HOST_WPDMA_1_BASE),
	.host_wfdma1_rx_ring_cnt_addr =
		CONNAC2X_WFDMA1_RX_RING_CNT(CONNAC2X_HOST_WPDMA_1_BASE),
	.host_wfdma1_rx_ring_ext_ctrl_base =
		CONNAC2X_WFDMA1_RX_RING_EXT_CTRL_BASE(
			CONNAC2X_HOST_WPDMA_1_BASE),

	.bus2chip = mt7961_bus2chip_cr_mapping,
	.max_static_map_addr = 0x000f0000,

	.tx_ring_fwdl_idx = CONNAC2X_FWDL_TX_RING_IDX,
	.tx_ring_cmd_idx = CONNAC2X_CMD_TX_RING_IDX,
	.tx_ring0_data_idx = 0,
	.tx_ring1_data_idx = 1,
	.fw_own_clear_addr = CONNAC2X_BN0_IRQ_STAT_ADDR,
	.fw_own_clear_bit = PCIE_LPCR_FW_CLR_OWN,
	.fgCheckDriverOwnInt = FALSE,
	.u4DmaMask = 32,
	.pdmaSetup = asicConnac2xWpdmaConfig,
	.enableInterrupt = asicConnac2xEnableExtInterrupt,
	.disableInterrupt = asicConnac2xDisableExtInterrupt,
	.processTxInterrupt = mt7961Connac2xProcessTxInterrupt,
	.processRxInterrupt = mt7961Connac2xProcessRxInterrupt,
	.tx_ring_ext_ctrl = asicConnac2xWfdmaTxRingExtCtrl,
	.rx_ring_ext_ctrl = asicConnac2xWfdmaRxRingExtCtrl,
	/* null wfdmaManualPrefetch if want to disable manual mode */
	.wfdmaManualPrefetch = mt7961Connac2xWfdmaManualPrefetch,
	.lowPowerOwnRead = asicConnac2xLowPowerOwnRead,
	.lowPowerOwnSet = asicConnac2xLowPowerOwnSet,
	.lowPowerOwnClear = asicConnac2xLowPowerOwnClear,
	.wakeUpWiFi = asicWakeUpWiFi,
	.processSoftwareInterrupt = asicConnac2xProcessSoftwareInterrupt,
	.softwareInterruptMcu = asicConnac2xSoftwareInterruptMcu,
	.hifRst = asicConnac2xHifRst,

	.initPcieInt = NULL, /* Todo: check if enable INT on driver side */
	.devReadIntStatus = asicConnac2xReadExtIntStatus,
	.pcieDmaShdlInit = NULL, /* Todo: hook it by owner */

#endif				/*_HIF_PCIE || _HIF_AXI */
};

#if CFG_ENABLE_FW_DOWNLOAD
struct FWDL_OPS_T mt7961_fw_dl_ops = {
	.constructFirmwarePrio = NULL,
	.constructPatchName = NULL,
	.downloadPatch = NULL, /*wlanDownloadPatch,*/
	.downloadFirmware = wlanConnacFormatDownload,
	.getFwInfo = wlanGetConnacFwInfo,
	.getFwDlInfo = asicGetFwDlInfo,
};
#endif				/* CFG_ENABLE_FW_DOWNLOAD */

struct TX_DESC_OPS_T mt7961TxDescOps = {
	.fillNicAppend = fillNicTxDescAppend,
	.fillHifAppend = fillTxDescAppendByHostV2,
	.fillTxByteCount = fillConnac2xTxDescTxByteCount,
};

struct RX_DESC_OPS_T mt7961RxDescOps = {
};

struct CHIP_DBG_OPS mt7961DebugOps = {
	.showPdmaInfo = NULL,
	.showPseInfo = NULL,
	.showPleInfo = NULL,
	.showTxdInfo = connac2x_show_txd_Info,
	.showWtblInfo = connac2x_show_wtbl_info,
	.showUmacFwtblInfo = connac2x_show_umac_wtbl_info,
	.showCsrInfo = NULL,
	.showDmaschInfo = NULL,
	.showHifInfo = NULL,
	.printHifDbgInfo = NULL,
	.show_rx_rate_info = connac2x_show_rx_rate_info,
	.show_rx_rssi_info = connac2x_show_rx_rssi_info,
};

/* Litien code refine to support multi chip */
struct mt66xx_chip_info mt66xx_chip_info_mt7961 = {
	.bus_info = &mt7961_bus_info,
#if CFG_ENABLE_FW_DOWNLOAD
	.fw_dl_ops = &mt7961_fw_dl_ops,
#endif				/* CFG_ENABLE_FW_DOWNLOAD */
	.prTxDescOps = &mt7961TxDescOps,
	.prRxDescOps = &mt7961RxDescOps,
	.prDebugOps = &mt7961DebugOps,
	.chip_id = MT7961_CHIP_ID,
	.should_verify_chip_id = FALSE,
	.sw_sync0 = MT7961_SW_SYNC0,
	.sw_ready_bits = WIFI_FUNC_NO_CR4_READY_BITS,
	.sw_ready_bit_offset = MT7961_SW_SYNC0_RDY_OFFSET,
	.patch_addr = MT7961_PATCH_START_ADDR,
	.is_support_cr4 = FALSE,
	.is_support_wacpu = FALSE,
	.txd_append_size = MT7961_TX_DESC_APPEND_LENGTH,
	.rxd_size = MT7961_RX_DESC_LENGTH,
	.pse_header_length = CONNAC2X_NIC_TX_PSE_HEADER_LENGTH,
	.init_event_size = CONNAC2X_RX_INIT_EVENT_LENGTH,
	.eco_info = mt7961_eco_table,
	.isNicCapV1 = FALSE,
	.top_hcr = CONNAC2X_TOP_HCR,
	.top_hvr = CONNAC2X_TOP_HVR,
	.top_fvr = CONNAC2X_TOP_FVR,
	.arb_ac_mode_addr = MT7961_ARB_AC_MODE_ADDR,
	.asicCapInit = asicConnac2xCapInit,
#if CFG_ENABLE_FW_DOWNLOAD
	.asicEnableFWDownload = NULL,
#endif				/* CFG_ENABLE_FW_DOWNLOAD */
	.downloadBufferBin = wlanConnacDownloadBufferBin,
	.is_support_hw_amsdu = TRUE,
	.is_support_asic_lp = TRUE,
	.is_support_wfdma = TRUE,
	.asicWfdmaReInit = asicConnac2xWfdmaReInit,
};

struct mt66xx_hif_driver_data mt66xx_driver_data_mt7961 = {
	.chip_info = &mt66xx_chip_info_mt7961,
};

#endif				/* MT7961 */

