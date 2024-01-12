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

#ifdef MT6655
#include "coda/mt6655/mawd_reg.h"
#include "coda/mt6655/wf_rro_top.h"
#endif

#if (CFG_SUPPORT_HOST_OFFLOAD == 1)

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */
#if (CFG_MTK_FPGA_PLATFORM == 1)
#define MAWD_WFDMA_HIGH_ADDR	0x5
#define MAWD_WFDMA_LOW_ADDR	0x0
#define MAWD_RRO_ADDR_OFFSET	(WF_RRO_TOP_BASE - 0xDA000)
#else
#define MAWD_WFDMA_HIGH_ADDR	0x0
#define MAWD_WFDMA_LOW_ADDR	0x18000000
#define MAWD_RRO_ADDR_OFFSET	(WF_RRO_TOP_BASE - 0x300DA000)
#endif

#define MAWD_WFDMA_ADDR_OFFSET	\
	(CONN_INFRA_REMAPPING_OFFSET - 0xD0000 + 0x18020000)

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
static void halMawdBackupCr(struct GLUE_INFO *prGlueInfo,
			    uint32_t u4Addr, uint32_t u4Val);
static u_int8_t halRroHashAdd(struct GL_HIF_INFO *prHifInfo, uint64_t u8Key,
			      struct sk_buff *prSkb, struct RX_CTRL_BLK *prRcb);
static u_int8_t halRroHashDel(struct GL_HIF_INFO *prHifInfo, uint64_t u8Key);
static struct RCB_NODE *halRroHashSearch(
	struct GL_HIF_INFO *prHifInfo, uint64_t u8Key);

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */
void halMawdWakeup(struct GLUE_INFO *prGlueInfo)
{
	struct GL_HIF_INFO *prHifInfo;
	struct RTMP_RX_RING *prRxRing;
	uint32_t u4Addr, u4Val;

	prHifInfo = &prGlueInfo->rHifInfo;
	prRxRing = &prHifInfo->RxBlkRing;

	if (!prGlueInfo->prAdapter->fgIsFwDownloaded)
		return;

	/* mawd speed up */
	u4Addr = MAWD_POWER_UP;
	u4Val = BIT(0);
	kalDevRegWrite(prGlueInfo, u4Addr, u4Val);

	/* restore AP_rx_blk_ring_cpu_idx */
	u4Addr = MAWD_AP_RX_BLK_CTRL2;
	u4Val = prRxRing->RxCpuIdx;
	kalDevRegWrite(prGlueInfo, u4Addr, u4Val);

	/* BKRS index form RRO */
	u4Addr = WF_RRO_TOP_IND_CMD_0_CTRL3_ADDR;
	kalDevRegRead(prGlueInfo, u4Addr, &u4Val);
	u4Addr = MAWD_IND_CMD_SIGNATURE1;
	kalDevRegWrite(prGlueInfo, u4Addr, u4Val);

	u4Addr = MAWD_AP_WAKE_UP;
	u4Val = BIT(0);
	kalDevRegWrite(prGlueInfo, u4Addr, u4Val);
}

void halMawdSleep(struct GLUE_INFO *prGlueInfo)
{
	struct GL_HIF_INFO *prHifInfo;
	struct RTMP_RX_RING *prRxRing;
	uint32_t u4Addr, u4Val;

	if (!prGlueInfo->prAdapter->fgIsFwDownloaded)
		return;

	prHifInfo = &prGlueInfo->rHifInfo;
	prRxRing = &prHifInfo->RxBlkRing;

	u4Addr = MAWD_AP_WAKE_UP;
	u4Val = 0;
	kalDevRegWrite(prGlueInfo, u4Addr, u4Val);

	u4Addr = MAWD_MD_WAKE_UP;
	kalDevRegRead(prGlueInfo, u4Addr, &u4Val);
	/* MD is wakeup */
	if (u4Val == 1)
		return;

	u4Addr = MAWD_POWER_UP;
	kalDevRegRead(prGlueInfo, u4Addr, &u4Val);
	while ((u4Val & BITS(8, 12)) != BITS(8, 12)) {
		kalDevRegRead(prGlueInfo, u4Addr, &u4Val);
		kalUdelay(20);
	}

	kalDevRegWrite(prGlueInfo, u4Addr, 3);

	while ((u4Val & BIT(2)) != BIT(2)) {
		kalDevRegRead(prGlueInfo, u4Addr, &u4Val);
		kalUdelay(20);
	}
}

