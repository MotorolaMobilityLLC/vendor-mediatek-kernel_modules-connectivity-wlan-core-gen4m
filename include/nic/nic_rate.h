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
 ** Id: //Department/DaVinci/BRANCHES/
 *      MT6620_WIFI_DRIVER_V2_3/include/nic/nic_rate.h#1
 */

/*! \file  nic_rate.h
 *    \brief This file contains the rate utility function of
 *	   IEEE 802.11 family for MediaTek 802.11 Wireless LAN Adapters.
 */


#ifndef _NIC_RATE_H
#define _NIC_RATE_H

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

/*******************************************************************************
 *                         D A T A   T Y P E S
 *******************************************************************************
 */

/*******************************************************************************
 *                            P U B L I C   D A T A
 *******************************************************************************
 */
#define BW_20		0
#define BW_40		1
#define BW_80		2
#define BW_160		3
#define BW_10		4
#define BW_5		6
#define BW_8080		7
#define BW_ALL		0xFF

#define HW_TX_RATE_TO_MODE(_x)			(((_x) & (0x7 << 6)) >> 6)
#define HW_TX_RATE_TO_MCS(_x, _mode)		((_x) & (0x3f))
#define HW_TX_RATE_TO_NSS(_x)			(((_x) & (0x3 << 9)) >> 9)
#define HW_TX_RATE_TO_STBC(_x)			(((_x) & (0x1 << 11)) >> 11)

#define TX_VECTOR_GET_TX_RATE(_txv)     (((_txv)->u4TxVector1) & BITS(0, 6))
#define TX_VECTOR_GET_TX_LDPC(_txv)     ((((_txv)->u4TxVector1) >> 7) & BIT(0))
#define TX_VECTOR_GET_TX_STBC(_txv)     ((((_txv)->u4TxVector1) >> 8) & \
					BITS(0, 1))
#define TX_VECTOR_GET_TX_FRMODE(_txv)   ((((_txv)->u4TxVector1) >> 10) & \
					BITS(0, 1))
#define TX_VECTOR_GET_TX_MODE(_txv)     ((((_txv)->u4TxVector1) >> 12) & \
					BITS(0, 2))
#define TX_VECTOR_GET_TX_NSTS(_txv)     ((((_txv)->u4TxVector1) >> 21) & \
					BITS(0, 1))
#define TX_VECTOR_GET_TX_PWR(_txv)      ((((_txv)->u4TxVector1) >> 24) & \
					BITS(0, 6))
#define TX_VECTOR_GET_BF_EN(_txv)       ((((_txv)->u4TxVector2) >> 31) & BIT(0))
#define TX_VECTOR_GET_DYN_BW(_txv)      ((((_txv)->u4TxVector4) >> 31) & BIT(0))
#define TX_VECTOR_GET_NO_SOUNDING(_txv) ((((_txv)->u4TxVector4) >> 28) & BIT(0))
#define TX_VECTOR_GET_TX_SGI(_txv)      ((((_txv)->u4TxVector4) >> 27) & BIT(0))

#define TX_RATE_MODE_CCK	0
#define TX_RATE_MODE_OFDM	1
#define TX_RATE_MODE_HTMIX	2
#define TX_RATE_MODE_HTGF	3
#define TX_RATE_MODE_VHT	4
#define TX_RATE_MODE_HE_SU      8
#define TX_RATE_MODE_HE_ER      9
#define TX_RATE_MODE_HE_TRIG    10
#define TX_RATE_MODE_HE_MU      11
#define MAX_TX_MODE             11


extern char *HW_TX_MODE_STR[];
extern char *HW_TX_RATE_CCK_STR[];
extern char *HW_TX_RATE_OFDM_STR[];
extern char *HW_TX_RATE_BW[];

struct FIXED_RATE_INFO {
	uint32_t u4Mode;
	uint32_t u4Bw;
	uint32_t u4Mcs;
	uint32_t u4VhtNss;
	uint32_t u4SGI;
	uint32_t u4Preamble;
	uint32_t u4STBC;
	uint32_t u4LDPC;
	uint32_t u4SpeEn;
	uint32_t u4HeLTF;
	uint32_t u4HeGI;
};

enum RATE_VER {
	RATE_VER_1 = 1,
	RATE_VER_2,
};

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
uint32_t
nicGetPhyRateByMcsRate(
	IN uint8_t ucIdx,
	IN uint8_t ucBw,
	IN uint8_t ucGI
);

uint32_t
nicGetHwRateByPhyRate(
	IN uint8_t ucIdx
);

uint32_t
nicSwIndex2RateIndex(
	IN uint8_t ucRateSwIndex,
	OUT uint8_t *pucRateIndex,
	OUT uint8_t *pucPreambleOption
);

uint32_t
nicRateIndex2RateCode(
	IN uint8_t ucPreambleOption,
	IN uint8_t ucRateIndex,
	OUT uint16_t *pu2RateCode
);

uint32_t
nicRateCode2PhyRate(
	IN uint16_t u2RateCode,
	IN uint8_t ucBandwidth,
	IN uint8_t ucGI,
	IN uint8_t ucRateNss
);

uint32_t
nicRateCode2DataRate(
	IN uint16_t u2RateCode,
	IN uint8_t ucBandwidth,
	IN uint8_t ucGI
);

u_int8_t
nicGetRateIndexFromRateSetWithLimit(
	IN uint16_t u2RateSet,
	IN uint32_t u4PhyRateLimit,
	IN u_int8_t fgGetLowest,
	OUT uint8_t *pucRateSwIndex
);

char *nicHwRateOfdmStr(
	uint16_t ofdm_idx);

uint32_t nicSetFixedRateData(
	struct FIXED_RATE_INFO *pFixedRate,
	uint32_t u4RateVer,
	uint32_t *pu4Data);
/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */

#endif /* _NIC_RATE_H */
