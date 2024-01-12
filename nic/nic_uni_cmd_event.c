/******************************************************************************
 *
 * This file is provided under a dual license.  When you use or
 * distribute this software, you may choose to be licensed under
 * version 2 of the GNU General Public License ("GPLv2 License")
 * or BSD License.
 *
 * GPLv2 License
 *
 * Copyright(C) 2016 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 *
 * BSD LICENSE
 *
 * Copyright(C) 2016 MediaTek Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/
/*
 *
 */

/*! \file   nic_uni_cmd_event.c
 *    \brief  Callback functions for Command packets.
 *
 *	Various Event packet handlers which will be setup in the callback
 *  function of a command packet.
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
#include "gl_ate_agent.h"

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */

/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */
typedef uint32_t (*PROCESS_LEGACY_TO_UNI_FUNCTION) (struct ADAPTER *,
	struct WIFI_UNI_SETQUERY_INFO *);

typedef void(*PROCESS_RX_UNI_EVENT_FUNCTION) (struct ADAPTER *,
	struct WIFI_UNI_EVENT *);

/*******************************************************************************
 *                            P U B L I C   D A T A
 *******************************************************************************
 */

static PROCESS_LEGACY_TO_UNI_FUNCTION arUniCmdTable[CMD_ID_END] = {
	[0 ... CMD_ID_END - 1] = NULL,
	[CMD_ID_SCAN_REQ_V2] = nicUniCmdScanReqV2,
	[CMD_ID_SCAN_CANCEL] = nicUniCmdScanCancel,
	[CMD_ID_SET_SCAN_SCHED_ENABLE] = nicUniCmdSchedScanEnable,
	[CMD_ID_SET_SCAN_SCHED_REQ] = nicUniCmdSchedScanReq,
	[CMD_ID_BSS_ACTIVATE_CTRL] = nicUniCmdBssActivateCtrl,
	[CMD_ID_GET_SET_CUSTOMER_CFG] = nicUniCmdCustomerCfg,
	[CMD_ID_CHIP_CONFIG] = nicUniCmdChipCfg,
	[CMD_ID_SW_DBG_CTRL] = nicUniCmdSwDbgCtrl,
	[CMD_ID_REMOVE_STA_RECORD] = nicUniCmdRemoveStaRec,
	[CMD_ID_INDICATE_PM_BSS_CREATED] = nicUniCmdNotSupport,
	[CMD_ID_INDICATE_PM_BSS_ABORT] = nicUniCmdPmDisable,
	[CMD_ID_INDICATE_PM_BSS_CONNECTED] = nicUniCmdPmEnable,
	[CMD_ID_UPDATE_BEACON_CONTENT] = nicUniCmdBcnContent,
	[CMD_ID_SET_BSS_INFO] = nicUniCmdSetBssInfo,
	[CMD_ID_SET_WMM_PS_TEST_PARMS] = nicUniCmdSetWmmPsTestParams,
	[CMD_ID_SET_UAPSD_PARAM] = nicUniCmdSetUapsd,
	[CMD_ID_UPDATE_STA_RECORD] = nicUniCmdUpdateStaRec,
	[CMD_ID_CH_PRIVILEGE] = nicUniCmdChPrivilege,
	[CMD_ID_GET_CNM] = nicUniCmdCnmGetInfo,
	[CMD_ID_SET_RX_FILTER] = nicUniCmdSetRxFilter,
	[CMD_ID_SET_DBDC_PARMS] = nicUniCmdSetMbmc,
	[CMD_ID_NIC_POWER_CTRL] = nicUniCmdPowerCtrl,
	[CMD_ID_SET_DOMAIN_INFO] = nicUniCmdSetDomain,
	[CMD_ID_LOG_UI_INFO] = nicUniCmdWsysFwLogUI,
	[CMD_ID_FW_LOG_2_HOST] = nicUniCmdWsysFwLog2Host,
	[CMD_ID_BASIC_CONFIG] = nicUniCmdWsysFwBasicConfig,
	[CMD_ID_SET_SUSPEND_MODE] = nicUniCmdSetSuspendMode,
	[CMD_ID_SET_WOWLAN] = nicUniCmdSetWOWLAN,
	[CMD_ID_POWER_SAVE_MODE] = nicUniCmdPowerSaveMode,
	[CMD_ID_LAYER_0_EXT_MAGIC_NUM] = nicUniCmdExtCommon,
	[CMD_ID_UPDATE_WMM_PARMS] = nicUniCmdUpdateEdcaSet,
	[CMD_ID_ACCESS_REG] = nicUniCmdAccessReg,
	[CMD_ID_SET_BSS_RLM_PARAM] = nicUniCmdSetBssRlm,
	[CMD_ID_MQM_UPDATE_MU_EDCA_PARMS] = nicUniCmdUpdateMuEdca,
	[CMD_ID_RLM_UPDATE_SR_PARAMS] = nicUniCmdUpdateSrParams,
	[CMD_ID_SET_IP_ADDRESS] = nicUniCmdOffloadIPV4,
	[CMD_ID_SET_IPV6_ADDRESS] = nicUniCmdOffloadIPV6,
	[CMD_ID_GET_LTE_CHN] = nicUniCmdGetIdcChnl,
	[CMD_ID_ROAMING_TRANSIT] = nicUniCmdRoaming,
	[CMD_ID_GET_STA_STATISTICS] = nicUniCmdNotSupport, // TODO: wait for FW ready
	[CMD_ID_GET_STATISTICS] = nicUniCmdGetStatistics,
	[CMD_ID_GET_LINK_QUALITY] = nicUniCmdGetLinkQuality,
	[CMD_ID_GET_BUG_REPORT] = nicUniCmdGetBugReport,
	[CMD_ID_PERF_IND] = nicUniCmdNotSupport, // TODO: wait for FW ready
	[CMD_ID_SG_PARAM] = nicUniCmdSetSGParam,
	[CMD_ID_SET_MONITOR] = nicUniCmdSetMonitor,
	[CMD_ID_ADD_REMOVE_KEY] = nicUniCmdInstallKey,
	[CMD_ID_DEFAULT_KEY_ID] = nicUniCmdInstallDefaultKey,
	[CMD_ID_SET_GTK_REKEY_DATA] = nicUniCmdOffloadKey,
	[CMD_ID_HIF_CTRL] = nicUniCmdHifCtrl,
	[CMD_ID_RDD_ON_OFF_CTRL] = nicUniCmdRddOnOffCtrl,
	[CMD_ID_SET_TDLS_CH_SW] = nicUniCmdTdls,
	[CMD_ID_SET_NOA_PARAM] = nicUniCmdSetP2pNoa,
	[CMD_ID_SET_OPPPS_PARAM] = nicUniCmdSetP2pOppps,
	[CMD_ID_SET_AP_CONSTRAINT_PWR_LIMIT] = nicUniCmdSetApConstraintPwrLimit,
	[CMD_ID_SET_RRM_CAPABILITY] = nicUniCmdSetRrmCapability,
	[CMD_ID_SET_COUNTRY_POWER_LIMIT] = nicUniCmdSetCountryPwrLimit,
	[CMD_ID_SET_MDVT] = nicUniCmdSetMdvt,
	[CMD_ID_SET_COUNTRY_POWER_LIMIT_PER_RATE] =
			nicUniCmdSetCountryPwrLimitPerRate,
	[CMD_ID_SET_NVRAM_SETTINGS] = nicUniCmdNotSupport,
	[CMD_ID_TEST_CTRL] = nicUniCmdTestmodeCtrl,
	[CMD_ID_ACCESS_RX_STAT] = nicUniCmdTestmodeRxStat,
	[CMD_ID_STATS_LOG] = nicUniCmdNotSupport,
	[CMD_ID_SET_LOW_LATENCY_MODE] = nicUniCmdNotSupport,
	[CMD_ID_SET_FORCE_RTS] = nicUniCmdNotSupport,
	[CMD_ID_TX_AMPDU] = nicUniCmdSetTxAmpdu,
	[CMD_ID_ADDBA_REJECT] = nicUniCmdSetRxAmpdu,
	[CMD_ID_MAC_MCAST_ADDR] = nicUniCmdNotSupport, // TODO: wait for FW ready
	[CMD_ID_RSSI_MONITOR] = nicUniCmdSetRssiMonitor,
	[CMD_ID_SET_ICS_SNIFFER] = nicUniCmdSetIcsSniffer,
};

static PROCESS_LEGACY_TO_UNI_FUNCTION arUniExtCmdTable[EXT_CMD_ID_END] = {
	[0 ... EXT_CMD_ID_END - 1] = NULL,
	[EXT_CMD_ID_TWT_AGRT_UPDATE] = nicUniCmdTwtArgtUpdate,
	[EXT_CMD_ID_STAREC_UPDATE] = nicUniCmdStaRecUpdateExt,
	[EXT_CMD_ID_BF_ACTION] = nicUniCmdBFAction,
	[EXT_CMD_ID_SER] = nicUniCmdSerAction,
	[EXT_CMD_ID_GET_MAC_INFO] = nicUniCmdGetTsf,
	[EXT_CMD_ID_DEVINFO_UPDATE] = nicUniUpdateDevInfo,
	[EXT_CMD_ID_TX_POWER_FEATURE_CTRL] = nicUniCmdTxPowerCtrl,
	[EXT_CMD_ID_THERMAL_PROTECT] = nicUniCmdThermalProtect,
	[EXT_CMD_ID_RF_TEST] = nicUniExtCmdTestmodeCtrl,
};

static PROCESS_RX_UNI_EVENT_FUNCTION arUniEventTable[UNI_EVENT_ID_NUM] = {
	[0 ... UNI_EVENT_ID_NUM - 1] = NULL,
	[UNI_EVENT_ID_SCAN_DONE] = nicUniEventScanDone,
	[UNI_EVENT_ID_CNM] = nicUniEventChMngrHandleChEvent,
	[UNI_EVENT_ID_MBMC] = nicUniEventMbmcHandleEvent,
	[UNI_EVENT_ID_STATUS_TO_HOST] = nicUniEventStatusToHost,
	[UNI_EVENT_ID_BA_OFFLOAD] = nicUniEventBaOffload,
	[UNI_EVENT_ID_SLEEP_NOTIFY] = nicUniEventSleepNotify,
	[UNI_EVENT_ID_BEACON_TIMEOUT] = nicUniEventBeaconTimeout,
	[UNI_EVENT_ID_UPDATE_COEX_PHYRATE] = nicUniEventUpdateCoex,
	[UNI_EVENT_ID_IDC] = nicUniEventIdc,
	[UNI_EVENT_ID_BSS_IS_ABSENCE] = nicUniEventBssIsAbsence,
	[UNI_EVENT_ID_PS_SYNC] = nicUniEventPsSync,
	[UNI_EVENT_ID_SAP] = nicUniEventSap,
	[UNI_EVENT_ID_OBSS_UPDATE] = nicUniEventOBSS,
	[UNI_EVENT_ID_ROAMING] = nicUniEventRoaming,
	[UNI_EVENT_ID_ADD_KEY_DONE] = nicUniEventAddKeyDone,
	[UNI_EVENT_ID_FW_LOG_2_HOST] = nicUniEventFwLog2Host,
	[UNI_EVENT_ID_P2P] = nicUniEventP2p,
	[UNI_EVENT_ID_IE_COUNTDOWN] = nicUniEventCountdown,
	[UNI_EVENT_ID_STAREC] = nicUniEventStaRec,
	[UNI_EVENT_ID_TDLS] = nicUniEventTdls,
	[UNI_EVENT_ID_BSS_ER] = nicUniEventBssER,
	[UNI_EVENT_ID_RSSI_MONITOR] = nicUniEventRssiMonitor,
	[UNI_EVENT_ID_HIF_CTRL] = nicUniEventHifCtrl,
};

extern struct RX_EVENT_HANDLER arEventTable[];
extern uint32_t arEventTableSize;

/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */

/*******************************************************************************
 *                                 M A C R O S
 *******************************************************************************
 */

/* This is for the usage of assign a 2 byte Wcid to WcidL and WcidH */
#define WCID_SET_H_L(_HnVer, _L, _u2Value) \
	do { \
		_HnVer = (uint8_t)(((_u2Value) >> 8) & 0x3); \
		_L = (uint8_t)((_u2Value) & 0xff); \
	} while (0)

#define	RUN_RX_EVENT_HANDLER(_EID, _pdata) \
	nicUniEventHelper(ad, evt, _EID, 0, \
		(uint8_t *)(_pdata), sizeof(*(_pdata)))

#define	RUN_RX_EVENT_HANDLER_EXT(_EID, _pdata, _len) \
	nicUniEventHelper(ad, evt, _EID, 0, (uint8_t *)(_pdata), _len)

#define	RUN_RX_EXT_EVENT_HANDLER(_EXT_EID, _pdata) \
	nicUniEventHelper(ad, evt, EVENT_ID_LAYER_0_EXT_MAGIC_NUM, _EXT_EID, \
		(uint8_t *)(_pdata), sizeof(*(_pdata)))

/*******************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */

struct WIFI_UNI_CMD_ENTRY *nicUniCmdAllocEntry(struct ADAPTER *ad,
	uint8_t ucid, uint32_t max_cmd_len,
	PFN_CMD_DONE_HANDLER pfCmdDoneHandler,
	PFN_CMD_TIMEOUT_HANDLER pfCmdTimeoutHandler)
{
	struct WIFI_UNI_CMD_ENTRY *entry;

	/* alloc entry + cmd_buf together */
	entry = (struct WIFI_UNI_CMD_ENTRY *) cnmMemAlloc(ad, RAM_TYPE_BUF,
			sizeof(*entry) + max_cmd_len);

	if (!entry)
		return NULL;

	kalMemZero(entry, sizeof(*entry) + max_cmd_len);
	entry->pucInfoBuffer = ((uint8_t *) entry) + sizeof(*entry);
	entry->u4SetQueryInfoLen = max_cmd_len;
	entry->pfCmdDoneHandler = pfCmdDoneHandler;
	entry->pfCmdTimeoutHandler = pfCmdTimeoutHandler;
	entry->ucUCID = ucid;

	return entry;
}

void nicUniCmdFreeEntry(struct ADAPTER *ad, struct WIFI_UNI_CMD_ENTRY *entry)
{
	if (entry)
		cnmMemFree(ad, entry);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief command packet generation utility
 *
 * \param[in] prAdapter          Pointer to the Adapter structure.
 * \param[in] ucCID              Command ID
 * \param[in] fgSetQuery         Set or Query
 * \param[in] fgNeedResp         Need for response
 * \param[in] pfCmdDoneHandler   Function pointer when command is done
 * \param[in] u4SetQueryInfoLen  The length of the set/query buffer
 * \param[in] pucInfoBuffer      Pointer to set/query buffer
 *
 *
 * \retval WLAN_STATUS_PENDING
 * \retval WLAN_STATUS_FAILURE
 */
/*----------------------------------------------------------------------------*/
uint32_t
wlanSendSetQueryCmdHelper(IN struct ADAPTER *prAdapter,
		    uint8_t ucCID,
		    uint8_t ucExtCID,
		    u_int8_t fgSetQuery,
		    u_int8_t fgNeedResp,
		    u_int8_t fgIsOid,
		    PFN_CMD_DONE_HANDLER pfCmdDoneHandler,
		    PFN_CMD_TIMEOUT_HANDLER pfCmdTimeoutHandler,
		    uint32_t u4SetQueryInfoLen,
		    uint8_t *pucInfoBuffer, OUT void *pvSetQueryBuffer,
		    IN uint32_t u4SetQueryBufferLen,
		    enum EUNM_CMD_SEND_METHOD eMethod)
{
	struct WIFI_UNI_SETQUERY_INFO info;
	struct LINK *link = &info.rUniCmdList;
	struct WIFI_UNI_CMD_ENTRY *entry, *next;
	uint32_t status = 0;

	if (kalIsResetting()) {
		DBGLOG(INIT, WARN, "Chip resetting, skip\n");
		return WLAN_STATUS_NOT_ACCEPTED;
	}

	if (!arUniCmdTable[ucCID])
		return wlanSendSetQueryCmdAdv(prAdapter,
			ucCID, ucExtCID, fgSetQuery, fgNeedResp, fgIsOid,
			pfCmdDoneHandler, pfCmdTimeoutHandler,
			u4SetQueryInfoLen, pucInfoBuffer,
			pvSetQueryBuffer, u4SetQueryBufferLen,
			eMethod);

	/* prepare info */
	info.ucCID = ucCID;
	info.ucExtCID = ucExtCID;
	info.fgSetQuery = fgSetQuery;
	info.fgNeedResp = fgNeedResp;
	info.fgIsOid = fgIsOid;
	info.pfCmdDoneHandler = pfCmdDoneHandler;
	info.pfCmdTimeoutHandler = pfCmdTimeoutHandler;
	info.u4SetQueryInfoLen = u4SetQueryInfoLen;
	info.pucInfoBuffer = pucInfoBuffer;
	info.pvSetQueryBuffer = pvSetQueryBuffer;
	info.u4SetQueryBufferLen = u4SetQueryBufferLen;
	info.eMethod = eMethod;
	LINK_INITIALIZE(&info.rUniCmdList);

	/* collect unified cmd info */
	status = arUniCmdTable[ucCID](prAdapter, &info);
	if (status != WLAN_STATUS_SUCCESS)
		goto done;

	LINK_FOR_EACH_ENTRY_SAFE(entry, next,
		link, rLinkEntry, struct WIFI_UNI_CMD_ENTRY) {

		DBGLOG(REQ, TRACE,
			"UCMD[0x%x] SET[%d] RSP[%d] OID[%d] LEN[%d]\n",
			entry->ucUCID, info.fgSetQuery, info.fgNeedResp,
			info.fgIsOid, entry->u4SetQueryInfoLen);
		status = wlanSendSetQueryUniCmdAdv(prAdapter,
			entry->ucUCID, info.fgSetQuery, info.fgNeedResp,
			info.fgIsOid, entry->pfCmdDoneHandler,
			entry->pfCmdTimeoutHandler, entry->u4SetQueryInfoLen,
			entry->pucInfoBuffer, info.pvSetQueryBuffer,
			info.u4SetQueryBufferLen, info.eMethod);
	}
done:
	/* clear before return, in case uni handler already insert any entry */
	LINK_FOR_EACH_ENTRY_SAFE(entry, next,
		link, rLinkEntry, struct WIFI_UNI_CMD_ENTRY) {

		LINK_REMOVE_KNOWN_ENTRY(link, entry);
		/* releaes info */
		nicUniCmdFreeEntry(prAdapter, entry);
	}

	return status;
}

uint32_t wlanSendSetQueryUniCmd(IN struct ADAPTER *prAdapter,
			uint8_t ucUCID,
			u_int8_t fgSetQuery,
			u_int8_t fgNeedResp,
			u_int8_t fgIsOid,
			PFN_CMD_DONE_HANDLER pfCmdDoneHandler,
			PFN_CMD_TIMEOUT_HANDLER pfCmdTimeoutHandler,
			uint32_t u4SetQueryInfoLen,
			uint8_t *pucInfoBuffer, OUT void *pvSetQueryBuffer,
			IN uint32_t u4SetQueryBufferLen)
{
	return wlanSendSetQueryUniCmdAdv(prAdapter,
			ucUCID, fgSetQuery, fgNeedResp, fgIsOid,
			pfCmdDoneHandler, pfCmdTimeoutHandler,
			u4SetQueryInfoLen, pucInfoBuffer,
			pvSetQueryBuffer, u4SetQueryBufferLen,
			CMD_SEND_METHOD_ENQUEUE);
}

/*----------------------------------------------------------------------------*/
/*!
* \brief command packet generation utility
*
* \param[in] prAdapter	   Pointer to the Adapter structure.
* \param[in] ucCID		   Command ID
* \param[in] fgSetQuery	   Set or Query
* \param[in] fgNeedResp	   Need for response
* \param[in] pfCmdDoneHandler   Function pointer when command is done
* \param[in] u4SetQueryInfoLen  The length of the set/query buffer
* \param[in] pucInfoBuffer	   Pointer to set/query buffer
*
*
* \retval WLAN_STATUS_PENDING
* \retval WLAN_STATUS_FAILURE
*/
/*----------------------------------------------------------------------------*/
uint32_t
wlanSendSetQueryUniCmdAdv(IN struct ADAPTER *prAdapter,
	      uint8_t ucUCID,
	      u_int8_t fgSetQuery,
	      u_int8_t fgNeedResp,
	      u_int8_t fgIsOid,
	      PFN_CMD_DONE_HANDLER pfCmdDoneHandler,
	      PFN_CMD_TIMEOUT_HANDLER pfCmdTimeoutHandler,
	      uint32_t u4SetQueryInfoLen,
	      uint8_t *pucInfoBuffer, OUT void *pvSetQueryBuffer,
	      IN uint32_t u4SetQueryBufferLen,
	      enum EUNM_CMD_SEND_METHOD eMethod)
{
	struct GLUE_INFO *prGlueInfo;
	struct CMD_INFO *prCmdInfo;
	uint8_t *pucCmfBuf;
	struct mt66xx_chip_info *prChipInfo;
	uint16_t cmd_size;
	uint8_t ucOption;
	uint32_t status = WLAN_STATUS_PENDING;

	if (kalIsResetting()) {
		DBGLOG(INIT, WARN, "Chip resetting, skip\n");
		return WLAN_STATUS_NOT_ACCEPTED;
	}

	prGlueInfo = prAdapter->prGlueInfo;
	prChipInfo = prAdapter->chip_info;
	cmd_size = prChipInfo->u2UniCmdTxHdrSize + u4SetQueryInfoLen;
	prCmdInfo = cmdBufAllocateCmdInfo(prAdapter, cmd_size);

	if (!prCmdInfo) {
		DBGLOG(INIT, ERROR, "Allocate CMD_INFO_T ==> FAILED.\n");
		return WLAN_STATUS_RESOURCES;
	}

	/* Setup common CMD Info Packet */
	prCmdInfo->eCmdType = COMMAND_TYPE_NETWORK_IOCTL;
	prCmdInfo->u2InfoBufLen = cmd_size;
	prCmdInfo->pfCmdDoneHandler = pfCmdDoneHandler;
	prCmdInfo->pfCmdTimeoutHandler = pfCmdTimeoutHandler;
	prCmdInfo->fgIsOid = fgIsOid;
	prCmdInfo->ucCID = ucUCID;
	prCmdInfo->fgSetQuery = fgSetQuery;
	prCmdInfo->fgNeedResp = fgNeedResp;
	prCmdInfo->u4SetInfoLen = u4SetQueryInfoLen;
	prCmdInfo->pvInformationBuffer = pvSetQueryBuffer;
	prCmdInfo->u4InformationBufferLength = u4SetQueryBufferLen;
	ucOption = UNI_CMD_OPT_BIT_1_UNI_CMD;
	if (fgSetQuery) /* it is a SET command */
		ucOption |= (prCmdInfo->fgNeedResp ? UNI_CMD_OPT_BIT_0_ACK : 0);
	ucOption |= (fgSetQuery ? UNI_CMD_OPT_BIT_2_SET_QUERY : 0);

	// TODO: uni cmd, fragment
	/* Setup WIFI_CMD_T (no payload) */
	NIC_FILL_UNI_CMD_TX_HDR(prAdapter,
		prCmdInfo->pucInfoBuffer,
		prCmdInfo->u2InfoBufLen,
		prCmdInfo->ucCID,
		CMD_PACKET_TYPE_ID,
		&prCmdInfo->ucCmdSeqNum,
		&pucCmfBuf, S2D_INDEX_CMD_H2N,
		ucOption);
	prCmdInfo->pucSetInfoBuffer = pucCmfBuf;
	if (u4SetQueryInfoLen > 0 && pucInfoBuffer != NULL)
		kalMemCopy(pucCmfBuf, pucInfoBuffer, u4SetQueryInfoLen);

	switch (eMethod) {
	case CMD_SEND_METHOD_ENQUEUE:
		/* insert into prCmdQueue */
		kalEnqueueCommand(prGlueInfo,
				  (struct QUE_ENTRY *) prCmdInfo);

		/* wakeup txServiceThread later */
		GLUE_SET_EVENT(prGlueInfo);
		break;
	case CMD_SEND_METHOD_REQ_RESOURCE:
		status = wlanSendCommand(prAdapter, prCmdInfo);
		cmdBufFreeCmdInfo(prAdapter, prCmdInfo);
		break;
	case CMD_SEND_METHOD_TX:
		status = nicTxCmd(prAdapter, prCmdInfo, TC4_INDEX);
		cmdBufFreeCmdInfo(prAdapter, prCmdInfo);
		break;
	}

	return status;
}

uint32_t nicUniCmdExtCommon(struct ADAPTER *ad,
	struct WIFI_UNI_SETQUERY_INFO *info)
{
	if (info->ucCID != CMD_ID_LAYER_0_EXT_MAGIC_NUM)
		return WLAN_STATUS_NOT_ACCEPTED;

	if (!arUniExtCmdTable[info->ucExtCID])
		return WLAN_STATUS_NOT_SUPPORTED;

	return arUniExtCmdTable[info->ucExtCID](ad, info);
}

uint32_t nicUniCmdNotSupport(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	DBGLOG(INIT, ERROR, "CID[0x%x] not support!!\n", info->ucCID);
	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdScanTagReq(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_SCAN_REQ_V2 *cmd)
{
	struct UNI_CMD_SCAN_REQ *tag = (struct UNI_CMD_SCAN_REQ *)buf;

	tag->u2Tag = UNI_CMD_SCAN_TAG_SCAN_REQ;
	tag->u2Length = sizeof(*tag);
	tag->ucScanType = cmd->ucScanType;
	tag->ucNumProbeReq = cmd->ucNumProbeReq;
	tag->ucScnFuncMask = cmd->ucScnFuncMask;
	tag->u2ChannelMinDwellTime = cmd->u2ChannelMinDwellTime;
	tag->u2ChannelDwellTime = cmd->u2ChannelDwellTime;
	tag->u2TimeoutValue = cmd->u2TimeoutValue;
	tag->u2ProbeDelayTime = cmd->u2ProbeDelayTime;
	tag->u4ScnFuncMaskExtend = cmd->u4ScnFuncMaskExtend;
	return tag->u2Length;
}

uint32_t nicUniCmdScanTagSsid(struct ADAPTER *ad, uint8_t *buf,
	struct CMD_SCAN_REQ_V2 *cmd)
{
	struct UNI_CMD_SCAN_SSID *tag = (struct UNI_CMD_SCAN_SSID *)buf;
	uint8_t i;
	uint8_t *pos = tag->aucSsidBuffer;
	uint8_t ssid_num = min((int)cmd->ucSSIDNum, 4);
	uint8_t ssid_ext_num = min((int)cmd->ucSSIDExtNum, 6);
	uint16_t len = sizeof(*tag) +
		(ssid_num + ssid_ext_num) * sizeof(struct PARAM_SSID);

	if ((ssid_num + ssid_ext_num) == 0)
		return 0;

	tag->u2Tag = UNI_CMD_SCAN_TAG_SCAN_SSID;
	tag->u2Length = len;
	tag->ucSSIDType = cmd->ucSSIDType;
	tag->ucSSIDNum = ssid_num + ssid_ext_num;
	for (i = 0; i < ssid_num; i++) {
		kalMemCopy(pos, &cmd->arSSID[i], sizeof(struct PARAM_SSID));
		pos += sizeof(struct PARAM_SSID);
	}
	for (i = 0; i < ssid_ext_num; i++) {
		kalMemCopy(pos, &cmd->arSSIDExtend[i],
					sizeof(struct PARAM_SSID));
		pos += sizeof(struct PARAM_SSID);
	}
	return tag->u2Length;
}

uint32_t nicUniCmdScanTagBssid(struct ADAPTER *ad, uint8_t *buf,
	struct CMD_SCAN_REQ_V2 *cmd)
{
	struct UNI_CMD_SCAN_BSSID *tag = (struct UNI_CMD_SCAN_BSSID *)buf;
	uint8_t i;

	if (cmd->ucScnFuncMask & ENUM_SCN_USE_PADDING_AS_BSSID) {
		for (i = 0; i < CFG_SCAN_OOB_MAX_NUM; i++) {
			tag->u2Tag = UNI_CMD_SCAN_TAG_SCAN_BSSID;
			tag->u2Length = sizeof(*tag);
			kalMemCopy(tag->aucBssid,
				cmd->aucExtBSSID[i], MAC_ADDR_LEN);

			tag->ucBssidMatchCh = cmd->ucBssidMatchCh[i];
			tag->ucBssidMatchSsidInd = cmd->ucBssidMatchSsidInd[i];
			tag++;
		}
		return ((uint8_t *)tag) - buf;
	} else {
		tag->u2Tag = UNI_CMD_SCAN_TAG_SCAN_BSSID;
		tag->u2Length = sizeof(*tag);
		kalMemCopy(tag->aucBssid, cmd->aucBSSID, MAC_ADDR_LEN);

		tag->ucBssidMatchCh = 0;
		tag->ucBssidMatchSsidInd = CFG_SCAN_OOB_MAX_NUM;

		return tag->u2Length;
	}
}

uint32_t nicUniCmdScanTagChnlInfo(struct ADAPTER *ad, uint8_t *buf,
	struct CMD_SCAN_REQ_V2 *cmd)
{
	struct UNI_CMD_SCAN_CHANNEL_INFO *tag =
		(struct UNI_CMD_SCAN_CHANNEL_INFO *)buf;
	uint8_t i;
	uint8_t *pos = tag->aucChnlInfoBuffer;
	uint8_t chnl_num = min((int)cmd->ucChannelListNum, 32);
	uint8_t chnl_ext_num = min((int)cmd->ucChannelListExtNum, 32);
	uint16_t len = sizeof(*tag) +
	       ALIGN_4((chnl_num + chnl_ext_num) * sizeof(struct CHANNEL_INFO));

	if ((chnl_num + chnl_ext_num) == 0)
		return 0;

	tag->u2Tag = UNI_CMD_SCAN_TAG_SCAN_CHANNEL;
	tag->u2Length = len;
	tag->ucChannelType = cmd->ucChannelType;
	tag->ucChannelListNum = chnl_num + chnl_ext_num;
	for (i = 0; i < chnl_num; i++) {
		kalMemCopy(pos, &cmd->arChannelList[i],
			sizeof(struct CHANNEL_INFO));
		pos += sizeof(struct CHANNEL_INFO);
	}
	for (i = 0; i < chnl_ext_num; i++) {
		kalMemCopy(pos, &cmd->arChannelListExtend[i],
			sizeof(struct CHANNEL_INFO));
		pos += sizeof(struct CHANNEL_INFO);
	}
	return tag->u2Length;
}

uint32_t nicUniCmdScanTagIe(struct ADAPTER *ad, uint8_t *buf,
	struct CMD_SCAN_REQ_V2 *cmd)
{
	struct UNI_CMD_SCAN_IE *tag = (struct UNI_CMD_SCAN_IE *)buf;

	if (cmd->u2IELen == 0)
		return 0;

	tag->u2Tag = UNI_CMD_SCAN_TAG_SCAN_IE;
	tag->u2Length = sizeof(*tag) + ALIGN_4(cmd->u2IELen);
	tag->u2IELen = cmd->u2IELen;
	kalMemCopy(tag->aucIEBuffer, cmd->aucIE, cmd->u2IELen);

	return tag->u2Length;
}

uint32_t nicUniCmdScanTagMisc(struct ADAPTER *ad, uint8_t *buf,
	struct CMD_SCAN_REQ_V2 *cmd)
{
	struct UNI_CMD_SCAN_MISC *tag = (struct UNI_CMD_SCAN_MISC *)buf;

	tag->u2Tag = UNI_CMD_SCAN_TAG_SCAN_MISC;
	tag->u2Length = sizeof(*tag);
	kalMemCopy(tag->aucRandomMac, cmd->aucRandomMac, MAC_ADDR_LEN);

	tag->ucShortSSIDNum = cmd->ucShortSSIDNum;

	return tag->u2Length;
}

struct UNI_CMD_SCAN_TAG_HANDLE arSetScanReqTable[] = {
	{sizeof(struct UNI_CMD_SCAN_REQ), nicUniCmdScanTagReq},
	{sizeof(struct UNI_CMD_SCAN_SSID) + sizeof(struct PARAM_SSID) * 10,
	 nicUniCmdScanTagSsid},
	{sizeof(struct UNI_CMD_SCAN_BSSID) * CFG_SCAN_SSID_MAX_NUM,
	 nicUniCmdScanTagBssid},
	{sizeof(struct UNI_CMD_SCAN_CHANNEL_INFO) +
	 sizeof(struct CHANNEL_INFO) * 64,
	 nicUniCmdScanTagChnlInfo},
	{sizeof(struct UNI_CMD_SCAN_IE) + 600, nicUniCmdScanTagIe},
	{sizeof(struct UNI_CMD_SCAN_MISC), nicUniCmdScanTagMisc},
};

uint32_t nicUniCmdScanReqV2(struct ADAPTER *ad,
	struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_SCAN_REQ_V2 *cmd;
	struct UNI_CMD_SCAN *uni_cmd;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint8_t *pos;
	uint32_t max_cmd_len = 0;
	int i;

	if (info->ucCID != CMD_ID_SCAN_REQ_V2 ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_SCAN_REQ_V2 *)info->pucInfoBuffer;
	max_cmd_len += sizeof(struct UNI_CMD_SCAN);
	for (i = 0; i < ARRAY_SIZE(arSetScanReqTable); i++)
		max_cmd_len += arSetScanReqTable[i].u4Size;

	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_SCAN_REQ,
			max_cmd_len, NULL, NULL);

	if (!entry)
		return WLAN_STATUS_RESOURCES;

	pos = entry->pucInfoBuffer;
	uni_cmd = (struct UNI_CMD_SCAN *) pos;
	uni_cmd->ucSeqNum = cmd->ucSeqNum;
	uni_cmd->ucBssIndex = cmd->ucBssIndex;
	pos += sizeof(*uni_cmd);
	for (i = 0; i < ARRAY_SIZE(arSetScanReqTable); i++)
		pos += arSetScanReqTable[i].pfHandler(ad, pos, cmd);
	entry->u4SetQueryInfoLen = pos - entry->pucInfoBuffer;

	ASSERT(entry->u4SetQueryInfoLen <= max_cmd_len);

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdBssInfoConnType(struct ADAPTER *ad, struct BSS_INFO *bssinfo)
{
	if (bssinfo->eNetworkType == NETWORK_TYPE_AIS) {
		return CONNECTION_INFRA_STA;
	} else if (bssinfo->eNetworkType == NETWORK_TYPE_P2P) {
		if (bssinfo->eCurrentOPMode == OP_MODE_INFRASTRUCTURE) {
			return CONNECTION_P2P_GC;
		} else if (bssinfo->eCurrentOPMode == OP_MODE_ACCESS_POINT) {
#if CFG_ENABLE_WIFI_DIRECT
			if (ad->fgIsP2PRegistered &&
			    !p2pFuncIsAPMode(ad->rWifiVar.prP2PConnSettings[
					bssinfo->u4PrivateData])) {
				return CONNECTION_P2P_GO;
			} else
#endif
				return CONNECTION_INFRA_AP;
		} else if (bssinfo->eCurrentOPMode == OP_MODE_P2P_DEVICE) {
			return CONNECTION_P2P_DEVICE;
		}
	}

	return 0;
}

uint32_t nicUniCmdBssInfoPhyMode(uint8_t ucPhyTypeSet)
{
	uint32_t phy_mode = 0;

	if (ucPhyTypeSet & PHY_TYPE_BIT_HR_DSSS)
		phy_mode |= WMODE_B;

	if (ucPhyTypeSet & PHY_TYPE_BIT_ERP) {
		phy_mode |= WMODE_G;
	}

	if (ucPhyTypeSet & PHY_TYPE_BIT_OFDM) {
		phy_mode |= WMODE_A;
	}

	if (ucPhyTypeSet & PHY_TYPE_BIT_HT) {
		phy_mode |= WMODE_GN;
		phy_mode |= WMODE_AN;
	}

	if (ucPhyTypeSet & PHY_TYPE_BIT_VHT)
		phy_mode |= WMODE_AC;

#if (CFG_SUPPORT_802_11AX == 1)
	if (ucPhyTypeSet & PHY_TYPE_BIT_HE) {
		phy_mode |= WMODE_AX_24G;
		phy_mode |= WMODE_AX_5G;
		phy_mode |= WMODE_AX_6G;
	}
#endif
#if (CFG_SUPPORT_802_11BE == 1)
	if (ucPhyTypeSet & PHY_TYPE_BIT_EHT) {
		phy_mode |= WMODE_BE_24G;
		phy_mode |= WMODE_BE_5G;
		phy_mode |= WMODE_BE_6G;
	}
#endif

	return phy_mode;
}

uint32_t nicUniCmdBssActivateCtrl(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_BSS_ACTIVATE_CTRL *cmd;
	struct UNI_CMD_DEVINFO *dev_cmd;
	struct UNI_CMD_BSSINFO *bss_cmd;
	struct UNI_CMD_DEVINFO_ACTIVE *dev_active_tag;
	struct UNI_CMD_BSSINFO_BASIC *bss_basic_tag;
	struct WIFI_UNI_CMD_ENTRY *dev_entry = NULL, *bss_entry = NULL;
	uint32_t max_cmd_len;
	struct BSS_INFO *bss;
	uint32_t phy_mode;

	cmd = (struct CMD_BSS_ACTIVATE_CTRL *) info->pucInfoBuffer;
	bss = GET_BSS_INFO_BY_INDEX(ad, cmd->ucBssIndex);

	if (info->ucCID != CMD_ID_BSS_ACTIVATE_CTRL ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	/* update devinfo */
	max_cmd_len = sizeof(struct UNI_CMD_DEVINFO) +
		      sizeof(struct UNI_CMD_DEVINFO_ACTIVE);
	dev_entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_DEVINFO,
			max_cmd_len, NULL, NULL);
	if (!dev_entry)
		goto fail;
	dev_cmd = (struct UNI_CMD_DEVINFO *) dev_entry->pucInfoBuffer;
	dev_cmd->ucOwnMacIdx = cmd->ucOwnMacAddrIndex;
	dev_cmd->ucDbdcIdx = ENUM_BAND_AUTO;
	dev_active_tag = (struct UNI_CMD_DEVINFO_ACTIVE *)dev_cmd->aucTlvBuffer;
	dev_active_tag->u2Tag = UNI_CMD_DEVINFO_TAG_ACTIVE;
	dev_active_tag->u2Length = sizeof(*dev_active_tag);
	dev_active_tag->ucActive = cmd->ucActive;
	dev_active_tag->ucMldLinkIdx = cmd->ucMldLinkIdx;
	COPY_MAC_ADDR(dev_active_tag->aucOwnMacAddr, cmd->aucBssMacAddr);

	/* update bssinfo */
	max_cmd_len = sizeof(struct UNI_CMD_BSSINFO) +
		      sizeof(struct UNI_CMD_BSSINFO_BASIC);
	bss_entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_BSSINFO,
			max_cmd_len, NULL, NULL);
	if (!bss_entry)
		goto fail;

	bss_cmd = (struct UNI_CMD_BSSINFO *) bss_entry->pucInfoBuffer;
	bss_cmd->ucBssInfoIdx = cmd->ucBssIndex;
	bss_basic_tag = (struct UNI_CMD_BSSINFO_BASIC *)bss_cmd->aucTlvBuffer;
	bss_basic_tag->u2Tag = UNI_CMD_BSSINFO_TAG_BASIC;
	bss_basic_tag->u2Length = sizeof(*bss_basic_tag);
	bss_basic_tag->ucActive = cmd->ucActive;
	bss_basic_tag->ucDbdcIdx = ENUM_BAND_AUTO;
	bss_basic_tag->ucOwnMacIdx = cmd->ucOwnMacAddrIndex;
	bss_basic_tag->ucHwBSSIndex = cmd->ucOwnMacAddrIndex;
	bss_basic_tag->u4ConnectionType = nicUniCmdBssInfoConnType(ad, bss);
	bss_basic_tag->ucConnectionState = bss->eConnectionState;
	bss_basic_tag->ucWmmIdx = bss->ucWmmQueSet;
	COPY_MAC_ADDR(bss_basic_tag->aucBSSID, bss->aucBSSID);
	bss_basic_tag->u2BcMcWlanidx = cmd->ucBMCWlanIndex;
	bss_basic_tag->u2BcnInterval = bss->u2BeaconInterval;
	bss_basic_tag->ucDtimPeriod = bss->ucDTIMPeriod;
	bss_basic_tag->u2StaRecIdxOfAP = STA_REC_INDEX_NOT_FOUND;
	bss_basic_tag->u2NonHTBasicPhyType = bss->ucNonHTBasicPhyType;
	phy_mode = nicUniCmdBssInfoPhyMode(bss->ucPhyTypeSet);
	bss_basic_tag->ucPhyMode = phy_mode & 0xff;
	bss_basic_tag->ucPhyModeExt = (phy_mode >> 8) & 0xff;
	bss_basic_tag->ucMldLinkIdx = cmd->ucMldLinkIdx;

	DBGLOG(INIT, INFO,
		"%s DevInfo[OMAC=%d, DBDC=%d], BssInfo%d[DBDC=%d, OMAC=%d, WMM=%d, ConnType=%d, ConnState=%d, BcIdx=%d, PhyMode=0x%x, PhyModeEx=0x%x]\n",
		cmd->ucActive ? "Activate" : "Deactivate",
		dev_cmd->ucOwnMacIdx, dev_cmd->ucDbdcIdx,
		bss_cmd->ucBssInfoIdx, bss_basic_tag->ucDbdcIdx,
		bss_basic_tag->ucOwnMacIdx, bss_basic_tag->ucWmmIdx,
		bss_basic_tag->u4ConnectionType,
		bss_basic_tag->ucConnectionState, bss_basic_tag->u2BcMcWlanidx,
		bss_basic_tag->ucPhyMode, bss_basic_tag->ucPhyModeExt);

	if (cmd->ucActive) {
		/* activate devinfo first */
		LINK_INSERT_TAIL(&info->rUniCmdList, &dev_entry->rLinkEntry);
		LINK_INSERT_TAIL(&info->rUniCmdList, &bss_entry->rLinkEntry);
	} else {
		/* deactivate bssinfo first */
		LINK_INSERT_TAIL(&info->rUniCmdList, &bss_entry->rLinkEntry);
		LINK_INSERT_TAIL(&info->rUniCmdList, &dev_entry->rLinkEntry);
	}

	return WLAN_STATUS_SUCCESS;

fail:
	nicUniCmdFreeEntry(ad, dev_entry);
	nicUniCmdFreeEntry(ad, bss_entry);
	return WLAN_STATUS_RESOURCES;
}

uint32_t nicUniCmdCustomerCfg(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_HEADER *cmd;
	struct UNI_CMD_CHIP_CONFIG *uni_cmd;
	struct UNI_CMD_CHIP_CONFIG_CUSTOMER_CFG *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_CHIP_CONFIG) +
	     		       sizeof(struct UNI_CMD_CHIP_CONFIG_CUSTOMER_CFG);

	if (info->ucCID != CMD_ID_GET_SET_CUSTOMER_CFG ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_HEADER *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_CHIP_CONFIG, max_cmd_len,
			info->fgSetQuery ? NULL : nicUniCmdEventQueryCfgRead,
			info->fgSetQuery ? NULL : nicUniCmdTimeoutCommon);

	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_CHIP_CONFIG *) entry->pucInfoBuffer;
	tag = (struct UNI_CMD_CHIP_CONFIG_CUSTOMER_CFG *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_CHIP_CONFIG_TAG_CUSTOMER_CFG;
	tag->u2Length = sizeof(*tag);
	tag->cmdBufferLen = cmd->cmdBufferLen;
	tag->itemNum = cmd->itemNum;
	kalMemCopy(tag->aucbuffer, cmd->buffer, cmd->cmdBufferLen);

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdChipCfg(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_CHIP_CONFIG *cmd;
	struct UNI_CMD_CHIP_CONFIG *uni_cmd;
	struct UNI_CMD_CHIP_CONFIG_CHIP_CFG *tag;
	struct UNI_CMD_CHIP_CONFIG_CHIP_CFG_RESP *resp;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_CHIP_CONFIG) +
			       sizeof(struct UNI_CMD_CHIP_CONFIG_CHIP_CFG) +
			       sizeof(struct UNI_CMD_CHIP_CONFIG_CHIP_CFG_RESP);

	if (info->ucCID != CMD_ID_CHIP_CONFIG ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_CHIP_CONFIG *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_CHIP_CONFIG, max_cmd_len,
			info->fgSetQuery ? nicUniCmdEventSetCommon :
			nicUniEventQueryChipConfig,
			nicUniCmdTimeoutCommon);

	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_CHIP_CONFIG *) entry->pucInfoBuffer;
	tag = (struct UNI_CMD_CHIP_CONFIG_CHIP_CFG *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_CHIP_CONFIG_TAG_CHIP_CFG;
	tag->u2Length = sizeof(*tag) + sizeof(*resp);
	resp = (struct UNI_CMD_CHIP_CONFIG_CHIP_CFG_RESP *) tag->aucbuffer;
	resp->u2Id = cmd->u2Id;
	resp->u2MsgSize = cmd->u2MsgSize;
	resp->ucType = cmd->ucType;
	resp->ucRespType = cmd->ucRespType;
	kalMemCopy(resp->aucCmd, cmd->aucCmd, cmd->u2MsgSize);

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdSwDbgCtrl(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_SW_DBG_CTRL *cmd;
	struct UNI_CMD_CHIP_CONFIG *uni_cmd;
	struct UNI_CMD_CHIP_CONFIG_SW_DBG_CTRL *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_CHIP_CONFIG) +
			       sizeof(struct UNI_CMD_CHIP_CONFIG_SW_DBG_CTRL);

	if (info->ucCID != CMD_ID_SW_DBG_CTRL ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_SW_DBG_CTRL *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_CHIP_CONFIG,	max_cmd_len,
			info->fgSetQuery ? nicUniCmdEventSetCommon :
			nicUniEventQuerySwDbgCtrl, nicUniCmdTimeoutCommon);

	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_CHIP_CONFIG *) entry->pucInfoBuffer;
	tag = (struct UNI_CMD_CHIP_CONFIG_SW_DBG_CTRL *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_CHIP_CONFIG_TAG_SW_DBG_CTRL;
	tag->u2Length = sizeof(*tag);
	tag->u4Id = cmd->u4Id;
	tag->u4Data = cmd->u4Data;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdSetRxFilter(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_RX_PACKET_FILTER *cmd;
	struct UNI_CMD_BAND_CONFIG *uni_cmd;
	struct UNI_CMD_BAND_CONFIG_SET_RX_FILTER *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_BAND_CONFIG) +
	       		       sizeof(struct UNI_CMD_BAND_CONFIG_SET_RX_FILTER);

	if (info->ucCID != CMD_ID_SET_RX_FILTER ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_RX_PACKET_FILTER *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_BAND_CONFIG,
				max_cmd_len, nicUniCmdEventSetCommon,
				nicUniCmdTimeoutCommon);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_BAND_CONFIG *) entry->pucInfoBuffer;
	uni_cmd->ucDbdcIdx = ENUM_BAND_ALL;
	tag = (struct UNI_CMD_BAND_CONFIG_SET_RX_FILTER *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_BAND_CONFIG_TAG_SET_RX_FILTER;
	tag->u2Length = sizeof(*tag);
	tag->u4RxPacketFilter = cmd->u4RxPacketFilter;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdSetMbmc(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_DBDC_SETTING *cmd;
	struct UNI_CMD_MBMC *uni_cmd;
	struct UNI_CMD_MBMC_SETTING *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_MBMC) +
	     		       sizeof(struct UNI_CMD_MBMC_SETTING);

	if (info->ucCID != CMD_ID_SET_DBDC_PARMS ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_DBDC_SETTING *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_MBMC,
					max_cmd_len, NULL, NULL);

	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_MBMC *) entry->pucInfoBuffer;
	tag = (struct UNI_CMD_MBMC_SETTING *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_MBMC_TAG_SETTING;
	tag->u2Length = sizeof(*tag);
	tag->ucMbmcEn = cmd->ucDbdcEn;
	tag->ucRfBand = 0; /* unused */

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdSetMdvt(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_MDVT_CFG *cmd;
	struct UNI_CMD_DVT *dvt_cmd;
	struct UNI_CMD_MDVT_PARA *mdvt;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_MDVT_PARA) +
			       sizeof(struct UNI_CMD_DVT);

	if (info->ucCID != CMD_ID_SET_MDVT ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_MDVT_CFG *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_DVT,
		max_cmd_len, nicUniCmdEventSetCommon, nicUniCmdTimeoutCommon);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	dvt_cmd = (struct UNI_CMD_DVT *) entry->pucInfoBuffer;
	dvt_cmd->ucTestType = UNI_CMD_MODULE_DVT;

	mdvt = (struct UNI_CMD_MDVT_PARA *) dvt_cmd->aucTlvBuffer;
	mdvt->u2Tag = UNI_CMD_DVT_TAG_SET_PARA;
	mdvt->u2Length = sizeof(*mdvt);
	mdvt->u2ModuleId = cmd->u4ModuleId;
	mdvt->u2CaseId = cmd->u4CaseId;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);
	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdPowerCtrl(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_NIC_POWER_CTRL *cmd;
	struct UNI_CMD_POWER_CTRL *uni_cmd;
	struct UNI_CMD_POWER_OFF *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_POWER_CTRL) +
	     		       sizeof(struct UNI_CMD_POWER_OFF);

	if (info->ucCID != CMD_ID_NIC_POWER_CTRL ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_NIC_POWER_CTRL *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_POWER_CTRL,
					max_cmd_len, NULL, NULL);

	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_POWER_CTRL *) entry->pucInfoBuffer;
	tag = (struct UNI_CMD_POWER_OFF *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_POWER_CTRL_TAG_OFF;
	tag->u2Length = sizeof(*tag);
	tag->ucPowerMode = cmd->ucPowerMode;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdSetDomainV1(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_SET_DOMAIN_INFO *cmd;
	struct UNI_CMD_DOMAIN_SET_INFO *uni_cmd;
	struct UNI_CMD_DOMAIN_SET_INFO_DOMAIN_SUBBAND *tag;
	struct UNI_CMD_DOMAIN_SUBBAND_INFO *sub;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_DOMAIN_SET_INFO) +
	     	sizeof(struct UNI_CMD_DOMAIN_SET_INFO_DOMAIN_SUBBAND) +
		sizeof(struct UNI_CMD_DOMAIN_SUBBAND_INFO) * MAX_SUBBAND_NUM;
	uint8_t i;

	cmd = (struct CMD_SET_DOMAIN_INFO *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_SET_DOMAIN_INFO,
					max_cmd_len, NULL, NULL);

	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_DOMAIN_SET_INFO *) entry->pucInfoBuffer;
	uni_cmd->u4CountryCode = cmd->u2CountryCode;
	uni_cmd->uc2G4Bandwidth = cmd->uc2G4Bandwidth;
	uni_cmd->uc5GBandwidth = cmd->uc5GBandwidth;
	uni_cmd->uc6GBandwidth = cmd->uc5GBandwidth; // TODO: uni cmd, 6g

	tag = (struct UNI_CMD_DOMAIN_SET_INFO_DOMAIN_SUBBAND *)
		uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_DOMAIN_TAG_SUBBAND;
	tag->u2Length = sizeof(*tag) +
		sizeof(struct UNI_CMD_DOMAIN_SUBBAND_INFO) * MAX_SUBBAND_NUM;

	tag->u2IsSetPassiveScan = cmd->u2IsSetPassiveScan;
	tag->ucSubBandNum = MAX_SUBBAND_NUM;
	sub = (struct UNI_CMD_DOMAIN_SUBBAND_INFO *)tag->aucSubBandInfoBuffer;
	for (i = 0; i < MAX_SUBBAND_NUM; i++) {
		sub->ucRegClass = cmd->rSubBand[i].ucRegClass;
		sub->ucBand = cmd->rSubBand[i].ucBand;
		sub->ucChannelSpan = cmd->rSubBand[i].ucChannelSpan;
		sub->ucFirstChannelNum = cmd->rSubBand[i].ucFirstChannelNum;
		sub->ucNumChannels = cmd->rSubBand[i].ucNumChannels;
		sub->fgDfs = FALSE; /* unused */
		sub++;
	}

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdSetDomainV2(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_SET_DOMAIN_INFO_V2 *cmd;
	struct UNI_CMD_DOMAIN_SET_INFO *uni_cmd;
	struct UNI_CMD_DOMAIN_SET_INFO_DOMAIN_ACTIVE_CHANNEL_LIST *tag;
	struct CMD_DOMAIN_CHANNEL *chl_dst, *chl_src;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len;
	uint8_t i;
	uint8_t valid_channel_count = 0;

	cmd = (struct CMD_SET_DOMAIN_INFO_V2 *) info->pucInfoBuffer;
	valid_channel_count = cmd->arActiveChannels.u1ActiveChNum2g +
			      cmd->arActiveChannels.u1ActiveChNum5g +
			      cmd->arActiveChannels.u1ActiveChNum6g;
	max_cmd_len = sizeof(struct UNI_CMD_DOMAIN_SET_INFO) +
	     sizeof(struct UNI_CMD_DOMAIN_SET_INFO_DOMAIN_ACTIVE_CHANNEL_LIST) +
	     (valid_channel_count * sizeof(struct CMD_DOMAIN_CHANNEL));
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_SET_DOMAIN_INFO,
					max_cmd_len, NULL, NULL);

	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_DOMAIN_SET_INFO *) entry->pucInfoBuffer;
	uni_cmd->u4CountryCode = cmd->u4CountryCode;
	uni_cmd->uc2G4Bandwidth = cmd->uc2G4Bandwidth;
	uni_cmd->uc5GBandwidth = cmd->uc5GBandwidth;
	uni_cmd->uc6GBandwidth = cmd->uc6GBandwidth;

	tag = (struct UNI_CMD_DOMAIN_SET_INFO_DOMAIN_ACTIVE_CHANNEL_LIST *)
		uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_DOMAIN_TAG_ACTCHNL;
	tag->u2Length = sizeof(*tag) +
			sizeof(struct CMD_DOMAIN_CHANNEL) * valid_channel_count;
	tag->u1ActiveChNum2g = cmd->arActiveChannels.u1ActiveChNum2g;
	tag->u1ActiveChNum5g = cmd->arActiveChannels.u1ActiveChNum5g;
	tag->u1ActiveChNum6g = cmd->arActiveChannels.u1ActiveChNum6g;

	chl_dst = (struct CMD_DOMAIN_CHANNEL*) tag->aucActChnlListBuffer;
	chl_src = cmd->arActiveChannels.arChannels;
	for (i = 0; i < valid_channel_count; i++) {
		kalMemCopy(chl_dst, chl_src, sizeof(*chl_dst));
		chl_dst++;
		chl_src++;
	}

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdSetDomain(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	if (info->ucCID != CMD_ID_SET_DOMAIN_INFO)
		return WLAN_STATUS_NOT_ACCEPTED;

	if (info->u4SetQueryInfoLen == sizeof(struct CMD_SET_DOMAIN_INFO))
		return nicUniCmdSetDomainV1(ad, info);
	else
		return nicUniCmdSetDomainV2(ad, info);
}

