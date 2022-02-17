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
#ifndef CONFIG_X86
#include <asm/memory.h>
#endif

#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/of_reserved_mem.h>

#include "mt66xx_reg.h"

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */

#define MTK_PCI_VENDOR_ID	0x14C3
#define NIC6632_PCIe_DEVICE_ID	0x6632
#define NIC7668_PCIe_DEVICE_ID	0x7668
#define MT7663_PCI_PFGA2_VENDOR_ID	0x0E8D
#define NIC7663_PCIe_DEVICE_ID	0x7663
#define CONNAC_PCI_VENDOR_ID	0x0E8D
#define CONNAC_PCIe_DEVICE_ID	0x3280
#define NIC7915_PCIe_DEVICE_ID	0x7915
#define NICSOC3_0_PCIe_DEVICE_ID  0x0789
#define NIC7961_PCIe_DEVICE_ID	0x7961
#define NICSOC5_0_PCIe_DEVICE_ID  0x0789
#define NICSOC7_0_PCIe_DEVICE_ID  0x0789
#define NICBELLWETHER_PCIe_DEVICE_ID1 0x3107 /* used for FPGA */
#ifdef CFG_COMBO_SLT_GOLDEN
#define NICBELLWETHER_PCIe_DEVICE_ID2 0xF922 /* used for SLT golden */
#else
#define NICBELLWETHER_PCIe_DEVICE_ID2 0x7902 /* used for asic & FPGA */
#endif
#define NIC6639_PCIe_DEVICE_ID1 0x3107
#ifdef CFG_COMBO_SLT_GOLDEN
#define NIC6639_PCIe_DEVICE_ID2 0xE639 /*used for SLT golden */
#else
#define NIC6639_PCIe_DEVICE_ID2 0x6639
#endif
#define NIC7990_PCIe_DEVICE_ID 0x7990

static const struct pci_device_id mtk_pci_ids[] = {
#ifdef MT6632
	{	PCI_DEVICE(MTK_PCI_VENDOR_ID, NIC6632_PCIe_DEVICE_ID),
		.driver_data = (kernel_ulong_t)&mt66xx_driver_data_mt6632},
#endif /* MT6632 */
#ifdef MT7668
	{	PCI_DEVICE(MTK_PCI_VENDOR_ID, NIC7668_PCIe_DEVICE_ID),
		.driver_data = (kernel_ulong_t)&mt66xx_driver_data_mt7668},
#endif /* MT7668 */
#ifdef MT7663
	{	PCI_DEVICE(MTK_PCI_VENDOR_ID, NIC7663_PCIe_DEVICE_ID),
		.driver_data = (kernel_ulong_t)&mt66xx_driver_data_mt7663},
	/* For FPGA2 temparay */
	{	PCI_DEVICE(MT7663_PCI_PFGA2_VENDOR_ID, NIC7663_PCIe_DEVICE_ID),
		.driver_data = (kernel_ulong_t)&mt66xx_driver_data_mt7663},
#endif /* MT7663 */
#ifdef CONNAC
	{	PCI_DEVICE(CONNAC_PCI_VENDOR_ID, CONNAC_PCIe_DEVICE_ID),
		.driver_data = (kernel_ulong_t)&mt66xx_driver_data_connac},
#endif /* CONNAC */
#ifdef SOC2_1X1
		.driver_data = (kernel_ulong_t)&mt66xx_driver_data_soc2_1x1},
#endif /* SOC2_1X1 */
#ifdef SOC2_2X2
		.driver_data = (kernel_ulong_t)&mt66xx_driver_data_soc2_2x2},
#endif /* SOC2_2X2 */
#ifdef MT7915
	{	PCI_DEVICE(MTK_PCI_VENDOR_ID, NIC7915_PCIe_DEVICE_ID),
		.driver_data = (kernel_ulong_t)&mt66xx_driver_data_mt7915},
#endif /* MT7915 */
#ifdef SOC3_0
	{	PCI_DEVICE(MTK_PCI_VENDOR_ID, NICSOC3_0_PCIe_DEVICE_ID),
		.driver_data = (kernel_ulong_t)&mt66xx_driver_data_soc3_0 },
#endif /* SOC3_0 */
#ifdef MT7961
	{	PCI_DEVICE(MTK_PCI_VENDOR_ID, NIC7961_PCIe_DEVICE_ID),
		.driver_data = (kernel_ulong_t)&mt66xx_driver_data_mt7961},
#endif /* MT7961 */
#ifdef SOC5_0
	{	PCI_DEVICE(MTK_PCI_VENDOR_ID, NICSOC5_0_PCIe_DEVICE_ID),
		.driver_data = (kernel_ulong_t)&mt66xx_driver_data_soc5_0},
#endif /* SOC5_0 */
#ifdef SOC7_0
	{	PCI_DEVICE(MTK_PCI_VENDOR_ID, NICSOC7_0_PCIe_DEVICE_ID),
		.driver_data = (kernel_ulong_t)&mt66xx_driver_data_soc7_0},
#endif /* SOC7_0 */
#ifdef BELLWETHER
	{	PCI_DEVICE(MTK_PCI_VENDOR_ID, NICBELLWETHER_PCIe_DEVICE_ID1),
		.driver_data = (kernel_ulong_t)&mt66xx_driver_data_bellwether},
	{	PCI_DEVICE(MTK_PCI_VENDOR_ID, NICBELLWETHER_PCIe_DEVICE_ID2),
		.driver_data = (kernel_ulong_t)&mt66xx_driver_data_bellwether},
#endif /* BELLWETHER */
#ifdef MT6639
	{	PCI_DEVICE(MTK_PCI_VENDOR_ID, NIC6639_PCIe_DEVICE_ID1),
		.driver_data = (kernel_ulong_t)&mt66xx_driver_data_mt6639},
	{	PCI_DEVICE(MTK_PCI_VENDOR_ID, NIC6639_PCIe_DEVICE_ID2),
		.driver_data = (kernel_ulong_t)&mt66xx_driver_data_mt6639},
#endif /* MT6639 */
#ifdef MT7990
	{	PCI_DEVICE(MTK_PCI_VENDOR_ID, NIC7990_PCIe_DEVICE_ID),
		.driver_data = (kernel_ulong_t)&mt66xx_driver_data_mt7990},
#endif /* MT7990 */
	{ /* end: all zeroes */ },
};

static const struct platform_device_id mtk_axi_ids[] = {
	{	.name = "CONNAC",
#if defined(BELLWETHER)
		.driver_data = (kernel_ulong_t)&mt66xx_driver_data_bellwether
#elif defined(MT6639)
		.driver_data = (kernel_ulong_t)&mt66xx_driver_data_mt6639
#endif
	},

	{ /* end: all zeroes */ },
};

MODULE_DEVICE_TABLE(pci, mtk_pci_ids);

#ifdef CONFIG_OF
const struct of_device_id mtk_axi_of_ids[] = {
	{.compatible = "mediatek,wifi",},
	{}
};
#endif

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
static probe_card pfWlanProbe;
static remove_card pfWlanRemove;

static struct platform_driver mtk_axi_driver = {
	.driver = {
		.name = "wlan",
		.owner = THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table = mtk_axi_of_ids,
#endif
	},
	.id_table = mtk_axi_ids,
	.probe = NULL,
	.remove = NULL,
};

static struct pci_driver mtk_pci_driver = {
	.name = KBUILD_MODNAME,
	.id_table = mtk_pci_ids,
	.probe = NULL,
	.remove = NULL,
};

static struct GLUE_INFO *g_prGlueInfo;
static u_int8_t g_fgDriverProbed = FALSE;
struct pci_dev *g_prDev;

struct platform_device *g_prPlatDev = NULL;
#if AXI_CFG_PREALLOC_MEMORY_BUFFER
struct HIF_PREALLOC_MEM grMem;
unsigned long long gWifiRsvMemSize;

struct wifi_rsrv_mem {
	phys_addr_t phy_base;
	void *vir_base;
	unsigned long long size;
};

/* Assume reserved memory size < BIT(32) */
static struct wifi_rsrv_mem wifi_rsrv_mems[32];
#endif /* AXI_CFG_PREALLOC_MEMORY_BUFFER */

/*******************************************************************************
 *                                 M A C R O S
 *******************************************************************************
 */

/*******************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */
#if AXI_CFG_PREALLOC_MEMORY_BUFFER
static int _init_resv_mem(struct platform_device *pdev);

static void axiAllocTxDesc(struct GL_HIF_INFO *prHifInfo,
			   struct RTMP_DMABUF *prDescRing,
			   uint32_t u4Num);
static void axiAllocRxDesc(struct GL_HIF_INFO *prHifInfo,
			   struct RTMP_DMABUF *prDescRing,
			   uint32_t u4Num);
static bool axiAllocTxCmdBuf(struct RTMP_DMABUF *prDmaBuf,
			     uint32_t u4Num, uint32_t u4Idx);
#endif /* AXI_CFG_PREALLOC_MEMORY_BUFFER */

static void axiAllocTxDataBuf(struct MSDU_TOKEN_ENTRY *prToken, uint32_t u4Idx);
static void *axiAllocRxBuf(struct GL_HIF_INFO *prHifInfo,
			   struct RTMP_DMABUF *prDmaBuf,
			   uint32_t u4Num, uint32_t u4Idx);
static bool axiCopyCmd(struct GL_HIF_INFO *prHifInfo,
		       struct RTMP_DMACB *prTxCell, void *pucBuf,
		       void *pucSrc1, uint32_t u4SrcLen1,
		       void *pucSrc2, uint32_t u4SrcLen2);
static bool axiCopyEvent(struct GL_HIF_INFO *prHifInfo,
			 struct RTMP_DMACB *pRxCell,
			 struct RXD_STRUCT *pRxD,
			 struct RTMP_DMABUF *prDmaBuf,
			 uint8_t *pucDst, uint32_t u4Len);
static bool axiCopyTxData(struct MSDU_TOKEN_ENTRY *prToken,
			  void *pucSrc, uint32_t u4Len);
static bool axiCopyRxData(struct GL_HIF_INFO *prHifInfo,
			  struct RTMP_DMACB *pRxCell,
			  struct RTMP_DMABUF *prDmaBuf,
			  struct SW_RFB *prSwRfb);
static void axiDumpTx(struct GL_HIF_INFO *prHifInfo,
		      struct RTMP_TX_RING *prTxRing,
		      uint32_t u4Idx, uint32_t u4DumpLen);
