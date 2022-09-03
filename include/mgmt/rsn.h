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
 * Id: include/mgmt/rsn.h#1
 */

/*! \file   rsn.h
 *  \brief  The wpa/rsn related define, macro and structure are described here.
 */

#ifndef _RSN_H
#define _RSN_H

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
/* ----- Definitions for Cipher Suite Selectors ----- */
#define RSN_CIPHER_SUITE_USE_GROUP_KEY  0x00AC0F00
#define RSN_CIPHER_SUITE_WEP40          0x01AC0F00
#define RSN_CIPHER_SUITE_TKIP           0x02AC0F00
#define RSN_CIPHER_SUITE_CCMP           0x04AC0F00
#define RSN_CIPHER_SUITE_WEP104         0x05AC0F00
#if CFG_SUPPORT_802_11W
#define RSN_CIPHER_SUITE_AES_128_CMAC   0x06AC0F00
#define RSN_CIPHER_SUITE_BIP_CMAC_128   0x06AC0F00
#endif
#define RSN_CIPHER_SUITE_GROUP_NOT_USED 0x07AC0F00
#define RSN_CIPHER_SUITE_GCMP           0x08AC0F00
#define RSN_CIPHER_SUITE_GCMP_256       0x09AC0F00
#define RSN_CIPHER_SUITE_CCMP_256       0x0AAC0F00
#define RSN_CIPHER_SUITE_BIP_GMAC_128   0x0BAC0F00
#define RSN_CIPHER_SUITE_BIP_GMAC_256   0x0CAC0F00
#define RSN_CIPHER_SUITE_BIP_CMAC_256   0x0DAC0F00

#define WPA_CIPHER_SUITE_NONE           0x00F25000
#define WPA_CIPHER_SUITE_WEP40          0x01F25000
#define WPA_CIPHER_SUITE_TKIP           0x02F25000
#define WPA_CIPHER_SUITE_CCMP           0x04F25000
#define WPA_CIPHER_SUITE_WEP104         0x05F25000

/* Definitions for Authentication and Key Management Suite Selectors */
#define RSN_AKM_SUITE_NONE              0x00AC0F00
#define RSN_AKM_SUITE_802_1X            0x01AC0F00
#define RSN_AKM_SUITE_PSK               0x02AC0F00
#define RSN_AKM_SUITE_FT_802_1X         0x03AC0F00
#define RSN_AKM_SUITE_FT_PSK            0x04AC0F00
#ifndef WLAN_AKM_SUITE_FT_8021X
#define WLAN_AKM_SUITE_FT_8021X         0x000FAC03
#endif
#ifndef WLAN_AKM_SUITE_FT_PSK
#define WLAN_AKM_SUITE_FT_PSK           0x000FAC04
#endif
#ifndef WLAN_AKM_SUITE_8021X_SUITE_B
#define WLAN_AKM_SUITE_8021X_SUITE_B    0x000FAC0B
#endif
#ifndef WLAN_AKM_SUITE_8021X_SUITE_B_192
#define WLAN_AKM_SUITE_8021X_SUITE_B_192 0x000FAC0C
#endif

/* Add AKM SUITE for OWE since kernel haven't defined it. */
#ifndef WLAN_AKM_SUITE_OWE
#define WLAN_AKM_SUITE_OWE              0x000FAC12
#endif
#if CFG_SUPPORT_802_11W
#define RSN_AKM_SUITE_802_1X_SHA256     0x05AC0F00
#define RSN_AKM_SUITE_PSK_SHA256        0x06AC0F00
#endif

