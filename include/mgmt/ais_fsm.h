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
 * Id: //Department/DaVinci/BRANCHES/MT6620_WIFI_DRIVER_V2_3/
 *						include/mgmt/ais_fsm.h#2
 */

/*! \file   ais_fsm.h
 *  \brief  Declaration of functions and finite state machine for AIS Module.
 *
 *  Declaration of functions and finite state machine for AIS Module.
 */


#ifndef _AIS_FSM_H
#define _AIS_FSM_H

/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */

/*******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 *******************************************************************************
 */

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */

#define AIS_DEFAULT_INDEX (0)
#define AIS_DEFAULT_BSS_INDEX (0)
#define AIS_SECONDARY_INDEX (1)

#define AIS_BG_SCAN_INTERVAL_MIN_SEC        2	/* 30 // exponential to 960 */
#define AIS_BG_SCAN_INTERVAL_MAX_SEC        2	/* 960 // 16min */

#define AIS_DELAY_TIME_OF_DISCONNECT_SEC    5	/* 10 */

#define AIS_IBSS_ALONE_TIMEOUT_SEC          20	/* seconds */

#define AIS_BEACON_TIMEOUT_COUNT_ADHOC      30
#define AIS_BEACON_TIMEOUT_COUNT_INFRA      10
#define AIS_BEACON_TIMEOUT_GUARD_TIME_SEC   1	/* Second */

#define AIS_BEACON_MAX_TIMEOUT_TU           100
#define AIS_BEACON_MIN_TIMEOUT_TU           5
#define AIS_BEACON_MAX_TIMEOUT_VALID        TRUE
#define AIS_BEACON_MIN_TIMEOUT_VALID        TRUE

#define AIS_BMC_MAX_TIMEOUT_TU              100
#define AIS_BMC_MIN_TIMEOUT_TU              5
#define AIS_BMC_MAX_TIMEOUT_VALID           TRUE
#define AIS_BMC_MIN_TIMEOUT_VALID           TRUE

#define AIS_JOIN_CH_GRANT_THRESHOLD         10
#define AIS_JOIN_CH_REQUEST_INTERVAL        4000
#define AIS_SCN_DONE_TIMEOUT_SEC            15 /* 15 for 2.4G + 5G */	/* 5 */
#define AIS_SCN_REPORT_SEQ_NOT_SET          (0xFFFF)

/* Support AP Selection*/
#define AIS_BLACKLIST_TIMEOUT               15 /* seconds */
#define AIS_AUTORN_MIN_INTERVAL		    20

#define AIS_MAIN_BSS_INDEX(_adapter, _ais_idx) \
	aisGetMainLinkBssIndex(_adapter, aisFsmGetInstance(_adapter, _ais_idx))

#define AIS_MAIN_BSS_INFO(_adapter, _ais_idx) \
	aisGetMainLinkBssInfo(aisFsmGetInstance(_adapter, _ais_idx))

#define AIS_INDEX(_adapter, _bss_idx) \
	aisGetAisFsmInfo(_adapter, _bss_idx)->ucAisIndex

/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */
enum ENUM_AIS_STATE {
	AIS_STATE_IDLE = 0,
	AIS_STATE_SEARCH,
	AIS_STATE_SCAN,
	AIS_STATE_ONLINE_SCAN,
	AIS_STATE_LOOKING_FOR,
	AIS_STATE_WAIT_FOR_NEXT_SCAN,
	AIS_STATE_REQ_CHANNEL_JOIN,
	AIS_STATE_JOIN,
	AIS_STATE_JOIN_FAILURE,
	AIS_STATE_IBSS_ALONE,
	AIS_STATE_IBSS_MERGE,
	AIS_STATE_NORMAL_TR,
	AIS_STATE_DISCONNECTING,
	AIS_STATE_REQ_REMAIN_ON_CHANNEL,
	AIS_STATE_REMAIN_ON_CHANNEL,
	AIS_STATE_OFF_CHNL_TX,
	AIS_STATE_NUM
};

#if CFG_SUPPORT_CSI
enum CSI_DATA_TLV_TAG {
	CSI_DATA_VER,
	CSI_DATA_TYPE,
	CSI_DATA_TS,
	CSI_DATA_RSSI,
	CSI_DATA_SNR,
	CSI_DATA_DBW,
	CSI_DATA_CH_IDX,
	CSI_DATA_TA,
	CSI_DATA_I,
	CSI_DATA_Q,
	CSI_DATA_EXTRA_INFO,
	CSI_DATA_RSVD1,
	CSI_DATA_RSVD2,
	CSI_DATA_RSVD3,
	CSI_DATA_RSVD4,
	CSI_DATA_TX_IDX,
	CSI_DATA_RX_IDX,
	CSI_DATA_FRAME_MODE,
	CSI_DATA_H_IDX,
	CSI_DATA_RX_RATE,
	CSI_DATA_TLV_TAG_NUM,
};
#endif

/* reconnect level for determining if we should reconnect */
enum ENUM_RECONNECT_LEVEL_T {
	RECONNECT_LEVEL_MIN = 0,
	RECONNECT_LEVEL_ROAMING_FAIL,	/* roaming failed */
	RECONNECT_LEVEL_BEACON_TIMEOUT,	/* driver beacon timeout */
	RECONNECT_LEVEL_USER_SET,	/* user set connect	 */
					/*	 or disassociate */
	RECONNECT_LEVEL_MAX
};

