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
/******************************************************************************
 *[File]             dbg_connac3x.c
 *[Version]          v1.0
 *[Revision Date]    2019-04-09
 *[Author]
 *[Description]
 *    The program provides WIFI FALCON MAC Debug APIs
 *[Copyright]
 *    Copyright (C) 2015 MediaTek Incorporation. All Rights Reserved.
 ******************************************************************************/

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
#include "mt_dmac.h"
#include "wf_ple.h"
#include "dbg_wtbl_connac3x.h"
#ifdef BELLWETHER
#include "coda/bellwether/wf_hif_dmashdl_top.h"
#include "coda/bellwether/wf_ple_top.h"
#include "coda/bellwether/wf_pse_top.h"
#include "coda/bellwether/wf_wfdma_host_dma0.h"
#include "coda/bellwether/bn0_wf_mib_top.h"
#include "coda/bellwether/bn1_wf_mib_top.h"
#include "coda/bellwether/wf_umib_top.h"
#include "coda/bellwether/ip1_bn0_wf_mib_top.h"
#endif
#ifdef MT6639
#include "coda/mt6639/wf_hif_dmashdl_top.h"
#include "coda/mt6639/wf_ple_top.h"
#include "coda/mt6639/wf_pse_top.h"
#include "coda/mt6639/wf_wfdma_host_dma0.h"
#include "coda/mt6639/bn0_wf_mib_top.h"
#include "coda/mt6639/bn1_wf_mib_top.h"
#include "coda/mt6639/wf_umib_top.h"
#include "coda/mt6639/mawd_reg.h"
#include "coda/mt6639/wf_rro_top.h"
#endif
#ifdef MT6655
#include "coda/mt6655/wf_hif_dmashdl_top.h"
#include "coda/mt6655/wf_ple_top.h"
#include "coda/mt6655/wf_pse_top.h"
#include "coda/mt6655/wf_wfdma_host_dma0.h"
#include "coda/mt6655/bn0_wf_mib_top.h"
#include "coda/mt6655/bn1_wf_mib_top.h"
#include "coda/mt6655/wf_umib_top.h"
#include "coda/mt6655/mawd_reg.h"
#include "coda/mt6655/wf_rro_top.h"
#endif
#ifdef MT7990
#include "coda/mt7990/wf_hif_dmashdl_top.h"
#include "coda/mt7990/wf_ple_top.h"
#include "coda/mt7990/wf_pse_top.h"
#include "coda/mt7990/wf_wfdma_host_dma0.h"
#include "coda/mt7990/bn0_wf_mib_top.h"
#include "coda/mt7990/bn1_wf_mib_top.h"
#include "coda/mt7990/wf_umib_top.h"
#include "coda/mt7990/ip1_bn0_wf_mib_top.h"
#endif

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */
#define WTBL_VER			3

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
static char *RATE_TBLE[] = {"B", "G", "N", "N_2SS", "AC", "AC_2SS", "BG",
				"HE", "HE_2SS", "EHT", "EHT_2SS", "N/A"};

static char *RA_STATUS_TBLE[] = {"INVALID", "POWER_SAVING", "SLEEP", "STANDBY",
					"RUNNING", "N/A"};

static struct EMPTY_QUEUE_INFO ple_queue_empty_info[] = {
	{"CPU Q0",  ENUM_UMAC_CPU_PORT_1,     ENUM_UMAC_CTX_Q_0, 0},
	{"CPU Q1",  ENUM_UMAC_CPU_PORT_1,     ENUM_UMAC_CTX_Q_1, 0},
	{"CPU Q2",  ENUM_UMAC_CPU_PORT_1,     ENUM_UMAC_CTX_Q_2, 0},
	{"CPU Q3",  ENUM_UMAC_CPU_PORT_1,     ENUM_UMAC_CTX_Q_3, 0},
	{"ALTX Q0", ENUM_UMAC_LMAC_PORT_2,    0x10, 0},
	{"BMC Q0",  ENUM_UMAC_LMAC_PORT_2,    0x11, 0},
	{"BCN Q0",  ENUM_UMAC_LMAC_PORT_2,    0x12, 0},
	{"PSMP Q0", ENUM_UMAC_LMAC_PORT_2,    0x13, 0},
	{"ALTX Q1", ENUM_UMAC_LMAC_PORT_2,    0x10, 1},
	{"BMC Q1",  ENUM_UMAC_LMAC_PORT_2,    0x11, 1},
	{"BCN Q1",  ENUM_UMAC_LMAC_PORT_2,    0x12, 1},
	{"PSMP Q1", ENUM_UMAC_LMAC_PORT_2,    0x13, 1},
	{"ALTX Q2", ENUM_UMAC_LMAC_PORT_2,    0x10, 2},
	{"BMC Q2",  ENUM_UMAC_LMAC_PORT_2,    0x11, 2},
	{"BCN Q2",  ENUM_UMAC_LMAC_PORT_2,    0x12, 2},
	{"PSMP Q2", ENUM_UMAC_LMAC_PORT_2,    0x13, 2},
	{"NAF Q",   ENUM_UMAC_LMAC_PORT_2,    0x18, 0},
	{"NBCN Q",  ENUM_UMAC_LMAC_PORT_2,    0x19, 0},
	{NULL, 0, 0, 0}, {NULL, 0, 0, 0}, /* 18, 19 not defined */
	{"FIXFID Q", ENUM_UMAC_LMAC_PORT_2, 0x1a, 0},
	{NULL, 0, 0, 0}, {NULL, 0, 0, 0}, {NULL, 0, 0, 0}, {NULL, 0, 0, 0}, {NULL, 0, 0, 0},
	{NULL, 0, 0, 0}, {NULL, 0, 0, 0},
	{"RLS4 Q",   ENUM_PLE_CTRL_PSE_PORT_3, 0x7c, 0},
	{"RLS3 Q",   ENUM_PLE_CTRL_PSE_PORT_3, 0x7d, 0},
	{"RLS2 Q",   ENUM_PLE_CTRL_PSE_PORT_3, 0x7e, 0},
	{"RLS Q",  ENUM_PLE_CTRL_PSE_PORT_3, 0x7f, 0}
};

static struct EMPTY_QUEUE_INFO pse_queue_empty_info[] = {
	{"CPU Q0", ENUM_UMAC_CPU_PORT_1, ENUM_UMAC_CTX_Q_0},
	{"CPU Q1", ENUM_UMAC_CPU_PORT_1, ENUM_UMAC_CTX_Q_1},
	{"CPU Q2", ENUM_UMAC_CPU_PORT_1, ENUM_UMAC_CTX_Q_2},
	{"CPU Q3", ENUM_UMAC_CPU_PORT_1, ENUM_UMAC_CTX_Q_3},
	{"HIF Q8", ENUM_UMAC_HIF_PORT_0, 8},
	{"HIF Q9", ENUM_UMAC_HIF_PORT_0, 9},
	{"HIF Q10", ENUM_UMAC_HIF_PORT_0, 10},
	{"HIF Q11", ENUM_UMAC_HIF_PORT_0, 11},
	{"HIF Q0", ENUM_UMAC_HIF_PORT_0, 0}, /*bit 8*/
	{"HIF Q1", ENUM_UMAC_HIF_PORT_0, 1},
	{"HIF Q2", ENUM_UMAC_HIF_PORT_0, 2},
	{"HIF Q3", ENUM_UMAC_HIF_PORT_0, 3},
	{"HIF Q4", ENUM_UMAC_HIF_PORT_0, 4},
	{"HIF Q5", ENUM_UMAC_HIF_PORT_0, 5},
	{"HIF Q6", ENUM_UMAC_HIF_PORT_0, 6},
	{"HIF Q7", ENUM_UMAC_HIF_PORT_0, 7},
	{"LMAC Q", ENUM_UMAC_LMAC_PORT_2, 0}, /*bit 16*/
	{"MDP TX Q", ENUM_UMAC_LMAC_PORT_2, 1},
	{"MDP RX Q", ENUM_UMAC_LMAC_PORT_2, 2},
	{"SEC TX Q", ENUM_UMAC_LMAC_PORT_2, 3},
	{"SEC RX Q", ENUM_UMAC_LMAC_PORT_2, 4},
	{"SFD_PARK Q", ENUM_UMAC_LMAC_PORT_2, 5},
	{"MDP_TXIOC Q", ENUM_UMAC_LMAC_PORT_2, 6},
	{"MDP_RXIOC Q", ENUM_UMAC_LMAC_PORT_2, 7},
	{"MDP_TX1 Q", ENUM_UMAC_LMAC_PORT_2, 17}, /*bit 24*/
	{"SEC_TX1 Q", ENUM_UMAC_LMAC_PORT_2, 19},
	{"MDP_TXIOC1 Q", ENUM_UMAC_LMAC_PORT_2, 22},
	{"MDP_RXIOC1 Q", ENUM_UMAC_LMAC_PORT_2, 23},
	{NULL, 0, 0},
	{NULL, 0, 0},
	{NULL, 0, 0}, /* 28~30 not defined */
	{"RLS Q", ENUM_PLE_CTRL_PSE_PORT_3, ENUM_UMAC_PLE_CTRL_P3_Q_0X1F}
};

static struct EMPTY_QUEUE_INFO pse_queue_empty2_info[] = {
	{"MDP_TDPIOC Q0", ENUM_UMAC_LMAC_PORT_2, 0x8},
	{"MDP_RDPIOC Q0", ENUM_UMAC_LMAC_PORT_2, 0x9},
	{"MDP_TDPIOC Q1", ENUM_UMAC_LMAC_PORT_2, 0x18},
	{"MDP_RDPIOC Q1", ENUM_UMAC_LMAC_PORT_2, 0x19},
	{"MDP_TDPIOC Q2", ENUM_UMAC_LMAC_PORT_2, 0x28},
	{"MDP_RDPIOC Q2", ENUM_UMAC_LMAC_PORT_2, 0x29},
	{NULL, 0, 0},
	{"MDP_RDPIOC Q3", ENUM_UMAC_LMAC_PORT_2, 0x39},
	{"MDP TX Q2", ENUM_UMAC_LMAC_PORT_2, 0x21},
	{"SEC TX Q2", ENUM_UMAC_LMAC_PORT_2, 0x23},
	{"MDP_TXIOC Q2", ENUM_UMAC_LMAC_PORT_2, 0x26},
	{"MDP_RXIOC Q2", ENUM_UMAC_LMAC_PORT_2, 0x27},
	{NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0},
	{"MDP_RXIOC Q3", ENUM_UMAC_LMAC_PORT_2, 0x37},
	{"HIF Q0", ENUM_UMAC_HIF_PORT_0,    0},
	{"HIF Q1", ENUM_UMAC_HIF_PORT_0,    1},
	{"HIF Q2", ENUM_UMAC_HIF_PORT_0,    2},
	{"HIF Q3", ENUM_UMAC_HIF_PORT_0,    3},
	{"HIF Q4", ENUM_UMAC_HIF_PORT_0,    4},
	{"HIF Q5", ENUM_UMAC_HIF_PORT_0,    5},
	{"HIF Q6", ENUM_UMAC_HIF_PORT_0,    6},
	{"HIF Q7", ENUM_UMAC_HIF_PORT_0,    7},
	{"HIF Q8", ENUM_UMAC_HIF_PORT_0,    8},
	{"HIF Q9", ENUM_UMAC_HIF_PORT_0,    9},
	{"HIF Q10", ENUM_UMAC_HIF_PORT_0,    10},
	{"HIF Q11", ENUM_UMAC_HIF_PORT_0,    11},
	{"HIF Q12", ENUM_UMAC_HIF_PORT_0,    12},
	{"HIF Q13", ENUM_UMAC_HIF_PORT_0,    13},
	{NULL, 0, 0}, {NULL, 0, 0}
};

static u_int8_t *sta_ctrl_reg[] = {"ENABLE", "DISABLE", "PAUSE"};

/*******************************************************************************
 *                                 M A C R O S
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

void connac3x_dump_tmac_info(
	struct ADAPTER *prAdapter,
	uint8_t *tmac_info)
{
	static const char * const pkt_ft_str[] = {
		"cut_through",
		"store_forward",
		"cmd",
		"PDA_FW_Download"
	};

	static const char * const hdr_fmt_str[] = {
		"Non-80211-Frame",
		"Command-Frame",
		"Normal-80211-Frame",
		"enhanced-80211-Frame",
	};
	struct HW_MAC_CONNAC3X_TX_DESC *txd =
		(struct HW_MAC_CONNAC3X_TX_DESC *)tmac_info;

	DBGLOG(HAL, INFO, "TMAC_TXD Fields:\n");
	DBGLOG(HAL, INFO, "\tTMAC_TXD_0:\n");

	/* DW0 */
	/* TX Byte Count [15:0]  */
	DBGLOG(HAL, INFO, "\t\tTxByteCnt = %d\n",
		((txd->u4DW0 & CONNAC3X_TX_DESC_TX_BYTE_COUNT_MASK) >>
		CONNAC3X_TX_DESC_TX_BYTE_COUNT_OFFSET));

	/* Ether Type Offset [22:16]  */
	DBGLOG(HAL, INFO, "\t\tEtherTypeOffset = %d\n",
		((txd->u4DW0 & CONNAC3X_TX_DESC_ETHER_TYPE_OFFSET_MASK) >>
		CONNAC3X_TX_DESC_ETHER_TYPE_OFFSET_OFFSET));

	/* PKT_FT: Packet Format [24:23] */
	DBGLOG(HAL, INFO, "\t\tpkt_ft = %d(%s)\n",
	((txd->u4DW0 & CONNAC3X_TX_DESC_PACKET_FORMAT_MASK) >>
		CONNAC3X_TX_DESC_PACKET_FORMAT_OFFSET),
	pkt_ft_str[((txd->u4DW0 & CONNAC3X_TX_DESC_PACKET_FORMAT_MASK) >>
		CONNAC3X_TX_DESC_PACKET_FORMAT_OFFSET)]);

	/* Q_IDX [31:25]  */
	DBGLOG(HAL, INFO, "\t\tQueID =0x%x\n",
		((txd->u4DW0 & CONNAC3X_TX_DESC_QUEUE_INDEX_MASK) >>
		CONNAC3X_TX_DESC_QUEUE_INDEX_OFFSET));

	DBGLOG(HAL, INFO, "\tTMAC_TXD_1:\n");
	/* DW1 */
	/* MLDIF [11:0] */
	DBGLOG(HAL, INFO, "\t\tMLDID = %d\n",
		((txd->u4DW1 & CONNAC3X_TX_DESC_MLD_ID_MASK) >>
		CONNAC3X_TX_DESC_MLD_ID_OFFSET));

	/* TGID [13:12] */
	DBGLOG(HAL, INFO, "\t\tTGID = %d\n",
		((txd->u4DW1 & CONNAC3X_TX_DESC_TGID_MASK) >>
		CONNAC3X_TX_DESC_TGID_OFFSET));

	/* HF: Header Format [15:14] */
	DBGLOG(HAL, INFO, "\t\tHdrFmt = %d(%s)\n",
	((txd->u4DW1 & CONNAC3X_TX_DESC_HEADER_FORMAT_MASK) >>
		CONNAC3X_TX_DESC_HEADER_FORMAT_OFFSET),
	hdr_fmt_str[((txd->u4DW1 & CONNAC3X_TX_DESC_HEADER_FORMAT_MASK) >>
		CONNAC3X_TX_DESC_HEADER_FORMAT_OFFSET)]);

	switch ((txd->u4DW1 & CONNAC3X_TX_DESC_HEADER_FORMAT_MASK) >>
		CONNAC3X_TX_DESC_HEADER_FORMAT_OFFSET) {
	case TMI_HDR_FT_NON_80211:
		/* MRD [16], EOSP [17], RMVL [18], VLAN [19], ETYPE [20] */
		DBGLOG(HAL, INFO,
		"\t\t\tMRD = %d, EOSP = %d, RMVL = %d, VLAN = %d, ETYP = %d\n",
		(txd->u4DW1 & CONNAC3X_TX_DESC_NON_802_11_MORE_DATA) ? 1 : 0,
		(txd->u4DW1 & CONNAC3X_TX_DESC_NON_802_11_EOSP) ? 1 : 0,
		(txd->u4DW1 & CONNAC3X_TX_DESC_NON_802_11_REMOVE_VLAN) ? 1 : 0,
		(txd->u4DW1 & CONNAC3X_TX_DESC_NON_802_11_VLAN_FIELD) ? 1 : 0,
		(txd->u4DW1 & CONNAC3X_TX_DESC_NON_802_11_ETHERNET_II) ? 1 : 0);
		break;

	case TMI_HDR_FT_NOR_80211:
		/* HEADER_LENGTH [20:16] */
		DBGLOG(HAL, INFO, "\t\t\tHeader Len = %d(WORD)\n",
		((txd->u4DW1 & CONNAC3X_TX_DESC_NOR_802_11_HEADER_LENGTH_MASK)
			>> CONNAC3X_TX_DESC_NOR_802_11_HEADER_LENGTH_OFFSET));
		break;

	case TMI_HDR_FT_ENH_80211:
		/* EOSP [17], AMS [18]	*/
		DBGLOG(HAL, INFO, "\t\t\tEOSP = %d, AMS = %d\n",
		(txd->u4DW1 & CONNAC3X_TX_DESC_ENH_802_11_EOSP) ? 1 : 0,
		(txd->u4DW1 & CONNAC3X_TX_DESC_ENH_802_11_AMSDU) ? 1 : 0);
		break;
	}

	/* TID MGMT TYPE [24:21] */
	DBGLOG(HAL, INFO, "\t\tTID MGMT TYPE = %d\n",
		((txd->u4DW1 & CONNAC3X_TX_DESC_TID_MGMT_TYPE_MASK) >>
		CONNAC3X_TX_DESC_TID_MGMT_TYPE_OFFSET));

	/* OM [30:25] */
	DBGLOG(HAL, INFO, "\t\town_mac = %d\n",
		((txd->u4DW1 & CONNAC3X_TX_DESC_OWN_MAC_MASK) >>
		CONNAC3X_TX_DESC_OWN_MAC_OFFSET));

	/* FR [31] */
	DBGLOG(HAL, INFO, "\t\tFixedRate = %d\n",
		(txd->u4DW1 & CONNAC3X_TX_DESC_FIXED_RATE) ? 1 : 0);

	DBGLOG(HAL, INFO, "\tTMAC_TXD_2:\n");
	/* DW2 */
	/* Subtype [3:0] */
	DBGLOG(HAL, INFO, "\t\tsub_type = %d\n",
		((txd->u4DW2 & CONNAC3X_TX_DESC_SUB_TYPE_MASK) >>
		CONNAC3X_TX_DESC_SUB_TYPE_OFFSET));

	/* Type[5:4] */
	DBGLOG(HAL, INFO, "\t\tfrm_type = %d\n",
		((txd->u4DW2 & CONNAC3X_TX_DESC_TYPE_MASK) >>
		CONNAC3X_TX_DESC_TYPE_OFFSET));

	/* Beamform Type[7:6] */
	DBGLOG(HAL, INFO, "\t\tBeanform_type = %d\n",
		((txd->u4DW2 & CONNAC3X_TX_DESC_BEAMFORM_TYPE_MASK) >>
		CONNAC3X_TX_DESC_BEAMFORM_TYPE_OFFSET));

	/* OM_MAP [8] */
	DBGLOG(HAL, INFO, "\t\tSounding = %d\n",
		((txd->u4DW2 & CONNAC3X_TX_DESC_OM_MAP) ? 1 : 0));

	/* RTS [9] */
	DBGLOG(HAL, INFO, "\t\tRTS = %d\n",
		((txd->u4DW2 & CONNAC3X_TX_DESC_FORCE_RTS_CTS) ? 1 : 0));

	/* Header Padding [11:10] */
	DBGLOG(HAL, INFO, "\t\tHeader_padding = %d\n",
		((txd->u4DW2 & CONNAC3X_TX_DESC_HEADER_PADDING) >>
		CONNAC3X_TX_DESC_HEADER_PADDING_OFFSET));

	/* DU [12] */
	DBGLOG(HAL, INFO, "\t\tDuration = %d\n",
	((txd->u4DW2 & CONNAC3X_TX_DESC_DURATION_FIELD_CONTROL) ? 1 : 0));

	/* HE [13] */
	DBGLOG(HAL, INFO, "\t\tHE(HTC Exist) = %d\n",
		((txd->u4DW2 & CONNAC3X_TX_DESC_HTC_EXISTS) ? 1 : 0));

	/* FRAG [15:14] */
	DBGLOG(HAL, INFO, "\t\tFRAG = %d\n",
		((txd->u4DW2 & CONNAC3X_TX_DESC_FRAGMENT_MASK) >>
		CONNAC3X_TX_DESC_FRAGMENT_OFFSET));

	/* Remaining Life Time [25:16]*/
	DBGLOG(HAL, INFO, "\t\tReamingLife/MaxTx time = %d\n",
		((txd->u4DW2 & CONNAC3X_TX_DESC_REMAINING_MAX_TX_TIME_MASK) >>
		CONNAC3X_TX_DESC_REMAINING_MAX_TX_TIME_OFFSET));

	/* Power Offset [31:26] */
	DBGLOG(HAL, INFO, "\t\tpwr_offset = %d\n",
		((txd->u4DW2 & CONNAC3X_TX_DESC_POWER_OFFSET_MASK) >>
		CONNAC3X_TX_DESC_POWER_OFFSET_OFFSET));

	DBGLOG(HAL, INFO, "\tTMAC_TXD_3:\n");
	/* DW3 */
	/* NA [0] */
	DBGLOG(HAL, INFO, "\t\tNoAck = %d\n",
		(txd->u4DW3 & CONNAC3X_TX_DESC_NO_ACK) ? 1 : 0);

	/* PF [1] */
	DBGLOG(HAL, INFO, "\t\tPF = %d\n",
		(txd->u4DW3 & CONNAC3X_TX_DESC_PROTECTED_FRAME) ? 1 : 0);

	/* EMRD [2] */
	DBGLOG(HAL, INFO, "\t\tEMRD = %d\n",
		(txd->u4DW3 & CONNAC3X_TX_DESC_EXTEND_MORE_DATA) ? 1 : 0);

	/* EEOSP [3] */
	DBGLOG(HAL, INFO, "\t\tEEOSP = %d\n",
		(txd->u4DW3 & CONNAC3X_TX_DESC_EXTEND_EOSP) ? 1 : 0);

	/* BMC [4] */
	DBGLOG(HAL, INFO, "\t\tBMC = %d\n",
		(txd->u4DW3 & CONNAC3X_TX_DESC_BROADCAST_MULTICAST) ? 1 : 0);

	/* HW Amsdu [5] */
	DBGLOG(HAL, INFO, "\t\tHw_amsdu = %d\n",
		(txd->u4DW3 & CONNAC3X_TX_DESC_HW_AMSDU) ? 1 : 0);

	/* TX Count [10:6] */
	DBGLOG(HAL, INFO, "\t\ttx_cnt = %d\n",
		((txd->u4DW3 & CONNAC3X_TX_DESC_TX_COUNT_MASK) >>
		CONNAC3X_TX_DESC_TX_COUNT_OFFSET));

	/* Remaining TX Count [15:11] */
	DBGLOG(HAL, INFO, "\t\tremain_tx_cnt = %d\n",
		((txd->u4DW3 & CONNAC3X_TX_DESC_REMAINING_TX_COUNT_MASK) >>
		CONNAC3X_TX_DESC_REMAINING_TX_COUNT_OFFSET));

	/* SN [27:16] */
	DBGLOG(HAL, INFO, "\t\tsn = %d\n",
		((txd->u4DW3 & CONNAC3X_TX_DESC_SEQUENCE_NUMBER_MASK) >>
		CONNAC3X_TX_DESC_SEQUENCE_NUMBER_MASK_OFFSET));

	/* BA_DIS [28] */
	DBGLOG(HAL, INFO, "\t\tba dis = %d\n",
		(txd->u4DW3 & CONNAC3X_TX_DESC_BA_DISABLE) ? 1 : 0);

	/* Power Management [29] */
	DBGLOG(HAL, INFO, "\t\tpwr_mgmt = 0x%x\n",
	(txd->u4DW3 & CONNAC3X_TX_DESC_POWER_MANAGEMENT_CONTROL) ? 1 : 0);

	/* PN_VLD [30] */
	DBGLOG(HAL, INFO, "\t\tpn_vld = %d\n",
		(txd->u4DW3 & CONNAC3X_TX_DESC_PN_IS_VALID) ? 1 : 0);

	/* SN_VLD [31] */
	DBGLOG(HAL, INFO, "\t\tsn_vld = %d\n",
		(txd->u4DW3 & CONNAC3X_TX_DESC_SN_IS_VALID) ? 1 : 0);

	/* DW4 */
	DBGLOG(HAL, INFO, "\tTMAC_TXD_4:\n");

	/* PN_LOW [31:0] */
	DBGLOG(HAL, INFO, "\t\tpn_low = 0x%x\n", txd->u4PN1);

	/* DW5 */
	DBGLOG(HAL, INFO, "\tTMAC_TXD_5:\n");

	/* PN_HIGH [31:16]  */
	DBGLOG(HAL, INFO, "\t\tpn_high = 0x%x\n", txd->u2PN2);

	/* PID [7:0] */
	DBGLOG(HAL, INFO, "\t\tpid = %d\n",
		(txd->u2DW5_0 & CONNAC3X_TX_DESC_PACKET_ID_MASK) >>
			CONNAC3X_TX_DESC_PACKET_ID_OFFSET);

	/* TXSFM [8] */
	DBGLOG(HAL, INFO, "\t\ttx_status_fmt = %d\n",
		(txd->u2DW5_0 & CONNAC3X_TX_DESC_TX_STATUS_FORMAT) ? 1 : 0);

	/* TXS2M [9] */
	DBGLOG(HAL, INFO, "\t\ttx_status_2_mcu = %d\n",
		(txd->u2DW5_0 & CONNAC3X_TX_DESC_TX_STATUS_TO_MCU) ? 1 : 0);

	/* TXS2H [10] */
	DBGLOG(HAL, INFO, "\t\ttx_status_2_host = %d\n",
		(txd->u2DW5_0 & CONNAC3X_TX_DESC_TX_STATUS_TO_HOST) ? 1 : 0);

	/* Force BSS color to zero [12] */
	DBGLOG(HAL, INFO, "\t\tForce_BSS_Color_2_Zero = %d\n",
		(txd->u2DW5_0 & CONNAC3X_TX_DESC_FORCE_BSS_COLOR_TO_ZERO)
		? 1 : 0);

	/* Bypass RX-based TX blocking check [13] */
	DBGLOG(HAL, INFO, "\t\tBypass_RX_based_TX_blcking_check = %d\n",
		(txd->u2DW5_0 & CONNAC3X_TX_DESC_BYPASS_RX_BASED_TX_BLOCKING)
		? 1 : 0);

	/* Bypass TX-based TX blocking check [14] */
	DBGLOG(HAL, INFO, "\t\tBypass_TX_based_TX_blcking_check = %d\n",
		(txd->u2DW5_0 & CONNAC3X_TX_DESC_BYPASS_TX_BASED_TX_BLOCKING)
		? 1 : 0);

	/* Force Assign Link [15] */
	DBGLOG(HAL, INFO, "\t\tForce_assign_link = %d\n",
		(txd->u2DW5_0 & CONNAC3X_TX_DESC_FORCE_ASSIGN_LINK) ? 1 : 0);

	/* DW6 */
	/* AMSDU CAP UTXB [1] */
	DBGLOG(HAL, INFO, "\t\tAMSDU_CAP_UTXB = %d\n",
		(txd->u4DW6 & CONNAC3X_TX_DESC_AMSDU_CAP_UTXB) ? 1 : 0);

	/* DA Source Selection [2] */
	DBGLOG(HAL, INFO, "\t\tDA_source_selection = %d\n",
		(txd->u4DW6 & CONNAC3X_TX_DESC_DA_SOURCE_SELECTION) ? 1 : 0);

	/* Disable MLD to Link Address Translation [3] */
	DBGLOG(HAL, INFO, "\t\tDIS_MAT = %d\n",
		(txd->u4DW6 & CONNAC3X_TX_DESC_DIS_MAT) ? 1 : 0);

	/* MSDU Count [9:4] */
	DBGLOG(HAL, INFO, "\t\tMSDU_count = %d\n",
		(txd->u4DW6 & CONNAC3X_TX_DESC_MSDU_COUNT_MASK) >>
			CONNAC3X_TX_DESC_MSDU_COUNT_OFFSET);

	/* Timestamp offset index [14:10] */
	DBGLOG(HAL, INFO, "\t\tTimestamp_offset_index = %d\n",
		(txd->u4DW6 & CONNAC3X_TX_DESC_TIMESTAMP_OFFSET_IDX_MASK) >>
			CONNAC3X_TX_DESC_TIMESTAMP_OFFSET_IDX_OFFSET);

	/* Timestamp offset enabled [15] */
	DBGLOG(HAL, INFO, "\t\tTimestamp_offset_enable = %d\n",
		(txd->u4DW6 & CONNAC3X_TX_DESC_TIMESTAMP_OFFSET_ENABLE)
		? 1 : 0);

	/* Fixed Rate Index [21:16] */
	DBGLOG(HAL, INFO, "\t\tFixed_rate_index = %d\n",
		(txd->u4DW6 & CONNAC3X_TX_DESC_FIXED_RATE_INDEX_MASK) >>
			CONNAC3X_TX_DESC_FIXED_RATE_INDEX_OFFSET);

	/* Bandwidth [25:22] */
	DBGLOG(HAL, INFO, "\t\tBandwidth = %d\n",
		(txd->u4DW6 & CONNAC3X_TX_DESC_BANDWIDTH_MASK) >>
			CONNAC3X_TX_DESC_BANDWIDTH_OFFSET);

	/* Valid TXD Arrival Time [28] */
	DBGLOG(HAL, INFO, "\t\tValid_TXD_arrival_time = %d\n",
		(txd->u4DW6 & CONNAC3X_TX_DESC_VALID_TXD_ARRIVAL_TIME)
		? 1 : 0);

	/* TX Packet Source [31:30] */
	DBGLOG(HAL, INFO, "\t\tTX_packet_source = %d\n",
		(txd->u4DW6 & CONNAC3X_TX_DESC_TX_PACKET_SOURCE_MASK) >>
			CONNAC3X_TX_DESC_TX_PACKET_SOURCE_OFFSET);

	/* DW7 */
	DBGLOG(HAL, INFO, "\tTMAC_TXD_7:\n");

	/* SW Predict TX Time [9:0] */
	DBGLOG(HAL, INFO, "\t\tSW_predict_TX_time = %d\n",
		(txd->u4DW7 & CONNAC3X_TX_DESC_SW_PREDICT_TX_TIME_MASK) >>
			CONNAC3X_TX_DESC_SW_PREDICT_TX_TIME_OFFSET);

	/* UT [15] */
	DBGLOG(HAL, INFO, "\t\tUT = %d\n",
		(txd->u4DW7 & CONNAC3X_TX_DESC_UDP_TCP_CHECKSUM_OFFLOAD)
		? 1 : 0);

	/* Aggregate TXD count [25:22] */
	DBGLOG(HAL, INFO, "\t\aggregated_txd_count = %d\n",
		((txd->u4DW7 & CONNAC3X_TX_DESC_AGGREGATED_TXD_COUNT_MASK) >>
		CONNAC3X_TX_DESC_AGGREGATED_TXD_COUNT_OFFSET));

	/* TXD Is Aggregate [26] */
	DBGLOG(HAL, INFO, "\t\tTXD_is_aggregate = %d\n",
		(txd->u4DW7 & CONNAC3X_TX_DESC_THIS_TXD_IS_AGGREGATED) ? 1 : 0);

	/* HM [27] */
	DBGLOG(HAL, INFO, "\t\tHif_or_Mac_TXD_SDO = %d\n",
		(txd->u4DW7 & CONNAC3X_TX_DESC_HIF_OR_MAC_TXD_SDO) ? 1 : 0);

	/* DP [28] */
	DBGLOG(HAL, INFO, "\t\tDrop_By_SDO = %d\n",
		(txd->u4DW7 & CONNAC3X_TX_DESC_DROP_BY_SDO) ? 1 : 0);

	/* I [29]  */
	DBGLOG(HAL, INFO, "\t\ti = %d\n",
		(txd->u4DW7 & CONNAC3X_TX_DESC_IP_CHKSUM_OFFLOAD) ? 1 : 0);

	/* TXDLEN [31:30] */
	DBGLOG(HAL, INFO, "\t\ttxd len= %d\n",
		((txd->u4DW7 & CONNAC3X_TX_DESC_TXD_LENGTH_MASK) >>
		CONNAC3X_TX_DESC_TXD_LENGTH_OFFSET));
}

