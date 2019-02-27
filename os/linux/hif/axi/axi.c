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
********************************************************************************
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

static uint8_t *CSRBaseAddress;
static u_int8_t g_fgDriverProbed = FALSE;
static uint32_t g_u4DmaMask = 32;

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/

/*******************************************************************************
*                   F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

static int hifAxiProbe(void)
{
	struct mt66xx_chip_info *prChipInfo;
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
			(void *)prPlatDev->id_entry->driver_data) != WLAN_STATUS_SUCCESS) {
		DBGLOG(INIT, INFO, "pfWlanProbe fail!call pfWlanRemove()\n");
		pfWlanRemove();
		ret = -1;
		goto out;
	}

	prChipInfo = ((struct mt66xx_hif_driver_data *)mtk_axi_ids[0].driver_data)->chip_info;
	g_fgDriverProbed = TRUE;
	g_u4DmaMask = prChipInfo->bus_info->u4DmaMask;
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
	uint32_t u4RegValue;

	prGlueInfo = (struct GLUE_INFO *)dev_instance;
	if (!prGlueInfo) {
		DBGLOG(HAL, INFO, "No glue info in mtk_axi_interrupt()\n");
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

	DBGLOG(HAL, INFO, "%s INT[0x%08x]\n", __func__, u4RegValue);

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

	rWmtCb.wlan_probe_cb = hifAxiProbe;
	rWmtCb.wlan_remove_cb = hifAxiRemove;
	mtk_wcn_wmt_wlan_reg(&rWmtCb);
#else
	hifAxiProbe();
#endif
	DBGLOG(INIT, INFO, "mtk_axi_probe() done\n");

	return 0;
}

static int mtk_axi_remove(IN struct platform_device *pdev)
{
#if !CFG_MTK_ANDROID_WMT
	hifAxiRemove();
#endif
	return 0;
}

static int mtk_axi_suspend(IN struct platform_device *pdev, IN pm_message_t state)
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

	ret = (platform_driver_register(&mtk_axi_driver) == 0) ? WLAN_STATUS_SUCCESS : WLAN_STATUS_FAILURE;
	DBGLOG(INIT, INFO, "platform_driver_register ret = %d\n", ret);
	DBGLOG(INIT, INFO, "bus_type = %s\n", mtk_axi_driver.driver.bus->name);
	if (!ret)
		ret = ((prPlatDev = platform_device_alloc("CONNAC", -1)) != NULL)
			? WLAN_STATUS_SUCCESS : WLAN_STATUS_FAILURE;

	DBGLOG(INIT, INFO, "platform_device_alloc ret = %d\n", ret);
	ret = (platform_device_add(prPlatDev) == 0) ? WLAN_STATUS_SUCCESS : WLAN_STATUS_FAILURE;
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
* \brief Initialize bus operation and hif related information, request resources.
*
* \param[out] pvData    A pointer to HIF-specific data type buffer.
*                       For eHPI, pvData is a pointer to UINT_32 type and stores a
*                       mapped base address.
*
* \return (none)
*/
/*----------------------------------------------------------------------------*/
u_int8_t glBusInit(void *pvData)
{
	struct platform_device *pdev = NULL;
	u64 required_mask;
	u64 dma_mask = DMA_BIT_MASK(g_u4DmaMask);
	int ret = 0;

	ASSERT(pvData);

	pdev = (struct platform_device *)pvData;
	required_mask = dma_get_required_mask(&pdev->dev);
	DBGLOG(INIT, INFO, "pdev = %x, name = %s, required_mask = %ld, sizeof(dma_addr_t) = %d\n",
	       pdev, pdev->id_entry->name, required_mask, sizeof(dma_addr_t));
	pdev->dev.dma_mask = &dma_mask;

	ret = dma_set_mask(&pdev->dev, dma_mask);
	if (ret != 0)
		DBGLOG(INIT, INFO, "set DMA mask failed!errno=%d\n", ret);

	request_mem_region(axi_resource_start(pdev, 0), axi_resource_len(pdev, 0), axi_name(pdev));

	/* map physical address to virtual address for accessing register */
	CSRBaseAddress = (uint8_t *)ioremap_nocache(axi_resource_start(pdev, 0), axi_resource_len(pdev, 0));
	DBGLOG(INIT, INFO, "CSRBaseAddress:0x%lX\n", CSRBaseAddress);
	DBGLOG(INIT, INFO, "ioremap for device %s, region 0x%lX @ 0x%lX\n",
		axi_name(pdev), (unsigned long)axi_resource_len(pdev, 0),
	       (unsigned long)axi_resource_start(pdev, 0));
	if (!CSRBaseAddress) {
		DBGLOG(INIT, INFO, "ioremap failed for device %s, region 0x%lX @ 0x%lX\n",
			axi_name(pdev), (unsigned long)axi_resource_len(pdev, 0),
		       (unsigned long)axi_resource_start(pdev, 0));
		goto err_out_free_res;
	}

	return TRUE;

err_out_free_res:
	release_mem_region(axi_resource_start(pdev, 0), axi_resource_len(pdev, 0));

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
	struct device_node *node = NULL;
	struct net_device *prNetDevice = NULL;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct platform_device *pdev = NULL;
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

	DBGLOG(INIT, INFO, "glBusSetIrq: request_irq  STATUS(%d)\n", ret);
	node = of_find_compatible_node(NULL, NULL, "mediatek,wifi");
	if (node) {
		prHifInfo->u4IrqId = irq_of_parse_and_map(node, 0);
		DBGLOG(INIT, INFO, "WIFI-OF: get wifi irq(%d)\n", prHifInfo->u4IrqId);
	} else {
		prHifInfo->u4IrqId = AXI_WLAN_IRQ_NUMBER;
		DBGLOG(INIT, ERROR, "WIFI-OF: get wifi device node fail\n");
	}

	ret = request_irq(prHifInfo->u4IrqId, mtk_axi_interrupt, IRQF_SHARED, prNetDevice->name, prGlueInfo);
	DBGLOG(INIT, INFO, "glBusSetIrq: request_irq  STATUS(%d)\n", ret);
	if (ret != 0)
		DBGLOG(INIT, INFO, "glBusSetIrq: request_irq  ERROR(%d)\n", ret);

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
