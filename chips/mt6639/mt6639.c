/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2021 MediaTek Inc.
 */
/*! \file   mt6639.c
*    \brief  Internal driver stack will export
*    the required procedures here for GLUE Layer.
*
*    This file contains all routines which are exported
     from MediaTek 802.11 Wireless LAN driver stack to GLUE Layer.
*/

#ifdef MT6639

#include "precomp.h"
#include "mt6639.h"
#include "coda/mt6639/wf_wfdma_ext_wrap_csr.h"
#include "coda/mt6639/wf_wfdma_host_dma0.h"
#include "coda/mt6639/wf_pse_top.h"
#include "coda/mt6639/pcie_mac_ireg.h"
#include "coda/mt6639/conn_mcu_bus_cr.h"
#include "coda/mt6639/conn_bus_cr_von.h"
#include "coda/mt6639/mawd_reg.h"
#include "coda/mt6639/wf_rro_top.h"
#include "hal_dmashdl_mt6639.h"

#define CFG_SUPPORT_VCODE_VDFS 0

#if (CFG_SUPPORT_VCODE_VDFS == 1)
#include <linux/pm_qos.h>

#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#endif /*#ifndef CFG_SUPPORT_VCODE_VDFS*/

/*******************************************************************************
*                         C O M P I L E R   F L A G S
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
*                   F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/
static uint32_t mt6639GetFlavorVer(struct ADAPTER *prAdapter);

static void mt6639_ConstructFirmwarePrio(struct GLUE_INFO *prGlueInfo,
	uint8_t **apucNameTable, uint8_t **apucName,
	uint8_t *pucNameIdx, uint8_t ucMaxNameIdx);

static void mt6639_ConstructPatchName(struct GLUE_INFO *prGlueInfo,
	uint8_t **apucName, uint8_t *pucNameIdx);

#if defined(_HIF_PCIE) || defined(_HIF_AXI)
static uint8_t mt6639SetRxRingHwAddr(struct RTMP_RX_RING *prRxRing,
		struct BUS_INFO *prBusInfo, uint32_t u4SwRingIdx);

static bool mt6639WfdmaAllocRxRing(struct GLUE_INFO *prGlueInfo,
		bool fgAllocMem);

static void mt6639ProcessTxInterrupt(
		struct ADAPTER *prAdapter);

static void mt6639ProcessRxInterrupt(
	struct ADAPTER *prAdapter);

static void mt6639WfdmaManualPrefetch(
	struct GLUE_INFO *prGlueInfo);

static void mt6639ReadIntStatus(struct ADAPTER *prAdapter,
		uint32_t *pu4IntStatus);

static void mt6639ConfigIntMask(struct GLUE_INFO *prGlueInfo,
		u_int8_t enable);

static void mt6639WpdmaConfig(struct GLUE_INFO *prGlueInfo,
		u_int8_t enable, bool fgResetHif);

static void mt6639SetupMcuEmiAddr(struct ADAPTER *prAdapter);

static void mt6639WfdmaRxRingExtCtrl(
	struct GLUE_INFO *prGlueInfo,
	struct RTMP_RX_RING *rx_ring,
	u_int32_t index);

#if defined(_HIF_PCIE)
static void mt6639InitPcieInt(struct GLUE_INFO *prGlueInfo);
#endif
#endif /*_HIF_PCIE || _HIF_AXI */

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

struct ECO_INFO mt6639_eco_table[] = {
	/* HW version,  ROM version,    Factory version */
	{0x00, 0x00, 0xA, 0x1},	/* E1 */
	{0x00, 0x00, 0x0, 0x0}	/* End of table */
};

uint8_t *apucmt6639FwName[] = {
	(uint8_t *) CFG_FW_FILENAME "_6639",
	NULL
};

#if defined(_HIF_PCIE)
struct PCIE_CHIP_CR_MAPPING mt6639_bus2chip_cr_mapping[] = {
	/* chip addr, bus addr, range */
	{0x830c0000, 0x00000, 0x1000}, /* WF_MCU_BUS_CR_REMAP */
	{0x54000000, 0x02000, 0x1000},  /* WFDMA PCIE0 MCU DMA0 */
	{0x55000000, 0x03000, 0x1000},  /* WFDMA PCIE0 MCU DMA1 */
	{0x56000000, 0x04000, 0x1000},  /* WFDMA reserved */
	{0x57000000, 0x05000, 0x1000},  /* WFDMA MCU wrap CR */
	{0x58000000, 0x06000, 0x1000},  /* WFDMA PCIE1 MCU DMA0 (MEM_DMA) */
	{0x59000000, 0x07000, 0x1000},  /* WFDMA PCIE1 MCU DMA1 */
	{0x820c0000, 0x08000, 0x4000},  /* WF_UMAC_TOP (PLE) */
	{0x820c8000, 0x0c000, 0x2000},  /* WF_UMAC_TOP (PSE) */
	{0x820cc000, 0x0e000, 0x2000},  /* WF_UMAC_TOP (PP) */
	{0x74030000, 0x10000, 0x1000},  /* PCIe MAC */
	{0x820e0000, 0x20000, 0x0400},  /* WF_LMAC_TOP BN0 (WF_CFG) */
	{0x820e1000, 0x20400, 0x0200},  /* WF_LMAC_TOP BN0 (WF_TRB) */
	{0x820e2000, 0x20800, 0x0400},  /* WF_LMAC_TOP BN0 (WF_AGG) */
	{0x820e3000, 0x20c00, 0x0400},  /* WF_LMAC_TOP BN0 (WF_ARB) */
	{0x820e4000, 0x21000, 0x0400},  /* WF_LMAC_TOP BN0 (WF_TMAC) */
	{0x820e5000, 0x21400, 0x0800},  /* WF_LMAC_TOP BN0 (WF_RMAC) */
	{0x820ce000, 0x21c00, 0x0200},  /* WF_LMAC_TOP (WF_SEC) */
	{0x820e7000, 0x21e00, 0x0200},  /* WF_LMAC_TOP BN0 (WF_DMA) */
	{0x820cf000, 0x22000, 0x1000},  /* WF_LMAC_TOP (WF_PF) */
	{0x820e9000, 0x23400, 0x0200},  /* WF_LMAC_TOP BN0 (WF_WTBLOFF) */
	{0x820ea000, 0x24000, 0x0200},  /* WF_LMAC_TOP BN0 (WF_ETBF) */
	{0x820eb000, 0x24200, 0x0400},  /* WF_LMAC_TOP BN0 (WF_LPON) */
	{0x820ec000, 0x24600, 0x0200},  /* WF_LMAC_TOP BN0 (WF_INT) */
	{0x820ed000, 0x24800, 0x0800},  /* WF_LMAC_TOP BN0 (WF_MIB) */
	{0x820ca000, 0x26000, 0x2000},  /* WF_LMAC_TOP BN0 (WF_MUCOP) */
	{0x820d0000, 0x30000, 0x10000}, /* WF_LMAC_TOP (WF_WTBLON) */
	{0x40000000, 0x70000, 0x10000}, /* WF_UMAC_SYSRAM */
	{0x00400000, 0x80000, 0x10000}, /* WF_MCU_SYSRAM */
	{0x00410000, 0x90000, 0x10000}, /* WF_MCU_SYSRAM (configure register) */
	{0x820f0000, 0xa0000, 0x0400},  /* WF_LMAC_TOP BN1 (WF_CFG) */
	{0x820f1000, 0xa0600, 0x0200},  /* WF_LMAC_TOP BN1 (WF_TRB) */
	{0x820f2000, 0xa0800, 0x0400},  /* WF_LMAC_TOP BN1 (WF_AGG) */
	{0x820f3000, 0xa0c00, 0x0400},  /* WF_LMAC_TOP BN1 (WF_ARB) */
	{0x820f4000, 0xa1000, 0x0400},  /* WF_LMAC_TOP BN1 (WF_TMAC) */
	{0x820f5000, 0xa1400, 0x0800},  /* WF_LMAC_TOP BN1 (WF_RMAC) */
	{0x820f7000, 0xa1e00, 0x0200},  /* WF_LMAC_TOP BN1 (WF_DMA) */
	{0x820f9000, 0xa3400, 0x0200},  /* WF_LMAC_TOP BN1 (WF_WTBLOFF) */
	{0x820fa000, 0xa4000, 0x0200},  /* WF_LMAC_TOP BN1 (WF_ETBF) */
	{0x820fb000, 0xa4200, 0x0400},  /* WF_LMAC_TOP BN1 (WF_LPON) */
	{0x820fc000, 0xa4600, 0x0200},  /* WF_LMAC_TOP BN1 (WF_INT) */
	{0x820fd000, 0xa4800, 0x0800},  /* WF_LMAC_TOP BN1 (WF_MIB) */
	{0x820c4000, 0xa8000, 0x4000},  /* WF_LMAC_TOP BN1 (WF_MUCOP) */
	{0x820b0000, 0xae000, 0x1000},  /* [APB2] WFSYS_ON */
	{0x80020000, 0xb0000, 0x10000}, /* WF_TOP_MISC_OFF */
	{0x81020000, 0xc0000, 0x10000}, /* WF_TOP_MISC_ON */
	{0x7c020000, 0xd0000, 0x10000}, /* CONN_INFRA, wfdma */
	{0x7c060000, 0xe0000, 0x10000}, /* CONN_INFRA, conn_host_csr_top */
	{0x7c000000, 0xf0000, 0x10000}, /* CONN_INFRA */
	{0x7c500000, MT6639_PCIE2AP_REMAP_BASE_ADDR, 0x2000000}, /* remap */
	{0x0, 0x0, 0x0} /* End */
};
#endif

