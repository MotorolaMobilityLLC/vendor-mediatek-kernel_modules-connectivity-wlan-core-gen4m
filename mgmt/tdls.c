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
 * Id: tdls.c#1
 */

/*! \file tdls.c
 *    \brief This file includes IEEE802.11z TDLS support.
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

#if CFG_SUPPORT_TDLS
#include "tdls.h"
#include "gl_cfg80211.h"
#include "queue.h"

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */

/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */

/*******************************************************************************
 *                            P U B L I C   D A T A
 *******************************************************************************
 */
uint8_t g_arTdlsLink[MAXNUM_TDLS_PEER] = {
		0,
		0,
		0,
		0
	};

/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */
static u_int8_t fgIsPtiTimeoutSkip = FALSE;

/*******************************************************************************
 *                                 M A C R O S
 *******************************************************************************
 */
#define ELEM_ID_LINK_IDENTIFIER_LENGTH 16

#define	TDLS_KEY_TIMEOUT_INTERVAL 43200

#define UNREACH_ABLE 25
#define TDLS_REASON_CODE_UNREACHABLE  25
#define TDLS_REASON_CODE_UNSPECIFIED  26

#define WLAN_REASON_TDLS_TEARDOWN_UNREACHABLE 25
#define WLAN_REASON_TDLS_TEARDOWN_UNSPECIFIED 26

/*******************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */

/*----------------------------------------------------------------------------*/
/*!
 * \brief This routine is called to hadel TDLS link oper from nl80211.
 *
 * \param[in] pvAdapter Pointer to the Adapter structure.
 * \param[in]
 * \param[in]
 * \param[in] buf includes RSN IE + FT IE + Lifetimeout IE
 *
 * \retval WLAN_STATUS_SUCCESS
 * \retval WLAN_STATUS_INVALID_LENGTH
 */
/*----------------------------------------------------------------------------*/
uint32_t TdlsexLinkMgt(struct ADAPTER *prAdapter,
		       void *pvSetBuffer, uint32_t u4SetBufferLen,
		       uint32_t *pu4SetInfoLen)
{
	/* from supplicant -- wpa_supplicant_tdls_peer_addset() */

	struct STA_RECORD *prStaRec;
	struct BSS_INFO *prBssInfo;
	struct TDLS_CMD_LINK_MGT *prCmd;

	prCmd = (struct TDLS_CMD_LINK_MGT *) pvSetBuffer;
	prBssInfo = prAdapter->prAisBssInfo;

	/* printk("\n\n\n  TdlsexLinkMgt\n\n\n"); */

#if 1
	/* AIS only */
	if (prBssInfo->eCurrentOPMode == OP_MODE_INFRASTRUCTURE) {
		prStaRec = prBssInfo->prStaRecOfAP;
		if (prStaRec == NULL)
			return 0;
	} else {
		return -EINVAL;
	}
#endif

	prStaRec = prBssInfo->prStaRecOfAP;

	switch (prCmd->ucActionCode) {

	case TDLS_FRM_ACTION_DISCOVERY_REQ:
		/* printk("\n\n\n  TDLS_FRM_ACTION_DISCOVERY_REQ\n\n\n"); */
		if (prStaRec == NULL)
			return 0;
		if (TdlsDataFrameSend_DISCOVERY_REQ(prAdapter,
					    prStaRec,
					    prCmd->aucPeer,
					    prCmd->ucActionCode,
					    prCmd->ucDialogToken,
					    prCmd->u2StatusCode,
					    (uint8_t *) (prCmd->aucSecBuf),
					    prCmd->u4SecBufLen)
						    != TDLS_STATUS_SUCCESS) {
			return -1;
		}

		break;

	case TDLS_FRM_ACTION_SETUP_REQ:
		/* printk("\n\n\n  TDLS_FRM_ACTION_SETUP_REQ\n\n\n"); */
		if (prStaRec == NULL)
			return 0;
		prStaRec = cnmGetTdlsPeerByAddress(prAdapter,
				prAdapter->prAisBssInfo->ucBssIndex,
				prCmd->aucPeer);
		g_arTdlsLink[prStaRec->ucTdlsIndex] = 0;
		if (TdlsDataFrameSend_SETUP_REQ(prAdapter,
					prStaRec,
					prCmd->aucPeer,
					prCmd->ucActionCode,
					prCmd->ucDialogToken,
					prCmd->u2StatusCode,
					(uint8_t *) (prCmd->aucSecBuf),
					prCmd->u4SecBufLen)
						!= TDLS_STATUS_SUCCESS) {
			return -1;
		}

		break;

	case TDLS_FRM_ACTION_SETUP_RSP:
		/* fix sigma bug 5.2.4.2, 5.2.4.7, we sent Status code decline,
		 * but the sigma recogniezis it as scucess, and it will fail
		 */
		/* if(prCmd->u2StatusCode != 0) */
		if (prBssInfo->fgTdlsIsProhibited)
			return 0;

		/* printk("\n\n\n  TDLS_FRM_ACTION_SETUP_RSP\n\n\n"); */
		if (TdlsDataFrameSend_SETUP_RSP(prAdapter,
					prStaRec,
					prCmd->aucPeer,
					prCmd->ucActionCode,
					prCmd->ucDialogToken,
					prCmd->u2StatusCode,
					(uint8_t *) (prCmd->aucSecBuf),
					prCmd->u4SecBufLen)
						!= TDLS_STATUS_SUCCESS) {
			return -1;
		}

		break;

	case TDLS_FRM_ACTION_DISCOVERY_RSP:
		/* printk("\n\n\n  TDLS_FRM_ACTION_DISCOVERY_RSP\n\n\n"); */
		if (TdlsDataFrameSend_DISCOVERY_RSP(prAdapter,
					    prStaRec,
					    prCmd->aucPeer,
					    prCmd->ucActionCode,
					    prCmd->ucDialogToken,
					    prCmd->u2StatusCode,
					    (uint8_t *) (prCmd->aucSecBuf),
					    prCmd->u4SecBufLen)
						!= TDLS_STATUS_SUCCESS) {
			return -1;
		}

		break;

	case TDLS_FRM_ACTION_CONFIRM:
		/* printk("\n\n\n  TDLS_FRM_ACTION_CONFIRM\n\n\n"); */
		if (TdlsDataFrameSend_CONFIRM(prAdapter,
					      prStaRec,
					      prCmd->aucPeer,
					      prCmd->ucActionCode,
					      prCmd->ucDialogToken,
					      prCmd->u2StatusCode,
					      (uint8_t *) (prCmd->aucSecBuf),
					      prCmd->u4SecBufLen)
						!= TDLS_STATUS_SUCCESS) {
			return -1;
		}
		break;

	case TDLS_FRM_ACTION_TEARDOWN:

		prStaRec = cnmGetTdlsPeerByAddress(prAdapter,
				prAdapter->prAisBssInfo->ucBssIndex,
				prCmd->aucPeer);
		if (prCmd->u2StatusCode == TDLS_REASON_CODE_UNREACHABLE)
			g_arTdlsLink[prStaRec->ucTdlsIndex] = 0;

		if (TdlsDataFrameSend_TearDown(prAdapter,
					       prStaRec,
					       prCmd->aucPeer,
					       prCmd->ucActionCode,
					       prCmd->ucDialogToken,
					       prCmd->u2StatusCode,
					       (uint8_t *) (prCmd->aucSecBuf),
					       prCmd->u4SecBufLen)
						!= TDLS_STATUS_SUCCESS) {
			/* printk("\n teardown frrame  send failure\n"); */
			return -1;
		}
		break;

	default:
		/* printk("\n\n\n  default\n\n\n"); */
		return -EINVAL;
	}

	return 0;

}

/*----------------------------------------------------------------------------*/
/*!
 * \brief This routine is called to hadel TDLS link mgt from nl80211.
 *
 * \param[in] pvAdapter Pointer to the Adapter structure.
 * \param[in]
 * \param[in]
 * \param[in] buf includes RSN IE + FT IE + Lifetimeout IE
 *
 * \retval WLAN_STATUS_SUCCESS
 * \retval WLAN_STATUS_INVALID_LENGTH
 */