void halRroAllocMem(struct GLUE_INFO *prGlueInfo)
{
	struct GL_HIF_INFO *prHifInfo;
	struct HIF_MEM_OPS *prMemOps;
	struct RTMP_DMABUF *prCache, *prAddrArray, *prIndCmd;
	struct RRO_ADDR_ELEM *prAddrElem;
	struct RRO_IND_CMD *prIndCmdElem;
	uint32_t u4Idx, u4AddrNum;

	prHifInfo = &prGlueInfo->rHifInfo;
	prMemOps = &prHifInfo->rMemOps;
	prCache = &prHifInfo->BaBitmapCache;
	prAddrArray = &prHifInfo->AddrArray;
	prIndCmd = &prHifInfo->IndCmdRing;

	prCache->AllocSize =
		RRO_BA_BITMAP_SIZE * RRO_MAX_WINDOW_NUM;
	if (prMemOps->allocExtBuf)
		prMemOps->allocExtBuf(prHifInfo, prCache);

	if (prCache->AllocVa == NULL) {
		DBGLOG(HAL, ERROR, "BaBitmap allocation failed!!\n");
		return;
	}

	/* session num + particular */
	u4AddrNum = RRO_TOTAL_ADDR_ELEM_NUM * (RRO_MAX_WINDOW_NUM + 1);
	prAddrArray->AllocSize = RRO_ADDR_ELEM_SIZE * u4AddrNum;
	if (prMemOps->allocExtBuf)
		prMemOps->allocExtBuf(prHifInfo, prAddrArray);

	if (prAddrArray->AllocVa == NULL) {
		DBGLOG(HAL, ERROR, "AddrArray allocation failed!!\n");
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
	if (prMemOps->allocExtBuf)
		prMemOps->allocExtBuf(prHifInfo, prIndCmd);

	if (prIndCmd->AllocVa == NULL) {
		DBGLOG(HAL, ERROR, "IndCmd allocation failed!!\n");
		return;
	}

	for (u4Idx = 0; u4Idx < RRO_IND_CMD_RING_SIZE; u4Idx++) {
		prIndCmdElem = (struct RRO_IND_CMD *)
			(prIndCmd->AllocVa +
			 u4Idx * sizeof(struct RRO_IND_CMD));
		prIndCmdElem->magic_cnt = 4;
	}
}

void halRroFreeMem(struct GLUE_INFO *prGlueInfo)
{
	struct GL_HIF_INFO *prHifInfo;
	struct HIF_MEM_OPS *prMemOps;
	struct RTMP_DMABUF *prCache, *prAddrArray, *prIndCmd;

	prHifInfo = &prGlueInfo->rHifInfo;
	prMemOps = &prHifInfo->rMemOps;
	prCache = &prHifInfo->BaBitmapCache;
	prAddrArray = &prHifInfo->AddrArray;
	prIndCmd = &prHifInfo->IndCmdRing;

	if (!prMemOps->freeExtBuf)
		return;

	prMemOps->freeExtBuf(prHifInfo, prCache);
	prMemOps->freeExtBuf(prHifInfo, prAddrArray);
	prMemOps->freeExtBuf(prHifInfo, prIndCmd);
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
	struct mt66xx_chip_info *prChipInfo;
	struct GL_HIF_INFO *prHifInfo;
	struct WIFI_VAR *prWifiVar;
	struct RTMP_DMABUF *prIndCmd;
	uint32_t u4Addr, u4Val;

	prChipInfo = prGlueInfo->prAdapter->chip_info;
	prHifInfo = &prGlueInfo->rHifInfo;
	prWifiVar = &prGlueInfo->prAdapter->rWifiVar;
	prIndCmd = &prHifInfo->IndCmdRing;

	prHifInfo->u4RroMagicCnt = 0;
	prHifInfo->u4IndCmdDmaIdx = 0;

	u4Addr = WF_RRO_TOP_IND_CMD_0_CTRL0_ADDR;
	u4Val = prIndCmd->AllocPa;
	kalDevRegWrite(prGlueInfo, u4Addr, u4Val);

	u4Addr = WF_RRO_TOP_IND_CMD_0_CTRL1_ADDR;
	u4Val = RRO_IND_CMD_RING_SIZE;
	kalDevRegWrite(prGlueInfo, u4Addr, u4Val);

	u4Addr = WF_RRO_TOP_IND_CMD_0_CTRL2_ADDR;
	u4Val = RRO_IND_CMD_RING_SIZE - 1;
	kalDevRegWrite(prGlueInfo, u4Addr, u4Val);

	if (prChipInfo->is_support_mawd &&
	    IS_FEATURE_ENABLED(prWifiVar->fgEnableMawd)) {
		u4Addr = WF_RRO_TOP_IND_CMD_SIGNATURE_BASE_0_ADDR;
		u4Val = MAWD_IND_CMD_SIGNATURE0 | MAWD_WFDMA_LOW_ADDR;
		kalDevRegWrite(prGlueInfo, u4Addr, u4Val);

		u4Addr = WF_RRO_TOP_IND_CMD_SIGNATURE_BASE_1_ADDR;
		u4Val = WF_RRO_TOP_IND_CMD_SIGNATURE_BASE_1_EN_MASK |
			MAWD_WFDMA_HIGH_ADDR;
		kalDevRegWrite(prGlueInfo, u4Addr, u4Val);
	}

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

	/* speed up the PLL after power up */
	u4Addr = MAWD_POWER_UP;
	u4Val = BIT(0);
	kalDevRegWrite(prGlueInfo, u4Addr, u4Val);

	/* setup addr array */
	u4Addr = MAWD_ADDR_ARRAY_BASE_L;
	u4Val = prAddrArray->AllocPa;
	kalDevRegWrite(prGlueInfo, u4Addr, u4Val);
	halMawdBackupCr(prGlueInfo, u4Addr, u4Val);

	u4Addr = MAWD_ADDR_ARRAY_BASE_M;
	u4Val = prAddrArray->AllocPa >> 32;
	kalDevRegWrite(prGlueInfo, u4Addr, u4Val);
	halMawdBackupCr(prGlueInfo, u4Addr, u4Val);

	/* setup ind cmd array */
	u4Addr = MAWD_IND_CMD_CTRL0;
	u4Val = prIndCmd->AllocPa;
	kalDevRegWrite(prGlueInfo, u4Addr, u4Val);
	halMawdBackupCr(prGlueInfo, u4Addr, u4Val);

	u4Addr = MAWD_IND_CMD_CTRL1;
	u4Val = RRO_IND_CMD_RING_SIZE << 4;
	kalDevRegWrite(prGlueInfo, u4Addr, u4Val);
	halMawdBackupCr(prGlueInfo, u4Addr, u4Val);

	/* setup ack sn */
	u4Addr = MAWD_RRO_ACK_SN_BASE_L;
	u4Val = WF_RRO_TOP_ACK_SN_CTRL_ADDR - MAWD_RRO_ADDR_OFFSET;
	kalDevRegWrite(prGlueInfo, u4Addr, u4Val);
	halMawdBackupCr(prGlueInfo, u4Addr, u4Val);

	u4Addr = MAWD_RRO_ACK_SN_BASE_M;
	u4Val = MAWD_WFDMA_HIGH_ADDR;
	kalDevRegWrite(prGlueInfo, u4Addr, u4Val);
	halMawdBackupCr(prGlueInfo, u4Addr, u4Val);
}

void halMawdAllocRxBlkRing(struct GLUE_INFO *prGlueInfo, u_int8_t fgAllocMem)
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
	if (fgAllocMem && prMemOps->allocExtBuf)
		prMemOps->allocExtBuf(prHifInfo, prRxDesc);

	if (prRxDesc->AllocVa == NULL) {
		DBGLOG(HAL, ERROR, "RxBlkRing allocation failed!!\n");
		return;
	}

	DBGLOG(HAL, TRACE, "RxBlkRing[%p]: total %lu bytes allocated\n",
		prRxDesc->AllocVa, prRxDesc->AllocSize);

	u4BufSize = CFG_RX_MAX_PKT_SIZE;

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

void halMawdFreeRxBlkRing(struct GLUE_INFO *prGlueInfo)
{
	struct GL_HIF_INFO *prHifInfo;
	struct HIF_MEM_OPS *prMemOps;
	struct RTMP_DMABUF *prRxDesc;

	prHifInfo = &prGlueInfo->rHifInfo;
	prMemOps = &prHifInfo->rMemOps;
	prRxDesc = &prHifInfo->RxBlkDescRing;

	if (prMemOps->freeExtBuf)
		prMemOps->freeExtBuf(prHifInfo, prRxDesc);
}

void halMawdInitRxBlkRing(struct GLUE_INFO *prGlueInfo)
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
	halMawdBackupCr(prGlueInfo, prRxRing->hw_desc_base, u4PhyAddr);
	halMawdBackupCr(prGlueInfo, prRxRing->hw_cnt_addr,
			prRxRing->u4RingSize << 16);

	DBGLOG(HAL, TRACE,
	       "-->RX_BLK_RING[0x%x]: Base=0x%x, Cnt=%d\n",
	       prRxRing->hw_desc_base,
	       u4PhyAddr, prRxRing->u4RingSize);
}