#if defined(_HIF_PCIE) || defined(_HIF_AXI)
struct pcie2ap_remap mt6639_pcie2ap_remap = {
	.reg_base = CONN_BUS_CR_VON_CONN_INFRA_PCIE2AP_REMAP_WF_0_76_cr_pcie2ap_public_remapping_wf_06_ADDR,
	.reg_mask = CONN_BUS_CR_VON_CONN_INFRA_PCIE2AP_REMAP_WF_0_76_cr_pcie2ap_public_remapping_wf_06_MASK,
	.reg_shift = CONN_BUS_CR_VON_CONN_INFRA_PCIE2AP_REMAP_WF_0_76_cr_pcie2ap_public_remapping_wf_06_SHFT,
	.base_addr = MT6639_PCIE2AP_REMAP_BASE_ADDR
};

struct ap2wf_remap mt6639_ap2wf_remap = {
	.reg_base = CONN_MCU_BUS_CR_AP2WF_REMAP_1_R_AP2WF_PUBLIC_REMAPPING_0_START_ADDRESS_ADDR,
	.reg_mask = CONN_MCU_BUS_CR_AP2WF_REMAP_1_R_AP2WF_PUBLIC_REMAPPING_0_START_ADDRESS_MASK,
	.reg_shift = CONN_MCU_BUS_CR_AP2WF_REMAP_1_R_AP2WF_PUBLIC_REMAPPING_0_START_ADDRESS_SHFT,
	.base_addr = MT6639_REMAP_BASE_ADDR
};

struct PCIE_CHIP_CR_REMAPPING mt6639_bus2chip_cr_remapping = {
	.pcie2ap = &mt6639_pcie2ap_remap,
	.ap2wf = &mt6639_ap2wf_remap,
};

struct wfdma_group_info mt6639_wfmda_host_tx_group[] = {
	{"P0T0:AP DATA0", WF_WFDMA_HOST_DMA0_WPDMA_TX_RING0_CTRL0_ADDR},
	{"P0T1:AP DATA1", WF_WFDMA_HOST_DMA0_WPDMA_TX_RING1_CTRL0_ADDR},
	{"P0T2:AP DATA2", WF_WFDMA_HOST_DMA0_WPDMA_TX_RING2_CTRL0_ADDR},
	{"P0T8:MD DATA0", WF_WFDMA_HOST_DMA0_WPDMA_TX_RING8_CTRL0_ADDR},
	{"P0T9:MD DATA1", WF_WFDMA_HOST_DMA0_WPDMA_TX_RING9_CTRL0_ADDR},
	{"P0T10:MD DATA2", WF_WFDMA_HOST_DMA0_WPDMA_TX_RING10_CTRL0_ADDR},
	{"P0T15:AP CMD", WF_WFDMA_HOST_DMA0_WPDMA_TX_RING15_CTRL0_ADDR},
	{"P0T16:FWDL", WF_WFDMA_HOST_DMA0_WPDMA_TX_RING16_CTRL0_ADDR},
};

struct wfdma_group_info mt6639_wfmda_host_rx_group[] = {
	{"P0R4:AP DATA0", WF_WFDMA_HOST_DMA0_WPDMA_RX_RING4_CTRL0_ADDR},
	{"P0R6:AP EVENT", WF_WFDMA_HOST_DMA0_WPDMA_RX_RING6_CTRL0_ADDR},
	{"P0R5:AP DATA1", WF_WFDMA_HOST_DMA0_WPDMA_RX_RING5_CTRL0_ADDR},
	{"P0R7:AP TDONE0", WF_WFDMA_HOST_DMA0_WPDMA_RX_RING7_CTRL0_ADDR},
};

struct pse_group_info mt6639_pse_group[] = {
	{"HIF0(TX data)", WF_PSE_TOP_PG_HIF0_GROUP_ADDR,
		WF_PSE_TOP_HIF0_PG_INFO_ADDR},
	{"HIF1(Talos CMD)", WF_PSE_TOP_PG_HIF1_GROUP_ADDR,
		WF_PSE_TOP_HIF1_PG_INFO_ADDR},
	{"CPU(I/O r/w)",  WF_PSE_TOP_PG_CPU_GROUP_ADDR,
		WF_PSE_TOP_CPU_PG_INFO_ADDR},
	{"PLE(host report)",  WF_PSE_TOP_PG_PLE_GROUP_ADDR,
		WF_PSE_TOP_PLE_PG_INFO_ADDR},
	{"PLE1(SPL report)", WF_PSE_TOP_PG_PLE1_GROUP_ADDR,
		WF_PSE_TOP_PLE1_PG_INFO_ADDR},
	{"LMAC0(RX data)", WF_PSE_TOP_PG_LMAC0_GROUP_ADDR,
			WF_PSE_TOP_LMAC0_PG_INFO_ADDR},
	{"LMAC1(RX_VEC)", WF_PSE_TOP_PG_LMAC1_GROUP_ADDR,
			WF_PSE_TOP_LMAC1_PG_INFO_ADDR},
	{"LMAC2(TXS)", WF_PSE_TOP_PG_LMAC2_GROUP_ADDR,
			WF_PSE_TOP_LMAC2_PG_INFO_ADDR},
	{"LMAC3(TXCMD/RXRPT)", WF_PSE_TOP_PG_LMAC3_GROUP_ADDR,
			WF_PSE_TOP_LMAC3_PG_INFO_ADDR},
	{"MDP",  WF_PSE_TOP_PG_MDP_GROUP_ADDR,
			WF_PSE_TOP_MDP_PG_INFO_ADDR},
};
#endif /*_HIF_PCIE || _HIF_AXI */