static void connac3x_event_dump_txd_mem(
	struct ADAPTER *prAdapter,
	struct CMD_INFO *prCmdInfo,
	uint8_t *pucEventBuf)

{
	struct EXT_CMD_EVENT_DUMP_MEM_T *prEventDumpMem;
	uint8_t data[DUMP_MEM_SIZE];
	uint32_t i = 0;

	ASSERT(prAdapter);
	ASSERT(prCmdInfo);
	ASSERT(pucEventBuf);

	kalMemZero(data, sizeof(data));
	prEventDumpMem = (struct EXT_CMD_EVENT_DUMP_MEM_T *)(pucEventBuf);
	kalMemCopy(data, prEventDumpMem->ucData, sizeof(data));
	for (i = 0; i < DUMP_MEM_SIZE; i = i + 4)
		DBGLOG(HAL, INFO, "DW%02d: 0x%02x%02x%02x%02x\n",
		i / 4,
		data[i + 3],
		data[i + 2],
		data[i + 1],
		data[i]
		);
	connac3x_dump_tmac_info(prAdapter, &data[0]);
}

void connac3x_show_txd_Info(
	struct ADAPTER *prAdapter,
	u_int32_t fid)
{
	struct EXT_CMD_EVENT_DUMP_MEM_T CmdMemDump;
	u_int32_t Addr = 0;
	u_int32_t rWlanStatus = WLAN_STATUS_SUCCESS;

	DBGLOG(HAL, INFO, "inShowTXDINFO fid=%d 0x%x\n", fid, fid);

	if (fid >= UMAC_FID_FAULT)
		return;

	Addr = 0xa << 28 | fid << 16; /* TXD addr: 0x{a}{fid}{0000}*/

	kalMemSet(&CmdMemDump, 0, sizeof(struct EXT_CMD_EVENT_DUMP_MEM_T));

	CmdMemDump.u4MemAddr = Addr;
	rWlanStatus = wlanSendSetQueryExtCmd(
		prAdapter,
		CMD_ID_LAYER_0_EXT_MAGIC_NUM, EXT_CMD_ID_DUMP_MEM,
		FALSE, /* Query Bit:  True->write  False->read*/
		TRUE,
		FALSE,
		connac3x_event_dump_txd_mem,
		nicOidCmdTimeoutCommon,
		sizeof(struct EXT_CMD_EVENT_DUMP_MEM_T),
		(u_int8_t *)(&CmdMemDump),
		NULL,
		0);
}

int32_t connac3x_show_rx_rate_info(
		struct ADAPTER *prAdapter,
		char *pcCommand,
		int32_t i4TotalLen,
		uint8_t ucStaIdx)
{
	int32_t i4BytesWritten = 0;
	uint32_t txmode, rate, frmode, sgi, nsts, ldpc, stbc, mu, dcm;

	/* P-RXV0 */
	rate = CONNAC3X_GET_RX_RATE_FROM_ADAPTER(prAdapter, ucStaIdx);
	nsts = CONNAC3X_GET_RX_NSTS_FROM_ADAPTER(prAdapter, ucStaIdx);
	ldpc = CONNAC3X_GET_RX_LDPC_FROM_ADAPTER(prAdapter, ucStaIdx);
	frmode = CONNAC3X_GET_RX_FR_MODE_FROM_ADAPTER(prAdapter, ucStaIdx);
	sgi = CONNAC3X_GET_RX_GI_FROM_ADAPTER(prAdapter, ucStaIdx);
	stbc = CONNAC3X_GET_RX_STBC_ADAPTER(prAdapter, ucStaIdx);
	txmode = CONNAC3X_GET_RX_MODE_ADAPTER(prAdapter, ucStaIdx);
	mu = CONNAC3X_GET_RX_MU_ADAPTER(prAdapter, ucStaIdx);
	dcm = CONNAC3X_GET_RX_DCM_ADAPTER(prAdapter, ucStaIdx);

	if (mu == 0)
		nsts += 1;

	i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
		i4TotalLen - i4BytesWritten, "%-20s%s", "Last RX Rate", " = ");

	if (txmode == TX_RATE_MODE_CCK)
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten, "%s, ",
			rate < 4 ? HW_TX_RATE_CCK_STR[rate] :
			(rate < 8 ? HW_TX_RATE_CCK_STR[rate - 4] :
				HW_TX_RATE_CCK_STR[4]));
	else if (txmode == TX_RATE_MODE_OFDM)
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten, "%s, ",
			nicHwRateOfdmStr(rate));
	else if ((txmode == TX_RATE_MODE_HTMIX) ||
		 (txmode == TX_RATE_MODE_HTGF))
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten, "MCS%d, ", rate);
	else
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten, "%s%d_MCS%d, ",
			stbc == 1 ? "NSTS" : "NSS", nsts, rate);

	i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
		i4TotalLen - i4BytesWritten, "%s, ",
		frmode < 5 ? HW_TX_RATE_BW[frmode] : HW_TX_RATE_BW[5]);

	if (txmode == TX_RATE_MODE_CCK)
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten, "%s, ",
			rate < 4 ? "LP" : "SP");
	else if (txmode == TX_RATE_MODE_OFDM)
		;
	else if (txmode == TX_RATE_MODE_HTMIX ||
		 txmode == TX_RATE_MODE_HTGF ||
		 txmode == TX_RATE_MODE_VHT)
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten, "%s, ",
			sgi == 0 ? "LGI" : "SGI");
	else
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten, "%s, ",
			sgi == 0 ? "SGI" : (sgi == 1 ? "MGI" : "LGI"));

	i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
		i4TotalLen - i4BytesWritten, "%s", stbc == 0 ? "" : "STBC, ");

	i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten, "%s",
			dcm == 0 ? "" : "DCM, ");

	if (mu)
		i4BytesWritten += kalScnprintf(
			pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten, "%s, %s, %s\n",
			txmode < ENUM_TX_MODE_NUM ?
			HW_TX_MODE_STR[txmode] : "N/A",
			ldpc == 0 ? "BCC" : "LDPC", "MU");
	else
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten, "%s, %s\n",
			txmode < ENUM_TX_MODE_NUM ?
			HW_TX_MODE_STR[txmode] : "N/A",
			ldpc == 0 ? "BCC" : "LDPC");

	return i4BytesWritten;
}

int32_t connac3x_show_rx_rssi_info(
		struct ADAPTER *prAdapter,
		char *pcCommand,
		int32_t i4TotalLen,
		uint8_t ucStaIdx)
{
	int32_t i4RSSI0 = 0, i4RSSI1 = 0, i4RSSI2 = 0, i4RSSI3 = 0;
	int32_t i4BytesWritten = 0;

	i4RSSI0 = RCPI_TO_dBm(CONNAC3X_GET_RCPI0_FROM_ADAPTER(
			prAdapter, ucStaIdx));
	i4RSSI1 = RCPI_TO_dBm(CONNAC3X_GET_RCPI1_FROM_ADAPTER(
			prAdapter, ucStaIdx));

	if (prAdapter->rWifiVar.ucNSS > 2) {
		i4RSSI2 = RCPI_TO_dBm(CONNAC3X_GET_RCPI2_FROM_ADAPTER(
				prAdapter, ucStaIdx));
		i4RSSI3 = RCPI_TO_dBm(CONNAC3X_GET_RCPI3_FROM_ADAPTER(
				prAdapter, ucStaIdx));

		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten, "%-20s%s%d %d %d %d\n",
			"Last RX Data RSSI", " = ",
			i4RSSI0, i4RSSI1, i4RSSI2, i4RSSI3);
	} else
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten, "%-20s%s%d %d\n",
			"Last RX Data RSSI", " = ", i4RSSI0, i4RSSI1);

	return i4BytesWritten;
}

int32_t connac3xGetTxRateInfo(char *pcCommand, int i4TotalLen,
			u_int8_t fgDumpAll,
			struct bwtbl_lmac_struct *pWtbl,
			struct PARAM_GET_STA_STATISTICS *prQueryStaStatistics)
{
	uint8_t i, txmode, rate, stbc, sgi, fcap;
	uint8_t nsts;
	int32_t i4BytesWritten = 0;
	uint8_t dcm, ersu106t;
	uint16_t arTxRate[AUTO_RATE_NUM];

	arTxRate[0] = pWtbl->auto_rate_tb.wtbl_d10.field.rate1;
	arTxRate[1] = pWtbl->auto_rate_tb.wtbl_d10.field.rate2;
	arTxRate[2] = pWtbl->auto_rate_tb.wtbl_d11.field.rate3;
	arTxRate[3] = pWtbl->auto_rate_tb.wtbl_d11.field.rate4;
	arTxRate[4] = pWtbl->auto_rate_tb.wtbl_d12.field.rate5;
	arTxRate[5] = pWtbl->auto_rate_tb.wtbl_d12.field.rate6;
	arTxRate[6] = pWtbl->auto_rate_tb.wtbl_d13.field.rate7;
	arTxRate[7] = pWtbl->auto_rate_tb.wtbl_d13.field.rate8;
	fcap = pWtbl->trx_cap.wtbl_d9.field.fcap;

	for (i = 0; i < AUTO_RATE_NUM; i++) {
		txmode = HW_TX_RATE_TO_MODE(arTxRate[i]);
		if (txmode >= ENUM_TX_MODE_NUM)
			txmode = ENUM_TX_MODE_NUM - 1;
		rate = HW_TX_RATE_TO_MCS(arTxRate[i]);
		nsts = HW_TX_RATE_TO_NSS(arTxRate[i]) + 1;
		stbc = HW_TX_RATE_TO_STBC(arTxRate[i]);
		sgi = connac3x_wtbl_get_sgi_info(pWtbl);
		dcm = HW_TX_RATE_TO_DCM(arTxRate[i]);
		ersu106t = HW_TX_RATE_TO_106T(arTxRate[i]);

		if (dcm)
			rate = CONNAC3X_HW_TX_RATE_UNMASK_DCM(rate);
		if (ersu106t)
			rate = CONNAC3X_HW_TX_RATE_UNMASK_106T(rate);

		if (fgDumpAll) {
			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"Rate index[%d]    ", i);

			if (pWtbl->trx_cap.wtbl_d9.field.rate_idx == i) {
				i4BytesWritten += kalScnprintf(
					pcCommand + i4BytesWritten,
					i4TotalLen - i4BytesWritten,
					"%s", "--> ");
			} else {
				i4BytesWritten += kalScnprintf(
					pcCommand + i4BytesWritten,
					i4TotalLen - i4BytesWritten,
					"%s", "    ");
			}
		}

		if (!fgDumpAll) {
			if (pWtbl->trx_cap.wtbl_d9.field.rate_idx != i)
				continue;
			else
				i4BytesWritten += kalScnprintf(
					pcCommand + i4BytesWritten,
					i4TotalLen - i4BytesWritten,
					"%-20s%s", "Auto TX Rate", " = ");
		}

		if (txmode == TX_RATE_MODE_CCK)
			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%s, ",
				HW_TX_RATE_CCK_STR[rate & (uint8_t)0x3]);
		else if (txmode == TX_RATE_MODE_OFDM)
			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%s, ", nicHwRateOfdmStr(rate));
		else if ((txmode == TX_RATE_MODE_HTMIX) ||
			 (txmode == TX_RATE_MODE_HTGF))
			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"MCS%d, ", rate);
		else
			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%s%d_MCS%d, ", stbc ? "NSTS" : "NSS",
				nsts, rate);

		if (prQueryStaStatistics->ucSkipAr) {
			i4BytesWritten += kalScnprintf(
			    pcCommand + i4BytesWritten,
			    i4TotalLen - i4BytesWritten, "%s, ",
			    fcap < 5 ? HW_TX_RATE_BW[fcap]
			      : HW_TX_RATE_BW[5]);
		} else {
			if ((txmode == TX_RATE_MODE_CCK) ||
			    (txmode == TX_RATE_MODE_OFDM))
				i4BytesWritten += kalScnprintf(
				    pcCommand + i4BytesWritten,
				    i4TotalLen - i4BytesWritten,
				    "%s, ", HW_TX_RATE_BW[0]);
			else {
				if (i >
					pWtbl->trx_cap.wtbl_d6.field.cbrn)
					i4BytesWritten += kalScnprintf(
					    pcCommand + i4BytesWritten,
					    i4TotalLen - i4BytesWritten,
					    "%s, ",
					    fcap < 5 ?
					    (fcap > BW_20 ?
						HW_TX_RATE_BW[fcap - 1] :
						HW_TX_RATE_BW[fcap]) :
					    HW_TX_RATE_BW[5]);
				else
					i4BytesWritten += kalScnprintf(
					    pcCommand + i4BytesWritten,
					    i4TotalLen - i4BytesWritten,
					    "%s, ",
					    fcap < 5 ?
					    HW_TX_RATE_BW[fcap] :
					    HW_TX_RATE_BW[5]);
			}
		}

		if (txmode == TX_RATE_MODE_CCK)
			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%s, ", rate < 4 ? "LP" : "SP");
		else if (txmode == TX_RATE_MODE_OFDM)
			;
		else if (
#if CFG_SUPPORT_802_11AX == 1
			 txmode == TX_RATE_MODE_PLR ||
#endif
			 txmode == TX_RATE_MODE_HTMIX ||
			 txmode == TX_RATE_MODE_HTGF ||
			 txmode == TX_RATE_MODE_VHT)
			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%s, ", sgi == 0 ? "LGI" : "SGI");
#if CFG_SUPPORT_802_11AX == 1 || CFG_SUPPORT_802_11BE == 1
		else
			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%s, ", sgi == 0 ? "SGI" :
				(sgi == 1 ? "MGI" : "LGI"));
#endif

		if (prQueryStaStatistics->ucSkipAr) {
			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%s%s%s%s%s\n",
				txmode <= ENUM_TX_MODE_NUM ?
				    HW_TX_MODE_STR[txmode] : "N/A",
				dcm ? ", DCM" : "", ersu106t ? ", 106t" : "",
				stbc ? ", STBC, " : ", ",
				connac3x_wtbl_get_ldpc_info(txmode, pWtbl)
					== 0 ? "BCC" : "LDPC");
		} else {
#if (CFG_SUPPORT_RA_GEN == 0)
			if (prQueryStaStatistics->aucArRatePer[
			    prQueryStaStatistics->aucRateEntryIndex[i]] == 0xFF)
				i4BytesWritten += kalScnprintf(
					pcCommand + i4BytesWritten,
					i4TotalLen - i4BytesWritten,
					"%s%s%s   (--)\n",
					txmode < ENUM_TX_MODE_NUM ?
					    HW_TX_MODE_STR[txmode] : "N/A",
					stbc ? ", STBC, " : ", ",
					((connac3x_wtbl_get_ldpc_info(txmode,
						pWtbl) == 0)
					    || (txmode == TX_RATE_MODE_CCK)
					    || (txmode == TX_RATE_MODE_OFDM)) ?
						"BCC" : "LDPC");
			else
				i4BytesWritten += kalScnprintf(
					pcCommand + i4BytesWritten,
					i4TotalLen - i4BytesWritten,
					"%s%s%s   (%d)\n",
					txmode < ENUM_TX_MODE_NUM ?
					    HW_TX_MODE_STR[txmode] : "N/A",
					stbc ? ", STBC, " : ", ",
					((connac3x_wtbl_get_ldpc_info(txmode,
						pWtbl) == 0)
					    || (txmode == TX_RATE_MODE_CCK)
					    || (txmode == TX_RATE_MODE_OFDM))
						? "BCC" : "LDPC",
					prQueryStaStatistics->aucArRatePer[
					    prQueryStaStatistics
					    ->aucRateEntryIndex[i]]);
#else
			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%s%s%s%s%s\n",
				txmode < ENUM_TX_MODE_NUM ?
				    HW_TX_MODE_STR[txmode] : "N/A",
				dcm ? ", DCM" : "", ersu106t ? ", 106t" : "",
				stbc ? ", STBC, " : ", ",
				((connac3x_wtbl_get_ldpc_info(txmode,
					pWtbl) == 0) ||
				    (txmode == TX_RATE_MODE_CCK) ||
				    (txmode == TX_RATE_MODE_OFDM)) ?
				    "BCC" : "LDPC");
#endif
		}

		if (!fgDumpAll)
			break;
	}

	return i4BytesWritten;
}

int32_t connac3x_get_tx_info_from_txv(char *pcCommand,
	int i4TotalLen,
	struct TX_VECTOR_BBP_LATCH *prTxV)
{
	uint8_t rate, txmode, frmode, sgi, ldpc, nsts, stbc, spe;
	int8_t txpwr, pos_txpwr;
	int32_t i4BytesWritten = 0;
	uint8_t dcm, ersu106t;

	rate = TX_VECTOR_GET_TX_RATE(prTxV);
	txmode = TX_VECTOR_GET_TX_MODE(prTxV);
	frmode = TX_VECTOR_GET_TX_FRMODE(prTxV);
	nsts = TX_VECTOR_GET_TX_NSTS(prTxV) + 1;
	sgi = TX_VECTOR_GET_TX_SGI(prTxV);
	ldpc = TX_VECTOR_GET_TX_LDPC(prTxV);
	stbc = TX_VECTOR_GET_TX_STBC(prTxV);
	txpwr = TX_VECTOR_GET_TX_PWR(prTxV);
	spe = TX_VECTOR_GET_TX_SPE_IDX(prTxV);

	dcm = TX_VECTOR_GET_TX_DCM(prTxV);
	ersu106t = TX_VECTOR_GET_TX_106T(prTxV);

	if (dcm)
		rate = CONNAC3X_TXV_GET_TX_RATE_UNMASK_DCM(rate);
	if (ersu106t)
		rate = CONNAC3X_TXV_GET_TX_RATE_UNMASK_106T(rate);

	if (prTxV->u4TxV[0] == 0xFFFFFFFF) {
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%-20s%s%s\n", "Last TX Rate", " = ", "N/A");
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%-20s%s%s\n", "Chip Out TX Power",
			" = ", "N/A");
	} else {
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%-20s%s", "Last TX Rate", " = ");

		if (txmode == TX_RATE_MODE_CCK)
			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%s, ", rate < 4 ? HW_TX_RATE_CCK_STR[rate] :
					(((rate >= 5) && (rate <= 7)) ?
						HW_TX_RATE_CCK_STR[rate - 4] :
						HW_TX_RATE_CCK_STR[4]));
		else if (txmode == TX_RATE_MODE_OFDM)
			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%s, ", nicHwRateOfdmStr(rate));
		else if ((txmode == TX_RATE_MODE_HTMIX) ||
			 (txmode == TX_RATE_MODE_HTGF))
			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"MCS%d, ", rate);
		else
			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%s%d_MCS%d, ", stbc ? "NSTS" : "NSS",
				nsts, rate);

		i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten, "%s, ",
				frmode < 5 ? HW_TX_RATE_BW[frmode] :
				HW_TX_RATE_BW[5]);

		if (txmode == TX_RATE_MODE_CCK)
			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%s, ", rate < 4 ? "LP" : "SP");
		else if (txmode == TX_RATE_MODE_OFDM)
			;
		else if ((txmode == TX_RATE_MODE_HTMIX) ||
			 (txmode == TX_RATE_MODE_HTGF) ||
			 (txmode == TX_RATE_MODE_VHT) ||
			 (txmode == TX_RATE_MODE_PLR))
			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%s, ", sgi == 0 ? "LGI" : "SGI");
		else
			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%s, ", sgi == 0 ? "SGI" :
				(sgi == 1 ? "MGI" : "LGI"));

		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten, "%s%s%s%s%s%s%d\n",
			(txmode < ENUM_TX_MODE_NUM ?
			HW_TX_MODE_STR[txmode] : "N/A"),
			dcm ? ", DCM" : "", ersu106t ? ", 106t" : "",
			stbc ? ", STBC, " : ", ", ldpc == 0 ? "BCC" : "LDPC",
			", SPE", spe);

		pos_txpwr = (txpwr < 0) ? (~txpwr + 1) : (txpwr);
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%-20s%s%c%d.%1d dBm\n",
			"Chip Out TX Power", " = ",
			(txpwr < 0) ? '-' : '+',
			(pos_txpwr / 2),
			5 * (pos_txpwr % 2));
	}

	return i4BytesWritten;
}

