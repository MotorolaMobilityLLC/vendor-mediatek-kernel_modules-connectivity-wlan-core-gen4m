// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

/*! \file   mt7935.c
*    \brief  Internal driver stack will export
*    the required procedures here for GLUE Layer.
*
*    This file contains all routines which are exported
     from MediaTek 802.11 Wireless LAN driver stack to GLUE Layer.
*/

#ifdef MT7935

#include "precomp.h"
#include "mt7935.h"
#include "coda/mt7935/cb_ckgen_top.h"
#include "coda/mt7935/cb_infra_misc0.h"
#include "coda/mt7935/cb_infra_rgu.h"
#include "coda/mt7935/cb_infra_slp_ctrl.h"
#include "coda/mt7935/cbtop_gpio_sw_def.h"
#include "coda/mt7935/conn_bus_cr.h"
#include "coda/mt7935/conn_cfg.h"
#include "coda/mt7935/conn_dbg_ctl.h"
#include "coda/mt7935/conn_host_csr_top.h"
#include "coda/mt7935/conn_semaphore.h"
#include "coda/mt7935/wf_cr_sw_def.h"
#include "coda/mt7935/wf_top_cfg.h"
#include "coda/mt7935/wf_wfdma_ext_wrap_csr.h"
#include "coda/mt7935/wf_wfdma_host_dma0.h"
#include "coda/mt7935/wf_wfdma_mcu_dma0.h"
#include "coda/mt7935/wf_pse_top.h"
#include "coda/mt7935/pcie_mac_ireg.h"
#include "coda/mt7935/conn_mcu_bus_cr.h"
#include "coda/mt7935/conn_bus_cr_von.h"
#include "coda/mt7935/conn_host_csr_top.h"
#include "coda/mt7935/vlp_uds_ctrl.h"
#include "coda/mt7935/wf_rro_top.h"
#include "coda/mt7935/wf_top_cfg_on.h"
#include "hal_dmashdl_mt7935.h"
#include "coda/mt7935/wf2ap_conn_infra_on_ccif4.h"
#include "coda/mt7935/ap2wf_conn_infra_on_ccif4.h"
#include "coda/mt7935/wf_top_cfg_on.h"
#include "coda/mt7935/wf_wtblon_top.h"
#include "coda/mt7935/wf_uwtbl_top.h"
#if IS_ENABLED(CFG_MTK_WIFI_CONNV3_SUPPORT)
#include "connv3.h"
#endif
#if CFG_MTK_WIFI_FW_LOG_MMIO
#include "fw_log_mmio.h"
#endif
#if CFG_MTK_WIFI_FW_LOG_EMI
#include "fw_log_emi.h"
#endif

#include "wlan_pinctrl.h"

#if CFG_MTK_MDDP_SUPPORT
#include "mddp_export.h"
#endif

#if CFG_MTK_CCCI_SUPPORT
#include "mtk_ccci_common.h"
#endif

#include "gl_coredump.h"

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
static uint32_t mt7935GetFlavorVer(uint8_t *flavor);

static void mt7935_ConstructFirmwarePrio(struct GLUE_INFO *prGlueInfo,
	uint8_t **apucNameTable, uint8_t **apucName,
	uint8_t *pucNameIdx, uint8_t ucMaxNameIdx);

static void mt7935_ConstructPatchName(struct GLUE_INFO *prGlueInfo,
	uint8_t **apucName, uint8_t *pucNameIdx);

#if CFG_MTK_WIFI_SUPPORT_DSP_FWDL
static void mt7935_ConstructDspName(struct GLUE_INFO *prGlueInfo,
	uint8_t **apucName, uint8_t *pucNameIdx);
#endif

#if (CFG_SUPPORT_FW_IDX_LOG_TRANS == 1)
static void mt7935_ConstructIdxLogBinName(struct GLUE_INFO *prGlueInfo,
	uint8_t **apucName);
#endif

#if defined(_HIF_PCIE)
static uint8_t mt7935SetRxRingHwAddr(struct RTMP_RX_RING *prRxRing,
		struct BUS_INFO *prBusInfo, uint32_t u4SwRingIdx);

static bool mt7935WfdmaAllocRxRing(struct GLUE_INFO *prGlueInfo,
		bool fgAllocMem);

static void mt7935ProcessTxInterrupt(
		struct ADAPTER *prAdapter);

static void mt7935ProcessRxInterrupt(
	struct ADAPTER *prAdapter);

static void mt7935WfdmaManualPrefetch(
	struct GLUE_INFO *prGlueInfo);

static void mt7935ReadIntStatusByMsi(struct ADAPTER *prAdapter,
		uint32_t *pu4IntStatus);

static void mt7935ReadIntStatus(struct ADAPTER *prAdapter,
		uint32_t *pu4IntStatus);

#if (CFG_SUPPORT_PCIE_PLAT_INT_FLOW == 1)
static void mt7935EnableInterruptViaPcie(struct ADAPTER *prAdapter);
static void mt7935DisableInterruptViaPcie(struct ADAPTER *prAdapter);
#endif
static void mt7935EnableInterrupt(struct ADAPTER *prAdapter);
static void mt7935DisableInterrupt(struct ADAPTER *prAdapter);

static void mt7935ConfigIntMask(struct GLUE_INFO *prGlueInfo,
		u_int8_t enable);

static void mt7935ConfigWfdmaRxRingThreshold(
	struct ADAPTER *prAdapter, uint32_t u4Num, u_int8_t fgIsData);

static void mt7935WpdmaConfig(struct GLUE_INFO *prGlueInfo,
		u_int8_t enable, bool fgResetHif);

#if CFG_MTK_WIFI_WFDMA_WB
/* TODO: update wfdma wb */
static void mt7935ProcessTxInterruptByEmi(struct ADAPTER *prAdapter);
static void mt7935ProcessRxInterruptByEmi(struct ADAPTER *prAdapter);
static void mt7935ReadIntStatusByEmi(struct ADAPTER *prAdapter,
				     uint32_t *pu4IntStatus);
static void mt7935ConfigEmiIntMask(struct GLUE_INFO *prGlueInfo,
				   u_int8_t enable);
#endif /* CFG_MTK_WIFI_WFDMA_WB */

static void mt7935SetupMcuEmiAddr(struct ADAPTER *prAdapter);

static void mt7935WfdmaTxRingExtCtrl(
	struct GLUE_INFO *prGlueInfo,
	struct RTMP_TX_RING *prTxRing,
	u_int32_t index);
static void mt7935WfdmaRxRingExtCtrl(
	struct GLUE_INFO *prGlueInfo,
	struct RTMP_RX_RING *rx_ring,
	u_int32_t index);

static void mt7935InitPcieInt(struct GLUE_INFO *prGlueInfo);

static void mt7935ShowPcieDebugInfo(struct GLUE_INFO *prGlueInfo);

static u_int8_t mt7935_get_sw_interrupt_status(struct ADAPTER *prAdapter,
	uint32_t *pu4Status);

static void mt7935_ccif_notify_utc_time_to_fw(struct ADAPTER *ad,
	uint32_t sec,
	uint32_t usec);
static uint32_t mt7935_ccif_get_interrupt_status(struct ADAPTER *ad);
static void mt7935_ccif_set_fw_log_read_pointer(struct ADAPTER *ad,
	enum ENUM_FW_LOG_CTRL_TYPE type,
	uint32_t read_pointer);
static uint32_t mt7935_ccif_get_fw_log_read_pointer(struct ADAPTER *ad,
	enum ENUM_FW_LOG_CTRL_TYPE type);
static int32_t mt7935_ccif_trigger_fw_assert(struct ADAPTER *ad);

static int32_t mt7935_trigger_fw_assert(struct ADAPTER *prAdapter);
static uint32_t mt7935_mcu_init(struct ADAPTER *ad);
static void mt7935_mcu_deinit(struct ADAPTER *ad);
static int mt7935ConnacPccifOn(struct ADAPTER *prAdapter);
static int mt7935ConnacPccifOff(struct ADAPTER *prAdapter);
static int mt7935_CheckBusHang(void *priv, uint8_t rst_enable);
static uint32_t mt7935_wlanDownloadPatch(struct ADAPTER *prAdapter);
static void mt7935WiFiNappingCtrl(struct GLUE_INFO *prGlueInfo, u_int8_t fgEn);
#endif

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

struct ECO_INFO mt7935_eco_table[] = {
	/* HW version,  ROM version,    Factory version */
	{0x00, 0x00, 0xA, 0x1},	/* E1 */
	{0x00, 0x00, 0x0, 0x0}	/* End of table */
};

uint8_t *apucmt7935FwName[] = {
	(uint8_t *) CFG_FW_FILENAME "_7935",
	NULL
};

#if defined(_HIF_PCIE)
struct PCIE_CHIP_CR_MAPPING mt7935_bus2chip_cr_mapping[] = {
	/* chip addr, bus addr, range */
	{0x830c0000, 0x00000, 0x1000}, /* WF_MCU_BUS_CR_REMAP */
	{0x54000000, 0x02000, 0x1000},  /* WFDMA PCIE0 MCU DMA0 */
	{0x55000000, 0x03000, 0x1000},  /* WFDMA PCIE0 MCU DMA1 */
	{0x56000000, 0x04000, 0x1000},  /* WFDMA reserved */
	{0x57000000, 0x05000, 0x1000},  /* WFDMA MCU wrap CR */
	{0x58000000, 0x06000, 0x1000},  /* WFDMA PCIE1 MCU DMA0 (MEM_DMA) */
	{0x59000000, 0x07000, 0x1000},  /* WFDMA PCIE1 MCU DMA1 */
	{0x820c0000, 0x08000, 0x4000},  /* WF_UMAC_TOP (PLE) */
	{0x820c8000, 0x0c000, 0xA000},  /* WF_UMAC_TOP (PSE) */
	{0x820cc000, 0x0e000, 0xE000},  /* WF_UMAC_TOP (PP) */
	{0x83000000, 0x10000, 0x10000},  /* WF_PHY_MAP3 */
#if (CFG_MTK_FPGA_PLATFORM == 1)
	{0x74030000, 0x10000, 0x2000}, /* PCIe MAC (conninfra remap) */
#else
	{0x74030000, 0x1d0000, 0x2000}, /* PCIe MAC (cbtop remap) */
#endif
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
	{0x830a0000, 0x40000, 0x10000},  /* WF_PHY_MAP0 */
	{0x83080000, 0x50000, 0x10000},  /* WF_PHY_MAP1 */
	{0x83090000, 0x60000, 0x10000},  /* WF_PHY_MAP2 */
	{0xe0400000, 0x70000, 0x10000}, /* WF_UMAC_SYSRAM */
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
	{0x820c4000, 0xa8000, 0x0400},  /* WF_LMAC_TOP BN1 (WF_UMTBL) */
	{0x81030000, 0xae000, 0x4100},  /* [APB2] WFSYS_ON */
	{0x80020000, 0xb0000, 0x10000}, /* WF_TOP_MISC_OFF */
	{0x81020000, 0xc0000, 0x10000}, /* WF_TOP_MISC_ON */
	{0x81040000, 0xd0000, 0x1000}, /* WF_MCU_CFG_ON */
	{0x81050000, 0xd1000, 0x1000}, /* WF_MCU_EINT */
	{0x81060000, 0xd2000, 0x1000}, /* WF_MCU_GPT */
	{0x81070000, 0xd3000, 0x1000}, /* WF_MCU_WDT */
	{0x80010000, 0xd4000, 0x1000}, /* WF_AXIDMA */
	{0x83010000, 0xe0000, 0x10000}, /* WF_PHY_MAP4 */
	{0x88000000, 0xf0000, 0x10000}, /* WF_MCU_CFG_LS */
	{0x7c020000, 0xd0000, 0x10000}, /* CONN_INFRA, wfdma */
	{0x7c060000, 0xe0000, 0x10000}, /* CONN_INFRA, conn_host_csr_top */
	{0x7c000000, 0xf0000, 0x10000}, /* CONN_INFRA */
	{0x7c010000, 0x100000, 0x10000}, /* CONN_INFRA */
	{0x7c030000, 0x160000, 0x10000}, /* CONN_INFRA_CCIF */
	{0x7c050000, 0x1a0000, 0x10000}, /* CONN_INFRA PCIE2AP REM */
#if (CFG_MTK_FPGA_PLATFORM == 0)
	{0x70010000, 0x1c0000, 0x10000},
	{0x70000000, 0x1e0000, 0x9000},
	{0x70020000, 0x1f0000, 0x10000}, /* Reserved for CBTOP, can't switch */
#endif
	{0x7c500000, MT7935_PCIE2AP_REMAP_BASE_ADDR, 0x200000}, /* remap */
	{0x0, 0x0, 0x0} /* End */
};
#endif

#if defined(_HIF_PCIE) || defined(_HIF_AXI)
struct pcie2ap_remap mt7935_pcie2ap_remap = {
	.reg_base = CONN_BUS_CR_VON_CONN_INFRA_PCIE2AP_REMAP_WF_0_76_CR_PCIE2AP_PUBLIC_REMAPPING_WF_06_ADDR,
	.reg_mask = CONN_BUS_CR_VON_CONN_INFRA_PCIE2AP_REMAP_WF_0_76_CR_PCIE2AP_PUBLIC_REMAPPING_WF_06_MASK,
	.reg_shift = CONN_BUS_CR_VON_CONN_INFRA_PCIE2AP_REMAP_WF_0_76_CR_PCIE2AP_PUBLIC_REMAPPING_WF_06_SHFT,
	.base_addr = MT7935_PCIE2AP_REMAP_BASE_ADDR
};

struct ap2wf_remap mt7935_ap2wf_remap = {
	.reg_base = CONN_MCU_BUS_CR_AP2WF_REMAP_1_R_AP2WF_PUBLIC_REMAPPING_0_START_ADDRESS_ADDR,
	.reg_mask = CONN_MCU_BUS_CR_AP2WF_REMAP_1_R_AP2WF_PUBLIC_REMAPPING_0_START_ADDRESS_MASK,
	.reg_shift = CONN_MCU_BUS_CR_AP2WF_REMAP_1_R_AP2WF_PUBLIC_REMAPPING_0_START_ADDRESS_SHFT,
	.base_addr = MT7935_REMAP_BASE_ADDR
};

struct PCIE_CHIP_CR_REMAPPING mt7935_bus2chip_cr_remapping = {
	.pcie2ap = &mt7935_pcie2ap_remap,
	.ap2wf = &mt7935_ap2wf_remap,
};

struct wfdma_group_info mt7935_wfmda_host_tx_group[] = {
	{"P0T0:AP DATA0", WF_WFDMA_HOST_DMA0_WPDMA_TX_RING0_CTRL0_ADDR, true},
	{"P0T1:AP DATA1", WF_WFDMA_HOST_DMA0_WPDMA_TX_RING1_CTRL0_ADDR, true},
	{"P0T2:AP DATA2", WF_WFDMA_HOST_DMA0_WPDMA_TX_RING2_CTRL0_ADDR, true},
	{"P0T3:AP DATA3", WF_WFDMA_HOST_DMA0_WPDMA_TX_RING3_CTRL0_ADDR, true},
	{"P0T3:AP DATA4", WF_WFDMA_HOST_DMA0_WPDMA_TX_RING4_CTRL0_ADDR, true},
	{"P0T3:AP DATA5", WF_WFDMA_HOST_DMA0_WPDMA_TX_RING5_CTRL0_ADDR, true},
	{"P0T15:AP CMD", WF_WFDMA_HOST_DMA0_WPDMA_TX_RING15_CTRL0_ADDR},
	{"P0T16:FWDL", WF_WFDMA_HOST_DMA0_WPDMA_TX_RING16_CTRL0_ADDR},
};

