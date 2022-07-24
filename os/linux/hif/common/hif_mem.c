/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2020 MediaTek Inc.
 */
/*
 * Id: @(#) sw_wfdma.c@@
 */
/*  \file   sw_wfdma.c
 *  \brief  The program provides SW WIFIDMA HIF APIs
 *
 *  This file contains the support sw version wifi dma.
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

#include <linux/mm.h>
#include <linux/poison.h>
#include <linux/cma.h>

#ifndef CONFIG_X86
#include <asm/memory.h>
#endif

#include "mt66xx_reg.h"
#include "gl_kal.h"

#if CFG_SUPPORT_RX_PAGE_POOL
#include <net/page_pool.h>
/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */

#define CFG_CMA_ALLOC 1

/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */
struct HIF_SKB_NODE {
	struct sk_buff *prSkb;
	struct list_head rNode;
};

/*******************************************************************************
 *                            P U B L I C   D A T A
 *******************************************************************************
 */

/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */
static struct device *g_prDev;
static struct page_pool *g_prPagePool;
struct list_head g_rHifSkbList;

#if CFG_CMA_ALLOC
struct page *g_prCmaPage;
#else
static struct kmem_cache *g_prCache;
#endif /* CFG_CMA_ALLOC */

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

#define RX_PAGE_POOL_SIZE (32768)

static void kalPutPagePool(struct page_pool *pool, struct page *page)
{
	init_page_count(page);
	page->pp = pool;
	page->pp_magic = PP_SIGNATURE;
	page_pool_recycle_direct(pool, page);
}

#if CFG_CMA_ALLOC
static bool kalAllocPagePoolMem(struct device *dev, struct page_pool *pool)
{
	uint32_t u4Idx;
	struct page *page;
	void *kaddr;
	bool ret = true;

	if (!dev->cma_area) {
		DBGLOG(HAL, ERROR, "no cma_area\n");
		return false;
	}
	g_prCmaPage = cma_alloc(dev->cma_area, RX_PAGE_POOL_SIZE,
			     get_order(SZ_4K), GFP_KERNEL);
	if (!g_prCmaPage) {
		DBGLOG(HAL, ERROR, "cma_page alloc fail\n");
		return false;
	}

	kaddr = page_address(g_prCmaPage);
	for (u4Idx = 0; u4Idx < RX_PAGE_POOL_SIZE; u4Idx++) {
		page = virt_to_page(kaddr);
		if (!page) {
			DBGLOG(HAL, ERROR, "page is NULL\n");
			break;
		}
		kalPutPagePool(pool, page);
		kaddr += SZ_4K;
	}
	DBGLOG(HAL, INFO, "alloc page num[%d]\n", u4Idx);

	return ret;
}
#else
static bool kalAllocPagePoolMem(struct device *dev, struct page_pool *pool)
{
	uint32_t u4Idx;
	void *rVirAddr;
	struct page *page;

	g_prCache = kmem_cache_create_usercopy(
		"wlan_rx_skb_cache",
		PAGE_SIZE,
		0,
		SLAB_CACHE_DMA32,
		0,
		PAGE_SIZE,
		NULL);
	if (!g_prCache) {
		DBGLOG(HAL, ERROR, "skb cache create fail/n");
		return false;
	}

	for (u4Idx = 0; u4Idx < RX_PAGE_POOL_SIZE; u4Idx++) {
		rVirAddr = kmem_cache_alloc(g_prCache, GFP_KERNEL);
		if (!rVirAddr) {
			DBGLOG(HAL, ERROR, "rVirAddr is NULL\n");
			continue;
		}
		page = virt_to_page(rVirAddr);
		if (!page) {
			DBGLOG(HAL, ERROR, "page is NULL\n");
			continue;
		}
		kalPutPagePool(pool, page);
	}

	return true;
}
#endif

static bool kalIsRxPagePoolEmpty(struct page_pool *pool)
{
	return !pool->alloc.count && __ptr_ring_empty(&pool->ring);
}

int kalCreateRxPagePool(struct device *dev)
{
	struct page_pool_params pp = { 0 };

	g_prDev = dev;

	pp.max_len = PAGE_SIZE;
	pp.flags = 0;
	pp.pool_size = RX_PAGE_POOL_SIZE;
	pp.nid = dev_to_node(dev);
	pp.dev = dev;
	pp.dma_dir = DMA_FROM_DEVICE;

	g_prPagePool = page_pool_create(&pp);

	if (KAL_IS_ERR(g_prPagePool)) {
		int err = PTR_ERR(g_prPagePool);

		DBGLOG(HAL, ERROR, "create page pool fail\n");
		g_prPagePool = NULL;
		return err;
	}
	kalAllocPagePoolMem(dev, g_prPagePool);
	return 0;
}

