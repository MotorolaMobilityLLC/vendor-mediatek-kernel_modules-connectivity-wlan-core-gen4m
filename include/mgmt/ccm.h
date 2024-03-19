/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2021 MediaTek Inc.
 */

#ifndef _CCM_H
#define _CCM_H

#if CFG_SUPPORT_CCM
enum ENUM_P2P_CCM_MODE {
	P2P_CCM_MODE_DISABLE,
	P2P_CCM_MODE_SCC
};
#endif /* CFG_SUPPORT_CCM */

/******************************************************************************
 *                                 M A C R O S
 ******************************************************************************
 */

/******************************************************************************
 *                  F U N C T I O N   D E C L A R A T I O N S
 ******************************************************************************
 */

#if CFG_SUPPORT_CCM
void ccmInit(struct ADAPTER *prAdapter);

void ccmChannelSwitchProducer(struct ADAPTER *prAdapter,
			      struct BSS_INFO *prTargetBss,
			      const char *pucSrcFunc);

void ccmChannelSwitchConsumer(struct ADAPTER *prAdapter);

#else
static inline void ccmInit(struct ADAPTER *prAdapter) {}

static inline void ccmChannelSwitchProducer(struct ADAPTER *prAdapter,
			      struct BSS_INFO *prTargetBss,
			      const char *pucSrcFunc)
{
#if CFG_ENABLE_WIFI_DIRECT
	p2pFuncSwitchSapChannel(prAdapter, P2P_DEFAULT_SCENARIO);
#endif
}

static inline void ccmChannelSwitchConsumer(struct ADAPTER *prAdapter) {}

#endif /* CFG_SUPPORT_CCM */

#endif /* _CCM_H */
