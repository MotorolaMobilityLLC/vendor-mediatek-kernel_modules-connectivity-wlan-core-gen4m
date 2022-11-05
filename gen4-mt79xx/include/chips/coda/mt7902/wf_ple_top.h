/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

#ifndef __WF_PLE_TOP_REGS_H__
#define __WF_PLE_TOP_REGS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* ************************************************************************** */
/* */
/* WF_PLE_TOP CR Definitions */
/* */
/* ************************************************************************** */

#define WF_PLE_TOP_BASE 0x820C0000

#define WF_PLE_TOP_PBUF_CTRL_ADDR \
	(WF_PLE_TOP_BASE + 0x04) /* 0004 */
#define WF_PLE_TOP_PBUF_CTRL_PAGE_SIZE_CFG_MASK \
	0x80000000 /* PAGE_SIZE_CFG[31] */
#define WF_PLE_TOP_PBUF_CTRL_PAGE_SIZE_CFG_SHFT 31
#define WF_PLE_TOP_PBUF_CTRL_PBUF_OFFSET_MASK \
	0x03FE0000 /* PBUF_OFFSET[25..17] */
#define WF_PLE_TOP_PBUF_CTRL_PBUF_OFFSET_SHFT 17
#define WF_PLE_TOP_PBUF_CTRL_TOTAL_PAGE_NUM_MASK \
	0x00000FFF /* TOTAL_PAGE_NUM[11..0] */
#define WF_PLE_TOP_PBUF_CTRL_TOTAL_PAGE_NUM_SHFT 0

#define WF_PLE_TOP_PG_HIF_GROUP_ADDR \
	(WF_PLE_TOP_BASE + 0x0c) /* 000C */
#define WF_PLE_TOP_PG_HIF_GROUP_HIF_MAX_QUOTA_MASK \
	0x0FFF0000 /* HIF_MAX_QUOTA[27..16] */
#define WF_PLE_TOP_PG_HIF_GROUP_HIF_MAX_QUOTA_SHFT 16
#define WF_PLE_TOP_PG_HIF_GROUP_HIF_MIN_QUOTA_MASK \
	0x00000FFF /* HIF_MIN_QUOTA[11..0] */
#define WF_PLE_TOP_PG_HIF_GROUP_HIF_MIN_QUOTA_SHFT 0

#define WF_PLE_TOP_PG_HIF_WMTXD_GROUP_ADDR \
	(WF_PLE_TOP_BASE + 0x10) /* 0010 */
#define WF_PLE_TOP_PG_HIF_WMTXD_GROUP_HIF_WMTXD_MAX_QUOTA_MASK \
	0x0FFF0000 /* HIF_WMTXD_MAX_QUOTA[27..16] */
#define WF_PLE_TOP_PG_HIF_WMTXD_GROUP_HIF_WMTXD_MAX_QUOTA_SHFT 16
#define WF_PLE_TOP_PG_HIF_WMTXD_GROUP_HIF_WMTXD_MIN_QUOTA_MASK \
	0x00000FFF /* HIF_WMTXD_MIN_QUOTA[11..0] */
#define WF_PLE_TOP_PG_HIF_WMTXD_GROUP_HIF_WMTXD_MIN_QUOTA_SHFT 0

#define WF_PLE_TOP_PG_HIF_TXCMD_GROUP_ADDR \
	(WF_PLE_TOP_BASE + 0x14) /* 0014 */
#define WF_PLE_TOP_PG_HIF_TXCMD_GROUP_HIF_TXCMD_MAX_QUOTA_MASK \
	0x0FFF0000 /* HIF_TXCMD_MAX_QUOTA[27..16] */
#define WF_PLE_TOP_PG_HIF_TXCMD_GROUP_HIF_TXCMD_MAX_QUOTA_SHFT 16
#define WF_PLE_TOP_PG_HIF_TXCMD_GROUP_HIF_TXCMD_MIN_QUOTA_MASK \
	0x00000FFF /* HIF_TXCMD_MIN_QUOTA[11..0] */
#define WF_PLE_TOP_PG_HIF_TXCMD_GROUP_HIF_TXCMD_MIN_QUOTA_SHFT 0

#define WF_PLE_TOP_PG_CPU_GROUP_ADDR \
	(WF_PLE_TOP_BASE + 0x18) /* 0018 */
