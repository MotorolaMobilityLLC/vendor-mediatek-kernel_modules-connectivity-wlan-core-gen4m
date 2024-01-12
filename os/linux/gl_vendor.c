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
** gl_vendor.c
**
**
*/

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/
#include "gl_os.h"
#include "debug.h"
#include "wlan_lib.h"
#include "gl_wext.h"
#include "precomp.h"
#include <linux/can/netlink.h>
#include <net/netlink.h>
#include <net/cfg80211.h>
#include "gl_cfg80211.h"
#include "gl_vendor.h"
#include "wlan_oid.h"

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/
uint8_t g_GetResultsBufferedCnt;
uint8_t g_GetResultsCmdCnt;

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/

/*******************************************************************************
*                   F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0)

int mtk_cfg80211_NLA_PUT(struct sk_buff *skb, int attrtype, int attrlen, const void *data)
{
	if (unlikely(nla_put(skb, attrtype, attrlen, data) < 0))
		return 0;
	return 1;
}

int mtk_cfg80211_nla_put_type(struct sk_buff *skb, enum ENUM_NLA_PUT_DATE_TYPE type, int attrtype, const void *value)
{
	u8 u8data = 0;
	u16 u16data = 0;
	u32 u32data = 0;
	u64 u64data = 0;

	switch (type) {
	case NLA_PUT_DATE_U8:
		u8data = *(u8 *)value;
		return mtk_cfg80211_NLA_PUT(skb, attrtype, sizeof(u8), &u8data);
	case NLA_PUT_DATE_U16:
		u16data = *(u16 *)value;
		return mtk_cfg80211_NLA_PUT(skb, attrtype, sizeof(u16), &u16data);
	case NLA_PUT_DATE_U32:
		u32data = *(u32 *)value;
		return mtk_cfg80211_NLA_PUT(skb, attrtype, sizeof(u32), &u32data);
	case NLA_PUT_DATE_U64:
		u64data = *(u64 *)value;
		return mtk_cfg80211_NLA_PUT(skb, attrtype, sizeof(u64), &u64data);
	default:
		break;
	}

	return 0;
}

int mtk_cfg80211_vendor_get_channel_list(struct wiphy *wiphy, struct wireless_dev *wdev, const void *data, int data_len)
	{
		struct GLUE_INFO *prGlueInfo;
		struct nlattr *attr;
		uint32_t band = 0;
		uint8_t ucNumOfChannel, i, j;
		struct RF_CHANNEL_INFO aucChannelList[MAX_CHN_NUM];
		uint32_t num_channels;
		uint32_t channels[MAX_CHN_NUM];
		struct sk_buff *skb;

		ASSERT(wiphy && wdev);
		if ((data == NULL) || !data_len)
			return -EINVAL;

		DBGLOG(REQ, INFO, "vendor command: data_len=%d, iftype=%d\n", data_len, wdev->iftype);

		attr = (struct nlattr *)data;
		if (attr->nla_type == WIFI_ATTRIBUTE_BAND)
			band = nla_get_u32(attr);

		DBGLOG(REQ, INFO, "Get channel list for band: %d\n", band);

#if CFG_ENABLE_UNIFY_WIPHY
		prGlueInfo = (struct GLUE_INFO *) wiphy_priv(wiphy);
#else	/* CFG_ENABLE_UNIFY_WIPHY */
		if (wdev == gprWdev)	/* wlan0 */
			prGlueInfo = (struct GLUE_INFO *) wiphy_priv(wiphy);
		else
			prGlueInfo = *((struct GLUE_INFO **) wiphy_priv(wiphy));
