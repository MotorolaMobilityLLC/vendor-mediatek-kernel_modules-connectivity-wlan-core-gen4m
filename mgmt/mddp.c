/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2019 MediaTek Inc.
 */
/*
** Id: @(#) mddp.c@@
*/

/*! \file   mddp.c
*    \brief  Main routines for modem direct path handling
*
*    This file contains the support routines of modem direct path operation.
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

#if CFG_MTK_MCIF_WIFI_SUPPORT

#include "gl_os.h"
#include "mddp_export.h"
#include "mddp.h"

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

/*******************************************************************************
*                   F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/
struct mddpw_drv_handle_t gMddpWFunc = {
	mddpMdNotifyInfo,
	NULL,
	NULL,
	NULL,
	NULL,
};

struct mddp_drv_conf_t gMddpDrvConf = {
	.app_type = MDDP_APP_TYPE_WH,
};

struct mddp_drv_handle_t gMddpFunc = {
	.change_state = mddpChangeState,
};

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/
#define MAC_ADDR_LEN            6

struct mddp_txd_t {
uint8_t version;
uint8_t wlan_idx;
uint8_t sta_idx;
uint8_t nw_if_name[8];
uint8_t sta_mode;
uint8_t bss_id;
uint8_t wmmset;
uint8_t aucMacAddr[MAC_ADDR_LEN];
uint8_t txd_length;
uint8_t txd[0];
} __packed;
/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

int32_t mddpRegisterCb(IN struct ADAPTER *prAdapter)
{
	int32_t ret = 0;

	gMddpFunc.wifi_handle = &gMddpWFunc;

	ret = mddp_drv_attach(&gMddpDrvConf, &gMddpFunc);
	DBGLOG(INIT, INFO, "MDDP Wlan callback reqister result: %d\n", ret);

	prAdapter->fgMddpActivated = true;

	return ret;
}

int32_t mddpGetMdStats(IN struct net_device *prDev)
{
	struct mddpw_net_stat_t mddpNetStats;

	if (gMddpWFunc.get_net_stat) {
		gMddpWFunc.get_net_stat(&mddpNetStats);
		prDev->stats.rx_packets += mddpNetStats.rx_packets;
		prDev->stats.tx_packets += mddpNetStats.tx_packets;
		prDev->stats.rx_bytes += mddpNetStats.rx_bytes;
		prDev->stats.tx_bytes += mddpNetStats.tx_bytes;
		prDev->stats.rx_errors += mddpNetStats.rx_errors;
		prDev->stats.tx_errors += mddpNetStats.tx_errors;
	}

	return 0;
}

int32_t mddpSetTxDescTemplate(IN struct ADAPTER *prAdapter,
	IN struct STA_RECORD *prStaRec,
	IN uint8_t fgActivate)
{
	struct mddpw_txd_t *prMddpTxd;
	uint32_t u32BufSize = 0;
	uint8_t *buff = NULL;

	if (gMddpWFunc.add_txd) {
		u32BufSize = (sizeof(struct mddpw_txd_t) +
			NIC_TX_DESC_LONG_FORMAT_LENGTH);
		buff = kalMemAlloc(u32BufSize, VIR_MEM_TYPE);

		if (buff == NULL) {
			DBGLOG(NIC, ERROR, "Can't allocate TXD buffer.\n");
			return -1;
		}
		prMddpTxd = (struct mddpw_txd_t *) buff;
		prMddpTxd->version = 0;
		prMddpTxd->sta_idx = prStaRec->ucIndex;
		prMddpTxd->wlan_idx = prStaRec->ucWlanIndex;
		memcpy(prMddpTxd->aucMacAddr,
			prStaRec->aucMacAddr, MAC_ADDR_LEN);
		if (fgActivate)
			prMddpTxd->txd_length = NIC_TX_DESC_LONG_FORMAT_LENGTH;
		else
			prMddpTxd->txd_length = 0;
		memcpy(prMddpTxd->txd,
			prStaRec->aprTxDescTemplate[0], prMddpTxd->txd_length);
		gMddpWFunc.add_txd(prMddpTxd);
		kalMemFree(buff, VIR_MEM_TYPE, u32BufSize);
	}
	return 0;
}

static bool mddpIsSsnSent(struct ADAPTER *prAdapter,
			  uint8_t *prReorderBuf, uint16_t u2SSN)
{
	uint8_t ucSent = 0;
	uint16_t u2Idx = u2SSN / 8;
	uint8_t ucBit = u2SSN % 8;

	ucSent = (prReorderBuf[u2Idx] & BIT(ucBit)) != 0;
	prReorderBuf[u2Idx] &= ~(BIT(ucBit));

	return ucSent;
}

static void mddpGetRxReorderBuffer(struct ADAPTER *prAdapter,
				   struct SW_RFB *prSwRfb,
				   struct mddpw_md_virtual_buf_t **prMdBuf,
				   struct mddpw_ap_virtual_buf_t **prApBuf)
{
	struct mddpw_md_reorder_sync_table_t *prMdTable = NULL;
	struct mddpw_ap_reorder_sync_table_t *prApTable = NULL;
	uint8_t ucStaRecIdx = prSwRfb->ucStaRecIdx;
	uint8_t ucTid = prSwRfb->ucTid;
	int32_t u4Idx = 0;

	if (gMddpWFunc.get_md_rx_reorder_buf &&
	    gMddpWFunc.get_ap_rx_reorder_buf) {
		if (!gMddpWFunc.get_md_rx_reorder_buf(&prMdTable) &&
		    !gMddpWFunc.get_ap_rx_reorder_buf(&prApTable)) {
			u4Idx = prMdTable->reorder_info[ucStaRecIdx].buf_idx;
			*prMdBuf = &prMdTable->virtual_buf[u4Idx][ucTid];
			*prApBuf = &prApTable->virtual_buf[u4Idx][ucTid];
		}
	}
}

void mddpUpdateReorderQueParm(struct ADAPTER *prAdapter,
			      struct RX_BA_ENTRY *prReorderQueParm,
			      struct SW_RFB *prSwRfb)
{
	struct mddpw_md_virtual_buf_t *prMdBuf = NULL;
	struct mddpw_ap_virtual_buf_t *prApBuf = NULL;
	uint16_t u2SSN = prReorderQueParm->u2WinStart, u2Idx;

	mddpGetRxReorderBuffer(prAdapter, prSwRfb, &prMdBuf, &prApBuf);
	if (!prMdBuf || !prApBuf) {
		DBGLOG(QM, ERROR, "Can't get reorder buffer.\n");
		return;
	}

	for (u2Idx = 0; u2Idx < prReorderQueParm->u2WinSize; u2Idx++) {
		if (prReorderQueParm->u2WinStart == prSwRfb->u2SSN ||
		    !mddpIsSsnSent(prAdapter, prMdBuf->virtual_buf, u2SSN))
			break;

		prReorderQueParm->u2WinStart =
			(u2SSN % MAX_SEQ_NO_COUNT);
		prReorderQueParm->u2WinEnd =
			(((prReorderQueParm->u2WinStart) +
			  (prReorderQueParm->u2WinSize) - 1) %
			 MAX_SEQ_NO_COUNT);
		u2SSN = (u2SSN + 1) % MAX_SEQ_NO_COUNT;
		DBGLOG(QM, TRACE,
			"Update reorder window: SSN: %d, start: %d, end: %d.\n",
			u2SSN,
			prReorderQueParm->u2WinStart,
			prReorderQueParm->u2WinEnd);
	}

	prApBuf->start_idx = prReorderQueParm->u2WinStart;
	prApBuf->end_idx = prReorderQueParm->u2WinEnd;
}

int32_t mddpNotifyDrvTxd(IN struct ADAPTER *prAdapter,
	IN struct STA_RECORD *prStaRec,
	IN uint8_t fgActivate)
{
	struct mddpw_drv_notify_info_t *prNotifyInfo;
	struct mddpw_drv_info_t *prDrvInfo;
	struct mddp_txd_t *prMddpTxd;
	struct BSS_INFO *prBssInfo = (struct BSS_INFO *) NULL;
	struct NET_INTERFACE_INFO prNetIfInfo;
	uint32_t u32BufSize = 0;
	uint8_t *buff = NULL;

	if (gMddpWFunc.notify_drv_info) {
		u32BufSize = (sizeof(struct mddpw_drv_notify_info_t) +
			sizeof(struct mddpw_drv_info_t) +
			sizeof(struct mddp_txd_t) +
			NIC_TX_DESC_LONG_FORMAT_LENGTH);
		buff = kalMemAlloc(u32BufSize, VIR_MEM_TYPE);

		if (buff == NULL) {
			DBGLOG(NIC, ERROR, "MDDP Can't allocate ");
			return -1;
		}
		prNotifyInfo = (struct mddpw_drv_notify_info_t *) buff;
		prNotifyInfo->version = 0;
		/* (3 = version+buf_len+info_num) */
		prNotifyInfo->buf_len = (u32BufSize-3);
		prNotifyInfo->info_num = 1;
		prDrvInfo = (struct mddpw_drv_info_t *) &(prNotifyInfo->buf[0]);
		prDrvInfo->info_id = 3; /* MDDPW_DRV_INFO_TXD; */
		prDrvInfo->info_len = (sizeof(struct mddpw_txd_t) +
			NIC_TX_DESC_LONG_FORMAT_LENGTH);
		prMddpTxd = (struct mddp_txd_t *) &(prDrvInfo->info[0]);
		prMddpTxd->version = 0;
		prMddpTxd->sta_idx = prStaRec->ucIndex;
		prMddpTxd->wlan_idx = prStaRec->ucWlanIndex;
		prMddpTxd->sta_mode = prStaRec->eStaType;
		prMddpTxd->bss_id = prStaRec->ucBssIndex;

		if ((prStaRec->ucBssIndex) < MAX_BSSID_NUM) {
			prBssInfo = prAdapter->aprBssInfo[prStaRec->ucBssIndex];
			prNetIfInfo = prAdapter->prGlueInfo
				->arNetInterfaceInfo[prStaRec->ucBssIndex];
			prMddpTxd->wmmset = prBssInfo->ucWmmQueSet;
			kalMemCopy(prMddpTxd->nw_if_name,
				prNetIfInfo.pvNetInterface, 6);
		}

		DBGLOG(NIC, INFO,
			"ver:%d,idx:%d,w_idx:%d,mod:%d,bss:%d,wmm:%d,act:%d\n",
				prMddpTxd->version,
				prMddpTxd->sta_idx,
				prMddpTxd->wlan_idx,
				prMddpTxd->sta_mode,
				prMddpTxd->bss_id,
				prMddpTxd->wmmset,
				fgActivate);
		DBGLOG(NIC, INFO, "nw_if_name:[%c%c%c%c%c%c%c%c]\n",
				prMddpTxd->nw_if_name[0],
				prMddpTxd->nw_if_name[1],
				prMddpTxd->nw_if_name[2],
				prMddpTxd->nw_if_name[3],
				prMddpTxd->nw_if_name[4],
				prMddpTxd->nw_if_name[5],
				prMddpTxd->nw_if_name[6],
				prMddpTxd->nw_if_name[7]);

		kalMemCopy(prMddpTxd->aucMacAddr,
			prStaRec->aucMacAddr, MAC_ADDR_LEN);
		if (fgActivate)
			prMddpTxd->txd_length = NIC_TX_DESC_LONG_FORMAT_LENGTH;
		else
			prMddpTxd->txd_length = 0;

		kalMemCopy(prMddpTxd->txd,
			prStaRec->aprTxDescTemplate[0], prMddpTxd->txd_length);

		gMddpWFunc.notify_drv_info(prNotifyInfo);
		kalMemFree(buff, VIR_MEM_TYPE, u32BufSize);
	}

	return 0;
}

