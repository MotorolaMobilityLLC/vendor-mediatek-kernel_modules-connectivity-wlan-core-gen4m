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

static struct pci_driver mtk_pci_driver = {
	.name = "wlan",
	.id_table = mtk_pci_ids,
	.probe = NULL,
	.remove = NULL,
};

static u_int8_t g_fgDriverProbed = FALSE;
static uint32_t g_u4DmaMask = 32;
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

/*----------------------------------------------------------------------------*/
/*!
 * \brief This function is a PCIE interrupt callback function
 *
 * \param[in] func  pointer to PCIE handle
 *
 * \return void
 */
/*----------------------------------------------------------------------------*/
static uint8_t *CSRBaseAddress;

static irqreturn_t mtk_pci_interrupt(int irq, void *dev_instance)
{
	struct GLUE_INFO *prGlueInfo = NULL;
	uint32_t u4RegValue;

	prGlueInfo = (struct GLUE_INFO *) dev_instance;
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

	if (pfWlanProbe((void *) pdev,
		(void *) id->driver_data) != WLAN_STATUS_SUCCESS) {
		DBGLOG(INIT, INFO, "pfWlanProbe fail!call pfWlanRemove()\n");
		pfWlanRemove();
		ret = -1;
	} else {
		struct mt66xx_chip_info *prChipInfo;

		prChipInfo = ((struct mt66xx_hif_driver_data *)
			id->driver_data)->chip_info;
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

	ret = (pci_register_driver(&mtk_pci_driver) == 0) ?
		WLAN_STATUS_SUCCESS : WLAN_STATUS_FAILURE;

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
void glSetHifInfo(struct GLUE_INFO *prGlueInfo, unsigned long ulCookie)
{
	struct GL_HIF_INFO *prHif = NULL;

	prHif = &prGlueInfo->rHifInfo;

	prHif->pdev = (struct pci_dev *)ulCookie;
	prHif->prDmaDev = prHif->pdev;

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
void glClearHifInfo(struct GLUE_INFO *prGlueInfo)
{
	struct GL_HIF_INFO *prHifInfo = &prGlueInfo->rHifInfo;
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
		DBGLOG(INIT, INFO,
			"Request PCI resource failed, errno=%d!\n", ret);
	}

	/* map physical address to virtual address for accessing register */
	CSRBaseAddress = (uint8_t *) ioremap(pci_resource_start(pdev, 0),
		pci_resource_len(pdev, 0));
	DBGLOG(INIT, INFO, "ioremap for device %s, region 0x%lX @ 0x%lX\n",
		pci_name(pdev), (unsigned long) pci_resource_len(pdev, 0),
		(unsigned long) pci_resource_start(pdev, 0));
	if (!CSRBaseAddress) {
		DBGLOG(INIT, INFO,
			"ioremap failed for device %s, region 0x%lX @ 0x%lX\n",
			pci_name(pdev),
			(unsigned long) pci_resource_len(pdev, 0),
			(unsigned long) pci_resource_start(pdev, 0));
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
	struct BUS_INFO *prBusInfo;
	struct net_device *prNetDevice = NULL;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct pci_dev *pdev = NULL;
	int ret = 0;

	ASSERT(pvData);
	if (!pvData)
		return -1;

	prNetDevice = (struct net_device *)pvData;
	prGlueInfo = (struct GLUE_INFO *)pvCookie;
	ASSERT(prGlueInfo);
	if (!prGlueInfo)
		return -1;

	prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;

	prHifInfo = &prGlueInfo->rHifInfo;
	pdev = prHifInfo->pdev;

	ret = request_irq(pdev->irq, mtk_pci_interrupt,
		IRQF_SHARED, prNetDevice->name, prGlueInfo);
	if (ret != 0)
		DBGLOG(INIT, INFO,
			"glBusSetIrq: request_irq  ERROR(%d)\n", ret);
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
void glBusFreeIrq(void *pvData, void *pvCookie)
{
	struct net_device *prNetDevice = NULL;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct pci_dev *pdev = NULL;

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

	synchronize_irq(pdev->irq);
	free_irq(pdev->irq, prGlueInfo);
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