uint32_t nicUniCmdScanCancel(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_SCAN_CANCEL *cmd;
	struct UNI_CMD_SCAN *uni_cmd;
	struct UNI_CMD_SCAN_CANCEL *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_SCAN) +
	     		       sizeof(struct UNI_CMD_SCAN_CANCEL);

	if (info->ucCID != CMD_ID_SCAN_CANCEL ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_SCAN_CANCEL *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_SCAN_REQ,
					max_cmd_len, NULL, NULL);

	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_SCAN *) entry->pucInfoBuffer;
	uni_cmd->ucSeqNum = cmd->ucSeqNum;
	uni_cmd->ucBssIndex = 0; /* unused */
	tag = (struct UNI_CMD_SCAN_CANCEL *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_SCAN_TAG_SCAN_CANCEL;
	tag->u2Length = sizeof(*tag);
	tag->ucIsExtChannel = cmd->ucIsExtChannel;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdSchedScanEnable(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_SET_SCHED_SCAN_ENABLE *cmd;
	struct UNI_CMD_SCAN *uni_cmd;
	struct UNI_CMD_SCAN_SCHED_SCAN_ENABLE *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_SCAN) +
	     		       sizeof(struct UNI_CMD_SCAN_SCHED_SCAN_ENABLE);

	if (info->ucCID != CMD_ID_SET_SCAN_SCHED_ENABLE ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_SET_SCHED_SCAN_ENABLE *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_SCAN_REQ,
		max_cmd_len, nicUniCmdEventSetCommon, nicUniCmdTimeoutCommon);

	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_SCAN *) entry->pucInfoBuffer;
	uni_cmd->ucSeqNum = 0; /* unused */
	uni_cmd->ucBssIndex = 0; /* unused */
	tag = (struct UNI_CMD_SCAN_SCHED_SCAN_ENABLE *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_SCAN_TAG_SCHED_SCAN_ENABLE;
	tag->u2Length = sizeof(*tag);
	tag->ucSchedScanAct = cmd->ucSchedScanAct;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdSchedScanTagReq(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_SCHED_SCAN_REQ *cmd)
{
	struct UNI_CMD_SCAN_SCHED_SCAN_REQ *tag =
			(struct UNI_CMD_SCAN_SCHED_SCAN_REQ *)buf;

	tag->u2Tag = UNI_CMD_SCAN_TAG_SCHED_SCAN_REQ;
	tag->u2Length = sizeof(*tag);
	tag->ucVersion = cmd->ucVersion;
	tag->fgStopAfterIndication = cmd->fgStopAfterIndication;
	tag->ucMspEntryNum = cmd->ucMspEntryNum;
	tag->ucScnFuncMask = cmd->ucScnFuncMask;
	kalMemCopy(tag->au2MspList, cmd->au2MspList, sizeof(tag->au2MspList));

	return tag->u2Length;
}

uint32_t nicUniCmdSchedScanTagSsid(struct ADAPTER *ad, uint8_t *buf,
	struct CMD_SCHED_SCAN_REQ *cmd)
{
	struct UNI_CMD_SCAN_SSID *tag = (struct UNI_CMD_SCAN_SSID *)buf;
	uint8_t i;
	uint8_t *pos = tag->aucSsidBuffer;
	uint8_t num = min((int)cmd->ucSsidNum, 10);
	uint16_t len = sizeof(*tag) + num * sizeof(struct PARAM_SSID);

	if (num == 0)
		return 0;

	tag->u2Tag = UNI_CMD_SCAN_TAG_SCAN_SSID;
	tag->u2Length = len;
	tag->ucSSIDType = SCAN_REQ_SSID_SPECIFIED;
	tag->ucSSIDNum = num;
	for (i = 0; i < num; i++) {
		kalMemCopy(pos, &cmd->auSsid[i], sizeof(struct PARAM_SSID));
		pos += sizeof(struct PARAM_SSID);
	}

	return tag->u2Length;
}

uint32_t nicUniCmdSchedScanTagChnlInfo(struct ADAPTER *ad, uint8_t *buf,
	struct CMD_SCHED_SCAN_REQ *cmd)
{
	struct UNI_CMD_SCAN_CHANNEL_INFO *tag =
		(struct UNI_CMD_SCAN_CHANNEL_INFO *)buf;
	uint8_t i;
	uint8_t *pos = tag->aucChnlInfoBuffer;
	uint8_t num = min((int)cmd->ucChnlNum, 64);
	uint16_t len = sizeof(*tag) + num * sizeof(struct CHANNEL_INFO);

	if (num == 0)
		return 0;

	tag->u2Tag = UNI_CMD_SCAN_TAG_SCAN_CHANNEL;
	tag->u2Length = len;
	tag->ucChannelType = cmd->ucChannelType;
	tag->ucChannelListNum = num;
	for (i = 0; i < num; i++) {
		kalMemCopy(pos, &cmd->aucChannel[i],
			sizeof(struct CHANNEL_INFO));
		pos += sizeof(struct CHANNEL_INFO);
	}

	return tag->u2Length;
}

uint32_t nicUniCmdSchedScanTagIe(struct ADAPTER *ad, uint8_t *buf,
	struct CMD_SCHED_SCAN_REQ *cmd)
{
	struct UNI_CMD_SCAN_IE *tag = (struct UNI_CMD_SCAN_IE *)buf;

	if (cmd->u2IELen == 0)
		return 0;

	tag->u2Tag = UNI_CMD_SCAN_TAG_SCAN_IE;
	tag->u2Length = sizeof(*tag) + cmd->u2IELen;
	tag->u2IELen = cmd->u2IELen;
	kalMemCopy(tag->aucIEBuffer, cmd->aucIE, cmd->u2IELen);

	return tag->u2Length;
}

uint32_t nicUniCmdSchedScanTagSsidMatchSets(struct ADAPTER *ad, uint8_t *buf,
	struct CMD_SCHED_SCAN_REQ *cmd)
{
	struct UNI_CMD_SCAN_SSID_MATCH_SETS *tag =
		(struct UNI_CMD_SCAN_SSID_MATCH_SETS *)buf;
	uint8_t i;
	uint8_t *pos = tag->aucMatchSsidBuffer;
	uint8_t num = min((int)cmd->ucMatchSsidNum, 16);
	uint16_t len = sizeof(*tag) + num * sizeof(struct SSID_MATCH_SETS);

	if (num == 0)
		return 0;

	tag->u2Tag = UNI_CMD_SCAN_TAG_SCAN_SSID_MATCH_SETS;
	tag->u2Length = len;
	tag->ucMatchSsidNum = num;
	for (i = 0; i < num; i++) {
		kalMemCopy(pos, &cmd->auMatchSsid[i],
			sizeof(struct SSID_MATCH_SETS));
		pos += sizeof(struct SSID_MATCH_SETS);
	}

	return tag->u2Length;
}

struct UNI_CMD_SCHED_SCAN_TAG_HANDLE arSetSchedScanReqTable[] = {
	{sizeof(struct UNI_CMD_SCAN_SCHED_SCAN_REQ), nicUniCmdSchedScanTagReq},
	{sizeof(struct UNI_CMD_SCAN_SSID) + sizeof(struct PARAM_SSID) * 10,
	 nicUniCmdSchedScanTagSsid},
	{sizeof(struct UNI_CMD_SCAN_CHANNEL_INFO) +
	 sizeof(struct CHANNEL_INFO) * 64,
	 nicUniCmdSchedScanTagChnlInfo},
	{sizeof(struct UNI_CMD_SCAN_IE) + 600, nicUniCmdSchedScanTagIe},
	{sizeof(struct UNI_CMD_SCAN_SSID_MATCH_SETS) +
	 sizeof(struct SSID_MATCH_SETS) * 16,
	 nicUniCmdSchedScanTagSsidMatchSets},
};

uint32_t nicUniCmdSchedScanReq(struct ADAPTER *ad,
	struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_SCHED_SCAN_REQ *cmd;
	struct UNI_CMD_SCAN *uni_cmd;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint8_t *pos;
	uint32_t max_cmd_len = 0;
	int i;

	if (info->ucCID != CMD_ID_SET_SCAN_SCHED_REQ)
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_SCHED_SCAN_REQ *)info->pucInfoBuffer;
	max_cmd_len += sizeof(struct UNI_CMD_SCAN);
	for (i = 0; i < ARRAY_SIZE(arSetSchedScanReqTable); i++)
		max_cmd_len += arSetSchedScanReqTable[i].u4Size;

	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_SCAN_REQ,
		max_cmd_len, nicUniCmdEventSetCommon, nicUniCmdTimeoutCommon);

	if (!entry)
		return WLAN_STATUS_RESOURCES;

	pos = entry->pucInfoBuffer;
	uni_cmd = (struct UNI_CMD_SCAN *) pos;
	uni_cmd->ucSeqNum = cmd->ucSeqNum;
	uni_cmd->ucBssIndex = cmd->ucBssIndex;
	pos += sizeof(*uni_cmd);
	for (i = 0; i < ARRAY_SIZE(arSetSchedScanReqTable); i++)
		pos += arSetSchedScanReqTable[i].pfHandler(ad, pos, cmd);
	entry->u4SetQueryInfoLen = pos - entry->pucInfoBuffer;

	ASSERT(entry->u4SetQueryInfoLen <= max_cmd_len);

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdWsysFwLogUI(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_EVENT_LOG_UI_INFO *cmd;
	struct UNI_CMD_WSYS_CONFIG *uni_cmd;
	struct UNI_CMD_WSYS_CONFIG_FW_LOG_UI_CTRL *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_WSYS_CONFIG) +
	     		      sizeof(struct UNI_CMD_WSYS_CONFIG_FW_LOG_UI_CTRL);

	if (info->ucCID != CMD_ID_LOG_UI_INFO ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_EVENT_LOG_UI_INFO *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_WSYS_CONFIG,
		  max_cmd_len, nicUniCmdEventSetCommon, nicUniCmdTimeoutCommon);

	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_WSYS_CONFIG *) entry->pucInfoBuffer;
	tag = (struct UNI_CMD_WSYS_CONFIG_FW_LOG_UI_CTRL *)
						uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_WSYS_CONFIG_TAG_FW_LOG_UI_CTRL;
	tag->u2Length = sizeof(*tag);
	tag->ucVersion = cmd->ucVersion;
	tag->ucLogLevel = cmd->ucLogLevel;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdWsysFwLog2Host(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_FW_LOG_2_HOST_CTRL *cmd;
	struct UNI_CMD_WSYS_CONFIG *uni_cmd;
	struct UNI_CMD_FW_LOG_CTRL_BASIC *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_WSYS_CONFIG) +
			       sizeof(struct UNI_CMD_FW_LOG_CTRL_BASIC);

	if (info->ucCID != CMD_ID_FW_LOG_2_HOST ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_FW_LOG_2_HOST_CTRL *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_WSYS_CONFIG,
		  max_cmd_len, nicUniCmdEventSetCommon, nicUniCmdTimeoutCommon);

	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_WSYS_CONFIG *) entry->pucInfoBuffer;
	tag = (struct UNI_CMD_FW_LOG_CTRL_BASIC *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_WSYS_CONFIG_TAG_FW_LOG_CTRL;
	tag->u2Length = sizeof(*tag);
	tag->ucFwLog2HostCtrl = cmd->ucFwLog2HostCtrl;
	tag->ucFwLog2HostInterval = 0;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdWsysFwBasicConfig(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_BASIC_CONFIG *cmd;
	struct UNI_CMD_WSYS_CONFIG *uni_cmd;
	struct UNI_CMD_WSYS_CONFIG_FW_BASIC_CONFIG *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_WSYS_CONFIG) +
	     		     sizeof(struct UNI_CMD_WSYS_CONFIG_FW_BASIC_CONFIG);

	if (info->ucCID != CMD_ID_BASIC_CONFIG ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_BASIC_CONFIG *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_WSYS_CONFIG,
		  max_cmd_len, nicUniCmdEventSetCommon, nicUniCmdTimeoutCommon);

	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_WSYS_CONFIG *) entry->pucInfoBuffer;
	tag = (struct UNI_CMD_WSYS_CONFIG_FW_BASIC_CONFIG *)
						uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_WSYS_CONFIG_TAG_FW_BASIC_CONFIG;
	tag->u2Length = sizeof(*tag);
	tag->u2RxChecksum = cmd->rCsumOffload.u2RxChecksum;
	tag->u2TxChecksum = cmd->rCsumOffload.u2TxChecksum;
	tag->ucCtrlFlagAssertPath = cmd->ucCtrlFlagAssertPath;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdSetSuspendMode(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_SUSPEND_MODE_SETTING *cmd;
	struct UNI_CMD_SUSPEND *uni_cmd;
	struct UNI_CMD_SUSPEND_MODE_SETTING *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_SUSPEND) +
	     		       sizeof(struct UNI_CMD_SUSPEND_MODE_SETTING);

	if (info->ucCID != CMD_ID_SET_SUSPEND_MODE ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_SUSPEND_MODE_SETTING *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_SUSPEND,
		  max_cmd_len, nicUniCmdEventSetCommon, nicUniCmdTimeoutCommon);

	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_SUSPEND *) entry->pucInfoBuffer;
	uni_cmd->ucBssInfoIdx = cmd->ucBssIndex;
	tag = (struct UNI_CMD_SUSPEND_MODE_SETTING *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_SUSPEND_TAG_MODE_SETTING;
	tag->u2Length = sizeof(*tag);
	tag->ucScreenStatus = cmd->ucEnableSuspendMode;
	tag->ucMdtim = cmd->ucMdtim;
	tag->ucWowSuspend = FALSE; /*unset */

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

void nicUniCmdWowEventSetCb(IN struct ADAPTER *prAdapter,
			IN struct CMD_INFO *prCmdInfo, IN uint8_t *pucEventBuf)
{
	if (prCmdInfo->ucCID == UNI_CMD_ID_SUSPEND) {
		DBGLOG(INIT, STATE, "CMD_ID_SET_WOWLAN cmd done\n");
		prAdapter->fgSetWowDone = TRUE;
	}
}

uint32_t nicUniCmdSetWOWLAN(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_WOWLAN_PARAM *cmd;
	struct UNI_CMD_SUSPEND *uni_cmd;
	struct UNI_CMD_SUSPEND_WOW_CTRL *ctrl_tag;
	struct UNI_CMD_SUSPEND_WOW_GPIO_PARAM *gpio_tag;
	struct UNI_CMD_SUSPEND_WOW_WAKEUP_PORT *port_tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_WSYS_CONFIG) +
	     		       sizeof(struct UNI_CMD_SUSPEND_WOW_CTRL) +
			       sizeof(struct UNI_CMD_SUSPEND_WOW_GPIO_PARAM) +
			       sizeof(struct UNI_CMD_SUSPEND_WOW_WAKEUP_PORT);
	uint8_t *pos;

	if (info->ucCID != CMD_ID_SET_WOWLAN ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_WOWLAN_PARAM *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_SUSPEND,
		  max_cmd_len, nicUniCmdWowEventSetCb, nicUniCmdTimeoutCommon);

	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_SUSPEND *) entry->pucInfoBuffer;
	uni_cmd->ucBssInfoIdx = 0; /* unused */
	pos = uni_cmd->aucTlvBuffer;

	ctrl_tag = (struct UNI_CMD_SUSPEND_WOW_CTRL *) pos;
	ctrl_tag->u2Tag = UNI_CMD_SUSPEND_TAG_WOW_CTRL;
	ctrl_tag->u2Length = sizeof(*ctrl_tag);
	ctrl_tag->ucCmd = cmd->ucCmd;
	ctrl_tag->ucDetectType = cmd->ucDetectType;
	ctrl_tag->ucWakeupHif = cmd->astWakeHif[0].ucWakeupHif;
	pos += sizeof(*ctrl_tag);

	gpio_tag = (struct UNI_CMD_SUSPEND_WOW_GPIO_PARAM *) pos;
	gpio_tag->u2Tag = UNI_CMD_SUSPEND_TAG_WOW_GPIO_PARAM;
	gpio_tag->u2Length = sizeof(*gpio_tag);
	gpio_tag->ucGpioPin = cmd->astWakeHif[0].ucGpioPin;
	gpio_tag->ucTriggerLvl = cmd->astWakeHif[0].ucTriggerLvl;
	gpio_tag->u4GpioInterval = cmd->astWakeHif[0].u4GpioInterval;
	pos += sizeof(*gpio_tag);

	port_tag = (struct UNI_CMD_SUSPEND_WOW_WAKEUP_PORT *) pos;
	port_tag->u2Tag = UNI_CMD_SUSPEND_TAG_WOW_WAKEUP_PORT;
	port_tag->u2Length = sizeof(*port_tag);
	port_tag->ucIPv4UdpPortCnt = cmd->stWowPort.ucIPv4UdpPortCnt;
	port_tag->ucIPv4TcpPortCnt = cmd->stWowPort.ucIPv4TcpPortCnt;
	port_tag->ucIPv4UdpPortCnt = cmd->stWowPort.ucIPv4UdpPortCnt;
	port_tag->ucIPv4TcpPortCnt = cmd->stWowPort.ucIPv4TcpPortCnt;
	kalMemCopy(port_tag->ausIPv4UdpPort, cmd->stWowPort.ausIPv4UdpPort,
		sizeof(port_tag->ausIPv4UdpPort));
	kalMemCopy(port_tag->ausIPv4TcpPort, cmd->stWowPort.ausIPv4TcpPort,
		sizeof(port_tag->ausIPv4TcpPort));
	kalMemCopy(port_tag->ausIPv6UdpPort, cmd->stWowPort.ausIPv6UdpPort,
		sizeof(port_tag->ausIPv6UdpPort));
	kalMemCopy(port_tag->ausIPv6TcpPort, cmd->stWowPort.ausIPv6TcpPort,
		sizeof(port_tag->ausIPv6TcpPort));
	pos += sizeof(*port_tag);

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