static void axiDumpRx(struct GL_HIF_INFO *prHifInfo,
		      struct RTMP_RX_RING *prRxRing,
		      uint32_t u4Idx, uint32_t u4DumpLen);

static void pcieAllocDesc(struct GL_HIF_INFO *prHifInfo,
			  struct RTMP_DMABUF *prDescRing,
			  uint32_t u4Num);
static void *pcieAllocRxBuf(struct GL_HIF_INFO *prHifInfo,
			    struct RTMP_DMABUF *prDmaBuf,
			    uint32_t u4Num, uint32_t u4Idx);
static void pcieAllocTxDataBuf(struct MSDU_TOKEN_ENTRY *prToken,
			       uint32_t u4Idx);
static void *pcieAllocRuntimeMem(uint32_t u4SrcLen);
static bool pcieCopyCmd(struct GL_HIF_INFO *prHifInfo,
			struct RTMP_DMACB *prTxCell, void *pucBuf,
			void *pucSrc1, uint32_t u4SrcLen1,
			void *pucSrc2, uint32_t u4SrcLen2);
static bool pcieCopyEvent(struct GL_HIF_INFO *prHifInfo,
			  struct RTMP_DMACB *pRxCell,
			  struct RXD_STRUCT *pRxD,
			  struct RTMP_DMABUF *prDmaBuf,
			  uint8_t *pucDst, uint32_t u4Len);
static bool pcieCopyTxData(struct MSDU_TOKEN_ENTRY *prToken,
			   void *pucSrc, uint32_t u4Len);
static bool pcieCopyRxData(struct GL_HIF_INFO *prHifInfo,
			   struct RTMP_DMACB *pRxCell,
			   struct RTMP_DMABUF *prDmaBuf,
			   struct SW_RFB *prSwRfb);
static phys_addr_t pcieMapTxBuf(struct GL_HIF_INFO *prHifInfo,
			  void *pucBuf, uint32_t u4Offset, uint32_t u4Len);
static phys_addr_t pcieMapRxBuf(struct GL_HIF_INFO *prHifInfo,
			  void *pucBuf, uint32_t u4Offset, uint32_t u4Len);
static void pcieUnmapTxBuf(struct GL_HIF_INFO *prHifInfo,
			   phys_addr_t rDmaAddr, uint32_t u4Len);
static void pcieUnmapRxBuf(struct GL_HIF_INFO *prHifInfo,
			   phys_addr_t rDmaAddr, uint32_t u4Len);
static void pcieFreeDesc(struct GL_HIF_INFO *prHifInfo,
			 struct RTMP_DMABUF *prDescRing);
static void pcieFreeBuf(void *pucSrc, uint32_t u4Len);
static void pcieFreePacket(struct GL_HIF_INFO *prHifInfo,
			   void *pvPacket, uint32_t u4Num);
static void pcieDumpTx(struct GL_HIF_INFO *prHifInfo,
		       struct RTMP_TX_RING *prTxRing,
		       uint32_t u4Idx, uint32_t u4DumpLen);
static void pcieDumpRx(struct GL_HIF_INFO *prHifInfo,
		       struct RTMP_RX_RING *prRxRing,
		       uint32_t u4Idx, uint32_t u4DumpLen);
static void pcieAllocMcuEmiMem(struct GL_HIF_INFO *prHifInfo);
static void pcieFreeMcuEmiMem(struct GL_HIF_INFO *prHifInfo);

static void halPciePreSuspendCmd(struct ADAPTER *prAdapter);
static void halPcieResumeCmd(struct ADAPTER *prAdapter);

#if (CFG_SUPPORT_HOST_OFFLOAD == 1)
static void *pcieAllocRxBlkBuf(struct GL_HIF_INFO *prHifInfo,
			       struct RTMP_DMABUF *prDmaBuf,
			       uint32_t u4Num, uint32_t u4Idx);
static void pcieUnmapRxBlkBuf(struct GL_HIF_INFO *prHifInfo,
			      phys_addr_t rDmaAddr, uint32_t u4Len);
static void pcieReleaseRxBlkBuf(struct GL_HIF_INFO *prHifInfo,
				void *pvPacket, uint32_t u4Num);
#endif /* CFG_SUPPORT_HOST_OFFLOAD == 1 */

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */

/*----------------------------------------------------------------------------*/
/*!
 * \brief This function is a PCIE interrupt callback function
 *
 * \param[in] func  pointer to PCIE handle
 *
 * \return void
 */
/*----------------------------------------------------------------------------*/

struct mt66xx_hif_driver_data *get_platform_driver_data(void)
{
	if (!g_prDev)
		return NULL;

	return (struct mt66xx_hif_driver_data *) pci_get_drvdata(g_prDev);
}

irqreturn_t mtk_pci_interrupt(int irq, void *dev_instance)
{
	struct GLUE_INFO *prGlueInfo = NULL;
	static DEFINE_RATELIMIT_STATE(_rs, 2 * HZ, 1);

	prGlueInfo = (struct GLUE_INFO *) dev_instance;
	if (!prGlueInfo) {
		DBGLOG(HAL, INFO, "No glue info in mtk_pci_interrupt()\n");
		return IRQ_NONE;
	}

	GLUE_INC_REF_CNT(prGlueInfo->prAdapter->rHifStats.u4HwIsrCount);
	halDisableInterrupt(prGlueInfo->prAdapter);

	if (test_bit(GLUE_FLAG_HALT_BIT, &prGlueInfo->ulFlag)) {
		DBGLOG(HAL, INFO, "GLUE_FLAG_HALT skip INT\n");
		return IRQ_NONE;
	}

	kalSetIntEvent(prGlueInfo);
	if (__ratelimit(&_rs))
		pr_info("[wlan] In HIF ISR(%d).\n", irq);

	return IRQ_HANDLED;
}

irqreturn_t pcie_sw_int_top_handler(int irq, void *dev_instance)
{
	disable_irq_nosync(irq);
	return IRQ_WAKE_THREAD;
}

irqreturn_t pcie_sw_int_thread_handler(int irq, void *dev_instance)
{
	struct GLUE_INFO *prGlueInfo = NULL;
	struct ADAPTER *prAdapter = NULL;
	u_int8_t fgEnInt = true;

	prGlueInfo = (struct GLUE_INFO *)dev_instance;
	prAdapter = prGlueInfo->prAdapter;
	if (!prAdapter) {
		DBGLOG(HAL, WARN, "NULL prAdapter.\n");
		goto exit;
	}

	GLUE_INC_REF_CNT(prAdapter->rHifStats.u4SwIsrCount);

	if (prGlueInfo->ulFlag & GLUE_FLAG_HALT) {
		DBGLOG(HAL, INFO, "GLUE_FLAG_HALT skip INT\n");
		goto exit;
	}

#if (CFG_SUPPORT_CONNAC3X == 1)
	fgEnInt = asicConnac3xSwIntHandler(prAdapter);
#endif

exit:
	if (fgEnInt)
		enable_irq(irq);

	return IRQ_HANDLED;
}

static int axiDmaSetup(struct platform_device *pdev,
		struct mt66xx_hif_driver_data *prDriverData)
{
	struct mt66xx_chip_info *prChipInfo;
	u64 dma_mask;
	int ret = 0;

	prChipInfo = prDriverData->chip_info;

#if AXI_CFG_PREALLOC_MEMORY_BUFFER
	ret = _init_resv_mem(pdev);
	if (ret)
		goto exit;
	ret = of_reserved_mem_device_init(&pdev->dev);
	if (ret) {
		DBGLOG(INIT, ERROR, "of_reserved_mem_device_init failed(%d).\n",
				ret);
		goto exit;
	}
#else
	ret = of_dma_configure(&pdev->dev, pdev->dev.of_node, true);
	if (ret) {
		DBGLOG(INIT, ERROR, "of_dma_configure failed(%d).\n",
				ret);
		goto exit;
	}
#endif

	dma_mask = DMA_BIT_MASK(prChipInfo->bus_info->u4DmaMask);
	ret = dma_set_mask_and_coherent(&pdev->dev, dma_mask);
	if (ret) {
		DBGLOG(INIT, ERROR, "dma_set_mask_and_coherent failed(%d)\n",
			ret);
		goto exit;
	}

exit:
	return ret;
}

#if AXI_CFG_PREALLOC_MEMORY_BUFFER
static bool axiGetRsvMemSizeRsvedByKernel(struct platform_device *pdev)
{
#ifdef CONFIG_OF
	int ret = 0;
	struct device_node *np;

	np = of_parse_phandle(pdev->dev.of_node, "memory-region", 0);
	if (!np) {
		DBGLOG(INIT, ERROR, "can NOT find memory-region.\n");
		return false;
	}

	ret = of_property_read_u64_array(np, "size", &gWifiRsvMemSize, 1);
	if (ret != 0)
		DBGLOG(INIT, ERROR, "get rsrv mem size failed(%d).\n", ret);
	else
		DBGLOG(INIT, INFO, "gWifiRsvMemSize: 0x%x\n", gWifiRsvMemSize);

	of_node_put(np);
	if (ret != 0)
		return false;
	else
		return true;
#else
	return false;
#endif
}

static bool axiAllocRsvMem(uint32_t u4Size, struct HIF_MEM *prMem)
{
	/* 8 bytes alignment */
	if (u4Size & 7)
		u4Size += 8 - (u4Size & 7);

	if ((grMem.u4Offset + u4Size) >= gWifiRsvMemSize)
		return false;

	prMem->pa = grMem.pucRsvMemBase + grMem.u4Offset;
	prMem->va = grMem.pucRsvMemVirBase + grMem.u4Offset;
	grMem.u4Offset += u4Size;

	return prMem->va != NULL;
}

static int axiAllocHifMem(struct platform_device *pdev,
		struct mt66xx_hif_driver_data *prDriverData)
{
	struct mt66xx_chip_info *prChipInfo;
	uint32_t u4Idx, u4Size;
	uint32_t i = sizeof(wifi_rsrv_mems) / sizeof(struct wifi_rsrv_mem);

	prChipInfo = prDriverData->chip_info;