/*----------------------------------------------------------------------------*/
uint32_t TdlsexLinkOper(struct ADAPTER *prAdapter,
			void *pvSetBuffer, uint32_t u4SetBufferLen,
			uint32_t *pu4SetInfoLen)
{
	/* printk("TdlsexLinkOper\n"); */

	/* from supplicant -- wpa_supplicant_tdls_peer_addset() */
	uint16_t i;
	struct STA_RECORD *prStaRec;

	struct TDLS_CMD_LINK_OPER *prCmd;

	prCmd = (struct TDLS_CMD_LINK_OPER *) pvSetBuffer;

	switch (prCmd->oper) {

	case TDLS_ENABLE_LINK:

		for (i = 0; i < MAXNUM_TDLS_PEER; i++) {
			if (!g_arTdlsLink[i]) {
				g_arTdlsLink[i] = 1;
				prStaRec =
				cnmGetTdlsPeerByAddress(prAdapter,
					prAdapter->prAisBssInfo->ucBssIndex,
					prCmd->aucPeerMac);
				prStaRec->ucTdlsIndex = i;
				break;
			}
		}

		/* printk("TDLS_ENABLE_LINK %d\n", i); */
		break;
	case TDLS_DISABLE_LINK:

		prStaRec = cnmGetTdlsPeerByAddress(prAdapter,
				prAdapter->prAisBssInfo->ucBssIndex,
				prCmd->aucPeerMac);

		/* printk("TDLS_ENABLE_LINK %d\n", prStaRec->ucTdlsIndex); */
		g_arTdlsLink[prStaRec->ucTdlsIndex] = 0;
		if (IS_DLS_STA(prStaRec))
			cnmStaRecFree(prAdapter, prStaRec);

		break;
	default:
		return 0;
	}

	return 0;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief This routine is called to append general IEs.
 *
 * \param[in] pvAdapter Pointer to the Adapter structure.
 * \param[in]
 *
 * \retval append length
 */
/*----------------------------------------------------------------------------*/
uint32_t TdlsFrameGeneralIeAppend(struct ADAPTER *prAdapter,
				  struct STA_RECORD *prStaRec, uint8_t *pPkt)
{
	struct GLUE_INFO *prGlueInfo;
	struct BSS_INFO *prBssInfo;
	struct PM_PROFILE_SETUP_INFO *prPmProfSetupInfo;
	uint32_t u4NonHTPhyType;
	uint16_t u2SupportedRateSet;
	uint8_t aucAllSupportedRates[RATE_NUM_SW] = { 0 };
	uint8_t ucAllSupportedRatesLen;
	uint8_t ucSupRatesLen;
	uint8_t ucExtSupRatesLen;
	uint32_t u4PktLen, u4IeLen;

	/* init */
	prGlueInfo = (struct GLUE_INFO *) prAdapter->prGlueInfo;
	prBssInfo = prAdapter->prAisBssInfo;	/* AIS only */

	prPmProfSetupInfo = &prBssInfo->rPmProfSetupInfo;
	u4PktLen = 0;

	/* 3. Frame Formation - (5) Supported Rates element */
	/* use all sup rate we can support */
	u4NonHTPhyType = prStaRec->ucNonHTBasicPhyType;
	u2SupportedRateSet =
		rNonHTPhyAttributes[u4NonHTPhyType].u2SupportedRateSet;
	rateGetDataRatesFromRateSet(u2SupportedRateSet, 0,
				    aucAllSupportedRates,
				    &ucAllSupportedRatesLen);

	ucSupRatesLen = ((ucAllSupportedRatesLen >
			  ELEM_MAX_LEN_SUP_RATES) ?
			 ELEM_MAX_LEN_SUP_RATES : ucAllSupportedRatesLen);

	ucExtSupRatesLen = ucAllSupportedRatesLen - ucSupRatesLen;

	if (ucSupRatesLen) {
		SUP_RATES_IE(pPkt)->ucId = ELEM_ID_SUP_RATES;
		SUP_RATES_IE(pPkt)->ucLength = ucSupRatesLen;
		kalMemCopy(SUP_RATES_IE(pPkt)->aucSupportedRates,
			   aucAllSupportedRates, ucSupRatesLen);

		u4IeLen = IE_SIZE(pPkt);
		pPkt += u4IeLen;
		u4PktLen += u4IeLen;
	}

	/* 3. Frame Formation - (7) Extended sup rates element */
	if (ucExtSupRatesLen) {

		EXT_SUP_RATES_IE(pPkt)->ucId = ELEM_ID_EXTENDED_SUP_RATES;
		EXT_SUP_RATES_IE(pPkt)->ucLength = ucExtSupRatesLen;

		kalMemCopy(EXT_SUP_RATES_IE(pPkt)->aucExtSupportedRates,
			   &aucAllSupportedRates[ucSupRatesLen],
			   ucExtSupRatesLen);

		u4IeLen = IE_SIZE(pPkt);
		pPkt += u4IeLen;
		u4PktLen += u4IeLen;
	}

	/* 3. Frame Formation - (8) Supported channels element */
	SUPPORTED_CHANNELS_IE(pPkt)->ucId = ELEM_ID_SUP_CHS;
	SUPPORTED_CHANNELS_IE(pPkt)->ucLength = 2;
	SUPPORTED_CHANNELS_IE(pPkt)->ucChannelNum[0] = 1;
	SUPPORTED_CHANNELS_IE(pPkt)->ucChannelNum[1] = 13;

	if (prAdapter->fgEnable5GBand == TRUE) {
		SUPPORTED_CHANNELS_IE(pPkt)->ucLength = 10;
		SUPPORTED_CHANNELS_IE(pPkt)->ucChannelNum[2] = 36;
		SUPPORTED_CHANNELS_IE(pPkt)->ucChannelNum[3] = 4;
		SUPPORTED_CHANNELS_IE(pPkt)->ucChannelNum[4] = 52;
		SUPPORTED_CHANNELS_IE(pPkt)->ucChannelNum[5] = 4;
		SUPPORTED_CHANNELS_IE(pPkt)->ucChannelNum[6] = 149;
		SUPPORTED_CHANNELS_IE(pPkt)->ucChannelNum[7] = 4;
		SUPPORTED_CHANNELS_IE(pPkt)->ucChannelNum[8] = 165;
		SUPPORTED_CHANNELS_IE(pPkt)->ucChannelNum[9] = 1;
	}

	u4IeLen = IE_SIZE(pPkt);
	pPkt += u4IeLen;
	u4PktLen += u4IeLen;

	return u4PktLen;
}

/*!
 * \brief This routine is called to transmit a TDLS data frame.
 *
 * \param[in] pvAdapter Pointer to the Adapter structure.
 * \param[in]
 * \param[in]
 * \param[in] buf includes RSN IE + FT IE + Lifetimeout IE
 *
 * \retval WLAN_STATUS_SUCCESS
 * \retval WLAN_STATUS_INVALID_LENGTH
 */
/*----------------------------------------------------------------------------*/
uint32_t
TdlsDataFrameSend_TearDown(struct ADAPTER *prAdapter,
			   struct STA_RECORD *prStaRec,
			   uint8_t *pPeerMac,
			   uint8_t ucActionCode,
			   uint8_t ucDialogToken, uint16_t u2StatusCode,
			   uint8_t *pAppendIe, uint32_t AppendIeLen)
{

	struct GLUE_INFO *prGlueInfo;
	struct BSS_INFO *prBssInfo;
	struct PM_PROFILE_SETUP_INFO *prPmProfSetupInfo;
	struct sk_buff *prMsduInfo;
	uint8_t *pPkt;
	uint32_t u4PktLen, u4IeLen;
	uint16_t ReasonCode;

	/* allocate/init packet */
	prGlueInfo = (struct GLUE_INFO *) prAdapter->prGlueInfo;
	prBssInfo = prAdapter->prAisBssInfo;	/* AIS only */

	prPmProfSetupInfo = &prBssInfo->rPmProfSetupInfo;
	u4PktLen = 0;

	prMsduInfo = kalPacketAllocWithHeadroom(prGlueInfo, 1600,
						&pPkt);
	if (prMsduInfo == NULL)
		return TDLS_STATUS_RESOURCES;

	prMsduInfo->dev = prGlueInfo->prDevHandler;
	if (prMsduInfo->dev == NULL) {
		kalPacketFree(prGlueInfo, prMsduInfo);
		return TDLS_STATUS_FAIL;
	}

	/* make up frame content */
	/* 1. 802.3 header */
	kalMemCopy(pPkt, pPeerMac, TDLS_FME_MAC_ADDR_LEN);
	pPkt += TDLS_FME_MAC_ADDR_LEN;
	kalMemCopy(pPkt, prAdapter->rMyMacAddr,
		   TDLS_FME_MAC_ADDR_LEN);
	pPkt += TDLS_FME_MAC_ADDR_LEN;
	*(uint16_t *) pPkt = htons(TDLS_FRM_PROT_TYPE);
	pPkt += 2;
	u4PktLen += TDLS_FME_MAC_ADDR_LEN * 2 + 2;

	/* 2. payload type */
	*pPkt = TDLS_FRM_PAYLOAD_TYPE;
	pPkt++;
	u4PktLen++;

	/* 3. Frame Formation - (1) Category */
	*pPkt = TDLS_FRM_CATEGORY;
	pPkt++;
	u4PktLen++;

	/* 3. Frame Formation - (2) Action */
	*pPkt = ucActionCode;
	pPkt++;
	u4PktLen++;

	/* 3. Frame Formation - status code */

	ReasonCode = u2StatusCode;

	/* printk("\n\n ReasonCode = %u\n\n",ReasonCode ); */

	kalMemCopy(pPkt, &ReasonCode, 2);
	pPkt = pPkt + 2;
	u4PktLen = u4PktLen + 2;

	if (pAppendIe != NULL) {
		if ((ucActionCode != TDLS_FRM_ACTION_TEARDOWN) ||
		    ((ucActionCode == TDLS_FRM_ACTION_TEARDOWN)
		     && (prStaRec != NULL))) {
			kalMemCopy(pPkt, pAppendIe, AppendIeLen);
			LR_TDLS_FME_FIELD_FILL(AppendIeLen);
		}
	}

	/* 7. Append Supported Operating Classes IE */
	if (ucActionCode != TDLS_FRM_ACTION_TEARDOWN) {
		/* Note: if we do not put the IE, Marvell STA will
		 *		decline our TDLS setup request
		 */
		u4IeLen = rlmDomainSupOperatingClassIeFill(pPkt);
		LR_TDLS_FME_FIELD_FILL(u4IeLen);
	}

	/* 3. Frame Formation - (16) Link identifier element */
	TDLS_LINK_IDENTIFIER_IE(pPkt)->ucId =
		ELEM_ID_LINK_IDENTIFIER;
	TDLS_LINK_IDENTIFIER_IE(pPkt)->ucLength = 18;

	kalMemCopy(TDLS_LINK_IDENTIFIER_IE(pPkt)->aBSSID,
		   prBssInfo->aucBSSID, 6);
	kalMemCopy(TDLS_LINK_IDENTIFIER_IE(pPkt)->aInitiator,
		   prAdapter->rMyMacAddr, 6);
	kalMemCopy(TDLS_LINK_IDENTIFIER_IE(pPkt)->aResponder,
		   pPeerMac, 6);

	u4IeLen = IE_SIZE(pPkt);
	pPkt += u4IeLen;
	u4PktLen += u4IeLen;

	/* 5. Update packet length */
	prMsduInfo->len = u4PktLen;

	/* if(u2StatusCode == UNREACH_ABLE ){ */
	/* g_arTdlsLink[prStaRec->ucTdlsIndex] = FALSE; */
	/* } */

	/* printk(" TdlsDataFrameSend_TearDown !!\n"); */

	/* 5. send the data frame */
	wlanHardStartXmit(prMsduInfo, prMsduInfo->dev);

	return TDLS_STATUS_SUCCESS;
}

/*!
 * \brief This routine is called to transmit a TDLS data frame.
 *
 * \param[in] pvAdapter Pointer to the Adapter structure.
 * \param[in]
 * \param[in]
 * \param[in] buf includes RSN IE + FT IE + Lifetimeout IE
 *
 * \retval WLAN_STATUS_SUCCESS
 * \retval WLAN_STATUS_INVALID_LENGTH
 */
/*----------------------------------------------------------------------------*/
uint32_t			/* TDLS_STATUS */
TdlsDataFrameSend_SETUP_REQ(struct ADAPTER *prAdapter,
			    struct STA_RECORD *prStaRec,
			    uint8_t *pPeerMac,
			    uint8_t ucActionCode,
			    uint8_t ucDialogToken, uint16_t u2StatusCode,
			    uint8_t *pAppendIe, uint32_t AppendIeLen)
{

	struct GLUE_INFO *prGlueInfo;
	struct BSS_INFO *prBssInfo;
	struct PM_PROFILE_SETUP_INFO *prPmProfSetupInfo;
	struct sk_buff *prMsduInfo;
	uint8_t *pPkt;
	uint32_t u4PktLen, u4IeLen;
	u_int8_t fg40mAllowed;
	uint16_t u2CapInfo;

	/* allocate/init packet */
	prGlueInfo = (struct GLUE_INFO *) prAdapter->prGlueInfo;
	prBssInfo = prAdapter->prAisBssInfo;	/* AIS only */

	prPmProfSetupInfo = &prBssInfo->rPmProfSetupInfo;
	u4PktLen = 0;

	if (prStaRec == NULL)
		return TDLS_STATUS_FAIL;

	prMsduInfo = kalPacketAllocWithHeadroom(prGlueInfo, 1600,
						&pPkt);
	if (prMsduInfo == NULL)
		return TDLS_STATUS_RESOURCES;

	prMsduInfo->dev = prGlueInfo->prDevHandler;
	if (prMsduInfo->dev == NULL) {
		kalPacketFree(prGlueInfo, prMsduInfo);
		return TDLS_STATUS_FAIL;
	}

	/* make up frame content */
	/* 1. 802.3 header */
	kalMemCopy(pPkt, pPeerMac, TDLS_FME_MAC_ADDR_LEN);
	pPkt += TDLS_FME_MAC_ADDR_LEN;
	kalMemCopy(pPkt, prAdapter->rMyMacAddr,
		   TDLS_FME_MAC_ADDR_LEN);
	pPkt += TDLS_FME_MAC_ADDR_LEN;
	*(uint16_t *) pPkt = htons(TDLS_FRM_PROT_TYPE);
	pPkt += 2;
	u4PktLen += TDLS_FME_MAC_ADDR_LEN * 2 + 2;

	/* 2. payload type */
	*pPkt = TDLS_FRM_PAYLOAD_TYPE;
	pPkt++;
	u4PktLen++;

	/* 3. Frame Formation - (1) Category */
	*pPkt = TDLS_FRM_CATEGORY;
	pPkt++;
	u4PktLen++;

	/* 3. Frame Formation - (2) Action */
	*pPkt = ucActionCode;
	pPkt++;
	u4PktLen++;

	/* 3. Frame Formation - (3) Dialog token */
	*pPkt = ucDialogToken;
	pPkt++;
	u4PktLen++;

	/* 3. Frame Formation - (4) Capability */
	u2CapInfo = assocBuildCapabilityInfo(prAdapter, prStaRec);
	WLAN_SET_FIELD_16(pPkt, u2CapInfo);
	pPkt = pPkt + 2;
	u4PktLen = u4PktLen + 2;

	/* 4. Append general IEs */
	u4IeLen = TdlsFrameGeneralIeAppend(prAdapter, prStaRec,
					   pPkt);
	pPkt += u4IeLen;
	u4PktLen += u4IeLen;

	/* 4. Append extra IEs */
	kalMemCopy(pPkt, pAppendIe, AppendIeLen);
	pPkt += AppendIeLen;
	u4PktLen += AppendIeLen;

	/* 3. Frame Formation - (10) Extended capabilities element */
	EXT_CAP_IE(pPkt)->ucId = ELEM_ID_EXTENDED_CAP;
	EXT_CAP_IE(pPkt)->ucLength = 5;
	/* 0320 !! */
	EXT_CAP_IE(pPkt)->aucCapabilities[0] =
		0x00;	/* bit0 ~ bit7 */
	EXT_CAP_IE(pPkt)->aucCapabilities[1] =
		0x00;	/* bit8 ~ bit15 */
	EXT_CAP_IE(pPkt)->aucCapabilities[2] =
		0x00;	/* bit16 ~ bit23 */
	EXT_CAP_IE(pPkt)->aucCapabilities[3] =
		0x00;	/* bit24 ~ bit31 */
	EXT_CAP_IE(pPkt)->aucCapabilities[4] =
		0xFF;	/* bit32 ~ bit39 */

	EXT_CAP_IE(pPkt)->aucCapabilities[3] |= BIT((28 - 24));
	EXT_CAP_IE(pPkt)->aucCapabilities[3] |= BIT((30 - 24));
	EXT_CAP_IE(pPkt)->aucCapabilities[4] |= BIT((37 - 32));

	/* EXT_CAP_IE(pPkt)->aucCapabilities[3] = 0x00; *//* bit24 ~ bit31 */

	u4IeLen = IE_SIZE(pPkt);
	pPkt += u4IeLen;
	u4PktLen += u4IeLen;

	/* HT capability IE append 0122 */
	HT_CAP_IE(pPkt)->ucId = ELEM_ID_HT_CAP;
	HT_CAP_IE(pPkt)->ucLength = 26;

	/* 3. Frame Formation - (14) HT capabilities element */
	if ((prAdapter->rWifiVar.ucAvailablePhyTypeSet &
	     PHY_TYPE_SET_802_11N)) {
		/* TODO: prAdapter->rWifiVar.rConnSettings.uc5GBandwidthMode */
		if (cnmBss40mBwPermitted(prAdapter, prBssInfo->ucBssIndex))
			fg40mAllowed = TRUE;
		else
			fg40mAllowed = FALSE;

		/* Add HT IE *//* try to reuse p2p path */
		u4IeLen = rlmFillHtCapIEByAdapter(prAdapter, prBssInfo,
						  pPkt);
		pPkt += u4IeLen;
		u4PktLen += u4IeLen;
		/* 0320 !! check newest driver !!! */
	}
	/* check */

	/* 3. Frame Formation -
	 *	(12) Timeout interval element (TPK Key Lifetime)
	 */
	TIMEOUT_INTERVAL_IE(pPkt)->ucId = ELEM_ID_TIMEOUT_INTERVAL;
	TIMEOUT_INTERVAL_IE(pPkt)->ucLength = 5;

	TIMEOUT_INTERVAL_IE(pPkt)->ucType =
		2;	/* IE_TIMEOUT_INTERVAL_TYPE_KEY_LIFETIME; */
	TIMEOUT_INTERVAL_IE(pPkt)->u4Value =
		TDLS_KEY_TIMEOUT_INTERVAL;	/* htonl(prCmd->u4Timeout); */

	u4IeLen = IE_SIZE(pPkt);
	pPkt += u4IeLen;
	u4PktLen += u4IeLen;

	if (ucActionCode != TDLS_FRM_ACTION_TEARDOWN) {
		/*
		 *  bit0 = 1: The Information Request field is used to indicate
		 *  that a transmitting STA is requesting the recipient
		 *  to transmit a 20/40 BSS
		 *  Coexistence Management frame with the transmitting STA as
		 *  the recipient.
		 *  bit1 = 0:The Forty MHz Intolerant field is set to 1
		 *	to prohibit an AP that receives this information
		 *	or reports of this information from operating a
		 *	20/40 MHz BSS.
		 *  bit2 = 0: The 20 MHz BSS Width Request field is set to 1
		 *	to prohibit a receiving AP from operating its BSS
		 *	as a 20/40 MHz BSS.
		 */
		BSS_20_40_COEXIST_IE(pPkt)->ucId =
			ELEM_ID_20_40_BSS_COEXISTENCE;
		BSS_20_40_COEXIST_IE(pPkt)->ucLength = 1;
		BSS_20_40_COEXIST_IE(pPkt)->ucData = 0x01;
		LR_TDLS_FME_FIELD_FILL(3);
	}

	if (pAppendIe != NULL) {
		if ((ucActionCode != TDLS_FRM_ACTION_TEARDOWN) ||
		    ((ucActionCode == TDLS_FRM_ACTION_TEARDOWN)
		     && (prStaRec != NULL))) {
			kalMemCopy(pPkt, pAppendIe, AppendIeLen);
			LR_TDLS_FME_FIELD_FILL(AppendIeLen);
		}
	}

	/* 7. Append Supported Operating Classes IE */
	if (ucActionCode != TDLS_FRM_ACTION_TEARDOWN) {
		/* Note: if we do not put the IE,
		 * Marvell STA will decline our TDLS setup request
		 */
		u4IeLen = rlmDomainSupOperatingClassIeFill(pPkt);
		LR_TDLS_FME_FIELD_FILL(u4IeLen);
	}

	/* 3. Frame Formation - (16) Link identifier element */
	TDLS_LINK_IDENTIFIER_IE(pPkt)->ucId =
		ELEM_ID_LINK_IDENTIFIER;
	TDLS_LINK_IDENTIFIER_IE(pPkt)->ucLength = 18;

	kalMemCopy(TDLS_LINK_IDENTIFIER_IE(pPkt)->aBSSID,
		   prBssInfo->aucBSSID, 6);
	kalMemCopy(TDLS_LINK_IDENTIFIER_IE(pPkt)->aInitiator,
		   prAdapter->rMyMacAddr, 6);
	kalMemCopy(TDLS_LINK_IDENTIFIER_IE(pPkt)->aResponder,
		   pPeerMac, 6);

	u4IeLen = IE_SIZE(pPkt);
	pPkt += u4IeLen;
	u4PktLen += u4IeLen;

	/* 3. Frame Formation - (17) WMM Information element */

	/* HT WMM IE append */
	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucQoS)) {

		/* Add WMM IE *//* try to reuse p2p path */
		u4IeLen = mqmGenerateWmmInfoIEByStaRec(prAdapter, prBssInfo,
						       prStaRec, pPkt);
		pPkt += u4IeLen;
		u4PktLen += u4IeLen;
	}

	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucQoS)) {
		u4IeLen = mqmGenerateWmmParamIEByParam(prAdapter, prBssInfo,
						       pPkt);

		LR_TDLS_FME_FIELD_FILL(u4IeLen);
	}