void nicUniCmdNicCapability(struct ADAPTER *prAdapter)
{
	struct UNI_CMD_CHIP_CONFIG *uni_cmd;
	struct UNI_CMD_CHIP_CONFIG_NIC_CAPABILITY *tag;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_CHIP_CONFIG) +
	     		      sizeof(struct UNI_CMD_CHIP_CONFIG_NIC_CAPABILITY);

	uni_cmd = (struct UNI_CMD_CHIP_CONFIG *)
		cnmMemAlloc(prAdapter, RAM_TYPE_BUF, max_cmd_len);
	tag = (struct UNI_CMD_CHIP_CONFIG_NIC_CAPABILITY *)
			uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_CHIP_CONFIG_TAG_NIC_CAPABILITY;
	tag->u2Length = sizeof(*tag);

	wlanSendSetQueryUniCmdAdv(
		prAdapter, UNI_CMD_ID_CHIP_CONFIG, FALSE,
		TRUE, FALSE, NULL, NULL, max_cmd_len,
		(uint8_t *) uni_cmd, NULL, 0,
		CMD_SEND_METHOD_REQ_RESOURCE);

	cnmMemFree(prAdapter, uni_cmd);
}

void nicUniCmdEventQueryNicCapabilityV2(IN struct ADAPTER *ad,
				     IN struct WIFI_UNI_EVENT *evt)
{
	int32_t tags_len;
	uint8_t *tag;
	uint16_t offset = 0;
	uint32_t fixed_len = sizeof(struct UNI_EVENT_CHIP_CAPABILITY);
	uint32_t data_len = GET_UNI_EVENT_DATA_LEN(evt);
	uint8_t *data = GET_UNI_EVENT_DATA(evt);

	/* copy tag to legacy event */
	tags_len = data_len - fixed_len;
	tag = data + fixed_len;
	TAG_FOR_EACH(tag, tags_len, offset) {
		DBGLOG(NIC, TRACE, "Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));
		nicParsingNicCapV2(ad, TAG_ID(tag), TAG_DATA(tag));
	}
}

uint32_t wlanQueryNicCapabilityV2(IN struct ADAPTER *prAdapter)
{
	uint32_t u4RxPktLength;
	uint8_t *prEventBuff;
	struct WIFI_UNI_EVENT *prEvent;
	struct mt66xx_chip_info *prChipInfo;
	uint32_t chip_id;
	uint32_t u4Time;

	ASSERT(prAdapter);
	prChipInfo = prAdapter->chip_info;
	chip_id = prChipInfo->chip_id;

	/* Get Nic resource information from FW */
	if (!prChipInfo->isNicCapV1
	    || (prAdapter->u4FwFeatureFlag0 &
		FEATURE_FLAG0_NIC_CAPABILITY_V2)) {

		DBGLOG(INIT, INFO, "Support NIC_CAPABILITY_V2 feature\n");

		/* get nic capability */
		nicUniCmdNicCapability(prAdapter);

		/*
		 * receive UNI_EVENT_ID_CHIP_CONFIG
		 */
		/* allocate event buffer */
		prEventBuff = cnmMemAlloc(prAdapter, RAM_TYPE_BUF,
					  CFG_RX_MAX_PKT_SIZE);
		if (!prEventBuff) {
			DBGLOG(INIT, WARN, "event buffer alloc failed!\n");
			return WLAN_STATUS_FAILURE;
		}

		/* get event */
		u4Time = kalGetTimeTick();
		while (TRUE) {
			if (nicRxWaitResponse(prAdapter,
					      1,
					      prEventBuff,
					      CFG_RX_MAX_PKT_SIZE,
					      &u4RxPktLength)
			    != WLAN_STATUS_SUCCESS) {
				DBGLOG(INIT, WARN,
				       "wait for event failed!\n");

				/* free event buffer */
				cnmMemFree(prAdapter, prEventBuff);

				return WLAN_STATUS_FAILURE;
			}

			if (CHECK_FOR_TIMEOUT(kalGetTimeTick(), u4Time,
				MSEC_TO_SYSTIME(3000))) {
				DBGLOG(HAL, ERROR,
					"Query nic capability timeout\n");
				return WLAN_STATUS_FAILURE;
			}

			/* header checking .. */
			if ((NIC_RX_GET_U2_SW_PKT_TYPE(prEventBuff) &
				prChipInfo->u2RxSwPktBitMap) !=
				prChipInfo->u2RxSwPktEvent) {
				DBGLOG(INIT, WARN,
				       "skip unexpected Rx pkt type[0x%04x]\n",
				       NIC_RX_GET_U2_SW_PKT_TYPE(prEventBuff));

				continue;
			}

			prEvent = (struct WIFI_UNI_EVENT *)
				(prEventBuff + prChipInfo->rxd_size);

			if (!IS_UNI_EVENT(prEvent) ||
			    prEvent->ucEID != UNI_EVENT_ID_CHIP_CAPABILITY) {
				DBGLOG(INIT, WARN,
				       "skip unexpected event ID[0x%02x] option:%d\n",
				        prEvent->ucEID, GET_UNI_EVENT_OPTION(prEvent));

				continue;
			} else {
				/* hit */
				break;
			}

		}

		nicUniCmdEventQueryNicCapabilityV2(prAdapter, prEvent);

		/*
		 * free event buffer
		 */
		cnmMemFree(prAdapter, prEventBuff);
	}

	/* Fill capability for different Chip version */
	if (chip_id == HQA_CHIP_ID_6632) {
		/* 6632 only */
		prAdapter->fgIsSupportBufferBinSize16Byte = TRUE;
		prAdapter->fgIsSupportDelayCal = FALSE;
		prAdapter->fgIsSupportGetFreeEfuseBlockCount = FALSE;
		prAdapter->fgIsSupportQAAccessEfuse = FALSE;
		prAdapter->fgIsSupportPowerOnSendBufferModeCMD = FALSE;
		prAdapter->fgIsSupportGetTxPower = FALSE;
	} else {
		prAdapter->fgIsSupportBufferBinSize16Byte = FALSE;
		prAdapter->fgIsSupportDelayCal = TRUE;
		prAdapter->fgIsSupportGetFreeEfuseBlockCount = TRUE;
		prAdapter->fgIsSupportQAAccessEfuse = TRUE;
		prAdapter->fgIsSupportPowerOnSendBufferModeCMD = TRUE;
		prAdapter->fgIsSupportGetTxPower = TRUE;
	}

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdRemoveStaRec(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_REMOVE_STA_RECORD *cmd;
	struct UNI_CMD_STAREC *uni_cmd;
	struct UNI_CMD_STAREC_REMOVE_INFO *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_STAREC) +
	     		       sizeof(struct UNI_CMD_STAREC_REMOVE_INFO);
	uint8_t widx;

	if (info->ucCID != CMD_ID_REMOVE_STA_RECORD ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_REMOVE_STA_RECORD *) info->pucInfoBuffer;
	if (cmd->ucActionType == STA_REC_CMD_ACTION_STA ||
	    cmd->ucActionType == STA_REC_CMD_ACTION_BSS_EXCLUDE_STA) {
		/* don't use cnmGetStaRecByIndex becayse starec is not in-use */
		if (cmd->ucStaIndex < CFG_STA_REC_NUM)
			widx = ad->arStaRec[cmd->ucStaIndex].ucWlanIndex;
		else
			return WLAN_STATUS_INVALID_DATA;
	} else {
		/* remove by bss, widx is not checked */
		widx = 0;
	}

	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_STAREC_INFO,
			max_cmd_len, NULL, NULL);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_STAREC *) entry->pucInfoBuffer;
	uni_cmd->ucBssInfoIdx = cmd->ucBssIndex;
	WCID_SET_H_L(uni_cmd->ucWlanIdxHnVer, uni_cmd->ucWlanIdxL, widx);

	tag = (struct UNI_CMD_STAREC_REMOVE_INFO *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_STAREC_TAG_REMOVE;
	tag->u2Length = sizeof(*tag);
	tag->ucActionType = cmd->ucActionType;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdBcnContent(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_BEACON_TEMPLATE_UPDATE *cmd;
	struct UNI_CMD_BSSINFO *uni_cmd;
	struct UNI_CMD_BSSINFO_BCN_CONTENT *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_BSSINFO) +
	     		       sizeof(struct UNI_CMD_BSSINFO_BCN_CONTENT);

	if (info->ucCID != CMD_ID_UPDATE_BEACON_CONTENT)
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_BEACON_TEMPLATE_UPDATE *) info->pucInfoBuffer;
	max_cmd_len += (2 + cmd->u2IELen); /* 2 for u2CapInfo */
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_BSSINFO,
			max_cmd_len, NULL, NULL);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_BSSINFO *) entry->pucInfoBuffer;
	uni_cmd->ucBssInfoIdx = cmd->ucBssIndex;
	tag = (struct UNI_CMD_BSSINFO_BCN_CONTENT *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_BSSINFO_TAG_BCN_CONTENT;
	tag->u2Length = sizeof(*tag) + 2 + cmd->u2IELen;
	if (cmd->ucUpdateMethod == IE_UPD_METHOD_UPDATE_PROBE_RSP)
		tag->ucAction = UPDATE_PROBE_RSP;
	else if (cmd->ucUpdateMethod == IE_UPD_METHOD_DELETE_ALL)
		tag->ucAction = BCN_ACTION_DISABLE;
	else
		tag->ucAction = BCN_ACTION_ENABLE;
	tag->u2PktLength = 2 + cmd->u2IELen;
	/* the aucPktContent field only include
	 * capablity field and followed IEs
	 */
	tag->aucPktContentType = 1;
	kalMemCopy(tag->aucPktContent, &cmd->u2Capability, 2);
	kalMemCopy(tag->aucPktContent + 2, cmd->aucIE, cmd->u2IELen);

	DBGLOG(INIT, INFO, "Bss=%d, Action=%d, PktLen=%d\n",
		cmd->ucBssIndex,
		tag->ucAction,
		tag->u2PktLength);

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdPmDisable(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_INDICATE_PM_BSS_ABORT *cmd;
	struct UNI_CMD_BSSINFO *uni_cmd;
	struct UNI_CMD_BSSINFO_STA_PM_DISABLE *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_BSSINFO) +
	     		       sizeof(struct UNI_CMD_BSSINFO_STA_PM_DISABLE);

	if (info->ucCID != CMD_ID_INDICATE_PM_BSS_ABORT ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_INDICATE_PM_BSS_ABORT *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_BSSINFO,
			max_cmd_len, NULL, NULL);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_BSSINFO *) entry->pucInfoBuffer;
	uni_cmd->ucBssInfoIdx = cmd->ucBssIndex;
	tag = (struct UNI_CMD_BSSINFO_STA_PM_DISABLE *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_BSSINFO_TAG_PM_DISABLE;
	tag->u2Length = sizeof(*tag);

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdPmEnable(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_INDICATE_PM_BSS_CONNECTED *cmd;
	struct UNI_CMD_BSSINFO *uni_cmd;
	struct UNI_CMD_BSSINFO_STA_PM_ENABLE *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_BSSINFO) +
	     		       sizeof(struct UNI_CMD_BSSINFO_STA_PM_ENABLE);

	if (info->ucCID != CMD_ID_INDICATE_PM_BSS_CONNECTED ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_INDICATE_PM_BSS_CONNECTED *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_BSSINFO,
			max_cmd_len, NULL, NULL);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_BSSINFO *) entry->pucInfoBuffer;
	uni_cmd->ucBssInfoIdx = cmd->ucBssIndex;
	tag = (struct UNI_CMD_BSSINFO_STA_PM_ENABLE *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_BSSINFO_TAG_PM_ENABLE;
	tag->u2Length = sizeof(*tag);
	tag->u2BcnInterval = cmd->u2BeaconInterval;
	tag->ucDtimPeriod = cmd->ucDtimPeriod;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdBssInfoTagBasic(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_SET_BSS_INFO *cmd)
{
	struct UNI_CMD_BSSINFO_BASIC *tag =
		(struct UNI_CMD_BSSINFO_BASIC *)buf;
	struct BSS_INFO *bss = GET_BSS_INFO_BY_INDEX(ad, cmd->ucBssIndex);
	uint32_t phy_mode;

	tag->u2Tag = UNI_CMD_BSSINFO_TAG_BASIC;
	tag->u2Length = sizeof(*tag);
	tag->ucActive = IS_BSS_ACTIVE(bss);
	tag->ucOwnMacIdx = bss->ucOwnMacIndex;
	tag->ucHwBSSIndex = bss->ucOwnMacIndex;
	tag->ucDbdcIdx = cmd->ucDBDCBand;
	tag->u4ConnectionType = nicUniCmdBssInfoConnType(ad, bss);
	tag->ucConnectionState = bss->eConnectionState;
	tag->ucWmmIdx = cmd->ucWmmSet;
	COPY_MAC_ADDR(tag->aucBSSID, cmd->aucBSSID);
	tag->u2BcMcWlanidx = cmd->ucBMCWlanIndex;
	tag->u2BcnInterval = bss->u2BeaconInterval;
	tag->ucDtimPeriod = bss->ucDTIMPeriod;
	tag->u2StaRecIdxOfAP = secGetWlanIdxByStaIdx(ad, cmd->ucStaRecIdxOfAP);
	tag->u2NonHTBasicPhyType = cmd->ucNonHTBasicPhyType;
	phy_mode = nicUniCmdBssInfoPhyMode(cmd->ucPhyTypeSet);
	tag->ucPhyMode = phy_mode & 0xff;
	tag->ucPhyModeExt = (phy_mode >> 8) & 0xff;

	return tag->u2Length;
}

uint32_t nicUniCmdSetBssRlmImpl(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_SET_BSS_RLM_PARAM *cmd)
{
	struct UNI_CMD_BSSINFO_RLM *rlm_tag;
	struct UNI_CMD_BSSINFO_PROTECT *prot_tag;
	struct UNI_CMD_BSSINFO_IFS_TIME *ifs_tag;
	uint8_t *pos = buf;

	rlm_tag = (struct UNI_CMD_BSSINFO_RLM *) pos;
	rlm_tag->u2Tag = UNI_CMD_BSSINFO_TAG_RLM;
	rlm_tag->u2Length = sizeof(*rlm_tag);
	rlm_tag->ucPrimaryChannel = cmd->ucPrimaryChannel;
	rlm_tag->ucCenterChannelSeg0 = cmd->ucVhtChannelFrequencyS1;
	rlm_tag->ucCenterChannelSeg1 = cmd->ucVhtChannelFrequencyS2;
	rlm_tag->ucTxStream = cmd->ucTxNss;
	rlm_tag->ucRxStream = cmd->ucRxNss;
	rlm_tag->ucHtOpInfo1 = cmd->ucHtOpInfo1;
	rlm_tag->ucSCO = cmd->ucRfSco;
	rlm_tag->ucRfBand = cmd->ucRfBand;
	rlm_tag->ucBandwidth = BW_20;
	switch (cmd->ucVhtChannelWidth) {
	case VHT_OP_CHANNEL_WIDTH_320:
		rlm_tag->ucBandwidth = BW_320;
		break;
	case VHT_OP_CHANNEL_WIDTH_80P80:
		rlm_tag->ucBandwidth = BW_8080;
		break;
	case VHT_OP_CHANNEL_WIDTH_160:
		rlm_tag->ucBandwidth = BW_160;
		break;
	case VHT_OP_CHANNEL_WIDTH_80:
		rlm_tag->ucBandwidth = BW_80;
		break;
	case VHT_OP_CHANNEL_WIDTH_20_40:
		if (cmd->ucRfSco != CHNL_EXT_SCN)
			rlm_tag->ucBandwidth = BW_40;
		break;
	default:
		break;
	}
	pos += sizeof(*rlm_tag);

	prot_tag = (struct UNI_CMD_BSSINFO_PROTECT *) pos;
	prot_tag->u2Tag = UNI_CMD_BSSINFO_TAG_PROTECT;
	prot_tag->u2Length = sizeof(*prot_tag);
	if (cmd->ucErpProtectMode)
		prot_tag->u4ProtectMode |= LEGACY_ERP_PROTECT;
	switch (cmd->ucHtProtectMode) {
	case HT_PROTECT_MODE_NON_MEMBER:
		prot_tag->u4ProtectMode |= HT_NON_MEMBER_PROTECT;
		break;
	case HT_PROTECT_MODE_20M:
		prot_tag->u4ProtectMode |= HT_BW20_PROTECT;
		break;
	case HT_PROTECT_MODE_NON_HT:
		prot_tag->u4ProtectMode |= HT_NON_HT_MIXMODE_PROTECT;
		break;
	default:
		break;
	}
	if (cmd->ucGfOperationMode == GF_MODE_PROTECT)
		prot_tag->u4ProtectMode |= VEND_GREEN_FIELD_PROTECT;
	pos += sizeof(*prot_tag);

	ifs_tag = (struct UNI_CMD_BSSINFO_IFS_TIME *) pos;
	ifs_tag->u2Tag = UNI_CMD_BSSINFO_TAG_IFS_TIME;
	ifs_tag->u2Length = sizeof(*ifs_tag);
	ifs_tag->fgSlotValid = TRUE;
	if (cmd->ucUseShortSlotTime)
		ifs_tag->u2SlotTime = 9;
	else
		ifs_tag->u2SlotTime = 20;
	pos += sizeof(*ifs_tag);

	return pos - buf;
}

uint32_t nicUniCmdBssInfoTagRlm(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_SET_BSS_INFO *cmd)
{
	return nicUniCmdSetBssRlmImpl(ad, buf, &cmd->rBssRlmParam);
}

uint32_t nicUniCmdBssInfoTagRate(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_SET_BSS_INFO *cmd)
{
	struct UNI_CMD_BSSINFO_RATE *tag = (struct UNI_CMD_BSSINFO_RATE *)buf;

	tag->u2Tag = UNI_CMD_BSSINFO_TAG_RATE;
	tag->u2Length = sizeof(*tag);
	tag->u2OperationalRateSet = cmd->u2OperationalRateSet;
	tag->u2BSSBasicRateSet = cmd->u2BSSBasicRateSet;

	return tag->u2Length;
}

uint32_t nicUniCmdBssInfoTagSec(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_SET_BSS_INFO *cmd)
{
	struct UNI_CMD_BSSINFO_SEC *tag = (struct UNI_CMD_BSSINFO_SEC *)buf;

	tag->u2Tag = UNI_CMD_BSSINFO_TAG_SEC;
	tag->u2Length = sizeof(*tag);
	tag->ucAuthMode = cmd->ucAuthMode;
	tag->ucEncStatus = cmd->ucEncStatus;

	return tag->u2Length;
}

uint32_t nicUniCmdBssInfoTagQbss(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_SET_BSS_INFO *cmd)
{
	struct UNI_CMD_BSSINFO_QBSS *tag = (struct UNI_CMD_BSSINFO_QBSS *)buf;

	tag->u2Tag = UNI_CMD_BSSINFO_TAG_QBSS;
	tag->u2Length = sizeof(*tag);
	tag->ucIsQBSS = cmd->ucIsQBSS;
	return tag->u2Length;
}

uint32_t nicUniCmdBssInfoTagSap(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_SET_BSS_INFO *cmd)
{
	struct UNI_CMD_BSSINFO_SAP *tag = (struct UNI_CMD_BSSINFO_SAP *)buf;

	tag->u2Tag = UNI_CMD_BSSINFO_TAG_SAP;
	tag->u2Length = sizeof(*tag);
	tag->fgIsHiddenSSID = cmd->ucHiddenSsidMode != ENUM_HIDDEN_SSID_NONE;
	COPY_SSID(tag->aucSSID, tag->ucSSIDLen, cmd->aucSSID, cmd->ucSSIDLen);

	return tag->u2Length;
}

uint32_t nicUniCmdBssInfoTagP2p(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_SET_BSS_INFO *cmd)
{
	struct UNI_CMD_BSSINFO_P2P *tag = (struct UNI_CMD_BSSINFO_P2P *)buf;

	tag->u2Tag = UNI_CMD_BSSINFO_TAG_P2P;
	tag->u2Length = sizeof(*tag);
	tag->u4PrivateData = cmd->u4PrivateData;

	return tag->u2Length;
}

#if (CFG_SUPPORT_802_11AX == 1)
uint32_t nicUniCmdBssInfoTagHe(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_SET_BSS_INFO *cmd)
{
	struct UNI_CMD_BSSINFO_HE *tag = (struct UNI_CMD_BSSINFO_HE *)buf;
	struct BSS_INFO *bss = GET_BSS_INFO_BY_INDEX(ad, cmd->ucBssIndex);

	tag->u2Tag = UNI_CMD_BSSINFO_TAG_HE;
	tag->u2Length = sizeof(*tag);
	tag->u2TxopDurationRtsThreshold =
		((bss->ucHeOpParams[0] &
			HE_OP_PARAM0_TXOP_DUR_RTS_THRESHOLD_MASK) >>
			HE_OP_PARAM0_TXOP_DUR_RTS_THRESHOLD_SHFT) |
		(((bss->ucHeOpParams[1] &
			HE_OP_PARAM1_TXOP_DUR_RTS_THRESHOLD_MASK) >>
			HE_OP_PARAM1_TXOP_DUR_RTS_THRESHOLD_SHFT) << 4);
	tag->au2MaxNssMcs[0] = CPU_TO_LE16(bss->u2HeBasicMcsSet);
	tag->au2MaxNssMcs[1] = CPU_TO_LE16(bss->u2HeBasicMcsSet);
	tag->au2MaxNssMcs[2] = CPU_TO_LE16(bss->u2HeBasicMcsSet);
	tag->ucDefaultPEDuration = ((bss->ucHeOpParams[0] &
		HE_OP_PARAM0_DEFAULT_PE_DUR_MASK) >>
		HE_OP_PARAM0_DEFAULT_PE_DUR_SHFT);
	tag->fgErSuDisable = (bss->ucHeOpParams[2] &
		(1 << HE_OP_PARAM2_ER_SU_DISABLE_SHFT)) ? TRUE : FALSE;

	return tag->u2Length;
}

uint32_t nicUniCmdBssInfoTagBssColor(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_SET_BSS_INFO *cmd)
{
	struct UNI_CMD_BSSINFO_BSS_COLOR *tag =
		(struct UNI_CMD_BSSINFO_BSS_COLOR *)buf;
	struct BSS_INFO *bss = GET_BSS_INFO_BY_INDEX(ad, cmd->ucBssIndex);

	tag->u2Tag = UNI_CMD_BSSINFO_TAG_BSS_COLOR;
	tag->u2Length = sizeof(*tag);
	tag->fgEnable =	(bss->ucBssColorInfo & HE_OP_BSSCOLOR_BSS_COLOR_DISABLE)
			? FALSE : TRUE;
	tag->ucBssColor = ((bss->ucBssColorInfo & HE_OP_BSSCOLOR_BSS_COLOR_MASK)
			>> HE_OP_BSSCOLOR_BSS_COLOR_SHFT);
	return tag->u2Length;
}

#endif

uint32_t nicUniCmdBssInfoTagMBSSID(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_SET_BSS_INFO *cmd)
{
	struct UNI_CMD_BSSINFO_11V_MBSSID *tag =
		(struct UNI_CMD_BSSINFO_11V_MBSSID *)buf;

	tag->u2Tag = UNI_CMD_BSSINFO_TAG_11V_MBSSID;
	tag->u2Length = sizeof(*tag);
	tag->ucMaxBSSIDIndicator = cmd->ucMaxBSSIDIndicator;
	tag->ucMBSSIDIndex = cmd->ucMBSSIDIndex;

	return tag->u2Length;
}

uint32_t nicUniCmdBssInfoTagWapi(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_SET_BSS_INFO *cmd)
{
	struct UNI_CMD_BSSINFO_WAPI *tag = (struct UNI_CMD_BSSINFO_WAPI *)buf;

	tag->u2Tag = UNI_CMD_BSSINFO_TAG_WAPI;
	tag->u2Length = sizeof(*tag);
	tag->fgWapiMode = cmd->ucWapiMode;

	return tag->u2Length;
}

uint32_t nicUniCmdBssInfoTagMld(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_SET_BSS_INFO *cmd)
{
	struct UNI_CMD_BSSINFO_MLD *tag = (struct UNI_CMD_BSSINFO_MLD *)buf;
	struct BSS_INFO *bss = GET_BSS_INFO_BY_INDEX(ad, cmd->ucBssIndex);
#if (CFG_SUPPORT_802_11BE_MLO == 1)
	struct MLD_BSS_INFO *prMldBssInfo = mldBssGetByBss(ad, bss);
#endif

	tag->u2Tag = UNI_CMD_BSSINFO_TAG_MLD;
	tag->u2Length = sizeof(*tag);

#if (CFG_SUPPORT_802_11BE_MLO == 1)
	if (prMldBssInfo) {
		tag->ucGroupMldId = prMldBssInfo->ucGroupMldId;
		tag->ucOwnMldId = bss->ucOwnMldId;
		COPY_MAC_ADDR(tag->aucOwnMldAddr, prMldBssInfo->aucOwnMldAddr);
		tag->ucOmRemapIdx = prMldBssInfo->ucOmRemapIdx;
	} else
#endif
	{
		tag->ucGroupMldId = MLD_GROUP_NONE;
		tag->ucOwnMldId = bss->ucOwnMldId;
		COPY_MAC_ADDR(tag->aucOwnMldAddr, bss->aucOwnMacAddr);
		tag->ucOmRemapIdx = OM_REMAP_IDX_NONE;
	}

	DBGLOG(INIT, INFO, "Bss=%d, GroupMldId=%d, OwnMldId=%d, OmRemapIdx=%d, OwnMldAddr=" MACSTR "\n",
		bss->ucBssIndex,
		tag->ucGroupMldId,
		tag->ucOwnMldId,
		tag->ucOmRemapIdx,
		MAC2STR(tag->aucOwnMldAddr));

	return tag->u2Length;
}

struct UNI_CMD_BSSINFO_TAG_HANDLE arSetBssInfoTable[] = {
	{sizeof(struct UNI_CMD_BSSINFO_BASIC), nicUniCmdBssInfoTagBasic},
	{sizeof(struct UNI_CMD_BSSINFO_RLM) +
	 sizeof(struct UNI_CMD_BSSINFO_PROTECT)	+
	 sizeof(struct UNI_CMD_BSSINFO_IFS_TIME), nicUniCmdBssInfoTagRlm},
	{sizeof(struct UNI_CMD_BSSINFO_RATE), nicUniCmdBssInfoTagRate},
	{sizeof(struct UNI_CMD_BSSINFO_SEC), nicUniCmdBssInfoTagSec},
	{sizeof(struct UNI_CMD_BSSINFO_QBSS), nicUniCmdBssInfoTagQbss},
	{sizeof(struct UNI_CMD_BSSINFO_SAP), nicUniCmdBssInfoTagSap},
	{sizeof(struct UNI_CMD_BSSINFO_P2P), nicUniCmdBssInfoTagP2p},
#if (CFG_SUPPORT_802_11AX == 1)
	{sizeof(struct UNI_CMD_BSSINFO_HE), nicUniCmdBssInfoTagHe},
	{sizeof(struct UNI_CMD_BSSINFO_BSS_COLOR), nicUniCmdBssInfoTagBssColor},
#endif
	{sizeof(struct UNI_CMD_BSSINFO_11V_MBSSID), nicUniCmdBssInfoTagMBSSID},
	{sizeof(struct UNI_CMD_BSSINFO_WAPI), nicUniCmdBssInfoTagWapi},
	{sizeof(struct UNI_CMD_BSSINFO_MLD), nicUniCmdBssInfoTagMld}
};

uint32_t nicUniCmdSetBssInfo(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_SET_BSS_INFO *cmd;
	struct UNI_CMD_BSSINFO *uni_cmd;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint8_t *pos;
	uint32_t max_cmd_len = 0;
	int i;

	if (info->ucCID != CMD_ID_SET_BSS_INFO ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_SET_BSS_INFO *) info->pucInfoBuffer;
	max_cmd_len += sizeof(struct UNI_CMD_BSSINFO);
	for (i = 0; i < ARRAY_SIZE(arSetBssInfoTable); i++)
		max_cmd_len += arSetBssInfoTable[i].u4Size;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_BSSINFO,
			max_cmd_len, NULL, NULL);

	if (!entry)
		return WLAN_STATUS_RESOURCES;

	pos = entry->pucInfoBuffer;
	uni_cmd = (struct UNI_CMD_BSSINFO *) pos;
	uni_cmd->ucBssInfoIdx = cmd->ucBssIndex;
	pos += sizeof(*uni_cmd);
	for (i = 0; i < ARRAY_SIZE(arSetBssInfoTable); i++)
		pos += arSetBssInfoTable[i].pfHandler(ad, pos, cmd);
	entry->u4SetQueryInfoLen = pos - entry->pucInfoBuffer;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdSetBssRlm(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_SET_BSS_RLM_PARAM *cmd;
	struct UNI_CMD_BSSINFO *uni_cmd;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_BSSINFO) +
	     		       sizeof(struct UNI_CMD_BSSINFO_RLM) +
			       sizeof(struct UNI_CMD_BSSINFO_PROTECT) +
			       sizeof(struct UNI_CMD_BSSINFO_IFS_TIME);

	if (info->ucCID != CMD_ID_SET_BSS_RLM_PARAM ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_SET_BSS_RLM_PARAM *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_BSSINFO, max_cmd_len,
			NULL, NULL);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_BSSINFO *) entry->pucInfoBuffer;
	uni_cmd->ucBssInfoIdx = cmd->ucBssIndex;
	nicUniCmdSetBssRlmImpl(ad, uni_cmd->aucTlvBuffer, cmd);

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdPowerSaveMode(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_PS_PROFILE *cmd;
	struct UNI_CMD_BSSINFO *uni_cmd;
	struct UNI_CMD_BSSINFO_POWER_SAVE *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_BSSINFO) +
	     		       sizeof(struct UNI_CMD_BSSINFO_POWER_SAVE);

	if (info->ucCID != CMD_ID_POWER_SAVE_MODE ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_PS_PROFILE *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_BSSINFO, max_cmd_len,
			info->fgIsOid ? nicUniCmdEventSetCommon : NULL,
			info->fgIsOid ? nicUniCmdTimeoutCommon : NULL);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_BSSINFO *) entry->pucInfoBuffer;
	uni_cmd->ucBssInfoIdx = cmd->ucBssIndex;
	tag = (struct UNI_CMD_BSSINFO_POWER_SAVE *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_BSSINFO_TAG_POWER_SAVE;
	tag->u2Length = sizeof(*tag);
	tag->ucPsProfile = cmd->ucPsProfile;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdSetWmmPsTestParams(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_SET_WMM_PS_TEST_STRUCT *cmd;
	struct UNI_CMD_BSSINFO *uni_cmd;
	struct UNI_CMD_BSSINFO_UAPSD *uapsd;
	struct UNI_CMD_BSSINFO_WMM_PS_TEST *wmm;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_BSSINFO) +
			       sizeof(struct UNI_CMD_BSSINFO_UAPSD) +
	     		       sizeof(struct UNI_CMD_BSSINFO_WMM_PS_TEST);

	if (info->ucCID != CMD_ID_SET_WMM_PS_TEST_PARMS ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_SET_WMM_PS_TEST_STRUCT *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_BSSINFO,
			max_cmd_len, NULL, NULL);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_BSSINFO *) entry->pucInfoBuffer;
	uni_cmd->ucBssInfoIdx = cmd->ucBssIndex;

	uapsd = (struct UNI_CMD_BSSINFO_UAPSD *) uni_cmd->aucTlvBuffer;
	uapsd->u2Tag = UNI_CMD_BSSINFO_TAG_UAPSD;
	uapsd->u2Length = sizeof(*uapsd);
	uapsd->ucBmpDeliveryAC = (cmd->bmfgApsdEnAc & BITS(4, 7)) >> 4;
	uapsd->ucBmpTriggerAC = cmd->bmfgApsdEnAc & BITS(0, 3);

	wmm = (struct UNI_CMD_BSSINFO_WMM_PS_TEST *)
		(uni_cmd->aucTlvBuffer + sizeof(*uapsd));
	wmm->u2Tag = UNI_CMD_BSSINFO_TAG_WMM_PS_TEST;
	wmm->u2Length = sizeof(*wmm);
	wmm->ucIsEnterPsAtOnce = cmd->ucIsEnterPsAtOnce;
	wmm->ucIsDisableUcTrigger = cmd->ucIsDisableUcTrigger;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdSetUapsd(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_CUSTOM_UAPSD_PARAM_STRUCT *cmd;
	struct UNI_CMD_BSSINFO *uni_cmd;
	struct UNI_CMD_BSSINFO_UAPSD *uapsd;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_BSSINFO) +
			       sizeof(struct UNI_CMD_BSSINFO_UAPSD);
	struct PARAM_CUSTOM_UAPSD_PARAM_STRUCT *param;

	if (info->ucCID != CMD_ID_SET_UAPSD_PARAM ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_CUSTOM_UAPSD_PARAM_STRUCT *) info->pucInfoBuffer;
	param = (struct PARAM_CUSTOM_UAPSD_PARAM_STRUCT *)
							info->pvSetQueryBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_BSSINFO, max_cmd_len,
			nicUniCmdEventSetCommon, nicUniCmdTimeoutCommon);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_BSSINFO *) entry->pucInfoBuffer;
	uni_cmd->ucBssInfoIdx = param->ucBssIdx;

	uapsd = (struct UNI_CMD_BSSINFO_UAPSD *) uni_cmd->aucTlvBuffer;
	uapsd->u2Tag = UNI_CMD_BSSINFO_TAG_UAPSD;
	uapsd->u2Length = sizeof(*uapsd);
	uapsd->ucBmpDeliveryAC =
		((cmd->fgEnAPSD_AcBe << 0) |
		 (cmd->fgEnAPSD_AcBk << 1) |
		 (cmd->fgEnAPSD_AcVi << 2) |
		 (cmd->fgEnAPSD_AcVo << 3));
	uapsd->ucBmpTriggerAC =
		((cmd->fgEnAPSD_AcBe << 0) |
		 (cmd->fgEnAPSD_AcBk << 1) |
		 (cmd->fgEnAPSD_AcVi << 2) |
		 (cmd->fgEnAPSD_AcVo << 3));

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdTwtArgtUpdate(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
#if (CFG_SUPPORT_TWT == 1)
	struct _EXT_CMD_TWT_ARGT_UPDATE_T *cmd;
	struct UNI_CMD_TWT *uni_cmd;
	struct UNI_CMD_TWT_ARGT_UPDATE *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_TWT) +
	     		       sizeof(struct UNI_CMD_TWT_ARGT_UPDATE);

	if (info->ucCID != CMD_ID_LAYER_0_EXT_MAGIC_NUM ||
	    info->ucExtCID != EXT_CMD_ID_TWT_AGRT_UPDATE ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct _EXT_CMD_TWT_ARGT_UPDATE_T *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_TWT, max_cmd_len,
			NULL, NULL);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_TWT *) entry->pucInfoBuffer;
	uni_cmd->ucBssInfoIdx = cmd->ucBssIndex;
	tag = (struct UNI_CMD_TWT_ARGT_UPDATE *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_TWT_TAG_AGRT_UPDATE;
	tag->u2Length = sizeof(*tag);
	tag->ucAgrtTblIdx = cmd->ucAgrtTblIdx;
	tag->ucAgrtCtrlFlag = cmd->ucAgrtCtrlFlag;
	tag->ucBssIndex = cmd->ucBssIndex;
	tag->ucOwnMacId = cmd->ucOwnMacId;
	tag->ucFlowId = cmd->ucFlowId;
	tag->u2PeerIdGrpId = cmd->u2PeerIdGrpId;
	tag->ucAgrtSpDuration = cmd->ucAgrtSpDuration;
	tag->u4AgrtSpStartTsf_low = cmd->u4AgrtSpStartTsfLow;
	tag->u4AgrtSpStartTsf_high = cmd->u4AgrtSpStartTsfHigh;
	tag->u2AgrtSpWakeIntvlMantissa = cmd->u2AgrtSpWakeIntvlMantissa;
	tag->ucAgrtSpWakeIntvlExponent = cmd->ucAgrtSpWakeIntvlExponent;
	tag->fgIsRoleAp = cmd->ucIsRoleAp;
	tag->ucAgrtParaBitmap = cmd->ucAgrtParaBitmap;
	tag->ucReserved_a = cmd->ucReserved_a;
	tag->u2Reserved_b = cmd->u2Reserved_b;
	tag->ucGrpMemberCnt = cmd->ucGrpMemberCnt;
	tag->ucReserved_c = cmd->ucReserved_c;
	tag->u2Reserved_d = cmd->u2Reserved_d;
#if 0  /* No need for STA*/
	kalMemCopy(tag->au2StaList, cmd->au2StaList, sizeof(tag->au2StaList));
#endif

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
#else
	return WLAN_STATUS_NOT_SUPPORTED;
#endif
}