	/* Allocation size should be a power of two */
	while (i > 0) {
		i--;
		if (!(gWifiRsvMemSize & BIT(i)))
			continue;

		wifi_rsrv_mems[i].size = BIT(i);
		wifi_rsrv_mems[i].vir_base =
			dma_alloc_coherent(&pdev->dev,
					   wifi_rsrv_mems[i].size,
					   &wifi_rsrv_mems[i].phy_base,
					   GFP_DMA);
		if (!wifi_rsrv_mems[i].vir_base) {
			DBGLOG(INIT, ERROR,
				"[%d] DMA_ALLOC_COHERENT failed, size: 0x%x\n",
				i, wifi_rsrv_mems[i].size);
			return -1;
		}
		if (!grMem.pucRsvMemBase) {
			grMem.pucRsvMemBase = wifi_rsrv_mems[i].phy_base;
			grMem.pucRsvMemVirBase = wifi_rsrv_mems[i].vir_base;
			grMem.u4RsvMemSize = (uint64_t) gWifiRsvMemSize;
		}
	}

	if (!grMem.pucRsvMemBase)
		return -1;

	DBGLOG(INIT, INFO, "pucRsvMemBase[%pa], pucRsvMemVirBase[%pa]\n",
	       &grMem.pucRsvMemBase,
	       &grMem.pucRsvMemVirBase);

	for (u4Idx = 0; u4Idx < NUM_OF_TX_RING; u4Idx++) {
		if (u4Idx == TX_RING_DATA1_IDX_1 &&
				!prChipInfo->bus_info->tx_ring1_data_idx)
			continue;
		else if (u4Idx == TX_RING_DATA2_IDX_2 &&
				!prChipInfo->bus_info->tx_ring2_data_idx)
			continue;
		if (!axiAllocRsvMem(TX_RING_SIZE * TXD_SIZE,
				    &grMem.rTxDesc[u4Idx]))
			DBGLOG(INIT, ERROR, "TxDesc[%u] alloc fail\n", u4Idx);
	}

	for (u4Idx = 0; u4Idx < NUM_OF_RX_RING; u4Idx++) {
		if (u4Idx == RX_RING_DATA_IDX_0 || u4Idx == RX_RING_DATA1_IDX_2 ||
				u4Idx == RX_RING_DATA2_IDX_5)
			u4Size = RX_RING0_SIZE;
		else
			u4Size = RX_RING1_SIZE;
		if (!axiAllocRsvMem(u4Size * RXD_SIZE, &grMem.rRxDesc[u4Idx]))
			DBGLOG(INIT, ERROR, "RxDesc[%u] alloc fail\n", u4Idx);
	}

	for (u4Idx = 0; u4Idx < TX_RING_SIZE; u4Idx++) {
		if (!axiAllocRsvMem(AXI_TX_CMD_BUFF_SIZE,
				    &grMem.rTxCmdBuf[u4Idx]))
			DBGLOG(INIT, ERROR, "TxCmdBuf[%u] alloc fail\n", u4Idx);
	}

	for (u4Idx = 0; u4Idx < RX_RING0_SIZE; u4Idx++) {
		if (!axiAllocRsvMem(CFG_RX_MAX_PKT_SIZE,
				    &grMem.rRxDataBuf[u4Idx]))
			DBGLOG(INIT, ERROR,
			       "RxDataBuf[%u] alloc fail\n", u4Idx);
	}

	for (u4Idx = 0; u4Idx < RX_RING1_SIZE; u4Idx++) {
		if (!axiAllocRsvMem(RX_BUFFER_AGGRESIZE,
				    &grMem.rRxEventBuf[u4Idx]))
			DBGLOG(INIT, ERROR,
			       "RxEventBuf[%u] alloc fail\n", u4Idx);
	}

#if (CFG_SUPPORT_CONNAC2X == 1 || CFG_SUPPORT_CONNAC3X == 1)
	for (u4Idx = 0; u4Idx < RX_RING0_SIZE; u4Idx++) {
		if (!axiAllocRsvMem(CFG_RX_MAX_PKT_SIZE,
				    &grMem.rRxData1Buf[u4Idx]))
			DBGLOG(INIT, ERROR,
			       "RxData1Buf[%u] alloc fail\n", u4Idx);
	}
	for (u4Idx = 0; u4Idx < RX_RING0_SIZE; u4Idx++) {
		if (!axiAllocRsvMem(CFG_RX_MAX_PKT_SIZE,
				    &grMem.rRxData2Buf[u4Idx]))
			DBGLOG(INIT, ERROR,
			       "RxData1Buf[%u] alloc fail\n", u4Idx);
	}

	for (u4Idx = 0; u4Idx < RX_RING1_SIZE; u4Idx++) {
		if (!axiAllocRsvMem(RX_BUFFER_AGGRESIZE,
				    &grMem.rTxFreeDoneEvent0Buf[u4Idx]))
			DBGLOG(INIT, ERROR,
			       "TxFreeDoneEvent0Buf[%u] alloc fail\n", u4Idx);
	}

	for (u4Idx = 0; u4Idx < RX_RING1_SIZE; u4Idx++) {
		if (!axiAllocRsvMem(RX_BUFFER_AGGRESIZE,
				    &grMem.rTxFreeDoneEvent1Buf[u4Idx]))
			DBGLOG(INIT, ERROR,
			       "TxFreeDoneEvent1Buf[%u] alloc fail\n", u4Idx);
	}

	for (u4Idx = 0; u4Idx < RX_RING1_SIZE; u4Idx++) {
		if (!axiAllocRsvMem(RX_BUFFER_AGGRESIZE,
				    &grMem.rTxFreeDoneEvent2Buf[u4Idx]))
			DBGLOG(INIT, ERROR,
			       "TxFreeDoneEvent1Buf[%u] alloc fail\n", u4Idx);
	}
#endif

#if HIF_TX_PREALLOC_DATA_BUFFER
	for (u4Idx = 0; u4Idx < HIF_TX_MSDU_TOKEN_NUM; u4Idx++) {
		if (!axiAllocRsvMem(AXI_TX_MAX_SIZE_PER_FRAME +
				    prChipInfo->txd_append_size,
				    &grMem.rMsduBuf[u4Idx]))
			DBGLOG(INIT, ERROR, "MsduBuf[%u] alloc fail\n", u4Idx);
	}
#endif

	DBGLOG(INIT, INFO, "grMem.u4Offset[0x%x]\n", grMem.u4Offset);

	return 0;
}

static void axiFreeHifMem(struct platform_device *pdev)
{
	uint32_t i = 0;
	uint32_t count = sizeof(wifi_rsrv_mems) / sizeof(struct wifi_rsrv_mem);

	for (i = 0; i < count; i++) {
		if (!wifi_rsrv_mems[i].vir_base)
			continue;
		dma_free_coherent(
			&pdev->dev,
			wifi_rsrv_mems[i].size,
			wifi_rsrv_mems[i].vir_base,
			(dma_addr_t) wifi_rsrv_mems[i].phy_base);
	}
}

static int _init_resv_mem(struct platform_device *pdev)
{
#ifdef CONFIG_OF
	int ret = 0;
	struct device_node *node = NULL;
	unsigned int RsvMemSize;

	node = pdev->dev.of_node;
	if (!node) {
		DBGLOG(INIT, ERROR, "WIFI-OF: get wifi device node fail\n");
		of_node_put(node);
		return false;
	}

	if (axiGetRsvMemSizeRsvedByKernel(pdev) == false) {
		ret = of_property_read_u32(node, "emi-size", &RsvMemSize);
		if (ret != 0)
			DBGLOG(INIT, ERROR,
				"MPU-in-lk get rsrv mem size failed(%d).\n",
				ret);
		else {
			gWifiRsvMemSize = (unsigned long long) RsvMemSize;
			DBGLOG(INIT, INFO, "MPU-in-lk gWifiRsvMemSize: 0x%x\n",
				gWifiRsvMemSize);
		}
	}

	of_node_put(node);

	return ret;
#else
	DBGLOG(INIT, ERROR, "kernel option CONFIG_OF not enabled.\n");
	return -1;
#endif
}

#endif /* AXI_CFG_PREALLOC_MEMORY_BUFFER */

static int mtk_axi_probe(IN struct platform_device *pdev)
{
	struct mt66xx_hif_driver_data *prDriverData;
	struct mt66xx_chip_info *prChipInfo;
	int ret = 0;

	g_prPlatDev = pdev;
	prDriverData = (struct mt66xx_hif_driver_data *)
			mtk_axi_ids[0].driver_data;
	prChipInfo = prDriverData->chip_info;

	platform_set_drvdata(pdev, (void *) prDriverData);

	ret = axiDmaSetup(pdev, prDriverData);
	if (ret)
		goto exit;

#if AXI_CFG_PREALLOC_MEMORY_BUFFER
	ret = axiAllocHifMem(pdev, prDriverData);
	if (ret)
		goto exit;
#endif

exit:
	DBGLOG(INIT, INFO, "mtk_axi_probe() done, ret: %d\n", ret);
	return ret;
}

