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
/******************************************************************************
 *[File]             pcie.c
 *[Version]          v1.0
 *[Revision Date]    2010-03-01
 *[Author]
 *[Description]
 *    The program provides PCIE HIF driver
 *[Copyright]
 *    Copyright (C) 2010 MediaTek Incorporation. All Rights Reserved.
 ******************************************************************************/


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
#include <linux/platform_device.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#ifndef CONFIG_X86
#include <asm/memory.h>
#endif

#include "mt66xx_reg.h"

#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/of.h>

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */

static const struct platform_device_id mtk_axi_ids[] = {
#ifdef CONNAC
	{	.name = "CONNAC",
		.driver_data = (kernel_ulong_t)&mt66xx_driver_data_connac},
#endif /* CONNAC */
	{ /* end: all zeroes */ },
};

MODULE_DEVICE_TABLE(axi, mtk_axi_ids);

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
static struct platform_device *prPlatDev;
static probe_card pfWlanProbe;
static remove_card pfWlanRemove;

static struct platform_driver mtk_axi_driver = {
	.driver = {
		.name = "wlan",
		.owner = THIS_MODULE,
	},
	.id_table = mtk_axi_ids,
	.probe = NULL,
	.remove = NULL,
};

static struct GLUE_INFO *g_prGlueInfo;
static uint8_t *CSRBaseAddress;
static u_int8_t g_fgDriverProbed = FALSE;

dma_addr_t g_prTxDescPa[NUM_OF_TX_RING];
void *g_prTxDescVa[NUM_OF_TX_RING];
dma_addr_t g_prRxDescPa[NUM_OF_RX_RING];
void *g_prRxDescVa[NUM_OF_RX_RING];
struct sk_buff *g_prRxDataBuf[RX_RING0_SIZE];
struct sk_buff *g_prRxEventBuf[RX_RING1_SIZE];
#if HIF_TX_PREALLOC_DATA_BUFFER
void *g_prMsduBuf[HIF_TX_MSDU_TOKEN_NUM];
#endif

/*******************************************************************************
 *                                 M A C R O S
 *******************************************************************************
 */

/*******************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */
static void *axiAllocDmaCoherent(size_t size, dma_addr_t *dma_handle,
			    bool fgIsTx, uint32_t u4Num);
static struct sk_buff *axiAllocRxPacketBuf(uint32_t u4Len, uint32_t u4Num,
				      uint32_t u4Idx);
static struct sk_buff *axiAllocMsduBuf(uint32_t u4Len, uint32_t u4Idx);
static void axiUpdateRxPacket(struct sk_buff *prSkb,
			      uint32_t u4Num, uint32_t u4Idx);

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */

static int hifAxiProbe(void)
{
	int ret = 0;

	ASSERT(prPlatDev);

	ret = axi_enable_device(prPlatDev);
	if (ret) {
		DBGLOG(INIT, INFO, "axi_enable_device failed!\n");
		goto out;
	}

	DBGLOG(INIT, INFO, "axi_enable_device done!\n");

	DBGLOG(INIT, INFO, "driver.name = %s\n", prPlatDev->id_entry->name);

	if (pfWlanProbe((void *)prPlatDev,
			(void *)prPlatDev->id_entry->driver_data) !=
			WLAN_STATUS_SUCCESS) {
		DBGLOG(INIT, INFO, "pfWlanProbe fail!call pfWlanRemove()\n");
		pfWlanRemove();
		ret = -1;
		goto out;
	}
	g_fgDriverProbed = TRUE;
out:
	DBGLOG(INIT, INFO, "hifAxiProbe() done(%d)\n", ret);

	return ret;
}

static int hifAxiRemove(void)
{
	ASSERT(prPlatDev);

	if (g_fgDriverProbed)
		pfWlanRemove();
	DBGLOG(INIT, INFO, "pfWlanRemove done\n");

	/* Unmap CSR base address */
	iounmap(CSRBaseAddress);

	/* release memory region */
	release_mem_region(axi_resource_start(prPlatDev, 0),
			   axi_resource_len(prPlatDev, 0));

	axi_disable_device(prPlatDev);
	DBGLOG(INIT, INFO, "hifAxiRemove() done\n");
	return 0;
}

#if CFG_MTK_ANDROID_WMT
static int hifAxiGetBusCnt(void)
{
	if (!g_prGlueInfo)
		return 0;

	return g_prGlueInfo->rHifInfo.u4HifCnt;
}