struct MSG_AIS_ABORT {
	struct MSG_HDR rMsgHdr;	/* Must be the first member */
	uint8_t ucReasonOfDisconnect;
	u_int8_t fgDelayIndication;
	uint8_t ucBssIndex;
};

struct MSG_AIS_IBSS_PEER_FOUND {
	struct MSG_HDR rMsgHdr;	/* Must be the first member */
	uint8_t ucBssIndex;
	u_int8_t fgIsMergeIn;	/* TRUE: Merge In, FALSE: Merge Out */
	struct STA_RECORD *prStaRec;
};

enum ENUM_AIS_REQUEST_TYPE {
	AIS_REQUEST_SCAN,
	AIS_REQUEST_RECONNECT,
	AIS_REQUEST_ROAMING_SEARCH,
	AIS_REQUEST_ROAMING_CONNECT,
	AIS_REQUEST_REMAIN_ON_CHANNEL,
	AIS_REQUEST_NUM
};

struct AIS_REQ_HDR {
	struct LINK_ENTRY rLinkEntry;
	enum ENUM_AIS_REQUEST_TYPE eReqType;
};

struct AIS_REQ_CHNL_INFO {
	enum ENUM_BAND eBand;
	enum ENUM_CHNL_EXT eSco;
	uint8_t ucChannelNum;
	uint32_t u4DurationMs;
	uint64_t u8Cookie;
	enum ENUM_CH_REQ_TYPE eReqType;
};

struct AIS_MGMT_TX_REQ_INFO {
	struct LINK rTxReqLink;
	u_int8_t fgIsMgmtTxRequested;
	struct MSDU_INFO *prMgmtTxMsdu;
	uint64_t u8Cookie;
};

struct ESS_CHNL_INFO {
	uint8_t ucChannel;
	uint8_t ucUtilization;
	uint8_t ucApNum;
	enum ENUM_BAND eBand;
};

struct AIS_SPECIFIC_BSS_INFO {
	/* This value indicate the roaming type used in AIS_JOIN */
	uint8_t ucRoamingAuthTypes;

	u_int8_t fgIsIBSSActive;

	/*! \brief Global flag to let arbiter stay at standby
	 *  and not connect to any network
	 */
	u_int8_t fgCounterMeasure;

#if 0
	u_int8_t fgWepWapiBcKeyExist;	/* WEP WAPI BC key exist flag */
	uint8_t ucWepWapiBcWlanIndex;	/* WEP WAPI BC wlan index */

	/* RSN BC key exist flag, map to key id 0, 1, 2, 3 */
	u_int8_t fgRsnBcKeyExist[4];
	/* RSN BC wlan index, map to key id 0, 1, 2, 3 */
	uint8_t ucRsnBcWlanIndex[4];
#endif

	/* While Do CounterMeasure procedure,
	 * check the EAPoL Error report have send out
	 */
	u_int8_t fgCheckEAPoLTxDone;

	uint32_t u4RsnaLastMICFailTime;

	/* By the flow chart of 802.11i,
	 *  wait 60 sec before associating to same AP
	 *  or roaming to a new AP
	 *  or sending data in IBSS,
	 *  keep a timer for handle the 60 sec counterMeasure
	 */
	struct TIMER rRsnaBlockTrafficTimer;
	struct TIMER rRsnaEAPoLReportTimeoutTimer;

	/* For Keep the Tx/Rx Mic key for TKIP SW Calculate Mic */
	/* This is only one for AIS/AP */
	uint8_t aucTxMicKey[8];
	uint8_t aucRxMicKey[8];

	/* Buffer for WPA2 PMKID */
	/* The PMKID cache lifetime is expire by media_disconnect_indication */
	struct LINK rPmkidCache;
#if CFG_SUPPORT_802_11W
	u_int8_t fgMgmtProtection;
	uint32_t u4SaQueryStart;
	uint32_t u4SaQueryCount;
	uint8_t ucSaQueryTimedOut;
	uint8_t *pucSaQueryTransId;
	struct TIMER rSaQueryTimer;
	u_int8_t fgBipKeyInstalled;
#endif
	uint8_t ucKeyAlgorithmId;

	/* Support AP Selection */
#if CFG_SUPPORT_ROAMING_SKIP_ONE_AP
	uint8_t	ucRoamSkipTimes;
	u_int8_t fgGoodRcpiArea;
	u_int8_t fgPoorRcpiArea;
#endif
	struct ESS_CHNL_INFO arCurEssChnlInfo[CFG_MAX_NUM_OF_CHNL_INFO];
	uint8_t ucCurEssChnlInfoNum;
	struct LINK rCurEssLink;
	/* end Support AP Selection */

	struct BSS_TRANSITION_MGT_PARAM rBTMParam;
	struct LINK_MGMT  rNeighborApList;
	OS_SYSTIME rNeiApRcvTime;
	uint32_t u4NeiApValidInterval;
};

/* Session for CONNECTION SETTINGS */
struct CONNECTION_SETTINGS {

	uint8_t aucMacAddress[MAC_ADDR_LEN];

	uint8_t ucDelayTimeOfDisconnectEvent;

