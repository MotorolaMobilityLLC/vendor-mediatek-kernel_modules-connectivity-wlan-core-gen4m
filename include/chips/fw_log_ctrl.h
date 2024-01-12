/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2020 MediaTek Inc.
 */

#ifndef _FW_LOG_CTRL_H
#define _FW_LOG_CTRL_H

uint32_t fwLogCtrlInitQueryBaseAddr(struct ADAPTER *prAdapter,
	uint32_t *pu4Addr);

uint32_t fwLogCtrlInitUpdateRead(struct ADAPTER *prAdapter,
	enum FW_LOG_CMD_CTRL_TYPE type,
	uint32_t u4Addr);

#endif /* _FW_LOG_CTRL_H */