#if CFG_SUPPORT_802_11AC
	if (prAdapter->rWifiVar.ucAvailablePhyTypeSet &
	    PHY_TYPE_SET_802_11AC) {
		/* Add VHT IE *//* try to reuse p2p path */
		u4IeLen = rlmFillVhtCapIEByAdapter(prAdapter, prBssInfo,
						   pPkt);
		pPkt += u4IeLen;
		u4PktLen += u4IeLen;
	}
#endif

	/* 5. Update packet length */
	prMsduInfo->len = u4PktLen;

	DBGLOG(TDLS, INFO, "\n\n\n  wlanHardStartXmit\n\n\n");

	/* 5. send the data frame */
	wlanHardStartXmit(prMsduInfo, prMsduInfo->dev);
	/* wlanTx ??? */
	return TDLS_STATUS_SUCCESS;
}

uint32_t
TdlsDataFrameSend_SETUP_RSP(struct ADAPTER *prAdapter,
			    struct STA_RECORD *prStaRec,
			    uint8_t *pPeerMac,
			    uint8_t ucActionCode,
			    uint8_t ucDialogToken, uint16_t u2StatusCode,
			    uint8_t *pAppendIe, uint32_t AppendIeLen)
{

	struct GLUE_INFO *prGlueInfo;
	struct BSS_INFO *prBssInfo;
	struct PM_PROFILE_SETUP_INFO *prPmProfSetupInfo;
	struct sk_buff *prMsduInfo;
	uint8_t *pPkt;
	uint32_t u4PktLen, u4IeLen;
	uint16_t u2CapInfo;
	uint16_t StatusCode;
	u_int8_t fg40mAllowed;

	/* allocate/init packet */
	prGlueInfo = (struct GLUE_INFO *) prAdapter->prGlueInfo;
	prBssInfo = prAdapter->prAisBssInfo;	/* AIS only */
	prPmProfSetupInfo = &prBssInfo->rPmProfSetupInfo;
	u4PktLen = 0;

	if (prStaRec == NULL)
		return TDLS_STATUS_FAIL;

	prMsduInfo = kalPacketAllocWithHeadroom(prGlueInfo, 1600,
						&pPkt);
	if (prMsduInfo == NULL)
		return TDLS_STATUS_RESOURCES;

	prMsduInfo->dev = prGlueInfo->prDevHandler;
	if (prMsduInfo->dev == NULL) {
		kalPacketFree(prGlueInfo, prMsduInfo);
		return TDLS_STATUS_FAIL;
	}

	/* make up frame content */
	/* 1. 802.3 header */
	kalMemCopy(pPkt, pPeerMac, TDLS_FME_MAC_ADDR_LEN);
	pPkt += TDLS_FME_MAC_ADDR_LEN;
	kalMemCopy(pPkt, prAdapter->rMyMacAddr,
		   TDLS_FME_MAC_ADDR_LEN);
	pPkt += TDLS_FME_MAC_ADDR_LEN;
	*(uint16_t *) pPkt = htons(TDLS_FRM_PROT_TYPE);
	pPkt += 2;
	u4PktLen += TDLS_FME_MAC_ADDR_LEN * 2 + 2;

	/* 2. payload type */
	*pPkt = TDLS_FRM_PAYLOAD_TYPE;
	pPkt++;
	u4PktLen++;

	/* 3. Frame Formation - (1) Category */
	*pPkt = TDLS_FRM_CATEGORY;
	pPkt++;
	u4PktLen++;

	/* 3. Frame Formation - (2) Action */
	*pPkt = ucActionCode;
	pPkt++;
	u4PktLen++;

	/* 3. Frame Formation - status code */
	StatusCode = u2StatusCode;
	kalMemCopy(pPkt, &StatusCode, 2);
	pPkt = pPkt + 2;
	u4PktLen = u4PktLen + 2;

	/* 3. Frame Formation - (3) Dialog token */
	*pPkt = ucDialogToken;
	pPkt++;
	u4PktLen++;

	/* 3. Frame Formation - (4) Capability */
	u2CapInfo = assocBuildCapabilityInfo(prAdapter, prStaRec);
	WLAN_SET_FIELD_16(pPkt, u2CapInfo);
	pPkt = pPkt + 2;
	u4PktLen = u4PktLen + 2;

	/* 4. Append general IEs */
	u4IeLen = TdlsFrameGeneralIeAppend(prAdapter, prStaRec,
					   pPkt);
	pPkt += u4IeLen;
	u4PktLen += u4IeLen;

	/* 4. Append extra IEs */
	kalMemCopy(pPkt, pAppendIe, AppendIeLen);
	pPkt += AppendIeLen;
	u4PktLen += AppendIeLen;

	/* 3. Frame Formation - (10) Extended capabilities element */
	EXT_CAP_IE(pPkt)->ucId = ELEM_ID_EXTENDED_CAP;
	EXT_CAP_IE(pPkt)->ucLength = 5;

	EXT_CAP_IE(pPkt)->aucCapabilities[0] =
		0x00;	/* bit0 ~ bit7 */
	EXT_CAP_IE(pPkt)->aucCapabilities[1] =
		0x00;	/* bit8 ~ bit15 */
	EXT_CAP_IE(pPkt)->aucCapabilities[2] =
		0x00;	/* bit16 ~ bit23 */
	EXT_CAP_IE(pPkt)->aucCapabilities[3] =
		0x00;	/* bit24 ~ bit31 */
	EXT_CAP_IE(pPkt)->aucCapabilities[4] =
		0xFF;	/* bit32 ~ bit39 */

	EXT_CAP_IE(pPkt)->aucCapabilities[3] |= BIT((28 - 24));

	EXT_CAP_IE(pPkt)->aucCapabilities[3] |= BIT((30 - 24));
	EXT_CAP_IE(pPkt)->aucCapabilities[4] |= BIT((37 - 32));
	/* EXT_CAP_IE(pPkt)->aucCapabilities[3] = 0x00; *//* bit24 ~ bit31 */
	u4IeLen = IE_SIZE(pPkt);
	pPkt += u4IeLen;
	u4PktLen += u4IeLen;

	/* HT capability IE append */
	HT_CAP_IE(pPkt)->ucId = ELEM_ID_HT_CAP;
	HT_CAP_IE(pPkt)->ucLength = 26;

	/* 3. Frame Formation - (14) HT capabilities element */
	if ((prAdapter->rWifiVar.ucAvailablePhyTypeSet &
	     PHY_TYPE_SET_802_11N)) {
		/* TODO: prAdapter->rWifiVar.rConnSettings.uc5GBandwidthMode */
		if (cnmBss40mBwPermitted(prAdapter, prBssInfo->ucBssIndex))
			fg40mAllowed = TRUE;
		else
			fg40mAllowed = FALSE;

		/* Add HT IE *//* try to reuse p2p path */
		u4IeLen = rlmFillHtCapIEByAdapter(prAdapter, prBssInfo,
						  pPkt);
		pPkt += u4IeLen;
		u4PktLen += u4IeLen;
	}

	/* 3. Frame Formation - (12) Timeout interval element
	 * (TPK Key Lifetime)
	 */
	TIMEOUT_INTERVAL_IE(pPkt)->ucId = ELEM_ID_TIMEOUT_INTERVAL;
	TIMEOUT_INTERVAL_IE(pPkt)->ucLength = 5;
	TIMEOUT_INTERVAL_IE(pPkt)->ucType =
		2;	/* IE_TIMEOUT_INTERVAL_TYPE_KEY_LIFETIME; */
	TIMEOUT_INTERVAL_IE(pPkt)->u4Value =
		TDLS_KEY_TIMEOUT_INTERVAL;	/* htonl(prCmd->u4Timeout); */

	u4IeLen = IE_SIZE(pPkt);
	pPkt += u4IeLen;
	u4PktLen += u4IeLen;

	if (ucActionCode != TDLS_FRM_ACTION_TEARDOWN) {
		/*
		 *  bit0 = 1: The Information Request field is used to indicate
		 *  that a transmitting STA is requesting the recipient
		 *  to transmit a 20/40 BSS
		 *  Coexistence Management frame with the transmitting STA as
		 *  the recipient.
		 *  bit1 = 0: The Forty MHz Intolerant field is set to 1
		 *	to prohibit an AP that receives this information
		 *	or reports of this information from operating a
		 *	20/40 MHz BSS.
		 *  bit2 = 0: The 20 MHz BSS Width Request field is set to 1
		 *	to prohibit a receiving AP from operating its BSS
		 *	as a 20/40 MHz BSS.
		 */
		BSS_20_40_COEXIST_IE(pPkt)->ucId =
			ELEM_ID_20_40_BSS_COEXISTENCE;
		BSS_20_40_COEXIST_IE(pPkt)->ucLength = 1;
		BSS_20_40_COEXIST_IE(pPkt)->ucData = 0x01;
		LR_TDLS_FME_FIELD_FILL(3);
	}

	if (pAppendIe != NULL) {
		if ((ucActionCode != TDLS_FRM_ACTION_TEARDOWN) ||
		    ((ucActionCode == TDLS_FRM_ACTION_TEARDOWN)
		     && (prStaRec != NULL))) {
			kalMemCopy(pPkt, pAppendIe, AppendIeLen);
			LR_TDLS_FME_FIELD_FILL(AppendIeLen);
		}
	}

	/* 7. Append Supported Operating Classes IE */
	if (ucActionCode != TDLS_FRM_ACTION_TEARDOWN) {
		/* Note: if we do not put the IE, Marvell STA will decline
		 * our TDLS setup request
		 */
		u4IeLen = rlmDomainSupOperatingClassIeFill(pPkt);
		LR_TDLS_FME_FIELD_FILL(u4IeLen);
	}

	/* 3. Frame Formation - (16) Link identifier element */
	TDLS_LINK_IDENTIFIER_IE(pPkt)->ucId =
		ELEM_ID_LINK_IDENTIFIER;
	TDLS_LINK_IDENTIFIER_IE(pPkt)->ucLength = 18;

	kalMemCopy(TDLS_LINK_IDENTIFIER_IE(pPkt)->aBSSID,
		   prBssInfo->aucBSSID, 6);
	kalMemCopy(TDLS_LINK_IDENTIFIER_IE(pPkt)->aInitiator,
		   pPeerMac, 6);	/* prAdapter->rMyMacAddr */
	kalMemCopy(TDLS_LINK_IDENTIFIER_IE(pPkt)->aResponder,
		   prAdapter->rMyMacAddr, 6);	/* pPeerMac */

	u4IeLen = IE_SIZE(pPkt);
	pPkt += u4IeLen;
	u4PktLen += u4IeLen;

	/* HT WMM IE append */
	/* HT WMM IE append */
	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucQoS)) {

		/* Add WMM IE *//* try to reuse p2p path */
		u4IeLen = mqmGenerateWmmInfoIEByStaRec(prAdapter, prBssInfo,
						       prStaRec, pPkt);
		pPkt += u4IeLen;
		u4PktLen += u4IeLen;
	}

	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucQoS)) {
		u4IeLen = mqmGenerateWmmParamIEByParam(prAdapter, prBssInfo,
						       pPkt);

		LR_TDLS_FME_FIELD_FILL(u4IeLen);
	}