static int hifAxiClrBusCnt(void)
{
	if (g_prGlueInfo)
		g_prGlueInfo->rHifInfo.u4HifCnt = 0;

	return 0;
}
#endif /* CFG_MTK_ANDROID_WMT */

static int hifAxiSetMpuProtect(bool enable)
{
	kalSetEmiMpuProtection(gConEmiPhyBase, WIFI_EMI_MEM_OFFSET,
			       WIFI_EMI_MEM_SIZE, enable);
	return 0;
}

static void axiDmaSetup(struct platform_device *pdev)
{
	struct mt66xx_chip_info *prChipInfo;
	const struct dma_map_ops *dma_ops = NULL;
	u64 required_mask, dma_mask;
	int ret = 0;

	prChipInfo = ((struct mt66xx_hif_driver_data *)
		mtk_axi_ids[0].driver_data)->chip_info;

	dma_mask = DMA_BIT_MASK(prChipInfo->bus_info->u4DmaMask);
	required_mask = dma_get_required_mask(&pdev->dev);
	DBGLOG(INIT, INFO,
	       "pdev=%p, pdev->dev=%p, name=%s, required_mask=%llx, dma_addr_t=%zu\n",
	       pdev, &pdev->dev, pdev->id_entry->name,
	       required_mask, sizeof(dma_addr_t));

	pdev->dev.coherent_dma_mask = dma_mask;
	pdev->dev.dma_mask = &(pdev->dev.coherent_dma_mask);

	KAL_ARCH_SETUP_DMA_OPS(&pdev->dev, 0, dma_mask, NULL, false);
	dma_ops = get_dma_ops(&pdev->dev);
	DBGLOG(INIT, INFO, "dma_supported=%d, dma_mask=%llx\n",
	       dma_supported(&pdev->dev, dma_mask), dma_mask);

	ret = dma_set_mask_and_coherent(&pdev->dev, dma_mask);
	if (ret)
		DBGLOG(INIT, INFO, "set DMA mask failed! errno=%d\n", ret);
}

#if AXI_CFG_PREALLOC_MEMORY_BUFFER
static void axiAllocHifMem(struct platform_device *pdev)
{
	uint32_t u4Idx;

	for (u4Idx = 0; u4Idx < NUM_OF_TX_RING; u4Idx++) {
		g_prTxDescVa[u4Idx] = (void *)KAL_DMA_ALLOC_COHERENT(
			&pdev->dev, TX_RING_SIZE * TXD_SIZE,
			&g_prTxDescPa[u4Idx]);
		if (!g_prTxDescVa[u4Idx])
			DBGLOG(INIT, ERROR, "TxDesc[%u] alloc fail\n", u4Idx);
	}

	g_prRxDescVa[0] = (void *)KAL_DMA_ALLOC_COHERENT(
		&pdev->dev, RX_RING0_SIZE * RXD_SIZE, &g_prRxDescPa[0]);
	if (!g_prRxDescVa[0])
		DBGLOG(INIT, ERROR, "RxDesc data alloc fail\n");

	g_prRxDescVa[1] = (void *)KAL_DMA_ALLOC_COHERENT(
		&pdev->dev, RX_RING1_SIZE * RXD_SIZE, &g_prRxDescPa[1]);
	if (!g_prRxDescVa[1])
		DBGLOG(INIT, ERROR, "RxDesc event alloc fail\n");

	for (u4Idx = 0; u4Idx < RX_RING0_SIZE; u4Idx++) {
		g_prRxDataBuf[u4Idx] = __dev_alloc_skb(CFG_RX_MAX_PKT_SIZE,
						       GFP_KERNEL);
		if (!g_prRxDataBuf[u4Idx])
			DBGLOG(INIT, ERROR, "RxDataBuf[%u] event alloc fail\n",
			       u4Idx);
	}

	for (u4Idx = 0; u4Idx < RX_RING1_SIZE; u4Idx++) {
		g_prRxEventBuf[u4Idx] = __dev_alloc_skb(RX_BUFFER_AGGRESIZE,
							GFP_KERNEL);
		if (!g_prRxEventBuf[u4Idx])
			DBGLOG(INIT, ERROR, "RxEventBuf[%u] event alloc fail\n",
			       u4Idx);
	}

#if HIF_TX_PREALLOC_DATA_BUFFER
	for (u4Idx = 0; u4Idx < HIF_TX_MSDU_TOKEN_NUM; u4Idx++) {
		g_prMsduBuf[u4Idx] = kmalloc(AXI_TX_MAX_SIZE_PER_FRAME,
					     GFP_KERNEL);
		if (!g_prMsduBuf[u4Idx])
			DBGLOG(INIT, ERROR, "MsduBuf[%u] event alloc fail\n",
			       u4Idx);
	}
#endif
}

