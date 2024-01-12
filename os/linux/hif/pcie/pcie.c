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
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/

#include "gl_os.h"

#include "hif_pci.h"

#include "precomp.h"

#include <linux/mm.h>
#ifndef CONFIG_X86
#include <asm/memory.h>
#endif

#include "mt66xx_reg.h"

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

#define MTK_PCI_VENDOR_ID	0x14C3
#define NIC6632_PCIe_DEVICE_ID	0x6632
#define NIC7668_PCIe_DEVICE_ID	0x7668
#define MT7663_PCI_PFGA2_VENDOR_ID	0x0E8D
#define NIC7663_PCIe_DEVICE_ID	0x7663
#define CONNAC_PCI_VENDOR_ID	0x0E8D
#define CONNAC_PCIe_DEVICE_ID	0x3280

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
	{ /* end: all zeroes */ },
};

MODULE_DEVICE_TABLE(pci, mtk_pci_ids);

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/
static probe_card pfWlanProbe;
static remove_card pfWlanRemove;

static struct pci_driver mtk_pci_driver = {
	.name = "wlan",
	.id_table = mtk_pci_ids,
	.probe = NULL,
	.remove = NULL,
};

static BOOLEAN g_fgDriverProbed = FALSE;
static UINT_32 g_u4DmaMask = 32;
/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/

/*******************************************************************************
*                   F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/
static BOOL kalDevWriteCmdByQueue(IN P_GLUE_INFO_T prGlueInfo, IN P_CMD_INFO_T prCmdInfo, IN UINT_8 ucTC);
static BOOL kalDevWriteDataByQueue(IN P_GLUE_INFO_T prGlueInfo, IN P_MSDU_INFO_T prMsduInfo);

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
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
static PUCHAR CSRBaseAddress;

static irqreturn_t mtk_pci_interrupt(int irq, void *dev_instance)
{
	P_GLUE_INFO_T prGlueInfo = NULL;
	UINT_32 u4RegValue;

	prGlueInfo = (P_GLUE_INFO_T) dev_instance;
	if (!prGlueInfo) {
		DBGLOG(HAL, INFO, "No glue info in mtk_pci_interrupt()\n");
		return IRQ_NONE;
	}

	HAL_MCR_RD(prGlueInfo->prAdapter, WPDMA_INT_STA, &u4RegValue);
	if (!u4RegValue)
		return IRQ_HANDLED;

	halDisableInterrupt(prGlueInfo->prAdapter);

	if (prGlueInfo->ulFlag & GLUE_FLAG_HALT) {
		DBGLOG(HAL, INFO, "GLUE_FLAG_HALT skip INT\n");
		return IRQ_NONE;
	}

	DBGLOG(HAL, TRACE, "%s INT[0x%08x]\n", __func__, u4RegValue);

	kalSetIntEvent(prGlueInfo);

	return IRQ_HANDLED;
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
	int ret = 0;

	ASSERT(pdev);
	ASSERT(id);

	ret = pci_enable_device(pdev);

	if (ret) {
		DBGLOG(INIT, INFO, "pci_enable_device failed!\n");
		goto out;
	}

	DBGLOG(INIT, INFO, "pci_enable_device done!\n");

	if (pfWlanProbe((PVOID) pdev, (PVOID) id->driver_data) != WLAN_STATUS_SUCCESS) {
		DBGLOG(INIT, INFO, "pfWlanProbe fail!call pfWlanRemove()\n");
		pfWlanRemove();
		ret = -1;
	} else {
		struct mt66xx_chip_info *prChipInfo;

		prChipInfo = ((struct mt66xx_hif_driver_data *)id->driver_data)->chip_info;
		g_fgDriverProbed = TRUE;
		g_u4DmaMask = prChipInfo->bus_info->u4DmaMask;
	}
out:
	DBGLOG(INIT, INFO, "mtk_pci_probe() done(%d)\n", ret);

	return ret;
}

static void mtk_pci_remove(struct pci_dev *pdev)
{
	ASSERT(pdev);

	if (g_fgDriverProbed)
		pfWlanRemove();
	DBGLOG(INIT, INFO, "pfWlanRemove done\n");

	/* Unmap CSR base address */
	iounmap(CSRBaseAddress);

	/* release memory region */
	pci_release_regions(pdev);

	pci_disable_device(pdev);
	DBGLOG(INIT, INFO, "mtk_pci_remove() done\n");
}

static int mtk_pci_suspend(struct pci_dev *pdev, pm_message_t state)
{
	return 0;
}

