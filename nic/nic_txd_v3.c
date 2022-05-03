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
 ** Id: //Department/DaVinci/BRANCHES/MT6620_WIFI_DRIVER_v3_3/nic/nic_tx.c#2
 */

/*! \file   nic_tx.c
 *    \brief  Functions that provide TX operation in NIC Layer.
 *
 *    This file provides TX functions which are responsible for both Hardware
 *    and Software Resource Management and keep their Synchronization.
 */


#if (CFG_SUPPORT_CONNAC3X == 1)
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
 *                             D A T A   T Y P E S
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
 *                                 M A C R O S
 *******************************************************************************
 */

/*******************************************************************************
 *                  F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */

uint8_t nic_txd_v3_long_format_op(
	void *prTxDesc,
	uint8_t fgSet)
{
	return TRUE;
}

uint8_t nic_txd_v3_tid_op(
	void *prTxDesc,
	uint8_t ucTid,
	uint8_t fgSet)
{
	if (fgSet)
		HAL_MAC_CONNAC3X_TXD_SET_TID_MGMT_TYPE(
			(struct HW_MAC_CONNAC3X_TX_DESC *)prTxDesc, ucTid);
	return HAL_MAC_CONNAC3X_TXD_GET_TID_MGMT_TYPE(
			(struct HW_MAC_CONNAC3X_TX_DESC *)prTxDesc);
}

uint8_t nic_txd_v3_queue_idx_op(
	void *prTxDesc,
	uint8_t ucQueIdx,
	uint8_t fgSet)
{
	if (fgSet)
		HAL_MAC_CONNAC3X_TXD_SET_QUEUE_INDEX(
			(struct HW_MAC_CONNAC3X_TX_DESC *)prTxDesc, ucQueIdx);
	return HAL_MAC_CONNAC3X_TXD_GET_QUEUE_INDEX(
			(struct HW_MAC_CONNAC3X_TX_DESC *)prTxDesc);
}

#if (CFG_TCP_IP_CHKSUM_OFFLOAD == 1)
void nic_txd_v3_chksum_op(void *prTxDesc, uint8_t ucChksumFlag,
			struct MSDU_INFO *prMsduInfo)
{
	if ((ucChksumFlag & TX_CS_IP_GEN))
		HAL_MAC_CONNAC3X_TXD_SET_IP_CHKSUM(
			(struct HW_MAC_CONNAC3X_TX_DESC *)prTxDesc);
	if ((ucChksumFlag & TX_CS_TCP_UDP_GEN))
		HAL_MAC_CONNAC3X_TXD_SET_TCP_UDP_CHKSUM(
			(struct HW_MAC_CONNAC3X_TX_DESC *)prTxDesc);
	/*
	 * If kernel do not expect HW checksum for this frame, set ~AMSDU.
	 * The ICMP frame check were done by checking pfTxDoneHandler
	 * in nic_txd_*_compose().
	 * In that case ICMP do not need HW checksum would cause following
	 * frames need checksum but skipped, but only happened if IcmpTxs
	 * were disabled for special test case.
	 */
	if (!(ucChksumFlag & (TX_CS_IP_GEN | TX_CS_TCP_UDP_GEN)) &&
	    prMsduInfo->ucPktType != ENUM_PKT_ICMP)
		HAL_MAC_CONNAC3X_TXD_UNSET_HW_AMSDU(
			(struct HW_MAC_CONNAC3X_TX_DESC *)prTxDesc);
}
#endif /* CFG_TCP_IP_CHKSUM_OFFLOAD == 1 */

void nic_txd_v3_header_format_op(
	void *prTxDesc,
	struct MSDU_INFO *prMsduInfo)
{
	if (!prMsduInfo->fgIs802_11) {
		if (prMsduInfo->fgIs802_3)
			HAL_MAC_CONNAC3X_TXD_UNSET_ETHERNET_II(
				(struct HW_MAC_CONNAC3X_TX_DESC *)prTxDesc);
		if (prMsduInfo->fgIsVlanExists)
			HAL_MAC_CONNAC3X_TXD_SET_VLAN(
				(struct HW_MAC_CONNAC3X_TX_DESC *)prTxDesc);
	}
}