#define WF_PLE_TOP_PG_CPU_GROUP_CPU_MAX_QUOTA_MASK \
	0x0FFF0000 /* CPU_MAX_QUOTA[27..16] */
#define WF_PLE_TOP_PG_CPU_GROUP_CPU_MAX_QUOTA_SHFT 16
#define WF_PLE_TOP_PG_CPU_GROUP_CPU_MIN_QUOTA_MASK \
	0x00000FFF /* CPU_MIN_QUOTA[11..0] */
#define WF_PLE_TOP_PG_CPU_GROUP_CPU_MIN_QUOTA_SHFT 0

#define WF_PLE_TOP_STATION_PAUSE0_ADDR \
	(WF_PLE_TOP_BASE + 0x100) /* 0100 */

#define WF_PLE_TOP_DIS_STA_MAP0_ADDR \
	(WF_PLE_TOP_BASE + 0x180) /* 0180 */

#define WF_PLE_TOP_INT_N9_ERR_STS_ADDR \
	(WF_PLE_TOP_BASE + 0x304) /* 0304 */

#define WF_PLE_TOP_INT_N9_ERR_STS_1_ADDR \
	(WF_PLE_TOP_BASE + 0x308) /* 0308 */

#define WF_PLE_TOP_QUEUE_EMPTY_ADDR \
	(WF_PLE_TOP_BASE + 0x360) /* 0360 */
#define WF_PLE_TOP_QUEUE_EMPTY_ALL_AC_EMPTY_MASK \
	0x01000000 /* ALL_AC_EMPTY[24] */

#define WF_PLE_TOP_FREEPG_CNT_ADDR \
	(WF_PLE_TOP_BASE + 0x380) /* 0380 */
#define WF_PLE_TOP_FREEPG_CNT_FFA_CNT_MASK 0x0FFF0000 /* FFA_CNT[27..16] */
#define WF_PLE_TOP_FREEPG_CNT_FFA_CNT_SHFT 16
#define WF_PLE_TOP_FREEPG_CNT_FREEPG_CNT_MASK 0x00000FFF /* FREEPG_CNT[11..0] */
#define WF_PLE_TOP_FREEPG_CNT_FREEPG_CNT_SHFT 0

#define WF_PLE_TOP_FREEPG_HEAD_TAIL_ADDR \
	(WF_PLE_TOP_BASE + 0x384) /* 0384 */
#define WF_PLE_TOP_FREEPG_HEAD_TAIL_FREEPG_TAIL_MASK  \
	0x0FFF0000 /* FREEPG_TAIL[27..16] */
#define WF_PLE_TOP_FREEPG_HEAD_TAIL_FREEPG_TAIL_SHFT 16
#define WF_PLE_TOP_FREEPG_HEAD_TAIL_FREEPG_HEAD_MASK  \
	0x00000FFF /* FREEPG_HEAD[11..0] */
#define WF_PLE_TOP_FREEPG_HEAD_TAIL_FREEPG_HEAD_SHFT 0

#define WF_PLE_TOP_HIF_PG_INFO_ADDR \
	(WF_PLE_TOP_BASE + 0x388) /* 0388 */
#define WF_PLE_TOP_HIF_PG_INFO_HIF_SRC_CNT_MASK \
	0x0FFF0000 /* HIF_SRC_CNT[27..16] */
#define WF_PLE_TOP_HIF_PG_INFO_HIF_SRC_CNT_SHFT 16
#define WF_PLE_TOP_HIF_PG_INFO_HIF_RSV_CNT_MASK \
	0x00000FFF /* HIF_RSV_CNT[11..0] */
#define WF_PLE_TOP_HIF_PG_INFO_HIF_RSV_CNT_SHFT 0

#define WF_PLE_TOP_HIF_WMTXD_PG_INFO_ADDR \
	(WF_PLE_TOP_BASE + 0x38c) /* 038C */
#define WF_PLE_TOP_HIF_WMTXD_PG_INFO_HIF_WMTXD_SRC_CNT_MASK  \
	0x0FFF0000 /* HIF_WMTXD_SRC_CNT[27..16] */
