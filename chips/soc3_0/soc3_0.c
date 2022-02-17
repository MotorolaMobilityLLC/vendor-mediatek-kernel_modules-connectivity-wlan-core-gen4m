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
/*! \file   soc3_0.c
*    \brief  Internal driver stack will export
*    the required procedures here for GLUE Layer.
*
*    This file contains all routines which are exported
     from MediaTek 802.11 Wireless LAN driver stack to GLUE Layer.
*/

#ifdef SOC3_0

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/

#include "coda/soc3_0/wf_wfdma_host_dma0.h"
#include "coda/soc3_0/wf_wfdma_host_dma1.h"

#if (CFG_SUPPORT_CONNINFRA == 1)
#include "coda/soc3_0/conn_infra_cfg.h"
#endif

#include "precomp.h"

#include "soc3_0.h"

#include <linux/platform_device.h>

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

#define CONN_MCU_CONFG_BASE                0x88000000
#define CONN_MCU_CONFG_COM_REG0_ADDR       (CONN_MCU_CONFG_BASE + 0x200)

#define PATCH_SEMAPHORE_COMM_REG 0
#define PATCH_SEMAPHORE_COMM_REG_PATCH_DONE 1	/* bit0 is for patch. */

#define SW_WORKAROUND_FOR_WFDMA_ISSUE_HWITS00009838 1

#define SOC3_0_FILE_NAME_TOTAL 8
#define SOC3_0_FILE_NAME_MAX 64
/* this is workaround for AXE, do not sync back to trunk*/

#if (CFG_SUPPORT_CONNINFRA == 1)
static struct sub_drv_ops_cb g_conninfra_wf_cb;
#endif

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
#if (CFG_POWER_ON_DOWNLOAD_EMI_ROM_PATCH == 1)
static uint8_t *soc3_0_apucFwName[] = {
	(uint8_t *) CFG_FW_FILENAME "_MT",
	NULL
};

static uint8_t *soc3_0_apucCr4FwName[] = {
	(uint8_t *) CFG_CR4_FW_FILENAME "_MT",
	NULL
};
#endif

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

uint8_t *apucsoc3_0FwName[] = {
	(uint8_t *) CFG_FW_FILENAME "_soc3_0",
	NULL
};

struct ECO_INFO soc3_0_eco_table[] = {
	/* HW version,  ROM version,    Factory version */
	{0x00, 0x00, 0xA, 0x1}, /* E1 */
	{0x00, 0x00, 0x0, 0x0}	/* End of table */
};

#if defined(_HIF_PCIE)
struct PCIE_CHIP_CR_MAPPING soc3_0_bus2chip_cr_mapping[] = {
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
	{0x820c4000, 0xa8000, 0x4000}, /* WF_LMAC_TOP (WF_UWTBL)  */
	{0x820b0000, 0xae000, 0x1000}, /* [APB2] WFSYS_ON */
	{0x80020000, 0xb0000, 0x10000}, /* WF_TOP_MISC_OFF */
	{0x81020000, 0xc0000, 0x10000}, /* WF_TOP_MISC_ON */
	{0x7c500000, 0x50000, 0x10000}, /* CONN_INFRA, dyn mem map */
	{0x7c020000, 0xd0000, 0x10000}, /* CONN_INFRA, wfdma */
	{0x7c060000, 0xe0000, 0x10000}, /* CONN_INFRA, conn_host_csr_top */
	{0x7c000000, 0xf0000, 0x10000}, /* CONN_INFRA */
	{0x0, 0x0, 0x0} /* End */
};
#elif defined(_HIF_AXI)
struct PCIE_CHIP_CR_MAPPING soc3_0_bus2chip_cr_mapping[] = {
	/* chip addr, bus addr, range */
	{0x54000000, 0x402000, 0x1000}, /* WFDMA PCIE0 MCU DMA0 */
	{0x55000000, 0x403000, 0x1000}, /* WFDMA PCIE0 MCU DMA1 */
	{0x56000000, 0x404000, 0x1000}, /* WFDMA reserved */
	{0x57000000, 0x405000, 0x1000}, /* WFDMA MCU wrap CR */
	{0x58000000, 0x406000, 0x1000}, /* WFDMA PCIE1 MCU DMA0 (MEM_DMA) */
	{0x59000000, 0x407000, 0x1000}, /* WFDMA PCIE1 MCU DMA1 */
	{0x820c0000, 0x408000, 0x4000}, /* WF_UMAC_TOP (PLE) */
	{0x820c8000, 0x40c000, 0x2000}, /* WF_UMAC_TOP (PSE) */
	{0x820cc000, 0x40e000, 0x2000}, /* WF_UMAC_TOP (PP) */
	{0x820e0000, 0x420000, 0x0400}, /* WF_LMAC_TOP BN0 (WF_CFG) */
	{0x820e1000, 0x420400, 0x0200}, /* WF_LMAC_TOP BN0 (WF_TRB) */
	{0x820e2000, 0x420800, 0x0400}, /* WF_LMAC_TOP BN0 (WF_AGG) */
	{0x820e3000, 0x420c00, 0x0400}, /* WF_LMAC_TOP BN0 (WF_ARB) */
	{0x820e4000, 0x421000, 0x0400}, /* WF_LMAC_TOP BN0 (WF_TMAC) */
	{0x820e5000, 0x421400, 0x0800}, /* WF_LMAC_TOP BN0 (WF_RMAC) */
	{0x820ce000, 0x421c00, 0x0200}, /* WF_LMAC_TOP (WF_SEC) */
	{0x820e7000, 0x421e00, 0x0200}, /* WF_LMAC_TOP BN0 (WF_DMA) */
	{0x820cf000, 0x422000, 0x1000}, /* WF_LMAC_TOP (WF_PF) */
	{0x820e9000, 0x423400, 0x0200}, /* WF_LMAC_TOP BN0 (WF_WTBLOFF) */
	{0x820ea000, 0x424000, 0x0200}, /* WF_LMAC_TOP BN0 (WF_ETBF) */
	{0x820eb000, 0x424200, 0x0400}, /* WF_LMAC_TOP BN0 (WF_LPON) */
	{0x820ec000, 0x424600, 0x0200}, /* WF_LMAC_TOP BN0 (WF_INT) */
	{0x820ed000, 0x424800, 0x0800}, /* WF_LMAC_TOP BN0 (WF_MIB) */
	{0x820ca000, 0x426000, 0x2000}, /* WF_LMAC_TOP BN0 (WF_MUCOP) */
	{0x820d0000, 0x430000, 0x10000}, /* WF_LMAC_TOP (WF_WTBLON) */
	{0x40000000, 0x470000, 0x10000}, /* WF_UMAC_SYSRAM */
	{0x00400000, 0x480000, 0x10000}, /* WF_MCU_SYSRAM */
	{0x00410000, 0x490000, 0x10000}, /* WF_MCU_SYSRAM (config register) */
	{0x820f0000, 0x4a0000, 0x0400}, /* WF_LMAC_TOP BN1 (WF_CFG) */
	{0x820f1000, 0x4a0600, 0x0200}, /* WF_LMAC_TOP BN1 (WF_TRB) */
	{0x820f2000, 0x4a0800, 0x0400}, /* WF_LMAC_TOP BN1 (WF_AGG) */
	{0x820f3000, 0x4a0c00, 0x0400}, /* WF_LMAC_TOP BN1 (WF_ARB) */
	{0x820f4000, 0x4a1000, 0x0400}, /* WF_LMAC_TOP BN1 (WF_TMAC) */
	{0x820f5000, 0x4a1400, 0x0800}, /* WF_LMAC_TOP BN1 (WF_RMAC) */
	{0x820f7000, 0x4a1e00, 0x0200}, /* WF_LMAC_TOP BN1 (WF_DMA) */
	{0x820f9000, 0x4a3400, 0x0200}, /* WF_LMAC_TOP BN1 (WF_WTBLOFF) */
	{0x820fa000, 0x4a4000, 0x0200}, /* WF_LMAC_TOP BN1 (WF_ETBF) */
	{0x820fb000, 0x4a4200, 0x0400}, /* WF_LMAC_TOP BN1 (WF_LPON) */
	{0x820fc000, 0x4a4600, 0x0200}, /* WF_LMAC_TOP BN1 (WF_INT) */
	{0x820fd000, 0x4a4800, 0x0800}, /* WF_LMAC_TOP BN1 (WF_MIB) */
	{0x820cc000, 0x4a5000, 0x2000}, /* WF_LMAC_TOP BN1 (WF_MUCOP) */
	{0x820c4000, 0x4a8000, 0x4000}, /* WF_LMAC_TOP (WF_UWTBL)  */
	{0x820b0000, 0x4ae000, 0x1000}, /* [APB2] WFSYS_ON */
	{0x80020000, 0x4b0000, 0x10000}, /* WF_TOP_MISC_OFF */
	{0x81020000, 0x4c0000, 0x10000}, /* WF_TOP_MISC_ON */
	{0x7c500000, 0x500000, 0x10000}, /* CONN_INFRA, dyn mem map */
	{0x7c020000, 0x20000, 0x10000}, /* CONN_INFRA, wfdma */
	{0x7c060000, 0x60000, 0x10000}, /* CONN_INFRA, conn_host_csr_top */
	{0x7c000000, 0x00000, 0x10000}, /* CONN_INFRA, conn_infra_on */
	{0x0, 0x0, 0x0} /* End */
};
#endif

static bool soc3_0WfdmaAllocRxRing(
	struct GLUE_INFO *prGlueInfo,
	bool fgAllocMem)
{
	if (!halWpdmaAllocRxRing(prGlueInfo, WFDMA0_RX_RING_IDX_2,
			RX_RING1_SIZE, RXD_SIZE, RX_BUFFER_AGGRESIZE,
			fgAllocMem)) {
		DBGLOG(HAL, ERROR, "AllocWfdmaRxRing fail\n");
		return false;
	}
	if (!halWpdmaAllocRxRing(prGlueInfo, WFDMA0_RX_RING_IDX_3,
			RX_RING1_SIZE, RXD_SIZE, RX_BUFFER_AGGRESIZE,
			fgAllocMem)) {
		DBGLOG(HAL, ERROR, "AllocWfdmaRxRing fail\n");
		return false;
	}
	if (!halWpdmaAllocRxRing(prGlueInfo, WFDMA1_RX_RING_IDX_0,
			RX_RING1_SIZE, RXD_SIZE, RX_BUFFER_AGGRESIZE,
			fgAllocMem)) {
		DBGLOG(HAL, ERROR, "AllocWfdmaRxRing fail\n");
		return false;
	}
	return true;
}

void soc3_0asicConnac2xProcessTxInterrupt(IN struct ADAPTER *prAdapter)
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