int32_t connac3x_show_stat_info(
		struct ADAPTER *prAdapter,
		char *pcCommand,
		int32_t i4TotalLen,
		struct PARAM_HW_WLAN_INFO *prHwWlanInfo,
		struct PARAM_GET_STA_STATISTICS *prQueryStaStatistics,
		uint8_t fgResetCnt,
		uint32_t u4StatGroup)
{
	int32_t i4BytesWritten = 0;
	int32_t rRssi;
	uint16_t u2LinkSpeed, u2Idx = 0;
	uint32_t u4Per, u4RxPer[ENUM_BAND_NUM], u4TxMpduPer[ENUM_BAND_NUM],
		 u4InstantPer;
	uint8_t ucDbdcIdx, ucSkipAr, ucStaIdx, ucNss;
	static uint32_t u4TotalTxCnt[CFG_STAT_DBG_PEER_NUM] = {0};
	static uint32_t u4TotalFailCnt[CFG_STAT_DBG_PEER_NUM] = {0};
	static uint32_t u4Rate1TxCnt[CFG_STAT_DBG_PEER_NUM] = {0};
	static uint32_t u4Rate1FailCnt[CFG_STAT_DBG_PEER_NUM] = {0};
	static uint32_t au4RxMpduCnt[ENUM_BAND_NUM] = {0};
	static uint32_t au4FcsError[ENUM_BAND_NUM] = {0};
	static uint32_t au4RxFifoCnt[ENUM_BAND_NUM] = {0};
	static uint32_t au4AmpduTxSfCnt[ENUM_BAND_NUM] = {0};
	static uint32_t au4AmpduTxAckSfCnt[ENUM_BAND_NUM] = {0};
	struct RX_CTRL *prRxCtrl;
	uint32_t u4InstantRxPer[ENUM_BAND_NUM];
	uint32_t u4InstantTxMpduPer[ENUM_BAND_NUM];
	uint32_t rStatus = WLAN_STATUS_SUCCESS;
#if CFG_SUPPORT_ADVANCE_CONTROL
	struct PARAM_CUSTOM_SW_CTRL_STRUCT rSwCtrlInfo;
	int16_t i2Wf0AvgPwr = 0, i2Wf1AvgPwr = 0;
#endif
	uint32_t u4BufLen = 0;
	uint8_t ucRaTableNum = sizeof(RATE_TBLE) / sizeof(char *);
	uint8_t ucRaStatusNum = sizeof(RA_STATUS_TBLE) / sizeof(char *);
	uint8_t ucBssIndex, ucBand;
	uint8_t uc256QAMState;
	struct PARAM_LINK_SPEED_EX rLinkSpeed;
	struct CHIP_DBG_OPS *prChipDbg;

	uint8_t *wtbl_raw_dw = NULL;
	struct bwtbl_lmac_struct *pWtbl;
	uint32_t *pau4TxRangeAmpduCnt = NULL;

#if 0
	uint8_t ucRaLtModeNum = sizeof(LT_MODE_TBLE) / sizeof(char *);
	uint8_t ucRaSgiUnSpStateNum = sizeof(SGI_UNSP_STATE_TBLE) /
								sizeof(char *);
	uint8_t ucRaBwStateNum = sizeof(BW_STATE_TBLE) / sizeof(char *);
#endif
	uint16_t au2AggRange[AGG_RANGE_SEL_NUM];
	uint32_t au4RangeCtrl[AGG_RANGE_SEL_4BYTE_NUM];
	enum AGG_RANGE_TYPE_T eRangeType = ENUM_AGG_RANGE_TYPE_TX;

	ucSkipAr = prQueryStaStatistics->ucSkipAr;
	prRxCtrl = &prAdapter->rRxCtrl;
	ucNss = prAdapter->rWifiVar.ucNSS;

	if (ucSkipAr) {
		u2Idx = nicGetStatIdxInfo(prAdapter,
			(uint8_t)(prHwWlanInfo->u4Index));

		if (u2Idx == 0xFFFF)
			return i4BytesWritten;
	}

	wtbl_raw_dw = (uint8_t *)kalMemAlloc(
		sizeof(struct bwtbl_lmac_struct), VIR_MEM_TYPE);
	if (!wtbl_raw_dw) {
		DBGLOG(REQ, ERROR, "WTBL : Memory alloc failed\n");
		return 0;
	}

	/* Read LWTBL Entries */
	connac3x_get_lwtbl(prAdapter,
		prHwWlanInfo->u4Index, wtbl_raw_dw);
	pWtbl = (struct bwtbl_lmac_struct *)wtbl_raw_dw;
	ucBand = pWtbl->peer_basic_info.wtbl_d0.field.band;
	pau4TxRangeAmpduCnt = ((ucBand < ENUM_BAND_NUM) ?
			g_arMibInfo[ucBand].au4TxRangeAmpduCnt : NULL);

	if (ucSkipAr) {
		u4TotalTxCnt[u2Idx] += prQueryStaStatistics->u4TransmitCount;
		u4TotalFailCnt[u2Idx] += prQueryStaStatistics->
							u4TransmitFailCount;
		u4Rate1TxCnt[u2Idx] += prQueryStaStatistics->u4Rate1TxCnt;
		u4Rate1FailCnt[u2Idx] += prQueryStaStatistics->u4Rate1FailCnt;
	}

	if (ucSkipAr) {
		u4Per = (u4Rate1TxCnt[u2Idx] == 0) ?
			(0) : (1000 * (u4Rate1FailCnt[u2Idx]) /
			(u4Rate1TxCnt[u2Idx]));

		u4InstantPer = (prQueryStaStatistics->u4Rate1TxCnt == 0) ?
			(0) : (1000 * (prQueryStaStatistics->u4Rate1FailCnt) /
				(prQueryStaStatistics->u4Rate1TxCnt));
	} else {
		u4Per = (prQueryStaStatistics->u4Rate1TxCnt == 0) ?
			(0) : (1000 * (prQueryStaStatistics->u4Rate1FailCnt) /
				(prQueryStaStatistics->u4Rate1TxCnt));

		u4InstantPer = (prQueryStaStatistics->ucPer == 0) ?
			(0) : (prQueryStaStatistics->ucPer);
	}

	for (ucDbdcIdx = 0; ucDbdcIdx < ENUM_BAND_NUM; ucDbdcIdx++) {
		au4RxMpduCnt[ucDbdcIdx] += g_arMibInfo[ucDbdcIdx].u4RxMpduCnt;
		au4FcsError[ucDbdcIdx] += g_arMibInfo[ucDbdcIdx].u4FcsError;
		au4RxFifoCnt[ucDbdcIdx] += g_arMibInfo[ucDbdcIdx].u4RxFifoFull;
		au4AmpduTxSfCnt[ucDbdcIdx] +=
			g_arMibInfo[ucDbdcIdx].u4AmpduTxSfCnt;
		au4AmpduTxAckSfCnt[ucDbdcIdx] +=
			g_arMibInfo[ucDbdcIdx].u4AmpduTxAckSfCnt;

		u4RxPer[ucDbdcIdx] =
		    ((au4RxMpduCnt[ucDbdcIdx] + au4FcsError[ucDbdcIdx]) == 0) ?
				(0) : (1000 * au4FcsError[ucDbdcIdx] /
					(au4RxMpduCnt[ucDbdcIdx] +
					au4FcsError[ucDbdcIdx]));

		u4TxMpduPer[ucDbdcIdx] =
		    (au4AmpduTxSfCnt[ucDbdcIdx] == 0) ?
			(0) : (1000 * (au4AmpduTxSfCnt[ucDbdcIdx] -
				au4AmpduTxAckSfCnt[ucDbdcIdx]) /
				au4AmpduTxSfCnt[ucDbdcIdx]);

		u4InstantRxPer[ucDbdcIdx] =
			((prQueryStaStatistics->rMibInfo[ucDbdcIdx].u4RxMpduCnt
			+ prQueryStaStatistics->rMibInfo[ucDbdcIdx].u4FcsError)
			== 0) ?
				(0) : (1000 * prQueryStaStatistics->
				rMibInfo[ucDbdcIdx].u4FcsError /
				(prQueryStaStatistics->rMibInfo[ucDbdcIdx].
				u4RxMpduCnt +
				prQueryStaStatistics->rMibInfo[ucDbdcIdx].
				u4FcsError));
		u4InstantTxMpduPer[ucDbdcIdx] =
			(prQueryStaStatistics->rMibInfo[ucDbdcIdx].
			u4AmpduTxSfCnt == 0) ?
				(0) : (1000 *
				(prQueryStaStatistics->rMibInfo[ucDbdcIdx].
				u4AmpduTxSfCnt -
				prQueryStaStatistics->rMibInfo[ucDbdcIdx].
				u4AmpduTxAckSfCnt) /
				prQueryStaStatistics->rMibInfo[ucDbdcIdx].
				u4AmpduTxSfCnt);
	}

	rRssi = RCPI_TO_dBm(prQueryStaStatistics->ucRcpi);
	u2LinkSpeed = (prQueryStaStatistics->u2LinkSpeed == 0) ? 0 :
					prQueryStaStatistics->u2LinkSpeed / 2;

	if (ucSkipAr) {
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%-20s%s%d(%d)\n", "\nWlanIdx(BackupIdx)", "  = ",
			prHwWlanInfo->u4Index, u2Idx);
	} else {
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%-20s%s%d\n", "\nWlanIdx", "  = ",
			prHwWlanInfo->u4Index);
	}

	if (ucBand >= ENUM_BAND_NUM)
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%-20s%s%d%s\n", "BandIdx", "  = ",
			ucBand, " Mib Count Invalid");

	/* =========== Group 0x0001 =========== */
	if (u4StatGroup & 0x0001) {
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%s", "----- STA Stat (Group 0x01) -----\n");

		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%-20s%s%u\n", "CurrTemperature", " = ",
			prQueryStaStatistics->ucTemperature);

		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%-20s%s%u\n", "Tx Total cnt", " = ",
			ucSkipAr ? (u4TotalTxCnt[u2Idx]) :
				(prQueryStaStatistics->u4TransmitCount));

		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%-20s%s%u\n", "Tx Fail Cnt", " = ",
			ucSkipAr ? (u4TotalFailCnt[u2Idx]) :
				(prQueryStaStatistics->u4TransmitFailCount));

		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%-20s%s%u\n", "Rate1 Tx Cnt", " = ",
			ucSkipAr ? (u4Rate1TxCnt[u2Idx]) :
				(prQueryStaStatistics->u4Rate1TxCnt));

		if (ucSkipAr)
			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%-20s%s%u, PER = %d.%1d%%, instant PER = %d.%1d%%\n",
				"Rate1 Fail Cnt", " = ",
				u4Rate1FailCnt[u2Idx], u4Per/10, u4Per%10,
				u4InstantPer/10, u4InstantPer%10);
		else
			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%-20s%s%u, PER = %d.%1d%%, instant PER = %d%%\n",
				"Rate1 Fail Cnt", " = ",
				prQueryStaStatistics->u4Rate1FailCnt,
				u4Per/10, u4Per%10, u4InstantPer);

		if ((ucSkipAr) && (fgResetCnt)) {
			u4TotalTxCnt[u2Idx] = 0;
			u4TotalFailCnt[u2Idx] = 0;
			u4Rate1TxCnt[u2Idx] = 0;
			u4Rate1FailCnt[u2Idx] = 0;
		}
	}

	/* =========== Group 0x0002 =========== */
	if (u4StatGroup & 0x0002) {
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
					       i4TotalLen - i4BytesWritten,
			"%s", "----- MIB Info (Group 0x02) -----\n");
#if 0
		if (prAdapter->rWifiVar.fgDbDcModeEn)
			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"[DBDC_%d] :\n", ucBand);
#endif
		if (ucBand < ENUM_BAND_NUM) {
			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%-20s%s%d\n", "RX Success", " = ",
				au4RxMpduCnt[ucBand]);

			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%-20s%s%d, PER = %d.%1d%%, instant PER = %d.%1d%%\n",
				"RX with CRC", " = ", au4FcsError[ucBand],
				u4RxPer[ucBand]/10, u4RxPer[ucBand]%10,
				u4InstantRxPer[ucBand]/10,
				u4InstantRxPer[ucBand]%10);

			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%-20s%s%d\n", "RX drop FIFO full", " = ",
				au4RxFifoCnt[ucBand]);
		} else {
			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%-20s%s%d\n", "RX Success", " = ", 0);

			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%-20s%s%d, PER = %d.%1d%%, instant PER = %d.%1d%%\n",
				"RX with CRC", " = ", 0, 0, 0, 0, 0);

			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%-20s%s%d\n", "RX drop FIFO full", " = ", 0);
		}

		if (fgResetCnt) {
			kalMemZero(au4RxMpduCnt, sizeof(au4RxMpduCnt));
			kalMemZero(au4FcsError, sizeof(au4RxMpduCnt));
			kalMemZero(au4RxFifoCnt, sizeof(au4RxMpduCnt));
			kalMemZero(au4AmpduTxSfCnt, sizeof(au4RxMpduCnt));
			kalMemZero(au4AmpduTxAckSfCnt, sizeof(au4RxMpduCnt));
		}
	}

	/* =========== Group 0x0004 =========== */
	if (u4StatGroup & 0x0004) {
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
					       i4TotalLen - i4BytesWritten,
			"%s", "----- Last Rx Info (Group 0x04) -----\n");

		/* get Beacon RSSI */
		ucBssIndex = secGetBssIdxByWlanIdx
			(prAdapter, (uint8_t)(prHwWlanInfo->u4Index));

		rStatus = kalIoctlByBssIdx(prAdapter->prGlueInfo,
				   wlanoidQueryRssi,
				   &rLinkSpeed, sizeof(rLinkSpeed),
				   &u4BufLen, ucBssIndex);

		if (rStatus != WLAN_STATUS_SUCCESS)
			DBGLOG(REQ, WARN, "unable to retrieve rssi\n");

		if (IS_BSS_INDEX_VALID(ucBssIndex))
			rRssi = rLinkSpeed.rLq[ucBssIndex].cRssi;

#if CFG_SUPPORT_ADVANCE_CONTROL
		rSwCtrlInfo.u4Data = 0;
		rSwCtrlInfo.u4Id = CMD_SW_DBGCTL_ADVCTL_GET_ID + 1;
#if 0
		rStatus = kalIoctl(prAdapter->prGlueInfo,
				   wlanoidQuerySwCtrlRead, &rSwCtrlInfo,
				   sizeof(rSwCtrlInfo), &u4BufLen);
#endif
		DBGLOG(REQ, LOUD, "rStatus %u, rSwCtrlInfo.u4Data 0x%x\n",
		       rStatus, rSwCtrlInfo.u4Data);
		if (rStatus == WLAN_STATUS_SUCCESS) {
			i2Wf0AvgPwr = rSwCtrlInfo.u4Data & 0xFFFF;
			i2Wf1AvgPwr = (rSwCtrlInfo.u4Data >> 16) & 0xFFFF;

			i4BytesWritten += kalScnprintf(
					pcCommand + i4BytesWritten,
					i4TotalLen - i4BytesWritten,
					"%-20s%s%d %d\n", "NOISE", " = ",
					i2Wf0AvgPwr, i2Wf1AvgPwr);
		}
#endif

#ifndef SOC3_0
		/* Last RX Rate */
		i4BytesWritten += nicGetRxRateInfo(prAdapter,
			pcCommand + i4BytesWritten, i4TotalLen - i4BytesWritten,
			(uint8_t)(prHwWlanInfo->u4Index));
#endif
		/* Last RX RSSI */
		i4BytesWritten += nicRxGetLastRxRssi(prAdapter,
			pcCommand + i4BytesWritten, i4TotalLen - i4BytesWritten,
			(uint8_t)(prHwWlanInfo->u4Index));

		/* Last TX Resp RSSI */
		if (ucNss > 2)
			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%-20s%s%d %d %d %d\n",
				"Tx Response RSSI", " = ",
				RCPI_TO_dBm(
				    pWtbl->rx_stat.wtbl_d34.field.resp_rcpi_0),
				RCPI_TO_dBm(
				    pWtbl->rx_stat.wtbl_d34.field.resp_rcpi_1),
				RCPI_TO_dBm(
				    pWtbl->rx_stat.wtbl_d34.field.resp_rcpi_2),
				RCPI_TO_dBm(
				    pWtbl->rx_stat.wtbl_d34.field.resp_rcpi_3));
		else
			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%-20s%s%d %d\n", "Tx Response RSSI", " = ",
				RCPI_TO_dBm(
				    pWtbl->rx_stat.wtbl_d34.field.resp_rcpi_0),
				RCPI_TO_dBm(
				    pWtbl->rx_stat.wtbl_d34.field.resp_rcpi_1));

		/* Last Beacon RSSI */
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%-20s%s%d\n", "Beacon RSSI", " = ", rRssi);
	}

	/* =========== Group 0x0008 =========== */
	if (u4StatGroup & 0x0008) {
		/* TxV */
		i4BytesWritten += kalScnprintf(
			pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%s", "----- Last TX Info (Group 0x08) -----\n");
#if 0
		i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"[DBDC_%d] :\n",
				ucBand);
#endif
		prChipDbg = prAdapter->chip_info->prDebugOps;
		if (prChipDbg && prChipDbg->get_tx_info_from_txv
			&& ucBand < ENUM_BAND_NUM) {
			i4BytesWritten += (
				prChipDbg->get_tx_info_from_txv(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				&prQueryStaStatistics->rTxVector[ucBand]));
		}
	}

	/* =========== Group 0x0010 =========== */
	if (u4StatGroup & 0x0010) {
		/* RX Reorder */
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%s", "----- RX Reorder (Group 0x10) -----\n");
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%-20s%s%lu\n", "Rx reorder miss", " = ",
			RX_GET_CNT(prRxCtrl, RX_DATA_REORDER_MISS_COUNT));
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%-20s%s%lu\n", "Rx reorder within", " = ",
			RX_GET_CNT(prRxCtrl, RX_DATA_REORDER_WITHIN_COUNT));
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%-20s%s%lu\n", "Rx reorder ahead", " = ",
			RX_GET_CNT(prRxCtrl, RX_DATA_REORDER_AHEAD_COUNT));
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%-20s%s%lu\n", "Rx reorder behind", " = ",
			RX_GET_CNT(prRxCtrl, RX_DATA_REORDER_BEHIND_COUNT));
	}

	/* =========== Group 0x0020 =========== */
	if (u4StatGroup & 0x0020) {
		/* RA info */
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%s", "----- RA Info (Group 0x20) -----\n");
#if 0
		/* Last TX Rate */
		i4BytesWritten += nicGetTxRateInfo(
			pcCommand + i4BytesWritten, i4TotalLen - i4BytesWritten,
			FALSE, prHwWlanInfo, prQueryStaStatistics);
#endif
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten, "%-20s%s%d\n", "LinkSpeed",
			" = ", u2LinkSpeed);

		uc256QAMState = (
			prQueryStaStatistics->ucDynamicGband256QAMState);
		if (!prQueryStaStatistics->ucSkipAr) {
			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%-20s%s%s\n", "RateTable", " = ",
				prQueryStaStatistics->ucArTableIdx <
				    (ucRaTableNum - 1) ?
				    RATE_TBLE[
					prQueryStaStatistics->ucArTableIdx] :
					RATE_TBLE[ucRaTableNum - 1]);

			if (wlanGetStaIdxByWlanIdx(prAdapter,
				(uint8_t)(prHwWlanInfo->u4Index), &ucStaIdx) ==
				WLAN_STATUS_SUCCESS &&
				ucStaIdx < CFG_STA_REC_NUM){
				i4BytesWritten += kalScnprintf(
					pcCommand + i4BytesWritten,
					i4TotalLen - i4BytesWritten,
					"%-20s%s%d\n", "2G Support 256QAM TX",
					" = ",
					((prAdapter->arStaRec[ucStaIdx].u4Flags
					& MTK_SYNERGY_CAP_SUPPORT_24G_MCS89)
					|| (uc256QAMState == 2)) ? 1 : 0);
			}
#if 0
			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%-20s%s%d%%\n", "Rate1 instantPer", " = ",
				u4InstantPer);
#endif
			if (prQueryStaStatistics->ucAvePer == 0xFF) {
				i4BytesWritten += kalScnprintf(
					pcCommand + i4BytesWritten,
					i4TotalLen - i4BytesWritten,
					"%-20s%s%s\n", "Train Down", " = ",
					"N/A");

				i4BytesWritten += kalScnprintf(
					pcCommand + i4BytesWritten,
					i4TotalLen - i4BytesWritten,
					"%-20s%s%s\n", "Train Up", " = ",
					"N/A");
			} else {
				i4BytesWritten += kalScnprintf(
					pcCommand + i4BytesWritten,
					i4TotalLen - i4BytesWritten,
					"%-20s%s%d -> %d\n", "Train Down",
					" = ",
					(uint16_t)
					(prQueryStaStatistics->u2TrainDown
						& BITS(0, 7)),
					(uint16_t)
					((prQueryStaStatistics->u2TrainDown >>
						8) & BITS(0, 7)));

				i4BytesWritten += kalScnprintf(
					pcCommand + i4BytesWritten,
					i4TotalLen - i4BytesWritten,
					"%-20s%s%d -> %d\n", "Train Up", " = ",
					(uint16_t)
					(prQueryStaStatistics->u2TrainUp
						& BITS(0, 7)),
					(uint16_t)
					((prQueryStaStatistics->u2TrainUp >> 8)
						& BITS(0, 7)));
			}

			if (prQueryStaStatistics->fgIsForceTxStream == 0)
				i4BytesWritten += kalScnprintf(
					pcCommand + i4BytesWritten,
					i4TotalLen - i4BytesWritten,
					"%-20s%s%s\n", "Force Tx Stream",
					" = ", "N/A");
			else
				i4BytesWritten += kalScnprintf(
					pcCommand + i4BytesWritten,
					i4TotalLen - i4BytesWritten,
					"%-20s%s%d\n", "Force Tx Stream", " = ",
					prQueryStaStatistics->
						fgIsForceTxStream);

			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%-20s%s%d\n", "Force SE off", " = ",
				prQueryStaStatistics->fgIsForceSeOff);
#if 0
			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%-20s%s%s%d%s%d%s%d%s%d\n", "TxQuality", " = ",
				"KEEP_", prQueryStaStatistics->aucTxQuality[0],
				", UP_", prQueryStaStatistics->aucTxQuality[1],
				", DOWN_", prQueryStaStatistics->
					aucTxQuality[2],
				", BWUP_", prQueryStaStatistics->
					aucTxQuality[3]);

			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%-20s%s%d\n", "UpPenalty", " = ",
				prQueryStaStatistics->ucTxRateUpPenalty);

			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%-20s%s%s\n", "LtMode", " = ",
				prQueryStaStatistics->ucLowTrafficMode <
				(ucRaLtModeNum - 1) ?
				LT_MODE_TBLE[prQueryStaStatistics->
				ucLowTrafficMode] :
				LT_MODE_TBLE[ucRaLtModeNum - 1]);

			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%-20s%s%d\n", "LtCnt", " = ",
				prQueryStaStatistics->ucLowTrafficCount);

			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%-20s%s%d\n", "LtDashBoard", " = ",
				prQueryStaStatistics->ucLowTrafficDashBoard);

			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%-20s%s%s\n", "SgiState", " = ",
				prQueryStaStatistics->ucDynamicSGIState <
				(ucRaSgiUnSpStateNum - 1) ?
				SGI_UNSP_STATE_TBLE[prQueryStaStatistics->
				ucDynamicSGIState] :
				SGI_UNSP_STATE_TBLE[ucRaSgiUnSpStateNum - 1]);

			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%-20s%s%d\n", "SgiScore", " = ",
				prQueryStaStatistics->ucDynamicSGIScore);

			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%-20s%s%s\n", "BwState", " = ",
				prQueryStaStatistics->ucDynamicBWState <
				(ucRaBwStateNum - 1) ?
				BW_STATE_TBLE[prQueryStaStatistics->
				ucDynamicBWState] :
				BW_STATE_TBLE[ucRaBwStateNum - 1]);

			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%-20s%s%s\n", "NonSpState", " = ",
				prQueryStaStatistics->ucDynamicSGIState <
				(ucRaSgiUnSpStateNum - 1) ?
				SGI_UNSP_STATE_TBLE[prQueryStaStatistics->
				ucVhtNonSpRateState] :
				SGI_UNSP_STATE_TBLE[ucRaSgiUnSpStateNum - 1]);
#endif
		}

		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%-20s%s%d\n", "RunningCnt", " = ",
			prQueryStaStatistics->u2RaRunningCnt);

		prQueryStaStatistics->ucRaStatus &= ~0x80;
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%-20s%s%s\n", "Status", " = ",
			prQueryStaStatistics->ucRaStatus < (ucRaStatusNum - 1) ?
			RA_STATUS_TBLE[prQueryStaStatistics->ucRaStatus] :
			RA_STATUS_TBLE[ucRaStatusNum - 1]);

		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%-20s%s%d\n", "MaxAF", " = ",
			pWtbl->trx_cap.wtbl_d5.field.af);

		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%-20s%s0x%x\n", "SpeIdx", " = ",
			pWtbl->trx_cap.wtbl_d6.field.spe_idx);
#if 0
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%-20s%s%d\n", "CBRN", " = ",
			prHwWlanInfo->rWtblPeerCap.ucChangeBWAfterRateN);
