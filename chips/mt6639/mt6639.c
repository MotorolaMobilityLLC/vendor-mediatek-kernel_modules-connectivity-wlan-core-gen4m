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
#include "coda/mt6639/cb_infra_misc0.h"
#include "coda/mt6639/cb_infra_rgu.h"
#include "coda/mt6639/cbtop_gpio_sw_def.h"
#include "coda/mt6639/conn_bus_cr.h"
#include "coda/mt6639/conn_cfg.h"
#include "coda/mt6639/conn_dbg_ctl.h"
#include "coda/mt6639/conn_host_csr_top.h"
#include "coda/mt6639/conn_semaphore.h"
#include "coda/mt6639/wf_cr_sw_def.h"
#include "coda/mt6639/wf_top_cfg.h"
#include "coda/mt6639/wf_wfdma_ext_wrap_csr.h"
#include "coda/mt6639/wf_wfdma_host_dma0.h"
#include "coda/mt6639/wf_wfdma_mcu_dma0.h"
#include "coda/mt6639/wf_pse_top.h"
#include "coda/mt6639/pcie_mac_ireg.h"
#include "coda/mt6639/conn_mcu_bus_cr.h"
#include "coda/mt6639/conn_bus_cr_von.h"
#include "coda/mt6639/conn_host_csr_top.h"
#include "coda/mt6639/mawd_reg.h"
#include "coda/mt6639/wf_rro_top.h"
#include "coda/mt6639/wf_top_cfg_on.h"
#include "hal_dmashdl_mt6639.h"
#include "coda/mt6639/wf2ap_conn_infra_on_ccif4.h"
#include "coda/mt6639/ap2wf_conn_infra_on_ccif4.h"
#include "coda/mt6639/wf_top_cfg_on.h"
#include "coda/mt6639/wf_wtblon_top.h"
#include "coda/mt6639/wf_uwtbl_top.h"
#include "coda/mt6639/cb_infra_slp_ctrl.h"
#if IS_ENABLED(CFG_MTK_WIFI_CONNV3_SUPPORT)
#include "connv3.h"
#endif
#include "wlan_pinctrl.h"
#if IS_ENABLED(CFG_MTK_WIFI_FW_LOG_MMIO)
#include "fw_log_mmio.h"
#endif
#if IS_ENABLED(CFG_MTK_WIFI_FW_LOG_EMI)
#include "fw_log_emi.h"
#endif

#if CFG_MTK_MDDP_SUPPORT
#include "mddp_export.h"
#endif

#if CFG_MTK_CCCI_SUPPORT
#include "mtk_ccci_common.h"
#endif

#include "gl_coredump.h"

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
static uint32_t mt6639GetFlavorVer(uint8_t *flavor);

static void mt6639_ConstructFirmwarePrio(struct GLUE_INFO *prGlueInfo,
	uint8_t **apucNameTable, uint8_t **apucName,
	uint8_t *pucNameIdx, uint8_t ucMaxNameIdx);

static void mt6639_ConstructPatchName(struct GLUE_INFO *prGlueInfo,
	uint8_t **apucName, uint8_t *pucNameIdx);

#if defined(_HIF_PCIE)
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

static void mt6639EnableInterrupt(struct ADAPTER *prAdapter);
static void mt6639DisableInterrupt(struct ADAPTER *prAdapter);

static void mt6639ConfigIntMask(struct GLUE_INFO *prGlueInfo,
		u_int8_t enable);

static void mt6639ConfigWfdmaRxRingThreshold(struct ADAPTER *prAdapter);

static void mt6639WpdmaConfig(struct GLUE_INFO *prGlueInfo,
		u_int8_t enable, bool fgResetHif);

static void mt6639SetupMcuEmiAddr(struct ADAPTER *prAdapter);

static bool mt6639IsValidRegAccess(struct ADAPTER *prAdapter,
				   uint32_t u4Register);

static void mt6639WfdmaRxRingExtCtrl(
	struct GLUE_INFO *prGlueInfo,
	struct RTMP_RX_RING *rx_ring,
	u_int32_t index);

static void mt6639InitPcieInt(struct GLUE_INFO *prGlueInfo);
static void mt6639PcieHwControlVote(
	struct ADAPTER *prAdapter,
	uint8_t enable,
	uint32_t u4WifiUser);

#if CFG_SUPPORT_PCIE_ASPM
static u_int8_t mt6639SetL1ssEnable(struct ADAPTER *prAdapter, u_int role,
					u_int8_t fgEn);
static void mt6639ConfigPcieAspm(struct GLUE_INFO *prGlueInfo, u_int8_t fgEn,
				u_int enable_role);
static void mt6639UpdatePcieAspm(struct GLUE_INFO *prGlueInfo, u_int8_t fgEn);
static void mt6639KeepPcieWakeup(struct GLUE_INFO *prGlueInfo,
				u_int8_t fgWakeup);
static u_int8_t mt6639DumpPcieDateFlowStatus(struct GLUE_INFO *prGlueInfo);
#endif

static void mt6639ShowPcieDebugInfo(struct GLUE_INFO *prGlueInfo);

static u_int8_t mt6639_get_sw_interrupt_status(struct ADAPTER *prAdapter,
	uint32_t *pu4Status);

static u_int8_t mt6639_set_crypto(struct ADAPTER *prAdapter);
static void mt6639_ccif_notify_utc_time_to_fw(struct ADAPTER *ad,
	uint32_t sec,
	uint32_t usec);
static uint32_t mt6639_ccif_get_interrupt_status(struct ADAPTER *ad);
static void mt6639_ccif_set_fw_log_read_pointer(struct ADAPTER *ad,
	enum ENUM_FW_LOG_CTRL_TYPE type,
	uint32_t read_pointer);
static uint32_t mt6639_ccif_get_fw_log_read_pointer(struct ADAPTER *ad,
	enum ENUM_FW_LOG_CTRL_TYPE type);
static int32_t mt6639_ccif_trigger_fw_assert(struct ADAPTER *ad);

#if IS_MOBILE_SEGMENT
static int32_t mt6639_trigger_fw_assert(struct ADAPTER *prAdapter);
static uint32_t mt6639_mcu_init(struct ADAPTER *ad);
static void mt6639_mcu_deinit(struct ADAPTER *ad);
static int mt6639ConnacPccifOn(struct ADAPTER *prAdapter);
static int mt6639ConnacPccifOff(struct ADAPTER *prAdapter);
static int mt6639_CheckBusHang(void *priv, uint8_t rst_enable);
static uint32_t mt6639_wlanDownloadPatch(struct ADAPTER *prAdapter);
#endif
#endif

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

#if CFG_SUPPORT_PCIE_ASPM
static spinlock_t rPCIELock;
#define WIFI_ROLE	(1)
#define MD_ROLE		(2)
#define POLLING_TIMEOUT		(200)
#define DUMP_PCIE_CR	"0x1F_5004=0x%08x, 0x1F_500C=0x%08x,"\
		"0x1F_5014=0x%08x, 0x1F_5400=0x%08x, 0x1F_5404=0x%08x,"\
		"0x1F_6008=0x%08x, 0x1F_6000=0x%08x, 0x1F_6100=0x%08x,"\
		"0x1F_5300=0x%08x, 0x1F_6550=0x%08x, 0x1F_801C=0x%08x,"\
		"0x1D_0E48=0x%08x, 0x1D_0E40=0x%08x, 0x1D_0E44=0x%08x\n"
#endif


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
	{0x74030000, 0x1d0000, 0x2000},  /* PCIe MAC */
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
	{0x7c010000, 0x100000, 0x10000}, /* CONN_INFRA */
	{0x7c500000, MT6639_PCIE2AP_REMAP_BASE_ADDR, 0x2000000}, /* remap */
	{0x70020000, 0x1f0000, 0x9000},
	{0x70000000, 0x1e0000, 0x9000},
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
	{"P0T3:AP DATA3", WF_WFDMA_HOST_DMA0_WPDMA_TX_RING3_CTRL0_ADDR},
	{"P0T8:MD DATA0", WF_WFDMA_HOST_DMA0_WPDMA_TX_RING8_CTRL0_ADDR},
	{"P0T9:MD DATA1", WF_WFDMA_HOST_DMA0_WPDMA_TX_RING9_CTRL0_ADDR},
	{"P0T10:MD DATA2", WF_WFDMA_HOST_DMA0_WPDMA_TX_RING10_CTRL0_ADDR},
	{"P0T11:MD DATA3", WF_WFDMA_HOST_DMA0_WPDMA_TX_RING11_CTRL0_ADDR},
	{"P0T14:MD CMD", WF_WFDMA_HOST_DMA0_WPDMA_TX_RING14_CTRL0_ADDR},
	{"P0T15:AP CMD", WF_WFDMA_HOST_DMA0_WPDMA_TX_RING15_CTRL0_ADDR},
	{"P0T16:FWDL", WF_WFDMA_HOST_DMA0_WPDMA_TX_RING16_CTRL0_ADDR},
};

struct wfdma_group_info mt6639_wfmda_host_rx_group[] = {
	{"P0R4:AP DATA0", WF_WFDMA_HOST_DMA0_WPDMA_RX_RING4_CTRL0_ADDR},
	{"P0R6:AP EVENT", WF_WFDMA_HOST_DMA0_WPDMA_RX_RING6_CTRL0_ADDR},
	{"P0R5:AP DATA1", WF_WFDMA_HOST_DMA0_WPDMA_RX_RING5_CTRL0_ADDR},
	{"P0R7:AP TDONE", WF_WFDMA_HOST_DMA0_WPDMA_RX_RING7_CTRL0_ADDR},
	{"P0R8:MD DATA0", WF_WFDMA_HOST_DMA0_WPDMA_RX_RING8_CTRL0_ADDR},
	{"P0R9:MD DATA1", WF_WFDMA_HOST_DMA0_WPDMA_RX_RING9_CTRL0_ADDR},
	{"P0R10:MD EVENT", WF_WFDMA_HOST_DMA0_WPDMA_RX_RING10_CTRL0_ADDR},
	{"P0R11:MD TDONE", WF_WFDMA_HOST_DMA0_WPDMA_RX_RING11_CTRL0_ADDR},
};

struct wfdma_group_info mt6639_wfmda_wm_tx_group[] = {
	{"P0T6:LMAC TXD", WF_WFDMA_MCU_DMA0_WPDMA_TX_RING6_CTRL0_ADDR},
};