void soc3_0asicConnac2xProcessRxInterrupt(
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

void soc3_0asicConnac2xWfdmaManualPrefetch(
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

struct BUS_INFO soc3_0_bus_info = {
#if defined(_HIF_PCIE) || defined(_HIF_AXI)
	.top_cfg_base = SOC3_0_TOP_CFG_BASE,

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

	.bus2chip = soc3_0_bus2chip_cr_mapping,
#if defined(_HIF_PCIE)
	.max_static_map_addr = 0x000f0000,
#elif defined(_HIF_AXI)
	.max_static_map_addr = 0x00700000,
#endif
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
	.processTxInterrupt = soc3_0asicConnac2xProcessTxInterrupt,
	.processRxInterrupt = soc3_0asicConnac2xProcessRxInterrupt,
	.tx_ring_ext_ctrl = asicConnac2xWfdmaTxRingExtCtrl,
	.rx_ring_ext_ctrl = asicConnac2xWfdmaRxRingExtCtrl,
	/* null wfdmaManualPrefetch if want to disable manual mode */
	.wfdmaManualPrefetch = soc3_0asicConnac2xWfdmaManualPrefetch,
	.lowPowerOwnRead = asicConnac2xLowPowerOwnRead,
	.lowPowerOwnSet = asicConnac2xLowPowerOwnSet,
	.lowPowerOwnClear = asicConnac2xLowPowerOwnClear,
	.wakeUpWiFi = asicWakeUpWiFi,
	.processSoftwareInterrupt = asicConnac2xProcessSoftwareInterrupt,
	.softwareInterruptMcu = asicConnac2xSoftwareInterruptMcu,
	.hifRst = asicConnac2xHifRst,

	.initPcieInt = NULL,
	.devReadIntStatus = asicConnac2xReadExtIntStatus,
	.DmaShdlInit = NULL,
	.wfdmaAllocRxRing = soc3_0WfdmaAllocRxRing,
#endif			/*_HIF_PCIE || _HIF_AXI */
};

#if CFG_ENABLE_FW_DOWNLOAD
struct FWDL_OPS_T soc3_0_fw_dl_ops = {
#if (CFG_POWER_ON_DOWNLOAD_EMI_ROM_PATCH == 1)
	.constructFirmwarePrio = soc3_0_ConstructFirmwarePrio,
#else
	.constructFirmwarePrio = NULL,
#endif
	.constructPatchName = NULL,
	.downloadPatch = NULL,
	.downloadFirmware = wlanConnacFormatDownload,
#if (CFG_DOWNLOAD_DYN_MEMORY_MAP == 1)
	.downloadByDynMemMap = soc3_0_DownloadByDynMemMap,
#else
	.downloadByDynMemMap = NULL,
#endif
	.getFwInfo = wlanGetConnacFwInfo,
	.getFwDlInfo = asicGetFwDlInfo,
};
#endif			/* CFG_ENABLE_FW_DOWNLOAD */

struct TX_DESC_OPS_T soc3_0_TxDescOps = {
	.fillNicAppend = fillNicTxDescAppend,
	.fillHifAppend = fillTxDescAppendByHostV2,
	.fillTxByteCount = fillConnac2xTxDescTxByteCount,
};

struct RX_DESC_OPS_T soc3_0_RxDescOps = {
};

struct CHIP_DBG_OPS soc3_0_debug_ops = {
	.showPdmaInfo = soc3_0_show_wfdma_info,
	.showPseInfo = soc3_0_show_pse_info,
	.showPleInfo = soc3_0_show_ple_info,
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
struct mt66xx_chip_info mt66xx_chip_info_soc3_0 = {
	.bus_info = &soc3_0_bus_info,
#if CFG_ENABLE_FW_DOWNLOAD
	.fw_dl_ops = &soc3_0_fw_dl_ops,
#endif				/* CFG_ENABLE_FW_DOWNLOAD */

	.prDebugOps = &soc3_0_debug_ops,
	.prTxDescOps = &soc3_0_TxDescOps,
	.prRxDescOps = &soc3_0_RxDescOps,
	.chip_id = SOC3_0_CHIP_ID,
	.should_verify_chip_id = FALSE,
	.sw_sync0 = SOC3_0_SW_SYNC0,
	.sw_ready_bits = WIFI_FUNC_NO_CR4_READY_BITS,
	.sw_ready_bit_offset = SOC3_0_SW_SYNC0_RDY_OFFSET,
	.patch_addr = SOC3_0_PATCH_START_ADDR,
	.is_support_cr4 = FALSE,
	.is_support_wacpu = FALSE,
	.txd_append_size = SOC3_0_TX_DESC_APPEND_LENGTH,
	.rxd_size = SOC3_0_RX_DESC_LENGTH,

	.pse_header_length = CONNAC2X_NIC_TX_PSE_HEADER_LENGTH,
	.init_event_size = CONNAC2X_RX_INIT_EVENT_LENGTH,
	.eco_info = soc3_0_eco_table,
	.isNicCapV1 = FALSE,
	.top_hcr = CONNAC2X_TOP_HCR,
	.top_hvr = CONNAC2X_TOP_HVR,
	.top_fvr = CONNAC2X_TOP_FVR,
	.arb_ac_mode_addr = SOC3_0_ARB_AC_MODE_ADDR,
	.asicCapInit = asicConnac2xCapInit,
#if CFG_ENABLE_FW_DOWNLOAD
	.asicEnableFWDownload = NULL,
#endif				/* CFG_ENABLE_FW_DOWNLOAD */
	.downloadBufferBin = wlanConnacDownloadBufferBin,
	.is_support_hw_amsdu = TRUE,
	.is_support_asic_lp = TRUE,
	.is_support_wfdma1 = TRUE,
	.asicWfdmaReInit = asicConnac2xWfdmaReInit,
	.asicWfdmaReInit_handshakeInit = asicConnac2xWfdmaDummyCrWrite,
	.group5_size = sizeof(struct HW_MAC_RX_STS_GROUP_5),
	.u4LmacWtblDUAddr = CONNAC2X_WIFI_LWTBL_BASE,
	.u4UmacWtblDUAddr = CONNAC2X_WIFI_UWTBL_BASE,
	.wmmcupwron = hifWmmcuPwrOn,
	.wmmcupwroff = hifWmmcuPwrOff,
#if (CFG_POWER_ON_DOWNLOAD_EMI_ROM_PATCH == 1)
	.pwrondownload = soc3_0_wlanPowerOnDownload,
#else
	.pwrondownload = NULL,
#endif
	.triggerfwassert = soc3_0_Trigger_fw_assert,
#if (CFG_SUPPORT_CONNINFRA == 1)
	.trigger_wholechiprst = soc3_0_Trigger_whole_chip_rst,
	.sw_interrupt_handler = soc3_0_Sw_interrupt_handler,
	.conninra_cb_register = soc3_0_Conninfra_cb_register
#endif
};

struct mt66xx_hif_driver_data mt66xx_driver_data_soc3_0 = {
	.chip_info = &mt66xx_chip_info_soc3_0,
};

#if (CFG_DOWNLOAD_DYN_MEMORY_MAP == 1)
uint32_t soc3_0_DownloadByDynMemMap(IN struct ADAPTER *prAdapter,
	IN uint32_t u4Addr, IN uint32_t u4Len,
	IN uint8_t *pucStartPtr, IN enum ENUM_IMG_DL_IDX_T eDlIdx)
{
	uint32_t u4Val = 0;
	uint32_t u4Idx = 0;
	uint32_t u4NonZeroMemCnt = 0;
#if defined(_HIF_AXI)
	struct GL_HIF_INFO *prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
#endif

#if defined(_HIF_PCIE)
	struct GL_HIF_INFO *prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
#endif

	if ((eDlIdx == IMG_DL_IDX_PATCH) || (eDlIdx == IMG_DL_IDX_N9_FW)) {
/*#pragma message("wlanDownloadSectionByDynMemMap()::SOC3_0")*/
#if defined(_HIF_AXI)
		/* AXI workable version by bytes of u4Len in one movement */
		/* for PCIe WLAN drv, use 0x7C000000 based;
		 * for AXI, also use 0x7C000000 based
		 */
		HAL_MCR_WR(prAdapter,
			CONN_INFRA_CFG_AP2WF_REMAP_1_ADDR,
			u4Addr);

		if (eDlIdx == IMG_DL_IDX_PATCH) {
			do {
				u4Val = 0;

				kalMemCopy(
					(void *)&u4Val,
					(void *)(
						prHifInfo->CSRBaseAddress+
						0x500000+(u4Idx * 4)),
					4);

				if (u4Val != 0)
					u4NonZeroMemCnt++;
			} while (u4Idx++ < 20);

			if (u4NonZeroMemCnt != 0) {
				DBGLOG(INIT, WARN,
					"[MJ]ROM patch exists(%d)!\n",
					u4NonZeroMemCnt);

				return WLAN_STATUS_NOT_ACCEPTED;
			}
		}

		memcpy_toio((void *)(prHifInfo->CSRBaseAddress+0x500000),
			(void *)pucStartPtr, u4Len);
#endif

#if defined(_HIF_PCIE)
		HAL_MCR_RD(prAdapter,
			CONN_INFRA_CFG_PCIE2AP_REMAP_2_ADDR, &u4Val);

		DBGLOG(INIT, WARN, "[MJ]ORIG(0x%08x) = 0x%08x\n",
			CONN_INFRA_CFG_PCIE2AP_REMAP_2_ADDR, u4Val);

		/*
		 * 0x18=0x7C
		 * 0x18001120[31:0] = 0x00100000
		 * 0x18500000  (AP) is 0x00100000(MCU)
		 * 0x18001198[31:16] = 0x1850
		 * 0x18001198 = 0x1850|Value[15:0]

		u4Val = ((~BITS(31,16) & u4Val) | ((0x1850) << 16));
		*/

		/* this hard code is verified with DE for SOC3_0 */
		u4Val = CONN_INFRA_CFG_PCIE2AP_REMAP_2_ADDR_DE_HARDCODE;

		HAL_MCR_WR(prAdapter,
			CONN_INFRA_CFG_PCIE2AP_REMAP_2_ADDR, u4Val);

		HAL_MCR_RD(prAdapter,
			CONN_INFRA_CFG_PCIE2AP_REMAP_2_ADDR, &u4Val);

		DBGLOG(INIT, WARN, "[MJ]NEW(0x%08x) = 0x%08x\n",
			CONN_INFRA_CFG_PCIE2AP_REMAP_2_ADDR, u4Val);

#if 1
		/* PCIe workable version by bytes of u4Len in one movement */
		/* for PCIe WLAN drv, use 0x7C000000 based;
		 * for AXI, use 0x18000000 based
		 */
		HAL_MCR_WR(prAdapter,
			CONN_INFRA_CFG_AP2WF_REMAP_1_ADDR,
			u4Addr);

		if (eDlIdx == IMG_DL_IDX_PATCH) {
			do {
				u4Val = 0;

				HAL_MCR_RD(prAdapter,
				(CONN_INFRA_CFG_AP2WF_BUS_ADDR +
				(u4Idx * 4)),	&u4Val);

				if (u4Val != 0)
					u4NonZeroMemCnt++;
			} while (u4Idx++ < 20);

			if (u4NonZeroMemCnt != 0) {
				DBGLOG(INIT, WARN,
					"[MJ]ROM patch exists(%d)!\n",
					u4NonZeroMemCnt);

				return WLAN_STATUS_NOT_ACCEPTED;
			}
		}

		/* because
		* soc3_0_bus2chip_cr_mapping =
		*	{0x7c500000, 0x50000, 0x10000}
		*/
		kalMemCopy((void *)(prHifInfo->CSRBaseAddress+0x50000),
			(void *)pucStartPtr, u4Len);
#else
		/* PCIe workable version by 4 bytes in one movement */
		/* for PCIe WLAN drv, use 0x7C000000 based;
		 * for AXI, use 0x18000000 based
		 */
		HAL_MCR_WR(prAdapter,
			CONN_INFRA_CFG_AP2WF_REMAP_1_ADDR,
			u4Addr);

		for (u4Offset = 0; u4Len > 0; u4Offset += u4RemapSize) {
			if (u4Len > 4)
				u4RemapSize = 4;
			else
				u4RemapSize = u4Len;

			u4Len -= u4RemapSize;

			HAL_MCR_WR(prAdapter,
			(CONN_INFRA_CFG_AP2WF_BUS_ADDR + u4Offset),
			(uint32_t)
			(*(uint32_t *)((uint8_t *)pucStartPtr + u4Offset)));

			HAL_MCR_RD(prAdapter,
			(CONN_INFRA_CFG_AP2WF_BUS_ADDR + u4Offset), &u4Val);

			/* You can uncomment it to see what is downloaded
			*if (u4Idx++ < 20) {
			*DBGLOG(INIT, WARN, "[MJ]0x%08x(%08x) = 0x%08x\n",
			*(0x7C500000 + u4Offset), u4Val,
			*(uint32_t)(*(uint32_t *)
			*((uint8_t *)pucStartPtr + u4Offset)));
			*}
			*/
		}
#endif
#endif /* _HIF_PCIE */
	} else {
		return WLAN_STATUS_NOT_SUPPORTED;
	}

	return WLAN_STATUS_SUCCESS;
}
#endif
int wf_ioremap_read(size_t addr, unsigned int *val)
{
	void *vir_addr = NULL;

	vir_addr = ioremap_nocache(addr, 0x10);
	if (!vir_addr) {
		DBGLOG(INIT, ERROR, "%s: Cannot remap address.\n", __func__);
		return -1;
	}

	*val = readl(vir_addr);
	iounmap(vir_addr);
	DBGLOG(INIT, ERROR, "Read CONSYS 0x%08x=0x%08x.\n", addr, *val);

	return 0;
}
int wf_ioremap_write(phys_addr_t addr, unsigned int val)
{
	void *vir_addr = NULL;

	vir_addr = ioremap_nocache(addr, 0x10);
	if (!vir_addr) {
		DBGLOG(INIT, ERROR, "%s: Cannot remap address.\n", __func__);
		return -1;
	}

	writel(val, vir_addr);
	iounmap(vir_addr);
	DBGLOG(INIT, ERROR, "Write CONSYS 0x%08x=0x%08x.\n", addr, val);

	return 0;
}
int soc3_0_Trigger_fw_assert(void)
{
	int ret = 0;
	int value;

	wf_ioremap_read(WF_TRIGGER_AP2CONN_EINT, &value);
	value &= 0xFFFFFF7F;
	ret = wf_ioremap_write(WF_TRIGGER_AP2CONN_EINT, value);
	udelay(1000);
	wf_ioremap_read(WF_TRIGGER_AP2CONN_EINT, &value);
	value |= 0x80;
	ret = wf_ioremap_write(WF_TRIGGER_AP2CONN_EINT, value);
	return ret;
}
int wf_pwr_on_consys_mcu(void)
{
	int check;
	int value;
	int ret = 0;
	unsigned int polling_count;

	DBGLOG(INIT, ERROR, "wmmcu power-on start.\n");
	/* Wakeup conn_infra off write 0x180601A4[0] = 1'b1 */
	wf_ioremap_read(CONN_INFRA_WAKEUP_WF_ADDR, &value);
	value |= 0x00000001;
	wf_ioremap_write(CONN_INFRA_WAKEUP_WF_ADDR, value);

	/* Check AP2CONN slpprot ready
	 * (polling "10 times" and each polling interval is "1ms")
	 * Address: 0x1806_0184[5]
	 * Data: 1'b0
	 * Action: polling
	 */
	wf_ioremap_read(CONN_INFRA_ON2OFF_SLP_PROT_ACK_ADDR, &value);
	check = 0;
	polling_count = 0;
	while ((value & 0x00000020) != 0) {
		if (polling_count > 10) {
			check = -1;
			ret = -1;
			break;
		}
		udelay(1000);
		wf_ioremap_read(CONN_INFRA_ON2OFF_SLP_PROT_ACK_ADDR, &value);
		polling_count++;
	}
	if (check != 0) {
		DBGLOG(INIT, ERROR, "Polling  AP2CONN slpprot ready fail.\n");
		return ret;
	}

	/* Check CONNSYS version ID
	 * (polling "10 times" and each polling interval is "1ms")
	 * Address: 0x1800_1000[31:0]
	 * Data: 0x2001_0000
	 * Action: polling
	 */
	wf_ioremap_read(CONN_HW_VER_ADDR, &value);
	check = 0;
	polling_count = 0;
	while (value != CONNSYS_VERSION_ID) {
		if (polling_count > 10) {
			check = -1;
			ret = -1;
			break;
		}
		udelay(1000);
		wf_ioremap_read(CONN_HW_VER_ADDR, &value);
		polling_count++;
	}
	if (check != 0) {
		DBGLOG(INIT, ERROR, "Polling CONNSYS version ID fail.\n");
		return ret;
	}

	/* Assert CONNSYS WM CPU SW reset write 0x18000010[0] = 1'b0*/
	wf_ioremap_read(WFSYS_CPU_SW_RST_B_ADDR, &value);
	value &= 0xFFFFFFFE;
	wf_ioremap_write(WFSYS_CPU_SW_RST_B_ADDR, value);

	/* Turn on wfsys_top_on
	 * 0x18000000[31:16] = 0x57460000,
	 * 0x18000000[7] = 1'b1
	 */
	wf_ioremap_read(WFSYS_ON_TOP_PWR_CTL_ADDR, &value);
	value &= 0x0000FF7F;
	value |= 0x57460080;
	wf_ioremap_write(WFSYS_ON_TOP_PWR_CTL_ADDR, value);
	/* Polling wfsys_rgu_off_hreset_rst_b
	 * (polling "10 times" and each polling interval is "0.5ms")
	 * Address: 0x1806_02CC[2] (TOP_DBG_DUMMY_3_CONNSYS_PWR_STATUS[2])
	 * Data: 1'b1
	 * Action: polling
	 */

	wf_ioremap_read(TOP_DBG_DUMMY_3_CONNSYS_PWR_STATUS_ADDR, &value);
	check = 0;
	polling_count = 0;
	while ((value & 0x00000004) == 0) {
		if (polling_count > 10) {
			check = -1;
			ret = -1;
			break;
		}
		udelay(500);
		wf_ioremap_read(TOP_DBG_DUMMY_3_CONNSYS_PWR_STATUS_ADDR,
				&value);
		polling_count++;
	}
	if (check != 0) {
		DBGLOG(INIT, ERROR,
			"Polling wfsys rgu off fail. (0x%x)\n",
			value);
		return ret;
	}
	/* Turn off "conn_infra to wfsys"/ wfsys to conn_infra" bus
	 * sleep protect 0x218001620[0] = 1'b0
	 */
	wf_ioremap_read(CONN_INFRA_WF_SLP_CTRL_R_ADDR, &value);
	value &= 0xFFFFFFFE;
	wf_ioremap_write(CONN_INFRA_WF_SLP_CTRL_R_ADDR, value);
	/* Polling WF_SLP_CTRL ready
	 * (polling "10 times" and each polling interval is "0.5ms")
	 * Address: 0x1800_1620[3] (CONN_INFRA_WF_SLP_CTRL_R_OFFSET[3])
	 * Data: 1'b0
	 * Action: polling
	 */
	wf_ioremap_read(CONN_INFRA_WF_SLP_CTRL_R_ADDR, &value);
	check = 0;
	polling_count = 0;
	while ((value & 0x00000008) != 0) {
		if (polling_count > 10) {
			check = -1;
			ret = -1;
			break;
		}
		udelay(500);
		wf_ioremap_read(CONN_INFRA_WF_SLP_CTRL_R_ADDR, &value);
		polling_count++;
		DBGLOG(INIT, ERROR, "WF_SLP_CTRL (0x%x) (%d)\n",
			value,
			polling_count);
	}
	if (check != 0) {
		DBGLOG(INIT, ERROR,
			"Polling WFSYS TO CONNINFRA SLEEP PROTECT fail. (0x%x)\n",
			value);
		return ret;
	}
	/* Turn off "wf_dma to conn_infra" bus sleep protect
	 * 0x18001624[0] = 1'b0
	 */
	wf_ioremap_read(CONN_INFRA_WFDMA_SLP_CTRL_R_ADDR, &value);
	value &= 0xFFFFFFFE;
	wf_ioremap_write(CONN_INFRA_WFDMA_SLP_CTRL_R_ADDR, value);
	/* Polling wfsys_rgu_off_hreset_rst_b
	 * (polling "10 times" and each polling interval is "0.5ms")
	 * Address: 0x1800_1624[3] (CONN_INFRA_WFDMA_SLP_CTRL_R_OFFSET[3])
	 * Data: 1'b0
	 * Action: polling
	 */
	wf_ioremap_read(CONN_INFRA_WFDMA_SLP_CTRL_R_ADDR, &value);
	check = 0;
	polling_count = 0;
	while ((value & 0x00000008) != 0) {
		if (polling_count > 10) {
			check = -1;
			ret = -1;
			break;
		}
		udelay(500);
		wf_ioremap_read(CONN_INFRA_WFDMA_SLP_CTRL_R_ADDR, &value);
		polling_count++;
		DBGLOG(INIT, ERROR, "WFDMA_SLP_CTRL (0x%x) (%d)\n",
			value,
			polling_count);
	}
	if (check != 0) {
		DBGLOG(INIT, ERROR,
			"Polling WFDMA TO CONNINFRA SLEEP PROTECT RDY fail. (0x%x)\n",
			value);
		return ret;
	}

	/*WF_VDNR_EN_ADDR 0x1800_E06C[0] =1'b1*/
	wf_ioremap_read(WF_VDNR_EN_ADDR, &value);
	value |= 0x00000001;
	wf_ioremap_write(WF_VDNR_EN_ADDR, value);

	/* Check WFSYS version ID (Polling) */
	wf_ioremap_read(WFSYS_VERSION_ID_ADDR, &value);
	check = 0;
	polling_count = 0;
	while (value != CONNSYS_VERSION_ID) {
		if (polling_count > 10) {
			check = -1;
			ret = -1;
			break;
		}
		udelay(500);
		wf_ioremap_read(WFSYS_VERSION_ID_ADDR, &value);
		polling_count++;
	}
	if (check != 0) {
		DBGLOG(INIT, ERROR, "Polling CONNSYS version ID fail.\n");
		return ret;
	}

	/* Setup CONNSYS firmware in EMI */
#if (CFG_POWER_ON_DOWNLOAD_EMI_ROM_PATCH == 1)
	soc3_0_wlanPowerOnInit(ENUM_WLAN_POWER_ON_DOWNLOAD_EMI);
#endif

	/* Default value update 2: EMI entry address */
	wf_ioremap_write(CONN_CFG_AP2WF_REMAP_1_ADDR, CONN_MCU_CONFG_HS_BASE);
	wf_ioremap_write(WF_DYNAMIC_BASE+MCU_EMI_ENTRY_OFFSET, 0x00000000);
	wf_ioremap_write(WF_DYNAMIC_BASE+WF_EMI_ENTRY_OFFSET, 0x00000000);

	/* De-assert WFSYS CPU SW reset 0x18000010[0] = 1'b1 */
	wf_ioremap_read(WFSYS_CPU_SW_RST_B_ADDR, &value);
	value |= 0x00000001;
	wf_ioremap_write(WFSYS_CPU_SW_RST_B_ADDR, value);

	/* Check CONNSYS power-on completion
	 * Polling "10 times" and each polling interval is "0.5ms"
	 * Polling 0x81021604[31:0] = 0x00001D1E
	 */
	wf_ioremap_read(WF_ROM_CODE_INDEX_ADDR, &value);
	check = 0;
	polling_count = 0;
	while (value != CONNSYS_ROM_DONE_CHECK) {
		if (polling_count > 10) {
			check = -1;
			ret = -1;
			break;
		}
		udelay(1000);
		wf_ioremap_read(WF_ROM_CODE_INDEX_ADDR, &value);
		polling_count++;
	}
	if (check != 0) {
		DBGLOG(INIT, ERROR,
			"Check CONNSYS power-on completion fail.\n");
		return ret;
	}
	/* Disable conn_infra off domain force on 0x180601A4[0] = 1'b0 */
	wf_ioremap_read(CONN_INFRA_WAKEUP_WF_ADDR, &value);
	value &= 0xFFFFFFFE;
	wf_ioremap_write(CONN_INFRA_WAKEUP_WF_ADDR, value);
	DBGLOG(INIT, ERROR, "wmmcu power-on done.\n");
	return ret;
}

int wf_pwr_off_consys_mcu(void)
{
	int check;
	int value;
	int ret = 0;
	unsigned int polling_count;
	/* Turn on "conn_infra to wfsys"/ wfsys to conn_infra" bus sleep protect
	 * 0x18001620[0] = 1'b1
	 */
	wf_ioremap_read(CONN_INFRA_WF_SLP_CTRL_R_ADDR, &value);
	value |= 0x00000001;
	wf_ioremap_write(CONN_INFRA_WF_SLP_CTRL_R_ADDR, value);

	/* Polling WF_SLP_CTRL ready
	 * (polling "10 times" and each polling interval is "0.5ms")
	 * Address: 0x1800_1620[3] (CONN_INFRA_WF_SLP_CTRL_R_OFFSET[3])
	 * Data: 1'b1
	 * Action: polling
	 */
	wf_ioremap_read(CONN_INFRA_WF_SLP_CTRL_R_ADDR, &value);
	check = 0;
	polling_count = 0;
	while ((value & 0x00000008) == 0) {
		if (polling_count > 10) {
			check = -1;
			ret = -1;
			break;
		}
		udelay(500);
		wf_ioremap_read(CONN_INFRA_WF_SLP_CTRL_R_ADDR, &value);
		polling_count++;
		DBGLOG(INIT, ERROR, "WF_SLP_CTRL (0x%x) (%d)\n",
			value,
			polling_count);
	}
	if (check != 0) {
		DBGLOG(INIT, ERROR,
			"Polling WFSYS TO CONNINFRA SLEEP PROTECT fail. (0x%x)\n",
			value);
		return ret;
	}
	/* Turn off "wf_dma to conn_infra" bus sleep protect
	 * 0x18001624[0] = 1'b1
	 */
	wf_ioremap_read(CONN_INFRA_WFDMA_SLP_CTRL_R_ADDR, &value);
	value |= 0x00000001;
	wf_ioremap_write(CONN_INFRA_WFDMA_SLP_CTRL_R_ADDR, value);

	/* Polling wfsys_rgu_off_hreset_rst_b
	 * (polling "10 times" and each polling interval is "0.5ms")
	 * Address: 0x1800_1624[3] (CONN_INFRA_WFDMA_SLP_CTRL_R_OFFSET[3])
	 * Data: 1'b1
	 * Action: polling
	 */
	wf_ioremap_read(CONN_INFRA_WFDMA_SLP_CTRL_R_ADDR, &value);
	check = 0;
	polling_count = 0;
	while ((value & 0x00000008) == 0) {
		if (polling_count > 10) {
			check = -1;
			ret = -1;
			break;
		}
		udelay(500);
		wf_ioremap_read(CONN_INFRA_WFDMA_SLP_CTRL_R_ADDR, &value);
		polling_count++;
		DBGLOG(INIT, ERROR, "WFDMA_SLP_CTRL (0x%x) (%d)\n",
			value,
			polling_count);
	}
	if (check != 0) {
		DBGLOG(INIT, ERROR,
			"Polling WFDMA TO CONNINFRA SLEEP PROTECT RDY fail. (0x%x)\n",
			value);
		return ret;
	}

	/* Turn off wfsys_top_on
	 * 0x18000000[31:16] = 0x57460000,
	 * 0x18000000[7] = 1'b0
	 */
	wf_ioremap_read(WFSYS_ON_TOP_PWR_CTL_ADDR, &value);
	value &= 0x0000FF7F;
	value |= 0x57460000;
	wf_ioremap_write(WFSYS_ON_TOP_PWR_CTL_ADDR, value);
	/* Polling wfsys_rgu_off_hreset_rst_b
	 * (polling "10 times" and each polling interval is "0.5ms")
	 * Address: 0x1806_02CC[2] (TOP_DBG_DUMMY_3_CONNSYS_PWR_STATUS[2])
	 * Data: 1'b0
	 * Action: polling
	 */

	wf_ioremap_read(TOP_DBG_DUMMY_3_CONNSYS_PWR_STATUS_ADDR, &value);
	check = 0;
	polling_count = 0;
	while ((value & 0x00000004) != 0) {
		if (polling_count > 10) {
			check = -1;
			ret = -1;
			break;
		}
		udelay(500);
		wf_ioremap_read(TOP_DBG_DUMMY_3_CONNSYS_PWR_STATUS_ADDR,
				&value);
		polling_count++;
	}
	if (check != 0) {
		DBGLOG(INIT, ERROR,
			"Polling wfsys rgu off fail. (0x%x)\n",
			value);
		return ret;
	}
	return ret;
}

int hifWmmcuPwrOn(void)
{
	int ret = 0;
#if (CFG_SUPPORT_CONNINFRA == 1)
	/* conninfra power on */
	ret = conninfra_pwr_on(CONNDRV_TYPE_WIFI);
	if (ret != 0)
		return ret;
#endif
	/* wf driver power on */
	ret = wf_pwr_on_consys_mcu();
	if (ret != 0)
		return ret;

	/* Power on download ROM patch*/
#if (CFG_POWER_ON_DOWNLOAD_EMI_ROM_PATCH == 1)
	soc3_0_wlanPowerOnInit(ENUM_WLAN_POWER_ON_DOWNLOAD_ROM_PATCH);
#endif

	return ret;
}
int hifWmmcuPwrOff(void)
{
	int ret = 0;
	/* wf driver power off */
	ret = wf_pwr_off_consys_mcu();
	if (ret != 0)
		return ret;
#if (CFG_SUPPORT_CONNINFRA == 1)
	/*
	 * conninfra power off sequence
	 * conninfra will do conninfra power off self during whole chip reset.
	 */
	if (!kalIsWholeChipResetting()) {
		ret = conninfra_pwr_off(CONNDRV_TYPE_WIFI);
		if (ret != 0)
			return ret;
	}
#endif
	return ret;
}
#if (CFG_SUPPORT_CONNINFRA == 1)
int soc3_0_Trigger_whole_chip_rst(char *reason)
{
	return conninfra_trigger_whole_chip_rst(CONNDRV_TYPE_WIFI, reason);
}
void soc3_0_Sw_interrupt_handler(struct ADAPTER *prAdapter)
{
	int value;

	HAL_MCR_WR(prAdapter,
		   CONN_INFRA_CFG_AP2WF_REMAP_1_ADDR,
		   CONN_MCU_CONFG_HS_BASE);
	HAL_MCR_RD(prAdapter,
		  (CONN_INFRA_CFG_AP2WF_BUS_ADDR + 0xc0),
		  &value);

	DBGLOG(HAL, ERROR, "SW INT happended!!!!!(0x%x)\n", value);
	HAL_MCR_WR(prAdapter,
		  (CONN_INFRA_CFG_AP2WF_BUS_ADDR + 0xc8),
		  value);
}

void soc3_0_Conninfra_cb_register(void)
{
	g_conninfra_wf_cb.rst_cb.pre_whole_chip_rst =
					glRstwlanPreWholeChipReset;
	g_conninfra_wf_cb.rst_cb.post_whole_chip_rst =
					glRstwlanPostWholeChipReset;

#if (CFG_SUPPORT_PRE_ON_PHY_ACTION == 1)
	/* Register conninfra call back */
	g_conninfra_wf_cb.pre_cal_cb.pwr_on_cb = hifWmmcuPwrOn;

#if (CFG_POWER_ON_DOWNLOAD_EMI_ROM_PATCH == 1)
	g_conninfra_wf_cb.pre_cal_cb.do_cal_cb = soc3_0_wlanPreCal;
#endif /* (CFG_POWER_ON_DOWNLOAD_EMI_ROM_PATCH == 1) */

#endif /* (CFG_SUPPORT_PRE_ON_PHY_ACTION == 1) */

	conninfra_sub_drv_ops_register(CONNDRV_TYPE_WIFI,
		&g_conninfra_wf_cb);
}
#endif

#if (CFG_POWER_ON_DOWNLOAD_EMI_ROM_PATCH == 1)
#pragma message("SOC3_0::CFG_POWER_ON_DOWNLOAD_EMI_ROM_PATCH == 1")
void soc3_0_ConstructFirmwarePrio(struct GLUE_INFO *prGlueInfo,
	uint8_t **apucNameTable, uint8_t **apucName,
	uint8_t *pucNameIdx, uint8_t ucMaxNameIdx)
{
	uint8_t ucIdx = 0;

	for (ucIdx = 0; apucsoc3_0FwName[ucIdx]; ucIdx++) {
		if ((*pucNameIdx + 3) < ucMaxNameIdx) {
			/* Type 1. WIFI_RAM_CODE_soc1_0_1_1 */
			snprintf(*(apucName + (*pucNameIdx)),
					CFG_FW_NAME_MAX_LEN, "%s_%u_%u",
					apucsoc3_0FwName[ucIdx],
					CFG_WIFI_IP_SET,
					wlanGetEcoVersion(
						prGlueInfo->prAdapter));
			(*pucNameIdx) += 1;

			/* Type 2. WIFI_RAM_CODE_soc1_0_1_1.bin */
			snprintf(*(apucName + (*pucNameIdx)),
					CFG_FW_NAME_MAX_LEN, "%s_%u_%u.bin",
					apucsoc3_0FwName[ucIdx],
					CFG_WIFI_IP_SET,
					wlanGetEcoVersion(
						prGlueInfo->prAdapter));
			(*pucNameIdx) += 1;

			/* Type 3. WIFI_RAM_CODE_soc1_0 */
			snprintf(*(apucName + (*pucNameIdx)),
					CFG_FW_NAME_MAX_LEN, "%s",
					apucsoc3_0FwName[ucIdx]);
			(*pucNameIdx) += 1;

			/* Type 4. WIFI_RAM_CODE_soc1_0.bin */
			snprintf(*(apucName + (*pucNameIdx)),
					CFG_FW_NAME_MAX_LEN, "%s.bin",
					apucsoc3_0FwName[ucIdx]);
			(*pucNameIdx) += 1;
		} else {
			/* the table is not large enough */
			DBGLOG(INIT, ERROR,
				"kalFirmwareImageMapping >> file name array is not enough.\n");
			ASSERT(0);
		}
	}
}

void *
soc3_0_kalFirmwareImageMapping(
			IN struct GLUE_INFO *prGlueInfo,
			OUT void **ppvMapFileBuf,
			OUT uint32_t *pu4FileLength,
			IN enum ENUM_IMG_DL_IDX_T eDlIdx)
{
	uint8_t **apucNameTable = NULL;
	uint8_t *apucName[SOC3_0_FILE_NAME_TOTAL +
					  1]; /* extra +1, for the purpose of
					       * detecting the end of the array
					       */
	uint8_t idx = 0, max_idx,
		aucNameBody[SOC3_0_FILE_NAME_TOTAL][SOC3_0_FILE_NAME_MAX],
		sub_idx = 0;
	struct mt66xx_chip_info *prChipInfo =
			prGlueInfo->prAdapter->chip_info;
	/* uint32_t chip_id = prChipInfo->chip_id; */

	DEBUGFUNC("kalFirmwareImageMapping");

	ASSERT(prGlueInfo);
	ASSERT(ppvMapFileBuf);
	ASSERT(pu4FileLength);

	*ppvMapFileBuf = NULL;
	*pu4FileLength = 0;

	do {
		/* <0.0> Get FW name prefix table */
		switch (eDlIdx) {
		case IMG_DL_IDX_N9_FW:
			apucNameTable = soc3_0_apucFwName;
			break;

		case IMG_DL_IDX_CR4_FW:
			apucNameTable = soc3_0_apucCr4FwName;
			break;

		case IMG_DL_IDX_PATCH:
			break;

		case IMG_DL_IDX_MCU_ROM_EMI:
			break;

		case IMG_DL_IDX_WIFI_ROM_EMI:
			break;

		default:
			ASSERT(0);
			break;
		}

		/* <0.2> Construct FW name */
		memset(apucName, 0, sizeof(apucName));

		/* magic number 1: reservation for detection
		 * of the end of the array
		 */
		max_idx = (sizeof(apucName) / sizeof(uint8_t *)) - 1;

		idx = 0;
		apucName[idx] = (uint8_t *)(aucNameBody + idx);

		if (eDlIdx == IMG_DL_IDX_PATCH) {
			/* construct the file name for patch */
			/* soc3_0_patch_wmmcu_1_1_hdr.bin */
			if (prChipInfo->fw_dl_ops->constructPatchName)
				prChipInfo->fw_dl_ops->constructPatchName(
					prGlueInfo, apucName, &idx);
			else
				snprintf(apucName[idx], SOC3_0_FILE_NAME_MAX,
					"soc3_0_patch_wmmcu_1_%x_hdr.bin",
					wlanGetEcoVersion(
						prGlueInfo->prAdapter));
			idx += 1;
		} else if (eDlIdx == IMG_DL_IDX_MCU_ROM_EMI) {
			/* construct the file name for MCU ROM EMI */
			/* soc3_0_ram_wmmcu_1_1_hdr.bin */
			snprintf(apucName[idx], SOC3_0_FILE_NAME_MAX,
					"soc3_0_ram_wmmcu_1_%x_hdr.bin",
					wlanGetEcoVersion(
						prGlueInfo->prAdapter));

			idx += 1;
		} else if (eDlIdx == IMG_DL_IDX_WIFI_ROM_EMI) {
			/* construct the file name for WiFi ROM EMI */
			/* soc3_0_ram_wifi_1_1_hdr.bin */
			snprintf(apucName[idx], SOC3_0_FILE_NAME_MAX,
					"soc3_0_ram_wifi_1_%x_hdr.bin",
					wlanGetEcoVersion(
						prGlueInfo->prAdapter));

			idx += 1;
		} else {
			for (sub_idx = 0; sub_idx < max_idx; sub_idx++)
				apucName[sub_idx] =
					(uint8_t *)(aucNameBody + sub_idx);

			if (prChipInfo->fw_dl_ops->constructFirmwarePrio)
				prChipInfo->fw_dl_ops->constructFirmwarePrio(
					prGlueInfo, apucNameTable, apucName,
					&idx, max_idx);
			else
				kalConstructDefaultFirmwarePrio(
					prGlueInfo, apucNameTable, apucName,
					&idx, max_idx);
		}

		/* let the last pointer point to NULL
		 * so that we can detect the end of the array in
		 * kalFirmwareOpen().
		 */
		apucName[idx] = NULL;

		apucNameTable = apucName;

		/* <1> Open firmware */
		if (kalFirmwareOpen(prGlueInfo,
				    apucNameTable) != WLAN_STATUS_SUCCESS)
			break;
		{
			uint32_t u4FwSize = 0;
			void *prFwBuffer = NULL;
			/* <2> Query firmare size */
			kalFirmwareSize(prGlueInfo, &u4FwSize);
			/* <3> Use vmalloc for allocating large memory trunk */
			prFwBuffer = vmalloc(ALIGN_4(u4FwSize));
			/* <4> Load image binary into buffer */
			if (kalFirmwareLoad(prGlueInfo, prFwBuffer, 0,
					    &u4FwSize) != WLAN_STATUS_SUCCESS) {
				vfree(prFwBuffer);
				kalFirmwareClose(prGlueInfo);
				break;
			}
			/* <5> write back info */
			*pu4FileLength = u4FwSize;
			*ppvMapFileBuf = prFwBuffer;

			return prFwBuffer;
		}
	} while (FALSE);

	return NULL;
}

uint32_t soc3_0_wlanImageSectionDownloadStage(
	IN struct ADAPTER *prAdapter, IN void *pvFwImageMapFile,
	IN uint32_t u4FwImageFileLength, IN uint8_t ucSectionNumber,
	IN enum ENUM_IMG_DL_IDX_T eDlIdx)
{
	uint32_t u4SecIdx, u4Offset = 0;
	uint32_t u4Addr, u4Len, u4DataMode = 0;
	u_int8_t fgIsEMIDownload = FALSE;
	u_int8_t fgIsNotDownload = FALSE;
	uint32_t u4Status = WLAN_STATUS_SUCCESS;
	struct mt66xx_chip_info *prChipInfo = prAdapter->chip_info;
	struct patch_dl_target target;
	struct PATCH_FORMAT_T *prPatchHeader;
	struct ROM_EMI_HEADER *prRomEmiHeader;
	struct FWDL_OPS_T *prFwDlOps;

	prFwDlOps = prChipInfo->fw_dl_ops;

	/* 3a. parse file header for decision of
	 * divided firmware download or not
	 */
	if (eDlIdx == IMG_DL_IDX_PATCH) {
		prPatchHeader = pvFwImageMapFile;
		if (prPatchHeader->u4PatchVersion == PATCH_VERSION_MAGIC_NUM) {
			wlanImageSectionGetPatchInfoV2(prAdapter,
				pvFwImageMapFile,
				u4FwImageFileLength,
				&u4DataMode,
				&target);
			DBGLOG(INIT, INFO,
				"FormatV2 num_of_regoin[%d] datamode[0x%08x]\n",
				target.num_of_region, u4DataMode);
		} else {
			wlanImageSectionGetPatchInfo(prAdapter,
				pvFwImageMapFile,
					     u4FwImageFileLength,
					     &u4Offset, &u4Addr,
					     &u4Len, &u4DataMode);
			DBGLOG(INIT, INFO,
		"FormatV1 DL Offset[%u] addr[0x%08x] len[%u] datamode[0x%08x]\n",
		       u4Offset, u4Addr, u4Len, u4DataMode);
		}

		if (prPatchHeader->u4PatchVersion == PATCH_VERSION_MAGIC_NUM)
			u4Status = wlanDownloadSectionV2(prAdapter,
				u4DataMode, eDlIdx, &target);
		else
/* For dynamic memory map::Begin */
#if (CFG_DOWNLOAD_DYN_MEMORY_MAP == 1)
			u4Status = prFwDlOps->downloadByDynMemMap(
						prAdapter, u4Addr, u4Len,
						pvFwImageMapFile
							+ u4Offset,
							eDlIdx);
#else
			u4Status = wlanDownloadSection(
							prAdapter,
							u4Addr,
							u4Len,
							u4DataMode,
							pvFwImageMapFile
								+ u4Offset,
						       eDlIdx);
#endif
/* For dynamic memory map::End */
	} else if ((eDlIdx == IMG_DL_IDX_MCU_ROM_EMI) ||
				(eDlIdx == IMG_DL_IDX_WIFI_ROM_EMI)) {
		prRomEmiHeader = (struct ROM_EMI_HEADER *)pvFwImageMapFile;

		u4Addr = prRomEmiHeader->u4PatchAddr;

		u4Len = u4FwImageFileLength - sizeof(struct ROM_EMI_HEADER);

		u4Offset = sizeof(struct ROM_EMI_HEADER);

		u4Status = wlanDownloadEMISection(prAdapter,
					u4Addr, u4Len,
					pvFwImageMapFile + u4Offset);
	} else {
		for (u4SecIdx = 0; u4SecIdx < ucSectionNumber;
		     u4SecIdx++, u4Offset += u4Len) {
			prChipInfo->fw_dl_ops->getFwInfo(prAdapter, u4SecIdx,
				eDlIdx, &u4Addr,
				&u4Len, &u4DataMode, &fgIsEMIDownload,
				&fgIsNotDownload);

			DBGLOG(INIT, INFO,
			       "DL Offset[%u] addr[0x%08x] len[%u] datamode[0x%08x]\n",
			       u4Offset, u4Addr, u4Len, u4DataMode);

			if (fgIsNotDownload)
				continue;
			else if (fgIsEMIDownload)
				u4Status = wlanDownloadEMISection(prAdapter,
					u4Addr, u4Len,
					pvFwImageMapFile + u4Offset);
/* For dynamic memory map:: Begin */
#if (CFG_DOWNLOAD_DYN_MEMORY_MAP == 1)
			else if ((u4DataMode &
				DOWNLOAD_CONFIG_ENCRYPTION_MODE) == 0) {
				/* Non-encrypted F/W region,
				 * use dynamic memory mapping for download
				 */
				u4Status = prFwDlOps->downloadByDynMemMap(
					prAdapter,
					u4Addr,
					u4Len,
					pvFwImageMapFile + u4Offset,
					eDlIdx);
			}
#endif
/* For dynamic memory map:: End */
			else
				u4Status = wlanDownloadSection(prAdapter,
					u4Addr, u4Len,
					u4DataMode,
					pvFwImageMapFile + u4Offset, eDlIdx);

			/* escape from loop if any pending error occurs */
			if (u4Status == WLAN_STATUS_FAILURE)
				break;
		}
	}

	return u4Status;
}

/* WiFi power on download MCU/WiFi ROM EMI + ROM patch */
/*----------------------------------------------------------------------------*/
/*!
 * \brief Wlan power on download function. This function prepare the job
 *  during power on stage:
 *  [1]Download MCU + WiFi ROM EMI
 *  [2]Download ROM patch
 *
 * \retval 0 Success
 * \retval negative value Failed
 */
/*----------------------------------------------------------------------------*/
uint32_t soc3_0_wlanPowerOnDownload(
	IN struct ADAPTER *prAdapter,
	IN uint8_t ucDownloadItem)
{
	uint32_t u4FwSize = 0;
	void *prFwBuffer = NULL;
	uint32_t u4Status;
#if CFG_SUPPORT_COMPRESSION_FW_OPTION
	uint8_t ucIsCompressed;
#endif
	if (!prAdapter)
		return WLAN_STATUS_FAILURE;


	DBGLOG(INIT, INFO, "Power on download start(%d)\n", ucDownloadItem);

	switch (ucDownloadItem) {
	case ENUM_WLAN_POWER_ON_DOWNLOAD_EMI:
		/* Download MCU ROM EMI*/
		soc3_0_kalFirmwareImageMapping(prAdapter->prGlueInfo,
			&prFwBuffer, &u4FwSize, IMG_DL_IDX_MCU_ROM_EMI);

		u4Status = soc3_0_wlanImageSectionDownloadStage(
			prAdapter, prFwBuffer, u4FwSize, 1,
			IMG_DL_IDX_MCU_ROM_EMI);

		kalFirmwareImageUnmapping(
			prAdapter->prGlueInfo, NULL, prFwBuffer);

		DBGLOG(INIT, INFO, "Power on download mcu ROM EMI %s\n",
			(u4Status == WLAN_STATUS_SUCCESS) ? "pass" : "failed");

		/* Download WiFi ROM EMI*/
		if (u4Status == WLAN_STATUS_SUCCESS) {
			soc3_0_kalFirmwareImageMapping(prAdapter->prGlueInfo,
				&prFwBuffer, &u4FwSize,
				IMG_DL_IDX_WIFI_ROM_EMI);

			u4Status = soc3_0_wlanImageSectionDownloadStage(
				prAdapter, prFwBuffer, u4FwSize, 1,
				IMG_DL_IDX_WIFI_ROM_EMI);

			kalFirmwareImageUnmapping(
				prAdapter->prGlueInfo, NULL, prFwBuffer);

			DBGLOG(INIT, INFO,
				"Power on download WiFi ROM EMI %s\n",
				(u4Status == WLAN_STATUS_SUCCESS)
					? "pass" : "failed");
		}

		break;

	case ENUM_WLAN_POWER_ON_DOWNLOAD_ROM_PATCH:
		prAdapter->rVerInfo.fgPatchIsDlByDrv = FALSE;

		soc3_0_kalFirmwareImageMapping(prAdapter->prGlueInfo,
			&prFwBuffer, &u4FwSize, IMG_DL_IDX_PATCH);

		if (prFwBuffer == NULL) {
			DBGLOG(INIT, WARN, "FW[%u] load error!\n",
			       IMG_DL_IDX_PATCH);
			return WLAN_STATUS_FAILURE;
		}

#if (CFG_ROM_PATCH_NO_SEM_CTRL == 0)
#pragma message("ROM code supports SEM-CTRL for ROM patch download")
		if (wlanPatchIsDownloaded(prAdapter)) {
			kalFirmwareImageUnmapping(prAdapter->prGlueInfo, NULL,
						  prFwBuffer);
			DBGLOG(INIT, INFO, "No need to download patch\n");
			return WLAN_STATUS_SUCCESS;
		}
#else
#pragma message("ROM code supports no SEM-CTRL for ROM patch download")
#endif

		/* Patch DL */
#if CFG_SUPPORT_COMPRESSION_FW_OPTION
		u4Status = wlanCompressedImageSectionDownloadStage(
			prAdapter, prFwBuffer, u4FwSize, 1,
			IMG_DL_IDX_PATCH, &ucIsCompressed, NULL);
#else
		u4Status = soc3_0_wlanImageSectionDownloadStage(
			prAdapter, prFwBuffer, u4FwSize, 1, IMG_DL_IDX_PATCH);
#endif

/* Dynamic memory map::Begin */
#if (CFG_DOWNLOAD_DYN_MEMORY_MAP == 1)
		if (u4Status == WLAN_STATUS_SUCCESS)
			wlanPatchDynMemMapSendComplete(prAdapter);
		else if (u4Status == WLAN_STATUS_NOT_ACCEPTED)
			u4Status = WLAN_STATUS_SUCCESS; /* already download*/
#else
		wlanPatchSendComplete(prAdapter);
#endif
/* Dynamic memory map::End */

		kalFirmwareImageUnmapping(
			prAdapter->prGlueInfo, NULL, prFwBuffer);

		prAdapter->rVerInfo.fgPatchIsDlByDrv = TRUE;

		break;

	default:
		return WLAN_STATUS_NOT_SUPPORTED;
	}

	DBGLOG(INIT, INFO, "Power on download end[%d].\n", u4Status);

	return u4Status;
}

/* WiFi power on init for MCU/WiFi ROM EMI + ROM patch download */
/*----------------------------------------------------------------------------*/
/*!
 * \brief Wlan power on init function. This function do the job in the
 *  power on stage, they are:
 *  [1]Download MCU + WiFi ROM EMI
 *  [2]Download ROM patch
 *
 *  It is to simulate wlanProbe() with the minimum effort to complete
 *  ROM EMI + ROM patch download.
 *
 * \retval 0 Success
 * \retval negative value Failed
 */
/*----------------------------------------------------------------------------*/
int32_t soc3_0_wlanPowerOnInit(
	enum ENUM_WLAN_POWER_ON_DOWNLOAD eDownloadItem)
{
/*
*	pvData     data passed by bus driver init function
*		_HIF_EHPI: NULL
*		_HIF_SDIO: sdio bus driver handle
*	see hifAxiProbe() for more detail...
*	pfWlanProbe((void *)prPlatDev,
*			(void *)prPlatDev->id_entry->driver_data)
*/
#if defined(SOC3_0)
#if defined(_HIF_AXI)
/* prPlatDev is already created by initWlan()->glRegisterBus()::axi.c */
	void *pvData = (void *)g_prPlatDev;
	void *pvDriverData = (void *)g_prPlatDev->id_entry->driver_data;
#endif

#if defined(_HIF_PCIE)
	void *pvData = NULL;
	void *pvDriverData = (void *)&mt66xx_driver_data_soc3_0;
#endif
#endif

	int32_t i4Status = 0;
	enum ENUM_POWER_ON_INIT_FAIL_REASON {
		BUS_INIT_FAIL = 0,
		NET_CREATE_FAIL,
		BUS_SET_IRQ_FAIL,
		ALLOC_ADAPTER_MEM_FAIL,
		DRIVER_OWN_FAIL,
		INIT_ADAPTER_FAIL,
		INIT_HIFINFO_FAIL,
		ROM_PATCH_DOWNLOAD_FAIL,
		POWER_ON_INIT_DONE,
		FAIL_REASON_NUM
	} eFailReason;
	uint32_t i = 0;
	int32_t i4DevIdx = 0;
	struct wireless_dev *prWdev = NULL;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct ADAPTER *prAdapter = NULL;
	struct mt66xx_chip_info *prChipInfo;
	struct FWDL_OPS_T *prFwDlOps;

	DBGLOG(INIT, INFO, "wlanPowerOnInit::begin\n");

	eFailReason = FAIL_REASON_NUM;

	do {
#if defined(_HIF_AXI)
		/* AXI goes over here, to be conti... */
		prChipInfo = ((struct mt66xx_hif_driver_data *)pvDriverData)
					->chip_info;

		pvData = (void *)prChipInfo->pdev;

		/* we should call axi_enable_device(
		*        (struct platform_device *)pvData);
		* since it is invoked from within hifAxiProbe()::axi.c,
		* before calling pfWlanProbe(), to be conti...
		*/
#endif

#if defined(_HIF_PCIE)
		prChipInfo = ((struct mt66xx_hif_driver_data *)pvDriverData)
					->chip_info;

		pvData = (void *)prChipInfo->pdev;

		/* no need pci_enable_device(dev),
		 * it has already been done in PCI driver's probe() function
		 */
#endif

		if (eDownloadItem == ENUM_WLAN_POWER_ON_DOWNLOAD_EMI) {
			prWdev = wlanNetCreate(pvData, pvDriverData);

			if (prWdev == NULL) {
				DBGLOG(INIT, ERROR,
				"[Wi-Fi PWR On] No memory for dev and its private\n");

				i4Status = -NET_CREATE_FAIL;
			} else {
				/* Set the ioaddr to HIF Info */
				prGlueInfo = (struct GLUE_INFO *)
					wiphy_priv(prWdev->wiphy);

				prAdapter = prGlueInfo->prAdapter;

				if (prChipInfo->pwrondownload) {
					DBGLOG_LIMITED(INIT, INFO,
					"[Wi-Fi PWR On] EMI download Start\n");

					if (prChipInfo->pwrondownload(
						prAdapter, eDownloadItem)
						!= WLAN_STATUS_SUCCESS) {
						i4Status =
						-ROM_PATCH_DOWNLOAD_FAIL;
					}

					DBGLOG_LIMITED(INIT, INFO,
					"[Wi-Fi PWR On] EMI download End\n");
				}
			}

			wlanWakeLockUninit(prGlueInfo);

			wlanNetDestroy(prWdev);

			return i4Status;
		}

		/* [1]Copy from wlanProbe()::Begin */
		/* Initialize the IO port of the interface */
		/* GeorgeKuo: pData has different meaning for _HIF_XXX:
		* _HIF_EHPI: pointer to memory base variable, which will be
		*      initialized by glBusInit().
		* _HIF_SDIO: bus driver handle
		*/

		/* Remember to call glBusRelease() in wlanPowerOnDeinit() */
		if (glBusInit(pvData) == FALSE) {
			DBGLOG(INIT, ERROR,
				"[Wi-Fi PWR On] glBusInit() fail\n");

			i4Status = -EIO;

			eFailReason = BUS_INIT_FAIL;

			break;
		}

#if (CFG_SUPPORT_TRACE_TC4 == 1)
		wlanDebugTC4Init();
#endif

		/* Create network device, Adapter, KalInfo,
		*       prDevHandler(netdev)
		*/
		prWdev = wlanNetCreate(pvData, pvDriverData);

		if (prWdev == NULL) {
			DBGLOG(INIT, ERROR,
				"[Wi-Fi PWR On] No memory for dev and its private\n");

			i4Status = -ENOMEM;

			eFailReason = NET_CREATE_FAIL;

			break;
		}

		/* Set the ioaddr to HIF Info */
		prGlueInfo = (struct GLUE_INFO *) wiphy_priv(prWdev->wiphy);

		/* Should we need this??? to be conti... */
		gPrDev = prGlueInfo->prDevHandler;

		/* Setup IRQ */
		i4Status = glBusSetIrq(prWdev->netdev, NULL, prGlueInfo);

		if (i4Status != WLAN_STATUS_SUCCESS) {
			DBGLOG(INIT, ERROR, "[Wi-Fi PWR On] Set IRQ error\n");

			eFailReason = BUS_SET_IRQ_FAIL;

			break;
		}

		prGlueInfo->i4DevIdx = i4DevIdx;

		prAdapter = prGlueInfo->prAdapter;
		/* [1]Copy from wlanProbe()::End */

		/* [2]Copy from wlanProbe()->wlanOnPreAdapterStart()::Begin */
		/* Init Chip Capability */
		prChipInfo = prAdapter->chip_info;

		if (prChipInfo->asicCapInit)
			prChipInfo->asicCapInit(prAdapter);

		/* Trigger the action of switching Pwr state to drv_own */
		prAdapter->fgIsFwOwn = TRUE;

		nicpmWakeUpWiFi(prAdapter);
		/* [2]Copy from wlanProbe()->wlanOnPreAdapterStart()::End */

		/* [3]Copy from
		* wlanProbe()
		*	->wlanAdapterStart()
		*		->wlanOnPreAllocAdapterMem()::Begin
		*/
#if 0  /* Sample's gen4m code base doesn't support */
		prAdapter->u4HifDbgFlag = 0;
		prAdapter->u4HifChkFlag = 0;
		prAdapter->u4TxHangFlag = 0;
		prAdapter->u4NoMoreRfb = 0;
#endif

		prAdapter->u4OwnFailedCount = 0;
		prAdapter->u4OwnFailedLogCount = 0;

#if 0  /* Sample's gen4m code base doesn't support */
		prAdapter->fgEnHifDbgInfo = TRUE;
#endif

		/* Additional with chip reset optimize*/
		prAdapter->ucCmdSeqNum = 0;
		prAdapter->u4PwrCtrlBlockCnt = 0;

		QUEUE_INITIALIZE(&(prAdapter->rPendingCmdQueue));
#if CFG_SUPPORT_MULTITHREAD
		QUEUE_INITIALIZE(&prAdapter->rTxCmdQueue);
		QUEUE_INITIALIZE(&prAdapter->rTxCmdDoneQueue);
#if CFG_FIX_2_TX_PORT
		QUEUE_INITIALIZE(&prAdapter->rTxP0Queue);
		QUEUE_INITIALIZE(&prAdapter->rTxP1Queue);
#else
		for (i = 0; i < TX_PORT_NUM; i++)
			QUEUE_INITIALIZE(&prAdapter->rTxPQueue[i]);
#endif
		QUEUE_INITIALIZE(&prAdapter->rRxQueue);
		QUEUE_INITIALIZE(&prAdapter->rTxDataDoneQueue);
#endif

		/* reset fgIsBusAccessFailed */
		fgIsBusAccessFailed = FALSE;

		/* Allocate mandatory resource for TX/RX */
		if (nicAllocateAdapterMemory(prAdapter)
			!= WLAN_STATUS_SUCCESS) {
			DBGLOG(INIT, ERROR,
				"[Wi-Fi PWR On] nicAllocateAdapterMemory Error!\n");

			i4Status = -ENOMEM;

			eFailReason = ALLOC_ADAPTER_MEM_FAIL;
/*
*#if CFG_ENABLE_KEYWORD_EXCEPTION_MECHANISM & 0
*			mtk_wcn_wmt_assert_keyword(WMTDRV_TYPE_WIFI,
*			"[Wi-Fi PWR On] nicAllocateAdapterMemory Error!");
*#endif
*/
			break;
		}

		/* should we need this?  to be conti... */
		prAdapter->u4OsPacketFilter = PARAM_PACKET_FILTER_SUPPORTED;

		/* WLAN driver acquire LP own */
		DBGLOG(INIT, TRACE, "[Wi-Fi PWR On] Acquiring LP-OWN\n");

		ACQUIRE_POWER_CONTROL_FROM_PM(prAdapter);

		DBGLOG(INIT, TRACE, "[Wi-Fi PWR On] Acquiring LP-OWN-end\n");

		if (prAdapter->fgIsFwOwn == TRUE) {
			DBGLOG(INIT, ERROR,
				"[Wi-Fi PWR On] nicpmSetDriverOwn() failed!\n");

			eFailReason = DRIVER_OWN_FAIL;
/*
*#if CFG_ENABLE_KEYWORD_EXCEPTION_MECHANISM & 0
*			mtk_wcn_wmt_assert_keyword(WMTDRV_TYPE_WIFI,
*				"[Wi-Fi PWR On] nicpmSetDriverOwn() failed!");
*#endif
*/
			break;
		}

		/* Initialize the Adapter:
		*       verify chipset ID, HIF init...
		*       the code snippet just do the copy thing
		*/
		if (nicInitializeAdapter(prAdapter) != WLAN_STATUS_SUCCESS) {
			DBGLOG(INIT, ERROR,
				"[Wi-Fi PWR On] nicInitializeAdapter failed!\n");

			eFailReason = INIT_ADAPTER_FAIL;

			break;
		}

		/* Do the post NIC init adapter:
		* copy only the mandatory task
		* in wlanOnPostNicInitAdapter(prAdapter, FALSE)::Begin
		*/
		nicInitSystemService(prAdapter, FALSE);

		/* Initialize Tx */
		nicTxInitialize(prAdapter);

		/* Initialize Rx */
		nicRxInitialize(prAdapter);
		/* Do the post NIC init adapter:
		* copy only the mandatory task
		* in wlanOnPostNicInitAdapter(prAdapter, FALSE)::End
		*/

		/* HIF SW info initialize */
		if (!halHifSwInfoInit(prAdapter)) {
			DBGLOG(INIT, ERROR,
				"[Wi-Fi PWR On] halHifSwInfoInit failed!\n");

			eFailReason = INIT_HIFINFO_FAIL;

			break;
		}

		/* Enable HIF  cut-through to N9 mode */
		HAL_ENABLE_FWDL(prAdapter, TRUE);

		wlanSetChipEcoInfo(prAdapter);

		/* should we open it, to be conti */
		/* wlanOnPostInitHifInfo(prAdapter); */

		/* Disable interrupt, download is done by polling mode only */
		nicDisableInterrupt(prAdapter);

		/* Initialize Tx Resource to fw download state */
		nicTxInitResetResource(prAdapter);

		/* MCU ROM EMI +
		* WiFi ROM EMI + ROM patch download goes over here::Begin
		*/
		/* assiggned in wlanNetCreate() */
		prChipInfo = prAdapter->chip_info;

		/* It is configured in mt66xx_chip_info_connac2x2.fw_dl_ops */
		prFwDlOps = prChipInfo->fw_dl_ops;

		/* No need to check F/W ready bit,
		* since we are downloading MCU ROM EMI
		* + WiFi ROM EMI + ROM patch
		*/
		/*
		*DBGLOG(INIT, INFO,
		*	"wlanDownloadFW:: Check ready_bits(=0x%x)\n",
		*	prChipInfo->sw_ready_bits);
		*
		*HAL_WIFI_FUNC_READY_CHECK(prAdapter,
		*	prChipInfo->sw_ready_bits, &fgReady);
		*/

		if (prChipInfo->pwrondownload) {
			HAL_ENABLE_FWDL(prAdapter, TRUE);

			DBGLOG_LIMITED(INIT, INFO,
				"[Wi-Fi PWR On] download Start\n");

			if (prChipInfo->pwrondownload(prAdapter, eDownloadItem)
				!= WLAN_STATUS_SUCCESS) {
				eFailReason = ROM_PATCH_DOWNLOAD_FAIL;

				HAL_ENABLE_FWDL(prAdapter, FALSE);

				break;
			}

			DBGLOG_LIMITED(INIT, INFO,
				"[Wi-Fi PWR On] download End\n");

			HAL_ENABLE_FWDL(prAdapter, FALSE);
		}
		/* MCU ROM EMI + WiFi ROM EMI
		* + ROM patch download goes over here::End
		*/

		eFailReason = POWER_ON_INIT_DONE;
		/* [3]Copy from wlanProbe()
		*		->wlanAdapterStart()
		*			->wlanOnPreAllocAdapterMem()::End
		*/
	} while (FALSE);

	switch (eFailReason) {
	case BUS_INIT_FAIL:
		break;

	case NET_CREATE_FAIL:
#if (CFG_SUPPORT_TRACE_TC4 == 1)
		wlanDebugTC4Uninit();  /* Uninit for TC4 debug */
#endif

		/* We should call this, although nothing is inside */
		glBusRelease(pvData);

		break;

	case BUS_SET_IRQ_FAIL:
#if (CFG_SUPPORT_TRACE_TC4 == 1)
		wlanDebugTC4Uninit();  /* Uninit for TC4 debug */
#endif

		/* We should call this, although nothing is inside */
		glBusRelease(pvData);

		wlanWakeLockUninit(prGlueInfo);

		wlanNetDestroy(prWdev);

		break;

	case ALLOC_ADAPTER_MEM_FAIL:
	case DRIVER_OWN_FAIL:
	case INIT_ADAPTER_FAIL:
		/* Should we set Onwership to F/W for advanced debug???
		* to be conti...
		*/
		/* nicpmSetFWOwn(prAdapter, FALSE); */

		glBusFreeIrq(prWdev->netdev,
			*((struct GLUE_INFO **)netdev_priv(prWdev->netdev)));

#if (CFG_SUPPORT_TRACE_TC4 == 1)
		wlanDebugTC4Uninit();  /* Uninit for TC4 debug */
#endif

		/* We should call this, although nothing is inside */
		glBusRelease(pvData);

		wlanWakeLockUninit(prGlueInfo);

		wlanNetDestroy(prWdev);

		break;

	case INIT_HIFINFO_FAIL:
		nicRxUninitialize(prAdapter);

		nicTxRelease(prAdapter, FALSE);

		/* System Service Uninitialization */
		nicUninitSystemService(prAdapter);

		/* Should we set Onwership to F/W for advanced debug???
		* to be conti...
		*/
		/* nicpmSetFWOwn(prAdapter, FALSE); */

		glBusFreeIrq(prWdev->netdev,
			*((struct GLUE_INFO **)netdev_priv(prWdev->netdev)));

#if (CFG_SUPPORT_TRACE_TC4 == 1)
		wlanDebugTC4Uninit();  /* Uninit for TC4 debug */
#endif

		/* We should call this, although nothing is inside */
		glBusRelease(pvData);

		wlanWakeLockUninit(prGlueInfo);

		wlanNetDestroy(prWdev);

		break;

	case ROM_PATCH_DOWNLOAD_FAIL:
	case POWER_ON_INIT_DONE:
		HAL_ENABLE_FWDL(prAdapter, FALSE);

		nicRxUninitialize(prAdapter);

		nicTxRelease(prAdapter, FALSE);

		/* System Service Uninitialization */
		nicUninitSystemService(prAdapter);

		/* Should we set Onwership to F/W for advanced debug???
		* to be conti...
		*/
		/* nicpmSetFWOwn(prAdapter, FALSE); */

		glBusFreeIrq(prWdev->netdev,
			*((struct GLUE_INFO **)netdev_priv(prWdev->netdev)));

#if (CFG_SUPPORT_TRACE_TC4 == 1)
		wlanDebugTC4Uninit();  /* Uninit for TC4 debug */
#endif

		/* We should call this, although nothing is inside */
		glBusRelease(pvData);

		wlanWakeLockUninit(prGlueInfo);

		wlanNetDestroy(prWdev);

		break;

	default:
		break;
	}

	DBGLOG(INIT, INFO, "wlanPowerOnInit::end\n");

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Wlan power on deinit function. This function revert whatever
 * has been altered in the
 * power on stage to restore to the most original state.
 *
 * \param[in] void
 *
 * \return (none)
 */
/*----------------------------------------------------------------------------*/
/*
*static void wlanPowerOnDeinit(void)
*{
*
*}
*/

#if (CFG_SUPPORT_PRE_ON_PHY_ACTION == 1)
int soc3_0_wlanPreCal(void)
{
#if defined(SOC3_0)
#if defined(_HIF_AXI)
/* prPlatDev is already created by initWlan()->glRegisterBus()::axi.c */
	void *pvData = (void *)g_prPlatDev;
	void *pvDriverData = (void *)g_prPlatDev->id_entry->driver_data;
#endif

#if defined(_HIF_PCIE)
	void *pvData = NULL;
	void *pvDriverData = (void *)&mt66xx_driver_data_soc3_0;
#endif
#endif

	int32_t i4Status = 0;
	enum ENUM_POWER_ON_INIT_FAIL_REASON {
					BUS_INIT_FAIL = 0,
					NET_CREATE_FAIL,
					BUS_SET_IRQ_FAIL,
					ALLOC_ADAPTER_MEM_FAIL,
					DRIVER_OWN_FAIL,
					INIT_ADAPTER_FAIL,
					INIT_HIFINFO_FAIL,
					ROM_PATCH_DOWNLOAD_FAIL,
					POWER_ON_INIT_DONE,
					FAIL_REASON_NUM
	} eFailReason;
	uint32_t i = 0;
	int32_t i4DevIdx = 0;
	struct wireless_dev *prWdev = NULL;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct ADAPTER *prAdapter = NULL;
	struct mt66xx_chip_info *prChipInfo;

	DBGLOG(INIT, INFO, "soc3_0_wlanPreCal::begin\n");

	eFailReason = FAIL_REASON_NUM;

	do {
#if defined(_HIF_AXI)
		/* AXI goes over here, to be conti... */
		prChipInfo = ((struct mt66xx_hif_driver_data *)
			pvDriverData)->chip_info;
		pvData = (void *)prChipInfo->pdev;

		/* we should call axi_enable_device(
		*        (struct platform_device *)pvData);
		* since it is invoked from within hifAxiProbe()::axi.c,
		* before calling pfWlanProbe(), to be conti...
		*/
#endif

#if defined(_HIF_PCIE)
		prChipInfo = ((struct mt66xx_hif_driver_data *)
			pvDriverData)->chip_info;
		pvData = (void *)prChipInfo->pdev;

		/* no need pci_enable_device(dev),
		* it has already been done in PCI driver's probe() function
		*/
#endif

		/* [1]Copy from wlanProbe()::Begin */
		/* Initialize the IO port of the interface */
		/* GeorgeKuo: pData has different meaning for _HIF_XXX:
		* _HIF_EHPI: pointer to memory base variable, which will be
		*      initialized by glBusInit().
		* _HIF_SDIO: bus driver handle
		*/

#if (CFG_SUPPORT_TRACE_TC4 == 1)
	    wlanDebugTC4Init();
#endif

		/* Create network device, Adapter, KalInfo,
		*       prDevHandler(netdev)
		*/
		prWdev = wlanNetCreate(pvData, pvDriverData);

		if (prWdev == NULL) {
			DBGLOG(INIT, ERROR,
				"[Wi-Fi Pre-Cal] No memory for dev and its private\n");

			i4Status = -ENOMEM;
			eFailReason = NET_CREATE_FAIL;
			break;
		}

	    /* Set the ioaddr to HIF Info */
	    prGlueInfo = (struct GLUE_INFO *) wiphy_priv(prWdev->wiphy);

	    /* Should we need this??? to be conti... */
	    gPrDev = prGlueInfo->prDevHandler;

	    /* Setup IRQ */
	    i4Status = glBusSetIrq(prWdev->netdev, NULL, prGlueInfo);

		if (i4Status != WLAN_STATUS_SUCCESS) {
			DBGLOG(INIT, ERROR, "[Wi-Fi Pre-Cal] Set IRQ error\n");

			eFailReason = BUS_SET_IRQ_FAIL;
			break;
		}

	    prGlueInfo->i4DevIdx = i4DevIdx;

	    prAdapter = prGlueInfo->prAdapter;
	    /* [1]Copy from wlanProbe()::End */

	    /* [2]Copy from wlanProbe()->wlanOnPreAdapterStart()::Begin */
	    /* Init Chip Capability */
	    prChipInfo = prAdapter->chip_info;

		if (prChipInfo->asicCapInit != NULL)
			prChipInfo->asicCapInit(prAdapter);

	    nicpmWakeUpWiFi(prAdapter);
	    /* [2]Copy from wlanProbe()->wlanOnPreAdapterStart()::End */

		/* [3]Copy from
		* wlanProbe()
		*	->wlanAdapterStart()
		*		->wlanOnPreAllocAdapterMem()::Begin
		*/

		prAdapter->u4OwnFailedCount = 0;
		prAdapter->u4OwnFailedLogCount = 0;

		/* Additional with chip reset optimize*/
		prAdapter->ucCmdSeqNum = 0;

		QUEUE_INITIALIZE(&(prAdapter->rPendingCmdQueue));
#if CFG_SUPPORT_MULTITHREAD
		QUEUE_INITIALIZE(&prAdapter->rTxCmdQueue);
		QUEUE_INITIALIZE(&prAdapter->rTxCmdDoneQueue);
#if CFG_FIX_2_TX_PORT
	    QUEUE_INITIALIZE(&prAdapter->rTxP0Queue);
	    QUEUE_INITIALIZE(&prAdapter->rTxP1Queue);
#else
		for (i = 0; i < TX_PORT_NUM; i++)
			QUEUE_INITIALIZE(&prAdapter->rTxPQueue[i]);
#endif
		QUEUE_INITIALIZE(&prAdapter->rRxQueue);
		QUEUE_INITIALIZE(&prAdapter->rTxDataDoneQueue);
#endif

		/* reset fgIsBusAccessFailed */
		fgIsBusAccessFailed = FALSE;

		/* Allocate mandatory resource for TX/RX */
		if (nicAllocateAdapterMemory(prAdapter) !=
			WLAN_STATUS_SUCCESS) {

			DBGLOG(INIT, ERROR,
				"[Wi-Fi Pre-Cal] nicAllocateAdapterMemory Error!\n");

			i4Status = -ENOMEM;
			eFailReason = ALLOC_ADAPTER_MEM_FAIL;
/*
*#if CFG_ENABLE_KEYWORD_EXCEPTION_MECHANISM & 0
*	mtk_wcn_wmt_assert_keyword(WMTDRV_TYPE_WIFI,
*	"[Wi-Fi PWR On] nicAllocateAdapterMemory Error!");
*#endif
*/
			break;
		}

		/* should we need this?  to be conti... */
		prAdapter->u4OsPacketFilter = PARAM_PACKET_FILTER_SUPPORTED;

		/* Initialize the Adapter:
		*       verify chipset ID, HIF init...
		*       the code snippet just do the copy thing
		*/
		if (nicInitializeAdapter(prAdapter) != WLAN_STATUS_SUCCESS) {

			DBGLOG(INIT, ERROR,
				"[Wi-Fi Pre-Cal] nicInitializeAdapter failed!\n");

			eFailReason = INIT_ADAPTER_FAIL;
			break;
		}

		/* Do the post NIC init adapter:
		* copy only the mandatory task
		* in wlanOnPostNicInitAdapter(prAdapter, FALSE)::Begin
		*/
		nicInitSystemService(prAdapter, FALSE);

		/* Initialize Tx */
		nicTxInitialize(prAdapter);

		/* Initialize Rx */
		nicRxInitialize(prAdapter);
		/* Do the post NIC init adapter:
		* copy only the mandatory task
		* in wlanOnPostNicInitAdapter(prAdapter, FALSE)::End
		*/

		/* HIF SW info initialize */
		if (!halHifSwInfoInit(prAdapter)) {

			DBGLOG(INIT, ERROR,
				"[Wi-Fi Pre-Cal] halHifSwInfoInit failed!\n");

			eFailReason = INIT_HIFINFO_FAIL;
			break;
		}

		/* Enable HIF  cut-through to N9 mode */
		HAL_ENABLE_FWDL(prAdapter, TRUE);

		/* Disable interrupt, download is done by polling mode only */
		nicDisableInterrupt(prAdapter);

		/* Initialize Tx Resource to fw download state */
		nicTxInitResetResource(prAdapter);

		/* MCU ROM EMI +
		* WiFi ROM EMI + ROM patch download goes over here::Begin
		*/
		/* assiggned in wlanNetCreate() */
		prChipInfo = prAdapter->chip_info;

		/* No need to check F/W ready bit,
		* since we are downloading MCU ROM EMI
		* + WiFi ROM EMI + ROM patch
		*/
		/*
		*DBGLOG(INIT, INFO,
		*             "wlanDownloadFW:: Check ready_bits(=0x%x)\n",
		*             prChipInfo->sw_ready_bits);
		*
		*HAL_WIFI_FUNC_READY_CHECK(prAdapter,
		*             prChipInfo->sw_ready_bits, &fgReady);
		*/

		wlanSendPhyAction(prAdapter, HAL_PHY_ACTION_CAL_FORCE_CAL_REQ);

		/* MCU ROM EMI + WiFi ROM EMI
		* + ROM patch download goes over here::End
		*/

		eFailReason = POWER_ON_INIT_DONE;
		/* [3]Copy from wlanProbe()
		*	->wlanAdapterStart()
		*	->wlanOnPreAllocAdapterMem()::End
		*/
	} while (FALSE);

	switch (eFailReason) {
	case BUS_INIT_FAIL:
	break;

	case NET_CREATE_FAIL:
#if (CFG_SUPPORT_TRACE_TC4 == 1)
		wlanDebugTC4Uninit();  /* Uninit for TC4 debug */
#endif

		break;

	case BUS_SET_IRQ_FAIL:
#if (CFG_SUPPORT_TRACE_TC4 == 1)
		wlanDebugTC4Uninit();  /* Uninit for TC4 debug */
#endif

		wlanWakeLockUninit(prGlueInfo);
		wlanNetDestroy(prWdev);
		break;

	case ALLOC_ADAPTER_MEM_FAIL:
	case DRIVER_OWN_FAIL:
	case INIT_ADAPTER_FAIL:
		glBusFreeIrq(prWdev->netdev,
			*((struct GLUE_INFO **)	netdev_priv(prWdev->netdev)));

#if (CFG_SUPPORT_TRACE_TC4 == 1)
		wlanDebugTC4Uninit();  /* Uninit for TC4 debug */
#endif

		wlanWakeLockUninit(prGlueInfo);
		wlanNetDestroy(prWdev);
		break;

	case INIT_HIFINFO_FAIL:
		nicRxUninitialize(prAdapter);
		nicTxRelease(prAdapter, FALSE);

		/* System Service Uninitialization */
		nicUninitSystemService(prAdapter);

		glBusFreeIrq(prWdev->netdev,
			*((struct GLUE_INFO **)netdev_priv(prWdev->netdev)));

#if (CFG_SUPPORT_TRACE_TC4 == 1)
		wlanDebugTC4Uninit();  /* Uninit for TC4 debug */
#endif

		wlanWakeLockUninit(prGlueInfo);
		wlanNetDestroy(prWdev);
		break;

	case ROM_PATCH_DOWNLOAD_FAIL:
	case POWER_ON_INIT_DONE:
		HAL_ENABLE_FWDL(prAdapter, FALSE);
		nicRxUninitialize(prAdapter);
		nicTxRelease(prAdapter, FALSE);

		/* System Service Uninitialization */
		nicUninitSystemService(prAdapter);

		glBusFreeIrq(prWdev->netdev,
			*((struct GLUE_INFO **)netdev_priv(prWdev->netdev)));

#if (CFG_SUPPORT_TRACE_TC4 == 1)
		wlanDebugTC4Uninit();  /* Uninit for TC4 debug */
#endif

		wlanWakeLockUninit(prGlueInfo);
		wlanNetDestroy(prWdev);
		break;

	default:
		break;
	}

	if (prChipInfo->wmmcupwroff)
		prChipInfo->wmmcupwroff();

	DBGLOG(INIT, INFO, "soc3_0_wlanPreCal::end\n");
	return (int)i4Status;
}
#endif /* (CFG_SUPPORT_PRE_ON_PHY_ACTION == 1) */

#endif
#endif				/* soc3_0 */
