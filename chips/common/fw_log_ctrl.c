/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2020 MediaTek Inc.
 */

/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */

/*******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 *******************************************************************************
 */
#include "precomp.h"

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */

/*******************************************************************************
 *                            P U B L I C   D A T A
 *******************************************************************************
 */

/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */

/*******************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */
uint32_t fwLogCtrlInitQueryBaseAddr(struct ADAPTER *prAdapter,
	uint32_t *pu4Addr)
{
	struct INIT_CMD_LOG_BUF_CTRL rCmd = {0};
	struct INIT_WIFI_EVENT_LOG_BUF_CTRL rEvent = {0};
	uint32_t u4Status = WLAN_STATUS_SUCCESS;

	rCmd.ucType = BIT(FW_LOG_CTRL_CMD_GET_BASE_ADDR);

	u4Status = wlanSendInitSetQueryCmd(prAdapter,
		INIT_CMD_ID_LOG_BUF_CTRL, &rCmd, sizeof(rCmd),
		TRUE, FALSE,
		INIT_EVENT_ID_LOG_BUF_CTRL, &rEvent, sizeof(rEvent));
	if (u4Status != WLAN_STATUS_SUCCESS)
		goto exit;

	if (rEvent.ucStatus != 0) {
		u4Status = WLAN_STATUS_FAILURE;
		goto exit;
	}

	if (rEvent.ucType != BIT(FW_LOG_CTRL_CMD_GET_BASE_ADDR)) {
		u4Status = WLAN_STATUS_FAILURE;
		goto exit;
	}

	DBGLOG(INIT, INFO, "Log base addr: 0x%x.\n",
		rEvent.u4Address);
	if (pu4Addr)
		*pu4Addr = rEvent.u4Address;

exit:
	return u4Status;
}

uint32_t fwLogCtrlInitUpdateRead(struct ADAPTER *prAdapter,
	enum FW_LOG_CMD_CTRL_TYPE type,
	uint32_t u4Addr)
{
	struct INIT_CMD_LOG_BUF_CTRL rCmd = {0};
	uint32_t u4Status = WLAN_STATUS_SUCCESS;

	rCmd.ucType = BIT(type);
	switch (type) {
	case FW_LOG_CTRL_CMD_UPDATE_MCU_READ:
		rCmd.u4Address_MCU = u4Addr;
		break;
	case FW_LOG_CTRL_CMD_UPDATE_WIFI_READ:
		rCmd.u4Address_WIFI = u4Addr;
		break;
	case FW_LOG_CTRL_CMD_UPDATE_BT_READ:
		rCmd.u4Address_BT = u4Addr;
		break;
	case FW_LOG_CTRL_CMD_UPDATE_GPS_READ:
		rCmd.u4Address_GPS = u4Addr;
		break;
	default:
		DBGLOG(INIT, ERROR, "Invalid log ctrl type: %d\n", type);
		u4Status = WLAN_STATUS_FAILURE;
		goto exit;
	}

	u4Status = wlanSendInitSetQueryCmd(prAdapter,
		INIT_CMD_ID_LOG_BUF_CTRL, &rCmd, sizeof(rCmd),
		FALSE, FALSE,
		0, NULL, 0);

exit:
	return u4Status;
}

