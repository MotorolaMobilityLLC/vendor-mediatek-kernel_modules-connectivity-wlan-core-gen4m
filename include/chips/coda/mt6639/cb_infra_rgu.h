/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2021 MediaTek Inc.
 */

#ifndef __CB_INFRA_RGU_REGS_H__
#define __CB_INFRA_RGU_REGS_H__

#include "hal_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CB_INFRA_RGU_BASE \
	0x70028000

#define CB_INFRA_RGU_HIF_MEM_CTL_PD_ADDR \
	(CB_INFRA_RGU_BASE + 0x01C)


#define CB_INFRA_RGU_HIF_MEM_CTL_PD_DUMMY_ECO_ADDR \
	CB_INFRA_RGU_HIF_MEM_CTL_PD_ADDR
#define CB_INFRA_RGU_HIF_MEM_CTL_PD_DUMMY_ECO_MASK \
	0xFF000000
#define CB_INFRA_RGU_HIF_MEM_CTL_PD_DUMMY_ECO_SHFT \
	24

#ifdef __cplusplus
}
#endif

#endif