struct wfdma_group_info mt7935_wfmda_host_rx_group[] = {
	{"P0R4:DATA", WF_WFDMA_HOST_DMA0_WPDMA_RX_RING4_CTRL0_ADDR, true},
	{"P0R7:EVT", WF_WFDMA_HOST_DMA0_WPDMA_RX_RING7_CTRL0_ADDR, true},
	{"P0R6:HPPDATA", WF_WFDMA_HOST_DMA0_WPDMA_RX_RING6_CTRL0_ADDR, true},
	{"P0R5:TDone", WF_WFDMA_HOST_DMA0_WPDMA_RX_RING5_CTRL0_ADDR, true},
	{"P0R8:ICS", WF_WFDMA_HOST_DMA0_WPDMA_RX_RING8_CTRL0_ADDR, true},
	{"P0R9:Coredump", WF_WFDMA_HOST_DMA0_WPDMA_RX_RING9_CTRL0_ADDR, true},
	{"P0R10:Fw log", WF_WFDMA_HOST_DMA0_WPDMA_RX_RING10_CTRL0_ADDR, true},
};

struct wfdma_group_info mt7935_wfmda_wm_tx_group[] = {
};

struct wfdma_group_info mt7935_wfmda_wm_rx_group[] = {
	{"P0R0:FWDL", WF_WFDMA_MCU_DMA0_WPDMA_RX_RING0_CTRL0_ADDR},
};

struct pse_group_info mt7935_pse_group[] = {
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
struct pcie_msi_layout mt7935_pcie_msi_layout[] = {
#if (WFDMA_AP_MSI_NUM == 8)
	{"conn_hif_tx_data0_int", mtk_pci_isr,
	 mtk_pci_isr_tx_data0_thread, AP_INT, 0},
	{"conn_hif_tx_data1_int", mtk_pci_isr,
	 mtk_pci_isr_tx_data0_thread, AP_INT, 0},
	{"conn_hif_tx_free_done_int", mtk_pci_isr,
	 mtk_pci_isr_tx_free_done_thread, AP_INT, 0},
	{"conn_hif_rx_data0_int", mtk_pci_isr,
	 mtk_pci_isr_rx_data0_thread, AP_INT, 0},
	{"conn_hif_rx_data1_int", mtk_pci_isr,
	 mtk_pci_isr_rx_data1_thread, AP_INT, 0},
	{"conn_hif_event_int", mtk_pci_isr,
	 mtk_pci_isr_rx_event_thread, AP_INT, 0},
	{"conn_hif_cmd_int", mtk_pci_isr,
	 mtk_pci_isr_tx_cmd_thread, AP_INT, 0},
	{"conn_hif_lump_int", mtk_pci_isr,
	 mtk_pci_isr_lump_thread, AP_INT, 0},
#else
	{"conn_hif_host_int", mtk_pci_isr,
	 mtk_pci_isr_thread, AP_INT, 0},
	{"conn_hif_host_int", NULL, NULL, AP_INT, 0},
	{"conn_hif_host_int", NULL, NULL, AP_INT, 0},
	{"conn_hif_host_int", NULL, NULL, AP_INT, 0},
	{"conn_hif_host_int", NULL, NULL, AP_INT, 0},
	{"conn_hif_host_int", NULL, NULL, AP_INT, 0},
	{"conn_hif_host_int", NULL, NULL, AP_INT, 0},
	{"conn_hif_host_int", NULL, NULL, AP_INT, 0},
#endif
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
	 pcie_sw_int_thread_handler, AP_MISC_INT, 0},
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
	{"reserved", NULL, NULL, NONE_INT, 0},
	{"reserved", NULL, NULL, NONE_INT, 0},
#if CFG_MTK_WIFI_FW_LOG_MMIO || CFG_MTK_WIFI_FW_LOG_EMI
	{"fw_log_irq", pcie_fw_log_top_handler,
	 pcie_fw_log_thread_handler, AP_MISC_INT, 0},
#else
	{"reserved", NULL, NULL, NONE_INT, 0},
#endif
	{"reserved", NULL, NULL, NONE_INT, 0},
	{"reserved", NULL, NULL, NONE_INT, 0},
};
#endif

struct BUS_INFO mt7935_bus_info = {
#if defined(_HIF_PCIE) || defined(_HIF_AXI)
	.top_cfg_base = MT7935_TOP_CFG_BASE,

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
	 WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_4_MASK |
	 WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_5_MASK |
#endif /* CFG_SUPPORT_DISABLE_DATA_DDONE_INTR == 0 */
#if (CFG_SUPPORT_DISABLE_CMD_DDONE_INTR == 0)
	 WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_15_MASK |
#endif /* CFG_SUPPORT_DISABLE_CMD_DDONE_INTR == 0 */
#if (WFDMA_AP_MSI_NUM == 1)
	 WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_16_MASK |
#endif
	 WF_WFDMA_HOST_DMA0_HOST_INT_STA_mcu2host_sw_int_sts_MASK),
	.host_int_rxdone_bits =
	(WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_4_MASK |
	 WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_5_MASK |
	 WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_6_MASK |
	 WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_7_MASK |
	 WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_8_MASK |
	 WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_9_MASK |
	 WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_10_MASK),

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

	.bus2chip = mt7935_bus2chip_cr_mapping,
	.bus2chip_remap = &mt7935_bus2chip_cr_remapping,
	.max_static_map_addr = 0x00200000,

	.tx_ring_fwdl_idx = CONNAC3X_FWDL_TX_RING_IDX,
	.tx_ring_cmd_idx = 15,
	.tx_ring0_data_idx = 0,
	.tx_ring1_data_idx = 1,
	.tx_ring2_data_idx = 2,
	.tx_ring3_data_idx = 3,
	.tx_prio_data_idx = 4,
	.tx_altx_data_idx = 5,
	.rx_data_ring_num = 2,
	.rx_evt_ring_num = 5,
	.rx_data_ring_size = 3072,
	.rx_evt_ring_size = 128,
	.rx_data_ring_prealloc_size = 1024,
	.fw_own_clear_addr = CONNAC3X_BN0_IRQ_STAT_ADDR,
	.fw_own_clear_bit = PCIE_LPCR_FW_CLR_OWN,
	.fgCheckDriverOwnInt = FALSE,
	.u4DmaMask = 34,
	.wfmda_host_tx_group = mt7935_wfmda_host_tx_group,
	.wfmda_host_tx_group_len = ARRAY_SIZE(mt7935_wfmda_host_tx_group),
	.wfmda_host_rx_group = mt7935_wfmda_host_rx_group,
	.wfmda_host_rx_group_len = ARRAY_SIZE(mt7935_wfmda_host_rx_group),
	.wfmda_wm_tx_group = mt7935_wfmda_wm_tx_group,
	.wfmda_wm_tx_group_len = ARRAY_SIZE(mt7935_wfmda_wm_tx_group),
	.wfmda_wm_rx_group = mt7935_wfmda_wm_rx_group,
	.wfmda_wm_rx_group_len = ARRAY_SIZE(mt7935_wfmda_wm_rx_group),
	.prDmashdlCfg = &rMt7935DmashdlCfg,
	.prPpTopCr = &rMt7935PpTopCr,
	.prPseGroup = mt7935_pse_group,
	.u4PseGroupLen = ARRAY_SIZE(mt7935_pse_group),
	.pdmaSetup = mt7935WpdmaConfig,
#if defined(_HIF_PCIE) && (CFG_SUPPORT_PCIE_PLAT_INT_FLOW == 1)
	.enableInterrupt = mt7935EnableInterruptViaPcie,
	.disableInterrupt = mt7935DisableInterruptViaPcie,
#else
	.enableInterrupt = mt7935EnableInterrupt,
	.disableInterrupt = mt7935DisableInterrupt,
#endif
#if CFG_MTK_WIFI_WFDMA_WB
	.configWfdmaIntMask = mt7935ConfigEmiIntMask,
#else
	.configWfdmaIntMask = mt7935ConfigIntMask,
#endif /* CFG_MTK_WIFI_WFDMA_WB */
	.configWfdmaRxRingTh = mt7935ConfigWfdmaRxRingThreshold,
#if defined(_HIF_PCIE)
	.initPcieInt = mt7935InitPcieInt,
	.pdmaStop = asicConnac3xWfdmaStop,
	.pdmaPollingIdle = asicConnac3xWfdmaPollingAllIdle,
	.pcie_msi_info = {
		.prMsiLayout = mt7935_pcie_msi_layout,
		.u4MaxMsiNum = ARRAY_SIZE(mt7935_pcie_msi_layout),
	},
	.showDebugInfo = mt7935ShowPcieDebugInfo,
#endif /* _HIF_PCIE */
#if CFG_MTK_WIFI_WFDMA_WB
	.processTxInterrupt = mt7935ProcessTxInterruptByEmi,
	.processRxInterrupt = mt7935ProcessRxInterruptByEmi,
#else
	.processTxInterrupt = mt7935ProcessTxInterrupt,
	.processRxInterrupt = mt7935ProcessRxInterrupt,
#endif /* CFG_MTK_WIFI_WFDMA_WB */
	.tx_ring_ext_ctrl = mt7935WfdmaTxRingExtCtrl,
	.rx_ring_ext_ctrl = mt7935WfdmaRxRingExtCtrl,
	/* null wfdmaManualPrefetch if want to disable manual mode */
	.wfdmaManualPrefetch = mt7935WfdmaManualPrefetch,
	.lowPowerOwnRead = asicConnac3xLowPowerOwnRead,
	.lowPowerOwnSet = asicConnac3xLowPowerOwnSet,
	.lowPowerOwnClear = asicConnac3xLowPowerOwnClear,
	.wakeUpWiFi = asicWakeUpWiFi,
	.processSoftwareInterrupt = asicConnac3xProcessSoftwareInterrupt,
	.softwareInterruptMcu = asicConnac3xSoftwareInterruptMcu,
	.hifRst = asicConnac3xHifRst,
#if defined(_HIF_PCIE) && (WFDMA_AP_MSI_NUM == 8)
	.devReadIntStatus = mt7935ReadIntStatusByMsi,
#else
#if CFG_MTK_WIFI_WFDMA_WB
	.devReadIntStatus = mt7935ReadIntStatusByEmi,
#else
	.devReadIntStatus = mt7935ReadIntStatus,
#endif /* CFG_MTK_WIFI_WFDMA_WB */
#endif /* _HIF_PCIE */
	.setRxRingHwAddr = mt7935SetRxRingHwAddr,
	.wfdmaAllocRxRing = mt7935WfdmaAllocRxRing,
	.setupMcuEmiAddr = mt7935SetupMcuEmiAddr,
#endif /*_HIF_PCIE || _HIF_AXI */
#if defined(_HIF_PCIE) || defined(_HIF_AXI)
	.DmaShdlInit = mt7935DmashdlInit,
#endif

#if defined(_HIF_NONE)
	/* for compiler need one entry */
	.DmaShdlInit = NULL
#endif
};

#if CFG_ENABLE_FW_DOWNLOAD
struct FWDL_OPS_T mt7935_fw_dl_ops = {
	.constructFirmwarePrio = mt7935_ConstructFirmwarePrio,
	.constructPatchName = mt7935_ConstructPatchName,
#if CFG_SUPPORT_SINGLE_FW_BINARY
	.parseSingleBinaryFile = wlanParseSingleBinaryFile,
#endif
#if (CFG_SUPPORT_FW_IDX_LOG_TRANS == 1)
	.constrcutIdxLogBin = mt7935_ConstructIdxLogBinName,
#endif /* CFG_SUPPORT_FW_IDX_LOG_TRANS */
#if defined(_HIF_PCIE)
	.downloadPatch = mt7935_wlanDownloadPatch,
#endif
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
#if defined(_HIF_PCIE)
	.mcu_init = mt7935_mcu_init,
	.mcu_deinit = mt7935_mcu_deinit,
#endif
#if CFG_SUPPORT_WIFI_DL_BT_PATCH
	.constructBtPatchName = asicConnac3xConstructBtPatchName,
	.downloadBtPatch = asicConnac3xDownloadBtPatch,
#if (CFG_SUPPORT_CONNAC3X == 1)
	.configBtImageSection = asicConnac3xConfigBtImageSection,
#endif
#endif
	.getFwVerInfo = wlanParseRamCodeReleaseManifest,
#if CFG_MTK_WIFI_SUPPORT_DSP_FWDL
	.constructDspName = mt7935_ConstructDspName,
	.downloadDspFw = wlanDownloadDspFw,
#endif
};
#endif /* CFG_ENABLE_FW_DOWNLOAD */

struct TX_DESC_OPS_T mt7935_TxDescOps = {
	.fillNicAppend = fillNicTxDescAppend,
	.fillHifAppend = fillTxDescAppendByHostV2,
	.fillTxByteCount = fillConnac3xTxDescTxByteCount,
};

struct RX_DESC_OPS_T mt7935_RxDescOps = {0};

struct CHIP_DBG_OPS mt7935_DebugOps = {
#if defined(_HIF_PCIE) || defined(_HIF_AXI)
	.showPdmaInfo = connac3x_show_wfdma_info,
#endif
	.showTxdInfo = connac3x_show_txd_Info,
	.showWtblInfo = connac3x_show_wtbl_info,
	.showUmacWtblInfo = connac3x_show_umac_wtbl_info,
	.showCsrInfo = NULL,
#if defined(_HIF_PCIE) || defined(_HIF_AXI)
	.showDmaschInfo = connac3x_show_dmashdl_info,
#endif
#if defined(_HIF_PCIE) || defined(_HIF_AXI)
	.getFwDebug = NULL,
	.setFwDebug = NULL,
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
	.show_wfdma_dbg_probe_info = mt7935_show_wfdma_dbg_probe_info,
	.show_wfdma_wrapper_info = mt7935_show_wfdma_wrapper_info,
	.dumpwfsyscpupcr = mt7935_dumpWfsyscpupcr,
	.dumpBusHangCr = mt7935_DumpBusHangCr,
#endif
#if CFG_SUPPORT_LINK_QUALITY_MONITOR
	.get_rx_rate_info = mt7935_get_rx_rate_info,
#endif
#if CFG_SUPPORT_LLS
	.get_rx_link_stats = mt7935_get_rx_link_stats,
#endif
	.dumpTxdInfo = connac3x_dump_tmac_info,
};

#if CFG_SUPPORT_QA_TOOL
struct ATE_OPS_T mt7935_AteOps = {
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
	.icapRiseVcoreClockRate = mt7935_icapRiseVcoreClockRate,
	.icapDownVcoreClockRate = mt7935_icapDownVcoreClockRate,
};
#endif /* CFG_SUPPORT_QA_TOOL */