struct wfdma_group_info mt6639_wfmda_wm_rx_group[] = {
	{"P0R0:FWDL", WF_WFDMA_MCU_DMA0_WPDMA_RX_RING0_CTRL0_ADDR},
	{"P0R2:TXD0", WF_WFDMA_MCU_DMA0_WPDMA_RX_RING2_CTRL0_ADDR},
	{"P0R3:TXD1", WF_WFDMA_MCU_DMA0_WPDMA_RX_RING3_CTRL0_ADDR},
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
	{"conn_hif_host_int", mtk_pci_isr, mtk_pci_isr_thread, AP_INT, 0},
	{"conn_hif_host_int", mtk_pci_isr, mtk_pci_isr_thread, AP_INT, 0},
	{"conn_hif_host_int", mtk_pci_isr, mtk_pci_isr_thread, AP_INT, 0},
	{"conn_hif_host_int", mtk_pci_isr, mtk_pci_isr_thread, AP_INT, 0},
	{"conn_hif_host_int", mtk_pci_isr, mtk_pci_isr_thread, AP_INT, 0},
	{"conn_hif_host_int", mtk_pci_isr, mtk_pci_isr_thread, AP_INT, 0},
	{"conn_hif_host_int", mtk_pci_isr, mtk_pci_isr_thread, AP_INT, 0},
	{"conn_hif_host_int", mtk_pci_isr, mtk_pci_isr_thread, AP_INT, 0},
#if CFG_MTK_MDDP_SUPPORT
	{"conn_hif_md_int", mtk_md_dummy_pci_interrupt, NULL, MDDP_INT, 0},
	{"conn_hif_md_int", mtk_md_dummy_pci_interrupt, NULL, MDDP_INT, 0},
	{"conn_hif_md_int", mtk_md_dummy_pci_interrupt, NULL, MDDP_INT, 0},
	{"conn_hif_md_int", mtk_md_dummy_pci_interrupt, NULL, MDDP_INT, 0},
	{"conn_hif_md_int", mtk_md_dummy_pci_interrupt, NULL, MDDP_INT, 0},
	{"conn_hif_md_int", mtk_md_dummy_pci_interrupt, NULL, MDDP_INT, 0},
	{"conn_hif_md_int", mtk_md_dummy_pci_interrupt, NULL, MDDP_INT, 0},
	{"conn_hif_md_int", mtk_md_dummy_pci_interrupt, NULL, MDDP_INT, 0},
#else
	{"conn_hif_host_int", NULL, NULL, NONE_INT, 0},
	{"conn_hif_host_int", NULL, NULL, NONE_INT, 0},
	{"conn_hif_host_int", NULL, NULL, NONE_INT, 0},
	{"conn_hif_host_int", NULL, NULL, NONE_INT, 0},
	{"conn_hif_host_int", NULL, NULL, NONE_INT, 0},
	{"conn_hif_host_int", NULL, NULL, NONE_INT, 0},
	{"conn_hif_host_int", NULL, NULL, NONE_INT, 0},
	{"conn_hif_host_int", NULL, NULL, NONE_INT, 0},
#endif
	{"wm_conn2ap_wdt_irq", NULL, NULL, NONE_INT, 0},
	{"wf_mcu_jtag_det_eint", NULL, NULL, NONE_INT, 0},
	{"pmic_eint", NULL, NULL, NONE_INT, 0},
#if CFG_MTK_CCCI_SUPPORT
	{"ccif_bgf2ap_sw_irq", mtk_md_dummy_pci_interrupt, NULL, CCIF_INT, 0},
#else
	{"ccif_bgf2ap_sw_irq", NULL, NULL, NONE_INT, 0},
#endif
	{"ccif_wf2ap_sw_irq", pcie_sw_int_top_handler,
		pcie_sw_int_thread_handler, AP_MISC_INT},
#if CFG_MTK_CCCI_SUPPORT
	{"ccif_bgf2ap_irq_0", mtk_md_dummy_pci_interrupt, NULL, CCIF_INT, 0},
	{"ccif_bgf2ap_irq_1", mtk_md_dummy_pci_interrupt, NULL, CCIF_INT, 0},
#else
	{"ccif_bgf2ap_irq_0", NULL, NULL, NONE_INT, 0},
	{"ccif_bgf2ap_irq_1", NULL, NULL, NONE_INT, 0},
#endif
	{"reserved", NULL, NULL, NONE_INT, 0},
	{"reserved", NULL, NULL, NONE_INT, 0},
	{"reserved", NULL, NULL, NONE_INT, 0},
	{"reserved", NULL, NULL, NONE_INT, 0},
	{"drv_own_host_timeout_irq", pcie_drv_own_top_handler,
		pcie_drv_own_thread_handler, AP_INT, 0},
	{"drv_own_md_timeout_irq", mtk_md_dummy_pci_interrupt,
				 NULL, MDDP_INT, 0},
	{"fw_log_irq", pcie_fw_log_top_handler,
		pcie_fw_log_thread_handler, AP_MISC_INT, 0},
	{"reserved", NULL, NULL, NONE_INT, 0},
	{"reserved", NULL, NULL, NONE_INT, 0},
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
	 WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_3_MASK |
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
	.tx_ring3_data_idx = 3,
	.fw_own_clear_addr = CONNAC3X_BN0_IRQ_STAT_ADDR,
	.fw_own_clear_bit = PCIE_LPCR_FW_CLR_OWN,
	.fgCheckDriverOwnInt = TRUE,
	.u4DmaMask = 32,
	.wfmda_host_tx_group = mt6639_wfmda_host_tx_group,
	.wfmda_host_tx_group_len = ARRAY_SIZE(mt6639_wfmda_host_tx_group),
	.wfmda_host_rx_group = mt6639_wfmda_host_rx_group,
	.wfmda_host_rx_group_len = ARRAY_SIZE(mt6639_wfmda_host_rx_group),
	.wfmda_wm_tx_group = mt6639_wfmda_wm_tx_group,
	.wfmda_wm_tx_group_len = ARRAY_SIZE(mt6639_wfmda_wm_tx_group),
	.wfmda_wm_rx_group = mt6639_wfmda_wm_rx_group,
	.wfmda_wm_rx_group_len = ARRAY_SIZE(mt6639_wfmda_wm_rx_group),
	.prDmashdlCfg = &rMt6639DmashdlCfg,
	.prPleTopCr = &rMt6639PleTopCr,
	.prPseTopCr = &rMt6639PseTopCr,
	.prPpTopCr = &rMt6639PpTopCr,
	.prPseGroup = mt6639_pse_group,
	.u4PseGroupLen = ARRAY_SIZE(mt6639_pse_group),
	.pdmaSetup = mt6639WpdmaConfig,
	.enableInterrupt = mt6639EnableInterrupt,
	.disableInterrupt = mt6639DisableInterrupt,
	.configWfdmaIntMask = mt6639ConfigIntMask,
#if defined(_HIF_PCIE)
	.initPcieInt = mt6639InitPcieInt,
	.hwControlVote = mt6639PcieHwControlVote,
#if CFG_SUPPORT_PCIE_ASPM
	.configPcieAspm = mt6639ConfigPcieAspm,
	.updatePcieAspm = mt6639UpdatePcieAspm,
	.keepPcieWakeup = mt6639KeepPcieWakeup,
	.fgWifiEnL1_2 = TRUE,
	.fgMDEnL1_2 = TRUE,
	.dumpPcieStatus = mt6639DumpPcieDateFlowStatus,
#endif
	.pdmaStop = asicConnac3xWfdmaStop,
	.pdmaPollingIdle = asicConnac3xWfdmaPollingAllIdle,
	.pcie_msi_info = {
		.prMsiLayout = mt6639_pcie_msi_layout,
		.u4MaxMsiNum = ARRAY_SIZE(mt6639_pcie_msi_layout),
	},
	.showDebugInfo = mt6639ShowPcieDebugInfo,
	.disableDevice = mtk_pci_disable_device,
#endif /* _HIF_PCIE */
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
	.u4SuspendVer = SUSPEND_V2,
	.asicUsbResume = asicConnac3xUsbResume,
	.asicUsbEventEpDetected = asicConnac3xUsbEventEpDetected,
	.asicUsbRxByteCount = asicConnac3xUsbRxByteCount,
	.asicUdmaRxFlush = asicConnac3xUdmaRxFlush,
#endif
};

#if CFG_ENABLE_FW_DOWNLOAD
struct FWDL_OPS_T mt6639_fw_dl_ops = {
	.constructFirmwarePrio = mt6639_ConstructFirmwarePrio,
	.constructPatchName = mt6639_ConstructPatchName,
#if CFG_SUPPORT_SINGLE_FW_BINARY
	.parseSingleBinaryFile = wlanParseSingleBinaryFile,
#endif
	.downloadPatch = mt6639_wlanDownloadPatch,
	.downloadFirmware = wlanConnacFormatDownload,
	.downloadByDynMemMap = NULL,
	.getFwInfo = wlanGetConnacFwInfo,
	.getFwDlInfo = asicGetFwDlInfo,
	.downloadEMI = wlanDownloadEMISectionViaDma,
#if (CFG_SUPPORT_PRE_ON_PHY_ACTION == 1)
	.phyAction = wlanPhyAction,
#else
	.phyAction = NULL,
#endif
#if defined(_HIF_PCIE) && IS_MOBILE_SEGMENT
	.mcu_init = mt6639_mcu_init,
	.mcu_deinit = mt6639_mcu_deinit,
#endif
#if CFG_SUPPORT_WIFI_DL_BT_PATCH
	.constructBtPatchName = asicConnac3xConstructBtPatchName,
	.downloadBtPatch = asicConnac3xDownloadBtPatch,
#if (CFG_SUPPORT_CONNAC3X == 1)
	.configBtImageSection = asicConnac3xConfigBtImageSection,
#endif
#endif
	.getFwVerInfo = wlanParseRamCodeReleaseManifest,
};
#endif /* CFG_ENABLE_FW_DOWNLOAD */

struct TX_DESC_OPS_T mt6639_TxDescOps = {
	.fillNicAppend = fillNicTxDescAppend,
	.fillHifAppend = fillTxDescAppendByHostV2,
	.fillTxByteCount = fillConnac3xTxDescTxByteCount,
};

struct RX_DESC_OPS_T mt6639_RxDescOps = {0};