void nic_txd_v3_fill_by_pkt_option(
	struct MSDU_INFO *prMsduInfo,
	void *prTxD)
{
	struct HW_MAC_CONNAC3X_TX_DESC *prTxDesc =
				(struct HW_MAC_CONNAC3X_TX_DESC *)prTxD;
	uint32_t u4PktOption = prMsduInfo->u4Option;
	u_int8_t fgProtected = FALSE;

	/* Skip this function if no options is set */
	if (!u4PktOption)
		return;

	/* Fields in DW0 and DW1 (Short Format) */
	if (u4PktOption & MSDU_OPT_NO_ACK)
		HAL_MAC_CONNAC3X_TXD_SET_NO_ACK(prTxDesc);

	if (u4PktOption & MSDU_OPT_PROTECTED_FRAME) {
		/* DBGLOG(RSN, INFO, "MSDU_OPT_PROTECTED_FRAME\n"); */
		HAL_MAC_CONNAC3X_TXD_SET_PROTECTION(prTxDesc);
		fgProtected = TRUE;
	}

	switch (HAL_MAC_CONNAC3X_TXD_GET_HEADER_FORMAT(prTxDesc)) {
	case HEADER_FORMAT_802_11_ENHANCE_MODE:
		if (u4PktOption & MSDU_OPT_EOSP)
			HAL_MAC_CONNAC3X_TXD_SET_EOSP(prTxDesc);

		if (u4PktOption & MSDU_OPT_AMSDU)
			HAL_MAC_CONNAC3X_TXD_SET_AMSDU(prTxDesc);
		break;

	case HEADER_FORMAT_NON_802_11:
		if (u4PktOption & MSDU_OPT_EOSP)
			HAL_MAC_CONNAC3X_TXD_SET_EOSP(prTxDesc);

		if (u4PktOption & MSDU_OPT_MORE_DATA)
			HAL_MAC_CONNAC3X_TXD_SET_MORE_DATA(prTxDesc);

		if (u4PktOption & MSDU_OPT_REMOVE_VLAN)
			HAL_MAC_CONNAC3X_TXD_SET_REMOVE_VLAN(prTxDesc);
		break;

	case HEADER_FORMAT_802_11_NORMAL_MODE:
		if (fgProtected && prMsduInfo->prPacket) {
			struct WLAN_MAC_HEADER *prWlanHeader =
			    (struct WLAN_MAC_HEADER *)
			    ((unsigned long) (prMsduInfo->prPacket)
				+ MAC_TX_RESERVED_FIELD);

			prWlanHeader->u2FrameCtrl |= MASK_FC_PROTECTED_FRAME;
		}
		break;

	default:
		break;
	}

	/* Fields in DW2~6 (Long Format) */
	if (u4PktOption & MSDU_OPT_NO_AGGREGATE)
		HAL_MAC_CONNAC3X_TXD_SET_BA_DISABLE(prTxDesc);

	if (u4PktOption & MSDU_OPT_FORCE_RTS)
		HAL_MAC_CONNAC3X_TXD_SET_FORCE_RTS_CTS(prTxDesc);

	/* SW field */
	if (u4PktOption & MSDU_OPT_SW_DURATION)
		HAL_MAC_CONNAC3X_TXD_SET_DURATION_CONTROL_BY_SW(prTxDesc);

	if (u4PktOption & MSDU_OPT_SW_PS_BIT)
		HAL_MAC_CONNAC3X_TXD_SET_SW_PM_CONTROL(prTxDesc);

	if (u4PktOption & MSDU_OPT_SW_HTC)
		HAL_MAC_CONNAC3X_TXD_SET_HTC_EXIST(prTxDesc);

	if (u4PktOption & MSDU_OPT_MANUAL_SN) {
		HAL_MAC_CONNAC3X_TXD_SET_TXD_SN_VALID(prTxDesc);
		HAL_MAC_CONNAC3X_TXD_SET_SEQUENCE_NUMBER
			(prTxDesc, prMsduInfo->u2SwSN);
	}

}