	u_int8_t fgIsConnByBssidIssued;
	uint8_t aucBSSID[MAC_ADDR_LEN];
	uint8_t aucBSSIDHint[MAC_ADDR_LEN];

	uint8_t ucSSIDLen;
	uint8_t aucSSID[ELEM_MAX_LEN_SSID];

	enum ENUM_PARAM_OP_MODE eOPMode;

	enum ENUM_PARAM_CONNECTION_POLICY eConnectionPolicy;

	enum ENUM_PARAM_AD_HOC_MODE eAdHocMode;

	enum ENUM_PARAM_AUTH_MODE eAuthMode;

	enum ENUM_WEP_STATUS eEncStatus;

	u_int8_t fgIsScanReqIssued;

	/* MIB attributes */
	uint16_t u2BeaconPeriod;

	uint16_t u2RTSThreshold;	/* User desired setting */

	uint16_t u2DesiredNonHTRateSet;	/* User desired setting */

	uint8_t ucAdHocChannelNum;	/* For AdHoc */

	enum ENUM_BAND eAdHocBand;	/* For AdHoc */

	uint32_t u4FreqInKHz;	/* Center frequency */

	/* ATIM windows using for IBSS power saving function */
	uint16_t u2AtimWindow;

	/* Features */
	u_int8_t fgIsEnableRoaming;

	u_int8_t fgIsAdHocQoSEnable;

	enum ENUM_PARAM_PHY_CONFIG eDesiredPhyConfig;

#if CFG_SUPPORT_802_11D
	u_int8_t fgMultiDomainCapabilityEnabled;
#endif				/* CFG_SUPPORT_802_11D */

#if 1				/* CFG_SUPPORT_WAPI */
	u_int8_t fgWapiMode;
	uint32_t u4WapiSelectedGroupCipher;
	uint32_t u4WapiSelectedPairwiseCipher;
	uint32_t u4WapiSelectedAKMSuite;
#endif

	/* for cfg80211 connected indication */
	uint32_t u4RspIeLength;
	uint8_t aucRspIe[CFG_CFG80211_IE_BUF_LEN];

	uint32_t u4ReqIeLength;
	uint8_t aucReqIe[CFG_CFG80211_IE_BUF_LEN];

	u_int8_t fgWpsActive;
	uint8_t aucWSCIE[GLUE_INFO_WSCIE_LENGTH];	/*for probe req */
	uint16_t u2WSCIELen;

	/*
	 * Buffer to hold non-wfa vendor specific IEs set
	 * from wpa_supplicant. This is used in sending
	 * Association Request in AIS mode.
	 */
	uint16_t non_wfa_vendor_ie_len;
	uint8_t non_wfa_vendor_ie_buf[NON_WFA_VENDOR_IE_MAX_LEN];

	/* 11R */
	struct FT_IES rFtIeForTx;
	struct cfg80211_ft_event_params rFtEventParam;

	/* CR1486, CR1640 */
	/* for WPS, disable the privacy check for AP selection policy */
	u_int8_t fgPrivacyCheckDisable;

	/* b0~3: trigger-en AC0~3. b4~7: delivery-en AC0~3 */
	uint8_t bmfgApsdEnAc;

	/* for RSN info store, when upper layer set rsn info */
	struct RSN_INFO rRsnInfo;

#if CFG_SUPPORT_DETECT_SECURITY_MODE_CHANGE
	u_int8_t fgSecModeChangeStartTimer;
#endif

	uint8_t *pucAssocIEs;
	size_t assocIeLen;
	u_int8_t fgAuthOsenWithRSN;
};

struct AIS_LINK_INFO {
	struct BSS_INFO *prBssInfo;
	struct BSS_DESC *prTargetBssDesc;
	struct STA_RECORD *prTargetStaRec;
};

/* Support AP Selection */
struct AIS_BLACKLIST_ITEM {
	struct LINK_ENTRY rLinkEntry;

	uint8_t aucBSSID[MAC_ADDR_LEN];
	uint16_t u2DeauthReason;
	uint16_t u2AuthStatus;
	uint8_t ucCount;
	uint8_t ucSSIDLen;
	uint8_t aucSSID[32];
	OS_SYSTIME rAddTime;
	u_int8_t fgDeauthLastTime;
	u_int8_t fgIsInFWKBlacklist;
#if CFG_SUPPORT_MBO
	uint8_t fgDisallowed;
	uint16_t u2DisallowSec;
	int32_t i4RssiThreshold;
#endif
};
/* end Support AP Selection */

struct AX_BLACKLIST_ITEM {
	struct LINK_ENTRY rLinkEntry;
	uint8_t aucBSSID[MAC_ADDR_LEN];
};

struct AIS_FSM_INFO {
	enum ENUM_AIS_STATE ePreviousState;
	enum ENUM_AIS_STATE eCurrentState;

	u_int8_t ucAisIndex;

	u_int8_t fgIsScanning;

	u_int8_t fgIsChannelRequested;
	u_int8_t fgIsChannelGranted;

	uint8_t ucChReqNum;

	uint8_t ucAvailableAuthTypes; /* Used for AUTH_MODE_AUTO_SWITCH */

	uint8_t ucLinkNum;
	uint8_t arBssId2LinkMap[MAX_BSSID_NUM + 1];
	struct AIS_LINK_INFO aprLinkInfo[MLD_LINK_MAX];