struct CHIP_DBG_OPS mt6639_DebugOps = {
#if defined(_HIF_PCIE) || defined(_HIF_AXI)
	.showPdmaInfo = connac3x_show_wfdma_info,
#endif
	.showPseInfo = connac3x_show_pse_info,
	.showPleInfo = connac3x_show_ple_info,
	.showTxdInfo = connac3x_show_txd_Info,
	.showWtblInfo = connac3x_show_wtbl_info,
	.showUmacWtblInfo = connac3x_show_umac_wtbl_info,
	.showCsrInfo = NULL,
#if defined(_HIF_PCIE) || defined(_HIF_AXI)
	.showDmaschInfo = connac3x_show_dmashdl_info,
	.getFwDebug = NULL,
	.setFwDebug = connac3x_set_ple_int_no_read,
#endif
	.showHifInfo = NULL,
	.printHifDbgInfo = NULL,
	.show_rx_rate_info = connac3x_show_rx_rate_info,
	.show_rx_rssi_info = connac3x_show_rx_rssi_info,
	.show_stat_info = connac3x_show_stat_info,
	.get_tx_info_from_txv = connac3x_get_tx_info_from_txv,
#if (CFG_SUPPORT_802_11BE_MLO == 1)
	.show_mld_info = connac3x_show_mld_info,
#endif
#if defined(_HIF_PCIE) || defined(_HIF_AXI)
	.show_wfdma_dbg_probe_info = mt6639_show_wfdma_dbg_probe_info,
	.show_wfdma_wrapper_info = mt6639_show_wfdma_wrapper_info,
	.dumpwfsyscpupcr = mt6639_dumpWfsyscpupcr,
	.dumpBusHangCr = mt6639_DumpBusHangCr,
#if IS_ENABLED(CFG_MTK_WIFI_CONNV3_SUPPORT)
	.dumpPcieCr = mt6639_dumpPcieReg,
#endif
#endif
#if CFG_SUPPORT_LINK_QUALITY_MONITOR
	.get_rx_rate_info = mt6639_get_rx_rate_info,
#endif
#if CFG_SUPPORT_LLS
	.get_rx_link_stats = mt6639_get_rx_link_stats,
#endif
	.dumpTxdInfo = connac3x_dump_tmac_info,
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

#if defined(_HIF_PCIE)
#if IS_MOBILE_SEGMENT
static struct CCIF_OPS mt6639_ccif_ops = {
	.get_interrupt_status = mt6639_ccif_get_interrupt_status,
	.notify_utc_time_to_fw = mt6639_ccif_notify_utc_time_to_fw,
	.set_fw_log_read_pointer = mt6639_ccif_set_fw_log_read_pointer,
	.get_fw_log_read_pointer = mt6639_ccif_get_fw_log_read_pointer,
	.trigger_fw_assert = mt6639_ccif_trigger_fw_assert,
};

#if IS_ENABLED(CFG_MTK_WIFI_FW_LOG_MMIO)
static struct FW_LOG_OPS mt6639_fw_log_mmio_ops = {
	.init = fwLogMmioInitMcu,
	.deinit = fwLogMmioDeInitMcu,
	.start = fwLogMmioStart,
	.stop = fwLogMmioStop,
	.handler = fwLogMmioHandler,
};
#endif

#if IS_ENABLED(CFG_MTK_WIFI_FW_LOG_EMI)
static struct FW_LOG_OPS mt6639_fw_log_emi_ops = {
	.init = fw_log_emi_init,
	.deinit = fw_log_emi_deinit,
	.start = fw_log_emi_start,
	.stop = fw_log_emi_stop,
	.handler = fw_log_emi_handler,
};
#endif
#endif
#endif

#if CFG_SUPPORT_THERMAL_QUERY
struct thermal_sensor_info mt6639_thermal_sensor_info[] = {
	{"wifi_adie_0", THERMAL_TEMP_TYPE_ADIE, 0},
	{"wifi_ddie_0", THERMAL_TEMP_TYPE_DDIE, 0},
	{"wifi_ddie_1", THERMAL_TEMP_TYPE_DDIE, 1},
	{"wifi_ddie_2", THERMAL_TEMP_TYPE_DDIE, 2},
	{"wifi_ddie_3", THERMAL_TEMP_TYPE_DDIE, 3},
};
#endif

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
#if defined(_HIF_PCIE)
#if (CFG_SUPPORT_HOST_OFFLOAD == 1)
	.is_support_mawd = TRUE,
	.is_support_sdo = TRUE,
	.is_support_rro = TRUE,
#endif /* CFG_SUPPORT_HOST_OFFLOAD == 1 */
	.is_en_wfdma_no_mmio_read = TRUE,
#endif /* _HIF_PCIE */
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
#if IS_CE_SEGMENT
	.downloadBufferBin = wlanConnac3XDownloadBufferBin,
#else
	.downloadBufferBin = NULL,
#endif
	.is_support_hw_amsdu = TRUE,
	.is_support_nvram_fragment = TRUE,
	.is_support_asic_lp = TRUE,
	.asicWfdmaReInit = asicConnac3xWfdmaReInit,
	.asicWfdmaReInit_handshakeInit = asicConnac3xWfdmaDummyCrWrite,
	.group5_size = sizeof(struct HW_MAC_RX_STS_GROUP_5),
	.u4LmacWtblDUAddr = CONNAC3X_WIFI_LWTBL_BASE,
	.u4UmacWtblDUAddr = CONNAC3X_WIFI_UWTBL_BASE,
#if IS_ENABLED(CFG_MTK_WIFI_CONNV3_SUPPORT)
	.coexpccifon = mt6639ConnacPccifOn,
	.coexpccifoff = mt6639ConnacPccifOff,
#endif
#if CFG_MTK_MDDP_SUPPORT
	.isSupportMddpAOR = false,
	.isSupportMddpSHM = true,
#else
	.isSupportMddpAOR = false,
	.isSupportMddpSHM = false,
#endif
	.cmd_max_pkt_size = CFG_TX_MAX_PKT_SIZE, /* size 1600 */
#if defined(CFG_MTK_WIFI_PMIC_QUERY)
	.queryPmicInfo = asicConnac3xQueryPmicInfo,
#endif
#if defined(_HIF_USB)
	.asicUsbInit = asicConnac3xWfdmaInitForUSB,
	.asicUsbInit_ic_specific = NULL,
	.u4SerUsbMcuEventAddr = WF_SW_DEF_CR_USB_MCU_EVENT_ADDR,
	.u4SerUsbHostAckAddr = WF_SW_DEF_CR_USB_HOST_ACK_ADDR,
#endif
#if defined(_HIF_PCIE)
#if IS_MOBILE_SEGMENT
	.chip_capability = BIT(CHIP_CAPA_FW_LOG_TIME_SYNC) |
		BIT(CHIP_CAPA_FW_LOG_TIME_SYNC_BY_CCIF) |
		BIT(CHIP_CAPA_XTAL_TRIM),
	.checkbushang = mt6639_CheckBusHang,
	.rEmiInfo = {
#if CFG_MTK_ANDROID_EMI
		.type = EMI_ALLOC_TYPE_LK,
		.coredump_size = (7 * 1024 * 1024),
#else
		.type = EMI_ALLOC_TYPE_IN_DRIVER,
#endif /* CFG_MTK_ANDROID_EMI */
	},
#if CFG_SUPPORT_THERMAL_QUERY
	.thermal_info = {
		.sensor_num = ARRAY_SIZE(mt6639_thermal_sensor_info),
		.sensor_info = mt6639_thermal_sensor_info,
	},
#endif
	.trigger_fw_assert = mt6639_trigger_fw_assert,
	.fw_log_info = {
#if IS_ENABLED(CFG_MTK_WIFI_FW_LOG_MMIO)
		.ops = &mt6639_fw_log_mmio_ops,
#endif
#if IS_ENABLED(CFG_MTK_WIFI_FW_LOG_EMI)
		.base = 0x538000,
		.ops = &mt6639_fw_log_emi_ops,
#endif
		.path = ENUM_LOG_READ_POINTER_PATH_CCIF,
	},
#else
	.chip_capability = BIT(CHIP_CAPA_FW_LOG_TIME_SYNC) |
		BIT(CHIP_CAPA_XTAL_TRIM),
#endif
	.ccif_ops = &mt6639_ccif_ops,
	.get_sw_interrupt_status = mt6639_get_sw_interrupt_status,
#else
	.chip_capability = BIT(CHIP_CAPA_FW_LOG_TIME_SYNC) |
		BIT(CHIP_CAPA_XTAL_TRIM),
#endif /* _HIF_PCIE */
	.custom_oid_interface_version = MTK_CUSTOM_OID_INTERFACE_VERSION,
	.em_interface_version = MTK_EM_INTERFACE_VERSION,

	.u4MinTxLen = 2,
	.setCrypto = mt6639_set_crypto,
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
	uint8_t aucFlavor[CFG_FW_FLAVOR_MAX_LEN];

	kalMemZero(aucFlavor, sizeof(aucFlavor));
	mt6639GetFlavorVer(&aucFlavor[0]);

#if CFG_SUPPORT_SINGLE_FW_BINARY
	/* Type 0. connsys_wifi_mt6639.bin */
	ret = kalSnprintf(*(apucName + (*pucNameIdx)),
			CFG_FW_NAME_MAX_LEN,
			"connsys_wifi_mt6639.bin");
	if (ret >= 0 && ret < CFG_FW_NAME_MAX_LEN)
		(*pucNameIdx) += 1;
	else
		DBGLOG(INIT, ERROR,
			"[%u] kalSnprintf failed, ret: %d\n",
			__LINE__, ret);

	/* Type 1. connsys_wifi_mt6639_flavor.bin */
	ret = kalSnprintf(*(apucName + (*pucNameIdx)),
			CFG_FW_NAME_MAX_LEN,
			"connsys_wifi_mt6639_%s.bin",
			aucFlavor);
	if (ret >= 0 && ret < CFG_FW_NAME_MAX_LEN)
		(*pucNameIdx) += 1;
	else
		DBGLOG(INIT, ERROR,
			"[%u] kalSnprintf failed, ret: %d\n",
			__LINE__, ret);
#endif

	/* Type 2. WIFI_RAM_CODE_MT6639_1_1.bin */
	ret = kalSnprintf(*(apucName + (*pucNameIdx)),
			CFG_FW_NAME_MAX_LEN,
			"WIFI_RAM_CODE_MT%x_%s_%u.bin",
			MT6639_CHIP_ID,
			aucFlavor,
			MT6639_ROM_VERSION);
	if (ret >= 0 && ret < CFG_FW_NAME_MAX_LEN)
		(*pucNameIdx) += 1;
	else
		DBGLOG(INIT, ERROR,
			"[%u] kalSnprintf failed, ret: %d\n",
			__LINE__, ret);

	for (ucIdx = 0; apucmt6639FwName[ucIdx]; ucIdx++) {
		if ((*pucNameIdx + 3) >= ucMaxNameIdx) {
			/* the table is not large enough */
			DBGLOG(INIT, ERROR,
				"kalFirmwareImageMapping >> file name array is not enough.\n");
			ASSERT(0);
			continue;
		}

		/* Type 3. WIFI_RAM_CODE_6639.bin */
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
	uint8_t aucFlavor[CFG_FW_FLAVOR_MAX_LEN];

	kalMemZero(aucFlavor, sizeof(aucFlavor));
	mt6639GetFlavorVer(&aucFlavor[0]);

#if CFG_SUPPORT_SINGLE_FW_BINARY
	/* Type 0. connsys_wifi_mt6639.bin */
	ret = kalSnprintf(*(apucName + (*pucNameIdx)),
			CFG_FW_NAME_MAX_LEN,
			"connsys_wifi_mt6639.bin");
	if (ret >= 0 && ret < CFG_FW_NAME_MAX_LEN)
		(*pucNameIdx) += 1;
	else
		DBGLOG(INIT, ERROR,
			"[%u] kalSnprintf failed, ret: %d\n",
			__LINE__, ret);

	/* Type 1. connsys_wifi_mt6639_flavor.bin */
	ret = kalSnprintf(*(apucName + (*pucNameIdx)),
			CFG_FW_NAME_MAX_LEN,
			"connsys_wifi_mt6639_%s.bin",
			aucFlavor);
	if (ret >= 0 && ret < CFG_FW_NAME_MAX_LEN)
		(*pucNameIdx) += 1;
	else
		DBGLOG(INIT, ERROR,
			"[%u] kalSnprintf failed, ret: %d\n",
			__LINE__, ret);
#endif

	/* Type 2. WIFI_MT6639_PATCH_MCU_1_1_hdr.bin */
	ret = kalSnprintf(apucName[(*pucNameIdx)],
			  CFG_FW_NAME_MAX_LEN,
			  "WIFI_MT%x_PATCH_MCU_%s_%u_hdr.bin",
			  MT6639_CHIP_ID,
			  aucFlavor,
			  MT6639_ROM_VERSION);
	if (ret >= 0 && ret < CFG_FW_NAME_MAX_LEN)
		(*pucNameIdx) += 1;
	else
		DBGLOG(INIT, ERROR,
			"[%u] kalSnprintf failed, ret: %d\n",
			__LINE__, ret);

	/* Type 3. mt6639_patch_e1_hdr.bin */
	ret = kalSnprintf(apucName[(*pucNameIdx)],
			  CFG_FW_NAME_MAX_LEN,
			  "mt6639_patch_e1_hdr.bin");
	if (ret < 0 || ret >= CFG_FW_NAME_MAX_LEN)
		DBGLOG(INIT, ERROR,
			"[%u] kalSnprintf failed, ret: %d\n",
			__LINE__, ret);
}

#if defined(_HIF_PCIE)
static uint8_t mt6639SetRxRingHwAddr(struct RTMP_RX_RING *prRxRing,
		struct BUS_INFO *prBusInfo, uint32_t u4SwRingIdx)
{
	uint32_t offset = 0;

