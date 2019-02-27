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
** Id: //Department/DaVinci/BRANCHES/MT6620_WIFI_DRIVER_V2_3/mgmt/cnm.c#2
*/

/*! \file   "cnm.c"
*    \brief  Module of Concurrent Network Management
*
*    Module of Concurrent Network Management
*/


/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/
#include "precomp.h"

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/
#if CFG_SUPPORT_DBDC
#define DBDC_SWITCH_GUARD_TIME		(4*1000)	/*ms*/
#define DBDC_DISABLE_COUNTDOWN_TIME	(2*1000)	/*ms*/
#endif /*CFG_SUPPORT_DBDC*/

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/
#if CFG_SUPPORT_DBDC
enum ENUM_DBDC_GUARD_TIMER_T {
	ENUM_DBDC_GUARD_TIMER_NONE,
	ENUM_DBDC_GUARD_TIMER_SWITCH_GUARD_TIME,    /* Prevent switch too quick*/
	ENUM_DBDC_GUARD_TIMER_DISABLE_COUNT_DOWN,   /* Prevent continuously trigger by reconnection  */
	ENUM_DBDC_GUARD_TIMER_NUM
};

enum ENUM_DBDC_FSM_STATE_T {
	ENUM_DBDC_FSM_STATE_DISABLE_IDLE,
	ENUM_DBDC_FSM_STATE_WAIT_PROTOCOL_ENABLE,
	ENUM_DBDC_FSM_STATE_WAIT_HW_ENABLE,
	ENUM_DBDC_FSM_STATE_ENABLE_GUARD,
	ENUM_DBDC_FSM_STATE_ENABLE_IDLE,
	ENUM_DBDC_FSM_STATE_WAIT_HW_DISABLE,
	ENUM_DBDC_FSM_STATE_DISABLE_GUARD,
	ENUM_DBDC_FSM_STATE_WAIT_PROTOCOL_DISABLE,
	ENUM_DBDC_FSM_STATE_NUM
};

enum ENUM_OPMODE_STATE_T {
	ENUM_OPMODE_STATE_DONE,
	ENUM_OPMODE_STATE_FAIL,
	ENUM_OPMODE_STATE_WAIT,
	ENUM_OPMODE_STATE_NUM
};

struct DBDC_INFO_T {
	enum ENUM_DBDC_FSM_STATE_T       eDbdcFsmCurrState;
	enum ENUM_DBDC_FSM_STATE_T       eDbdcFsmPrevState;
	enum ENUM_DBDC_FSM_STATE_T       eDbdcFsmNextState;

	TIMER_T                          rDbdcGuardTimer;
	enum ENUM_DBDC_GUARD_TIMER_T     eDdbcGuardTimerType;

	BOOLEAN                          fgReqPrivelegeLock;
	LINK_T                           rPendingMsgList;

	BOOLEAN                          fgDbdcDisableOpmodeChangeDone;
	enum ENUM_OPMODE_STATE_T         eBssOpModeState[BSSID_NUM];
};

enum ENUM_DBDC_FSM_EVENT_T {
	DBDC_FSM_EVENT_BSS_DISCONNECT_LEAVE_AG,
	DBDC_FSM_EVENT_BSS_CONNECTING_ENTER_AG,
	DBDC_FSM_EVENT_SWITCH_GUARD_TIME_TO,
	DBDC_FSM_EVENT_DISABLE_COUNT_DOWN_TO,
	DBDC_FSM_EVENT_ACTION_FRAME_ALL_SUCCESS,
	DBDC_FSM_EVENT_ACTION_FRAME_SOME_FAIL,
	DBDC_FSM_EVENT_DBDC_HW_SWITCH_DONE,
	DBDC_FSM_EVENT_ERR,
	DBDC_FSM_EVENT_NUM
};

enum ENUM_DBDC_PROTOCOL_STATUS_T {
	ENUM_DBDC_PROTOCOL_STATUS_DONE_SUCCESS = 0,
	ENUM_DBDC_PROTOCOL_STATUS_DONE_FAIL,
	ENUM_DBDC_PROTOCOL_STATUS_WAIT,
	ENUM_DBDC_PROTOCOL_STATUS_NUM
};

typedef VOID (*DBDC_ENTRY_FUNC)(P_ADAPTER_T);
typedef VOID (*DBDC_EVENT_HNDL_FUNC)(P_ADAPTER_T, enum ENUM_DBDC_FSM_EVENT_T);
typedef VOID (*DBDC_EXIT_FUNC)(P_ADAPTER_T);

struct DBDC_FSM_T {
	DBDC_ENTRY_FUNC               pfEntryFunc;
	DBDC_EVENT_HNDL_FUNC          pfEventHandlerFunc;
	DBDC_EXIT_FUNC                pfExitFunc;
};

#endif /*CFG_SUPPORT_DBDC*/

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/
#if CFG_SUPPORT_DBDC
static struct DBDC_INFO_T g_rDbdcInfo;
#endif

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/

#if CFG_SUPPORT_DBDC
#define DBDC_IS_BSS_ALIVE(_prBssInfo) \
	(_prBssInfo->fgIsInUse && \
	_prBssInfo->fgIsNetActive && \
	(_prBssInfo->eConnectionState == PARAM_MEDIA_STATE_CONNECTED || \
	_prBssInfo->eCurrentOPMode == OP_MODE_ACCESS_POINT))

#define DBDC_IS_BSS_NOT_ALIVE(_prBssInfo) \
	(!_prBssInfo->fgIsInUse || \
	!_prBssInfo->fgIsNetActive || \
	(_prBssInfo->eConnectionState != PARAM_MEDIA_STATE_CONNECTED && \
	_prBssInfo->eCurrentOPMode != OP_MODE_ACCESS_POINT))

#define DBDC_SET_GUARD_TIME(_prAdapter) { \
	cnmTimerStartTimer(_prAdapter, \
						&g_rDbdcInfo.rDbdcGuardTimer, \
						DBDC_SWITCH_GUARD_TIME); \
	g_rDbdcInfo.eDdbcGuardTimerType = \
						ENUM_DBDC_GUARD_TIMER_SWITCH_GUARD_TIME; \
	}

#define DBDC_SET_DISABLE_COUNTDOWN(_prAdapter) { \
	cnmTimerStartTimer(_prAdapter, \
						&g_rDbdcInfo.rDbdcGuardTimer, \
						DBDC_DISABLE_COUNTDOWN_TIME); \
	g_rDbdcInfo.eDdbcGuardTimerType = \
						ENUM_DBDC_GUARD_TIMER_DISABLE_COUNT_DOWN; \
	}

#define DBDC_FSM_MSG_WRONG_EVT(_eEvent) \
	DBGLOG(CNM, WARN, "[DBDC] Should not reveice evt %u during state %u\n", \
				_eEvent, \
				g_rDbdcInfo.eDbdcFsmCurrState)

#define DBDC_FSM_MSG_ERROR_EVT(_eEvent) \
	DBGLOG(CNM, ERROR, "[DBDC] Reveice evt %u during state %u\n", \
				_eEvent, \
				g_rDbdcInfo.eDbdcFsmCurrState) \

#define USE_DBDC_CAPABILITY() \
	((g_rDbdcInfo.eDbdcFsmCurrState == ENUM_DBDC_FSM_STATE_WAIT_PROTOCOL_ENABLE || \
	g_rDbdcInfo.eDbdcFsmCurrState == ENUM_DBDC_FSM_STATE_WAIT_HW_ENABLE || \
	g_rDbdcInfo.eDbdcFsmCurrState == ENUM_DBDC_FSM_STATE_ENABLE_GUARD || \
	g_rDbdcInfo.eDbdcFsmCurrState == ENUM_DBDC_FSM_STATE_ENABLE_IDLE)?TRUE:FALSE)

#endif

/*******************************************************************************
*                   F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/
static BOOLEAN
cnmDBDCIsReqPeivilegeLock(
	VOID
	);

static VOID
cnmDbdcFsmEntryFunc_DISABLE_IDLE(
	IN P_ADAPTER_T	prAdapter
	);

static VOID
cnmDbdcFsmEntryFunc_WAIT_PROTOCOL_ENABLE(
	IN P_ADAPTER_T	prAdapter
	);

static VOID
cnmDbdcFsmEntryFunc_WAIT_HW_ENABLE(
	IN P_ADAPTER_T	prAdapter
	);

static VOID
cnmDbdcFsmEntryFunc_ENABLE_GUARD(
	IN P_ADAPTER_T	prAdapter
	);

static VOID
cnmDbdcFsmEntryFunc_WAIT_HW_DISABLE(
	IN P_ADAPTER_T	prAdapter
	);

static VOID
cnmDbdcFsmEntryFunc_DISABLE_GUARD(
	IN P_ADAPTER_T	prAdapter
	);

static VOID
cnmDbdcFsmEventHandler_DISABLE_IDLE(
	IN P_ADAPTER_T	prAdapter,
	IN enum ENUM_DBDC_FSM_EVENT_T	eEvent
	);

static VOID
cnmDbdcFsmEventHandler_WAIT_PROTOCOL_ENABLE(
	IN P_ADAPTER_T	prAdapter,
	IN enum ENUM_DBDC_FSM_EVENT_T	eEvent
	);

static VOID
cnmDbdcFsmEventHandler_WAIT_HW_ENABLE(
	IN P_ADAPTER_T	prAdapter,
	IN enum ENUM_DBDC_FSM_EVENT_T	eEvent
	);

static VOID
cnmDbdcFsmEventHandler_ENABLE_GUARD(
	IN P_ADAPTER_T                  prAdapter,
	IN enum ENUM_DBDC_FSM_EVENT_T   eEvent
	);

static VOID
cnmDbdcFsmEventHandler_ENABLE_IDLE(
	IN P_ADAPTER_T                  prAdapter,
	IN enum ENUM_DBDC_FSM_EVENT_T   eEvent
	);

static VOID
cnmDbdcFsmEventHandler_WAIT_HW_DISABLE(
	IN P_ADAPTER_T                  prAdapter,
	IN enum ENUM_DBDC_FSM_EVENT_T   eEvent
	);

static VOID
cnmDbdcFsmEventHandler_DISABLE_GUARD(
	IN P_ADAPTER_T                  prAdapter,
	IN enum ENUM_DBDC_FSM_EVENT_T   eEvent
	);

static VOID
cnmDbdcFsmEventHandler_WAIT_PROTOCOL_DISABLE(
	IN P_ADAPTER_T                  prAdapter,
	IN enum ENUM_DBDC_FSM_EVENT_T   eEvent
	);

static VOID
cnmDbdcFsmExitFunc_WAIT_HW_ENABLE(
	IN P_ADAPTER_T	prAdapter
	);

/*******************************************************************************
*                           P R I V A T E   D A T A 2
********************************************************************************
*/

static struct DBDC_FSM_T arDdbcFsmActionTable[] = {
	/* ENUM_DBDC_FSM_STATE_DISABLE_IDLE */
	{cnmDbdcFsmEntryFunc_DISABLE_IDLE,
	cnmDbdcFsmEventHandler_DISABLE_IDLE,
	NULL},

	/* ENUM_DBDC_FSM_STATE_WAIT_PROTOCOL_ENABLE */
	{cnmDbdcFsmEntryFunc_WAIT_PROTOCOL_ENABLE,
	cnmDbdcFsmEventHandler_WAIT_PROTOCOL_ENABLE,
	NULL},

	/* ENUM_DBDC_FSM_STATE_WAIT_HW_ENABLE */
	{cnmDbdcFsmEntryFunc_WAIT_HW_ENABLE,
	cnmDbdcFsmEventHandler_WAIT_HW_ENABLE,
	cnmDbdcFsmExitFunc_WAIT_HW_ENABLE},

	/* ENUM_DBDC_FSM_STATE_ENABLE_GUARD */
	{cnmDbdcFsmEntryFunc_ENABLE_GUARD,
	cnmDbdcFsmEventHandler_ENABLE_GUARD,
	NULL},

	/* ENUM_DBDC_FSM_STATE_ENABLE_IDLE */
	{NULL,
	cnmDbdcFsmEventHandler_ENABLE_IDLE,
	NULL},

	/* ENUM_DBDC_FSM_STATE_WAIT_HW_DISABLE */
	{cnmDbdcFsmEntryFunc_WAIT_HW_DISABLE,
	cnmDbdcFsmEventHandler_WAIT_HW_DISABLE,
	NULL},

	/* ENUM_DBDC_FSM_STATE_DISABLE_GUARD */
	{cnmDbdcFsmEntryFunc_DISABLE_GUARD,
	cnmDbdcFsmEventHandler_DISABLE_GUARD,
	NULL},

	/* ENUM_DBDC_FSM_STATE_WAIT_PROTOCOL_DISABLE */
	{NULL,
	cnmDbdcFsmEventHandler_WAIT_PROTOCOL_DISABLE,
	NULL},
};