#endif	/* CFG_ENABLE_UNIFY_WIPHY */

		if (!prGlueInfo)
			return -EFAULT;

		if (band == 0) { /* 2.4G band */
			rlmDomainGetChnlList(prGlueInfo->prAdapter, BAND_2G4, TRUE,
					 MAX_CHN_NUM, &ucNumOfChannel, aucChannelList);
		} else { /* 5G band */
			rlmDomainGetChnlList(prGlueInfo->prAdapter, BAND_5G, TRUE,
					 MAX_CHN_NUM, &ucNumOfChannel, aucChannelList);
		}

		kalMemZero(channels, sizeof(channels));
		for (i = 0, j = 0; i < ucNumOfChannel; i++) {
			/* We need to report frequency list to HAL */
			channels[j] = nicChannelNum2Freq(aucChannelList[i].ucChannelNum) / 1000;
			if (channels[j] == 0)
				continue;
			else if ((prGlueInfo->prAdapter->rWifiVar.rConnSettings.u2CountryCode == COUNTRY_CODE_TW) &&
				(channels[j] >= 5180 && channels[j] <= 5260)) {
				/* Taiwan NCC has resolution to follow FCC spec to support 5G Band 1/2/3/4
				 * (CH36~CH48, CH52~CH64, CH100~CH140, CH149~CH165)
				 * Filter CH36~CH52 for compatible with some old devices.
				 */
				continue;
			} else {
				DBGLOG(REQ, INFO, "channels[%d] = %d\n", j, channels[j]);
				j++;
			}
		}
		num_channels = j;

		skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, sizeof(channels));
		if (!skb) {
			DBGLOG(REQ, ERROR, "Allocate skb failed\n");
			return -ENOMEM;
		}

		if (unlikely(nla_put_u32(skb, WIFI_ATTRIBUTE_NUM_CHANNELS, num_channels) < 0))
			goto nla_put_failure;

		if (unlikely(nla_put(skb, WIFI_ATTRIBUTE_CHANNEL_LIST,
			(sizeof(uint32_t) * num_channels), channels) < 0))
			goto nla_put_failure;

		return cfg80211_vendor_cmd_reply(skb);

nla_put_failure:
		kfree_skb(skb);
		return -EFAULT;
	}



int mtk_cfg80211_vendor_set_country_code(struct wiphy *wiphy, struct wireless_dev *wdev, const void *data, int data_len)
{
	struct GLUE_INFO *prGlueInfo;
	uint32_t rStatus;
	uint32_t u4BufLen;
	struct nlattr *attr;
	uint8_t country[2] = {0};

	ASSERT(wiphy && wdev);
	if ((data == NULL) || (data_len == 0))
		return -EINVAL;

	DBGLOG(REQ, INFO, "vendor command: data_len=%d, iftype=%d\n", data_len, wdev->iftype);

	attr = (struct nlattr *)data;
	if (attr->nla_type == WIFI_ATTRIBUTE_COUNTRY_CODE) {
		country[0] = *((uint8_t *)nla_data(attr));
		country[1] = *((uint8_t *)nla_data(attr) + 1);
	}

	DBGLOG(REQ, INFO, "Set country code: %c%c\n", country[0], country[1]);

#if CFG_ENABLE_UNIFY_WIPHY
	prGlueInfo = (struct GLUE_INFO *) wiphy_priv(wiphy);
#else	/* CFG_ENABLE_UNIFY_WIPHY */
	if (wdev == gprWdev)	/* wlan0 */
		prGlueInfo = (struct GLUE_INFO *) wiphy_priv(wiphy);
	else
		prGlueInfo = *((struct GLUE_INFO **) wiphy_priv(wiphy));
#endif	/* CFG_ENABLE_UNIFY_WIPHY */

	if (!prGlueInfo)
		return -EFAULT;

	rStatus = kalIoctl(prGlueInfo, wlanoidSetCountryCode, country, 2, FALSE, FALSE, TRUE, &u4BufLen);
	if (rStatus != WLAN_STATUS_SUCCESS) {
		DBGLOG(REQ, ERROR, "Set country code error: %x\n", rStatus);
		return -EFAULT;
	}

	return 0;
}