#if CFG_SUPPORT_802_11AC
	if (prAdapter->rWifiVar.ucAvailablePhyTypeSet &
	    PHY_TYPE_SET_802_11AC) {
		/* Add VHT IE *//* try to reuse p2p path */
		u4IeLen = rlmFillVhtCapIEByAdapter(prAdapter, prBssInfo,
						   pPkt);
		pPkt += u4IeLen;
		u4PktLen += u4IeLen;
	}
#endif

	/* 5. Update packet length */
	prMsduInfo->len = u4PktLen;

	/* 5. send the data frame */
	wlanHardStartXmit(prMsduInfo, prMsduInfo->dev);

	return TDLS_STATUS_SUCCESS;
}

uint32_t
TdlsDataFrameSend_CONFIRM(struct ADAPTER *prAdapter,
			  struct STA_RECORD *prStaRec,
			  uint8_t *pPeerMac,
			  uint8_t ucActionCode,
			  uint8_t ucDialogToken, uint16_t u2StatusCode,
			  uint8_t *pAppendIe, uint32_t AppendIeLen)
{

	struct GLUE_INFO *prGlueInfo;
	struct BSS_INFO *prBssInfo;
	struct PM_PROFILE_SETUP_INFO *prPmProfSetupInfo;
	struct sk_buff *prMsduInfo;
	uint8_t *pPkt;
	uint32_t u4PktLen, u4IeLen;
	uint16_t StatusCode;

	/* allocate/init packet */
	prGlueInfo = (struct GLUE_INFO *) prAdapter->prGlueInfo;
	prBssInfo = prAdapter->prAisBssInfo;	/* AIS only */

	prPmProfSetupInfo = &prBssInfo->rPmProfSetupInfo;
	u4PktLen = 0;

	prMsduInfo = kalPacketAllocWithHeadroom(prGlueInfo, 1600,
						&pPkt);
	if (prMsduInfo == NULL)
		return TDLS_STATUS_RESOURCES;

	prMsduInfo->dev = prGlueInfo->prDevHandler;
	if (prMsduInfo->dev == NULL) {
		kalPacketFree(prGlueInfo, prMsduInfo);
		return TDLS_STATUS_FAIL;
	}

	/* make up frame content */
	/* 1. 802.3 header */
	kalMemCopy(pPkt, pPeerMac, TDLS_FME_MAC_ADDR_LEN);
	pPkt += TDLS_FME_MAC_ADDR_LEN;
	kalMemCopy(pPkt, prAdapter->rMyMacAddr,
		   TDLS_FME_MAC_ADDR_LEN);
	pPkt += TDLS_FME_MAC_ADDR_LEN;
	*(uint16_t *) pPkt = htons(TDLS_FRM_PROT_TYPE);
	pPkt += 2;
	u4PktLen += TDLS_FME_MAC_ADDR_LEN * 2 + 2;

	/* 2. payload type */
	*pPkt = TDLS_FRM_PAYLOAD_TYPE;
	pPkt++;
	u4PktLen++;

	/* 3. Frame Formation - (1) Category */
	*pPkt = TDLS_FRM_CATEGORY;
	pPkt++;
	u4PktLen++;

	/* 3. Frame Formation - (2) Action */
	*pPkt = ucActionCode;
	pPkt++;
	u4PktLen++;

	/* 3. Frame Formation - status code */

	StatusCode =
		u2StatusCode;	/* 0;  //u2StatusCode;  //ahiu 0224 */
	kalMemCopy(pPkt, &StatusCode, 2);
	pPkt = pPkt + 2;
	u4PktLen = u4PktLen + 2;

	/* 3. Frame Formation - (3) Dialog token */
	*pPkt = ucDialogToken;
	pPkt++;
	u4PktLen++;

	/* 4. Append extra IEs */
	kalMemCopy(pPkt, pAppendIe, AppendIeLen);
	pPkt += AppendIeLen;
	u4PktLen += AppendIeLen;

	/* 3. Frame Formation - (12) Timeout interval element
	 * (TPK Key Lifetime)
	 */
	TIMEOUT_INTERVAL_IE(pPkt)->ucId = ELEM_ID_TIMEOUT_INTERVAL;
	TIMEOUT_INTERVAL_IE(pPkt)->ucLength = 5;

	TIMEOUT_INTERVAL_IE(pPkt)->ucType = 2;
	TIMEOUT_INTERVAL_IE(pPkt)->u4Value =
		TDLS_KEY_TIMEOUT_INTERVAL;	/* htonl(prCmd->u4Timeout); */

	u4IeLen = IE_SIZE(pPkt);
	pPkt += u4IeLen;
	u4PktLen += u4IeLen;

	if (ucActionCode != TDLS_FRM_ACTION_TEARDOWN) {
		/*
		 *  bit0 = 1: The Information Request field is used to
		 *  indicate that a transmitting STA is requesting the recipient
		 *  to transmit a 20/40 BSS
		 *  Coexistence Management frame with the transmitting STA as
		 *  the recipient.
		 *  bit1 = 0: The Forty MHz Intolerant field is set to 1
		 *	to prohibit an AP that receives this information
		 *	or reports of this information from operating
		 *	a 20/40 MHz BSS.
		 *  bit2 = 0: The 20 MHz BSS Width Request field is set to 1
		 *	to prohibit a receiving AP from operating its BSS
		 *	as a 20/40 MHz BSS.
		 */
		BSS_20_40_COEXIST_IE(pPkt)->ucId =
			ELEM_ID_20_40_BSS_COEXISTENCE;
		BSS_20_40_COEXIST_IE(pPkt)->ucLength = 1;
		BSS_20_40_COEXIST_IE(pPkt)->ucData = 0x01;
		LR_TDLS_FME_FIELD_FILL(3);
	}

	if (pAppendIe != NULL) {
		if ((ucActionCode != TDLS_FRM_ACTION_TEARDOWN) ||
		    ((ucActionCode == TDLS_FRM_ACTION_TEARDOWN)
		     && (prStaRec != NULL))) {
			kalMemCopy(pPkt, pAppendIe, AppendIeLen);
			LR_TDLS_FME_FIELD_FILL(AppendIeLen);
		}
	}

	/* 7. Append Supported Operating Classes IE */
	if (ucActionCode != TDLS_FRM_ACTION_TEARDOWN) {
		/* Note: if we do not put the IE, Marvell STA will decline
		 * our TDLS setup request
		 */
		u4IeLen = rlmDomainSupOperatingClassIeFill(pPkt);
		LR_TDLS_FME_FIELD_FILL(u4IeLen);
	}

	/* 3. Frame Formation - (16) Link identifier element */
	TDLS_LINK_IDENTIFIER_IE(pPkt)->ucId =
		ELEM_ID_LINK_IDENTIFIER;
	TDLS_LINK_IDENTIFIER_IE(pPkt)->ucLength = 18;

	kalMemCopy(TDLS_LINK_IDENTIFIER_IE(pPkt)->aBSSID,
		   prBssInfo->aucBSSID, 6);
	kalMemCopy(TDLS_LINK_IDENTIFIER_IE(pPkt)->aInitiator,
		   prAdapter->rMyMacAddr, 6);
	kalMemCopy(TDLS_LINK_IDENTIFIER_IE(pPkt)->aResponder,
		   pPeerMac, 6);

	u4IeLen = IE_SIZE(pPkt);
	pPkt += u4IeLen;
	u4PktLen += u4IeLen;

	/* HT WMM IE append */
	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucQoS)) {

		/* Add WMM IE *//* try to reuse p2p path */
		u4IeLen = mqmGenerateWmmInfoIEByStaRec(prAdapter, prBssInfo,
						       prStaRec, pPkt);
		pPkt += u4IeLen;
		u4PktLen += u4IeLen;
	}

	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucQoS)) {
		u4IeLen = mqmGenerateWmmParamIEByParam(prAdapter, prBssInfo,
						       pPkt);

		LR_TDLS_FME_FIELD_FILL(u4IeLen);
	}

	/* 5. Update packet length */
	prMsduInfo->len = u4PktLen;

	/* 5. send the data frame */
	wlanHardStartXmit(prMsduInfo, prMsduInfo->dev);

	return TDLS_STATUS_SUCCESS;
}