uint32_t nicUniCmdStaRecUpdateExt(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_STAREC_UPDATE *cmd;
	struct UNI_CMD_STAREC *uni_cmd;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_STAREC);

	if (info->ucCID != CMD_ID_LAYER_0_EXT_MAGIC_NUM ||
	    info->ucExtCID != EXT_CMD_ID_STAREC_UPDATE)
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_STAREC_UPDATE *) info->pucInfoBuffer;
	switch (TAG_ID(cmd->aucBuffer)) {
	case STA_REC_BASIC:
		max_cmd_len += sizeof(struct UNI_CMD_STAREC_BASIC);
		break;
	case STA_REC_BF:
		max_cmd_len += sizeof(struct UNI_CMD_STAREC_BF);
		break;
	default:
		return WLAN_STATUS_NOT_ACCEPTED;
	}

	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_STAREC_INFO, max_cmd_len,
			nicUniCmdEventSetCommon, nicUniCmdTimeoutCommon);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_STAREC *) entry->pucInfoBuffer;
	uni_cmd->ucBssInfoIdx = cmd->ucBssIndex;
	WCID_SET_H_L(uni_cmd->ucWlanIdxHnVer, uni_cmd->ucWlanIdxL,
		cmd->ucWlanIdx);

	switch (TAG_ID(cmd->aucBuffer)) {
	case STA_REC_BASIC: {
		struct STAREC_COMMON *cmn;
		struct UNI_CMD_STAREC_BASIC *tag;

		cmn = (struct STAREC_COMMON *) cmd->aucBuffer;
		tag = (struct UNI_CMD_STAREC_BASIC *) uni_cmd->aucTlvBuffer;
		tag->u2Tag = UNI_CMD_STAREC_TAG_BASIC;
		tag->u2Length = sizeof(*tag);
		COPY_MAC_ADDR(tag->aucPeerMacAddr, cmn->aucPeerMacAddr);
		tag->u4ConnectionType = cmn->u4ConnectionType;
		tag->ucConnectionState = cmn->ucConnectionState;
		tag->ucIsQBSS = cmn->ucIsQBSS;
		tag->u2AID = cmn->u2AID;
		tag->u2ExtraInfo = cmn->u2ExtraInfo;
	}
		break;
	case STA_REC_BF: {
		struct CMD_STAREC_BF *bf;
		struct UNI_CMD_STAREC_BF *tag;

		bf = (struct CMD_STAREC_BF *) cmd->aucBuffer;
		tag = (struct UNI_CMD_STAREC_BF *) uni_cmd->aucTlvBuffer;
		tag->u2Tag = UNI_CMD_STAREC_TAG_BF;
		tag->u2Length = sizeof(*tag);
		tag->rTxBfPfmuInfo.u2PfmuId = bf->rTxBfPfmuInfo.u2PfmuId;
		tag->rTxBfPfmuInfo.fgSU_MU = bf->rTxBfPfmuInfo.fgSU_MU;
		tag->rTxBfPfmuInfo.u1TxBfCap = bf->rTxBfPfmuInfo.u1TxBfCap;
		tag->rTxBfPfmuInfo.ucSoundingPhy =
			bf->rTxBfPfmuInfo.ucSoundingPhy;
		tag->rTxBfPfmuInfo.ucNdpaRate = bf->rTxBfPfmuInfo.ucNdpaRate;
		tag->rTxBfPfmuInfo.ucNdpRate = bf->rTxBfPfmuInfo.ucNdpRate;
		tag->rTxBfPfmuInfo.ucReptPollRate =
			bf->rTxBfPfmuInfo.ucReptPollRate;
		tag->rTxBfPfmuInfo.ucTxMode = bf->rTxBfPfmuInfo.ucTxMode;
		tag->rTxBfPfmuInfo.ucNc = bf->rTxBfPfmuInfo.ucNc;
		tag->rTxBfPfmuInfo.ucNr = bf->rTxBfPfmuInfo.ucNr;
		tag->rTxBfPfmuInfo.ucCBW = bf->rTxBfPfmuInfo.ucCBW;
		tag->rTxBfPfmuInfo.ucTotMemRequire =
			bf->rTxBfPfmuInfo.ucTotMemRequire;
		tag->rTxBfPfmuInfo.ucMemRequire20M =
			bf->rTxBfPfmuInfo.ucMemRequire20M;
		tag->rTxBfPfmuInfo.ucMemRow0 = bf->rTxBfPfmuInfo.ucMemRow0;
		tag->rTxBfPfmuInfo.ucMemCol0 = bf->rTxBfPfmuInfo.ucMemCol0;
		tag->rTxBfPfmuInfo.ucMemRow1 = bf->rTxBfPfmuInfo.ucMemRow1;
		tag->rTxBfPfmuInfo.ucMemCol1 = bf->rTxBfPfmuInfo.ucMemCol1;
		tag->rTxBfPfmuInfo.ucMemRow2 = bf->rTxBfPfmuInfo.ucMemRow2;
		tag->rTxBfPfmuInfo.ucMemCol2 = bf->rTxBfPfmuInfo.ucMemCol2;
		tag->rTxBfPfmuInfo.ucMemRow3 = bf->rTxBfPfmuInfo.ucMemRow3;
		tag->rTxBfPfmuInfo.ucMemCol3 = bf->rTxBfPfmuInfo.ucMemCol3;
		tag->rTxBfPfmuInfo.u2SmartAnt = bf->rTxBfPfmuInfo.u2SmartAnt;
		tag->rTxBfPfmuInfo.ucSEIdx = bf->rTxBfPfmuInfo.ucSEIdx;
		tag->rTxBfPfmuInfo.ucAutoSoundingCtrl =
			bf->rTxBfPfmuInfo.ucAutoSoundingCtrl;
		tag->rTxBfPfmuInfo.uciBfTimeOut =
			bf->rTxBfPfmuInfo.uciBfTimeOut;
		tag->rTxBfPfmuInfo.uciBfDBW = bf->rTxBfPfmuInfo.uciBfDBW;
		tag->rTxBfPfmuInfo.uciBfNcol = bf->rTxBfPfmuInfo.uciBfNcol;
		tag->rTxBfPfmuInfo.uciBfNrow = bf->rTxBfPfmuInfo.uciBfNrow;
		tag->rTxBfPfmuInfo.nr_bw160 = bf->rTxBfPfmuInfo.u1NrBw160;
		tag->rTxBfPfmuInfo.nc_bw160 = bf->rTxBfPfmuInfo.u1NcBw160;
		tag->rTxBfPfmuInfo.ru_start_idx =
			bf->rTxBfPfmuInfo.u1RuStartIdx;
		tag->rTxBfPfmuInfo.ru_end_idx = bf->rTxBfPfmuInfo.u1RuEndIdx;
		tag->rTxBfPfmuInfo.trigger_su = bf->rTxBfPfmuInfo.fgTriggerSu;
		tag->rTxBfPfmuInfo.trigger_mu = bf->rTxBfPfmuInfo.fgTriggerMu;
		tag->rTxBfPfmuInfo.ng16_su = bf->rTxBfPfmuInfo.fgNg16Su;
		tag->rTxBfPfmuInfo.ng16_mu = bf->rTxBfPfmuInfo.fgNg16Mu;
		tag->rTxBfPfmuInfo.codebook42_su =
			bf->rTxBfPfmuInfo.fgCodebook42Su;
		tag->rTxBfPfmuInfo.codebook75_mu =
			bf->rTxBfPfmuInfo.fgCodebook75Mu;
		tag->rTxBfPfmuInfo.he_ltf = bf->rTxBfPfmuInfo.u1HeLtf;

	}
		break;
	}

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdBFAction(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	union CMD_TXBF_ACTION *cmd;
	struct UNI_CMD_BF *uni_cmd;
	struct UNI_CMD_BF_SND *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_BF) +
	     		       sizeof(struct UNI_CMD_BF_SND);
	uint32_t bf_id;
	uint8_t i;

	if (info->ucCID != CMD_ID_LAYER_0_EXT_MAGIC_NUM ||
	    info->ucExtCID != EXT_CMD_ID_BF_ACTION)
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (union CMD_TXBF_ACTION *) info->pucInfoBuffer;

	bf_id =	cmd->rTxBfSoundingStart.ucCmdCategoryID;
	if (bf_id != BF_SOUNDING_ON)
		return WLAN_STATUS_NOT_ACCEPTED;

	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_BF, max_cmd_len,
			nicUniCmdEventSetCommon, nicUniCmdTimeoutCommon);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_BF *) entry->pucInfoBuffer;
	tag = (struct UNI_CMD_BF_SND *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_BF_TAG_SOUNDING_ON;
	tag->u2Length = sizeof(*tag);
 	tag->u1SuMuSndMode = cmd->rTxBfSoundingStart.ucSuMuSndMode;
	tag->u1StaNum = cmd->rTxBfSoundingStart.ucStaNum;
	for (i = 0; i < 4; i++)
		tag->u2WlanId[i] = cmd->rTxBfSoundingStart.ucWlanId[i];
	tag->u4SndIntv = cmd->rTxBfSoundingStart.u4SoundingInterval;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdSerAction(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct EXT_CMD_SER_T *cmd;
	struct UNI_CMD_SER *uni_cmd;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_SER);

	if (info->ucCID != CMD_ID_LAYER_0_EXT_MAGIC_NUM ||
	    info->ucExtCID != EXT_CMD_ID_SER)
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct EXT_CMD_SER_T *) info->pucInfoBuffer;
	switch (cmd->ucAction) {
	case SER_ACTION_QUERY:
		max_cmd_len += sizeof(struct UNI_CMD_SER_QUERY);
		break;
	case SER_ACTION_SET:
		max_cmd_len += sizeof(struct UNI_CMD_SER_ENABLE);
		break;
	case SER_ACTION_SET_ENABLE_MASK:
		max_cmd_len += sizeof(struct UNI_CMD_SER_SET);
		break;
	case SER_ACTION_RECOVER:
		max_cmd_len += sizeof(struct UNI_CMD_SER_TRIGGER);
		break;
	default:
		DBGLOG(NIC, ERROR, "unknown action %d\n", cmd->ucAction);
		return WLAN_STATUS_NOT_ACCEPTED;
	}

	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_SER, max_cmd_len,
			NULL, nicUniCmdTimeoutCommon);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_SER *) entry->pucInfoBuffer;
	switch (cmd->ucAction) {
	case SER_ACTION_QUERY: {
		struct UNI_CMD_SER_QUERY *tag =
			(struct UNI_CMD_SER_QUERY *)uni_cmd->aucTlvBuffer;

		tag->u2Tag = UNI_CMD_SER_TAG_QUERY;
		tag->u2Length = sizeof(*tag);
	}
		break;
	case SER_ACTION_SET: {
		struct UNI_CMD_SER_ENABLE *tag =
			(struct UNI_CMD_SER_ENABLE *)uni_cmd->aucTlvBuffer;

		tag->u2Tag = UNI_CMD_SER_TAG_ENABLE;
		tag->u2Length = sizeof(*tag);
		tag->fgEnable = cmd->ucSerSet;
	}
		break;
	case SER_ACTION_SET_ENABLE_MASK: {
		struct UNI_CMD_SER_SET *tag =
			(struct UNI_CMD_SER_SET *)uni_cmd->aucTlvBuffer;

		tag->u2Tag = UNI_CMD_SER_TAG_SET;
		tag->u2Length = sizeof(*tag);
		tag->u4EnableMask = cmd->ucSerSet;
	}
		break;
	case SER_ACTION_RECOVER: {
		struct UNI_CMD_SER_TRIGGER *tag =
			(struct UNI_CMD_SER_TRIGGER *)uni_cmd->aucTlvBuffer;

		tag->u2Tag = UNI_CMD_SER_TAG_TRIGGER;
		tag->u2Length = sizeof(*tag);
		tag->ucTriggerMethod = cmd->ucSerSet;
		tag->ucDbdcIdx = cmd->ucDbdcIdx;
	}
		break;
	}

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdGetTsf(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct _EXT_CMD_GET_MAC_INFO_T *cmd;
	struct UNI_CMD_GET_MAC_INFO *uni_cmd;
	struct UNI_CMD_MAC_INFO_TSF *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	struct _TWT_GET_TSF_CONTEXT_T *twt;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_GET_MAC_INFO) +
	     		       sizeof(struct UNI_CMD_MAC_INFO_TSF);

	if (info->ucCID != CMD_ID_LAYER_0_EXT_MAGIC_NUM ||
	    info->ucExtCID != EXT_CMD_ID_GET_MAC_INFO)
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct _EXT_CMD_GET_MAC_INFO_T *) info->pucInfoBuffer;
	twt = (struct _TWT_GET_TSF_CONTEXT_T *) info->pvSetQueryBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_GET_MAC_INFO, max_cmd_len,
			nicUniCmdEventGetTsfDone, NULL);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_GET_MAC_INFO *) entry->pucInfoBuffer;
	tag = (struct UNI_CMD_MAC_INFO_TSF *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_MAC_INFO_TAG_TSF;
	tag->u2Length = sizeof(*tag);
 	tag->ucDbdcIdx = ENUM_BAND_AUTO;
	tag->ucHwBssidIndex = cmd->rExtraArgument.rTsfArg.ucHwBssidIndex;
	tag->ucBssIndex = twt->ucBssIdx;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniUpdateDevInfo(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_DEV_INFO_UPDATE *cmd;
	struct CMD_DEVINFO_ACTIVE *data;
	struct UNI_CMD_DEVINFO *uni_cmd;
	struct UNI_CMD_DEVINFO_ACTIVE *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_DEVINFO) +
			       sizeof(struct UNI_CMD_DEVINFO_ACTIVE);

	if (info->ucCID != CMD_ID_LAYER_0_EXT_MAGIC_NUM ||
	    info->ucExtCID != EXT_CMD_ID_DEVINFO_UPDATE)
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_DEV_INFO_UPDATE *) info->pucInfoBuffer;
	data = (struct CMD_DEVINFO_ACTIVE *) cmd->aucBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_DEVINFO, max_cmd_len,
			nicUniCmdEventSetCommon, nicUniCmdTimeoutCommon);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_DEVINFO *) entry->pucInfoBuffer;
	uni_cmd->ucOwnMacIdx = cmd->ucOwnMacIdx;
	uni_cmd->ucDbdcIdx = cmd->ucDbdcIdx;

	tag = (struct UNI_CMD_DEVINFO_ACTIVE *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_DEVINFO_TAG_ACTIVE;
	tag->u2Length = sizeof(*tag);
	tag->ucActive = data->ucActive;
	COPY_MAC_ADDR(tag->aucOwnMacAddr, data->aucOwnMacAddr);

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdStaRecConnType(struct ADAPTER *ad, uint32_t legacy_sta_type)
{
	if (legacy_sta_type == STA_TYPE_LEGACY_AP)
		return CONNECTION_INFRA_AP;
	else if (legacy_sta_type == STA_TYPE_LEGACY_CLIENT)
		return CONNECTION_INFRA_STA;
	else if (legacy_sta_type == STA_TYPE_P2P_GO)
		return CONNECTION_P2P_GO;
	else if (legacy_sta_type == STA_TYPE_P2P_GC)
		return CONNECTION_P2P_GC;

	DBGLOG(NIC, ERROR, "wrong sta_type=%d\n", legacy_sta_type);
	return CONNECTION_INFRA_STA;
}

uint32_t nicUniCmdStaRecTagBasic(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_UPDATE_STA_RECORD *cmd)
{
	struct UNI_CMD_STAREC_BASIC *tag = (struct UNI_CMD_STAREC_BASIC *)buf;

	tag->u2Tag = UNI_CMD_STAREC_TAG_BASIC;
	tag->u2Length = sizeof(*tag);
	tag->u4ConnectionType = nicUniCmdStaRecConnType(ad, cmd->ucStaType);
	tag->ucConnectionState = STATE_CONNECTED;
	tag->ucIsQBSS = cmd->ucIsQoS;
	tag->u2AID = cmd->u2AssocId;
	COPY_MAC_ADDR(tag->aucPeerMacAddr, cmd->aucMacAddr);
	tag->u2ExtraInfo = STAREC_COMMON_EXTRAINFO_V2 |
			   STAREC_COMMON_EXTRAINFO_NEWSTAREC;

	return tag->u2Length;
}

uint32_t nicUniCmdStaRecTagState(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_UPDATE_STA_RECORD *cmd)
{
	struct UNI_CMD_STAREC_STATE_INFO *tag =
		(struct UNI_CMD_STAREC_STATE_INFO *)buf;

	tag->u2Tag = UNI_CMD_STAREC_TAG_STATE_CHANGED;
	tag->u2Length = sizeof(*tag);
	tag->ucStaState = cmd->ucStaState;
	tag->u4Flags = cmd->u4Flags;
	tag->ucVhtOpMode = cmd->ucVhtOpMode;
	tag->ucActionType = STA_REC_CMD_ACTION_STA;

	return tag->u2Length;
}

uint32_t nicUniCmdStaRecTagPhyInfo(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_UPDATE_STA_RECORD *cmd)
{
	struct UNI_CMD_STAREC_PHY_INFO *tag =
		(struct UNI_CMD_STAREC_PHY_INFO *)buf;

	tag->u2Tag = UNI_CMD_STAREC_TAG_PHY_INFO;
	tag->u2Length = sizeof(*tag);
	tag->u2BSSBasicRateSet = cmd->u2BSSBasicRateSet;
	tag->ucDesiredPhyTypeSet = cmd->ucDesiredPhyTypeSet;
	tag->ucAmpduParam = cmd->ucAmpduParam;
	tag->ucRtsPolicy = cmd->ucRtsPolicy;
	tag->ucRCPI = cmd->ucRCPI;

	return tag->u2Length;
}

uint32_t nicUniCmdStaRecTagHtInfo(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_UPDATE_STA_RECORD *cmd)
{
	struct UNI_CMD_STAREC_HT_INFO *tag =
		(struct UNI_CMD_STAREC_HT_INFO *)buf;

	if (cmd->u2HtCapInfo == 0 && cmd->u2HtExtendedCap == 0)
		return 0;

	tag->u2Tag = UNI_CMD_STAREC_TAG_HT_BASIC;
	tag->u2Length = sizeof(*tag);
	tag->u2HtCap = cmd->u2HtCapInfo;
	tag->u2HtExtendedCap = cmd->u2HtExtendedCap;

	return tag->u2Length;
}

uint32_t nicUniCmdStaRecTagVhtInfo(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_UPDATE_STA_RECORD *cmd)
{
	struct UNI_CMD_STAREC_VHT_INFO *tag =
		(struct UNI_CMD_STAREC_VHT_INFO *)buf;

	if (cmd->u4VhtCapInfo == 0)
		return 0;

	tag->u2Tag = UNI_CMD_STAREC_TAG_VHT_BASIC;
	tag->u2Length = sizeof(*tag);
	tag->u4VhtCap = cmd->u4VhtCapInfo;
	tag->u2VhtRxMcsMap = cmd->u2VhtRxMcsMap;
	tag->u2VhtTxMcsMap = cmd->u2VhtTxMcsMap;

	return tag->u2Length;
}

#if (CFG_SUPPORT_802_11AX == 1)
uint32_t nicUniCmdStaRecTagHeBasic(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_UPDATE_STA_RECORD *cmd)
{
	struct UNI_CMD_STAREC_HE_BASIC *tag =
		(struct UNI_CMD_STAREC_HE_BASIC *)buf;
	static uint8_t zero[HE_PHY_CAP_BYTE_NUM];

	if (!kalMemCmp(cmd->ucHeMacCapInfo, zero, HE_MAC_CAP_BYTE_NUM) &&
	    !kalMemCmp(cmd->ucHePhyCapInfo, zero, HE_PHY_CAP_BYTE_NUM))
		return 0;

	tag->u2Tag = UNI_CMD_STAREC_TAG_HE_BASIC;
	tag->u2Length = sizeof(*tag);
	kalMemCopy(tag->aucHeMacCapInfo, cmd->ucHeMacCapInfo,
			sizeof(tag->aucHeMacCapInfo));
	kalMemCopy(tag->aucHePhyCapInfo, cmd->ucHePhyCapInfo,
			sizeof(tag->aucHePhyCapInfo));
	tag->ucPktExt = 2; /* mobile */
	tag->au2RxMaxNssMcs[0] = cmd->u2HeRxMcsMapBW80;
	tag->au2RxMaxNssMcs[1] = cmd->u2HeRxMcsMapBW160;
	tag->au2RxMaxNssMcs[2] = cmd->u2HeRxMcsMapBW80P80;

	return tag->u2Length;
}

uint32_t nicUniCmdStaRecTagHe6gCap(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_UPDATE_STA_RECORD *cmd)
{
	struct UNI_CMD_STAREC_HE_6G_CAP *tag =
		(struct UNI_CMD_STAREC_HE_6G_CAP *)buf;

	if (cmd->u2He6gBandCapInfo == 0)
		return 0;

	tag->u2Tag = UNI_CMD_STAREC_TAG_HE_6G_CAP;
	tag->u2Length = sizeof(*tag);
	tag->u2He6gBandCapInfo = cmd->u2He6gBandCapInfo;
	return tag->u2Length;
}
#endif

#if (CFG_SUPPORT_802_11BE == 1)
uint32_t nicUniCmdStaRecTagEhtInfo(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_UPDATE_STA_RECORD *cmd)
{
	struct UNI_CMD_STAREC_EHT_BASIC *tag =
		(struct UNI_CMD_STAREC_EHT_BASIC *)buf;
	static uint8_t zero[EHT_PHY_CAP_BYTE_NUM];

	if (!kalMemCmp(cmd->ucEhtMacCapInfo, zero, EHT_MAC_CAP_BYTE_NUM) &&
	    !kalMemCmp(cmd->ucEhtPhyCapInfo, zero, EHT_PHY_CAP_BYTE_NUM))
		return 0;

	tag->u2Tag = UNI_CMD_STAREC_TAG_EHT_BASIC;
	tag->u2Length = sizeof(*tag);
	tag->ucTidBitmap = 0xff;
	WLAN_GET_FIELD_16(&cmd->ucEhtMacCapInfo[0], &tag->u2EhtMacCap);
	WLAN_GET_FIELD_64(&cmd->ucEhtPhyCapInfo[0], &tag->u8EhtPhyCap);

	DBGLOG(INIT, INFO, "[%d] bss=%d,tid=0x%x,mac_cap=0x%x,phy_cap=0x%lx\n",
		cmd->ucStaIndex,
		cmd->ucBssIndex,
		tag->ucTidBitmap,
		tag->u2EhtMacCap,
		tag->u8EhtPhyCap);

	return tag->u2Length;
}
#endif

#if (CFG_SUPPORT_802_11BE_MLO == 1)
uint32_t nicUniCmdStaRecTagMldSetup(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_UPDATE_STA_RECORD *cmd)
{
	struct UNI_CMD_STAREC_MLD_SETUP *tag = (struct UNI_CMD_STAREC_MLD_SETUP *)buf;
	struct UNI_CMD_STAREC_LINK_INFO *link;
	struct STA_RECORD *prStaRec = cnmGetStaRecByIndex(ad, cmd->ucStaIndex);
	struct MLD_STA_RECORD *prMldStaRec = mldStarecGetByStarec(ad, prStaRec);
	struct LINK *prStaList = &prMldStaRec->rStarecList;
	struct STA_RECORD *prCurStaRec;

	if (prStaRec->ucStaState != STA_STATE_3)
		return 0;

	if (!prMldStaRec)
		return 0;

	tag->u2Tag = UNI_CMD_STAREC_TAG_MLD_SETUP;
	tag->u2Length = sizeof(*tag) + sizeof(*link) * prStaList->u4NumElem;
	COPY_MAC_ADDR(tag->aucPeerMldAddr, prMldStaRec->aucPeerMldAddr);
	tag->u2PrimaryMldId = prMldStaRec->u2PrimaryMldId;
	tag->u2SecondMldId = prMldStaRec->u2SecondMldId;
	tag->u2SetupWlanId = prMldStaRec->u2SetupWlanId;
	tag->ucLinkNumber = prStaList->u4NumElem;

	DBGLOG(INIT, INFO, "[%d] bss=%d,pri=%d,sec=%d,setup=%d,num=%d,mac=" MACSTR "\n",
		prStaRec->ucIndex,
		cmd->ucBssIndex,
		tag->u2PrimaryMldId,
		tag->u2SecondMldId,
		tag->u2SetupWlanId,
		tag->ucLinkNumber,
		MAC2STR(prMldStaRec->aucPeerMldAddr));

	link = (struct UNI_CMD_STAREC_LINK_INFO *)tag->aucLinkInfo;
	LINK_FOR_EACH_ENTRY(prCurStaRec, prStaList, rLinkEntryMld,
			struct STA_RECORD) {
		link->ucBssIdx = prCurStaRec->ucBssIndex;
		link->u2WlanIdx = prCurStaRec->ucWlanIndex;
		DBGLOG(INIT, INFO, "\tbss=%d,wlan_idx=%d\n",
			link->ucBssIdx,
			link->u2WlanIdx);
		link++;
	}

	return tag->u2Length;
}

uint32_t nicUniCmdMldStaTeardown(struct ADAPTER *ad,
	struct STA_RECORD *prStaRec)
{
	struct UNI_CMD_STAREC *uni_cmd;
	struct UNI_CMD_STAREC_MLD_TEARDOWN *tag;
	struct MLD_STA_RECORD *prMldStaRec = mldStarecGetByStarec(ad, prStaRec);
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_STAREC) +
			sizeof(struct UNI_CMD_STAREC_MLD_TEARDOWN);
	uint32_t status = WLAN_STATUS_SUCCESS;

	if (!prMldStaRec)
		return WLAN_STATUS_SUCCESS;

	uni_cmd = (struct UNI_CMD_STAREC *) cnmMemAlloc(ad,
				RAM_TYPE_MSG, max_cmd_len);
	if (!uni_cmd) {
		DBGLOG(INIT, ERROR,
		       "Allocate UNI_CMD_BF ==> FAILED.\n");
		return WLAN_STATUS_FAILURE;
	}

	uni_cmd->ucBssInfoIdx = prStaRec->ucBssIndex;
	WCID_SET_H_L(uni_cmd->ucWlanIdxHnVer, uni_cmd->ucWlanIdxL,
		prStaRec->ucWlanIndex);
	tag = (struct UNI_CMD_STAREC_MLD_TEARDOWN *)uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_STAREC_TAG_MLD_TEARDOWN;
	tag->u2Length = sizeof(*tag);

	DBGLOG(INIT, INFO, "[%d] bss_idx: %d\n",
		prStaRec->ucIndex,
		uni_cmd->ucBssInfoIdx);

	status = wlanSendSetQueryUniCmd(ad,
			     UNI_CMD_ID_STAREC_INFO,
			     TRUE,
			     FALSE,
			     FALSE,
			     nicUniCmdEventSetCommon,
			     nicUniCmdTimeoutCommon,
			     max_cmd_len,
			     (void *)uni_cmd, NULL, 0);

	cnmMemFree(ad, uni_cmd);
	return status;
}

uint32_t nicUniCmdStaRecTagEhtMld(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_UPDATE_STA_RECORD *cmd)
{
	struct STA_RECORD *prStaRec = cnmGetStaRecByIndex(ad, cmd->ucStaIndex);
	struct UNI_CMD_STAREC_EHT_MLD *tag = (struct UNI_CMD_STAREC_EHT_MLD *)buf;
	struct MLD_STA_RECORD *prMldStarec = mldStarecGetByStarec(ad, prStaRec);

	if (prStaRec->ucStaState != STA_STATE_3)
		return 0;

	if (!prMldStarec)
		return 0;

	tag->u2Tag = UNI_CMD_STAREC_TAG_EHT_MLD;
	tag->u2Length = sizeof(struct UNI_CMD_STAREC_EHT_MLD);
	tag->fgNSEP = prMldStarec->fgNSEP;
	tag->ucEmlmrBitmap = prMldStarec->ucEmlmrBitmap;
	tag->ucEmlsrBitmap = prMldStarec->ucEmlsrBitmap;
	kalMemCopy(tag->afgStrCapBitmap,
		prMldStarec->aucStrBitmap,
		sizeof(tag->afgStrCapBitmap));

	DBGLOG(INIT, INFO, "[%d] bss=%d,nsep=%d,emlmr=0x%x,emlsr=0x%x,str[0x%x,0x%x,0x%x]\n",
		prStaRec->ucIndex,
		cmd->ucBssIndex,
		tag->fgNSEP,
		tag->ucEmlmrBitmap,
		tag->ucEmlsrBitmap,
		tag->afgStrCapBitmap[0],
		tag->afgStrCapBitmap[1],
		tag->afgStrCapBitmap[2]);

	return tag->u2Length;
}
#endif

uint32_t nicUniCmdStaRecTagRA(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_UPDATE_STA_RECORD *cmd)
{
	struct UNI_CMD_STAREC_RA_INFO *tag =
		(struct UNI_CMD_STAREC_RA_INFO *)buf;

	tag->u2Tag = UNI_CMD_STAREC_TAG_RA;
	tag->u2Length = sizeof(*tag);
	tag->u2DesiredNonHTRateSet = cmd->u2DesiredNonHTRateSet;
	kalMemCopy(tag->aucRxMcsBitmask, cmd->aucRxMcsBitmask, 10);

	return tag->u2Length;
}

uint32_t nicUniCmdStaRecTagBAOffload(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_UPDATE_STA_RECORD *cmd)
{
	struct UNI_CMD_STAREC_BA_OFFLOAD_INFO *tag =
		(struct UNI_CMD_STAREC_BA_OFFLOAD_INFO *)buf;

	tag->u2Tag = UNI_CMD_STAREC_TAG_BA_OFFLOAD;
	tag->u2Length = sizeof(*tag);
	tag->ucTxAmpdu = cmd->ucTxAmpdu;
	tag->ucRxAmpdu = cmd->ucRxAmpdu;
	tag->ucTxAmsduInAmpdu = cmd->ucTxAmsduInAmpdu;
	tag->ucRxAmsduInAmpdu = cmd->ucRxAmsduInAmpdu;
	tag->u4TxMaxAmsduInAmpduLen = cmd->u4TxMaxAmsduInAmpduLen;

	if (cmd->ucDesiredPhyTypeSet & PHY_TYPE_SET_802_11BE) {
		tag->u2TxBaSize = cmd->rBaSize.rEhtBaSize.u2TxBaSize;
		tag->u2RxBaSize = cmd->rBaSize.rEhtBaSize.u2RxBaSize;
	} else if (cmd->ucDesiredPhyTypeSet & PHY_TYPE_SET_802_11AX) {
		tag->u2TxBaSize = cmd->rBaSize.rHeBaSize.u2TxBaSize;
		tag->u2RxBaSize = cmd->rBaSize.rHeBaSize.u2RxBaSize;
	} else {
		tag->u2TxBaSize = cmd->rBaSize.rHtVhtBaSize.ucTxBaSize;
		tag->u2RxBaSize = cmd->rBaSize.rHtVhtBaSize.ucRxBaSize;
	}
	return tag->u2Length;
}

uint32_t nicUniCmdStaRecTagUapsd(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_UPDATE_STA_RECORD *cmd)
{
	struct UNI_CMD_STAREC_UAPSD_INFO *tag =
		(struct UNI_CMD_STAREC_UAPSD_INFO *)buf;

	tag->u2Tag = UNI_CMD_STAREC_TAG_UAPSD;
	tag->u2Length = sizeof(*tag);
	tag->fgIsUapsdSupported = cmd->ucIsUapsdSupported;
	tag->ucUapsdAc = cmd->ucUapsdAc;
	tag->ucUapsdSp = cmd->ucUapsdSp;
	return tag->u2Length;
}