static int mtk_axi_remove(IN struct platform_device *pdev)
{
#if AXI_CFG_PREALLOC_MEMORY_BUFFER
	axiFreeHifMem(pdev);
#endif
	platform_set_drvdata(pdev, NULL);
	return 0;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief This function is a PCIE probe function
 *
 * \param[in] func   pointer to PCIE handle
 * \param[in] id     pointer to PCIE device id table
 *
 * \return void
 */
/*----------------------------------------------------------------------------*/
static int mtk_pci_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
	struct mt66xx_hif_driver_data *prDriverData;
	struct mt66xx_chip_info *prChipInfo;
	struct BUS_INFO *prBusInfo;
	struct pcie_msi_info *prMsiInfo;
	uint32_t u4MaxMsiNum;
	int ret = 0;

	ASSERT(pdev);
	ASSERT(id);

	prDriverData = (struct mt66xx_hif_driver_data *)id->driver_data;
	prChipInfo = prDriverData->chip_info;
	prBusInfo = prChipInfo->bus_info;
	prMsiInfo = &prBusInfo->pcie_msi_info;

	ret = pcim_enable_device(pdev);

	if (ret) {
		DBGLOG(INIT, INFO,
			"pci_enable_device failed, ret=%d\n", ret);
		goto out;
	}

	ret = pcim_iomap_regions(pdev, BIT(0), pci_name(pdev));
	if (ret) {
		DBGLOG(INIT, INFO,
			"pcim_iomap_regions failed, ret=%d\n", ret);
		goto out;
	}

	pci_set_master(pdev);

#if IS_ENABLED(CFG_MTK_WIFI_PCIE_MSI_SUPPORT)
	u4MaxMsiNum = prMsiInfo->u4MaxMsiNum ?
		prMsiInfo->u4MaxMsiNum : 1;
#else
	u4MaxMsiNum = 1;
#endif
	ret = pci_alloc_irq_vectors(pdev, 1,
				    u4MaxMsiNum,
				    PCI_IRQ_MSI);
	if (ret < 0) {
		DBGLOG(INIT, INFO,
			"pci_alloc_irq_vectors(1, %d) failed, ret=%d\n",
			u4MaxMsiNum,
			ret);
		goto err_free_iomap;
	}
	if (u4MaxMsiNum > 1 && ret == prMsiInfo->u4MaxMsiNum) {
		prMsiInfo->fgMsiEnabled = TRUE;
		prMsiInfo->u4MsiNum = ret;
	} else {
		prMsiInfo->fgMsiEnabled = FALSE;
		prMsiInfo->u4MsiNum = 1;
	}
	DBGLOG(INIT, INFO, "ret=%d, fgMsiEnabled=%d, u4MsiNum=%d\n",
		ret, prMsiInfo->fgMsiEnabled, prMsiInfo->u4MsiNum);

	ret = pci_set_dma_mask(pdev,
		DMA_BIT_MASK(prChipInfo->bus_info->u4DmaMask));
	if (ret != 0) {
		DBGLOG(INIT, INFO,
			"pci_set_dma_mask failed, ret=%d\n", ret);
		goto err_free_irq_vectors;
	}

	g_prDev = pdev;
	prChipInfo->pdev = (void *)pdev;
	prChipInfo->CSRBaseAddress = pcim_iomap_table(pdev)[0];

	DBGLOG(INIT, INFO, "ioremap for device %s, region 0x%lX @ 0x%lX\n",
		pci_name(pdev), (unsigned long) pci_resource_len(pdev, 0),
		(unsigned long) pci_resource_start(pdev, 0));

	pci_set_drvdata(pdev, (void *)id->driver_data);

	if (pfWlanProbe((void *) pdev,
		(void *) id->driver_data) != WLAN_STATUS_SUCCESS) {
		DBGLOG(INIT, INFO, "pfWlanProbe fail!call pfWlanRemove()\n");
		ret = -1;
		goto err_free_irq_vectors;
	}

	g_fgDriverProbed = TRUE;
	goto out;

err_free_irq_vectors:
	pci_free_irq_vectors(pdev);

err_free_iomap:
	pcim_iounmap_regions(pdev, BIT(0));

out:
	DBGLOG(INIT, INFO, "mtk_pci_probe() done(%d)\n", ret);

	return ret;
}

static void mtk_pci_remove(struct pci_dev *pdev)
{
	ASSERT(pdev);

	if (g_fgDriverProbed) {
		pfWlanRemove();
		DBGLOG(INIT, INFO, "pfWlanRemove done\n");
	}
	pci_set_drvdata(pdev, NULL);
	pci_free_irq_vectors(pdev);
	pcim_iounmap_regions(pdev, BIT(0));
}

static int mtk_pci_suspend(struct pci_dev *pdev, pm_message_t state)
{
	struct GLUE_INFO *prGlueInfo = NULL;
	struct BUS_INFO *prBusInfo;
	uint32_t count = 0;
	int wait = 0;
	struct ADAPTER *prAdapter = NULL;
	uint8_t drv_own_fail = FALSE;

	DBGLOG(HAL, STATE, "mtk_pci_suspend()\n");

	prGlueInfo = g_prGlueInfo;
	if (!prGlueInfo) {
		DBGLOG(HAL, ERROR, "prGlueInfo is NULL!\n");
		return -1;
	}

	prAdapter = prGlueInfo->prAdapter;
	prGlueInfo->fgIsInSuspendMode = TRUE;

	ACQUIRE_POWER_CONTROL_FROM_PM(prAdapter);

	/* Stop upper layers calling the device hard_start_xmit routine. */
	netif_tx_stop_all_queues(prGlueInfo->prDevHandler);

#if CFG_ENABLE_WAKE_LOCK
	prGlueInfo->rHifInfo.eSuspendtate = PCIE_STATE_SUSPEND_ENTERING;
#endif

	/* wait wiphy device do cfg80211 suspend done, then start hif suspend */
	if (IS_FEATURE_ENABLED(prGlueInfo->prAdapter->rWifiVar.ucWow))
		wlanWaitCfg80211SuspendDone(prGlueInfo);

	wlanSuspendPmHandle(prGlueInfo);

#if !CFG_ENABLE_WAKE_LOCK
	prGlueInfo->rHifInfo.eSuspendtate = PCIE_STATE_PRE_SUSPEND_WAITING;
#endif

	halPciePreSuspendCmd(prAdapter);

	while (prGlueInfo->rHifInfo.eSuspendtate !=
		PCIE_STATE_PRE_SUSPEND_DONE) {
		if (count > 500) {
			DBGLOG(HAL, ERROR, "pcie pre_suspend timeout\n");
			return -EAGAIN;
		}
		kalMsleep(2);
		count++;
	}
	DBGLOG(HAL, ERROR, "pcie pre_suspend done\n");

	prGlueInfo->rHifInfo.eSuspendtate = PCIE_STATE_SUSPEND;

	/* Polling until HIF side PDMAs are all idle */
	prBusInfo = prAdapter->chip_info->bus_info;
	if (prBusInfo->pdmaPollingIdle) {
		if (prBusInfo->pdmaPollingIdle(prGlueInfo) != TRUE)
			return -EAGAIN;
	} else
		DBGLOG(HAL, ERROR, "PDMA polling idle API didn't register\n");

	/* Disable HIF side PDMA TX/RX */
	if (prBusInfo->pdmaStop)
		prBusInfo->pdmaStop(prGlueInfo, TRUE);
	else
		DBGLOG(HAL, ERROR, "PDMA config API didn't register\n");

	halDisableInterrupt(prAdapter);

	/* FW own */
	/* Set FW own directly without waiting sleep notify */
	prAdapter->fgWiFiInSleepyState = TRUE;
	RECLAIM_POWER_CONTROL_TO_PM(prAdapter, FALSE);

	/* Wait for
	*  1. The other unfinished ownership handshakes
	*  2. FW own back
	*/
	while (wait < 500) {
		if ((prAdapter->u4PwrCtrlBlockCnt == 0) &&
		    (prAdapter->fgIsFwOwn == TRUE) &&
		    (drv_own_fail == FALSE)) {
			DBGLOG(HAL, STATE, "*********************\n");
			DBGLOG(HAL, STATE, "* Enter PCIE Suspend *\n");
			DBGLOG(HAL, STATE, "*********************\n");
			DBGLOG(HAL, INFO, "wait = %d\n\n", wait);
			break;
		}

		ACQUIRE_POWER_CONTROL_FROM_PM(prAdapter);
		/* Prevent that suspend without FW Own:
		 * Set Drv own has failed,
		 * and then Set FW Own is skipped
		 */
		if (prAdapter->fgIsFwOwn == FALSE)
			drv_own_fail = FALSE;
		else
			drv_own_fail = TRUE;
		/* For single core CPU */
		/* let hif_thread can be completed */
		usleep_range(1000, 3000);
		RECLAIM_POWER_CONTROL_TO_PM(prAdapter, FALSE);

		wait++;
	}

	if (wait >= 500) {
		DBGLOG(HAL, ERROR, "Set FW Own Timeout !!\n");
		return -EAGAIN;
	}

	pci_save_state(pdev);
	pci_set_power_state(pdev, pci_choose_state(pdev, state));

	DBGLOG(HAL, STATE, "mtk_pci_suspend() done!\n");

	/* pending cmd will be kept in queue and no one to handle it after HIF resume.
	 * In STR, it will result in cmd buf full and then cmd buf alloc fail .
	 */
	if (IS_FEATURE_ENABLED(prGlueInfo->prAdapter->rWifiVar.ucWow))
		wlanReleaseAllTxCmdQueue(prGlueInfo->prAdapter);

	return 0;
}


int mtk_pci_resume(struct pci_dev *pdev)
{
	struct GLUE_INFO *prGlueInfo = NULL;
	struct BUS_INFO *prBusInfo;

	DBGLOG(HAL, STATE, "mtk_pci_resume()\n");

	prGlueInfo = g_prGlueInfo;
	if (!prGlueInfo) {
		DBGLOG(HAL, ERROR, "prGlueInfo is NULL!\n");
		return -1;
	}

	prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;

	pci_set_power_state(pdev, PCI_D0);
	pci_restore_state(pdev);

	/* Driver own */
	/* Include restore PDMA settings */
	ACQUIRE_POWER_CONTROL_FROM_PM(prGlueInfo->prAdapter);

	if (prBusInfo->initPcieInt)
		prBusInfo->initPcieInt(prGlueInfo);

	halEnableInterrupt(prGlueInfo->prAdapter);

	/* Enable HIF side PDMA TX/RX */
	if (prBusInfo->pdmaStop)
		prBusInfo->pdmaStop(prGlueInfo, FALSE);
	else
		DBGLOG(HAL, ERROR, "PDMA config API didn't register\n");

	halPcieResumeCmd(prGlueInfo->prAdapter);

	wlanResumePmHandle(prGlueInfo);

	/* FW own */
	RECLAIM_POWER_CONTROL_TO_PM(prGlueInfo->prAdapter, FALSE);

	prGlueInfo->fgIsInSuspendMode = FALSE;
	/* Allow upper layers to call the device hard_start_xmit routine. */
	netif_tx_wake_all_queues(prGlueInfo->prDevHandler);

	DBGLOG(HAL, STATE, "mtk_pci_resume() done!\n");

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

	mtk_pci_driver.probe = mtk_pci_probe;
	mtk_pci_driver.remove = mtk_pci_remove;

	mtk_pci_driver.suspend = mtk_pci_suspend;
	mtk_pci_driver.resume = mtk_pci_resume;

	mtk_axi_driver.probe = mtk_axi_probe;
	mtk_axi_driver.remove = mtk_axi_remove;

	if (platform_driver_register(&mtk_axi_driver))
		DBGLOG(HAL, ERROR, "platform_driver_register fail\n");

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
	platform_driver_unregister(&mtk_axi_driver);
}