	struct CONNECTION_SETTINGS rConnSettings;

#if CFG_SUPPORT_ROAMING
	struct ROAMING_INFO rRoamingInfo;
#endif	/* CFG_SUPPORT_ROAMING */

	struct AIS_SPECIFIC_BSS_INFO rAisSpecificBssInfo;

	uint32_t u4SleepInterval;

	struct TIMER rBGScanTimer;

	struct TIMER rIbssAloneTimer;

	uint32_t u4PostponeIndStartTime;

	struct TIMER rJoinTimeoutTimer;

	struct TIMER rChannelTimeoutTimer;

	struct TIMER rScanDoneTimer;

	struct TIMER rDeauthDoneTimer;

#if CFG_SUPPORT_DETECT_SECURITY_MODE_CHANGE
	struct TIMER rSecModeChangeTimer;
#endif
	struct TIMER rBtmRespTxDoneTimer;

	uint8_t ucSeqNumOfReqMsg;
	uint8_t ucSeqNumOfChReq;
	uint8_t ucSeqNumOfScanReq;

	/* Save SeqNum for reporting scan done.
	 * In order to distinguish seq num and default value, make sure that
	 * sizeof(u2SeqNumOfScanReport) > sizeof(ucSeqNumOfScanReq).
	 * Set AIS_SCN_REPORT_SEQ_NOT_SET as default value
	 */
	uint16_t u2SeqNumOfScanReport;

	uint32_t u4ChGrantedInterval;

	uint8_t ucConnTrialCount;
	uint8_t ucConnTrialCountLimit;

	struct PARAM_SCAN_REQUEST_ADV rScanRequest;
	uint8_t aucScanIEBuf[MAX_IE_LENGTH];

	u_int8_t fgIsScanOidAborted;

	/* Pending Request List */
	struct LINK rPendingReqList;

	/* Join Request Timestamp */
	OS_SYSTIME rJoinReqTime;

	/* for cfg80211 REMAIN_ON_CHANNEL support */
	struct AIS_REQ_CHNL_INFO rChReqInfo;

	/* Mgmt tx related. */
	struct AIS_MGMT_TX_REQ_INFO rMgmtTxInfo;

	/* Packet filter for AIS module. */
	uint32_t u4AisPacketFilter;

	/* for roaming target */
	struct PARAM_SSID rRoamingSSID;

	/* Support AP Selection */
	uint8_t ucJoinFailCntAfterScan;
	/* end Support AP Selection */

	/* 11K */
	struct RADIO_MEASUREMENT_REQ_PARAMS rRmReqParams;
	struct RADIO_MEASUREMENT_REPORT_PARAMS rRmRepParams;

	/* WMMAC */
	struct WMM_INFO rWmmInfo;

#if CFG_SUPPORT_PASSPOINT
	struct HS20_INFO rHS20Info;
#endif	/* CFG_SUPPORT_PASSPOINT */

#if CFG_SUPPORT_802_11W
	/* STA PMF: for encrypted deauth frame */
	kal_completion rDeauthComp;
	u_int8_t encryptedDeauthIsInProcess;
#endif

	/* MTK WLAN NIC driver IEEE 802.11 MIB */
	struct IEEE_802_11_MIB rMib;

	struct PARAM_BSSID_EX rCurrBssId;

	/*! \brief wext wpa related information */
	struct GL_WPA_INFO rWpaInfo;
#if CFG_SUPPORT_REPLAY_DETECTION
	struct GL_DETECT_REPLAY_INFO prDetRplyInfo;
#endif

	/* Indicated media state */
	enum ENUM_PARAM_MEDIA_STATE eParamMediaStateIndicated;

	uint8_t ucReasonOfDisconnect;	/* Used by media state indication */

	/* Scan target channel when device roaming */
	uint8_t fgTargetChnlScanIssued;

	struct LINK rAxBlacklist;
	struct LINK rHeHtcBlacklist;
#if (CFG_SUPPORT_802_11BE_MLO == 1)
	struct MLD_BSS_INFO *prMldBssInfo;
	uint8_t ucMlProbeSendCount;
	uint8_t ucMlProbeEnable;
#endif
};

struct AIS_OFF_CHNL_TX_REQ_INFO {
	struct LINK_ENTRY rLinkEntry;
	struct MSDU_INFO *prMgmtTxMsdu;
	u_int8_t fgNoneCckRate;
	struct RF_CHANNEL_INFO rChannelInfo;	/* Off channel TX. */
	enum ENUM_CHNL_EXT eChnlExt;
	/* See if driver should keep at the same channel. */
	u_int8_t fgIsWaitRsp;
	uint64_t u8Cookie; /* cookie used to match with supplicant */
	uint32_t u4Duration; /* wait time for tx request */
};

enum WNM_AIS_BSS_TRANSITION {
	BSS_TRANSITION_NO_MORE_ACTION,
	BSS_TRANSITION_REQ_ROAMING,
	BSS_TRANSITION_DISASSOC,
	BSS_TRANSITION_MAX_NUM
};
struct MSG_AIS_BSS_TRANSITION {
	struct MSG_HDR rMsgHdr;	/* Must be the first member */
	uint8_t ucBssIndex;
};
/*******************************************************************************
 *                            P U B L I C   D A T A
 *******************************************************************************
 */

