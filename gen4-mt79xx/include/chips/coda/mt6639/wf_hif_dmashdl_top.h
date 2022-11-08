/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2021 MediaTek Inc.
 */

#ifndef __WF_HIF_DMASHDL_TOP_REGS_H__
#define __WF_HIF_DMASHDL_TOP_REGS_H__

#ifdef __cplusplus
extern "C" {
#endif


/* ************************************************************************** */
/*  */
/* WF_HIF_DMASHDL_TOP CR Definitions */
/*  */
/* ************************************************************************** */

#define WF_HIF_DMASHDL_TOP_BASE                                0x7C026000

#define WF_HIF_DMASHDL_TOP_WACPU_REFILL_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x00) /* 6000 */
#define WF_HIF_DMASHDL_TOP_SW_CONTROL_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x04) /* 6004 */
#define WF_HIF_DMASHDL_TOP_OPTIONAL_CONTROL_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x08) /* 6008 */
#define WF_HIF_DMASHDL_TOP_PAGE_SETTING_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x0C) /* 600C */
#define WF_HIF_DMASHDL_TOP_REFILL_CONTROL_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x10) /* 6010 */
#define WF_HIF_DMASHDL_TOP_PKT_IN_CNT_CTRL_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x14) /* 6014 */
#define WF_HIF_DMASHDL_TOP_CONTROL_SIGNAL_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x18) /* 6018 */
#define WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x1c) /* 601C */
#define WF_HIF_DMASHDL_TOP_GROUP0_CONTROL_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x20) /* 6020 */
#define WF_HIF_DMASHDL_TOP_GROUP1_CONTROL_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x24) /* 6024 */
#define WF_HIF_DMASHDL_TOP_GROUP2_CONTROL_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x28) /* 6028 */
#define WF_HIF_DMASHDL_TOP_GROUP3_CONTROL_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x2C) /* 602C */
#define WF_HIF_DMASHDL_TOP_GROUP4_CONTROL_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x30) /* 6030 */
#define WF_HIF_DMASHDL_TOP_GROUP5_CONTROL_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x34) /* 6034 */
#define WF_HIF_DMASHDL_TOP_GROUP6_CONTROL_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x38) /* 6038 */
#define WF_HIF_DMASHDL_TOP_GROUP7_CONTROL_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x3C) /* 603C */
#define WF_HIF_DMASHDL_TOP_GROUP8_CONTROL_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x40) /* 6040 */
#define WF_HIF_DMASHDL_TOP_GROUP9_CONTROL_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x44) /* 6044 */
#define WF_HIF_DMASHDL_TOP_GROUP10_CONTROL_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x48) /* 6048 */
#define WF_HIF_DMASHDL_TOP_GROUP11_CONTROL_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x4C) /* 604C */
#define WF_HIF_DMASHDL_TOP_GROUP12_CONTROL_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x50) /* 6050 */
#define WF_HIF_DMASHDL_TOP_GROUP13_CONTROL_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x54) /* 6054 */
#define WF_HIF_DMASHDL_TOP_GROUP14_CONTROL_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x58) /* 6058 */
#define WF_HIF_DMASHDL_TOP_GROUP15_CONTROL_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x5C) /* 605C */
#define WF_HIF_DMASHDL_TOP_QUEUE_MAPPING0_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x60) /* 6060 */
#define WF_HIF_DMASHDL_TOP_QUEUE_MAPPING1_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x64) /* 6064 */
#define WF_HIF_DMASHDL_TOP_QUEUE_MAPPING2_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x68) /* 6068 */
#define WF_HIF_DMASHDL_TOP_QUEUE_MAPPING3_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x6c) /* 606C */
#define WF_HIF_DMASHDL_TOP_HIF_SCHEDULER_SETTING0_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x70) /* 6070 */
#define WF_HIF_DMASHDL_TOP_HIF_SCHEDULER_SETTING1_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x74) /* 6074 */
#define WF_HIF_DMASHDL_TOP_SLOT_SETTING0_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x78) /* 6078 */
#define WF_HIF_DMASHDL_TOP_SLOT_SETTING1_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x7C) /* 607C */
#define WF_HIF_DMASHDL_TOP_SLOT_PERIOD_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x80) /* 6080 */
#define WF_HIF_DMASHDL_TOP_QUEUE_MAPPING4_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x84) /* 6084 */
#define WF_HIF_DMASHDL_TOP_DEBUG_PORT00_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0xC0) /* 60C0 */
#define WF_HIF_DMASHDL_TOP_DEBUG_PORT01_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0xC4) /* 60C4 */
#define WF_HIF_DMASHDL_TOP_STATUS_RD02_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0xC8) /* 60C8 */
#define WF_HIF_DMASHDL_TOP_COUNTER_UDF_CLR_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0xCC) /* 60CC */
#define WF_HIF_DMASHDL_TOP_TX_PACKET_SIZE_PAGE_MAP_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0xD0) /* 60D0 */
#define WF_HIF_DMASHDL_TOP_HIF_ASK_LONG_CHECK_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0xD4) /* 60D4 */
#define WF_HIF_DMASHDL_TOP_CPU_QUOTA_SET_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0xD8) /* 60D8 */
#define WF_HIF_DMASHDL_TOP_ERROR_FLAG_CTRL_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0xDC) /* 60DC */
#define WF_HIF_DMASHDL_TOP_STATUS_RD_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x100) /* 6100 */
#define WF_HIF_DMASHDL_TOP_STATUS_RD0_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x110) /* 6110 */
#define WF_HIF_DMASHDL_TOP_STATUS_RD_GP0_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x140) /* 6140 */
#define WF_HIF_DMASHDL_TOP_STATUS_RD_GP1_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x144) /* 6144 */
#define WF_HIF_DMASHDL_TOP_STATUS_RD_GP2_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x148) /* 6148 */
#define WF_HIF_DMASHDL_TOP_STATUS_RD_GP3_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x14c) /* 614C */
#define WF_HIF_DMASHDL_TOP_STATUS_RD_GP4_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x150) /* 6150 */
#define WF_HIF_DMASHDL_TOP_STATUS_RD_GP5_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x154) /* 6154 */
#define WF_HIF_DMASHDL_TOP_STATUS_RD_GP6_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x158) /* 6158 */
#define WF_HIF_DMASHDL_TOP_STATUS_RD_GP7_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x15C) /* 615C */
#define WF_HIF_DMASHDL_TOP_STATUS_RD_GP8_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x160) /* 6160 */
#define WF_HIF_DMASHDL_TOP_STATUS_RD_GP9_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x164) /* 6164 */
#define WF_HIF_DMASHDL_TOP_STATUS_RD_GP10_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x168) /* 6168 */
#define WF_HIF_DMASHDL_TOP_STATUS_RD_GP11_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x16c) /* 616C */
#define WF_HIF_DMASHDL_TOP_STATUS_RD_GP12_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x170) /* 6170 */
#define WF_HIF_DMASHDL_TOP_STATUS_RD_GP13_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x174) /* 6174 */
#define WF_HIF_DMASHDL_TOP_STATUS_RD_GP14_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x178) /* 6178 */
#define WF_HIF_DMASHDL_TOP_STATUS_RD_GP15_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x17C) /* 617C */
#define WF_HIF_DMASHDL_TOP_RD_GROUP_PKT_CNT0_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x180) /* 6180 */
#define WF_HIF_DMASHDL_TOP_RD_GROUP_PKT_CNT1_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x184) /* 6184 */
#define WF_HIF_DMASHDL_TOP_RD_GROUP_PKT_CNT2_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x188) /* 6188 */
#define WF_HIF_DMASHDL_TOP_RD_GROUP_PKT_CNT3_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x18c) /* 618C */
#define WF_HIF_DMASHDL_TOP_RD_GROUP_PKT_CNT4_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x190) /* 6190 */
#define WF_HIF_DMASHDL_TOP_RD_GROUP_PKT_CNT5_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x194) /* 6194 */
#define WF_HIF_DMASHDL_TOP_RD_GROUP_PKT_CNT6_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x198) /* 6198 */
#define WF_HIF_DMASHDL_TOP_RD_GROUP_PKT_CNT7_ADDR \
	(WF_HIF_DMASHDL_TOP_BASE + 0x19c) /* 619C */



#define WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_PLE_PACKET_MAX_SIZE_ADDR \
	WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_ADDR
#define WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_PLE_PACKET_MAX_SIZE_MASK 0x00000FFF
	/* PLE_PACKET_MAX_SIZE[11..0] */
#define WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_PLE_PACKET_MAX_SIZE_SHFT 0

#define WF_HIF_DMASHDL_TOP_REFILL_CONTROL_GROUP0_REFILL_DISABLE_ADDR \
	WF_HIF_DMASHDL_TOP_REFILL_CONTROL_ADDR
#define WF_HIF_DMASHDL_TOP_REFILL_CONTROL_GROUP0_REFILL_DISABLE_MASK 0x00010000
	/* GROUP0_REFILL_DISABLE[16] */
#define WF_HIF_DMASHDL_TOP_REFILL_CONTROL_GROUP0_REFILL_DISABLE_SHFT 16

#define WF_HIF_DMASHDL_TOP_GROUP0_CONTROL_GROUP0_MAX_QUOTA_ADDR \
	WF_HIF_DMASHDL_TOP_GROUP0_CONTROL_ADDR
#define WF_HIF_DMASHDL_TOP_GROUP0_CONTROL_GROUP0_MAX_QUOTA_MASK 0x0FFF0000
	/* GROUP0_MAX_QUOTA[27..16] */
#define WF_HIF_DMASHDL_TOP_GROUP0_CONTROL_GROUP0_MAX_QUOTA_SHFT 16

#define WF_HIF_DMASHDL_TOP_QUEUE_MAPPING0_QUEUE0_MAPPING_ADDR \
	WF_HIF_DMASHDL_TOP_QUEUE_MAPPING0_ADDR
#define WF_HIF_DMASHDL_TOP_QUEUE_MAPPING0_QUEUE0_MAPPING_MASK  0x0000000F
	/* QUEUE0_MAPPING[3..0] */
#define WF_HIF_DMASHDL_TOP_QUEUE_MAPPING0_QUEUE0_MAPPING_SHFT  0

#define WF_HIF_DMASHDL_TOP_PAGE_SETTING_GROUP_SEQUENCE_ORDER_TYPE_ADDR \
	WF_HIF_DMASHDL_TOP_PAGE_SETTING_ADDR
#define WF_HIF_DMASHDL_TOP_PAGE_SETTING_GROUP_SEQUENCE_ORDER_TYPE_MASK \
	0x00010000 /* GROUP_SEQUENCE_ORDER_TYPE[16] */
#define WF_HIF_DMASHDL_TOP_PAGE_SETTING_GROUP_SEQUENCE_ORDER_TYPE_SHFT 16

#define WF_HIF_DMASHDL_TOP_HIF_SCHEDULER_SETTING0_PRIORITY0_GROUP_ADDR \
	WF_HIF_DMASHDL_TOP_HIF_SCHEDULER_SETTING0_ADDR
#define WF_HIF_DMASHDL_TOP_HIF_SCHEDULER_SETTING0_PRIORITY0_GROUP_MASK \
	0x0000000F /* PRIORITY0_GROUP[3..0] */
#define WF_HIF_DMASHDL_TOP_HIF_SCHEDULER_SETTING0_PRIORITY0_GROUP_SHFT 0

#define WF_HIF_DMASHDL_TOP_STATUS_RD_GP0_G0_RSV_CNT_ADDR \
	WF_HIF_DMASHDL_TOP_STATUS_RD_GP0_ADDR
#define WF_HIF_DMASHDL_TOP_STATUS_RD_GP0_G0_RSV_CNT_MASK       0x0FFF0000
	/* G0_RSV_CNT[27..16] */
#define WF_HIF_DMASHDL_TOP_STATUS_RD_GP0_G0_RSV_CNT_SHFT       16

#define WF_HIF_DMASHDL_TOP_STATUS_RD_GP0_G0_SRC_CNT_ADDR \
	WF_HIF_DMASHDL_TOP_STATUS_RD_GP0_ADDR
#define WF_HIF_DMASHDL_TOP_STATUS_RD_GP0_G0_SRC_CNT_MASK       0x00000FFF
	/* G0_SRC_CNT[11..0] */
#define WF_HIF_DMASHDL_TOP_STATUS_RD_GP0_G0_SRC_CNT_SHFT       0

#define WF_HIF_DMASHDL_TOP_OPTIONAL_CONTROL_CR_HIF_ACK_CNT_TH_ADDR \
	WF_HIF_DMASHDL_TOP_OPTIONAL_CONTROL_ADDR
#define WF_HIF_DMASHDL_TOP_OPTIONAL_CONTROL_CR_HIF_ACK_CNT_TH_MASK 0x00FF0000
	/* CR_HIF_ACK_CNT_TH[23..16] */
#define WF_HIF_DMASHDL_TOP_OPTIONAL_CONTROL_CR_HIF_ACK_CNT_TH_SHFT 16

#define WF_HIF_DMASHDL_TOP_OPTIONAL_CONTROL_CR_HIF_GUP_ACT_MAP_ADDR \
	WF_HIF_DMASHDL_TOP_OPTIONAL_CONTROL_ADDR
#define WF_HIF_DMASHDL_TOP_OPTIONAL_CONTROL_CR_HIF_GUP_ACT_MAP_MASK 0x0000FFFF
	/* CR_HIF_GUP_ACT_MAP[15..0] */
#define WF_HIF_DMASHDL_TOP_OPTIONAL_CONTROL_CR_HIF_GUP_ACT_MAP_SHFT 0

#define WF_HIF_DMASHDL_TOP_STATUS_RD_FFA_CNT_ADDR \
	WF_HIF_DMASHDL_TOP_STATUS_RD_ADDR
#define WF_HIF_DMASHDL_TOP_STATUS_RD_FFA_CNT_MASK              0x00000FFF
	/* FFA_CNT[11..0] */
#define WF_HIF_DMASHDL_TOP_STATUS_RD_FFA_CNT_SHFT              0

#define WF_HIF_DMASHDL_TOP_STATUS_RD_FREE_PAGE_CNT_ADDR \
	WF_HIF_DMASHDL_TOP_STATUS_RD_ADDR
#define WF_HIF_DMASHDL_TOP_STATUS_RD_FREE_PAGE_CNT_MASK        0x0FFF0000
	/* FREE_PAGE_CNT[27..16] */
#define WF_HIF_DMASHDL_TOP_STATUS_RD_FREE_PAGE_CNT_SHFT        16

#define WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_PSE_PACKET_MAX_SIZE_ADDR \
	WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_ADDR
#define WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_PSE_PACKET_MAX_SIZE_MASK 0x0FFF0000
	/* PSE_PACKET_MAX_SIZE[27..16] */
#define WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_PSE_PACKET_MAX_SIZE_SHFT 16

#define WF_HIF_DMASHDL_TOP_GROUP0_CONTROL_GROUP0_MIN_QUOTA_ADDR \
	WF_HIF_DMASHDL_TOP_GROUP0_CONTROL_ADDR
#define WF_HIF_DMASHDL_TOP_GROUP0_CONTROL_GROUP0_MIN_QUOTA_MASK 0x00000FFF
	/* GROUP0_MIN_QUOTA[11..0] */
#define WF_HIF_DMASHDL_TOP_GROUP0_CONTROL_GROUP0_MIN_QUOTA_SHFT 0

#ifdef __cplusplus
}
#endif

#endif /* __WF_HIF_DMASHDL_TOP_REGS_H__ */