static void glPopulateMemOps(struct mt66xx_chip_info *prChipInfo,
			     struct HIF_MEM_OPS *prMemOps)
{
	prMemOps->allocTxDesc = pcieAllocDesc;
	prMemOps->allocRxDesc = pcieAllocDesc;
	prMemOps->allocTxCmdBuf = NULL;
	prMemOps->allocTxDataBuf = pcieAllocTxDataBuf;
	prMemOps->allocRxBuf = pcieAllocRxBuf;
	prMemOps->allocRuntimeMem = pcieAllocRuntimeMem;
	prMemOps->copyCmd = pcieCopyCmd;
	prMemOps->copyEvent = pcieCopyEvent;
	prMemOps->copyTxData = pcieCopyTxData;
	prMemOps->copyRxData = pcieCopyRxData;
	prMemOps->mapTxBuf = pcieMapTxBuf;
	prMemOps->mapRxBuf = pcieMapRxBuf;
	prMemOps->unmapTxBuf = pcieUnmapTxBuf;
	prMemOps->unmapRxBuf = pcieUnmapRxBuf;
	prMemOps->freeDesc = pcieFreeDesc;
	prMemOps->freeBuf = pcieFreeBuf;
	prMemOps->freePacket = pcieFreePacket;
#if 0
	prMemOps->dumpTx = pcieDumpTx;
	prMemOps->dumpRx = pcieDumpRx;
#endif
#ifdef MT6639
	prMemOps->allocMcuEmiMem = pcieAllocMcuEmiMem;
	prMemOps->freeMcuEmiMem = pcieFreeMcuEmiMem;
#endif

#if AXI_CFG_PREALLOC_MEMORY_BUFFER
	if (g_prPlatDev) {
		DBGLOG(HAL, INFO, "Use pre-alloc mem ops instead.\n");
		prMemOps->allocTxDesc = axiAllocTxDesc;
		prMemOps->allocRxDesc = axiAllocRxDesc;
		prMemOps->allocTxCmdBuf = axiAllocTxCmdBuf;
		prMemOps->allocTxDataBuf = axiAllocTxDataBuf;
		prMemOps->allocRxBuf = axiAllocRxBuf;
		prMemOps->allocRuntimeMem = NULL;
		prMemOps->copyCmd = axiCopyCmd;
		prMemOps->copyEvent = axiCopyEvent;
		prMemOps->copyTxData = axiCopyTxData;
		prMemOps->copyRxData = axiCopyRxData;
		prMemOps->mapTxBuf = NULL;
		prMemOps->mapRxBuf = NULL;
		prMemOps->unmapTxBuf = NULL;
		prMemOps->unmapRxBuf = NULL;
		prMemOps->freeDesc = NULL;
		prMemOps->freeBuf = NULL;
		prMemOps->freePacket = NULL;
		prMemOps->dumpTx = axiDumpTx;
		prMemOps->dumpRx = axiDumpRx;
	}
#endif

#if (CFG_SUPPORT_HOST_OFFLOAD == 1)
	if (prChipInfo->is_support_rro) {
		prMemOps->allocRxBuf = pcieAllocRxBlkBuf;
		prMemOps->unmapRxBuf = pcieUnmapRxBlkBuf;
		prMemOps->freePacket = pcieReleaseRxBlkBuf;
	}
#endif /* CFG_SUPPORT_HOST_OFFLOAD == 1 */
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
	struct mt66xx_chip_info *prChipInfo = NULL;
	struct GL_HIF_INFO *prHif = NULL;
	struct HIF_MEM_OPS *prMemOps;

	prHif = &prGlueInfo->rHifInfo;
	glGetChipInfo((void **)&prChipInfo);

	prHif->pdev = (struct pci_dev *)ulCookie;
	prMemOps = &prHif->rMemOps;
	prHif->prDmaDev = prHif->pdev;

	g_prGlueInfo = prGlueInfo;

	if (!prChipInfo)
		prHif->CSRBaseAddress = prChipInfo->CSRBaseAddress;

	SET_NETDEV_DEV(prGlueInfo->prDevHandler, &prHif->pdev->dev);

	prGlueInfo->u4InfType = MT_DEV_INF_PCIE;

	glPopulateMemOps(prChipInfo, prMemOps);
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
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This function reset necessary hif related info when chip reset.
*
* \param[in] prGlueInfo Pointer to glue info structure
*
* \return (none)
*/
/*----------------------------------------------------------------------------*/
void glResetHifInfo(struct GLUE_INFO *prGlueInfo)
{
	ASSERT(prGlueInfo);
} /* end of glResetHifInfo() */

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
	return TRUE;
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

static int32_t glBusSetMsiIrq(struct pci_dev *pdev,
	struct GLUE_INFO *prGlueInfo,
	struct BUS_INFO *prBusInfo)
{
	struct pcie_msi_info *prMsiInfo = &prBusInfo->pcie_msi_info;
	uint8_t i = 0;
	int ret = 0;

	for (i = 0; i < prMsiInfo->u4MsiNum; i++) {
		struct pcie_msi_layout *prMsiLayout =
			&prMsiInfo->prMsiLayout[i];
		int irqn = pci_irq_vector(pdev, i);
		int en_wake_ret = 0;

		if (prMsiLayout && !prMsiLayout->top_handler &&
		    !prMsiLayout->thread_handler)
			continue;

		ret = devm_request_threaded_irq(&pdev->dev,
			irqn,
			prMsiLayout->top_handler,
			prMsiLayout->thread_handler,
			IRQF_SHARED,
			KBUILD_MODNAME,
			prGlueInfo);
		en_wake_ret = enable_irq_wake(irqn);
		DBGLOG(INIT, INFO, "request_irq(%d %s %d %d)\n",
			irqn, prMsiLayout->name, ret, en_wake_ret);
		if (ret)
			goto err;
	}

	return 0;

err:
	while (i--) {
		struct pcie_msi_layout *prMsiLayout =
			&prMsiInfo->prMsiLayout[i];
		int irqn = pci_irq_vector(pdev, i);

		if (prMsiLayout && !prMsiLayout->top_handler &&
		    !prMsiLayout->thread_handler)
			continue;

		devm_free_irq(&pdev->dev, irqn, prGlueInfo);
	}
	return ret;
}

static int32_t glBusSetLegacyIrq(struct pci_dev *pdev,
	struct GLUE_INFO *prGlueInfo,
	struct BUS_INFO *prBusInfo)
{
	struct GL_HIF_INFO *prHifInfo = NULL;

	prHifInfo = &prGlueInfo->rHifInfo;

	return devm_request_irq(&pdev->dev,
			prHifInfo->u4IrqId,
			mtk_pci_interrupt,
			IRQF_SHARED,
			KBUILD_MODNAME,
			prGlueInfo);
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
	struct GLUE_INFO *prGlueInfo = NULL;
	struct BUS_INFO *prBusInfo = NULL;
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct pcie_msi_info *prMsiInfo = NULL;
	struct pci_dev *pdev = NULL;
	int ret = 0;

	prGlueInfo = (struct GLUE_INFO *)pvCookie;
	ASSERT(prGlueInfo);
	if (!prGlueInfo)
		return -1;

	prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;
	prMsiInfo = &prBusInfo->pcie_msi_info;
	prHifInfo = &prGlueInfo->rHifInfo;
	pdev = prHifInfo->pdev;

	prHifInfo->u4IrqId = pdev->irq;
	if (prMsiInfo && prMsiInfo->fgMsiEnabled)
		ret = glBusSetMsiIrq(pdev, prGlueInfo, prBusInfo);
	else
		ret = glBusSetLegacyIrq(pdev, prGlueInfo, prBusInfo);

	if (ret)
		goto exit;

	if (prBusInfo->initPcieInt)
		prBusInfo->initPcieInt(prGlueInfo);

exit:
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
	struct GLUE_INFO *prGlueInfo = NULL;
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct BUS_INFO *prBusInfo = NULL;
	struct pcie_msi_info *prMsiInfo = NULL;
	struct pci_dev *pdev = NULL;
	uint8_t i = 0;

	prGlueInfo = (struct GLUE_INFO *) pvCookie;
	ASSERT(prGlueInfo);
	if (!prGlueInfo) {
		DBGLOG(INIT, INFO, "%s no glue info\n", __func__);
		return;
	}

	prHifInfo = &prGlueInfo->rHifInfo;
	prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;
	prMsiInfo = &prBusInfo->pcie_msi_info;
	pdev = prHifInfo->pdev;

	for (i = 0; i < prMsiInfo->u4MsiNum; i++) {
		struct pcie_msi_layout *prMsiLayout =
			&prMsiInfo->prMsiLayout[i];
		int irqn = pci_irq_vector(pdev, i);

		if (prMsiLayout && !prMsiLayout->top_handler &&
		    !prMsiLayout->thread_handler)
			continue;

		synchronize_irq(irqn);
		devm_free_irq(&pdev->dev, irqn, prGlueInfo);
	}
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