/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */

/*******************************************************************************
 *                                 M A C R O S
 *******************************************************************************
 */
#define aisChangeMediaState(_prAisBssInfo, _eNewMediaState) \
	(_prAisBssInfo->eConnectionState = (_eNewMediaState))

/*******************************************************************************
 *                  F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */

void aisInitializeConnectionSettings(IN struct ADAPTER
		*prAdapter, IN struct REG_INFO *prRegInfo,
		IN uint8_t ucBssIndex);

void aisFsmInit(IN struct ADAPTER *prAdapter,
		IN struct REG_INFO *prRegInfo,
		IN uint8_t ucAisIndex);

void aisFsmUninit(IN struct ADAPTER *prAdapter, uint8_t ucAisIndex);

bool aisFsmIsInProcessPostpone(IN struct ADAPTER *prAdapter,
	uint8_t ucBssIndex);

bool aisFsmIsInBeaconTimeout(IN struct ADAPTER *prAdapter,
	uint8_t ucBssIndex);

bool aisFsmIsReassociation(IN struct ADAPTER *prAdapter,
	uint8_t ucBssIndex);

void aisFsmStateInit_JOIN(IN struct ADAPTER *prAdapter,
	struct AIS_FSM_INFO *prAisFsmInfo, uint8_t ucLinkIndex);

u_int8_t aisFsmStateInit_RetryJOIN(IN struct ADAPTER
				   *prAdapter, IN struct STA_RECORD *prStaRec,
				   uint8_t ucBssIndex);

void aisFsmStateInit_IBSS_ALONE(IN struct ADAPTER
				*prAdapter, uint8_t ucBssIndex);

void aisFsmStateInit_IBSS_MERGE(IN struct ADAPTER
	*prAdapter, struct BSS_DESC *prBssDesc, uint8_t ucBssIndex);

void aisFsmStateAbort(IN struct ADAPTER *prAdapter,
		      uint8_t ucReasonOfDisconnect, u_int8_t fgDelayIndication,
		      uint8_t ucBssIndex);

void aisFsmStateAbort_JOIN(IN struct ADAPTER *prAdapter, uint8_t ucBssIndex);

void aisFsmStateAbort_SCAN(IN struct ADAPTER *prAdapter, uint8_t ucBssIndex);

void aisFsmStateAbort_NORMAL_TR(IN struct ADAPTER
				*prAdapter, uint8_t ucBssIndex);

void aisFsmStateAbort_IBSS(IN struct ADAPTER *prAdapter, uint8_t ucBssIndex);

void aisFsmSteps(IN struct ADAPTER *prAdapter,
		 enum ENUM_AIS_STATE eNextState, uint8_t ucBssIndex);

/*----------------------------------------------------------------------------*/
/* Mailbox Message Handling                                                   */
/*----------------------------------------------------------------------------*/
void aisFsmRunEventScanDone(IN struct ADAPTER *prAdapter,
			    IN struct MSG_HDR *prMsgHdr);

void aisFsmRunEventAbort(IN struct ADAPTER *prAdapter,
			 IN struct MSG_HDR *prMsgHdr);

void aisFsmRunEventJoinComplete(IN struct ADAPTER
				*prAdapter, IN struct MSG_HDR *prMsgHdr);

enum ENUM_AIS_STATE aisFsmJoinCompleteAction(
	IN struct ADAPTER *prAdapter, IN struct MSG_HDR *prMsgHdr);

void aisFsmRunEventRemainOnChannel(IN struct ADAPTER
				   *prAdapter, IN struct MSG_HDR *prMsgHdr);

void aisFsmRunEventCancelRemainOnChannel(IN struct ADAPTER
		*prAdapter, IN struct MSG_HDR *prMsgHdr);

/*----------------------------------------------------------------------------*/
/* Handling for Ad-Hoc Network                                                */
/*----------------------------------------------------------------------------*/
void aisFsmCreateIBSS(IN struct ADAPTER *prAdapter, uint8_t ucBssIndex);

void aisFsmMergeIBSS(IN struct ADAPTER *prAdapter,
		     IN struct STA_RECORD *prStaRec);

/*----------------------------------------------------------------------------*/
/* Handling of Incoming Mailbox Message from CNM                              */
/*----------------------------------------------------------------------------*/
void aisFsmRunEventChGrant(IN struct ADAPTER *prAdapter,
			   IN struct MSG_HDR *prMsgHdr);

/*----------------------------------------------------------------------------*/
/* Generating Outgoing Mailbox Message to CNM                                 */
/*----------------------------------------------------------------------------*/
void aisFsmReleaseCh(IN struct ADAPTER *prAdapter, IN uint8_t ucBssIndex);

/*----------------------------------------------------------------------------*/
/* Event Indication                                                           */
/*----------------------------------------------------------------------------*/
void
aisIndicationOfMediaStateToHost(IN struct ADAPTER
				*prAdapter,
				enum ENUM_PARAM_MEDIA_STATE eConnectionState,
				u_int8_t fgDelayIndication,
				uint8_t ucBssIndex);

void aisPostponedEventOfDisconnTimeout(IN struct ADAPTER *prAdapter,
				IN uint8_t ucBssIndex);