struct UNI_CMD_STAREC_TAG_HANDLE arUpdateStaRecTable[] = {
	{sizeof(struct UNI_CMD_STAREC_BASIC), nicUniCmdStaRecTagBasic},
	{sizeof(struct UNI_CMD_STAREC_HT_INFO), nicUniCmdStaRecTagHtInfo},
	{sizeof(struct UNI_CMD_STAREC_VHT_INFO), nicUniCmdStaRecTagVhtInfo},
#if (CFG_SUPPORT_802_11AX == 1)
	{sizeof(struct UNI_CMD_STAREC_HE_BASIC), nicUniCmdStaRecTagHeBasic},
	{sizeof(struct UNI_CMD_STAREC_HE_6G_CAP), nicUniCmdStaRecTagHe6gCap},
#endif
	{sizeof(struct UNI_CMD_STAREC_STATE_INFO), nicUniCmdStaRecTagState},
	{sizeof(struct UNI_CMD_STAREC_PHY_INFO), nicUniCmdStaRecTagPhyInfo},
	{sizeof(struct UNI_CMD_STAREC_RA_INFO), nicUniCmdStaRecTagRA},
	{sizeof(struct UNI_CMD_STAREC_BA_OFFLOAD_INFO),
	 nicUniCmdStaRecTagBAOffload},
	{sizeof(struct UNI_CMD_STAREC_UAPSD_INFO), nicUniCmdStaRecTagUapsd},
#if (CFG_SUPPORT_802_11BE == 1)
	{sizeof(struct UNI_CMD_STAREC_EHT_BASIC), nicUniCmdStaRecTagEhtInfo},
#endif
#if (CFG_SUPPORT_802_11BE_MLO == 1)
	{sizeof(struct UNI_CMD_STAREC_EHT_MLD), nicUniCmdStaRecTagEhtMld},
	{sizeof(struct UNI_CMD_STAREC_MLD_SETUP), nicUniCmdStaRecTagMldSetup},
#endif
};

uint32_t nicUniCmdUpdateStaRec(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_UPDATE_STA_RECORD *cmd;
	struct UNI_CMD_STAREC *uni_cmd;
	struct WIFI_UNI_CMD_ENTRY *entry;
	struct BSS_INFO *bss;
	uint8_t *pos;
	uint32_t max_cmd_len = 0;
	int i;

	if (info->ucCID != CMD_ID_UPDATE_STA_RECORD ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_UPDATE_STA_RECORD *) info->pucInfoBuffer;
	bss = GET_BSS_INFO_BY_INDEX(ad, cmd->ucBssIndex);
	max_cmd_len += sizeof(struct UNI_CMD_STAREC);
	for (i = 0; i < ARRAY_SIZE(arUpdateStaRecTable); i++)
		max_cmd_len += arUpdateStaRecTable[i].u4Size;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_STAREC_INFO, max_cmd_len,
		cmd->ucNeedResp ? nicUniCmdStaRecHandleEventPkt : NULL,	NULL);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	pos = entry->pucInfoBuffer;
	uni_cmd = (struct UNI_CMD_STAREC *) pos;
	uni_cmd->ucBssInfoIdx = cmd->ucBssIndex;
	WCID_SET_H_L(uni_cmd->ucWlanIdxHnVer, uni_cmd->ucWlanIdxL,
		cmd->ucWlanIndex);
	pos += sizeof(*uni_cmd);
	for (i = 0; i < ARRAY_SIZE(arUpdateStaRecTable); i++)
		pos += arUpdateStaRecTable[i].pfHandler(ad, pos, cmd);
	entry->u4SetQueryInfoLen = pos - entry->pucInfoBuffer;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

static uint32_t nicUniCmdChReqPrivilege(struct ADAPTER *ad,
		struct MSG_CH_REQ *msg,
		struct WIFI_UNI_CMD_ENTRY **out_entry)
{
	struct UNI_CMD_CNM *uni_cmd;
	struct WIFI_UNI_CMD_ENTRY *entry;
	struct UNI_CMD_CNM_CH_PRIVILEGE_REQ *tag;
	uint8_t i = 0;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_CNM);

	max_cmd_len += sizeof(struct UNI_CMD_CNM_CH_PRIVILEGE_REQ) *
		(msg->ucExtraChReqNum + 1);
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_CNM,
			max_cmd_len, NULL, NULL);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_CNM *) entry->pucInfoBuffer;

	tag = (struct UNI_CMD_CNM_CH_PRIVILEGE_REQ *)&uni_cmd->aucTlvBuffer[0];
	for (i = 0; i < msg->ucExtraChReqNum + 1; i++, tag++) {
		struct MSG_CH_REQ *sub_req = NULL;

		if (i == 0) {
			sub_req = (struct MSG_CH_REQ *)msg;
			tag->u2Tag = UNI_CMD_CNM_TAG_CH_PRIVILEGE_REQ;
		} else {
			sub_req = (struct MSG_CH_REQ *)&msg->aucBuffer[i];
			tag->u2Tag = UNI_CMD_CNM_TAG_CH_PRIVILEGE_MLO_SUB_REQ;
		}
		tag->u2Length = sizeof(*tag);
		tag->ucTokenID = sub_req->ucTokenID;
		tag->ucReqType = sub_req->eReqType;
		tag->u4MaxInterval = sub_req->u4MaxInterval;
		tag->ucBssIndex = sub_req->ucBssIndex;
		tag->ucRfBand = sub_req->eRfBand;
		tag->ucPrimaryChannel = sub_req->ucPrimaryChannel;
		tag->ucRfChannelWidth = sub_req->eRfChannelWidth;
		tag->ucRfSco = sub_req->eRfSco;
		tag->ucRfCenterFreqSeg1 = sub_req->ucRfCenterFreqSeg1;
		tag->ucRfCenterFreqSeg2 = sub_req->ucRfCenterFreqSeg2;
		tag->ucRfChannelWidthFromAP = sub_req->eRfChannelWidth;
		tag->ucRfCenterFreqSeg1FromAP = sub_req->ucRfCenterFreqSeg1;
		tag->ucRfCenterFreqSeg2FromAP = sub_req->ucRfCenterFreqSeg2;
		tag->ucDBDCBand = sub_req->eDBDCBand;

		DBGLOG(INIT, INFO, "bss=%d,token=%d,type=%d,interval=%d,ch[%d %d %d %d %d %d],dbdc=%d\n",
			tag->ucBssIndex,
			tag->ucTokenID,
			tag->ucReqType,
			tag->u4MaxInterval,
			tag->ucRfBand,
			tag->ucPrimaryChannel,
			tag->ucRfChannelWidth,
			tag->ucRfSco,
			tag->ucRfCenterFreqSeg1,
			tag->ucRfCenterFreqSeg2,
			tag->ucDBDCBand);
	}
	*out_entry = entry;

	return WLAN_STATUS_SUCCESS;
}

static uint32_t nicUniCmdChAbortPrivilege(struct ADAPTER *ad,
		struct MSG_CH_ABORT *msg,
		struct WIFI_UNI_CMD_ENTRY **out_entry)
{
	struct UNI_CMD_CNM *uni_cmd;
	struct WIFI_UNI_CMD_ENTRY *entry;
	struct UNI_CMD_CNM_CH_PRIVILEGE_ABORT *tag;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_CNM);

	max_cmd_len += sizeof(struct UNI_CMD_CNM_CH_PRIVILEGE_ABORT);
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_CNM,
			max_cmd_len, NULL, NULL);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_CNM *) entry->pucInfoBuffer;

	tag = (struct UNI_CMD_CNM_CH_PRIVILEGE_ABORT *)&uni_cmd->aucTlvBuffer[0];
	tag->u2Tag = UNI_CMD_CNM_TAG_CH_PRIVILEGE_ABORT;
	tag->u2Length = sizeof(*tag);
	tag->ucBssIndex = msg->ucBssIndex;
	tag->ucTokenID = msg->ucTokenID;
	if (msg->ucExtraChReqNum >= 1)
		tag->ucDBDCBand = ENUM_BAND_ALL;
	else
		tag->ucDBDCBand = msg->eDBDCBand;

	DBGLOG(INIT, INFO, "bss=%d,token=%d,dbdc=%d\n",
		tag->ucBssIndex,
		tag->ucTokenID,
		tag->ucDBDCBand);

	*out_entry = entry;

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdChPrivilege(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_CH_PRIVILEGE *cmd;
	struct WIFI_UNI_CMD_ENTRY *entry = NULL;
	uint32_t status = WLAN_STATUS_SUCCESS;

	if (info->ucCID != CMD_ID_CH_PRIVILEGE ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_CH_PRIVILEGE *) info->pucInfoBuffer;

	if (cmd->ucAction == CMD_CH_ACTION_ABORT)
		status = nicUniCmdChAbortPrivilege(ad, info->pvSetQueryBuffer, &entry);
	else
		status = nicUniCmdChReqPrivilege(ad, info->pvSetQueryBuffer, &entry);

	if (status == WLAN_STATUS_SUCCESS && entry)
		LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return status;
}

uint32_t nicUniCmdCnmGetInfo(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct PARAM_GET_CNM_T *cmd;
	struct UNI_CMD_CNM *uni_cmd;
	struct UNI_CMD_CNM_GET_INFO *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_CNM) +
			       sizeof(struct UNI_CMD_CNM_GET_INFO);

	if (info->ucCID != CMD_ID_GET_CNM ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct PARAM_GET_CNM_T *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_CNM,
		max_cmd_len, nicUniEventQueryCnmInfo, nicUniCmdTimeoutCommon);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_CNM *) entry->pucInfoBuffer;
	tag = (struct UNI_CMD_CNM_GET_INFO *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_CNM_TAG_GET_INFO;
	tag->u2Length = sizeof(*tag);

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdAccessReg(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_ACCESS_REG *cmd;
	struct UNI_CMD_ACCESS_REG *uni_cmd;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_ACCESS_REG);

	if (info->ucCID != CMD_ID_ACCESS_REG ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_ACCESS_REG *) info->pucInfoBuffer;

	if (UNI_IS_RFCR(cmd->u4Address))
		max_cmd_len += sizeof(struct UNI_CMD_ACCESS_RF_REG_BASIC);
	else
		max_cmd_len += sizeof(struct UNI_CMD_ACCESS_REG_BASIC);

	if (info->fgSetQuery)
		entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_ACCESS_REG,
				max_cmd_len, nicUniCmdEventSetCommon,
				nicUniCmdTimeoutCommon);
	else
		entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_ACCESS_REG,
				max_cmd_len, nicUniCmdEventQueryMcrRead,
				nicUniCmdTimeoutCommon);

	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_ACCESS_REG *) entry->pucInfoBuffer;
	if (UNI_IS_RFCR(cmd->u4Address)) {
		struct UNI_CMD_ACCESS_RF_REG_BASIC *tag =
		   (struct UNI_CMD_ACCESS_RF_REG_BASIC *) uni_cmd->aucTlvBuffer;

		tag->u2Tag = UNI_CMD_ACCESS_REG_TAG_RF_REG_BASIC;
		tag->u2Length = sizeof(*tag);
		tag->u2WifiStream = UNI_STREAM_FROM_RFCR(cmd->u4Address);
		tag->u4Addr = cmd->u4Address;
		tag->u4Value = cmd->u4Data;
	} else {
		struct UNI_CMD_ACCESS_REG_BASIC *tag =
		      (struct UNI_CMD_ACCESS_REG_BASIC *) uni_cmd->aucTlvBuffer;

		tag->u2Tag = UNI_CMD_ACCESS_REG_TAG_BASIC;
		tag->u2Length = sizeof(*tag);
		tag->u4Addr = cmd->u4Address;
		tag->u4Value = cmd->u4Data;
	}

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdUpdateEdcaSet(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_UPDATE_WMM_PARMS *cmd;
	struct UNI_CMD_EDCA *uni_cmd;
	struct UNI_CMD_EDCA_AC_PARM *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_EDCA) +
	     		       sizeof(struct UNI_CMD_EDCA_AC_PARM) * AC_NUM;
	uint8_t i;

	if (info->ucCID != CMD_ID_UPDATE_WMM_PARMS ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_UPDATE_WMM_PARMS *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_EDCA_SET,
					max_cmd_len, NULL, NULL);

	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_EDCA *) entry->pucInfoBuffer;
	uni_cmd->ucBssInfoIdx = cmd->ucBssIndex;
	tag = (struct UNI_CMD_EDCA_AC_PARM *) uni_cmd->aucTlvBuffer;
	for (i = 0; i < AC_NUM; ++i) {
		tag->u2Tag = UNI_CMD_EDCA_TAG_AC_PARM;
		tag->u2Length = sizeof(*tag);
		tag->ucAcIndex = i;
		tag->ucValidBitmap = UNI_CMD_EDCA_ALL_BITS;
		tag->ucCWmin = (uint8_t) ffs(cmd->arACQueParms[i].u2CWmin + 1) - 1;
		tag->ucCWmax = (uint8_t) ffs(cmd->arACQueParms[i].u2CWmax + 1) - 1;
		tag->u2TxopLimit = cmd->arACQueParms[i].u2TxopLimit;
		tag->ucAifsn = (uint8_t) cmd->arACQueParms[i].u2Aifsn;
		tag++;
	}

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdUpdateMuEdca(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
#if (CFG_SUPPORT_802_11AX == 1)
	struct _CMD_MQM_UPDATE_MU_EDCA_PARMS_T *cmd;
	struct UNI_CMD_MQM_UPDATE_MU_EDCA *uni_cmd;
	struct UNI_CMD_UPDATE_MU_EDCA *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_MQM_UPDATE_MU_EDCA) +
	     		       sizeof(struct UNI_CMD_UPDATE_MU_EDCA);
	uint8_t i;

	if (info->ucCID != CMD_ID_MQM_UPDATE_MU_EDCA_PARMS ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct _CMD_MQM_UPDATE_MU_EDCA_PARMS_T *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_MQM_UPDATE_MU_EDCA_PARMS,
			max_cmd_len, NULL, NULL);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_MQM_UPDATE_MU_EDCA *) entry->pucInfoBuffer;
	tag = (struct UNI_CMD_UPDATE_MU_EDCA *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_MQM_UPDATE_MU_EDCA_TAG_PARMS;
	tag->u2Length = sizeof(*tag);
	tag->ucBssIndex = cmd->ucBssIndex;
	tag->fgIsQBSS = cmd->fgIsQBSS;
	tag->ucWmmSet = cmd->ucWmmSet;

	for (i = 0; i < 4; i++) {
		tag->arMUEdcaParams[i].ucECWmin =
			cmd->arMUEdcaParams[i].ucECWmin;
		tag->arMUEdcaParams[i].ucECWmax =
			cmd->arMUEdcaParams[i].ucECWmax;
		tag->arMUEdcaParams[i].ucAifsn =
			cmd->arMUEdcaParams[i].ucAifsn;
		tag->arMUEdcaParams[i].ucIsACMSet =
			cmd->arMUEdcaParams[i].ucIsACMSet;
		tag->arMUEdcaParams[i].ucMUEdcaTimer =
			cmd->arMUEdcaParams[i].ucMUEdcaTimer;
	}

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
#else
	return WLAN_STATUS_NOT_SUPPORTED;
#endif
}

uint32_t nicUniCmdUpdateSrParams(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
#if (CFG_SUPPORT_802_11AX == 1)
	struct _CMD_RLM_UPDATE_SR_PARMS_T *cmd;
	struct UNI_CMD_SR *uni_cmd;
	struct UNI_CMD_SR_UPDATE_SR_PARMS *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_SR) +
	     		       sizeof(struct UNI_CMD_SR_UPDATE_SR_PARMS);

	if (info->ucCID != CMD_ID_RLM_UPDATE_SR_PARAMS ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct _CMD_RLM_UPDATE_SR_PARMS_T *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_SR, max_cmd_len, NULL, NULL);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_SR *) entry->pucInfoBuffer;
	uni_cmd->u1BandIdx = ENUM_BAND_AUTO;
	tag = (struct UNI_CMD_SR_UPDATE_SR_PARMS *) uni_cmd->au1TlvBuffer;
	tag->u2Tag = UNI_CMD_SR_TAG_UPDATE_SR_PARAMS;
	tag->u2Length = sizeof(*tag);
	tag->ucCmdVer = cmd->ucCmdVer;
	tag->u2CmdLen = cmd->u2CmdLen;
	tag->ucBssIndex = cmd->ucBssIndex;
	tag->ucSRControl = cmd->ucSRControl;
	tag->ucNonSRGObssPdMaxOffset = cmd->ucNonSRGObssPdMaxOffset;
	tag->ucSRGObssPdMinOffset = cmd->ucSRGObssPdMinOffset;
	tag->ucSRGObssPdMaxOffset = cmd->ucSRGObssPdMaxOffset;
	tag->u4SRGBSSColorBitmapLow = cmd->u4SRGBSSColorBitmapLow;
	tag->u4SRGBSSColorBitmapHigh = cmd->u4SRGBSSColorBitmapHigh;
	tag->u4SRGPartialBSSIDBitmapLow = cmd->u4SRGPartialBSSIDBitmapLow;
	tag->u4SRGPartialBSSIDBitmapHigh = cmd->u4SRGPartialBSSIDBitmapHigh;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
#else
	return WLAN_STATUS_NOT_SUPPORTED;
#endif
}

uint32_t nicUniCmdOffloadIPV4(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_SET_NETWORK_ADDRESS_LIST *cmd;
	struct UNI_CMD_OFFLOAD *uni_cmd;
	struct UNI_CMD_OFFLOAD_ARPNS_IPV4 *tag;
	struct IPV4_ADDRESS *ipv4;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len;
	uint8_t i;

	if (info->ucCID != CMD_ID_SET_IP_ADDRESS)
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_SET_NETWORK_ADDRESS_LIST *) info->pucInfoBuffer;
	max_cmd_len = sizeof(struct UNI_CMD_OFFLOAD) +
		      sizeof(struct UNI_CMD_OFFLOAD_ARPNS_IPV4) +
		      sizeof(struct IPV4_ADDRESS) * cmd->ucAddressCount;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_OFFLOAD,
		max_cmd_len, nicUniCmdEventSetCommon, nicUniCmdTimeoutCommon);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_OFFLOAD *) entry->pucInfoBuffer;
	uni_cmd->ucBssInfoIdx = cmd->ucBssIndex;
	tag = (struct UNI_CMD_OFFLOAD_ARPNS_IPV4 *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_OFFLOAD_TAG_ARPNS_IPV4;
	tag->u2Length = sizeof(*tag) +
			sizeof(struct IPV4_ADDRESS) * cmd->ucAddressCount;
	tag->ucIpv4AddressCount = cmd->ucAddressCount;
	tag->ucVersion = cmd->ucVersion;
	ipv4 = tag->arIpv4NetAddress;
	for (i = 0; i < cmd->ucAddressCount; i++) {
		kalMemCopy(ipv4, &cmd->arNetAddress[i], sizeof(*ipv4));
		ipv4++;
	}

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdOffloadIPV6(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_IPV6_NETWORK_ADDRESS_LIST *cmd;
	struct UNI_CMD_OFFLOAD *uni_cmd;
	struct UNI_CMD_OFFLOAD_ARPNS_IPV6 *tag;
	struct IPV6_ADDRESS *ipv6;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len;
	uint8_t i;

	if (info->ucCID != CMD_ID_SET_IPV6_ADDRESS)
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_IPV6_NETWORK_ADDRESS_LIST *) info->pucInfoBuffer;
	max_cmd_len = sizeof(struct UNI_CMD_OFFLOAD) +
		      sizeof(struct UNI_CMD_OFFLOAD_ARPNS_IPV6) +
		      sizeof(struct IPV6_ADDRESS) * cmd->ucAddressCount;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_OFFLOAD,
		max_cmd_len, nicUniCmdEventSetCommon, nicUniCmdTimeoutCommon);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_OFFLOAD *) entry->pucInfoBuffer;
	uni_cmd->ucBssInfoIdx = cmd->ucBssIndex;
	tag = (struct UNI_CMD_OFFLOAD_ARPNS_IPV6 *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_OFFLOAD_TAG_ARPNS_IPV6;
	tag->u2Length = sizeof(*tag) +
			sizeof(struct IPV6_ADDRESS) * cmd->ucAddressCount;
	tag->ucIpv6AddressCount = cmd->ucAddressCount;
	ipv6 = tag->arIpv6NetAddress;
	for (i = 0; i < cmd->ucAddressCount; i++) {
		kalMemCopy(ipv6, &cmd->arNetAddress[i], sizeof(*ipv6));
		ipv6++;
	}

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdGetIdcChnl(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_GET_LTE_SAFE_CHN *cmd;
	struct UNI_CMD_IDC *uni_cmd;
	struct UNI_CMD_GET_IDC_CHN *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_IDC) +
	     		       sizeof(struct UNI_CMD_GET_IDC_CHN);

	if (info->ucCID != CMD_ID_GET_LTE_CHN ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_GET_LTE_SAFE_CHN *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_IDC,
		max_cmd_len, nicUniEventQueryIdcChnl, nicUniCmdTimeoutCommon);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_IDC *) entry->pucInfoBuffer;
	tag = (struct UNI_CMD_GET_IDC_CHN *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_IDC_TAG_GET_IDC_CHN;
	tag->u2Length = sizeof(*tag);

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdSetSGParam(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
#if CFG_SUPPORT_SMART_GEAR
	struct CMD_SMART_GEAR_PARAM *cmd;
	struct UNI_CMD_SMART_GEAR *uni_cmd;
	struct UNI_CMD_SMART_GEAR_PARAM *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_SMART_GEAR) +
	     		       sizeof(struct UNI_CMD_SMART_GEAR_PARAM);

	if (info->ucCID != CMD_ID_SG_PARAM ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_SMART_GEAR_PARAM *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_SMART_GEAR,
		max_cmd_len, NULL, NULL);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_SMART_GEAR *) entry->pucInfoBuffer;
	tag = (struct UNI_CMD_SMART_GEAR_PARAM *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_SMART_GEAR_TAG_PARAM;
	tag->u2Length = sizeof(*tag);
	tag->ucSGEnable = cmd->ucSGEnable;
	tag->ucSGSpcCmd = cmd->ucSGSpcCmd;
	tag->ucNSSCap = cmd->ucNSSCap;
	tag->ucSGCfg = cmd->ucSGCfg;
	tag->ucSG24GFavorANT = cmd->ucSG24GFavorANT;
	tag->ucSG5GFavorANT = cmd->ucSG5GFavorANT;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
#else
	return WLAN_STATUS_NOT_SUPPORTED;
#endif
}

uint32_t nicUniCmdSetMonitor(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_MONITOR_SET_INFO *cmd;
	struct UNI_CMD_SNIFFER_MODE *uni_cmd;
	struct UNI_CMD_SNIFFER_MODE_ENABLE *tag0;
	struct UNI_CMD_SNIFFER_MODE_CONFIG *tag1;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len;

	cmd = (struct CMD_MONITOR_SET_INFO *) info->pucInfoBuffer;
	max_cmd_len = sizeof(struct UNI_CMD_SNIFFER_MODE) +
		sizeof(struct UNI_CMD_SNIFFER_MODE_ENABLE) +
		sizeof(struct UNI_CMD_SNIFFER_MODE_CONFIG);

	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_SNIFFER_MODE,
					max_cmd_len, nicUniCmdEventSetCommon,
					nicUniCmdTimeoutCommon);

	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_SNIFFER_MODE *) entry->pucInfoBuffer;
	uni_cmd->ucBandIdx = cmd->ucBandIdx;

	tag0 = (struct UNI_CMD_SNIFFER_MODE_ENABLE *) uni_cmd->aucTlvBuffer;
	tag0->u2Tag = UNI_CMD_SNIFFER_MODE_TAG_ENABLE;
	tag0->u2Length = sizeof(*tag0);
	tag0->ucSNEnable = cmd->ucEnable;

	tag1 = (struct UNI_CMD_SNIFFER_MODE_CONFIG *)
		(uni_cmd->aucTlvBuffer + tag0->u2Length);
	tag1->u2Tag = UNI_CMD_SNIFFER_MODE_TAG_CONFIG;
	tag1->u2Length = sizeof(*tag1);
	tag1->ucBand = cmd->ucBand;
	tag1->ucPriChannel = cmd->ucPriChannel;
	tag1->ucSco = cmd->ucSco;
	tag1->ucChannelWidth = cmd->ucChannelWidth;
	tag1->ucChannelS1 = cmd->ucChannelS1;
	tag1->ucChannelS2 = cmd->ucChannelS2;
	tag1->u2Aid = cmd->u2Aid;
	tag1->fgDropFcsErrorFrame = cmd->fgDropFcsErrorFrame;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);
	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdRoaming(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_ROAMING_TRANSIT *cmd;
	struct UNI_CMD_ROAMING *uni_cmd;
	struct UNI_CMD_ROAMING_TRANSIT_FSM *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_ROAMING) +
	     		       sizeof(struct UNI_CMD_ROAMING_TRANSIT_FSM);

	if (info->ucCID != CMD_ID_ROAMING_TRANSIT ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_ROAMING_TRANSIT *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_ROAMING,
		max_cmd_len, NULL, NULL);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_ROAMING *) entry->pucInfoBuffer;
	uni_cmd->ucBssInfoIdx = cmd->ucBssidx;
	uni_cmd->ucDbdcIdx = ENUM_BAND_AUTO;
	tag = (struct UNI_CMD_ROAMING_TRANSIT_FSM *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_ROAMING_TAG_TRANSIT_FSM;
	tag->u2Length = sizeof(*tag);
	tag->u2Event = cmd->u2Event;
	tag->u2Data = cmd->u2Data;
	tag->eReason = cmd->eReason;
	tag->u4RoamingTriggerTime = cmd->u4RoamingTriggerTime;
	tag->u2RcpiLowThreshold = cmd->u2RcpiLowThreshold;
	tag->ucIsSupport11B = cmd->ucIsSupport11B;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdPerfInd(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_PERF_IND *cmd;
	struct UNI_CMD_PERF_IND *uni_cmd;
	struct UNI_CMD_PERF_IND_PARM *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_PERF_IND) +
	     		       sizeof(struct UNI_CMD_PERF_IND_PARM);

	if (info->ucCID != CMD_ID_PERF_IND ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_PERF_IND *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_PERF_IND,
		max_cmd_len, NULL, NULL);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	// TODO: uni cmd
	uni_cmd = (struct UNI_CMD_PERF_IND *) entry->pucInfoBuffer;
	tag = (struct UNI_CMD_PERF_IND_PARM *) uni_cmd->aucTlvBuffer;
	/* tag->u2Tag = UNI_CMD_PERF_IND_TAG_PARM; */
	/* tag->u2Length = sizeof(*tag); */
	tag->ucCmdVer = cmd->ucCmdVer;
	tag->u2CmdLen = cmd->u2CmdLen;
	tag->u4VaildPeriod = cmd->u4VaildPeriod;

	kalMemCopy(tag->ulCurTxBytes, cmd->ulCurTxBytes,
				sizeof(tag->ulCurTxBytes));
	kalMemCopy(tag->ulCurRxBytes, cmd->ulCurRxBytes,
				sizeof(tag->ulCurRxBytes));
	kalMemCopy(tag->u2CurRxRate, cmd->u2CurRxRate,
				sizeof(tag->u2CurRxRate));
	kalMemCopy(tag->ucCurRxRCPI0, cmd->ucCurRxRCPI0,
				sizeof(tag->ucCurRxRCPI0));
	kalMemCopy(tag->ucCurRxRCPI1, cmd->ucCurRxRCPI1,
				sizeof(tag->ucCurRxRCPI1));
	kalMemCopy(tag->ucCurRxNss, cmd->ucCurRxNss,
				sizeof(tag->ucCurRxNss));

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdInstallKey(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_802_11_KEY *cmd;
	struct UNI_CMD_STAREC *uni_cmd;
	struct UNI_CMD_STAREC_INSTALL_KEY3 *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_STAREC) +
	     		       sizeof(struct UNI_CMD_STAREC_INSTALL_KEY3);

	if (info->ucCID != CMD_ID_ADD_REMOVE_KEY ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_802_11_KEY *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_STAREC_INFO,
		max_cmd_len, cmd->ucAddRemove ? nicUniCmdEventInstallKey :
		nicUniCmdEventSetCommon, nicUniCmdTimeoutCommon);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_STAREC *) entry->pucInfoBuffer;
	uni_cmd->ucBssInfoIdx = cmd->ucBssIdx;
	WCID_SET_H_L(uni_cmd->ucWlanIdxHnVer, uni_cmd->ucWlanIdxL,
		cmd->ucWlanIndex);
	tag = (struct UNI_CMD_STAREC_INSTALL_KEY3 *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_STAREC_TAG_INSTALL_KEY_V3;
	tag->u2Length = sizeof(*tag);
	tag->ucAddRemove = cmd->ucAddRemove;
	tag->ucTxKey = cmd->ucTxKey;
	tag->ucKeyType = cmd->ucKeyType;
	tag->ucIsAuthenticator = cmd->ucIsAuthenticator;
	COPY_MAC_ADDR(tag->aucPeerAddr, cmd->aucPeerAddr);
	tag->ucBssIdx = cmd->ucBssIdx;
	tag->ucAlgorithmId = cmd->ucAlgorithmId;
	tag->ucKeyId = cmd->ucKeyId;
	tag->ucKeyLen = cmd->ucKeyLen;
	tag->ucWlanIndex = cmd->ucWlanIndex;
	tag->ucMgmtProtection = cmd->ucMgmtProtection;
	kalMemCopy(tag->aucKeyMaterial, cmd->aucKeyMaterial,
		sizeof(tag->aucKeyMaterial));
	kalMemCopy(tag->aucKeyRsc, cmd->aucKeyRsc, sizeof(tag->aucKeyRsc));

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdInstallDefaultKey(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_DEFAULT_KEY *cmd;
	struct UNI_CMD_STAREC *uni_cmd;
	struct UNI_CMD_STAREC_DEFAULT_KEY *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_STAREC) +
	     		       sizeof(struct UNI_CMD_STAREC_DEFAULT_KEY);

	if (info->ucCID != CMD_ID_DEFAULT_KEY_ID ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_DEFAULT_KEY *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_STAREC_INFO,
		max_cmd_len, nicUniCmdEventSetCommon, nicUniCmdTimeoutCommon);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_STAREC *) entry->pucInfoBuffer;
	uni_cmd->ucBssInfoIdx = cmd->ucBssIdx;
	WCID_SET_H_L(uni_cmd->ucWlanIdxHnVer, uni_cmd->ucWlanIdxL,
		cmd->ucWlanIndex);
	tag = (struct UNI_CMD_STAREC_DEFAULT_KEY *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_STAREC_TAG_INSTALL_DEFAULT_KEY;
	tag->u2Length = sizeof(*tag);
	tag->ucKeyId = cmd->ucKeyId;
	tag->ucMulticast = cmd->ucMulticast;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdOffloadKey(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct PARAM_GTK_REKEY_DATA *cmd;
	struct UNI_CMD_OFFLOAD *uni_cmd;
	struct UNI_CMD_OFFLOAD_GTK_REKEY *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_OFFLOAD) +
	     		       sizeof(struct UNI_CMD_OFFLOAD_GTK_REKEY);

	if (info->ucCID != CMD_ID_SET_GTK_REKEY_DATA ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct PARAM_GTK_REKEY_DATA *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_OFFLOAD,
		max_cmd_len, nicUniCmdEventSetCommon, nicUniCmdTimeoutCommon);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_OFFLOAD *) entry->pucInfoBuffer;
	uni_cmd->ucBssInfoIdx = cmd->ucBssIndex;
	tag = (struct UNI_CMD_OFFLOAD_GTK_REKEY *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_OFFLOAD_TAG_GTK_REKEY;
	tag->u2Length = sizeof(*tag);
	kalMemCopy(tag->aucKek, cmd->aucKek, sizeof(tag->aucKek));
	kalMemCopy(tag->aucReplayCtr,
		cmd->aucReplayCtr, sizeof(tag->aucReplayCtr));
	tag->ucRekeyMode = 0;
	tag->ucCurKeyId = 0;
	tag->ucOption = 0;
	tag->u4Proto = cmd->u4Proto;
	tag->u4PairwiseCipher = cmd->u4PairwiseCipher;
	tag->u4GroupCipher = cmd->u4GroupCipher;
	tag->u4KeyMgmt = cmd->u4KeyMgmt;
	tag->u4MgmtGroupCipher = cmd->u4MgmtGroupCipher;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdHifCtrl(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_HIF_CTRL *cmd;
	struct UNI_CMD_HIF_CTRL *uni_cmd;
	struct UNI_CMD_HIF_CTRL_BASIC *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_HIF_CTRL) +
	     		       sizeof(struct UNI_CMD_HIF_CTRL_BASIC);

	if (info->ucCID != CMD_ID_HIF_CTRL ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_HIF_CTRL *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_HIF_CTRL,
		max_cmd_len, NULL, NULL);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_HIF_CTRL *) entry->pucInfoBuffer;
	uni_cmd->ucHifType = cmd->ucHifType;
	tag = (struct UNI_CMD_HIF_CTRL_BASIC *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_HIF_CTRL_TAG_BASIC;
	tag->u2Length = sizeof(*tag);
	tag->ucHifSuspend = cmd->ucHifSuspend;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdRddOnOffCtrl(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
#if (CFG_SUPPORT_DFS_MASTER == 1)
	struct CMD_RDD_ON_OFF_CTRL *cmd;
	struct UNI_CMD_RDD *uni_cmd;
	struct UNI_CMD_RDD_ON_OFF_CTRL_PARM *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_RDD) +
			       sizeof(struct UNI_CMD_RDD_ON_OFF_CTRL_PARM);

	if (info->ucCID != CMD_ID_RDD_ON_OFF_CTRL ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_RDD_ON_OFF_CTRL *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_RDD_ON_OFF_CTRL,
		max_cmd_len, nicUniCmdEventSetCommon, nicUniCmdTimeoutCommon);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_RDD *) entry->pucInfoBuffer;
	tag = (struct UNI_CMD_RDD_ON_OFF_CTRL_PARM *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_RDD_TAG_ON_OFF_CTRL_PARM;
	tag->u2Length = sizeof(*tag);
	tag->u1DfsCtrl = cmd->ucDfsCtrl;
	tag->u1RddIdx = cmd->ucRddIdx;
	tag->u1RddRxSel = cmd->ucRddRxSel;
	tag->u1SetVal = cmd->ucSetVal;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
#else
	return WLAN_STATUS_NOT_SUPPORTED;
#endif
}

uint32_t nicUniCmdTdls(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
#if (CFG_SUPPORT_TDLS == 1)
	struct CMD_TDLS_CH_SW *cmd;
	struct UNI_CMD_TDLS *uni_cmd;
	struct UNI_CMD_SET_TDLS_CH_SW *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_TDLS) +
	     		       sizeof(struct UNI_CMD_SET_TDLS_CH_SW);

	if (info->ucCID != CMD_ID_SET_TDLS_CH_SW ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_TDLS_CH_SW *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_TDLS,
		max_cmd_len, NULL, NULL);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_TDLS *) entry->pucInfoBuffer;
	tag = (struct UNI_CMD_SET_TDLS_CH_SW *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_TDLS_TAG_SET_TDLS_CH_SW;
	tag->u2Length = sizeof(*tag);
	tag->fgIsTDLSChSwProhibit = cmd->fgIsTDLSChSwProhibit;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
#else
	return WLAN_STATUS_NOT_SUPPORTED;
#endif
}

