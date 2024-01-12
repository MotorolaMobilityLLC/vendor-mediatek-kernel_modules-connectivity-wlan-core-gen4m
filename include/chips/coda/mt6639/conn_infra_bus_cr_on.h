/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2021 MediaTek Inc.
 */

#ifndef __CONN_INFRA_BUS_CR_ON_REGS_H__
#define __CONN_INFRA_BUS_CR_ON_REGS_H__

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************
*
*                     CONN_INFRA_BUS_CR_ON CR Definitions
*
*****************************************************************************
*/

#define CONN_INFRA_BUS_CR_ON_BASE                              0x7C00E000

#define CONN_INFRA_BUS_CR_ON_CONN_INFRA_PCIE2AP_REMAP_WF__5__4_ADDR \
	(CONN_INFRA_BUS_CR_ON_BASE + 0x418)

#define CONN_INFRA_BUS_CR_ON_CONN_INFRA_PCIE2AP_REMAP_WF__5__4_cr_pcie2ap_public_remapping_wf_5_ADDR \
	CONN_INFRA_BUS_CR_ON_CONN_INFRA_PCIE2AP_REMAP_WF__5__4_ADDR
#define CONN_INFRA_BUS_CR_ON_CONN_INFRA_PCIE2AP_REMAP_WF__5__4_cr_pcie2ap_public_remapping_wf_5_MASK \
	0xFFFF0000
#define CONN_INFRA_BUS_CR_ON_CONN_INFRA_PCIE2AP_REMAP_WF__5__4_cr_pcie2ap_public_remapping_wf_5_SHFT \
	16

#ifdef __cplusplus
}
#endif

#endif