#if defined(_HIF_PCIE)
struct pcie_msi_layout mt6639_pcie_msi_layout[] = {
	[0 ... 7] = {"conn_hif_host_int", mtk_pci_interrupt, NULL},
	[8 ... 15] = {"conn_hif_host_int", NULL, NULL},
	[16] = {"wm_conn2ap_wdt_irq", NULL, NULL},
	[17] = {"wf_mcu_jtag_det_eint", NULL, NULL},
	[18] = {"pmic_eint", NULL, NULL},
	[19] = {"ccif_bgf2ap_sw_irq", NULL, NULL},
	[20] = {"ccif_wf2ap_sw_irq", pcie_sw_int_top_handler,
		pcie_sw_int_thread_handler},
	[21] = {"ccif_bgf2ap_irq_0", NULL, NULL},
	[22] = {"ccif_bgf2ap_irq_1", NULL, NULL},
	[23 ... 31] = {"reserved", NULL, NULL},
};
#endif

struct BUS_INFO mt6639_bus_info = {
#if defined(_HIF_PCIE) || defined(_HIF_AXI)
	.top_cfg_base = MT6639_TOP_CFG_BASE,

	/* host_dma0 for TXP */
	.host_dma0_base = WF_WFDMA_HOST_DMA0_BASE,
	.host_int_status_addr = WF_WFDMA_HOST_DMA0_HOST_INT_STA_ADDR,

	.host_int_txdone_bits =
	(
#if (CFG_SUPPORT_DISABLE_DATA_DDONE_INTR == 0)
	 WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_0_MASK |
	 WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_1_MASK |
	 WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_2_MASK |
#endif /* CFG_SUPPORT_DISABLE_DATA_DDONE_INTR == 0 */
	 WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_15_MASK |
	 WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_16_MASK |
	 WF_WFDMA_HOST_DMA0_HOST_INT_STA_mcu2host_sw_int_sts_MASK),
	.host_int_rxdone_bits =
	(WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_4_MASK |
	 WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_5_MASK |
	 WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_6_MASK |
	 WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_7_MASK),

	.host_tx_ring_base = WF_WFDMA_HOST_DMA0_WPDMA_TX_RING0_CTRL0_ADDR,
	.host_tx_ring_ext_ctrl_base =
		WF_WFDMA_HOST_DMA0_WPDMA_TX_RING0_EXT_CTRL_ADDR,
	.host_tx_ring_cidx_addr = WF_WFDMA_HOST_DMA0_WPDMA_TX_RING0_CTRL2_ADDR,
	.host_tx_ring_didx_addr = WF_WFDMA_HOST_DMA0_WPDMA_TX_RING0_CTRL3_ADDR,
	.host_tx_ring_cnt_addr = WF_WFDMA_HOST_DMA0_WPDMA_TX_RING0_CTRL1_ADDR,

	.host_rx_ring_base = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING0_CTRL0_ADDR,
	.host_rx_ring_ext_ctrl_base =
		WF_WFDMA_HOST_DMA0_WPDMA_RX_RING0_EXT_CTRL_ADDR,
	.host_rx_ring_cidx_addr = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING0_CTRL2_ADDR,
	.host_rx_ring_didx_addr = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING0_CTRL3_ADDR,
	.host_rx_ring_cnt_addr = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING0_CTRL1_ADDR,

#if (CFG_SUPPORT_HOST_OFFLOAD == 1)
	/* MAWD */
	.mawd_rx_blk_ctrl0 = MAWD_AP_RX_BLK_CTRL0,
	.mawd_rx_blk_ctrl1 = MAWD_AP_RX_BLK_CTRL1,
	.mawd_rx_blk_ctrl2 = MAWD_AP_RX_BLK_CTRL2,
	.mawd_ring_ctrl0 = MAWD_WFDMA_RING_MD_CTRL0,
	.mawd_ring_ctrl1 = MAWD_WFDMA_RING_MD_CTRL1,
	.mawd_ring_ctrl2 = MAWD_WFDMA_RING_MD_CTRL2,
	.mawd_ring_ctrl3 = MAWD_WFDMA_RING_MD_CTRL3,
	.mawd_ring_ctrl4 = MAWD_WFDMA_RING_MD_CTRL4,
	.mawd_hif_txd_ctrl0 = MAWD_HIF_TXD_MD_CTRL0,
	.mawd_hif_txd_ctrl1 = MAWD_HIF_TXD_MD_CTRL1,
	.mawd_hif_txd_ctrl2 = MAWD_HIF_TXD_MD_CTRL2,
	.mawd_err_rpt_ctrl0 = MAWD_ERR_RPT_CTRL0,
	.mawd_err_rpt_ctrl1 = MAWD_ERR_RPT_CTRL1,
	.mawd_err_rpt_ctrl2 = MAWD_ERR_RPT_CTRL2,
	.mawd_settings0 = MAWD_SETTING0,
	.mawd_settings1 = MAWD_SETTING1,
	.mawd_settings2 = MAWD_SETTING2,
	.mawd_settings3 = MAWD_SETTING3,
	.mawd_settings4 = MAWD_SETTING4,
	.mawd_settings5 = MAWD_SETTING5,
	.mawd_settings6 = MAWD_SETTING6,
#endif /* CFG_SUPPORT_HOST_OFFLOAD == 1 */

	.bus2chip = mt6639_bus2chip_cr_mapping,
	.bus2chip_remap = &mt6639_bus2chip_cr_remapping,
	.max_static_map_addr = 0x00200000,

	.tx_ring_fwdl_idx = CONNAC3X_FWDL_TX_RING_IDX,
	.tx_ring_cmd_idx = 15,
	.tx_ring0_data_idx = 0,
	.tx_ring1_data_idx = 1,
	.tx_ring2_data_idx = 2,
	.fw_own_clear_addr = CONNAC3X_BN0_IRQ_STAT_ADDR,
	.fw_own_clear_bit = PCIE_LPCR_FW_CLR_OWN,
	.fgCheckDriverOwnInt = FALSE,
	.u4DmaMask = 32,
	.wfmda_host_tx_group = mt6639_wfmda_host_tx_group,
	.wfmda_host_tx_group_len = ARRAY_SIZE(mt6639_wfmda_host_tx_group),
	.wfmda_host_rx_group = mt6639_wfmda_host_rx_group,
	.wfmda_host_rx_group_len = ARRAY_SIZE(mt6639_wfmda_host_rx_group),
	.wfmda_wm_tx_group = NULL,
	.wfmda_wm_tx_group_len = 0,
	.wfmda_wm_rx_group = NULL,
	.wfmda_wm_rx_group_len = 0,
	.prDmashdlCfg = &rMt6639DmashdlCfg,
	.prPleTopCr = &rMt6639PleTopCr,
	.prPseTopCr = &rMt6639PseTopCr,
	.prPpTopCr = &rMt6639PpTopCr,
	.prPseGroup = mt6639_pse_group,
	.u4PseGroupLen = ARRAY_SIZE(mt6639_pse_group),
	.pdmaSetup = mt6639WpdmaConfig,
	.enableInterrupt = asicConnac3xEnablePlatformIRQ,
	.disableInterrupt = asicConnac3xDisablePlatformIRQ,
#if defined(_HIF_PCIE)
	.initPcieInt = mt6639InitPcieInt,
	.pdmaStop = asicConnac3xWfdmaStop,
	.pdmaPollingIdle = asicConnac3xWfdmaPollingAllIdle,
	.pcie_msi_info = {
		.prMsiLayout = mt6639_pcie_msi_layout,
		.u4MaxMsiNum = ARRAY_SIZE(mt6639_pcie_msi_layout),
	},
#endif
	.processTxInterrupt = mt6639ProcessTxInterrupt,
	.processRxInterrupt = mt6639ProcessRxInterrupt,
	.tx_ring_ext_ctrl = asicConnac3xWfdmaTxRingExtCtrl,
	.rx_ring_ext_ctrl = mt6639WfdmaRxRingExtCtrl,
	/* null wfdmaManualPrefetch if want to disable manual mode */
	.wfdmaManualPrefetch = mt6639WfdmaManualPrefetch,
	.lowPowerOwnRead = asicConnac3xLowPowerOwnRead,
	.lowPowerOwnSet = asicConnac3xLowPowerOwnSet,
	.lowPowerOwnClear = asicConnac3xLowPowerOwnClear,
	.wakeUpWiFi = asicWakeUpWiFi,
	.processSoftwareInterrupt = asicConnac3xProcessSoftwareInterrupt,
	.softwareInterruptMcu = asicConnac3xSoftwareInterruptMcu,
	.hifRst = asicConnac3xHifRst,
	.devReadIntStatus = mt6639ReadIntStatus,
	.setRxRingHwAddr = mt6639SetRxRingHwAddr,
	.wfdmaAllocRxRing = mt6639WfdmaAllocRxRing,
	.setupMcuEmiAddr = mt6639SetupMcuEmiAddr,
#endif /*_HIF_PCIE || _HIF_AXI */
#if defined(_HIF_PCIE) || defined(_HIF_AXI) || defined(_HIF_USB)
	.DmaShdlInit = mt6639DmashdlInit,
#endif
#if defined(_HIF_USB)
	.prDmashdlCfg = &rMt6639DmashdlCfg,
	.u4UdmaWlCfg_0_Addr = CONNAC3X_UDMA_WLCFG_0,
	.u4UdmaWlCfg_1_Addr = CONNAC3X_UDMA_WLCFG_1,
	.u4UdmaWlCfg_0 =
	    (CONNAC3X_UDMA_WLCFG_0_WL_TX_EN(1) |
	     CONNAC3X_UDMA_WLCFG_0_WL_RX_EN(1) |
	     CONNAC3X_UDMA_WLCFG_0_WL_RX_MPSZ_PAD0(1) |
	     CONNAC3X_UDMA_WLCFG_0_TICK_1US_EN(1)),
	.u4UdmaTxQsel = CONNAC3X_UDMA_TX_QSEL,
	.u4device_vender_request_in = DEVICE_VENDOR_REQUEST_IN_CONNAC2,
	.u4device_vender_request_out = DEVICE_VENDOR_REQUEST_OUT_CONNAC2,
	.asicUsbEventEpDetected = asicConnac3xUsbEventEpDetected,
	.asicUsbRxByteCount = asicConnac3xUsbRxByteCount,
#endif
};