int32_t mddpNotifyDrvMac(IN struct ADAPTER *prAdapter)
{
	struct mddpw_drv_notify_info_t *prNotifyInfo;
	struct mddpw_drv_info_t *prDrvInfo;
	uint32_t u32BufSize = 0;
	uint8_t *buff = NULL;
	struct BSS_INFO *prP2pBssInfo = (struct BSS_INFO *) NULL;

	if (gMddpWFunc.notify_drv_info) {
		u32BufSize = (sizeof(struct mddpw_drv_notify_info_t) +
			sizeof(struct mddpw_drv_info_t) + MAC_ADDR_LEN);
		buff = kalMemAlloc(u32BufSize, VIR_MEM_TYPE);

		if (buff == NULL) {
			DBGLOG(NIC, ERROR, "Can't allocate TXD buffer.\n");
			return -1;
		}
		prNotifyInfo = (struct mddpw_drv_notify_info_t *) buff;
		prNotifyInfo->version = 0;
		/* (3= version+buf_len+info_num) */
		prNotifyInfo->buf_len = (u32BufSize-3);
		prNotifyInfo->info_num = 1;
		prDrvInfo = (struct mddpw_drv_info_t *) &(prNotifyInfo->buf[0]);
		prDrvInfo->info_id = MDDPW_DRV_INFO_DEVICE_MAC;
		prDrvInfo->info_len = MAC_ADDR_LEN;
    /*SY MCIF TBC 0916*/
		prP2pBssInfo = prAdapter->prAisBssInfo[0];
		COPY_MAC_ADDR(prDrvInfo->info, prP2pBssInfo->aucOwnMacAddr);

		gMddpWFunc.notify_drv_info(prNotifyInfo);
		kalMemFree(buff, VIR_MEM_TYPE, u32BufSize);
	}
	return 0;
}

