/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

/*! \file   "p2p_link.h"
 *  \brief
 */

#ifndef _P2P_LINK_H
#define _P2P_LINK_H

#if (CFG_SUPPORT_802_11BE_MLO == 1)
struct MLD_BSS_INFO *p2pGetMldBssInfo(
	IN struct ADAPTER *prAdapter,
	IN struct P2P_ROLE_FSM_INFO *prP2pRoleFsmInfo);

uint32_t p2pLinkProcessRxAuthReqFrame(
	IN struct ADAPTER *prAdapter,
	IN struct BSS_INFO *prBssInfo,
	IN struct STA_RECORD *prStaRec,
	IN struct SW_RFB *prSwRfb);

uint32_t p2pLinkProcessRxAssocReqFrame(
	IN struct ADAPTER *prAdapter,
	IN struct BSS_INFO *prBssInfo,
	IN struct STA_RECORD *prStaRec,
	IN struct SW_RFB *prSwRfb);

int8_t p2pMldBssInit(struct ADAPTER *prAdapter);

void p2MldBssUninit(struct ADAPTER *prAdapter);

#endif

void p2pLinkInitRoleFsm(IN struct ADAPTER *prAdapter);

void p2pLinkUninitRoleFsm(IN struct ADAPTER *prAdapter);

void p2pTargetBssDescResetConnecting(
	IN struct ADAPTER *prAdapter,
	IN struct P2P_ROLE_FSM_INFO *fsm);

void p2pClearAllLink(IN struct P2P_ROLE_FSM_INFO *prP2pRoleFsmInfo);

void p2pDeactivateAllLink(
	IN struct ADAPTER *prAdapter,
	IN struct P2P_ROLE_FSM_INFO *prP2pRoleFsmInfo,
	IN uint8_t fgClearStaRec);

void p2pFillLinkBssDesc(
	IN struct ADAPTER *prAdapter,
	IN struct P2P_ROLE_FSM_INFO *prP2pRoleFsmInfo,
	IN struct BSS_DESC_SET *prBssDescSet);

struct BSS_INFO *p2pGetLinkBssInfo(
	IN struct ADAPTER *prAdapter,
	IN struct P2P_ROLE_FSM_INFO *prP2pRoleFsmInfo,
	IN uint8_t ucLinkIdx);

void p2pGetLinkWmmQueSet(
	IN struct ADAPTER *prAdapter,
	IN struct BSS_INFO *prBssInfo);

void p2pSetLinkBssDesc(
	IN struct P2P_ROLE_FSM_INFO *prP2pRoleFsmInfo,
	IN struct BSS_DESC *prBssDesc,
	IN uint8_t ucLinkIdx);

struct BSS_DESC *p2pGetLinkBssDesc(
	IN struct P2P_ROLE_FSM_INFO *prP2pRoleFsmInfo,
	IN uint8_t ucLinkIdx);

uint8_t p2pGetLinkNum(
	IN struct P2P_ROLE_FSM_INFO *prP2pRoleFsmInfo);

void p2pSetLinkStaRec(
	IN struct P2P_ROLE_FSM_INFO *prP2pRoleFsmInfo,
	IN struct STA_RECORD *prStaRec,
	IN uint8_t ucLinkIdx);

struct STA_RECORD *p2pGetLinkStaRec(
	IN struct P2P_ROLE_FSM_INFO *prP2pRoleFsmInfo,
	IN uint8_t ucLinkIdx);

struct P2P_CHNL_REQ_INFO *p2pGetChnlReqInfo(
	IN struct ADAPTER *prAdapter,
	IN struct P2P_ROLE_FSM_INFO *prP2pRoleFsmInfo,
	IN uint8_t ucLinkIdx);

struct P2P_CONNECTION_REQ_INFO *p2pGetConnReqInfo(
	IN struct ADAPTER *prAdapter,
	IN struct P2P_ROLE_FSM_INFO *prP2pRoleFsmInfo,
	IN uint8_t ucLinkIdx);

void p2pLinkAcquireChJoin(
	IN struct ADAPTER *prAdapter,
	IN struct P2P_ROLE_FSM_INFO *prP2pRoleFsmInfo,
	IN struct P2P_CHNL_REQ_INFO *prChnlReq);

struct P2P_ROLE_FSM_INFO *p2pGetDefaultRoleFsmInfo(
	IN struct ADAPTER *prAdapter,
	IN enum ENUM_IFTYPE eIftype);

struct BSS_INFO *p2pGetDefaultLinkBssInfo(
	IN struct ADAPTER *prAdapter,
	IN enum ENUM_IFTYPE eIftype);

struct STA_RECORD *p2pGetDefaultLinkStaRec(
	IN struct ADAPTER *prAdapter,
	IN enum ENUM_IFTYPE eIftype);

uint16_t bssAssignAssocID(IN struct STA_RECORD *prStaRec);

#if (CFG_SUPPORT_802_11BE_MLO == 1)
void p2pScanFillSecondaryLink(struct ADAPTER *prAdapter,
	struct BSS_DESC_SET *prBssDescSet);
#endif

#endif /* !_P2P_MLO_H */