#if CFG_ENABLE_FW_DOWNLOAD
struct FWDL_OPS_T mt6639_fw_dl_ops = {
	.constructFirmwarePrio = mt6639_ConstructFirmwarePrio,
	.constructPatchName = mt6639_ConstructPatchName,
	.downloadPatch = wlanDownloadPatch,
	.downloadFirmware = wlanConnacFormatDownload,
	.downloadByDynMemMap = NULL,
	.getFwInfo = wlanGetConnacFwInfo,
	.getFwDlInfo = asicGetFwDlInfo,
	.downloadEMI = wlanDownloadEMISectionViaDma,
};
#endif /* CFG_ENABLE_FW_DOWNLOAD */

struct TX_DESC_OPS_T mt6639_TxDescOps = {
	.fillNicAppend = fillNicTxDescAppend,
	.fillHifAppend = fillTxDescAppendByHostV2,
	.fillTxByteCount = fillConnac3xTxDescTxByteCount,
};

struct RX_DESC_OPS_T mt6639_RxDescOps = {};

struct CHIP_DBG_OPS mt6639_DebugOps = {
#if defined(_HIF_PCIE) || defined(_HIF_AXI)
	.showPdmaInfo = connac3x_show_wfdma_info,
#endif
	.showPseInfo = connac3x_show_pse_info,
	.showPleInfo = connac3x_show_ple_info,
	.showTxdInfo = connac3x_show_txd_Info,
	.showWtblInfo = connac3x_show_wtbl_info,
	.showMibInfo = connac3x_show_mib_info,
	//.showUmacFwtblInfo = connac3x_show_umac_wtbl_info,
	.showCsrInfo = NULL,
#if defined(_HIF_PCIE) || defined(_HIF_AXI)
	.showDmaschInfo = connac3x_show_dmashdl_info,
#endif
	.showHifInfo = NULL,
	.printHifDbgInfo = NULL,
	.show_rx_rate_info = connac3x_show_rx_rate_info,
	.show_rx_rssi_info = connac3x_show_rx_rssi_info,
	.show_stat_info = connac3x_show_stat_info,
#if defined(_HIF_PCIE) || defined(_HIF_AXI)
	.show_wfdma_dbg_probe_info = mt6639_show_wfdma_dbg_probe_info,
	.show_wfdma_wrapper_info = mt6639_show_wfdma_wrapper_info,
#endif
};

#if CFG_SUPPORT_QA_TOOL
struct ATE_OPS_T mt6639_AteOps = {
	/* ICapStart phase out , wlan_service instead */
	.setICapStart = connacSetICapStart,
	/* ICapStatus phase out , wlan_service instead */
	.getICapStatus = connacGetICapStatus,
	/* CapIQData phase out , wlan_service instead */
	.getICapIQData = connacGetICapIQData,
	.getRbistDataDumpEvent = nicExtEventICapIQData,
#if (CFG_SUPPORT_ICAP_SOLICITED_EVENT == 1)
	.getICapDataDumpCmdEvent = nicExtCmdEventSolicitICapIQData,
#endif
	.icapRiseVcoreClockRate = mt6639_icapRiseVcoreClockRate,
	.icapDownVcoreClockRate = mt6639_icapDownVcoreClockRate,
};
#endif /* CFG_SUPPORT_QA_TOOL */

struct mt66xx_chip_info mt66xx_chip_info_mt6639 = {
	.bus_info = &mt6639_bus_info,
#if CFG_ENABLE_FW_DOWNLOAD
	.fw_dl_ops = &mt6639_fw_dl_ops,
#endif /* CFG_ENABLE_FW_DOWNLOAD */
#if CFG_SUPPORT_QA_TOOL
	.prAteOps = &mt6639_AteOps,
#endif /* CFG_SUPPORT_QA_TOOL */
	.prTxDescOps = &mt6639_TxDescOps,
	.prRxDescOps = &mt6639_RxDescOps,
	.prDebugOps = &mt6639_DebugOps,
	.chip_id = MT6639_CHIP_ID,
	.should_verify_chip_id = FALSE,
	.sw_sync0 = Connac3x_CONN_CFG_ON_CONN_ON_MISC_ADDR,
	.sw_ready_bits = WIFI_FUNC_NO_CR4_READY_BITS,
	.sw_ready_bit_offset =
		Connac3x_CONN_CFG_ON_CONN_ON_MISC_DRV_FM_STAT_SYNC_SHFT,
	.patch_addr = MT6639_PATCH_START_ADDR,
	.is_support_cr4 = FALSE,
	.is_support_wacpu = FALSE,
#if (CFG_SUPPORT_HOST_OFFLOAD == 1)
	.is_support_mawd_tx = FALSE,
	.is_support_sdo = FALSE,
	.is_support_rro = FALSE,
	.is_en_rro_int = FALSE,
#endif /* CFG_SUPPORT_HOST_OFFLOAD == 1 */
	.txd_append_size = MT6639_TX_DESC_APPEND_LENGTH,
	.hif_txd_append_size = MT6639_HIF_TX_DESC_APPEND_LENGTH,
	.rxd_size = MT6639_RX_DESC_LENGTH,
	.init_evt_rxd_size = MT6639_RX_INIT_DESC_LENGTH,
	.pse_header_length = CONNAC3X_NIC_TX_PSE_HEADER_LENGTH,
	.init_event_size = CONNAC3X_RX_INIT_EVENT_LENGTH,
	.eco_info = mt6639_eco_table,
	.isNicCapV1 = FALSE,
	.top_hcr = CONNAC3X_TOP_HCR,
	.top_hvr = CONNAC3X_TOP_HVR,
	.top_fvr = CONNAC3X_TOP_FVR,
#if (CFG_SUPPORT_802_11AX == 1)
	.arb_ac_mode_addr = MT6639_ARB_AC_MODE_ADDR,
#endif
	.asicCapInit = asicConnac3xCapInit,
#if CFG_ENABLE_FW_DOWNLOAD
	.asicEnableFWDownload = NULL,
#endif /* CFG_ENABLE_FW_DOWNLOAD */
	.downloadBufferBin = NULL,
	.is_support_hw_amsdu = TRUE,
	.is_support_nvram_fragment = TRUE,
	.is_support_asic_lp = TRUE,
	.asicWfdmaReInit = asicConnac3xWfdmaReInit,
	.asicWfdmaReInit_handshakeInit = asicConnac3xWfdmaDummyCrWrite,
	.group5_size = sizeof(struct HW_MAC_RX_STS_GROUP_5),
	.u4LmacWtblDUAddr = CONNAC3X_WIFI_LWTBL_BASE,
	.u4UmacWtblDUAddr = CONNAC3X_WIFI_UWTBL_BASE,
	.isSupportMddpAOR = false,
	.cmd_max_pkt_size = CFG_TX_MAX_PKT_SIZE, /* size 1600 */
#if IS_ENABLED(CFG_MTK_WIFI_PMIC_QUERY)
	.queryPmicInfo = asicConnac3xQueryPmicInfo,
#endif
#if defined(_HIF_USB)
	.asicUsbInit = asicConnac3xWfdmaInitForUSB,
	.asicUsbInit_ic_specific = NULL,
#endif
};