	/*
	 * RX_RING_DATA0   (RX_Ring0) - Band0 Rx Data
	 * RX_RING_DATA1 (RX_Ring1) - Band1 Rx Data
	 * RX_RING_EVT    (RX_Ring2) - Band0 Tx Free Done Event / Rx Event
	 * RX_RING_TXDONE0 (RX_Ring3) - Band1 Tx Free Done Event
	*/
	switch (u4SwRingIdx) {
	case RX_RING_EVT:
		offset = 6;
		break;
	case RX_RING_DATA0:
		offset = 4;
		break;
	case RX_RING_DATA1:
		offset = 5;
		break;
	case RX_RING_TXDONE0:
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
	/* Port idx sanity */
	if (RX_RING_DATA1 >= NUM_OF_RX_RING) {
		DBGLOG(HAL, ERROR, "Invalid P[%u]\n", RX_RING_DATA1);
		return false;
	}

	/* Band1 Data Rx path */
	if (!halWpdmaAllocRxRing(prGlueInfo,
			RX_RING_DATA1, RX_RING0_SIZE,
			RXD_SIZE, CFG_RX_MAX_PKT_SIZE, fgAllocMem)) {
		DBGLOG(HAL, ERROR, "AllocRxRing[2] fail\n");
		return false;
	}

	/* Port idx sanity */
	if (RX_RING_TXDONE0 >= NUM_OF_RX_RING) {
		DBGLOG(HAL, ERROR, "Invalid P[%u]\n", RX_RING_TXDONE0);
		return false;
	}

	/* Band0 Tx Free Done Event */
	if (!halWpdmaAllocRxRing(prGlueInfo,
			RX_RING_TXDONE0, RX_RING1_SIZE,
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

	if (u4Sta & WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_16_MASK)
		halWpdmaProcessCmdDmaDone(
			prAdapter->prGlueInfo, TX_RING_FWDL);

	if (u4Sta & WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_15_MASK)
		halWpdmaProcessCmdDmaDone(
			prAdapter->prGlueInfo, TX_RING_CMD);

#if (CFG_SUPPORT_DISABLE_DATA_DDONE_INTR == 0)
	if (u4Sta & WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_0_MASK) {
		halWpdmaProcessDataDmaDone(
			prAdapter->prGlueInfo, TX_RING_DATA0);
		kalSetTxEvent2Hif(prAdapter->prGlueInfo);
	}

	if (u4Sta & WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_1_MASK) {
		halWpdmaProcessDataDmaDone(
			prAdapter->prGlueInfo, TX_RING_DATA1);
		kalSetTxEvent2Hif(prAdapter->prGlueInfo);
	}

	if (u4Sta & WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_2_MASK) {
		halWpdmaProcessDataDmaDone(
			prAdapter->prGlueInfo, TX_RING_DATA_PRIO);
		kalSetTxEvent2Hif(prAdapter->prGlueInfo);
	}

	if (u4Sta & WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_3_MASK) {
		halWpdmaProcessDataDmaDone(
			prAdapter->prGlueInfo, TX_RING_DATA_ALTX);
		kalSetTxEvent2Hif(prAdapter->prGlueInfo);
	}
#endif /* CFG_SUPPORT_DISABLE_DATA_DDONE_INTR == 0 */
}

static void mt6639ProcessRxDataInterrupt(struct ADAPTER *prAdapter)
{
	struct GLUE_INFO *prGlueInfo = prAdapter->prGlueInfo;
	struct GL_HIF_INFO *prHifInfo = &prGlueInfo->rHifInfo;
#if defined(_HIF_PCIE) || defined(_HIF_AXI)
#if (CFG_SUPPORT_HOST_OFFLOAD == 1)
	struct WIFI_VAR *prWifiVar = &prAdapter->rWifiVar;
#endif /* CFG_SUPPORT_HOST_OFFLOAD == 1 */
#endif
	uint32_t u4Sta = prHifInfo->u4IntStatus;

#if defined(_HIF_PCIE) || defined(_HIF_AXI)
#if (CFG_SUPPORT_HOST_OFFLOAD == 1)
	if (IS_FEATURE_ENABLED(prWifiVar->fgEnableRro)) {
		if (prHifInfo->u4OffloadIntStatus ||
		    (KAL_TEST_BIT(RX_RRO_DATA, prAdapter->ulNoMoreRfb)))
			halRroReadRxData(prAdapter);
	} else
#endif /* CFG_SUPPORT_HOST_OFFLOAD == 1 */
#endif
	{
		if ((u4Sta &
		     WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_4_MASK) ||
		    (KAL_TEST_BIT(RX_RING_DATA0, prAdapter->ulNoMoreRfb)))
			halRxReceiveRFBs(prAdapter, RX_RING_DATA0, TRUE);

		if ((u4Sta &
		     WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_5_MASK) ||
		    (KAL_TEST_BIT(RX_RING_DATA1, prAdapter->ulNoMoreRfb)))
			halRxReceiveRFBs(prAdapter, RX_RING_DATA1, TRUE);
	}
}

static void mt6639ProcessRxInterrupt(struct ADAPTER *prAdapter)
{
	struct GLUE_INFO *prGlueInfo = prAdapter->prGlueInfo;
	struct GL_HIF_INFO *prHifInfo = &prGlueInfo->rHifInfo;
	uint32_t u4Sta = prHifInfo->u4IntStatus;

	if ((u4Sta & WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_6_MASK) ||
	    (KAL_TEST_BIT(RX_RING_EVT, prAdapter->ulNoMoreRfb)))
		halRxReceiveRFBs(prAdapter, RX_RING_EVT, FALSE);

	if ((u4Sta & WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_7_MASK) ||
	    (KAL_TEST_BIT(RX_RING_TXDONE0, prAdapter->ulNoMoreRfb)))
		halRxReceiveRFBs(prAdapter, RX_RING_TXDONE0, FALSE);

	mt6639ProcessRxDataInterrupt(prAdapter);
}

static void mt6639SetMDTRXRingPriorityInterrupt(struct ADAPTER *prAdapter)
{
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA0_WPDMA_INT_RX_PRI_SEL_ADDR, 0xF00);
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA0_WPDMA_INT_TX_PRI_SEL_ADDR, 0x7F00);
}

static void mt6639WfdmaManualPrefetch(
	struct GLUE_INFO *prGlueInfo)
{
	struct ADAPTER *prAdapter = prGlueInfo->prAdapter;
	u_int32_t val = 0;
	uint32_t u4WrVal = 0, u4Addr = 0;
	uint32_t u4PrefetchCnt = 0x4, u4TxDataPrefetchCnt = 0x10;
	uint32_t u4PrefetchBase = 0x00400000, u4TxDataPrefetchBase = 0x01000000;

	HAL_MCR_RD(prAdapter, WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_ADDR, &val);
	/* disable prefetch offset calculation auto-mode */
	val &=
	~WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_CSR_DISP_BASE_PTR_CHAIN_EN_MASK;
	HAL_MCR_WR(prAdapter, WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_ADDR, val);

	/* Rx ring */
	for (u4Addr = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING4_EXT_CTRL_ADDR;
	     u4Addr <= WF_WFDMA_HOST_DMA0_WPDMA_RX_RING7_EXT_CTRL_ADDR;
	     u4Addr += 0x4) {
		u4WrVal = (u4WrVal & 0xFFFF0000) | u4PrefetchCnt;
		HAL_MCR_WR(prAdapter, u4Addr, u4WrVal);
		u4WrVal += u4PrefetchBase;
	}

#if CFG_MTK_MDDP_SUPPORT
	for (u4Addr = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING8_EXT_CTRL_ADDR;
	     u4Addr <= WF_WFDMA_HOST_DMA0_WPDMA_RX_RING11_EXT_CTRL_ADDR;
	     u4Addr += 0x4) {
		u4WrVal = (u4WrVal & 0xFFFF0000) | u4PrefetchCnt;
		HAL_MCR_WR(prAdapter, u4Addr, u4WrVal);
		u4WrVal += u4PrefetchBase;
	}
#endif

	/* Tx ring */
	for (u4Addr = WF_WFDMA_HOST_DMA0_WPDMA_TX_RING0_EXT_CTRL_ADDR;
	     u4Addr <= WF_WFDMA_HOST_DMA0_WPDMA_TX_RING1_EXT_CTRL_ADDR;
	     u4Addr += 0x4) {
		u4WrVal = (u4WrVal & 0xFFFF0000) | u4TxDataPrefetchCnt;
		HAL_MCR_WR(prAdapter, u4Addr, u4WrVal);
		u4WrVal += u4TxDataPrefetchBase;
	}

	for (u4Addr = WF_WFDMA_HOST_DMA0_WPDMA_TX_RING2_EXT_CTRL_ADDR;
	     u4Addr <= WF_WFDMA_HOST_DMA0_WPDMA_TX_RING3_EXT_CTRL_ADDR;
	     u4Addr += 0x4) {
		u4WrVal = (u4WrVal & 0xFFFF0000) | u4PrefetchCnt;
		HAL_MCR_WR(prAdapter, u4Addr, u4WrVal);
		u4WrVal += u4PrefetchBase;
	}

#if CFG_MTK_MDDP_SUPPORT
	for (u4Addr = WF_WFDMA_HOST_DMA0_WPDMA_TX_RING8_EXT_CTRL_ADDR;
	     u4Addr <= WF_WFDMA_HOST_DMA0_WPDMA_TX_RING9_EXT_CTRL_ADDR;
	     u4Addr += 0x4) {
		u4WrVal = (u4WrVal & 0xFFFF0000) | u4TxDataPrefetchCnt;
		HAL_MCR_WR(prAdapter, u4Addr, u4WrVal);
		u4WrVal += u4TxDataPrefetchBase;
	}

	for (u4Addr = WF_WFDMA_HOST_DMA0_WPDMA_TX_RING10_EXT_CTRL_ADDR;
	     u4Addr <= WF_WFDMA_HOST_DMA0_WPDMA_TX_RING11_EXT_CTRL_ADDR;
	     u4Addr += 0x4) {
		u4WrVal = (u4WrVal & 0xFFFF0000) | u4PrefetchCnt;
		HAL_MCR_WR(prAdapter, u4Addr, u4WrVal);
		u4WrVal += u4PrefetchBase;
	}
	u4Addr = WF_WFDMA_HOST_DMA0_WPDMA_TX_RING14_EXT_CTRL_ADDR;
	u4WrVal = (u4WrVal & 0xFFFF0000) | u4PrefetchCnt;
	HAL_MCR_WR(prAdapter, u4Addr, u4WrVal);
	u4WrVal += u4PrefetchBase;
#endif

	for (u4Addr = WF_WFDMA_HOST_DMA0_WPDMA_TX_RING15_EXT_CTRL_ADDR;
	     u4Addr <= WF_WFDMA_HOST_DMA0_WPDMA_TX_RING16_EXT_CTRL_ADDR;
	     u4Addr += 0x4) {
		u4WrVal = (u4WrVal & 0xFFFF0000) | u4PrefetchCnt;
		HAL_MCR_WR(prAdapter, u4Addr, u4WrVal);
		u4WrVal += u4PrefetchBase;
	}

	mt6639SetMDTRXRingPriorityInterrupt(prAdapter);

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
	struct mt66xx_chip_info *prChipInfo = prAdapter->chip_info;
	struct BUS_INFO *prBusInfo = prChipInfo->bus_info;
#if (CFG_SUPPORT_HOST_OFFLOAD == 1)
	struct WIFI_VAR *prWifiVar = &prAdapter->rWifiVar;
#endif /* CFG_SUPPORT_HOST_OFFLOAD == 1 */
	uint32_t u4RegValue = 0, u4WrValue = 0, u4Addr;

	*pu4IntStatus = 0;

	u4Addr = WF_WFDMA_HOST_DMA0_HOST_INT_STA_ADDR;
	HAL_MCR_RD(prAdapter, u4Addr, &u4RegValue);

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

	prHifInfo->u4IntStatus = u4RegValue;