#define RSN_AKM_SUITE_TDLS              0x07AC0F00
#define RSN_AKM_SUITE_SAE               0x08AC0F00
#define RSN_AKM_SUITE_FT_OVER_SAE       0x09AC0F00
#define RSN_AKM_SUITE_8021X_SUITE_B     0x0BAC0F00
#define RSN_AKM_SUITE_8021X_SUITE_B_192 0x0CAC0F00
#define RSN_AKM_SUITE_FILS_SHA256       0x0EAC0F00
#define RSN_AKM_SUITE_FILS_SHA384       0x0FAC0F00
#define RSN_AKM_SUITE_FT_FILS_SHA256    0x10AC0F00
#define RSN_AKM_SUITE_FT_FILS_SHA384    0x11AC0F00
#define RSN_AKM_SUITE_OWE               0x12AC0F00
#define RSN_AKM_SUITE_DPP               0x029A6F50
#define WPA_AKM_SUITE_NONE              0x00F25000
#define WPA_AKM_SUITE_802_1X            0x01F25000
#define WPA_AKM_SUITE_PSK               0x02F25000

#define WFA_AKM_SUITE_OSEN              0x019A6F50
/* this define should be in ieee80211.h, but kernel didn't update it.
 * so we define here temporary
 */
#define WLAN_AKM_SUITE_OSEN             0x506f9a01
#define WLAN_CIPHER_SUITE_NO_GROUP_ADDR 0x000fac07

#define WLAN_AKM_SUITE_DPP              0x506F9A02

/* The RSN IE len for associate request */
#define ELEM_ID_RSN_LEN_FIXED           20

/* The WPA IE len for associate request */
#define ELEM_ID_WPA_LEN_FIXED           22

#define MASK_RSNIE_CAP_PREAUTH          BIT(0)

#define GET_SELECTOR_TYPE(x)           ((uint8_t)(((x) >> 24) & 0x000000FF))
#define SET_SELECTOR_TYPE(x, y)		(x = (((x) & 0x00FFFFFF) | \
					(((uint32_t)(y) << 24) & 0xFF000000)))

#define AUTH_CIPHER_CCMP                0x00000008

/* Cihpher suite flags */
#define CIPHER_FLAG_NONE                        0x00000000
#define CIPHER_FLAG_WEP40                       0x00000001	/* BIT 1 */
#define CIPHER_FLAG_TKIP                        0x00000002	/* BIT 2 */
#define CIPHER_FLAG_CCMP                        0x00000008	/* BIT 4 */
#define CIPHER_FLAG_WEP104                      0x00000010	/* BIT 5 */
#define CIPHER_FLAG_WEP128                      0x00000020	/* BIT 6 */
#define CIPHER_FLAG_GCMP128                     0x00000040      /* BIT 7 */
#define CIPHER_FLAG_GCMP256                     0x00000080	/* BIT 8 */

#define TKIP_COUNTERMEASURE_SEC                 60	/* seconds */

#if CFG_SUPPORT_802_11W
#define RSN_AUTH_MFP_DISABLED   0	/* MFP disabled */
#define RSN_AUTH_MFP_OPTIONAL   1	/* MFP optional */
#define RSN_AUTH_MFP_REQUIRED   2	/* MFP required */
#endif

/* Extended RSN Capabilities */
/* bits 0-3: Field length (n-1) */
#define WLAN_RSNX_CAPAB_PROTECTED_TWT 4
#define WLAN_RSNX_CAPAB_SAE_H2E 5
#define WLAN_RSNX_CAPAB_SAE_PK 6
#define WLAN_RSNX_CAPAB_SECURE_LTF 8
#define WLAN_RSNX_CAPAB_SECURE_RTT 9
#define WLAN_RSNX_CAPAB_PROT_RANGE_NEG 10

#ifdef CFG_SUPPORT_UNIFIED_COMMAND
#define GTK_REKEY_CMD_MODE_OFFLOAD_ON           0
#define GTK_REKEY_CMD_MODE_OFLOAD_OFF           1
#define GTK_REKEY_CMD_MODE_OFFLOAD_UPDATE       2
#define GTK_REKEY_CMD_MODE_SET_BCMC_PN          3
#define GTK_REKEY_CMD_MODE_GET_BCMC_PN          4
#define GTK_REKEY_CMD_MODE_RPY_OFFLOAD_ON       5
#define GTK_REKEY_CMD_MODE_RPY_OFFLOAD_OFF      6