/*******************************************************************************
*                                 M A C R O S 2
********************************************************************************
*/
#define DBDC_FSM_EVENT_HANDLER(_prAdapter, _event) { \
		arDdbcFsmActionTable[g_rDbdcInfo.eDbdcFsmCurrState].pfEventHandlerFunc(_prAdapter, _event); \
	}

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is used to initialize variables in CNM_INFO_T.
*
* @param (none)
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
VOID cnmInit(P_ADAPTER_T prAdapter)
{
	P_CNM_INFO_T prCnmInfo;

	ASSERT(prAdapter);

	prCnmInfo = &prAdapter->rCnmInfo;
	prCnmInfo->fgChGranted = FALSE;
}				/* end of cnmInit()*/

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is used to initialize variables in CNM_INFO_T.
*
* @param (none)
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
VOID cnmUninit(P_ADAPTER_T prAdapter)
{
	cnmTimerStopTimer(prAdapter, &g_rDbdcInfo.rDbdcGuardTimer);
}				/* end of cnmUninit()*/

/*----------------------------------------------------------------------------*/
/*!
* @brief Before handle the message from other module, it need to obtain
*        the Channel privilege from Channel Manager
*
* @param[in] prMsgHdr   The message need to be handled.
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
VOID cnmChMngrRequestPrivilege(P_ADAPTER_T prAdapter, P_MSG_HDR_T prMsgHdr)
{
	P_MSG_CH_REQ_T prMsgChReq;
	P_CMD_CH_PRIVILEGE_T prCmdBody;
	WLAN_STATUS rStatus;

	ASSERT(prAdapter);
	ASSERT(prMsgHdr);

	prMsgChReq = (P_MSG_CH_REQ_T)prMsgHdr;

#if CFG_SUPPORT_DBDC
	if (cnmDBDCIsReqPeivilegeLock()) {
		LINK_INSERT_TAIL(&g_rDbdcInfo.rPendingMsgList, &prMsgHdr->rLinkEntry);
		DBGLOG(CNM, INFO, "[DBDC] ChReq: queued BSS %u Token %u REQ\n",
		       prMsgChReq->ucBssIndex, prMsgChReq->ucTokenID);
		return;
	}
#endif

	prCmdBody = (P_CMD_CH_PRIVILEGE_T)
	    cnmMemAlloc(prAdapter, RAM_TYPE_BUF, sizeof(CMD_CH_PRIVILEGE_T));
	ASSERT(prCmdBody);

	/* To do: exception handle */
	if (!prCmdBody) {
		DBGLOG(CNM, ERROR, "ChReq: fail to get buf (net=%d, token=%d)\n",
		       prMsgChReq->ucBssIndex, prMsgChReq->ucTokenID);

		cnmMemFree(prAdapter, prMsgHdr);
		return;
	}

	DBGLOG(CNM, INFO, "ChReq net=%d token=%d b=%d c=%d s=%d w=%d s1=%d s2=%d\n",
	       prMsgChReq->ucBssIndex, prMsgChReq->ucTokenID,
	       prMsgChReq->eRfBand, prMsgChReq->ucPrimaryChannel, prMsgChReq->eRfSco, prMsgChReq->eRfChannelWidth,
	       prMsgChReq->ucRfCenterFreqSeg1, prMsgChReq->ucRfCenterFreqSeg2);

	prCmdBody->ucBssIndex = prMsgChReq->ucBssIndex;
	prCmdBody->ucTokenID = prMsgChReq->ucTokenID;
	prCmdBody->ucAction = CMD_CH_ACTION_REQ;	/* Request */
	prCmdBody->ucPrimaryChannel = prMsgChReq->ucPrimaryChannel;
	prCmdBody->ucRfSco = (UINT_8)prMsgChReq->eRfSco;
	prCmdBody->ucRfBand = (UINT_8)prMsgChReq->eRfBand;
	prCmdBody->ucRfChannelWidth = (UINT_8)prMsgChReq->eRfChannelWidth;
	prCmdBody->ucRfCenterFreqSeg1 = (UINT_8)prMsgChReq->ucRfCenterFreqSeg1;
	prCmdBody->ucRfCenterFreqSeg2 = (UINT_8)prMsgChReq->ucRfCenterFreqSeg2;
	prCmdBody->ucReqType = (UINT_8)prMsgChReq->eReqType;
	prCmdBody->ucDBDCBand = (UINT_8)prMsgChReq->eDBDCBand;
	prCmdBody->aucReserved = 0;
	prCmdBody->u4MaxInterval = prMsgChReq->u4MaxInterval;
	prCmdBody->aucReserved2[0] = 0;
	prCmdBody->aucReserved2[1] = 0;
	prCmdBody->aucReserved2[2] = 0;
	prCmdBody->aucReserved2[3] = 0;
	prCmdBody->aucReserved2[4] = 0;
	prCmdBody->aucReserved2[5] = 0;
	prCmdBody->aucReserved2[6] = 0;
	prCmdBody->aucReserved2[7] = 0;

	ASSERT(prCmdBody->ucBssIndex <= prAdapter->ucHwBssIdNum);

	/* For monkey testing 20110901 */
	if (prCmdBody->ucBssIndex > prAdapter->ucHwBssIdNum)
		DBGLOG(CNM, ERROR, "CNM: ChReq with wrong netIdx=%d\n\n", prCmdBody->ucBssIndex);

	rStatus = wlanSendSetQueryCmd(prAdapter,	/* prAdapter */
				      CMD_ID_CH_PRIVILEGE,	/* ucCID */
				      TRUE,	/* fgSetQuery */
				      FALSE,	/* fgNeedResp */
				      FALSE,	/* fgIsOid */
				      NULL,	/* pfCmdDoneHandler */
				      NULL,	/* pfCmdTimeoutHandler */
				      sizeof(CMD_CH_PRIVILEGE_T),	/* u4SetQueryInfoLen */
				      (PUINT_8)prCmdBody,	/* pucInfoBuffer */
				      NULL,	/* pvSetQueryBuffer */
				      0	/* u4SetQueryBufferLen */
	   );

	/* ASSERT(rStatus == WLAN_STATUS_PENDING); */

	cnmMemFree(prAdapter, prCmdBody);
	cnmMemFree(prAdapter, prMsgHdr);
}				/* end of cnmChMngrRequestPrivilege()*/

/*----------------------------------------------------------------------------*/
/*!
* @brief Before deliver the message to other module, it need to release
*        the Channel privilege to Channel Manager.
*
* @param[in] prMsgHdr   The message need to be delivered
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
VOID cnmChMngrAbortPrivilege(P_ADAPTER_T prAdapter, P_MSG_HDR_T prMsgHdr)
{
	P_MSG_CH_ABORT_T prMsgChAbort;
	P_CMD_CH_PRIVILEGE_T prCmdBody;
	P_CNM_INFO_T prCnmInfo;
	WLAN_STATUS rStatus;
#if CFG_SISO_SW_DEVELOP
	P_BSS_INFO_T prBssInfo;
#endif
#if CFG_SUPPORT_DBDC
	P_LINK_ENTRY_T prLinkEntry_pendingMsg;
	P_MSG_CH_REQ_T prPendingMsg;
#endif

	ASSERT(prAdapter);
	ASSERT(prMsgHdr);

	prMsgChAbort = (P_MSG_CH_ABORT_T)prMsgHdr;

#if CFG_SUPPORT_DBDC
	if (cnmDBDCIsReqPeivilegeLock()) {
		LINK_FOR_EACH(prLinkEntry_pendingMsg, &g_rDbdcInfo.rPendingMsgList) {
			prPendingMsg = (P_MSG_CH_REQ_T)
							LINK_ENTRY(prLinkEntry_pendingMsg, MSG_HDR_T, rLinkEntry);

			/* Find matched request and check if it is being served. */
			if (prPendingMsg->ucBssIndex == prMsgChAbort->ucBssIndex &&
				prPendingMsg->ucTokenID == prMsgChAbort->ucTokenID) {

				LINK_REMOVE_KNOWN_ENTRY(&g_rDbdcInfo.rPendingMsgList,
									&prPendingMsg->rMsgHdr.rLinkEntry);

				DBGLOG(CNM, INFO, "[DBDC] ChAbort: remove BSS %u Token %u REQ)\n",
								prPendingMsg->ucBssIndex, prPendingMsg->ucTokenID);

				cnmMemFree(prAdapter, prPendingMsg);
				cnmMemFree(prAdapter, prMsgHdr);

				return;
		    }
		}
	}
#endif

	/* Check if being granted channel privilege is aborted */
	prCnmInfo = &prAdapter->rCnmInfo;
	if (prCnmInfo->fgChGranted &&
	    prCnmInfo->ucBssIndex == prMsgChAbort->ucBssIndex && prCnmInfo->ucTokenID == prMsgChAbort->ucTokenID) {

		prCnmInfo->fgChGranted = FALSE;
	}

	prCmdBody = (P_CMD_CH_PRIVILEGE_T)
	    cnmMemAlloc(prAdapter, RAM_TYPE_BUF, sizeof(CMD_CH_PRIVILEGE_T));
	ASSERT(prCmdBody);

	/* To do: exception handle */
	if (!prCmdBody) {
		DBGLOG(CNM, ERROR, "ChAbort: fail to get buf (net=%d, token=%d)\n",
		       prMsgChAbort->ucBssIndex, prMsgChAbort->ucTokenID);

		cnmMemFree(prAdapter, prMsgHdr);
		return;
	}

	prCmdBody->ucBssIndex = prMsgChAbort->ucBssIndex;
	prCmdBody->ucTokenID = prMsgChAbort->ucTokenID;
	prCmdBody->ucAction = CMD_CH_ACTION_ABORT;	/* Abort */
	prCmdBody->ucDBDCBand = (UINT_8)prMsgChAbort->eDBDCBand;

	DBGLOG(CNM, INFO, "ChAbort net=%d token=%d dbdc=%u\n",
		prCmdBody->ucBssIndex, prCmdBody->ucTokenID, prCmdBody->ucDBDCBand);

	ASSERT(prCmdBody->ucBssIndex <= prAdapter->ucHwBssIdNum);

	/* For monkey testing 20110901 */
	if (prCmdBody->ucBssIndex > prAdapter->ucHwBssIdNum)
		DBGLOG(CNM, ERROR, "CNM: ChAbort with wrong netIdx=%d\n\n", prCmdBody->ucBssIndex);

	rStatus = wlanSendSetQueryCmd(prAdapter,	/* prAdapter */
				      CMD_ID_CH_PRIVILEGE,	/* ucCID */
				      TRUE,	/* fgSetQuery */
				      FALSE,	/* fgNeedResp */
				      FALSE,	/* fgIsOid */
				      NULL,	/* pfCmdDoneHandler */
				      NULL,	/* pfCmdTimeoutHandler */
				      sizeof(CMD_CH_PRIVILEGE_T),	/* u4SetQueryInfoLen */
				      (PUINT_8)prCmdBody,	/* pucInfoBuffer */
				      NULL,	/* pvSetQueryBuffer */
				      0	/* u4SetQueryBufferLen */
	   );

	/* ASSERT(rStatus == WLAN_STATUS_PENDING); */

#if CFG_SISO_SW_DEVELOP
	prBssInfo = prAdapter->aprBssInfo[prMsgChAbort->ucBssIndex];
	/* Driver clear granted CH in BSS info */
	prBssInfo->fgIsGranted = FALSE;
	prBssInfo->eBandGranted = BAND_NULL;
	prBssInfo->ucPrimaryChannelGranted = 0;
#endif

	cnmMemFree(prAdapter, prCmdBody);
	cnmMemFree(prAdapter, prMsgHdr);
}				/* end of cnmChMngrAbortPrivilege()*/

/*----------------------------------------------------------------------------*/
/*!
* @brief
*
* @param (none)
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
VOID cnmChMngrHandleChEvent(P_ADAPTER_T prAdapter, P_WIFI_EVENT_T prEvent)
{
	P_EVENT_CH_PRIVILEGE_T prEventBody;
	P_MSG_CH_GRANT_T prChResp;
	P_BSS_INFO_T prBssInfo;
	P_CNM_INFO_T prCnmInfo;

	ASSERT(prAdapter);
	ASSERT(prEvent);

	prEventBody = (P_EVENT_CH_PRIVILEGE_T)(prEvent->aucBuffer);
	prChResp = (P_MSG_CH_GRANT_T)
	    cnmMemAlloc(prAdapter, RAM_TYPE_MSG, sizeof(MSG_CH_GRANT_T));
	ASSERT(prChResp);

	/* To do: exception handle */
	if (!prChResp) {
		DBGLOG(CNM, ERROR, "ChGrant: fail to get buf (net=%d, token=%d)\n",
		       prEventBody->ucBssIndex, prEventBody->ucTokenID);

		return;
	}

	DBGLOG(CNM, INFO, "ChGrant net=%d token=%d ch=%d sco=%d u4GrantInterval=%d\n",
	       prEventBody->ucBssIndex, prEventBody->ucTokenID, prEventBody->ucPrimaryChannel,
	       prEventBody->ucRfSco, prEventBody->u4GrantInterval);

	ASSERT(prEventBody->ucBssIndex <= prAdapter->ucHwBssIdNum);
	ASSERT(prEventBody->ucStatus == EVENT_CH_STATUS_GRANT);

	prBssInfo = prAdapter->aprBssInfo[prEventBody->ucBssIndex];

	/* Decide message ID based on network and response status */
	if (IS_BSS_AIS(prBssInfo))
		prChResp->rMsgHdr.eMsgId = MID_CNM_AIS_CH_GRANT;
