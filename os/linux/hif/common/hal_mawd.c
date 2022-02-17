/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2021 MediaTek Inc.
 */
/*! \file   hal_mawd.c
*    \brief  Internal driver stack will export
*    the required procedures here for GLUE Layer.
*
*    This file contains all routines which are exported
     from MediaTek 802.11 Wireless LAN driver stack to GLUE Layer.
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

#include "hif_pdma.h"

#include <linux/mm.h>
#ifndef CONFIG_X86
#include <asm/memory.h>
#endif

#include "mt66xx_reg.h"
#include "gl_kal.h"

#ifdef MT6639
#include "coda/mt6639/mawd_reg.h"
#include "coda/mt6639/wf_rro_top.h"
#endif

#if (CFG_SUPPORT_HOST_OFFLOAD == 1)

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */

#define MAWD_WFDMA_HIGH_ADDR 0x5
#define MAWD_WFDMA_ADDR_OFFSET \
	(CONN_INFRA_REMAPPING_OFFSET - 0xd0000 + 0x18020000)
#define MAWD_RRO_ADDR_OFFSET (WF_RRO_TOP_BASE - 0xda000)

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
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */
void halRroAllocMem(struct GLUE_INFO *prGlueInfo)
{
	struct GL_HIF_INFO *prHifInfo;
	struct HIF_MEM_OPS *prMemOps;
	struct RTMP_DMABUF *prCache, *prAddrArray, *prIndCmd;
	struct RRO_ADDR_ELEM *prAddrElem;
	uint32_t u4Idx, u4AddrNum;

	prHifInfo = &prGlueInfo->rHifInfo;
	prMemOps = &prHifInfo->rMemOps;
	prCache = &prHifInfo->BaBitmapCache;
	prAddrArray = &prHifInfo->AddrArray;
	prIndCmd = &prHifInfo->IndCmdRing;

	prCache->AllocSize =
		RRO_BA_BITMAP_SIZE * RRO_MAX_SESSION_NUM;
	if (prMemOps->allocRxDesc)
		prMemOps->allocRxDesc(prHifInfo, prCache, 0);

	if (prCache->AllocVa == NULL) {
		DBGLOG(HAL, ERROR, "BaBitmap allocation failed!!\n");
		return;
	}

	/* session num + particular */
	u4AddrNum = RRO_TOTAL_ADDR_ELEM_NUM * (RRO_MAX_SESSION_NUM + 1);
	prAddrArray->AllocSize = RRO_ADDR_ELEM_SIZE * u4AddrNum;
	if (prMemOps->allocRxDesc)
		prMemOps->allocRxDesc(prHifInfo, prAddrArray, 0);

	if (prAddrArray->AllocVa == NULL) {
		DBGLOG(HAL, ERROR, "ArrdArray allocation failed!!\n");
		return;
	}

	for (u4Idx = 0; u4Idx < u4AddrNum; u4Idx++) {
		prAddrElem = (struct RRO_ADDR_ELEM *)
			(prAddrArray->AllocVa +
			 u4Idx * sizeof(struct RRO_ADDR_ELEM));
		prAddrElem->signature_0 = 0x7;
		prAddrElem->signature_1 = 0x7;
	}

	prIndCmd->AllocSize =
		RRO_IND_CMD_RING_SIZE * sizeof(struct RRO_IND_CMD);
	if (prMemOps->allocRxDesc)
		prMemOps->allocRxDesc(prHifInfo, prIndCmd, 0);

	if (prIndCmd->AllocVa == NULL) {
		DBGLOG(HAL, ERROR, "IndCmd allocation failed!!\n");
		return;
	}
}

static void halRroFwCrSetup(struct GLUE_INFO *prGlueInfo)
{
	uint32_t u4Addr = 0, u4Val = 0, u4Cnt = 0;

	u4Addr = WF_RRO_TOP_GLOBAL_CONFG_ADDR;
	kalDevRegRead(prGlueInfo, u4Addr, &u4Val);
	u4Val |= BITS(2, 3);
	kalDevRegWrite(prGlueInfo, u4Addr, u4Val);

	for (u4Cnt = 0; u4Cnt < 10; u4Cnt++) {
		kalDevRegRead(prGlueInfo, u4Addr, &u4Val);
		if (u4Val & BIT(4))
			break;
		kalUdelay(5);
	}
	if (u4Cnt == 10)
		DBGLOG(HAL, ERROR, "Polling RRO_CACHE timeout");

	for (u4Cnt = 0; u4Cnt < 10; u4Cnt++) {
		kalDevRegRead(prGlueInfo, u4Addr, &u4Val);
		if (u4Val & BIT(5))
			break;
		kalUdelay(20);
	}
	if (u4Cnt == 10)
		DBGLOG(HAL, ERROR, "Polling RRO_SETBL timeout");

	kalDevRegRead(prGlueInfo, u4Addr, &u4Val);
	u4Val &= ~BITS(0, 1);
	kalDevRegWrite(prGlueInfo, u4Addr, u4Val);
}

static void halRroSetupBaBitmap(struct GLUE_INFO *prGlueInfo)
{
	struct GL_HIF_INFO *prHifInfo;
	struct RTMP_DMABUF *prCache;
	uint32_t u4Addr, u4Val;

	prHifInfo = &prGlueInfo->rHifInfo;
	prCache = &prHifInfo->BaBitmapCache;

	u4Addr = WF_RRO_TOP_BA_BITMAP_BASE_0_ADDR;
	u4Val = prCache->AllocPa;
	kalDevRegWrite(prGlueInfo, u4Addr, u4Val);

	u4Addr = WF_RRO_TOP_BA_BITMAP_BASE_1_ADDR;
	u4Val = prCache->AllocPa >> 32;
	kalDevRegWrite(prGlueInfo, u4Addr, u4Val);
}

static void halRroSetupAddressElement(struct GLUE_INFO *prGlueInfo)
{
	struct GL_HIF_INFO *prHifInfo;
	struct RTMP_DMABUF *prAddrArray;
	uint32_t u4Addr, u4Val;

	prHifInfo = &prGlueInfo->rHifInfo;
	prAddrArray = &prHifInfo->AddrArray;

	u4Addr = WF_RRO_TOP_ADDR_ARRAY_BASE_0_ADDR;
	u4Val = prAddrArray->AllocPa;
	kalDevRegWrite(prGlueInfo, u4Addr, u4Val);

	u4Addr = WF_RRO_TOP_ADDR_ARRAY_BASE_1_ADDR;
	u4Val = prAddrArray->AllocPa >> 32;
	kalDevRegWrite(prGlueInfo, u4Addr, u4Val);

	u4Addr = WF_RRO_TOP_BA_STA_CFG_ADDR;
	kalDevRegRead(prGlueInfo, u4Addr, &u4Val);
	u4Val = (u4Val & BITS(16, 31)) |
		((RRO_MAX_TID_NUM - 1) << 12) |
		(RRO_MAX_STA_NUM - 1);
	kalDevRegWrite(prGlueInfo, u4Addr, u4Val);
}