/* sub-mode for GTK_REKEY_CMD_MODE_OFFLOAD_UPDATE */
#define GTK_REKEY_UPDATE_AND_ON                 0
#define GTK_REKEY_UPDATE_ONLY                   1
#else
/* CFG_BC_MC_RP_DETECTION_FWOFFLOAD:             */
/* for GTK rekey and BC/MC replay detection use. */
#define GTK_REKEY_CMD_MODE_OFFLOAD_ON           0
#define GTK_REKEY_CMD_MODE_OFLOAD_OFF           1
#define GTK_REKEY_CMD_MODE_SET_BCMC_PN          2
#define GTK_REKEY_CMD_MODE_GET_BCMC_PN          3
#define GTK_REKEY_CMD_MODE_RPY_OFFLOAD_ON       4
#define GTK_REKEY_CMD_MODE_RPY_OFFLOAD_OFF      5
#endif

#define SA_QUERY_RETRY_TIMEOUT	3000
#define SA_QUERY_TIMEOUT	501

/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */

/* Flags for PMKID Candidate list structure */
#define EVENT_PMKID_CANDIDATE_PREAUTH_ENABLED   0x01

#define CONTROL_FLAG_UC_MGMT_NO_ENC             BIT(5)

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
#define RSN_IE(fp)              ((struct RSN_INFO_ELEM *) fp)
#define WPA_IE(fp)              ((struct WPA_INFO_ELEM *) fp)
#define RSNX_IE(fp)             ((struct RSNX_INFO_ELEM *) fp)

#define ELEM_MAX_LEN_ASSOC_RSP_WSC_IE          (32 - ELEM_HDR_LEN)
#define ELEM_MAX_LEN_TIMEOUT_IE          (5)

/*******************************************************************************
 *                  F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */
u_int8_t rsnParseRsnIE(struct ADAPTER *prAdapter,
		       struct RSN_INFO_ELEM *prInfoElem,
		       struct RSN_INFO *prRsnInfo);

u_int8_t rsnParseWpaIE(struct ADAPTER *prAdapter,
		       struct WPA_INFO_ELEM *prInfoElem,
		       struct RSN_INFO *prWpaInfo);

u_int8_t rsnSearchSupportedCipher(struct ADAPTER
				  *prAdapter,
				  uint32_t u4Cipher, uint32_t *pu4Index,
				  uint8_t ucBssIndex);

u_int8_t rsnIsSuitableBSS(struct ADAPTER *prAdapter,
			  struct BSS_DESC *prBss,
			  struct RSN_INFO *prBssRsnInfo,
			  uint8_t ucBssIndex);

u_int8_t rsnSearchAKMSuite(struct ADAPTER *prAdapter,
			   uint32_t u4AkmSuite, uint32_t *pu4Index,
			   uint8_t ucBssIndex);

u_int8_t rsnPerformPolicySelection(struct ADAPTER
				   *prAdapter,
				   struct BSS_DESC *prBss,
				   uint8_t ucBssIndex);

void rsnGenerateWpaNoneIE(struct ADAPTER *prAdapter,
			  struct MSDU_INFO *prMsduInfo);

void rsnGenerateWPAIE(struct ADAPTER *prAdapter,
		      struct MSDU_INFO *prMsduInfo);

void rsnGenerateRSNIE(struct ADAPTER *prAdapter,
		      struct MSDU_INFO *prMsduInfo);

void rsnGenerateRSNXIE(struct ADAPTER *prAdapter,
		      struct MSDU_INFO *prMsduInfo);

void rsnGenerateOWEIE(struct ADAPTER *prAdapter,
		      struct MSDU_INFO *prMsduInfo);

u_int8_t
rsnParseCheckForWFAInfoElem(struct ADAPTER *prAdapter,
			    uint8_t *pucBuf, uint8_t *pucOuiType,
			    uint16_t *pu2SubTypeVersion);

#if CFG_SUPPORT_AAA
void rsnParserCheckForRSNCCMPPSK(struct ADAPTER *prAdapter,
				 struct RSN_INFO_ELEM *prIe,
				 struct STA_RECORD *prStaRec,
				 uint16_t *pu2StatusCode);
#endif