static u_int8_t halRroHashAdd(struct GL_HIF_INFO *prHifInfo, uint64_t u8Key,
			      struct sk_buff *prSkb, struct RX_CTRL_BLK *prRcb)
{
	struct RCB_NODE *prNewNode;

	if (halRroHashSearch(prHifInfo, u8Key))
		return FALSE;

	prNewNode = kalMemAlloc(sizeof(struct RCB_NODE), VIR_MEM_TYPE);
	prNewNode->u8Key = u8Key;
	prNewNode->prSkb = prSkb;
	prNewNode->prRcb = prRcb;

	hash_add_rcu(prHifInfo->arRcbHTbl, &prNewNode->rNode, u8Key);

	return TRUE;
}

static u_int8_t halRroHashDel(struct GL_HIF_INFO *prHifInfo, uint64_t u8Key)
{
	struct RCB_NODE *prRcbNode;

	prRcbNode = halRroHashSearch(prHifInfo, u8Key);
	if (prRcbNode) {
		hash_del_rcu(&prRcbNode->rNode);
		kalMemFree(prRcbNode, VIR_MEM_TYPE, BUF_SIZE);
	}

	return prRcbNode != NULL;
}

static struct RCB_NODE *halRroHashSearch(
	struct GL_HIF_INFO *prHifInfo, uint64_t u8Key)
{
	struct RCB_NODE *prRcbNode;

	hash_for_each_possible_rcu(prHifInfo->arRcbHTbl,
				   prRcbNode, rNode, u8Key) {
		if (prRcbNode->u8Key == u8Key)
			return prRcbNode;
	}

	return NULL;
}

static void halRroAddNewRcbBlk(struct GL_HIF_INFO *prHifInfo,
			 struct sk_buff *prSkb, phys_addr_t rAddr)
{
	struct RX_CTRL_BLK *prRcb;

	prRcb = (struct RX_CTRL_BLK *)prSkb->cb;
	prRcb->rPhyAddr = rAddr;
	prRcb->prSkb = prSkb;

	list_add_tail(&prRcb->rNode, &prHifInfo->rRcbFreeList);
	prHifInfo->u4RcbFreeListCnt++;
}

struct RX_CTRL_BLK *halRroGetFreeRcbBlk(
	struct GL_HIF_INFO *prHifInfo,
	struct RTMP_DMABUF *pDmaBuf,
	uint32_t u4Idx)
{
	struct list_head *prNode;
	struct RX_CTRL_BLK *prRcb;

	if (list_empty(&prHifInfo->rRcbFreeList)) {
		DBGLOG(HAL, ERROR, "rcb list is empty\n");
		return NULL;
	}

	prNode = prHifInfo->rRcbFreeList.next;
	list_del(prNode);
	prHifInfo->u4RcbFreeListCnt--;

	list_add_tail(prNode, &prHifInfo->rRcbUsedList[u4Idx]);
	prHifInfo->u4RcbUsedListCnt[u4Idx]++;

	prRcb = list_entry(prNode, struct RX_CTRL_BLK, rNode);
	prRcb->u4Idx = u4Idx;
	pDmaBuf->AllocPa = prRcb->rPhyAddr;
	pDmaBuf->AllocVa = (void *)prRcb->prSkb->data;
	memset(pDmaBuf->AllocVa, 0, pDmaBuf->AllocSize);

	halRroHashAdd(prHifInfo, (uint64_t)prRcb->rPhyAddr,
		      prRcb->prSkb, prRcb);

	return prRcb;
}

void halRroAllocRcbList(struct GLUE_INFO *prGlueInfo)
{
	struct GL_HIF_INFO *prHifInfo;
	struct HIF_MEM_OPS *prMemOps;
	struct sk_buff *prSkb = NULL;
	struct RTMP_DMABUF rDmaBuf;
	uint32_t u4Cnt, u4Idx;

	prHifInfo = &prGlueInfo->rHifInfo;
	prMemOps = &prHifInfo->rMemOps;

	for (u4Idx = 0; u4Idx < NUM_OF_RX_RING; u4Idx++) {
		INIT_LIST_HEAD(&prHifInfo->rRcbUsedList[u4Idx]);
		prHifInfo->u4RcbUsedListCnt[u4Idx] = 0;
	}
	INIT_LIST_HEAD(&prHifInfo->rRcbFreeList);
	prHifInfo->u4RcbFreeListCnt = 0;
	hash_init(prHifInfo->arRcbHTbl);

	rDmaBuf.AllocSize = CFG_RX_MAX_PKT_SIZE;
	for (u4Cnt = 0; u4Cnt < RRO_PREALLOC_RX_BUF_NUM; u4Cnt++) {
		if (prMemOps->allocRxBuf)
			prSkb = prMemOps->allocRxBuf(
				prHifInfo, &rDmaBuf, 0, u4Cnt);
		if (!prSkb) {
			DBGLOG(HAL, ERROR,
			       "can't allocate rx %lu size packet\n",
			       CFG_RX_MAX_PKT_SIZE);
			break;
		}
		halRroAddNewRcbBlk(prHifInfo, prSkb, rDmaBuf.AllocPa);
	}

	DBGLOG(HAL, TRACE, "Alloc Rcb[%d]", u4Cnt);
}