#if CFG_ENABLE_WIFI_DIRECT
	else if (prAdapter->fgIsP2PRegistered && IS_BSS_P2P(prBssInfo))
		prChResp->rMsgHdr.eMsgId = MID_CNM_P2P_CH_GRANT;
#endif
#if CFG_ENABLE_BT_OVER_WIFI
	else if (IS_BSS_BOW(prBssInfo))
		prChResp->rMsgHdr.eMsgId = MID_CNM_BOW_CH_GRANT;
#endif
	else {
		cnmMemFree(prAdapter, prChResp);
		return;
	}

	prChResp->ucBssIndex = prEventBody->ucBssIndex;
	prChResp->ucTokenID = prEventBody->ucTokenID;
	prChResp->ucPrimaryChannel = prEventBody->ucPrimaryChannel;
	prChResp->eRfSco = (ENUM_CHNL_EXT_T)prEventBody->ucRfSco;
	prChResp->eRfBand = (ENUM_BAND_T)prEventBody->ucRfBand;
	prChResp->eRfChannelWidth = (ENUM_CHANNEL_WIDTH_T)prEventBody->ucRfChannelWidth;
	prChResp->ucRfCenterFreqSeg1 = prEventBody->ucRfCenterFreqSeg1;
	prChResp->ucRfCenterFreqSeg2 = prEventBody->ucRfCenterFreqSeg2;
	prChResp->eReqType = (ENUM_CH_REQ_TYPE_T)prEventBody->ucReqType;
	prChResp->eDBDCBand = (ENUM_DBDC_BN_T)prEventBody->ucDBDCBand;
	prChResp->u4GrantInterval = prEventBody->u4GrantInterval;

	mboxSendMsg(prAdapter, MBOX_ID_0, (P_MSG_HDR_T)prChResp, MSG_SEND_METHOD_BUF);

	/* Record current granted BSS for TXM's reference */
	prCnmInfo = &prAdapter->rCnmInfo;
	prCnmInfo->ucBssIndex = prEventBody->ucBssIndex;
	prCnmInfo->ucTokenID = prEventBody->ucTokenID;
	prCnmInfo->fgChGranted = TRUE;
}

#if (CFG_SUPPORT_DFS_MASTER == 1)
VOID cnmRadarDetectEvent(IN P_ADAPTER_T prAdapter, IN P_WIFI_EVENT_T prEvent)
{
	P_EVENT_RDD_REPORT_T prEventBody;
	P_BSS_INFO_T prBssInfo;
	P_MSG_P2P_RADAR_DETECT_T prP2pRddDetMsg;
	UINT_8 ucBssIndex;

	DBGLOG(CNM, INFO, "cnmRadarDetectEvent.\n");

	prEventBody = (P_EVENT_RDD_REPORT_T)(prEvent->aucBuffer);

	prP2pRddDetMsg = (P_MSG_P2P_RADAR_DETECT_T) cnmMemAlloc(prAdapter,
					RAM_TYPE_MSG, sizeof(*prP2pRddDetMsg));

	if (!prP2pRddDetMsg) {
		DBGLOG(CNM, ERROR, "cnmMemAlloc for prP2pRddDetMsg failed!\n");
		return;
	}

	prP2pRddDetMsg->rMsgHdr.eMsgId = MID_CNM_P2P_RADAR_DETECT;

	for (ucBssIndex = 0; ucBssIndex < BSS_DEFAULT_NUM; ucBssIndex++) {
		prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, ucBssIndex);

		if (prBssInfo && prBssInfo->fgIsDfsActive) {
			prP2pRddDetMsg->ucBssIndex = ucBssIndex;
			break;
		}
	}

	p2pFuncSetDfsState(DFS_STATE_DETECTED);

	p2pFuncRadarInfoInit();

	g_rP2pRadarInfo.ucRadarReportMode = prEventBody->ucRadarReportMode;
	g_rP2pRadarInfo.ucRddIdx = prEventBody->ucRddIdx;
	g_rP2pRadarInfo.ucLongDetected = prEventBody->ucLongDetected;
	g_rP2pRadarInfo.ucPeriodicDetected = prEventBody->ucPeriodicDetected;
	g_rP2pRadarInfo.ucLPBNum = prEventBody->ucLPBNum;
	g_rP2pRadarInfo.ucPPBNum = prEventBody->ucPPBNum;
	g_rP2pRadarInfo.ucLPBPeriodValid = prEventBody->ucLPBPeriodValid;
	g_rP2pRadarInfo.ucLPBWidthValid = prEventBody->ucLPBWidthValid;
	g_rP2pRadarInfo.ucPRICountM1 = prEventBody->ucPRICountM1;
	g_rP2pRadarInfo.ucPRICountM1TH = prEventBody->ucPRICountM1TH;
	g_rP2pRadarInfo.ucPRICountM2 = prEventBody->ucPRICountM2;
	g_rP2pRadarInfo.ucPRICountM2TH = prEventBody->ucPRICountM2TH;
	g_rP2pRadarInfo.u4PRI1stUs = prEventBody->u4PRI1stUs;
	kalMemCopy(&g_rP2pRadarInfo.arLpbContent[0], &prEventBody->arLpbContent[0],
				prEventBody->ucLPBNum*sizeof(LONG_PULSE_BUFFER_T));
	kalMemCopy(&g_rP2pRadarInfo.arPpbContent[0], &prEventBody->arPpbContent[0],
				prEventBody->ucPPBNum*sizeof(PERIODIC_PULSE_BUFFER_T));

	mboxSendMsg(prAdapter, MBOX_ID_0, (P_MSG_HDR_T)prP2pRddDetMsg, MSG_SEND_METHOD_BUF);
}

VOID cnmCsaDoneEvent(IN P_ADAPTER_T prAdapter, IN P_WIFI_EVENT_T prEvent)
{
	P_BSS_INFO_T prBssInfo;
	P_MSG_P2P_CSA_DONE_T prP2pCsaDoneMsg;
	UINT_8 ucBssIndex;

	DBGLOG(CNM, INFO, "cnmCsaDoneEvent.\n");

	prP2pCsaDoneMsg = (P_MSG_P2P_CSA_DONE_T) cnmMemAlloc(prAdapter,
					RAM_TYPE_MSG, sizeof(*prP2pCsaDoneMsg));

	if (!prP2pCsaDoneMsg) {
		DBGLOG(CNM, ERROR, "cnmMemAlloc for prP2pCsaDoneMsg failed!\n");
		return;
	}

	prAdapter->rWifiVar.fgCsaInProgress = FALSE;

	prP2pCsaDoneMsg->rMsgHdr.eMsgId = MID_CNM_P2P_CSA_DONE;

	for (ucBssIndex = 0; ucBssIndex < BSS_DEFAULT_NUM; ucBssIndex++) {
		prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, ucBssIndex);

		if (prBssInfo && prBssInfo->fgIsDfsActive) {
			prP2pCsaDoneMsg->ucBssIndex = ucBssIndex;
			break;
		}
	}

	mboxSendMsg(prAdapter, MBOX_ID_0, (P_MSG_HDR_T)prP2pCsaDoneMsg, MSG_SEND_METHOD_BUF);
}
#endif

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is invoked for P2P or BOW networks
*
* @param (none)
*
* @return TRUE: suggest to adopt the returned preferred channel
*         FALSE: No suggestion. Caller should adopt its preference
*/
/*----------------------------------------------------------------------------*/
BOOLEAN
cnmPreferredChannel(P_ADAPTER_T prAdapter, P_ENUM_BAND_T prBand, PUINT_8 pucPrimaryChannel, P_ENUM_CHNL_EXT_T prBssSCO)
{
	P_BSS_INFO_T prBssInfo;
	UINT_8 i;

	ASSERT(prAdapter);
	ASSERT(prBand);
	ASSERT(pucPrimaryChannel);
	ASSERT(prBssSCO);

	for (i = 0; i < prAdapter->ucHwBssIdNum; i++) {
		prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, i);

		if (prBssInfo) {
			if (IS_BSS_AIS(prBssInfo) && RLM_NET_PARAM_VALID(prBssInfo)) {
				*prBand = prBssInfo->eBand;
				*pucPrimaryChannel = prBssInfo->ucPrimaryChannel;
				*prBssSCO = prBssInfo->eBssSCO;

				return TRUE;
			}
		}
	}

	return FALSE;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief
*
* @param (none)
*
* @return TRUE: available channel is limited to return value
*         FALSE: no limited
*/
/*----------------------------------------------------------------------------*/
BOOLEAN cnmAisInfraChannelFixed(P_ADAPTER_T prAdapter, P_ENUM_BAND_T prBand, PUINT_8 pucPrimaryChannel)
{
	P_BSS_INFO_T prBssInfo;
	UINT_8 i;
	P_WIFI_VAR_T prWifiVar;

	ASSERT(prAdapter);

	prWifiVar = &prAdapter->rWifiVar;

	if (prWifiVar->u4ScanCtrl & SCN_CTRL_DEFAULT_SCAN_CTRL) {
		/*DBGLOG(CNM, INFO, "cnmAisInfraChannelFixed: ByPass AIS channel Fix check\n");*/
		return FALSE;
	}

	for (i = 0; i < prAdapter->ucHwBssIdNum; i++) {
		prBssInfo = prAdapter->aprBssInfo[i];

#if 0
		DBGLOG(INIT, INFO, "%s BSS[%u] active[%u] netType[%u]\n",
		       __func__, i, prBssInfo->fgIsNetActive, prBssInfo->eNetworkType);
#endif

		if (!IS_NET_ACTIVE(prAdapter, i))
			continue;

#if CFG_ENABLE_WIFI_DIRECT
		if (prBssInfo->eNetworkType == NETWORK_TYPE_P2P) {
			BOOLEAN fgFixedChannel =
			p2pFuncIsAPMode(prAdapter->rWifiVar.prP2PConnSettings[prBssInfo->u4PrivateData]);

			if (fgFixedChannel) {

				*prBand = prBssInfo->eBand;
				*pucPrimaryChannel = prBssInfo->ucPrimaryChannel;

				return TRUE;

			}
		}
#endif

#if CFG_ENABLE_BT_OVER_WIFI && CFG_BOW_LIMIT_AIS_CHNL
		if (prBssInfo->eNetworkType == NETWORK_TYPE_BOW) {
			*prBand = prBssInfo->eBand;
			*pucPrimaryChannel = prBssInfo->ucPrimaryChannel;

			return TRUE;
		}
#endif

	}

	return FALSE;
}

#if CFG_SUPPORT_CHNL_CONFLICT_REVISE
BOOLEAN cnmAisDetectP2PChannel(P_ADAPTER_T prAdapter, P_ENUM_BAND_T prBand, PUINT_8 pucPrimaryChannel)
{
	UINT_8 i = 0;
	P_BSS_INFO_T prBssInfo;

	ASSERT(prAdapter);

#if CFG_ENABLE_WIFI_DIRECT
	for (; i < prAdapter->ucHwBssIdNum; i++) {
		prBssInfo = prAdapter->aprBssInfo[i];
		if (prBssInfo->eNetworkType != NETWORK_TYPE_P2P)
			continue;
		if (prBssInfo->eConnectionState == PARAM_MEDIA_STATE_CONNECTED ||
		    (prBssInfo->eCurrentOPMode == OP_MODE_ACCESS_POINT && prBssInfo->eIntendOPMode == OP_MODE_NUM)) {
			*prBand = prBssInfo->eBand;
			*pucPrimaryChannel = prBssInfo->ucPrimaryChannel;
			return TRUE;
		}
	}
#endif
	return FALSE;
}
#endif