	*dev = &((struct pci_dev *)ctx)->dev;
}

void glGetHifDev(struct GL_HIF_INFO *prHif, struct device **dev)
{
	*dev = &(prHif->pdev->dev);
}

void glGetChipInfo(void **prChipInfo)
{
	struct mt66xx_hif_driver_data *prDriverData;

	prDriverData = get_platform_driver_data();
	if (!prDriverData)
		return;

	*prChipInfo = (void *)prDriverData->chip_info;
}

static void pcieAllocDesc(struct GL_HIF_INFO *prHifInfo,
			  struct RTMP_DMABUF *prDescRing,
			  uint32_t u4Num)
{
	dma_addr_t rAddr;

	prDescRing->AllocVa = KAL_DMA_ALLOC_COHERENT(
		prHifInfo->prDmaDev, prDescRing->AllocSize, &rAddr);
	prDescRing->AllocPa = (phys_addr_t)rAddr;
	if (prDescRing->AllocVa)
		memset(prDescRing->AllocVa, 0, prDescRing->AllocSize);
}

static void pcieAllocTxDataBuf(struct MSDU_TOKEN_ENTRY *prToken, uint32_t u4Idx)
{
	prToken->prPacket = kalMemAlloc(prToken->u4DmaLength, PHY_MEM_TYPE);
	prToken->rDmaAddr = 0;
}

static void *pcieAllocRxBuf(struct GL_HIF_INFO *prHifInfo,
			    struct RTMP_DMABUF *prDmaBuf,
			    uint32_t u4Num, uint32_t u4Idx)
{
	struct sk_buff *pkt = dev_alloc_skb(prDmaBuf->AllocSize);
	dma_addr_t rAddr;

	if (!pkt) {
		DBGLOG(HAL, ERROR, "can't allocate rx %lu size packet\n",
		       prDmaBuf->AllocSize);
		prDmaBuf->AllocPa = 0;
		prDmaBuf->AllocVa = NULL;
		return NULL;
	}

#ifdef CFG_SUPPORT_SNIFFER_RADIOTAP
	skb_reserve(pkt, CFG_RADIOTAP_HEADROOM);
#endif
	prDmaBuf->AllocVa = (void *)pkt->data;
	memset(prDmaBuf->AllocVa, 0, prDmaBuf->AllocSize);

	rAddr = KAL_DMA_MAP_SINGLE(prHifInfo->prDmaDev, prDmaBuf->AllocVa,
				   prDmaBuf->AllocSize, KAL_DMA_FROM_DEVICE);
	if (KAL_DMA_MAPPING_ERROR(prHifInfo->prDmaDev, rAddr)) {
		DBGLOG(HAL, ERROR, "sk_buff dma mapping error!\n");
		dev_kfree_skb(pkt);
		return NULL;
	}
	prDmaBuf->AllocPa = (phys_addr_t)rAddr;
	return (void *)pkt;
}

static void *pcieAllocRuntimeMem(uint32_t u4SrcLen)
{
	return kalMemAlloc(u4SrcLen, PHY_MEM_TYPE);
}

static bool pcieCopyCmd(struct GL_HIF_INFO *prHifInfo,
			struct RTMP_DMACB *prTxCell, void *pucBuf,
			void *pucSrc1, uint32_t u4SrcLen1,
			void *pucSrc2, uint32_t u4SrcLen2)
{
	dma_addr_t rAddr;
	uint32_t u4TotalLen = u4SrcLen1 + u4SrcLen2;

	prTxCell->pBuffer = pucBuf;

	memcpy(pucBuf, pucSrc1, u4SrcLen1);
	if (pucSrc2 != NULL && u4SrcLen2 > 0)
		memcpy(pucBuf + u4SrcLen1, pucSrc2, u4SrcLen2);
	rAddr = KAL_DMA_MAP_SINGLE(prHifInfo->prDmaDev, pucBuf,
				   u4TotalLen, KAL_DMA_TO_DEVICE);
	if (KAL_DMA_MAPPING_ERROR(prHifInfo->prDmaDev, rAddr)) {
		DBGLOG(HAL, ERROR, "KAL_DMA_MAP_SINGLE() error!\n");
		return false;
	}

	prTxCell->PacketPa = (phys_addr_t)rAddr;

	return true;
}

static bool pcieCopyEvent(struct GL_HIF_INFO *prHifInfo,
			  struct RTMP_DMACB *pRxCell,
			  struct RXD_STRUCT *pRxD,
			  struct RTMP_DMABUF *prDmaBuf,
			  uint8_t *pucDst, uint32_t u4Len)
{
	struct sk_buff *prSkb = NULL;
	void *pRxPacket = NULL;
	dma_addr_t rAddr;

	KAL_DMA_UNMAP_SINGLE(prHifInfo->prDmaDev,
			     (dma_addr_t)prDmaBuf->AllocPa,
			     prDmaBuf->AllocSize, KAL_DMA_FROM_DEVICE);

	pRxPacket = pRxCell->pPacket;
	ASSERT(pRxPacket)

	prSkb = (struct sk_buff *)pRxPacket;
	memcpy(pucDst, (uint8_t *)prSkb->data, u4Len);

	prDmaBuf->AllocVa = ((struct sk_buff *)pRxCell->pPacket)->data;
	rAddr = KAL_DMA_MAP_SINGLE(prHifInfo->prDmaDev, prDmaBuf->AllocVa,
				   prDmaBuf->AllocSize, KAL_DMA_FROM_DEVICE);
	if (KAL_DMA_MAPPING_ERROR(prHifInfo->prDmaDev, rAddr)) {
		DBGLOG(HAL, ERROR, "KAL_DMA_MAP_SINGLE() error!\n");
		return false;
	}
	prDmaBuf->AllocPa = (phys_addr_t)rAddr;
	return true;
}

static bool pcieCopyTxData(struct MSDU_TOKEN_ENTRY *prToken,
			   void *pucSrc, uint32_t u4Len)
{
	memcpy(prToken->prPacket, pucSrc, u4Len);
	return true;
}

static bool pcieCopyRxData(struct GL_HIF_INFO *prHifInfo,
			   struct RTMP_DMACB *pRxCell,
			   struct RTMP_DMABUF *prDmaBuf,
			   struct SW_RFB *prSwRfb)
{
	void *pRxPacket = NULL;
	dma_addr_t rAddr;

	pRxPacket = pRxCell->pPacket;
	ASSERT(pRxPacket);

	pRxCell->pPacket = prSwRfb->pvPacket;

	KAL_DMA_UNMAP_SINGLE(prHifInfo->prDmaDev,
			     (dma_addr_t)prDmaBuf->AllocPa,
			     prDmaBuf->AllocSize, KAL_DMA_FROM_DEVICE);
	prSwRfb->pvPacket = pRxPacket;

	prDmaBuf->AllocVa = ((struct sk_buff *)pRxCell->pPacket)->data;
	rAddr = KAL_DMA_MAP_SINGLE(prHifInfo->prDmaDev,
		prDmaBuf->AllocVa, prDmaBuf->AllocSize, KAL_DMA_FROM_DEVICE);
	if (KAL_DMA_MAPPING_ERROR(prHifInfo->prDmaDev, rAddr)) {
		DBGLOG(HAL, ERROR, "KAL_DMA_MAP_SINGLE() error!\n");
		ASSERT(0);
		return false;
	}
	prDmaBuf->AllocPa = (phys_addr_t)rAddr;

	return true;
}

static phys_addr_t pcieMapTxBuf(struct GL_HIF_INFO *prHifInfo,
			  void *pucBuf, uint32_t u4Offset, uint32_t u4Len)
{
	dma_addr_t rDmaAddr;

	rDmaAddr = KAL_DMA_MAP_SINGLE(prHifInfo->prDmaDev, pucBuf + u4Offset,
				      u4Len, KAL_DMA_TO_DEVICE);
	if (KAL_DMA_MAPPING_ERROR(prHifInfo->prDmaDev, rDmaAddr)) {
		DBGLOG(HAL, ERROR, "KAL_DMA_MAP_SINGLE() error!\n");
		return 0;
	}

	return (phys_addr_t)rDmaAddr;
}

static phys_addr_t pcieMapRxBuf(struct GL_HIF_INFO *prHifInfo,
			  void *pucBuf, uint32_t u4Offset, uint32_t u4Len)
{
	dma_addr_t rDmaAddr;

	rDmaAddr = KAL_DMA_MAP_SINGLE(prHifInfo->prDmaDev, pucBuf + u4Offset,
				      u4Len, KAL_DMA_FROM_DEVICE);
	if (KAL_DMA_MAPPING_ERROR(prHifInfo->prDmaDev, rDmaAddr)) {
		DBGLOG(HAL, ERROR, "KAL_DMA_MAP_SINGLE() error!\n");
		return 0;
	}

	return (phys_addr_t)rDmaAddr;
}

static void pcieUnmapTxBuf(struct GL_HIF_INFO *prHifInfo,
			   phys_addr_t rDmaAddr, uint32_t u4Len)
{
	KAL_DMA_UNMAP_SINGLE(prHifInfo->prDmaDev,
			     (dma_addr_t)rDmaAddr,
			     u4Len, KAL_DMA_TO_DEVICE);
}

static void pcieUnmapRxBuf(struct GL_HIF_INFO *prHifInfo,
			   phys_addr_t rDmaAddr, uint32_t u4Len)
{
	KAL_DMA_UNMAP_SINGLE(prHifInfo->prDmaDev,
			     (dma_addr_t)rDmaAddr,
			     u4Len, KAL_DMA_FROM_DEVICE);
}

static void pcieFreeDesc(struct GL_HIF_INFO *prHifInfo,
			 struct RTMP_DMABUF *prDescRing)
{
	if (prDescRing->AllocVa == NULL)
		return;