static void halRroSetupIndicateCmdRing(struct GLUE_INFO *prGlueInfo)
{
	struct GL_HIF_INFO *prHifInfo;
	struct RTMP_DMABUF *prIndCmd;
	uint32_t u4Addr, u4Val;

	prHifInfo = &prGlueInfo->rHifInfo;
	prIndCmd = &prHifInfo->IndCmdRing;

	prHifInfo->u4RroMagicCnt = 0;
	prHifInfo->u4IndCmdLastDmaIdx = 0;

	u4Addr = WF_RRO_TOP_IND_CMD_0_CTRL0_ADDR;
	u4Val = prIndCmd->AllocPa;
	kalDevRegWrite(prGlueInfo, u4Addr, u4Val);

	u4Addr = WF_RRO_TOP_IND_CMD_0_CTRL1_ADDR;
	u4Val = RRO_IND_CMD_RING_SIZE;
	kalDevRegWrite(prGlueInfo, u4Addr, u4Val);

	u4Addr = WF_RRO_TOP_IND_CMD_0_CTRL2_ADDR;
	u4Val = RRO_IND_CMD_RING_SIZE - 1;
	kalDevRegWrite(prGlueInfo, u4Addr, u4Val);

	u4Addr = WF_RRO_TOP_IND_CMD_SIGNATURE_BASE_0_ADDR;
	u4Val = MAWD_IND_CMD_SIGNATURE0;
	kalDevRegWrite(prGlueInfo, u4Addr, u4Val);

	u4Addr = WF_RRO_TOP_IND_CMD_SIGNATURE_BASE_1_ADDR;
	u4Val = WF_RRO_TOP_IND_CMD_SIGNATURE_BASE_1_EN_MASK |
		MAWD_WFDMA_HIGH_ADDR;
	kalDevRegWrite(prGlueInfo, u4Addr, u4Val);

	/* enable rro host interrupt */
	u4Addr = WF_RRO_TOP_HOST_INT_ENA_ADDR;
	u4Val = WF_RRO_TOP_HOST_INT_ENA_HOST_RRO_DONE_ENA_MASK;
	kalDevRegWrite(prGlueInfo, u4Addr, u4Val);
}

static void halRroSetupTimeoutConfig(struct GLUE_INFO *prGlueInfo)
{
	uint32_t u4Addr, u4Val;

	u4Addr = WF_RRO_TOP_TIMEOUT_CONF_0_TO_STEP_ONE_0_ADDR;
	u4Val = 9;
	kalDevRegWrite(prGlueInfo, u4Addr, u4Val);
}

void halRroMawdInit(struct GLUE_INFO *prGlueInfo)
{
	struct GL_HIF_INFO *prHifInfo;
	struct RTMP_DMABUF *prAddrArray, *prIndCmd;
	uint32_t u4Addr, u4Val;

	prHifInfo = &prGlueInfo->rHifInfo;
	prAddrArray = &prHifInfo->AddrArray;
	prIndCmd = &prHifInfo->IndCmdRing;

	/* setup addr array */
	u4Val = prAddrArray->AllocPa;
	u4Addr = MAWD_ADDR_ARRAY_BASE_L;
	kalDevRegWrite(prGlueInfo, u4Addr, u4Val);

	u4Val = prAddrArray->AllocPa >> 32;
	u4Addr = MAWD_ADDR_ARRAY_BASE_M;
	kalDevRegWrite(prGlueInfo, u4Addr, u4Val);

	/* setup ind cmd array */
	u4Addr = MAWD_IND_CMD_CTRL0;
	u4Val = prIndCmd->AllocPa;
	kalDevRegWrite(prGlueInfo, u4Addr, u4Val);

	u4Addr = MAWD_IND_CMD_CTRL1;
	u4Val = RRO_IND_CMD_RING_SIZE << 4;
	kalDevRegWrite(prGlueInfo, u4Addr, u4Val);

	/* setup ack sn */
	u4Addr = MAWD_RRO_ACK_SN_BASE_L;
	u4Val = WF_RRO_TOP_ACK_SN_CTRL_ADDR - MAWD_RRO_ADDR_OFFSET;
	kalDevRegWrite(prGlueInfo, u4Addr, u4Val);

	u4Addr = MAWD_RRO_ACK_SN_BASE_M;
	u4Val = MAWD_WFDMA_HIGH_ADDR;
	kalDevRegWrite(prGlueInfo, u4Addr, u4Val);
}

void halMawdAllocRxBlkRing(struct GLUE_INFO *prGlueInfo,
			   bool fgAllocMem)
{
	struct GL_HIF_INFO *prHifInfo;
	struct HIF_MEM_OPS *prMemOps;
	struct RTMP_RX_RING *pRxRing;
	struct RTMP_DMABUF *prRxDesc;
	struct RTMP_DMABUF *pDmaBuf;
	struct RTMP_DMACB *prRxCell;
	phys_addr_t RingBasePa;
	void *RingBaseVa;
	uint32_t u4BufSize, u4DescSize = sizeof(struct RX_BLK_DESC);
	uint32_t u4Idx = 0;

	prHifInfo = &prGlueInfo->rHifInfo;
	prMemOps = &prHifInfo->rMemOps;
	prRxDesc = &prHifInfo->RxBlkDescRing;

	/* Don't re-alloc memory when second time call alloc ring */
	prRxDesc->AllocSize = RX_RING_SIZE * u4DescSize;
	if (fgAllocMem && prMemOps->allocRxDesc)
		prMemOps->allocRxDesc(prHifInfo, prRxDesc, 0);

	if (prRxDesc->AllocVa == NULL) {
		DBGLOG(HAL, ERROR, "RxBlkRing allocation failed!!\n");
		return;
	}

	DBGLOG(HAL, TRACE, "RxBlkRing[%p]: total %lu bytes allocated\n",
		prRxDesc->AllocVa, prRxDesc->AllocSize);

	u4BufSize = RX_BUFFER_AGGRESIZE;

	/* Initialize Rx Ring and associated buffer memory */
	RingBasePa = prRxDesc->AllocPa;
	RingBaseVa = prRxDesc->AllocVa;

	pRxRing = &prHifInfo->RxBlkRing;
	pRxRing->u4BufSize = u4BufSize;
	pRxRing->u4RingSize = RX_RING_SIZE;
	pRxRing->fgRxSegPkt = FALSE;
	pRxRing->pvPacket = NULL;
	pRxRing->u4PacketLen = 0;

	for (u4Idx = 0; u4Idx < pRxRing->u4RingSize; u4Idx++) {
		/* Init RX Ring Size, Va, Pa variables */
		prRxCell = &pRxRing->Cell[u4Idx];
		prRxCell->AllocSize = u4DescSize;
		prRxCell->AllocVa = RingBaseVa;
		prRxCell->AllocPa = RingBasePa;
		prRxCell->prToken = NULL;

		RingBasePa += u4DescSize;
		RingBaseVa += u4DescSize;

		pDmaBuf = &prRxCell->DmaBuf;
		pDmaBuf->AllocSize = u4BufSize;
	}

	DBGLOG(HAL, TRACE, "RxBlkRing: total %d entry allocated\n", u4Idx);
}