#endif
		/* Rate1~Rate8 */
		i4BytesWritten += connac3xGetTxRateInfo(
			pcCommand + i4BytesWritten, i4TotalLen - i4BytesWritten,
			TRUE, pWtbl, prQueryStaStatistics);
	}

	/* =========== Group 0x0040 =========== */
	if (u4StatGroup & 0x0040) {
		uint8_t ucIdx;

		au4RangeCtrl[0] = prQueryStaStatistics->u4AggRangeCtrl_0;
		au4RangeCtrl[1] = prQueryStaStatistics->u4AggRangeCtrl_1;
		au4RangeCtrl[2] = prQueryStaStatistics->u4AggRangeCtrl_2;
		au4RangeCtrl[3] = prQueryStaStatistics->u4AggRangeCtrl_3;
		au4RangeCtrl[4] = prQueryStaStatistics->u4AggRangeCtrl_4;
		au4RangeCtrl[5] = prQueryStaStatistics->u4AggRangeCtrl_5;
		au4RangeCtrl[6] = prQueryStaStatistics->u4AggRangeCtrl_6;
		au4RangeCtrl[7] = prQueryStaStatistics->u4AggRangeCtrl_7;

		eRangeType = (enum AGG_RANGE_TYPE_T)
					prQueryStaStatistics->ucRangeType;

		for (ucIdx = 0; ucIdx < AGG_RANGE_SEL_NUM; ucIdx++) {
			if (ucIdx % 2 == 0)
				au2AggRange[ucIdx] =
					((au4RangeCtrl[ucIdx >> 1] &
					AGG_RANGE_SEL_0_MASK) >>
					AGG_RANGE_SEL_0_OFFSET);
			else
				au2AggRange[ucIdx] =
					((au4RangeCtrl[ucIdx >> 1] &
					AGG_RANGE_SEL_1_MASK) >>
					AGG_RANGE_SEL_1_OFFSET);
		}

		/* Tx Agg */
		i4BytesWritten += kalScnprintf(
			pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%s%s%s", "------ ",
			(eRangeType > ENUM_AGG_RANGE_TYPE_TX) ? (
				(eRangeType == ENUM_AGG_RANGE_TYPE_TRX) ?
				("TRX") : ("RX")) : ("TX"),
				" AGG (Group 0x40) -----\n");

		if (eRangeType == ENUM_AGG_RANGE_TYPE_TRX) {
			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%-6s%8d%5d%1s%2d%5d%1s%2d%5d%1s%2d%5d%1s%2d%5d%1s%2d%5d%1s%2d%5d%3s",
				" TX  :", au2AggRange[0] + 1,
				au2AggRange[0] + 2, "~", au2AggRange[1] + 1,
				au2AggRange[1] + 2, "~", au2AggRange[2] + 1,
				au2AggRange[2] + 2, "~", au2AggRange[3] + 1,
				au2AggRange[3] + 2, "~", au2AggRange[4] + 1,
				au2AggRange[4] + 2, "~", au2AggRange[5] + 1,
				au2AggRange[5] + 2, "~", au2AggRange[6] + 1,
				au2AggRange[6] + 2, "~1024\n");

			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"DBDC%d:", ucBand);
			if (pau4TxRangeAmpduCnt)
				i4BytesWritten += kalScnprintf(
					pcCommand + i4BytesWritten,
					i4TotalLen - i4BytesWritten,
					"%8u%8u%8u%8u%8u%8u%8u%8u\n",
					pau4TxRangeAmpduCnt[0],
					pau4TxRangeAmpduCnt[1],
					pau4TxRangeAmpduCnt[2],
					pau4TxRangeAmpduCnt[3],
					pau4TxRangeAmpduCnt[4],
					pau4TxRangeAmpduCnt[5],
					pau4TxRangeAmpduCnt[6],
					pau4TxRangeAmpduCnt[7]);
			else
				i4BytesWritten += kalScnprintf(
					pcCommand + i4BytesWritten,
					i4TotalLen - i4BytesWritten,
					"%8u%8u%8u%8u%8u%8u%8u%8u\n",
					0, 0, 0, 0, 0, 0, 0, 0);

			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%-6s%8d%5d%1s%2d%5d%1s%2d%5d%1s%2d%5d%1s%2d%5d%1s%2d%5d%1s%2d%5d%3s",
				" RX  :", au2AggRange[7] + 1,
				au2AggRange[7] + 2, "~", au2AggRange[8] + 1,
				au2AggRange[8] + 2, "~", au2AggRange[9] + 1,
				au2AggRange[9] + 2, "~", au2AggRange[10] + 1,
				au2AggRange[10] + 2, "~", au2AggRange[11] + 1,
				au2AggRange[11] + 2, "~", au2AggRange[12] + 1,
				au2AggRange[12] + 2, "~", au2AggRange[13] + 1,
				au2AggRange[13] + 2, "~1024\n");

			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"DBDC%d:", ucBand);
			if (pau4TxRangeAmpduCnt)
				i4BytesWritten += kalScnprintf(
					pcCommand + i4BytesWritten,
					i4TotalLen - i4BytesWritten,
					"%8u%8u%8u%8u%8u%8u%8u%8u\n",
					pau4TxRangeAmpduCnt[8],
					pau4TxRangeAmpduCnt[9],
					pau4TxRangeAmpduCnt[10],
					pau4TxRangeAmpduCnt[11],
					pau4TxRangeAmpduCnt[12],
					pau4TxRangeAmpduCnt[13],
					pau4TxRangeAmpduCnt[14],
					pau4TxRangeAmpduCnt[15]);
			else
				i4BytesWritten += kalScnprintf(
					pcCommand + i4BytesWritten,
					i4TotalLen - i4BytesWritten,
					"%8u%8u%8u%8u%8u%8u%8u%8u\n",
					0, 0, 0, 0, 0, 0, 0, 0);
		} else {
			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%-6s%8d%6d%1s%2d%6d%1s%2d%6d%1s%2d%5d%1s%3d%5d%1s%3d%5d%1s%3d%5d%1s%3d\n",
				"Range:", au2AggRange[0] + 1,
				au2AggRange[0] + 2, "~",
				au2AggRange[1] + 1,
				au2AggRange[1] + 2, "~",
				au2AggRange[2] + 1,
				au2AggRange[2] + 2, "~",
				au2AggRange[3] + 1,
				au2AggRange[3] + 2, "~",
				au2AggRange[4] + 1,
				au2AggRange[4] + 2, "~",
				au2AggRange[5] + 1,
				au2AggRange[5] + 2, "~",
				au2AggRange[6] + 1,
				au2AggRange[6] + 2, "~",
				au2AggRange[7] + 1);

			if (pau4TxRangeAmpduCnt)
				i4BytesWritten += kalScnprintf(
					pcCommand + i4BytesWritten,
					i4TotalLen - i4BytesWritten,
					"DBDC%d%9u%9u%9u%9u%9u%9u%9u%9u\n",
					ucBand,
					pau4TxRangeAmpduCnt[0],
					pau4TxRangeAmpduCnt[1],
					pau4TxRangeAmpduCnt[2],
					pau4TxRangeAmpduCnt[3],
					pau4TxRangeAmpduCnt[4],
					pau4TxRangeAmpduCnt[5],
					pau4TxRangeAmpduCnt[6],
					pau4TxRangeAmpduCnt[7]);
			else
				i4BytesWritten += kalScnprintf(
					pcCommand + i4BytesWritten,
					i4TotalLen - i4BytesWritten,
					"DBDC%d%9u%9u%9u%9u%9u%9u%9u%9u\n",
					ucBand, 0, 0, 0, 0, 0, 0, 0, 0);

			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%-6s%4d%1s%3d%5d%1s%3d%5d%1s%3d%5d%1s%3d%5d%1s%3d%5d%1s%3d%5d%1s%3d%5d%5s",
				"Range:", au2AggRange[7] + 2, "~",
				au2AggRange[8] + 1,
				au2AggRange[8] + 2, "~",
				au2AggRange[9] + 1,
				au2AggRange[9] + 2, "~",
				au2AggRange[10] + 1,
				au2AggRange[10] + 2, "~",
				au2AggRange[11] + 1,
				au2AggRange[11] + 2, "~",
				au2AggRange[12] + 1,
				au2AggRange[12] + 2, "~",
				au2AggRange[13] + 1,
				au2AggRange[13] + 2, "~",
				au2AggRange[14] + 1,
				au2AggRange[14] + 2, "~1024\n");

			if (pau4TxRangeAmpduCnt)
				i4BytesWritten += kalScnprintf(
					pcCommand + i4BytesWritten,
					i4TotalLen - i4BytesWritten,
					"DBDC%d%9u%9u%9u%9u%9u%9u%9u%9u\n",
					ucBand,
					pau4TxRangeAmpduCnt[8],
					pau4TxRangeAmpduCnt[9],
					pau4TxRangeAmpduCnt[10],
					pau4TxRangeAmpduCnt[11],
					pau4TxRangeAmpduCnt[12],
					pau4TxRangeAmpduCnt[13],
					pau4TxRangeAmpduCnt[14],
					pau4TxRangeAmpduCnt[15]);
			else
				i4BytesWritten += kalScnprintf(
					pcCommand + i4BytesWritten,
					i4TotalLen - i4BytesWritten,
					"DBDC%d%9u%9u%9u%9u%9u%9u%9u%9u\n",
					ucBand, 0, 0, 0, 0, 0, 0, 0, 0);
		}
	}

	kalMemZero(g_arMibInfo, sizeof(g_arMibInfo));
	kalMemFree(wtbl_raw_dw, VIR_MEM_TYPE,
			sizeof(struct bwtbl_lmac_struct));

	return i4BytesWritten;
}

#if (CFG_SUPPORT_802_11BE_MLO == 1)
int32_t connac3x_show_mld_info(
		struct ADAPTER *prAdapter,
		char *pcCommand,
		int32_t i4TotalLen,
		struct PARAM_MLD_REC *mld)
{
	int32_t i4BytesWritten = 0;
	struct MLD_RECORD_LINK *prMldLink;
	struct MLO_AGC_DISP_PARAM_TX *prAgcParam;
	struct MLO_AGC_DISP_PARAM_TRIG *prAgcParamTrig;
	struct MLO_OVLP_RPT_CNT *prOvlpRptCnt;
	struct MLO_OVLP_RPT_CNT *prOvlpRptCntTrig;
	uint8_t i, j;

	i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"\n%s%d, %s%d, %s%d, %s%d/%d\n",
			"MldRecState=", mld->u1MldRecState,
			"MldRecIdx=", mld->u1MldRecIdx,
			"StaRecMldIdx=", mld->u2MldIdx,
			"MldId[P/S]=", mld->u2PrimaryMldId, mld->u2SecondMldId);

	i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%s%d, %s0x%x, %s0x%x\n%s%d(0x%x), %s%d/%d\n",
			"AllSTR=", mld->fgAllStrLinks,
			"StrBmp=", mld->u1StrBmp,
			"EmlsrBmp=", mld->u1EmlsrBmp,
			"ActLinkNum/Bmp=", mld->u1ActiveLinkNum,
			mld->u1ActiveLinkBmp,
			"Aggressive[Trig/Tx]=", mld->fgAgcAggressiveMode[0],
			mld->fgAgcAggressiveMode[1]);

	for (i = 0; i < MLD_LINK_MAX; i++) {
		prMldLink = &(mld->arMldRecLink[i]);
		prAgcParam = &(prMldLink->rAgcDispParamTx);
		prAgcParamTrig = &(prMldLink->rAgcDispParamTrig);

		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"\n=== Link_%d ===\n", i);

		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%s%d, %s%d, %s%d, %s%d, %s%d\n",
			"Act=", prMldLink->fgActive,
			"Suspend=", prMldLink->fgSuspend,
			"ParMldRec=", prMldLink->u1ParentMldRecIdx,
			"Band=", prMldLink->u1Band,
			"WlanIdx=", prMldLink->u2WlanIdx);

		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"---AgcDispParamTx---\n");

		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%s%d, %s%d, %s%d, %s%d\n",
			"State=", prAgcParam->u1AgcStateTx,
			"Ratio=", prAgcParam->u1DispRatioTx,
			"Order=", prAgcParam->u1DispOrderTx,
			"Mgf=", prAgcParam->u2DispMgfTx);

		for (j = 0; j < MAX_MLO_MGMT_SUPPORT_AC_NUM; j++) {
			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"Pol_AC%d=%d",
				j, prAgcParam->au1DispPolTx[j]);

			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%s",
				((j == MAX_MLO_MGMT_SUPPORT_AC_NUM - 1) ?
				"\n" : ", "));
		}

		i4BytesWritten += kalScnprintf(
			pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%s%s",
			"---OvlpRptCnt---\n",
			"Corr0-Corr1-InCorr0-InCorr1\n");

		for (j = 0; j < MAX_MLO_MGMT_SUPPORT_AC_NUM; j++) {
			prOvlpRptCnt = &(prMldLink->arOvlpRptCntTx[j]);

			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%s%d: %d-%d-%d-%d\n",
				"AC", j,
				prOvlpRptCnt->u2Corr0,
				prOvlpRptCnt->u2Corr1,
				prOvlpRptCnt->u2InCorr0,
				prOvlpRptCnt->u2InCorr1);
		}

		/* Trig */
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"---AgcDispParamTrig---\n");

		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%s%d, %s%d, %s%d, %s%d\n",
			"State=", prAgcParamTrig->u1AgcStateTrig,
			"Ratio=", prAgcParamTrig->u1DispRatioTrig,
			"MuLen=", prAgcParamTrig->u1DispMuLenTrig,
			"Mgf=", prAgcParamTrig->u2DispMgfTrig);

		for (j = 0; j < MAX_MLO_MGMT_SUPPORT_AC_NUM; j++) {
			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"Pol_AC%d=%d",
				j, prAgcParamTrig->au1DispPolTrig[j]);

			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%s",
				((j == MAX_MLO_MGMT_SUPPORT_AC_NUM - 1) ?
				"\n" : ", "));
		}

		i4BytesWritten += kalScnprintf(
			pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%s%s",
			"---OvlpRptCntTrig---\n",
			"Corr0-Corr1-InCorr0-InCorr1\n");

		for (j = 0; j < MAX_MLO_MGMT_SUPPORT_AC_NUM; j++) {
			prOvlpRptCntTrig = &(prMldLink->arOvlpRptCntTrig[j]);

			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%s%d: %d-%d-%d-%d\n",
				"AC", j,
				prOvlpRptCntTrig->u2Corr0,
				prOvlpRptCntTrig->u2Corr1,
				prOvlpRptCntTrig->u2InCorr0,
				prOvlpRptCntTrig->u2InCorr1);
		}
	}

	return i4BytesWritten;
}
#endif

#if defined(_HIF_PCIE) || defined(_HIF_AXI)
static void connac3x_show_wfdma_axi_debug_log(
	struct ADAPTER *prAdapter,
	enum _ENUM_WFDMA_TYPE_T enum_wfdma_type)
{
	uint32_t pdma_base_cr;
	uint32_t i = 0;

	if (enum_wfdma_type == WFDMA_TYPE_HOST)
		pdma_base_cr = CONNAC3X_HOST_EXT_CONN_HIF_WRAP;
	else
		pdma_base_cr = CONNAC3X_MCU_INT_CONN_HIF_WRAP;

	for (i = 0; i < 13; i++) {
		uint32_t target_cr = pdma_base_cr + 0x500 + (i * 4);
		uint32_t u4RegValue = 0;

		HAL_MCR_RD(prAdapter, target_cr, &u4RegValue);
		DBGLOG(INIT, INFO, "get(0x%08x):0x%08x\n",
			target_cr,
			u4RegValue);
	}
}

void connac3x_show_wfdma_interrupt_info(
	struct ADAPTER *prAdapter,
	enum _ENUM_WFDMA_TYPE_T enum_wfdma_type,
	uint32_t u4DmaNum)
{
	uint32_t idx;
	uint32_t u4hostBaseCrAddr;
	uint32_t u4DmaCfgCrAddr = 0;
	uint32_t u4RegValue = 0;

	/* Dump Interrupt Status info */
	DBGLOG(HAL, INFO, "Interrupt Status:\n");

	/* Dump Global Status CR */
	u4hostBaseCrAddr = WFDMA_TYPE_HOST ?
		CONNAC3X_MCU_INT_CONN_HIF_WRAP :
		CONNAC3X_HOST_EXT_CONN_HIF_WRAP;

	u4DmaCfgCrAddr = CONNAC3X_WPDMA_EXT_INT_STA(u4hostBaseCrAddr);

	HAL_MCR_RD(prAdapter, u4DmaCfgCrAddr, &u4RegValue);

	DBGLOG(INIT, INFO, "\t Global INT STA(0x%08x): 0x%08x\n",
		u4DmaCfgCrAddr, u4RegValue);

	/* Dump PDMA Status CR */
	for (idx = 0; idx < u4DmaNum; idx++) {
		if (enum_wfdma_type == WFDMA_TYPE_HOST)
			u4hostBaseCrAddr = idx ?
				CONNAC3X_HOST_WPDMA_1_BASE :
				CONNAC3X_HOST_WPDMA_0_BASE;
		else
			u4hostBaseCrAddr = idx ?
				CONNAC3X_MCU_WPDMA_1_BASE :
				CONNAC3X_MCU_WPDMA_0_BASE;

		u4DmaCfgCrAddr = CONNAC3X_WPDMA_INT_STA(u4hostBaseCrAddr);

		HAL_MCR_RD(prAdapter, u4DmaCfgCrAddr, &u4RegValue);

		DBGLOG(HAL, INFO, "\t WFDMA DMA %d INT STA(0x%08x): 0x%08x\n",
				idx, u4DmaCfgCrAddr, u4RegValue);
	}

	/* Dump Interrupt Enable Info */
	DBGLOG(HAL, INFO, "Interrupt Enable:\n");

	/* Dump Global Enable CR */
	u4hostBaseCrAddr = WFDMA_TYPE_HOST ?
		CONNAC3X_MCU_INT_CONN_HIF_WRAP :
		CONNAC3X_HOST_EXT_CONN_HIF_WRAP;

	u4DmaCfgCrAddr = CONNAC3X_WPDMA_EXT_INT_MASK(u4hostBaseCrAddr);

	HAL_MCR_RD(prAdapter, u4DmaCfgCrAddr, &u4RegValue);

	DBGLOG(INIT, INFO, "\t Global INT ENA(0x%08x): 0x%08x\n",
		u4DmaCfgCrAddr, u4RegValue);

	/* Dump PDMA Enable CR */
	for (idx = 0; idx < u4DmaNum; idx++) {
		if (enum_wfdma_type == WFDMA_TYPE_HOST)
			u4hostBaseCrAddr = idx ?
				CONNAC3X_HOST_WPDMA_1_BASE :
				CONNAC3X_HOST_WPDMA_0_BASE;
		else
			u4hostBaseCrAddr = idx ?
				CONNAC3X_MCU_WPDMA_1_BASE :
				CONNAC3X_MCU_WPDMA_0_BASE;

		u4DmaCfgCrAddr = CONNAC3X_WPDMA_INT_MASK(u4hostBaseCrAddr);

		HAL_MCR_RD(prAdapter, u4DmaCfgCrAddr, &u4RegValue);

		DBGLOG(HAL, INFO, "\t WFDMA DMA %d INT ENA(0x%08x): 0x%08x\n",
			idx, u4DmaCfgCrAddr, u4RegValue);
	}
}

void connac3x_show_wfdma_glo_info(
	struct ADAPTER *prAdapter,
	enum _ENUM_WFDMA_TYPE_T enum_wfdma_type,
	uint32_t u4DmaNum)
{
	uint32_t idx;
	uint32_t u4hostBaseCrAddr;
	uint32_t u4DmaCfgCrAddr = 0;
	union WPDMA_GLO_CFG_STRUCT GloCfgValue = {0};

	for (idx = 0; idx < u4DmaNum; idx++) {
		if (enum_wfdma_type == WFDMA_TYPE_HOST)
			u4hostBaseCrAddr = idx ?
			CONNAC3X_HOST_WPDMA_1_BASE :
			CONNAC3X_HOST_WPDMA_0_BASE;
		else
			u4hostBaseCrAddr = idx ?
			CONNAC3X_MCU_WPDMA_1_BASE :
			CONNAC3X_MCU_WPDMA_0_BASE;

		u4DmaCfgCrAddr = CONNAC3X_WPDMA_GLO_CFG(u4hostBaseCrAddr);

		HAL_MCR_RD(prAdapter, u4DmaCfgCrAddr,
			   &GloCfgValue.word);

		DBGLOG(HAL, INFO, "WFDMA DMA (%d) GLO Config Info:\n", idx);
		DBGLOG(INIT, INFO, "\t GLO Control (0x%08x): 0x%08x\n",
			u4DmaCfgCrAddr, GloCfgValue.word);
		DBGLOG(INIT, INFO,
			"\t GLO Control EN T/R bit=(%d/%d), Busy T/R bit=(%d/%d)\n",
			GloCfgValue.field_conn2x.tx_dma_en,
			GloCfgValue.field_conn2x.rx_dma_en,
			GloCfgValue.field_conn2x.tx_dma_busy,
			GloCfgValue.field_conn2x.rx_dma_busy
			);
	}
}

void connac3x_show_wfdma_ring_info(
	struct ADAPTER *prAdapter,
	enum _ENUM_WFDMA_TYPE_T enum_wfdma_type)
{
	uint32_t idx;
	uint32_t group_cnt;
	uint32_t u4DmaCfgCrAddr;
	struct wfdma_group_info *group;
	uint32_t u4_hw_desc_base_value = 0;
	uint32_t u4_hw_cnt_value = 0;
	uint32_t u4_hw_cidx_value = 0;
	uint32_t u4_hw_didx_value = 0;
	uint32_t queue_cnt;
	struct mt66xx_chip_info *prChipInfo = NULL;
	struct BUS_INFO *prBusInfo;

	glGetChipInfo((void **)&prChipInfo);
	if (!prChipInfo)
		return;

	prBusInfo = prChipInfo->bus_info;

	/* Dump All Ring Info */
	DBGLOG(HAL, INFO, "TX Ring Configuration\n");
	DBGLOG(HAL, INFO, "%4s %20s %8s %10s %6s %6s %6s %6s\n",
		"Idx", "Attr", "Reg", "Base", "Cnt", "CIDX", "DIDX", "QCnt");

	/* Dump TX Ring */
	if (enum_wfdma_type == WFDMA_TYPE_HOST)
		group_cnt = prBusInfo->wfmda_host_tx_group_len;
	else
		group_cnt = prBusInfo->wfmda_wm_tx_group_len;

	for (idx = 0; idx < group_cnt; idx++) {
		if (enum_wfdma_type == WFDMA_TYPE_HOST)
			group = &prBusInfo->wfmda_host_tx_group[idx];
		else
			group = &prBusInfo->wfmda_wm_tx_group[idx];

		u4DmaCfgCrAddr = group->hw_desc_base;

		HAL_MCR_RD(prAdapter, u4DmaCfgCrAddr, &u4_hw_desc_base_value);
		HAL_MCR_RD(prAdapter, u4DmaCfgCrAddr+0x04, &u4_hw_cnt_value);
		HAL_MCR_RD(prAdapter, u4DmaCfgCrAddr+0x08, &u4_hw_cidx_value);
		HAL_MCR_RD(prAdapter, u4DmaCfgCrAddr+0x0c, &u4_hw_didx_value);

		group->cnt = u4_hw_cnt_value & MT_RING_CNT_MASK;
		group->cidx = u4_hw_cidx_value;
		group->didx = u4_hw_didx_value;

		queue_cnt = (u4_hw_cidx_value >= u4_hw_didx_value) ?
			(u4_hw_cidx_value - u4_hw_didx_value) :
			(u4_hw_cidx_value - u4_hw_didx_value + u4_hw_cnt_value);

		DBGLOG(HAL, INFO, "%4d %20s %8x %10x %8x %6x %6x %6x\n",
			idx,
			group->name,
			u4DmaCfgCrAddr, u4_hw_desc_base_value,
			u4_hw_cnt_value, u4_hw_cidx_value,
			u4_hw_didx_value, queue_cnt);

	}

	DBGLOG(HAL, INFO, "RX Ring Configuration\n");
	DBGLOG(HAL, INFO, "%4s %20s %8s %10s %6s %6s %6s %6s\n",
		"Idx", "Attr", "Reg", "Base", "Cnt", "CIDX", "DIDX", "QCnt");

	/* Dump RX Ring */
	if (enum_wfdma_type == WFDMA_TYPE_HOST)
		group_cnt = prBusInfo->wfmda_host_rx_group_len;
	else
		group_cnt = prBusInfo->wfmda_wm_rx_group_len;

	for (idx = 0; idx < group_cnt; idx++) {
		if (enum_wfdma_type == WFDMA_TYPE_HOST)
			group = &prBusInfo->wfmda_host_rx_group[idx];
		else
			group = &prBusInfo->wfmda_wm_rx_group[idx];

		u4DmaCfgCrAddr = group->hw_desc_base;

		HAL_MCR_RD(prAdapter, u4DmaCfgCrAddr, &u4_hw_desc_base_value);
		HAL_MCR_RD(prAdapter, u4DmaCfgCrAddr+0x04, &u4_hw_cnt_value);
		HAL_MCR_RD(prAdapter, u4DmaCfgCrAddr+0x08, &u4_hw_cidx_value);
		HAL_MCR_RD(prAdapter, u4DmaCfgCrAddr+0x0c, &u4_hw_didx_value);

		group->cnt = u4_hw_cnt_value & MT_RING_CNT_MASK;
		group->cidx = u4_hw_cidx_value;
		group->didx = u4_hw_didx_value;

		queue_cnt = (u4_hw_didx_value > u4_hw_cidx_value) ?
			(u4_hw_didx_value - u4_hw_cidx_value - 1) :
			(u4_hw_didx_value - u4_hw_cidx_value
			+ u4_hw_cnt_value - 1);

		DBGLOG(HAL, INFO, "%4d %20s %8x %10x %8x %6x %6x %6x\n",
			idx,
			group->name,
			u4DmaCfgCrAddr, u4_hw_desc_base_value,
			u4_hw_cnt_value, u4_hw_cidx_value,
			u4_hw_didx_value, queue_cnt);
	}
}

void connac3x_show_wfdma_desc(struct ADAPTER *prAdapter)
{
	struct BUS_INFO *prBusInfo;
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct RTMP_TX_RING *prTxRing;
	struct RTMP_RX_RING *prRxRing;
	struct wfdma_group_info *prGroup;
	uint32_t i = 0, u4SwIdx;

	if (!prAdapter)
		return;

	/* PDMA Tx/Rx descriptor & packet content */
	prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	prBusInfo = prAdapter->chip_info->bus_info;

	for (i = 0; i < prBusInfo->wfmda_host_tx_group_len; i++) {
		prGroup = &prBusInfo->wfmda_host_tx_group[i];
		if (!prGroup->dump_ring_content)
			continue;
		DBGLOG(HAL, INFO, "Dump PDMA Tx Ring[%u]\n", i);
		prTxRing = &prHifInfo->TxRing[i];
		u4SwIdx = prGroup->didx;
		kalDumpTxRing(prAdapter->prGlueInfo, prTxRing,
			      u4SwIdx, true);
		u4SwIdx = prGroup->didx == 0 ?
			prGroup->cnt - 1 : prGroup->didx - 1;
		kalDumpTxRing(prAdapter->prGlueInfo, prTxRing, u4SwIdx, true);
		u4SwIdx = prGroup->didx == 0 ?
			prGroup->cnt - 2 : prGroup->didx - 2;
		kalDumpTxRing(prAdapter->prGlueInfo, prTxRing, u4SwIdx, true);
	}

	for (i = 0; i < prBusInfo->wfmda_host_rx_group_len; i++) {
		prGroup = &prBusInfo->wfmda_host_rx_group[i];
		if (!prGroup->dump_ring_content)
			continue;
		DBGLOG(HAL, INFO, "Dump PDMA Rx Ring[%u]\n", i);
		prRxRing = &prHifInfo->RxRing[i];
		u4SwIdx = prGroup->didx;
		kalDumpRxRing(prAdapter->prGlueInfo, prRxRing, u4SwIdx, true);
		u4SwIdx = prGroup->didx == 0 ?
			prGroup->cnt - 1 : prGroup->didx - 1;
		kalDumpRxRing(prAdapter->prGlueInfo, prRxRing, u4SwIdx, true);
	}
}