void halRroFreeRcbList(struct GLUE_INFO *prGlueInfo)
{
	struct GL_HIF_INFO *prHifInfo;
	struct HIF_MEM_OPS *prMemOps;
	struct RX_CTRL_BLK *prRcb;
	struct RCB_NODE *prRcbNode;
	struct hlist_node *prTmpNode;
	struct list_head *prCur, *prNext;
	int i4Bkt = 0;

	prHifInfo = &prGlueInfo->rHifInfo;

	prMemOps = &prHifInfo->rMemOps;

	hash_for_each_safe(prHifInfo->arRcbHTbl, i4Bkt,
			   prTmpNode, prRcbNode, rNode) {
		hash_del_rcu(&prRcbNode->rNode);
		kalMemFree(prRcbNode, VIR_MEM_TYPE, BUF_SIZE);
	}

	/* only release free list, used list release on wfdma free stage */
	list_for_each_safe(prCur, prNext, &prHifInfo->rRcbFreeList) {
		prRcb = list_entry(prCur, struct RX_CTRL_BLK, rNode);
		list_del(prCur);
		if (!prRcb)
			continue;
		if (prMemOps->unmapRxBuf)
			prMemOps->unmapRxBuf(
				prHifInfo, prRcb->rPhyAddr,
				CFG_RX_MAX_PKT_SIZE);
		if (prMemOps->freePacket && prRcb->prSkb)
			prMemOps->freePacket(
				prHifInfo, (void *)prRcb->prSkb, 0);
	}
	prHifInfo->u4RcbFreeListCnt = 0;

	DBGLOG(HAL, TRACE, "Release Rcb");
}

void halRroTurnOff(struct GLUE_INFO *prGlueInfo)
{
	uint32_t u4Addr = 0, u4Val = 0;

	u4Addr = WF_RRO_TOP_GLOBAL_CONFG_ADDR;
	u4Val != BITS(0, 1);
	kalDevRegWrite(prGlueInfo, u4Addr, u4Val);
}

void halRroInit(struct GLUE_INFO *prGlueInfo)
{
	struct mt66xx_chip_info *prChipInfo = prGlueInfo->prAdapter->chip_info;
	struct WIFI_VAR *prWifiVar = &prGlueInfo->prAdapter->rWifiVar;

	halRroSetupBaBitmap(prGlueInfo);
	halRroSetupAddressElement(prGlueInfo);
	halRroSetupIndicateCmdRing(prGlueInfo);
	halRroSetupTimeoutConfig(prGlueInfo);

	if (prChipInfo->is_support_mawd &&
	    IS_FEATURE_ENABLED(prWifiVar->fgEnableMawd))
		halRroMawdInit(prGlueInfo);
}

void halRroUninit(struct GLUE_INFO *prGlueInfo)
{
	struct mt66xx_chip_info *prChipInfo = prGlueInfo->prAdapter->chip_info;
	struct WIFI_VAR *prWifiVar = &prGlueInfo->prAdapter->rWifiVar;

	halRroFreeMem(prGlueInfo);
	halRroFreeRcbList(prGlueInfo);

	if (prChipInfo->is_support_mawd &&
	    IS_FEATURE_ENABLED(prWifiVar->fgEnableMawd))
		halMawdFreeRxBlkRing(prGlueInfo);
}

uint32_t halMawdGetRxBlkDoneCnt(struct GLUE_INFO *prGlueInfo)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct RTMP_RX_RING *prRxRing = NULL;
	uint32_t u4MaxCnt = 0, u4CpuIdx = 0, u4DmaIdx = 0, u4RxPktCnt;

	prHifInfo = &prGlueInfo->rHifInfo;
	prRxRing = &prHifInfo->RxBlkRing;
	HAL_GET_RING_DIDX(prGlueInfo, prRxRing, &prRxRing->RxDmaIdx);
	u4MaxCnt = prRxRing->u4RingSize;
	u4CpuIdx = prRxRing->RxCpuIdx;
	u4DmaIdx = prRxRing->RxDmaIdx;

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

static u_int8_t halRroHandleRxRcb(
	struct ADAPTER *prAdapter, struct RX_CTRL_BLK *prRcb,
	uint32_t u4Reason, uint32_t *au4RingCnt,
	struct QUE *prFreeSwRfbList, struct QUE *prRecvRfbList)
{
	struct GLUE_INFO *prGlueInfo;
	struct GL_HIF_INFO *prHifInfo;
	struct HIF_MEM_OPS *prMemOps;
	struct RTMP_DMACB rRxCell;
	struct RTMP_DMABUF rDmaBuf;
	struct RX_CTRL *prRxCtrl;
	struct SW_RFB *prSwRfb;
	struct sk_buff *prSkb;

	KAL_SPIN_LOCK_DECLARATION();

	prGlueInfo = prAdapter->prGlueInfo;
	prHifInfo = &prGlueInfo->rHifInfo;
	prMemOps = &prHifInfo->rMemOps;
	prRxCtrl = &prAdapter->rRxCtrl;

	KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_RX_FREE_QUE);
	QUEUE_REMOVE_HEAD(&prRxCtrl->rFreeSwRfbList,
			  prSwRfb, struct SW_RFB *);
	KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_RX_FREE_QUE);
	if (!prSwRfb) {
		DBGLOG_LIMITED(RX, WARN, "No More RFB\n");
		return FALSE;
	}

	list_del(&prRcb->rNode);
	prHifInfo->u4RcbUsedListCnt[prRcb->u4Idx]--;
	au4RingCnt[prRcb->u4Idx]++;

	halRroHashDel(prHifInfo, prRcb->rPhyAddr);

	rDmaBuf.AllocSize = CFG_RX_MAX_PKT_SIZE;
	rDmaBuf.AllocPa = prRcb->rPhyAddr;
	rRxCell.pPacket = (void *)prRcb->prSkb;

	if (prMemOps->copyRxData &&
	    !prMemOps->copyRxData(prHifInfo, &rRxCell, &rDmaBuf, prSwRfb)) {
		DBGLOG_LIMITED(RX, WARN, "Read Rxblk fail\n");
		QUEUE_INSERT_TAIL(prFreeSwRfbList, &prSwRfb->rQueEntry);
		return FALSE;
	}

	prSkb = (struct sk_buff *)rRxCell.pPacket;
	halRroAddNewRcbBlk(prHifInfo, prSkb, rDmaBuf.AllocPa);

	prSwRfb->pucRecvBuff = ((struct sk_buff *)prSwRfb->pvPacket)->data;
	prSwRfb->prRxStatus = (void *)prSwRfb->pucRecvBuff;
	prSwRfb->u4IndReason = u4Reason;

	halUpdateRFBInfo(prAdapter, prSwRfb);