void halMawdInitRxBlkRing(IN struct GLUE_INFO *prGlueInfo)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct BUS_INFO *prBusInfo = NULL;
	struct mt66xx_chip_info *prChipInfo;
	struct RTMP_RX_RING *prRxRing = NULL;
	uint32_t u4PhyAddr = 0;

	prHifInfo = &prGlueInfo->rHifInfo;
	prChipInfo = prGlueInfo->prAdapter->chip_info;
	prBusInfo = prChipInfo->bus_info;

	/* reset all RX Ring register */
	prRxRing = &prHifInfo->RxBlkRing;
	prRxRing->hw_desc_base = prBusInfo->mawd_rx_blk_ctrl0;
	prRxRing->hw_cidx_addr = prBusInfo->mawd_rx_blk_ctrl2;
	prRxRing->hw_cidx_mask = BITS(0, 11);
	prRxRing->hw_cidx_shift = 0;
	prRxRing->hw_didx_addr = prBusInfo->mawd_rx_blk_ctrl2;
	prRxRing->hw_didx_mask = BITS(12, 23);
	prRxRing->hw_didx_shift = 12;
	prRxRing->hw_cnt_addr = prBusInfo->mawd_rx_blk_ctrl1;
	prRxRing->hw_cnt_mask = BITS(16, 27);
	prRxRing->hw_cnt_shift = 16;

	u4PhyAddr = ((uint64_t)prRxRing->Cell[0].AllocPa &
		     DMA_LOWER_32BITS_MASK);
	prRxRing->RxCpuIdx = 0;
	kalDevRegWrite(prGlueInfo, prRxRing->hw_desc_base, u4PhyAddr);
	kalDevRegWrite(prGlueInfo, prRxRing->hw_cnt_addr,
		       prRxRing->u4RingSize << 16);

	DBGLOG(HAL, TRACE,
	       "-->RX_BLK_RING[0x%x]: Base=0x%x, Cnt=%d\n",
	       prRxRing->hw_desc_base,
	       u4PhyAddr, prRxRing->u4RingSize);
}

bool halMawdHashAdd(struct GL_HIF_INFO *prHifInfo, uint64_t u8Key,
		    struct sk_buff *prSkb, struct RX_CTRL_BLK *prRcb)
{
	struct RCB_NODE *prNewNode;

	if (halMawdHashSearch(prHifInfo, u8Key))
		return false;

	prNewNode = kalMemAlloc(sizeof(struct RCB_NODE), VIR_MEM_TYPE);
	prNewNode->u8Key = u8Key;
	prNewNode->prSkb = prSkb;
	prNewNode->prRcb = prRcb;

	hash_add_rcu(prHifInfo->arRcbHTbl, &prNewNode->rNode, u8Key);

	return true;
}

bool halMawdHashDel(struct GL_HIF_INFO *prHifInfo, uint64_t u8Key)
{
	struct RCB_NODE *prRcbNode;

	prRcbNode = halMawdHashSearch(prHifInfo, u8Key);
	if (prRcbNode) {
		hash_del_rcu(&prRcbNode->rNode);
		kalMemFree(prRcbNode, VIR_MEM_TYPE, BUF_SIZE);
	}

	return prRcbNode != NULL;
}

struct RCB_NODE *halMawdHashSearch(struct GL_HIF_INFO *prHifInfo,
				   uint64_t u8Key)
{
	struct RCB_NODE *prRcbNode;

	hash_for_each_possible_rcu(prHifInfo->arRcbHTbl,
				   prRcbNode, rNode, u8Key) {
		if (prRcbNode->u8Key == u8Key)
			return prRcbNode;
	}

	return NULL;
}

void halMawdAddNewRcbBlk(struct GL_HIF_INFO *prHifInfo,
			 struct sk_buff *prSkb)
{
	struct RX_CTRL_BLK *prRcb;

	prRcb = (struct RX_CTRL_BLK *)prSkb->cb;
	prRcb->prSkb = prSkb;
	list_add_tail(&prRcb->rNode, &prHifInfo->rRcbFreeList);
}

struct RX_CTRL_BLK *halMawdGetFreeRcbBlk(struct GL_HIF_INFO *prHifInfo)
{
	struct list_head *prNode;

	if (list_empty(&prHifInfo->rRcbFreeList)) {
		DBGLOG(HAL, ERROR, "rcb list is empty\n");
		return NULL;
	}

	prNode = prHifInfo->rRcbFreeList.next;
	list_del(prNode);
	list_add_tail(prNode, &prHifInfo->rRcbUsedList);

	return list_entry(prNode, struct RX_CTRL_BLK, rNode);
}

void halMawdPutFreeRcbBlk(struct GL_HIF_INFO *prHifInfo,
			  struct RX_CTRL_BLK *prRcb)
{
	struct list_head *prNode = &prRcb->rNode;

	list_del(prNode);
	list_add_tail(prNode, &prHifInfo->rRcbFreeList);
}

void halMawdAllocRcbList(struct GLUE_INFO *prGlueInfo)
{
	struct GL_HIF_INFO *prHifInfo;
	struct RX_CTRL_BLK *prRcb;
	struct sk_buff *prSkb;
	uint32_t u4Cnt;

	prHifInfo = &prGlueInfo->rHifInfo;

	INIT_LIST_HEAD(&prHifInfo->rRcbUsedList);
	INIT_LIST_HEAD(&prHifInfo->rRcbFreeList);
	hash_init(prHifInfo->arRcbHTbl);

	for (u4Cnt = 0; u4Cnt < RRO_PREALLOC_RX_BUF_NUM; u4Cnt++) {
		prSkb = dev_alloc_skb(CFG_RX_MAX_PKT_SIZE);
		if (!prSkb) {
			DBGLOG(HAL, ERROR,
			       "can't allocate rx %lu size packet\n",
			       CFG_RX_MAX_PKT_SIZE);
			break;
		}

		prRcb = (struct RX_CTRL_BLK *)prSkb->cb;
		prRcb->prSkb = prSkb;
		list_add_tail(&prRcb->rNode, &prHifInfo->rRcbFreeList);
	}

	DBGLOG(HAL, TRACE, "Alloc Rcb[%d]", u4Cnt);
}