/*
 * \brief This routine is called to transmit a TDLS data frame.
 *
 * \param[in] pvAdapter Pointer to the Adapter structure.
 * \param[in]
 * \param[in]
 * \param[in] buf includes RSN IE + FT IE + Lifetimeout IE
 *
 * \retval WLAN_STATUS_SUCCESS
 * \retval WLAN_STATUS_INVALID_LENGTH
 */
/*----------------------------------------------------------------------------*/
uint32_t			/* TDLS_STATUS */
TdlsDataFrameSend_DISCOVERY_REQ(struct ADAPTER *prAdapter,
				struct STA_RECORD *prStaRec,
				uint8_t *pPeerMac,
				uint8_t ucActionCode,
				uint8_t ucDialogToken, uint16_t u2StatusCode,
				uint8_t *pAppendIe, uint32_t AppendIeLen)
{
	struct GLUE_INFO *prGlueInfo;
	struct BSS_INFO *prBssInfo;
	struct PM_PROFILE_SETUP_INFO *prPmProfSetupInfo;
	struct sk_buff *prMsduInfo;
	struct MSDU_INFO *prMsduInfoMgmt;
	uint8_t *pPkt, *pucInitiator, *pucResponder;
	uint32_t u4PktLen, u4IeLen;
	uint16_t u2CapInfo;

	prGlueInfo = (struct GLUE_INFO *) prAdapter->prGlueInfo;

	if (prStaRec != NULL)
		prBssInfo = prAdapter->prAisBssInfo;	/* AIS only */
	else
		return TDLS_STATUS_FAIL;

	/* allocate/init packet */
	prPmProfSetupInfo = &prBssInfo->rPmProfSetupInfo;
	u4PktLen = 0;
	prMsduInfo = NULL;
	prMsduInfoMgmt = NULL;

	/* make up frame content */
	if (ucActionCode != TDLS_FRM_ACTION_DISCOVERY_RSP) {
		/* TODO: reduce 1600 to correct size */
		prMsduInfo = kalPacketAllocWithHeadroom(prGlueInfo, 1600,
							&pPkt);
		if (prMsduInfo == NULL)
			return TDLS_STATUS_RESOURCES;

		prMsduInfo->dev = prGlueInfo->prDevHandler;
		if (prMsduInfo->dev == NULL) {

			kalPacketFree(prGlueInfo, prMsduInfo);
			return TDLS_STATUS_FAIL;
		}

		/* 1. 802.3 header */
		kalMemCopy(pPkt, pPeerMac, TDLS_FME_MAC_ADDR_LEN);
		LR_TDLS_FME_FIELD_FILL(TDLS_FME_MAC_ADDR_LEN);
		kalMemCopy(pPkt, prBssInfo->aucOwnMacAddr,
			   TDLS_FME_MAC_ADDR_LEN);
		LR_TDLS_FME_FIELD_FILL(TDLS_FME_MAC_ADDR_LEN);
		*(uint16_t *) pPkt = htons(TDLS_FRM_PROT_TYPE);
		LR_TDLS_FME_FIELD_FILL(2);

		/* 2. payload type */
		*pPkt = TDLS_FRM_PAYLOAD_TYPE;
		LR_TDLS_FME_FIELD_FILL(1);

		/* 3. Frame Formation - (1) Category */
		*pPkt = TDLS_FRM_CATEGORY;
		LR_TDLS_FME_FIELD_FILL(1);
	} else {
		struct WLAN_MAC_HEADER *prHdr;

		prMsduInfoMgmt = (struct MSDU_INFO *)
					cnmMgtPktAlloc(prAdapter,
						PUBLIC_ACTION_MAX_LEN);
		if (prMsduInfoMgmt == NULL)
			return TDLS_STATUS_RESOURCES;

		pPkt = (uint8_t *) prMsduInfoMgmt->prPacket;
		prHdr = (struct WLAN_MAC_HEADER *) pPkt;

		/* 1. 802.11 header */
		prHdr->u2FrameCtrl = MAC_FRAME_ACTION;
		prHdr->u2DurationID = 0;
		kalMemCopy(prHdr->aucAddr1, pPeerMac,
			   TDLS_FME_MAC_ADDR_LEN);
		kalMemCopy(prHdr->aucAddr2, prBssInfo->aucOwnMacAddr,
			   TDLS_FME_MAC_ADDR_LEN);
		kalMemCopy(prHdr->aucAddr3, prBssInfo->aucBSSID,
			   TDLS_FME_MAC_ADDR_LEN);
		prHdr->u2SeqCtrl = 0;
		LR_TDLS_FME_FIELD_FILL(sizeof(struct WLAN_MAC_HEADER));

		/* Frame Formation - (1) Category */
		*pPkt = CATEGORY_PUBLIC_ACTION;
		LR_TDLS_FME_FIELD_FILL(1);
	}

	/* 3. Frame Formation - (2) Action */
	*pPkt = ucActionCode;
	LR_TDLS_FME_FIELD_FILL(1);

	/* 3. Frame Formation - Status Code */
	switch (ucActionCode) {
	case TDLS_FRM_ACTION_SETUP_RSP:
	case TDLS_FRM_ACTION_CONFIRM:
	case TDLS_FRM_ACTION_TEARDOWN:
		WLAN_SET_FIELD_16(pPkt, u2StatusCode);
		LR_TDLS_FME_FIELD_FILL(2);
		break;
	}

	/* 3. Frame Formation - (3) Dialog token */
	if (ucActionCode != TDLS_FRM_ACTION_TEARDOWN) {
		*pPkt = ucDialogToken;
		LR_TDLS_FME_FIELD_FILL(1);
	}

	/* Fill elements */
	if (ucActionCode != TDLS_FRM_ACTION_TEARDOWN) {
		/*
		 *  Capability
		 *  Support Rates
		 *  Extended Support Rates
		 *  Supported Channels
		 *  HT Capabilities
		 *  WMM Information Element
		 *  Extended Capabilities
		 *  Link Identifier
		 *  RSNIE
		 *  FTIE
		 *  Timeout Interval
		 */
		if (ucActionCode != TDLS_FRM_ACTION_CONFIRM) {
			/* 3. Frame Formation - (4) Capability: 0x31 0x04,
			 * privacy bit will be set
			 */
			u2CapInfo = assocBuildCapabilityInfo(prAdapter,
								prStaRec);
			WLAN_SET_FIELD_16(pPkt, u2CapInfo);
			LR_TDLS_FME_FIELD_FILL(2);

			/* 4. Append general IEs */
			/*
			 * TODO check HT:
			 *  prAdapter->rWifiVar.rConnSettings.uc2G4BandwidthMode
			 *	must be CONFIG_BW_20_40M.
			 * TODO check HT: HT_CAP_INFO_40M_INTOLERANT
			 *	must be clear if Tdls 20/40 is enabled.
			 */
			u4IeLen = TdlsFrameGeneralIeAppend(prAdapter, prStaRec,
							   pPkt);
			LR_TDLS_FME_FIELD_FILL(u4IeLen);

			/* 5. Frame Formation - Extended capabilities element */
			EXT_CAP_IE(pPkt)->ucId = ELEM_ID_EXTENDED_CAP;
			EXT_CAP_IE(pPkt)->ucLength = 5;

			EXT_CAP_IE(pPkt)->aucCapabilities[0] =
				0x00;	/* bit0 ~ bit7 */
			EXT_CAP_IE(pPkt)->aucCapabilities[1] =
				0x00;	/* bit8 ~ bit15 */
			EXT_CAP_IE(pPkt)->aucCapabilities[2] =
				0x00;	/* bit16 ~ bit23 */
			EXT_CAP_IE(pPkt)->aucCapabilities[3] =
				0x00;	/* bit24 ~ bit31 */
			EXT_CAP_IE(pPkt)->aucCapabilities[4] =
				0x00;	/* bit32 ~ bit39 */

			/* if (prCmd->ucExCap & TDLS_EX_CAP_PEER_UAPSD) */
			EXT_CAP_IE(pPkt)->aucCapabilities[3] |= BIT((28 - 24));
			/* if (prCmd->ucExCap & TDLS_EX_CAP_CHAN_SWITCH) */
			EXT_CAP_IE(pPkt)->aucCapabilities[3] |= BIT((30 - 24));
			/* if (prCmd->ucExCap & TDLS_EX_CAP_TDLS) */
			EXT_CAP_IE(pPkt)->aucCapabilities[4] |= BIT((37 - 32));

			u4IeLen = IE_SIZE(pPkt);
			LR_TDLS_FME_FIELD_FILL(u4IeLen);
		} else {
			/* 5. Frame Formation - WMM Information element */
			if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucQoS)) {

				/* Add WMM IE *//* try to reuse p2p path */
				u4IeLen = mqmGenerateWmmInfoIEByStaRec(
						prAdapter, prBssInfo,
						prStaRec, pPkt);
				pPkt += u4IeLen;
				u4PktLen += u4IeLen;
			}

		}
	}

	/* 6. Frame Formation - 20/40 BSS Coexistence */
	/*
	 * Follow WiFi test plan,
	 * add 20/40 element to request/response/confirm.
	 */

	/* 6. Frame Formation - HT Operation element */
	/* u4IeLen = rlmFillHtOpIeBody(prBssInfo, pPkt); */
	/* LR_TDLS_FME_FIELD_FILL(u4IeLen); */

	/* 7. Frame Formation - Link identifier element */
	/* Note: Link ID sequence must be correct;
	 * Or the calculated MIC will be error
	 */
	TDLS_LINK_IDENTIFIER_IE(pPkt)->ucId =
		ELEM_ID_LINK_IDENTIFIER;
	TDLS_LINK_IDENTIFIER_IE(pPkt)->ucLength = 18;

	kalMemCopy(TDLS_LINK_IDENTIFIER_IE(pPkt)->aBSSID,
		   prBssInfo->aucBSSID, 6);

	switch (ucActionCode) {
	case TDLS_FRM_ACTION_SETUP_REQ:
	case TDLS_FRM_ACTION_CONFIRM:
	case TDLS_FRM_ACTION_DISCOVERY_RSP:
	default:
		/* we are initiator */
		pucInitiator = prBssInfo->aucOwnMacAddr;
		pucResponder = pPeerMac;

		if (prStaRec != NULL)
			prStaRec->flgTdlsIsInitiator = TRUE;
		break;

	case TDLS_FRM_ACTION_SETUP_RSP:
		/* peer is initiator */
		pucInitiator = pPeerMac;
		pucResponder = prBssInfo->aucOwnMacAddr;

		if (prStaRec != NULL)
			prStaRec->flgTdlsIsInitiator = FALSE;
		break;

	case TDLS_FRM_ACTION_TEARDOWN:
		if (prStaRec != NULL) {
			if (prStaRec->flgTdlsIsInitiator == TRUE) {
				/* we are initiator */
				pucInitiator = prBssInfo->aucOwnMacAddr;
				pucResponder = pPeerMac;
			} else {
				/* peer is initiator */
				pucInitiator = pPeerMac;
				pucResponder = prBssInfo->aucOwnMacAddr;
			}
		}
		break;
	}

	kalMemCopy(TDLS_LINK_IDENTIFIER_IE(pPkt)->aInitiator,
		   pucInitiator, 6);
	kalMemCopy(TDLS_LINK_IDENTIFIER_IE(pPkt)->aResponder,
		   pucResponder, 6);

	u4IeLen = IE_SIZE(pPkt);
	LR_TDLS_FME_FIELD_FILL(u4IeLen);

	/* 8. Append security IEs */
	if ((ucActionCode != TDLS_FRM_ACTION_TEARDOWN)
	    && (pAppendIe != NULL)) {
		kalMemCopy(pPkt, pAppendIe, AppendIeLen);
		LR_TDLS_FME_FIELD_FILL(AppendIeLen);
	}

	/* 10. send the data or management frame */
	if (ucActionCode != TDLS_FRM_ACTION_DISCOVERY_RSP) {
		/* 9. Update packet length */
		prMsduInfo->len = u4PktLen;

		wlanHardStartXmit(prMsduInfo, prMsduInfo->dev);
	} else {
		prMsduInfoMgmt->ucPacketType = TX_PACKET_TYPE_MGMT;
		prMsduInfoMgmt->ucStaRecIndex =
			prBssInfo->prStaRecOfAP->ucIndex;
		prMsduInfoMgmt->ucBssIndex = prBssInfo->ucBssIndex;
		prMsduInfoMgmt->ucMacHeaderLength =
			WLAN_MAC_MGMT_HEADER_LEN;
		prMsduInfoMgmt->fgIs802_1x = FALSE;
		prMsduInfoMgmt->fgIs802_11 = TRUE;
		prMsduInfoMgmt->u2FrameLength = u4PktLen;
		prMsduInfoMgmt->ucTxSeqNum = nicIncreaseTxSeqNum(prAdapter);
		prMsduInfoMgmt->pfTxDoneHandler = NULL;

		/* Send them to HW queue */
		nicTxEnqueueMsdu(prAdapter, prMsduInfoMgmt);
	}

	return TDLS_STATUS_SUCCESS;
}