/*----------------------------------------------------------------------------*/
/*!
* @brief In this function, we'll compose the Tx descriptor of the MSDU.
*
* @param prAdapter              Pointer to the Adapter structure.
* @param prMsduInfo             Pointer to the Msdu info
* @param prTxDesc               Pointer to the Tx descriptor buffer
*
* @retval VOID
*/
/*----------------------------------------------------------------------------*/
void nic_txd_v3_compose(
	struct ADAPTER *prAdapter,
	struct MSDU_INFO *prMsduInfo,
	u_int32_t u4TxDescLength,
	u_int8_t fgIsTemplate,
	u_int8_t *prTxDescBuffer)
{
	struct HW_MAC_CONNAC3X_TX_DESC *prTxDesc;
	struct STA_RECORD *prStaRec;
	struct BSS_INFO *prBssInfo;
#if (CFG_SUPPORT_802_11BE_MLO == 1)
	struct MLD_STA_RECORD *prMldSta;
#endif
	u_int32_t u4TxDescAndPaddingLength;
	u_int8_t ucWmmQueSet, ucTarQueue, ucTarPort;
	uint8_t ucEtherTypeOffsetInWord;
	uint8_t fgIsALTXQueue = FALSE;
	uint8_t *apucPktType[ENUM_PKT_FLAG_NUM] = {
		(uint8_t *) DISP_STRING("802_11"),
		(uint8_t *) DISP_STRING("802_3"),
		(uint8_t *) DISP_STRING("1X"),
		(uint8_t *) DISP_STRING("NON_PROTECTED_1X"),
		(uint8_t *) DISP_STRING("VLAN_EXIST"),
		(uint8_t *) DISP_STRING("DHCP"),
		(uint8_t *) DISP_STRING("ARP"),
		(uint8_t *) DISP_STRING("ICMP"),
		(uint8_t *) DISP_STRING("TDLS"),
		(uint8_t *) DISP_STRING("DNS"),
#if CFG_SUPPORT_TPENHANCE_MODE
		(uint8_t *) DISP_STRING("TCP_ACK"),
#endif /* CFG_SUPPORT_TPENHANCE_MODE */
#if CFG_SUPPORT_TX_MGMT_USE_DATAQ
		(uint8_t *) DISP_STRING("802_11_MGMT"),
#endif
	};

	prTxDesc = (struct HW_MAC_CONNAC3X_TX_DESC *) prTxDescBuffer;
	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prMsduInfo->ucBssIndex);
	prStaRec = cnmGetStaRecByIndex(prAdapter, prMsduInfo->ucStaRecIndex);
#if (CFG_SUPPORT_802_11BE_MLO == 1)
	prMldSta = mldStarecGetByStarec(prAdapter, prStaRec);
#endif

	u4TxDescAndPaddingLength = u4TxDescLength + NIC_TX_DESC_PADDING_LENGTH;

	kalMemZero(prTxDesc, u4TxDescAndPaddingLength);

	ucTarPort = nicTxGetTxDestPortIdxByTc(prMsduInfo->ucTC);

	nicTxForceAmsduForCert(prAdapter, (uint8_t *)prTxDesc);
	/** DW0 **/
	/* Packet Format */
	ucWmmQueSet = prBssInfo->ucWmmQueSet;
	if (fgIsTemplate != TRUE
		&& prMsduInfo->ucPacketType == TX_PACKET_TYPE_DATA
		&& ucWmmQueSet != prMsduInfo->ucWmmQueSet) {
		DBGLOG(RSN, ERROR,
			"ucStaRecIndex:%x ucWmmQueSet mismatch[%d,%d]\n",
			prMsduInfo->ucStaRecIndex,
			ucWmmQueSet, prMsduInfo->ucWmmQueSet);
	}

#if (CFG_SUPPORT_FORCE_ALTX == 1)
	fgIsALTXQueue |=
		(ucTarPort == PORT_INDEX_MCU &&
		prMsduInfo->ucControlFlag &
		MSDU_CONTROL_FLAG_FORCE_TX);
#endif /* CFG_SUPPORT_FORCE_ALTX == 1 */

#if (CFG_TX_MGMT_BY_DATA_Q == 1)
	fgIsALTXQueue |=
		(prMsduInfo->fgMgmtUseDataQ &&
		prMsduInfo->ucControlFlag &
		MSDU_CONTROL_FLAG_FORCE_TX);