static void halMawdFreeRcbList(struct GLUE_INFO *prGlueInfo)
{
	struct GL_HIF_INFO *prHifInfo;
	struct RX_CTRL_BLK *prRcb;
	struct RCB_NODE *prRcbNode;
	struct hlist_node *prTmpNode;
	struct list_head *prCur, *prNext;
	uint32_t u4Cnt = 0;
	int i4Bkt = 0;

	prHifInfo = &prGlueInfo->rHifInfo;

	list_for_each_safe(prCur, prNext, &prHifInfo->rRcbUsedList) {
		prRcb = list_entry(prCur, struct RX_CTRL_BLK, rNode);
		list_del(prCur);
		kalPacketFree(NULL, (void *)prRcb->prSkb);
	}

	list_for_each_safe(prCur, prNext, &prHifInfo->rRcbFreeList) {
		prRcb = list_entry(prCur, struct RX_CTRL_BLK, rNode);
		list_del(prCur);
		kalPacketFree(NULL, (void *)prRcb->prSkb);
	}

	hash_for_each_safe(prHifInfo->arRcbHTbl, i4Bkt,
			   prTmpNode, prRcbNode, rNode) {
		hash_del_rcu(&prRcbNode->rNode);
		kalMemFree(prRcbNode, VIR_MEM_TYPE, BUF_SIZE);
	}

	DBGLOG(HAL, TRACE, "Release Rcb[%d]", u4Cnt);
}

void halRroInit(struct GLUE_INFO *prGlueInfo)
{
	halRroFwCrSetup(prGlueInfo);
	halRroSetupBaBitmap(prGlueInfo);
	halRroSetupAddressElement(prGlueInfo);
	halRroSetupIndicateCmdRing(prGlueInfo);
	halRroSetupTimeoutConfig(prGlueInfo);

	halRroMawdInit(prGlueInfo);
}

void halRroUninit(struct GLUE_INFO *prGlueInfo)
{
	halMawdFreeRcbList(prGlueInfo);
}

uint32_t halMawdGetRxBlkDoneCnt(struct GLUE_INFO *prGlueInfo,
				uint8_t ucRingNum)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct RTMP_RX_RING *prRxRing = NULL;
	uint32_t u4MaxCnt = 0, u4CpuIdx = 0, u4DmaIdx = 0, u4RxPktCnt;

	prHifInfo = &prGlueInfo->rHifInfo;
	prRxRing = &prHifInfo->RxBlkRing;

	HAL_GET_RING_MCNT(prGlueInfo, prRxRing, &u4MaxCnt);
	HAL_GET_RING_CIDX(prGlueInfo, prRxRing, &u4CpuIdx);
	HAL_GET_RING_DIDX(prGlueInfo, prRxRing, &u4DmaIdx);

	if (u4MaxCnt == 0 || u4MaxCnt > RX_RING_SIZE)
		return 0;

	if (u4CpuIdx > u4DmaIdx)
		u4RxPktCnt = u4MaxCnt + u4DmaIdx - u4CpuIdx;
	else
		u4RxPktCnt = u4DmaIdx - u4CpuIdx;

	return u4RxPktCnt;
}

void halUpdateRFBInfo(struct ADAPTER *prAdapter, struct SW_RFB *prSwRfb)
{
	struct GLUE_INFO *prGlueInfo;
	struct mt66xx_chip_info *prChipInfo;
	struct RX_DESC_OPS_T *prRxDescOps;
	void *prRxStatus;

	prGlueInfo = prAdapter->prGlueInfo;
	prChipInfo = prAdapter->chip_info;
	prRxDescOps = prChipInfo->prRxDescOps;
	prRxStatus = prSwRfb->prRxStatus;

	if (!prRxStatus) {
		DBGLOG(HAL, ERROR, "prRxStatus is NULL\n");
		return;
	}

#if (CFG_DUMP_RXD == 1)
	DBGLOG(HAL, INFO, "Dump RXD:\n");
	dumpMemory8((uint8_t *)prRxStatus, prChipInfo->rxd_size);
#endif
	if (prRxDescOps->nic_rxd_get_pkt_type) {
		prSwRfb->ucPacketType =
			prRxDescOps->nic_rxd_get_pkt_type(prRxStatus);
	}
#if DBG
	if (prRxDescOps->nic_rxd_get_sec_mode) {
		DBGLOG_LIMITED(RX, LOUD, "ucPacketType = %u, ucSecMode = %u\n",
			       prSwRfb->ucPacketType,
			       prRxDescOps->nic_rxd_get_sec_mode(prRxStatus));
	}
#endif /* DBG */

	GLUE_RX_SET_PKT_INT_TIME(prSwRfb->pvPacket,
				 prGlueInfo->u8HifIntTime);
	GLUE_RX_SET_PKT_RX_TIME(prSwRfb->pvPacket, sched_clock());

	if (prRxDescOps->nic_rxd_get_wlan_idx) {
		prSwRfb->ucStaRecIdx =
			secGetStaIdxByWlanIdx(
				prAdapter,
				prRxDescOps->nic_rxd_get_wlan_idx(prRxStatus));
	}
}

static bool halMawdHandleRxblk(struct ADAPTER *prAdapter,
			       struct RX_CTRL_BLK *prRcb,
			       uint32_t u4Port,
			       uint32_t u4Reason)
{
	struct GLUE_INFO *prGlueInfo;
	struct GL_HIF_INFO *prHifInfo;
	struct RX_CTRL *prRxCtrl;
	struct SW_RFB *prSwRfb;

	KAL_SPIN_LOCK_DECLARATION();

	prGlueInfo = prAdapter->prGlueInfo;
	prHifInfo = &prGlueInfo->rHifInfo;
	prRxCtrl = &prAdapter->rRxCtrl;

	KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_RX_FREE_QUE);
	QUEUE_REMOVE_HEAD(&prRxCtrl->rFreeSwRfbList,
			  prSwRfb, struct SW_RFB *);
	KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_RX_FREE_QUE);
	if (!prSwRfb) {
		DBGLOG_LIMITED(
			RX, WARN, "No More RFB for P[%u]\n", u4Port);
		prAdapter->u4NoMoreRfb |= BIT(u4Port);
		return false;
	}

	halMawdAddNewRcbBlk(prHifInfo, (struct sk_buff *)prSwRfb->pvPacket);
	list_del(&prRcb->rNode);

	prSwRfb->pvPacket = (void *)prRcb->prSkb;
	prSwRfb->pucRecvBuff = prRcb->prSkb->data;
	prSwRfb->prRxStatus = (void *)prSwRfb->pucRecvBuff;
	prSwRfb->u4IndReason = u4Reason;

	halUpdateRFBInfo(prAdapter, prSwRfb);

	KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_RX_QUE);