/*----------------------------------------------------------------------------*/
/*!
* @brief
*
* @param (none)
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
VOID cnmAisInfraConnectNotify(P_ADAPTER_T prAdapter)
{
#if CFG_ENABLE_BT_OVER_WIFI
	P_BSS_INFO_T prBssInfo, prAisBssInfo, prBowBssInfo;
	UINT_8 i;

	ASSERT(prAdapter);

	prAisBssInfo = NULL;
	prBowBssInfo = NULL;

	for (i = 0; i < prAdapter->ucHwBssIdNum; i++) {
		prBssInfo = prAdapter->aprBssInfo[i];

		if (prBssInfo && IS_BSS_ACTIVE(prBssInfo)) {
			if (IS_BSS_AIS(prBssInfo))
				prAisBssInfo = prBssInfo;
			else if (IS_BSS_BOW(prBssInfo))
				prBowBssInfo = prBssInfo;
		}
	}

	if (prAisBssInfo && prBowBssInfo && RLM_NET_PARAM_VALID(prAisBssInfo) && RLM_NET_PARAM_VALID(prBowBssInfo)) {
		if (prAisBssInfo->eBand != prBowBssInfo->eBand ||
		    prAisBssInfo->ucPrimaryChannel != prBowBssInfo->ucPrimaryChannel) {

			/* Notify BOW to do deactivation */
			bowNotifyAllLinkDisconnected(prAdapter);
		}
	}
#endif
}