#if RRO_DROP_BY_HIF
	if (u4Reason == RRO_REPEAT || u4Reason == RRO_OLDPKT) {
		QUEUE_INSERT_TAIL(prFreeSwRfbList, &prSwRfb->rQueEntry);
		DBGLOG(HAL, INFO, "Drop skb by hif[%d]\n", u4Reason);
	} else {
		QUEUE_INSERT_TAIL(prRecvRfbList, &prSwRfb->rQueEntry);
	}
#else
	QUEUE_INSERT_TAIL(prRecvRfbList, &prSwRfb->rQueEntry);
#endif
	RX_INC_CNT(prRxCtrl, RX_MPDU_TOTAL_COUNT);
	DBGLOG(RX, TEMP, "Recv p=%p total:%lu\n",
	       prSwRfb, RX_GET_CNT(prRxCtrl, RX_MPDU_TOTAL_COUNT));
	kalTraceEvent("Recv p=%p total:%lu",
		      prSwRfb, RX_GET_CNT(prRxCtrl, RX_MPDU_TOTAL_COUNT));

	return TRUE;
}

static u_int8_t halRroHandleRxRcbMsdu(
	struct ADAPTER *prAdapter,
	struct RX_CTRL_BLK *prRcb,
	uint32_t u4Reason,
	uint32_t u4MsduCnt,
	uint32_t *au4RingCnt,
	struct QUE *prFreeSwRfbList,
	struct QUE *prRecvRfbList)
{
	struct RX_CTRL *prRxCtrl;
	struct RX_CTRL_BLK *prNextRcb;
	uint32_t u4Idx;

	prRxCtrl = &prAdapter->rRxCtrl;

	if (prRxCtrl->rFreeSwRfbList.u4NumElem < u4MsduCnt) {
		DBGLOG_LIMITED(RX, WARN, "RFB[%u], [%u]\n",
			       prRxCtrl->rFreeSwRfbList.u4NumElem, u4MsduCnt);
		return FALSE;
	}

	for (u4Idx = 0; u4Idx < u4MsduCnt; u4Idx++) {
		prNextRcb = list_entry(
			prRcb->rNode.next,
			struct RX_CTRL_BLK, rNode);
		if (!halRroHandleRxRcb(prAdapter, prRcb,
				       u4Reason, au4RingCnt,
				       prFreeSwRfbList, prRecvRfbList))
			return FALSE;
		prRcb = prNextRcb;
	}

	return TRUE;
}

static u_int8_t halRroHandleReadRxBlk(
	struct GLUE_INFO *prGlueInfo,
	struct RX_BLK_DESC *prRxBlkD,
	uint32_t *au4RingCnt,
	struct QUE *prFreeSwRfbList,
	struct QUE *prRecvRfbList)
{
	struct GL_HIF_INFO *prHifInfo;
	struct HIF_MEM_OPS *prMemOps;
	struct RX_CTRL_BLK *prRcb;
	struct RCB_NODE *prRcbHNode;
	uint64_t u8Addr;

	prHifInfo = &prGlueInfo->rHifInfo;
	prMemOps = &prHifInfo->rMemOps;

	u8Addr = prRxBlkD->addr_h;
	u8Addr = (u8Addr << 32) | prRxBlkD->addr;
	prRcbHNode = halRroHashSearch(prHifInfo, u8Addr);
	if (!prRcbHNode) {
		DBGLOG(HAL, ERROR, "Cannot find RCB\n");
		dumpMemory32((uint32_t *)prRxBlkD, sizeof(struct RX_BLK_DESC));
		return FALSE;
	}
	if (!prRcbHNode->prSkb) {
		DBGLOG(HAL, ERROR, "skb is null\n");
		dumpMemory32((uint32_t *)prRxBlkD, sizeof(struct RX_BLK_DESC));
		return FALSE;
	}
	prRcb = prRcbHNode->prRcb;
	if (!prRcb) {
		DBGLOG(HAL, ERROR, "rcb is null\n");
		dumpMemory32((uint32_t *)prRxBlkD, sizeof(struct RX_BLK_DESC));
		return FALSE;
	}

	if (!halRroHandleRxRcbMsdu(
		    prGlueInfo->prAdapter, prRcb,
		    prRxBlkD->ind_reason, prRxBlkD->msdu_cnt,
		    au4RingCnt, prFreeSwRfbList, prRecvRfbList))
		return FALSE;

	return TRUE;
}

static void halMawdReadRxBlkRing(
	struct ADAPTER *prAdapter,
	uint32_t *au4RingCnt,
	struct QUE *prFreeSwRfbList,
	struct QUE *prRecvRfbList)
{
	struct GLUE_INFO *prGlueInfo;
	struct GL_HIF_INFO *prHifInfo;
	struct RTMP_RX_RING *prRxRing;
	struct RTMP_DMACB *prRxCell;
	struct RX_BLK_DESC *prRxBlkD;
	uint32_t u4RxCnt;

	prGlueInfo = prAdapter->prGlueInfo;
	prHifInfo = &prGlueInfo->rHifInfo;
	prRxRing = &prHifInfo->RxBlkRing;

	u4RxCnt = halMawdGetRxBlkDoneCnt(prGlueInfo);

	DBGLOG(RX, TRACE, "halMawdReadRxBlks: u4RxCnt:%d\n", u4RxCnt);

	while (u4RxCnt) {
		prRxCell = &prRxRing->Cell[prRxRing->RxCpuIdx];
		prRxBlkD = (struct RX_BLK_DESC *)prRxCell->AllocVa;
		if (!halRroHandleReadRxBlk(prGlueInfo, prRxBlkD, au4RingCnt,
					   prFreeSwRfbList, prRecvRfbList))
			break;
		DBGLOG(HAL, TRACE, "rx blk ring idx[%d]\n", prRxRing->RxCpuIdx);
		INC_RING_INDEX(prRxRing->RxCpuIdx, prRxRing->u4RingSize);
		u4RxCnt--;
	}

	HAL_SET_RING_CIDX(prGlueInfo, prRxRing, prRxRing->RxCpuIdx);

	prRxRing->u4PendingCnt = u4RxCnt;