static void axiFreeHifMem(struct platform_device *pdev)
{
	uint32_t u4Idx;

	for (u4Idx = 0; u4Idx < NUM_OF_TX_RING; u4Idx++) {
		if (g_prTxDescVa[u4Idx])
			KAL_DMA_FREE_COHERENT(
				&pdev->dev, TX_RING_SIZE * TXD_SIZE,
				g_prTxDescVa[u4Idx], g_prTxDescPa[u4Idx]);
	}

	if (g_prRxDescVa[0])
		KAL_DMA_FREE_COHERENT(&pdev->dev, RX_RING0_SIZE * RXD_SIZE,
			      g_prRxDescVa[0], g_prRxDescPa[0]);
	if (g_prRxDescVa[1])
		KAL_DMA_FREE_COHERENT(&pdev->dev, RX_RING1_SIZE * RXD_SIZE,
				      g_prRxDescVa[1], g_prRxDescPa[1]);

	for (u4Idx = 0; u4Idx < RX_RING0_SIZE; u4Idx++) {
		if (g_prRxDataBuf[u4Idx])
			dev_kfree_skb(g_prRxDataBuf[u4Idx]);
	}

	for (u4Idx = 0; u4Idx < RX_RING1_SIZE; u4Idx++) {
		if (g_prRxEventBuf[u4Idx])
			dev_kfree_skb(g_prRxEventBuf[u4Idx]);
	}

#if HIF_TX_PREALLOC_DATA_BUFFER
	for (u4Idx = 0; u4Idx < HIF_TX_MSDU_TOKEN_NUM; u4Idx++)
		kfree(g_prMsduBuf[u4Idx]);
#endif
}
#endif /* AXI_CFG_PREALLOC_MEMORY_BUFFER */

/*----------------------------------------------------------------------------*/
/*!
 * \brief This function is a AXI interrupt callback function
 *
 * \param[in] func  pointer to AXI handle
 *
 * \return void
 */