static void connac3xDumpPPDebugCr(struct ADAPTER *prAdapter)
{
	struct BUS_INFO *prBusInfo;
	struct PP_TOP_CR *prCr;
	uint32_t u4Value[4] = {0};

	if (!prAdapter)
		return;

	prBusInfo = prAdapter->chip_info->bus_info;
	prCr = prBusInfo->prPpTopCr;

	HAL_MCR_RD(prAdapter, prCr->rDbgCtrl.u4Addr, &u4Value[0]);
	HAL_MCR_RD(prAdapter, prCr->rDbgCs0.u4Addr, &u4Value[1]);
	HAL_MCR_RD(prAdapter, prCr->rDbgCs1.u4Addr, &u4Value[2]);
	HAL_MCR_RD(prAdapter, prCr->rDbgCs2.u4Addr, &u4Value[3]);

	DBGLOG(HAL, INFO,
	"PP[0x%08x]=0x%08x,[0x%08x]=0x%08x,[0x%08x]=0x%08x,[0x%08x]=0x%08x,",
		prCr->rDbgCtrl.u4Addr, u4Value[0],
		prCr->rDbgCs0.u4Addr, u4Value[1],
		prCr->rDbgCs1.u4Addr, u4Value[2],
		prCr->rDbgCs2.u4Addr, u4Value[3]);
}

static void connac3x_dump_wfdma_dbg_value(
	struct ADAPTER *prAdapter,
	enum _ENUM_WFDMA_TYPE_T enum_wfdma_type,
	uint32_t wfdma_idx)
{
#define BUF_SIZE 1024

	uint32_t pdma_base_cr;
	uint32_t set_debug_flag_value;
	char *buf;
	uint32_t pos = 0;
	uint32_t set_debug_cr, get_debug_cr, get_debug_value = 0;

	if (enum_wfdma_type == WFDMA_TYPE_HOST) {
		if (wfdma_idx == 0)
			pdma_base_cr = CONNAC3X_HOST_WPDMA_0_BASE;
		else
			pdma_base_cr = CONNAC3X_HOST_WPDMA_1_BASE;
	} else {
		if (wfdma_idx == 0)
			pdma_base_cr = CONNAC3X_MCU_WPDMA_0_BASE;
		else
			pdma_base_cr = CONNAC3X_MCU_WPDMA_1_BASE;
	}

	buf = (char *) kalMemAlloc(BUF_SIZE, VIR_MEM_TYPE);
	if (!buf) {
		DBGLOG(HAL, ERROR, "Mem allocation failed.\n");
		return;
	}
	set_debug_cr = pdma_base_cr + 0x124;
	get_debug_cr = pdma_base_cr + 0x128;
	kalMemZero(buf, BUF_SIZE);
	pos += kalSnprintf(buf + pos, 50,
			"set_debug_cr:0x%08x get_debug_cr:0x%08x; ",
			set_debug_cr, get_debug_cr);
	for (set_debug_flag_value = 0x100; set_debug_flag_value <= 0x112;
			set_debug_flag_value++) {
		HAL_MCR_WR(prAdapter, set_debug_cr, set_debug_flag_value);
		HAL_MCR_RD(prAdapter, get_debug_cr, &get_debug_value);
		pos += kalSnprintf(buf + pos, 40, "Set:0x%03x, result=0x%08x%s",
			set_debug_flag_value,
			get_debug_value,
			set_debug_flag_value == 0x112 ? "\n" : "; ");
	}
	DBGLOG(HAL, INFO, "%s", buf);
	kalMemFree(buf, VIR_MEM_TYPE, BUF_SIZE);
}

void connac3x_show_wfdma_dbg_flag_log(
	struct ADAPTER *prAdapter,
	enum _ENUM_WFDMA_TYPE_T enum_wfdma_type,
	uint32_t u4DmaNum)
{
	uint32_t u4Idx;

	for (u4Idx = 0; u4Idx < u4DmaNum; u4Idx++)
		connac3x_dump_wfdma_dbg_value(
			prAdapter, enum_wfdma_type, u4Idx);
}

void connac3x_show_wfdma_info_by_type(
	struct ADAPTER *prAdapter,
	enum _ENUM_WFDMA_TYPE_T enum_wfdma_type,
	uint32_t u4DmaNum)
{
	struct CHIP_DBG_OPS *prDbgOps = NULL;

	if (prAdapter)
		prDbgOps = prAdapter->chip_info->prDebugOps;

	/* Dump WFMDA info */
	DBGLOG(HAL, INFO, "==============================\n");
	DBGLOG(HAL, INFO, "%s WFMDA Configuration:\n",
	       enum_wfdma_type == WFDMA_TYPE_HOST ? "HOST" : "WM");
	DBGLOG(HAL, INFO, "==============================\n");
	connac3x_show_wfdma_interrupt_info(
		prAdapter, enum_wfdma_type, u4DmaNum);
	connac3x_show_wfdma_glo_info(
		prAdapter, enum_wfdma_type, u4DmaNum);
	connac3x_show_wfdma_ring_info(
		prAdapter, enum_wfdma_type);
	if (prDbgOps && prDbgOps->show_wfdma_dbg_probe_info)
		prDbgOps->show_wfdma_dbg_probe_info(prAdapter,
			enum_wfdma_type);
	connac3x_show_wfdma_axi_debug_log(prAdapter, WFDMA_TYPE_HOST);
	if (prDbgOps && prDbgOps->show_wfdma_wrapper_info)
		prDbgOps->show_wfdma_wrapper_info(prAdapter,
			enum_wfdma_type);
}

#if (CFG_SUPPORT_HOST_OFFLOAD == 1)
void connac3x_show_mawd_info(struct ADAPTER *prAdapter)
{
	struct mt66xx_chip_info *prChipInfo;
	struct GL_HIF_INFO *prHifInfo;
	struct BUS_INFO *prBusInfo;
	struct RTMP_DMABUF *prErrRpt;
	uint32_t *pu4ErrRpt, *pu4HifTxd;
	uint32_t u4Cidx = 0, u4Didx = 0, u4Val = 0, u4Idx, u4Num;

	prChipInfo = prAdapter->chip_info;
	prBusInfo = prAdapter->chip_info->bus_info;
	prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	prErrRpt = &prHifInfo->ErrRptRing;
	pu4ErrRpt = prErrRpt->AllocVa;

	DBGLOG(HAL, INFO, "==============================\n");
	DBGLOG(HAL, INFO, " MAWD DEBUG DUMP\n");
	DBGLOG(HAL, INFO, "==============================\n");

	HAL_MCR_RD(prAdapter, prBusInfo->mawd_err_rpt_ctrl2, &u4Val);
	u4Didx = (u4Val & BITS(16, 28)) >> 16;
	u4Cidx = u4Val & BITS(0, 12);
	DBGLOG(HAL, INFO, "ERR_RPT_CTRL2:0x%08x!\n", u4Val);
	while (u4Cidx != u4Didx) {
		if (!pu4ErrRpt)
			break;
		DBGLOG(HAL, INFO, "ErrRpt[%d]:0x%08x!\n",
		       u4Cidx, pu4ErrRpt[u4Cidx]);
		INC_RING_INDEX(u4Cidx, RX_RING_SIZE);
	}
	HAL_MCR_WR(prAdapter, prBusInfo->mawd_err_rpt_ctrl2, u4Cidx);

	for (u4Idx = MAWD_HIF_TXD_MD_CTRL0;
	     u4Idx <= MAWD_SETTING3; u4Idx += 4) {
		HAL_MCR_RD(prAdapter, u4Idx, &u4Val);
		DBGLOG(HAL, TRACE, "CR [0x%08x]=[0x%08x]", u4Idx, u4Val);
	}

	for (u4Num = 0; u4Num < MAWD_MD_TX_RING_NUM; u4Num++) {
		pu4HifTxd = prHifInfo->HifTxDescRing[u4Num].AllocVa;
		if (!pu4HifTxd)
			continue;
		for (u4Idx = 0; u4Idx < 3; u4Idx++) {
			DBGLOG(HAL, INFO, "HIF TXD %d-%d\n", u4Num, u4Idx);
			dumpMemory32((uint32_t *)pu4HifTxd,
				     NIC_TX_DESC_AND_PADDING_LENGTH +
				     prChipInfo->txd_append_size);
			pu4HifTxd += 19;
		}
	}
}

void connac3x_show_rro_info(struct ADAPTER *prAdapter)
{
	struct mt66xx_chip_info *prChipInfo;
	struct GL_HIF_INFO *prHifInfo;
	struct WIFI_VAR *prWifiVar;
	struct RTMP_DMABUF *prIndCmd;
	struct RTMP_DMABUF *prAddrArray;
	uint32_t u4Val = 0, u4Idx, u4DebugLevel = 0;

	prChipInfo = prAdapter->chip_info;
	prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	prIndCmd = &prHifInfo->IndCmdRing;
	prAddrArray = &prHifInfo->AddrArray;
	prWifiVar = &prAdapter->rWifiVar;
	wlanGetDriverDbgLevel(DBG_HAL_IDX, &u4DebugLevel);

	if (IS_FEATURE_ENABLED(prWifiVar->fgEnableMawd) &&
	    halMawdCheckInfra(prAdapter)) {

		DBGLOG(HAL, INFO, "==============================\n");
		DBGLOG(HAL, INFO, " MAWD DEBUG DUMP\n");
		DBGLOG(HAL, INFO, "==============================\n");

		for (u4Idx = MAWD_IND_CMD_CTRL0;
		     u4Idx <= MAWD_MD_RX_BLK_CTRL2; u4Idx += 4) {
			HAL_MCR_RD(prAdapter, u4Idx, &u4Val);
			DBGLOG(HAL, INFO, "CR [0x%08x]=[0x%08x]", u4Idx, u4Val);
		}

		for (u4Idx = MAWD_IND_CMD_SIGNATURE0;
		     u4Idx <= MAWD_R2AXI_CTRL3; u4Idx += 4) {
			HAL_MCR_RD(prAdapter, u4Idx, &u4Val);
			DBGLOG(HAL, INFO, "CR [0x%08x]=[0x%08x]", u4Idx, u4Val);
		}

		for (u4Idx = MAWD_MD_INTERRUPT_SETTING0;
		     u4Idx <= MAWD_AP_INTERRUPT_SETTING1; u4Idx += 4) {
			HAL_MCR_RD(prAdapter, u4Idx, &u4Val);
			DBGLOG(HAL, INFO, "CR [0x%08x]=[0x%08x]", u4Idx, u4Val);
		}
	}

	DBGLOG(HAL, INFO, "==============================\n");
	DBGLOG(HAL, INFO, " RRO DEBUG DUMP\n");
	DBGLOG(HAL, INFO, "==============================\n");

	for (u4Idx = WF_RRO_TOP_GLOBAL_CONFG_ADDR;
	     u4Idx <= WF_RRO_TOP_DBG_RDAT_DW3_ADDR; u4Idx += 4) {
		HAL_MCR_RD(prAdapter, u4Idx, &u4Val);
		DBGLOG(HAL, INFO, "CR [0x%08x]=[0x%08x]", u4Idx, u4Val);
	}

	for (u4Idx = 0x100; u4Idx <= 0x10c; u4Idx++) {
		HAL_MCR_WR(prAdapter, WF_RRO_TOP_DBG_FLAG_CTRL_ADDR, u4Idx);
		HAL_MCR_RD(prAdapter, WF_RRO_TOP_DBG_FLAG_OUTPUT_ADDR, &u4Val);
		DBGLOG(HAL, INFO, "CR [0x%08x]=[0x%08x] [0x%08x]=[0x%08x]",
		       WF_RRO_TOP_DBG_FLAG_CTRL_ADDR, u4Idx,
		       WF_RRO_TOP_DBG_FLAG_OUTPUT_ADDR, u4Val);
	}

	if (u4DebugLevel & DBG_CLASS_TRACE) {
		if (prIndCmd->AllocVa) {
			DBGLOG(HAL, INFO, " IndCmd DUMP\n");
			dumpMemory32((uint32_t *)prIndCmd->AllocVa,
				     prIndCmd->AllocSize);
		}
		if (prAddrArray->AllocVa) {
			DBGLOG(HAL, INFO, " AddrArray DUMP\n");
			dumpMemory128((uint32_t *)prAddrArray->AllocVa,
				      prAddrArray->AllocSize);
		}
	}

	DBGLOG(HAL, INFO, "BLK Used0[%u] Used1[%u] Free[%u]",
	       prHifInfo->u4RcbUsedListCnt[RX_RING_DATA0],
	       prHifInfo->u4RcbUsedListCnt[RX_RING_DATA1],
	       prHifInfo->u4RcbFreeListCnt);
}
#endif /* CFG_SUPPORT_HOST_OFFLOAD == 1 */

void connac3x_DumpWfsyscpupcr(struct ADAPTER *prAdapter)
{
#define CPUPCR_LOG_NUM	5
#define CPUPCR_BUF_SZ	50

	uint32_t i = 0;
	uint32_t var_pc = 0;
	uint32_t var_lp = 0;
	uint64_t log_sec = 0;
	uint64_t log_nsec = 0;
	char log_buf_pc[CPUPCR_LOG_NUM][CPUPCR_BUF_SZ];
	char log_buf_lp[CPUPCR_LOG_NUM][CPUPCR_BUF_SZ];

	for (i = 0; i < CPUPCR_LOG_NUM; i++) {
		log_sec = local_clock();
		log_nsec = do_div(log_sec, 1000000000)/1000;
		HAL_MCR_RD(prAdapter, WFSYS_CPUPCR_ADDR, &var_pc);
		HAL_MCR_RD(prAdapter, WFSYS_LP_ADDR, &var_lp);

		kalSnprintf(log_buf_pc[i],
			    CPUPCR_BUF_SZ,
			    "%llu.%06llu/0x%08x;",
			    log_sec,
			    log_nsec,
			    var_pc);

		kalSnprintf(log_buf_lp[i],
			    CPUPCR_BUF_SZ,
			    "%llu.%06llu/0x%08x;",
			    log_sec,
			    log_nsec,
			    var_lp);
	}

	DBGLOG(HAL, INFO, "wm pc=%s%s%s%s%s\n",
	       log_buf_pc[0],
	       log_buf_pc[1],
	       log_buf_pc[2],
	       log_buf_pc[3],
	       log_buf_pc[4]);

	DBGLOG(HAL, INFO, "wm lp=%s%s%s%s%s\n",
	       log_buf_lp[0],
	       log_buf_lp[1],
	       log_buf_lp[2],
	       log_buf_lp[3],
	       log_buf_lp[4]);
}

void connac3x_DbgCrRead(
	struct ADAPTER *prAdapter, uint32_t addr, unsigned int *val)
{
	if (prAdapter == NULL)
		wf_ioremap_read(addr, val);
	else
		HAL_MCR_RD(prAdapter, (addr | 0x64000000), val);
}

void connac3x_DbgCrWrite(
	struct ADAPTER *prAdapter, uint32_t addr, unsigned int val)
{
	if (prAdapter == NULL)
		wf_ioremap_write(addr, val);
	else
		HAL_MCR_WR(prAdapter, (addr | 0x64000000), val);
}

void connac3x_dump_format_memory32(
	uint32_t *pu4StartAddr, uint32_t u4Count, char *aucInfo)
{
#define ONE_LINE_MAX_COUNT	16
#define TMP_BUF_SZ			20
	uint32_t i, endCount;
	char buf[ONE_LINE_MAX_COUNT*10];
	char tmp[TMP_BUF_SZ] = {'\0'};

	ASSERT(pu4StartAddr);

	LOG_FUNC("%s, Count(%d)\n", aucInfo, u4Count);

	while (u4Count > 0) {

		kalSnprintf(buf, TMP_BUF_SZ, "%08x", pu4StartAddr[0]);

		if (u4Count > ONE_LINE_MAX_COUNT)
			endCount = ONE_LINE_MAX_COUNT;
		else
			endCount = u4Count;

		for (i = 1; i < endCount; i++) {
			kalSnprintf(tmp, TMP_BUF_SZ, " %08x", pu4StartAddr[i]);
			strncat(buf, tmp, strlen(tmp));
		}

		LOG_FUNC("%s\n", buf);

		if (u4Count > ONE_LINE_MAX_COUNT) {
			u4Count -= ONE_LINE_MAX_COUNT;
			pu4StartAddr += ONE_LINE_MAX_COUNT;
		} else
			u4Count = 0;
	}
}

void connac3x_DumpCrRange(
	struct ADAPTER *prAdapter,
	uint32_t cr_start, uint32_t word_count, char *str)
{
#define LOG_MAIX_ITEM 16

	uint32_t u4Cr, i;
	uint32_t dummy[LOG_MAIX_ITEM] = {0};

	if (word_count > LOG_MAIX_ITEM)
		word_count = LOG_MAIX_ITEM;

	for (i = 0, u4Cr = cr_start; i < word_count; i++) {
		connac3x_DbgCrRead(prAdapter, u4Cr, &dummy[i]);
		u4Cr += 0x04;
	}
	connac3x_dump_format_memory32(dummy, word_count, str);
}
#endif /* _HIF_PCIE || _HIF_AXI */

void connac3x_show_wfdma_info(struct ADAPTER *prAdapter)
{
#if defined(_HIF_PCIE) || defined(_HIF_AXI)
	struct BUS_INFO *prBusInfo;
	struct mt66xx_chip_info *prChipInfo;
	struct SW_WFDMA_INFO *prSwWfdmaInfo;
	struct WIFI_VAR *prWifiVar;
	uint32_t u4DmaNum = 1;

	prChipInfo = prAdapter->chip_info;
	prBusInfo = prChipInfo->bus_info;
	prSwWfdmaInfo = &prBusInfo->rSwWfdmaInfo;
	prWifiVar = &prAdapter->rWifiVar;

	if (prSwWfdmaInfo->rOps.dumpDebugLog)
		prSwWfdmaInfo->rOps.dumpDebugLog(prAdapter->prGlueInfo);

	if (prChipInfo->is_support_wfdma1)
		u4DmaNum++;

	connac3x_show_wfdma_info_by_type(prAdapter, WFDMA_TYPE_HOST, u4DmaNum);
	connac3x_show_wfdma_dbg_flag_log(prAdapter, WFDMA_TYPE_HOST, u4DmaNum);

	if (prBusInfo->wfmda_wm_tx_group && prBusInfo->wfmda_wm_rx_group) {
		connac3x_show_wfdma_info_by_type(
			prAdapter, WFDMA_TYPE_WM, u4DmaNum);
		connac3x_show_wfdma_dbg_flag_log(
			prAdapter, WFDMA_TYPE_WM, u4DmaNum);
	}

	connac3x_show_wfdma_desc(prAdapter);

	connac3xDumpPPDebugCr(prAdapter);

#if (CFG_SUPPORT_HOST_OFFLOAD == 1)
	if (IS_FEATURE_ENABLED(prWifiVar->fgEnableMawdTx))
		connac3x_show_mawd_info(prAdapter);

	if (IS_FEATURE_ENABLED(prWifiVar->fgEnableRro))
		connac3x_show_rro_info(prAdapter);
#endif

	if (prBusInfo->showDebugInfo)
		prBusInfo->showDebugInfo(prAdapter->prGlueInfo);
#endif /*_HIF_PCIE || _HIF_AXI */
}

void connac3x_show_dmashdl_info(struct ADAPTER *prAdapter)
{
	struct BUS_INFO *prBusInfo;
	struct DMASHDL_CFG *prCfg;
	uint32_t value = 0;
	uint8_t idx;
	uint32_t rsv_cnt = 0;
	uint32_t src_cnt = 0;
	uint32_t total_src_cnt = 0;
	uint32_t total_rsv_cnt = 0;
	uint32_t ffa_cnt = 0;
	uint32_t free_pg_cnt = 0;
	uint32_t ple_rpg_hif;
	uint32_t ple_upg_hif;
	uint8_t is_mismatch = FALSE;

	DBGLOG(HAL, INFO, "DMASHDL info:\n");

	prBusInfo = prAdapter->chip_info->bus_info;
	prCfg = prBusInfo->prDmashdlCfg;

	asicConnac3xDmashdlGetRefill(prAdapter);
	asicConnac3xDmashdlGetPktMaxPage(prAdapter);

	HAL_MCR_RD(prAdapter, prCfg->rErrorFlagCtrl.u4Addr, &value);
	DBGLOG(HAL, INFO, "DMASHDL ERR FLAG CTRL(0x%08x): 0x%08x\n",
	       prCfg->rErrorFlagCtrl.u4Addr, value);

	for (idx = 0; idx <= ENUM_DMASHDL_GROUP_14; idx++) {
		if (prCfg->afgRefillEn[idx] == 0)
			continue;
		DBGLOG(HAL, INFO, "Group %d info:\n", idx);
		asicConnac3xDmashdlGetGroupControl(prAdapter, idx);
		rsv_cnt = asicConnac3xDmashdlGetRsvCount(prAdapter, idx);
		src_cnt = asicConnac3xDmashdlGetSrcCount(prAdapter, idx);
		asicConnac3xDmashdlGetPKTCount(prAdapter, idx);
		total_src_cnt += src_cnt;
		total_rsv_cnt += rsv_cnt;
	}

	/* Dump Group 15 info */
	idx = ENUM_DMASHDL_GROUP_15;
	DBGLOG(HAL, INFO, "Group %d info:\n", idx);
	asicConnac3xDmashdlGetGroupControl(prAdapter, idx);
	asicConnac3xDmashdlGetRsvCount(prAdapter, idx);
	asicConnac3xDmashdlGetSrcCount(prAdapter, idx);
	asicConnac3xDmashdlGetPKTCount(prAdapter, idx);

	HAL_MCR_RD(prAdapter, prCfg->rStatusRdFfaCnt.u4Addr, &value);
	ffa_cnt = (value & prCfg->rStatusRdFfaCnt.u4Mask) >>
		prCfg->rStatusRdFfaCnt.u4Shift;
	free_pg_cnt = (value & prCfg->rStatusRdFreePageCnt.u4Mask) >>
		prCfg->rStatusRdFreePageCnt.u4Shift;
	DBGLOG(HAL, INFO, "\tDMASHDL Status_RD(0x%08x): 0x%08x\n",
		prCfg->rStatusRdFreePageCnt.u4Addr, value);
	DBGLOG(HAL, INFO, "\tfree page cnt = 0x%03x, ffa cnt = 0x%03x\n",
		free_pg_cnt, ffa_cnt);

	DBGLOG(HAL, INFO, "\nDMASHDL Counter Check:\n");
	HAL_MCR_RD(prAdapter, prCfg->rHifPgInfoHifRsvCnt.u4Addr, &value);
	ple_rpg_hif = (value & prCfg->rHifPgInfoHifRsvCnt.u4Mask) >>
		  prCfg->rHifPgInfoHifRsvCnt.u4Shift;
	ple_upg_hif = (value & prCfg->rHifPgInfoHifSrcCnt.u4Mask) >>
		prCfg->rHifPgInfoHifSrcCnt.u4Shift;
	DBGLOG(HAL, INFO,
		"\tPLE:The used/reserved pages of PLE HIF group=0x%03x/0x%03x\n",
		 ple_upg_hif, ple_rpg_hif);
	DBGLOG(HAL, INFO,
		"\tDMASHDL:The total used pages of group0~14=0x%03x\n",
		total_src_cnt);

	if (ple_upg_hif != total_src_cnt) {
		DBGLOG(HAL, INFO,
			"\tPLE used pages & total used pages mismatch!\n");
		is_mismatch = TRUE;
	}

	DBGLOG(HAL, INFO,
		"\tThe total reserved pages of group0~14=0x%03x\n",
		total_rsv_cnt);
	DBGLOG(HAL, INFO,
		"\tThe total ffa pages of group0~14=0x%03x\n",
		ffa_cnt);
	DBGLOG(HAL, INFO,
		"\tThe total free pages of group0~14=0x%03x\n",
		free_pg_cnt);

	if (free_pg_cnt != total_rsv_cnt + ffa_cnt) {
		DBGLOG(HAL, INFO,
			"\tmismatch(total_rsv_cnt + ffa_cnt in DMASHDL)\n");
		is_mismatch = TRUE;
	}

	if (free_pg_cnt != ple_rpg_hif) {
		DBGLOG(HAL, INFO, "\tmismatch(reserved pages in PLE)\n");
		is_mismatch = TRUE;
	}


	if (!is_mismatch)
		DBGLOG(HAL, INFO, "DMASHDL: no counter mismatch\n");
}

static void chip_get_ple_acq_stat(struct ADAPTER *prAdapter, uint32_t *ple_stat)
{
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_QUEUE_EMPTY_ADDR, &ple_stat[0]);

	HAL_MCR_RD(prAdapter, WF_PLE_TOP_AC0_QUEUE_EMPTY0_ADDR, &ple_stat[1]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_AC0_QUEUE_EMPTY1_ADDR, &ple_stat[2]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_AC0_QUEUE_EMPTY2_ADDR, &ple_stat[3]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_AC0_QUEUE_EMPTY3_ADDR, &ple_stat[4]);

	HAL_MCR_RD(prAdapter, WF_PLE_TOP_AC1_QUEUE_EMPTY0_ADDR, &ple_stat[5]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_AC1_QUEUE_EMPTY1_ADDR, &ple_stat[6]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_AC1_QUEUE_EMPTY2_ADDR, &ple_stat[7]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_AC1_QUEUE_EMPTY3_ADDR, &ple_stat[8]);

	HAL_MCR_RD(prAdapter, WF_PLE_TOP_AC2_QUEUE_EMPTY0_ADDR, &ple_stat[9]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_AC2_QUEUE_EMPTY1_ADDR, &ple_stat[10]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_AC2_QUEUE_EMPTY2_ADDR, &ple_stat[11]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_AC2_QUEUE_EMPTY3_ADDR, &ple_stat[12]);

	HAL_MCR_RD(prAdapter, WF_PLE_TOP_AC3_QUEUE_EMPTY0_ADDR, &ple_stat[13]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_AC3_QUEUE_EMPTY1_ADDR, &ple_stat[14]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_AC3_QUEUE_EMPTY2_ADDR, &ple_stat[15]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_AC3_QUEUE_EMPTY3_ADDR, &ple_stat[16]);
}

static void chip_get_dis_sta_map(struct ADAPTER *prAdapter, uint32_t *dis_sta_map)
{
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_DIS_STA_MAP0_ADDR, &dis_sta_map[0]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_DIS_STA_MAP1_ADDR, &dis_sta_map[1]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_DIS_STA_MAP2_ADDR, &dis_sta_map[2]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_DIS_STA_MAP3_ADDR, &dis_sta_map[3]);
}

#if defined(_HIF_PCIE) || defined(_HIF_AXI)
/* =============================================================================
 *                         Debug Interrupt Interface v1
 * +---------------------------------------------------------------------------+
 * |Toggle|Rsv[30:28]|Ver[27:24]|Rsv[23:18]|BSSInx[17:14]|Mod[13:8]|Reason[7:0]|
 * +---------------------------------------------------------------------------+
 * =============================================================================
 */
