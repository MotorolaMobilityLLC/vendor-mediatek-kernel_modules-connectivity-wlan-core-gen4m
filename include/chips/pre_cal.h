/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2021 MediaTek Inc.
 */

#ifndef _PRE_CAL_H
#define _PRE_CAL_H

#if (CFG_SUPPORT_PRE_ON_PHY_ACTION == 1)
#if CFG_MTK_ANDROID_WMT
extern void update_pre_cal_status(uint8_t fgIsPreCal);
int wlanGetCalResultCb(uint32_t *pEmiCalOffset, uint32_t *pEmiCalSize);
int wlanPreCalPwrOn(void);
int wlanPreCal(void);
int wlanPreCalErr(void);
void set_cal_enabled(u_int8_t enabled);
u_int8_t is_cal_flow_finished(void);
#endif
uint32_t wlanPhyAction(struct ADAPTER *prAdapter);
void wlanCalDebugCmd(uint32_t cmd, uint32_t para);
#else
static void set_cal_enabled(u_int8_t enabled) {}
static u_int8_t is_cal_flow_finished(void) { return TRUE; }
#endif

#endif