struct mt66xx_hif_driver_data mt66xx_driver_data_mt6639 = {
	.chip_info = &mt66xx_chip_info_mt6639,
};

void mt6639_icapRiseVcoreClockRate(void)
{

#if (CFG_SUPPORT_VCODE_VDFS == 1)
	int value = 0;

#if (KERNEL_VERSION(5, 4, 0) <= CFG80211_VERSION_CODE)
	/* Implementation for kernel-5.4 */
	struct mt66xx_hif_driver_data *prDriverData =
		get_platform_driver_data();
	struct mt66xx_chip_info *prChipInfo;
	void *pdev;

	prChipInfo = ((struct mt66xx_hif_driver_data *)prDriverData)
		->chip_info;
	pdev = (void *)prChipInfo->pdev;

	dvfsrc_vcore_power = regulator_get(
		&((struct platform_device *)pdev)->dev, "dvfsrc-vcore");

	/* Enable VCore to 0.725 */
	regulator_set_voltage(dvfsrc_vcore_power, 725000, INT_MAX);
#else
	/* init */
	if (!pm_qos_request_active(&wifi_req))
		pm_qos_add_request(&wifi_req, PM_QOS_VCORE_OPP,
						PM_QOS_VCORE_OPP_DEFAULT_VALUE);

	/* update Vcore */
	pm_qos_update_request(&wifi_req, 0);
#endif

	DBGLOG(HAL, STATE, "icapRiseVcoreClockRate done\n");

	/* Seq2: update clock rate sel bus clock to 213MHz */

	/* 0x1801_2050[6:4]=3'b111 */
	wf_ioremap_read(WF_CONN_INFA_BUS_CLOCK_RATE, &value);
	value |= 0x00000070;
	wf_ioremap_write(WF_CONN_INFA_BUS_CLOCK_RATE, value);

	/* Seq3: enable clock select sw mode */

	/* 0x1801_2050[0]=1'b1 */
	wf_ioremap_read(WF_CONN_INFA_BUS_CLOCK_RATE, &value);
	value |= 0x1;
	wf_ioremap_write(WF_CONN_INFA_BUS_CLOCK_RATE, value);

#else
	DBGLOG(HAL, STATE, "icapRiseVcoreClockRate skip\n");
#endif  /*#ifndef CFG_BUILD_X86_PLATFORM*/
}

void mt6639_icapDownVcoreClockRate(void)
{

#if (CFG_SUPPORT_VCODE_VDFS == 1)
	int value = 0;

#if (KERNEL_VERSION(5, 4, 0) <= CFG80211_VERSION_CODE)
	/* Implementation for kernel-5.4 */
	struct mt66xx_chip_info *prChipInfo;
	struct mt66xx_hif_driver_data *prDriverData =
		get_platform_driver_data();
	void *pdev;

	prChipInfo = ((struct mt66xx_hif_driver_data *)prDriverData)
		->chip_info;
	pdev = (void *)prChipInfo->pdev;

	dvfsrc_vcore_power = regulator_get(
		&((struct platform_device *)pdev)->dev, "dvfsrc-vcore");

	/* resume to default Vcore value */
	regulator_set_voltage(dvfsrc_vcore_power, 575000, INT_MAX);
#else
	/*init*/
	if (!pm_qos_request_active(&wifi_req))
		pm_qos_add_request(&wifi_req, PM_QOS_VCORE_OPP,
						PM_QOS_VCORE_OPP_DEFAULT_VALUE);

	/*restore to default Vcore*/
	pm_qos_update_request(&wifi_req,
		PM_QOS_VCORE_OPP_DEFAULT_VALUE);
#endif

	/*disable VCore to normal setting*/
	DBGLOG(HAL, STATE, "icapDownVcoreClockRate done!\n");

	/* Seq2: update clock rate sel bus clock to default value */

	/* 0x1801_2050[6:4]=3'b000 */
	wf_ioremap_read(WF_CONN_INFA_BUS_CLOCK_RATE, &value);
	value &= ~(0x00000070);
	wf_ioremap_write(WF_CONN_INFA_BUS_CLOCK_RATE, value);

	/* Seq3: disble clock select sw mode */

	/* 0x1801_2050[0]=1'b0 */
	wf_ioremap_read(WF_CONN_INFA_BUS_CLOCK_RATE, &value);
	value &= ~(0x1);
	wf_ioremap_write(WF_CONN_INFA_BUS_CLOCK_RATE, value);

#else
	DBGLOG(HAL, STATE, "icapDownVcoreClockRate skip\n");
#endif  /*#ifndef CFG_BUILD_X86_PLATFORM*/
}

static void mt6639_ConstructFirmwarePrio(struct GLUE_INFO *prGlueInfo,
	uint8_t **apucNameTable, uint8_t **apucName,
	uint8_t *pucNameIdx, uint8_t ucMaxNameIdx)
{
	int ret = 0;
	uint8_t ucIdx = 0;

	for (ucIdx = 0; apucmt6639FwName[ucIdx]; ucIdx++) {
		if ((*pucNameIdx + 3) >= ucMaxNameIdx) {
			/* the table is not large enough */
			DBGLOG(INIT, ERROR,
				"kalFirmwareImageMapping >> file name array is not enough.\n");
			ASSERT(0);
			continue;
		}

		/* Type 1. WIFI_RAM_CODE_MT6639_1_1.bin */
		ret = kalSnprintf(*(apucName + (*pucNameIdx)),
				CFG_FW_NAME_MAX_LEN,
				"WIFI_RAM_CODE_MT%x_%x_%u.bin",
				MT6639_CHIP_ID,
				mt6639GetFlavorVer(prGlueInfo->prAdapter),
				MT6639_ROM_VERSION);
		if (ret >= 0 && ret < CFG_FW_NAME_MAX_LEN)
			(*pucNameIdx) += 1;
		else
			DBGLOG(INIT, ERROR,
				"[%u] kalSnprintf failed, ret: %d\n",
				__LINE__, ret);

		/* Type 2. WIFI_RAM_CODE_6639.bin */
		ret = kalSnprintf(*(apucName + (*pucNameIdx)),
				CFG_FW_NAME_MAX_LEN, "%s.bin",
				apucmt6639FwName[ucIdx]);
		if (ret >= 0 && ret < CFG_FW_NAME_MAX_LEN)
			(*pucNameIdx) += 1;
		else
			DBGLOG(INIT, ERROR,
				"[%u] kalSnprintf failed, ret: %d\n",
				__LINE__, ret);
	}
}