int32_t mddpNotifyWifiStatus(IN enum mddp_drv_onoff_status wifiOnOffStatus)
{
	struct mddpw_drv_notify_info_t *prNotifyInfo;
	struct mddpw_drv_info_t *prDrvInfo;
	uint32_t u32BufSize = 0;
	uint8_t *buff = NULL;

	DBGLOG(INIT, INFO, "mddpNotifyWifiStatus power: %d.\n",
			wifiOnOffStatus);

	if (gMddpWFunc.notify_drv_info) {
		u32BufSize = (sizeof(struct mddpw_drv_notify_info_t) +
			sizeof(struct mddpw_drv_info_t) + sizeof(bool));
		buff = kalMemAlloc(u32BufSize, VIR_MEM_TYPE);

		if (buff == NULL) {
			DBGLOG(NIC, ERROR, "Can't allocate buffer.\n");
			return -1;
		}
		prNotifyInfo = (struct mddpw_drv_notify_info_t *) buff;
		prNotifyInfo->version = 0;
		prNotifyInfo->buf_len = u32BufSize;
		prNotifyInfo->info_num = 1;
		prDrvInfo = (struct mddpw_drv_info_t *) &(prNotifyInfo->buf[0]);
		prDrvInfo->info_id = MDDPW_DRV_INFO_NOTIFY_WIFI_ONOFF;
		prDrvInfo->info_len = WIFI_ONOFF_NOTIFICATION_LEN;
		prDrvInfo->info[0] = wifiOnOffStatus;

		gMddpWFunc.notify_drv_info(prNotifyInfo);
		kalMemFree(buff, VIR_MEM_TYPE, u32BufSize);
	}


	return 0;
}