/*----------------------------------------------------------------------------*/
static irqreturn_t mtk_axi_interrupt(int irq, void *dev_instance)
{
	struct GLUE_INFO *prGlueInfo = NULL;

	prGlueInfo = (struct GLUE_INFO *)dev_instance;
	if (!prGlueInfo) {
		DBGLOG(HAL, INFO, "No glue info in mtk_axi_interrupt()\n");
		return IRQ_NONE;
	}

	halDisableInterrupt(prGlueInfo->prAdapter);

	if (prGlueInfo->ulFlag & GLUE_FLAG_HALT) {
		DBGLOG(HAL, INFO, "GLUE_FLAG_HALT skip INT\n");
		return IRQ_NONE;
	}

	kalSetIntEvent(prGlueInfo);

	return IRQ_HANDLED;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief This function is a AXI probe function
 *
 * \param[in] func   pointer to AXI handle
 * \param[in] id     pointer to AXI device id table
 *
 * \return void
 */
/*----------------------------------------------------------------------------*/
static int mtk_axi_probe(IN struct platform_device *pdev)
{
#if CFG_MTK_ANDROID_WMT
	struct MTK_WCN_WMT_WLAN_CB_INFO rWmtCb;
#endif

	axiDmaSetup(pdev);

#if AXI_CFG_PREALLOC_MEMORY_BUFFER
	axiAllocHifMem(pdev);
#endif

#if CFG_MTK_ANDROID_WMT
	rWmtCb.wlan_probe_cb = hifAxiProbe;
	rWmtCb.wlan_remove_cb = hifAxiRemove;
	rWmtCb.wlan_bus_cnt_get_cb = hifAxiGetBusCnt;
	rWmtCb.wlan_bus_cnt_clr_cb = hifAxiClrBusCnt;
	rWmtCb.wlan_emi_mpu_set_protection_cb = hifAxiSetMpuProtect;
	mtk_wcn_wmt_wlan_reg(&rWmtCb);
#else
	hifAxiProbe();
#endif
	DBGLOG(INIT, INFO, "mtk_axi_probe() done\n");

	return 0;
}

static int mtk_axi_remove(IN struct platform_device *pdev)
{
#if AXI_CFG_PREALLOC_MEMORY_BUFFER
	axiFreeHifMem(pdev);
#endif

#if CFG_MTK_ANDROID_WMT
	mtk_wcn_wmt_wlan_unreg();
#else
	hifAxiRemove();
#endif
	return 0;
}

static int mtk_axi_suspend(IN struct platform_device *pdev,
	IN pm_message_t state)
{
	return 0;
}

int mtk_axi_resume(IN struct platform_device *pdev)
{
	return 0;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief This function will register pci bus to the os
 *
 * \param[in] pfProbe    Function pointer to detect card
 * \param[in] pfRemove   Function pointer to remove card
 *
 * \return The result of registering pci bus
 */
/*----------------------------------------------------------------------------*/
uint32_t glRegisterBus(probe_card pfProbe, remove_card pfRemove)
{
	int ret = 0;

	ASSERT(pfProbe);
	ASSERT(pfRemove);

	pfWlanProbe = pfProbe;
	pfWlanRemove = pfRemove;

	mtk_axi_driver.probe = mtk_axi_probe;
	mtk_axi_driver.remove = mtk_axi_remove;

	mtk_axi_driver.suspend = mtk_axi_suspend;
	mtk_axi_driver.resume = mtk_axi_resume;

	ret = (platform_driver_register(&mtk_axi_driver) == 0)
		? WLAN_STATUS_SUCCESS : WLAN_STATUS_FAILURE;
	DBGLOG(INIT, INFO, "platform_driver_register ret = %d\n", ret);
	DBGLOG(INIT, INFO, "bus_type = %s\n", mtk_axi_driver.driver.bus->name);
	if (!ret)
		ret = ((prPlatDev =
			platform_device_alloc("CONNAC", -1)) != NULL)
			? WLAN_STATUS_SUCCESS : WLAN_STATUS_FAILURE;

	DBGLOG(INIT, INFO, "platform_device_alloc ret = %d\n", ret);
	ret = (platform_device_add(prPlatDev) == 0)
		? WLAN_STATUS_SUCCESS : WLAN_STATUS_FAILURE;
	DBGLOG(INIT, INFO, "platform_device_add ret = %d\n", ret);
	return ret;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief This function will unregister pci bus to the os
 *
 * \param[in] pfRemove Function pointer to remove card
 *
 * \return (none)
 */
/*----------------------------------------------------------------------------*/
void glUnregisterBus(remove_card pfRemove)
{
	if (g_fgDriverProbed) {
		pfRemove();
		g_fgDriverProbed = FALSE;
	}
	if (prPlatDev)
		platform_device_del(prPlatDev);
	platform_driver_unregister(&mtk_axi_driver);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief This function stores hif related info, which is initialized before.
 *
 * \param[in] prGlueInfo Pointer to glue info structure
 * \param[in] u4Cookie   Pointer to UINT_32 memory base variable for _HIF_HPI
 *
 * \return (none)
 */
/*----------------------------------------------------------------------------*/
void glSetHifInfo(struct GLUE_INFO *prGlueInfo, unsigned long ulCookie)
{
	struct GL_HIF_INFO *prHif = NULL;

	g_prGlueInfo = prGlueInfo;
	prHif = &prGlueInfo->rHifInfo;

	prHif->pdev = (struct platform_device *)ulCookie;
	prHif->prDmaDev = &prHif->pdev->dev;

	prHif->CSRBaseAddress = CSRBaseAddress;

	platform_set_drvdata(prHif->pdev, prGlueInfo);

	SET_NETDEV_DEV(prGlueInfo->prDevHandler, &prHif->pdev->dev);

	spin_lock_init(&prHif->rDynMapRegLock);

	prGlueInfo->u4InfType = MT_DEV_INF_AXI;

	prHif->rErrRecoveryCtl.eErrRecovState = ERR_RECOV_STOP_IDLE;
	prHif->rErrRecoveryCtl.u4Status = 0;
	prHif->fgIsErrRecovery = FALSE;
	spin_lock_init(&prHif->rSerLock);

	INIT_LIST_HEAD(&prHif->rTxCmdQ);
	INIT_LIST_HEAD(&prHif->rTxDataQ);
	spin_lock_init(&prHif->rTxCmdQLock);
	spin_lock_init(&prHif->rTxDataQLock);

	prHif->fgIsPreAllocMem = AXI_CFG_PREALLOC_MEMORY_BUFFER;
	prHif->allocDmaCoherent = axiAllocDmaCoherent;
	prHif->allocRxPacket = axiAllocRxPacketBuf;
	prHif->allocMsduBuf = axiAllocMsduBuf;
	prHif->updateRxPacket = axiUpdateRxPacket;
	prHif->fgIsPowerOff = true;
	prHif->fgIsDumpLog = false;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief This function clears hif related info.
 *
 * \param[in] prGlueInfo Pointer to glue info structure
 *
 * \return (none)
 */
/*----------------------------------------------------------------------------*/
void glClearHifInfo(struct GLUE_INFO *prGlueInfo)
{
	struct GL_HIF_INFO *prHifInfo = &prGlueInfo->rHifInfo;
	struct list_head *prCur, *prNext;
	struct TX_CMD_REQ_T *prTxCmdReq;
	struct TX_DATA_REQ_T *prTxDataReq;

	halUninitMsduTokenInfo(prGlueInfo->prAdapter);
	halWpdmaFreeRing(prGlueInfo);

	list_for_each_safe(prCur, prNext, &prHifInfo->rTxCmdQ) {
		prTxCmdReq = list_entry(prCur, struct TX_CMD_REQ_T, list);
		list_del(prCur);
		kfree(prTxCmdReq);
	}

	list_for_each_safe(prCur, prNext, &prHifInfo->rTxDataQ) {
		prTxDataReq = list_entry(prCur, struct TX_DATA_REQ_T, list);
		list_del(prCur);
		kfree(prTxDataReq);
	}
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Initialize bus operation and hif related information, request
 *        resources.
 *
 * \param[out] pvData    A pointer to HIF-specific data type buffer.
 *                       For eHPI, pvData is a pointer to UINT_32 type and
 *                       stores a mapped base address.
 *
 * \return (none)
 */
/*----------------------------------------------------------------------------*/
u_int8_t glBusInit(void *pvData)
{
	struct platform_device *pdev = NULL;
#ifdef CONFIG_OF
	struct device_node *node = NULL;
	u32 u4OffsetLow = 0, u4OffsetHigh = 0;
#endif
	u64 u8Offset = 0;
	u32 u4Size = 0;

	ASSERT(pvData);

	pdev = (struct platform_device *)pvData;

#ifdef CONFIG_OF
	node = of_find_compatible_node(NULL, NULL, "mediatek,wifi");
	if (node) {
		if (of_property_read_u32_index(node, "reg", 0, &u4OffsetHigh) ||
		    of_property_read_u32_index(node, "reg", 1, &u4OffsetLow) ||
		    of_property_read_u32_index(node, "reg", 3, &u4Size))
			DBGLOG(INIT, ERROR, "Failed to get base addr\n");
		u8Offset = (((u64)u4OffsetHigh << 16) << 16) | (u64)u4OffsetLow;
	} else
		DBGLOG(INIT, ERROR, "WIFI-OF: get wifi device node fail\n");
#else
	u8Offset = axi_resource_start(pdev, 0);
	u4Size = axi_resource_len(pdev, 0);
#endif

	request_mem_region(u8Offset, u4Size, axi_name(pdev));
	/* map physical address to virtual address for accessing register */
	CSRBaseAddress = (uint8_t *)ioremap_nocache(u8Offset, u4Size);
	DBGLOG(INIT, INFO,
	       "CSRBaseAddress:0x%lX ioremap region 0x%X @ 0x%lX\n",
	       CSRBaseAddress, u4Size, u8Offset);
	if (!CSRBaseAddress) {
		DBGLOG(INIT, INFO,
			"ioremap failed for device %s, region 0x%X @ 0x%lX\n",
			axi_name(pdev), u4Size, u8Offset);
		goto err_out_free_res;
	}

	return TRUE;

err_out_free_res:
	release_mem_region(axi_resource_start(pdev, 0),
		axi_resource_len(pdev, 0));

	axi_disable_device(pdev);

	return FALSE;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Stop bus operation and release resources.
 *
 * \param[in] pvData A pointer to struct net_device.
 *
 * \return (none)
 */
/*----------------------------------------------------------------------------*/
void glBusRelease(void *pvData)
{
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Setup bus interrupt operation and interrupt handler for os.
 *
 * \param[in] pvData     A pointer to struct net_device.
 * \param[in] pfnIsr     A pointer to interrupt handler function.
 * \param[in] pvCookie   Private data for pfnIsr function.
 *
 * \retval WLAN_STATUS_SUCCESS   if success
 *         NEGATIVE_VALUE   if fail
 */
/*----------------------------------------------------------------------------*/

int32_t glBusSetIrq(void *pvData, void *pfnIsr, void *pvCookie)
{
	struct net_device *prNetDevice = NULL;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct platform_device *pdev = NULL;
#ifdef CONFIG_OF
	struct device_node *node = NULL;
#endif
	int ret = 0;

	ASSERT(pvData);
	if (!pvData)
		return -1;

	prNetDevice = (struct net_device *)pvData;
	prGlueInfo = (struct GLUE_INFO *)pvCookie;
	ASSERT(prGlueInfo);
	if (!prGlueInfo)
		return -1;

	prHifInfo = &prGlueInfo->rHifInfo;
	pdev = prHifInfo->pdev;

	prHifInfo->u4IrqId = AXI_WLAN_IRQ_NUMBER;
#ifdef CONFIG_OF
	node = of_find_compatible_node(NULL, NULL, "mediatek,wifi");
	if (node) {
		prHifInfo->u4IrqId = irq_of_parse_and_map(node, 0);
		DBGLOG(INIT, INFO,
			"WIFI-OF: get wifi irq(%d)\n", prHifInfo->u4IrqId);
	} else
		DBGLOG(INIT, ERROR,
			"WIFI-OF: get wifi device node fail\n");
#endif
	DBGLOG(INIT, INFO, "glBusSetIrq: request_irq num(%d)\n",
	       prHifInfo->u4IrqId);
	ret = request_irq(prHifInfo->u4IrqId, mtk_axi_interrupt, IRQF_SHARED,
			  prNetDevice->name, prGlueInfo);
	if (ret != 0)
		DBGLOG(INIT, INFO,
			"glBusSetIrq: request_irq  ERROR(%d)\n", ret);

	return ret;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Stop bus interrupt operation and disable interrupt handling for os.
 *
 * \param[in] pvData     A pointer to struct net_device.
 * \param[in] pvCookie   Private data for pfnIsr function.
 *
 * \return (none)
 */
/*----------------------------------------------------------------------------*/
void glBusFreeIrq(void *pvData, void *pvCookie)
{
	struct net_device *prNetDevice = NULL;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct platform_device *pdev = NULL;

	ASSERT(pvData);
	if (!pvData) {
		DBGLOG(INIT, INFO, "%s null pvData\n", __func__);
		return;
	}
	prNetDevice = (struct net_device *)pvData;
	prGlueInfo = (struct GLUE_INFO *) pvCookie;
	ASSERT(prGlueInfo);
	if (!prGlueInfo) {
		DBGLOG(INIT, INFO, "%s no glue info\n", __func__);
		return;
	}

	prHifInfo = &prGlueInfo->rHifInfo;
	pdev = prHifInfo->pdev;

	synchronize_irq(prHifInfo->u4IrqId);
	free_irq(prHifInfo->u4IrqId, prGlueInfo);
}

u_int8_t glIsReadClearReg(uint32_t u4Address)
{
	return TRUE;
}

void glSetPowerState(IN struct GLUE_INFO *prGlueInfo, IN uint32_t ePowerMode)
{
}

void glGetDev(void *ctx, struct device **dev)
{
	*dev = &((struct platform_device *)ctx)->dev;
}

void glGetHifDev(struct GL_HIF_INFO *prHif, struct device **dev)
{
	*dev = &(prHif->pdev->dev);
}

static void *axiAllocDmaCoherent(size_t size, dma_addr_t *dma_handle,
				 bool fgIsTx, uint32_t u4Num)
{
	void *va = NULL;

	if (fgIsTx) {
		*dma_handle = g_prTxDescPa[u4Num];
		va = g_prTxDescVa[u4Num];
	} else {
		*dma_handle = g_prRxDescPa[u4Num];
		va = g_prRxDescVa[u4Num];
	}

	return va;
}

static struct sk_buff *axiAllocRxPacketBuf(uint32_t u4Len, uint32_t u4Num,
					   uint32_t u4Idx)
{
	struct sk_buff *prSkb;

	if (u4Num == 0)
		prSkb = g_prRxDataBuf[u4Idx];
	else
		prSkb = g_prRxEventBuf[u4Idx];

	return prSkb;
}

static struct sk_buff *axiAllocMsduBuf(uint32_t u4Len, uint32_t u4Idx)
{
	return g_prMsduBuf[u4Idx];
}

static void axiUpdateRxPacket(struct sk_buff *prSkb,
			      uint32_t u4Num, uint32_t u4Idx)
{
	if (u4Num == 0)
		g_prRxDataBuf[u4Idx] = prSkb;
	else
		g_prRxEventBuf[u4Idx] = prSkb;
}
