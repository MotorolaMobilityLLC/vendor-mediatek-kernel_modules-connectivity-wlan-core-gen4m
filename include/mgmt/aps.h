/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2021 MediaTek Inc.
 */

#ifndef _APS_H
#define _APS_H

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */

/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */

struct AP_COLLECTION {
	struct LINK_ENTRY rLinkEntry;
	struct AP_COLLECTION *hnext; /* next entry in hash table list */
	struct BSS_DESC *aprTarget[MLD_LINK_MAX];
	struct LINK arLinks[MLD_LINK_MAX]; /* categorize AP by link */
	uint8_t ucLinkNum;
	uint8_t ucTotalCount; /* total BssDesc count */
	uint8_t aucMask[MLD_LINK_MAX];
	uint8_t fgIsMld;
	uint8_t fgIsMatchBssid;
	uint8_t fgIsMatchBssidHint;
	uint8_t fgIsAllLinkInBlackList;
	uint8_t aucAddr[MAC_ADDR_LEN]; /* mld addr or bssid */
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

/*******************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */

struct BSS_DESC *apsSearchBssDescByScore(struct ADAPTER *prAdapter,
	enum ENUM_CONN_ROAM_REASON eRoamReason,
	uint8_t ucBssIndex, struct BSS_DESC_SET *prBssDescSet);

#endif