	/* clear interrupt */
	HAL_MCR_WR(prAdapter, u4Addr, u4WrValue);

#if (CFG_SUPPORT_HOST_OFFLOAD == 1)
	if (IS_FEATURE_ENABLED(prWifiVar->fgEnableRro)) {
		if (IS_FEATURE_ENABLED(prWifiVar->fgEnableMawd)) {
			u4Addr = MAWD_AP_INTERRUPT_SETTING0;
			HAL_MCR_RD(prAdapter, u4Addr, &u4RegValue);
			if (u4RegValue & BIT(0))
				*pu4IntStatus |= WHISR_RX0_DONE_INT;
			u4Addr = MAWD_AP_INTERRUPT_SETTING1;
		} else {
			u4Addr = WF_RRO_TOP_HOST_INT_STS_ADDR;
			HAL_MCR_RD(prAdapter, u4Addr, &u4RegValue);
			if (u4RegValue &
			    WF_RRO_TOP_HOST_INT_STS_HOST_RRO_DONE_INT_MASK)
				*pu4IntStatus |= WHISR_RX0_DONE_INT;
		}
		prHifInfo->u4OffloadIntStatus = u4RegValue;
		HAL_MCR_WR(prAdapter, u4Addr, u4RegValue);
	}
#endif /* CFG_SUPPORT_HOST_OFFLOAD == 1 */
}

static void mt6639ConfigIntMask(struct GLUE_INFO *prGlueInfo,
		u_int8_t enable)
{
	struct ADAPTER *prAdapter = prGlueInfo->prAdapter;
	struct mt66xx_chip_info *prChipInfo;
	struct WIFI_VAR *prWifiVar;
	uint32_t u4Addr = 0, u4WrVal = 0;

	prChipInfo = prAdapter->chip_info;
	prWifiVar = &prAdapter->rWifiVar;

	u4Addr = enable ? WF_WFDMA_HOST_DMA0_HOST_INT_ENA_SET_ADDR :
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_CLR_ADDR;
	u4WrVal =
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_HOST_RX_DONE_INT_ENA6_MASK |
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_HOST_RX_DONE_INT_ENA7_MASK |
#if (CFG_SUPPORT_DISABLE_DATA_DDONE_INTR == 0)
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_HOST_TX_DONE_INT_ENA0_MASK |
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_HOST_TX_DONE_INT_ENA1_MASK |
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_HOST_TX_DONE_INT_ENA2_MASK |
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_HOST_TX_DONE_INT_ENA3_MASK
#endif /* CFG_SUPPORT_DISABLE_DATA_DDONE_INTR == 0 */
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_HOST_TX_DONE_INT_ENA15_MASK |
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_HOST_TX_DONE_INT_ENA16_MASK |
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_mcu2host_sw_int_ena_MASK;

#if (CFG_SUPPORT_HOST_OFFLOAD == 1)
	if (IS_FEATURE_ENABLED(prWifiVar->fgEnableRro)) {
		if (!(IS_FEATURE_ENABLED(prWifiVar->fgEnableMawd))) {
			u4WrVal |=
			WF_WFDMA_HOST_DMA0_HOST_INT_ENA_subsys_int_ena_MASK;
		}
	}
#endif /* CFG_SUPPORT_HOST_OFFLOAD == 1 */

	u4WrVal |=
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_HOST_RX_DONE_INT_ENA4_MASK |
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_HOST_RX_DONE_INT_ENA5_MASK;

	HAL_MCR_WR(prGlueInfo->prAdapter, u4Addr, u4WrVal);
}

static void mt6639EnableInterrupt(struct ADAPTER *prAdapter)
{
	asicConnac3xEnablePlatformIRQ(prAdapter);
}

static void mt6639DisableInterrupt(struct ADAPTER *prAdapter)
{
	asicConnac3xDisablePlatformIRQ(prAdapter);
}

static void mt6639WpdmaMsiConfig(struct ADAPTER *prAdapter)
{
#define WFDMA_AP_MSI_NUM		1
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

#if CFG_MTK_MDDP_SUPPORT
#if (WFDMA_MD_MSI_NUM == 1)
	u4Value = 0x0F00087F;
#else
	u4Value = 0x0000083C;
#endif
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_EXT_WRAP_CSR_WFDMA_MD_INT_LUMP_SEL,
		u4Value);
#endif
}

static void mt6639ConfigWfdmaRxRingThreshold(struct ADAPTER *prAdapter)
{
	if (!prAdapter)
		return;

	HAL_MCR_WR(prAdapter, WF_WFDMA_HOST_DMA0_WPDMA_PAUSE_RX_Q_TH10_ADDR,
		0x20002);
	HAL_MCR_WR(prAdapter, WF_WFDMA_HOST_DMA0_WPDMA_PAUSE_RX_Q_TH32_ADDR,
		0x20002);
	HAL_MCR_WR(prAdapter, WF_WFDMA_HOST_DMA0_WPDMA_PAUSE_RX_Q_TH54_ADDR,
		0x20002);
	HAL_MCR_WR(prAdapter, WF_WFDMA_HOST_DMA0_WPDMA_PAUSE_RX_Q_TH76_ADDR,
		0x20002);
	HAL_MCR_WR(prAdapter, WF_WFDMA_HOST_DMA0_WPDMA_PAUSE_RX_Q_TH98_ADDR,
		0x20002);
	HAL_MCR_WR(prAdapter, WF_WFDMA_HOST_DMA0_WPDMA_PAUSE_RX_Q_TH1110_ADDR,
		0x20002);

	DBGLOG(HAL, TRACE, "Set WFDMA Rx que threshold to 0x20002\n");
}

static void mt6639WpdmaConfig(struct GLUE_INFO *prGlueInfo,
		u_int8_t enable, bool fgResetHif)
{
	struct ADAPTER *prAdapter = prGlueInfo->prAdapter;
#if (CFG_SUPPORT_HOST_OFFLOAD == 1)
	struct WIFI_VAR *prWifiVar = &prAdapter->rWifiVar;
#endif
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
		mt6639ConfigWfdmaRxRingThreshold(prAdapter);
	}

#if (CFG_SUPPORT_HOST_OFFLOAD == 1)
	if (IS_FEATURE_ENABLED(prWifiVar->fgEnableSdo)) {
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

	/* Enable RX periodic delayed interrupt 0x20 * 20us */
	HAL_MCR_WR(prAdapter,
		   WF_WFDMA_HOST_DMA0_HOST_PER_DLY_INT_CFG_ADDR,
		   0xF00020);
}

static void mt6639WfdmaRxRingExtCtrl(
	struct GLUE_INFO *prGlueInfo,
	struct RTMP_RX_RING *prRxRing,
	u_int32_t index)
{
	struct ADAPTER *prAdapter;
	struct mt66xx_chip_info *prChipInfo;
	struct BUS_INFO *prBusInfo;
#if (CFG_SUPPORT_HOST_OFFLOAD == 1)
	struct WIFI_VAR *prWifiVar;
#endif /* CFG_SUPPORT_HOST_OFFLOAD == 1 */
	uint32_t ext_offset;

	prAdapter = prGlueInfo->prAdapter;
	prChipInfo = prAdapter->chip_info;
	prBusInfo = prChipInfo->bus_info;
#if (CFG_SUPPORT_HOST_OFFLOAD == 1)
	prWifiVar = &prAdapter->rWifiVar;
#endif /* CFG_SUPPORT_HOST_OFFLOAD == 1 */

	switch (index) {
	case RX_RING_EVT:
		ext_offset = 6 * 4;
		break;
	case RX_RING_DATA0:
		ext_offset = 4 * 4;
		break;
	case RX_RING_DATA1:
		ext_offset = 5 * 4;
		break;
	case RX_RING_TXDONE0:
		ext_offset = 7 * 4;
		break;
	default:
		DBGLOG(RX, ERROR, "Error index=%d\n", index);
		return;
	}

	prRxRing->hw_desc_base_ext =
		prBusInfo->host_rx_ring_ext_ctrl_base + ext_offset;

	HAL_MCR_WR(prAdapter, prRxRing->hw_desc_base_ext,
		   CONNAC3X_RX_RING_DISP_MAX_CNT);

#if (CFG_SUPPORT_HOST_OFFLOAD == 1)
	/* enable wfdma magic cnt */
	if ((IS_FEATURE_ENABLED(prWifiVar->fgEnableRro)) &&
	    halIsDataRing(RX_RING, index)) {
		uint32_t u4Val = 0;

		HAL_MCR_RD(prAdapter, prRxRing->hw_cnt_addr, &u4Val);
		u4Val |= WF_WFDMA_HOST_DMA0_WPDMA_RX_RING0_CTRL1_MGC_ENA_MASK;
		HAL_MCR_WR(prAdapter, prRxRing->hw_cnt_addr, u4Val);
	}
#endif /* CFG_SUPPORT_HOST_OFFLOAD == 1 */
}

void *pcie_vir_addr;

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

	HAL_MCR_WR(prGlueInfo->prAdapter, 0x74030074, 0x08021000);
	if (pcie_vir_addr) {
		writel(0x08021000, (pcie_vir_addr + 0x74));
		DBGLOG(HAL, INFO, "pcie_vir_addr=0x%x\n", pcie_vir_addr);
	} else {
		DBGLOG(HAL, INFO, "pcie_vir_addr is null\n");
	}
}

static void mt6639PcieHwControlVote(
	struct ADAPTER *prAdapter,
	uint8_t enable,
	uint32_t u4WifiUser)
{
	halPcieHwControlVote(prAdapter, enable, u4WifiUser);
}

#if CFG_SUPPORT_PCIE_ASPM
uint32_t value_ori;
static u_int8_t mt6639SetL1ssEnable(struct ADAPTER *prAdapter,
				u_int role, u_int8_t fgEn)
{
	struct mt66xx_chip_info *prChipInfo;
	struct BUS_INFO *prBusInfo;
	unsigned long flags = 0;

	prChipInfo = prAdapter->chip_info;
	prBusInfo = prChipInfo->bus_info;

	spin_lock_irqsave(&rPCIELock, flags);

	if (role == WIFI_ROLE)
		prChipInfo->bus_info->fgWifiEnL1_2 = fgEn;
	else if (role == MD_ROLE)
		prChipInfo->bus_info->fgMDEnL1_2 = fgEn;

	spin_unlock_irqrestore(&rPCIELock, flags);

	DBGLOG(HAL, TRACE, "fgWifiEnL1_2 = %d, fgMDEnL1_2=%d\n",
		prChipInfo->bus_info->fgWifiEnL1_2,
		prChipInfo->bus_info->fgMDEnL1_2);

	if (prChipInfo->bus_info->fgWifiEnL1_2
		&& prChipInfo->bus_info->fgMDEnL1_2)
		return TRUE;
	else
		return FALSE;
}
static void mt6639ConfigPcieAspm(struct GLUE_INFO *prGlueInfo,
				u_int8_t fgEn, u_int enable_role)
{
	struct GL_HIF_INFO *prHifInfo = &prGlueInfo->rHifInfo;
	uint32_t value = 0, delay = 0;
	struct mt66xx_chip_info *prChipInfo;
	struct BUS_INFO *prBusInfo;
	u_int8_t enableL1ss = FALSE;

	prChipInfo = prGlueInfo->prAdapter->chip_info;
	prBusInfo = prChipInfo->bus_info;

	enableL1ss =
		mt6639SetL1ssEnable(prGlueInfo->prAdapter, enable_role, fgEn);

	if (fgEn) {
		/* Restore original setting*/
		if (enableL1ss) {
			if ((value_ori & 0xf00) == 0) {
				if (pcie_vir_addr)
					writel(0xe0f, (pcie_vir_addr + 0x194));
				delay += 10;
				udelay(10);

				/* Polling RC 0x112f0150[28:24] until =0x10 */
				while (1) {
					value = readl(pcie_vir_addr + 0x150);
					if (((value & BITS(24, 28))
						>> 24) == 0x10)
						break;
					if (delay >= POLLING_TIMEOUT)
						return;
					delay += 10;
					udelay(10);
				}

				HAL_MCR_WR(prGlueInfo->prAdapter,
					0x74030194, 0xf);
				HAL_MCR_RD(prGlueInfo->prAdapter,
					0x74030194, &value);
				if (pcie_vir_addr)
					writel(0xf, (pcie_vir_addr + 0x194));
			} else {
				DBGLOG(HAL, TRACE, "Enable aspm no match\n");
			}
			DBGLOG(HAL, TRACE, "Enable aspm L1.1/L1.2..\n");
		}
	} else {
		/*
		 *	Backup original setting then
		 *	disable L1.1, L1.2 and set LTR to 0
		 */
		HAL_MCR_RD(prGlueInfo->prAdapter, 0x74030194, &value_ori);
		if ((value_ori & 0xf00) == 0) {
			if (pcie_vir_addr)
				writel(0x20f, (pcie_vir_addr + 0x194));
			delay += 10;
			udelay(10);

			/* Polling RC 0x112f0150[28:24] until =0x10 */
			while (1) {
				value = readl(pcie_vir_addr + 0x150);
				if (((value & BITS(24, 28))
					>> 24) == 0x10)
					break;
				if (delay >= POLLING_TIMEOUT)
					return;
				delay += 10;
				udelay(10);
			}

			HAL_MCR_WR(prGlueInfo->prAdapter, 0x74030194, 0xc0f);
			HAL_MCR_RD(prGlueInfo->prAdapter, 0x74030194, &value);
			if (pcie_vir_addr)
				writel(0xc0f, (pcie_vir_addr + 0x194));
		} else {
			DBGLOG(HAL, TRACE, "Disable aspm no match\n");
		}

		if (prHifInfo->eCurPcieState == PCIE_STATE_L0)
			DBGLOG(HAL, TRACE, "Disable aspm L1..\n");
		else
			DBGLOG(HAL, TRACE, "Disable aspm L1.1/L1.2..\n");
	}
}