#if defined(_HIF_PCIE)
#if (CFG_MTK_FPGA_PLATFORM == 0)
static struct CCIF_OPS mt7935_ccif_ops = {
	.get_interrupt_status = mt7935_ccif_get_interrupt_status,
	.notify_utc_time_to_fw = mt7935_ccif_notify_utc_time_to_fw,
	.set_fw_log_read_pointer = mt7935_ccif_set_fw_log_read_pointer,
	.get_fw_log_read_pointer = mt7935_ccif_get_fw_log_read_pointer,
	.trigger_fw_assert = mt7935_ccif_trigger_fw_assert,
};
#endif
#if CFG_MTK_WIFI_FW_LOG_MMIO
static struct FW_LOG_OPS mt7935_fw_log_mmio_ops = {
	.init = fwLogMmioInitMcu,
	.deinit = fwLogMmioDeInitMcu,
	.start = fwLogMmioStart,
	.stop = fwLogMmioStop,
	.handler = fwLogMmioHandler,
};
#endif

#if CFG_MTK_WIFI_FW_LOG_EMI
static struct FW_LOG_OPS mt7935_fw_log_emi_ops = {
	.init = fw_log_emi_init,
	.deinit = fw_log_emi_deinit,
	.start = fw_log_emi_start,
	.stop = fw_log_emi_stop,
	.set_enabled = fw_log_emi_set_enabled,
	.handler = fw_log_emi_handler,
};
#endif
#endif

#if CFG_SUPPORT_THERMAL_QUERY
struct thermal_sensor_info mt7935_thermal_sensor_info[] = {
	{"wifi_adie_0", THERMAL_TEMP_TYPE_ADIE, 0},
	{"wifi_ddie_0", THERMAL_TEMP_TYPE_DDIE, 0},
	{"wifi_ddie_1", THERMAL_TEMP_TYPE_DDIE, 1},
	{"wifi_ddie_2", THERMAL_TEMP_TYPE_DDIE, 2},
	{"wifi_ddie_3", THERMAL_TEMP_TYPE_DDIE, 3},
};
#endif

#if CFG_NEW_HIF_DEV_REG_IF
enum HIF_DEV_REG_REASON mt7935ValidMmioReadReason[] = {
	HIF_DEV_REG_HIF_READ,
	HIF_DEV_REG_HIF_DBG,
	HIF_DEV_REG_HIF_EXTDBG,
	HIF_DEV_REG_ONOFF_READ,
	HIF_DEV_REG_ONOFF_DBG,
	HIF_DEV_REG_RESET_READ,
	HIF_DEV_REG_COREDUMP_DBG,
	HIF_DEV_REG_LPOWN_READ,
	HIF_DEV_REG_SER_READ,
	HIF_DEV_REG_CCIF_READ,
	HIF_DEV_REG_PLAT_DBG,
	HIF_DEV_REG_UMAC_DBG,
	HIF_DEV_REG_WTBL_DBG,
	HIF_DEV_REG_OID_DBG,
};
#endif /* CFG_NEW_HIF_DEV_REG_IF */

struct mt66xx_chip_info mt66xx_chip_info_mt7935 = {
	.bus_info = &mt7935_bus_info,
#if CFG_ENABLE_FW_DOWNLOAD
	.fw_dl_ops = &mt7935_fw_dl_ops,
#endif /* CFG_ENABLE_FW_DOWNLOAD */
#if CFG_SUPPORT_QA_TOOL
	.prAteOps = &mt7935_AteOps,
#endif /* CFG_SUPPORT_QA_TOOL */
	.prTxDescOps = &mt7935_TxDescOps,
	.prRxDescOps = &mt7935_RxDescOps,
	.prDebugOps = &mt7935_DebugOps,
	.chip_id = MT7935_CHIP_ID,
	.should_verify_chip_id = FALSE,
	.sw_sync0 = Connac3x_CONN_CFG_ON_CONN_ON_MISC_ADDR,
	.sw_ready_bits = WIFI_FUNC_NO_CR4_READY_BITS,
	.sw_ready_bit_offset =
		Connac3x_CONN_CFG_ON_CONN_ON_MISC_DRV_FM_STAT_SYNC_SHFT,
	.patch_addr = MT7935_PATCH_START_ADDR,
	.is_support_cr4 = FALSE,
	.is_support_wacpu = FALSE,
#if defined(_HIF_PCIE)
	.is_en_wfdma_no_mmio_read = FALSE,
#endif /* _HIF_PCIE */
#if CFG_MTK_WIFI_WFDMA_WB
	.is_support_wfdma_write_back = TRUE,
#endif
	.txd_append_size = MT7935_TX_DESC_APPEND_LENGTH,
	.hif_txd_append_size = MT7935_HIF_TX_DESC_APPEND_LENGTH,
	.rxd_size = MT7935_RX_DESC_LENGTH,
	.init_evt_rxd_size = MT7935_RX_INIT_DESC_LENGTH,
	.pse_header_length = CONNAC3X_NIC_TX_PSE_HEADER_LENGTH,
	.init_event_size = CONNAC3X_RX_INIT_EVENT_LENGTH,
	.eco_info = mt7935_eco_table,
	.isNicCapV1 = FALSE,
	.is_support_efuse = TRUE,
	.top_hcr = CONNAC3X_TOP_HCR,
	.top_hvr = CONNAC3X_TOP_HVR,
	.top_fvr = CONNAC3X_TOP_FVR,
#if (CFG_SUPPORT_802_11AX == 1)
	.arb_ac_mode_addr = MT7935_ARB_AC_MODE_ADDR,
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
	.coexpccifon = mt7935ConnacPccifOn,
	.coexpccifoff = mt7935ConnacPccifOff,
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

	.prTxPwrLimitFile = "TxPwrLimit_MT66x9.dat",
#if (CFG_SUPPORT_SINGLE_SKU_6G == 1)
	.prTxPwrLimit6GFile = "TxPwrLimit6G_MT66x9.dat",
#if (CFG_SUPPORT_SINGLE_SKU_6G_1SS1T == 1)
	.prTxPwrLimit6G1ss1tFile = "TxPwrLimit6G_MT66x9_1ss1t.dat",
#endif
#endif

	.ucTxPwrLimitBatchSize = 3,
#if defined(_HIF_PCIE)
	.chip_capability = BIT(CHIP_CAPA_FW_LOG_TIME_SYNC) |
		BIT(CHIP_CAPA_FW_LOG_TIME_SYNC_BY_CCIF) |
		BIT(CHIP_CAPA_XTAL_TRIM),
	.checkbushang = mt7935_CheckBusHang,
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
		.sensor_num = ARRAY_SIZE(mt7935_thermal_sensor_info),
		.sensor_info = mt7935_thermal_sensor_info,
	},
#endif
	.trigger_fw_assert = mt7935_trigger_fw_assert,
	.fw_log_info = {
#if CFG_MTK_WIFI_FW_LOG_MMIO
		.ops = &mt7935_fw_log_mmio_ops,
#endif
#if CFG_MTK_WIFI_FW_LOG_EMI
		.base = 0x538000,
		.ops = &mt7935_fw_log_emi_ops,
#endif
		.path = ENUM_LOG_READ_POINTER_PATH_CCIF,
	},
#if (CFG_MTK_FPGA_PLATFORM == 0)
	.ccif_ops = &mt7935_ccif_ops,
#endif
	.get_sw_interrupt_status = mt7935_get_sw_interrupt_status,
#else
	.chip_capability = BIT(CHIP_CAPA_FW_LOG_TIME_SYNC) |
		BIT(CHIP_CAPA_XTAL_TRIM),
#endif /* _HIF_PCIE */
	.custom_oid_interface_version = MTK_CUSTOM_OID_INTERFACE_VERSION,
	.em_interface_version = MTK_EM_INTERFACE_VERSION,
#if CFG_CHIP_RESET_SUPPORT
	.asicWfsysRst = NULL,
	.asicPollWfsysSwInitDone = NULL,
#endif
#if defined(_HIF_PCIE) || defined(_HIF_AXI)
	/* owner set true when feature is ready. */
	.fgIsSupportL0p5Reset = TRUE,
#elif defined(_HIF_SDIO)
	/* owner set true when feature is ready. */
	.fgIsSupportL0p5Reset = FALSE,
#endif
	.u4MinTxLen = 2,
	.wifiNappingCtrl = mt7935WiFiNappingCtrl,
#if CFG_NEW_HIF_DEV_REG_IF
	.isValidMmioReadReason = connac3xIsValidMmioReadReason,
	.prValidMmioReadReason = mt7935ValidMmioReadReason,
	.u4ValidMmioReadReasonSize = ARRAY_SIZE(mt7935ValidMmioReadReason),
#endif /* CFG_NEW_HIF_DEV_REG_IF */
};

struct mt66xx_hif_driver_data mt66xx_driver_data_mt7935 = {
	.chip_info = &mt66xx_chip_info_mt7935,
};

void mt7935_icapRiseVcoreClockRate(void)
{
	DBGLOG(HAL, STATE, "icapRiseVcoreClockRate skip\n");
}

void mt7935_icapDownVcoreClockRate(void)
{
	DBGLOG(HAL, STATE, "icapDownVcoreClockRate skip\n");
}

static void mt7935_ConstructFirmwarePrio(struct GLUE_INFO *prGlueInfo,
	uint8_t **apucNameTable, uint8_t **apucName,
	uint8_t *pucNameIdx, uint8_t ucMaxNameIdx)
{
	int ret = 0;
	uint8_t ucIdx = 0;
	uint8_t aucFlavor[CFG_FW_FLAVOR_MAX_LEN];

	kalMemZero(aucFlavor, sizeof(aucFlavor));
	mt7935GetFlavorVer(&aucFlavor[0]);

#if CFG_SUPPORT_SINGLE_FW_BINARY
	/* Type 0. mt7935_wifi.bin */
	ret = kalSnprintf(*(apucName + (*pucNameIdx)),
			CFG_FW_NAME_MAX_LEN,
			"mt7935_wifi.bin");
	if (ret >= 0 && ret < CFG_FW_NAME_MAX_LEN)
		(*pucNameIdx) += 1;
	else
		DBGLOG(INIT, ERROR,
			"[%u] kalSnprintf failed, ret: %d\n",
			__LINE__, ret);

	/* Type 1. mt7935_wifi_flavor.bin */
	ret = kalSnprintf(*(apucName + (*pucNameIdx)),
			CFG_FW_NAME_MAX_LEN,
			"mt7935_wifi_%s.bin",
			aucFlavor);
	if (ret >= 0 && ret < CFG_FW_NAME_MAX_LEN)
		(*pucNameIdx) += 1;
	else
		DBGLOG(INIT, ERROR,
			"[%u] kalSnprintf failed, ret: %d\n",
			__LINE__, ret);
#endif

	/* Type 2. WIFI_RAM_CODE_MT7935_1_1.bin */
	ret = kalSnprintf(*(apucName + (*pucNameIdx)),
			CFG_FW_NAME_MAX_LEN,
			"WIFI_RAM_CODE_MT%x_%s_%u.bin",
			MT7935_CHIP_ID,
			aucFlavor,
			MT7935_ROM_VERSION);
	if (ret >= 0 && ret < CFG_FW_NAME_MAX_LEN)
		(*pucNameIdx) += 1;
	else
		DBGLOG(INIT, ERROR,
			"[%u] kalSnprintf failed, ret: %d\n",
			__LINE__, ret);

	for (ucIdx = 0; apucmt7935FwName[ucIdx]; ucIdx++) {
		if ((*pucNameIdx + 3) >= ucMaxNameIdx) {
			/* the table is not large enough */
			DBGLOG(INIT, ERROR,
				"kalFirmwareImageMapping >> file name array is not enough.\n");
			ASSERT(0);
			continue;
		}

		/* Type 3. WIFI_RAM_CODE_7935.bin */
		ret = kalSnprintf(*(apucName + (*pucNameIdx)),
				CFG_FW_NAME_MAX_LEN, "%s.bin",
				apucmt7935FwName[ucIdx]);
		if (ret >= 0 && ret < CFG_FW_NAME_MAX_LEN)
			(*pucNameIdx) += 1;
		else
			DBGLOG(INIT, ERROR,
				"[%u] kalSnprintf failed, ret: %d\n",
				__LINE__, ret);
	}
}

static void mt7935_ConstructPatchName(struct GLUE_INFO *prGlueInfo,
	uint8_t **apucName, uint8_t *pucNameIdx)
{
	int ret = 0;
	uint8_t aucFlavor[CFG_FW_FLAVOR_MAX_LEN];

	kalMemZero(aucFlavor, sizeof(aucFlavor));
	mt7935GetFlavorVer(&aucFlavor[0]);

#if CFG_SUPPORT_SINGLE_FW_BINARY
	/* Type 0. mt7935_wifi.bin */
	ret = kalSnprintf(*(apucName + (*pucNameIdx)),
			CFG_FW_NAME_MAX_LEN,
			"mt7935_wifi.bin");
	if (ret >= 0 && ret < CFG_FW_NAME_MAX_LEN)
		(*pucNameIdx) += 1;
	else
		DBGLOG(INIT, ERROR,
			"[%u] kalSnprintf failed, ret: %d\n",
			__LINE__, ret);

	/* Type 1. mt7935_wifi_flavor.bin */
	ret = kalSnprintf(*(apucName + (*pucNameIdx)),
			CFG_FW_NAME_MAX_LEN,
			"mt7935_wifi_%s.bin",
			aucFlavor);
	if (ret >= 0 && ret < CFG_FW_NAME_MAX_LEN)
		(*pucNameIdx) += 1;
	else
		DBGLOG(INIT, ERROR,
			"[%u] kalSnprintf failed, ret: %d\n",
			__LINE__, ret);
#endif

	/* Type 2. WIFI_MT7935_PATCH_MCU_1_1_hdr.bin */
	ret = kalSnprintf(apucName[(*pucNameIdx)],
			  CFG_FW_NAME_MAX_LEN,
			  "WIFI_MT%x_PATCH_MCU_%s_%u_hdr.bin",
			  MT7935_CHIP_ID,
			  aucFlavor,
			  MT7935_ROM_VERSION);
	if (ret >= 0 && ret < CFG_FW_NAME_MAX_LEN)
		(*pucNameIdx) += 1;
	else
		DBGLOG(INIT, ERROR,
			"[%u] kalSnprintf failed, ret: %d\n",
			__LINE__, ret);

	/* Type 3. mt7935_patch_e1_hdr.bin */
	ret = kalSnprintf(apucName[(*pucNameIdx)],
			  CFG_FW_NAME_MAX_LEN,
			  "mt7935_patch_e1_hdr.bin");
	if (ret < 0 || ret >= CFG_FW_NAME_MAX_LEN)
		DBGLOG(INIT, ERROR,
			"[%u] kalSnprintf failed, ret: %d\n",
			__LINE__, ret);
}

#if CFG_MTK_WIFI_SUPPORT_DSP_FWDL
static void mt7935_ConstructDspName(struct GLUE_INFO *prGlueInfo,
	uint8_t **apucName, uint8_t *pucNameIdx)
{
	int ret = 0;
	uint8_t aucFlavor[CFG_FW_FLAVOR_MAX_LEN];

	kalMemZero(aucFlavor, sizeof(aucFlavor));
	mt7935GetFlavorVer(&aucFlavor[0]);

	/* Type 1. WIFI_MT7935_PHY_RAM_CODE_1_1_hdr.bin */
	ret = kalSnprintf(apucName[(*pucNameIdx)],
			  CFG_FW_NAME_MAX_LEN,
			  "WIFI_MT%x_PHY_RAM_CODE_%s_%u.bin",
			  MT7935_CHIP_ID,
			  aucFlavor,
			  MT7935_ROM_VERSION);
	if (ret < 0 || ret >= CFG_FW_NAME_MAX_LEN)
		DBGLOG(INIT, ERROR,
			"[%u] kalSnprintf failed, ret: %d\n",
			__LINE__, ret);
	else
		(*pucNameIdx) += 1;
}
#endif