static void mt6639_ConstructPatchName(struct GLUE_INFO *prGlueInfo,
	uint8_t **apucName, uint8_t *pucNameIdx)
{
	int ret = 0;

	/* Type 1. WIFI_MT6639_PATCH_MCU_1_1_hdr.bin */
	ret = kalSnprintf(apucName[(*pucNameIdx)],
			  CFG_FW_NAME_MAX_LEN,
			  "WIFI_MT%x_PATCH_MCU_%x_%u_hdr.bin",
			  MT6639_CHIP_ID,
			  mt6639GetFlavorVer(prGlueInfo->prAdapter),
			  MT6639_ROM_VERSION);
	if (ret >= 0 && ret < CFG_FW_NAME_MAX_LEN)
		(*pucNameIdx) += 1;
	else
		DBGLOG(INIT, ERROR,
			"[%u] kalSnprintf failed, ret: %d\n",
			__LINE__, ret);

	/* Type 2. mt6639_patch_e1_hdr.bin */
	ret = kalSnprintf(apucName[(*pucNameIdx)],
			  CFG_FW_NAME_MAX_LEN,
			  "mt6639_patch_e1_hdr.bin");
	if (ret < 0 || ret >= CFG_FW_NAME_MAX_LEN)
		DBGLOG(INIT, ERROR,
			"[%u] kalSnprintf failed, ret: %d\n",
			__LINE__, ret);
}

#if defined(_HIF_PCIE) || defined(_HIF_AXI)
static uint8_t mt6639SetRxRingHwAddr(struct RTMP_RX_RING *prRxRing,
		struct BUS_INFO *prBusInfo, uint32_t u4SwRingIdx)
{
	uint32_t offset = 0;

	/*
	 * RX_RING_DATA_IDX_0   (RX_Ring0) - Band0 Rx Data
	 * RX_RING_DATA1_IDX_2 (RX_Ring1) - Band1 Rx Data
	 * RX_RING_EVT_IDX_1    (RX_Ring2) - Band0 Tx Free Done Event / Rx Event
	 * RX_RING_TXDONE0_IDX_3 (RX_Ring3) - Band1 Tx Free Done Event
	*/
	switch (u4SwRingIdx) {
	case RX_RING_EVT_IDX_1:
		offset = 6;
		break;
	case RX_RING_DATA_IDX_0:
		offset = 4;
		break;
	case RX_RING_DATA1_IDX_2:
		offset = 5;
		break;
	case RX_RING_TXDONE0_IDX_3:
		offset = 7;
		break;
	default:
		return FALSE;
	}

	halSetRxRingHwAddr(prRxRing, prBusInfo, offset);

	return TRUE;
}

static bool mt6639WfdmaAllocRxRing(struct GLUE_INFO *prGlueInfo,
		bool fgAllocMem)
{
	/* Band1 Data Rx path */
	if (!halWpdmaAllocRxRing(prGlueInfo,
			RX_RING_DATA1_IDX_2, RX_RING0_SIZE,
			RXD_SIZE, CFG_RX_MAX_PKT_SIZE, fgAllocMem)) {
		DBGLOG(HAL, ERROR, "AllocRxRing[2] fail\n");
		return false;
	}
	/* Band0 Tx Free Done Event */
	if (!halWpdmaAllocRxRing(prGlueInfo,
			RX_RING_TXDONE0_IDX_3, RX_RING1_SIZE,
			RXD_SIZE, RX_BUFFER_AGGRESIZE, fgAllocMem)) {
		DBGLOG(HAL, ERROR, "AllocRxRing[3] fail\n");
		return false;
	}
	return true;
}

static void mt6639ProcessTxInterrupt(
		struct ADAPTER *prAdapter)
{
	struct GL_HIF_INFO *prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	uint32_t u4Sta = prHifInfo->u4IntStatus;

	if (u4Sta | WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_16_MASK)
		halWpdmaProcessCmdDmaDone(
			prAdapter->prGlueInfo, TX_RING_FWDL_IDX_4);

	if (u4Sta | WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_15_MASK)
		halWpdmaProcessCmdDmaDone(
			prAdapter->prGlueInfo, TX_RING_CMD_IDX_3);

	if (u4Sta | WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_0_MASK) {
		halWpdmaProcessDataDmaDone(
			prAdapter->prGlueInfo, TX_RING_DATA0_IDX_0);
		kalSetTxEvent2Hif(prAdapter->prGlueInfo);
	}

	if (u4Sta | WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_1_MASK) {
		halWpdmaProcessDataDmaDone(
			prAdapter->prGlueInfo, TX_RING_DATA1_IDX_1);
		kalSetTxEvent2Hif(prAdapter->prGlueInfo);
	}

	if (u4Sta | WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_2_MASK) {
		halWpdmaProcessDataDmaDone(
			prAdapter->prGlueInfo, TX_RING_DATA2_IDX_2);
		kalSetTxEvent2Hif(prAdapter->prGlueInfo);
	}
}

static void mt6639ProcessRxInterrupt(
	struct ADAPTER *prAdapter)
{
	struct GL_HIF_INFO *prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	struct mt66xx_chip_info *prChipInfo = prAdapter->chip_info;
	uint32_t u4Sta = prHifInfo->u4IntStatus;
	uint32_t u4MawdSta = prHifInfo->u4MawdIntStatus;
	uint32_t u4Addr, u4Val;

	if ((u4Sta | WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_4_MASK) ||
	    (KAL_TEST_BIT(RX_RING_DATA_IDX_0, prAdapter->ulNoMoreRfb))) {
#if (CFG_SUPPORT_HOST_OFFLOAD == 1)
		if (prChipInfo->is_support_rro)
			halMawdUpdateWfdmaRxBlk(prAdapter->prGlueInfo,
						RX_RING_DATA_IDX_0);
		else
#endif /* CFG_SUPPORT_HOST_OFFLOAD == 1 */
			halRxReceiveRFBs(prAdapter, RX_RING_DATA_IDX_0, TRUE);
	}

	if ((u4Sta | WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_5_MASK) ||
	    (KAL_TEST_BIT(RX_RING_DATA1_IDX_2, prAdapter->ulNoMoreRfb))) {
#if (CFG_SUPPORT_HOST_OFFLOAD == 1)
		if (prChipInfo->is_support_rro)
			halMawdUpdateWfdmaRxBlk(prAdapter->prGlueInfo,
						RX_RING_DATA1_IDX_2);
		else
#endif /* CFG_SUPPORT_HOST_OFFLOAD == 1 */
			halRxReceiveRFBs(prAdapter, RX_RING_DATA1_IDX_2, TRUE);
	}

	if ((u4Sta | WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_6_MASK) ||
	    (KAL_TEST_BIT(RX_RING_EVT_IDX_1, prAdapter->ulNoMoreRfb)))
		halRxReceiveRFBs(prAdapter, RX_RING_EVT_IDX_1, FALSE);

	if ((u4Sta | WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_7_MASK) ||
	    (KAL_TEST_BIT(RX_RING_TXDONE0_IDX_3, prAdapter->ulNoMoreRfb)))
		halRxReceiveRFBs(prAdapter, RX_RING_TXDONE0_IDX_3, FALSE);

#if (CFG_SUPPORT_HOST_OFFLOAD == 1)
	if (prChipInfo->is_support_rro) {
		if (prChipInfo->is_en_rro_int) {
			u4Addr = WF_RRO_TOP_HOST_INT_STS_ADDR;
			kalDevRegRead(prAdapter->prGlueInfo, u4Addr, &u4Val);
			if (u4Val &
			    WF_RRO_TOP_HOST_INT_STS_HOST_RRO_DONE_INT_MASK)
				kalDevRegWrite(prAdapter->prGlueInfo,
					       u4Addr, u4Val);
		}

		if (u4MawdSta & BIT(0))
			halMawdReadRxBlks(prAdapter, RX_RING_DATA_IDX_0);
	}
#endif /* CFG_SUPPORT_HOST_OFFLOAD == 1 */
}