#endif /* CFG_TX_MGMT_BY_DATA_Q == 1 */

	if (fgIsALTXQueue)
		/* packet with always tx flag */
		ucTarQueue = MAC_TXQ_ALTX_0_INDEX;
	else
	{
		ucTarQueue = nicTxGetTxDestQIdxByTc(prMsduInfo->ucTC);
		if (ucTarPort == PORT_INDEX_LMAC)
			ucTarQueue +=
				(ucWmmQueSet * WMM_AC_INDEX_NUM);
	}

	HAL_MAC_CONNAC3X_TXD_SET_QUEUE_INDEX(prTxDesc, ucTarQueue);

	/* Packet Format */
	HAL_MAC_CONNAC3X_TXD_SET_PKT_FORMAT(
		prTxDesc, prMsduInfo->ucPacketFormat);

	/* Ether-type offset */
	if (!prMsduInfo->fgIs802_11) {
		ucEtherTypeOffsetInWord = ((ETHER_HEADER_LEN - ETHER_TYPE_LEN)
				+ prAdapter->chip_info->pse_header_length) >> 1;

		HAL_MAC_CONNAC3X_TXD_SET_ETHER_TYPE_OFFSET(
			prTxDesc, ucEtherTypeOffsetInWord);
	}

	/** DW1 **/
	/* MLDID-legacy */
	prMsduInfo->ucWlanIndex = nicTxGetWlanIdx(prAdapter,
		prMsduInfo->ucBssIndex, prMsduInfo->ucStaRecIndex);
#if (CFG_SUPPORT_802_11BE_MLO == 1)
	if (prMldSta) {
		HAL_MAC_CONNAC3X_TXD_SET_MLD_ID(
			prTxDesc,
			(prMsduInfo->ucUserPriority & 0xf) % 2 == 0 ?
				prMldSta->u2PrimaryMldId : prMldSta->u2SecondMldId);
	} else