#if (CFG_SUPPORT_FW_IDX_LOG_TRANS == 1)
static void mt7935_ConstructIdxLogBinName(struct GLUE_INFO *prGlueInfo,
	uint8_t **apucName)
{
	int ret = 0;
	uint8_t aucFlavor[CFG_FW_FLAVOR_MAX_LEN];

	mt7935GetFlavorVer(&aucFlavor[0]);

	/* ex: WIFI_RAM_CODE_MT7935_2_1_idxlog.bin */
	ret = kalSnprintf(apucName[0],
			  CFG_FW_NAME_MAX_LEN,
			  "WIFI_RAM_CODE_MT%x_%s_%u_idxlog.bin",
			  MT7935_CHIP_ID,
			  aucFlavor,
			  MT7935_ROM_VERSION);

	if (ret < 0 || ret >= CFG_FW_NAME_MAX_LEN)
		DBGLOG(INIT, ERROR,
			"[%u] kalSnprintf failed, ret: %d\n",
			__LINE__, ret);
}
#endif /* CFG_SUPPORT_FW_IDX_LOG_TRANS */

#if defined(_HIF_PCIE)
static uint8_t mt7935SetRxRingHwAddr(struct RTMP_RX_RING *prRxRing,
		struct BUS_INFO *prBusInfo, uint32_t u4SwRingIdx)
{
	uint32_t offset = 0;

	/*
	 * RX_RING_DATA0   (RX_Ring4) - Rx Data
	 * RX_RING_DATA1   (RX_Ring6) - HPP Rx Data
	 * RX_RING_EVT     (RX_Ring7) - Rx Event
	 * RX_RING_TXDONE0 (RX_Ring5) - Tx Free Done Event
	 * RX_RING_TXDONE1 (RX_Ring8) - ICS / RXPRT
	 * RX_RING_TXDONE2 (RX_Ring9) - Coredump event
	 * RX_RING_WAEVT0  (RX_Ring10) - FW log
	*/
	switch (u4SwRingIdx) {
	case RX_RING_EVT:
		offset = 7;
		break;
	case RX_RING_DATA0:
		offset = 4;
		break;
	case RX_RING_DATA1:
		offset = 6;
		break;
	case RX_RING_TXDONE0:
		offset = 5;
		break;
	case RX_RING_TXDONE1:
		offset = 8;
		break;
	case RX_RING_TXDONE2:
		offset = 9;
		break;
	case RX_RING_WAEVT0:
		offset = 10;
		break;
	default:
		return FALSE;
	}

	halSetRxRingHwAddr(prRxRing, prBusInfo, offset);

	return TRUE;
}

static bool mt7935WfdmaAllocRxRing(struct GLUE_INFO *prGlueInfo,
		bool fgAllocMem)
{
	struct GL_HIF_INFO *prHifInfo = &prGlueInfo->rHifInfo;

	/* HPP Data Rx path */
	if (!halWpdmaAllocRxRing(prGlueInfo,
			RX_RING_DATA1, prHifInfo->u4RxDataRingSize,
			RXD_SIZE, CFG_RX_MAX_PKT_SIZE, fgAllocMem)) {
		DBGLOG(HAL, ERROR, "AllocRxRing[2] fail\n");
		return false;
	}

	/* TX Free Done */
	if (!halWpdmaAllocRxRing(prGlueInfo,
			RX_RING_TXDONE0, prHifInfo->u4RxEvtRingSize,
			RXD_SIZE, RX_BUFFER_AGGRESIZE, fgAllocMem)) {
		DBGLOG(HAL, ERROR, "AllocRxRing[3] fail\n");
		return false;
	}

	/* ICS log */
	if (!halWpdmaAllocRxRing(prGlueInfo,
			RX_RING_TXDONE1, prHifInfo->u4RxEvtRingSize,
			RXD_SIZE, RX_BUFFER_AGGRESIZE, fgAllocMem)) {
		DBGLOG(HAL, ERROR, "AllocRxRing[4] fail\n");
		return false;
	}

	/* Coredump */
	if (!halWpdmaAllocRxRing(prGlueInfo,
			RX_RING_TXDONE2, prHifInfo->u4RxEvtRingSize,
			RXD_SIZE, RX_BUFFER_AGGRESIZE, fgAllocMem)) {
		DBGLOG(HAL, ERROR, "AllocRxRing[6] fail\n");
		return false;
	}

	/* Fw log */
	if (!halWpdmaAllocRxRing(prGlueInfo,
			RX_RING_WAEVT0, prHifInfo->u4RxEvtRingSize,
			RXD_SIZE, RX_BUFFER_AGGRESIZE, fgAllocMem)) {
		DBGLOG(HAL, ERROR, "AllocRxRing[7] fail\n");
		return false;
	}
	return true;
}

static void mt7935ProcessTxInterrupt(
		struct ADAPTER *prAdapter)
{
	struct GL_HIF_INFO *prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	uint32_t u4Sta = prHifInfo->u4IntStatus;

	if (u4Sta & WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_16_MASK)
		halWpdmaProcessCmdDmaDone(
			prAdapter->prGlueInfo, TX_RING_FWDL);

#if (CFG_SUPPORT_DISABLE_CMD_DDONE_INTR == 0)
	if (u4Sta & WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_15_MASK)
		halWpdmaProcessCmdDmaDone(
			prAdapter->prGlueInfo, TX_RING_CMD);
#endif /* CFG_SUPPORT_DISABLE_CMD_DDONE_INTR == 0 */

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
			prAdapter->prGlueInfo, TX_RING_DATA2);
		kalSetTxEvent2Hif(prAdapter->prGlueInfo);
	}
	if (u4Sta & WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_3_MASK) {
		halWpdmaProcessDataDmaDone(
			prAdapter->prGlueInfo, TX_RING_DATA3);
		kalSetTxEvent2Hif(prAdapter->prGlueInfo);
	}

	if (u4Sta & WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_4_MASK) {
		halWpdmaProcessDataDmaDone(
			prAdapter->prGlueInfo, TX_RING_DATA_PRIO);
		kalSetTxEvent2Hif(prAdapter->prGlueInfo);
	}

	if (u4Sta & WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_5_MASK) {
		halWpdmaProcessDataDmaDone(
			prAdapter->prGlueInfo, TX_RING_DATA_ALTX);
		kalSetTxEvent2Hif(prAdapter->prGlueInfo);
	}
#endif /* CFG_SUPPORT_DISABLE_DATA_DDONE_INTR == 0 */
}

static void mt7935ProcessRxDataInterrupt(struct ADAPTER *prAdapter)
{
	struct GLUE_INFO *prGlueInfo = prAdapter->prGlueInfo;
	struct GL_HIF_INFO *prHifInfo = &prGlueInfo->rHifInfo;
	uint32_t u4Sta = prHifInfo->u4IntStatus;

	if ((u4Sta &
	     WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_4_MASK) ||
	    (KAL_TEST_BIT(RX_RING_DATA0, prAdapter->ulNoMoreRfb)))
		halRxReceiveRFBs(prAdapter, RX_RING_DATA0, TRUE);

	if ((u4Sta &
	     WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_6_MASK) ||
	    (KAL_TEST_BIT(RX_RING_DATA1, prAdapter->ulNoMoreRfb)))
		halRxReceiveRFBs(prAdapter, RX_RING_DATA1, TRUE);
}

static void mt7935ProcessRxInterrupt(struct ADAPTER *prAdapter)
{
	struct GLUE_INFO *prGlueInfo = prAdapter->prGlueInfo;
	struct GL_HIF_INFO *prHifInfo = &prGlueInfo->rHifInfo;
	uint32_t u4Sta = prHifInfo->u4IntStatus;

	if ((u4Sta & WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_5_MASK) ||
	    (KAL_TEST_BIT(RX_RING_TXDONE0, prAdapter->ulNoMoreRfb)))
		halRxReceiveRFBs(prAdapter, RX_RING_TXDONE0, FALSE);

	if ((u4Sta & WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_7_MASK) ||
	    (KAL_TEST_BIT(RX_RING_EVT, prAdapter->ulNoMoreRfb)))
		halRxReceiveRFBs(prAdapter, RX_RING_EVT, FALSE);

	if ((u4Sta & WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_8_MASK) ||
	    (KAL_TEST_BIT(RX_RING_TXDONE1, prAdapter->ulNoMoreRfb)))
		halRxReceiveRFBs(prAdapter, RX_RING_TXDONE1, FALSE);

	if ((u4Sta & WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_9_MASK) ||
	    (KAL_TEST_BIT(RX_RING_TXDONE2, prAdapter->ulNoMoreRfb)))
		halRxReceiveRFBs(prAdapter, RX_RING_TXDONE2, FALSE);

	if ((u4Sta & WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_10_MASK) ||
	    (KAL_TEST_BIT(RX_RING_WAEVT0, prAdapter->ulNoMoreRfb)))
		halRxReceiveRFBs(prAdapter, RX_RING_WAEVT0, FALSE);

	mt7935ProcessRxDataInterrupt(prAdapter);
}

static void mt7935SetTRXRingPriorityInterrupt(struct ADAPTER *prAdapter)
{
	uint32_t u4Val = 0;

#if (WFDMA_AP_MSI_NUM == 8)
	u4Val |= 0xF0;
#endif
#if CFG_MTK_MDDP_SUPPORT && (WFDMA_MD_MSI_NUM == 8)
	u4Val |= 0xF00;
#endif
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA0_WPDMA_INT_RX_PRI_SEL_ADDR, u4Val);

	u4Val = 0;
#if (WFDMA_AP_MSI_NUM == 8)
	u4Val |= 0x180FF;
#endif
#if CFG_MTK_MDDP_SUPPORT && (WFDMA_MD_MSI_NUM == 8)
	u4Val |= 0x7F00;
#endif
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA0_WPDMA_INT_TX_PRI_SEL_ADDR, u4Val);
}

static void mt7935WfdmaManualPrefetch(
	struct GLUE_INFO *prGlueInfo)
{
	struct ADAPTER *prAdapter = prGlueInfo->prAdapter;
	uint32_t u4WrVal = 0, u4Addr = 0;
	uint32_t u4PrefetchCnt = 0x4, u4TxDataPrefetchCnt = 0x10;
	uint32_t u4PrefetchBase = 0x00400000, u4TxDataPrefetchBase = 0x01000000;
	uint32_t u4RxDataPrefetchCnt = 0x8;
	uint32_t u4RxDataPrefetchBase = 0x00800000;

	/* Rx ring */
	for (u4Addr = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING4_EXT_CTRL_ADDR;
	     u4Addr <= WF_WFDMA_HOST_DMA0_WPDMA_RX_RING6_EXT_CTRL_ADDR;
	     u4Addr += 0x4) {
		u4WrVal = (u4WrVal & 0xFFFF0000) | u4RxDataPrefetchCnt;
		HAL_MCR_WR(prAdapter, u4Addr, u4WrVal);
		u4WrVal += u4RxDataPrefetchBase;
	}

	for (u4Addr = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING7_EXT_CTRL_ADDR;
	     u4Addr <= WF_WFDMA_HOST_DMA0_WPDMA_RX_RING10_EXT_CTRL_ADDR;
	     u4Addr += 0x4) {
		u4WrVal = (u4WrVal & 0xFFFF0000) | u4PrefetchCnt;
		HAL_MCR_WR(prAdapter, u4Addr, u4WrVal);
		u4WrVal += u4PrefetchBase;
	}

	/* Tx ring */
	for (u4Addr = WF_WFDMA_HOST_DMA0_WPDMA_TX_RING0_EXT_CTRL_ADDR;
		 u4Addr <= WF_WFDMA_HOST_DMA0_WPDMA_TX_RING5_EXT_CTRL_ADDR;
	     u4Addr += 0x4) {
		u4WrVal = (u4WrVal & 0xFFFF0000) | u4TxDataPrefetchCnt;
		HAL_MCR_WR(prAdapter, u4Addr, u4WrVal);
		u4WrVal += u4TxDataPrefetchBase;
	}

	u4Addr = WF_WFDMA_HOST_DMA0_WPDMA_TX_RING15_EXT_CTRL_ADDR;
	u4WrVal = (u4WrVal & 0xFFFF0000) | u4PrefetchCnt;
	HAL_MCR_WR(prAdapter, u4Addr, u4WrVal);
	u4WrVal += u4PrefetchBase;

	u4Addr = WF_WFDMA_HOST_DMA0_WPDMA_TX_RING16_EXT_CTRL_ADDR;
	u4WrVal = (u4WrVal & 0xFFFF0000) | u4PrefetchCnt;
	HAL_MCR_WR(prAdapter, u4Addr, u4WrVal);
	u4WrVal += u4PrefetchBase;

	mt7935SetTRXRingPriorityInterrupt(prAdapter);

	/* reset dma TRX idx */
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA0_WPDMA_RST_DTX_PTR_ADDR, 0xFFFFFFFF);
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA0_WPDMA_RST_DRX_PTR_ADDR, 0xFFFFFFFF);
}

#if defined(_HIF_PCIE)
static void mt7935ReadIntStatusByMsi(struct ADAPTER *prAdapter,
		uint32_t *pu4IntStatus)
{
	struct GL_HIF_INFO *prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	struct mt66xx_chip_info *prChipInfo = prAdapter->chip_info;
	struct BUS_INFO *prBusInfo = prChipInfo->bus_info;
	struct pcie_msi_info *prMsiInfo = &prBusInfo->pcie_msi_info;
	uint32_t u4Value = 0, u4WrValue = 0;

	*pu4IntStatus = 0;

	if (KAL_TEST_BIT(PCIE_MSI_TX_FREE_DONE, prMsiInfo->ulEnBits)) {
		*pu4IntStatus |= WHISR_RX0_DONE_INT;
		u4Value |=
			WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_7_MASK;
	}

	if (KAL_TEST_BIT(PCIE_MSI_RX_DATA_BAND0, prMsiInfo->ulEnBits)) {
		*pu4IntStatus |= WHISR_RX0_DONE_INT;
		u4Value |=
			WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_4_MASK;
	}

	if (KAL_TEST_BIT(PCIE_MSI_RX_DATA_BAND1, prMsiInfo->ulEnBits)) {
		*pu4IntStatus |= WHISR_RX0_DONE_INT;
		u4Value |=
			WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_5_MASK;
	}

	if (KAL_TEST_BIT(PCIE_MSI_EVENT, prMsiInfo->ulEnBits)) {
		*pu4IntStatus |= WHISR_RX0_DONE_INT;
		u4Value |=
			WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_6_MASK;
	}

#if (CFG_SUPPORT_DISABLE_CMD_DDONE_INTR == 0)
	if (KAL_TEST_BIT(PCIE_MSI_CMD, prMsiInfo->ulEnBits)) {
		*pu4IntStatus |= WHISR_TX_DONE_INT;
		u4WrValue |=
			WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_15_MASK;
	}
#endif /* CFG_SUPPORT_DISABLE_CMD_DDONE_INTR == 0 */

	if (KAL_TEST_BIT(PCIE_MSI_LUMP, prMsiInfo->ulEnBits)) {
		*pu4IntStatus |= WHISR_D2H_SW_INT;
		u4WrValue |= CONNAC_MCU_SW_INT;
	}

	/* force process all interrupt */
	if (KAL_TEST_BIT(GLUE_FLAG_HALT_BIT, prAdapter->prGlueInfo->ulFlag)) {
		*pu4IntStatus |= WHISR_RX0_DONE_INT | WHISR_TX_DONE_INT |
			WHISR_D2H_SW_INT;
		u4WrValue |= prBusInfo->host_int_rxdone_bits |
			prBusInfo->host_int_txdone_bits |
			CONNAC_MCU_SW_INT;
	}

	prHifInfo->u4IntStatus = u4Value | u4WrValue;

	/* clear interrupt */
	if (u4WrValue)
		HAL_MCR_WR(prAdapter, WF_WFDMA_HOST_DMA0_HOST_INT_STA_ADDR,
			   u4WrValue);
}
#endif

