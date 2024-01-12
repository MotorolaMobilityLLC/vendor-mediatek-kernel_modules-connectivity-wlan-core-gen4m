/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2020 MediaTek Inc.
 */

#ifndef __WF_TOP_CFG_ON_REGS_H__
#define __WF_TOP_CFG_ON_REGS_H__

#include "hal_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define WF_TOP_CFG_ON_BASE \
	0x81021000

#define WF_TOP_CFG_ON_ROMCODE_INDEX_ADDR \
	(WF_TOP_CFG_ON_BASE + 0x604)

#define WF_TOP_CFG_ON_ROMCODE_INDEX_ROM_CODE_INDEX_ADDR \
	WF_TOP_CFG_ON_ROMCODE_INDEX_ADDR
#define WF_TOP_CFG_ON_ROMCODE_INDEX_ROM_CODE_INDEX_MASK \
	0xFFFFFFFF
#define WF_TOP_CFG_ON_ROMCODE_INDEX_ROM_CODE_INDEX_SHFT \
	0

#ifdef __cplusplus
}
#endif

#endif