#endif
	{
		HAL_MAC_CONNAC3X_TXD_SET_MLD_ID(
			prTxDesc,
			prMsduInfo->ucWlanIndex);
	}

	/* Header format */
	if (prMsduInfo->fgIs802_11) {
		HAL_MAC_CONNAC3X_TXD_SET_HEADER_FORMAT(
			prTxDesc, HEADER_FORMAT_802_11_NORMAL_MODE);
		HAL_MAC_CONNAC3X_TXD_SET_802_11_HEADER_LENGTH(
			prTxDesc, (prMsduInfo->ucMacHeaderLength >> 1));
	} else {
		HAL_MAC_CONNAC3X_TXD_SET_HEADER_FORMAT(
			prTxDesc, HEADER_FORMAT_NON_802_11);
		HAL_MAC_CONNAC3X_TXD_SET_ETHERNET_II(prTxDesc);
	}
	HAL_MAC_CONNAC3X_TXD_SET_HEADER_PADDING(
		prTxDesc, NIC_TX_DESC_HEADER_PADDING_LENGTH);

	/* TID mgmt type */
	if (prMsduInfo->fgIs802_11) {
		HAL_MAC_CONNAC3X_TXD_SET_TID_MGMT_TYPE(prTxDesc,
			TYPE_NORMAL_MANAGEMENT);
	} else {
		HAL_MAC_CONNAC3X_TXD_SET_TID_MGMT_TYPE(prTxDesc,
			prMsduInfo->ucUserPriority);
	}

	/* Own MAC */
	HAL_MAC_CONNAC3X_TXD_SET_OWN_MAC_INDEX(
		prTxDesc, prBssInfo->ucOwnMacIndex);

	/* TGID should align HW band idx */
	HAL_MAC_CONNAC3X_TXD_SET_TGID(prTxDesc, prBssInfo->eBandIdx);

	/** DW2 **/
	/* Type */
	if (prMsduInfo->fgIs802_11) {
		struct WLAN_MAC_HEADER *prWlanHeader = (struct WLAN_MAC_HEADER *)(
			(unsigned long) (prMsduInfo->prPacket) + MAC_TX_RESERVED_FIELD);

		HAL_MAC_CONNAC3X_TXD_SET_TYPE(
			prTxDesc,
			(prWlanHeader->u2FrameCtrl & MASK_FC_TYPE) >> 2);

		HAL_MAC_CONNAC3X_TXD_SET_SUB_TYPE(
			prTxDesc,
			(prWlanHeader->u2FrameCtrl & MASK_FC_SUBTYPE)
				>> OFFSET_OF_FC_SUBTYPE);
	} else {
		HAL_MAC_CONNAC3X_TXD_SET_TYPE(
			prTxDesc,
			MAC_FRAME_TYPE_DATA >> 2);

		HAL_MAC_CONNAC3X_TXD_SET_SUB_TYPE(
			prTxDesc,
			(prStaRec && prStaRec->fgIsQoS) ?
				MAC_FRAME_QOS_DATA >> 4 : MAC_FRAME_DATA >> 4);
	}

	/* Remaining TX time */
	if (!(prMsduInfo->u4Option & MSDU_OPT_MANUAL_LIFE_TIME))
		prMsduInfo->u4RemainingLifetime =
			nicTxGetRemainingTxTimeByTc(prMsduInfo->ucTC);
	HAL_MAC_CONNAC3X_TXD_SET_REMAINING_LIFE_TIME_IN_MS(
		prTxDesc, prMsduInfo->u4RemainingLifetime);

	/* Power Offset */
	HAL_MAC_CONNAC3X_TXD_SET_POWER_OFFSET(
		prTxDesc, prMsduInfo->cPowerOffset);

	/* OM MAP */
	//HAL_MAC_CONNAC3X_TXD_SET_OM_MAP(prTxDesc);

	/** DW3 **/
	/* Tx count limit */
	if (!(prMsduInfo->u4Option & MSDU_OPT_MANUAL_RETRY_LIMIT)) {
		/* Note: BMC packet retry limit is set to unlimited */
		prMsduInfo->ucRetryLimit =
			nicTxGetTxCountLimitByTc(prMsduInfo->ucTC);
	}
	HAL_MAC_CONNAC3X_TXD_SET_REMAINING_TX_COUNT(
		prTxDesc, prMsduInfo->ucRetryLimit);

	/* BMC packet */
	if (prMsduInfo->ucStaRecIndex == STA_REC_INDEX_BMCAST) {
		HAL_MAC_CONNAC3X_TXD_SET_BMC(prTxDesc);
		/* Must set No ACK to mask retry bit in FC */
		HAL_MAC_CONNAC3X_TXD_SET_NO_ACK(prTxDesc);
	}

	/* Protection */
	if (secIsProtectedFrame(prAdapter, prMsduInfo, prStaRec)) {
		/* Update Packet option, */
		/* PF bit will be set in nicTxFillDescByPktOption() */
		if ((prStaRec
			&& prStaRec->fgTransmitKeyExist) || fgIsTemplate) {
			DBGLOG_LIMITED(RSN, TRACE,
				"Set MSDU_OPT_PROTECTED_FRAME\n");
			nicTxConfigPktOption(
				prMsduInfo, MSDU_OPT_PROTECTED_FRAME, TRUE);

			if (prMsduInfo->fgIs802_1x &&
			    prMsduInfo->fgIs802_1x_NonProtected) {
				nicTxConfigPktOption(
					prMsduInfo,
					MSDU_OPT_PROTECTED_FRAME, FALSE);
				DBGLOG(RSN, LOUD,
					"Pairwise EAPoL not protect!\n");
			}
		} else if (prMsduInfo->ucStaRecIndex
				== STA_REC_INDEX_BMCAST) {/* BMC packet */
			nicTxConfigPktOption(
				prMsduInfo, MSDU_OPT_PROTECTED_FRAME, TRUE);
			DBGLOG(RSN, LOUD, "Protect BMC frame!\n");
		}
	}

	/* Update Packet option */
	nic_txd_v3_fill_by_pkt_option(prMsduInfo, prTxDesc);

	/** DW5 **/
	/* PID */
	if (prMsduInfo->pfTxDoneHandler) {
		prMsduInfo->ucPID = nicTxAssignPID(
				prAdapter, prMsduInfo->ucWlanIndex);

		DBGLOG(TX, INFO, "TX[%s] PID[%d]\n",
			apucPktType[prMsduInfo->ucPktType], prMsduInfo->ucPID);
		HAL_MAC_CONNAC3X_TXD_SET_PID(prTxDesc, prMsduInfo->ucPID);
		HAL_MAC_CONNAC3X_TXD_SET_TXS_TO_MCU(prTxDesc);
		/* TXS is MPDU based, AMSDU will cause TX skb leak in driver */
		HAL_MAC_CONNAC3X_TXD_UNSET_HW_AMSDU(prTxDesc);
	} else if (prAdapter->rWifiVar.ucDataTxDone == 2) {
		/* Log mode: only TxS to FW, no event to driver */
		HAL_MAC_CONNAC3X_TXD_SET_PID(
			prTxDesc, NIC_TX_DESC_PID_RESERVED);
		HAL_MAC_CONNAC3X_TXD_SET_TXS_TO_MCU(prTxDesc);
	}

	/* altx set TGID and force link */
	if (ucTarQueue == MAC_TXQ_ALTX_0_INDEX)
		HAL_MAC_CONNAC3X_TXD_SET_FORCE_ASSIGN_LINK(prTxDesc);

	/** DW6 **/
	/* Disable MLD to link address translation */
	if (prMsduInfo->ucPacketType == TX_PACKET_TYPE_MGMT)
		HAL_MAC_CONNAC3X_TXD_SET_DIS_MAT(prTxDesc);

	/* Msdu count */
	HAL_MAC_CONNAC3X_TXD_SET_MSDU_COUNT(prTxDesc, 1);

	if (prMsduInfo->ucStaRecIndex != STA_REC_INDEX_BMCAST)
		HAL_MAC_CONNAC3X_TXD_SET_DA_SRC_SELECTION(prTxDesc);

	/** DW7 **/
	HAL_MAC_CONNAC3X_TXD_SET_TXD_LENGTH(prTxDesc, TXD_LEN_1_PAGE);

	/* Fix rate */
	DBGLOG(TX, INFO, "Rate mode[%d]\n", prMsduInfo->ucRateMode);
	// TODO: Revise rate configuration for low rate
	switch (prMsduInfo->ucRateMode) {
	case MSDU_RATE_MODE_MANUAL_DESC:
		HAL_MAC_CONNAC3X_TXD_SET_FIXED_RATE_ENABLE(prTxDesc);
		HAL_MAC_CONNAC3X_TXD_SET_FIXED_RATE_IDX(prTxDesc,
				prBssInfo->eBand == BAND_2G4 ?
					FIXED_RATE_INDEX_CCK_1M :
					FIXED_RATE_INDEX_OFDM_6M);
		HAL_MAC_CONNAC3X_TXD_SET_FR_BW(prTxDesc, 0x8);
	case MSDU_RATE_MODE_MANUAL_CR:
	case MSDU_RATE_MODE_AUTO:
	default:
		break;
	}
}