void mddpNotifyWifiOnStart(void)
{
	mddpNotifyWifiStatus(MDDPW_DRV_INFO_WLAN_ON_START);
}

void mddpNotifyWifiOnEnd(void)
{
	mddpNotifyWifiStatus(MDDPW_DRV_INFO_WLAN_ON_END);
}

void mddpNotifyWifiOffStart(void)
{
	mddpNotifyWifiStatus(MDDPW_DRV_INFO_WLAN_OFF_START);
}

void mddpNotifyWifiOffEnd(void)
{
	mddpNotifyWifiStatus(MDDPW_DRV_INFO_WLAN_OFF_END);
}

int32_t mddpMdNotifyInfo(struct mddpw_md_notify_info_t *prMdInfo)
{
	struct GLUE_INFO *prGlueInfo = NULL;
	struct ADAPTER *prAdapter = NULL;

	DBGLOG(INIT, ERROR, "MD notify mddpMdNotifyInfo.\n");

	if (gPrDev == NULL) {
		DBGLOG(INIT, ERROR, "gPrDev is NULL.\n");
		return 0;
	}

	prGlueInfo = *((struct GLUE_INFO **)netdev_priv(gPrDev));
	if (prGlueInfo == NULL) {
		DBGLOG(INIT, ERROR, "prGlueInfo is NULL.\n");
		return 0;
	}
	prAdapter = prGlueInfo->prAdapter;
	if (prAdapter == NULL) {
		DBGLOG(INIT, ERROR, "prAdapter is NULL.\n");
		return 0;
	}

	if (prMdInfo->info_type == 1) {
		uint32_t i;
		struct BSS_INFO *prP2pBssInfo = (struct BSS_INFO *) NULL;

		DBGLOG(INIT, ERROR, "MD notify power on.\n");
		if (prGlueInfo->u4ReadyFlag) {
			mddpNotifyWifiOnStart();
			mddpNotifyWifiOnEnd();
			mddpNotifyDrvMac(prAdapter);
		}
		/* Notify STA's TXD to MD */
		for (i = 0; i < KAL_AIS_NUM; i++) {
			struct BSS_INFO *prAisBssInfo = aisGetAisBssInfo(
					prAdapter,
					i);

			if (prAisBssInfo && prAisBssInfo->eConnectionState ==
					MEDIA_STATE_CONNECTED)
				mddpNotifyDrvTxd(prAdapter,
						prAisBssInfo->prStaRecOfAP,
						TRUE);
		}
		/* Notify SAP clients' TXD to MD */
		prP2pBssInfo = cnmGetSapBssInfo(prAdapter);
		if (prP2pBssInfo) {
			struct LINK *prClientList;
			struct STA_RECORD *prCurrStaRec;

			prClientList = &prP2pBssInfo->rStaRecOfClientList;
			LINK_FOR_EACH_ENTRY(prCurrStaRec, prClientList,
					rLinkEntry, struct STA_RECORD) {
				mddpNotifyDrvTxd(prAdapter,
						prCurrStaRec,
						TRUE);
			}
		}
	}
	return 0;
}