/*----------------------------------------------------------------------------*/
/*!
* @brief
*
* @param (none)
*
* @return TRUE: permitted
*         FALSE: Not permitted
*/
/*----------------------------------------------------------------------------*/
BOOLEAN cnmAisIbssIsPermitted(P_ADAPTER_T prAdapter)
{
	P_BSS_INFO_T prBssInfo;
	UINT_8 i;

	ASSERT(prAdapter);

	/* P2P device network shall be included */
	for (i = 0; i <= prAdapter->ucHwBssIdNum; i++) {
		prBssInfo = prAdapter->aprBssInfo[i];

		if (prBssInfo && IS_BSS_ACTIVE(prBssInfo) && !IS_BSS_AIS(prBssInfo))
			return FALSE;
	}

	return TRUE;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief
*
* @param (none)
*
* @return TRUE: permitted
*         FALSE: Not permitted
*/
/*----------------------------------------------------------------------------*/
BOOLEAN cnmP2PIsPermitted(P_ADAPTER_T prAdapter)
{
	P_BSS_INFO_T prBssInfo;
	UINT_8 i;
	BOOLEAN fgBowIsActive;

	ASSERT(prAdapter);

	fgBowIsActive = FALSE;

	for (i = 0; i < prAdapter->ucHwBssIdNum; i++) {
		prBssInfo = prAdapter->aprBssInfo[i];

		if (prBssInfo && IS_BSS_ACTIVE(prBssInfo)) {
			if (prBssInfo->eCurrentOPMode == OP_MODE_IBSS)
				return FALSE;
			else if (IS_BSS_BOW(prBssInfo))
				fgBowIsActive = TRUE;
		}
	}

#if CFG_ENABLE_BT_OVER_WIFI
	if (fgBowIsActive) {
		/* Notify BOW to do deactivation */
		bowNotifyAllLinkDisconnected(prAdapter);
	}
#endif

	return TRUE;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief
*
* @param (none)
*
* @return TRUE: permitted
*         FALSE: Not permitted
*/
/*----------------------------------------------------------------------------*/
BOOLEAN cnmBowIsPermitted(P_ADAPTER_T prAdapter)
{
	P_BSS_INFO_T prBssInfo;
	UINT_8 i;

	ASSERT(prAdapter);

	/* P2P device network shall be included */
	for (i = 0; i <= prAdapter->ucHwBssIdNum; i++) {
		prBssInfo = prAdapter->aprBssInfo[i];

		if (prBssInfo && IS_BSS_ACTIVE(prBssInfo) &&
		    (IS_BSS_P2P(prBssInfo) || prBssInfo->eCurrentOPMode == OP_MODE_IBSS)) {
			return FALSE;
		}
	}

	return TRUE;
}



static UINT_8 cnmGetAPBwPermitted(P_ADAPTER_T prAdapter, UINT_8 ucBssIndex)
{
	P_BSS_INFO_T prBssInfo;
	UINT_8 ucAPBandwidth = MAX_BW_160MHZ;
	P_BSS_DESC_T    prBssDesc = NULL;
	P_P2P_ROLE_FSM_INFO_T prP2pRoleFsmInfo = (P_P2P_ROLE_FSM_INFO_T)NULL;
	UINT_8 i = 0;
	UINT_8 ucOffset = (MAX_BW_80MHZ - CW_80MHZ);


	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, ucBssIndex);


	if (IS_BSS_AIS(prBssInfo)) {
		/*AIS station mode*/
		prBssDesc = prAdapter->rWifiVar.rAisFsmInfo.prTargetBssDesc;
	} else if (IS_BSS_P2P(prBssInfo)) {
		/* P2P mode */

		for (i = 0 ; i < BSS_P2P_NUM; i++) {

			if (!prAdapter->rWifiVar.aprP2pRoleFsmInfo[i])
				continue;

			if (prAdapter->rWifiVar.aprP2pRoleFsmInfo[i]->ucBssIndex == ucBssIndex)
				break;

		}

		if (i >= BSS_P2P_NUM) {
			prP2pRoleFsmInfo = NULL;
		} else {
			prP2pRoleFsmInfo = prAdapter->rWifiVar.aprP2pRoleFsmInfo[i];

			/*only GC need to consider GO's BW*/
			if (!p2pFuncIsAPMode(prAdapter->rWifiVar.prP2PConnSettings[prBssInfo->u4PrivateData]))
				prBssDesc = prP2pRoleFsmInfo->rJoinInfo.prTargetBssDesc;

		}


	}

	if (prBssDesc) {
		if (prBssDesc->eChannelWidth == CW_20_40MHZ) {
			if ((prBssDesc->eSco == CHNL_EXT_SCA) || (prBssDesc->eSco == CHNL_EXT_SCB))
				ucAPBandwidth = MAX_BW_40MHZ;
			else
				ucAPBandwidth = MAX_BW_20MHZ;
		} else {
			ucAPBandwidth = prBssDesc->eChannelWidth + ucOffset;
		}

	}

	return ucAPBandwidth;
}


/*----------------------------------------------------------------------------*/
/*!
* @brief
*
* @param (none)
*
* @return TRUE: permitted
*         FALSE: Not permitted
*/
/*----------------------------------------------------------------------------*/
BOOLEAN cnmBss40mBwPermitted(P_ADAPTER_T prAdapter, UINT_8 ucBssIndex)
{
	ASSERT(prAdapter);

	/* Note: To support real-time decision instead of current activated-time,
	 *       the STA roaming case shall be considered about synchronization
	 *       problem. Another variable fgAssoc40mBwAllowed is added to
	 *       represent HT capability when association
	 */

	/* Decide max bandwidth by feature option */
	if (cnmGetBssMaxBw(prAdapter, ucBssIndex) < MAX_BW_40MHZ)
		return FALSE;

	/*check AP or GO capbility for Station or GC */
	if (cnmGetAPBwPermitted(prAdapter, ucBssIndex) < MAX_BW_40MHZ)
		return FALSE;
#if 0
	/* Decide max by other BSS */
	for (i = 0; i < prAdapter->ucHwBssIdNum; i++) {
		if (i != ucBssIndex) {
			prBssInfo = prAdapter->aprBssInfo[i];

			if (prBssInfo && IS_BSS_ACTIVE(prBssInfo) &&
			    (prBssInfo->fg40mBwAllowed || prBssInfo->fgAssoc40mBwAllowed))
				return FALSE;
		}
	}
#endif

	return TRUE;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief
*
* @param (none)
*
* @return TRUE: permitted
*         FALSE: Not permitted
*/
/*----------------------------------------------------------------------------*/
BOOLEAN cnmBss80mBwPermitted(P_ADAPTER_T prAdapter, UINT_8 ucBssIndex)
{
	ASSERT(prAdapter);

	/* Note: To support real-time decision instead of current activated-time,
	 *       the STA roaming case shall be considered about synchronization
	 *       problem. Another variable fgAssoc40mBwAllowed is added to
	 *       represent HT capability when association
	 */

	/* Check 40Mhz first */
	if (!cnmBss40mBwPermitted(prAdapter, ucBssIndex))
		return FALSE;

	/* Decide max bandwidth by feature option */
	if (cnmGetBssMaxBw(prAdapter, ucBssIndex) < MAX_BW_80MHZ)
		return FALSE;

	/*check AP or GO capbility for Station or GC */
	if (cnmGetAPBwPermitted(prAdapter, ucBssIndex) < MAX_BW_80MHZ)
		return FALSE;

	return TRUE;
}

UINT_8 cnmGetBssMaxBw(P_ADAPTER_T prAdapter, UINT_8 ucBssIndex)
{
	P_BSS_INFO_T prBssInfo;
	UINT_8 ucMaxBandwidth = MAX_BW_80_80_MHZ; /*chip capability*/
	P_BSS_DESC_T    prBssDesc = NULL;
	ENUM_BAND_T eBand = BAND_NULL;
	P_P2P_ROLE_FSM_INFO_T prP2pRoleFsmInfo = (P_P2P_ROLE_FSM_INFO_T) NULL;
	P_P2P_CONNECTION_REQ_INFO_T prP2pConnReqInfo = (P_P2P_CONNECTION_REQ_INFO_T) NULL;

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, ucBssIndex);

	if (IS_BSS_AIS(prBssInfo)) {
		/* STA mode */


		/*should check Bss_info could be used or not
		*the info might not be trustable before state3
		*/

		prBssDesc = prAdapter->rWifiVar.rAisFsmInfo.prTargetBssDesc;
		if (prBssDesc)
			eBand = prBssDesc->eBand;
		else
			eBand = prBssInfo->eBand;


		ASSERT(eBand != BAND_NULL);

		if (eBand == BAND_2G4)
			ucMaxBandwidth = prAdapter->rWifiVar.ucSta2gBandwidth;
		else
			ucMaxBandwidth = prAdapter->rWifiVar.ucSta5gBandwidth;

		if (ucMaxBandwidth > prAdapter->rWifiVar.ucStaBandwidth)
			ucMaxBandwidth = prAdapter->rWifiVar.ucStaBandwidth;
	} else if (IS_BSS_P2P(prBssInfo)) {
		prP2pRoleFsmInfo = p2pFuncGetRoleByBssIdx(prAdapter, ucBssIndex);
		if (!prAdapter->rWifiVar.ucApChnlDefFromCfg && prP2pRoleFsmInfo
			&& prBssInfo->eCurrentOPMode == OP_MODE_ACCESS_POINT) {
			prP2pConnReqInfo = &(prP2pRoleFsmInfo->rConnReqInfo);
			ucMaxBandwidth = prP2pConnReqInfo->eChnlBw;
		} else {
			/* AP mode */
			if (p2pFuncIsAPMode(prAdapter->rWifiVar.prP2PConnSettings[prBssInfo->u4PrivateData])) {
				if (prBssInfo->eBand == BAND_2G4)
					ucMaxBandwidth = prAdapter->rWifiVar.ucAp2gBandwidth;
				else
					ucMaxBandwidth = prAdapter->rWifiVar.ucAp5gBandwidth;

				if (ucMaxBandwidth > prAdapter->rWifiVar.ucApBandwidth)
					ucMaxBandwidth = prAdapter->rWifiVar.ucApBandwidth;
			}
			/* P2P mode */
			else {
				if (prBssInfo->eBand == BAND_2G4)
					ucMaxBandwidth = prAdapter->rWifiVar.ucP2p2gBandwidth;
				else
					ucMaxBandwidth = prAdapter->rWifiVar.ucP2p5gBandwidth;
			}

		}

	}

	return ucMaxBandwidth;
}


UINT_8 cnmGetBssMaxBwToChnlBW(P_ADAPTER_T prAdapter, UINT_8 ucBssIndex)
{
	UINT_8 ucMaxBandwidth = cnmGetBssMaxBw(prAdapter, ucBssIndex);
	return ucMaxBandwidth == MAX_BW_20MHZ ? ucMaxBandwidth:(ucMaxBandwidth - 1);
}

/*----------------------------------------------------------------------------*/
/*!
* @brief    Search available HW ID and BSS_INFO structure and initialize
*           these parameters, i.e., fgIsNetActive, ucBssIndex, eNetworkType
*           and ucOwnMacIndex
*
* @param (none)
*
* @return
*/
/*----------------------------------------------------------------------------*/
P_BSS_INFO_T cnmGetBssInfoAndInit(P_ADAPTER_T prAdapter, ENUM_NETWORK_TYPE_T eNetworkType, BOOLEAN fgIsP2pDevice)
{
	P_BSS_INFO_T prBssInfo;
	UINT_8 ucBssIndex, ucOwnMacIdx;

	ASSERT(prAdapter);

	/*specific case for p2p device scan*/
	if (eNetworkType == NETWORK_TYPE_P2P && fgIsP2pDevice) {
		prBssInfo = prAdapter->aprBssInfo[prAdapter->ucP2PDevBssIdx];

		prBssInfo->fgIsInUse = TRUE;
		prBssInfo->ucBssIndex = prAdapter->ucP2PDevBssIdx;
		prBssInfo->eNetworkType = eNetworkType;
		prBssInfo->ucOwnMacIndex = prAdapter->ucHwBssIdNum;
#if CFG_SUPPORT_PNO
		prBssInfo->fgIsPNOEnable = FALSE;
		prBssInfo->fgIsNetRequestInActive = FALSE;
#endif
		return prBssInfo;
	}

	/*reserve ownMAC0 for MBSS*/
	ucOwnMacIdx = (eNetworkType == NETWORK_TYPE_MBSS) ? 0 : 1;

	/* Find available HW set  with the order 1,2,..*/
	do {
		for (ucBssIndex = 0; ucBssIndex < prAdapter->ucHwBssIdNum; ucBssIndex++) {
			prBssInfo = prAdapter->aprBssInfo[ucBssIndex];

			if (prBssInfo && prBssInfo->fgIsInUse && ucOwnMacIdx == prBssInfo->ucOwnMacIndex)
				break;
		}

		if (ucBssIndex >= prAdapter->ucHwBssIdNum) {
			/* No hit  the ucOwnMacIndex could be assigned to this new bss */
			break;
		}
	} while (++ucOwnMacIdx < prAdapter->ucHwBssIdNum);


	/*should not dispatch P2P_DEV_BSS_INDEX (prAdapter->ucHwBssIdNum)to general bss,
	*It means total BSS_INFO_NUM BSS are created,
	*no more reseve for MBSS
	*/
	if (ucOwnMacIdx == prAdapter->ucHwBssIdNum) {

		for (ucBssIndex = 0; ucBssIndex < prAdapter->ucHwBssIdNum; ucBssIndex++) {
			prBssInfo = prAdapter->aprBssInfo[ucBssIndex];

			/*If the Bss was alredy assigned, and in use*/
			if (prBssInfo && prBssInfo->fgIsInUse && prBssInfo->ucOwnMacIndex == 0)
				break;
		}

		if (ucBssIndex >= prAdapter->ucHwBssIdNum) {
			/*there is no NETWORK_TYPE_MBSS used before */
			DBGLOG(INIT, WARN,
				"[Warning] too much Bss in use, take reserve OwnMac(%d)for usage!\n",
				ucOwnMacIdx);
			ucOwnMacIdx = 0;
		}

	}

	/* Find available BSS_INFO */
	for (ucBssIndex = 0; ucBssIndex < prAdapter->ucHwBssIdNum; ucBssIndex++) {
		prBssInfo = prAdapter->aprBssInfo[ucBssIndex];

		if (prBssInfo && !prBssInfo->fgIsInUse) {
			prBssInfo->fgIsInUse = TRUE;
			prBssInfo->ucBssIndex = ucBssIndex;
			prBssInfo->eNetworkType = eNetworkType;
			prBssInfo->ucOwnMacIndex = ucOwnMacIdx;
			break;
		}
	}

	if (ucOwnMacIdx >= prAdapter->ucHwBssIdNum || ucBssIndex >= prAdapter->ucHwBssIdNum)
		prBssInfo = NULL;
#if CFG_SUPPORT_PNO
	if (prBssInfo) {
		prBssInfo->fgIsPNOEnable = FALSE;
		prBssInfo->fgIsNetRequestInActive = FALSE;
	}
#endif
	return prBssInfo;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief    Search available HW ID and BSS_INFO structure and initialize
*           these parameters, i.e., ucBssIndex, eNetworkType and ucOwnMacIndex
*
* @param (none)
*
* @return
*/
/*----------------------------------------------------------------------------*/
VOID cnmFreeBssInfo(P_ADAPTER_T prAdapter, P_BSS_INFO_T prBssInfo)
{
	ASSERT(prAdapter);
	ASSERT(prBssInfo);

	prBssInfo->fgIsInUse = FALSE;
}

#if CFG_SUPPORT_DBDC
/*----------------------------------------------------------------------------*/
/*!
* @brief    Init DBDC
*
* @param (none)
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
VOID cnmInitDbdcSetting(IN P_ADAPTER_T prAdapter)
{
	UINT_8			ucBssLoopIndex;

	/* Parameter decision */
	switch (prAdapter->rWifiVar.eDbdcMode) {
	case ENUM_DBDC_MODE_DISABLED:
		cnmUpdateDbdcSetting(prAdapter, FALSE);
		break;

	case ENUM_DBDC_MODE_DYNAMIC:
		g_rDbdcInfo.eDbdcFsmCurrState = ENUM_DBDC_FSM_STATE_DISABLE_IDLE;
		g_rDbdcInfo.eDbdcFsmPrevState = ENUM_DBDC_FSM_STATE_DISABLE_IDLE;
		g_rDbdcInfo.eDbdcFsmNextState = ENUM_DBDC_FSM_STATE_DISABLE_IDLE;

		cnmTimerInitTimer(prAdapter,
						&g_rDbdcInfo.rDbdcGuardTimer,
						(PFN_MGMT_TIMEOUT_FUNC)cnmDbdcGuardTimerCallback,
						(ULONG) NULL);

		g_rDbdcInfo.eDdbcGuardTimerType = ENUM_DBDC_GUARD_TIMER_NONE;
		g_rDbdcInfo.fgReqPrivelegeLock = FALSE;
		LINK_INITIALIZE(&g_rDbdcInfo.rPendingMsgList);
		g_rDbdcInfo.fgDbdcDisableOpmodeChangeDone = TRUE;

		for (ucBssLoopIndex = 0; ucBssLoopIndex < prAdapter->ucHwBssIdNum; ucBssLoopIndex++)
			g_rDbdcInfo.eBssOpModeState[ucBssLoopIndex] = ENUM_OPMODE_STATE_DONE;

		cnmUpdateDbdcSetting(prAdapter, FALSE);
		break;

	case ENUM_DBDC_MODE_STATIC:
		cnmUpdateDbdcSetting(prAdapter, TRUE);
		break;

	default:
		DBGLOG(CNM, ERROR, "[DBDC]Incorrect DBDC mode %u\n", prAdapter->rWifiVar.eDbdcMode);
		break;
	}
}

/*----------------------------------------------------------------------------*/
/*!
* @brief    Check A+G Condition
*
* @param (none)
*
* @return TRUE: A+G, FALSE: NOT A+G
*/
/*----------------------------------------------------------------------------*/
static BOOLEAN cnmDbdcIsAGConcurrent(
	IN P_ADAPTER_T	prAdapter,
	IN ENUM_BAND_T	eRfBand_Connecting
	)
{
	P_BSS_INFO_T	prBssInfo;
	UINT_8			ucBssIndex;
	ENUM_BAND_T		eBandCompare = eRfBand_Connecting;
	BOOLEAN			fgAGConcurrent = FALSE;
	ENUM_BAND_T		eBssBand[BSSID_NUM] = {BAND_NULL};

	for (ucBssIndex = 0; ucBssIndex < prAdapter->ucHwBssIdNum; ucBssIndex++) {

		prBssInfo = prAdapter->aprBssInfo[ucBssIndex];

		if (DBDC_IS_BSS_NOT_ALIVE(prBssInfo))
			continue;

		if (prBssInfo->eBand != BAND_2G4 && prBssInfo->eBand != BAND_5G)
			continue;

		eBssBand[ucBssIndex] = prBssInfo->eBand;

		if (eBandCompare != BAND_2G4 && eBandCompare != BAND_5G)
			eBandCompare = prBssInfo->eBand;

		if (eBandCompare != prBssInfo->eBand)
			fgAGConcurrent = TRUE;	/*A+G*/
	}

	DBGLOG(CNM, INFO, "[DBDC] BSS AG[%u.%u.%u.%u][%u]\n",
							eBssBand[BSSID_0],
							eBssBand[BSSID_1],
							eBssBand[BSSID_2],
							eBssBand[BSSID_3],
							eRfBand_Connecting);

	return fgAGConcurrent;	/*NOT A+G*/
}

/*----------------------------------------------------------------------------*/
/*!
* @brief    MT6632 HW capability will change between BW160+NSS2 and BW80+NSS1
*
* @param (none)
*
* @return TRUE: WAIT/WAIT FAIL/Done Success/Done Fail
*/
/*----------------------------------------------------------------------------*/
static enum ENUM_DBDC_PROTOCOL_STATUS_T cnmDbdcOpmodeChangeAndWait(
	IN P_ADAPTER_T	prAdapter,
	IN BOOLEAN		fgDbdcEn
	)
{
	UINT_8			ucBssIndex;
	UINT_8			ucWmmSetBitmap = 0;
	UINT_8			ucOpBw;
	UINT_8			ucNss;
	P_BSS_INFO_T	prBssInfo;
	enum ENUM_OP_CHANGE_STATUS_T eBssOpmodeChange;
	enum ENUM_DBDC_PROTOCOL_STATUS_T eRetVar = ENUM_DBDC_PROTOCOL_STATUS_DONE_SUCCESS;

#define IS_BSS_CLIENT(_prBssInfo) (_prBssInfo->eCurrentOPMode == OP_MODE_INFRASTRUCTURE)

	if (fgDbdcEn)
		ucWmmSetBitmap |= BIT(DBDC_2G_WMM_INDEX);

	for (ucBssIndex = 0; ucBssIndex <= prAdapter->ucHwBssIdNum; ucBssIndex++) {
		prBssInfo = prAdapter->aprBssInfo[ucBssIndex];
		if (DBDC_IS_BSS_ALIVE(prBssInfo)) {

			ucOpBw = rlmGetBssOpBwByVhtAndHtOpInfo(prBssInfo);
			if (fgDbdcEn && ucOpBw > MAX_BW_80MHZ)
				ucOpBw = MAX_BW_80MHZ;

			ucNss = fgDbdcEn?1:wlanGetSupportNss(prAdapter, ucBssIndex);

			eBssOpmodeChange = rlmChangeOperationMode(prAdapter,
				ucBssIndex,
				ucOpBw,
				ucNss,
				IS_BSS_CLIENT(prBssInfo)?cnmDbdcOpModeChangeDoneCallback:NULL);

			DBGLOG(CNM, INFO, "[DBDC] BSS index[%u] to BW %u NSS %u Mode:%s, status %u\n",
							ucBssIndex,
							ucOpBw,
							ucNss,
							IS_BSS_CLIENT(prBssInfo)?"Client":"Master",
							eBssOpmodeChange);

			switch (eBssOpmodeChange) {
			case OP_CHANGE_STATUS_VALID_CHANGE_CALLBACK_WAIT:
				g_rDbdcInfo.fgDbdcDisableOpmodeChangeDone = FALSE;
				g_rDbdcInfo.eBssOpModeState[ucBssIndex] = ENUM_OPMODE_STATE_WAIT;
				eRetVar = ENUM_DBDC_PROTOCOL_STATUS_WAIT;

				break;

			case OP_CHANGE_STATUS_VALID_CHANGE_CALLBACK_DONE:
			case OP_CHANGE_STATUS_VALID_NO_CHANGE:
				g_rDbdcInfo.eBssOpModeState[ucBssIndex] = ENUM_OPMODE_STATE_DONE;
				break;

			case OP_CHANGE_STATUS_INVALID:
				g_rDbdcInfo.eBssOpModeState[ucBssIndex] = ENUM_OPMODE_STATE_FAIL;

				if (eRetVar == ENUM_DBDC_PROTOCOL_STATUS_DONE_SUCCESS)
					eRetVar = ENUM_DBDC_PROTOCOL_STATUS_DONE_FAIL;
				else if (eRetVar == ENUM_DBDC_PROTOCOL_STATUS_WAIT)
					eRetVar = ENUM_DBDC_PROTOCOL_STATUS_WAIT;

				break;

			default:
				ASSERT(0);
				break;
			}
		} else {
			g_rDbdcInfo.eBssOpModeState[ucBssIndex] = ENUM_OPMODE_STATE_DONE;
		}
	}

	return eRetVar;
}


VOID cnmDbdcOpModeChangeDoneCallback(
	IN P_ADAPTER_T	prAdapter,
	IN UINT_8		ucBssIndex,
	IN BOOLEAN		fgSuccess)
{
	UINT_8			ucBssLoopIndex;
	BOOLEAN			fgIsAllActionFrameSuccess = TRUE;

	if (fgSuccess)
		g_rDbdcInfo.eBssOpModeState[ucBssIndex] = ENUM_OPMODE_STATE_DONE;
	else
		g_rDbdcInfo.eBssOpModeState[ucBssIndex] = ENUM_OPMODE_STATE_FAIL;

	DBGLOG(CNM, INFO, "[DBDC] OPMODE STATE [%u/%u/%u/%u]\n",
		g_rDbdcInfo.eBssOpModeState[BSSID_0],
		g_rDbdcInfo.eBssOpModeState[BSSID_1],
		g_rDbdcInfo.eBssOpModeState[BSSID_2],
		g_rDbdcInfo.eBssOpModeState[BSSID_3]);

	for (ucBssLoopIndex = 0; ucBssLoopIndex <= prAdapter->ucHwBssIdNum; ucBssLoopIndex++) {

		if (g_rDbdcInfo.eBssOpModeState[ucBssIndex] == ENUM_OPMODE_STATE_WAIT)
			return;

		if (g_rDbdcInfo.eBssOpModeState[ucBssIndex] == ENUM_OPMODE_STATE_FAIL &&
			fgIsAllActionFrameSuccess == TRUE) {
			/* Some OP mode change FAIL */
			fgIsAllActionFrameSuccess = FALSE;
		}
	}

	if (!g_rDbdcInfo.fgDbdcDisableOpmodeChangeDone) {
		if (fgIsAllActionFrameSuccess) {
			/* CODING_STYLE_WORKAROUND
			* Gerrit wish no {} for "one line if statement", but build command as } before else
			*/
			DBDC_FSM_EVENT_HANDLER(prAdapter, DBDC_FSM_EVENT_ACTION_FRAME_ALL_SUCCESS);
		} else
			DBDC_FSM_EVENT_HANDLER(prAdapter, DBDC_FSM_EVENT_ACTION_FRAME_SOME_FAIL);

		g_rDbdcInfo.fgDbdcDisableOpmodeChangeDone = TRUE;
	}

	return;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief    Send DBDC Enable/Disable command to FW
*
* @param (none)
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
VOID cnmUpdateDbdcSetting(IN P_ADAPTER_T prAdapter, IN BOOLEAN fgDbdcEn)
{
	CMD_DBDC_SETTING_T		rDbdcSetting;
	P_CMD_DBDC_SETTING_T	prCmdBody;
	WLAN_STATUS				rStatus = WLAN_STATUS_SUCCESS;

	DBGLOG(CNM, INFO, "[DBDC] %s\n", fgDbdcEn?"Enable":"Disable");

	/* Send event to FW */
	prCmdBody = (P_CMD_DBDC_SETTING_T)&rDbdcSetting;

	kalMemZero(prCmdBody, sizeof(CMD_DBDC_SETTING_T));

	prCmdBody->ucDbdcEn = fgDbdcEn;

	if (fgDbdcEn)
		prCmdBody->ucWmmBandBitmap |= BIT(DBDC_2G_WMM_INDEX);

	rStatus = wlanSendSetQueryCmd(prAdapter,	/* prAdapter */
				  CMD_ID_SET_DBDC_PARMS,	/* ucCID */
				  TRUE, /* fgSetQuery */
				  FALSE,	/* fgNeedResp */
				  FALSE,	/* fgIsOid */
				  NULL, /* pfCmdDoneHandler */
				  NULL, /* pfCmdTimeoutHandler */
				  sizeof(CMD_DBDC_SETTING_T),	/* u4SetQueryInfoLen */
				  (PUINT_8)prCmdBody,	/* pucInfoBuffer */
				  NULL, /* pvSetQueryBuffer */
				  0 /* u4SetQueryBufferLen */);
}

/*----------------------------------------------------------------------------*/
/*!
* \brief DBDC FSM Entry
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
static VOID
cnmDbdcFsmSteps(
	IN P_ADAPTER_T                  prAdapter,
	IN enum ENUM_DBDC_FSM_STATE_T   eNextState,
	IN enum ENUM_DBDC_FSM_EVENT_T   eEvent
	)
{
	/* Do entering Next State and do its initial function. */
	g_rDbdcInfo.eDbdcFsmPrevState = g_rDbdcInfo.eDbdcFsmCurrState;
	g_rDbdcInfo.eDbdcFsmCurrState = eNextState;
	g_rDbdcInfo.eDbdcFsmNextState = eNextState;

	DBGLOG(CNM, INFO, "[DBDC] event %d state %d->%d\n",
			eEvent,
			g_rDbdcInfo.eDbdcFsmPrevState,
			g_rDbdcInfo.eDbdcFsmCurrState);

	if (g_rDbdcInfo.eDbdcFsmPrevState != g_rDbdcInfo.eDbdcFsmCurrState) {
		/* state change, call exit function of previous state */
		if (arDdbcFsmActionTable[g_rDbdcInfo.eDbdcFsmPrevState].pfExitFunc)
			arDdbcFsmActionTable[g_rDbdcInfo.eDbdcFsmPrevState].pfExitFunc(prAdapter);

		/* state change, call entry function of current state */
		if (arDdbcFsmActionTable[g_rDbdcInfo.eDbdcFsmCurrState].pfEntryFunc)
			arDdbcFsmActionTable[g_rDbdcInfo.eDbdcFsmCurrState].pfEntryFunc(prAdapter);
	}
}

static BOOLEAN
cnmDBDCIsReqPeivilegeLock(
	VOID
	)
{
	return g_rDbdcInfo.fgReqPrivelegeLock;
}

static VOID
cnmDBDCFsmActionReqPeivilegeLock(
	VOID
	)
{
	g_rDbdcInfo.fgReqPrivelegeLock = TRUE;
	DBGLOG(CNM, INFO, "[DBDC] ReqPrivelege Lock!!\n");
}

static VOID
cnmDBDCFsmActionReqPeivilegeUnLock(
	IN P_ADAPTER_T	prAdapter
	)
{
	P_MSG_CH_REQ_T prPendingMsg;
	P_MSG_HDR_T prMsgHdr;

	g_rDbdcInfo.fgReqPrivelegeLock = FALSE;
	DBGLOG(CNM, INFO, "[DBDC] ReqPrivelege Unlock!!\n");

	while (!LINK_IS_EMPTY(&g_rDbdcInfo.rPendingMsgList)) {

		LINK_REMOVE_HEAD(&g_rDbdcInfo.rPendingMsgList, prMsgHdr, P_MSG_HDR_T);

		if (prMsgHdr) {
			prPendingMsg = (P_MSG_CH_REQ_T)prMsgHdr;

			DBGLOG(CNM, INFO, "[DBDC] ChReq: send queued REQ of BSS %u Token %u\n",
						prPendingMsg->ucBssIndex, prPendingMsg->ucTokenID);

			cnmChMngrRequestPrivilege(prAdapter, &prPendingMsg->rMsgHdr);
		} else {
			ASSERT(0);
		}
	}
}

static VOID
cnmDbdcFsmEntryFunc_DISABLE_IDLE(
	IN P_ADAPTER_T	prAdapter
	)
{
	cnmDBDCFsmActionReqPeivilegeUnLock(prAdapter);
}

static VOID
cnmDbdcFsmEntryFunc_WAIT_PROTOCOL_ENABLE(
	IN P_ADAPTER_T	prAdapter
	)
{
	cnmDBDCFsmActionReqPeivilegeLock();
}

static VOID
cnmDbdcFsmEntryFunc_WAIT_HW_ENABLE(
	IN P_ADAPTER_T	prAdapter
	)
{
	if (!cnmDBDCIsReqPeivilegeLock())
		cnmDBDCFsmActionReqPeivilegeLock();

	cnmUpdateDbdcSetting(prAdapter, TRUE);
}

static VOID
cnmDbdcFsmEntryFunc_ENABLE_GUARD(
	IN P_ADAPTER_T	prAdapter
	)
{
	if (timerPendingTimer(&g_rDbdcInfo.rDbdcGuardTimer)) {
		DBGLOG(CNM, WARN, "[DBDC] Guard Timer type %u should not exist, stop it\n",
			g_rDbdcInfo.eDdbcGuardTimerType);
		cnmTimerStopTimer(prAdapter, &g_rDbdcInfo.rDbdcGuardTimer);
		g_rDbdcInfo.eDdbcGuardTimerType =
						ENUM_DBDC_GUARD_TIMER_NONE;
	}
	DBDC_SET_GUARD_TIME(prAdapter);
}

static VOID
cnmDbdcFsmEntryFunc_WAIT_HW_DISABLE(
	IN P_ADAPTER_T	prAdapter
	)
{
	cnmUpdateDbdcSetting(prAdapter, FALSE);
}

static VOID
cnmDbdcFsmEntryFunc_DISABLE_GUARD(
	IN P_ADAPTER_T	prAdapter
	)
{
	if (timerPendingTimer(&g_rDbdcInfo.rDbdcGuardTimer)) {
		DBGLOG(CNM, WARN, "[DBDC] Guard Timer type %u should not exist, stop it\n",
			g_rDbdcInfo.eDdbcGuardTimerType);
		cnmTimerStopTimer(prAdapter, &g_rDbdcInfo.rDbdcGuardTimer);
		g_rDbdcInfo.eDdbcGuardTimerType =
						ENUM_DBDC_GUARD_TIMER_NONE;
	}
	DBDC_SET_GUARD_TIME(prAdapter);

	cnmDbdcOpmodeChangeAndWait(prAdapter, FALSE);
}

static VOID
cnmDbdcFsmEventHandler_DISABLE_IDLE(
	IN P_ADAPTER_T	prAdapter,
	IN enum ENUM_DBDC_FSM_EVENT_T	eEvent
	)
{
	switch (eEvent) {
	case DBDC_FSM_EVENT_BSS_DISCONNECT_LEAVE_AG:
		/* Do Nothing */
		break;

	case DBDC_FSM_EVENT_BSS_CONNECTING_ENTER_AG:
		/* Enable DBDC */
		switch (cnmDbdcOpmodeChangeAndWait(prAdapter, TRUE)) {
		case ENUM_DBDC_PROTOCOL_STATUS_WAIT:
			g_rDbdcInfo.eDbdcFsmNextState = ENUM_DBDC_FSM_STATE_WAIT_PROTOCOL_ENABLE;
			break;

		case ENUM_DBDC_PROTOCOL_STATUS_DONE_SUCCESS:
			g_rDbdcInfo.eDbdcFsmNextState = ENUM_DBDC_FSM_STATE_WAIT_HW_ENABLE;
			break;

		case ENUM_DBDC_PROTOCOL_STATUS_DONE_FAIL:
			/* Should NOT FAIL, not recover anything now. Stop enable DBDC */
			break;
		default:
			break;
		}
		break;

	case DBDC_FSM_EVENT_SWITCH_GUARD_TIME_TO:
	case DBDC_FSM_EVENT_DISABLE_COUNT_DOWN_TO:
	case DBDC_FSM_EVENT_ACTION_FRAME_ALL_SUCCESS:
	case DBDC_FSM_EVENT_ACTION_FRAME_SOME_FAIL:
	case DBDC_FSM_EVENT_DBDC_HW_SWITCH_DONE:
		/* ABNORMAL CASE*/
		DBDC_FSM_MSG_WRONG_EVT(eEvent);
		break;
	default:
		/* WRONG EVENT */
		DBDC_FSM_MSG_ERROR_EVT(eEvent);
		break;
	}

	cnmDbdcFsmSteps(prAdapter, g_rDbdcInfo.eDbdcFsmNextState, eEvent);
}

static VOID
cnmDbdcFsmEventHandler_WAIT_PROTOCOL_ENABLE(
	IN P_ADAPTER_T                  prAdapter,
	IN enum ENUM_DBDC_FSM_EVENT_T   eEvent
	)
{
	switch (eEvent) {
	case DBDC_FSM_EVENT_BSS_DISCONNECT_LEAVE_AG:
	case DBDC_FSM_EVENT_BSS_CONNECTING_ENTER_AG:
		/* IGNORE */
		break;

	case DBDC_FSM_EVENT_SWITCH_GUARD_TIME_TO:
	case DBDC_FSM_EVENT_DISABLE_COUNT_DOWN_TO:
		/* ABNORMAL CASE*/
		DBDC_FSM_MSG_WRONG_EVT(eEvent);
		break;

	case DBDC_FSM_EVENT_ACTION_FRAME_ALL_SUCCESS:
		g_rDbdcInfo.eDbdcFsmNextState = ENUM_DBDC_FSM_STATE_WAIT_HW_ENABLE;
		break;

	case DBDC_FSM_EVENT_ACTION_FRAME_SOME_FAIL:
		/* Not recover anything. Stop Enable DBDC */
		g_rDbdcInfo.eDbdcFsmNextState = ENUM_DBDC_FSM_STATE_DISABLE_IDLE;
		break;

	case DBDC_FSM_EVENT_DBDC_HW_SWITCH_DONE:
		/* ABNORMAL CASE*/
		DBDC_FSM_MSG_WRONG_EVT(eEvent);
		break;

	default:
		/* WRONG EVENT */
		DBDC_FSM_MSG_ERROR_EVT(eEvent);
		break;
	}

	cnmDbdcFsmSteps(prAdapter, g_rDbdcInfo.eDbdcFsmNextState, eEvent);
}

static VOID
cnmDbdcFsmEventHandler_WAIT_HW_ENABLE(
	IN P_ADAPTER_T                  prAdapter,
	IN enum ENUM_DBDC_FSM_EVENT_T   eEvent
	)
{
	/* Prepare to Enable DBDC */

	switch (eEvent) {
	case DBDC_FSM_EVENT_BSS_DISCONNECT_LEAVE_AG:
	case DBDC_FSM_EVENT_BSS_CONNECTING_ENTER_AG:
		/* IGNORE */
		break;

	case DBDC_FSM_EVENT_SWITCH_GUARD_TIME_TO:
	case DBDC_FSM_EVENT_DISABLE_COUNT_DOWN_TO:
	case DBDC_FSM_EVENT_ACTION_FRAME_ALL_SUCCESS:
	case DBDC_FSM_EVENT_ACTION_FRAME_SOME_FAIL:
		/* ABNORMAL CASE*/
		DBDC_FSM_MSG_WRONG_EVT(eEvent);
		break;

	case DBDC_FSM_EVENT_DBDC_HW_SWITCH_DONE:
		g_rDbdcInfo.eDbdcFsmNextState = ENUM_DBDC_FSM_STATE_ENABLE_GUARD;
		break;

	default:
		/* WRONG EVENT */
		DBDC_FSM_MSG_ERROR_EVT(eEvent);
		break;
	}

	cnmDbdcFsmSteps(prAdapter, g_rDbdcInfo.eDbdcFsmNextState, eEvent);
}


static VOID
cnmDbdcFsmEventHandler_ENABLE_GUARD(
	IN P_ADAPTER_T                  prAdapter,
	IN enum ENUM_DBDC_FSM_EVENT_T   eEvent
	)
{
	switch (eEvent) {
	case DBDC_FSM_EVENT_BSS_DISCONNECT_LEAVE_AG:
	case DBDC_FSM_EVENT_BSS_CONNECTING_ENTER_AG:
		/* IGNORE */
		break;

	case DBDC_FSM_EVENT_SWITCH_GUARD_TIME_TO:
		if (cnmDbdcIsAGConcurrent(prAdapter, BAND_NULL)) {
			/* CODEING_STYLE_WORKAROUND
			* Gerrit wish no {} for "one line if statement", but build command as } before else
			*/
			g_rDbdcInfo.eDbdcFsmNextState = ENUM_DBDC_FSM_STATE_ENABLE_IDLE;
		} else
			g_rDbdcInfo.eDbdcFsmNextState = ENUM_DBDC_FSM_STATE_WAIT_HW_DISABLE;
		break;

	case DBDC_FSM_EVENT_DISABLE_COUNT_DOWN_TO:
	case DBDC_FSM_EVENT_ACTION_FRAME_ALL_SUCCESS:
	case DBDC_FSM_EVENT_ACTION_FRAME_SOME_FAIL:
	case DBDC_FSM_EVENT_DBDC_HW_SWITCH_DONE:
		/* ABNORMAL CASE*/
		DBDC_FSM_MSG_WRONG_EVT(eEvent);
		break;

	default:
		/* WRONG EVENT */
		DBDC_FSM_MSG_ERROR_EVT(eEvent);
		break;
	}

	cnmDbdcFsmSteps(prAdapter, g_rDbdcInfo.eDbdcFsmNextState, eEvent);
}

static VOID
cnmDbdcFsmEventHandler_ENABLE_IDLE(
	IN P_ADAPTER_T                  prAdapter,
	IN enum ENUM_DBDC_FSM_EVENT_T   eEvent
	)
{
	switch (eEvent) {
	case DBDC_FSM_EVENT_BSS_DISCONNECT_LEAVE_AG:
		/* start DBDC disable countdown timer */
		if (timerPendingTimer(&g_rDbdcInfo.rDbdcGuardTimer)) {
			DBGLOG(CNM, WARN, "[DBDC] Guard Timer type %u should not exist, stop it\n",
				g_rDbdcInfo.eDdbcGuardTimerType);
			cnmTimerStopTimer(prAdapter, &g_rDbdcInfo.rDbdcGuardTimer);
			g_rDbdcInfo.eDdbcGuardTimerType =
							ENUM_DBDC_GUARD_TIMER_NONE;
		}
		DBDC_SET_DISABLE_COUNTDOWN(prAdapter);
		break;

	case DBDC_FSM_EVENT_BSS_CONNECTING_ENTER_AG:
		/* cancel DBDC disable countdown if exist */
		if (timerPendingTimer(&g_rDbdcInfo.rDbdcGuardTimer) &&
			g_rDbdcInfo.eDdbcGuardTimerType == ENUM_DBDC_GUARD_TIMER_DISABLE_COUNT_DOWN) {
			cnmTimerStopTimer(prAdapter, &g_rDbdcInfo.rDbdcGuardTimer);
		}
		break;

	case DBDC_FSM_EVENT_SWITCH_GUARD_TIME_TO:
		/* ABNORMAL CASE*/
		DBDC_FSM_MSG_WRONG_EVT(eEvent);
		break;

	case DBDC_FSM_EVENT_DISABLE_COUNT_DOWN_TO:
		if (!cnmDbdcIsAGConcurrent(prAdapter, BAND_NULL))
			g_rDbdcInfo.eDbdcFsmNextState = ENUM_DBDC_FSM_STATE_WAIT_HW_DISABLE;
		break;

	case DBDC_FSM_EVENT_ACTION_FRAME_ALL_SUCCESS:
	case DBDC_FSM_EVENT_ACTION_FRAME_SOME_FAIL:
	case DBDC_FSM_EVENT_DBDC_HW_SWITCH_DONE:
		/* ABNORMAL CASE*/
		DBDC_FSM_MSG_WRONG_EVT(eEvent);
		break;

	default:
		/* WRONG EVENT */
		DBDC_FSM_MSG_ERROR_EVT(eEvent);
		break;
	}

	cnmDbdcFsmSteps(prAdapter, g_rDbdcInfo.eDbdcFsmNextState, eEvent);
}

static VOID
cnmDbdcFsmEventHandler_WAIT_HW_DISABLE(
	IN P_ADAPTER_T                  prAdapter,
	IN enum ENUM_DBDC_FSM_EVENT_T   eEvent
	)
{
	switch (eEvent) {
	case DBDC_FSM_EVENT_BSS_DISCONNECT_LEAVE_AG:
	case DBDC_FSM_EVENT_BSS_CONNECTING_ENTER_AG:
		/* IGNORE */
		break;

	case DBDC_FSM_EVENT_SWITCH_GUARD_TIME_TO:
	case DBDC_FSM_EVENT_DISABLE_COUNT_DOWN_TO:
	case DBDC_FSM_EVENT_ACTION_FRAME_ALL_SUCCESS:
	case DBDC_FSM_EVENT_ACTION_FRAME_SOME_FAIL:
		/* ABNORMAL CASE*/
		DBDC_FSM_MSG_WRONG_EVT(eEvent);
		break;

	case DBDC_FSM_EVENT_DBDC_HW_SWITCH_DONE:
		g_rDbdcInfo.eDbdcFsmNextState = ENUM_DBDC_FSM_STATE_DISABLE_GUARD;
		break;

	default:
		/* WRONG EVENT */
		DBDC_FSM_MSG_ERROR_EVT(eEvent);
		break;
	}

	cnmDbdcFsmSteps(prAdapter, g_rDbdcInfo.eDbdcFsmNextState, eEvent);
}

static VOID
cnmDbdcFsmEventHandler_DISABLE_GUARD(
	IN P_ADAPTER_T                  prAdapter,
	IN enum ENUM_DBDC_FSM_EVENT_T   eEvent
	)
{
	switch (eEvent) {
	case DBDC_FSM_EVENT_BSS_DISCONNECT_LEAVE_AG:
	case DBDC_FSM_EVENT_BSS_CONNECTING_ENTER_AG:
		/* IGNORE */
		break;

	case DBDC_FSM_EVENT_SWITCH_GUARD_TIME_TO:
		if (g_rDbdcInfo.fgDbdcDisableOpmodeChangeDone) {
			if (cnmDbdcIsAGConcurrent(prAdapter, BAND_NULL)) {
				switch (cnmDbdcOpmodeChangeAndWait(prAdapter, TRUE)) {
				case ENUM_DBDC_PROTOCOL_STATUS_WAIT:
					g_rDbdcInfo.eDbdcFsmNextState = ENUM_DBDC_FSM_STATE_WAIT_PROTOCOL_ENABLE;
					break;
				case ENUM_DBDC_PROTOCOL_STATUS_DONE_SUCCESS:
					g_rDbdcInfo.eDbdcFsmNextState = ENUM_DBDC_FSM_STATE_WAIT_HW_ENABLE;
					break;
				case ENUM_DBDC_PROTOCOL_STATUS_DONE_FAIL:
					if (cnmDbdcOpmodeChangeAndWait(prAdapter, FALSE) ==
						ENUM_DBDC_PROTOCOL_STATUS_WAIT)
						g_rDbdcInfo.eDbdcFsmNextState =
							ENUM_DBDC_FSM_STATE_WAIT_PROTOCOL_DISABLE;
					else
						g_rDbdcInfo.eDbdcFsmNextState = ENUM_DBDC_FSM_STATE_DISABLE_IDLE;
					break;
				default:
					break;
				}
			} else {
				g_rDbdcInfo.eDbdcFsmNextState = ENUM_DBDC_FSM_STATE_DISABLE_IDLE;
			}
		} else {
			g_rDbdcInfo.eDbdcFsmNextState = ENUM_DBDC_FSM_STATE_WAIT_PROTOCOL_DISABLE;
		}

		break;

	case DBDC_FSM_EVENT_DISABLE_COUNT_DOWN_TO:
		/* ABNORMAL CASE */
		DBDC_FSM_MSG_WRONG_EVT(eEvent);
		break;

	case DBDC_FSM_EVENT_ACTION_FRAME_ALL_SUCCESS:
	case DBDC_FSM_EVENT_ACTION_FRAME_SOME_FAIL:
		/* Do nothing */
		break;

	case DBDC_FSM_EVENT_DBDC_HW_SWITCH_DONE:
		/* ABNORMAL CASE */
		DBDC_FSM_MSG_WRONG_EVT(eEvent);
		break;

	default:
		/* WRONG EVENT */
		DBDC_FSM_MSG_ERROR_EVT(eEvent);
		break;
	}

	cnmDbdcFsmSteps(prAdapter, g_rDbdcInfo.eDbdcFsmNextState, eEvent);
}

static VOID
cnmDbdcFsmEventHandler_WAIT_PROTOCOL_DISABLE(
	IN P_ADAPTER_T                  prAdapter,
	IN enum ENUM_DBDC_FSM_EVENT_T   eEvent
	)
{
	/* Prepare to Enable DBDC */

	switch (eEvent) {
	case DBDC_FSM_EVENT_BSS_DISCONNECT_LEAVE_AG:
	case DBDC_FSM_EVENT_BSS_CONNECTING_ENTER_AG:
		/* IGNORE */
		break;

	case DBDC_FSM_EVENT_SWITCH_GUARD_TIME_TO:
	case DBDC_FSM_EVENT_DISABLE_COUNT_DOWN_TO:
		/* ABNORMAL CASE*/
		DBDC_FSM_MSG_WRONG_EVT(eEvent);
		break;

	case DBDC_FSM_EVENT_ACTION_FRAME_ALL_SUCCESS:
	case DBDC_FSM_EVENT_ACTION_FRAME_SOME_FAIL:
		g_rDbdcInfo.eDbdcFsmNextState = ENUM_DBDC_FSM_STATE_DISABLE_IDLE;
		break;

	case DBDC_FSM_EVENT_DBDC_HW_SWITCH_DONE:
		/* ABNORMAL CASE*/
		DBDC_FSM_MSG_WRONG_EVT(eEvent);
		break;

	default:
		/* WRONG EVENT */
		DBDC_FSM_MSG_ERROR_EVT(eEvent);
		break;
	}

	cnmDbdcFsmSteps(prAdapter, g_rDbdcInfo.eDbdcFsmNextState, eEvent);
}

static VOID
cnmDbdcFsmExitFunc_WAIT_HW_ENABLE(
	IN P_ADAPTER_T	prAdapter
	)
{
	cnmDBDCFsmActionReqPeivilegeUnLock(prAdapter);
}

/*----------------------------------------------------------------------------*/
/*!
* @brief    Get the connection capability.
*
* @param (none)
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
VOID cnmGetDbdcCapability(
	IN P_ADAPTER_T			prAdapter,
	IN UINT_8				ucBssIndex,
	IN ENUM_BAND_T			eRfBand,
	IN UINT_8				ucPrimaryChannel,
	IN UINT_8				ucNss,
	OUT P_CNM_DBDC_CAP_T	prDbdcCap
)
{
	if (!prDbdcCap)
		return;

	/* BSS index */
	prDbdcCap->ucBssIndex = ucBssIndex;

	/* WMM set */
	if (eRfBand == BAND_5G)
		prDbdcCap->ucWmmSetIndex = DBDC_5G_WMM_INDEX;
	else
		prDbdcCap->ucWmmSetIndex =
			(prAdapter->rWifiVar.eDbdcMode == ENUM_DBDC_MODE_DISABLED) ?
			DBDC_5G_WMM_INDEX : DBDC_2G_WMM_INDEX;

	/* Nss & band 0/1 */
	switch (prAdapter->rWifiVar.eDbdcMode) {
	case ENUM_DBDC_MODE_DISABLED:
		/* DBDC is disabled, all BSS run on band 0 */
		if (wlanGetSupportNss(prAdapter, ucBssIndex) < ucNss)
			prDbdcCap->ucNss = wlanGetSupportNss(prAdapter, ucBssIndex);
		else
			prDbdcCap->ucNss = ucNss;
		prDbdcCap->ucDbdcBandIndex = ENUM_BAND_0;
		break;

	case ENUM_DBDC_MODE_STATIC:
		/* Static DBDC mode, 1SS only */
		prDbdcCap->ucNss = 1;
		if (eRfBand == BAND_5G)
			prDbdcCap->ucDbdcBandIndex = ENUM_BAND_0;
		else
			prDbdcCap->ucDbdcBandIndex = ENUM_BAND_1;
		break;

	case ENUM_DBDC_MODE_DYNAMIC:
		if (USE_DBDC_CAPABILITY()) {
			/* CODEING_STYLE_WORKAROUND
			* Gerrit wish no {} for "one line if statement", but build command as } before else
			*/
			prDbdcCap->ucNss = 1;
		} else
			prDbdcCap->ucNss = wlanGetSupportNss(prAdapter, ucBssIndex);
		prDbdcCap->ucDbdcBandIndex = ENUM_BAND_AUTO;
		break;

	default:
		break;
	}

	DBGLOG(CNM, INFO, "[DBDC] BSS%u RF%u CH%u Nss%u get Wmm%u Nss%u\n",
						ucBssIndex,
						eRfBand,
						ucPrimaryChannel,
						ucNss,
						prDbdcCap->ucWmmSetIndex,
						prDbdcCap->ucNss
						);
}