	if (prRxRing->u4PendingCnt == 0)
		KAL_CLR_BIT(RX_RRO_DATA, prAdapter->ulNoMoreRfb);
	else
		KAL_SET_BIT(RX_RRO_DATA, prAdapter->ulNoMoreRfb);
}

static u_int8_t halRroWaitMagicCnt(struct ADAPTER *prAdapter,
			       struct RRO_IND_CMD *prIndCmd,
			       uint32_t u4MagicCnt)
{
	uint32_t u4Count = 0;

	for (u4Count = 0; u4MagicCnt != prIndCmd->magic_cnt; u4Count++) {
		if (u4Count > DMA_DONE_WAITING_COUNT) {
			DBGLOG(HAL, ERROR,
			       "Magic cnt mismatch IndCmd[%u] exp[%u]\n",
			       prIndCmd->magic_cnt, u4MagicCnt);
			dumpMemory32((uint32_t *)prIndCmd,
				     sizeof(struct RRO_IND_CMD));
			return FALSE;
		}

		kalUdelay(DMA_DONE_WAITING_TIME);
	}
	return TRUE;
}

static void halRroReadIndCmd(
	struct ADAPTER *prAdapter,
	uint32_t *au4RingCnt,
	struct QUE *prFreeSwRfbList,
	struct QUE *prRecvRfbList)
{
	struct GLUE_INFO *prGlueInfo;
	struct GL_HIF_INFO *prHifInfo;
	struct RTMP_DMABUF *prIndCmd, *prAddrArray;
	struct RRO_IND_CMD *aurIndCmd;
	struct RRO_ADDR_ELEM *prAddrElem;
	struct RX_BLK_DESC rRxBlkD;
	union RRO_ACK_SN_CMD rAckSn;
	uint32_t u4Addr, u4Val, u4DmaIdx, u4IndCmdIdx, u4Idx;
	uint32_t u4Id, u4Sn, u4AddrNum;

	prGlueInfo = prAdapter->prGlueInfo;
	prHifInfo = &prGlueInfo->rHifInfo;
	prIndCmd = &prHifInfo->IndCmdRing;
	prAddrArray = &prHifInfo->AddrArray;
	aurIndCmd = (struct RRO_IND_CMD *)prIndCmd->AllocVa;

	u4Addr = WF_RRO_TOP_IND_CMD_0_CTRL3_ADDR;
	kalDevRegRead(prGlueInfo, u4Addr, &u4Val);
	prHifInfo->u4RroMagicCnt = (u4Val & BITS(28, 30)) >> 28;
	u4DmaIdx = prHifInfo->u4IndCmdDmaIdx;
	u4IndCmdIdx = u4Val & BITS(0, 11);

	while (u4DmaIdx != u4IndCmdIdx) {
		u4Id = aurIndCmd[u4DmaIdx].session_id;
		u4Sn = aurIndCmd[u4DmaIdx].start_sn;

		if (!halRroWaitMagicCnt(prAdapter, &aurIndCmd[u4DmaIdx],
					prHifInfo->u4RroMagicCnt))
			break;

		for (u4Idx = 0; u4Idx < aurIndCmd[u4DmaIdx].ind_cnt; u4Idx++) {
			u4AddrNum = u4Id * RRO_MAX_WINDOW_NUM +
				(u4Sn + u4Idx) % RRO_MAX_WINDOW_NUM;
			prAddrElem = (struct RRO_ADDR_ELEM *)
				(prAddrArray->AllocVa +
				 u4AddrNum * sizeof(struct RRO_ADDR_ELEM));
			if (prAddrElem->signature_0 !=
			    ((u4Sn + u4Idx) / RRO_MAX_WINDOW_NUM)) {
				kalMemSet(prAddrElem, 0xff,
					  sizeof(struct RRO_ADDR_ELEM));
				continue;
			}

			rRxBlkD.addr = prAddrElem->addr_0;
			rRxBlkD.addr_h = prAddrElem->addr_h_0;
			rRxBlkD.msdu_cnt = prAddrElem->msdu_cnt_0;
			rRxBlkD.out_of_range = prAddrElem->out_of_range_0;
			rRxBlkD.ind_reason = aurIndCmd[u4DmaIdx].ind_reason;
			rRxBlkD.magic_cnt = aurIndCmd[u4DmaIdx].magic_cnt;

			if (!halRroHandleReadRxBlk(
				    prGlueInfo, &rRxBlkD, au4RingCnt,
				    prFreeSwRfbList, prRecvRfbList))
				break;
		}

		if (u4Idx == 0)
			break;

		rAckSn.field.session_id = u4Id;
		rAckSn.field.ack_sn = u4Sn + u4Idx - 1;
		rAckSn.field.is_last = 1;

		if (u4Idx != aurIndCmd[u4DmaIdx].ind_cnt) {
			rAckSn.field.is_last = 0;
			break;
		}

		INC_RING_INDEX(u4DmaIdx, RRO_IND_CMD_RING_SIZE);
		if (u4DmaIdx == 0)
			INC_RING_INDEX(prHifInfo->u4RroMagicCnt,
				       WFDMA_MAGIC_CNT_NUM);
	}

	u4Addr = WF_RRO_TOP_ACK_SN_CTRL_ADDR;
	kalDevRegWrite(prGlueInfo, WF_RRO_TOP_ACK_SN_CTRL_ADDR, rAckSn.word);
	DBGLOG(HAL, INFO, "update ind cmd[0x%08x]\n", rAckSn.word);

	if (u4DmaIdx == u4IndCmdIdx)
		KAL_CLR_BIT(RX_RRO_DATA, prAdapter->ulNoMoreRfb);
	else
		KAL_SET_BIT(RX_RRO_DATA, prAdapter->ulNoMoreRfb);

	prHifInfo->u4IndCmdDmaIdx = u4DmaIdx;
}