#if RRO_DROP_BY_HIF
	if (u4Reason == RRO_REPEAT || u4Reason == RRO_OLDPKT) {
		QUEUE_INSERT_TAIL(&prRxCtrl->rFreeSwRfbList,
				  &prSwRfb->rQueEntry);
		DBGLOG(HAL, INFO, "Drop skb by hif[%d]\n", u4Reason);
	} else {
		QUEUE_INSERT_TAIL(&prRxCtrl->rReceivedRfbList,
				  &prSwRfb->rQueEntry);
	}
#else
	QUEUE_INSERT_TAIL(&prRxCtrl->rReceivedRfbList,
			  &prSwRfb->rQueEntry);
#endif
	RX_INC_CNT(prRxCtrl, RX_MPDU_TOTAL_COUNT);
	DBGLOG(RX, TEMP, "Recv p=%p total:%lu\n",
	       prSwRfb, RX_GET_CNT(prRxCtrl, RX_MPDU_TOTAL_COUNT));
	kalTraceEvent("Recv p=%p total:%lu",
		      prSwRfb, RX_GET_CNT(prRxCtrl, RX_MPDU_TOTAL_COUNT));
	KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_RX_QUE);

	return true;
}

static uint32_t halMawdReadRxBlkRing(struct GLUE_INFO *prGlueInfo,
				     uint16_t u2Port,
				     struct RX_CTRL_BLK **pvRcb,
				     uint32_t *u4Reason)
{
	struct GL_HIF_INFO *prHifInfo;
	struct HIF_MEM_OPS *prMemOps;
	struct RTMP_RX_RING *prRxRing;
	struct RTMP_DMACB *prRxCell;
	struct RX_BLK_DESC *prRxBlkD;
	struct RCB_NODE *prRcbHNode;
	uint64_t u8Addr;
	uint32_t u4Cnt = 0;

	prHifInfo = &prGlueInfo->rHifInfo;
	prMemOps = &prHifInfo->rMemOps;
	prRxRing = &prHifInfo->RxBlkRing;

	prRxCell = &prRxRing->Cell[prRxRing->RxCpuIdx];
	prRxBlkD = (struct RX_BLK_DESC *)prRxCell->AllocVa;

	u8Addr = prRxBlkD->addr;
	DBGLOG(HAL, INFO, "rx blk msdu cnt[%d]\n", prRxBlkD->msdu_cnt);
	dumpMemory32((uint32_t *)prRxBlkD, sizeof(struct RX_BLK_DESC));
	prRcbHNode = halMawdHashSearch(prHifInfo, u8Addr);
	if (!prRcbHNode) {
		DBGLOG(HAL, ERROR, "cannot find RCB[%d]\n",
		       prRxRing->RxCpuIdx);
		dumpMemory32((uint32_t *)prRxBlkD, 8);
		goto skip;
	}
	if (!prRcbHNode->prSkb) {
		DBGLOG(HAL, ERROR, "rcb[%d] skb is null\n",
		       prRxRing->RxCpuIdx);
		goto skip;
	}
	if (!prRcbHNode->prRcb) {
		DBGLOG(HAL, ERROR, "rcb[%d] rcb is null\n",
		       prRxRing->RxCpuIdx);
		goto skip;
	}

	if (prMemOps->unmapRxBuf)
		prMemOps->unmapRxBuf(prHifInfo, (phys_addr_t)u8Addr,
				     CFG_RX_MAX_PKT_SIZE);
	u4Cnt = prRxBlkD->msdu_cnt;
	*u4Reason = prRxBlkD->ind_reason;
	*pvRcb = prRcbHNode->prRcb;

skip:
	INC_RING_INDEX(prRxRing->RxCpuIdx, prRxRing->u4RingSize);

	DBGLOG(HAL, INFO, "rx blk ring idx[%d]\n", prRxRing->RxCpuIdx);

	return u4Cnt;
}

void halMawdReadRxBlks(struct ADAPTER *prAdapter, uint32_t u4Port)
{
	struct GLUE_INFO *prGlueInfo;
	struct GL_HIF_INFO *prHifInfo;
	struct RTMP_RX_RING *prRxRing;
	struct RX_CTRL_BLK *prRcb, *prNextRcb;
	uint32_t u4RxCnt, u4MsduCnt, u4Idx, u4Reason;

	prGlueInfo = prAdapter->prGlueInfo;
	prHifInfo = &prGlueInfo->rHifInfo;
	prRxRing = &prHifInfo->RxBlkRing;

	u4RxCnt = halMawdGetRxBlkDoneCnt(prGlueInfo, u4Port);

	DBGLOG(RX, TEMP, "halRxReceiveRFBs: u4RxCnt:%d\n", u4RxCnt);

	while (u4RxCnt) {
		u4MsduCnt = halMawdReadRxBlkRing(prGlueInfo, u4Port,
						 &prRcb, &u4Reason);
		for (u4Idx = 0; u4Idx < u4MsduCnt; u4Idx++) {
			prNextRcb = list_entry(
				prRcb->rNode.next,
				struct RX_CTRL_BLK, rNode);
			halMawdHandleRxblk(prAdapter, prRcb, u4Port, u4Reason);
			prRcb = prNextRcb;
		}
		u4RxCnt--;
	}

	HAL_SET_RING_CIDX(prGlueInfo, prRxRing, prRxRing->RxCpuIdx);

	prRxRing->u4PendingCnt = u4RxCnt;
}