static void mt7935ReadIntStatus(struct ADAPTER *prAdapter,
		uint32_t *pu4IntStatus)
{
	struct GL_HIF_INFO *prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	struct mt66xx_chip_info *prChipInfo = prAdapter->chip_info;
	struct BUS_INFO *prBusInfo = prChipInfo->bus_info;
	uint32_t u4RegValue = 0, u4WrValue = 0, u4Addr;

	*pu4IntStatus = 0;

	u4Addr = WF_WFDMA_HOST_DMA0_HOST_INT_STA_ADDR;
	HAL_RMCR_RD(HIF_READ, prAdapter, u4Addr, &u4RegValue);

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
}

static void mt7935ConfigIntMask(struct GLUE_INFO *prGlueInfo,
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
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_HOST_RX_DONE_INT_ENA4_MASK |
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_HOST_RX_DONE_INT_ENA5_MASK |
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_HOST_RX_DONE_INT_ENA6_MASK |
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_HOST_RX_DONE_INT_ENA7_MASK |
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_HOST_RX_DONE_INT_ENA8_MASK |
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_HOST_RX_DONE_INT_ENA9_MASK |
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_HOST_RX_DONE_INT_ENA10_MASK |
#if (CFG_SUPPORT_DISABLE_DATA_DDONE_INTR == 0)
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_HOST_TX_DONE_INT_ENA0_MASK |
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_HOST_TX_DONE_INT_ENA1_MASK |
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_HOST_TX_DONE_INT_ENA2_MASK |
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_HOST_TX_DONE_INT_ENA3_MASK |
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_HOST_TX_DONE_INT_ENA4_MASK |
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_HOST_TX_DONE_INT_ENA5_MASK |
#endif /* CFG_SUPPORT_DISABLE_DATA_DDONE_INTR == 0 */
#if (CFG_SUPPORT_DISABLE_CMD_DDONE_INTR == 0)
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_HOST_TX_DONE_INT_ENA15_MASK |
#endif /* CFG_SUPPORT_DISABLE_CMD_DDONE_INTR */
#if (WFDMA_AP_MSI_NUM == 1)
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_HOST_TX_DONE_INT_ENA16_MASK |
#endif
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_mcu2host_sw_int_ena_MASK;

	HAL_MCR_WR(prGlueInfo->prAdapter, u4Addr, u4WrVal);
}

#if CFG_MTK_WIFI_WFDMA_WB
static void mt7935ReadIntStatusByEmi(struct ADAPTER *prAdapter,
				     uint32_t *pu4IntStatus)
{
	struct GL_HIF_INFO *prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	struct RTMP_DMABUF *prRingIntSta0 = &prHifInfo->rRingIntSta0;
#if CFG_ENABLE_MAWD_MD_RING
	struct RTMP_DMABUF *prRingIntSta1 = &prHifInfo->rRingIntSta1;
#endif
	uint32_t u4RegValue = 0, u4WrValue = 0, u4Addr;
	u_int8_t fgClrCr = FALSE;

	*pu4IntStatus = 0;
	u4RegValue = *((uint32_t *)prRingIntSta0->AllocVa);
	prHifInfo->u4IntStatus = u4RegValue & 0xFFFF;

	u4Addr = WF_WFDMA_HOST_DMA0_HOST_TX_INT_WB_EN_ADDR;
#if (CFG_SUPPORT_DISABLE_DATA_DDONE_INTR == 0)
	if (u4RegValue & BITS(0, 10)) {
		*pu4IntStatus |= WHISR_TX_DONE_INT;
		fgClrCr = TRUE;
	}
	u4WrValue |= BITS(0, 6);
#endif
#if (CFG_SUPPORT_DISABLE_CMD_DDONE_INTR == 0)
	if (u4RegValue & BITS(0, 10)) {
		*pu4IntStatus |= WHISR_TX_DONE_INT;
		fgClrCr = TRUE;
	}
	u4WrValue |= BITS(7, 10);
#endif
	if (u4RegValue & BITS(11, 15)) {
		*pu4IntStatus |= WHISR_RX0_DONE_INT;
		fgClrCr = TRUE;
	}
	u4WrValue |= BITS(11, 15);

	/* clear interrupt */
	if (fgClrCr) {
		HAL_MCR_WR(prAdapter, u4Addr, u4WrValue);
		DBGLOG(HAL, LOUD,
		       "EmiIntSta[0x%08x][0x%08x] CR[0x%08x]=[0x%08x]\n",
		       prHifInfo->u4IntStatus, u4RegValue,
		       u4Addr, u4WrValue);
	}

	/* clear err int */
	if (u4RegValue & BIT(27)) {
		*pu4IntStatus |= WHISR_D2H_SW_INT;
		u4Addr = WF_WFDMA_HOST_DMA0_HOST_INT_STA_ADDR;
		u4WrValue = CONNAC_MCU_SW_INT | CONNAC_SUBSYS_INT;
		HAL_MCR_WR(prAdapter, u4Addr, u4WrValue);
	}

#if CFG_ENABLE_MAWD_MD_RING
	fgClrCr = FALSE;
	u4RegValue = *((uint32_t *)prRingIntSta1->AllocVa);
	prHifInfo->u4IntStatus |= (u4RegValue << 16);

	u4Addr = WF_WFDMA_HOST_DMA0_HOST_RX_INT_WB_EN_ADDR;
	u4WrValue = 0;
	if (u4RegValue & BITS(8, 12)) {
		fgClrCr = TRUE;
		*pu4IntStatus |= WHISR_RX0_DONE_INT;
	}
	u4WrValue |= BITS(8, 12);

	/* clear interrupt */
	if (fgClrCr) {
		HAL_MCR_WR(prAdapter, u4Addr, u4WrValue);
		DBGLOG(HAL, LOUD,
		       "MdEmiIntSta[0x%08x][0x%08x] CR[0x%08x]=[0x%08x]\n",
		       prHifInfo->u4IntStatus, u4RegValue,
		       u4Addr, u4WrValue);
	}
#endif
}

static void mt7935ProcessTxInterruptByEmi(struct ADAPTER *prAdapter)
{
	struct GLUE_INFO *prGlueInfo = prAdapter->prGlueInfo;
	struct GL_HIF_INFO *prHifInfo = &prGlueInfo->rHifInfo;
	uint32_t u4Sta = prHifInfo->u4IntStatus;

	if (u4Sta & BIT(8))
		halWpdmaProcessCmdDmaDone(prGlueInfo, TX_RING_FWDL);

	if (u4Sta & BIT(7))
		halWpdmaProcessCmdDmaDone(prGlueInfo, TX_RING_CMD);

#if (CFG_SUPPORT_DISABLE_DATA_DDONE_INTR == 0)
	if (u4Sta & BIT(0)) {
		halWpdmaProcessDataDmaDone(pGlueInfo, TX_RING_DATA0);
		kalSetTxEvent2Hif(prGlueInfo);
	}

	if (u4Sta & BIT(1)) {
		halWpdmaProcessDataDmaDone(prGlueInfo, TX_RING_DATA1);
		kalSetTxEvent2Hif(prGlueInfo);
	}

	if (u4Sta & BIT(2)) {
		halWpdmaProcessDataDmaDone(pGlueInfo, TX_RING_DATA2);
		kalSetTxEvent2Hif(prGlueInfo);
	}

	if (u4Sta & BIT(3)) {
		halWpdmaProcessDataDmaDone(prGlueInfo, TX_RING_DATA3);
		kalSetTxEvent2Hif(prGlueInfo);
	}

	if (u4Sta & BIT(4)) {
		halWpdmaProcessDataDmaDone(prGlueInfo, TX_RING_DATA_PRIO);
		kalSetTxEvent2Hif(prGlueInfo);
	}

	if (u4Sta & BIT(5)) {
		halWpdmaProcessDataDmaDone(prGlueInfo, TX_RING_DATA_ALTX);
		kalSetTxEvent2Hif(prGlueInfo);
	}
#endif /* CFG_SUPPORT_DISABLE_DATA_DDONE_INTR == 0 */
}

static void mt7935ProcessRxDataInterruptByEmi(struct ADAPTER *prAdapter)
{
	struct GLUE_INFO *prGlueInfo = prAdapter->prGlueInfo;
	struct GL_HIF_INFO *prHifInfo = &prGlueInfo->rHifInfo;
	uint32_t u4Sta = prHifInfo->u4IntStatus;

	if ((u4Sta & BIT(11)) ||
	    (KAL_TEST_BIT(RX_RING_DATA0, prAdapter->ulNoMoreRfb)))
		halRxReceiveRFBs(prAdapter, RX_RING_DATA0, TRUE);

	if ((u4Sta & BIT(12)) ||
	    (KAL_TEST_BIT(RX_RING_DATA1, prAdapter->ulNoMoreRfb)))
		halRxReceiveRFBs(prAdapter, RX_RING_DATA1, TRUE);
}

static void mt7935ProcessRxInterruptByEmi(struct ADAPTER *prAdapter)
{
	struct GLUE_INFO *prGlueInfo = prAdapter->prGlueInfo;
	struct GL_HIF_INFO *prHifInfo = &prGlueInfo->rHifInfo;
	uint32_t u4Sta = prHifInfo->u4IntStatus;

	if ((u4Sta & BIT(14)) ||
	    (KAL_TEST_BIT(RX_RING_EVT, prAdapter->ulNoMoreRfb)))
		halRxReceiveRFBs(prAdapter, RX_RING_EVT, FALSE);

	if ((u4Sta & BIT(15)) ||
	    (KAL_TEST_BIT(RX_RING_TXDONE0, prAdapter->ulNoMoreRfb)))
		halRxReceiveRFBs(prAdapter, RX_RING_TXDONE0, FALSE);

#if CFG_ENABLE_MAWD_MD_RING
	if ((u4Sta & BIT(27)) ||
	    (KAL_TEST_BIT(RX_RING_TXDONE1, prAdapter->ulNoMoreRfb)))
		halRxReceiveRFBs(prAdapter, RX_RING_TXDONE1, FALSE);

	if ((u4Sta & BIT(28)) ||
	    (KAL_TEST_BIT(RX_RING_TXDONE2, prAdapter->ulNoMoreRfb)))
		halRxReceiveRFBs(prAdapter, RX_RING_TXDONE2, FALSE);
#endif /* CFG_ENABLE_MAWD_MD_RING */

	mt7935ProcessRxDataInterruptByEmi(prAdapter);
}

