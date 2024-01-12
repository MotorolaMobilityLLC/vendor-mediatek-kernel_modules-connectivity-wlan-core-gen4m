/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2020 MediaTek Inc.
 */

/*! \file  soc5_0.h
*    \brief This file contains the info of soc5_0
*/

#ifdef SOC5_0

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/
#define CONN_INFRA_CFG_BASE					0x830C0000
#define SOC5_0_TX_DESC_APPEND_LENGTH				32
#define SOC5_0_RX_DESC_LENGTH					24
#define SOC5_0_CHIP_ID						0x7961
#define CONNAC2X_TOP_HCR					0x88000000
#define CONNAC2X_TOP_HVR					0x88000000
#define CONNAC2X_TOP_FVR					0x88000004
#define SOC5_0_TOP_CFG_BASE					CONN_CFG_BASE
#define SOC5_0_PATCH_START_ADDR					0x00900000
#define SOC5_0_ARB_AC_MODE_ADDR					(0x820E315C)
#define MTK_CUSTOM_OID_INTERFACE_VERSION     0x00000200	/* for WPDWifi DLL */
#define MTK_EM_INTERFACE_VERSION		0x0001
#define RX_DATA_RING_BASE_IDX					2

#define CONN_INFRA_CFG_AP2WF_REMAP_1_ADDR \
	(CONN_INFRA_CFG_BASE + 0x0120)

#define CONN_INFRA_CFG_PCIE2AP_REMAP_2_ADDR \
	(0x7C00E000 + 0x24C)

/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/
void soc5_0_show_wfdma_info(struct ADAPTER *prAdapter);
void soc5_0_show_ple_info(struct ADAPTER *prAdapter, u_int8_t fgDumpTxd);
void soc5_0_show_pse_info(struct ADAPTER *prAdapter);
bool soc5_0_show_host_csr_info(struct ADAPTER *prAdapter);
void soc5_0_show_wfdma_dbg_probe_info(IN struct ADAPTER *prAdapter,
	IN enum _ENUM_WFDMA_TYPE_T enum_wfdma_type);
void soc5_0_show_wfdma_wrapper_info(IN struct ADAPTER *prAdapter,
	IN enum _ENUM_WFDMA_TYPE_T enum_wfdma_type);

#endif  /* soc5_0 */