void halMawdUpdateWfdmaRxBlk(struct GLUE_INFO *prGlueInfo, uint16_t u2Port)
{
	struct GL_HIF_INFO *prHifInfo;
	struct HIF_MEM_OPS *prMemOps;
	struct RTMP_RX_RING *prRxRing;
	struct RTMP_DMABUF *pDmaBuf;
	struct RTMP_DMACB *prRxCell;
	struct RXD_STRUCT *pRxD;
	uint32_t u4CpuIdx, u4Cnt, u4Idx;

	prHifInfo = &prGlueInfo->rHifInfo;
	prMemOps = &prHifInfo->rMemOps;
	prRxRing = &prHifInfo->RxRing[u2Port];

	kalDevRegRead(prGlueInfo, prRxRing->hw_cidx_addr,
		      &prRxRing->RxCpuIdx);
	u4CpuIdx = prRxRing->RxCpuIdx;
	u4Cnt = halWpdmaGetRxDmaDoneCnt(prGlueInfo, u2Port);

	DBGLOG(HAL, INFO, "update rx blk cnt:[%d]!\n", u4Cnt);

	if (u4Cnt == 0)
		return;

	for (u4Idx = 0; u4Idx < u4Cnt; u4Idx++) {
		INC_RING_INDEX(u4CpuIdx, prRxRing->u4RingSize);

		prRxCell = &prRxRing->Cell[u4CpuIdx];
		pDmaBuf = &prRxCell->DmaBuf;

		if (prMemOps->allocRxBuf)
			prRxCell->pPacket = prMemOps->allocRxBuf(
				prHifInfo, pDmaBuf, u2Port, u4Idx);
		if (pDmaBuf->AllocVa == NULL) {
			log_dbg(HAL, ERROR,
				"Failed to allocate Rx blk buffer idx[%u]\n",
				u4Idx);
			break;
		}

		pRxD = (struct RXD_STRUCT *)prRxCell->AllocVa;
		pRxD->SDPtr0 = ((uint64_t)pDmaBuf->AllocPa) &
			DMA_LOWER_32BITS_MASK;
#ifdef CONFIG_PHYS_ADDR_T_64BIT
		pRxD->SDPtr1 = (((uint64_t)pDmaBuf->AllocPa >>
			DMA_BITS_OFFSET) & DMA_HIGHER_4BITS_MASK);
#else
		pRxD->SDPtr1 = 0;
#endif
		pRxD->SDLen0 = pDmaBuf->AllocSize;
		pRxD->DMADONE = 0;
	}

	prRxRing->RxCpuIdx = u4CpuIdx;
	kalDevRegWrite(prGlueInfo, prRxRing->hw_cidx_addr,
		       prRxRing->RxCpuIdx);
}

void halMawdUpdateIndCmd(struct GLUE_INFO *prGlueInfo)
{
	struct GL_HIF_INFO *prHifInfo;
	struct RTMP_DMABUF *prIndCmd;
	struct RRO_IND_CMD *aurIndCmd;
	uint32_t u4Addr, u4Val, u4DmaIdx;

	prHifInfo = &prGlueInfo->rHifInfo;
	prIndCmd = &prHifInfo->IndCmdRing;
	aurIndCmd = (struct RRO_IND_CMD *)prIndCmd->AllocVa;

	u4Addr = WF_RRO_TOP_IND_CMD_0_CTRL3_ADDR;
	kalDevRegRead(prGlueInfo, u4Addr, &u4Val);
	u4DmaIdx = u4Val & BITS(0, 11);

	if ((u4DmaIdx == 0) && prHifInfo->u4IndCmdLastDmaIdx) {
		DBGLOG(HAL, INFO,
		       "ind cmd ctrl3[0x%08x] sw_magic_cnt[%d] last[%d]\n",
		       u4Val,
		       prHifInfo->u4RroMagicCnt,
		       prHifInfo->u4IndCmdLastDmaIdx);
		aurIndCmd[RRO_IND_CMD_RING_SIZE - 1].magic_cnt =
			prHifInfo->u4RroMagicCnt;
		INC_RING_INDEX(prHifInfo->u4RroMagicCnt, 8);
		u4Val = (u4Val & BITS(0, 11)) | u4Val & BIT(31) |
			(prHifInfo->u4RroMagicCnt << 28);
		kalDevRegWrite(prGlueInfo, u4Addr, u4Val);
	} else if (u4DmaIdx == (RRO_IND_CMD_RING_SIZE - 1)) {
		aurIndCmd[RRO_IND_CMD_RING_SIZE - 1].magic_cnt =
			prHifInfo->u4RroMagicCnt;
	} else {
		prHifInfo->u4RroMagicCnt =
			(u4Val & BITS(28, 30)) >> 28;
	}
	prHifInfo->u4IndCmdLastDmaIdx = u4DmaIdx;

	u4Addr = MAWD_IND_CMD_SIGNATURE0;
	kalDevRegWrite(prGlueInfo, u4Addr, u4Val);
	DBGLOG(HAL, INFO, "update ind cmd[0x%08x]\n", u4Val);
}

static void updateMawdSram(
	struct GLUE_INFO *prGlueInfo,
	uint32_t u4Offset,
	uint32_t u4ValL,
	uint32_t u4ValH)
{
	struct BUS_INFO *prBusInfo;
	uint32_t u4Val;

	prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;

	kalDevRegWrite(prGlueInfo, prBusInfo->mawd_settings0, u4ValL);
	kalDevRegWrite(prGlueInfo, prBusInfo->mawd_settings1, u4ValH);
	kalDevRegRead(prGlueInfo, prBusInfo->mawd_settings2, &u4Val);
	u4Val = (u4Val & BITS(16, 25)) | (u4Offset & BITS(0, 6));
	kalDevRegWrite(prGlueInfo, prBusInfo->mawd_settings2, u4Val);

	kalDevRegRead(prGlueInfo, prBusInfo->mawd_settings4, &u4Val);
	while (u4Val & BIT(8)) {
		kalUdelay(10);
		kalDevRegRead(prGlueInfo, prBusInfo->mawd_settings4, &u4Val);
	}
	kalDevRegWrite(prGlueInfo, prBusInfo->mawd_settings4, BIT(0));

	DBGLOG(HAL, TRACE, "Update SRAM[%d] H[0x%08x] L[0x%08x]",
	       u4Offset, u4ValH, u4ValL);
}

