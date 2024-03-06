/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2021 MediaTek Inc.
 */

#ifndef _CCM_H
#define _CCM_H

#if CFG_SUPPORT_CCM && CFG_ENABLE_WIFI_DIRECT
enum ENUM_P2P_CCM_MODE {
	P2P_CCM_MODE_DISABLE,
	P2P_CCM_MODE_SCC
};
#endif /* CFG_SUPPORT_CCM && CFG_ENABLE_WIFI_DIRECT */

/******************************************************************************
 *                                 M A C R O S
 ******************************************************************************
 */

#if CFG_SUPPORT_CCM && CFG_ENABLE_WIFI_DIRECT
#define CCM_SWITCH_CH(_a, _b) ccmChannelSwitchProducer(_a, _b, __func__)
#else
#define CCM_SWITCH_CH(_a, _b) p2pFuncSwitchSapChannel(_a, \
	P2P_DEFAULT_SCENARIO)
#endif /* CFG_SUPPORT_CCM && CFG_ENABLE_WIFI_DIRECT */

/******************************************************************************
 *                  F U N C T I O N   D E C L A R A T I O N S
 ******************************************************************************
 */

#if CFG_SUPPORT_CCM && CFG_ENABLE_WIFI_DIRECT
void ccmInit(struct ADAPTER *prAdapter);

void ccmChannelSwitchConsumer(struct ADAPTER *prAdapter);

void ccmChannelSwitchProducer(struct ADAPTER *prAdapter,
			      struct BSS_INFO *prTargetBss,
			      const char *pucSrcFunc);
#endif /* CFG_SUPPORT_CCM && CFG_ENABLE_WIFI_DIRECT */

#endif /* _CCM_H */