/*----------------------------------------------------------------------------*/
/*!
* @brief Get maximum bandwidth capability with considering DBDC mode
*
* @param (none)
*
* @return
*/
/*----------------------------------------------------------------------------*/
UINT_8 cnmGetDbdcBwCapability(IN P_ADAPTER_T prAdapter, IN UINT_8 ucBssIndex)
{
	UINT_8 ucMaxBw = MAX_BW_20MHZ;

	ucMaxBw = cnmGetBssMaxBw(prAdapter, ucBssIndex);

	/* Can't use BW160 when DBDC enabled */
	if (USE_DBDC_CAPABILITY() && (ucMaxBw >= MAX_BW_160MHZ))
		ucMaxBw = MAX_BW_80MHZ;

	/* TODO: BW80+80 support */
	if (ucMaxBw == MAX_BW_80_80_MHZ)
		ucMaxBw = MAX_BW_80MHZ; /* VHT should default support BW80 */

	return ucMaxBw;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief    Run-time check if DBDC Need enable or update guard time.
*
* @param (none)
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
VOID cnmDbdcEnableDecision(
	IN P_ADAPTER_T	prAdapter,
	IN UINT_8		ucChangedBssIndex,
	IN ENUM_BAND_T	eRfBand
)
{
	DBGLOG(CNM, INFO, "[DBDC] BSS %u Rf %u", ucChangedBssIndex, eRfBand);

	if (prAdapter->rWifiVar.eDbdcMode != ENUM_DBDC_MODE_DYNAMIC) {
		DBGLOG(CNM, INFO, "[DBDC Debug] DBDC Mode %u Return", prAdapter->rWifiVar.eDbdcMode);
		return;
	}

	if (prAdapter->rWifiVar.fgDbDcModeEn) {
		if (timerPendingTimer(&g_rDbdcInfo.rDbdcGuardTimer) &&
			g_rDbdcInfo.eDdbcGuardTimerType == ENUM_DBDC_GUARD_TIMER_SWITCH_GUARD_TIME) {
			/* update timer for connection retry */
			DBGLOG(CNM, INFO, "[DBDC] DBDC guard time extend\n");
			cnmTimerStopTimer(prAdapter,
								&g_rDbdcInfo.rDbdcGuardTimer);
			cnmTimerStartTimer(prAdapter,
								&g_rDbdcInfo.rDbdcGuardTimer,
								DBDC_SWITCH_GUARD_TIME);
		}
		return;
	}

	if (timerPendingTimer(&g_rDbdcInfo.rDbdcGuardTimer) &&
		g_rDbdcInfo.eDdbcGuardTimerType == ENUM_DBDC_GUARD_TIMER_SWITCH_GUARD_TIME) {
		DBGLOG(CNM, INFO, "[DBDC Debug] Guard Time Return");
		return;
	}

	if (eRfBand != BAND_2G4 && eRfBand != BAND_5G) {
		DBGLOG(CNM, INFO, "[DBDC Debug] Wrong RF band Return");
		return;
	}

	if (cnmDbdcIsAGConcurrent(prAdapter, eRfBand))
		DBDC_FSM_EVENT_HANDLER(prAdapter, DBDC_FSM_EVENT_BSS_CONNECTING_ENTER_AG);
}

/*----------------------------------------------------------------------------*/
/*!
* @brief    Run-time check if DBDC Need disable or update guard time.
*
* @param (none)
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
VOID cnmDbdcDisableDecision(IN P_ADAPTER_T prAdapter,	IN UINT_8 ucChangedBssIndex)
{
	DBGLOG(CNM, INFO, "[DBDC Debug] BSS %u", ucChangedBssIndex);

	if (prAdapter->rWifiVar.eDbdcMode != ENUM_DBDC_MODE_DYNAMIC) {
		DBGLOG(CNM, INFO, "[DBDC Debug] DBDC Mode %u Return", prAdapter->rWifiVar.eDbdcMode);
		return;
	}

	if (!prAdapter->rWifiVar.fgDbDcModeEn) {
		if (timerPendingTimer(&g_rDbdcInfo.rDbdcGuardTimer) &&
			g_rDbdcInfo.eDdbcGuardTimerType == ENUM_DBDC_GUARD_TIMER_SWITCH_GUARD_TIME) {
			/* update timer for connection retry */
			DBGLOG(CNM, INFO, "[DBDC] DBDC guard time extend\n");
			cnmTimerStopTimer(prAdapter,
								&g_rDbdcInfo.rDbdcGuardTimer);
			cnmTimerStartTimer(prAdapter,
								&g_rDbdcInfo.rDbdcGuardTimer,
								DBDC_SWITCH_GUARD_TIME);
		}
		return;
	}

	if (timerPendingTimer(&g_rDbdcInfo.rDbdcGuardTimer) &&
		g_rDbdcInfo.eDdbcGuardTimerType == ENUM_DBDC_GUARD_TIMER_DISABLE_COUNT_DOWN) {
		DBGLOG(CNM, INFO, "[DBDC Debug] Disable Countdown Return");
		return;
	}

	if (timerPendingTimer(&g_rDbdcInfo.rDbdcGuardTimer) &&
		g_rDbdcInfo.eDdbcGuardTimerType == ENUM_DBDC_GUARD_TIMER_SWITCH_GUARD_TIME) {
		DBGLOG(CNM, INFO, "[DBDC Debug] Guard Time Return");
		return;
	}

	if (!cnmDbdcIsAGConcurrent(prAdapter, BAND_NULL))
		DBDC_FSM_EVENT_HANDLER(prAdapter, DBDC_FSM_EVENT_BSS_DISCONNECT_LEAVE_AG);
}