void aisUpdateAllBssInfoForJOIN(IN struct ADAPTER *prAdapter,
	struct AIS_FSM_INFO *prAisFsmInfo,
	struct SW_RFB *prAssocRspSwRfb,
	struct STA_RECORD *prSetupStaRec);

void aisUpdateBssInfoForJOIN(IN struct ADAPTER *prAdapter,
			     struct STA_RECORD *prStaRec,
			     struct SW_RFB *prAssocRspSwRfb);
uint32_t
aisFsmRunEventMgmtFrameTxDone(IN struct ADAPTER *prAdapter,
			      IN struct MSDU_INFO *prMsduInfo,
			      IN enum ENUM_TX_RESULT_CODE rTxDoneStatus);

/*----------------------------------------------------------------------------*/
/* Disconnection Handling                                                     */
/*----------------------------------------------------------------------------*/
void aisFsmDisconnect(IN struct ADAPTER *prAdapter,
		      IN u_int8_t fgDelayIndication,
		      IN uint8_t ucBssIndex);

/*----------------------------------------------------------------------------*/
/* Event Handling                                                             */
/*----------------------------------------------------------------------------*/
void aisBssBeaconTimeout(IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex);

void aisBssBeaconTimeout_impl(IN struct ADAPTER *prAdapter,
	IN uint8_t ucBcnTimeoutReason, IN uint8_t ucDisconnectReason,
	IN uint8_t ucBssIndex);

void aisBssLinkDown(IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex);

#if CFG_SUPPORT_DETECT_SECURITY_MODE_CHANGE
void aisBssSecurityChanged(IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex);
#endif

uint32_t
aisDeauthXmitComplete(IN struct ADAPTER *prAdapter,
		      IN struct MSDU_INFO *prMsduInfo,
		      IN enum ENUM_TX_RESULT_CODE rTxDoneStatus);
uint32_t
aisDeauthXmitCompleteBss(IN struct ADAPTER *prAdapter,
		      IN uint8_t ucBssIndex,
		      IN enum ENUM_TX_RESULT_CODE rTxDoneStatus);

#if CFG_SUPPORT_ROAMING
void aisFsmRunEventRoamingDiscovery(
	IN struct ADAPTER *prAdapter,
	uint32_t u4ReqScan,
	uint8_t ucBssIndex);

enum ENUM_AIS_STATE aisFsmRoamingScanResultsUpdate(
				   IN struct ADAPTER *prAdapter,
				   IN uint8_t ucBssIndex);

void aisFsmRoamingDisconnectPrevAP(IN struct ADAPTER *prAdapter,
				   IN struct AIS_FSM_INFO *prAisFsmInfo,
				   IN struct STA_RECORD *prTargetStaRec);

void aisUpdateBssInfoForRoamingAP(IN struct ADAPTER
				  *prAdapter, IN struct STA_RECORD *prStaRec,
				  IN struct SW_RFB *prAssocRspSwRfb);
#endif /*CFG_SUPPORT_ROAMING */

/*----------------------------------------------------------------------------*/
/* Timeout Handling                                                           */
/*----------------------------------------------------------------------------*/
void aisFsmRunEventBGSleepTimeOut(IN struct ADAPTER
				  *prAdapter, unsigned long ulParamPtr);

void aisFsmRunEventIbssAloneTimeOut(IN struct ADAPTER
				    *prAdapter, unsigned long ulParamPtr);

void aisFsmRunEventJoinTimeout(IN struct ADAPTER *prAdapter,
			       unsigned long ulParamPtr);

void aisFsmRunEventChannelTimeout(IN struct ADAPTER
				  *prAdapter, unsigned long ulParamPtr);

void aisFsmRunEventDeauthTimeout(IN struct ADAPTER
				 *prAdapter, unsigned long ulParamPtr);

#if CFG_SUPPORT_DETECT_SECURITY_MODE_CHANGE
void aisFsmRunEventSecModeChangeTimeout(IN struct ADAPTER
					*prAdapter, unsigned long ulParamPtr);
#endif

/*----------------------------------------------------------------------------*/
/* OID/IOCTL Handling                                                         */
/*----------------------------------------------------------------------------*/
void aisFsmScanRequest(IN struct ADAPTER *prAdapter,
		       IN struct PARAM_SSID *prSsid, IN uint8_t *pucIe,
		       IN uint32_t u4IeLength,
		       IN uint8_t ucBssIndex);

void
aisFsmScanRequestAdv(IN struct ADAPTER *prAdapter,
		IN struct PARAM_SCAN_REQUEST_ADV *prRequestIn);

/*----------------------------------------------------------------------------*/
/* Internal State Checking                                                    */
/*----------------------------------------------------------------------------*/
u_int8_t aisFsmIsRequestPending(IN struct ADAPTER *prAdapter,
				IN enum ENUM_AIS_REQUEST_TYPE eReqType,
				IN u_int8_t bRemove,
				IN uint8_t ucBssIndex);

void aisFsmRemoveRoamingRequest(
	IN struct ADAPTER *prAdapter, IN uint8_t ucBssIndex);

struct AIS_REQ_HDR *aisFsmGetNextRequest(IN struct ADAPTER *prAdapter,
				IN uint8_t ucBssIndex);