#define WF_PLE_TOP_HIF_WMTXD_PG_INFO_HIF_WMTXD_SRC_CNT_SHFT 16
#define WF_PLE_TOP_HIF_WMTXD_PG_INFO_HIF_WMTXD_RSV_CNT_MASK  \
	0x00000FFF /* HIF_WMTXD_RSV_CNT[11..0] */
#define WF_PLE_TOP_HIF_WMTXD_PG_INFO_HIF_WMTXD_RSV_CNT_SHFT 0

#define WF_PLE_TOP_HIF_TXCMD_PG_INFO_ADDR \
	(WF_PLE_TOP_BASE + 0x390) /* 0390 */
#define WF_PLE_TOP_HIF_TXCMD_PG_INFO_HIF_TXCMD_SRC_CNT_MASK  \
	0x0FFF0000 /* HIF_TXCMD_SRC_CNT[27..16] */
#define WF_PLE_TOP_HIF_TXCMD_PG_INFO_HIF_TXCMD_SRC_CNT_SHFT 16
#define WF_PLE_TOP_HIF_TXCMD_PG_INFO_HIF_TXCMD_RSV_CNT_MASK  \
	0x00000FFF /* HIF_TXCMD_RSV_CNT[11..0] */
#define WF_PLE_TOP_HIF_TXCMD_PG_INFO_HIF_TXCMD_RSV_CNT_SHFT 0

#define WF_PLE_TOP_CPU_PG_INFO_ADDR \
	(WF_PLE_TOP_BASE + 0x394) /* 0394 */
#define WF_PLE_TOP_CPU_PG_INFO_CPU_SRC_CNT_MASK \
	0x0FFF0000 /* CPU_SRC_CNT[27..16] */
#define WF_PLE_TOP_CPU_PG_INFO_CPU_SRC_CNT_SHFT 16
#define WF_PLE_TOP_CPU_PG_INFO_CPU_RSV_CNT_MASK \
	0x00000FFF /* CPU_RSV_CNT[11..0] */
#define WF_PLE_TOP_CPU_PG_INFO_CPU_RSV_CNT_SHFT 0

#define WF_PLE_TOP_FL_QUE_CTRL_0_ADDR \
	(WF_PLE_TOP_BASE + 0x3e0) /* 03E0 */
#define WF_PLE_TOP_FL_QUE_CTRL_0_EXECUTE_MASK 0x80000000 /* EXECUTE[31] */
#define WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_QID_SHFT 24
#define WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_PID_SHFT 10
#define WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_WLANID_SHFT 0

#define WF_PLE_TOP_FL_QUE_CTRL_2_ADDR \
	(WF_PLE_TOP_BASE + 0x3e8) /* 03E8 */
#define WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_TAIL_FID_MASK  \
	0x0FFF0000 /* QUEUE_TAIL_FID[27..16] */
#define WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_TAIL_FID_SHFT 16
#define WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_HEAD_FID_MASK  \
	0x00000FFF /* QUEUE_HEAD_FID[11..0] */
#define WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_HEAD_FID_SHFT 0

#define WF_PLE_TOP_FL_QUE_CTRL_3_ADDR \
	(WF_PLE_TOP_BASE + 0x3ec) /* 03EC */
#define WF_PLE_TOP_FL_QUE_CTRL_3_QUEUE_PKT_NUM_MASK  \
	0x00000FFF /* QUEUE_PKT_NUM[11..0] */
#define WF_PLE_TOP_FL_QUE_CTRL_3_QUEUE_PKT_NUM_SHFT 0

#define WF_PLE_TOP_AC0_QUEUE_EMPTY0_ADDR \
	(WF_PLE_TOP_BASE + 0x600) /* 0600 */

#define WF_PLE_TOP_AC1_QUEUE_EMPTY0_ADDR \
	(WF_PLE_TOP_BASE + 0x680) /* 0680 */

#define WF_PLE_TOP_AC2_QUEUE_EMPTY0_ADDR \
	(WF_PLE_TOP_BASE + 0x700) /* 0700 */

#define WF_PLE_TOP_AC3_QUEUE_EMPTY0_ADDR \
	(WF_PLE_TOP_BASE + 0x780) /* 0780 */

#ifdef __cplusplus
}
#endif

#endif /* __WF_PLE_TOP_REGS_H__ */