#if 0
int mtk_cfg80211_vendor_llstats_get_info(struct wiphy *wiphy, struct wireless_dev *wdev, const void *data, int data_len)
{
	int32_t i4Status = -EINVAL;
	struct WIFI_RADIO_STAT *pRadioStat;
	struct sk_buff *skb;
	uint32_t u4BufLen;

	DBGLOG(REQ, INFO, "%s for vendor command\r\n", __func__);

	ASSERT(wiphy);
	ASSERT(wdev);

	u4BufLen = sizeof(struct WIFI_RADIO_STAT) + sizeof(struct WIFI_IFACE_STAT);
	pRadioStat = kalMemAlloc(u4BufLen, VIR_MEM_TYPE);
	if (!pRadioStat) {
		DBGLOG(REQ, ERROR, "%s kalMemAlloc pRadioStat failed\n", __func__);
		return -ENOMEM;
	}
	kalMemZero(pRadioStat, u4BufLen);

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, u4BufLen);
	if (!skb) {
		DBGLOG(REQ, TRACE, "%s allocate skb failed:%x\n", __func__, i4Status);
		return -ENOMEM;
	}

	/*rStatus = kalIoctl(prGlueInfo,
	 *  wlanoidQueryStatistics,
	 *  &rRadioStat,
	 *  sizeof(rRadioStat),
	 *  TRUE,
	 *  TRUE,
	 *  TRUE,
	 *  FALSE,
	 *  &u4BufLen);
	 */
	/* only for test */
	pRadioStat->radio = 10;
	pRadioStat->on_time = 11;
	pRadioStat->tx_time = 12;
	pRadioStat->num_channels = 4;

	NLA_PUT(skb, NL80211_ATTR_VENDOR_LLSTAT, u4BufLen, pRadioStat);

	i4Status = cfg80211_vendor_cmd_reply(skb);

	kalMemFree(pRadioStat, VIR_MEM_TYPE, u4BufLen);

	return i4Status;
}
#endif
#endif

int mtk_cfg80211_vendor_set_band(struct wiphy *wiphy, struct wireless_dev *wdev,
					const void *data, int data_len)
{
	struct GLUE_INFO *prGlueInfo = NULL;
	struct nlattr *attr;
	uint8_t setBand = 0;
	enum ENUM_BAND band;

	ASSERT(wiphy);
	ASSERT(wdev);

	DBGLOG(REQ, INFO, "%s()\n", __func__);

	if ((data == NULL) || !data_len)
		goto nla_put_failure;

	DBGLOG(REQ, TRACE, "vendor command: data_len=%d, data=0x%x 0x%x\r\n",
		data_len, *((uint32_t *) data), *((uint32_t *) data + 1));

	attr = (struct nlattr *)data;
	setBand = nla_get_u32(attr);
	prGlueInfo = (struct GLUE_INFO *) wiphy_priv(wiphy);
	ASSERT(prGlueInfo);

	DBGLOG(REQ, INFO, "Vendor Set Band value=%d\r\n", setBand);

	if (setBand == QCA_SETBAND_5G)
		band = BAND_5G;
	else if (setBand == QCA_SETBAND_2G)
		band = BAND_2G4;
	else
		band = BAND_NULL;

	prGlueInfo->prAdapter->aePreferBand[NETWORK_TYPE_AIS] = band;
	return 0;

nla_put_failure:
	return -1;
}

int mtk_cfg80211_vendor_set_roaming_policy(struct wiphy *wiphy, struct wireless_dev *wdev,
					const void *data, int data_len)
{
	struct GLUE_INFO *prGlueInfo = NULL;
	uint32_t rStatus = WLAN_STATUS_SUCCESS;
	struct nlattr *attr;
	uint32_t setRoaming = 0;
	uint32_t u4BufLen = 0;
	int32_t i4Status = -EINVAL;

	ASSERT(wiphy);
	ASSERT(wdev);

	if ((data == NULL) || !data_len)
		goto nla_put_failure;

	attr = (struct nlattr *)data;
	setRoaming = nla_get_u32(attr);
	prGlueInfo = (struct GLUE_INFO *) wiphy_priv(wiphy);
	ASSERT(prGlueInfo);

	DBGLOG(REQ, INFO, "vendor command: data_len=%d, data=0x%x 0x%x, roaming policy=%d\r\n",
		data_len, *((uint32_t *) data), *((uint32_t *) data + 1), setRoaming);

	rStatus = kalIoctl(prGlueInfo,
			   wlanoidSetDrvRoamingPolicy,
			   &setRoaming, sizeof(uint32_t), FALSE, FALSE, TRUE, &u4BufLen);

	return rStatus;

nla_put_failure:
	return i4Status;

}