void nic_txd_v3_set_pkt_fixed_rate_option_full(struct MSDU_INFO
	*prMsduInfo,
	uint16_t u2RateCode,
	uint8_t ucBandwidth,
	u_int8_t fgShortGI,
	u_int8_t fgLDPC,
	u_int8_t fgDynamicBwRts, u_int8_t fgBeamforming,
	uint8_t ucAntennaIndex)
{
#define UNUSED(x) ((void)(x))

	UNUSED(prMsduInfo);
	UNUSED(u2RateCode);
	UNUSED(ucBandwidth);
	UNUSED(fgShortGI);
	UNUSED(fgLDPC);
	UNUSED(fgDynamicBwRts);
	UNUSED(fgBeamforming);
	UNUSED(ucAntennaIndex);

	/* Fixed Rate */
	prMsduInfo->ucRateMode = MSDU_RATE_MODE_MANUAL_DESC;
}

void nic_txd_v3_set_pkt_fixed_rate_option(
	struct MSDU_INFO *prMsduInfo,
	uint16_t u2RateCode,
	uint8_t ucBandwidth,
	u_int8_t fgShortGI,
	u_int8_t fgDynamicBwRts)
{
#define UNUSED(x) ((void)(x))

	UNUSED(prMsduInfo);
	UNUSED(u2RateCode);
	UNUSED(ucBandwidth);
	UNUSED(fgShortGI);
	UNUSED(fgDynamicBwRts);

	/* Fixed Rate */
	prMsduInfo->ucRateMode = MSDU_RATE_MODE_MANUAL_DESC;
}

void nic_txd_v3_set_hw_amsdu_template(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	u_int8_t ucTid,
	IN u_int8_t fgSet)
{
	struct HW_MAC_CONNAC3X_TX_DESC *prTxDesc;

	DBGLOG(QM, INFO,
		"Update HW Amsdu field of TXD template for STA[%u] Tid[%u]\n",
		prStaRec->ucIndex, ucTid);

	if (prStaRec->aprTxDescTemplate[ucTid]) {
		prTxDesc =
			(struct HW_MAC_CONNAC3X_TX_DESC *)
			prStaRec->aprTxDescTemplate[ucTid];
		if (fgSet)
			HAL_MAC_CONNAC3X_TXD_SET_HW_AMSDU(prTxDesc);
		else
			HAL_MAC_CONNAC3X_TXD_UNSET_HW_AMSDU(prTxDesc);
	}
}
#endif /* CFG_SUPPORT_CONNAC3X == 1 */