void halRroReadRxData(struct ADAPTER *prAdapter)
{
	struct GLUE_INFO *prGlueInfo;
	struct mt66xx_chip_info *prChipInfo;
	struct GL_HIF_INFO *prHifInfo;
	struct WIFI_VAR *prWifiVar;
	struct RX_CTRL *prRxCtrl;
	struct QUE rFreeSwRfbList, rRecvRfbList;
	struct QUE *prFreeSwRfbList = NULL, *prRecvRfbList = NULL;
	uint32_t au4RingCnt[NUM_OF_RX_RING] = {0};
	uint32_t u4Idx, u4TotalCnt = 0;

	KAL_SPIN_LOCK_DECLARATION();

	prGlueInfo = prAdapter->prGlueInfo;
	prChipInfo = prAdapter->chip_info;
	prHifInfo = &prGlueInfo->rHifInfo;
	prWifiVar = &prAdapter->rWifiVar;

	prRxCtrl = &prAdapter->rRxCtrl;
	prFreeSwRfbList = &rFreeSwRfbList;
	prRecvRfbList = &rRecvRfbList;
	QUEUE_INITIALIZE(prFreeSwRfbList);
	QUEUE_INITIALIZE(prRecvRfbList);

	if (prChipInfo->is_support_mawd &&
	    IS_FEATURE_ENABLED(prWifiVar->fgEnableMawd))
		halMawdReadRxBlkRing(prAdapter, au4RingCnt,
				     prFreeSwRfbList, prRecvRfbList);
	else
		halRroReadIndCmd(prAdapter, au4RingCnt,
				 prFreeSwRfbList, prRecvRfbList);

	u4TotalCnt = prFreeSwRfbList->u4NumElem + prRecvRfbList->u4NumElem;

	KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_RX_FREE_QUE);
	QUEUE_CONCATENATE_QUEUES(&prRxCtrl->rFreeSwRfbList, prFreeSwRfbList);
	KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_RX_FREE_QUE);

	KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_RX_QUE);
	QUEUE_CONCATENATE_QUEUES(&prRxCtrl->rReceivedRfbList, prRecvRfbList);
	KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_RX_QUE);

	for (u4Idx = 0; u4Idx < NUM_OF_RX_RING; u4Idx++) {
		if (!halIsDataRing(prAdapter, RX_RING, u4Idx))
			continue;

		if (prHifInfo->u4RcbFreeListCnt >= u4TotalCnt) {
			halRroUpdateWfdmaRxBlk(
				prGlueInfo, u4Idx, prHifInfo->u4RcbFreeListCnt);
		} else {
			halRroUpdateWfdmaRxBlk(
				prGlueInfo, u4Idx, au4RingCnt[u4Idx]);
		}
	}
}

void halRroUpdateWfdmaRxBlk(struct GLUE_INFO *prGlueInfo,
			    uint16_t u2Port, uint32_t u4ResCnt)
{
	struct GL_HIF_INFO *prHifInfo;
	struct HIF_MEM_OPS *prMemOps;
	struct RTMP_RX_RING *prRxRing;
	struct RTMP_DMABUF *pDmaBuf;
	struct RTMP_DMACB *prRxCell;
	struct RXD_STRUCT *pRxD;
	struct RX_CTRL_BLK *prRcb;
	uint32_t u4CpuIdx, u4Cnt, u4Idx;

	if (u4ResCnt == 0)
		return;

	prHifInfo = &prGlueInfo->rHifInfo;
	prMemOps = &prHifInfo->rMemOps;
	prRxRing = &prHifInfo->RxRing[u2Port];
	u4Cnt = halWpdmaGetRxDmaDoneCnt(prGlueInfo, u2Port);
	if (u4Cnt == 0)
		return;

	DBGLOG(HAL, TRACE, "update rx blk[%d] cnt[%d] res[%d]\n",
	       u2Port, u4Cnt, u4ResCnt);

	if (u4ResCnt < u4Cnt)
		u4Cnt = u4ResCnt;

	u4CpuIdx = prRxRing->RxCpuIdx;
	for (u4Idx = 0; u4Idx < u4Cnt; u4Idx++) {
		INC_RING_INDEX(u4CpuIdx, prRxRing->u4RingSize);
		if (u4CpuIdx == 0)
			INC_RING_INDEX(prRxRing->u4MagicCnt,
				       WFDMA_MAGIC_CNT_NUM);

		prRxCell = &prRxRing->Cell[u4CpuIdx];
		pDmaBuf = &prRxCell->DmaBuf;
		prRcb = halRroGetFreeRcbBlk(prHifInfo, pDmaBuf, u2Port);
		if (!prRcb) {
			DBGLOG(HAL, ERROR, "no rcb resource\n");
			return;
		}

		prRxCell->pPacket = prRcb->prSkb;

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
		pRxD->MagicCnt = prRxRing->u4MagicCnt;
	}

	prRxRing->RxCpuIdx = u4CpuIdx;
	HAL_SET_RING_CIDX(prGlueInfo, prRxRing, prRxRing->RxCpuIdx);
}

static void halMawdReadSram(
	struct GLUE_INFO *prGlueInfo,
	uint32_t u4Offset,
	uint32_t *pu4ValL,
	uint32_t *pu4ValH)
{
	struct BUS_INFO *prBusInfo;
	uint32_t u4Val;

	prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;

	kalDevRegRead(prGlueInfo, prBusInfo->mawd_settings2, &u4Val);
	u4Val = (u4Val & BITS(16, 25)) | (u4Offset & BITS(0, 6));
	kalDevRegWrite(prGlueInfo, prBusInfo->mawd_settings2, u4Val);

	kalDevRegRead(prGlueInfo, prBusInfo->mawd_settings4, &u4Val);
	while (u4Val & BIT(8)) {
		kalUdelay(10);
		kalDevRegRead(prGlueInfo, prBusInfo->mawd_settings4, &u4Val);
	}
	kalDevRegRead(prGlueInfo, prBusInfo->mawd_settings5, pu4ValL);
	kalDevRegRead(prGlueInfo, prBusInfo->mawd_settings6, pu4ValH);
}