int32_t mddpChangeState(enum mddp_state_e event, void *buf, uint32_t *buf_len)
{
	struct GLUE_INFO *prGlueInfo = NULL;
	struct ADAPTER *prAdapter = NULL;

	if (gPrDev == NULL) {
		DBGLOG(INIT, ERROR, "gPrDev is NULL.\n");
		return 0;
	}

	prGlueInfo = *((struct GLUE_INFO **)netdev_priv(gPrDev));
	if (prGlueInfo == NULL) {
		DBGLOG(INIT, ERROR, "prGlueInfo is NULL.\n");
		return 0;
	}
	prAdapter = prGlueInfo->prAdapter;
		if (prAdapter == NULL) {
			DBGLOG(INIT, ERROR, "prAdapter is NULL.\n");
			return 0;
		}

	switch (event) {
	case MDDP_STATE_ENABLING:
		break;

	case MDDP_STATE_ACTIVATING:
		break;

	case MDDP_STATE_ACTIVATED:
		DBGLOG(INIT, INFO, "Mddp activated.\n");
		prAdapter->fgMddpActivated = true;
		break;

	case MDDP_STATE_DEACTIVATING:
		break;

	case MDDP_STATE_DEACTIVATED:
		DBGLOG(INIT, INFO, "Mddp deactivated.\n");
		prAdapter->fgMddpActivated = false;
		break;

	case MDDP_STATE_DISABLING:
	case MDDP_STATE_UNINIT:
	case MDDP_STATE_CNT:
	case MDDP_STATE_DUMMY:
	default:
		break;
	}

	return 0;

}

#endif