uint32_t
TdlsDataFrameSend_DISCOVERY_RSP(struct ADAPTER *prAdapter,
				struct STA_RECORD *prStaRec,
				uint8_t *pPeerMac,
				uint8_t ucActionCode,
				uint8_t ucDialogToken, uint16_t u2StatusCode,
				uint8_t *pAppendIe, uint32_t AppendIeLen)
{
	struct GLUE_INFO *prGlueInfo;
	struct BSS_INFO *prBssInfo;
	struct PM_PROFILE_SETUP_INFO *prPmProfSetupInfo;
	struct sk_buff *prMsduInfo;
	struct MSDU_INFO *prMsduInfoMgmt;
	uint8_t *pPkt, *pucInitiator, *pucResponder;
	uint32_t u4PktLen, u4IeLen;
	uint16_t u2CapInfo;

	prGlueInfo = (struct GLUE_INFO *) prAdapter->prGlueInfo;

	/* sanity check */
	if (prStaRec != NULL)
		prBssInfo = prAdapter->prAisBssInfo;	/* AIS only */
	else
		return TDLS_STATUS_FAIL;

	/* allocate/init packet */
	prPmProfSetupInfo = &prBssInfo->rPmProfSetupInfo;
	u4PktLen = 0;
	prMsduInfo = NULL;
	prMsduInfoMgmt = NULL;

	/* make up frame content */
	if (ucActionCode != TDLS_FRM_ACTION_DISCOVERY_RSP) {
		/* TODO: reduce 1600 to correct size */
		prMsduInfo = kalPacketAllocWithHeadroom(prGlueInfo, 1600,
							&pPkt);
		if (prMsduInfo == NULL)
			return TDLS_STATUS_RESOURCES;

		prMsduInfo->dev = prGlueInfo->prDevHandler;
		if (prMsduInfo->dev == NULL) {

			kalPacketFree(prGlueInfo, prMsduInfo);
			return TDLS_STATUS_FAIL;
		}

		/* 1. 802.3 header */
		kalMemCopy(pPkt, pPeerMac, TDLS_FME_MAC_ADDR_LEN);
		LR_TDLS_FME_FIELD_FILL(TDLS_FME_MAC_ADDR_LEN);
		kalMemCopy(pPkt, prBssInfo->aucOwnMacAddr,
			   TDLS_FME_MAC_ADDR_LEN);
		LR_TDLS_FME_FIELD_FILL(TDLS_FME_MAC_ADDR_LEN);
		*(uint16_t *) pPkt = htons(TDLS_FRM_PROT_TYPE);
		LR_TDLS_FME_FIELD_FILL(2);

		/* 2. payload type */
		*pPkt = TDLS_FRM_PAYLOAD_TYPE;
		LR_TDLS_FME_FIELD_FILL(1);

		/* 3. Frame Formation - (1) Category */
		*pPkt = TDLS_FRM_CATEGORY;
		LR_TDLS_FME_FIELD_FILL(1);
	} else {
		struct WLAN_MAC_HEADER *prHdr;

		prMsduInfoMgmt = (struct MSDU_INFO *)
					cnmMgtPktAlloc(prAdapter,
						PUBLIC_ACTION_MAX_LEN);
		if (prMsduInfoMgmt == NULL)
			return TDLS_STATUS_RESOURCES;

		pPkt = (uint8_t *) prMsduInfoMgmt->prPacket;
		prHdr = (struct WLAN_MAC_HEADER *) pPkt;

		/* 1. 802.11 header */
		prHdr->u2FrameCtrl = MAC_FRAME_ACTION;
		prHdr->u2DurationID = 0;
		kalMemCopy(prHdr->aucAddr1, pPeerMac,
			   TDLS_FME_MAC_ADDR_LEN);
		kalMemCopy(prHdr->aucAddr2, prBssInfo->aucOwnMacAddr,
			   TDLS_FME_MAC_ADDR_LEN);
		kalMemCopy(prHdr->aucAddr3, prBssInfo->aucBSSID,
			   TDLS_FME_MAC_ADDR_LEN);
		prHdr->u2SeqCtrl = 0;
		LR_TDLS_FME_FIELD_FILL(sizeof(struct WLAN_MAC_HEADER));

		/* Frame Formation - (1) Category */
		*pPkt = CATEGORY_PUBLIC_ACTION;
		LR_TDLS_FME_FIELD_FILL(1);
	}

	/* 3. Frame Formation - (2) Action */
	*pPkt = ucActionCode;
	LR_TDLS_FME_FIELD_FILL(1);

	/* 3. Frame Formation - Status Code */
	switch (ucActionCode) {
	case TDLS_FRM_ACTION_SETUP_RSP:
	case TDLS_FRM_ACTION_CONFIRM:
	case TDLS_FRM_ACTION_TEARDOWN:
		WLAN_SET_FIELD_16(pPkt, u2StatusCode);
		LR_TDLS_FME_FIELD_FILL(2);
		break;
	}

	/* 3. Frame Formation - (3) Dialog token */
	if (ucActionCode != TDLS_FRM_ACTION_TEARDOWN) {
		*pPkt = ucDialogToken;
		LR_TDLS_FME_FIELD_FILL(1);
	}

	/* Fill elements */
	if (ucActionCode != TDLS_FRM_ACTION_TEARDOWN) {
		/*
		 *  Capability
		 *  Support Rates
		 *  Extended Support Rates
		 *  Supported Channels
		 *  HT Capabilities
		 *  WMM Information Element
		 *  Extended Capabilities
		 *  Link Identifier
		 *  RSNIE
		 *  FTIE
		 *  Timeout Interval
		 */
		if (ucActionCode != TDLS_FRM_ACTION_CONFIRM) {
			/* 3. Frame Formation - (4) Capability: 0x31 0x04,
			 * privacy bit will be set
			 */
			u2CapInfo = assocBuildCapabilityInfo(prAdapter,
					prStaRec);
			WLAN_SET_FIELD_16(pPkt, u2CapInfo);
			LR_TDLS_FME_FIELD_FILL(2);

			/* 4. Append general IEs */
			/*
			 * TODO check HT:
			 *  prAdapter->rWifiVar.rConnSettings.uc2G4BandwidthMode
			 *	must be CONFIG_BW_20_40M.
			 * TODO check HT: HT_CAP_INFO_40M_INTOLERANT
			 *	must be clear if Tdls 20/40 is enabled.
			 */
			u4IeLen = TdlsFrameGeneralIeAppend(prAdapter, prStaRec,
							   pPkt);
			LR_TDLS_FME_FIELD_FILL(u4IeLen);

			/* 5. Frame Formation - Extended capabilities element */
			EXT_CAP_IE(pPkt)->ucId = ELEM_ID_EXTENDED_CAP;
			EXT_CAP_IE(pPkt)->ucLength = 5;

			EXT_CAP_IE(pPkt)->aucCapabilities[0] =
				0x00;	/* bit0 ~ bit7 */
			EXT_CAP_IE(pPkt)->aucCapabilities[1] =
				0x00;	/* bit8 ~ bit15 */
			EXT_CAP_IE(pPkt)->aucCapabilities[2] =
				0x00;	/* bit16 ~ bit23 */
			EXT_CAP_IE(pPkt)->aucCapabilities[3] =
				0x00;	/* bit24 ~ bit31 */
			EXT_CAP_IE(pPkt)->aucCapabilities[4] =
				0x00;	/* bit32 ~ bit39 */

			/* if (prCmd->ucExCap & TDLS_EX_CAP_PEER_UAPSD) */
			EXT_CAP_IE(pPkt)->aucCapabilities[3] |= BIT((28 - 24));
			/* if (prCmd->ucExCap & TDLS_EX_CAP_CHAN_SWITCH) */
			EXT_CAP_IE(pPkt)->aucCapabilities[3] |= BIT((30 - 24));
			/* if (prCmd->ucExCap & TDLS_EX_CAP_TDLS) */
			EXT_CAP_IE(pPkt)->aucCapabilities[4] |= BIT((37 - 32));

			u4IeLen = IE_SIZE(pPkt);
			LR_TDLS_FME_FIELD_FILL(u4IeLen);
		} else {
			/* 5. Frame Formation - WMM Information element */
			if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucQoS)) {

				/* Add WMM IE *//* try to reuse p2p path */
				u4IeLen = mqmGenerateWmmInfoIEByStaRec(
					prAdapter, prBssInfo, prStaRec, pPkt);
				pPkt += u4IeLen;
				u4PktLen += u4IeLen;
			}

		}
	}

	/* 6. Frame Formation - 20/40 BSS Coexistence */
	/*
	 * Follow WiFi test plan, add 20/40 element
	 * to request/response/confirm.
	 */

	/* 6. Frame Formation - HT Operation element */
	/* u4IeLen = rlmFillHtOpIeBody(prBssInfo, pPkt); */
	/* LR_TDLS_FME_FIELD_FILL(u4IeLen); */

	/* 7. Frame Formation - Link identifier element */
	/* Note: Link ID sequence must be correct;
	 * Or the calculated MIC will be error
	 */
	TDLS_LINK_IDENTIFIER_IE(pPkt)->ucId =
		ELEM_ID_LINK_IDENTIFIER;
	TDLS_LINK_IDENTIFIER_IE(pPkt)->ucLength = 18;

	kalMemCopy(TDLS_LINK_IDENTIFIER_IE(pPkt)->aBSSID,
		   prBssInfo->aucBSSID, 6);

	switch (ucActionCode) {
	case TDLS_FRM_ACTION_SETUP_REQ:
	case TDLS_FRM_ACTION_CONFIRM:
	default:
		/* we are initiator */
		pucInitiator = prBssInfo->aucOwnMacAddr;
		pucResponder = pPeerMac;

		if (prStaRec != NULL)
			prStaRec->flgTdlsIsInitiator = TRUE;
		break;
	case TDLS_FRM_ACTION_DISCOVERY_RSP:
	case TDLS_FRM_ACTION_SETUP_RSP:
		/* peer is initiator */
		pucInitiator = pPeerMac;
		pucResponder = prBssInfo->aucOwnMacAddr;

		if (prStaRec != NULL)
			prStaRec->flgTdlsIsInitiator = FALSE;
		break;

	case TDLS_FRM_ACTION_TEARDOWN:
		if (prStaRec != NULL) {
			if (prStaRec->flgTdlsIsInitiator == TRUE) {
				/* we are initiator */
				pucInitiator = prBssInfo->aucOwnMacAddr;
				pucResponder = pPeerMac;
			} else {
				/* peer is initiator */
				pucInitiator = pPeerMac;
				pucResponder = prBssInfo->aucOwnMacAddr;
			}
		}
		break;
	}

	/* 3. Frame Formation - (12) Timeout interval element
	 *	(TPK Key Lifetime)
	 */
	TIMEOUT_INTERVAL_IE(pPkt)->ucId = ELEM_ID_TIMEOUT_INTERVAL;
	TIMEOUT_INTERVAL_IE(pPkt)->ucLength = 5;
	TIMEOUT_INTERVAL_IE(pPkt)->ucType =
		2;	/* IE_TIMEOUT_INTERVAL_TYPE_KEY_LIFETIME; */
	TIMEOUT_INTERVAL_IE(pPkt)->u4Value =
		TDLS_KEY_TIMEOUT_INTERVAL;	/* htonl(prCmd->u4Timeout); */

	u4IeLen = IE_SIZE(pPkt);
	pPkt += u4IeLen;
	u4PktLen += u4IeLen;

	if (ucActionCode != TDLS_FRM_ACTION_TEARDOWN) {
		/*
		 *  bit0 = 1: The Information Request field is used to
		 *  indicate that a transmitting STA is requesting the recipient
		 *  to transmit a 20/40 BSS
		 *  Coexistence Management frame with the transmitting STA as
		 *  the recipient.
		 *  bit1 = 0: The Forty MHz Intolerant field is set to 1
		 *	to prohibit an AP that receives this information
		 *	or reports of this information from operating
		 *	a 20/40 MHz BSS.
		 *  bit2 = 0: The 20 MHz BSS Width Request field is set to 1
		 *	to prohibit a receiving AP from operating its BSS as
		 *	a 20/40 MHz BSS.
		 */
		BSS_20_40_COEXIST_IE(pPkt)->ucId =
			ELEM_ID_20_40_BSS_COEXISTENCE;
		BSS_20_40_COEXIST_IE(pPkt)->ucLength = 1;
		BSS_20_40_COEXIST_IE(pPkt)->ucData = 0x01;
		LR_TDLS_FME_FIELD_FILL(3);
	}

	if (pAppendIe != NULL) {

		kalMemCopy(pPkt, pAppendIe, AppendIeLen);
		LR_TDLS_FME_FIELD_FILL(AppendIeLen);

	}

	/* 7. Append Supported Operating Classes IE */
	if (ucActionCode != TDLS_FRM_ACTION_TEARDOWN) {
		/* Note: if we do not put the IE, Marvell STA will
		 * decline our TDLS setup request
		 */
		u4IeLen = rlmDomainSupOperatingClassIeFill(pPkt);
		LR_TDLS_FME_FIELD_FILL(u4IeLen);
	}

	/* 3. Frame Formation - (16) Link identifier element */
	TDLS_LINK_IDENTIFIER_IE(pPkt)->ucId =
		ELEM_ID_LINK_IDENTIFIER;
	TDLS_LINK_IDENTIFIER_IE(pPkt)->ucLength = 18;

	kalMemCopy(TDLS_LINK_IDENTIFIER_IE(pPkt)->aBSSID,
		   prBssInfo->aucBSSID, 6);
	kalMemCopy(TDLS_LINK_IDENTIFIER_IE(pPkt)->aInitiator,
		   pPeerMac, 6);	/* prAdapter->rMyMacAddr */
	kalMemCopy(TDLS_LINK_IDENTIFIER_IE(pPkt)->aResponder,
		   prAdapter->rMyMacAddr, 6);	/* pPeerMac */

	u4IeLen = IE_SIZE(pPkt);
	pPkt += u4IeLen;
	u4PktLen += u4IeLen;

	/* HT WMM IE append */
	/* HT WMM IE append */
	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucQoS)) {

		/* Add WMM IE *//* try to reuse p2p path */
		u4IeLen = mqmGenerateWmmInfoIEByStaRec(prAdapter, prBssInfo,
						       prStaRec, pPkt);
		pPkt += u4IeLen;
		u4PktLen += u4IeLen;
	}

	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucQoS)) {
		u4IeLen = mqmGenerateWmmParamIEByParam(prAdapter, prBssInfo,
						       pPkt);

		LR_TDLS_FME_FIELD_FILL(u4IeLen);
	}
