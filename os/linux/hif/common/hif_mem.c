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

#if !CFG_CMA_ALLOC
static struct kmem_cache *g_prCache;
#endif

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
	struct page *cma_page, *page;
	uint32_t phys_size = RX_PAGE_POOL_SIZE << PAGE_SHIFT;
	void *kaddr;
	bool ret = true;

	if (!dev->cma_area) {
		DBGLOG(HAL, ERROR, "no cma_area\n");
		return false;
	}
	cma_page = cma_alloc(dev->cma_area, phys_size >> PAGE_SHIFT,
			     get_order(SZ_4K), GFP_KERNEL);
	if (!cma_page) {
		DBGLOG(HAL, ERROR, "cma_page alloc fail\n");
		return false;
	}

	kaddr = page_address(cma_page);
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
static bool kalAllocPagePoolMem(struct page_pool *pool)
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

int kalCreateRxPagePool(struct device *dev)
{
	struct page_pool_params pp = { 0 };

	g_prDev = dev;

	pp.max_len = PAGE_SIZE;
	/* pp.flags = PP_FLAG_DMA_MAP | PP_FLAG_DMA_SYNC_DEV; */
	pp.flags = 0;
	pp.pool_size = RX_PAGE_POOL_SIZE;
	pp.nid = dev_to_node(dev);
	pp.dev = dev;
	pp.dma_dir = DMA_BIDIRECTIONAL;

	g_prPagePool = page_pool_create(&pp);
	if (IS_ERR(g_prPagePool)) {
		int err = PTR_ERR(g_prPagePool);

		DBGLOG(HAL, ERROR, "create page pool fail\n");
		g_prPagePool = NULL;
		return err;
	}
	kalAllocPagePoolMem(dev, g_prPagePool);
	return 0;
}

static void page_pool_dma_sync_for_device(struct page_pool *pool,
					  struct page *page,
					  unsigned int dma_sync_size)
{
	dma_addr_t dma_addr = page_pool_get_dma_addr(page);

	dma_sync_size = min(dma_sync_size, pool->p.max_len);
	dma_sync_single_range_for_device(pool->p.dev, dma_addr,
					 pool->p.offset, dma_sync_size,
					 pool->p.dma_dir);
}

void kalDmaSyncForDevice(void *rAddr)
{
	if (!g_prPagePool)
		return;

	page_pool_dma_sync_for_device(
		g_prPagePool, virt_to_page(rAddr), PAGE_SIZE);
}

static struct page *kalAllocRxPage(struct device *dev, dma_addr_t *prAddr)
{
	struct page *page;

	if (!g_prPagePool)
		return NULL;

	page = page_pool_alloc_pages(g_prPagePool, GFP_KERNEL);
	if (!page) {
		DBGLOG(HAL, ERROR, "page alloc fail\n");
		return NULL;
	}

	init_page_count(page);
	page->pp = g_prPagePool;
	page->pp_magic = PP_SIGNATURE;

	*prAddr = dma_map_page_attrs(dev, page, 0, PAGE_SIZE,
				      DMA_BIDIRECTIONAL,
				      DMA_ATTR_WEAK_ORDERING);
	if (dma_mapping_error(dev, *prAddr)) {
		page_pool_recycle_direct(g_prPagePool, page);
		return NULL;
	}

	return page;
}

struct sk_buff *kalAllocRxSkb(dma_addr_t *prAddr)
{
	struct page *page;
	struct sk_buff *pkt;

	if (!g_prDev) {
		DBGLOG(HAL, ERROR, "device is NULL\n");
		return NULL;
	}

	page = kalAllocRxPage(g_prDev, prAddr);
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

	return pkt;
}
#endif /* CFG_SUPPORT_RX_PAGE_POOL */

