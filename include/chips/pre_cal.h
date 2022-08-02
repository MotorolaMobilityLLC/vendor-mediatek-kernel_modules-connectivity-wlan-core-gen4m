/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2020 MediaTek Inc.
 */
#ifndef _PRE_CAL_H
#define _PRE_CAL_H

#if (CFG_SUPPORT_PRE_ON_PHY_ACTION == 1)
#if CFG_MTK_ANDROID_WMT
extern void update_pre_cal_status(uint8_t fgIsPreCal);
extern int8_t get_pre_cal_status(void);
extern int32_t get_wifi_process_status(void);
extern int32_t get_wifi_powered_status(void);
extern int32_t update_wr_mtx_down_up_status(uint8_t ucDownUp,
		uint8_t ucIsBlocking);
int wlanGetCalResultCb(uint32_t *pEmiCalOffset, uint32_t *pEmiCalSize);
int wlanPreCalPwrOn(void);
int wlanPreCal(void);
int wlanPreCalErr(void);
void set_cal_enabled(u_int8_t enabled);
u_int8_t is_cal_flow_finished(void);
#endif
uint32_t wlanPhyAction(struct ADAPTER *prAdapter);
uint8_t *wlanGetCalResult(uint32_t *prCalSize);
void wlanCalDebugCmd(uint32_t cmd, uint32_t para);
#else
static void set_cal_enabled(u_int8_t enabled) {}
static u_int8_t is_cal_flow_finished(void) { return TRUE; }
#endif

#endif