#if CFG_SUPPORT_802_11AC
	if (prAdapter->rWifiVar.ucAvailablePhyTypeSet &
	    PHY_TYPE_SET_802_11AC) {
		/* Add VHT IE *//* try to reuse p2p path */
		u4IeLen = rlmFillVhtCapIEByAdapter(prAdapter, prBssInfo,
						   pPkt);
		pPkt += u4IeLen;
		u4PktLen += u4IeLen;
	}
#endif

	/* 8. Append security IEs */
	if ((ucActionCode != TDLS_FRM_ACTION_TEARDOWN)
	    && (pAppendIe != NULL)) {
		kalMemCopy(pPkt, pAppendIe, AppendIeLen);
		LR_TDLS_FME_FIELD_FILL(AppendIeLen);
	}

	if (prMsduInfoMgmt != NULL) {
		prMsduInfoMgmt->ucPacketType = TX_PACKET_TYPE_MGMT;
		prMsduInfoMgmt->ucStaRecIndex =
		prBssInfo->prStaRecOfAP->ucIndex;
		prMsduInfoMgmt->ucBssIndex = prBssInfo->ucBssIndex;
		prMsduInfoMgmt->ucMacHeaderLength =
				WLAN_MAC_MGMT_HEADER_LEN;
		prMsduInfoMgmt->fgIs802_1x = FALSE;
		prMsduInfoMgmt->fgIs802_11 = TRUE;
		prMsduInfoMgmt->u2FrameLength = u4PktLen;
		prMsduInfoMgmt->ucTxSeqNum = nicIncreaseTxSeqNum(prAdapter);
		prMsduInfoMgmt->pfTxDoneHandler = NULL;

		/* Send them to HW queue */
		nicTxEnqueueMsdu(prAdapter, prMsduInfoMgmt);
	}

	if (prMsduInfo)
		kalPacketFree(prGlueInfo, prMsduInfo);

	return TDLS_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*! \brief  This routine is called to send a command to TDLS module.
 *
 * \param[in] prGlueInfo	Pointer to the Adapter structure
 * \param[in] prInBuf		A pointer to the command string buffer
 * \param[in] u4InBufLen	The length of the buffer
 * \param[out] None
 *
 * \retval None
 */
