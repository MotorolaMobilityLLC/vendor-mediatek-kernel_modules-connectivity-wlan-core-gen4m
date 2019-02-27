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
#endif
#define RSN_CIPHER_SUITE_GROUP_NOT_USED 0x07AC0F00

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
#define WLAN_AKM_SUITE_FT_8021X         0x000FAC03
#define WLAN_AKM_SUITE_FT_PSK           0x000FAC04
#if CFG_SUPPORT_802_11W
#define RSN_AKM_SUITE_802_1X_SHA256     0x05AC0F00
#define RSN_AKM_SUITE_PSK_SHA256        0x06AC0F00
#endif

#define WPA_AKM_SUITE_NONE              0x00F25000
#define WPA_AKM_SUITE_802_1X            0x01F25000
#define WPA_AKM_SUITE_PSK               0x02F25000

#define WFA_AKM_SUITE_OSEN              0x019A6F50
/* this define should be in ieee80211.h, but kernel didn't update it.
 * so we define here temporary
 */
#define WLAN_AKM_SUITE_OSEN             0x506f9a01
#define WLAN_CIPHER_SUITE_NO_GROUP_ADDR 0x000fac07

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

#define WAIT_TIME_IND_PMKID_CANDICATE_SEC       6	/* seconds */
#define TKIP_COUNTERMEASURE_SEC                 60	/* seconds */

#if CFG_SUPPORT_802_11W
#define RSN_AUTH_MFP_DISABLED   0	/* MFP disabled */
#define RSN_AUTH_MFP_OPTIONAL   1	/* MFP optional */
#define RSN_AUTH_MFP_REQUIRED   2	/* MFP required */
#endif

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

#define ELEM_MAX_LEN_ASSOC_RSP_WSC_IE          (32 - ELEM_HDR_LEN)
#define ELEM_MAX_LEN_TIMEOUT_IE          (5)

/*******************************************************************************
 *                  F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */
u_int8_t rsnParseRsnIE(IN struct ADAPTER *prAdapter,
		       IN struct RSN_INFO_ELEM *prInfoElem,
		       OUT struct RSN_INFO *prRsnInfo);

u_int8_t rsnParseWpaIE(IN struct ADAPTER *prAdapter,
		       IN struct WPA_INFO_ELEM *prInfoElem,
		       OUT struct RSN_INFO *prWpaInfo);

u_int8_t rsnSearchSupportedCipher(IN struct ADAPTER
				  *prAdapter,
				  IN uint32_t u4Cipher, OUT uint32_t *pu4Index);

u_int8_t rsnIsSuitableBSS(IN struct ADAPTER *prAdapter,
			  IN struct RSN_INFO *prBssRsnInfo);

u_int8_t rsnSearchAKMSuite(IN struct ADAPTER *prAdapter,
			   IN uint32_t u4AkmSuite, OUT uint32_t *pu4Index);

u_int8_t rsnPerformPolicySelection(IN struct ADAPTER
				   *prAdapter,
				   IN struct BSS_DESC *prBss);

void rsnGenerateWpaNoneIE(IN struct ADAPTER *prAdapter,
			  IN struct MSDU_INFO *prMsduInfo);

void rsnGenerateWPAIE(IN struct ADAPTER *prAdapter,
		      IN struct MSDU_INFO *prMsduInfo);

void rsnGenerateRSNIE(IN struct ADAPTER *prAdapter,
		      IN struct MSDU_INFO *prMsduInfo);

u_int8_t
rsnParseCheckForWFAInfoElem(IN struct ADAPTER *prAdapter,
			    IN uint8_t *pucBuf, OUT uint8_t *pucOuiType,
			    OUT uint16_t *pu2SubTypeVersion);

#if CFG_SUPPORT_AAA
void rsnParserCheckForRSNCCMPPSK(struct ADAPTER *prAdapter,
				 struct RSN_INFO_ELEM *prIe,
				 struct STA_RECORD *prStaRec,
				 uint16_t *pu2StatusCode);
#endif

void rsnTkipHandleMICFailure(IN struct ADAPTER *prAdapter,
			     IN struct STA_RECORD *prSta,
			     IN u_int8_t fgErrorKeyType);

void rsnSelectPmkidCandidateList(IN struct ADAPTER
				 *prAdapter, IN struct BSS_DESC *prBssDesc);

void rsnUpdatePmkidCandidateList(IN struct ADAPTER
				 *prAdapter, IN struct BSS_DESC *prBssDesc);

u_int8_t rsnSearchPmkidEntry(IN struct ADAPTER *prAdapter,
			     IN uint8_t *pucBssid, OUT uint32_t *pu4EntryIndex);

u_int8_t rsnCheckPmkidCandicate(IN struct ADAPTER
				*prAdapter);

void rsnCheckPmkidCache(IN struct ADAPTER *prAdapter,
			IN struct BSS_DESC *prBss);

void rsnGeneratePmkidIndication(IN struct ADAPTER
				*prAdapter);

void rsnIndicatePmkidCand(IN struct ADAPTER *prAdapter,
			  IN unsigned long ulParamPtr);
#if CFG_SUPPORT_WPS2
void rsnGenerateWSCIE(IN struct ADAPTER *prAdapter,
		      IN struct MSDU_INFO *prMsduInfo);
#endif

#if CFG_SUPPORT_802_11W
uint32_t rsnCheckBipKeyInstalled(IN struct ADAPTER
				 *prAdapter, IN struct STA_RECORD *prStaRec);

uint8_t rsnCheckSaQueryTimeout(IN struct ADAPTER
			       *prAdapter);

void rsnStartSaQueryTimer(IN struct ADAPTER *prAdapter,
			  IN unsigned long ulParamPtr);

void rsnStartSaQuery(IN struct ADAPTER *prAdapter);

void rsnStopSaQuery(IN struct ADAPTER *prAdapter);

void rsnSaQueryRequest(IN struct ADAPTER *prAdapter,
		       IN struct SW_RFB *prSwRfb);

void rsnSaQueryAction(IN struct ADAPTER *prAdapter,
		      IN struct SW_RFB *prSwRfb);

uint16_t rsnPmfCapableValidation(IN struct ADAPTER
				 *prAdapter, IN struct BSS_INFO *prBssInfo,
				 IN struct STA_RECORD *prStaRec);

void rsnPmfGenerateTimeoutIE(struct ADAPTER *prAdapter,
			     struct MSDU_INFO *prMsduInfo);

void rsnApStartSaQuery(IN struct ADAPTER *prAdapter,
		       IN struct STA_RECORD *prStaRec);

void rsnApSaQueryAction(IN struct ADAPTER *prAdapter,
			IN struct SW_RFB *prSwRfb);

#endif /* CFG_SUPPORT_802_11W */

#if CFG_SUPPORT_AAA
void rsnGenerateWSCIEForAssocRsp(struct ADAPTER *prAdapter,
				 struct MSDU_INFO *prMsduInfo);
#endif

u_int8_t rsnParseOsenIE(struct ADAPTER *prAdapter,
			struct IE_WFA_OSEN *prInfoElem,
			struct RSN_INFO *prOsenInfo);

u_int8_t rsnCheckSecurityModeChanged(struct ADAPTER
				     *prAdapter, struct BSS_INFO *prBssInfo,
				     struct BSS_DESC *prBssDesc);

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */

#endif /* _RSN_H */