static void mt6639WfdmaManualPrefetch(
	struct GLUE_INFO *prGlueInfo)
{
	struct ADAPTER *prAdapter = prGlueInfo->prAdapter;
	u_int32_t val = 0;
	uint32_t u4WrVal = 0x00000004, u4Addr = 0;

	HAL_MCR_RD(prAdapter, WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_ADDR, &val);
	/* disable prefetch offset calculation auto-mode */
	val &=
	~WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_CSR_DISP_BASE_PTR_CHAIN_EN_MASK;
	HAL_MCR_WR(prAdapter, WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_ADDR, val);

	/* Rx ring */
	for (u4Addr = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING4_EXT_CTRL_ADDR;
	     u4Addr <= WF_WFDMA_HOST_DMA0_WPDMA_RX_RING7_EXT_CTRL_ADDR;
	     u4Addr += 0x4) {
		HAL_MCR_WR(prAdapter, u4Addr, u4WrVal);
		u4WrVal += 0x00400000;
	}

	/* Tx ring */
	for (u4Addr = WF_WFDMA_HOST_DMA0_WPDMA_TX_RING0_EXT_CTRL_ADDR;
	     u4Addr <= WF_WFDMA_HOST_DMA0_WPDMA_TX_RING2_EXT_CTRL_ADDR;
	     u4Addr += 0x4) {
		HAL_MCR_WR(prAdapter, u4Addr, u4WrVal);
		u4WrVal += 0x00400000;
	}

	for (u4Addr = WF_WFDMA_HOST_DMA0_WPDMA_TX_RING8_EXT_CTRL_ADDR;
	     u4Addr <= WF_WFDMA_HOST_DMA0_WPDMA_TX_RING10_EXT_CTRL_ADDR;
	     u4Addr += 0x4) {
		HAL_MCR_WR(prAdapter, u4Addr, u4WrVal);
		u4WrVal += 0x00400000;
	}

	for (u4Addr = WF_WFDMA_HOST_DMA0_WPDMA_TX_RING15_EXT_CTRL_ADDR;
	     u4Addr <= WF_WFDMA_HOST_DMA0_WPDMA_TX_RING16_EXT_CTRL_ADDR;
	     u4Addr += 0x4) {
		HAL_MCR_WR(prAdapter, u4Addr, u4WrVal);
		u4WrVal += 0x00400000;
	}

	/* reset dma TRX idx */
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA0_WPDMA_RST_DTX_PTR_ADDR, 0xFFFFFFFF);
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA0_WPDMA_RST_DRX_PTR_ADDR, 0xFFFFFFFF);
}

static void mt6639ReadIntStatus(struct ADAPTER *prAdapter,
		uint32_t *pu4IntStatus)
{
	struct GL_HIF_INFO *prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	struct BUS_INFO *prBusInfo = prAdapter->chip_info->bus_info;
	uint32_t u4RegValue, u4WrValue = 0;
#if (CFG_SUPPORT_HOST_OFFLOAD == 1)
	uint32_t u4MawdSta = 0;
#endif /* CFG_SUPPORT_HOST_OFFLOAD == 1 */

	*pu4IntStatus = 0;

	HAL_MCR_RD(prAdapter,
		WF_WFDMA_HOST_DMA0_HOST_INT_STA_ADDR, &u4RegValue);

#if (CFG_SUPPORT_HOST_OFFLOAD == 1)
	HAL_MCR_RD(prAdapter, MAWD_AP_INTERRUPT_SETTING0, &u4MawdSta);
#endif /* CFG_SUPPORT_HOST_OFFLOAD == 1 */
#if defined(_HIF_PCIE) || defined(_HIF_AXI)
	if (HAL_IS_CONNAC3X_EXT_RX_DONE_INTR(
		    u4RegValue, prBusInfo->host_int_rxdone_bits)) {
		*pu4IntStatus |= WHISR_RX0_DONE_INT;
		u4WrValue |= (u4RegValue & prBusInfo->host_int_rxdone_bits);
	}

	if (HAL_IS_CONNAC3X_EXT_TX_DONE_INTR(
		    u4RegValue, prBusInfo->host_int_txdone_bits)) {
		*pu4IntStatus |= WHISR_TX_DONE_INT;
		u4WrValue |= (u4RegValue & prBusInfo->host_int_txdone_bits);
	}
#endif
	if (u4RegValue & CONNAC_MCU_SW_INT) {
		*pu4IntStatus |= WHISR_D2H_SW_INT;
		u4WrValue |= (u4RegValue & CONNAC_MCU_SW_INT);
	}

	if (u4RegValue & CONNAC_SUBSYS_INT) {
		*pu4IntStatus |= WHISR_RX0_DONE_INT;
		u4WrValue |= (u4RegValue & CONNAC_SUBSYS_INT);
	}

#if CFG_SUPPORT_DISABLE_DATA_DDONE_INTR
	prHifInfo->u4IntStatus = u4WrValue;
#else
	prHifInfo->u4IntStatus = u4RegValue;
#endif /* CFG_SUPPORT_DISABLE_DATA_DDONE_INTR */

#if (CFG_SUPPORT_HOST_OFFLOAD == 1)
	if (u4MawdSta & BIT(0))
		*pu4IntStatus |= WHISR_RX0_DONE_INT;

	prHifInfo->u4MawdIntStatus = u4MawdSta;
#endif /* CFG_SUPPORT_HOST_OFFLOAD == 1 */

	/* clear interrupt */
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA0_HOST_INT_STA_ADDR, u4RegValue);
#if (CFG_SUPPORT_HOST_OFFLOAD == 1)
	HAL_MCR_WR(prAdapter, MAWD_AP_INTERRUPT_SETTING1, u4MawdSta);
#endif /* CFG_SUPPORT_HOST_OFFLOAD == 1 */
}

static void mt6639ConfigIntMask(struct GLUE_INFO *prGlueInfo,
		u_int8_t enable)
{
	struct ADAPTER *prAdapter = prGlueInfo->prAdapter;
	uint32_t u4Addr = 0, u4WrVal = 0, u4Val = 0;

	u4Addr = enable ? WF_WFDMA_HOST_DMA0_HOST_INT_ENA_SET_ADDR :
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_CLR_ADDR;
	u4WrVal =
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_HOST_RX_DONE_INT_ENA4_MASK |
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_HOST_RX_DONE_INT_ENA5_MASK |
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_HOST_RX_DONE_INT_ENA6_MASK |
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_HOST_RX_DONE_INT_ENA7_MASK |
#if (CFG_SUPPORT_DISABLE_DATA_DDONE_INTR == 0)
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_HOST_TX_DONE_INT_ENA0_MASK |
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_HOST_TX_DONE_INT_ENA1_MASK |
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_HOST_TX_DONE_INT_ENA2_MASK |
#endif /* CFG_SUPPORT_DISABLE_DATA_DDONE_INTR == 0 */
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_HOST_TX_DONE_INT_ENA15_MASK |
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_HOST_TX_DONE_INT_ENA16_MASK |
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_mcu2host_sw_int_ena_MASK;
#if (CFG_SUPPORT_HOST_OFFLOAD == 1)
	if (prAdapter->chip_info->is_en_rro_int)
		u4WrVal |=
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_subsys_int_ena_MASK;
#endif /* CFG_SUPPORT_HOST_OFFLOAD == 1 */

	HAL_MCR_WR(prGlueInfo->prAdapter, u4Addr, u4WrVal);

	HAL_MCR_RD(prGlueInfo->prAdapter,
		   WF_WFDMA_HOST_DMA0_HOST_INT_ENA_ADDR, &u4Val);

	DBGLOG(HAL, TRACE,
	       "HOST_INT_ENA(0x%08x):0x%08x, En:%u, Word:0x%08x\n",
	       WF_WFDMA_HOST_DMA0_HOST_INT_ENA_ADDR,
	       u4Val,
	       enable,
	       u4WrVal);
}