static void mt7935ConfigEmiIntMask(struct GLUE_INFO *prGlueInfo,
				   u_int8_t enable)
{
	struct ADAPTER *prAdapter = prGlueInfo->prAdapter;
	struct mt66xx_chip_info *prChipInfo;
	struct GL_HIF_INFO *prHifInfo;
	struct WIFI_VAR *prWifiVar;
	struct RTMP_DMABUF *prRingIdx0, *prRingIntSta0;
	struct RTMP_DMABUF *prRingIdx1, *prRingIntSta1;
	struct RTMP_DMABUF *prRingDmyRd;
	uint32_t u4Addr = 0, u4WrVal = 0;
	uint32_t u4DmyRdExt = 0;

	prChipInfo = prAdapter->chip_info;
	prHifInfo = &prGlueInfo->rHifInfo;
	prWifiVar = &prAdapter->rWifiVar;

	prRingIdx0 = &prHifInfo->rRingIdx0;
	prRingIntSta0 = &prHifInfo->rRingIntSta0;
	prRingIdx1 = &prHifInfo->rRingIdx1;
	prRingIntSta1 = &prHifInfo->rRingIntSta1;
	prRingDmyRd = &prHifInfo->rRingDmyRd;

	u4Addr = WF_WFDMA_HOST_DMA0_HOST_INT_ENA_ADDR;
	u4WrVal = 0;
	HAL_MCR_WR(prAdapter, u4Addr, u4WrVal);

	u4Addr = WF_WFDMA_HOST_DMA0_WPDMA_TRINFO_WB_DMY_CTRL2_ADDR;
	u4WrVal = ((uint64_t)prRingDmyRd->AllocPa) & DMA_LOWER_32BITS_MASK;
	HAL_MCR_WR(prAdapter, u4Addr, u4WrVal);

	u4DmyRdExt = (((uint64_t)prRingDmyRd->AllocPa >> DMA_BITS_OFFSET) <<
	WF_WFDMA_HOST_DMA0_WPDMA_TRINFO_WB_DMY_CTRL3_DMY_RD_BASE_PTR_EXT_SHFT) &
	WF_WFDMA_HOST_DMA0_WPDMA_TRINFO_WB_DMY_CTRL3_DMY_RD_BASE_PTR_EXT_MASK;
	u4Addr = WF_WFDMA_HOST_DMA0_WPDMA_TRINFO_WB_DMY_CTRL3_ADDR;
	u4WrVal = u4DmyRdExt |
		WF_WFDMA_HOST_DMA0_WPDMA_TRINFO_WB_CTRL2_MSI_DBG_MASK;
#if CFG_MTK_MDDP_SUPPORT
	/* enable md write back */
	u4WrVal |= BIT(10);
#endif
	HAL_MCR_WR(prAdapter, u4Addr, u4WrVal);

	u4Addr = WF_WFDMA_HOST_DMA0_WPDMA_TRINFO_WB_CTRL0_ADDR;
	u4WrVal = ((uint64_t)prRingIdx0->AllocPa) & DMA_LOWER_32BITS_MASK;
	HAL_MCR_WR(prAdapter, u4Addr, u4WrVal);

	u4Addr = WF_WFDMA_HOST_DMA0_WPDMA_TRINFO_WB_CTRL1_ADDR;
	u4WrVal = ((uint64_t)prRingIntSta0->AllocPa) & DMA_LOWER_32BITS_MASK;
	HAL_MCR_WR(prAdapter, u4Addr, u4WrVal);

	u4Addr = WF_WFDMA_HOST_DMA0_WPDMA_TRINFO_WB_CTRL2_ADDR;
	u4WrVal = WF_WFDMA_HOST_DMA0_WPDMA_TRINFO_WB_CTRL2_TRINFO_WB_EN_MASK |
		WF_WFDMA_HOST_DMA0_WPDMA_TRINFO_WB_CTRL2_NOC_BUS_SEL_MASK;
	u4WrVal |= (((uint64_t)prRingIdx0->AllocPa >> DMA_BITS_OFFSET) &
		 DMA_HIGHER_4BITS_MASK) <<
	WF_WFDMA_HOST_DMA0_WPDMA_TRINFO_WB_CTRL2_DIDX_WB_BASE_PTR_EXT_SHFT;
	u4WrVal |= (((uint64_t)prRingIntSta0->AllocPa >> DMA_BITS_OFFSET) &
		 DMA_HIGHER_4BITS_MASK) <<
	WF_WFDMA_HOST_DMA0_WPDMA_TRINFO_WB_CTRL2_INT_WB_BASE_PTR_EXT_SHFT;
	HAL_MCR_WR(prAdapter, u4Addr, u4WrVal);

	/* disable tx done interrupt */
	u4Addr = WF_WFDMA_HOST_DMA0_HOST_TX_INT_PCIE_SEL_ADDR;
	HAL_MCR_WR(prAdapter, u4Addr, 0xffffffff);

	u4Addr = WF_WFDMA_HOST_DMA0_HOST_TX_INT_WB_EN_ADDR;
	u4WrVal = enable ? 0xF800 : 0;
	HAL_MCR_WR(prAdapter, u4Addr, u4WrVal);

#if (CFG_MTK_MDDP_SUPPORT == 1) && (CFG_MTK_CCCI_SUPPORT == 1) && \
	(CFG_ENABLE_MAWD_MD_RING == 0)
	u4Addr = WF_WFDMA_HOST_DMA0_WPDMA_TRINFO_WB_MD_CTRL0_ADDR;
	u4WrVal = (prAdapter->u8MdRingIdxBase) & DMA_LOWER_32BITS_MASK;
	HAL_MCR_WR(prAdapter, u4Addr, u4WrVal);

	u4Addr = WF_WFDMA_HOST_DMA0_WPDMA_TRINFO_WB_MD_CTRL1_ADDR;
	u4WrVal = (prAdapter->u8MdRingStaBase) & DMA_LOWER_32BITS_MASK;
	HAL_MCR_WR(prAdapter, u4Addr, u4WrVal);

	u4Addr = WF_WFDMA_HOST_DMA0_WPDMA_TRINFO_WB_CTRL2_ADDR;
	u4WrVal = WF_WFDMA_HOST_DMA0_WPDMA_TRINFO_WB_CTRL2_TRINFO_WB_EN_MASK |
		WF_WFDMA_HOST_DMA0_WPDMA_TRINFO_WB_CTRL2_NOC_BUS_SEL_MASK;
	u4WrVal |= ((prAdapter->u8MdRingIdxBase >> DMA_BITS_OFFSET) &
		 DMA_HIGHER_4BITS_MASK) <<
	WF_WFDMA_HOST_DMA0_WPDMA_TRINFO_WB_CTRL2_DIDX_WB_BASE_PTR_EXT_SHFT;
	u4WrVal |= ((prAdapter->u8MdRingStaBase >> DMA_BITS_OFFSET) &
		 DMA_HIGHER_4BITS_MASK) <<
	WF_WFDMA_HOST_DMA0_WPDMA_TRINFO_WB_CTRL2_INT_WB_BASE_PTR_EXT_SHFT;
	HAL_MCR_WR(prAdapter, u4Addr, u4WrVal);
#else
	u4Addr = WF_WFDMA_HOST_DMA0_WPDMA_TRINFO_WB_MD_CTRL0_ADDR;
	u4WrVal = ((uint64_t)prRingIdx1->AllocPa) & DMA_LOWER_32BITS_MASK;
	HAL_MCR_WR(prAdapter, u4Addr, u4WrVal);

	u4Addr = WF_WFDMA_HOST_DMA0_WPDMA_TRINFO_WB_MD_CTRL1_ADDR;
	u4WrVal = ((uint64_t)prRingIntSta1->AllocPa) & DMA_LOWER_32BITS_MASK;
	HAL_MCR_WR(prAdapter, u4Addr, u4WrVal);

	u4Addr = WF_WFDMA_HOST_DMA0_WPDMA_TRINFO_WB_CTRL2_ADDR;
	u4WrVal = WF_WFDMA_HOST_DMA0_WPDMA_TRINFO_WB_CTRL2_TRINFO_WB_EN_MASK |
		WF_WFDMA_HOST_DMA0_WPDMA_TRINFO_WB_CTRL2_NOC_BUS_SEL_MASK;
	u4WrVal |= (((uint64_t)prRingIdx1->AllocPa >> DMA_BITS_OFFSET) &
		 DMA_HIGHER_4BITS_MASK) <<
	WF_WFDMA_HOST_DMA0_WPDMA_TRINFO_WB_CTRL2_DIDX_WB_BASE_PTR_EXT_SHFT;
	u4WrVal |= (((uint64_t)prRingIntSta1->AllocPa >> DMA_BITS_OFFSET) &
		 DMA_HIGHER_4BITS_MASK) <<
	WF_WFDMA_HOST_DMA0_WPDMA_TRINFO_WB_CTRL2_INT_WB_BASE_PTR_EXT_SHFT;
	HAL_MCR_WR(prAdapter, u4Addr, u4WrVal);

#if CFG_ENABLE_MAWD_MD_RING
	u4Addr = WF_WFDMA_HOST_DMA0_HOST_RX_INT_WB_EN_ADDR;
	u4WrVal = 0x3E00;
	HAL_MCR_WR(prAdapter, u4Addr, u4WrVal);
#endif /* CFG_ENABLE_MAWD_MD_RING */
#endif /* CFG_MTK_MDDP_SUPPORT */
}
#endif /* CFG_MTK_WIFI_WFDMA_WB */

#if defined(_HIF_PCIE) && (CFG_SUPPORT_PCIE_PLAT_INT_FLOW == 1)
static void mt7935EnableInterruptViaPcie(struct ADAPTER *prAdapter)
{
	struct mt66xx_chip_info *prChipInfo = prAdapter->chip_info;
	struct BUS_INFO *prBusInfo = prChipInfo->bus_info;
	/*
	 * Problem Statement:
	 * Current rx driver own flow is disable wfdma
	 * interrupt, then set driver own.
	 * It may cause Falcon enter sleep after disable
	 * wfdma interrupt and cause read driver own timeout.
	 *
	 * Solution:
	 * Confirmed with DE, correct rx driver own flow
	 * Set driver own and read driver own before disable/enable
	 * wfdma interrupt
	 */

	if (!prChipInfo->is_support_wfdma_write_back &&
	    prBusInfo->configWfdmaIntMask) {
		prBusInfo->configWfdmaIntMask(prAdapter->prGlueInfo, FALSE);
		prBusInfo->configWfdmaIntMask(prAdapter->prGlueInfo, TRUE);
	}
	asicConnac3xEnablePlatformIRQ(prAdapter);
}

static void mt7935DisableInterruptViaPcie(struct ADAPTER *prAdapter)
{
	asicConnac3xDisablePlatformIRQ(prAdapter);
}
#endif

static void mt7935EnableInterrupt(struct ADAPTER *prAdapter)
{
	struct BUS_INFO *prBusInfo = prAdapter->chip_info->bus_info;

	if (prBusInfo->configWfdmaIntMask)
		prBusInfo->configWfdmaIntMask(prAdapter->prGlueInfo, TRUE);
	asicConnac3xEnablePlatformIRQ(prAdapter);
}

static void mt7935DisableInterrupt(struct ADAPTER *prAdapter)
{
	struct BUS_INFO *prBusInfo = prAdapter->chip_info->bus_info;

	if (prBusInfo->configWfdmaIntMask)
		prBusInfo->configWfdmaIntMask(prAdapter->prGlueInfo, FALSE);
	asicConnac3xDisablePlatformIRQ(prAdapter);
}