static void halMawdSetupTxRing(struct GLUE_INFO *prGlueInfo,
			      struct RTMP_TX_RING *prWfdmaTxRing,
			      struct RTMP_TX_RING *prTxRing,
			      uint32_t u4Idx,
			      uint32_t u4PhyAddr)
{
	struct mt66xx_chip_info *prChipInfo;
	struct BUS_INFO *prBusInfo;
	struct RTMP_DMACB *prTxCell = &prTxRing->Cell[0];
	uint32_t u4Base = u4Idx, u4Val = 0, u4DWCnt;
	uint32_t u4HifTxdOffset = u4Base * 0xC;
	uint32_t u4WfdmaOffset = u4Base * 0x14;

	prChipInfo = prGlueInfo->prAdapter->chip_info;
	prBusInfo = prChipInfo->bus_info;

	kalDevRegWrite(prGlueInfo,
		       prBusInfo->mawd_ring_ctrl0 + u4WfdmaOffset,
		       u4PhyAddr);
	kalDevRegWrite(prGlueInfo,
		       prBusInfo->mawd_ring_ctrl1 + u4WfdmaOffset,
		       prWfdmaTxRing->hw_cidx_addr - MAWD_WFDMA_ADDR_OFFSET);
	kalDevRegWrite(prGlueInfo,
		       prBusInfo->mawd_ring_ctrl2 + u4WfdmaOffset,
		       prWfdmaTxRing->hw_didx_addr - MAWD_WFDMA_ADDR_OFFSET);
	u4Val = (TX_RING_SIZE << 19) | (TXD_SIZE << 12) |
		(MAWD_WFDMA_HIGH_ADDR << 4) | MAWD_WFDMA_HIGH_ADDR;
	kalDevRegWrite(prGlueInfo,
		       prBusInfo->mawd_ring_ctrl3 + u4WfdmaOffset,
		       u4Val);

	prTxRing->hw_desc_base =
		prBusInfo->mawd_hif_txd_ctrl0 + u4HifTxdOffset;

	prTxRing->hw_cidx_addr =
		prBusInfo->mawd_hif_txd_ctrl2 + u4HifTxdOffset;
	prTxRing->hw_cidx_mask = BITS(0, 12);
	prTxRing->hw_cidx_shift = 0;

	prTxRing->hw_didx_addr =
		prBusInfo->mawd_hif_txd_ctrl2 + u4HifTxdOffset;
	prTxRing->hw_didx_mask = BITS(16, 28);
	prTxRing->hw_didx_shift = 16;

	prTxRing->hw_cnt_addr =
		prBusInfo->mawd_hif_txd_ctrl1 + u4HifTxdOffset;
	prTxRing->hw_cnt_mask = BITS(16, 28);
	prTxRing->hw_cnt_shift = 16;

	kalDevRegWrite(prGlueInfo, prTxRing->hw_desc_base,
		       prTxCell->AllocPa);
	/* setup tx ring/hif txd size */
	u4DWCnt = BYTE_TO_DWORD(NIC_TX_DESC_LONG_FORMAT_LENGTH +
				prChipInfo->hif_txd_append_size);
	u4Val = (TX_RING_SIZE << 16) | (u4DWCnt << 8);
	kalDevRegWrite(prGlueInfo, prTxRing->hw_cnt_addr, u4Val);
}

static void halMawdInitErrRptRing(struct GLUE_INFO *prGlueInfo)
{
	struct GL_HIF_INFO *prHifInfo;
	struct BUS_INFO *prBusInfo;
	struct RTMP_DMABUF *prErrRpt;
	uint32_t u4Val;

	prHifInfo = &prGlueInfo->rHifInfo;
	prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;
	prErrRpt = &prHifInfo->ErrRptRing;

	kalDevRegWrite(prGlueInfo,
		       prBusInfo->mawd_err_rpt_ctrl0,
		       prErrRpt->AllocPa);
	u4Val = (RX_RING_SIZE << 16) | (32 << 8);
	kalDevRegWrite(prGlueInfo,
		       prBusInfo->mawd_err_rpt_ctrl1,
		       u4Val);
}

void halMawdUpdateL2Tbl(struct GLUE_INFO *prGlueInfo,
		    union mawd_l2tbl rL2Tbl, uint32_t u4Set)
{
	struct GL_HIF_INFO *prHifInfo;
	struct BUS_INFO *prBusInfo;
	uint32_t u4Idx = 0, u4Val = 0;

	prHifInfo = &prGlueInfo->rHifInfo;
	prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;

	for (u4Idx = 0; u4Idx < 4; u4Idx++) {
		updateMawdSram(prGlueInfo,
			       u4Set * 4 + u4Idx,
			       rL2Tbl.data[u4Idx * 2],
			       rL2Tbl.data[u4Idx * 2 + 1]);
	}

	kalDevRegRead(prGlueInfo, prBusInfo->mawd_settings2, &u4Val);
	u4Val = (prHifInfo->u4MawdL2TblCnt << 21) | (u4Val & BITS(0, 20));
	kalDevRegWrite(prGlueInfo, prBusInfo->mawd_settings2, u4Val);

	kalDevRegRead(prGlueInfo, prBusInfo->mawd_settings3, &u4Val);
	u4Val |= BIT(u4Set);
	kalDevRegWrite(prGlueInfo, prBusInfo->mawd_settings3, u4Val);
}

static bool halMawdAllocHifTxRing(struct GLUE_INFO *prGlueInfo,
				  uint32_t u4Num,
				  uint32_t u4Size,
				  uint32_t u4DescSize,
				  bool fgAllocMem)
{
	struct GL_HIF_INFO *prHifInfo;
	struct HIF_MEM_OPS *prMemOps;
	struct RTMP_TX_RING *pTxRing;
	struct RTMP_DMABUF *prTxDesc;
	struct RTMP_DMACB *prTxCell;
	phys_addr_t RingBasePa;
	void *RingBaseVa;
	uint32_t u4Idx;

	prHifInfo = &prGlueInfo->rHifInfo;
	prMemOps = &prHifInfo->rMemOps;
	prTxDesc = &prHifInfo->HifTxDescRing[u4Num];

	prTxDesc->AllocSize = u4Size * u4DescSize;
	if (fgAllocMem && prMemOps->allocTxDesc)
		prMemOps->allocTxDesc(prHifInfo, prTxDesc, u4Num);

	if (prTxDesc->AllocVa == NULL) {
		DBGLOG(HAL, ERROR,
		       "MawdTxDescRing[%d] allocation failed\n", u4Num);
		return false;
	}

	/* Save PA & VA for further operation */
	RingBasePa = prTxDesc->AllocPa;
	RingBaseVa = prTxDesc->AllocVa;

	/*
	 * Initialize Tx Ring Descriptor and associated buffer memory
	 */
	pTxRing = &prHifInfo->MawdTxRing[u4Num];
	for (u4Idx = 0; u4Idx < u4Size; u4Idx++) {
		/* Init Tx Ring Size, Va, Pa variables */
		prTxCell = &pTxRing->Cell[u4Idx];
		prTxCell->pPacket = NULL;
		prTxCell->pBuffer = NULL;

		prTxCell->AllocSize = u4DescSize;
		prTxCell->AllocVa = RingBaseVa;
		prTxCell->AllocPa = RingBasePa;

		RingBasePa += u4DescSize;
		RingBaseVa += u4DescSize;
	}

	return true;
}