	KAL_DMA_FREE_COHERENT(prHifInfo->prDmaDev,
			      prDescRing->AllocSize,
			      prDescRing->AllocVa,
			      (dma_addr_t)prDescRing->AllocPa);
	memset(prDescRing, 0, sizeof(struct RTMP_DMABUF));
}

static void pcieFreeBuf(void *pucSrc, uint32_t u4Len)
{
	kalMemFree(pucSrc, PHY_MEM_TYPE, u4Len);
}

static void pcieFreePacket(struct GL_HIF_INFO *prHifInfo,
			   void *pvPacket, uint32_t u4Num)
{
	kalPacketFree(NULL, pvPacket);
}

static void pcieDumpTx(struct GL_HIF_INFO *prHifInfo,
		       struct RTMP_TX_RING *prTxRing,
		       uint32_t u4Idx, uint32_t u4DumpLen)
{
	struct RTMP_DMACB *prTxCell;
	void *prAddr = NULL;

	prTxCell = &prTxRing->Cell[u4Idx];

	if (prTxCell->prToken)
		prAddr = prTxCell->prToken->prPacket;
	else
		prAddr = prTxCell->pBuffer;

	if (prAddr)
		DBGLOG_MEM32(HAL, INFO, prAddr, u4DumpLen);
}

static void pcieDumpRx(struct GL_HIF_INFO *prHifInfo,
		       struct RTMP_RX_RING *prRxRing,
		       uint32_t u4Idx, uint32_t u4DumpLen)
{
	struct RTMP_DMACB *prRxCell;
	struct RTMP_DMABUF *prDmaBuf;

	prRxCell = &prRxRing->Cell[u4Idx];
	prDmaBuf = &prRxCell->DmaBuf;

	if (!prRxCell->pPacket || !prDmaBuf)
		return;

	pcieUnmapRxBuf(prHifInfo, prDmaBuf->AllocPa, prDmaBuf->AllocSize);

	DBGLOG_MEM32(HAL, INFO, ((struct sk_buff *)prRxCell->pPacket)->data,
		     u4DumpLen);

	prDmaBuf->AllocPa = pcieMapRxBuf(prHifInfo, prDmaBuf->AllocVa,
					0, prDmaBuf->AllocSize);
}

static void pcieAllocMcuEmiMem(struct GL_HIF_INFO *prHifInfo)
{
	struct HIF_MEM *prEmiMem = &prHifInfo->rMcuEmiMem;
	dma_addr_t rAddr;

	prEmiMem->va = KAL_DMA_ALLOC_COHERENT(
		prHifInfo->prDmaDev, MCU_EMI_SIZE, &rAddr);
	prEmiMem->pa = (phys_addr_t)rAddr;

	DBGLOG(HAL, INFO, "McuEmiMemAddr = 0x%x\n", (uint64_t)prEmiMem->pa);
}

static void pcieFreeMcuEmiMem(struct GL_HIF_INFO *prHifInfo)
{
	struct HIF_MEM *prEmiMem = &prHifInfo->rMcuEmiMem;

	KAL_DMA_FREE_COHERENT(prHifInfo->prDmaDev,
			      MCU_EMI_SIZE,
			      prEmiMem->va,
			      (dma_addr_t)prEmiMem->pa);
}

#if CFG_CHIP_RESET_SUPPORT
void kalRemoveProbe(IN struct GLUE_INFO *prGlueInfo)
{
	DBGLOG(INIT, WARN, "[SER][L0] not support...\n");
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief API for set L0.5 reset flag. If flag is TRUE, then it means L0.5 reset
 *        is on-going.
 *
 * \param[in] prAdapter
 * \param[in] fgIsReset
 *
 * \return void
 */
/*----------------------------------------------------------------------------*/
void kalSetWfsysResetFlag(IN struct ADAPTER *prAdapter, IN u_int8_t fgIsReset)
{
	prAdapter->fgIsWfsysReset = fgIsReset;
}
#endif

#if CFG_SUPPORT_PCIE_ASPM
static void pcieSetASPML1SS(struct pci_dev *dev, int i4Enable)
{
	int pos;
	uint32_t u4Reg = 0;

	pos = pci_find_ext_capability(dev, PCI_EXT_CAP_ID_L1PMSS);
	pci_read_config_dword(dev, pos + PCI_L1PMSS_CTR1, &u4Reg);
	u4Reg &= ~PCI_L1PMSS_ENABLE_MASK;
	u4Reg |= i4Enable;
	pci_write_config_dword(dev, pos + PCI_L1PMSS_CTR1, u4Reg);
}
static void pcieSetASPML1(struct pci_dev *dev, int i4Enable)
{
	uint16_t u2Reg = 0;
	int i4Pos = dev->pcie_cap;

	pci_read_config_word(dev, i4Pos + PCI_EXP_LNKCTL, &u2Reg);
	u2Reg &= ~PCI_L1PM_ENABLE_MASK;
	u2Reg |= i4Enable;
	pci_write_config_word(dev, i4Pos + PCI_EXP_LNKCTL, u2Reg);
}
static bool pcieCheckASPML1SS(struct pci_dev *dev, int i4BitMap)
{
	int i4Pos;
	uint32_t u4Reg = 0;

	i4Pos = pci_find_ext_capability(dev, PCI_EXT_CAP_ID_L1PMSS);


	if (!i4Pos) {
		DBGLOG(INIT, INFO, "L1 PM Substate capability is not found!\n");
		return FALSE;
	}
	pci_read_config_dword(dev, i4Pos + PCI_L1PMSS_CAP, &u4Reg);
	if (i4BitMap != 0) {
		if ((i4BitMap & PCI_L1PM_CAP_ASPM_L12) &
				(!(u4Reg & PCI_L1PM_CAP_ASPM_L12))) {
			DBGLOG(INIT, INFO, "not support ASPM L1.2!\n");
			return FALSE;
		}
		if ((i4BitMap & PCI_L1PM_CAP_ASPM_L11) &
				(!(u4Reg & PCI_L1PM_CAP_ASPM_L11))) {
			DBGLOG(INIT, INFO, "not support ASPM L1.1!\n");
			return FALSE;
		}
	}
	return TRUE;
}
bool glBusConfigASPM(struct pci_dev *dev, int i4Enable)
{

	uint32_t u4Reg = 0;
	struct pci_dev *parent = dev->bus->self;
	int pos = parent->pcie_cap;


	pci_read_config_dword(parent, pos + PCI_EXP_LNKCAP, &u4Reg);
	if (PCIE_ASPM_CHECK_L1(u4Reg)) {
		pos = dev->pcie_cap;
		pci_read_config_dword(dev, pos + PCI_EXP_LNKCAP, &u4Reg);
		if (PCIE_ASPM_CHECK_L1(u4Reg)) {
			pcieSetASPML1(parent, i4Enable);
			pcieSetASPML1(dev, i4Enable);
			DBGLOG(INIT, INFO, "ASPM STATUS %d\n", i4Enable);
			return TRUE;
		}
	}
	return FALSE;

}
bool glBusConfigASPML1SS(struct pci_dev *dev, int i4Enable)
{
	struct pci_dev *parent = dev->bus->self;

	if (pcieCheckASPML1SS(parent, i4Enable)) {
		if (pcieCheckASPML1SS(dev, i4Enable)) {
			pcieSetASPML1SS(parent, i4Enable);
			pcieSetASPML1SS(dev, i4Enable);
			DBGLOG(INIT, INFO, "Config ASPM-L1SS\n");
			return TRUE;
		}
	}
	return FALSE;
}

#endif

static void halPciePreSuspendCmd(struct ADAPTER *prAdapter)
{
	struct CMD_HIF_CTRL rCmdHifCtrl;
	uint32_t rStatus;

	rCmdHifCtrl.ucHifType = ENUM_HIF_TYPE_PCIE;
	rCmdHifCtrl.ucHifDirection = ENUM_HIF_TX;
	rCmdHifCtrl.ucHifStop = TRUE;
	rCmdHifCtrl.ucHifSuspend = TRUE;
	rCmdHifCtrl.u4WakeupHifType = ENUM_CMD_HIF_WAKEUP_TYPE_PCIE;

	rStatus = wlanSendSetQueryCmd(prAdapter,	/* prAdapter */
			CMD_ID_HIF_CTRL,	/* ucCID */
			TRUE,	/* fgSetQuery */
			FALSE,	/* fgNeedResp */
			FALSE,	/* fgIsOid */
			NULL,	/* pfCmdDoneHandler */
			NULL,	/* pfCmdTimeoutHandler */
			sizeof(struct CMD_HIF_CTRL), /* u4SetQueryInfoLen */
			(uint8_t *)&rCmdHifCtrl, /* pucInfoBuffer */
			NULL,	/* pvSetQueryBuffer */
			0	/* u4SetQueryBufferLen */
			);

	ASSERT(rStatus == WLAN_STATUS_PENDING);
}

static void halPcieResumeCmd(struct ADAPTER *prAdapter)
{
	struct CMD_HIF_CTRL rCmdHifCtrl;
	uint32_t rStatus;

	rCmdHifCtrl.ucHifType = ENUM_HIF_TYPE_PCIE;
	rCmdHifCtrl.ucHifDirection = ENUM_HIF_TX;
	rCmdHifCtrl.ucHifStop = FALSE;
	rCmdHifCtrl.ucHifSuspend = FALSE;
	rCmdHifCtrl.u4WakeupHifType = ENUM_CMD_HIF_WAKEUP_TYPE_PCIE;

	rStatus = wlanSendSetQueryCmd(prAdapter,	/* prAdapter */
			CMD_ID_HIF_CTRL,	/* ucCID */
			TRUE,	/* fgSetQuery */
			FALSE,	/* fgNeedResp */
			FALSE,	/* fgIsOid */
			NULL,	/* pfCmdDoneHandler */
			NULL,	/* pfCmdTimeoutHandler */
			sizeof(struct CMD_HIF_CTRL), /* u4SetQueryInfoLen */
			(uint8_t *)&rCmdHifCtrl, /* pucInfoBuffer */
			NULL,	/* pvSetQueryBuffer */
			0	/* u4SetQueryBufferLen */
			);

	ASSERT(rStatus == WLAN_STATUS_PENDING);
}

void halPciePreSuspendDone(
	struct ADAPTER *prAdapter,
	struct CMD_INFO *prCmdInfo,
	uint8_t *pucEventBuf)
{
	ASSERT(prAdapter);

	prAdapter->prGlueInfo->rHifInfo.eSuspendtate =
		PCIE_STATE_PRE_SUSPEND_DONE;
}

void halPciePreSuspendTimeout(
	struct ADAPTER *prAdapter,
	struct CMD_INFO *prCmdInfo)
{
	ASSERT(prAdapter);

	prAdapter->prGlueInfo->rHifInfo.eSuspendtate =
		PCIE_STATE_PRE_SUSPEND_FAIL;
}

#if AXI_CFG_PREALLOC_MEMORY_BUFFER
static void axiAllocTxDesc(struct GL_HIF_INFO *prHifInfo,
			   struct RTMP_DMABUF *prDescRing,
			   uint32_t u4Num)
{
	prDescRing->AllocVa = grMem.rTxDesc[u4Num].va;
	prDescRing->AllocPa = grMem.rTxDesc[u4Num].pa;
	if (prDescRing->AllocVa == NULL)
		DBGLOG(HAL, ERROR, "prDescRing->AllocVa is NULL\n");
	else
		memset(prDescRing->AllocVa, 0, prDescRing->AllocSize);
}

static void axiAllocRxDesc(struct GL_HIF_INFO *prHifInfo,
			   struct RTMP_DMABUF *prDescRing,
			   uint32_t u4Num)
{
	prDescRing->AllocVa = grMem.rRxDesc[u4Num].va;
	prDescRing->AllocPa = grMem.rRxDesc[u4Num].pa;
	if (prDescRing->AllocVa == NULL)
		DBGLOG(HAL, ERROR, "prDescRing->AllocVa is NULL\n");
	else
		memset(prDescRing->AllocVa, 0, prDescRing->AllocSize);
}

static bool axiAllocTxCmdBuf(struct RTMP_DMABUF *prDmaBuf,
			     uint32_t u4Num, uint32_t u4Idx)
{
	/* only for cmd & fw download ring */
	if (u4Num == TX_RING_CMD_IDX_3 || u4Num == TX_RING_FWDL_IDX_4) {
		prDmaBuf->AllocSize = AXI_TX_CMD_BUFF_SIZE;
		prDmaBuf->AllocPa = grMem.rTxCmdBuf[u4Idx].pa;
		prDmaBuf->AllocVa = grMem.rTxCmdBuf[u4Idx].va;
		if (prDmaBuf->AllocVa  == NULL) {
			DBGLOG(HAL, ERROR, "prDescRing->AllocVa is NULL\n");
			return false;
		}
		memset(prDmaBuf->AllocVa, 0, prDmaBuf->AllocSize);
	}
	return true;
}

static void axiAllocTxDataBuf(struct MSDU_TOKEN_ENTRY *prToken, uint32_t u4Idx)
{
	prToken->prPacket = grMem.rMsduBuf[u4Idx].va;
	prToken->rDmaAddr = grMem.rMsduBuf[u4Idx].pa;
}

static void *axiAllocRxBuf(struct GL_HIF_INFO *prHifInfo,
			   struct RTMP_DMABUF *prDmaBuf,
			   uint32_t u4Num, uint32_t u4Idx)
{
	switch (u4Num) {
	case RX_RING_DATA_IDX_0:
		prDmaBuf->AllocPa = grMem.rRxDataBuf[u4Idx].pa;
		prDmaBuf->AllocVa = grMem.rRxDataBuf[u4Idx].va;
		break;
	case RX_RING_EVT_IDX_1:
		prDmaBuf->AllocPa = grMem.rRxEventBuf[u4Idx].pa;
		prDmaBuf->AllocVa = grMem.rRxEventBuf[u4Idx].va;
		break;
#if (CFG_SUPPORT_CONNAC2X == 1 || CFG_SUPPORT_CONNAC3X == 1)
	case RX_RING_DATA1_IDX_2:
		prDmaBuf->AllocPa = grMem.rRxData1Buf[u4Idx].pa;
		prDmaBuf->AllocVa = grMem.rRxData1Buf[u4Idx].va;
		break;
	case RX_RING_TXDONE0_IDX_3:
		prDmaBuf->AllocPa = grMem.rTxFreeDoneEvent0Buf[u4Idx].pa;
		prDmaBuf->AllocVa = grMem.rTxFreeDoneEvent0Buf[u4Idx].va;
		break;
	case RX_RING_TXDONE1_IDX_4:
		prDmaBuf->AllocPa = grMem.rTxFreeDoneEvent1Buf[u4Idx].pa;
		prDmaBuf->AllocVa = grMem.rTxFreeDoneEvent1Buf[u4Idx].va;
		break;
	case RX_RING_DATA2_IDX_5:
		prDmaBuf->AllocPa = grMem.rRxData2Buf[u4Idx].pa;
		prDmaBuf->AllocVa = grMem.rRxData2Buf[u4Idx].va;
		break;
	case RX_RING_TXDONE2_IDX_6:
		prDmaBuf->AllocPa = grMem.rTxFreeDoneEvent2Buf[u4Idx].pa;
		prDmaBuf->AllocVa = grMem.rTxFreeDoneEvent2Buf[u4Idx].va;
		break;
#endif
	default:
		DBGLOG(RX, ERROR, "RX alloc fail error number=%d\n", u4Num);
		return prDmaBuf->AllocVa;
	}

	if (prDmaBuf->AllocVa == NULL)
		DBGLOG(HAL, ERROR, "prDmaBuf->AllocVa is NULL\n");
	else
		memset(prDmaBuf->AllocVa, 0, prDmaBuf->AllocSize);

	return prDmaBuf->AllocVa;
}

static bool axiCopyCmd(struct GL_HIF_INFO *prHifInfo,
		       struct RTMP_DMACB *prTxCell, void *pucBuf,
		       void *pucSrc1, uint32_t u4SrcLen1,
		       void *pucSrc2, uint32_t u4SrcLen2)
{
	struct RTMP_DMABUF *prDmaBuf = &prTxCell->DmaBuf;

	memcpy(prDmaBuf->AllocVa, pucSrc1, u4SrcLen1);
	if (pucSrc2 != NULL && u4SrcLen2 > 0)
		memcpy(prDmaBuf->AllocVa + u4SrcLen1, pucSrc2, u4SrcLen2);
	prTxCell->PacketPa = prDmaBuf->AllocPa;

	return true;
}

static bool axiCopyEvent(struct GL_HIF_INFO *prHifInfo,
			 struct RTMP_DMACB *pRxCell,
			 struct RXD_STRUCT *pRxD,
			 struct RTMP_DMABUF *prDmaBuf,
			 uint8_t *pucDst, uint32_t u4Len)
{
	memcpy(pucDst, prDmaBuf->AllocVa, u4Len);

	return true;
}

static bool axiCopyTxData(struct MSDU_TOKEN_ENTRY *prToken,
			  void *pucSrc, uint32_t u4Len)
{
	memcpy(prToken->prPacket, pucSrc, u4Len);

	return true;
}

static bool axiCopyRxData(struct GL_HIF_INFO *prHifInfo,
			  struct RTMP_DMACB *pRxCell,
			  struct RTMP_DMABUF *prDmaBuf,
			  struct SW_RFB *prSwRfb)
{
	struct RXD_STRUCT *pRxD = (struct RXD_STRUCT *)pRxCell->AllocVa;
	struct sk_buff *prSkb = ((struct sk_buff *)prSwRfb->pvPacket);
	uint32_t u4Size = pRxD->SDLen0;

	if (prSkb == NULL) {
		DBGLOG(RX, ERROR, "prSkb == NULL\n");
		return false;
	}

	if (prSkb->data == NULL) {
		DBGLOG(RX, ERROR, "prSkb->data == NULL\n");
		return false;
	}

	if (u4Size > CFG_RX_MAX_PKT_SIZE) {
		DBGLOG(RX, ERROR, "Rx Data too large[%u]\n", u4Size);
		return false;
	}

	memcpy(prSkb->data, prDmaBuf->AllocVa, u4Size);

	return true;
}

static void axiDumpTx(struct GL_HIF_INFO *prHifInfo,
		      struct RTMP_TX_RING *prTxRing,
		      uint32_t u4Idx, uint32_t u4DumpLen)
{
	struct RTMP_DMACB *prTxCell;
	struct RTMP_DMABUF *prDmaBuf;
	void *prAddr = NULL;

	prTxCell = &prTxRing->Cell[u4Idx];
	prDmaBuf = &prTxCell->DmaBuf;

	if (prTxCell->prToken)
		prAddr = prTxCell->prToken->prPacket;
	else if (prDmaBuf->AllocVa)
		prAddr = prDmaBuf->AllocVa;

	if (prAddr)
		DBGLOG_MEM32(HAL, INFO, prAddr, u4DumpLen);
}

static void axiDumpRx(struct GL_HIF_INFO *prHifInfo,
		      struct RTMP_RX_RING *prRxRing,
		      uint32_t u4Idx, uint32_t u4DumpLen)
{
	struct RTMP_DMACB *prRxCell;
	struct RTMP_DMABUF *prDmaBuf;

	prRxCell = &prRxRing->Cell[u4Idx];
	prDmaBuf = &prRxCell->DmaBuf;

	if (prRxCell->pPacket)
		DBGLOG_MEM32(HAL, INFO, prRxCell->pPacket, u4DumpLen);
}
#endif /* AXI_CFG_PREALLOC_MEMORY_BUFFER */

int32_t glBusFunOn(void)
{
	int ret = pci_register_driver(&mtk_pci_driver);

	if (ret)
		return ret;

	if (g_prDev == NULL) {
		pci_unregister_driver(&mtk_pci_driver);
		return -EINVAL;
	}

	return 0;
}

void glBusFunOff(void)
{
	pci_unregister_driver(&mtk_pci_driver);
	g_fgDriverProbed = FALSE;
}

#if (CFG_SUPPORT_HOST_OFFLOAD == 1)
static void *pcieAllocRxBlkBuf(struct GL_HIF_INFO *prHifInfo,
			       struct RTMP_DMABUF *prDmaBuf,
			       uint32_t u4Num, uint32_t u4Idx)
{
	struct sk_buff *prSkb;
	struct RX_CTRL_BLK *prRcb;
	dma_addr_t rAddr;

	if (u4Num != RX_RING_DATA_IDX_0 && u4Num != RX_RING_DATA1_IDX_2)
		return pcieAllocRxBuf(prHifInfo, prDmaBuf, u4Num, u4Idx);

	prRcb = halMawdGetFreeRcbBlk(prHifInfo);
	if (!prRcb) {
		DBGLOG(HAL, ERROR, "no rcb resource\n",
		       prDmaBuf->AllocSize);
		prDmaBuf->AllocPa = 0;
		prDmaBuf->AllocVa = NULL;
		return NULL;
	}
	prSkb = prRcb->prSkb;

	prDmaBuf->AllocVa = (void *)prSkb->data;
	memset(prDmaBuf->AllocVa, 0, prDmaBuf->AllocSize);

	rAddr = KAL_DMA_MAP_SINGLE(prHifInfo->prDmaDev, prDmaBuf->AllocVa,
				   prDmaBuf->AllocSize, KAL_DMA_FROM_DEVICE);
	if (KAL_DMA_MAPPING_ERROR(prHifInfo->prDmaDev, rAddr)) {
		DBGLOG(HAL, ERROR, "sk_buff dma mapping error!\n");
		return NULL;
	}

	prDmaBuf->AllocPa = (phys_addr_t)rAddr;

	halMawdHashAdd(prHifInfo, (uint64_t)rAddr, prSkb, prRcb);

	return (void *)prSkb;
}

static void pcieUnmapRxBlkBuf(struct GL_HIF_INFO *prHifInfo,
			      phys_addr_t rDmaAddr, uint32_t u4Len)
{
	KAL_DMA_UNMAP_SINGLE(prHifInfo->prDmaDev,
			     (dma_addr_t)rDmaAddr,
			     u4Len, KAL_DMA_FROM_DEVICE);

	halMawdHashDel(prHifInfo, (uint64_t)rDmaAddr);
}

static void pcieReleaseRxBlkBuf(struct GL_HIF_INFO *prHifInfo,
				void *pvPacket, uint32_t u4Num)
{
	if (u4Num == RX_RING_DATA_IDX_0 || u4Num == RX_RING_DATA1_IDX_2)
		return;

	pcieFreePacket(prHifInfo, pvPacket, u4Num);
}
#endif /* CFG_SUPPORT_HOST_OFFLOAD == 1 */