static void mt7935WpdmaMsiConfig(struct ADAPTER *prAdapter)
{
/*
 * ilog2(WFDMA_AP_MSI 1) = WFDMA_AP_MSI_NUM for CR shitf
 * please do NOT use linux API, which is finally implemented in assembly
 */
#if (WFDMA_AP_MSI_NUM == 8)
#define WFDMA_AP_MSI_SETTING_VAL		3
#else
#define WFDMA_AP_MSI_SETTING_VAL		0
#endif

#if (WFDMA_MD_MSI_NUM == 8)
#define WFDMA_MD_MSI_SETTING_VAL		3
#else
#define WFDMA_MD_MSI_SETTING_VAL		0
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

#if (WFDMA_AP_MSI_NUM == 8)
	/* No need to read int status if msi num is 8 */
	prAdapter->rWifiVar.u4HifIstLoopCount = 1;

	/* enable msi 2~5 auto clear feature */
	u4Value = 0x3C;
	/* enable deassert timer */
	u4Value |= (0x40 <<
	WF_WFDMA_EXT_WRAP_CSR_WFDMA_MSI_CONFIG_msi_deassert_tmr_ticks_SHFT) |
	(1 << WF_WFDMA_EXT_WRAP_CSR_WFDMA_MSI_CONFIG_msi_deassert_tmr_en_SHFT);

	HAL_MCR_WR(prAdapter,
		   WF_WFDMA_EXT_WRAP_CSR_WFDMA_MSI_CONFIG_ADDR,
		   u4Value);
#endif

	/* configure MSI number */
	u4Value = ((WFDMA_AP_MSI_SETTING_VAL <<
		WF_WFDMA_EXT_WRAP_CSR_WFDMA_HOST_CONFIG_pcie0_msi_num_SHFT) &
		WF_WFDMA_EXT_WRAP_CSR_WFDMA_HOST_CONFIG_pcie0_msi_num_MASK);
#if CFG_MTK_MDDP_SUPPORT
	u4Value |= ((WFDMA_MD_MSI_SETTING_VAL <<
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
		WF_WFDMA_EXT_WRAP_CSR_WFDMA_MD_INT_LUMP_SEL_ADDR,
		u4Value);
#endif
}

static void mt7935ConfigWfdmaRxRingThreshold(
	struct ADAPTER *prAdapter, uint32_t u4Th, u_int8_t fgIsData)
{
	uint32_t u4Addr = 0, u4Val = 0, u4Num = 2;

	/* set rxq th to 1 if tput is high */
	if (u4Th == 2)
		u4Num = 1;

	u4Val = u4Num | (u4Num <<
		 WF_WFDMA_HOST_DMA0_WPDMA_PAUSE_RX_Q_TH10_RX_DMAD_TH1_SHFT);
	if (fgIsData) {
		u4Addr = WF_WFDMA_HOST_DMA0_WPDMA_PAUSE_RX_Q_TH54_ADDR;
		HAL_MCR_WR(prAdapter, u4Addr, u4Val);
		goto exit;
	}

	for (u4Addr = WF_WFDMA_HOST_DMA0_WPDMA_PAUSE_RX_Q_TH10_ADDR;
	     u4Addr <= WF_WFDMA_HOST_DMA0_WPDMA_PAUSE_RX_Q_TH1110_ADDR;
	     u4Addr += 0x4)
		HAL_MCR_WR(prAdapter, u4Addr, u4Val);

exit:
	DBGLOG(HAL, INFO, "Set WFDMA RxQ[%u] threshold[0x%08x]\n",
	       fgIsData, u4Val);
}

static void mt7935WpdmaDlyInt(struct GLUE_INFO *prGlueInfo)
{
	struct ADAPTER *prAdapter = prGlueInfo->prAdapter;
	struct WIFI_VAR *prWifiVar = &prAdapter->rWifiVar;
	uint32_t u4Addr, u4Val;

	/* Enable RX periodic delayed interrupt (unit: 20us) */
	u4Val = 0xF00000 | prWifiVar->u4PrdcIntTime;
	u4Addr = WF_WFDMA_HOST_DMA0_HOST_PER_DLY_INT_CFG_ADDR;
	HAL_MCR_WR(prAdapter, u4Addr, u4Val);

	u4Addr = WF_WFDMA_HOST_DMA0_WPDMA_PRI_DLY_INT_CFG2_ADDR;
	u4Val = prWifiVar->u4DlyIntTime <<
		WF_WFDMA_HOST_DMA0_WPDMA_PRI_DLY_INT_CFG2_PRI0_MAX_PTIME_SHFT |
		prWifiVar->u4DlyIntCnt <<
		WF_WFDMA_HOST_DMA0_WPDMA_PRI_DLY_INT_CFG2_PRI0_MAX_PINT_SHFT |
		prWifiVar->fgEnDlyInt <<
		WF_WFDMA_HOST_DMA0_WPDMA_PRI_DLY_INT_CFG2_PRI0_DLY_INT_EN_SHFT |
		prWifiVar->u4DlyIntTime <<
		WF_WFDMA_HOST_DMA0_WPDMA_PRI_DLY_INT_CFG2_PRI1_MAX_PTIME_SHFT |
		prWifiVar->u4DlyIntCnt <<
		WF_WFDMA_HOST_DMA0_WPDMA_PRI_DLY_INT_CFG2_PRI1_MAX_PINT_SHFT |
		prWifiVar->fgEnDlyInt <<
		WF_WFDMA_HOST_DMA0_WPDMA_PRI_DLY_INT_CFG2_PRI1_DLY_INT_EN_SHFT;
	HAL_MCR_WR(prAdapter, u4Addr, u4Val);

	DBGLOG(HAL, INFO, "prdc int: %uus, dly int[%u]: %uus, cnt=%u",
	       prWifiVar->u4PrdcIntTime * 20,
	       prWifiVar->fgEnDlyInt,
	       prWifiVar->u4DlyIntTime * 20,
	       prWifiVar->u4DlyIntCnt);
}

static void mt7935WpdmaConfigExt0(struct ADAPTER *prAdapter)
{
}

static void mt7935WpdmaConfigExt1(struct ADAPTER *prAdapter)
{
	uint32_t u4Addr = 0, u4Val = 0;

	/* packet based TX flow control */
	u4Addr = WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_EXT1_ADDR;
	/* default settings */
	u4Val = 0x8C800404 |
		WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_EXT1_CSR_TX_FCTRL_MODE_MASK;
	HAL_MCR_WR(prAdapter, u4Addr, u4Val);
}

static void mt7935WpdmaConfigExt2(struct ADAPTER *prAdapter)
{
	uint32_t u4Addr = 0, u4Val = 0;

	u4Addr = WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_EXT2_ADDR;
	/* default settings */
	u4Val = 0x00001404;
	HAL_MCR_WR(prAdapter, u4Addr, u4Val);
}

static void mt7935WfdmaControl(struct ADAPTER *prAdapter, u_int8_t fgEn)
{
	struct GL_HIF_INFO *prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	union WPDMA_GLO_CFG_STRUCT *prGloCfg = &prHifInfo->GloCfg;
	struct mt66xx_chip_info *prChipInfo = prAdapter->chip_info;
	struct BUS_INFO *prBusInfo = prChipInfo->bus_info;
	uint32_t u4Addr = 0;

	u4Addr = WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_ADDR;
	/* default settings */
	prGloCfg->word = 0x5030b850;

	if (fgEn) {
		prGloCfg->word |=
			WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_TX_DMA_EN_MASK |
			WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_RX_DMA_EN_MASK;
	}

	if (prBusInfo->u4DmaMask > 32) {
		prGloCfg->word |=
			WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_PDMA_ADDR_EXT_EN_MASK;
	}

	HAL_MCR_WR(prAdapter, u4Addr, prGloCfg->word);
	prHifInfo->GloCfg.word = prGloCfg->word;
}

static void mt7935WpdmaConfig(struct GLUE_INFO *prGlueInfo,
		u_int8_t enable, bool fgResetHif)
{
	struct ADAPTER *prAdapter = prGlueInfo->prAdapter;
	struct mt66xx_chip_info *prChipInfo = prAdapter->chip_info;
	struct BUS_INFO *prBusInfo = prChipInfo->bus_info;

	mt7935WfdmaControl(prAdapter, enable);

	if (!enable)
		return;

	if (prChipInfo->is_support_wfdma_write_back &&
	    prBusInfo->configWfdmaIntMask)
		prBusInfo->configWfdmaIntMask(prGlueInfo, TRUE);

#if defined(_HIF_PCIE)
	mt7935WpdmaMsiConfig(prAdapter);
#endif
	mt7935ConfigWfdmaRxRingThreshold(prAdapter, 0, FALSE);

	mt7935WpdmaConfigExt0(prAdapter);
	mt7935WpdmaConfigExt1(prAdapter);
	mt7935WpdmaConfigExt2(prAdapter);

	mt7935WpdmaDlyInt(prGlueInfo);
}

#if CFG_MTK_WIFI_WFDMA_WB
static void mt7935WfdmaTxRingWbExtCtrl(
	struct GLUE_INFO *prGlueInfo,
	struct RTMP_TX_RING *prTxRing,
	uint32_t u4Idx)
{
	struct mt66xx_chip_info *prChipInfo;
	struct GL_HIF_INFO *prHifInfo;
	struct WFDMA_EMI_RING_IDX_0 *prRingIdx0;
	struct WFDMA_EMI_RING_IDX_1 *prRingIdx1;
	int i4EmiRingIdx = -1;
	u_int8_t fgIsSet1 = FALSE;

	prChipInfo = prGlueInfo->prAdapter->chip_info;
	prHifInfo = &prGlueInfo->rHifInfo;
	prRingIdx0 = (struct WFDMA_EMI_RING_IDX_0 *)
		prHifInfo->rRingIdx0.AllocVa;
	prRingIdx1 = (struct WFDMA_EMI_RING_IDX_1 *)
		prHifInfo->rRingIdx1.AllocVa;

	if (!prChipInfo->is_support_wfdma_write_back)
		return;

	switch (u4Idx) {
	case TX_RING_DATA0:
		i4EmiRingIdx = 0;
		break;
	case TX_RING_DATA1:
		i4EmiRingIdx = 1;
		break;
	case TX_RING_DATA2:
		i4EmiRingIdx = 2;
		break;
	case TX_RING_DATA3:
#if CFG_ENABLE_MAWD_MD_RING
		fgIsSet1 = TRUE;
		i4EmiRingIdx = 0;
#else
		i4EmiRingIdx = 3;
#endif
		break;
	case TX_RING_DATA_PRIO:
#if CFG_ENABLE_MAWD_MD_RING
		fgIsSet1 = TRUE;
		i4EmiRingIdx = 1;
#else
		i4EmiRingIdx = 4;
#endif
		break;
	case TX_RING_DATA_ALTX:
#if CFG_ENABLE_MAWD_MD_RING
		fgIsSet1 = TRUE;
		i4EmiRingIdx = 2;
#else
		i4EmiRingIdx = 5;
#endif
		break;
	case TX_RING_CMD:
		i4EmiRingIdx = 7;
		break;
	case TX_RING_FWDL:
		i4EmiRingIdx = 8;
		break;
	default:
		return;
	}

	if (fgIsSet1)
		prTxRing->pu2EmiIdx = &prRingIdx1->u2TxRing[i4EmiRingIdx];
	else
		prTxRing->pu2EmiIdx = &prRingIdx0->u2TxRing[i4EmiRingIdx];

	prTxRing->fgEnEmiIdx = TRUE;
	*prTxRing->pu2EmiIdx = 0;
}

static void mt7935WfdmaRxRingWbExtCtrl(
	struct GLUE_INFO *prGlueInfo,
	struct RTMP_RX_RING *prRxRing,
	uint32_t u4Idx)
{
	struct mt66xx_chip_info *prChipInfo;
	struct GL_HIF_INFO *prHifInfo;
	struct WFDMA_EMI_RING_IDX_0 *prRingIdx0;
	struct WFDMA_EMI_RING_IDX_1 *prRingIdx1;
	int i4EmiRingIdx = -1;
	u_int8_t fgIsSet1 = FALSE;

	prChipInfo = prGlueInfo->prAdapter->chip_info;
	prHifInfo = &prGlueInfo->rHifInfo;
	prRingIdx0 = (struct WFDMA_EMI_RING_IDX_0 *)
		prHifInfo->rRingIdx0.AllocVa;
	prRingIdx1 = (struct WFDMA_EMI_RING_IDX_1 *)
		prHifInfo->rRingIdx1.AllocVa;

	if (!prChipInfo->is_support_wfdma_write_back)
		return;

	switch (u4Idx) {
	case RX_RING_EVT:
		i4EmiRingIdx = 3;
		break;
	case RX_RING_DATA0:
		i4EmiRingIdx = 0;
		break;
	case RX_RING_DATA1:
		i4EmiRingIdx = 1;
		break;
	case RX_RING_DATA2:
		i4EmiRingIdx = 2;
		break;
	case RX_RING_TXDONE0:
		i4EmiRingIdx = 4;
		break;
#if CFG_ENABLE_MAWD_MD_RING
	case RX_RING_DATA3:
		fgIsSet1 = TRUE;
		i4EmiRingIdx = 0;
		break;
	case RX_RING_DATA4:
		fgIsSet1 = TRUE;
		i4EmiRingIdx = 1;
		break;
	case RX_RING_DATA5:
		fgIsSet1 = TRUE;
		i4EmiRingIdx = 2;
		break;
	case RX_RING_TXDONE1:
		fgIsSet1 = TRUE;
		i4EmiRingIdx = 3;
		break;
	case RX_RING_TXDONE2:
		fgIsSet1 = TRUE;
		i4EmiRingIdx = 4;
		break;
#endif /* CFG_ENABLE_MAWD_MD_RING */
	default:
		return;
	}

	if (fgIsSet1)
		prRxRing->pu2EmiIdx = &prRingIdx1->u2RxRing[i4EmiRingIdx];
	else
		prRxRing->pu2EmiIdx = &prRingIdx0->u2RxRing[i4EmiRingIdx];

	prRxRing->fgEnEmiIdx = TRUE;
	*prRxRing->pu2EmiIdx = 0;
}
#endif /* CFG_MTK_WIFI_WFDMA_WB */

static void mt7935WfdmaTxRingExtCtrl(
	struct GLUE_INFO *prGlueInfo,
	struct RTMP_TX_RING *prTxRing,
	u_int32_t index)
{
	struct BUS_INFO *prBusInfo;
	struct ADAPTER *prAdapter;
	struct mt66xx_chip_info *prChipInfo;
	uint32_t u4Offset = 0, u4RingIdx = 0;

	prAdapter = prGlueInfo->prAdapter;
	prChipInfo = prAdapter->chip_info;
	prBusInfo = prChipInfo->bus_info;

	switch (index) {
	case TX_RING_DATA0:
		u4RingIdx = prBusInfo->tx_ring0_data_idx;
		break;
	case TX_RING_DATA1:
		u4RingIdx = prBusInfo->tx_ring1_data_idx;
		break;
	case TX_RING_DATA2:
		u4RingIdx = prBusInfo->tx_ring2_data_idx;
		break;
	case TX_RING_DATA3:
		u4RingIdx = prBusInfo->tx_ring3_data_idx;
		break;
	case TX_RING_DATA_PRIO:
		u4RingIdx = prBusInfo->tx_prio_data_idx;
		break;
	case TX_RING_DATA_ALTX:
		u4RingIdx = prBusInfo->tx_altx_data_idx;
		break;
	case TX_RING_CMD:
		u4RingIdx = prBusInfo->tx_ring_cmd_idx;
		break;
	case TX_RING_FWDL:
		u4RingIdx = prBusInfo->tx_ring_fwdl_idx;
		break;
	default:
		u4RingIdx = index;
		break;

	}
	u4Offset = u4RingIdx * 4;

	prTxRing->hw_desc_base_ext =
		prBusInfo->host_tx_ring_ext_ctrl_base + u4Offset;
	HAL_MCR_WR(prAdapter, prTxRing->hw_desc_base_ext,
		   CONNAC3X_TX_RING_DISP_MAX_CNT);

	asicConnac3xWfdmaTxRingBasePtrExtCtrl(prGlueInfo,
		prTxRing, index);

#if CFG_MTK_WIFI_WFDMA_WB
	mt7935WfdmaTxRingWbExtCtrl(prGlueInfo, prTxRing, index);
#endif /* CFG_MTK_WIFI_WFDMA_WB */
}

static void mt7935WfdmaRxRingExtCtrl(
	struct GLUE_INFO *prGlueInfo,
	struct RTMP_RX_RING *prRxRing,
	u_int32_t index)
{
	struct ADAPTER *prAdapter;
	struct mt66xx_chip_info *prChipInfo;
	struct BUS_INFO *prBusInfo;
	uint32_t u4Offset = 0, u4RingIdx = 0;

	prAdapter = prGlueInfo->prAdapter;
	prChipInfo = prAdapter->chip_info;
	prBusInfo = prChipInfo->bus_info;

	switch (index) {
	case RX_RING_EVT:
		u4RingIdx = 7;
		break;
	case RX_RING_DATA0:
		u4RingIdx = 4;
		break;
	case RX_RING_DATA1:
		u4RingIdx = 6;
		break;
	case RX_RING_TXDONE0:
		u4RingIdx = 5;
		break;
	case RX_RING_TXDONE1:
		u4RingIdx = 8;
		break;
	case RX_RING_TXDONE2:
		u4RingIdx = 9;
		break;
	case RX_RING_WAEVT0:
		u4RingIdx = 10;
		break;
	default:
		DBGLOG(RX, ERROR, "Error index=%d\n", index);
		return;
	}
	u4Offset = u4RingIdx * 4;

	prRxRing->hw_desc_base_ext =
		prBusInfo->host_rx_ring_ext_ctrl_base + u4Offset;
	HAL_MCR_WR(prAdapter, prRxRing->hw_desc_base_ext,
		   CONNAC3X_RX_RING_DISP_MAX_CNT);

	asicConnac3xWfdmaRxRingBasePtrExtCtrl(prGlueInfo,
		prRxRing, index);

#if CFG_MTK_WIFI_WFDMA_WB
	mt7935WfdmaRxRingWbExtCtrl(prGlueInfo, prRxRing, index);
#endif /* CFG_MTK_WIFI_WFDMA_WB */
}

static void mt7935InitPcieInt(struct GLUE_INFO *prGlueInfo)
{
}

static void mt7935ShowPcieDebugInfo(struct GLUE_INFO *prGlueInfo)
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

static void mt7935SetupMcuEmiAddr(struct ADAPTER *prAdapter)
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
		   MT7935_EMI_SIZE_ADDR,
		   size);
}

static u_int8_t mt7935_get_sw_interrupt_status(struct ADAPTER *prAdapter,
	uint32_t *pu4Status)
{
	*pu4Status = ccif_get_interrupt_status(prAdapter);
	return TRUE;
}

static uint32_t mt7935_ccif_get_interrupt_status(struct ADAPTER *ad)
{
	uint32_t u4Status = 0;

	HAL_RMCR_RD(CCIF_READ, ad,
		AP2WF_CONN_INFRA_ON_CCIF4_AP2WF_PCCIF_RCHNUM_ADDR,
		&u4Status);
	HAL_MCR_WR(ad,
		AP2WF_CONN_INFRA_ON_CCIF4_AP2WF_PCCIF_ACK_ADDR,
		u4Status);

	return u4Status;
}

static void mt7935_ccif_notify_utc_time_to_fw(struct ADAPTER *ad,
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

static void mt7935_ccif_set_fw_log_read_pointer(struct ADAPTER *ad,
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

static uint32_t mt7935_ccif_get_fw_log_read_pointer(struct ADAPTER *ad,
	enum ENUM_FW_LOG_CTRL_TYPE type)
{
	uint32_t u4Addr = 0, u4Value = 0;

	if (type == ENUM_FW_LOG_CTRL_TYPE_MCU)
		u4Addr = WF2AP_CONN_INFRA_ON_CCIF4_WF2AP_PCCIF_DUMMY2_ADDR;
	else
		u4Addr = WF2AP_CONN_INFRA_ON_CCIF4_WF2AP_PCCIF_DUMMY1_ADDR;

	HAL_RMCR_RD(CCIF_READ, ad, u4Addr, &u4Value);

	return u4Value;
}

static int32_t mt7935_ccif_trigger_fw_assert(struct ADAPTER *ad)
{
	HAL_MCR_WR(ad,
		AP2WF_CONN_INFRA_ON_CCIF4_AP2WF_PCCIF_TCHNUM_ADDR,
		SW_INT_SUBSYS_RESET);

	return 0;
}

u_int8_t mt7935_is_ap2conn_off_readable(struct ADAPTER *ad)
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
		HAL_RMCR_RD(PLAT_DBG, ad,
			   CONN_DBG_CTL_CONN_INFRA_BUS_CLK_DETECT_ADDR,
			   &value);
		if ((value & BIT(1)) && (value & BIT(3)))
			break;

		retry++;
		kalMdelay(1);
	}

	HAL_RMCR_RD(PLAT_DBG, ad,
		   CONN_CFG_IP_VERSION_IP_VERSION_ADDR,
		   &value);
	if (value != MT7935_CONNINFRA_VERSION_ID) {
		DBGLOG(HAL, ERROR,
			"Conninfra ver id: 0x%08x\n",
			value);
		return FALSE;
	}

	HAL_RMCR_RD(PLAT_DBG, ad,
		   CONN_DBG_CTL_CONN_INFRA_BUS_DBG_CR_00_ADDR,
		   &value);
	if ((value & BITS(0, 9)) == 0x3FF)
		DBGLOG(HAL, ERROR,
			"Conninfra bus hang irq status: 0x%08x\n",
			value);

	return TRUE;
}

u_int8_t mt7935_is_conn2wf_readable(struct ADAPTER *ad)
{
	uint32_t value = 0;

	HAL_RMCR_RD(PLAT_DBG, ad,
		   CONN_BUS_CR_ADDR_CONN2SUBSYS_0_AHB_GALS_DBG_ADDR,
		   &value);
	if ((value & BIT(26)) != 0x0) {
		DBGLOG(HAL, ERROR,
			"conn2wf sleep protect: 0x%08x\n",
			value);
		return FALSE;
	}

	HAL_RMCR_RD(PLAT_DBG, ad,
		   WF_TOP_CFG_IP_VERSION_ADDR,
		   &value);
	if (value != MT7935_WF_VERSION_ID) {
		DBGLOG(HAL, ERROR,
			"WF ver id: 0x%08x\n",
			value);
		return FALSE;
	}

	HAL_RMCR_RD(PLAT_DBG, ad,
		   CONN_DBG_CTL_WF_MCUSYS_INFRA_VDNR_GEN_DEBUG_CTRL_AO_BUS_TIMEOUT_IRQ_ADDR,
		   &value);
	if ((value & BIT(0)) != 0x0) {
		DBGLOG(HAL, WARN,
			"WF mcusys bus hang irq status: 0x%08x\n",
			value);
		HAL_RMCR_RD(PLAT_DBG, ad,
			   CONN_DBG_CTL_CONN_INFRA_BUS_DBG_CR_00_ADDR,
			   &value);
		if (value == 0x100)
			DBGLOG(HAL, INFO,
				"Skip conn_infra_vdnr timeout irq.\n");
		else
			return FALSE;
	}

	return TRUE;
}