u_int8_t aisFsmInsertRequest(IN struct ADAPTER *prAdapter,
			     IN enum ENUM_AIS_REQUEST_TYPE eReqType,
			     IN uint8_t ucBssIndex);

u_int8_t aisFsmInsertRequestToHead(IN struct ADAPTER *prAdapter,
			     IN enum ENUM_AIS_REQUEST_TYPE eReqType,
			     IN uint8_t ucBssIndex);

u_int8_t aisFsmClearRequest(IN struct ADAPTER *prAdapter,
			     IN enum ENUM_AIS_REQUEST_TYPE eReqType,
			     IN uint8_t ucBssIndex);

void aisFsmFlushRequest(IN struct ADAPTER *prAdapter,
				IN uint8_t ucBssIndex);

uint32_t
aisFuncTxMgmtFrame(IN struct ADAPTER *prAdapter,
		   IN struct AIS_MGMT_TX_REQ_INFO *prMgmtTxReqInfo,
		   IN struct MSDU_INFO *prMgmtTxMsdu, IN uint64_t u8Cookie,
		   IN uint8_t ucBssIndex);

void aisFsmRunEventMgmtFrameTx(IN struct ADAPTER *prAdapter,
				IN struct MSG_HDR *prMsgHdr);

#if CFG_SUPPORT_NCHO
void aisFsmRunEventNchoActionFrameTx(IN struct ADAPTER *prAdapter,
				IN struct MSG_HDR *prMsgHdr);
#endif

void aisFuncValidateRxActionFrame(IN struct ADAPTER *prAdapter,
				IN struct SW_RFB *prSwRfb);

void aisFsmRunEventBssTransition(IN struct ADAPTER *prAdapter,
				IN struct MSG_HDR *prMsgHdr);

void aisFsmBtmRespTxDoneTimeout(
	IN struct ADAPTER *prAdapter, unsigned long ulParam);

void aisFsmRunEventCancelTxWait(IN struct ADAPTER *prAdapter,
		IN struct MSG_HDR *prMsgHdr);

enum ENUM_AIS_STATE aisFsmStateSearchAction(
	IN struct ADAPTER *prAdapter, uint8_t ucBssIndex);
#if defined(CFG_TEST_MGMT_FSM) && (CFG_TEST_MGMT_FSM != 0)
void aisTest(void);
#endif /* CFG_TEST_MGMT_FSM */

/* Support AP Selection */
void aisRefreshFWKBlacklist(struct ADAPTER *prAdapter);
struct AIS_BLACKLIST_ITEM *aisAddBlacklist(struct ADAPTER *prAdapter,
	struct BSS_DESC *prBssDesc);
void aisRemoveBlackList(struct ADAPTER *prAdapter, struct BSS_DESC *prBssDesc);
void aisRemoveTimeoutBlacklist(struct ADAPTER *prAdapter);
struct AIS_BLACKLIST_ITEM *aisQueryBlackList(struct ADAPTER *prAdapter,
	struct BSS_DESC *prBssDesc);
void aisBssTmpDisallow(struct ADAPTER *prAdapter, struct BSS_DESC *prBssDesc,
	uint32_t sec, int32_t rssiThreshold, uint8_t ucBssIndex);

/* Support 11K */
#if CFG_SUPPORT_802_11K
uint32_t aisCollectNeighborAP(struct ADAPTER *prAdapter, uint8_t *pucApBuf,
			  uint16_t u2ApBufLen, uint8_t ucValidInterval,
			  uint8_t ucBssIndex);
void aisResetNeighborApList(struct ADAPTER *prAdapter,
	uint8_t ucBssIndex);
void aisCheckNeighborApValidity(IN struct ADAPTER *prAdapter,
	uint8_t ucBssIndex);
#endif
void aisSendNeighborRequest(struct ADAPTER *prAdapter,
	uint8_t ucBssIndex);
/* end Support 11K */

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */

struct AIS_FSM_INFO *aisGetAisFsmInfo(
	IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex);

struct AIS_SPECIFIC_BSS_INFO *aisGetAisSpecBssInfo(
	IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex);

struct BSS_TRANSITION_MGT_PARAM *
	aisGetBTMParam(
	IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex);

struct BSS_INFO *aisGetConnectedBssInfo(
	IN struct ADAPTER *prAdapter);

struct BSS_INFO *aisGetAisBssInfo(
	IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex);

struct STA_RECORD *aisGetStaRecOfAP(
	IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex);

struct BSS_DESC *aisGetTargetBssDesc(
	IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex);

struct STA_RECORD *aisGetTargetStaRec(
	IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex);

struct AIS_FSM_INFO *aisFsmGetInstance(
	IN struct ADAPTER *prAdapter, IN uint8_t ucAisIndex);
struct AIS_FSM_INFO *aisGetDefaultAisInfo(IN struct ADAPTER *prAdapter);
struct AIS_LINK_INFO *aisGetDefaultLink(IN struct ADAPTER *prAdapter);
struct BSS_INFO *aisGetDefaultLinkBssInfo(IN struct ADAPTER *prAdapter);
uint8_t aisGetDefaultLinkBssIndex(IN struct ADAPTER *prAdapter);
struct STA_RECORD *aisGetDefaultStaRecOfAP(IN struct ADAPTER *prAdapter);
struct AIS_LINK_INFO *aisGetLink(IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex);
void aisSetLinkBssInfo(IN struct AIS_FSM_INFO *prAisFsmInfo,
	struct BSS_INFO *prBssInfo, uint8_t ucLinkIdx);