int mtk_pci_resume(struct pci_dev *pdev)
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
WLAN_STATUS glRegisterBus(probe_card pfProbe, remove_card pfRemove)
{
	int ret = 0;

	ASSERT(pfProbe);
	ASSERT(pfRemove);

	/* printk(KERN_INFO "mtk_pci: MediaTek PCIE WLAN driver\n"); */
	/* printk(KERN_INFO "mtk_pci: Copyright MediaTek Inc.\n"); */

	pfWlanProbe = pfProbe;
	pfWlanRemove = pfRemove;

	mtk_pci_driver.probe = mtk_pci_probe;
	mtk_pci_driver.remove = mtk_pci_remove;

	mtk_pci_driver.suspend = mtk_pci_suspend;
	mtk_pci_driver.resume = mtk_pci_resume;

	ret = (pci_register_driver(&mtk_pci_driver) == 0) ? WLAN_STATUS_SUCCESS : WLAN_STATUS_FAILURE;

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
VOID glUnregisterBus(remove_card pfRemove)
{
	if (g_fgDriverProbed) {
		pfRemove();
		g_fgDriverProbed = FALSE;
	}
	pci_unregister_driver(&mtk_pci_driver);
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
VOID glSetHifInfo(P_GLUE_INFO_T prGlueInfo, ULONG ulCookie)
{
	P_GL_HIF_INFO_T prHif = NULL;

	prHif = &prGlueInfo->rHifInfo;

	prHif->pdev = (struct pci_dev *)ulCookie;

	prHif->CSRBaseAddress = CSRBaseAddress;

	pci_set_drvdata(prHif->pdev, prGlueInfo);

	SET_NETDEV_DEV(prGlueInfo->prDevHandler, &prHif->pdev->dev);

	spin_lock_init(&prHif->rDynMapRegLock);

	prGlueInfo->u4InfType = MT_DEV_INF_PCIE;

	prHif->rErrRecoveryCtl.eErrRecovState = ERR_RECOV_STOP_IDLE;
	prHif->rErrRecoveryCtl.u4Status = 0;
	prHif->fgIsErrRecovery = FALSE;
	spin_lock_init(&prHif->rSerLock);

	INIT_LIST_HEAD(&prHif->rTxCmdQ);
	INIT_LIST_HEAD(&prHif->rTxDataQ);
	spin_lock_init(&prHif->rTxCmdQLock);
	spin_lock_init(&prHif->rTxDataQLock);
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
VOID glClearHifInfo(P_GLUE_INFO_T prGlueInfo)
{
	P_GL_HIF_INFO_T prHifInfo = &prGlueInfo->rHifInfo;
	struct list_head *prCur, *prNext;
	struct TX_CMD_REQ_T *prTxCmdReq;
	struct TX_DATA_REQ_T *prTxDataReq;

	halUninitMsduTokenInfo(prGlueInfo->prAdapter);
	halWpdmaFreeRing(prGlueInfo);
	halHifRst(prGlueInfo);

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
* \brief Initialize bus operation and hif related information, request resources.
*
* \param[out] pvData    A pointer to HIF-specific data type buffer.
*                       For eHPI, pvData is a pointer to UINT_32 type and stores a
*                       mapped base address.
*
* \return (none)
*/
/*----------------------------------------------------------------------------*/
BOOL glBusInit(PVOID pvData)
{
	int ret = 0;
	struct pci_dev *pdev = NULL;

	ASSERT(pvData);

	pdev = (struct pci_dev *)pvData;

	ret = pci_set_dma_mask(pdev, DMA_BIT_MASK(g_u4DmaMask));
	if (ret != 0) {
		DBGLOG(INIT, INFO, "set DMA mask failed!errno=%d\n", ret);
		return FALSE;
	}

	ret = pci_request_regions(pdev, pci_name(pdev));
	if (ret != 0) {
		DBGLOG(INIT, INFO, "Request PCI resource failed, errno=%d!\n", ret);
	}

	/* map physical address to virtual address for accessing register */
	CSRBaseAddress = (PUCHAR) ioremap(pci_resource_start(pdev, 0), pci_resource_len(pdev, 0));
	DBGLOG(INIT, INFO, "ioremap for device %s, region 0x%lX @ 0x%lX\n",
		pci_name(pdev), (ULONG) pci_resource_len(pdev, 0), (ULONG) pci_resource_start(pdev, 0));
	if (!CSRBaseAddress) {
		DBGLOG(INIT, INFO, "ioremap failed for device %s, region 0x%lX @ 0x%lX\n",
			pci_name(pdev), (ULONG) pci_resource_len(pdev, 0), (ULONG) pci_resource_start(pdev, 0));
		goto err_out_free_res;
	}

	/* Set DMA master */
	pci_set_master(pdev);

	return TRUE;

err_out_free_res:
	pci_release_regions(pdev);

	pci_disable_device(pdev);

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
VOID glBusRelease(PVOID pvData)
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
INT_32 glBusSetIrq(PVOID pvData, PVOID pfnIsr, PVOID pvCookie)
{
	int ret = 0;

	P_BUS_INFO prBusInfo;
	struct net_device *prNetDevice = NULL;
	P_GLUE_INFO_T prGlueInfo = NULL;
	P_GL_HIF_INFO_T prHifInfo = NULL;
	struct pci_dev *pdev = NULL;

	ASSERT(pvData);
	if (!pvData)
		return -1;

	prNetDevice = (struct net_device *)pvData;
	prGlueInfo = (P_GLUE_INFO_T) pvCookie;
	ASSERT(prGlueInfo);
	if (!prGlueInfo)
		return -1;

	prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;

	prHifInfo = &prGlueInfo->rHifInfo;
	pdev = prHifInfo->pdev;

#if defined(CONFIG_ARCH_MT7623) || defined(CONFIG_ARCH_MT7621)
	ret = request_irq(pdev->irq, mtk_pci_interrupt, IRQF_SHARED |
		IRQF_TRIGGER_FALLING, prNetDevice->name, prGlueInfo);
#else
	ret = request_irq(pdev->irq, mtk_pci_interrupt, IRQF_SHARED, prNetDevice->name, prGlueInfo);
#endif

	if (ret != 0)
		DBGLOG(INIT, INFO, "glBusSetIrq: request_irq  ERROR(%d)\n", ret);
	else if (prBusInfo->fgInitPCIeInt)
		HAL_MCR_WR(prGlueInfo->prAdapter, MT_PCIE_IRQ_ENABLE, 1);

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
VOID glBusFreeIrq(PVOID pvData, PVOID pvCookie)
{
	struct net_device *prNetDevice = NULL;
	P_GLUE_INFO_T prGlueInfo = NULL;
	P_GL_HIF_INFO_T prHifInfo = NULL;
	struct pci_dev *pdev = NULL;

	ASSERT(pvData);
	if (!pvData) {
		DBGLOG(INIT, INFO, "%s null pvData\n", __func__);
		return;
	}
	prNetDevice = (struct net_device *)pvData;
	prGlueInfo = (P_GLUE_INFO_T) pvCookie;
	ASSERT(prGlueInfo);
	if (!prGlueInfo) {
		DBGLOG(INIT, INFO, "%s no glue info\n", __func__);
		return;
	}

	prHifInfo = &prGlueInfo->rHifInfo;
	pdev = prHifInfo->pdev;

	synchronize_irq(pdev->irq);
	free_irq(pdev->irq, prGlueInfo);
}

BOOLEAN glIsReadClearReg(UINT_32 u4Address)
{
	return TRUE;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief Read a 32-bit device register
*
* \param[in] prGlueInfo Pointer to the GLUE_INFO_T structure.
* \param[in] u4Register Register offset
* \param[in] pu4Value   Pointer to variable used to store read value
*
* \retval TRUE          operation success
* \retval FALSE         operation fail
*/
/*----------------------------------------------------------------------------*/
BOOL kalDevRegRead(IN P_GLUE_INFO_T prGlueInfo, IN UINT_32 u4Register, OUT PUINT_32 pu4Value)
{
	P_BUS_INFO prBusInfo = NULL;
	P_GL_HIF_INFO_T prHifInfo = NULL;
	UINT_32 u4BusAddr = u4Register;
	BOOLEAN fgResult = TRUE;

	ASSERT(prGlueInfo);
	ASSERT(pu4Value);

	prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;
	prHifInfo = &prGlueInfo->rHifInfo;

	if (halChipToStaticMapBusAddr(prBusInfo, u4Register, &u4BusAddr)) {
		/* Static mapping */
		RTMP_IO_READ32(prHifInfo, u4BusAddr, pu4Value);
	} else {
		/* Dynamic mapping */
		fgResult = halGetDynamicMapReg(prGlueInfo, u4BusAddr, pu4Value);
	}

	if ((u4Register & 0xFFFFF000) != PCIE_HIF_BASE)
		DBGLOG(HAL, INFO, "Get CR[0x%08x/0x%08x] value[0x%08x]\n", u4Register, u4BusAddr, *pu4Value);

	return TRUE;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief Write a 32-bit device register
*
* \param[in] prGlueInfo Pointer to the GLUE_INFO_T structure.
* \param[in] u4Register Register offset
* \param[in] u4Value    Value to be written
*
* \retval TRUE          operation success
* \retval FALSE         operation fail
*/
/*----------------------------------------------------------------------------*/
BOOL kalDevRegWrite(IN P_GLUE_INFO_T prGlueInfo, IN UINT_32 u4Register, IN UINT_32 u4Value)
{
	P_BUS_INFO prBusInfo = NULL;
	P_GL_HIF_INFO_T prHifInfo = NULL;
	UINT_32 u4BusAddr = u4Register;
	BOOLEAN fgResult = TRUE;

	ASSERT(prGlueInfo);

	prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;
	prHifInfo = &prGlueInfo->rHifInfo;

	if (halChipToStaticMapBusAddr(prBusInfo, u4Register, &u4BusAddr)) {
		/* Static mapping */
		RTMP_IO_WRITE32(prHifInfo, u4BusAddr, u4Value);
	} else {
		/* Dynamic mapping */
		fgResult = halSetDynamicMapReg(prGlueInfo, u4BusAddr, u4Value);
	}

	if ((u4Register & 0xFFFFF000) != PCIE_HIF_BASE)
		DBGLOG(HAL, INFO, "Set CR[0x%08x/0x%08x] value[0x%08x]\n", u4Register, u4BusAddr, u4Value);

	return TRUE;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief Read device I/O port
*
* \param[in] prGlueInfo         Pointer to the GLUE_INFO_T structure.
* \param[in] u2Port             I/O port offset
* \param[in] u2Len              Length to be read
* \param[out] pucBuf            Pointer to read buffer
* \param[in] u2ValidOutBufSize  Length of the buffer valid to be accessed
*
* \retval TRUE          operation success
* \retval FALSE         operation fail
*/
/*----------------------------------------------------------------------------*/
BOOL
kalDevPortRead(IN P_GLUE_INFO_T prGlueInfo, IN UINT_16 u2Port, IN UINT_32 u4Len,
	OUT PUINT_8 pucBuf, IN UINT_32 u4ValidOutBufSize)
{
	P_GL_HIF_INFO_T prHifInfo = NULL;
	PUINT_8 pucDst = NULL;
	RXD_STRUCT *pRxD;
	P_RTMP_RX_RING prRxRing;
	spinlock_t *pRxRingLock;
	PVOID pRxPacket = NULL;
	RTMP_DMACB *pRxCell;
	struct pci_dev *pdev = NULL;
	BOOL fgRet = TRUE;
	ULONG flags = 0;
	PRTMP_DMABUF prDmaBuf;

	ASSERT(prGlueInfo);
	prHifInfo = &prGlueInfo->rHifInfo;

	ASSERT(pucBuf);
	pucDst = pucBuf;

	ASSERT(u4Len <= u4ValidOutBufSize);

	pdev = prHifInfo->pdev;

	prRxRing = &prHifInfo->RxRing[u2Port];
	pRxRingLock = &prHifInfo->RxRingLock[u2Port];

	spin_lock_irqsave(pRxRingLock, flags);

	kalCheckAndResetRXReg(prGlueInfo, u2Port);
	pRxCell = &prRxRing->Cell[prRxRing->RxSwReadIdx];

	/* Point to Rx indexed rx ring descriptor */
	pRxD = (RXD_STRUCT *) pRxCell->AllocVa;

	if (pRxD->DMADONE == 0) {
		/* Get how may packets had been received */
		kalDevRegRead(prGlueInfo, prRxRing->hw_didx_addr, &prRxRing->RxDmaIdx);

		DBGLOG(HAL, TRACE, "Rx DMA done P[%u] DMA[%u] SW_RD[%u]\n", u2Port,
			prRxRing->RxDmaIdx, prRxRing->RxSwReadIdx);
		fgRet = FALSE;
		goto done;
	}

	prDmaBuf = &pRxCell->DmaBuf;

	if (pRxD->LastSec0 == 0 || prRxRing->fgRxSegPkt) {
		/* Rx segmented packet */
		DBGLOG(HAL, WARN, "Skip Rx segmented packet, SDL0[%u] LS0[%u]\n", pRxD->SDLen0, pRxD->LastSec0);
		if (pRxD->LastSec0 == 1) {
			/* Last segmented packet */
			prRxRing->fgRxSegPkt = FALSE;
		} else {
			/* Segmented packet */
			prRxRing->fgRxSegPkt = TRUE;
		}

		fgRet = FALSE;
		goto skip;
	}

	pci_unmap_single(pdev, prDmaBuf->AllocPa, prDmaBuf->AllocSize, PCI_DMA_FROMDEVICE);

	if (pRxD->SDLen0 <= u4Len) {
		pRxPacket = pRxCell->pPacket;
		ASSERT(pRxPacket);
		kalMemCopy(pucDst, ((UCHAR *) ((struct sk_buff *)(pRxPacket))->data), pRxD->SDLen0);
	} else
		DBGLOG(HAL, WARN, "Skip Rx packet, SDL0[%u] > SwRfb max len[%u]\n", pRxD->SDLen0, u4Len);

	prDmaBuf->AllocVa = ((struct sk_buff *)pRxCell->pPacket)->data;
	prDmaBuf->AllocPa = pci_map_single(pdev, prDmaBuf->AllocVa, prDmaBuf->AllocSize, PCI_DMA_FROMDEVICE);
	if (pci_dma_mapping_error(pdev, prDmaBuf->AllocPa)) {
		DBGLOG(HAL, ERROR, "pci_map_single() error!\n");
		spin_unlock_irqrestore(pRxRingLock, flags);
		ASSERT(0);
		return FALSE;
	}

	pRxD->SDPtr0 = prDmaBuf->AllocPa & DMA_LOWER_32BITS_MASK;
	pRxD->SDPtr1 = ((UINT_64)prDmaBuf->AllocPa >> DMA_BITS_OFFSET) & DMA_HIGHER_4BITS_MASK;
skip:
	pRxD->SDLen0 = prRxRing->u4BufSize;
	pRxD->DMADONE = 0;

done:
	prRxRing->RxCpuIdx = prRxRing->RxSwReadIdx;
	kalDevRegWrite(prGlueInfo, prRxRing->hw_cidx_addr, prRxRing->RxCpuIdx);
	INC_RING_INDEX(prRxRing->RxSwReadIdx, prRxRing->u4RingSize);

	spin_unlock_irqrestore(pRxRingLock, flags);

	return fgRet;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief Write device I/O port
*
* \param[in] prGlueInfo         Pointer to the GLUE_INFO_T structure.
* \param[in] u2Port             I/O port offset
* \param[in] u2Len              Length to be write
* \param[in] pucBuf             Pointer to write buffer
* \param[in] u2ValidInBufSize   Length of the buffer valid to be accessed
*
* \retval TRUE          operation success
* \retval FALSE         operation fail
*/
/*----------------------------------------------------------------------------*/
BOOL
kalDevPortWrite(IN P_GLUE_INFO_T prGlueInfo,
		IN UINT_16 u2Port, IN UINT_32 u4Len, IN PUINT_8 pucBuf, IN UINT_32 u4ValidInBufSize)
{
	P_GL_HIF_INFO_T prHifInfo = NULL;
	PUINT_8 pucSrc = NULL;
	UINT_32 u4SrcLen = u4Len;
	ULONG flags = 0;
	UINT_32 SwIdx = 0;
	P_RTMP_TX_RING prTxRing;
	spinlock_t *prTxRingLock;
	TXD_STRUCT *pTxD;
	struct pci_dev *pdev = NULL;

	ASSERT(prGlueInfo);
	prHifInfo = &prGlueInfo->rHifInfo;

	ASSERT(pucBuf);

	ASSERT(u4Len <= u4ValidInBufSize);

	pucSrc = kalMemAlloc(u4SrcLen, PHY_MEM_TYPE);
	ASSERT(pucSrc);

	kalMemCopy(pucSrc, pucBuf, u4SrcLen);

	pdev = prHifInfo->pdev;
	prTxRing = &prHifInfo->TxRing[u2Port];
	prTxRingLock = &prHifInfo->TxRingLock[u2Port];

	spin_lock_irqsave((spinlock_t *)prTxRingLock, flags);

	kalCheckAndResetTXReg(prGlueInfo, u2Port);
	SwIdx = prTxRing->TxCpuIdx;
	pTxD = (TXD_STRUCT *) prTxRing->Cell[SwIdx].AllocVa;

	prTxRing->Cell[SwIdx].pPacket = NULL;
	prTxRing->Cell[SwIdx].pBuffer = pucSrc;
	prTxRing->Cell[SwIdx].PacketPa = pci_map_single(pdev, pucSrc, u4SrcLen, PCI_DMA_TODEVICE);
	if (pci_dma_mapping_error(pdev, prTxRing->Cell[SwIdx].PacketPa)) {
		DBGLOG(HAL, ERROR, "pci_map_single() error!\n");
		kalMemFree(pucSrc, PHY_MEM_TYPE, u4SrcLen);
		spin_unlock_irqrestore((spinlock_t *)prTxRingLock, flags);
		ASSERT(0);
		return FALSE;
	}

	pTxD->LastSec0 = 1;
	pTxD->LastSec1 = 0;
	pTxD->SDLen0 = u4SrcLen;
	pTxD->SDLen1 = 0;
	pTxD->SDPtr0 = prTxRing->Cell[SwIdx].PacketPa & DMA_LOWER_32BITS_MASK;
	pTxD->SDPtr0Ext = ((UINT_64)prTxRing->Cell[SwIdx].PacketPa >> DMA_BITS_OFFSET) & DMA_HIGHER_4BITS_MASK;
	pTxD->SDPtr1 = 0;
	pTxD->Burst = 0;
	pTxD->DMADONE = 0;

	/* Increase TX_CTX_IDX, but write to register later. */
	INC_RING_INDEX(prTxRing->TxCpuIdx, TX_RING_SIZE);

	prTxRing->u4UsedCnt++;

	kalDevRegWrite(prGlueInfo, prTxRing->hw_cidx_addr, prTxRing->TxCpuIdx);

	spin_unlock_irqrestore((spinlock_t *)prTxRingLock, flags);

	return TRUE;
}

VOID kalDevReadIntStatus(IN P_ADAPTER_T prAdapter, OUT PUINT_32 pu4IntStatus)
{
	UINT_32 u4RegValue;
	P_GL_HIF_INFO_T prHifInfo = &prAdapter->prGlueInfo->rHifInfo;

	*pu4IntStatus = 0;

	HAL_MCR_RD(prAdapter, WPDMA_INT_STA, &u4RegValue);

	if (HAL_IS_RX_DONE_INTR(u4RegValue))
		*pu4IntStatus |= WHISR_RX0_DONE_INT;

	if (HAL_IS_TX_DONE_INTR(u4RegValue))
		*pu4IntStatus |= WHISR_TX_DONE_INT;

	if (u4RegValue & CONNAC_MCU_SW_INT)
		*pu4IntStatus |= WHISR_D2H_SW_INT;

	prHifInfo->u4IntStatus = u4RegValue;

	/* clear interrupt */
	HAL_MCR_WR(prAdapter, WPDMA_INT_STA, u4RegValue);

}

BOOL kalDevWriteCmd(IN P_GLUE_INFO_T prGlueInfo, IN P_CMD_INFO_T prCmdInfo, IN UINT_8 ucTC)
{
	P_GL_HIF_INFO_T prHifInfo = NULL;

	ASSERT(prGlueInfo);
	prHifInfo = &prGlueInfo->rHifInfo;

	if (prHifInfo->fgIsErrRecovery)
		return kalDevWriteCmdByQueue(prGlueInfo, prCmdInfo, ucTC);

	return halWpdmaWriteCmd(prGlueInfo, prCmdInfo, ucTC);
}

static BOOL kalDevWriteCmdByQueue(IN P_GLUE_INFO_T prGlueInfo, IN P_CMD_INFO_T prCmdInfo, IN UINT_8 ucTC)
{
	P_GL_HIF_INFO_T prHifInfo = NULL;
	ULONG flags = 0;
	struct TX_CMD_REQ_T *prTxReq;

	ASSERT(prGlueInfo);
	prHifInfo = &prGlueInfo->rHifInfo;

	spin_lock_irqsave(&prHifInfo->rTxCmdQLock, flags);

	prTxReq = kalMemAlloc(sizeof(struct TX_CMD_REQ_T), PHY_MEM_TYPE);
	if (prTxReq == NULL) {
		DBGLOG(HAL, ERROR, "kmalloc() TX_CMD_REQ error\n");
		halWpdmaWriteCmd(prGlueInfo, prCmdInfo, ucTC);
		goto error;
	}
	prTxReq->prCmdInfo = prCmdInfo;
	prTxReq->ucTC = ucTC;
	list_add_tail(&prTxReq->list, &prHifInfo->rTxCmdQ);
error:
	spin_unlock_irqrestore(&prHifInfo->rTxCmdQLock, flags);

	return TRUE;
}

BOOL kalDevKickCmd(IN P_GLUE_INFO_T prGlueInfo)
{
	P_GL_HIF_INFO_T prHifInfo = NULL;
	ULONG flags = 0;
	struct list_head *prCur, *prNext;
	struct TX_CMD_REQ_T *prTxReq;

	ASSERT(prGlueInfo);
	prHifInfo = &prGlueInfo->rHifInfo;

	spin_lock_irqsave(&prHifInfo->rTxCmdQLock, flags);

	list_for_each_safe(prCur, prNext, &prHifInfo->rTxCmdQ) {
		prTxReq = list_entry(prCur, struct TX_CMD_REQ_T, list);
		if (prTxReq->prCmdInfo)
			halWpdmaWriteCmd(prGlueInfo, prTxReq->prCmdInfo, prTxReq->ucTC);
		list_del(prCur);
		kfree(prTxReq);
	}

	spin_unlock_irqrestore(&prHifInfo->rTxCmdQLock, flags);

	return TRUE;
}

BOOL kalDevWriteData(IN P_GLUE_INFO_T prGlueInfo, IN P_MSDU_INFO_T prMsduInfo)
{
	P_GL_HIF_INFO_T prHifInfo = NULL;
	P_BUS_INFO prBusInfo = NULL;

	ASSERT(prGlueInfo);
	prHifInfo = &prGlueInfo->rHifInfo;
	prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;

	if (prHifInfo->fgIsErrRecovery)
		return kalDevWriteDataByQueue(prGlueInfo, prMsduInfo);

	return halWpdmaWriteData(prGlueInfo, prMsduInfo);
}

static BOOL kalDevWriteDataByQueue(IN P_GLUE_INFO_T prGlueInfo, IN P_MSDU_INFO_T prMsduInfo)
{
	P_GL_HIF_INFO_T prHifInfo = NULL;
	ULONG flags = 0;
	struct TX_DATA_REQ_T *prTxReq;

	ASSERT(prGlueInfo);
	prHifInfo = &prGlueInfo->rHifInfo;

	spin_lock_irqsave(&prHifInfo->rTxDataQLock, flags);

	prTxReq = kalMemAlloc(sizeof(struct TX_DATA_REQ_T), PHY_MEM_TYPE);
	if (prTxReq == NULL) {
		DBGLOG(HAL, ERROR, "kmalloc() TX_DATA_REQ error\n");
		halWpdmaWriteData(prGlueInfo, prMsduInfo);
		goto error;
	}
	prTxReq->prMsduInfo = prMsduInfo;
	list_add_tail(&prTxReq->list, &prHifInfo->rTxDataQ);

error:
	spin_unlock_irqrestore(&prHifInfo->rTxDataQLock, flags);
	return TRUE;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief Kick Tx data to device
*
* \param[in] prGlueInfo         Pointer to the GLUE_INFO_T structure.
*
* \retval TRUE          operation success
* \retval FALSE         operation fail
*/
/*----------------------------------------------------------------------------*/
BOOL kalDevKickData(IN P_GLUE_INFO_T prGlueInfo)
{
	P_GL_HIF_INFO_T prHifInfo = NULL;
	ULONG flags = 0;
	struct list_head *prCur, *prNext;
	struct TX_DATA_REQ_T *prTxReq;

	ASSERT(prGlueInfo);
	prHifInfo = &prGlueInfo->rHifInfo;

	spin_lock_irqsave(&prHifInfo->rTxDataQLock, flags);

	list_for_each_safe(prCur, prNext, &prHifInfo->rTxDataQ) {
		prTxReq = list_entry(prCur, struct TX_DATA_REQ_T, list);
		if (prTxReq->prMsduInfo)
			halWpdmaWriteData(prGlueInfo, prTxReq->prMsduInfo);
		list_del(prCur);
		kfree(prTxReq);
	}

	spin_unlock_irqrestore(&prHifInfo->rTxDataQLock, flags);

	return TRUE;
}

BOOL kalDevReadData(IN P_GLUE_INFO_T prGlueInfo, IN UINT_16 u2Port, IN OUT P_SW_RFB_T prSwRfb)
{
	P_GL_HIF_INFO_T prHifInfo = NULL;
	RXD_STRUCT *pRxD;
	P_RTMP_RX_RING prRxRing;
	spinlock_t *pRxRingLock;
	PVOID pRxPacket = NULL;
	RTMP_DMACB *pRxCell;
	struct pci_dev *pdev = NULL;
	BOOL fgRet = TRUE;
	ULONG flags = 0;
	PRTMP_DMABUF prDmaBuf;

	ASSERT(prGlueInfo);
	prHifInfo = &prGlueInfo->rHifInfo;

	pdev = prHifInfo->pdev;

	prRxRing = &prHifInfo->RxRing[u2Port];
	pRxRingLock = &prHifInfo->RxRingLock[u2Port];

	spin_lock_irqsave(pRxRingLock, flags);

	kalCheckAndResetRXReg(prGlueInfo, u2Port);
	pRxCell = &prRxRing->Cell[prRxRing->RxSwReadIdx];

	/* Point to Rx indexed rx ring descriptor */
	pRxD = (RXD_STRUCT *) pRxCell->AllocVa;

	if (pRxD->DMADONE == 0) {
		/* Get how may packets had been received */
		kalDevRegRead(prGlueInfo, prRxRing->hw_didx_addr, &prRxRing->RxDmaIdx);

		DBGLOG(HAL, TRACE, "Rx DMA done P[%u] DMA[%u] SW_RD[%u]\n", u2Port,
			prRxRing->RxDmaIdx, prRxRing->RxSwReadIdx);
		fgRet = FALSE;
		goto done;
	}

	if (pRxD->LastSec0 == 0 || prRxRing->fgRxSegPkt) {
		/* Rx segmented packet */
		DBGLOG(HAL, WARN, "Skip Rx segmented data packet, SDL0[%u] LS0[%u]\n",
		       pRxD->SDLen0, pRxD->LastSec0);
		if (pRxD->LastSec0 == 1) {
			/* Last segmented packet */
			prRxRing->fgRxSegPkt = FALSE;
		} else {
			/* Segmented packet */
			prRxRing->fgRxSegPkt = TRUE;
		}

		fgRet = FALSE;
		goto skip;
	}

	pRxPacket = pRxCell->pPacket;
	ASSERT(pRxPacket);

	prDmaBuf = &pRxCell->DmaBuf;

	pRxCell->pPacket = prSwRfb->pvPacket;

	pci_unmap_single(pdev, prDmaBuf->AllocPa, prDmaBuf->AllocSize, PCI_DMA_FROMDEVICE);
	prSwRfb->pvPacket = pRxPacket;
	prSwRfb->pucRecvBuff = ((struct sk_buff *)pRxPacket)->data;
	prSwRfb->prRxStatus = (P_HW_MAC_RX_DESC_T)prSwRfb->pucRecvBuff;

#if CFG_TCP_IP_CHKSUM_OFFLOAD
	prSwRfb->u4TcpUdpIpCksStatus = pRxD->RXINFO;
	DBGLOG(RX, TRACE, "u4TcpUdpIpCksStatus[0x%02x]\n", prSwRfb->u4TcpUdpIpCksStatus);
#endif /* CFG_TCP_IP_CHKSUM_OFFLOAD */

	prDmaBuf->AllocVa = ((struct sk_buff *)pRxCell->pPacket)->data;
	prDmaBuf->AllocPa = pci_map_single(pdev, prDmaBuf->AllocVa, prDmaBuf->AllocSize, PCI_DMA_FROMDEVICE);
	if (pci_dma_mapping_error(pdev, prDmaBuf->AllocPa)) {
		DBGLOG(HAL, ERROR, "pci_map_single() error!\n");
		spin_unlock_irqrestore(pRxRingLock, flags);
		ASSERT(0);
		return FALSE;
	}

	pRxD->SDPtr0 = prDmaBuf->AllocPa & DMA_LOWER_32BITS_MASK;
	pRxD->SDPtr1 = ((UINT_64)prDmaBuf->AllocPa >> DMA_BITS_OFFSET) & DMA_HIGHER_4BITS_MASK;
skip:
	pRxD->SDLen0 = prRxRing->u4BufSize;
	pRxD->DMADONE = 0;

done:
	prRxRing->RxCpuIdx = prRxRing->RxSwReadIdx;
	kalDevRegWrite(prGlueInfo, prRxRing->hw_cidx_addr, prRxRing->RxCpuIdx);
	INC_RING_INDEX(prRxRing->RxSwReadIdx, prRxRing->u4RingSize);

	spin_unlock_irqrestore(pRxRingLock, flags);

	return fgRet;
}


VOID kalPciUnmapToDev(IN P_GLUE_INFO_T prGlueInfo, IN dma_addr_t rDmaAddr, IN UINT_32 u4Length)
{
	P_GL_HIF_INFO_T prHifInfo = NULL;
	struct pci_dev *pdev = NULL;

	prHifInfo = &prGlueInfo->rHifInfo;
	pdev = prHifInfo->pdev;
	pci_unmap_single(pdev, rDmaAddr, u4Length, PCI_DMA_TODEVICE);
}

/*----------------------------------------------------------------------------*/
/*!
* \brief Get current CONNSYS TX ring relative register's value, make sure
*        the value matches our AP side's value (register may get reset in sleep mode)
*
* \param[in] prGlueInfo         Pointer to the GLUE_INFO_T structure.
* \param[in] u2Port             TX RING INDEX
*
*/
/*----------------------------------------------------------------------------*/
VOID kalCheckAndResetTXReg(IN P_GLUE_INFO_T prGlueInfo, IN UINT_16 u2Port)
{
	P_GL_HIF_INFO_T prHifInfo = NULL;
	P_RTMP_TX_RING prTxRing = NULL;
	UINT_32 u4CpuIdx = 0;

	ASSERT(prGlueInfo);
	prHifInfo = &prGlueInfo->rHifInfo;
	prTxRing = &prHifInfo->TxRing[u2Port];

	kalDevRegRead(prGlueInfo, prTxRing->hw_cidx_addr, &u4CpuIdx);
	DBGLOG(HAL, TRACE, "prTxRing->TxCpuIdx = %d, u4CpuIdx = %d\n", prTxRing->TxCpuIdx, u4CpuIdx);

	/* Reset PDMA in FW deep sleep mode */
	if (prTxRing->TxCpuIdx != u4CpuIdx)
		halWpdmaInitTxRing(prGlueInfo);
}

/*----------------------------------------------------------------------------*/
/*!
* \brief Get current CONNSYS RX ring relative register's value, make sure
*        the value matches our AP side's value (register may get reset in sleep mode)
*
* \param[in] prGlueInfo         Pointer to the GLUE_INFO_T structure.
* \param[in] u2Port             RX RING INDEX
*
*/
/*----------------------------------------------------------------------------*/
VOID kalCheckAndResetRXReg(IN P_GLUE_INFO_T prGlueInfo, IN UINT_16 u2Port)
{
	P_GL_HIF_INFO_T prHifInfo = NULL;
	P_RTMP_RX_RING prRxRing = NULL;
	UINT_32 u4CpuIdx = 0;

	ASSERT(prGlueInfo);
	prHifInfo = &prGlueInfo->rHifInfo;
	prRxRing = &prHifInfo->RxRing[u2Port];

	kalDevRegRead(prGlueInfo, prRxRing->hw_cidx_addr, &u4CpuIdx);
	DBGLOG(HAL, TRACE, "prRxRing->RxCpuIdx = %d, u4CpuIdx = %d\n", prRxRing->RxCpuIdx, u4CpuIdx);

	/* Reset PDMA in FW deep sleep mode */
	if (prRxRing->RxCpuIdx != u4CpuIdx)
		halWpdmaInitRxRing(prGlueInfo);
}

VOID glSetPowerState(IN P_GLUE_INFO_T prGlueInfo, IN UINT_32 ePowerMode)
{
}

void glGetDev(PVOID ctx, struct device **dev)
{
	*dev = &((struct pci_dev *)ctx)->dev;
}

void glGetHifDev(P_GL_HIF_INFO_T prHif, struct device **dev)
{
	*dev = &(prHif->pdev->dev);
}