void kalReleaseRxPagePool(struct device *dev)
{
	struct page *page;

	if (!g_prPagePool)
		return;

#if CFG_CMA_ALLOC
	if (!g_prCmaPage || !dev->cma_area)
		return;

	while (!kalIsRxPagePoolEmpty(g_prPagePool))
		page = page_pool_alloc_pages(g_prPagePool, GFP_KERNEL);

	cma_release(dev->cma_area, g_prCmaPage, RX_PAGE_POOL_SIZE);
	g_prCmaPage = NULL;
#else
	if (!g_prCache)
		return;

	while (!kalIsRxPagePoolEmpty(g_prPagePool)) {
		page = page_pool_alloc_pages(g_prPagePool, GFP_KERNEL);
		kmem_cache_free(g_prCache, page_to_virt(page));
	}

	kmem_cache_destroy(g_prCache);
	g_prCache = NULL;
#endif

	page_pool_destroy(g_prPagePool);
	g_prPagePool = NULL;
}

static struct page *kalAllocRxPage(struct device *dev)
{
	struct page *page;

	if (!g_prPagePool)
		return NULL;

	if (kalIsRxPagePoolEmpty(g_prPagePool))
		return NULL;

	page = page_pool_alloc_pages(g_prPagePool, GFP_KERNEL);
	if (!page) {
		DBGLOG(HAL, ERROR, "page alloc fail\n");
		return NULL;
	}

	init_page_count(page);
	page->pp = g_prPagePool;
	page->pp_magic = PP_SIGNATURE;

	return page;
}

struct sk_buff *kalAllocRxSkb(uint8_t **ppucData)
{
	struct page *page;
	struct sk_buff *pkt;

	if (!g_prDev) {
		DBGLOG(HAL, ERROR, "device is NULL\n");
		return NULL;
	}

	page = kalAllocRxPage(g_prDev);
	if (!page) {
		DBGLOG(HAL, ERROR, "allocate page fail\n");
		return NULL;
	}

	pkt = build_skb(page_to_virt(page), PAGE_SIZE);
	if (!pkt) {
		page_pool_recycle_direct(g_prPagePool, page);
		DBGLOG(HAL, ERROR, "allocate skb fail\n");
		return NULL;
	}
#if KERNEL_VERSION(5, 15, 0) <= LINUX_VERSION_CODE
	skb_mark_for_recycle(pkt);
#else
	skb_mark_for_recycle(pkt, page, g_prPagePool);
#endif
	*ppucData = (uint8_t *) (pkt->data);

	return pkt;
}

u_int8_t kalCreateHifSkbList(void)
{
	struct HIF_SKB_NODE *prNode;
	struct sk_buff *prSkb;
	uint8_t *pucRecvBuff;
	uint32_t u4Num = 0, u4Idx;
	u_int8_t fgRet = TRUE;

	INIT_LIST_HEAD(&g_rHifSkbList);

	for (u4Idx = 0; u4Idx < NUM_OF_RX_RING; u4Idx++) {
		if (halIsDataRing(RX_RING, u4Idx))
			u4Num += RX_RING0_SIZE;
		else
			u4Num += RX_RING1_SIZE;
	}

	for (u4Idx = 0; u4Idx < u4Num; u4Idx++) {
		prSkb = kalAllocRxSkb(&pucRecvBuff);
		if (!prSkb) {
			DBGLOG(HAL, ERROR, "hif skb reserve fail[%u]!\n",
			       u4Idx);
			kalReleaseHifSkbList();
			fgRet = FALSE;
			goto exit;
		}

		prNode = (struct HIF_SKB_NODE *)prSkb->cb;
		prNode->prSkb = prSkb;
		list_add_tail(&prNode->rNode, &g_rHifSkbList);
	}
	DBGLOG(HAL, INFO, "hif skb reserve count[%u]!\n", u4Num);

exit:
	return fgRet;
}

void kalReleaseHifSkbList(void)
{
	struct HIF_SKB_NODE *prNode;
	struct list_head *prCur, *prNext;

	list_for_each_safe(prCur, prNext, &g_rHifSkbList) {
		prNode = list_entry(prCur, struct HIF_SKB_NODE, rNode);
		list_del(prCur);
		kalPacketFree(NULL, (void *)prNode->prSkb);
	}
}

struct sk_buff *kalAllocHifSkb(void)
{
	struct HIF_SKB_NODE *prNode;

	if (list_empty(&g_rHifSkbList)) {
		DBGLOG(HAL, ERROR, "hif skb list is empty\n");
		return NULL;
	}

	prNode = list_entry(g_rHifSkbList.next, struct HIF_SKB_NODE, rNode);
	list_del(&prNode->rNode);

	return prNode->prSkb;
}

void kalFreeHifSkb(struct sk_buff *prSkb)
{
	struct HIF_SKB_NODE *prNode;

	prNode = (struct HIF_SKB_NODE *)prSkb->cb;
	prNode->prSkb = prSkb;
	list_add_tail(&prNode->rNode, &g_rHifSkbList);
}
#endif /* CFG_SUPPORT_RX_PAGE_POOL */