/*----------------------------------------------------------------------------*/
/*!
* @brief    DBDC Guard Time/Countdown Callback
*
* @param (none)
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
VOID cnmDbdcGuardTimerCallback(IN P_ADAPTER_T prAdapter, IN ULONG plParamPtr)
{
	DBGLOG(CNM, INFO, "[DBDC Debug] Timer %u", g_rDbdcInfo.eDdbcGuardTimerType);

	if (prAdapter->rWifiVar.eDbdcMode != ENUM_DBDC_MODE_DYNAMIC) {
		DBGLOG(CNM, INFO, "[DBDC Debug] DBDC Mode %u Return", prAdapter->rWifiVar.eDbdcMode);
		return;
	}

	if (g_rDbdcInfo.eDdbcGuardTimerType == ENUM_DBDC_GUARD_TIMER_SWITCH_GUARD_TIME) {

		g_rDbdcInfo.eDdbcGuardTimerType = ENUM_DBDC_GUARD_TIMER_NONE;
		DBDC_FSM_EVENT_HANDLER(prAdapter, DBDC_FSM_EVENT_SWITCH_GUARD_TIME_TO);

	} else if (g_rDbdcInfo.eDdbcGuardTimerType == ENUM_DBDC_GUARD_TIMER_DISABLE_COUNT_DOWN) {

		g_rDbdcInfo.eDdbcGuardTimerType = ENUM_DBDC_GUARD_TIMER_NONE;
		DBDC_FSM_EVENT_HANDLER(prAdapter, DBDC_FSM_EVENT_DISABLE_COUNT_DOWN_TO);

	} else
		DBGLOG(CNM, ERROR, "[DBDC] WRONG DBDC TO TYPE %u\n", g_rDbdcInfo.eDdbcGuardTimerType);
}

/*----------------------------------------------------------------------------*/
/*!
* @brief    DBDC HW Switch done event
*
* @param (none)
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
VOID cnmDbdcEventHwSwitchDone(IN P_ADAPTER_T prAdapter, IN P_WIFI_EVENT_T prEvent)
{
	P_CMD_INFO_T prCmdInfo;
	UINT_8 ucBssIndex;
	P_BSS_INFO_T prBssInfo;
	BOOLEAN fgDbdcEn;

	/* command response handling */
	prCmdInfo = nicGetPendingCmdInfo(prAdapter, prEvent->ucSeqNum);

	if (prCmdInfo != NULL) {
		if (prCmdInfo->pfCmdDoneHandler)
			prCmdInfo->pfCmdDoneHandler(prAdapter, prCmdInfo, prEvent->aucBuffer);
		else if (prCmdInfo->fgIsOid)
			kalOidComplete(prAdapter->prGlueInfo, prCmdInfo->fgSetQuery, 0, WLAN_STATUS_SUCCESS);
		/* return prCmdInfo */
		cmdBufFreeCmdInfo(prAdapter, prCmdInfo);
	}

	/* Check DBDC state by FSM */
	if (g_rDbdcInfo.eDbdcFsmCurrState == ENUM_DBDC_FSM_STATE_WAIT_HW_ENABLE) {
		fgDbdcEn = TRUE;
	} else if (g_rDbdcInfo.eDbdcFsmCurrState == ENUM_DBDC_FSM_STATE_WAIT_HW_DISABLE) {
		fgDbdcEn = FALSE;
	} else {
		DBGLOG(CNM, ERROR, "[DBDC] switch event happen in state %u\n", g_rDbdcInfo.eDbdcFsmCurrState);
		return;
	}

	/* Change DBDC state */
	prAdapter->rWifiVar.fgDbDcModeEn = fgDbdcEn;
	for (ucBssIndex = 0; ucBssIndex <= prAdapter->ucHwBssIdNum; ucBssIndex++) {
		prBssInfo = prAdapter->aprBssInfo[ucBssIndex];

		if (prBssInfo->eBand == BAND_2G4) {
			if (fgDbdcEn)
				prBssInfo->eDBDCBand = ENUM_BAND_1;
			else
				prBssInfo->eDBDCBand = ENUM_BAND_0;
		}
	}

	DBDC_FSM_EVENT_HANDLER(prAdapter, DBDC_FSM_EVENT_DBDC_HW_SWITCH_DONE);
}

#endif /*CFG_SUPPORT_DBDC*/


enum ENUM_CNM_NETWORK_TYPE_T cnmGetBssNetworkType(P_BSS_INFO_T prBssInfo)
{
	if (prBssInfo->eNetworkType == NETWORK_TYPE_AIS)
		return ENUM_CNM_NETWORK_TYPE_AIS;
	else if (prBssInfo->eNetworkType == NETWORK_TYPE_P2P) {
		if (prBssInfo->eCurrentOPMode == OP_MODE_INFRASTRUCTURE)
			return ENUM_CNM_NETWORK_TYPE_P2P_GC;
		else if (prBssInfo->eCurrentOPMode == OP_MODE_ACCESS_POINT)
			return ENUM_CNM_NETWORK_TYPE_P2P_GO;
	}
	return ENUM_CNM_NETWORK_TYPE_OTHER;
}