void rsnTkipHandleMICFailure(struct ADAPTER *prAdapter,
			     struct STA_RECORD *prSta,
			     u_int8_t fgErrorKeyType);

struct PMKID_ENTRY *rsnSearchPmkidEntry(struct ADAPTER *prAdapter,
					uint8_t *pucBssid,
					uint8_t ucBssIndex);

void rsnGeneratePmkidIndication(struct ADAPTER *prAdapter,
				struct PARAM_PMKID_CANDIDATE *prCandi,
				uint8_t ucBssIndex);

uint32_t rsnSetPmkid(struct ADAPTER *prAdapter,
		     struct PARAM_PMKID *prPmkid);

uint32_t rsnDelPmkid(struct ADAPTER *prAdapter,
		     struct PARAM_PMKID *prPmkid);

uint32_t rsnFlushPmkid(struct ADAPTER *prAdapter,
	uint8_t ucBssIndex);

#if CFG_SUPPORT_802_11W
uint32_t rsnCheckBipKeyInstalled(struct ADAPTER
				 *prAdapter, struct STA_RECORD *prStaRec);

uint32_t rsnCheckBipGmacKeyInstall(struct ADAPTER
				 *prAdapter, struct STA_RECORD *prStaRec);

uint8_t rsnCheckSaQueryTimeout(
	struct ADAPTER *prAdapter, uint8_t ucBssIdx);

void rsnStartSaQueryTimer(struct ADAPTER *prAdapter,
			  uintptr_t ulParamPtr);

void rsnStartSaQuery(struct ADAPTER *prAdapter,
	uint8_t ucBssIdx);

void rsnStopSaQuery(struct ADAPTER *prAdapter,
	uint8_t ucBssIdx);

void rsnSaQueryRequest(struct ADAPTER *prAdapter,
		       struct SW_RFB *prSwRfb);

void rsnSaQueryAction(struct ADAPTER *prAdapter,
		      struct SW_RFB *prSwRfb);

uint16_t rsnPmfCapableValidation(struct ADAPTER
				 *prAdapter, struct BSS_INFO *prBssInfo,
				 struct STA_RECORD *prStaRec);

void rsnPmfGenerateTimeoutIE(struct ADAPTER *prAdapter,
			     struct MSDU_INFO *prMsduInfo);

void rsnApStartSaQuery(struct ADAPTER *prAdapter,
		       struct STA_RECORD *prStaRec);

void rsnApSaQueryAction(struct ADAPTER *prAdapter,
			struct SW_RFB *prSwRfb);

uint8_t rsnCheckBipGmac(struct ADAPTER *prAdapter,
			struct SW_RFB *prSwRfb);
#endif /* CFG_SUPPORT_802_11W */

#if CFG_SUPPORT_AAA
void rsnGenerateWSCIEForAssocRsp(struct ADAPTER *prAdapter,
				 struct MSDU_INFO *prMsduInfo);
#endif

u_int8_t rsnParseOsenIE(struct ADAPTER *prAdapter,
			struct IE_WFA_OSEN *prInfoElem,
			struct RSN_INFO *prOsenInfo);

#if CFG_SUPPORT_DETECT_SECURITY_MODE_CHANGE
u_int8_t rsnCheckSecurityModeChanged(struct ADAPTER
				     *prAdapter, struct BSS_INFO *prBssInfo,
				     struct BSS_DESC *prBssDesc);
#endif

uint32_t rsnCalculateFTIELen(struct ADAPTER *prAdapter, uint8_t ucBssIdx,
			     struct STA_RECORD *prStaRec);

void rsnGenerateFTIE(struct ADAPTER *prAdapter,
		     struct MSDU_INFO *prMsduInfo);

u_int8_t rsnIsFtOverTheAir(struct ADAPTER *prAdapter,
			uint8_t ucBssIdx, uint8_t ucStaRecIdx);

u_int8_t rsnParseRsnxIE(struct ADAPTER *prAdapter,
		       struct RSNX_INFO_ELEM *prInfoElem,
		       struct RSNX_INFO *prRsnxeInfo);

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */

#endif /* _RSN_H */