static void mt6639UpdatePcieAspm(struct GLUE_INFO *prGlueInfo, u_int8_t fgEn)
{
	struct GL_HIF_INFO *prHifInfo = &prGlueInfo->rHifInfo;

	if (fgEn) {
		prHifInfo->eNextPcieState = PCIE_STATE_L1_2;
	} else {
		if (prHifInfo->eNextPcieState != PCIE_STATE_L0)
			prHifInfo->eNextPcieState = PCIE_STATE_L1;
	}

	if (prHifInfo->eCurPcieState != prHifInfo->eNextPcieState) {
		if (prHifInfo->eNextPcieState == PCIE_STATE_L1_2)
			mt6639ConfigPcieAspm(prGlueInfo, TRUE, 1);
		else
			mt6639ConfigPcieAspm(prGlueInfo, FALSE, 1);
		prHifInfo->eCurPcieState = prHifInfo->eNextPcieState;
	}
}

static void mt6639KeepPcieWakeup(struct GLUE_INFO *prGlueInfo,
				u_int8_t fgWakeup)
{
	struct GL_HIF_INFO *prHifInfo = &prGlueInfo->rHifInfo;

	if (fgWakeup) {
		prHifInfo->eNextPcieState = PCIE_STATE_L0;
	} else {
		if (prHifInfo->eCurPcieState == PCIE_STATE_L0)
			prHifInfo->eNextPcieState = PCIE_STATE_L1;
	}
}

static u_int8_t mt6639DumpPcieDateFlowStatus(struct GLUE_INFO *prGlueInfo)
{
	struct pci_dev *pci_dev = NULL;
	struct GL_HIF_INFO *prHifInfo = NULL;
	u_int32_t u4RegVal[20] = {0};
	int32_t err = 0;

	err = mtk_pcie_dump_link_info(0);

	if (err & BIT(5)) {
		/*read pcie cfg.space 0x488 // level1: pcie*/
		prHifInfo = &prGlueInfo->rHifInfo;
		if (prHifInfo)
			pci_dev = prHifInfo->pdev;

		if (pci_dev) {
			pci_read_config_dword(pci_dev, 0x0, &u4RegVal[0]);
			if (u4RegVal[0] == 0) {
				DBGLOG(HAL, INFO,
					"PCIE link down 0x0=0x%08x\n",
					u4RegVal[0]);
				return FALSE;
			}

			/*1. read pcie cfg.space 0x488 // level1: pcie*/
			pci_read_config_dword(pci_dev, 0x488, &u4RegVal[1]);
			if (u4RegVal[1] != 0xC0093301)
				DBGLOG(HAL, INFO,
					"state mismatch 0x488=0x%08x\n",
					u4RegVal[1]);
		}

		/*2. cb_infra/cbtop status*/
		HAL_MCR_RD(prGlueInfo->prAdapter,
			0x1E7204,
			&u4RegVal[2]);
		if (u4RegVal[2] < 0x20220811) {
			DBGLOG(HAL, INFO, "version error 0x1E7204=0x%08x\n",
				u4RegVal[2]);
			return FALSE;
		}

		/*3. cb_infra_slp_status*/
		HAL_MCR_RD(prGlueInfo->prAdapter,
			0x1F500C,
			&u4RegVal[3]);
		if ((u4RegVal[3] & BITS(1, 3)) != BITS(1, 3)) {
			DBGLOG(HAL, INFO, "cb_infra_slp error=0x%08x\n",
				u4RegVal[3]);
			return FALSE;
		}

		/*4. MMIO dump slp_ctrl setting*/
		HAL_MCR_RD(prGlueInfo->prAdapter,
			0x1F5004,
			&u4RegVal[4]);

		HAL_MCR_RD(prGlueInfo->prAdapter,
			0x1F500C,
			&u4RegVal[5]);

		HAL_MCR_RD(prGlueInfo->prAdapter,
			0x1F5014,
			&u4RegVal[6]);

		/*5. MMIO dump slp_ctrl cnt:*/
		HAL_MCR_RD(prGlueInfo->prAdapter,
			0x1F5400,
			&u4RegVal[7]);

		HAL_MCR_RD(prGlueInfo->prAdapter,
			0x1F5404,
			&u4RegVal[8]);

		/*6. MMIO dump ap2conn gals dbg*/
		HAL_MCR_RD(prGlueInfo->prAdapter,
			0x1F6008,
			&u4RegVal[9]);

		/*6. MMIO dump conn2ap gals dbg*/
		HAL_MCR_RD(prGlueInfo->prAdapter,
			0x1F6000,
			&u4RegVal[10]);

		/*6. MMIO dump dma2ap gals dbg*/
		HAL_MCR_RD(prGlueInfo->prAdapter,
			0x1F6100,
			&u4RegVal[11]);

		/*7. MMIO dump 0x1F_5300*/
		HAL_MCR_RD(prGlueInfo->prAdapter,
			0x1F5300,
			&u4RegVal[12]);

		/*5. MMIO dump 0x1F_6550*/
		HAL_MCR_RD(prGlueInfo->prAdapter,
			0x1F6550,
			&u4RegVal[13]);

		/*6. MMIO dump 0x1F_801C*/
		HAL_MCR_RD(prGlueInfo->prAdapter,
			0x1F801C,
			&u4RegVal[14]);

		HAL_MCR_RD(prGlueInfo->prAdapter,
			0x1D0E48,
			&u4RegVal[16]);
		HAL_MCR_RD(prGlueInfo->prAdapter,
			0x1D0E40,
			&u4RegVal[17]);
		HAL_MCR_RD(prGlueInfo->prAdapter,
			0x1D0E44,
			&u4RegVal[18]);

		DBGLOG(HAL, INFO, DUMP_PCIE_CR,
		u4RegVal[4], u4RegVal[5], u4RegVal[6], u4RegVal[7],
		u4RegVal[8], u4RegVal[9], u4RegVal[10], u4RegVal[11],
		u4RegVal[12], u4RegVal[13], u4RegVal[14], u4RegVal[16],
		u4RegVal[17], u4RegVal[18]);

		/*7. MMIO write 0x1E_3020 = 0x0*/
		HAL_MCR_WR(prGlueInfo->prAdapter, 0x1E3020, 0x0);
		/*8. MMIO write 0x1E_7150 = 0x2*/
		HAL_MCR_WR(prGlueInfo->prAdapter, 0x1E7150, 0x2);
		/*9. CBTOP REGs dump  0x1E_7154*/
		HAL_MCR_RD(prGlueInfo->prAdapter,
			0x1E7154,
			&u4RegVal[15]);
		if (u4RegVal[15] != 0x0) {
			DBGLOG(HAL, INFO, "0x1E7154=0x%08x\n",
				u4RegVal[15]);
			return FALSE;
		}


	} else {
		return FALSE;
	}
	return TRUE;
}
#endif

static u_int8_t mt6639_set_crypto(struct ADAPTER *prAdapter)
{
	if (!prAdapter->fgIsWiFiOnDrvOwn)
		HAL_MCR_WR(prAdapter,
			CB_INFRA_SLP_CTRL_CB_INFRA_CRYPTO_TOP_MCU_OWN_SET_ADDR,
			BIT(0));
	return TRUE;
}


static void mt6639ShowPcieDebugInfo(struct GLUE_INFO *prGlueInfo)
{
	uint32_t u4Addr, u4Val = 0;

	if (!in_interrupt()) {
		u4Addr = 0x112F0184;
		wf_ioremap_read(u4Addr, &u4Val);
		DBGLOG(HAL, INFO, "PCIE CR [0x%08x]=[0x%08x]", u4Addr, u4Val);
		for (u4Addr = 0x112F0C04; u4Addr <= 0x112F0C1C; u4Addr += 4) {
			wf_ioremap_read(u4Addr, &u4Val);
			DBGLOG(HAL, INFO, "PCIE CR [0x%08x]=[0x%08x]",
			       u4Addr, u4Val);
		}
	}
}

static void mt6639SetupMcuEmiAddr(struct ADAPTER *prAdapter)
{
	phys_addr_t base = emi_mem_get_phy_base(prAdapter->chip_info);
	uint32_t size = emi_mem_get_size(prAdapter->chip_info);

	if (!base)
		return;

	DBGLOG(HAL, INFO, "base: 0x%llx, size: 0x%x\n", base, size);

	HAL_MCR_WR(prAdapter,
		   CONNAC3X_CONN_CFG_ON_CONN_ON_EMI_ADDR,
		   ((uint32_t)base >> 16));

	HAL_MCR_WR(prAdapter,
		   MT6639_EMI_SIZE_ADDR,
		   size);
}

static bool mt6639IsValidRegAccess(struct ADAPTER *prAdapter,
				   uint32_t u4Register)
{
	uint32_t au4ExcludeRegs[] = {
		CONNAC3X_BN0_LPCTL_ADDR,
		CONNAC3X_BN0_IRQ_STAT_ADDR,
		CONNAC3X_BN0_IRQ_ENA_ADDR,
		AP2WF_CONN_INFRA_ON_CCIF4_AP2WF_PCCIF_TCHNUM_ADDR,
		AP2WF_CONN_INFRA_ON_CCIF4_AP2WF_PCCIF_RCHNUM_ADDR,
		AP2WF_CONN_INFRA_ON_CCIF4_AP2WF_PCCIF_ACK_ADDR,
		AP2WF_CONN_INFRA_ON_CCIF4_AP2WF_PCCIF_DUMMY1_ADDR,
		AP2WF_CONN_INFRA_ON_CCIF4_AP2WF_PCCIF_DUMMY2_ADDR,
		WF2AP_CONN_INFRA_ON_CCIF4_WF2AP_PCCIF_DUMMY1_ADDR,
		WF2AP_CONN_INFRA_ON_CCIF4_WF2AP_PCCIF_DUMMY2_ADDR,
		CONN_BUS_CR_VON_CONN_INFRA_PCIE2AP_REMAP_WF_0_76_ADDR,
		0x6d010,
		0x6d014,
	};
	struct GL_HIF_INFO *prHifInfo;
	uint32_t u4Idx, u4Size = sizeof(au4ExcludeRegs) / sizeof(uint32_t);

	if (fgIsBusAccessFailed)
		return false;

	if (prAdapter) {
		prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
		if (!prHifInfo->fgIsPowerOn)
			return true;
		if (!prAdapter->fgIsFwOwn)
			return true;
	}

	/* only own control and wfdma int register can be accessed on fw own */
	if ((u4Register & 0xFFFF0000) == CONNAC3X_HOST_CSR_TOP_BASE)
		return true;

	for (u4Idx = 0; u4Idx < u4Size; u4Idx++) {
		if (u4Register == au4ExcludeRegs[u4Idx])
			return true;
	}

	return false;
}

static u_int8_t mt6639_get_sw_interrupt_status(struct ADAPTER *prAdapter,
	uint32_t *pu4Status)
{
	*pu4Status = ccif_get_interrupt_status(prAdapter);
	return TRUE;
}