uint32_t connac3x_get_ple_int(struct ADAPTER *prAdapter)
{
	struct BUS_INFO *prBusInfo;
	struct PLE_TOP_CR *prCr;
	uint32_t u4Val = 0;

	prBusInfo = prAdapter->chip_info->bus_info;
	prCr = prBusInfo->prPleTopCr;

	HAL_MCR_RD(prAdapter, prCr->rToN9IntToggle.u4Addr, &u4Val);

	return u4Val;
}

void connac3x_set_ple_int(struct ADAPTER *prAdapter, bool fgTrigger,
			  uint32_t u4ClrMask, uint32_t u4SetMask)
{
	struct BUS_INFO *prBusInfo;
	struct PLE_TOP_CR *prCr;
	uint32_t u4Val = 0;

	prBusInfo = prAdapter->chip_info->bus_info;
	prCr = prBusInfo->prPleTopCr;

	HAL_MCR_RD(prAdapter, prCr->rToN9IntToggle.u4Addr, &u4Val);

	if (fgTrigger) {
		u4Val = (~u4Val & prCr->rToN9IntToggle.u4Mask) |
			(u4Val & ~prCr->rToN9IntToggle.u4Mask);
	}

	u4Val &= ~u4ClrMask;
	u4Val |= u4SetMask;

	HAL_MCR_WR(prAdapter, prCr->rToN9IntToggle.u4Addr, u4Val);
}

void connac3x_set_ple_int_no_read(struct ADAPTER *prAdapter, bool fgTrigger,
			  uint32_t u4ClrMask, uint32_t u4SetMask)
{
	struct BUS_INFO *prBusInfo;
	struct PLE_TOP_CR *prCr;
	uint32_t u4Val = 0;

	prBusInfo = prAdapter->chip_info->bus_info;
	prCr = prBusInfo->prPleTopCr;

	if (fgTrigger) {
		u4Val = (~u4Val & prCr->rToN9IntToggle.u4Mask) |
			(u4Val & ~prCr->rToN9IntToggle.u4Mask);
	}

	u4Val &= ~u4ClrMask;
	u4Val |= u4SetMask;

	/* Write CR (WF_PLE_TOP_TO_N9_INT_ADDR) with 0x0000FFFF and
	 * Inverted Bit[31] and BIT[0:27] to trigger FW debug SOP
	*/
	HAL_MCR_WR(prAdapter, prCr->rToN9IntToggle.u4Addr, 0x0000FFFF);
	HAL_MCR_WR(prAdapter, prCr->rToN9IntToggle.u4Addr, u4Val);
}

#endif /*_HIF_PCIE || _HIF_AXI */

void connac3x_show_ple_info(struct ADAPTER *prAdapter, u_int8_t fgDumpTxd)
{
	uint32_t int_n9_sts = 0, int_n9_err_sts = 0, int_n9_err_sts_1 = 0;
	uint32_t ple_buf_ctrl = 0, pg_sz, pg_num = 0;
	uint32_t ple_stat[ALL_CR_NUM_OF_ALL_AC + 1] = {0}, pg_flow_ctrl[10] = {0};
	uint32_t dis_sta_map[CR_NUM_OF_AC] = {0};
	uint32_t fpg_cnt, ffa_cnt, fpg_head, fpg_tail, hif_max_q, hif_min_q;
	uint32_t rpg_hif, upg_hif, cpu_max_q, cpu_min_q, rpg_cpu, upg_cpu;
	uint32_t bn0_txd = 0, bn1_txd = 0, bn2_txd = 0;
	uint32_t i, j;

	HAL_MCR_RD(prAdapter, WF_PLE_TOP_INT_N9_STS_ADDR, &int_n9_sts);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_INT_N9_ERR_STS_ADDR, &int_n9_err_sts);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_INT_N9_ERR_STS_1_ADDR, &int_n9_err_sts_1);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_PBUF_CTRL_ADDR, &ple_buf_ctrl);
	chip_get_ple_acq_stat(prAdapter, ple_stat);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_FREEPG_CNT_ADDR, &pg_flow_ctrl[0]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_FREEPG_HEAD_TAIL_ADDR, &pg_flow_ctrl[1]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_PG_HIF_GROUP_ADDR, &pg_flow_ctrl[2]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_HIF_PG_INFO_ADDR, &pg_flow_ctrl[3]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_PG_CPU_GROUP_ADDR, &pg_flow_ctrl[4]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_CPU_PG_INFO_ADDR, &pg_flow_ctrl[5]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_PG_HIF_TXCMD_GROUP_ADDR, &pg_flow_ctrl[6]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_HIF_TXCMD_PG_INFO_ADDR, &pg_flow_ctrl[7]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_PG_HIF_WMTXD_GROUP_ADDR, &pg_flow_ctrl[8]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_HIF_WMTXD_PG_INFO_ADDR, &pg_flow_ctrl[9]);
	chip_get_dis_sta_map(prAdapter, dis_sta_map);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_TXD_QUEUE_EMPTY_ADDR, &bn0_txd);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_BN1_TXD_QUEUE_EMPTY_ADDR, &bn1_txd);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_BN2_TXD_QUEUE_EMPTY_ADDR, &bn2_txd);

	/* Configuration Info */
	DBGLOG(HAL, INFO, "PLE Configuration Info:\n");
	DBGLOG(HAL, INFO, "\tPacket Buffer Control(0x%08x): 0x%08x\n", WF_PLE_TOP_PBUF_CTRL_ADDR, ple_buf_ctrl);
	pg_sz = (ple_buf_ctrl & WF_PLE_TOP_PBUF_CTRL_PAGE_SIZE_CFG_MASK) >> WF_PLE_TOP_PBUF_CTRL_PAGE_SIZE_CFG_SHFT;
	DBGLOG(HAL, INFO, "\t\tPage Size=%d(%d bytes per page)\n", pg_sz, (pg_sz == 1 ? 128 : 64));
	DBGLOG(HAL, INFO, "\t\tPage Offset=%d(in unit of 2KB)\n",
		(ple_buf_ctrl & WF_PLE_TOP_PBUF_CTRL_PBUF_OFFSET_MASK) >> WF_PLE_TOP_PBUF_CTRL_PBUF_OFFSET_SHFT);
	pg_num = (ple_buf_ctrl & WF_PLE_TOP_PBUF_CTRL_TOTAL_PAGE_NUM_MASK) >> WF_PLE_TOP_PBUF_CTRL_TOTAL_PAGE_NUM_SHFT;
	DBGLOG(HAL, INFO, "\t\tTotal Page=%d pages\n", pg_num);
	for (i = 0; i <= 8; i++) {
		uint32_t addr = WF_PLE_TOP_PEEK_CR_00_ADDR + i * 4;
		uint32_t value = 0;

		HAL_MCR_RD(prAdapter, addr, &value);
		DBGLOG(HAL, INFO, "\tPEEK_CR_%02d(0x%08x): 0x%08x\n",
			i, addr, value);
	}
	for (i = 0; i < 2; i++) {
		uint32_t addr = WF_PLE_TOP_MACTX0_DBG0_ADDR + i * 4;
		uint32_t value = 0;

		HAL_MCR_RD(prAdapter, addr, &value);
		DBGLOG(HAL, INFO, "\tMACTX0_DBG%d(0x%08x): 0x%08x\n",
			i, addr, value);
	}
	for (i = 0; i < 2; i++) {
		uint32_t addr = WF_PLE_TOP_MACTX1_DBG0_ADDR + i * 4;
		uint32_t value = 0;

		HAL_MCR_RD(prAdapter, addr, &value);
		DBGLOG(HAL, INFO, "\tMACTX1_DBG%d(0x%08x): 0x%08x\n",
			i, addr, value);
	}
	DBGLOG(HAL, INFO, "\tINT_STS(0x%08x): 0x%08x, INT_ERR_STS(0x%08x): 0x%08x, INT_ERR_STS_1(0x%08x): 0x%08x\n",
		WF_PSE_TOP_INT_N9_STS_ADDR, int_n9_sts,
		WF_PSE_TOP_INT_N9_ERR_STS_ADDR, int_n9_err_sts,
		WF_PSE_TOP_INT_N9_ERR1_STS_ADDR, int_n9_err_sts_1);

	/* Page Flow Control */
	DBGLOG(HAL, INFO, "PLE Page Flow Control:\n");
	DBGLOG(HAL, INFO, "\tFree page counter: 0x%08x\n", pg_flow_ctrl[0]);
	fpg_cnt = (pg_flow_ctrl[0] & WF_PLE_TOP_FREEPG_CNT_FREEPG_CNT_MASK) >> WF_PLE_TOP_FREEPG_CNT_FREEPG_CNT_SHFT;
	DBGLOG(HAL, INFO, "\t\tThe toal page number of free=0x%03x\n", fpg_cnt);
	ffa_cnt = (pg_flow_ctrl[0] & WF_PLE_TOP_FREEPG_CNT_FFA_CNT_MASK) >> WF_PLE_TOP_FREEPG_CNT_FFA_CNT_SHFT;
	DBGLOG(HAL, INFO, "\t\tThe free page numbers of free for all=0x%03x\n", ffa_cnt);
	DBGLOG(HAL, INFO, "\tFree page head and tail: 0x%08x\n", pg_flow_ctrl[1]);
	fpg_head = (pg_flow_ctrl[1] & WF_PLE_TOP_FREEPG_HEAD_TAIL_FREEPG_HEAD_MASK) >> WF_PLE_TOP_FREEPG_HEAD_TAIL_FREEPG_HEAD_SHFT;
	fpg_tail = (pg_flow_ctrl[1] & WF_PLE_TOP_FREEPG_HEAD_TAIL_FREEPG_TAIL_MASK) >> WF_PLE_TOP_FREEPG_HEAD_TAIL_FREEPG_TAIL_SHFT;
	DBGLOG(HAL, INFO, "\t\tThe tail/head page of free page list=0x%03x/0x%03x\n", fpg_tail, fpg_head);
	DBGLOG(HAL, INFO, "\tReserved page counter of HIF group: 0x%08x\n", pg_flow_ctrl[2]);
	DBGLOG(HAL, INFO, "\tHIF group page status: 0x%08x\n", pg_flow_ctrl[3]);
	hif_min_q = (pg_flow_ctrl[2] & WF_PLE_TOP_PG_HIF_GROUP_HIF_MIN_QUOTA_MASK) >> WF_PLE_TOP_PG_HIF_GROUP_HIF_MIN_QUOTA_SHFT;
	hif_max_q = (pg_flow_ctrl[2] & WF_PLE_TOP_PG_HIF_GROUP_HIF_MAX_QUOTA_MASK) >> WF_PLE_TOP_PG_HIF_GROUP_HIF_MAX_QUOTA_SHFT;
	DBGLOG(HAL, INFO, "\t\tThe max/min quota pages of HIF group=0x%03x/0x%03x\n", hif_max_q, hif_min_q);
	rpg_hif = (pg_flow_ctrl[3] & WF_PLE_TOP_HIF_PG_INFO_HIF_RSV_CNT_MASK) >> WF_PLE_TOP_HIF_PG_INFO_HIF_RSV_CNT_SHFT;
	upg_hif = (pg_flow_ctrl[3] & WF_PLE_TOP_HIF_PG_INFO_HIF_SRC_CNT_MASK) >> WF_PLE_TOP_HIF_PG_INFO_HIF_SRC_CNT_SHFT;
	DBGLOG(HAL, INFO, "\t\tThe used/reserved pages of HIF group=0x%03x/0x%03x\n", upg_hif, rpg_hif);

	DBGLOG(HAL, INFO, "\tReserved page counter of WMTXD group: 0x%08x\n", pg_flow_ctrl[8]);
	DBGLOG(HAL, INFO, "\tWMTXD group page status: 0x%08x\n", pg_flow_ctrl[9]);
	cpu_min_q = (pg_flow_ctrl[8] & WF_PLE_TOP_PG_HIF_WMTXD_GROUP_HIF_WMTXD_MIN_QUOTA_MASK) >> WF_PLE_TOP_PG_HIF_WMTXD_GROUP_HIF_WMTXD_MIN_QUOTA_SHFT;
	cpu_max_q = (pg_flow_ctrl[8] & WF_PLE_TOP_PG_HIF_WMTXD_GROUP_HIF_WMTXD_MAX_QUOTA_MASK) >> WF_PLE_TOP_PG_HIF_WMTXD_GROUP_HIF_WMTXD_MAX_QUOTA_SHFT;
	DBGLOG(HAL, INFO, "\t\tThe max/min quota pages of WMTXD group=0x%03x/0x%03x\n", cpu_max_q, cpu_min_q);
	rpg_cpu = (pg_flow_ctrl[9] & WF_PLE_TOP_HIF_WMTXD_PG_INFO_HIF_WMTXD_RSV_CNT_MASK) >> WF_PLE_TOP_HIF_WMTXD_PG_INFO_HIF_WMTXD_RSV_CNT_SHFT;
	upg_cpu = (pg_flow_ctrl[9] & WF_PLE_TOP_HIF_WMTXD_PG_INFO_HIF_WMTXD_SRC_CNT_MASK) >> WF_PLE_TOP_HIF_WMTXD_PG_INFO_HIF_WMTXD_SRC_CNT_SHFT;
	DBGLOG(HAL, INFO, "\t\tThe used/reserved pages of WMTXD group=0x%03x/0x%03x\n", upg_cpu, rpg_cpu);

	DBGLOG(HAL, INFO, "\tReserved page counter of HIF_TXCMD group: 0x%08x\n", pg_flow_ctrl[6]);
	DBGLOG(HAL, INFO, "\tHIF_TXCMD group page status: 0x%08x\n", pg_flow_ctrl[7]);
	cpu_min_q = (pg_flow_ctrl[6] & WF_PLE_TOP_PG_HIF_TXCMD_GROUP_HIF_TXCMD_MIN_QUOTA_MASK) >> WF_PLE_TOP_PG_HIF_TXCMD_GROUP_HIF_TXCMD_MIN_QUOTA_SHFT;
	cpu_max_q = (pg_flow_ctrl[6] & WF_PLE_TOP_PG_HIF_TXCMD_GROUP_HIF_TXCMD_MAX_QUOTA_MASK) >> WF_PLE_TOP_PG_HIF_TXCMD_GROUP_HIF_TXCMD_MAX_QUOTA_SHFT;
	DBGLOG(HAL, INFO, "\t\tThe max/min quota pages of HIF_TXCMD group=0x%03x/0x%03x\n", cpu_max_q, cpu_min_q);
	rpg_cpu = (pg_flow_ctrl[7] & WF_PLE_TOP_HIF_TXCMD_PG_INFO_HIF_TXCMD_RSV_CNT_MASK) >> WF_PLE_TOP_HIF_TXCMD_PG_INFO_HIF_TXCMD_RSV_CNT_SHFT;
	upg_cpu = (pg_flow_ctrl[7] & WF_PLE_TOP_HIF_TXCMD_PG_INFO_HIF_TXCMD_SRC_CNT_MASK) >> WF_PLE_TOP_HIF_TXCMD_PG_INFO_HIF_TXCMD_SRC_CNT_SHFT;
	DBGLOG(HAL, INFO, "\t\tThe used/reserved pages of HIF_TXCMD group=0x%03x/0x%03x\n", upg_cpu, rpg_cpu);

	DBGLOG(HAL, INFO, "\tReserved page counter of CPU group(0x%08x): 0x%08x\n", WF_PLE_TOP_PG_CPU_GROUP_ADDR, pg_flow_ctrl[4]);
	DBGLOG(HAL, INFO, "\tCPU group page status(0x%08x): 0x%08x\n", WF_PLE_TOP_CPU_PG_INFO_ADDR, pg_flow_ctrl[5]);
	cpu_min_q = (pg_flow_ctrl[4] & WF_PLE_TOP_PG_CPU_GROUP_CPU_MIN_QUOTA_MASK) >> WF_PLE_TOP_PG_CPU_GROUP_CPU_MIN_QUOTA_SHFT;
	cpu_max_q = (pg_flow_ctrl[4] & WF_PLE_TOP_PG_CPU_GROUP_CPU_MAX_QUOTA_MASK) >> WF_PLE_TOP_PG_CPU_GROUP_CPU_MAX_QUOTA_SHFT;
	DBGLOG(HAL, INFO, "\t\tThe max/min quota pages of CPU group=0x%03x/0x%03x\n", cpu_max_q, cpu_min_q);
	rpg_cpu = (pg_flow_ctrl[5] & WF_PLE_TOP_CPU_PG_INFO_CPU_RSV_CNT_MASK) >> WF_PLE_TOP_CPU_PG_INFO_CPU_RSV_CNT_SHFT;
	upg_cpu = (pg_flow_ctrl[5] & WF_PLE_TOP_CPU_PG_INFO_CPU_SRC_CNT_MASK) >> WF_PLE_TOP_CPU_PG_INFO_CPU_SRC_CNT_SHFT;
	DBGLOG(HAL, INFO, "\t\tThe used/reserved pages of CPU group=0x%03x/0x%03x\n", upg_cpu, rpg_cpu);

	DBGLOG(HAL, INFO, "\tBN_0_TXD(0x%08x): 0x%08x, BN_1_TXD(0x%08x): 0x%08x, BN_2_TXD(0x%08x): 0x%08x\n",
		WF_PLE_TOP_TXD_QUEUE_EMPTY_ADDR, bn0_txd,
		WF_PLE_TOP_BN1_TXD_QUEUE_EMPTY_ADDR, bn1_txd,
		WF_PLE_TOP_BN2_TXD_QUEUE_EMPTY_ADDR, bn2_txd);

	if ((ple_stat[0] & WF_PLE_TOP_QUEUE_EMPTY_ALL_AC_EMPTY_MASK) == 0) {
		for (j = 0; j < ALL_CR_NUM_OF_ALL_AC; j++) {
			for (i = 0; i < 32; i++) {
				uint32_t ctrl = 0;

				if (((ple_stat[j + 1] & (0x1 << i)) >> i) != 0)
					continue;

				if (((dis_sta_map[j % CR_NUM_OF_AC] & 0x1 << i) >> i) == 1)
					ctrl = 1;

				DBGLOG(HAL, INFO, "\tNonempty AC%d Q of STA#: %d, ctrl = %s\n",
					j / CR_NUM_OF_AC,
					i + (j % CR_NUM_OF_AC) * 32,
					sta_ctrl_reg[ctrl]);
			}
		}
	}

	DBGLOG(HAL, INFO, "Nonempty Q info:\n");
	for (i = 0; i < 32; i++) {
		if (((ple_stat[0] & (0x1 << i)) >> i) == 0) {
			if (ple_queue_empty_info[i].QueueName != NULL)
				DBGLOG(HAL, INFO, "\t%s: ", ple_queue_empty_info[i].QueueName);
		}
	}
}