/*----------------------------------------------------------------------------*/
void TdlsexEventHandle(struct GLUE_INFO *prGlueInfo,
		       uint8_t *prInBuf, uint32_t u4InBufLen)
{
	uint32_t u4EventId;

	DBGLOG(TDLS, INFO, "TdlsexEventHandle\n");

	/* sanity check */
	if ((prGlueInfo == NULL) || (prInBuf == NULL))
		return;		/* shall not be here */

	/* handle */
	u4EventId = *(uint32_t *) prInBuf;
	u4InBufLen -= 4;

	switch (u4EventId) {
	case TDLS_HOST_EVENT_TEAR_DOWN:
		DBGLOG(TDLS, INFO, "TDLS_HOST_EVENT_TEAR_DOWN\n");
		TdlsEventTearDown(prGlueInfo, prInBuf + 4, u4InBufLen);
		break;

	case TDLS_HOST_EVENT_TX_DONE:

		break;
	}
}

/*----------------------------------------------------------------------------*/
/*! \brief  This routine is called to do tear down.
 *
 * \param[in] prGlueInfo	Pointer to the Adapter structure
 * \param[in] prInBuf		A pointer to the command string buffer,
 *					from u4EventSubId
 * \param[in] u4InBufLen	The length of the buffer
 * \param[out] None
 *
 * \retval None
 *
 */
/*----------------------------------------------------------------------------*/
void TdlsEventTearDown(struct GLUE_INFO *prGlueInfo,
		       uint8_t *prInBuf, uint32_t u4InBufLen)
{
	struct STA_RECORD *prStaRec;
	uint16_t u2ReasonCode = TDLS_REASON_CODE_NONE;
	uint32_t u4TearDownSubId;
	uint8_t *pMac, aucZeroMac[6];

	/* init */
	u4TearDownSubId = *(uint32_t *) prInBuf;
	kalMemZero(aucZeroMac, sizeof(aucZeroMac));
	pMac = aucZeroMac;

	prStaRec = cnmGetStaRecByIndex(prGlueInfo->prAdapter,
				       *(prInBuf + 4));

	/* sanity check */
	if (prStaRec == NULL)
		return;
	else
		pMac = prStaRec->aucMacAddr;

	if (fgIsPtiTimeoutSkip == TRUE) {
		/* skip PTI timeout event */
		if (u4TearDownSubId == TDLS_HOST_EVENT_TD_PTI_TIMEOUT)
			return;
	}

	if (u4TearDownSubId == TDLS_HOST_EVENT_TD_PTI_TIMEOUT) {
		DBGLOG(TDLS, INFO,
	       "TDLS_HOST_EVENT_TD_PTI_TIMEOUT TDLS_REASON_CODE_UNSPECIFIED\n");
		u2ReasonCode = TDLS_REASON_CODE_UNSPECIFIED;

		cfg80211_tdls_oper_request(prGlueInfo->prDevHandler,
				prStaRec->aucMacAddr,
				NL80211_TDLS_TEARDOWN,
				WLAN_REASON_TDLS_TEARDOWN_UNREACHABLE,
				GFP_ATOMIC);
	}

	if (u4TearDownSubId == TDLS_HOST_EVENT_TD_AGE_TIMEOUT) {
		DBGLOG(TDLS, INFO,
	       "TDLS_HOST_EVENT_TD_AGE_TIMEOUT TDLS_REASON_CODE_UNREACHABLE\n");
		u2ReasonCode = TDLS_REASON_CODE_UNREACHABLE;

		cfg80211_tdls_oper_request(prGlueInfo->prDevHandler,
				prStaRec->aucMacAddr, NL80211_TDLS_TEARDOWN,
				WLAN_REASON_TDLS_TEARDOWN_UNREACHABLE,
				GFP_ATOMIC);

	}

	DBGLOG(TDLS, INFO, "\n\n u2ReasonCode = %u\n\n",
	       u2ReasonCode);
}

void TdlsBssExtCapParse(struct STA_RECORD *prStaRec,
			uint8_t *pucIE)
{
	uint8_t *pucIeExtCap;

	/* sanity check */
	if ((prStaRec == NULL) || (pucIE == NULL))
		return;

	if (IE_ID(pucIE) != ELEM_ID_EXTENDED_CAP)
		return;

	/*
	 *  from bit0 ~
	 *  bit 38: TDLS Prohibited
	 *  The TDLS Prohibited subfield indicates whether the use of TDLS is
	 *	prohibited.
	 *  The field is set to 1 to indicate that TDLS is prohibited
	 *	and to 0 to indicate that TDLS is allowed.
	 */
	if (IE_LEN(pucIE) < 5)
		return;		/* we need 39/8 = 5 bytes */

	/* init */
	prStaRec->fgTdlsIsProhibited = FALSE;
	prStaRec->fgTdlsIsChSwProhibited = FALSE;

	/* parse */
	pucIeExtCap = pucIE + 2;
	pucIeExtCap += 4;	/* shift to the byte we care about */

	if ((*pucIeExtCap) & BIT(38 - 32))
		prStaRec->fgTdlsIsProhibited = TRUE;
	if ((*pucIeExtCap) & BIT(39 - 32))
		prStaRec->fgTdlsIsChSwProhibited = TRUE;

}

/*----------------------------------------------------------------------------*/
/*!
 * \brief        Generate CMD_ID_SET_TDLS_CH_SW command
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
uint32_t
TdlsSendChSwControlCmd(struct ADAPTER *prAdapter,
		       void *pvSetBuffer, uint32_t u4SetBufferLen,
		       uint32_t *pu4SetInfoLen)
{

	struct CMD_TDLS_CH_SW rCmdTdlsChSwCtrl;

	ASSERT(prAdapter);

	/* send command packet for scan */
	kalMemZero(&rCmdTdlsChSwCtrl,
		   sizeof(struct CMD_TDLS_CH_SW));

	rCmdTdlsChSwCtrl.fgIsTDLSChSwProhibit =
		prAdapter->prAisBssInfo->fgTdlsIsChSwProhibited;

	wlanSendSetQueryCmd(prAdapter,
			    CMD_ID_SET_TDLS_CH_SW,
			    TRUE,
			    FALSE, FALSE, NULL, NULL,
			    sizeof(struct CMD_TDLS_CH_SW),
			    (uint8_t *)&rCmdTdlsChSwCtrl, NULL, 0);
	return TDLS_STATUS_SUCCESS;
}

#endif /* CFG_SUPPORT_TDLS */

/* End of tdls.c */