uint32_t nicUniCmdSetP2pNoa(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_CUSTOM_NOA_PARAM_STRUCT *cmd;
	struct UNI_CMD_P2P *uni_cmd;
	struct UNI_CMD_SET_NOA_PARAM *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_P2P) +
			       sizeof(struct UNI_CMD_SET_NOA_PARAM);

	if (info->ucCID != CMD_ID_SET_NOA_PARAM ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_CUSTOM_NOA_PARAM_STRUCT *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_P2P,
		max_cmd_len, nicUniCmdEventSetCommon, nicUniCmdTimeoutCommon);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_P2P *) entry->pucInfoBuffer;
	tag = (struct UNI_CMD_SET_NOA_PARAM *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_P2P_TAG_SET_NOA_PARAM;
	tag->u2Length = sizeof(*tag);
	tag->u4NoaDurationMs = cmd->u4NoaDurationMs;
	tag->u4NoaIntervalMs = cmd->u4NoaIntervalMs;
	tag->u4NoaCount = cmd->u4NoaCount;
	tag->ucBssIdx = cmd->ucBssIdx;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdSetP2pOppps(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_CUSTOM_OPPPS_PARAM_STRUCT *cmd;
	struct UNI_CMD_P2P *uni_cmd;
	struct UNI_CMD_SET_OPPPS_PARAM *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_P2P) +
			       sizeof(struct UNI_CMD_SET_OPPPS_PARAM);

	if (info->ucCID != CMD_ID_SET_OPPPS_PARAM ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_CUSTOM_OPPPS_PARAM_STRUCT *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_P2P,
		max_cmd_len, nicUniCmdEventSetCommon, nicUniCmdTimeoutCommon);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_P2P *) entry->pucInfoBuffer;
	tag = (struct UNI_CMD_SET_OPPPS_PARAM *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_P2P_TAG_SET_OPPPS_PARAM;
	tag->u2Length = sizeof(*tag);
	tag->u4CTwindowMs = cmd->u4CTwindowMs;
	tag->ucBssIdx = cmd->ucBssIdx;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdSetApConstraintPwrLimit(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
#if CFG_SUPPORT_802_11K
	struct CMD_SET_AP_CONSTRAINT_PWR_LIMIT *cmd;
	struct UNI_CMD_RRM_11K *uni_cmd;
	struct UNI_CMD_SET_AP_CONSTRAINT_PWR_LIMIT_PARAM *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_RRM_11K) +
		sizeof(struct UNI_CMD_SET_AP_CONSTRAINT_PWR_LIMIT_PARAM);

	if (info->ucCID != CMD_ID_SET_AP_CONSTRAINT_PWR_LIMIT ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_SET_AP_CONSTRAINT_PWR_LIMIT *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_RRM_11K,
		max_cmd_len, nicUniCmdEventSetCommon, nicUniCmdTimeoutCommon);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_RRM_11K *) entry->pucInfoBuffer;
	tag = (struct UNI_CMD_SET_AP_CONSTRAINT_PWR_LIMIT_PARAM *)
		uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_SET_AP_CONSTRAINT_PWR_LIMIT;
	tag->u2Length = sizeof(*tag);
	tag->ucPwrSetEnable = cmd->ucPwrSetEnable;
	tag->cMaxTxPwr = cmd->cMaxTxPwr;
	tag->cMinTxPwr = cmd->cMinTxPwr;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
#else
	return WLAN_STATUS_NOT_SUPPORTED;
#endif
}

uint32_t nicUniCmdSetRrmCapability(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
#if CFG_SUPPORT_802_11K
	struct CMD_SET_RRM_CAPABILITY *cmd;
	struct UNI_CMD_RRM_11K *uni_cmd;
	struct UNI_CMD_SET_RRM_CAPABILITY_PARAM *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_RRM_11K) +
		sizeof(struct UNI_CMD_SET_RRM_CAPABILITY_PARAM);

	if (info->ucCID != CMD_ID_SET_RRM_CAPABILITY ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_SET_RRM_CAPABILITY *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_RRM_11K,
		max_cmd_len, NULL, nicUniCmdTimeoutCommon);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_RRM_11K *) entry->pucInfoBuffer;
	tag = (struct UNI_CMD_SET_RRM_CAPABILITY_PARAM *)
		uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_SET_RRM_CAPABILITY;
	tag->u2Length = sizeof(*tag);
	tag->ucRrmEnable = cmd->ucRrmEnable;
	kalMemCopy(tag->ucCapabilities, cmd->ucCapabilities,
		sizeof(tag->ucCapabilities));

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
#else
	return WLAN_STATUS_NOT_SUPPORTED;
#endif

}

uint32_t nicUniCmdSetCountryPwrLimit(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_SET_COUNTRY_CHANNEL_POWER_LIMIT_V2 *cmd;
	struct UNI_CMD_POWER_LIMIT *uni_cmd;
	struct UNI_CMD_SET_PWR_LIMIT_PARAM *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_POWER_LIMIT) +
		sizeof(struct UNI_CMD_SET_PWR_LIMIT_PARAM);

	if (info->ucCID != CMD_ID_SET_COUNTRY_POWER_LIMIT ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_SET_COUNTRY_CHANNEL_POWER_LIMIT_V2 *)
		info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_POWER_LIMIT,
		max_cmd_len, NULL, NULL);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_POWER_LIMIT *) entry->pucInfoBuffer;

	tag = (struct UNI_CMD_SET_PWR_LIMIT_PARAM *)
		uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_POWER_LIMIT_TABLE_CTRL;
	tag->u2Length = sizeof(*tag);
	kalMemCopy(&tag->config, cmd, sizeof(tag->config));

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdSetCountryPwrLimitPerRate(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_SET_TXPOWER_COUNTRY_TX_POWER_LIMIT_PER_RATE *cmd;
	struct UNI_CMD_POWER_LIMIT *uni_cmd;
	struct UNI_CMD_SET_PWR_LIMIT_PER_RATE_TABLE_PARAM *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_POWER_LIMIT) +
		sizeof(struct UNI_CMD_SET_PWR_LIMIT_PER_RATE_TABLE_PARAM);

	if (info->ucCID != CMD_ID_SET_COUNTRY_POWER_LIMIT_PER_RATE ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_SET_TXPOWER_COUNTRY_TX_POWER_LIMIT_PER_RATE *)
		info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_POWER_LIMIT,
		max_cmd_len, NULL, NULL);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_POWER_LIMIT *) entry->pucInfoBuffer;

	tag = (struct UNI_CMD_SET_PWR_LIMIT_PER_RATE_TABLE_PARAM *)
		uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_POWER_LIMIT_PER_RATE_TABLE;
	tag->u2Length = sizeof(*tag);
	kalMemCopy(&tag->config, cmd, sizeof(tag->config));

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;

}

uint32_t nicUniCmdNvramFragmentHandler(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_NVRAM_FRAGMENT *cmd;
	struct UNI_CMD_NVRAM_SETTINGS *uni_cmd;
	struct UNI_CMD_NVRAM_SETTINGS_FRAGMENT_PARAM *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_NVRAM_SETTINGS) +
		sizeof(struct UNI_CMD_NVRAM_SETTINGS_FRAGMENT_PARAM);

	if (info->ucCID != CMD_ID_SET_NVRAM_SETTINGS ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_NVRAM_FRAGMENT *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_NVRAM_SETTINGS,
		max_cmd_len, NULL, NULL);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_NVRAM_SETTINGS *) entry->pucInfoBuffer;
	tag = (struct UNI_CMD_NVRAM_SETTINGS_FRAGMENT_PARAM *)
		uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_NVRAM_SETTINGS_FRAGMENT;
	tag->u2Length = sizeof(*tag);
	kalMemCopy(&tag->config, cmd, sizeof(tag->config));

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdNvramLegacyHandler(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_NVRAM_SETTING *cmd;
	struct UNI_CMD_NVRAM_SETTINGS *uni_cmd;
	struct UNI_CMD_NVRAM_SETTINGS_LEGACY_PARAM *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_NVRAM_SETTINGS) +
		sizeof(struct UNI_CMD_NVRAM_SETTINGS_LEGACY_PARAM);

	if (info->ucCID != CMD_ID_SET_NVRAM_SETTINGS ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_NVRAM_SETTING *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_NVRAM_SETTINGS,
		max_cmd_len, NULL, NULL);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_NVRAM_SETTINGS *) entry->pucInfoBuffer;
	tag = (struct UNI_CMD_NVRAM_SETTINGS_LEGACY_PARAM *)
		uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_NVRAM_SETTINGS_LEGACY;
	tag->u2Length = sizeof(*tag);
	kalMemCopy(&tag->config, cmd, sizeof(tag->config));

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdSetNvramSettings(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	uint8_t fgIsFragCmd = !!(ad->chip_info->is_support_nvram_fragment);

	DBGLOG(RX, INFO, "fgIsFragCmd[%d]", fgIsFragCmd);

	if (fgIsFragCmd)
		return nicUniCmdNvramFragmentHandler(ad, info);
	else
		return nicUniCmdNvramLegacyHandler(ad, info);
}

uint32_t nicUniCmdSetTxAmpdu(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_TX_AMPDU *cmd;
	struct UNI_CMD_BA_OFFLOAD *uni_cmd;
	struct UNI_CMD_TX_AMPDU_PARAM *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_BA_OFFLOAD) +
		sizeof(struct UNI_CMD_TX_AMPDU_PARAM);

	if (info->ucCID != CMD_ID_TX_AMPDU ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_TX_AMPDU *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_BA_OFFLOAD,
		max_cmd_len, NULL, NULL);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_BA_OFFLOAD *) entry->pucInfoBuffer;
	tag = (struct UNI_CMD_TX_AMPDU_PARAM *)
		uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_BA_OFFLOAD_TAG_TX_AMPDU;
	tag->u2Length = sizeof(*tag);
	tag->fgEnable = cmd->fgEnable;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdSetRxAmpdu(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_ADDBA_REJECT *cmd;
	struct UNI_CMD_BA_OFFLOAD *uni_cmd;
	struct UNI_CMD_RX_AMPDU_PARAM *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_BA_OFFLOAD) +
		sizeof(struct UNI_CMD_RX_AMPDU_PARAM);

	if (info->ucCID != CMD_ID_ADDBA_REJECT ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_ADDBA_REJECT *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_BA_OFFLOAD,
		max_cmd_len, NULL, NULL);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_BA_OFFLOAD *) entry->pucInfoBuffer;
	tag = (struct UNI_CMD_RX_AMPDU_PARAM *)
		uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_BA_OFFLOAD_TAG_RX_AMPDU;
	tag->u2Length = sizeof(*tag);
	/* Legacy ADDBA reject true equal to UNI CMD RX AMPDU false */
	tag->fgEnable = !cmd->fgEnable;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}


uint32_t nicUniCmdSetMultiAddr(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_MAC_MCAST_ADDR *cmd;
	struct UNI_CMD_MUAR *uni_cmd;
	struct UNI_CMD_MUAR_CLEAN_PARAM *clean_tag;
	struct UNI_CMD_MUAR_ENTRY_PARAM *config_tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_MUAR) +
		sizeof(struct UNI_CMD_MUAR_CLEAN_PARAM);
	uint8_t ucIdx = 0;

	if (info->ucCID != CMD_ID_MAC_MCAST_ADDR ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_MAC_MCAST_ADDR *) info->pucInfoBuffer;
	max_cmd_len +=
		sizeof(struct UNI_CMD_MUAR_ENTRY_PARAM) * cmd->u4NumOfGroupAddr;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_NORM_MUAR,
		max_cmd_len, nicUniCmdEventSetCommon, nicUniCmdTimeoutCommon);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_MUAR *) entry->pucInfoBuffer;
	uni_cmd->ucBand = ENUM_BAND_AUTO;

	DBGLOG(RX, INFO, "clean_tag[%p]", uni_cmd->aucTlvBuffer);

	/* Fill-in clean tag*/
	clean_tag = (struct UNI_CMD_MUAR_CLEAN_PARAM *) uni_cmd->aucTlvBuffer;
	clean_tag->u2Tag = UNI_CMD_MUAR_TAG_CLEAN;
	clean_tag->u2Length = sizeof(*clean_tag);
	clean_tag->ucHwBssIndex = cmd->ucBssIndex;
	clean_tag++;

	DBGLOG(RX, INFO, "Number of address[%d] config_tag[%p]",
		cmd->u4NumOfGroupAddr, clean_tag);

	/* Fill-in config tag*/
	config_tag = (struct UNI_CMD_MUAR_ENTRY_PARAM *) clean_tag;
	for (ucIdx = 0; ucIdx < cmd->u4NumOfGroupAddr; ucIdx++, config_tag++) {
		config_tag->u2Tag = UNI_CMD_MUAR_TAG_ENTRY;
		config_tag->u2Length = sizeof(*config_tag);
		config_tag->fgSmesh = FALSE;
		config_tag->ucHwBssIndex = cmd->ucBssIndex;
		config_tag->ucMuarIdx = 0xFF;
		COPY_MAC_ADDR(config_tag->aucMacAddr, cmd->arAddress[ucIdx]);
	}
	DBGLOG_MEM8(NIC, INFO, uni_cmd->aucTlvBuffer,
		(sizeof(struct UNI_CMD_MUAR_CLEAN_PARAM) +
		sizeof(struct UNI_CMD_MUAR_ENTRY_PARAM) * cmd->u4NumOfGroupAddr));

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdSetRssiMonitor(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct PARAM_RSSI_MONITOR_T *cmd;
	struct UNI_CMD_RSSI_MONITOR *uni_cmd;
	struct UNI_CMD_RSSI_MONITOR_SET *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_RSSI_MONITOR) +
		sizeof(struct UNI_CMD_RSSI_MONITOR_SET);

	if (info->ucCID != CMD_ID_RSSI_MONITOR ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct PARAM_RSSI_MONITOR_T *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_RSSI_MONITOR,
		max_cmd_len, nicUniCmdEventSetCommon, nicUniCmdTimeoutCommon);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_RSSI_MONITOR *) entry->pucInfoBuffer;
	tag = (struct UNI_CMD_RSSI_MONITOR_SET *)
		uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_RSSI_MONITOR_TAG_SET;
	tag->u2Length = sizeof(*tag);
	tag->fgEnable = cmd->enable;
	tag->cMaxRssi = cmd->max_rssi_value;
	tag->cMinRssi = cmd->min_rssi_value;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;

}

uint32_t nicUniCmdSetIcsSniffer(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_ICS_SNIFFER_INFO *cmd;
	struct UNI_CMD_ICS *uni_cmd;
	struct UNI_CMD_ICS_SNIFFER *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_ICS) +
		sizeof(struct UNI_CMD_ICS_SNIFFER);

	if (info->ucCID != CMD_ID_SET_ICS_SNIFFER ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_ICS_SNIFFER_INFO *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_ICS,
		max_cmd_len, nicUniCmdEventSetCommon, nicUniCmdTimeoutCommon);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_ICS *) entry->pucInfoBuffer;
	tag = (struct UNI_CMD_ICS_SNIFFER *)
		uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_ICS_TAG_CTRL;
	tag->u2Length = sizeof(*tag);
	tag->ucAction = cmd->ucAction;
	tag->ucModule = cmd->ucModule;
	tag->ucFilter = cmd->ucFilter;
	tag->ucOperation = cmd->ucOperation;
	kalMemCopy(&tag->ucCondition, cmd, sizeof(tag->ucCondition));

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdGetStaStatistics(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_GET_STA_STATISTICS *cmd;
	struct UNI_CMD_GET_STATISTICS *uni_cmd;
	struct UNI_CMD_STA_STATISTICS *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_GET_STATISTICS) +
			       sizeof(struct UNI_CMD_STA_STATISTICS);

	if (info->ucCID != CMD_ID_GET_STA_STATISTICS ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_GET_STA_STATISTICS *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_GET_STATISTICS,
		max_cmd_len, nicUniEventStaStatistics, nicUniCmdTimeoutCommon);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_GET_STATISTICS *) entry->pucInfoBuffer;
	tag = (struct UNI_CMD_STA_STATISTICS *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_GET_STATISTICS_TAG_STA;
	tag->u2Length = sizeof(*tag);
	tag->u1Index = secGetWlanIdxByStaIdx(ad, cmd->ucIndex);
	tag->ucReadClear = cmd->ucReadClear;
	tag->ucLlsReadClear = cmd->ucLlsReadClear;
	tag->ucResetCounter = cmd->ucResetCounter;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdGetStatistics(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct UNI_CMD_GET_STATISTICS *uni_cmd;
	struct UNI_CMD_BASIC_STATISTICS *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_GET_STATISTICS) +
			       sizeof(struct UNI_CMD_BASIC_STATISTICS);

	if (info->ucCID != CMD_ID_GET_STATISTICS)
		return WLAN_STATUS_NOT_ACCEPTED;

	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_GET_STATISTICS,
		max_cmd_len, nicUniEventStatistics, nicUniCmdTimeoutCommon);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_GET_STATISTICS *) entry->pucInfoBuffer;
	tag = (struct UNI_CMD_BASIC_STATISTICS *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_GET_STATISTICS_TAG_BASIC;
	tag->u2Length = sizeof(*tag);

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdGetLinkQuality(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct UNI_CMD_GET_STATISTICS *uni_cmd;
	struct UNI_CMD_LINK_QUALITY *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_GET_STATISTICS) +
	     		       sizeof(struct UNI_CMD_LINK_QUALITY);

	if (info->ucCID != CMD_ID_GET_LINK_QUALITY)
		return WLAN_STATUS_NOT_ACCEPTED;

	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_GET_STATISTICS,
		max_cmd_len, nicUniEventLinkQuality, nicUniCmdTimeoutCommon);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_GET_STATISTICS *) entry->pucInfoBuffer;
	tag = (struct UNI_CMD_LINK_QUALITY *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_GET_STATISTICS_TAG_LINK_QUALITY;
	tag->u2Length = sizeof(*tag);

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdTestmodeCtrl(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_TEST_CTRL *cmd;
	struct UNI_CMD_TESTMODE_CTRL *uni_cmd;
	struct UNI_CMD_TESTMODE_RF_CTRL *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_TESTMODE_CTRL) +
	     		       sizeof(struct UNI_CMD_TESTMODE_RF_CTRL);

	if (info->ucCID != CMD_ID_TEST_CTRL)
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_TEST_CTRL *) info->pucInfoBuffer;

	switch (cmd->ucAction) {
		case CMD_TEST_CTRL_ACT_SWITCH_MODE:
			if(cmd->u.u4OpMode == CMD_TEST_CTRL_ACT_SWITCH_MODE_NORMAL) {
				entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_TESTMODE_CTRL,
						max_cmd_len, nicCmdEventLeaveRfTest,
						nicUniCmdTimeoutCommon);

			} else {
				/* CMD_TEST_CTRL_ACT_SWITCH_MODE_RF_TEST */
				/* CMD_TEST_CTRL_ACT_SWITCH_MODE_ICAP */
				entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_TESTMODE_CTRL,
						max_cmd_len, nicCmdEventEnterRfTest,
						nicOidCmdEnterRFTestTimeout);
			}
			break;

		case CMD_TEST_CTRL_ACT_SET_AT:
			/* convert for unify cmd */
			cmd->ucAction = CMD_TEST_CTRL_ACT_SET_AT_ENG;
			entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_TESTMODE_CTRL,
					max_cmd_len, nicUniCmdEventSetCommon,
					nicUniCmdTimeoutCommon);
			break;

		case CMD_TEST_CTRL_ACT_GET_AT:
			/* convert for unify cmd */
			cmd->ucAction = CMD_TEST_CTRL_ACT_GET_AT_ENG;
			entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_TESTMODE_CTRL,
					max_cmd_len, nicUniEventQueryRfTestATInfo,
					nicUniCmdTimeoutCommon);
		break;

		default:
			return WLAN_STATUS_NOT_ACCEPTED;
	}

	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_TESTMODE_CTRL *) entry->pucInfoBuffer;
	tag = (struct UNI_CMD_TESTMODE_RF_CTRL *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_TESTMODE_TAG_RF_CTRL;
	tag->u2Length = sizeof(*tag);
	tag->ucAction = cmd->ucAction;
	kalMemCopy(&tag->u, &cmd->u, sizeof(tag->u));

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniExtCmdTestmodeCtrl(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_TEST_CTRL_EXT_T *cmd;
	struct UNI_CMD_TESTMODE_CTRL *uni_cmd;
	struct UNI_CMD_TESTMODE_RF_CTRL *tag;
	struct WIFI_UNI_CMD_ENTRY *entry = NULL;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_TESTMODE_CTRL) +
							sizeof(struct UNI_CMD_TESTMODE_RF_CTRL);

	DBGLOG(NIC, INFO, "nicUniExtCmdTestmodeCtrl\n");

	if (info->ucCID != CMD_ID_LAYER_0_EXT_MAGIC_NUM ||
		info->ucExtCID != EXT_CMD_ID_RF_TEST)
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_TEST_CTRL_EXT_T *) info->pucInfoBuffer;

	switch (cmd->ucAction) {
		case ACTION_IN_RFTEST:
			if (cmd->u.rRfATInfo.u4FuncIndex == SET_ICAP_CAPTURE_START) {
				entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_TESTMODE_CTRL,
						max_cmd_len, nicUniCmdEventSetCommon,
						nicUniCmdTimeoutCommon);
			} else if (cmd->u.rRfATInfo.u4FuncIndex == GET_ICAP_CAPTURE_STATUS) {
				entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_TESTMODE_CTRL,
						max_cmd_len, nicUniEventRfTestHandler,
						nicUniCmdTimeoutCommon);
			} else if (cmd->u.rRfATInfo.u4FuncIndex == GET_ICAP_RAW_DATA) {
				entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_TESTMODE_CTRL,
						max_cmd_len, nicUniEventRfTestHandler,
						nicUniCmdTimeoutCommon);
			} else {
				DBGLOG(NIC, WARN, "Unknown funcIdx rf test cmd = %d\n",
					cmd->u.rRfATInfo);
			}
			break;

		default:
			return WLAN_STATUS_NOT_ACCEPTED;
	}

	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_TESTMODE_CTRL *) entry->pucInfoBuffer;
	tag = (struct UNI_CMD_TESTMODE_RF_CTRL *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_TESTMODE_TAG_RF_CTRL;
	tag->u2Length = sizeof(*tag);
	tag->ucAction = cmd->ucAction;
	kalMemCopy(&tag->u, &cmd->u, sizeof(tag->u));

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdTestmodeRxStat(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_ACCESS_RX_STAT *cmd;
	struct UNI_CMD_TESTMODE_RX_STAT *uni_cmd;
	struct UNI_CMD_TESTMODE_RX_GET_STAT_ALL *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;

	uint32_t max_cmd_len = sizeof(struct UNI_CMD_TESTMODE_RX_STAT) +
	     		       sizeof(struct UNI_CMD_TESTMODE_RX_GET_STAT_ALL);

	if (info->ucCID != CMD_ID_ACCESS_RX_STAT)
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_ACCESS_RX_STAT *) info->pucInfoBuffer;

	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_TESTMODE_RX_STAT,
				max_cmd_len, nicUniEventQueryRxStatAll,
				nicUniCmdTimeoutCommon);

	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_TESTMODE_RX_STAT *)entry->pucInfoBuffer;
	tag = (struct UNI_CMD_TESTMODE_RX_GET_STAT_ALL *)uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_TESTMODE_RX_TAG_GET_STAT_ALL;
	tag->u2Length = sizeof(*tag);
	tag->u1DbdcIdx = cmd->ucDbdcIdx;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdGetBugReport(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct UNI_CMD_GET_STATISTICS *uni_cmd;
	struct UNI_CMD_BUG_REPORT *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_GET_STATISTICS) +
			       sizeof(struct UNI_CMD_BUG_REPORT);

	if (info->ucCID != CMD_ID_GET_BUG_REPORT)
		return WLAN_STATUS_NOT_ACCEPTED;

	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_GET_STATISTICS,
		max_cmd_len, nicUniEventBugReport, nicUniCmdTimeoutCommon);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_GET_STATISTICS *) entry->pucInfoBuffer;
	tag = (struct UNI_CMD_BUG_REPORT *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_GET_STATISTICS_TAG_BUG_REPORT;
	tag->u2Length = sizeof(*tag);

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdTxPowerCtrl(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct UNI_CMD_TXPOWER_CONFIG *uni_cmd;
	struct TAG_HDR *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint8_t tag_id = *info->pucInfoBuffer;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_TXPOWER_CONFIG) +
			       sizeof(struct TAG_HDR) +
			       info->u4SetQueryInfoLen;

	if (info->ucCID != CMD_ID_LAYER_0_EXT_MAGIC_NUM ||
	   (tag_id != TX_POWER_SHOW_INFO && tag_id != TX_RATE_POWER_CTRL))
		return WLAN_STATUS_NOT_ACCEPTED;

	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_TXPOWER, max_cmd_len,
		tag_id == TX_POWER_SHOW_INFO ? nicUniEventTxPowerInfo :
		nicUniCmdEventSetCommon, nicUniCmdTimeoutCommon);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_TXPOWER_CONFIG *) entry->pucInfoBuffer;
	tag = (struct TAG_HDR *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = tag_id;
	tag->u2Length = sizeof(*tag) + info->u4SetQueryInfoLen;
	kalMemCopy(tag->aucBuffer, info->pucInfoBuffer,
		info->u4SetQueryInfoLen);

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdThermalProtect(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
#if (CFG_SUPPORT_POWER_THROTTLING == 1)
	struct UNI_CMD_THERMAL *uni_cmd;
	struct TAG_HDR *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint8_t tag_id = *info->pucInfoBuffer;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_THERMAL) +
			       sizeof(struct TAG_HDR) +
			       info->u4SetQueryInfoLen;

	if (info->ucCID != CMD_ID_LAYER_0_EXT_MAGIC_NUM)
		return WLAN_STATUS_NOT_ACCEPTED;

	switch (tag_id) {
	case THERMAL_PROTECT_ENABLE:
		tag_id = UNI_CMD_THERMAL_TAG_PROTECT_ENABLE;
		break;
	case THERMAL_PROTECT_DISABLE:
		tag_id = UNI_CMD_THERMAL_TAG_PROTECT_DISABLE;
		break;
	case THERMAL_PROTECT_DUTY_CONFIG:
		tag_id = UNI_CMD_THERMAL_TAG_PROTECT_DUTY_CONFIG;
		break;
	case THERMAL_PROTECT_STATE_ACT:
		tag_id = UNI_CMD_THERMAL_TAG_PROTECT_STATE_ACT;
		break;
	default:
		return WLAN_STATUS_NOT_ACCEPTED;
	}

	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_THERMAL, max_cmd_len,
		nicUniCmdEventSetCommon, nicUniCmdTimeoutCommon);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_THERMAL *) entry->pucInfoBuffer;
	tag = (struct TAG_HDR *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = tag_id;
	tag->u2Length = sizeof(*tag) + info->u4SetQueryInfoLen;
	kalMemCopy(tag->aucBuffer, info->pucInfoBuffer,
		info->u4SetQueryInfoLen);

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
#else
	return WLAN_STATUS_NOT_SUPPORTED;
#endif
}

/*******************************************************************************
 *                                 Event
 *******************************************************************************
 */

void nicUniEventHelper(struct ADAPTER *ad, struct WIFI_UNI_EVENT *evt,
	uint8_t eid, uint8_t ext_id, uint8_t *data, uint32_t len)
{
	struct WIFI_EVENT *legacy;
	uint32_t size = sizeof(*legacy) + len;
	uint8_t i;

	legacy = (struct WIFI_EVENT *) kalMemAlloc(size, VIR_MEM_TYPE);
	legacy->u2PacketLength = size;
	legacy->u2PacketType = evt->u2PacketType;
	legacy->ucEID = eid;
	legacy->ucSeqNum = evt->ucSeqNum;
	legacy->ucEventVersion = evt->ucOption;
	legacy->ucExtenEID = ext_id;
	legacy->ucS2DIndex = evt->ucS2DIndex;

	if (data && len)
		kalMemCopy(legacy->aucBuffer, data, len);

	for (i = 0; i < arEventTableSize; i++) {
		if (eid == arEventTable[i].eEID) {
			arEventTable[i].pfnHandler(ad, legacy);
			break;
		}
	}

	kalMemFree(legacy, VIR_MEM_TYPE, size);
}

/*******************************************************************************
 *                   Solicited Event
 *******************************************************************************
 */

void nicRxProcessUniEventPacket(IN struct ADAPTER *prAdapter,
			     IN OUT struct SW_RFB *prSwRfb)
{
	struct mt66xx_chip_info *prChipInfo;
	struct CMD_INFO *prCmdInfo;
	struct WIFI_UNI_EVENT *prEvent;

	ASSERT(prAdapter);
	ASSERT(prSwRfb);
	prChipInfo = prAdapter->chip_info;
	prEvent = (struct WIFI_UNI_EVENT *)
			(prSwRfb->pucRecvBuff + prChipInfo->rxd_size);

	if (prEvent->ucEID != UNI_EVENT_ID_FW_LOG_2_HOST) {
		DBGLOG(NIC, TRACE,
			"RX UNI EVENT: ID[0x%02X] SEQ[%u] LEN[%u] OPT[0x%x]\n",
			prEvent->ucEID, prEvent->ucSeqNum,
			prEvent->u2PacketLength, prEvent->ucOption);
	}

#if (CFG_SUPPORT_STATISTICS == 1)
	wlanWakeLogEvent(prEvent->ucEID);
#endif

	if (IS_UNI_UNSOLICIT_EVENT(prEvent)) {
		if (arUniEventTable[GET_UNI_EVENT_ID(prEvent)])
			arUniEventTable[GET_UNI_EVENT_ID(prEvent)](
				prAdapter, prEvent);
	} else {
		prCmdInfo = nicGetPendingCmdInfo(prAdapter,
						 prEvent->ucSeqNum);

		if (prCmdInfo != NULL) {
			/* input prEvent instead of prEvent->aucBuffer because
			 * handler needs the size of buffer to parse tags
			 */
			if (prCmdInfo->pfCmdDoneHandler)
				prCmdInfo->pfCmdDoneHandler(
					prAdapter, prCmdInfo,
					(uint8_t *)prEvent);
			else if (prCmdInfo->fgIsOid)
				kalOidComplete(
					prAdapter->prGlueInfo,
					prCmdInfo,
					0,
					WLAN_STATUS_SUCCESS);

			/* return prCmdInfo */
			cmdBufFreeCmdInfo(prAdapter, prCmdInfo);
		} else {
			DBGLOG(RX, INFO,
				"UNHANDLED RX EVENT: ID[0x%02X] SEQ[%u] LEN[%u]\n",
			  prEvent->ucEID, prEvent->ucSeqNum,
			  prEvent->u2PacketLength);
		}
	}

	/* Reset Chip NoAck flag */
	if (prAdapter->fgIsChipNoAck) {
		DBGLOG_LIMITED(RX, WARN,
		       "Got response from chip, clear NoAck flag!\n");
		WARN_ON(TRUE);
	}
	prAdapter->ucOidTimeoutCount = 0;
	prAdapter->fgIsChipNoAck = FALSE;

	nicRxReturnRFB(prAdapter, prSwRfb);
}

void nicUniCmdEventSetCommon(IN struct ADAPTER
	*prAdapter, IN struct CMD_INFO *prCmdInfo,
	IN uint8_t *pucEventBuf)
{
	DBGLOG(NIC, TRACE, "prCmdInfo:ucCID=0x%x SQ=%d NR=%d OID=%d\n",
		prCmdInfo->ucCID, prCmdInfo->fgSetQuery,
		prCmdInfo->fgNeedResp, prCmdInfo->fgIsOid);

	if (!prCmdInfo->fgSetQuery || prCmdInfo->fgNeedResp) {
		struct WIFI_UNI_EVENT *evt =
			(struct WIFI_UNI_EVENT *) pucEventBuf;

		DBGLOG(NIC, TRACE, "Resp:EID=0x%x seq=%d option=%d\n",
			evt->ucEID, evt->ucSeqNum, evt->ucOption);
		nicCmdEventSetCommon(prAdapter, prCmdInfo, evt->aucBuffer);
	} else {
		nicCmdEventSetCommon(prAdapter, prCmdInfo, pucEventBuf);
	}
}

void nicUniCmdTimeoutCommon(IN struct ADAPTER *prAdapter,
			    IN struct CMD_INFO *prCmdInfo)
{
	DBGLOG(NIC, TRACE, "prCmdInfo:ucCID=0x%x SQ=%d NR=%d OID=%d\n",
		prCmdInfo->ucCID, prCmdInfo->fgSetQuery,
		prCmdInfo->fgNeedResp, prCmdInfo->fgIsOid);
	nicOidCmdTimeoutCommon(prAdapter, prCmdInfo);
}

void nicUniCmdEventQueryCfgRead(IN struct ADAPTER *prAdapter,
	IN struct CMD_INFO *prCmdInfo, IN uint8_t *pucEventBuf)
{
	struct WIFI_UNI_EVENT *uni_evt = (struct WIFI_UNI_EVENT *) pucEventBuf;
	struct UNI_EVENT_CHIP_CONFIG *evt =
		(struct UNI_EVENT_CHIP_CONFIG *)uni_evt->aucBuffer;
	struct UNI_CMD_CHIP_CONFIG_CUSTOMER_CFG *tag =
		(struct UNI_CMD_CHIP_CONFIG_CUSTOMER_CFG *) evt->aucTlvBuffer;
	struct CMD_HEADER legacy;

	legacy.itemNum = tag->itemNum;
	legacy.cmdBufferLen = tag->cmdBufferLen;
	kalMemCopy(legacy.buffer, tag->aucbuffer, tag->cmdBufferLen);

	nicCmdEventQueryCfgRead(prAdapter, prCmdInfo, (uint8_t *)&legacy);
}

void nicUniEventQueryChipConfig(IN struct ADAPTER *prAdapter,
	IN struct CMD_INFO *prCmdInfo, IN uint8_t *pucEventBuf)
{
	struct WIFI_UNI_EVENT *uni_evt = (struct WIFI_UNI_EVENT *) pucEventBuf;
	struct UNI_EVENT_CHIP_CONFIG *evt =
		(struct UNI_EVENT_CHIP_CONFIG *)uni_evt->aucBuffer;
	struct UNI_CMD_CHIP_CONFIG_CHIP_CFG *tag =
		(struct UNI_CMD_CHIP_CONFIG_CHIP_CFG *) evt->aucTlvBuffer;
	struct UNI_CMD_CHIP_CONFIG_CHIP_CFG_RESP *resp =
		(struct UNI_CMD_CHIP_CONFIG_CHIP_CFG_RESP *) tag->aucbuffer;
	struct CMD_CHIP_CONFIG legacy;

	legacy.u2Id = resp->u2Id;
	legacy.ucType = resp->ucType;
	legacy.ucRespType = resp->ucRespType;
	legacy.u2MsgSize = resp->u2MsgSize;
	kalMemCopy(legacy.aucCmd, resp->aucCmd, resp->u2MsgSize);

	nicCmdEventQueryChipConfig(prAdapter, prCmdInfo, (uint8_t *)&legacy);
}

void nicUniEventQuerySwDbgCtrl(IN struct ADAPTER *prAdapter,
	IN struct CMD_INFO *prCmdInfo, IN uint8_t *pucEventBuf)
{
	struct WIFI_UNI_EVENT *uni_evt = (struct WIFI_UNI_EVENT *) pucEventBuf;
	struct UNI_EVENT_CHIP_CONFIG *evt =
		(struct UNI_EVENT_CHIP_CONFIG *)uni_evt->aucBuffer;
	struct UNI_CMD_CHIP_CONFIG_SW_DBG_CTRL *tag =
		(struct UNI_CMD_CHIP_CONFIG_SW_DBG_CTRL *) evt->aucTlvBuffer;
	struct CMD_SW_DBG_CTRL legacy;

	legacy.u4Id = tag->u4Id;
	legacy.u4Data = tag->u4Data;

	nicCmdEventQuerySwCtrlRead(prAdapter, prCmdInfo, (uint8_t *)&legacy);
}

