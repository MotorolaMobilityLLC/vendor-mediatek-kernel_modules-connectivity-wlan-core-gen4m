/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2020 MediaTek Inc.
 */
/*
 * Id: @(#) hif_mem.c@@
 */
/*  \file   hif_mem.c
 */

/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */

/*******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 *******************************************************************************
 */

#include "gl_os.h"

#include "hif_pdma.h"

#include "precomp.h"

#include "mt66xx_reg.h"
#include "gl_kal.h"

#if CFG_SUPPORT_RX_PAGE_POOL
#include <net/page_pool.h>
/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */

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
struct sk_buff_head g_rHifSkbList;

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
void kalSkbMarkForRecycle(struct sk_buff *pkt)
{
#if KERNEL_VERSION(5, 15, 0) <= CFG80211_VERSION_CODE
	skb_mark_for_recycle(pkt);
#else
	skb_mark_for_recycle(pkt, page, page->pp);
#endif
}

struct sk_buff *kalAllocRxSkb(uint8_t **ppucData)
{
	struct page *page;
	struct sk_buff *pkt;

	page = wifi_page_pool_alloc_page();
	if (!page) {
		DBGLOG_LIMITED(HAL, ERROR, "allocate page fail\n");
		return NULL;
	}

	pkt = build_skb(page_to_virt(page), PAGE_SIZE); /* ptr to sk_buff */
	if (!pkt) {
		page_pool_recycle_direct(page->pp, page);
		DBGLOG(HAL, ERROR, "allocate skb fail\n");
		return NULL;
	}
	kmemleak_not_leak(pkt); /* Omit memleak check */
	kalSkbMarkForRecycle(pkt);
	*ppucData = (uint8_t *) (pkt->data);

	return pkt;
}

u_int8_t kalCreateHifSkbList(void)
{
	struct sk_buff *prSkb;
	uint8_t *pucRecvBuff;
	uint32_t u4Num = 0, u4Idx;
	u_int8_t fgRet = TRUE;

	skb_queue_head_init(&g_rHifSkbList);

	for (u4Idx = 0; u4Idx < NUM_OF_RX_RING; u4Idx++) {
		if (halIsDataRing(RX_RING, u4Idx))
			u4Num += RX_RING0_SIZE;
		else
			u4Num += RX_RING1_SIZE;
	}

#if (CFG_SUPPORT_HOST_OFFLOAD == 1)
	if (kalIsSupportRro())
		u4Num += RX_RING0_SIZE;
#endif

	for (u4Idx = 0; u4Idx < u4Num; u4Idx++) {
		prSkb = kalAllocRxSkb(&pucRecvBuff);
		if (!prSkb) {
			DBGLOG(HAL, ERROR, "hif skb reserve fail[%u]!\n",
			       u4Idx);
			kalReleaseHifSkbList();
			fgRet = FALSE;
			goto exit;
		}
		skb_queue_tail(&g_rHifSkbList, prSkb);
	}
	DBGLOG(HAL, INFO, "hif skb reserve count[%u]!\n", u4Num);

exit:
	return fgRet;
}

void kalReleaseHifSkbList(void)
{
	struct sk_buff *prSkb;

	while (!skb_queue_empty(&g_rHifSkbList)) {
		prSkb = skb_dequeue(&g_rHifSkbList);
		if (!prSkb) {
			DBGLOG(HAL, ERROR, "hif skb is NULL!\n");
			break;
		}
		kalPacketFree(NULL, (void *)prSkb);
	}
}

struct sk_buff *kalAllocHifSkb(void)
{
	if (skb_queue_empty(&g_rHifSkbList)) {
		DBGLOG(HAL, ERROR, "hif skb list is empty\n");
		return NULL;
	}

	return skb_dequeue(&g_rHifSkbList);
}

void kalFreeHifSkb(struct sk_buff *prSkb)
{
	skb_queue_tail(&g_rHifSkbList, prSkb);
}
#endif /* CFG_SUPPORT_RX_PAGE_POOL */