void connac3x_show_pse_info(struct ADAPTER *prAdapter)
{
	uint32_t int_n9_sts = 0, int_n9_err_sts = 0, int_n9_err_sts_1 = 0;
	uint32_t pse_buf_ctrl = 0, pg_sz, pg_num = 0;
	uint32_t que_empty = 0, que_empty1 = 0, que_empty_mask = 0;
	uint32_t fpg_cnt, ffa_cnt, fpg_head, fpg_tail;
	uint32_t max_q, min_q, rsv_pg, used_pg;
	uint32_t mdp_grp = 0, mdp_grp_info = 0;
	uint32_t hif_grp = 0, hif_grp_info = 0;
	uint32_t cpu_grp = 0, cpu_grp_info = 0;
	uint32_t lmac_grp = 0, lmac_grp_info = 0;
	uint32_t ple_grp = 0, ple_grp_info = 0;
	uint32_t value = 0;
	uint32_t i;

	HAL_MCR_RD(prAdapter, WF_PSE_TOP_INT_N9_STS_ADDR, &int_n9_sts);
	HAL_MCR_RD(prAdapter, WF_PSE_TOP_INT_N9_ERR_STS_ADDR, &int_n9_err_sts);
	HAL_MCR_RD(prAdapter, WF_PSE_TOP_INT_N9_ERR1_STS_ADDR, &int_n9_err_sts_1);
	HAL_MCR_RD(prAdapter, WF_PSE_TOP_PBUF_CTRL_ADDR, &pse_buf_ctrl);
	HAL_MCR_RD(prAdapter, WF_PSE_TOP_QUEUE_EMPTY_ADDR, &que_empty);
	HAL_MCR_RD(prAdapter, WF_PSE_TOP_QUEUE_EMPTY_1_ADDR, &que_empty1);
	HAL_MCR_RD(prAdapter, WF_PSE_TOP_QUEUE_EMPTY_MASK_ADDR, &que_empty_mask);

	/* Configuration Info */
	DBGLOG(HAL, INFO, "PSE Configuration Info:\n");
	DBGLOG(HAL, INFO, "\tPacket Buffer Control: 0x%08x\n", pse_buf_ctrl);
	pg_sz = (pse_buf_ctrl & WF_PSE_TOP_PBUF_CTRL_PAGE_SIZE_CFG_MASK) >> WF_PSE_TOP_PBUF_CTRL_PAGE_SIZE_CFG_SHFT;
	DBGLOG(HAL, INFO, "\t\tPage Size=%d(%d bytes per page)\n", pg_sz, (pg_sz == 1 ? 256 : 128));
	DBGLOG(HAL, INFO, "\t\tPage Offset=%d(in unit of 64KB)\n",
			 (pse_buf_ctrl & WF_PSE_TOP_PBUF_CTRL_PBUF_OFFSET_MASK) >> WF_PSE_TOP_PBUF_CTRL_PBUF_OFFSET_SHFT);
	pg_num = (pse_buf_ctrl & WF_PSE_TOP_PBUF_CTRL_TOTAL_PAGE_NUM_MASK) >> WF_PSE_TOP_PBUF_CTRL_TOTAL_PAGE_NUM_SHFT;
	DBGLOG(HAL, INFO, "\t\tTotal page numbers=%d pages\n", pg_num);
	for (i = 0; i <= 10; i++) {
		uint32_t addr = WF_PSE_TOP_PSE_SEEK_CR_00_ADDR + i * 4;
		uint32_t value = 0;

		HAL_MCR_RD(prAdapter, addr, &value);
		DBGLOG(HAL, INFO, "\tSEEK_CR_%02d(0x%08x): 0x%08x\n",
			i, addr, value);
	}
	DBGLOG(HAL, INFO, "\tINT_STS(0x%08x): 0x%08x, INT_ERR_STS(0x%08x): 0x%08x, INT_ERR_STS1(0x%08x): 0x%08x\n",
		WF_PSE_TOP_INT_N9_STS_ADDR, int_n9_sts,
		WF_PSE_TOP_INT_N9_ERR_STS_ADDR, int_n9_err_sts,
		WF_PSE_TOP_INT_N9_ERR1_STS_ADDR, int_n9_err_sts_1);
	/* Page Flow Control */
	DBGLOG(HAL, INFO, "PSE Page Flow Control:\n");
	HAL_MCR_RD(prAdapter, WF_PSE_TOP_FREEPG_CNT_ADDR, &value);
	DBGLOG(HAL, INFO, "\tFree page counter: 0x%08x\n", value);
	fpg_cnt = (value & WF_PSE_TOP_FREEPG_CNT_FREEPG_CNT_MASK) >> WF_PSE_TOP_FREEPG_CNT_FREEPG_CNT_SHFT;
	DBGLOG(HAL, INFO, "\t\tThe toal page number of free=0x%03x\n", fpg_cnt);
	ffa_cnt = (value & WF_PSE_TOP_FREEPG_CNT_FFA_CNT_MASK) >> WF_PSE_TOP_FREEPG_CNT_FFA_CNT_SHFT;
	DBGLOG(HAL, INFO, "\t\tThe free page numbers of free for all=0x%03x\n", ffa_cnt);

	HAL_MCR_RD(prAdapter, WF_PSE_TOP_FREEPG_HEAD_TAIL_ADDR, &value);
	DBGLOG(HAL, INFO, "\tFree page head and tail: 0x%08x\n", value);
	fpg_head = (value & WF_PSE_TOP_FREEPG_HEAD_TAIL_FREEPG_HEAD_MASK) >> WF_PSE_TOP_FREEPG_HEAD_TAIL_FREEPG_HEAD_SHFT;
	fpg_tail = (value & WF_PSE_TOP_FREEPG_HEAD_TAIL_FREEPG_TAIL_MASK) >> WF_PSE_TOP_FREEPG_HEAD_TAIL_FREEPG_TAIL_SHFT;
	DBGLOG(HAL, INFO, "\t\tThe tail/head page of free page list=0x%03x/0x%03x\n", fpg_tail, fpg_head);

	HAL_MCR_RD(prAdapter, WF_PSE_TOP_PG_HIF0_GROUP_ADDR, &hif_grp);
	HAL_MCR_RD(prAdapter, WF_PSE_TOP_HIF0_PG_INFO_ADDR, &hif_grp_info);
	DBGLOG(HAL, INFO, "\tReserved page counter of HIF0 group: 0x%08x\n", hif_grp);
	DBGLOG(HAL, INFO, "\tHIF0 group page status: 0x%08x\n", hif_grp_info);
	min_q = (hif_grp & WF_PSE_TOP_PG_HIF0_GROUP_HIF0_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_HIF0_GROUP_HIF0_MIN_QUOTA_SHFT;
	max_q = (hif_grp & WF_PSE_TOP_PG_HIF0_GROUP_HIF0_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_HIF0_GROUP_HIF0_MAX_QUOTA_SHFT;
	DBGLOG(HAL, INFO, "\t\tThe max/min quota pages of HIF0 group=0x%03x/0x%03x\n", max_q, min_q);
	rsv_pg = (hif_grp_info & WF_PSE_TOP_HIF0_PG_INFO_HIF0_RSV_CNT_MASK) >> WF_PSE_TOP_HIF0_PG_INFO_HIF0_RSV_CNT_SHFT;
	used_pg = (hif_grp_info & WF_PSE_TOP_HIF0_PG_INFO_HIF0_SRC_CNT_MASK) >> WF_PSE_TOP_HIF0_PG_INFO_HIF0_SRC_CNT_SHFT;
	DBGLOG(HAL, INFO, "\t\tThe used/reserved pages of HIF0 group=0x%03x/0x%03x\n", used_pg, rsv_pg);

	HAL_MCR_RD(prAdapter, WF_PSE_TOP_PG_HIF1_GROUP_ADDR, &hif_grp);
	HAL_MCR_RD(prAdapter, WF_PSE_TOP_HIF1_PG_INFO_ADDR, &hif_grp_info);
	DBGLOG(HAL, INFO, "\tReserved page counter of HIF1 group: 0x%08x\n", hif_grp);
	DBGLOG(HAL, INFO, "\tHIF1 group page status: 0x%08x\n", hif_grp_info);
	min_q = (hif_grp & WF_PSE_TOP_PG_HIF1_GROUP_HIF1_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_HIF1_GROUP_HIF1_MIN_QUOTA_SHFT;
	max_q = (hif_grp & WF_PSE_TOP_PG_HIF1_GROUP_HIF1_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_HIF1_GROUP_HIF1_MAX_QUOTA_SHFT;
	DBGLOG(HAL, INFO, "\t\tThe max/min quota pages of HIF1 group=0x%03x/0x%03x\n", max_q, min_q);
	rsv_pg = (hif_grp_info & WF_PSE_TOP_HIF1_PG_INFO_HIF1_RSV_CNT_MASK) >> WF_PSE_TOP_HIF1_PG_INFO_HIF1_RSV_CNT_SHFT;
	used_pg = (hif_grp_info & WF_PSE_TOP_HIF1_PG_INFO_HIF1_SRC_CNT_MASK) >> WF_PSE_TOP_HIF1_PG_INFO_HIF1_SRC_CNT_SHFT;
	DBGLOG(HAL, INFO, "\t\tThe used/reserved pages of HIF1 group=0x%03x/0x%03x\n", used_pg, rsv_pg);

	HAL_MCR_RD(prAdapter, WF_PSE_TOP_PG_HIF2_GROUP_ADDR, &hif_grp);
	HAL_MCR_RD(prAdapter, WF_PSE_TOP_HIF2_PG_INFO_ADDR, &hif_grp_info);
	DBGLOG(HAL, INFO, "\tReserved page counter of HIF2 group: 0x%08x\n", hif_grp);
	DBGLOG(HAL, INFO, "\tHIF2 group page status: 0x%08x\n", hif_grp_info);
	min_q = (hif_grp & WF_PSE_TOP_PG_HIF2_GROUP_HIF2_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_HIF2_GROUP_HIF2_MIN_QUOTA_SHFT;
	max_q = (hif_grp & WF_PSE_TOP_PG_HIF2_GROUP_HIF2_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_HIF2_GROUP_HIF2_MAX_QUOTA_SHFT;
	DBGLOG(HAL, INFO, "\t\tThe max/min quota pages of HIF2 group=0x%03x/0x%03x\n", max_q, min_q);
	rsv_pg = (hif_grp_info & WF_PSE_TOP_HIF2_PG_INFO_HIF2_RSV_CNT_MASK) >> WF_PSE_TOP_HIF2_PG_INFO_HIF2_RSV_CNT_SHFT;
	used_pg = (hif_grp_info & WF_PSE_TOP_HIF2_PG_INFO_HIF2_SRC_CNT_MASK) >> WF_PSE_TOP_HIF2_PG_INFO_HIF2_SRC_CNT_SHFT;
	DBGLOG(HAL, INFO, "\t\tThe used/reserved pages of HIF2 group=0x%03x/0x%03x\n", used_pg, rsv_pg);

	HAL_MCR_RD(prAdapter, WF_PSE_TOP_PG_CPU_GROUP_ADDR, &cpu_grp);
	HAL_MCR_RD(prAdapter, WF_PSE_TOP_CPU_PG_INFO_ADDR, &cpu_grp_info);
	DBGLOG(HAL, INFO, "\tReserved page counter of CPU group: 0x%08x\n", cpu_grp);
	DBGLOG(HAL, INFO, "\tCPU group page status: 0x%08x\n", cpu_grp_info);
	min_q = (cpu_grp & WF_PSE_TOP_PG_CPU_GROUP_CPU_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_CPU_GROUP_CPU_MIN_QUOTA_SHFT;
	max_q = (cpu_grp & WF_PSE_TOP_PG_CPU_GROUP_CPU_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_CPU_GROUP_CPU_MAX_QUOTA_SHFT;
	DBGLOG(HAL, INFO, "\t\tThe max/min quota pages of CPU group=0x%03x/0x%03x\n", max_q, min_q);
	rsv_pg = (cpu_grp_info & WF_PSE_TOP_CPU_PG_INFO_CPU_RSV_CNT_MASK) >> WF_PSE_TOP_CPU_PG_INFO_CPU_RSV_CNT_SHFT;
	used_pg = (cpu_grp_info & WF_PSE_TOP_CPU_PG_INFO_CPU_SRC_CNT_MASK) >> WF_PSE_TOP_CPU_PG_INFO_CPU_SRC_CNT_SHFT;
	DBGLOG(HAL, INFO, "\t\tThe used/reserved pages of CPU group=0x%03x/0x%03x\n", used_pg, rsv_pg);

	HAL_MCR_RD(prAdapter, WF_PSE_TOP_PG_LMAC0_GROUP_ADDR, &lmac_grp);
	HAL_MCR_RD(prAdapter, WF_PSE_TOP_LMAC0_PG_INFO_ADDR, &lmac_grp_info);
	DBGLOG(HAL, INFO, "\tReserved page counter of LMAC0 group: 0x%08x\n", lmac_grp);
	DBGLOG(HAL, INFO, "\tLMAC0 group page status: 0x%08x\n", lmac_grp_info);
	min_q = (lmac_grp & WF_PSE_TOP_PG_LMAC0_GROUP_LMAC0_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_LMAC0_GROUP_LMAC0_MIN_QUOTA_SHFT;
	max_q = (lmac_grp & WF_PSE_TOP_PG_LMAC0_GROUP_LMAC0_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_LMAC0_GROUP_LMAC0_MAX_QUOTA_SHFT;
	DBGLOG(HAL, INFO, "\t\tThe max/min quota pages of LMAC0 group=0x%03x/0x%03x\n", max_q, min_q);
	rsv_pg = (lmac_grp_info & WF_PSE_TOP_LMAC0_PG_INFO_LMAC0_RSV_CNT_MASK) >> WF_PSE_TOP_LMAC0_PG_INFO_LMAC0_RSV_CNT_SHFT;
	used_pg = (lmac_grp_info & WF_PSE_TOP_LMAC0_PG_INFO_LMAC0_SRC_CNT_MASK) >> WF_PSE_TOP_LMAC0_PG_INFO_LMAC0_SRC_CNT_SHFT;
	DBGLOG(HAL, INFO, "\t\tThe used/reserved pages of LMAC0 group=0x%03x/0x%03x\n", used_pg, rsv_pg);

	HAL_MCR_RD(prAdapter, WF_PSE_TOP_PG_LMAC1_GROUP_ADDR, &lmac_grp);
	HAL_MCR_RD(prAdapter, WF_PSE_TOP_LMAC1_PG_INFO_ADDR, &lmac_grp_info);
	DBGLOG(HAL, INFO, "\tReserved page counter of LMAC1 group: 0x%08x\n", lmac_grp);
	DBGLOG(HAL, INFO, "\tLMAC1 group page status: 0x%08x\n", lmac_grp_info);
	min_q = (lmac_grp & WF_PSE_TOP_PG_LMAC1_GROUP_LMAC1_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_LMAC1_GROUP_LMAC1_MIN_QUOTA_SHFT;
	max_q = (lmac_grp & WF_PSE_TOP_PG_LMAC1_GROUP_LMAC1_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_LMAC1_GROUP_LMAC1_MAX_QUOTA_SHFT;
	DBGLOG(HAL, INFO, "\t\tThe max/min quota pages of LMAC1 group=0x%03x/0x%03x\n", max_q, min_q);
	rsv_pg = (lmac_grp_info & WF_PSE_TOP_LMAC1_PG_INFO_LMAC1_RSV_CNT_MASK) >> WF_PSE_TOP_LMAC1_PG_INFO_LMAC1_RSV_CNT_SHFT;
	used_pg = (lmac_grp_info & WF_PSE_TOP_LMAC1_PG_INFO_LMAC1_SRC_CNT_MASK) >> WF_PSE_TOP_LMAC1_PG_INFO_LMAC1_SRC_CNT_SHFT;
	DBGLOG(HAL, INFO, "\t\tThe used/reserved pages of LMAC1 group=0x%03x/0x%03x\n", used_pg, rsv_pg);

	HAL_MCR_RD(prAdapter, WF_PSE_TOP_PG_LMAC2_GROUP_ADDR, &lmac_grp);
	HAL_MCR_RD(prAdapter, WF_PSE_TOP_LMAC2_PG_INFO_ADDR, &lmac_grp_info);
	DBGLOG(HAL, INFO, "\tReserved page counter of LMAC2 group: 0x%08x\n", lmac_grp);
	DBGLOG(HAL, INFO, "\tLMAC2 group page status: 0x%08x\n", lmac_grp_info);
	min_q = (lmac_grp & WF_PSE_TOP_PG_LMAC2_GROUP_LMAC2_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_LMAC2_GROUP_LMAC2_MIN_QUOTA_SHFT;
	max_q = (lmac_grp & WF_PSE_TOP_PG_LMAC2_GROUP_LMAC2_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_LMAC2_GROUP_LMAC2_MAX_QUOTA_SHFT;
	DBGLOG(HAL, INFO, "\t\tThe max/min quota pages of LMAC2 group=0x%03x/0x%03x\n", max_q, min_q);
	rsv_pg = (lmac_grp_info & WF_PSE_TOP_LMAC2_PG_INFO_LMAC2_RSV_CNT_MASK) >> WF_PSE_TOP_LMAC2_PG_INFO_LMAC2_RSV_CNT_SHFT;
	used_pg = (lmac_grp_info & WF_PSE_TOP_LMAC2_PG_INFO_LMAC2_SRC_CNT_MASK) >> WF_PSE_TOP_LMAC2_PG_INFO_LMAC2_SRC_CNT_SHFT;
	DBGLOG(HAL, INFO, "\t\tThe used/reserved pages of LMAC2 group=0x%03x/0x%03x\n", used_pg, rsv_pg);

	HAL_MCR_RD(prAdapter, WF_PSE_TOP_PG_LMAC3_GROUP_ADDR, &lmac_grp);
	HAL_MCR_RD(prAdapter, WF_PSE_TOP_LMAC3_PG_INFO_ADDR, &lmac_grp_info);
	DBGLOG(HAL, INFO, "\tReserved page counter of LMAC3 group: 0x%08x\n", lmac_grp);
	DBGLOG(HAL, INFO, "\tLMAC3 group page status: 0x%08x\n", lmac_grp_info);
	min_q = (lmac_grp & WF_PSE_TOP_PG_LMAC3_GROUP_LMAC3_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_LMAC3_GROUP_LMAC3_MIN_QUOTA_SHFT;
	max_q = (lmac_grp & WF_PSE_TOP_PG_LMAC3_GROUP_LMAC3_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_LMAC3_GROUP_LMAC3_MAX_QUOTA_SHFT;
	DBGLOG(HAL, INFO, "\t\tThe max/min quota pages of LMAC3 group=0x%03x/0x%03x\n", max_q, min_q);
	rsv_pg = (lmac_grp_info & WF_PSE_TOP_LMAC3_PG_INFO_LMAC3_RSV_CNT_MASK) >> WF_PSE_TOP_LMAC3_PG_INFO_LMAC3_RSV_CNT_SHFT;
	used_pg = (lmac_grp_info & WF_PSE_TOP_LMAC3_PG_INFO_LMAC3_SRC_CNT_MASK) >> WF_PSE_TOP_LMAC3_PG_INFO_LMAC3_SRC_CNT_SHFT;
	DBGLOG(HAL, INFO, "\t\tThe used/reserved pages of LMAC3 group=0x%03x/0x%03x\n", used_pg, rsv_pg);

	HAL_MCR_RD(prAdapter, WF_PSE_TOP_PG_PLE_GROUP_ADDR, &ple_grp);
	HAL_MCR_RD(prAdapter, WF_PSE_TOP_PLE_PG_INFO_ADDR, &ple_grp_info);
	DBGLOG(HAL, INFO, "\tReserved page counter of PLE group: 0x%08x\n", ple_grp);
	DBGLOG(HAL, INFO, "\tPLE group page status: 0x%08x\n", ple_grp_info);
	min_q = (ple_grp & WF_PSE_TOP_PG_PLE_GROUP_PLE_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_PLE_GROUP_PLE_MIN_QUOTA_SHFT;
	max_q = (ple_grp & WF_PSE_TOP_PG_PLE_GROUP_PLE_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_PLE_GROUP_PLE_MAX_QUOTA_SHFT;
	DBGLOG(HAL, INFO, "\t\tThe max/min quota pages of PLE group=0x%03x/0x%03x\n", max_q, min_q);
	rsv_pg = (ple_grp_info & WF_PSE_TOP_PLE_PG_INFO_PLE_RSV_CNT_MASK) >> WF_PSE_TOP_PLE_PG_INFO_PLE_RSV_CNT_SHFT;
	used_pg = (ple_grp_info & WF_PSE_TOP_PLE_PG_INFO_PLE_SRC_CNT_MASK) >> WF_PSE_TOP_PLE_PG_INFO_PLE_SRC_CNT_SHFT;
	DBGLOG(HAL, INFO, "\t\tThe used/reserved pages of PLE group=0x%03x/0x%03x\n", used_pg, rsv_pg);

	HAL_MCR_RD(prAdapter, WF_PSE_TOP_PG_PLE1_GROUP_ADDR, &ple_grp);
	HAL_MCR_RD(prAdapter, WF_PSE_TOP_PLE1_PG_INFO_ADDR, &ple_grp_info);
	DBGLOG(HAL, INFO, "\tReserved page counter of PLE1 group: 0x%08x\n", ple_grp);
	DBGLOG(HAL, INFO, "\tPLE1 group page status: 0x%08x\n", ple_grp_info);
	min_q = (ple_grp & WF_PSE_TOP_PG_PLE_GROUP_PLE_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_PLE_GROUP_PLE_MIN_QUOTA_SHFT;
	max_q = (ple_grp & WF_PSE_TOP_PG_PLE_GROUP_PLE_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_PLE_GROUP_PLE_MAX_QUOTA_SHFT;
	DBGLOG(HAL, INFO, "\t\tThe max/min quota pages of PLE1 group=0x%03x/0x%03x\n", max_q, min_q);
	rsv_pg = (ple_grp_info & WF_PSE_TOP_PLE_PG_INFO_PLE_RSV_CNT_MASK) >> WF_PSE_TOP_PLE_PG_INFO_PLE_RSV_CNT_SHFT;
	used_pg = (ple_grp_info & WF_PSE_TOP_PLE_PG_INFO_PLE_SRC_CNT_MASK) >> WF_PSE_TOP_PLE_PG_INFO_PLE_SRC_CNT_SHFT;
	DBGLOG(HAL, INFO, "\t\tThe used/reserved pages of PLE1 group=0x%03x/0x%03x\n", used_pg, rsv_pg);

	HAL_MCR_RD(prAdapter, WF_PSE_TOP_PG_MDP_GROUP_ADDR, &mdp_grp);
	HAL_MCR_RD(prAdapter, WF_PSE_TOP_MDP_PG_INFO_ADDR, &mdp_grp_info);
	DBGLOG(HAL, INFO, "\tReserved page counter of MDP group: 0x%08x\n", mdp_grp);
	DBGLOG(HAL, INFO, "\tMDP group page status: 0x%08x\n", mdp_grp_info);
	min_q = (mdp_grp & WF_PSE_TOP_PG_MDP_GROUP_MDP_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_MDP_GROUP_MDP_MIN_QUOTA_SHFT;
	max_q = (mdp_grp & WF_PSE_TOP_PG_MDP_GROUP_MDP_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_MDP_GROUP_MDP_MAX_QUOTA_SHFT;
	DBGLOG(HAL, INFO, "\t\tThe max/min quota pages of MDP group=0x%03x/0x%03x\n", max_q, min_q);
	rsv_pg = (mdp_grp_info & WF_PSE_TOP_MDP_PG_INFO_MDP_RSV_CNT_MASK) >> WF_PSE_TOP_MDP_PG_INFO_MDP_RSV_CNT_SHFT;
	used_pg = (mdp_grp_info & WF_PSE_TOP_MDP_PG_INFO_MDP_SRC_CNT_MASK) >> WF_PSE_TOP_MDP_PG_INFO_MDP_SRC_CNT_SHFT;
	DBGLOG(HAL, INFO, "\t\tThe used/reserved pages of MDP group=0x%03x/0x%03x\n", used_pg, rsv_pg);

	HAL_MCR_RD(prAdapter, WF_PSE_TOP_PG_MDP1_GROUP_ADDR, &mdp_grp);
	HAL_MCR_RD(prAdapter, WF_PSE_TOP_MDP1_PG_INFO_ADDR, &mdp_grp_info);
	DBGLOG(HAL, INFO, "\tReserved page counter of MDP1 group: 0x%08x\n", mdp_grp);
	DBGLOG(HAL, INFO, "\tMDP group page status: 0x%08x\n", mdp_grp_info);
	min_q = (mdp_grp & WF_PSE_TOP_PG_MDP1_GROUP_MDP1_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_MDP1_GROUP_MDP1_MIN_QUOTA_SHFT;
	max_q = (mdp_grp & WF_PSE_TOP_PG_MDP1_GROUP_MDP1_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_MDP1_GROUP_MDP1_MAX_QUOTA_SHFT;
	DBGLOG(HAL, INFO, "\t\tThe max/min quota pages of MDP1 group=0x%03x/0x%03x\n", max_q, min_q);
	rsv_pg = (mdp_grp_info & WF_PSE_TOP_MDP1_PG_INFO_MDP1_RSV_CNT_MASK) >> WF_PSE_TOP_MDP1_PG_INFO_MDP1_RSV_CNT_SHFT;
	used_pg = (mdp_grp_info & WF_PSE_TOP_MDP1_PG_INFO_MDP1_SRC_CNT_MASK) >> WF_PSE_TOP_MDP1_PG_INFO_MDP1_SRC_CNT_SHFT;
	DBGLOG(HAL, INFO, "\t\tThe used/reserved pages of MDP1 group=0x%03x/0x%03x\n", used_pg, rsv_pg);

	HAL_MCR_RD(prAdapter, WF_PSE_TOP_PG_MDP2_GROUP_ADDR, &mdp_grp);
	HAL_MCR_RD(prAdapter, WF_PSE_TOP_MDP2_PG_INFO_ADDR, &mdp_grp_info);
	DBGLOG(HAL, INFO, "\tReserved page counter of MDP2 group: 0x%08x\n", mdp_grp);
	DBGLOG(HAL, INFO, "\tMDP group page status: 0x%08x\n", mdp_grp_info);
	min_q = (mdp_grp & WF_PSE_TOP_PG_MDP2_GROUP_MDP2_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_MDP2_GROUP_MDP2_MIN_QUOTA_SHFT;
	max_q = (mdp_grp & WF_PSE_TOP_PG_MDP2_GROUP_MDP2_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_MDP2_GROUP_MDP2_MAX_QUOTA_SHFT;
	DBGLOG(HAL, INFO, "\t\tThe max/min quota pages of MDP2 group=0x%03x/0x%03x\n", max_q, min_q);
	rsv_pg = (mdp_grp_info & WF_PSE_TOP_MDP2_PG_INFO_MDP2_RSV_CNT_MASK) >> WF_PSE_TOP_MDP2_PG_INFO_MDP2_RSV_CNT_SHFT;
	used_pg = (mdp_grp_info & WF_PSE_TOP_MDP2_PG_INFO_MDP2_SRC_CNT_MASK) >> WF_PSE_TOP_MDP2_PG_INFO_MDP2_SRC_CNT_SHFT;
	DBGLOG(HAL, INFO, "\t\tThe used/reserved pages of MDP2 group=0x%03x/0x%03x\n", used_pg, rsv_pg);

#if defined(BELLWETHER) || defined(MT7990)
	HAL_MCR_RD(prAdapter, WF_PSE_TOP_PG_MDP3_GROUP_ADDR, &mdp_grp);
	HAL_MCR_RD(prAdapter, WF_PSE_TOP_MDP3_PG_INFO_ADDR, &mdp_grp_info);
	DBGLOG(HAL, INFO, "\tReserved page counter of MDP3 group: 0x%08x\n", mdp_grp);
	DBGLOG(HAL, INFO, "\tMDP group page status: 0x%08x\n", mdp_grp_info);
	min_q = (mdp_grp & WF_PSE_TOP_PG_MDP3_GROUP_MDP3_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_MDP3_GROUP_MDP3_MIN_QUOTA_SHFT;
	max_q = (mdp_grp & WF_PSE_TOP_PG_MDP3_GROUP_MDP3_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_MDP3_GROUP_MDP3_MAX_QUOTA_SHFT;
	DBGLOG(HAL, INFO, "\t\tThe max/min quota pages of MDP3 group=0x%03x/0x%03x\n", max_q, min_q);
	rsv_pg = (mdp_grp_info & WF_PSE_TOP_MDP3_PG_INFO_MDP3_RSV_CNT_MASK) >> WF_PSE_TOP_MDP3_PG_INFO_MDP3_RSV_CNT_SHFT;
	used_pg = (mdp_grp_info & WF_PSE_TOP_MDP3_PG_INFO_MDP3_SRC_CNT_MASK) >> WF_PSE_TOP_MDP3_PG_INFO_MDP3_SRC_CNT_SHFT;
	DBGLOG(HAL, INFO, "\t\tThe used/reserved pages of MDP3 group=0x%03x/0x%03x\n", used_pg, rsv_pg);
#endif

	/* Queue Empty Status */
	DBGLOG(HAL, INFO, "PSE Queue Empty Status:\n");
	DBGLOG(HAL, INFO, "\tQUEUE_EMPTY: 0x%08x, QUEUE_EMPTY1: 0x%08x, QUEUE_EMPTY_MASK: 0x%08x\n",
		que_empty, que_empty1, que_empty_mask);
	DBGLOG(HAL, INFO, "\t\tCPU Q0/1/2/3/4 empty=%d/%d/%d/%d/%d\n",
			  (que_empty & WF_PSE_TOP_QUEUE_EMPTY_CPU_Q0_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_CPU_Q0_EMPTY_SHFT,
			  ((que_empty & WF_PSE_TOP_QUEUE_EMPTY_CPU_Q1_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_CPU_Q1_EMPTY_SHFT),
			  ((que_empty & WF_PSE_TOP_QUEUE_EMPTY_CPU_Q2_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_CPU_Q2_EMPTY_SHFT),
			  ((que_empty & WF_PSE_TOP_QUEUE_EMPTY_CPU_Q3_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_CPU_Q3_EMPTY_SHFT),
			  ((que_empty & WF_PSE_TOP_QUEUE_EMPTY_CPU_Q4_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_CPU_Q4_EMPTY_SHFT));
	DBGLOG(HAL, INFO, "\t\tHIF Q0/1/2/3/4/5/6/7/8/9/10/11/12/13 empty=%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d\n",
			  ((que_empty1 & WF_PSE_TOP_QUEUE_EMPTY_1_HIF_0_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_1_HIF_0_EMPTY_SHFT),
			  ((que_empty1 & WF_PSE_TOP_QUEUE_EMPTY_1_HIF_1_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_1_HIF_1_EMPTY_SHFT),
			  ((que_empty1 & WF_PSE_TOP_QUEUE_EMPTY_1_HIF_2_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_1_HIF_2_EMPTY_SHFT),
			  ((que_empty1 & WF_PSE_TOP_QUEUE_EMPTY_1_HIF_3_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_1_HIF_3_EMPTY_SHFT),
			  ((que_empty1 & WF_PSE_TOP_QUEUE_EMPTY_1_HIF_4_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_1_HIF_4_EMPTY_SHFT),
			  ((que_empty1 & WF_PSE_TOP_QUEUE_EMPTY_1_HIF_5_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_1_HIF_5_EMPTY_SHFT),
			  ((que_empty1 & WF_PSE_TOP_QUEUE_EMPTY_1_HIF_6_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_1_HIF_6_EMPTY_SHFT),
			  ((que_empty1 & WF_PSE_TOP_QUEUE_EMPTY_1_HIF_7_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_1_HIF_7_EMPTY_SHFT),
			  ((que_empty1 & WF_PSE_TOP_QUEUE_EMPTY_1_HIF_8_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_1_HIF_8_EMPTY_SHFT),
			  ((que_empty1 & WF_PSE_TOP_QUEUE_EMPTY_1_HIF_9_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_1_HIF_9_EMPTY_SHFT),
			  ((que_empty1 & WF_PSE_TOP_QUEUE_EMPTY_1_HIF_10_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_1_HIF_10_EMPTY_SHFT),
			  ((que_empty1 & WF_PSE_TOP_QUEUE_EMPTY_1_HIF_11_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_1_HIF_11_EMPTY_SHFT),
			  ((que_empty1 & WF_PSE_TOP_QUEUE_EMPTY_1_HIF_12_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_1_HIF_12_EMPTY_SHFT),
			  ((que_empty1 & WF_PSE_TOP_QUEUE_EMPTY_1_HIF_13_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_1_HIF_13_EMPTY_SHFT));
	DBGLOG(HAL, INFO, "\t\tLMAC TX Q empty=%d\n",
			  ((que_empty & WF_PSE_TOP_QUEUE_EMPTY_LMAC_TX_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_LMAC_TX_QUEUE_EMPTY_SHFT));
	DBGLOG(HAL, INFO, "\t\tMDP TX Q0/Q1/Q2/RX Q empty=%d/%d/%d/%d\n",
			  ((que_empty & WF_PSE_TOP_QUEUE_EMPTY_MDP_TX_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_MDP_TX_QUEUE_EMPTY_SHFT),
			  ((que_empty & WF_PSE_TOP_QUEUE_EMPTY_MDP_TX1_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_MDP_TX1_QUEUE_EMPTY_SHFT),
			  ((que_empty1 & WF_PSE_TOP_QUEUE_EMPTY_1_MDP_TX2_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_1_MDP_TX2_QUEUE_EMPTY_SHFT),
			  ((que_empty & WF_PSE_TOP_QUEUE_EMPTY_MDP_RX_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_MDP_RX_QUEUE_EMPTY_SHFT));
	DBGLOG(HAL, INFO, "\t\tSEC TX Q0/Q1/Q2/RX Q empty=%d/%d/%d/%d\n",
			  ((que_empty & WF_PSE_TOP_QUEUE_EMPTY_SEC_TX_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_SEC_TX_QUEUE_EMPTY_SHFT),
			  ((que_empty & WF_PSE_TOP_QUEUE_EMPTY_SEC_TX1_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_SEC_TX1_QUEUE_EMPTY_SHFT),
			  ((que_empty1 & WF_PSE_TOP_QUEUE_EMPTY_1_SEC_TX2_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_1_SEC_TX2_QUEUE_EMPTY_SHFT),
			  ((que_empty & WF_PSE_TOP_QUEUE_EMPTY_SEC_RX_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_SEC_RX_QUEUE_EMPTY_SHFT));
	DBGLOG(HAL, INFO, "\t\tSFD PARK Q empty=%d\n",
			  ((que_empty & WF_PSE_TOP_QUEUE_EMPTY_SFD_PARK_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_SFD_PARK_QUEUE_EMPTY_SHFT));
	DBGLOG(HAL, INFO, "\t\tMDP TXIOC Q0/Q1/Q2 empty=%d/%d/%d\n",
			  ((que_empty & WF_PSE_TOP_QUEUE_EMPTY_MDP_TXIOC_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_MDP_TXIOC_QUEUE_EMPTY_SHFT),
			  ((que_empty & WF_PSE_TOP_QUEUE_EMPTY_MDP_TXIOC1_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_MDP_TXIOC1_QUEUE_EMPTY_SHFT),
			  ((que_empty1 & WF_PSE_TOP_QUEUE_EMPTY_1_MDP_TXIOC2_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_1_MDP_TXIOC2_QUEUE_EMPTY_SHFT));
	DBGLOG(HAL, INFO, "\t\tMDP RXIOC Q0/Q1/Q2/Q3 empty=%d/%d/%d/%d\n",
			  ((que_empty & WF_PSE_TOP_QUEUE_EMPTY_MDP_RXIOC_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_MDP_RXIOC_QUEUE_EMPTY_SHFT),
			  ((que_empty & WF_PSE_TOP_QUEUE_EMPTY_MDP_RXIOC1_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_MDP_RXIOC1_QUEUE_EMPTY_SHFT),
			  ((que_empty1 & WF_PSE_TOP_QUEUE_EMPTY_1_MDP_RXIOC2_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_1_MDP_RXIOC2_QUEUE_EMPTY_SHFT),
			  ((que_empty1 & WF_PSE_TOP_QUEUE_EMPTY_1_MDP_RXIOC3_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_1_MDP_RXIOC3_QUEUE_EMPTY_SHFT));
	DBGLOG(HAL, INFO, "\t\tRLS Q empty=%d\n",
			  ((que_empty & WF_PSE_TOP_QUEUE_EMPTY_RLS_Q_EMTPY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_RLS_Q_EMTPY_SHFT));

	DBGLOG(HAL, INFO, "Nonempty Q info:\n");
	for (i = 0; i < 31; i++) {
		if (((que_empty & (0x1 << i)) >> i) == 0)
			if (pse_queue_empty_info[i].QueueName != NULL)
				DBGLOG(HAL, INFO, "\t%s: ", pse_queue_empty_info[i].QueueName);
	}
	for (i = 0; i < 31; i++) {
		if (((que_empty1 & (0x1 << i)) >> i) == 0)
			if (pse_queue_empty2_info[i].QueueName != NULL)
				DBGLOG(HAL, INFO, "\t%s: ", pse_queue_empty2_info[i].QueueName);
	}
}

#if CFG_SUPPORT_LINK_QUALITY_MONITOR
int connac3x_get_rx_rate_info(const uint32_t *prRxV,
		struct RxRateInfo *prRxRateInfo)
{
	uint32_t rxmode = 0, rate = 0, frmode = 0, sgi = 0, nsts = 0;
	uint32_t stbc = 0, nss = 0, mu = 0;

	if (!prRxRateInfo || !prRxV)
		return -1;

	/* P-RXV1 */
	rate = (prRxV[0] & CONNAC3X_RX_VT_RX_RATE_MASK)
				>> CONNAC3X_RX_VT_RX_RATE_OFFSET;
	nsts = (prRxV[0] & CONNAC3X_RX_VT_NSTS_MASK)
				>> CONNAC3X_RX_VT_NSTS_OFFSET;
	frmode = (prRxV[2] & CONNAC3X_RX_VT_FR_MODE_MASK)
					>> CONNAC3X_RX_VT_FR_MODE_OFFSET;
	sgi = (prRxV[2] & CONNAC3X_RX_VT_GI_MASK)
					>> CONNAC3X_RX_VT_GI_OFFSET;
	rxmode = (prRxV[2] & CONNAC3X_RX_VT_RX_MODE_MASK)
					>> CONNAC3X_RX_VT_RX_MODE_OFFSET;
	stbc = (prRxV[2] & CONNAC3X_RX_VT_STBC_MASK)
				>> CONNAC3X_RX_VT_STBC_OFFSET;
	mu = prRxV[0] & CONNAC3X_RX_VT_MU_MASK;

	if (mu == 0)
		nsts += 1;

	if (nsts == 1)
		nss = nsts;
	else
		nss = stbc ? (nsts >> 1) : nsts;

	if (frmode >= 4) {
		DBGLOG(SW4, ERROR, "frmode error: %u\n", frmode);
		return -1;
	}

	prRxRateInfo->u4Rate = rate;
	prRxRateInfo->u4Nss = nss;
	prRxRateInfo->u4Mode = rxmode;
	prRxRateInfo->u4Bw = frmode;
	prRxRateInfo->u4Gi = sgi;

	DBGLOG(SW4, TRACE,
		   "rxmode=[%u], rate=[%u], bw=[%u], sgi=[%u], nss=[%u]\n",
		   rxmode, rate, frmode, sgi, nss
	);

	return 0;
}
#endif

int32_t connac3x_show_mib_info(
	struct ADAPTER *prAdapter,
	uint32_t u4Index,
	char *pcCommand,
	int i4TotalLen)
{
	int32_t i4BytesWritten = 0;
	uint8_t bss_nums = 4;
	uint32_t idx;
	uint32_t mac_val = 0, band_idx = u4Index;
	uint32_t band_offset = 0, band_offset_umib = 0;
	uint32_t msdr6 = 0, msdr9 = 0, msdr18 = 0;
	uint32_t rvsr0 = 0, rscr26 = 0, mctr5 = 0, mctr6 = 0;
	uint32_t msr0 = 0, msr1 = 0, msr2 = 0;
	uint32_t tbcr0 = 0, tbcr1 = 0, tbcr2 = 0, tbcr3 = 0, tbcr4 = 0;
	uint32_t btscr[7] = {0};
	uint32_t tdrcr[5] = {0};
	uint32_t mbtocr[16], mbtbcr[16], mbrocr[16], mbrbcr[16];
	uint32_t btcr = 0, btbcr = 0, brocr = 0;
	uint32_t brbcr = 0, btdcr = 0, brdcr = 0;
#if defined(BELLWETHER) || defined(MT7990)
	uint32_t mu_cnt[5] = {0};
#endif
	uint32_t rx_cnt[6] = {0};
	uint32_t ampdu_cnt[3] = {0};
	uint64_t per;
	uint32_t per_rem;
	struct RX_CTRL *prRxCtrl;

	switch (band_idx) {
	case 0:
		band_offset = 0;
		band_offset_umib = 0;
		break;
	case 1:
		band_offset = BN1_WF_MIB_TOP_BASE - BN0_WF_MIB_TOP_BASE;
		band_offset_umib = WF_UMIB_TOP_B1BROCR_ADDR - WF_UMIB_TOP_B0BROCR_ADDR;
		break;
#if defined(BELLWETHER) || defined(MT7990)
	case 2:
		band_offset = IP1_BN0_WF_MIB_TOP_BASE - BN0_WF_MIB_TOP_BASE;
		band_offset_umib = WF_UMIB_TOP_B2BROCR_ADDR - WF_UMIB_TOP_B0BROCR_ADDR;
		break;
#endif
	default:
		return TRUE;
	}
	prRxCtrl = &prAdapter->rRxCtrl;
	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"Dump MIB info of IDX         = %d\n",
			band_idx);

	/* RX count */
	HAL_MCR_RD(prAdapter, BN0_WF_MIB_TOP_RSCR1_ADDR + band_offset,
		&rx_cnt[0]);
	HAL_MCR_RD(prAdapter, BN0_WF_MIB_TOP_RSCR33_ADDR + band_offset,
		&rx_cnt[1]);
	HAL_MCR_RD(prAdapter, BN0_WF_MIB_TOP_RSCR36_ADDR + band_offset,
		&rx_cnt[2]);
	HAL_MCR_RD(prAdapter, BN0_WF_MIB_TOP_RSCR31_ADDR + band_offset,
		&rx_cnt[3]);
	HAL_MCR_RD(prAdapter, BN0_WF_MIB_TOP_RSCR27_ADDR + band_offset,
		&rx_cnt[4]);
	HAL_MCR_RD(prAdapter, BN0_WF_MIB_TOP_RSCR28_ADDR + band_offset,
		&rx_cnt[5]);
	HAL_MCR_RD(prAdapter, BN0_WF_MIB_TOP_RVSR0_ADDR + band_offset,
		&rvsr0);

	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"%s", "===Rx Related Counters===\n");
	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"\tRx with CRC=%d\n",
		rx_cnt[0] &
			BN0_WF_MIB_TOP_RSCR1_RX_FCS_ERROR_COUNT_MASK);
	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"\tRx drop due to out of resource=%d\n",
		rx_cnt[1] &
			BN0_WF_MIB_TOP_RSCR33_RX_FIFO_FULL_COUNT_MASK);
	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"\tRx Mpdu=%d\n",
		rx_cnt[3] & BN0_WF_MIB_TOP_RSCR31_RX_MPDU_COUNT_MASK);

	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"\tRx AMpdu=%d\n",
		rx_cnt[4]);
	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"\tRx Vec Q Mismatch=%d\n",
		rvsr0 & BN0_WF_MIB_TOP_RVSR0_VEC_MISS_COUNT_MASK);
	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"\tRx Len Mismatch=%d\n",
		rx_cnt[2] &
			BN0_WF_MIB_TOP_RSCR36_RX_LEN_MISMATCH_MASK);
	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"\tRx data indicate total=%llu\n",
		RX_GET_CNT(prRxCtrl, RX_DATA_INDICATION_COUNT));
	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"\tRx data retain total=%llu\n",
		RX_GET_CNT(prRxCtrl, RX_DATA_RETAINED_COUNT));
	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"\tRx drop by SW total=%llu\n",
		RX_GET_CNT(prRxCtrl, RX_DROP_TOTAL_COUNT));
	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"\tRx reorder miss=%llu\n",
		RX_GET_CNT(prRxCtrl, RX_DATA_REORDER_MISS_COUNT));
	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"\tRx reorder within=%llu\n",
		RX_GET_CNT(prRxCtrl, RX_DATA_REORDER_WITHIN_COUNT));
	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"\tRx reorder ahead=%llu\n",
		RX_GET_CNT(prRxCtrl, RX_DATA_REORDER_AHEAD_COUNT));
	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"\tRx reorder behind=%llu\n",
		RX_GET_CNT(prRxCtrl, RX_DATA_REORDER_BEHIND_COUNT));

	do {
		uint32_t u4AmsduCntx100 = 0;

		if (RX_GET_CNT(prRxCtrl, RX_DATA_AMSDU_COUNT))
			u4AmsduCntx100 = (uint32_t)div64_u64(
				RX_GET_CNT(prRxCtrl,
					RX_DATA_MSDU_IN_AMSDU_COUNT) * 100,
				RX_GET_CNT(prRxCtrl, RX_DATA_AMSDU_COUNT));
		i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen,
			i4BytesWritten,
			"\tRx avg MSDU in AMSDU=%1d.%02d\n",
			u4AmsduCntx100 / 100, u4AmsduCntx100 % 100);

	} while (FALSE);

	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"\tRx total MSDU in AMSDU=%llu\n",
		RX_GET_CNT(prRxCtrl, RX_DATA_MSDU_IN_AMSDU_COUNT));
	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"\tRx AMSDU=%llu\n",
		RX_GET_CNT(prRxCtrl, RX_DATA_AMSDU_COUNT));
	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"\tRx AMSDU miss=%llu\n",
		RX_GET_CNT(prRxCtrl, RX_DATA_AMSDU_MISS_COUNT));
	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"\tRx no StaRec drop=%llu\n",
		RX_GET_CNT(prRxCtrl, RX_NO_STA_DROP_COUNT));
	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"\tRx inactive BSS drop=%llu\n",
		RX_GET_CNT(prRxCtrl, RX_INACTIVE_BSS_DROP_COUNT));
	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"\tRx HS20 drop=%llu\n",
		RX_GET_CNT(prRxCtrl, RX_HS20_DROP_COUNT));
	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"\tRx low SwRfb drop=%llu\n",
		RX_GET_CNT(prRxCtrl, RX_LESS_SW_RFB_DROP_COUNT));
	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"\tRx dupicate drop=%llu\n",
		RX_GET_CNT(prRxCtrl, RX_DUPICATE_DROP_COUNT));
	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"\tRx MIC err drop=%llu\n",
		RX_GET_CNT(prRxCtrl, RX_MIC_ERROR_DROP_COUNT));
	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"\tRx BAR handle=%llu\n",
		RX_GET_CNT(prRxCtrl, RX_BAR_DROP_COUNT));
#if CFG_SUPPORT_BAR_DELAY_INDICATION
	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"\tRx BAR delayed=%llu\n",
		RX_GET_CNT(prRxCtrl, RX_BAR_DELAY_COUNT));
#endif /* CFG_SUPPORT_BAR_DELAY_INDICATION */
	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"\tRx non-interest drop=%llu\n",
		RX_GET_CNT(prRxCtrl, RX_NO_INTEREST_DROP_COUNT));
	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"\tRx type err drop=%llu\n",
		RX_GET_CNT(prRxCtrl, RX_TYPE_ERR_DROP_COUNT));
	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"\tRx class err drop=%llu\n",
		RX_GET_CNT(prRxCtrl, RX_CLASS_ERR_DROP_COUNT));

#if defined(BELLWETHER) || defined(MT7990)
	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"%s", "===MU TX Counters===\n");
	HAL_MCR_RD(prAdapter, BN0_WF_MIB_TOP_BSCR2_ADDR + band_offset,
		&mu_cnt[0]);
	HAL_MCR_RD(prAdapter, BN0_WF_MIB_TOP_TSCR5_ADDR + band_offset,
		&mu_cnt[1]);
	HAL_MCR_RD(prAdapter, BN0_WF_MIB_TOP_TSCR6_ADDR + band_offset,
		&mu_cnt[2]);
	HAL_MCR_RD(prAdapter, BN0_WF_MIB_TOP_TSCR8_ADDR + band_offset,
		&mu_cnt[3]);
	HAL_MCR_RD(prAdapter, BN0_WF_MIB_TOP_TSCR7_ADDR + band_offset,
		&mu_cnt[4]);

	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"\tMUBF=%u, MuToMuFail=%u (PPDU)\n",
		mu_cnt[0] & BN0_WF_MIB_TOP_BSCR2_MUBF_TX_COUNT_MASK,
		mu_cnt[3]);
	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"\tMuTotal=%u, MuOK=%u, SU_OK=%u (MPDU)\n",
		mu_cnt[1],
		mu_cnt[2],
		mu_cnt[4]);
#endif

	/* PHY/Timing count  */
	HAL_MCR_RD(prAdapter, BN0_WF_MIB_TOP_M0SDR6_ADDR + band_offset, &msdr6);
	HAL_MCR_RD(prAdapter, BN0_WF_MIB_TOP_M0SDR9_ADDR + band_offset, &msdr9);
	HAL_MCR_RD(prAdapter, BN0_WF_MIB_TOP_RSCR26_ADDR + band_offset, &rscr26);
	HAL_MCR_RD(prAdapter, BN0_WF_MIB_TOP_MCTR5_ADDR + band_offset, &mctr5);
	HAL_MCR_RD(prAdapter, BN0_WF_MIB_TOP_MCTR6_ADDR + band_offset, &mctr6);
	HAL_MCR_RD(prAdapter, BN0_WF_MIB_TOP_M0SDR18_ADDR + band_offset, &msdr18);
	HAL_MCR_RD(prAdapter, BN0_WF_MIB_TOP_MSR0_ADDR + band_offset, &msr0);
	HAL_MCR_RD(prAdapter, BN0_WF_MIB_TOP_MSR1_ADDR + band_offset, &msr1);
	HAL_MCR_RD(prAdapter, BN0_WF_MIB_TOP_MSR2_ADDR + band_offset, &msr2);
	HAL_MCR_RD(prAdapter, BN0_WF_MIB_TOP_TSCR0_ADDR + band_offset, &ampdu_cnt[0]);
	HAL_MCR_RD(prAdapter, BN0_WF_MIB_TOP_TSCR3_ADDR + band_offset, &ampdu_cnt[1]);
	HAL_MCR_RD(prAdapter, BN0_WF_MIB_TOP_TSCR4_ADDR + band_offset, &ampdu_cnt[2]);
	ampdu_cnt[1] &= BN0_WF_MIB_TOP_TSCR3_AMPDU_MPDU_COUNT_MASK;
	ampdu_cnt[2] &= BN0_WF_MIB_TOP_TSCR4_AMPDU_ACKED_COUNT_MASK;

	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"%s", "===Phy/Timing Related Counters===\n");
	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"\tChannelIdleCnt=%d\n",
		msdr6 & BN0_WF_MIB_TOP_M0SDR6_CHANNEL_IDLE_COUNT_MASK);
	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"\tCCA_NAV_Tx_Time=%d\n",
		msdr9 & BN0_WF_MIB_TOP_M0SDR9_CCA_NAV_TX_TIME_MASK);
	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"\tRx_MDRDY_CNT=%d\n",
		rscr26 & BN0_WF_MIB_TOP_RSCR26_RX_MDRDY_COUNT_MASK);
	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"\tMdrdyTime CCK=%d, OFDM=0x%x, OFDM_GREEN=0x%x\n",
		msr0 & BN0_WF_MIB_TOP_MSR0_CCK_MDRDY_TIME_MASK,
		msr1 &
			BN0_WF_MIB_TOP_MSR1_OFDM_LG_MIXED_VHT_MDRDY_TIME_MASK,
		msr2 & BN0_WF_MIB_TOP_MSR2_OFDM_GREEN_MDRDY_TIME_MASK);
	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"\tPrim CCA Time=%d\n",
		mctr5 & BN0_WF_MIB_TOP_MCTR5_P_CCA_TIME_MASK);
	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"\tSec CCA Time=%d\n",
		mctr6 & BN0_WF_MIB_TOP_MCTR6_S_CCA_TIME_MASK);
	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"\tPrim ED Time=%d\n",
		msdr18 & BN0_WF_MIB_TOP_M0SDR18_P_ED_TIME_MASK);
	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"%s", "===Tx Related Counters(Generic)===\n");
	HAL_MCR_RD(prAdapter, BN0_WF_MIB_TOP_TSCR18_ADDR + band_offset, &mac_val);
	HAL_MCR_RD(prAdapter, BN0_WF_MIB_TOP_TBCR0_ADDR + band_offset, &tbcr0);
	HAL_MCR_RD(prAdapter, BN0_WF_MIB_TOP_TBCR1_ADDR + band_offset, &tbcr1);
	HAL_MCR_RD(prAdapter, BN0_WF_MIB_TOP_TBCR2_ADDR + band_offset, &tbcr2);
	HAL_MCR_RD(prAdapter, BN0_WF_MIB_TOP_TBCR3_ADDR + band_offset, &tbcr3);
	HAL_MCR_RD(prAdapter, BN0_WF_MIB_TOP_TBCR4_ADDR + band_offset, &tbcr4);

	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"\tBeaconTxCnt=%d\n",
		mac_val);
	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"\tTx 40MHz Cnt=%d\n",
		tbcr1 & BN0_WF_MIB_TOP_TBCR1_TX_40MHZ_CNT_MASK);
	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"\tTx 80MHz Cnt=%d\n",
		tbcr2 & BN0_WF_MIB_TOP_TBCR2_TX_80MHZ_CNT_MASK);
	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"\tTx 160MHz Cnt=%d\n",
		tbcr3 & BN0_WF_MIB_TOP_TBCR3_TX_160MHZ_CNT_MASK);
	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"\tTx 320MHz Cnt=%d\n",
		tbcr4 & BN0_WF_MIB_TOP_TBCR4_TX_320MHZ_CNT_MASK);
	per = (ampdu_cnt[1] == 0 ?
		0 :
		1000 * (uint64_t)(ampdu_cnt[1] - ampdu_cnt[2]) / ampdu_cnt[1]);
	per_rem = do_div(per, 10);

	i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen, i4BytesWritten,
		"\tAMPDU[Cnt:MpduCnt:MpduAckCnt:MpduPER]=[%u:%u:%u:%ld.%1ld%%]\n",
		ampdu_cnt[0],
		ampdu_cnt[1],
		ampdu_cnt[2],
		per, per_rem);

	for (idx = 0; idx < BSSID_NUM; idx++) {
		HAL_MCR_RD(prAdapter,
			BN0_WF_MIB_TOP_BTSCR5_ADDR + band_offset + idx * 4,
			&btscr[0]);
		HAL_MCR_RD(prAdapter,
			BN0_WF_MIB_TOP_BTSCR6_ADDR + band_offset + idx * 4,
			&btscr[1]);
		HAL_MCR_RD(prAdapter,
			BN0_WF_MIB_TOP_BTSCR0_ADDR + band_offset + idx * 4,
			&btscr[2]);
		HAL_MCR_RD(prAdapter,
			BN0_WF_MIB_TOP_BTSCR1_ADDR + band_offset + idx * 4,
			&btscr[3]);
		HAL_MCR_RD(prAdapter,
			BN0_WF_MIB_TOP_BTSCR2_ADDR + band_offset + idx * 4,
			&btscr[4]);
		HAL_MCR_RD(prAdapter,
			BN0_WF_MIB_TOP_BTSCR3_ADDR + band_offset + idx * 4,
			&btscr[5]);
		HAL_MCR_RD(prAdapter,
			BN0_WF_MIB_TOP_BTSCR4_ADDR + band_offset + idx * 4,
			&btscr[6]);
		HAL_MCR_RD(prAdapter,
			BN0_WF_MIB_TOP_BTCR_ADDR + band_offset + idx * 4,
			&btcr);
		HAL_MCR_RD(prAdapter,
			BN0_WF_MIB_TOP_BTDCR_ADDR + band_offset + idx * 4,
			&btdcr);
		HAL_MCR_RD(prAdapter,
			BN0_WF_MIB_TOP_BTBCR_ADDR + band_offset + idx * 4,
			&btbcr);

		HAL_MCR_RD(prAdapter,
			WF_UMIB_TOP_B0BROCR_ADDR + band_offset_umib + idx * 4,
			&brocr);
		HAL_MCR_RD(prAdapter,
			WF_UMIB_TOP_B0BRDCR_ADDR + band_offset_umib + idx * 4,
			&brdcr);
		HAL_MCR_RD(prAdapter,
			WF_UMIB_TOP_B0BRBCR_ADDR + band_offset_umib + idx * 4,
			&brbcr);

		i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen,
			i4BytesWritten,
			"===BSSID[%d] Related Counters===\n", idx);
		i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen,
			i4BytesWritten,
			"\tBA Miss Cnt=%d\n",
			btscr[2] & BN0_WF_MIB_TOP_BTSCR0_BAMISSCOUNTn_MASK);
		i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen,
			i4BytesWritten,
			"\tRTS Tx Cnt=%d\n",
			btscr[0] & BN0_WF_MIB_TOP_BTSCR5_RTSTXCOUNTn_MASK);
		i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen,
			i4BytesWritten,
			"\tFrame Retry Cnt=%d\n",
			btscr[4] & BN0_WF_MIB_TOP_BTSCR2_FRAMERETRYCOUNTn_MASK);
		i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen,
			i4BytesWritten,
			"\tFrame Retry >2 Cnt=%d\n",
			btscr[5] &
				BN0_WF_MIB_TOP_BTSCR3_FRAMERETRY2COUNTn_MASK);
		i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen,
			i4BytesWritten,
			"\tFrame Retry >3 Cnt=%d\n",
			btscr[6] &
				BN0_WF_MIB_TOP_BTSCR4_FRAMERETRY3COUNTn_MASK);
		i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen,
			i4BytesWritten,
			"\tRTS Retry Cnt=%d\n",
			btscr[1] & BN0_WF_MIB_TOP_BTSCR6_RTSRETRYCOUNTn_MASK);
		i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen,
			i4BytesWritten,
			"\tAck Failed Cnt=%d\n",
			btscr[3] & BN0_WF_MIB_TOP_BTSCR1_ACKFAILCOUNTn_MASK);
		i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen,
			i4BytesWritten,
			"\tTxOk=%u, TxData=%u, TxByte=%u\n",
			btcr, btdcr, btbcr);
		i4BytesWritten = SHOW_DBGLOG(pcCommand, i4TotalLen,
			i4BytesWritten,
			"\tRxOk=%u, RxData=%u, RxByte=%u\n",
			brocr, brdcr, brbcr);
	}

	/* Dummy delimiter insertion result */
	DBGLOG(HAL, INFO, "===Dummy delimiter insertion result===\n");
	HAL_MCR_RD(prAdapter, BN0_WF_MIB_TOP_TDRCR0_ADDR + band_offset, &tdrcr[0]);
	HAL_MCR_RD(prAdapter, BN0_WF_MIB_TOP_TDRCR1_ADDR + band_offset, &tdrcr[1]);
	HAL_MCR_RD(prAdapter, BN0_WF_MIB_TOP_TDRCR2_ADDR + band_offset, &tdrcr[2]);
	HAL_MCR_RD(prAdapter, BN0_WF_MIB_TOP_TDRCR3_ADDR + band_offset, &tdrcr[3]);
	HAL_MCR_RD(prAdapter, BN0_WF_MIB_TOP_TDRCR4_ADDR + band_offset, &tdrcr[4]);
	DBGLOG(HAL, INFO, "\tRange[0:1:2:3:4]=[%u:%u:%u:%u:%u]\n",
		tdrcr[0], tdrcr[1], tdrcr[2], tdrcr[3], tdrcr[4]);

	/* Per-MBSS T/RX Counters */
	DBGLOG(HAL, INFO, "===MBSSID Related Counters===\n", idx);
	for (idx = 0; idx < 16; idx++) {
		HAL_MCR_RD(prAdapter, BN0_WF_MIB_TOP_BTOCR_ADDR + band_offset + (bss_nums + idx) * 4, &mbtocr[idx]);
		HAL_MCR_RD(prAdapter, BN0_WF_MIB_TOP_BTBCR_ADDR + band_offset + (bss_nums + idx) * 4, &mbtbcr[idx]);

		HAL_MCR_RD(prAdapter, WF_UMIB_TOP_B0BROCR_ADDR + band_offset_umib + (bss_nums + idx) * 4, &mbrocr[idx]);
		HAL_MCR_RD(prAdapter, WF_UMIB_TOP_B0BRBCR_ADDR + band_offset_umib + (bss_nums + idx) * 4, &mbrbcr[idx]);
	}
	for (idx = 0; idx < 16; idx++) {
		DBGLOG(HAL, INFO, "%s\n",
			"\tID[%d] TxOk=%u TxByte=%u RxOk=%u RxByte=%u",
			idx, mbtocr[idx], mbtbcr[idx], mbrocr[idx], mbrbcr[idx]);
	}
	return i4BytesWritten;
}

#endif /* CFG_SUPPORT_CONNAC3X */