static u_int8_t mt7935_check_recovery_needed(struct ADAPTER *ad)
{
	uint32_t u4Value = 0;
	u_int8_t fgResult = FALSE;

	/*
	 * if (0x81021604[31:16]==0xdead &&
	 *     (0x70005350[30:28]!=0x0 || 0x70005360[6:4]!=0x0)) == 0x1
	 * do recovery flow
	 */

	HAL_RMCR_RD(ONOFF_READ, ad, WF_TOP_CFG_ON_ROMCODE_INDEX_ADDR,
		&u4Value);
	DBGLOG(INIT, INFO, "0x%08x=0x%08x\n",
		WF_TOP_CFG_ON_ROMCODE_INDEX_ADDR, u4Value);
	if ((u4Value & 0xFFFF0000) != 0xDEAD0000) {
		fgResult = FALSE;
		goto exit;
	}

	HAL_RMCR_RD(ONOFF_READ, ad, CBTOP_GPIO_MODE5_ADDR,
		&u4Value);
	DBGLOG(INIT, INFO, "0x%08x=0x%08x\n",
		CBTOP_GPIO_MODE5_ADDR, u4Value);
	if (((u4Value & CBTOP_GPIO_MODE5_GPIO47_MASK) >>
	    CBTOP_GPIO_MODE5_GPIO47_SHFT) != 0x0) {
		fgResult = TRUE;
		goto exit;
	}

	HAL_RMCR_RD(ONOFF_READ, ad, CBTOP_GPIO_MODE6_ADDR,
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

static uint32_t mt7935_mcu_reinit(struct ADAPTER *ad)
{
#define CONNINFRA_ID_MAX_POLLING_COUNT		10

	uint32_t u4Value = 0, u4PollingCnt = 0;
	uint32_t rStatus = WLAN_STATUS_SUCCESS;

	/* Check recovery needed */
	if (mt7935_check_recovery_needed(ad) == FALSE)
		goto exit;

	DBGLOG(INIT, INFO, "mt7935_mcu_reinit.\n");

	/* Force on conninfra */
	HAL_MCR_WR(ad,
		CONN_HOST_CSR_TOP_CONN_INFRA_WAKEPU_TOP_ADDR,
		0x1);

	/* Wait conninfra wakeup */
	while (TRUE) {
		HAL_RMCR_RD(ONOFF_READ, ad,
			CONN_CFG_IP_VERSION_IP_VERSION_ADDR,
			&u4Value);

		if (u4Value == MT7935_CONNINFRA_VERSION_ID)
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

	HAL_RMCR_RD(ONOFF_READ, ad, CBTOP_GPIO_MODE5_ADDR, &u4Value);
	DBGLOG(INIT, INFO, "0x%08x=0x%08x\n",
		CBTOP_GPIO_MODE5_ADDR, u4Value);

	HAL_RMCR_RD(ONOFF_READ, ad, CBTOP_GPIO_MODE6_ADDR, &u4Value);
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
static uint32_t mt7935_mcu_reset(struct ADAPTER *ad)
{
	uint32_t u4Value = 0;
	uint32_t rStatus = WLAN_STATUS_SUCCESS;

	DBGLOG(INIT, INFO, "mt7935_mcu_reset..\n");

	HAL_RMCR_RD(RESET_READ, ad,
		CB_INFRA_RGU_WF_SUBSYS_RST_ADDR,
		&u4Value);
	u4Value &= ~CB_INFRA_RGU_WF_SUBSYS_RST_WF_SUBSYS_RST_MASK;
	u4Value |= (0x1 << CB_INFRA_RGU_WF_SUBSYS_RST_WF_SUBSYS_RST_SHFT);
	HAL_MCR_WR(ad,
		CB_INFRA_RGU_WF_SUBSYS_RST_ADDR,
		u4Value);

	kalMdelay(1);

	HAL_RMCR_RD(RESET_READ, ad,
		CB_INFRA_RGU_WF_SUBSYS_RST_ADDR,
		&u4Value);
	u4Value &= ~CB_INFRA_RGU_WF_SUBSYS_RST_WF_SUBSYS_RST_MASK;
	u4Value |= (0x0 << CB_INFRA_RGU_WF_SUBSYS_RST_WF_SUBSYS_RST_SHFT);
	HAL_MCR_WR(ad,
		CB_INFRA_RGU_WF_SUBSYS_RST_ADDR,
		u4Value);

	HAL_RMCR_RD(RESET_READ, ad,
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

#if (CFG_MTK_FPGA_PLATFORM == 0)
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
#endif

static uint32_t mt7935_mcu_init(struct ADAPTER *ad)
{
#if (0)
#define MCU_IDLE		0x1D1E

	uint32_t u4Value = 0, u4PollingCnt = 0;
	uint32_t rStatus = WLAN_STATUS_SUCCESS;

	if (!ad) {
		DBGLOG(INIT, ERROR, "NULL ADAPTER.\n");
		rStatus = WLAN_STATUS_FAILURE;
		goto exit;
	}

#if (CFG_MTK_FPGA_PLATFORM == 0)
	set_cbinfra_remap(ad);
#endif

#if (CFG_MTK_ANDROID_WMT == 0) && (CFG_MTK_FPGA_PLATFORM == 0)
	rStatus = mt7935_mcu_reset(ad);
	if (rStatus != WLAN_STATUS_SUCCESS)
		goto dump;
#endif

	while (TRUE) {
		if (u4PollingCnt >= 1000) {
			DBGLOG(INIT, ERROR, "timeout.\n");
			rStatus = WLAN_STATUS_FAILURE;
			goto dump;
		}

		HAL_RMCR_RD(ONOFF_READ, ad, WF_TOP_CFG_ON_ROMCODE_INDEX_ADDR,
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
		goto dump;
	}
#endif

	if (ad->chip_info->coexpccifon)
		ad->chip_info->coexpccifon(ad);

dump:
	if (rStatus != WLAN_STATUS_SUCCESS) {
		DBGLOG(INIT, ERROR, "u4Value: 0x%x\n",
			u4Value);
		mt7935_dumpWfsyscpupcr(ad);
		mt7935_dumpPcGprLog(ad);
		mt7935_dumpN45CoreReg(ad);
		mt7935_dumpWfTopReg(ad);
		mt7935_dumpWfBusReg(ad);

		/* Clock detection for ULPOSC */
		HAL_MCR_WR(ad,
			   VLP_UDS_CTRL_CBTOP_ULPOSC_CTRL1_ADDR,
			   0x06030138);
		HAL_MCR_WR(ad,
			   CB_CKGEN_TOP_CBTOP_ULPOSC_1_ADDR,
			   0x000f0000);
		HAL_MCR_WR(ad,
			   CB_CKGEN_TOP_CBTOP_ULPOSC_1_ADDR,
			   0x001f0000);
		HAL_MCR_WR(ad,
			   CB_CKGEN_TOP_CBTOP_ULPOSC_1_ADDR,
			   0x011f0000);
		kalUdelay(1);
		HAL_RMCR_RD(ONOFF_DBG, ad,
			   CB_CKGEN_TOP_CBTOP_ULPOSC_2_ADDR,
			   &u4Value);
		DBGLOG(INIT, INFO,
			"0x%08x=0x%08x\n",
			CB_CKGEN_TOP_CBTOP_ULPOSC_2_ADDR,
			u4Value);
		HAL_RMCR_RD(ONOFF_DBG, ad,
			   CB_INFRA_SLP_CTRL_CB_INFRA_CRYPTO_TOP_MCU_OWN_ADDR,
			   &u4Value);
		DBGLOG(INIT, INFO,
			"0x%08x=0x%08x\n",
			CB_INFRA_SLP_CTRL_CB_INFRA_CRYPTO_TOP_MCU_OWN_ADDR,
			u4Value);
	}

exit:
	return rStatus;
#else
	return 0;
#endif
}

static void mt7935_mcu_deinit(struct ADAPTER *ad)
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

	if (ad->chip_info->coexpccifoff)
		ad->chip_info->coexpccifoff(ad);
}

static int32_t mt7935_trigger_fw_assert(struct ADAPTER *prAdapter)
{
	int32_t ret = 0;

	ccif_trigger_fw_assert(prAdapter);

#if CFG_WMT_RESET_API_SUPPORT
	ret = reset_wait_for_trigger_completion();
#endif

	return ret;
}

#define MCIF_EMI_MEMORY_SIZE 128
#define MCIF_EMI_COEX_SWMSG_OFFSET 0xF8518000
#define MCIF_EMI_BASE_OFFSET 0xE4
static int mt7935ConnacPccifOn(struct ADAPTER *prAdapter)
{
#if CFG_MTK_CCCI_SUPPORT
	uint32_t mcif_emi_base, u4Val = 0;
	void *vir_addr = NULL;
	int size = 0;

#if CFG_MTK_ANDROID_WMT
#if IS_ENABLED(CFG_MTK_WIFI_CONNV3_SUPPORT)
	if (is_pwr_on_notify_processing())
		return -1;
#endif
#endif

	mcif_emi_base = get_smem_phy_start_addr(
		MD_SYS1, SMEM_USER_RAW_MD_CONSYS, &size);
	if (!mcif_emi_base) {
		DBGLOG(INIT, ERROR, "share memory is NULL.\n");
		return -1;
	}

	vir_addr = ioremap(mcif_emi_base, MCIF_EMI_MEMORY_SIZE);
	if (!vir_addr) {
		DBGLOG(INIT, ERROR, "ioremap fail.\n");
		return -1;
	}

#if CFG_MTK_WIFI_WFDMA_WB
#if CFG_MTK_MDDP_SUPPORT
	if (size >= (WFDMA_WB_MEMORY_SIZE * 2)) {
		prAdapter->u8MdRingStaBase =
			((uint64_t)mcif_emi_base + size - WFDMA_WB_MEMORY_SIZE);
		prAdapter->u8MdRingIdxBase =
			prAdapter->u8MdRingStaBase - WFDMA_WB_MEMORY_SIZE;
	}
#endif /* CFG_MTK_MDDP_SUPPORT */
#endif /* CFG_MTK_WIFI_WFDMA_WB */

	/* To Do */
	/*kalDevRegWrite(
		NULL,
		CONN_BUS_CR_VON_CONN_INFRA_PCIE2AP_REMAP_WF_1_BA_ADDR,
		0x18051803);
	*/

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
	HAL_MCR_WR(prAdapter, MT7935_MCIF_MD_STATE_WHEN_WIFI_ON_ADDR, u4Val);

	DBGLOG(INIT, TRACE, "MCIF_EMI_BASE_OFFSET=[0x%08x]\n", u4Val);
	DBGLOG_MEM128(HAL, TRACE, vir_addr, MCIF_EMI_MEMORY_SIZE);

	iounmap(vir_addr);
#else
	DBGLOG(INIT, ERROR, "[%s] ECCCI Driver is not supported.\n", __func__);
#endif
	return 0;
}

static int mt7935ConnacPccifOff(struct ADAPTER *prAdapter)
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

static int mt7935_CheckBusHang(void *priv, uint8_t rst_enable)
{
	struct ADAPTER *ad = priv;
	u_int8_t readable = FALSE;

	if (fgIsBusAccessFailed) {
		readable = FALSE;
		goto exit;
	}

	if (mt7935_is_ap2conn_off_readable(ad) &&
	    mt7935_is_conn2wf_readable(ad))
		readable = TRUE;
	else
		readable = FALSE;

exit:
	return readable ? 0 : 1;
}

static uint32_t mt7935_wlanDownloadPatch(struct ADAPTER *prAdapter)
{
	uint32_t status  = wlanDownloadPatch(prAdapter);

	if (status == WLAN_STATUS_SUCCESS)
		wifi_coredump_set_enable(TRUE);

	return status;
}
#endif /* _HIF_PCIE */

static uint32_t mt7935GetFlavorVer(uint8_t *flavor)
{
	uint32_t ret = WLAN_STATUS_FAILURE;
	uint32_t u4StrLen = 0;
	uint8_t aucFlavor[CFG_FW_FLAVOR_MAX_LEN] = {0};

	if (kalGetFwFlavor(&aucFlavor[0]) == 1) {
		u4StrLen = kalStrnLen(aucFlavor,
						CFG_FW_FLAVOR_MAX_LEN);
		if (u4StrLen == 1) {
			kalScnprintf(flavor,
					CFG_FW_FLAVOR_MAX_LEN,
					"%u%s", CFG_WIFI_IP_SET, aucFlavor);
		} else {
			kalScnprintf(flavor,
					CFG_FW_FLAVOR_MAX_LEN,
					"%s", aucFlavor);
		}
		ret = WLAN_STATUS_SUCCESS;
	} else if (kalScnprintf(flavor,
					CFG_FW_FLAVOR_MAX_LEN,
					"1") > 0) {
		ret = WLAN_STATUS_SUCCESS;
	} else {
		ret = WLAN_STATUS_FAILURE;
	}

	return ret;
}

static void mt7935WiFiNappingCtrl(
	struct GLUE_INFO *prGlueInfo, u_int8_t fgEn)
{
	struct mt66xx_chip_info *prChipInfo = NULL;
	uint32_t u4value = 0;
	u_int8_t fgNappingEn = FALSE;

	if (!prGlueInfo->prAdapter) {
		DBGLOG(HAL, ERROR, "adapter is null\n");
		return;
	}

	prChipInfo = prGlueInfo->prAdapter->chip_info;

	if (prChipInfo->fgWifiNappingForceDisable)
		fgNappingEn = FALSE;
	else
		fgNappingEn = fgEn;

	/* return if setting no chang */
	if (prChipInfo->fgWifiNappingEn == fgNappingEn)
		return;

	prChipInfo->fgWifiNappingEn = fgNappingEn;

	/*
	 * [0]: 1: set wf bus active from wf napping sleep by driver.
	 *      0: set wf bus goes back to napping sleep by driver
	 */
	if (fgNappingEn)
		u4value = CONN_AON_WF_NAPPING_ENABLE;
	else
		u4value = CONN_AON_WF_NAPPING_DISABLE;

	DBGLOG(INIT, TRACE,
		"fgEn[%u] fgNappingEn[%u], WrAddr[0x%08x]=[0x%08x]\n",
		fgEn, fgNappingEn,
		CONN_HOST_CSR_TOP_ADDR_CR_CONN_AON_TOP_RESERVE_ADDR,
		u4value);
	HAL_MCR_WR(prGlueInfo->prAdapter,
		   CONN_HOST_CSR_TOP_ADDR_CR_CONN_AON_TOP_RESERVE_ADDR,
		   u4value);
}
#endif  /* MT7935 */