static uint32_t mt6639_ccif_get_interrupt_status(struct ADAPTER *ad)
{
	uint32_t u4Status = 0;

	HAL_MCR_RD(ad,
		AP2WF_CONN_INFRA_ON_CCIF4_AP2WF_PCCIF_RCHNUM_ADDR,
		&u4Status);
	HAL_MCR_WR(ad,
		AP2WF_CONN_INFRA_ON_CCIF4_AP2WF_PCCIF_ACK_ADDR,
		u4Status);

	return u4Status;
}

static void mt6639_ccif_notify_utc_time_to_fw(struct ADAPTER *ad,
	uint32_t sec,
	uint32_t usec)
{
	ACQUIRE_POWER_CONTROL_FROM_PM(ad);
	if (ad->fgIsFwOwn == TRUE)
		goto exit;

	HAL_MCR_WR(ad,
		AP2WF_CONN_INFRA_ON_CCIF4_AP2WF_PCCIF_DUMMY1_ADDR,
		sec);
	HAL_MCR_WR(ad,
		AP2WF_CONN_INFRA_ON_CCIF4_AP2WF_PCCIF_DUMMY2_ADDR,
		usec);
	HAL_MCR_WR(ad,
		AP2WF_CONN_INFRA_ON_CCIF4_AP2WF_PCCIF_TCHNUM_ADDR,
		SW_INT_TIME_SYNC);

exit:
	RECLAIM_POWER_CONTROL_TO_PM(ad, FALSE);
}

static void mt6639_ccif_set_fw_log_read_pointer(struct ADAPTER *ad,
	enum ENUM_FW_LOG_CTRL_TYPE type,
	uint32_t read_pointer)
{
	uint32_t u4Addr = 0;

	if (type == ENUM_FW_LOG_CTRL_TYPE_MCU)
		u4Addr = WF2AP_CONN_INFRA_ON_CCIF4_WF2AP_PCCIF_DUMMY2_ADDR;
	else
		u4Addr = WF2AP_CONN_INFRA_ON_CCIF4_WF2AP_PCCIF_DUMMY1_ADDR;

	HAL_MCR_WR(ad, u4Addr, read_pointer);
}

static uint32_t mt6639_ccif_get_fw_log_read_pointer(struct ADAPTER *ad,
	enum ENUM_FW_LOG_CTRL_TYPE type)
{
	uint32_t u4Addr = 0, u4Value = 0;

	if (type == ENUM_FW_LOG_CTRL_TYPE_MCU)
		u4Addr = WF2AP_CONN_INFRA_ON_CCIF4_WF2AP_PCCIF_DUMMY2_ADDR;
	else
		u4Addr = WF2AP_CONN_INFRA_ON_CCIF4_WF2AP_PCCIF_DUMMY1_ADDR;

	HAL_MCR_RD(ad, u4Addr, &u4Value);

	return u4Value;
}

static int32_t mt6639_ccif_trigger_fw_assert(struct ADAPTER *ad)
{
	HAL_MCR_WR(ad,
		AP2WF_CONN_INFRA_ON_CCIF4_AP2WF_PCCIF_TCHNUM_ADDR,
		SW_INT_SUBSYS_RESET);

	return 0;
}

#if IS_MOBILE_SEGMENT
static u_int8_t mt6639_check_recovery_needed(struct ADAPTER *ad)
{
	uint32_t u4Value = 0;
	u_int8_t fgResult = FALSE;

	/*
	 * if (0x81021604[31:16]==0xdead &&
	 *     (0x70005350[30:28]!=0x0 || 0x70005360[6:4]!=0x0)) == 0x1
	 * do recovery flow
	 */

	HAL_MCR_RD(ad, WF_TOP_CFG_ON_ROMCODE_INDEX_ADDR,
		&u4Value);
	DBGLOG(INIT, INFO, "0x%08x=0x%08x\n",
		WF_TOP_CFG_ON_ROMCODE_INDEX_ADDR, u4Value);
	if ((u4Value & 0xFFFF0000) != 0xDEAD0000) {
		fgResult = FALSE;
		goto exit;
	}

	HAL_MCR_RD(ad, CBTOP_GPIO_MODE5_ADDR,
		&u4Value);
	DBGLOG(INIT, INFO, "0x%08x=0x%08x\n",
		CBTOP_GPIO_MODE5_ADDR, u4Value);
	if (((u4Value & CBTOP_GPIO_MODE5_GPIO47_MASK) >>
	    CBTOP_GPIO_MODE5_GPIO47_SHFT) != 0x0) {
		fgResult = TRUE;
		goto exit;
	}

	HAL_MCR_RD(ad, CBTOP_GPIO_MODE6_ADDR,
		&u4Value);
	DBGLOG(INIT, INFO, "0x%08x=0x%08x\n",
		CBTOP_GPIO_MODE6_ADDR, u4Value);
	if (((u4Value & CBTOP_GPIO_MODE6_GPIO49_MASK) >>
	    CBTOP_GPIO_MODE6_GPIO49_SHFT) != 0x0) {
		fgResult = TRUE;
		goto exit;
	}

exit:
	return fgResult;
}

static uint32_t mt6639_mcu_reinit(struct ADAPTER *ad)
{
#define CONNINFRA_ID_MAX_POLLING_COUNT		10

	uint32_t u4Value = 0, u4PollingCnt = 0;
	uint32_t rStatus = WLAN_STATUS_SUCCESS;

	/* Check recovery needed */
	if (mt6639_check_recovery_needed(ad) == FALSE)
		goto exit;

	DBGLOG(INIT, INFO, "mt6639_mcu_reinit.\n");

	/* Force on conninfra */
	HAL_MCR_WR(ad,
		CONN_HOST_CSR_TOP_CONN_INFRA_WAKEPU_TOP_ADDR,
		0x1);

	/* Wait conninfra wakeup */
	while (TRUE) {
		HAL_MCR_RD(ad, CONN_CFG_IP_VERSION_IP_VERSION_ADDR,
			&u4Value);

		if (u4Value == MT6639_CONNINFRA_VERSION_ID ||
		    u4Value == MT6639_CONNINFRA_VERSION_ID_E2)
			break;

		u4PollingCnt++;
		if (u4PollingCnt >= CONNINFRA_ID_MAX_POLLING_COUNT) {
			rStatus = WLAN_STATUS_FAILURE;
			DBGLOG(INIT, ERROR,
				"Conninfra ID polling failed, value=0x%x\n",
				u4Value);
			goto exit;
		}

		kalUdelay(1000);
	}

	/* Switch to GPIO mode */
	HAL_MCR_WR(ad,
		CBTOP_GPIO_MODE5_MOD_ADDR,
		0x80000000);
	HAL_MCR_WR(ad,
		CBTOP_GPIO_MODE6_MOD_ADDR,
		0x80);
	kalUdelay(100);

	/* Reset */
	HAL_MCR_WR(ad,
		CB_INFRA_RGU_BT_SUBSYS_RST_ADDR,
		0x10351);
	HAL_MCR_WR(ad,
		CB_INFRA_RGU_WF_SUBSYS_RST_ADDR,
		0x10351);
	kalMdelay(10);
	HAL_MCR_WR(ad,
		CB_INFRA_RGU_BT_SUBSYS_RST_ADDR,
		0x10340);
	HAL_MCR_WR(ad,
		CB_INFRA_RGU_WF_SUBSYS_RST_ADDR,
		0x10340);

	kalMdelay(50);

	HAL_MCR_RD(ad, CBTOP_GPIO_MODE5_ADDR, &u4Value);
	DBGLOG(INIT, INFO, "0x%08x=0x%08x\n",
		CBTOP_GPIO_MODE5_ADDR, u4Value);

	HAL_MCR_RD(ad, CBTOP_GPIO_MODE6_ADDR, &u4Value);
	DBGLOG(INIT, INFO, "0x%08x=0x%08x\n",
		CBTOP_GPIO_MODE6_ADDR, u4Value);

	/* Clean force on conninfra */
	HAL_MCR_WR(ad,
		CONN_HOST_CSR_TOP_CONN_INFRA_WAKEPU_TOP_ADDR,
		0x0);

exit:
	return rStatus;
}

#if (CFG_MTK_ANDROID_WMT == 0)
static uint32_t mt6639_mcu_reset(struct ADAPTER *ad)
{
	uint32_t u4Value = 0;
	uint32_t rStatus = WLAN_STATUS_SUCCESS;

	DBGLOG(INIT, INFO, "mt6639_mcu_reset..\n");

	HAL_MCR_RD(ad,
		CB_INFRA_RGU_WF_SUBSYS_RST_ADDR,
		&u4Value);
	u4Value &= ~CB_INFRA_RGU_WF_SUBSYS_RST_WF_SUBSYS_RST_MASK;
	u4Value |= (0x1 << CB_INFRA_RGU_WF_SUBSYS_RST_WF_SUBSYS_RST_SHFT);
	HAL_MCR_WR(ad,
		CB_INFRA_RGU_WF_SUBSYS_RST_ADDR,
		u4Value);

	kalMdelay(1);

	HAL_MCR_RD(ad,
		CB_INFRA_RGU_WF_SUBSYS_RST_ADDR,
		&u4Value);
	u4Value &= ~CB_INFRA_RGU_WF_SUBSYS_RST_WF_SUBSYS_RST_MASK;
	u4Value |= (0x0 << CB_INFRA_RGU_WF_SUBSYS_RST_WF_SUBSYS_RST_SHFT);
	HAL_MCR_WR(ad,
		CB_INFRA_RGU_WF_SUBSYS_RST_ADDR,
		u4Value);

	HAL_MCR_RD(ad,
		CONN_SEMAPHORE_CONN_SEMA_OWN_BY_M0_STA_REP_1_ADDR,
		&u4Value);
	DBGLOG(INIT, INFO, "0x%08x=0x%08x.\n",
		CONN_SEMAPHORE_CONN_SEMA_OWN_BY_M0_STA_REP_1_ADDR,
		u4Value);
	if ((u4Value &
	     CONN_SEMAPHORE_CONN_SEMA_OWN_BY_M0_STA_REP_1_CONN_SEMA00_OWN_BY_M0_STA_REP_MASK) != 0x0)
		DBGLOG(INIT, ERROR, "L0.5 reset failed.\n");

	return rStatus;
}
#endif

static void set_cbinfra_remap(struct ADAPTER *ad)
{
	DBGLOG(INIT, INFO, "set_cbinfra_remap.\n");

	HAL_MCR_WR(ad,
		CB_INFRA_MISC0_CBTOP_PCIE_REMAP_WF_ADDR,
		0x74037001);
	HAL_MCR_WR(ad,
		CB_INFRA_MISC0_CBTOP_PCIE_REMAP_WF_BT_ADDR,
		0x70007000);
}