void nicUniCmdStaRecHandleEventPkt(IN struct ADAPTER
	*prAdapter, IN struct CMD_INFO *prCmdInfo, IN uint8_t *pucEventBuf)
{
	struct UNI_CMD_STAREC *uni_cmd =
		(struct UNI_CMD_STAREC *) GET_UNI_CMD_DATA(prCmdInfo);
	struct WIFI_UNI_EVENT *uni_evt = (struct WIFI_UNI_EVENT *) pucEventBuf;
	struct UNI_EVENT_CMD_RESULT *evt =
		(struct UNI_EVENT_CMD_RESULT *)uni_evt->aucBuffer;

	DBGLOG(NIC, TRACE,
		"cmd_result:ucCID=0x%x, status=%d, wlanidx=%d\n",
		evt->u2CID, evt->u4Status, uni_cmd->ucWlanIdxL);

	if (evt->u2CID == UNI_CMD_ID_STAREC_INFO && evt->u4Status == 0) {
		struct STA_RECORD *prStaRec = cnmGetStaRecByIndex(prAdapter,
			secGetStaIdxByWlanIdx(prAdapter, uni_cmd->ucWlanIdxL));

		qmActivateStaRec(prAdapter, prStaRec);
	}
}

void nicUniEventQueryIdcChnl(IN struct ADAPTER *prAdapter,
		IN struct CMD_INFO *prCmdInfo,
		IN uint8_t *pucEventBuf)
{
	struct WIFI_UNI_EVENT *uni_evt = (struct WIFI_UNI_EVENT *) pucEventBuf;
	struct UNI_EVENT_IDC *evt = (struct UNI_EVENT_IDC *)uni_evt->aucBuffer;
	struct UNI_EVENT_MD_SAFE_CHN *tag =
		(struct UNI_EVENT_MD_SAFE_CHN *) evt->aucTlvBuffer;
	struct EVENT_LTE_SAFE_CHN legacy;

	legacy.ucVersion = tag->ucVersion;
	legacy.u4Flags = tag->u4Flags;
	kalMemCopy(legacy.rLteSafeChn.au4SafeChannelBitmask,
		   tag->u4SafeChannelBitmask,
		   sizeof(uint32_t) * ENUM_SAFE_CH_MASK_MAX_NUM);

	nicCmdEventQueryLteSafeChn(prAdapter, prCmdInfo, (uint8_t *)&legacy);
}

void nicUniEventBFStaRec(IN struct ADAPTER *prAdapter,
	IN struct CMD_INFO *prCmdInfo, IN uint8_t *pucEventBuf)
{
	struct WIFI_UNI_EVENT *uni_evt = (struct WIFI_UNI_EVENT *) pucEventBuf;
	struct UNI_EVENT_BF *evt = (struct UNI_EVENT_BF *)uni_evt->aucBuffer;
	struct UNI_EVENT_BF_STA_REC *tag =
		(struct UNI_EVENT_BF_STA_REC *) evt->au1TlvBuffer;
	struct TXBF_PFMU_STA_INFO *info =
		(struct TXBF_PFMU_STA_INFO *) &tag->rTxBfPfmuInfo;

	DBGLOG(RFTEST, INFO,
		 "====================================== BF StaRec ========================================\n"
		 "rStaRecBf.u2PfmuId	  = %d\n"
		 "rStaRecBf.fgSU_MU	  = %d\n"
		 "rStaRecBf.u1TxBfCap	  = %d\n"
		 "rStaRecBf.ucSoundingPhy = %d\n"
		 "rStaRecBf.ucNdpaRate	  = %d\n"
		 "rStaRecBf.ucNdpRate	  = %d\n"
		 "rStaRecBf.ucReptPollRate= %d\n"
		 "rStaRecBf.ucTxMode	  = %d\n"
		 "rStaRecBf.ucNc	  = %d\n"
		 "rStaRecBf.ucNr	  = %d\n"
		 "rStaRecBf.ucCBW	  = %d\n"
		 "rStaRecBf.ucTotMemRequire = %d\n"
		 "rStaRecBf.ucMemRequire20M = %d\n"
		 "rStaRecBf.ucMemRow0	  = %d\n"
		 "rStaRecBf.ucMemCol0	  = %d\n"
		 "rStaRecBf.ucMemRow1	  = %d\n"
		 "rStaRecBf.ucMemCol1	  = %d\n"
		 "rStaRecBf.ucMemRow2	  = %d\n"
		 "rStaRecBf.ucMemCol2	  = %d\n"
		 "rStaRecBf.ucMemRow3	  = %d\n"
		 "rStaRecBf.ucMemCol3	  = %d\n",
		 info->u2PfmuId,
		 info->fgSU_MU,
		 info->u1TxBfCap,
		 info->ucSoundingPhy,
		 info->ucNdpaRate,
		 info->ucNdpRate,
		 info->ucReptPollRate,
		 info->ucTxMode,
		 info->ucNc,
		 info->ucNr,
		 info->ucCBW,
		 info->ucTotMemRequire,
		 info->ucMemRequire20M,
		 info->ucMemRow0,
		 info->ucMemCol0,
		 info->ucMemRow1,
		 info->ucMemCol1,
		 info->ucMemRow2,
		 info->ucMemCol2,
		 info->ucMemRow3,
		 info->ucMemCol3);
	DBGLOG(RFTEST, INFO,
		 "rStaRecBf.u2SmartAnt	  = 0x%x\n"
		 "rStaRecBf.ucSEIdx	  = %d\n"
		 "rStaRecBf.ucAutoSoundingCtrl = %d\n"
		 "rStaRecBf.uciBfTimeOut  = 0x%x\n"
		 "rStaRecBf.uciBfDBW	  = %d\n"
		 "rStaRecBf.uciBfNcol	  = %d\n"
		 "rStaRecBf.uciBfNrow	  = %d\n"
		 "rStaRecBf.nr_bw160	  = %d\n"
		 "rStaRecBf.nc_bw160	  = %d\n"
		 "rStaRecBf.ru_start_idx  = %d\n"
		 "rStaRecBf.ru_end_idx	  = %d\n"
		 "rStaRecBf.trigger_su	  = %d\n"
		 "rStaRecBf.trigger_mu	  = %d\n"
		 "rStaRecBf.ng16_su	  = %d\n"
		 "rStaRecBf.ng16_mu	  = %d\n"
		 "rStaRecBf.codebook42_su = %d\n"
		 "rStaRecBf.codebook75_mu = %d\n"
		 "rStaRecBf.he_ltf	      = %d\n"
		 "=======================================================================================\n",
		 info->u2SmartAnt,
		 info->ucSEIdx,
		 info->ucAutoSoundingCtrl,
		 info->uciBfTimeOut,
		 info->uciBfDBW,
		 info->uciBfNcol,
		 info->uciBfNrow,
		 info->u1NrBw160,
		 info->u1NcBw160,
		 info->u1RuStartIdx,
		 info->u1RuEndIdx,
		 info->fgTriggerSu,
		 info->fgTriggerMu,
		 info->fgNg16Su,
		 info->fgNg16Mu,
		 info->fgCodebook42Su,
		 info->fgCodebook75Mu,
		 info->u1HeLtf);
	nicCmdEventSetCommon(prAdapter, prCmdInfo, pucEventBuf);
}

void nicUniCmdEventQueryMcrRead(IN struct ADAPTER *prAdapter,
	IN struct CMD_INFO *prCmdInfo, IN uint8_t *pucEventBuf)
{
	struct UNI_CMD_ACCESS_REG *uni_cmd =
		(struct UNI_CMD_ACCESS_REG *) GET_UNI_CMD_DATA(prCmdInfo);
	struct WIFI_UNI_EVENT *uni_evt = (struct WIFI_UNI_EVENT *) pucEventBuf;
	struct UNI_EVENT_ACCESS_REG *evt =
		(struct UNI_EVENT_ACCESS_REG *)uni_evt->aucBuffer;
	struct CMD_ACCESS_REG legacy;


	if (TAG_ID(uni_cmd->aucTlvBuffer) == UNI_CMD_ACCESS_REG_TAG_BASIC) {
		struct UNI_EVENT_ACCESS_REG_BASIC *tag =
			(struct UNI_EVENT_ACCESS_REG_BASIC *) evt->aucTlvBuffer;

		legacy.u4Address = tag->u4Addr;
		legacy.u4Data = tag->u4Value;
	} else { /* RF */
		struct UNI_EVENT_ACCESS_RF_REG_BASIC *tag =
		     (struct UNI_EVENT_ACCESS_RF_REG_BASIC *) evt->aucTlvBuffer;

		legacy.u4Address = tag->u4Addr;
		legacy.u4Data = tag->u4Value;
	}

	DBGLOG(RFTEST, INFO, "CMD:0x%x read addr=0x%x, value=0x%x\n",
		TAG_ID(uni_cmd->aucTlvBuffer), legacy.u4Address, legacy.u4Data);
	nicCmdEventQueryMcrRead(prAdapter, prCmdInfo, (uint8_t *)&legacy);
}

void nicUniCmdEventGetTsfDone(IN struct ADAPTER *prAdapter,
	IN struct CMD_INFO *prCmdInfo, IN uint8_t *pucEventBuf)
{
	struct WIFI_UNI_EVENT *uni_evt = (struct WIFI_UNI_EVENT *) pucEventBuf;
	struct UNI_EVENT_MAC_IFNO *evt =
		(struct UNI_EVENT_MAC_IFNO *)uni_evt->aucBuffer;
	struct UNI_EVENT_MAC_INFO_TSF *tag =
		(struct UNI_EVENT_MAC_INFO_TSF *) evt->aucTlvBuffer;
	struct EXT_EVENT_MAC_INFO_T legacy;

	legacy.rMacInfoResult.rTsfResult.u4TsfBitsLow = tag->u4TsfBit0_31;
	legacy.rMacInfoResult.rTsfResult.u4TsfBitsHigh = tag->u4TsfBit63_32;

	twtPlannerGetTsfDone(prAdapter, prCmdInfo, (uint8_t *)&legacy);
}

void nicUniCmdEventInstallKey(IN struct ADAPTER
	*prAdapter, IN struct CMD_INFO *prCmdInfo, IN uint8_t *pucEventBuf)
{
	struct UNI_CMD_STAREC *uni_cmd =
		(struct UNI_CMD_STAREC *) GET_UNI_CMD_DATA(prCmdInfo);
	struct UNI_CMD_STAREC_INSTALL_KEY3 *tag =
	   (struct UNI_CMD_STAREC_INSTALL_KEY3 *) uni_cmd->aucTlvBuffer;

	nicUniCmdEventSetCommon(prAdapter, prCmdInfo, pucEventBuf);

#if CFG_SUPPORT_REPLAY_DETECTION
	nicCmdEventDetectReplayInfo(prAdapter, tag->ucKeyId,
		tag->ucKeyType, tag->ucBssIdx);
#endif

}

void nicUniEventQueryCnmInfo(IN struct ADAPTER
	*prAdapter, IN struct CMD_INFO *prCmdInfo, IN uint8_t *pucEventBuf)
{
	int32_t tags_len;
	uint8_t *tag;
	uint16_t offset = 0;
	uint32_t fixed_len = sizeof(struct UNI_EVENT_CNM);
	uint32_t data_len = GET_UNI_EVENT_DATA_LEN(pucEventBuf);
	uint8_t *data = GET_UNI_EVENT_DATA(pucEventBuf);
	uint32_t fail_cnt = 0;
	struct PARAM_GET_CNM_T legacy = {0};
	uint8_t i, j;

	tags_len = data_len - fixed_len;
	tag = data + fixed_len;
	TAG_FOR_EACH(tag, tags_len, offset) {
		DBGLOG(CNM, INFO, "Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_CNM_TAG_GET_BSS_INFO: {
			struct UNI_EVENT_CNM_GET_BSS_INFO *info =
				(struct UNI_EVENT_CNM_GET_BSS_INFO *)tag;
			struct UNI_EVENT_CNM_BSS_INFO *bss =
			      (struct UNI_EVENT_CNM_BSS_INFO *)info->aucBssInfo;

			for (i = 0; i < info->ucBssNum; i++) {
				if (i > BSSID_NUM) break;

				legacy.ucBssInuse[i] = bss->ucBssInuse;
				legacy.ucBssActive[i] = bss->ucBssActive;
				legacy.ucBssConnectState[i] =
							bss->ucBssConnectState;
				legacy.ucBssCh[i] = bss->ucBssPriChannel;
				legacy.ucBssDBDCBand[i] = bss->ucBssDBDCBand;
				legacy.ucBssOMACSet[i] = bss->ucBssOMACIndex;
				legacy.ucBssOMACDBDCBand[i] =
							bss->ucBssOMACDBDCBand;
				bss++;
			}
		}
			break;
		case UNI_EVENT_CNM_TAG_GET_CHANNEL_INFO: {
			struct UNI_EVENT_CNM_GET_CHANNEL_INFO *info =
				(struct UNI_EVENT_CNM_GET_CHANNEL_INFO *)tag;
			struct UNI_EVENT_CNM_CHANNEL_INFO *chnl =
			 (struct UNI_EVENT_CNM_CHANNEL_INFO *)info->aucChnlInfo;
			uint8_t b = info->ucDBDCBand;

			if (b >= ENUM_BAND_NUM)
				break;

			legacy.ucOpChNum[b] = info->ucOpChNum;
			for (i = 0; i < info->ucOpChNum; i++) {
				if (i >= MAX_OP_CHNL_NUM) break;

				legacy.ucChList[b][i] = chnl->ucPriChannel;
				legacy.ucChBw[b][i] = chnl->ucChBw;
				legacy.ucChSco[b][i] = chnl->ucChSco;
				legacy.ucChNetNum[b][i] = chnl->ucChBssNum;

				for (j = 0; j < 16; j++) {
					if (chnl->u2ChBssBitmapList & BIT(j))
						legacy.ucChBssList[b][i][j] = 1;
				}

				chnl++;
			}
		}
			break;
		default:
			fail_cnt++;
			ASSERT(fail_cnt < MAX_UNI_EVENT_FAIL_TAG_COUNT)
			DBGLOG(CNM, WARN, "invalid tag = %d\n", TAG_ID(tag));
			break;
		}
	}

	nicCmdEventQueryCnmInfo(prAdapter, prCmdInfo, (uint8_t *)&legacy);
}

void nicUniEventRfTestHandler(IN struct ADAPTER
	*prAdapter, IN struct CMD_INFO *prCmdInfo, IN uint8_t *pucEventBuf)
{
	int32_t tags_len;
	uint8_t *tag;
	uint16_t offset = 0;
	uint32_t fixed_len = sizeof(struct UNI_EVENT_TESTMODE_CTRL);
	uint32_t data_len = GET_UNI_EVENT_DATA_LEN(pucEventBuf);
	uint8_t *data = GET_UNI_EVENT_DATA(pucEventBuf);
	uint32_t fail_cnt = 0;
	struct mt66xx_chip_info *prChipInfo = NULL;
	struct ATE_OPS_T *prAteOps = NULL;
	struct ICAP_INFO_T *prIcapInfo;
	struct UNI_EVENT_RF_TEST_RESULT *prRfResult;
	struct EXT_EVENT_RBIST_CAP_STATUS_T *prCapStatus;

	ASSERT(prAdapter);
	prChipInfo = prAdapter->chip_info;
	ASSERT(prChipInfo);
	prAteOps = prChipInfo->prAteOps;
	ASSERT(prAteOps);
	prIcapInfo = &prAdapter->rIcapInfo;

	tags_len = data_len - fixed_len;
	tag = data + fixed_len;
	TAG_FOR_EACH(tag, tags_len, offset) {
		DBGLOG(NIC, INFO, "Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_RF_TEST_RESULT_TAG: {

			prRfResult = (struct UNI_EVENT_RF_TEST_RESULT *)
				(tag + sizeof(struct UNI_EVENT_RF_TEST_TLV));

			if (prRfResult->u4FuncIndex == GET_ICAP_CAPTURE_STATUS) {

				prCapStatus = (struct EXT_EVENT_RBIST_CAP_STATUS_T *)
							(tag + sizeof(struct UNI_EVENT_RF_TEST_TLV));

				DBGLOG(RFTEST, INFO, "%s:iCapDone=%d, icap state=%d\n",
						__func__,
					   prCapStatus->u4CapDone,
					   prAdapter->rIcapInfo.eIcapState);

				if (prCapStatus->u4CapDone &&
					prIcapInfo->eIcapState != ICAP_STATE_FW_DUMP_DONE) {

					wlanoidRfTestICapRawDataProc(prAdapter,
					 0 /*prCapStatus->u4CapStartAddr*/,
					 0 /*prCapStatus->u4TotalBufferSize*/);

					prIcapInfo->eIcapState = ICAP_STATE_FW_DUMPING;

				}
			} else if (prRfResult->u4FuncIndex == GET_ICAP_RAW_DATA) {

				DBGLOG(RFTEST, INFO, "%s:u4FuncIndex=%d, icap state=%d\n",
						__func__,
					   prRfResult->u4FuncIndex,
					   prAdapter->rIcapInfo.eIcapState);

				if (prAteOps->getRbistDataDumpEvent) {
					prAteOps->getRbistDataDumpEvent(prAdapter,
							tag + sizeof(struct UNI_EVENT_RF_TEST_TLV));

					if (prIcapInfo->eIcapState != ICAP_STATE_FW_DUMP_DONE)
						wlanoidRfTestICapRawDataProc(prAdapter,
						0 /*prCapStatus->u4CapStartAddr*/,
						0 /*prCapStatus->u4TotalBufferSize*/);
				}

			} else {
				DBGLOG(NIC, WARN, "Unknown funcIdx rf test event = %d\n",
					prRfResult->u4FuncIndex);
			}
		}
			break;

		default:
			fail_cnt++;
			ASSERT(fail_cnt < MAX_UNI_EVENT_FAIL_TAG_COUNT)
			DBGLOG(NIC, WARN, "invalid tag = %d\n", TAG_ID(tag));
			break;
		}
	}

}

void nicUniEventStaStatistics(IN struct ADAPTER
	*prAdapter, IN struct CMD_INFO *prCmdInfo, IN uint8_t *pucEventBuf)
{
	struct WIFI_UNI_EVENT *uni_evt = (struct WIFI_UNI_EVENT *) pucEventBuf;
	struct UNI_EVENT_STATISTICS *evt =
		(struct UNI_EVENT_STATISTICS *)uni_evt->aucBuffer;
	struct UNI_EVENT_STA_STATISTICS *tag =
		(struct UNI_EVENT_STA_STATISTICS *) evt->aucTlvBuffer;

	nicCmdEventQueryStaStatistics(prAdapter, prCmdInfo, tag->aucBuffer);
}

void nicUniEventStatistics(IN struct ADAPTER
	*prAdapter, IN struct CMD_INFO *prCmdInfo, IN uint8_t *pucEventBuf)
{
	struct WIFI_UNI_EVENT *uni_evt = (struct WIFI_UNI_EVENT *) pucEventBuf;
	struct UNI_EVENT_STATISTICS *evt =
		(struct UNI_EVENT_STATISTICS *)uni_evt->aucBuffer;
	struct UNI_EVENT_GET_STATISTICS *tag =
		(struct UNI_EVENT_GET_STATISTICS *) evt->aucTlvBuffer;
	struct UNI_EVENT_BASIC_STATISTICS *basic =
		(struct UNI_EVENT_BASIC_STATISTICS *)tag->aucBuffer;
	struct EVENT_STATISTICS legacy;

	kalMemSet(&legacy, 0, sizeof(legacy));
	legacy.rTransmittedFragmentCount.QuadPart =
		basic->u8TransmittedFragmentCount;
	legacy.rMulticastTransmittedFrameCount.QuadPart =
		basic->u8MulticastTransmittedFrameCount;
	legacy.rFailedCount.QuadPart = basic->u8FailedCount;
	legacy.rRetryCount.QuadPart = basic->u8RetryCount;
	legacy.rMultipleRetryCount.QuadPart = basic->u8MultipleRetryCount;
	legacy.rRTSSuccessCount.QuadPart = basic->u8RTSSuccessCount;
	legacy.rRTSFailureCount.QuadPart = basic->u8RTSFailureCount;
	legacy.rACKFailureCount.QuadPart = basic->u8ACKFailureCount;
	legacy.rFrameDuplicateCount.QuadPart = basic->u8FrameDuplicateCount;
	legacy.rReceivedFragmentCount.QuadPart = basic->u8ReceivedFragmentCount;
	legacy.rMulticastReceivedFrameCount.QuadPart =
		basic->u8MulticastReceivedFrameCount;
	legacy.rFCSErrorCount.QuadPart = basic->u8FCSErrorCount;
	legacy.rMdrdyCnt.QuadPart = basic->u8MdrdyCnt;
	legacy.rChnlIdleCnt.QuadPart = basic->u8ChnlIdleCnt;
	legacy.u4HwMacAwakeDuration = basic->u4HwMacAwakeDuration;

	nicCmdEventQueryStatistics(prAdapter, prCmdInfo, (uint8_t*)&legacy);
}

void nicUniEventLinkQuality(IN struct ADAPTER
	*prAdapter, IN struct CMD_INFO *prCmdInfo, IN uint8_t *pucEventBuf)
{
	struct WIFI_UNI_EVENT *uni_evt = (struct WIFI_UNI_EVENT *) pucEventBuf;
	struct UNI_EVENT_STATISTICS *evt =
		(struct UNI_EVENT_STATISTICS *)uni_evt->aucBuffer;
	struct UNI_EVENT_LINK_QUALITY *tag =
		(struct UNI_EVENT_LINK_QUALITY *) evt->aucTlvBuffer;
	struct EVENT_LINK_QUALITY legacy;
	uint8_t i;

	for (i = 0; i < 4; i++) {
		legacy.rLq[i].cRssi = tag->rLq[i].cRssi;
		legacy.rLq[i].cLinkQuality = tag->rLq[i].cLinkQuality;
		legacy.rLq[i].u2LinkSpeed = tag->rLq[i].u2LinkSpeed;
		legacy.rLq[i].ucMediumBusyPercentage =
				tag->rLq[i].ucMediumBusyPercentage;
		legacy.rLq[i].ucIsLQ0Rdy = tag->rLq[i].ucIsLQ0Rdy;
	}

	nicCmdEventQueryLinkQuality(prAdapter, prCmdInfo, (uint8_t *)&legacy);
}

void nicUniEventQueryRfTestATInfo(IN struct ADAPTER
	  *prAdapter, IN struct CMD_INFO *prCmdInfo, IN uint8_t *pucEventBuf)
{
	struct WIFI_UNI_EVENT *uni_evt = (struct WIFI_UNI_EVENT *)pucEventBuf;
	struct UNI_EVENT_TESTMODE_CTRL *evt =
		(struct UNI_EVENT_TESTMODE_CTRL *)uni_evt->aucBuffer;
	struct UNI_EVENT_RF_TEST_TLV *tag =
		(struct UNI_EVENT_RF_TEST_TLV *)evt->aucTlvBuffer;

	nicCmdEventQueryRfTestATInfo(prAdapter, prCmdInfo, tag->aucBuffer);
}

void nicUniEventQueryRxStatAll(IN struct ADAPTER
	  *prAdapter, IN struct CMD_INFO *prCmdInfo, IN uint8_t *pucEventBuf)
{
	struct WIFI_UNI_EVENT *uni_evt = (struct WIFI_UNI_EVENT *)pucEventBuf;
	struct UNI_EVENT_TESTMODE_RX_STAT *rx_evt =
		(struct UNI_EVENT_TESTMODE_RX_STAT *)uni_evt->aucBuffer;
	struct UNI_EVENT_TESTMODE_STAT_ALL *tag =
		(struct UNI_EVENT_TESTMODE_STAT_ALL *)rx_evt->aucTlvBuffer;

	struct GLUE_INFO *prGlueInfo = prAdapter->prGlueInfo;
	uint32_t *prElement = (uint32_t *)&g_HqaRxStat;
	uint32_t i, u4Temp;

	for (i = 0; i < UNI_EVENT_TESTMODE_RX_STAT_ALL_ITEM; i++) {
		u4Temp = ntohl(tag->u4Buffer[i]);
		kalMemCopy(prElement, &u4Temp, 4);

		if (i < (UNI_EVENT_TESTMODE_RX_STAT_ALL_ITEM - 1))
			prElement++;
	}

	if (prCmdInfo->fgIsOid) {
		kalOidComplete(prGlueInfo,
						prCmdInfo,
						sizeof(struct CMD_ACCESS_RX_STAT),
						WLAN_STATUS_SUCCESS);
	}
}

void nicUniEventBugReport(IN struct ADAPTER
	*prAdapter, IN struct CMD_INFO *prCmdInfo, IN uint8_t *pucEventBuf)
{
	struct WIFI_UNI_EVENT *uni_evt = (struct WIFI_UNI_EVENT *) pucEventBuf;
	struct UNI_EVENT_STATISTICS *evt =
		(struct UNI_EVENT_STATISTICS *)uni_evt->aucBuffer;
	struct UNI_EVENT_BUG_REPORT *tag =
		(struct UNI_EVENT_BUG_REPORT *) evt->aucTlvBuffer;

	nicCmdEventQueryBugReport(prAdapter, prCmdInfo,
		(uint8_t *)&tag->u4BugReportVersion);
}

void nicUniEventTxPowerInfo(IN struct ADAPTER
	*prAdapter, IN struct CMD_INFO *prCmdInfo, IN uint8_t *pucEventBuf)
{
#if (CFG_SUPPORT_TXPOWER_INFO == 1)
	struct WIFI_UNI_EVENT *uni_evt = (struct WIFI_UNI_EVENT *) pucEventBuf;
	struct UNI_EVENT_TXPOWER *evt =
		(struct UNI_EVENT_TXPOWER *)uni_evt->aucBuffer;
	struct UNI_EVENT_TXPOWER_RSP *tag =
		(struct UNI_EVENT_TXPOWER_RSP *) evt->aucTlvBuffer;

	nicCmdEventQueryTxPowerInfo(prAdapter, prCmdInfo, tag->aucBuffer);
#endif
}

/*******************************************************************************
 *                   Unsolicited Event
 *******************************************************************************
 */

void nicUniEventScanDone(struct ADAPTER *ad, struct WIFI_UNI_EVENT *evt)
{
	int32_t tags_len;
	uint8_t *tag;
	uint16_t offset = 0;
	uint32_t fixed_len = sizeof(struct UNI_EVENT_SCAN_DONE);
	uint32_t data_len = GET_UNI_EVENT_DATA_LEN(evt);
	uint8_t *data = GET_UNI_EVENT_DATA(evt);
	uint32_t fail_cnt = 0;
	int i;
	struct UNI_EVENT_SCAN_DONE *scan_done;
	struct EVENT_SCAN_DONE legacy;

	scan_done = (struct UNI_EVENT_SCAN_DONE *) data;
	legacy.ucSeqNum = scan_done->ucSeqNum;

	tags_len = data_len - fixed_len;
	tag = data + fixed_len;
	TAG_FOR_EACH(tag, tags_len, offset) {
		DBGLOG(SCN, INFO, "Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_SCAN_DONE_TAG_BASIC: {
			struct UNI_EVENT_SCAN_DONE_BASIC *basic =
				(struct UNI_EVENT_SCAN_DONE_BASIC *) tag;

			legacy.ucCompleteChanCount = basic->ucCompleteChanCount;
			legacy.ucCurrentState = basic->ucCurrentState;
			legacy.ucScanDoneVersion = basic->ucScanDoneVersion;
			legacy.fgIsPNOenabled = basic->fgIsPNOenabled;
			legacy.u4ScanDurBcnCnt = basic->u4ScanDurBcnCnt;
		}
			break;
		case UNI_EVENT_SCAN_DONE_TAG_SPARSECHNL: {
			struct UNI_EVENT_SCAN_DONE_SPARSECHNL *sparse =
				(struct UNI_EVENT_SCAN_DONE_SPARSECHNL *) tag;

			legacy.ucSparseChannelValid =
				sparse->ucSparseChannelValid;
			legacy.rSparseChannel.ucBand = sparse->ucBand;
			legacy.rSparseChannel.ucChannelNum =
				sparse->ucChannelNum;
			legacy.ucSparseChannelArrayValidNum =
				sparse->ucSparseChannelArrayValidNum;
		}
			break;
		case UNI_EVENT_SCAN_DONE_TAG_CHNLINFO: {
			struct UNI_EVENT_SCAN_DONE_CHNLINFO *chnlinfo =
				(struct UNI_EVENT_SCAN_DONE_CHNLINFO *) tag;
			struct UNI_EVENT_CHNLINFO *chnl =
				(struct UNI_EVENT_CHNLINFO *)
				chnlinfo->aucChnlInfoBuffer;

			ASSERT(chnlinfo->ucNumOfChnl <
				SCAN_DONE_EVENT_MAX_CHANNEL_NUM);
			ASSERT(chnlinfo->u2Length == sizeof(*chnlinfo) +
				chnlinfo->ucNumOfChnl * sizeof(*chnl));
			for (i = 0; i < chnlinfo->ucNumOfChnl; i++, chnl++) {
				legacy.aucChannelNum[i] =
					chnl->ucChannelNum;
				legacy.au2ChannelIdleTime[i] =
					chnl->u2ChannelIdleTime;
				legacy.aucChannelBAndPCnt[i] =
					chnl->ucChannelBAndPCnt;
				legacy.aucChannelMDRDYCnt[i] =
					chnl->ucChannelMDRDYCnt;
				legacy.au2ChannelScanTime[i] =
					chnl->u2ChannelScanTime;
			}
		}
			break;
		case UNI_EVENT_SCAN_DONE_TAG_NLO:{
			struct UNI_EVENT_SCAN_DONE_NLO *nlo =
				(struct UNI_EVENT_SCAN_DONE_NLO *) tag;
			struct EVENT_SCHED_SCAN_DONE sched;

			sched.ucStatus = nlo->ucStatus;
			sched.ucSeqNum = legacy.ucSeqNum;

			/* sched scan done, return directly */
			return scnEventSchedScanDone(ad, &sched);
		}
			break;
		default:
			fail_cnt++;
			ASSERT(fail_cnt < MAX_UNI_EVENT_FAIL_TAG_COUNT)
			DBGLOG(SCN, WARN, "invalid tag = %d\n", TAG_ID(tag));
			break;
		}
	}

	scnEventScanDone(ad, &legacy, TRUE);
}

void nicUniEventChMngrHandleChEvent(struct ADAPTER *ad,
	struct WIFI_UNI_EVENT *evt)
{
	int32_t tags_len;
	uint8_t *tag;
	uint16_t offset = 0;
	uint32_t fixed_len = sizeof(struct UNI_EVENT_CNM);
	uint32_t data_len = GET_UNI_EVENT_DATA_LEN(evt);
	uint8_t *data = GET_UNI_EVENT_DATA(evt);
	uint32_t fail_cnt = 0;

	DBGLOG_MEM8(CNM, TRACE, data, data_len);

	tags_len = data_len - fixed_len;
	tag = data + fixed_len;
	TAG_FOR_EACH(tag, tags_len, offset) {
		DBGLOG(CNM, INFO, "Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_CNM_TAG_CH_PRIVILEGE_MLO_SUB_GRANT: {
			struct UNI_EVENT_CNM_CH_PRIVILEGE_GRANT *grant =
				(struct UNI_EVENT_CNM_CH_PRIVILEGE_GRANT *)tag;

			cnmUpdateMbmcIdx(ad, grant->ucBssIndex, grant->ucDBDCBand);
		}
			break;
		case UNI_EVENT_CNM_TAG_CH_PRIVILEGE_GRANT: {
			struct UNI_EVENT_CNM_CH_PRIVILEGE_GRANT *grant =
				(struct UNI_EVENT_CNM_CH_PRIVILEGE_GRANT *)tag;
			struct EVENT_CH_PRIVILEGE legacy;

			cnmUpdateMbmcIdx(ad, grant->ucBssIndex, grant->ucDBDCBand);

			legacy.ucBssIndex = grant->ucBssIndex;
			legacy.ucTokenID = grant->ucTokenID;
			legacy.ucStatus = grant->ucStatus;
			legacy.ucPrimaryChannel = grant->ucPrimaryChannel;
			legacy.ucRfSco = grant->ucRfSco;
			legacy.ucRfBand = grant->ucRfBand;
			legacy.ucRfChannelWidth = grant->ucRfChannelWidth;
			legacy.ucRfCenterFreqSeg1 = grant->ucRfCenterFreqSeg1;
			legacy.ucRfCenterFreqSeg2 = grant->ucRfCenterFreqSeg2;
			legacy.ucReqType = grant->ucReqType;
			legacy.ucDBDCBand = grant->ucDBDCBand;
			legacy.u4GrantInterval = grant->u4GrantInterval;

			RUN_RX_EVENT_HANDLER(EVENT_ID_CH_PRIVILEGE, &legacy);
		}
			break;
		case UNI_EVENT_CNM_TAG_OPMODE_CHANGE: {
			struct UNI_EVENT_CNM_OPMODE_CHANGE *opmode =
				(struct UNI_EVENT_CNM_OPMODE_CHANGE *)tag;
			struct EVENT_OPMODE_CHANGE legacy;

			legacy.ucBssBitmap = (uint8_t) opmode->u2BssBitmap;
			legacy.ucEnable = opmode->ucEnable;
			legacy.ucOpTxNss = opmode->ucOpTxNss;
			legacy.ucOpRxNss = opmode->ucOpRxNss;
			legacy.ucReason = opmode->ucReason;

			RUN_RX_EVENT_HANDLER(EVENT_ID_OPMODE_CHANGE, &legacy);
		}
			break;
		default:
			fail_cnt++;
			ASSERT(fail_cnt < MAX_UNI_EVENT_FAIL_TAG_COUNT)
			DBGLOG(CNM, WARN, "invalid tag = %d\n", TAG_ID(tag));
			break;
		}
	}
}

void nicUniEventMbmcHandleEvent(struct ADAPTER *ad, struct WIFI_UNI_EVENT *evt)
{
	int32_t tags_len;
	uint8_t *tag;
	uint16_t offset = 0;
	uint32_t fixed_len = sizeof(struct UNI_EVENT_MBMC);
	uint32_t data_len = GET_UNI_EVENT_DATA_LEN(evt);
	uint8_t *data = GET_UNI_EVENT_DATA(evt);
	uint32_t fail_cnt = 0;

	tags_len = data_len - fixed_len;
	tag = data + fixed_len;
	TAG_FOR_EACH(tag, tags_len, offset) {
		DBGLOG(CNM, INFO, "Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_MBMC_TAG_SWITCH_DONE:
			RUN_RX_EVENT_HANDLER_EXT(EVENT_ID_DBDC_SWITCH_DONE,
								NULL, 0);
			break;
		default:
			fail_cnt++;
			ASSERT(fail_cnt < MAX_UNI_EVENT_FAIL_TAG_COUNT)
			DBGLOG(CNM, WARN, "invalid tag = %d\n", TAG_ID(tag));
			break;
		}
	}
}