bool halMawdAllocTxRing(struct GLUE_INFO *prGlueInfo, bool fgAllocMem)
{
	struct mt66xx_chip_info *prChipInfo;
	struct GL_HIF_INFO *prHifInfo;
	struct HIF_MEM_OPS *prMemOps;
	struct RTMP_DMABUF *prErrRpt;
	int32_t u4Num, u4TxDSize;

	prHifInfo = &prGlueInfo->rHifInfo;
	prChipInfo = prGlueInfo->prAdapter->chip_info;
	prMemOps = &prHifInfo->rMemOps;
	prErrRpt = &prHifInfo->ErrRptRing;

	u4TxDSize = NIC_TX_DESC_LONG_FORMAT_LENGTH +
		     prChipInfo->hif_txd_append_size;

	for (u4Num = 0; u4Num < MAWD_MD_TX_RING_NUM; u4Num++) {
		if (!halMawdAllocHifTxRing(prGlueInfo, u4Num, TX_RING_SIZE,
					   u4TxDSize, fgAllocMem)) {
			DBGLOG(HAL, ERROR, "AllocMawdTxRing[%d] fail\n",
			       u4Num);
			return false;
		}
		prHifInfo->MawdTxRing[u4Num].TxSwUsedIdx = 0;
		prHifInfo->MawdTxRing[u4Num].TxCpuIdx = 0;
	}

	prErrRpt->AllocSize = RX_RING_SIZE * 4;
	if (prMemOps->allocRxDesc)
		prMemOps->allocRxDesc(prHifInfo, prErrRpt, 0);

	if (prErrRpt->AllocVa == NULL) {
		DBGLOG(HAL, ERROR, "ErrRpt allocation failed!!\n");
		return false;
	}

	return true;
}

void halMawdInitTxRing(struct GLUE_INFO *prGlueInfo)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct BUS_INFO *prBusInfo = NULL;
	struct mt66xx_chip_info *prChipInfo = NULL;
	struct RTMP_TX_RING *prTxRing = NULL;
	struct RTMP_TX_RING *prMawdTxRing = NULL;
	struct RTMP_DMACB *prTxCell = NULL;
	uint32_t u4Idx = 0, u4PhyAddr = 0;

	prHifInfo = &prGlueInfo->rHifInfo;
	prChipInfo = prGlueInfo->prAdapter->chip_info;
	prBusInfo = prChipInfo->bus_info;

	for (u4Idx = 0; u4Idx < MAWD_MD_TX_RING_NUM; u4Idx++) {
		prTxRing = &prHifInfo->TxRing[u4Idx];
		prMawdTxRing = &prHifInfo->MawdTxRing[u4Idx];
		prTxCell = &prTxRing->Cell[0];
		u4PhyAddr = ((uint64_t)prTxCell->AllocPa) &
			DMA_LOWER_32BITS_MASK;

		halMawdSetupTxRing(prGlueInfo, prTxRing,
				   prMawdTxRing, u4Idx, u4PhyAddr);

		DBGLOG(HAL, TRACE,
		       "-->MAWD TX_RING_%d[0x%x]: Base=0x%x, Cnt=%d!\n",
		       u4Idx, prHifInfo->TxRing[u4Idx].hw_desc_base,
		       u4PhyAddr, TX_RING_SIZE);
	}

	halMawdInitErrRptRing(prGlueInfo);
}

static uint8_t halMawdTxRingDataSelect(struct ADAPTER *prAdapter,
				       struct MSDU_INFO *prMsduInfo)
{
	return prMsduInfo->ucWmmQueSet;
}

bool halMawdFillTxRing(struct GLUE_INFO *prGlueInfo,
		       struct MSDU_TOKEN_ENTRY *prToken)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct mt66xx_chip_info *prChipInfo;
	struct RTMP_TX_RING *prTxRing, *prWfdmaTxRing;
	struct RTMP_DMACB *pTxCell;
	uint16_t u2Port = TX_RING_DATA0_IDX_0;

	prHifInfo = &prGlueInfo->rHifInfo;
	prChipInfo = prGlueInfo->prAdapter->chip_info;

	u2Port = halTxRingDataSelect(
		prGlueInfo->prAdapter, prToken->prMsduInfo);
	prTxRing = &prHifInfo->MawdTxRing[u2Port];
	prWfdmaTxRing = &prHifInfo->TxRing[u2Port];

	pTxCell = &prTxRing->Cell[prTxRing->TxCpuIdx];
	prToken->u4CpuIdx = prTxRing->TxCpuIdx;
	prToken->u2Port = u2Port;
	pTxCell->prToken = prToken;

#if (CFG_DUMP_TXDMAD == 1)
	DBGLOG(HAL, INFO, "Dump TXDMAD:\n");
	dumpMemory8((uint8_t *)pTxD, sizeof(struct TXD_STRUCT));
#endif

	/* Increase TX_CTX_IDX, but write to register later. */
	INC_RING_INDEX(prTxRing->TxCpuIdx, TX_RING_SIZE);

	/* Update HW Tx DMA ring */
	prTxRing->u4UsedCnt++;
	prWfdmaTxRing->u4UsedCnt += 2;
	HAL_SET_RING_CIDX(prGlueInfo, prTxRing, prTxRing->TxCpuIdx);

	DBGLOG_LIMITED(HAL, TRACE,
		"MAWD Tx Data:Ring%d CPU idx[0x%x] Used[%u]\n",
		u2Port, prTxRing->TxCpuIdx, prTxRing->u4UsedCnt);

	GLUE_INC_REF_CNT(prGlueInfo->prAdapter->rHifStats.u4DataTxCount);

	return TRUE;
}

void halMawdReset(struct GLUE_INFO *prGlueInfo)
{
	uint32_t u4Addr = 0, u4Val = 0;

	u4Addr = MAWD_AXI_SLEEP_PROT_SETTING;
	kalDevRegRead(prGlueInfo, u4Addr, &u4Val);
	u4Val |= BIT(0);
	kalDevRegWrite(prGlueInfo, u4Addr, u4Val);

	kalDevRegRead(prGlueInfo, u4Addr, &u4Val);
	while ((u4Val & BITS(5, 6)) != BITS(5, 6)) {
		kalDevRegRead(prGlueInfo, u4Addr, &u4Val);
		kalUdelay(20);
	}

	u4Addr = MAWD_SOFTRESET;
	u4Val = BITS(0, 2);
	kalDevRegWrite(prGlueInfo, u4Addr, u4Val);

	u4Addr = MAWD_POWER_UP;
	kalDevRegRead(prGlueInfo, u4Addr, &u4Val);
	while ((u4Val & BITS(8, 12)) != BITS(8, 12)) {
		kalDevRegRead(prGlueInfo, u4Addr, &u4Val);
		kalUdelay(20);
	}

	u4Addr = MAWD_AXI_SLEEP_PROT_SETTING;
	kalDevRegRead(prGlueInfo, u4Addr, &u4Val);
	u4Val &= ~BIT(0);
	kalDevRegWrite(prGlueInfo, u4Addr, u4Val);
}

#endif /* CFG_SUPPORT_HOST_OFFLOAD == 1 */