static uint32_t mt6639_mcu_init(struct ADAPTER *ad)
{
#define MCU_IDLE		0x1D1E

	uint32_t u4Value = 0, u4PollingCnt = 0;
	uint32_t rStatus = WLAN_STATUS_SUCCESS;

	set_cbinfra_remap(ad);

	rStatus = mt6639_mcu_reinit(ad);
	if (rStatus != WLAN_STATUS_SUCCESS)
		goto exit;

#if (CFG_MTK_ANDROID_WMT == 0)
	rStatus = mt6639_mcu_reset(ad);
	if (rStatus != WLAN_STATUS_SUCCESS)
		goto exit;
#endif

	while (TRUE) {
		if (u4PollingCnt >= 1000) {
			DBGLOG(INIT, ERROR, "timeout.\n");
			rStatus = WLAN_STATUS_FAILURE;
			goto exit;
		}

		HAL_MCR_RD(ad, WF_TOP_CFG_ON_ROMCODE_INDEX_ADDR,
			&u4Value);
		if (u4Value == MCU_IDLE)
			break;

		u4PollingCnt++;
		kalUdelay(1000);
	}

#if IS_ENABLED(CFG_MTK_WIFI_CONNV3_SUPPORT)
	if (connv3_ext_32k_on()) {
		DBGLOG(INIT, ERROR, "connv3_ext_32k_on failed.\n");
		rStatus = WLAN_STATUS_FAILURE;
		goto exit;
	}
#endif

	if (ad->chip_info->coexpccifon)
		ad->chip_info->coexpccifon(ad);

	wlan_pinctrl_action(ad->chip_info, WLAN_PINCTRL_MSG_FUNC_PTA_UART_ON);

	pcie_vir_addr = ioremap(0x112f0000, 0x2000);

#if CFG_SUPPORT_PCIE_ASPM
	spin_lock_init(&rPCIELock);
#endif

exit:
	if (rStatus != WLAN_STATUS_SUCCESS) {
		WARN_ON_ONCE(TRUE);
		DBGLOG(INIT, ERROR, "u4Value: 0x%x\n",
			u4Value);
		mt6639_dumpWfsyscpupcr(ad);
		mt6639_dumpPcGprLog(ad);
		mt6639_dumpN45CoreReg(ad);
		mt6639_dumpWfTopReg(ad);
		mt6639_dumpWfBusReg(ad);

		HAL_MCR_WR(ad, 0x70003304, 0x06030138);
		HAL_MCR_WR(ad, 0x70000244, 0x000f0000);
		HAL_MCR_WR(ad, 0x70000244, 0x001f0000);
		HAL_MCR_WR(ad, 0x70000244, 0x011f0000);
		kalUdelay(1);
		HAL_MCR_RD(ad, 0x70000248, &u4Value);
		DBGLOG(INIT, INFO, "0x70000248: 0x%08x\n",
			u4Value);
		HAL_MCR_RD(ad, 0x70025030, &u4Value);
		DBGLOG(INIT, INFO, "0x70025030: 0x%08x\n",
			u4Value);
	}

	return rStatus;
}

static void mt6639_mcu_deinit(struct ADAPTER *ad)
{
#define MAX_WAIT_COREDUMP_COUNT 10

	int retry = 0;

	while (is_wifi_coredump_processing()) {
		if (retry >= MAX_WAIT_COREDUMP_COUNT) {
			DBGLOG(INIT, WARN,
				"Coredump spend long time, retry = %d\n",
				retry);
		}
		kalMsleep(100);
		retry++;
	}

	wifi_coredump_set_enable(FALSE);
	wlan_pinctrl_action(ad->chip_info, WLAN_PINCTRL_MSG_FUNC_PTA_UART_OFF);

	if (ad->chip_info->coexpccifoff)
		ad->chip_info->coexpccifoff(ad);

	if (pcie_vir_addr)
		iounmap(pcie_vir_addr);
}

static int32_t mt6639_trigger_fw_assert(struct ADAPTER *prAdapter)
{
	int32_t ret = 0;

	ccif_trigger_fw_assert(prAdapter);

	ret = reset_wait_for_trigger_completion();

	return ret;
}

#define MCIF_EMI_MEMORY_SIZE 128
#define MCIF_EMI_COEX_SWMSG_OFFSET 0xF8518000
#define MCIF_EMI_BASE_OFFSET 0xE4
static int mt6639ConnacPccifOn(struct ADAPTER *prAdapter)
{
#if CFG_MTK_CCCI_SUPPORT
	uint32_t mcif_emi_base, u4Val = 0;
	void *vir_addr = NULL;
	int ret = 0;

#if CFG_MTK_ANDROID_WMT
#if IS_ENABLED(CFG_MTK_WIFI_CONNV3_SUPPORT)
	if (is_pwr_on_notify_processing())
		return -1;
#endif
#endif

	mcif_emi_base = get_smem_phy_start_addr(
		MD_SYS1, SMEM_USER_RAW_MD_CONSYS, &ret);
	if (!mcif_emi_base) {
		DBGLOG(INIT, ERROR, "share memory is NULL.\n");
		return -1;
	}

	vir_addr = ioremap(mcif_emi_base, MCIF_EMI_MEMORY_SIZE);
	if (!vir_addr) {
		DBGLOG(INIT, ERROR, "ioremap fail.\n");
		return -1;
	}

	kalDevRegWrite(
		NULL,
		CONN_BUS_CR_VON_CONN_INFRA_PCIE2AP_REMAP_WF_1_BA_ADDR,
		0x18051803);

	kalMemSetIo(vir_addr, 0xFF, MCIF_EMI_MEMORY_SIZE);
	writel(0x4D4D434D, vir_addr);
	writel(0x4D4D434D, vir_addr + 0x4);
	writel(0x00000000, vir_addr + 0x8);
	writel(0x00000000, vir_addr + 0xC);
	writel(0x301B5801, vir_addr + 0x10);
	writel(0x02000010, vir_addr + 0x14);
	writel(0x301AF00C, vir_addr + 0x18);
	writel(0x00000001, vir_addr + 0x1C);
	writel(0x00000000, vir_addr + 0x70);
	writel(0x00000000, vir_addr + 0x74);
	writel(0x4D434D4D, vir_addr + 0x78);
	writel(0x4D434D4D, vir_addr + 0x7C);

	u4Val = readl(vir_addr + MCIF_EMI_BASE_OFFSET);
	HAL_MCR_WR(prAdapter, MT6639_MCIF_MD_STATE_WHEN_WIFI_ON_ADDR, u4Val);

	DBGLOG(INIT, TRACE, "MCIF_EMI_BASE_OFFSET=[0x%08x]\n", u4Val);
	DBGLOG_MEM128(HAL, TRACE, vir_addr, MCIF_EMI_MEMORY_SIZE);

	iounmap(vir_addr);
#else
	DBGLOG(INIT, ERROR, "[%s] ECCCI Driver is not supported.\n", __func__);
#endif
	return 0;
}

static int mt6639ConnacPccifOff(struct ADAPTER *prAdapter)
{
#if CFG_MTK_CCCI_SUPPORT
	uint32_t mcif_emi_base;
	void *vir_addr = NULL;
	int ret = 0;

	mcif_emi_base =	get_smem_phy_start_addr(
		MD_SYS1, SMEM_USER_RAW_MD_CONSYS, &ret);
	if (!mcif_emi_base) {
		DBGLOG(INIT, ERROR, "share memory is NULL.\n");
		return -1;
	}

	vir_addr = ioremap(mcif_emi_base, MCIF_EMI_MEMORY_SIZE);
	if (!vir_addr) {
		DBGLOG(INIT, ERROR, "ioremap fail.\n");
		return -1;
	}

	writel(0, vir_addr + 0x10);
	writel(0, vir_addr + 0x14);
	writel(0, vir_addr + 0x18);
	writel(0, vir_addr + 0x1C);

	iounmap(vir_addr);
#else
	DBGLOG(INIT, ERROR, "[%s] ECCCI Driver is not supported.\n", __func__);
#endif
	return 0;
}

u_int8_t mt6639_is_ap2conn_off_readable(struct ADAPTER *ad)
{
#define MAX_POLLING_COUNT		4

	uint32_t value = 0, retry = 0;

	while (TRUE) {
		if (retry >= MAX_POLLING_COUNT) {
			DBGLOG(HAL, ERROR,
				"Conninfra off bus clk: 0x%08x\n",
				value);
			return FALSE;
		}

		HAL_MCR_WR(ad,
			   CONN_DBG_CTL_CONN_INFRA_BUS_CLK_DETECT_ADDR,
			   BIT(0));
		HAL_MCR_RD(ad,
			   CONN_DBG_CTL_CONN_INFRA_BUS_CLK_DETECT_ADDR,
			   &value);
		if ((value & BIT(1)) && (value & BIT(3)))
			break;

		retry++;
		kalMdelay(1);
	}

	HAL_MCR_RD(ad,
		   CONN_CFG_IP_VERSION_IP_VERSION_ADDR,
		   &value);
	if (value != MT6639_CONNINFRA_VERSION_ID &&
	    value != MT6639_CONNINFRA_VERSION_ID_E2) {
		DBGLOG(HAL, ERROR,
			"Conninfra ver id: 0x%08x\n",
			value);
		return FALSE;
	}

	HAL_MCR_RD(ad,
		   CONN_DBG_CTL_CONN_INFRA_BUS_TIMEOUT_IRQ_ADDR,
		   &value);
	if ((value & BITS(0, 9)) == 0x3FF)
		DBGLOG(HAL, ERROR,
			"Conninfra bus hang irq status: 0x%08x\n",
			value);

	return TRUE;
}

u_int8_t mt6639_is_conn2wf_readable(struct ADAPTER *ad)
{
	uint32_t value = 0;

	HAL_MCR_RD(ad,
		   CONN_BUS_CR_ADDR_CONN2SUBSYS_0_AHB_GALS_DBG_ADDR,
		   &value);
	if ((value & BIT(26)) != 0x0) {
		DBGLOG(HAL, ERROR,
			"conn2wf sleep protect: 0x%08x\n",
			value);
		return FALSE;
	}

	HAL_MCR_RD(ad,
		   WF_TOP_CFG_IP_VERSION_ADDR,
		   &value);
	if (value != MT6639_WF_VERSION_ID) {
		DBGLOG(HAL, ERROR,
			"WF ver id: 0x%08x\n",
			value);
		return FALSE;
	}

	HAL_MCR_RD(ad,
		   CONN_DBG_CTL_WF_MCUSYS_INFRA_VDNR_GEN_DEBUG_CTRL_AO_BUS_TIMEOUT_IRQ_ADDR,
		   &value);
	if ((value & BIT(0)) != 0x0) {
		DBGLOG(HAL, WARN,
			"WF mcusys bus hang irq status: 0x%08x\n",
			value);
		HAL_MCR_RD(ad,
			   CONN_DBG_CTL_CONN_INFRA_BUS_TIMEOUT_IRQ_ADDR,
			   &value);
		if (value == 0x100)
			DBGLOG(HAL, INFO,
				"Skip conn_infra_vdnr timeout irq.\n");
		else
			return FALSE;
	}

	return TRUE;
}

static int mt6639_CheckBusHang(void *priv, uint8_t rst_enable)
{
	struct ADAPTER *ad = priv;
	struct BUS_INFO *bus_info = NULL;
	u_int8_t readable = TRUE;

	if (fgIsBusAccessFailed)
		return 1;

	bus_info = ad->chip_info->bus_info;

	if (bus_info && bus_info->dumpPcieStatus)
		readable = bus_info->dumpPcieStatus(ad->prGlueInfo);

	if (readable == FALSE)
		return 1;

	if (mt6639_is_ap2conn_off_readable(ad) &&
	    mt6639_is_conn2wf_readable(ad))
		return 0;
	else
		return 1;
}
#endif /* IS_MOBILE_SEGMENT */
#endif /* _HIF_PCIE */

static uint32_t mt6639GetFlavorVer(uint8_t *flavor)
{
	uint32_t ret = WLAN_STATUS_FAILURE;

	if (IS_MOBILE_SEGMENT) {
		uint8_t aucFlavor[2] = {0};

		if (kalGetFwFlavor(&aucFlavor[0]) == 1) {
			kalScnprintf(flavor,
				     CFG_FW_FLAVOR_MAX_LEN,
				     "%u%s",
				     CFG_WIFI_IP_SET,
				     aucFlavor);
			ret = WLAN_STATUS_SUCCESS;
		} else if (kalScnprintf(flavor,
					CFG_FW_FLAVOR_MAX_LEN,
					"1") > 0) {
			ret = WLAN_STATUS_SUCCESS;
		} else {
			ret = WLAN_STATUS_FAILURE;
		}
	} else if (kalScnprintf(flavor,
				CFG_FW_FLAVOR_MAX_LEN,
				"2") > 0) {
		ret = WLAN_STATUS_SUCCESS;
	} else {
		ret = WLAN_STATUS_FAILURE;
	}

	return ret;
}
#endif  /* MT6639 */

static uint32_t mt6639_wlanDownloadPatch(struct ADAPTER *prAdapter)
{
	uint32_t status  = wlanDownloadPatch(prAdapter);

	if (status == WLAN_STATUS_SUCCESS)
		wifi_coredump_set_enable(TRUE);

	return status;
}