struct BSS_INFO *aisGetLinkBssInfo(IN struct AIS_FSM_INFO *prAisFsmInfo,
	uint8_t ucLinkIdx);
struct BSS_INFO *aisGetMainLinkBssInfo(IN struct AIS_FSM_INFO *prAisFsmInfo);
uint8_t aisGetMainLinkBssIndex(IN struct ADAPTER *prAdapter,
	IN struct AIS_FSM_INFO *prAisFsmInfo);
void aisSetLinkBssDesc(IN struct AIS_FSM_INFO *prAisFsmInfo,
	struct BSS_DESC *prBssDesc, uint8_t ucLinkIdx);
struct BSS_DESC *aisGetLinkBssDesc(IN struct AIS_FSM_INFO *prAisFsmInfo,
	uint8_t ucLinkIdx);
uint8_t aisGetLinkNum(IN struct AIS_FSM_INFO *prAisFsmInfo);
struct BSS_DESC *aisGetMainLinkBssDesc(IN struct AIS_FSM_INFO *prAisFsmInfo);
void aisSetLinkStaRec(IN struct AIS_FSM_INFO *prAisFsmInfo,
	 struct STA_RECORD *prStaRec, uint8_t ucLinkIdx);
struct STA_RECORD *aisGetLinkStaRec(IN struct AIS_FSM_INFO *prAisFsmInfo,
	uint8_t ucLinkIdx);
struct STA_RECORD *aisGetMainLinkStaRec(IN struct AIS_FSM_INFO *prAisFsmInfo);
void aisClearAllLink(IN struct AIS_FSM_INFO *prAisFsmInfo);
void aisDeactivateAllLink(IN struct ADAPTER *prAdapter,
			IN struct AIS_FSM_INFO *prAisFsmInfo);

void aisTargetBssSetConnected(IN struct ADAPTER *prAdapter,
	IN struct AIS_FSM_INFO *ais);

void aisTargetBssSetConnecting(IN struct ADAPTER *prAdapter,
	IN struct AIS_FSM_INFO *ais);

void aisTargetBssResetConnected(IN struct ADAPTER *prAdapter,
	IN struct AIS_FSM_INFO *ais);

void aisTargetBssResetConnecting(IN struct ADAPTER *prAdapter,
	IN struct AIS_FSM_INFO *ais);

#if CFG_SUPPORT_DETECT_SECURITY_MODE_CHANGE
struct TIMER *aisGetSecModeChangeTimer(
	IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex);
#endif

struct TIMER *aisGetScanDoneTimer(
	IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex);

enum ENUM_AIS_STATE aisGetCurrState(
	IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex);

struct CONNECTION_SETTINGS *
	aisGetConnSettings(
	IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex);

struct GL_WPA_INFO *aisGetWpaInfo(
	IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex);

u_int8_t aisGetWapiMode(
	IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex);

enum ENUM_PARAM_AUTH_MODE aisGetAuthMode(
	IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex);

enum ENUM_PARAM_OP_MODE aisGetOPMode(
	IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex);

enum ENUM_WEP_STATUS aisGetEncStatus(
	IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex);

struct IEEE_802_11_MIB *aisGetMib(
	IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex);

struct ROAMING_INFO *aisGetRoamingInfo(
	IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex);

struct PARAM_BSSID_EX *aisGetCurrBssId(
	IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex);

#if CFG_SUPPORT_PASSPOINT
struct HS20_INFO *aisGetHS20Info(
	IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex);
#endif

struct RADIO_MEASUREMENT_REQ_PARAMS *aisGetRmReqParam(
	IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex);

struct RADIO_MEASUREMENT_REPORT_PARAMS *
	aisGetRmReportParam(
	IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex);

struct WMM_INFO *
	aisGetWMMInfo(
	IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex);

#ifdef CFG_SUPPORT_REPLAY_DETECTION
struct GL_DETECT_REPLAY_INFO *
	aisGetDetRplyInfo(
	IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex);
#endif

uint8_t *
	aisGetFsmState(
	IN enum ENUM_AIS_STATE);

struct FT_IES *
	aisGetFtIe(
	IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex);

struct cfg80211_ft_event_params *
	aisGetFtEventParam(
	IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex);

u_int8_t addAxBlacklist(IN struct ADAPTER *prAdapter,
	IN uint8_t aucBSSID[],
	IN uint8_t ucBssIndex,
	IN uint8_t ucType);

u_int8_t queryAxBlacklist(IN struct ADAPTER *prAdapter,
	IN uint8_t aucBSSID[],
	IN uint8_t ucBssIndex,
	IN uint8_t ucType);

u_int8_t clearAxBlacklist(IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex,
	IN uint8_t ucType);

void aisPreSuspendFlow(
	IN struct ADAPTER *prAdapter);

#if (CFG_SUPPORT_ANDROID_DUAL_STA == 1)
void aisMultiStaSetQuoteTime(
	struct ADAPTER *prAdapter,
	uint8_t fgSetQuoteTime);
#endif

#endif /* _AIS_FSM_H */