#if defined(_HIF_PCIE)
static void mt6639WpdmaMsiConfig(struct ADAPTER *prAdapter)
{
#define WFDMA_AP_MSI_NUM		8
#if CFG_MTK_MDDP_SUPPORT
#define WFDMA_MD_MSI_NUM		8
#endif

	struct mt66xx_chip_info *prChipInfo = NULL;
	struct BUS_INFO *prBusInfo = NULL;
	struct pcie_msi_info *prMsiInfo = NULL;
	uint32_t u4Value = 0;

	prChipInfo = prAdapter->chip_info;
	prBusInfo = prChipInfo->bus_info;
	prMsiInfo = &prBusInfo->pcie_msi_info;

	if (!prMsiInfo->fgMsiEnabled)
		return;

	/* configure MSI number */
	HAL_MCR_RD(prAdapter, WF_WFDMA_EXT_WRAP_CSR_WFDMA_HOST_CONFIG_ADDR,
		&u4Value);
	u4Value |= ((ilog2(WFDMA_AP_MSI_NUM) <<
		WF_WFDMA_EXT_WRAP_CSR_WFDMA_HOST_CONFIG_pcie0_msi_num_SHFT) &
		WF_WFDMA_EXT_WRAP_CSR_WFDMA_HOST_CONFIG_pcie0_msi_num_MASK);
#if CFG_MTK_MDDP_SUPPORT
	u4Value |= ((ilog2(WFDMA_MD_MSI_NUM) <<
		WF_WFDMA_EXT_WRAP_CSR_WFDMA_HOST_CONFIG_pcie0_md_msi_num_SHFT) &
		WF_WFDMA_EXT_WRAP_CSR_WFDMA_HOST_CONFIG_pcie0_md_msi_num_MASK);
#endif
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_EXT_WRAP_CSR_WFDMA_HOST_CONFIG_ADDR,
		u4Value);

	/* Set WFDMA MSI_Ring Mapping */
	u4Value = 0x00660077;
#if CFG_MTK_MDDP_SUPPORT
	u4Value |= 0xAA00BB00;
#endif
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_EXT_WRAP_CSR_MSI_INT_CFG0_ADDR,
		u4Value);

	u4Value = 0x00001100;
#if CFG_MTK_MDDP_SUPPORT
	u4Value |= 0x99880000;
#endif
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_EXT_WRAP_CSR_MSI_INT_CFG1_ADDR,
		u4Value);

	u4Value = 0x0030004F;
#if CFG_MTK_MDDP_SUPPORT
	u4Value |= 0x00005E00;
#endif
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_EXT_WRAP_CSR_MSI_INT_CFG2_ADDR,
		u4Value);

	u4Value = 0x00542200;
#if CFG_MTK_MDDP_SUPPORT
	u4Value |= 0x98000800;
#endif
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_EXT_WRAP_CSR_MSI_INT_CFG3_ADDR,
		u4Value);
}
#endif

static void mt6639WpdmaConfig(struct GLUE_INFO *prGlueInfo,
		u_int8_t enable, bool fgResetHif)
{
	struct ADAPTER *prAdapter = prGlueInfo->prAdapter;
	union WPDMA_GLO_CFG_STRUCT GloCfg;
	uint32_t u4DmaCfgCr = 0;
	uint32_t idx = 0, u4Val = 0;

#if defined(_HIF_PCIE) || defined(_HIF_AXI)
	asicConnac3xWfdmaControl(prGlueInfo, 0, enable);
	u4DmaCfgCr = asicConnac3xWfdmaCfgAddrGet(prGlueInfo, 0);
#endif
	HAL_MCR_RD(prAdapter, u4DmaCfgCr, &GloCfg.word);

	mt6639ConfigIntMask(prGlueInfo, enable);

	if (enable) {
#if defined(_HIF_PCIE)
		mt6639WpdmaMsiConfig(prGlueInfo->prAdapter);
#endif
#if defined(_HIF_PCIE) || defined(_HIF_AXI)
		u4DmaCfgCr = asicConnac3xWfdmaCfgAddrGet(prGlueInfo, idx);
#endif
		GloCfg.field_conn3x.tx_dma_en = 1;
		GloCfg.field_conn3x.rx_dma_en = 1;
		HAL_MCR_WR(prAdapter, u4DmaCfgCr, GloCfg.word);
	}

#if (CFG_SUPPORT_HOST_OFFLOAD == 1)
	if (prAdapter->chip_info->is_support_sdo) {
		/* enable SDO */
		HAL_MCR_RD(prAdapter,
			   WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_EXT0_ADDR,
			   &u4Val);
		u4Val |=
		WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_EXT0_CSR_SDO_DISP_MODE_MASK;
		HAL_MCR_WR(prAdapter,
			   WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_EXT0_ADDR,
			   u4Val);
	}
#endif /* CFG_SUPPORT_HOST_OFFLOAD == 1 */

	/* packet based TX flow control */
	kalDevRegRead(prGlueInfo,
		      WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_EXT1_ADDR,
		      &u4Val);
	u4Val |= WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_EXT1_CSR_TX_FCTRL_MODE_MASK;
	kalDevRegWrite(prGlueInfo,
		       WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_EXT1_ADDR,
		       u4Val);
}

static void mt6639WfdmaRxRingExtCtrl(
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
		ext_offset = 6 * 4;
		break;
	case RX_RING_DATA_IDX_0:
		ext_offset = 4 * 4;
		break;
	case RX_RING_DATA1_IDX_2:
		ext_offset = 5 * 4;
		break;
	case RX_RING_TXDONE0_IDX_3:
		ext_offset = 7 * 4;
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

#if defined(_HIF_PCIE)
static void mt6639InitPcieInt(struct GLUE_INFO *prGlueInfo)
{
	uint32_t value = 0;

	HAL_MCR_RD(prGlueInfo->prAdapter,
		PCIE_MAC_IREG_IMASK_HOST_ADDR,
		&value);
	value |= PCIE_MAC_IREG_IMASK_HOST_INT_REQUEST_EN_MASK |
		PCIE_MAC_IREG_IMASK_HOST_P_ATR_EVT_EN_MASK |
		PCIE_MAC_IREG_IMASK_HOST_A_ATR_EVT_EN_MASK |
		PCIE_MAC_IREG_IMASK_HOST_DMA_ERROR_EN_MASK |
		PCIE_MAC_IREG_IMASK_HOST_DMA_END_EN_MASK;
	HAL_MCR_WR(prGlueInfo->prAdapter,
		PCIE_MAC_IREG_IMASK_HOST_ADDR,
		value);
}
#endif

static void mt6639SetupMcuEmiAddr(struct ADAPTER *prAdapter)
{
	struct GL_HIF_INFO *prHifInfo =
		&prAdapter->prGlueInfo->rHifInfo;

	HAL_MCR_WR(prAdapter,
		   CONNAC3X_CONN_CFG_ON_CONN_ON_EMI_ADDR,
		   ((uint32_t)prHifInfo->rMcuEmiMem.pa >> 16));
}
#endif /*_HIF_PCIE || _HIF_AXI */

static uint32_t mt6639GetFlavorVer(struct ADAPTER *prAdapter)
{
	if (IS_MOBILE_SEGMENT)
		return 0x1;
	else
		return 0x2;
}

#endif  /* MT6639 */