void nicUniEventStatusToHost(struct ADAPTER *ad, struct WIFI_UNI_EVENT *evt)
{
	int32_t tags_len;
	uint8_t *tag;
	uint16_t offset = 0;
	uint32_t fixed_len = sizeof(struct UNI_EVENT_STATUS_TO_HOST);
	uint32_t data_len = GET_UNI_EVENT_DATA_LEN(evt);
	uint8_t *data = GET_UNI_EVENT_DATA(evt);
	uint32_t fail_cnt = 0;

	tags_len = data_len - fixed_len;
	tag = data + fixed_len;
	TAG_FOR_EACH(tag, tags_len, offset) {
		DBGLOG(NIC, TRACE, "Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_STATUS_TO_HOST_TAG_TX_DONE:{
			struct UNI_EVENT_TX_DONE *tx =
				(struct UNI_EVENT_TX_DONE *) tag;
			struct EVENT_TX_DONE legacy;

			legacy.ucPacketSeq = tx->ucPacketSeq;
			legacy.ucStatus = tx->ucStatus;
			legacy.u2SequenceNumber = tx->u2SequenceNumber;
			legacy.ucWlanIndex = tx->ucWlanIndex;
			legacy.ucTxCount = tx->ucTxCount;
			legacy.u2TxRate = tx->u2TxRate;

			legacy.ucFlag = tx->ucFlag;
			legacy.ucTid = tx->ucTid;
			legacy.ucRspRate = tx->ucRspRate;
			legacy.ucRateTableIdx = tx->ucRateTableIdx;

			legacy.ucBandwidth = tx->ucBandwidth;
			legacy.ucTxPower = tx->ucTxPower;
			legacy.ucFlushReason = tx->ucFlushReason;

			legacy.u4TxDelay = tx->u4TxDelay;
			legacy.u4Timestamp = tx->u4Timestamp;
			legacy.u4AppliedFlag = tx->u4AppliedFlag;

			RUN_RX_EVENT_HANDLER(EVENT_ID_TX_DONE, &legacy);
		}
			break;
		default:
			fail_cnt++;
			ASSERT(fail_cnt < MAX_UNI_EVENT_FAIL_TAG_COUNT)
			DBGLOG(NIC, WARN, "invalid tag = %d\n", TAG_ID(tag));
			break;
		}
	}
}

void nicUniEventBaOffload(struct ADAPTER *ad, struct WIFI_UNI_EVENT *evt)
{
	int32_t tags_len;
	uint8_t *tag;
	uint16_t offset = 0;
	uint32_t fixed_len = sizeof(struct UNI_EVENT_BA_OFFLOAD);
	uint32_t data_len = GET_UNI_EVENT_DATA_LEN(evt);
	uint8_t *data = GET_UNI_EVENT_DATA(evt);
	uint32_t fail_cnt = 0;

	tags_len = data_len - fixed_len;
	tag = data + fixed_len;
	TAG_FOR_EACH(tag, tags_len, offset) {
		DBGLOG(NIC, TRACE, "Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_BA_OFFLOAD_TAG_RX_ADDBA:{
			struct UNI_EVENT_RX_ADDBA *ba =
				(struct UNI_EVENT_RX_ADDBA *) tag;
			struct EVENT_RX_ADDBA legacy;

			legacy.ucStaRecIdx =
				secGetStaIdxByWlanIdx(ad, ba->u2WlanIdx);
			legacy.ucDialogToken = ba->ucDialogToken;
			legacy.u2BATimeoutValue = ba->u2BATimeoutValue;
			legacy.u2BAStartSeqCtrl = ba->u2BAStartSeqCtrl;
			legacy.u2BAParameterSet =
				(ba->ucTid << BA_PARAM_SET_TID_MASK_OFFSET) |
				(ba->u2WinSize <<
					BA_PARAM_SET_BUFFER_SIZE_MASK_OFFSET);

			RUN_RX_EVENT_HANDLER(EVENT_ID_RX_ADDBA, &legacy);
		}
			break;
		case UNI_EVENT_BA_OFFLOAD_TAG_RX_DELBA:{
			struct UNI_EVENT_RX_DELBA *ba =
				(struct UNI_EVENT_RX_DELBA *) tag;
			struct EVENT_RX_DELBA legacy;

			legacy.ucStaRecIdx =
				secGetStaIdxByWlanIdx(ad, ba->u2WlanIdx);
			legacy.ucTid = ba->ucTid;

			RUN_RX_EVENT_HANDLER(EVENT_ID_RX_DELBA, &legacy);
		}
			break;
		case UNI_EVENT_BA_OFFLOAD_TAG_TX_ADDBA:{
			struct UNI_EVENT_TX_ADDBA *ba =
				(struct UNI_EVENT_TX_ADDBA *) tag;
			struct EVENT_TX_ADDBA legacy;

			legacy.ucStaRecIdx =
				secGetStaIdxByWlanIdx(ad, ba->u2WlanIdx);
			legacy.ucTid = ba->ucTid;
			/* WARN: ba1024 win size is truncated, it's okay
			 * now because qmHandleEventTxAddBa doesn't use it
			 */
			legacy.ucWinSize = ba->u2WinSize;
			legacy.ucAmsduEnBitmap = ba->ucAmsduEnBitmap;
			legacy.u2SSN = ba->u2SSN;
			legacy.ucMaxMpduCount = ba->ucMaxMpduCount;
			legacy.u4MaxMpduLen = ba->u4MaxMpduLen;
			legacy.u4MinMpduLen = ba->u4MinMpduLen;

			RUN_RX_EVENT_HANDLER(EVENT_ID_TX_ADDBA, &legacy);
		}
			break;
		default:
			fail_cnt++;
			ASSERT(fail_cnt < MAX_UNI_EVENT_FAIL_TAG_COUNT)
			DBGLOG(NIC, WARN, "invalid tag = %d\n", TAG_ID(tag));
			break;
		}
	}
}

void nicUniEventSleepNotify(struct ADAPTER *ad, struct WIFI_UNI_EVENT *evt)
{
	int32_t tags_len;
	uint8_t *tag;
	uint16_t offset = 0;
	uint32_t fixed_len = sizeof(struct UNI_EVENT_SLEEP_NOTIFY);
	uint32_t data_len = GET_UNI_EVENT_DATA_LEN(evt);
	uint8_t *data = GET_UNI_EVENT_DATA(evt);
	uint32_t fail_cnt = 0;

	tags_len = data_len - fixed_len;
	tag = data + fixed_len;
	TAG_FOR_EACH(tag, tags_len, offset) {
		DBGLOG(NIC, TRACE, "Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_SLEEP_NOTYFY_TAG_SLEEP_INFO:{
			struct UNI_EVENT_SLEEP_INFO *info =
				(struct UNI_EVENT_SLEEP_INFO *) tag;
			struct EVENT_SLEEPY_INFO legacy;

			legacy.ucSleepyState = info->ucSleepyState;

			RUN_RX_EVENT_HANDLER(EVENT_ID_SLEEPY_INFO, &legacy);
		}
			break;
		default:
			fail_cnt++;
			ASSERT(fail_cnt < MAX_UNI_EVENT_FAIL_TAG_COUNT)
			DBGLOG(NIC, WARN, "invalid tag = %d\n", TAG_ID(tag));
			break;
		}
	}
}

void nicUniEventBeaconTimeout(struct ADAPTER *ad, struct WIFI_UNI_EVENT *evt)
{
	int32_t tags_len;
	uint8_t *tag;
	uint16_t offset = 0;
	uint32_t fixed_len = sizeof(struct UNI_EVENT_BEACON_TIMEOUT);
	uint32_t data_len = GET_UNI_EVENT_DATA_LEN(evt);
	uint8_t *data = GET_UNI_EVENT_DATA(evt);
	uint32_t fail_cnt = 0;
	struct UNI_EVENT_BEACON_TIMEOUT *timeout;
	struct EVENT_BSS_BEACON_TIMEOUT legacy;

	timeout = (struct UNI_EVENT_BEACON_TIMEOUT *) data;
	legacy.ucBssIndex = timeout->ucBssIndex;

	tags_len = data_len - fixed_len;
	tag = data + fixed_len;
	TAG_FOR_EACH(tag, tags_len, offset) {
		DBGLOG(NIC, TRACE, "Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_BEACON_TIMEOUT_TAG_INFO:{
			struct UNI_EVENT_BEACON_TIMEOUT_INFO *info =
				(struct UNI_EVENT_BEACON_TIMEOUT_INFO *) tag;

			legacy.ucReasonCode = info->ucReasonCode;

			RUN_RX_EVENT_HANDLER(EVENT_ID_BSS_BEACON_TIMEOUT,
					&legacy);
		}
			break;
		default:
			fail_cnt++;
			ASSERT(fail_cnt < MAX_UNI_EVENT_FAIL_TAG_COUNT)
			DBGLOG(NIC, WARN, "invalid tag = %d\n", TAG_ID(tag));
			break;
		}
	}
}

void nicUniEventUpdateCoex(struct ADAPTER *ad, struct WIFI_UNI_EVENT *evt)
{
	int32_t tags_len;
	uint8_t *tag;
	uint16_t offset = 0;
	uint32_t fixed_len = sizeof(struct UNI_EVENT_UPDATE_COEX);
	uint32_t data_len = GET_UNI_EVENT_DATA_LEN(evt);
	uint8_t *data = GET_UNI_EVENT_DATA(evt);
	uint32_t fail_cnt = 0;
	uint8_t i;

	tags_len = data_len - fixed_len;
	tag = data + fixed_len;
	TAG_FOR_EACH(tag, tags_len, offset) {
		DBGLOG(NIC, TRACE, "Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_UPDATE_COEX_TAG_PHYRATE:{
			struct UNI_EVENT_UPDATE_COEX_PHYRATE *phyrate =
				(struct UNI_EVENT_UPDATE_COEX_PHYRATE *) tag;
			struct EVENT_UPDATE_COEX_PHYRATE legacy;

			for (i = 0; i < MAX_BSSID_NUM + 1; i++) {
				legacy.au4PhyRateLimit[i] =
					phyrate->au4PhyRateLimit[i];
			}

			RUN_RX_EVENT_HANDLER(EVENT_ID_UPDATE_COEX_PHYRATE,
					&legacy);
		}
			break;
		default:
			fail_cnt++;
			ASSERT(fail_cnt < MAX_UNI_EVENT_FAIL_TAG_COUNT)
			DBGLOG(NIC, WARN, "invalid tag = %d\n", TAG_ID(tag));
			break;
		}
	}
}

void nicUniEventIdc(struct ADAPTER *ad, struct WIFI_UNI_EVENT *evt)
{
	int32_t tags_len;
	uint8_t *tag;
	uint16_t offset = 0;
	uint32_t fixed_len = sizeof(struct UNI_EVENT_IDC);
	uint32_t data_len = GET_UNI_EVENT_DATA_LEN(evt);
	uint8_t *data = GET_UNI_EVENT_DATA(evt);
	uint32_t fail_cnt = 0;

	tags_len = data_len - fixed_len;
	tag = data + fixed_len;
	TAG_FOR_EACH(tag, tags_len, offset) {
		DBGLOG(NIC, TRACE, "Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_IDC_TAG_MD_SAFE_CHN:{
			struct UNI_EVENT_MD_SAFE_CHN *chn =
				(struct UNI_EVENT_MD_SAFE_CHN *) tag;
			struct EVENT_LTE_SAFE_CHN legacy;

			legacy.ucVersion = chn->ucVersion;
			legacy.u4Flags = chn->u4Flags;
			kalMemCopy(legacy.rLteSafeChn.au4SafeChannelBitmask,
				chn->u4SafeChannelBitmask,
				sizeof(uint32_t) * ENUM_SAFE_CH_MASK_MAX_NUM);

#if CFG_SUPPORT_IDC_CH_SWITCH
			RUN_RX_EVENT_HANDLER(EVENT_ID_LTE_IDC_REPORT, &legacy);
#endif
		}
			break;
		case UNI_EVENT_IDC_TAG_CCCI_MSG:
			break;
		default:
			fail_cnt++;
			ASSERT(fail_cnt < MAX_UNI_EVENT_FAIL_TAG_COUNT)
			DBGLOG(NIC, WARN, "invalid tag = %d\n", TAG_ID(tag));
			break;
		}
	}
}

void nicUniEventBssIsAbsence(struct ADAPTER *ad, struct WIFI_UNI_EVENT *evt)
{
	int32_t tags_len;
	uint8_t *tag;
	uint16_t offset = 0;
	uint32_t fixed_len = sizeof(struct UNI_EVENT_BSS_IS_ABSENCE);
	uint32_t data_len = GET_UNI_EVENT_DATA_LEN(evt);
	uint8_t *data = GET_UNI_EVENT_DATA(evt);
	uint32_t fail_cnt = 0;
	struct UNI_EVENT_BSS_IS_ABSENCE *absence;

	absence = (struct UNI_EVENT_BSS_IS_ABSENCE *) data;

	tags_len = data_len - fixed_len;
	tag = data + fixed_len;
	TAG_FOR_EACH(tag, tags_len, offset) {
		DBGLOG(NIC, TRACE, "Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_BSS_IS_ABSENCE_TAG_INFO: {
			struct UNI_EVENT_BSS_IS_ABSENCE_INFO *info=
				(struct UNI_EVENT_BSS_IS_ABSENCE_INFO *) tag;
			struct EVENT_BSS_ABSENCE_PRESENCE legacy;

			legacy.ucBssIndex = absence->ucBssIndex;
			legacy.ucIsAbsent = info->ucIsAbsent;
			legacy.ucBssFreeQuota = info->ucBssFreeQuota;

			RUN_RX_EVENT_HANDLER(EVENT_ID_BSS_ABSENCE_PRESENCE,
								&legacy);
		}
			break;
		default:
			fail_cnt++;
			ASSERT(fail_cnt < MAX_UNI_EVENT_FAIL_TAG_COUNT)
			DBGLOG(NIC, WARN, "invalid tag = %d\n", TAG_ID(tag));
			break;
		}
	}
}

void nicUniEventPsSync(struct ADAPTER *ad, struct WIFI_UNI_EVENT *evt)
{
	int32_t tags_len;
	uint8_t *tag;
	uint16_t offset = 0;
	uint32_t fixed_len = sizeof(struct UNI_EVENT_PS_SYNC);
	uint32_t data_len = GET_UNI_EVENT_DATA_LEN(evt);
	uint8_t *data = GET_UNI_EVENT_DATA(evt);
	uint32_t fail_cnt = 0;

	tags_len = data_len - fixed_len;
	tag = data + fixed_len;
	TAG_FOR_EACH(tag, tags_len, offset) {
		DBGLOG(NIC, TRACE, "Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_PS_SYNC_TAG_CLIENT_PS_INFO: {
			struct UNI_EVENT_CLIENT_PS_INFO *ps =
				(struct UNI_EVENT_CLIENT_PS_INFO *) tag;
			struct EVENT_STA_CHANGE_PS_MODE legacy;

			legacy.ucStaRecIdx = ps->ucWtblIndex;
			legacy.ucIsInPs = ps->ucPsBit;
			legacy.ucFreeQuota = ps->ucBufferSize;

			RUN_RX_EVENT_HANDLER(EVENT_ID_STA_CHANGE_PS_MODE,
								&legacy);
		}
			break;
		default:
			fail_cnt++;
			ASSERT(fail_cnt < MAX_UNI_EVENT_FAIL_TAG_COUNT)
			DBGLOG(NIC, WARN, "invalid tag = %d\n", TAG_ID(tag));
			break;
		}
	}
}

void nicUniEventSap(struct ADAPTER *ad, struct WIFI_UNI_EVENT *evt)
{
	int32_t tags_len;
	uint8_t *tag;
	uint16_t offset = 0;
	uint32_t fixed_len = sizeof(struct UNI_EVENT_SAP);
	uint32_t data_len = GET_UNI_EVENT_DATA_LEN(evt);
	uint8_t *data = GET_UNI_EVENT_DATA(evt);
	uint32_t fail_cnt = 0;

	tags_len = data_len - fixed_len;
	tag = data + fixed_len;
	TAG_FOR_EACH(tag, tags_len, offset) {
		DBGLOG(NIC, TRACE, "Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_SAP_TAG_AGING_TIMEOUT: {
			struct UNI_EVENT_SAP_AGING_TIMEOUT *aging =
				(struct UNI_EVENT_SAP_AGING_TIMEOUT *) tag;
			struct EVENT_STA_AGING_TIMEOUT legacy;

			legacy.ucStaRecIdx = aging->u2StaRecIdx;

			RUN_RX_EVENT_HANDLER(EVENT_ID_STA_AGING_TIMEOUT,
								&legacy);
		}
			break;
		case UNI_EVENT_SAP_TAG_UPDATE_STA_FREE_QUOTA: {
			struct UNI_EVENT_UPDATE_STA_FREE_QUOTA *quota =
				(struct UNI_EVENT_UPDATE_STA_FREE_QUOTA *) tag;
			struct EVENT_STA_UPDATE_FREE_QUOTA legacy;

			legacy.ucStaRecIdx = quota->u2StaRecIdx;
			legacy.ucUpdateMode = quota->ucUpdateMode;
			legacy.ucFreeQuota = quota->ucFreeQuota;

			RUN_RX_EVENT_HANDLER(EVENT_ID_STA_UPDATE_FREE_QUOTA,
								&legacy);
		}
			break;
		default:
			fail_cnt++;
			ASSERT(fail_cnt < MAX_UNI_EVENT_FAIL_TAG_COUNT)
			DBGLOG(NIC, WARN, "invalid tag = %d\n", TAG_ID(tag));
			break;
		}
	}
}

void nicUniEventOBSS(struct ADAPTER *ad, struct WIFI_UNI_EVENT *evt)
{
	int32_t tags_len;
	uint8_t *tag;
	uint16_t offset = 0;
	uint32_t fixed_len = sizeof(struct UNI_EVENT_OBSS_UPDATE);
	uint32_t data_len = GET_UNI_EVENT_DATA_LEN(evt);
	uint8_t *data = GET_UNI_EVENT_DATA(evt);
	uint32_t fail_cnt = 0;
	struct UNI_EVENT_OBSS_UPDATE *obss;
	struct EVENT_AP_OBSS_STATUS legacy;

	obss = (struct UNI_EVENT_OBSS_UPDATE *) data;
	legacy.ucBssIndex = obss->ucBssIndex;

	tags_len = data_len - fixed_len;
	tag = data + fixed_len;
	TAG_FOR_EACH(tag, tags_len, offset) {
		DBGLOG(NIC, TRACE, "Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_OBSS_UPDATE_TAG_STATUS: {
			struct UNI_EVENT_OBSS_STATUS *status =
				(struct UNI_EVENT_OBSS_STATUS *) tag;

			legacy.ucObssErpProtectMode =
				status->ucObssErpProtectMode;
			legacy.ucObssHtProtectMode =
				status->ucObssHtProtectMode;
			legacy.ucObssGfOperationMode =
				status->ucObssGfOperationMode;
			legacy.ucObssRifsOperationMode =
				status->ucObssRifsOperationMode;
			legacy.ucObssBeaconForcedTo20M =
				status->ucObssBeaconForcedTo20M;

			RUN_RX_EVENT_HANDLER(EVENT_ID_AP_OBSS_STATUS,
								&legacy);
		}
			break;
		default:
			fail_cnt++;
			ASSERT(fail_cnt < MAX_UNI_EVENT_FAIL_TAG_COUNT)
			DBGLOG(NIC, WARN, "invalid tag = %d\n", TAG_ID(tag));
			break;
		}
	}
}

void nicUniEventRoaming(struct ADAPTER *ad, struct WIFI_UNI_EVENT *evt)
{
	int32_t tags_len;
	uint8_t *tag;
	uint16_t offset = 0;
	uint32_t fixed_len = sizeof(struct UNI_EVENT_ROAMING);
	uint32_t data_len = GET_UNI_EVENT_DATA_LEN(evt);
	uint8_t *data = GET_UNI_EVENT_DATA(evt);
	uint32_t fail_cnt = 0;
	struct UNI_EVENT_ROAMING *roam;
	struct CMD_ROAMING_TRANSIT legacy;

	roam = (struct UNI_EVENT_ROAMING *) data;
	legacy.ucBssidx = roam->ucBssIndex;

	tags_len = data_len - fixed_len;
	tag = data + fixed_len;
	TAG_FOR_EACH(tag, tags_len, offset) {
		DBGLOG(NIC, TRACE, "Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_ROAMING_TAG_STATUS: {
			struct UNI_EVENT_ROAMING_STATUS *status =
				(struct UNI_EVENT_ROAMING_STATUS *) tag;

			legacy.u2Event = status->u2Event;
			legacy.u2Data = status->u2Data;
			legacy.u2RcpiLowThreshold = status->u2RcpiLowThreshold;
			legacy.ucIsSupport11B = TRUE; /* unused */
			legacy.eReason = status->eReason;
			legacy.u4RoamingTriggerTime =
				status->u4RoamingTriggerTime;
			legacy.u2RcpiHighThreshold =
				status->u2RcpiHighThreshold;

			roamingFsmProcessEvent(ad, &legacy);
		}
			break;
		default:
			fail_cnt++;
			ASSERT(fail_cnt < MAX_UNI_EVENT_FAIL_TAG_COUNT)
			DBGLOG(NIC, WARN, "invalid tag = %d\n", TAG_ID(tag));
			break;
		}
	}
}

void nicUniEventAddKeyDone(struct ADAPTER *ad, struct WIFI_UNI_EVENT *evt)
{
	int32_t tags_len;
	uint8_t *tag;
	uint16_t offset = 0;
	uint32_t fixed_len = sizeof(struct UNI_EVENT_ADD_KEY_DONE);
	uint32_t data_len = GET_UNI_EVENT_DATA_LEN(evt);
	uint8_t *data = GET_UNI_EVENT_DATA(evt);
	uint32_t fail_cnt = 0;
	struct UNI_EVENT_ADD_KEY_DONE *done;
	struct EVENT_ADD_KEY_DONE_INFO legacy;

	done = (struct UNI_EVENT_ADD_KEY_DONE *) data;
	legacy.ucBSSIndex = done->ucBssIndex;

	tags_len = data_len - fixed_len;
	tag = data + fixed_len;
	TAG_FOR_EACH(tag, tags_len, offset) {
		DBGLOG(NIC, TRACE, "Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_ADD_KEY_DONE_TAG_PKEY: {
			struct UNI_EVENT_ADD_PKEY_DONE *pkey =
				(struct UNI_EVENT_ADD_PKEY_DONE *) tag;

			COPY_MAC_ADDR(legacy.aucStaAddr, pkey->aucStaAddr);

			RUN_RX_EVENT_HANDLER(EVENT_ID_ADD_PKEY_DONE, &legacy);
		}
			break;
		default:
			fail_cnt++;
			ASSERT(fail_cnt < MAX_UNI_EVENT_FAIL_TAG_COUNT)
			DBGLOG(NIC, WARN, "invalid tag = %d\n", TAG_ID(tag));
			break;
		}
	}
}

void nicUniEventFwLog2Host(struct ADAPTER *ad, struct WIFI_UNI_EVENT *evt)
{
	int32_t tags_len;
	uint8_t *tag;
	uint16_t offset = 0;
	uint32_t fixed_len = sizeof(struct UNI_EVENT_FW_LOG2HOST);
	uint32_t data_len = GET_UNI_EVENT_DATA_LEN(evt);
	uint8_t *data = GET_UNI_EVENT_DATA(evt);
	uint32_t fail_cnt = 0;

	tags_len = data_len - fixed_len;
	tag = data + fixed_len;
	TAG_FOR_EACH(tag, tags_len, offset) {
		DBGLOG(NIC, TRACE, "Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_FWLOG2HOST_TAG_FORMAT: {
			struct UNI_EVENT_FW_LOG_FORMAT *log =
				(struct UNI_EVENT_FW_LOG_FORMAT *) tag;
			struct EVENT_DEBUG_MSG *legacy;
			uint32_t len = log->u2Length - sizeof(*log);
			uint32_t size = sizeof(*legacy) + len;

			ASSERT(log->u2Length > sizeof(*log));

			legacy = (struct EVENT_DEBUG_MSG *) kalMemAlloc(
				size, VIR_MEM_TYPE);
			kalMemZero(legacy, sizeof(*legacy));

			legacy->ucMsgType = log->ucMsgFmt;
			legacy->u2MsgSize = len;
			kalMemCopy(legacy->aucMsg, log->acMsg, len);

			RUN_RX_EVENT_HANDLER_EXT(EVENT_ID_DEBUG_MSG,
							legacy, size);

			kalMemFree(legacy, size, VIR_MEM_TYPE);
		}
			break;
		default:
			fail_cnt++;
			ASSERT(fail_cnt < MAX_UNI_EVENT_FAIL_TAG_COUNT)
			DBGLOG(NIC, WARN, "invalid tag = %d\n", TAG_ID(tag));
			break;
		}
	}
}

void nicUniEventP2p(struct ADAPTER *ad, struct WIFI_UNI_EVENT *evt)
{
	int32_t tags_len;
	uint8_t *tag;
	uint16_t offset = 0;
	uint32_t fixed_len = sizeof(struct UNI_EVENT_P2P);
	uint32_t data_len = GET_UNI_EVENT_DATA_LEN(evt);
	uint8_t *data = GET_UNI_EVENT_DATA(evt);
	uint32_t fail_cnt = 0;

	tags_len = data_len - fixed_len;
	tag = data + fixed_len;
	TAG_FOR_EACH(tag, tags_len, offset) {
		DBGLOG(NIC, TRACE, "Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_P2P_TAG_UPDATE_NOA_PARAM: {
			struct UNI_EVENT_UPDATE_NOA_PARAM *noa =
				(struct UNI_EVENT_UPDATE_NOA_PARAM *) tag;
			struct EVENT_UPDATE_NOA_PARAMS legacy;
			uint8_t i;

			legacy.ucBssIndex = noa->ucBssIndex;
			legacy.ucEnableOppPS = noa->ucEnableOppPS;
			legacy.u2CTWindow = noa->u2CTWindow;
			legacy.ucNoAIndex = noa->ucNoAIndex;
			legacy.ucNoATimingCount = noa->ucNoATimingCount;

			for (i = 0; i < 8; i++) {
				legacy.arEventNoaTiming[i].ucIsInUse =
					noa->arEventNoaTiming[i].ucIsInUse;
				legacy.arEventNoaTiming[i].ucCount =
					noa->arEventNoaTiming[i].ucCount;
				legacy.arEventNoaTiming[i].u4Duration =
					noa->arEventNoaTiming[i].u4Duration;
				legacy.arEventNoaTiming[i].u4Interval =
					noa->arEventNoaTiming[i].u4Interval;
				legacy.arEventNoaTiming[i].u4StartTime =
					noa->arEventNoaTiming[i].u4StartTime;
			}

			RUN_RX_EVENT_HANDLER(EVENT_ID_UPDATE_NOA_PARAMS,
								&legacy);
		}
			break;
		default:
			fail_cnt++;
			ASSERT(fail_cnt < MAX_UNI_EVENT_FAIL_TAG_COUNT)
			DBGLOG(NIC, WARN, "invalid tag = %d\n", TAG_ID(tag));
			break;
		}
	}
}

void nicUniEventCountdown(struct ADAPTER *ad, struct WIFI_UNI_EVENT *evt)
{
	int32_t tags_len;
	uint8_t *tag;
	uint16_t offset = 0;
	uint32_t fixed_len = sizeof(struct UNI_EVENT_IE_COUNTDOWNT);
	uint32_t data_len = GET_UNI_EVENT_DATA_LEN(evt);
	uint8_t *data = GET_UNI_EVENT_DATA(evt);
	uint32_t fail_cnt = 0;

	tags_len = data_len - fixed_len;
	tag = data + fixed_len;
	TAG_FOR_EACH(tag, tags_len, offset) {
		DBGLOG(NIC, TRACE, "Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_IE_COUNTDOWN_CSA: {
			struct UNI_EVENT_CSA_NOTIFY *csa =
				(struct UNI_EVENT_CSA_NOTIFY *) tag;

			DBGLOG(CNM, INFO, "OM=%d, count=%d\n",
				csa->ucOwnMacIdx, csa->ucChannelSwitchCount);
			RUN_RX_EVENT_HANDLER_EXT(EVENT_ID_CSA_DONE, NULL, 0);
		}
			break;
		case UNI_EVENT_IE_COUNTDOWN_BCC: {
			// TODO: uni cmd
		}
			break;
		default:
			fail_cnt++;
			ASSERT(fail_cnt < MAX_UNI_EVENT_FAIL_TAG_COUNT)
			DBGLOG(NIC, WARN, "invalid tag = %d\n", TAG_ID(tag));
			break;
		}
	}
}

void nicUniEventStaRec(struct ADAPTER *ad, struct WIFI_UNI_EVENT *evt)
{
	int32_t tags_len;
	uint8_t *tag;
	uint16_t offset = 0;
	uint32_t fixed_len = sizeof(struct UNI_EVENT_STAREC);
	uint32_t data_len = GET_UNI_EVENT_DATA_LEN(evt);
	uint8_t *data = GET_UNI_EVENT_DATA(evt);
	uint32_t fail_cnt = 0;
	struct UNI_EVENT_STAREC *common;

	common = (struct UNI_EVENT_STAREC *) data;
	tags_len = data_len - fixed_len;
	tag = data + fixed_len;
	TAG_FOR_EACH(tag, tags_len, offset) {
		DBGLOG(NIC, TRACE, "Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_STAREC_TAG_UPDATE_MAX_AMSDU_LEN: {
			struct UNI_EVENT_STAREC_UPDATE_MAX_AMSDU_LEN *len =
			    (struct UNI_EVENT_STAREC_UPDATE_MAX_AMSDU_LEN *)tag;
			struct EXT_EVENT_MAX_AMSDU_LENGTH_UPDATE legacy;

			legacy.ucWlanIdx = common->u2WlanIdx;
			legacy.ucAmsduLen = len->u2AmsduLen;
			RUN_RX_EXT_EVENT_HANDLER(
				EXT_EVENT_ID_MAX_AMSDU_LENGTH_UPDATE, &legacy);
		}
			break;
		case UNI_EVENT_STAREC_TAG_PN_INFO: {
			// TODO: uni cmd
		}
			break;
		default:
			fail_cnt++;
			ASSERT(fail_cnt < MAX_UNI_EVENT_FAIL_TAG_COUNT)
			DBGLOG(NIC, WARN, "invalid tag = %d\n", TAG_ID(tag));
			break;
		}
	}
}

void nicUniEventTdls(struct ADAPTER *ad, struct WIFI_UNI_EVENT *evt)
{
	int32_t tags_len;
	uint8_t *tag;
	uint16_t offset = 0;
	uint32_t fixed_len = sizeof(struct UNI_EVENT_TDLS);
	uint32_t data_len = GET_UNI_EVENT_DATA_LEN(evt);
	uint8_t *data = GET_UNI_EVENT_DATA(evt);
	uint32_t fail_cnt = 0;

	tags_len = data_len - fixed_len;
	tag = data + fixed_len;
	TAG_FOR_EACH(tag, tags_len, offset) {
		DBGLOG(NIC, TRACE, "Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_TDLS_TAG_TEAR_DOWN: {
			struct UNI_EVENT_TDLS_TEAR_DOWN *down =
			    (struct UNI_EVENT_TDLS_TEAR_DOWN *)tag;
			struct TDLS_EVENT legacy;

			legacy.u4HostId = TDLS_HOST_EVENT_TEAR_DOWN;
			legacy.u4SubId = down->u4Subid;
			legacy.u4StaIdx = down->u4StaIdx;

			RUN_RX_EVENT_HANDLER(EVENT_ID_TDLS, &legacy);
		}
			break;
		default:
			fail_cnt++;
			ASSERT(fail_cnt < MAX_UNI_EVENT_FAIL_TAG_COUNT)
			DBGLOG(NIC, WARN, "invalid tag = %d\n", TAG_ID(tag));
			break;
		}
	}
}

void nicUniEventBssER(struct ADAPTER *ad, struct WIFI_UNI_EVENT *evt)
{
	int32_t tags_len;
	uint8_t *tag;
	uint16_t offset = 0;
	uint32_t fixed_len = sizeof(struct UNI_EVENT_BSS_ER);
	uint32_t data_len = GET_UNI_EVENT_DATA_LEN(evt);
	uint8_t *data = GET_UNI_EVENT_DATA(evt);
	uint32_t fail_cnt = 0;

	tags_len = data_len - fixed_len;
	tag = data + fixed_len;
	TAG_FOR_EACH(tag, tags_len, offset) {
		DBGLOG(NIC, TRACE, "Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_BSS_ER_TAG_TX_MODE: {

#if (CFG_SUPPORT_HE_ER == 1)
			struct UNI_EVENT_BSS_ER_TX_MODE *mode =
			    (struct UNI_EVENT_BSS_ER_TX_MODE *)tag;
			struct EVENT_ER_TX_MODE legacy;

			legacy.ucBssInfoIdx = mode->ucBssInfoIdx;
			legacy.ucErMode = mode->ucErMode;

			RUN_RX_EVENT_HANDLER(EVENT_ID_BSS_ER_TX_MODE, &legacy);
#endif
		}
			break;
		default:
			fail_cnt++;
			ASSERT(fail_cnt < MAX_UNI_EVENT_FAIL_TAG_COUNT)
			DBGLOG(NIC, WARN, "invalid tag = %d\n", TAG_ID(tag));
			break;
		}
	}
}

void nicUniEventRssiMonitor(struct ADAPTER *ad, struct WIFI_UNI_EVENT *evt)
{
	int32_t tags_len;
	uint8_t *tag;
	uint16_t offset = 0;
	uint32_t fixed_len = sizeof(struct UNI_EVENT_RSSI_MONITOR);
	uint32_t data_len = GET_UNI_EVENT_DATA_LEN(evt);
	uint8_t *data = GET_UNI_EVENT_DATA(evt);
	uint32_t fail_cnt = 0;

	tags_len = data_len - fixed_len;
	tag = data + fixed_len;
	TAG_FOR_EACH(tag, tags_len, offset) {
		DBGLOG(NIC, TRACE, "Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_RSSI_MONITOR_TAG_INFO: {
			struct UNI_EVENT_RSSI_MONITOR_INFO *info =
			    (struct UNI_EVENT_RSSI_MONITOR_INFO *)tag;

			RUN_RX_EVENT_HANDLER(EVENT_ID_RSSI_MONITOR,
				&info->cRssi);
		}
			break;
		default:
			fail_cnt++;
			ASSERT(fail_cnt < MAX_UNI_EVENT_FAIL_TAG_COUNT)
			DBGLOG(NIC, WARN, "invalid tag = %d\n", TAG_ID(tag));
			break;
		}
	}
}

void nicUniEventHifCtrl(struct ADAPTER *ad, struct WIFI_UNI_EVENT *evt)
{
	int32_t tags_len;
	uint8_t *tag;
	uint16_t offset = 0;
	uint32_t fixed_len = sizeof(struct UNI_EVENT_HIF_CTRL);
	uint32_t data_len = GET_UNI_EVENT_DATA_LEN(evt);
	uint8_t *data = GET_UNI_EVENT_DATA(evt);
	uint32_t fail_cnt = 0;

	tags_len = data_len - fixed_len;
	tag = data + fixed_len;
	TAG_FOR_EACH(tag, tags_len, offset) {
		DBGLOG(NIC, TRACE, "Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_HIF_CTRL_TAG_BASIC: {
			struct UNI_EVENT_HIF_CTRL_BASIC *basic =
			    (struct UNI_EVENT_HIF_CTRL_BASIC *)tag;
			struct EVENT_HIF_CTRL legacy;

			legacy.ucHifType = basic->ucHifType;
			legacy.ucHifTxTrafficStatus =
				basic->ucHifTxTrafficStatus;
			legacy.ucHifRxTrafficStatus =
				basic->ucHifRxTrafficStatus;
			legacy.ucHifSuspend = basic->ucHifSuspend;

			RUN_RX_EVENT_HANDLER(EVENT_ID_HIF_CTRL,	&legacy);
		}
			break;
		default:
			fail_cnt++;
			ASSERT(fail_cnt < MAX_UNI_EVENT_FAIL_TAG_COUNT)
			DBGLOG(NIC, WARN, "invalid tag = %d\n", TAG_ID(tag));
			break;
		}
	}
}