static void halMawdUpdateSram(
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

static void halMawdBackupCr(struct GLUE_INFO *prGlueInfo,
			    uint32_t u4Addr, uint32_t u4Val)
{
	uint32_t u4CrNum, u4Offset;
	uint32_t u4ValL, u4ValH;

	u4CrNum = (u4Addr - MAWD_REG_BASE) >> 2;
	u4Offset = u4CrNum >> 1;

	halMawdReadSram(prGlueInfo, u4Offset, &u4ValL, &u4ValH);
	if (u4CrNum % 2)
		u4ValH = u4Val;
	else
		u4ValL = u4Val;
	halMawdUpdateSram(prGlueInfo, u4Offset, u4ValL, u4ValH);
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
	uint32_t u4Base = u4Idx, u4Val = 0, u4Addr, u4DWCnt;
	uint32_t u4HifTxdOffset = u4Base * 0xC;
	uint32_t u4WfdmaOffset = u4Base * 0x14;

	prChipInfo = prGlueInfo->prAdapter->chip_info;
	prBusInfo = prChipInfo->bus_info;

	u4Addr = prBusInfo->mawd_ring_ctrl0 + u4WfdmaOffset;
	kalDevRegWrite(prGlueInfo, u4Addr, u4PhyAddr);
	u4Addr = prBusInfo->mawd_ring_ctrl1 + u4WfdmaOffset;
	u4Val = prWfdmaTxRing->hw_cidx_addr - MAWD_WFDMA_ADDR_OFFSET;
	kalDevRegWrite(prGlueInfo, u4Addr, u4Val);
	u4Addr = prBusInfo->mawd_ring_ctrl2 + u4WfdmaOffset;
	u4Val = prWfdmaTxRing->hw_didx_addr - MAWD_WFDMA_ADDR_OFFSET;
	kalDevRegWrite(prGlueInfo, u4Addr, u4Val);
	u4Addr = prBusInfo->mawd_ring_ctrl3 + u4WfdmaOffset;
	u4Val = (TX_RING_SIZE << 19) | (TXD_SIZE << 12) |
		(MAWD_WFDMA_HIGH_ADDR << 4) | MAWD_WFDMA_HIGH_ADDR;
	kalDevRegWrite(prGlueInfo, u4Addr, u4Val);

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
		halMawdUpdateSram(prGlueInfo,
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

static u_int8_t halMawdAllocHifTxRing(struct GLUE_INFO *prGlueInfo,
				  uint32_t u4Num,
				  uint32_t u4Size,
				  uint32_t u4DescSize,
				  u_int8_t fgAllocMem)
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
		return FALSE;
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

	return TRUE;
}

u_int8_t halMawdAllocTxRing(struct GLUE_INFO *prGlueInfo, u_int8_t fgAllocMem)
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
			return FALSE;
		}
		prHifInfo->MawdTxRing[u4Num].TxSwUsedIdx = 0;
		prHifInfo->MawdTxRing[u4Num].TxCpuIdx = 0;
	}

	prErrRpt->AllocSize = RX_RING_SIZE * 4;
	if (prMemOps->allocExtBuf)
		prMemOps->allocExtBuf(prHifInfo, prErrRpt);

	if (prErrRpt->AllocVa == NULL) {
		DBGLOG(HAL, ERROR, "ErrRpt allocation failed!!\n");
		return FALSE;
	}

	return TRUE;
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

u_int8_t halMawdFillTxRing(struct GLUE_INFO *prGlueInfo,
			   struct MSDU_TOKEN_ENTRY *prToken)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct mt66xx_chip_info *prChipInfo;
	struct RTMP_TX_RING *prTxRing, *prWfdmaTxRing;
	struct RTMP_DMACB *pTxCell;
	uint16_t u2Port = TX_RING_DATA0;

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
	uint32_t u4Addr = 0, u4Val = 0, u4Idx, u4Cnt;
	/* reset mawd idx to default value
	 * 0: md_rx_blk_ring_dma_idx	(default = 0)
	 * 1: ap_rx_blk_ring_dma_idx	(default = 0)
	 * 2: ind_cmd_q_magic		(default = 0)
	 * 3: ind_cmd_q_rdix		(default = 0)
	 * 4: ring0_hiftxd_adr_off	(default = ‘d32)
	 * 5: hiftxd_q0_ridx		(default = 0)
	 * 6: ring1_hiftxd_adr_off	(default = ‘d32)
	 * 7: hiftxd_q1_ridx		(default = 0)
	 * 8: ring2_hiftxd_adr_off	(default = ‘d32)
	 * 9: hiftxd_q2_ridx		(default = 0)
	 * 10: err_rpt_dma_idx		(default = 0)
	 * 11: dmad_q0_widx		(default = 0)
	 * 12: dmad_q1_widx		(default = 0)
	 * 13: dmad_q2_widx		(default = 0)
	 * 14: dmad_q0_ridx		(default = 0)
	 * 15: dmad_q1_ridx		(default = 0)
	 * 16: dmad_q2_ridx		(default = 0)
	 * 17: md_rx_blk_ing_magic_cnt	(default = 0)
	 * 18: ap_rx_blk_ing_magic_cnt	(default = 0)
	 */
	uint32_t u4IdxPatchNum[MAWD_MAX_PATCH_NUM] = {
		0, 0, 0, 0,
		32, 0, 32, 0,
		32, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0
	};

	u4Addr = MAWD_AXI_SLEEP_PROT_SETTING;
	kalDevRegRead(prGlueInfo, u4Addr, &u4Val);
	u4Val |= BIT(0);
	kalDevRegWrite(prGlueInfo, u4Addr, u4Val);

	kalDevRegRead(prGlueInfo, u4Addr, &u4Val);
	while ((u4Val & BITS(5, 6)) != BITS(5, 6)) {
		kalDevRegRead(prGlueInfo, u4Addr, &u4Val);
		kalUdelay(20);
	}

	u4Addr = MAWD_IDX_REG_PATCH;
	for (u4Idx = 0; u4Idx < MAWD_MAX_PATCH_NUM; u4Idx++) {
		u4Val = BIT(31) | (u4Idx << 24) | u4IdxPatchNum[u4Idx];
		kalDevRegWrite(prGlueInfo, u4Addr, u4Val);
	}

	u4Addr = MAWD_SOFTRESET;
	u4Val = BITS(0, 2);
	kalDevRegWrite(prGlueInfo, u4Addr, u4Val);

	u4Addr = MAWD_POWER_UP;
	kalDevRegRead(prGlueInfo, u4Addr, &u4Val);
	for (u4Cnt = 0; (u4Val & BITS(8, 12)) != BITS(8, 12); u4Cnt++) {
		if (u4Cnt > DMA_DONE_WAITING_COUNT) {
			DBGLOG(HAL, ERROR, "Wait MAWD ready timeout\n");
			break;
		}
		kalDevRegRead(prGlueInfo, u4Addr, &u4Val);
		kalUdelay(20);
	}

	u4Addr = MAWD_AXI_SLEEP_PROT_SETTING;
	kalDevRegRead(prGlueInfo, u4Addr, &u4Val);
	u4Val &= ~BIT(0);
	kalDevRegWrite(prGlueInfo, u4Addr, u4Val);
}

#endif /* CFG_SUPPORT_HOST_OFFLOAD == 1 */
