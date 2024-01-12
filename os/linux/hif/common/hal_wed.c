// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

/*! \file   "hal_wed.c"
 *  \brief
 *
 *
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
#include "precomp.h"
#include "hif_pdma.h"
#include "mt66xx_reg.h"
#include "gl_hook_api.h"
#include <linux/mm.h>
#ifndef CONFIG_X86
#include <asm/memory.h>
#endif

#include "coda/mt6639/wf_wfdma_host_dma0.h"
#include "nic_uni_cmd_event.h"

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

static struct WED_INFO grWedInfo;
static struct dma_token_que grWedToken;
static struct net_device *grNetList[MAX_BSSID_NUM];
uint32_t g_u4SuspendCnt;
uint32_t g_u4ResumeCnt;

/* chip dependency */
static uint32_t mt6639_wed_mirror_table[] = {
	WIFI_INT_MSK,
	WIFI_INT_STA,
	WIFI_TX_RING0_BASE,
	WIFI_TX_RING0_CNT,
	WIFI_TX_RING0_CIDX,
	WIFI_TX_RING0_DIDX,
	WIFI_TX_RING1_BASE,
	WIFI_TX_RING1_CNT,
	WIFI_TX_RING1_CIDX,
	WIFI_TX_RING1_DIDX,
	WIFI_RX_RING4_BASE,
	WIFI_RX_RING4_CNT,
	WIFI_RX_RING4_CIDX,
	WIFI_RX_RING4_DIDX,
	WIFI_RX_RING5_BASE,
	WIFI_RX_RING5_CNT,
	WIFI_RX_RING5_CIDX,
	WIFI_RX_RING5_DIDX,
	WIFI_RX_RING6_BASE,
	WIFI_RX_RING6_CNT,
	WIFI_RX_RING6_CIDX,
	WIFI_RX_RING6_DIDX,
	0x0
};

/*******************************************************************************
 *                                 M A C R O S
 *******************************************************************************
 */

/*******************************************************************************
 *                  F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */

void wedProxyIoRead(struct GLUE_INFO *prGlueInfo,
	uint32_t u4BusAddr, uint32_t *pu4Value)
{
	struct mt66xx_chip_info *prChipInfo = NULL;
	struct WED_CR_ACCESS rWedCr;

	glGetChipInfo((void **)&prChipInfo);
	if (!prChipInfo) {
		DBGLOG(HAL, WARN, "Chip info get fail\n");
		return;
	}

	/* Check mirror first */
	if (wedMirrorAddrCheck(u4BusAddr)) {
		rWedCr.reg_addr = u4BusAddr;
		rWedCr.reg_val = 0;
		wedProxyHookCall(PROXY_WLAN_HOOK_READ, &rWedCr);
		*pu4Value = rWedCr.reg_val;
	} else {
		RTMP_IO_READ32(prChipInfo, u4BusAddr, pu4Value);
	}
}

void wedProxyIoWrite(struct GLUE_INFO *prGlueInfo,
	uint32_t u4BusAddr, uint32_t u4Value)
{
	struct mt66xx_chip_info *prChipInfo = NULL;
	struct WED_CR_ACCESS rWedCr;

	glGetChipInfo((void **)&prChipInfo);
	if (!prChipInfo) {
		DBGLOG(HAL, WARN, "Chip info get fail\n");
		return;
	}

	/* Check mirror first */
	if (wedMirrorAddrCheck(u4BusAddr)) {
		rWedCr.reg_addr = u4BusAddr;
		rWedCr.reg_val = u4Value;
		wedProxyHookCall(PROXY_WLAN_HOOK_WRITE, &rWedCr);
	} else {
		RTMP_IO_WRITE32(prChipInfo, u4BusAddr, u4Value);
	}
}

int wedInitial(struct ADAPTER *prAdapter)
{
	if (prAdapter->prWedInfo != NULL) {
		DBGLOG(HAL, WARN, "WED already initialized before\n");
		return -1;
	}
	memset(&grWedInfo, 0, sizeof(struct WED_INFO));
	prAdapter->prWedInfo = &grWedInfo;
	grWedInfo.pAdAdapter = prAdapter;
	grWedInfo.prGlueInfo = prAdapter->prGlueInfo;

	DBGLOG(HAL, STATE, "WED info initialized\n");
	return 0;
}

void wedInterruptSwap(int irq, void *dev_instance)
{
	struct GLUE_INFO *prGlueInfo = NULL;
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct pci_dev *pdev = NULL;

	prGlueInfo = (struct GLUE_INFO *) dev_instance;
	prHifInfo = &prGlueInfo->rHifInfo;
	pdev = prHifInfo->pdev;

	/* Notify wifi host to handle the irq belongs to WED/PCIE */
	prHifInfo->u4IrqId = irq;
	pdev->irq = irq;
}

/* chip dependency */
int wedInfoSetup(struct ADAPTER *prAdapter)
{
	struct WED_INFO *prwedinfo;
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct mt66xx_chip_info *prChipInfo = NULL;
	struct BUS_INFO *prBusInfo = NULL;
	struct pcie_msi_info *prMsiInfo = NULL;

	prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	glGetChipInfo((void **)&prChipInfo);
	if (!prChipInfo) {
		DBGLOG(HAL, WARN, "Chip info get fail\n");
		return -1;
	}

	prwedinfo = &grWedInfo;
	prBusInfo = prAdapter->chip_info->bus_info;
	prMsiInfo = &prBusInfo->pcie_msi_info;

	prwedinfo->prGlueInfo = prAdapter->prGlueInfo;
	prwedinfo->pAdAdapter = prAdapter;
	prwedinfo->ChipID = prChipInfo->chip_id;
	prwedinfo->infType = BUS_TYPE_PCIE;
	prwedinfo->u4IrqId = prHifInfo->u4IrqId;
	prwedinfo->fgMirrorEnable = FALSE;
	prwedinfo->macVer = MAC_TYPE_BMAC; /* MAC type to be parsed by WO-CPU */
	prwedinfo->fgMsiEnable = (prMsiInfo && prMsiInfo->fgMsiEnabled) ?
		true : false;
	prwedinfo->pcie_msi_msg_addr_lo = prMsiInfo->address_lo;
	prwedinfo->pcie_msi_msg_addr_hi = prMsiInfo->address_hi;
	prwedinfo->whnat_en = TRUE;
	prwedinfo->wed_idx = 0;

	prwedinfo->pci_dev = prChipInfo->pdev;
	prwedinfo->base_addr = (unsigned long)prChipInfo->CSRBaseAddress;

	prwedinfo->dma_offset = WPDMA_OFFSET;
	prwedinfo->int_sta = WIFI_INT_STA;
	prwedinfo->int_mask = WIFI_INT_MSK;

	/* trigger WED's interrupt enable config */
	kalDevRegRead(prAdapter->prGlueInfo,
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_ADDR,
		&prwedinfo->int_enable_mask);

	DBGLOG(HAL, STATE, "prwedinfo->int_enable_mask is 0x%x\n",
		prwedinfo->int_enable_mask);
	/* error recoverying(pending for now) */
	prwedinfo->int_ser = 0;
	prwedinfo->int_ser_value = 0;

	prwedinfo->tx_dma_glo_cfg = WIFI_HOST_DMA0_WPDMA_GLO_CFG;
	prwedinfo->rx_dma_glo_cfg = WIFI_HOST_DMA0_WPDMA_GLO_CFG;
	prwedinfo->txd_size = TXD_SIZE;
	prwedinfo->rxd_size = MAX_RXD_SIZE;
	prwedinfo->tx_pkt_size = CFG_TX_MAX_PKT_SIZE;
	prwedinfo->rx_pkt_size = CFG_RX_MAX_PKT_SIZE;
	prwedinfo->tx_ring_size = TX_RING_SIZE;
	prwedinfo->rx_ring_size = RX_RING_MAX_SIZE;
	prwedinfo->tx_token_nums = HIF_TX_MSDU_TOKEN_NUM; /* 0x2F00 */

	prwedinfo->ring_offset = WIFI_RING_OFFSET;
	prwedinfo->fbuf_size = 128;

	/* Tx Ring, Ring0 for TxData(band0), Ring1 for TxData(band1) */
	prwedinfo->tx_ring[0].base = WIFI_TX_RING0_BASE; /* 4300 */
	prwedinfo->tx_ring[0].cnt = WIFI_TX_RING0_CNT;	 /* 4304 */
	prwedinfo->tx_ring[0].cidx = WIFI_TX_RING0_CIDX; /* 4308 */
	prwedinfo->tx_ring[0].didx = WIFI_TX_RING0_DIDX; /* 430C */
	prwedinfo->tx_ring[1].base = WIFI_TX_RING1_BASE; /* 4310 */
	prwedinfo->tx_ring[1].cnt = WIFI_TX_RING1_CNT;	 /* 4314 */
	prwedinfo->tx_ring[1].cidx = WIFI_TX_RING1_CIDX; /* 4318 */
	prwedinfo->tx_ring[1].didx = WIFI_TX_RING1_DIDX; /* 431C */

	/* Rx Ring, Ring4 for RxData(band0) and Ring5 for RxData(band1) */
	prwedinfo->rx_ring[0].base = WIFI_RX_RING4_BASE; /* 4540 */
	prwedinfo->rx_ring[0].cnt = WIFI_RX_RING4_CNT;	 /* 4544 */
	prwedinfo->rx_ring[0].cidx = WIFI_RX_RING4_CIDX; /* 4548 */
	prwedinfo->rx_ring[0].didx = WIFI_RX_RING4_DIDX; /* 454C */
	prwedinfo->rx_ring[1].base = WIFI_RX_RING5_BASE; /* 4550 */
	prwedinfo->rx_ring[1].cnt = WIFI_RX_RING5_CNT;	 /* 4554 */
	prwedinfo->rx_ring[1].cidx = WIFI_RX_RING5_CIDX; /* 4558 */
	prwedinfo->rx_ring[1].didx = WIFI_RX_RING5_DIDX; /* 455C */

	/* Event Ring, Ring6 for Event/TxFreeDone */
	prwedinfo->event.base = WIFI_RX_RING6_BASE; /* 4560 */
	prwedinfo->event.cnt = WIFI_RX_RING6_CNT;	/* 4564 */
	prwedinfo->event.cidx = WIFI_RX_RING6_CIDX; /* 4568 */
	prwedinfo->event.didx = WIFI_RX_RING6_DIDX; /* 456C */

	/* Interrupt status bit set */
	prwedinfo->wfdma_tx_done_trig0_bit =
		WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_0_SHFT;
	prwedinfo->wfdma_tx_done_trig1_bit =
		WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_1_SHFT;
	prwedinfo->wfdma_rx_done_trig0_bit =
		WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_4_SHFT;
	prwedinfo->wfdma_rx_done_trig1_bit =
		WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_5_SHFT;
	prwedinfo->wfdma_tx_done_free_notify_trig_bit =
		WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_6_SHFT;

	/* Let proxy driver could pass IRQ process flow back to gen4m */
	prwedinfo->irq_handler = prHifInfo->irq_handler;
	prwedinfo->irq_handler_thread = prHifInfo->irq_handler_thread;
	prwedinfo->irq_swap = wedInterruptSwap;

	/* wedRxTokenInit callback*/
	prwedinfo->wedRxTokenInit = wedRxTokenInit;
	/* enable WED's DMA TX/RX */
	prwedinfo->wed_dma_ctrl = DMA_TX_RX;

	return 0;
}

static void wedRxTokenInfoRelease(struct ADAPTER *prAdapter)
{
	struct GL_HIF_INFO *prHifInfo;
	struct WED_DMABUF *prWedDmaBuf;
	struct RTMP_RX_RING *pRxRing;
	struct RTMP_DMACB *prRxCell;
	struct RTMP_DMABUF *pDmaBuf;
	struct RXD_STRUCT *pRxD;
	uint32_t u4token_id;
	uint32_t u4Idx;

	prHifInfo = &prAdapter->prGlueInfo->rHifInfo;

	/* Release the RxBM + WiFi buffer by the individual way */
	for (u4token_id = 0; u4token_id < grWedToken.u4MaxSize; u4token_id++) {
		prWedDmaBuf = &grWedToken.pkt_token[u4token_id];
		if (!prWedDmaBuf->fgIsSKB && prWedDmaBuf->pkt != NULL)
			page_frag_free(prWedDmaBuf->pkt);

		/* skb would be freed by kernel, no need to free it again */
	}

	/* free all memory allocated at WARP attach phase */
	kalMemFree(grWedToken.pkt_token, VIR_MEM_TYPE,
		grWedToken.u4MaxSize*sizeof(struct WED_DMABUF));

	grWedToken.u4FreeIdx = 0;
	grWedToken.u4MaxSize = 0;

	/* Flush tokenID to gen4m ring1's buffer */
	pRxRing = &prHifInfo->RxRing[RX_RING_DATA0];
	for (u4Idx = 0; u4Idx < pRxRing->u4RingSize; u4Idx++) {
		prRxCell = &pRxRing->Cell[u4Idx];
		pDmaBuf = &prRxCell->DmaBuf;
		pRxD = (struct RXD_STRUCT *) prRxCell->AllocVa;

		pRxD->SDPtr1 &= ~(RXDMAD_TOKEN_ID_MASK);
		pRxD->SDLen1 &= ~(BIT(8));
	}

	/* Flush tokenID to gen4m ring2's buffer */
	pRxRing = &prHifInfo->RxRing[RX_RING_DATA1];
	for (u4Idx = 0; u4Idx < pRxRing->u4RingSize; u4Idx++) {
		prRxCell = &pRxRing->Cell[u4Idx];
		pDmaBuf = &prRxCell->DmaBuf;
		pRxD = (struct RXD_STRUCT *) prRxCell->AllocVa;

		pRxD->SDPtr1 &= ~(RXDMAD_TOKEN_ID_MASK);
		pRxD->SDLen1 &= ~(BIT(8));
	}

}

int wedRxTokenInfoSetup(struct ADAPTER *prAdapter)
{
	struct GL_HIF_INFO *prHifInfo;
	struct RTMP_RX_RING *pRxRing;
	struct RTMP_DMACB *prRxCell;
	struct RTMP_DMABUF *pDmaBuf;
	struct RXD_STRUCT *pRxD;
	uint32_t u4token_id;
	uint32_t u4MaxSize;
	uint32_t u4Idx;

	DBGLOG(HAL, STATE, "WED Rx Token set up\n");

	/* Total token number would be gen4m's
	 * ring1 + ring2 + buffer number in warp
	 */
	u4MaxSize = 0;
	prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	pRxRing = &prHifInfo->RxRing[RX_RING_DATA0];
	u4MaxSize += pRxRing->u4RingSize;
	pRxRing = &prHifInfo->RxRing[RX_RING_DATA1];
	u4MaxSize += pRxRing->u4RingSize;

	/* hard code number in warp driver */
	u4MaxSize += WARP_RXBM_CB_NUM;

	grWedToken.u4MaxSize = u4MaxSize;
	grWedToken.u4FreeIdx = 0;

	grWedToken.pkt_token = kalMemAlloc(
		u4MaxSize * sizeof(struct WED_DMABUF), VIR_MEM_TYPE);
	if (grWedToken.pkt_token == NULL) {
		DBGLOG(HAL, ERROR, "WED RX token[%d] allocate fail\n",
			u4MaxSize);
		return -1;
	}

	DBGLOG(HAL, STATE, "Total WED RX token[%d] allocated\n",
		u4MaxSize);

	/* Assign tokenID to gen4m ring1's buffer */
	pRxRing = &prHifInfo->RxRing[RX_RING_DATA0];
	for (u4Idx = 0; u4Idx < pRxRing->u4RingSize; u4Idx++) {
		prRxCell = &pRxRing->Cell[u4Idx];
		pDmaBuf = &prRxCell->DmaBuf;
		pRxD = (struct RXD_STRUCT *) prRxCell->AllocVa;

		u4token_id = wedRxTokenInit(NULL, TRUE,
			prRxCell->pPacket, pDmaBuf->AllocSize,
			pDmaBuf->AllocVa, pDmaBuf->AllocPa);

		pRxD->SDPtr1 = (u4token_id << RXDMAD_TOKEN_ID_SHIFT);
		pRxD->SDLen1 = BIT(8);
	}

	/* Assign tokenID to gen4m ring2's buffer */
	pRxRing = &prHifInfo->RxRing[RX_RING_DATA1];
	for (u4Idx = 0; u4Idx < pRxRing->u4RingSize; u4Idx++) {
		prRxCell = &pRxRing->Cell[u4Idx];
		pDmaBuf = &prRxCell->DmaBuf;
		pRxD = (struct RXD_STRUCT *)prRxCell->AllocVa;

		u4token_id = wedRxTokenInit(NULL, TRUE,
			prRxCell->pPacket, pDmaBuf->AllocSize,
			pDmaBuf->AllocVa, pDmaBuf->AllocPa);

		pRxD->SDPtr1 = (u4token_id << RXDMAD_TOKEN_ID_SHIFT);
		pRxD->SDLen1 = BIT(8);
	}

	return 0;
}

static void wedNetDevSet(struct net_device *dev)
{
	uint32_t i = 0;

	for (i = 0; i < MAX_BSSID_NUM; i++) {
		if (grNetList[i] == dev)
			return;

		if (grNetList[i] == NULL) {
			grNetList[i] = dev;
			break;
		}
	}
}

static void wedNetDevDel(struct net_device *dev)
{
	uint32_t i = 0;

	for (i = 0; i < MAX_BSSID_NUM; i++) {
		if (grNetList[i] == dev) {
			grNetList[i] = NULL;
			break;
		}
	}
}

static void wedHwnatSet(struct ADAPTER *prAdapter,
	bool fgEnable, struct net_device *prNetDev)
{
	if (!prNetDev) {
		DBGLOG(HAL, ERROR, "net_device is NULL!\n");
		return;
	}

	if (!ppe_dev_register_hook || !ppe_dev_unregister_hook) {
		DBGLOG(HAL, ERROR, "HWNAT driver got error! fail on %s\n",
			   fgEnable ? "Register" : "Unregister");
		return;
	}

	DBGLOG(HAL, STATE, "%s device %s to hw_nat\n",
		   fgEnable ? "Register" : "Unregister", prNetDev->name);

	if (fgEnable) {
		wedNetDevSet(prNetDev);
		ppe_dev_register_hook(prNetDev);
	} else {
		wedNetDevDel(prNetDev);
		ppe_dev_unregister_hook(prNetDev);
	}
}

int wedAttachWarp(struct ADAPTER *prAdapter, struct net_device *prNetDev,
						uint8_t AttachType)
{
	uint32_t val = 0;
	int ret = 0;
	struct WED_INFO *prwedinfo;

	if (IS_FEATURE_DISABLED(prAdapter->rWifiVar.fgEnableWed)) {
		DBGLOG(HAL, WARN, "WED disabled by wifi.cfg\n");
		return ret;
	}
	if (grWedInfo.proxy_ops == NULL) {
		DBGLOG(HAL, WARN, "warp_proxy not regisgered\n");
		return -1;
	}

	/* step.1 Register net_dev to hwnat */
	/* Direct call to cover DBDC */
	wedHwnatSet(prAdapter, TRUE, prNetDev);

	if (IsWedAttached()) {
		DBGLOG(HAL, WARN, "WED already attached before\n");
		return -1;
	}

	ACQUIRE_POWER_CONTROL_FROM_PM(prAdapter);
	DBGLOG(HAL, STATE, "WED proxy attaching, reason: %u\n", AttachType);

	prwedinfo = &grWedInfo;

	/* Before hooking up with warp's HW,
	 * initial local ring's buffer with tokenID
	 */
	ret = wedRxTokenInfoSetup(prAdapter);
	if (ret < 0)
		goto error;

	/* step.2 fill all necessary information for warp proxy driver */
	ret = wedInfoSetup(prAdapter);
	if (ret < 0)
		goto error;

	/* step.3 Ask WARP driver to perform basic WED HW initialization
	 * with diff flow between interface up and resume
	 */
	if (AttachType == WED_ATTACH_IFON)
		ret = wedProxyHookCall(PROXY_WLAN_HOOK_SYS_UP, &grWedInfo);
	else
		ret = wedProxyHookCall(PROXY_WLAN_HOOK_RESUME, &grWedInfo);

	if (ret < 0)
		goto error;

	grWedInfo.fgMirrorEnable = 1;
	disable_irq_nosync(prwedinfo->u4IrqId);
	DBGLOG(HAL, STATE, "disable irq for %d\n", prwedinfo->u4IrqId);

	/* step.4 Force reset WFDMA HW */
	kalDevRegWrite(prAdapter->prGlueInfo, 0x7c024100, 0x00000020);
	kalDevRegWrite(prAdapter->prGlueInfo, 0x7c024100, 0x00000030);

	/* step.5 Reinitial TX/RX rings in WFDMA */
	halWpdmaInitRing(prAdapter->prGlueInfo, false);

	/* step.6 Disable WFDMA's RX(warp driver will enable it later)
	 * and keep RXDMAD reserved context which used by WED
	 */
	kalDevRegRead(prAdapter->prGlueInfo,
		WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_ADDR, &val);
	val &= ~(BIT(2));
	kalDevRegWrite(prAdapter->prGlueInfo,
		WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_ADDR, val);

	kalDevRegRead(prAdapter->prGlueInfo,
		WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_EXT0_ADDR, &val);
	val |= BIT(10);
	kalDevRegWrite(prAdapter->prGlueInfo,
		WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_EXT0_ADDR, val);

	/* step.7  trigger warp driver to take control on WFDMA */
	if (prwedinfo->wed_ver > 0) {
		wedProxyHookCall(PROXY_WLAN_HOOK_HIF_INIT, &grWedInfo);
		wedProxyHookCall(PROXY_WLAN_HOOK_DMA_SET, &grWedInfo);
	} else {
		DBGLOG(HAL, WARN, "WED attach failed by system kernel %d\n");
		ret = -1;
		goto error;
	}

	/* step.8 Enable INT again(mapping to warp) Read from gen4m, and write
	 * to WED. Direct use halEnableInterrupt may cause Kernel warnning
	 * about enable irq twice
	 */
	kalDevRegWrite(prAdapter->prGlueInfo,
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_ADDR,
		grWedInfo.int_enable_mask);

	/* step.9 Enable PCIE irq to WED */
	wedProxyHookCall(PROXY_WLAN_HOOK_SWAP_IRQ, &grWedInfo);

	grWedInfo.fgAttached = TRUE;

	/*this will enable wed irq*/
	enable_irq(prwedinfo->u4IrqId);
	DBGLOG(HAL, STATE, "enable irq for %d\n", prwedinfo->u4IrqId);

	ret = 0;
error:
	RECLAIM_POWER_CONTROL_TO_PM(prAdapter, FALSE);
	return ret;
}

int wedDetachWarp(struct ADAPTER *prAdapter, struct net_device *prNetDev,
	uint8_t DetachType)
{
	struct BUS_INFO *prBusInfo;
	struct WED_INFO *prwedinfo;
	struct mt66xx_chip_info *prChipInfo;
	struct GL_HIF_INFO *prHifInfo = NULL;
	uint32_t i = 0;
	int ret = 0;

	if (IS_FEATURE_DISABLED(prAdapter->rWifiVar.fgEnableWed)) {
		DBGLOG(HAL, WARN, "WED disabled by wifi.cfg\n");
		return ret;
	}

	/* step.1 Unregister net_dev to hwnat */
	/* Direct call to cover DBDC */
	wedHwnatSet(prAdapter, FALSE, prNetDev);

	/* Detach is required only when all interfaces go down */
	for (i = 0; i < MAX_BSSID_NUM; i++) {
		if (DetachType == WED_DETACH_SUSPEND)
			break;
		if (grNetList[i] != NULL)
			return -1;
	}

	if (!IsWedAttached()) {
		DBGLOG(HAL, WARN, "WED not attached yet\n");
		return -1;
	}

	ACQUIRE_POWER_CONTROL_FROM_PM(prAdapter);

	DBGLOG(HAL, STATE, "WED proxy detaching, reason: %u\n", DetachType);

	prwedinfo = &grWedInfo;
	prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	prBusInfo = prAdapter->chip_info->bus_info;
	prChipInfo = prAdapter->chip_info;

	/* step.3 Release the ring control on WED and CR mirror*/
	wedProxyHookCall(PROXY_WLAN_HOOK_HIF_EXIT, &grWedInfo);
	prwedinfo->fgMirrorEnable = FALSE;

	/* step.4 Release the RXBM buffer which is allocated by WARP Nothing
	 * but WiFi host know the status of this buffer Also release the
	 * gen4m's one
	 */
	halUninitMsduTokenInfo(prAdapter);
	halWpdmaFreeRing(prAdapter->prGlueInfo);
	wedRxTokenInfoRelease(prAdapter);

	/* step.5 Force WFDMA reinit after take over from WED to CPU
	 * Reallocate the Ring Buffer to avoid buffer free error when rmmod
	 */
	if (!halWpdmaAllocRing(prAdapter->prGlueInfo, true)) {
		ret = -1;
		goto error;
	}

	halWpdmaInitRing(prAdapter->prGlueInfo, true);
	halInitMsduTokenInfo(prAdapter);
	/* Initialize wfdma reInit handshake parameters */
	if ((prChipInfo->asicWfdmaReInit)
	    && (prChipInfo->asicWfdmaReInit_handshakeInit))
		prChipInfo->asicWfdmaReInit_handshakeInit(prAdapter);

	/* step.6 WARP & WO resource release and IRQ swap back */
	/* with diff flow between interface down and suspend */
	if (DetachType == WED_DETACH_IFDOWN)
		wedProxyHookCall(PROXY_WLAN_HOOK_SYS_DOWN, &grWedInfo);
	else
		wedProxyHookCall(PROXY_WLAN_HOOK_SUSPEND, &grWedInfo);

	prwedinfo->whnat_en = FALSE;

	/* step.7 Enable HIF side PDMA TX/RX */
	if (prBusInfo->pdmaStop)
		prBusInfo->pdmaStop(prAdapter->prGlueInfo, FALSE);
	else
		DBGLOG(HAL, ERROR, "PDMA config API didn't register\n");


	grWedInfo.fgAttached = FALSE;
	ret = 0;

error:
	RECLAIM_POWER_CONTROL_TO_PM(prAdapter, FALSE);
	return ret;
}

int wedProxyHookCall(uint16_t hook, void *priv)
{
	struct proxy_wlan_hook_ops *ops;
	static const char *const hook_str[] = {
		"PROXY_WLAN_HOOK_HIF_INIT",
		"PROXY_WLAN_HOOK_HIF_EXIT",		/* 1 */
		"PROXY_WLAN_HOOK_TX",			/* 2 */
		"PROXY_WLAN_HOOK_RX",			/* 3 */
		"PROXY_WLAN_HOOK_SYS_UP",		/* 4 */
		"PROXY_WLAN_HOOK_SYS_DOWN",		/* 5 */
		"PROXY_WLAN_HOOK_ISR",			/* 6 */
		"PROXY_WLAN_HOOK_DMA_SET",		/* 7 */
		"PROXY_WLAN_HOOK_SER",			/* 8 */
		"PROXY_WLAN_HOOK_SUSPEND",		/* 9 */
		"PROXY_WLAN_HOOK_RESUME",		/* 10 */
		"PROXY_WLAN_HOOK_READ",			/* 11 */
		"PROXY_WLAN_HOOK_WRITE",		/* 12 */
		"PROXY_WLAN_HOOK_SEND_CMD",		/* 13 */
		"PROXY_WLAN_HOOK_SWAP_IRQ",		/* 14 */
		"PROXY_WLAN_HOOK_END"			/* 15 */
	};

	ops = grWedInfo.proxy_ops;

	if (ops->hooks & BIT(hook)) {
		DBGLOG(SW1, LOUD, "hook: %s\n", hook_str[hook]);
		if (ops->fun(hook, &grWedInfo, priv) == 0)
			return 0;
	}

	DBGLOG(HAL, WARN, "WED proxy ops invalid\n");
	return -1;
}


int wedProxyHookRegister(struct proxy_wlan_hook_ops *ops)
{
	if (!ops)
		return -1;

	if (grWedInfo.proxy_ops != NULL) {
		DBGLOG(HAL, WARN, "WED proxy ops registered before\n");
		return -1;
	}

	DBGLOG(HAL, INFO, "WED proxy ops registered\n");
	grWedInfo.proxy_ops = ops;

	return 0;

}
EXPORT_SYMBOL(wedProxyHookRegister);

int wedProxyHookUnregister(struct proxy_wlan_hook_ops *ops)
{
	uint32_t i = 0;

	if (!ops)
		return -1;

	if (grWedInfo.proxy_ops != ops) {
		DBGLOG(HAL, INFO, "WED proxy ops mismatch\n");
		return -1;
	}

	/* Error handle to avoid unregister before detach */
	if (IsWedAttached()) {
		dump_stack();
		DBGLOG(HAL, ERROR, "WED not yet Detach!\n");
		for (i = 0; i < MAX_BSSID_NUM; i++) {
			if (grNetList[i] != NULL) {
				DBGLOG(HAL, ERROR, "need disable %s!\n",
					grNetList[i]->name);
				wedDetachWarp(grWedInfo.pAdAdapter,
					grNetList[i], WED_DETACH_IFDOWN);
			}
		}
	}

	DBGLOG(HAL, INFO, "WED proxy ops unregistered\n");
	grWedInfo.proxy_ops = NULL;

	return 0;
}
EXPORT_SYMBOL(wedProxyHookUnregister);

uint32_t wedMirrorAddrCheck(uint32_t u4BusAddr)
{
	uint32_t *pu4AddrList;

	if (grWedInfo.proxy_ops == NULL)
		return 0;

	if (grWedInfo.fgMirrorEnable == FALSE)
		return 0;

	pu4AddrList = mt6639_wed_mirror_table;
	while ((*pu4AddrList != 0) && (*pu4AddrList != u4BusAddr))
		pu4AddrList++;

	return *pu4AddrList;
}

static uint32_t wedRxInfoGet(struct RXD_STRUCT *pRxD, struct SW_RFB *prSwRfb)
{
	struct WED_RX_INFO *prWedRxInfo = NULL;
	uint32_t DW1 = *(uint32_t *)((uint32_t *)pRxD + 1);
	uint32_t DW3 = *(uint32_t *)((uint32_t *)pRxD + 3);
	uint8_t fgDrop = FALSE;

	struct sk_buff *prSkb = (struct sk_buff *)prSwRfb->pvPacket;

	if (DW3 & BIT(RXDMAD_PPE_VLD)) {

		prWedRxInfo = kmalloc(sizeof(struct WED_RX_INFO), GFP_ATOMIC);
		if (!prWedRxInfo) {
			DBGLOG(HAL, WARN, "Failed to alloc WED RX INFO!!\n");
			kfree(prWedRxInfo);
			return 0;
		}

		prWedRxInfo->pPacket = prSwRfb->pvPacket;
		prWedRxInfo->u2PpeEntry = ((DW3 & RXDMAD_PPE_ENTRY_MASK) >>
			RXDMAD_PPE_ENTRY_SHIFT);
		prWedRxInfo->ucCsrn = ((DW3 & RXDMAD_CSRN_MASK) >>
			RXDMAD_CSRN_SHIFT);

		prSwRfb->prWedRxInfo = prWedRxInfo;
		WED_SET_PPE_TYPE(prSkb, RX_PPE_VALID);

		/* only extract info from rxdmad here, but not call warp hook
		 * PROXY_WLAN_HOOK_RX due the headroom space of pvPacket here
		 * is 0 and expand manually may cause kernel panic
		 */
	} else {
		WED_SET_PPE_TYPE(prSkb, RX_PPE_UNVALID);
	}

	/* do not drop packet even fdDrop is true for now*/
	fgDrop = (DW1 & (RXDMAD_RXD_DROP | RXDMAD_TO_HOST_A)) ? 1 : 0;

	return fgDrop;
}

uint32_t wedHwRxInfoWrapper(struct SW_RFB *prSwRfb)
{
	struct WED_RX_INFO *prWedRxInfo = NULL;
	struct sk_buff *prSkb = NULL;

	if (!IsWedAttached())
		return 0;

	if (WED_GET_PPE_TYPE(prSwRfb->pvPacket) != RX_PPE_VALID)
		return 0;

	prWedRxInfo = (struct WED_RX_INFO *)prSwRfb->prWedRxInfo;
	prSkb = (struct sk_buff *)prWedRxInfo->pPacket;

	if (skb_headroom(prSkb) < FOE_INFO_LEN) {
		DBGLOG(HAL, WARN,
			"SKB has no enough headroom (%d -> %d) bytes!\n",
			skb_headroom(prSkb), FOE_INFO_LEN);
		return -1;
	}

	wedProxyHookCall(PROXY_WLAN_HOOK_RX, prWedRxInfo);
	kfree(prWedRxInfo);

	return 0;
}

uint32_t wedHwRxRequest(void *prSkb)
{
	struct WED_INFO *prwedinfo;

	if (!IsWedAttached())
		return 0;

	if (ra_sw_nat_hook_rx) {

		prwedinfo = &grWedInfo;

		if (prwedinfo->whnat_en &&
			WED_GET_PPE_TYPE(prSkb) == RX_PPE_VALID) {
			ra_sw_nat_hook_rx(prSkb);
		}
	}
	return 0;
}

bool wedRxSkbGen(struct GLUE_INFO *prGlueInfo,
	struct WED_DMABUF *prWedDmaBuf, void **prPacket)
{
	struct sk_buff *prSKB;
	uint32_t u4Size;

	/* This buffer belong to a SKB already */
	if (prWedDmaBuf->fgIsSKB) {
		*prPacket = prWedDmaBuf->pkt;
		return TRUE;
	}

	/* Generate SKB for this buffer */
	u4Size = SKB_DATA_ALIGN(prWedDmaBuf->AllocSize + NET_SKB_PAD) +
		SKB_DATA_ALIGN(sizeof(struct skb_shared_info));
	prSKB = build_skb(prWedDmaBuf->AllocVa, u4Size);
	if (prSKB == NULL) {
		DBGLOG(HAL, ERROR, "SKB build fail!\n");
		return FALSE;
	}
	*prPacket = (void *) prSKB;

	return TRUE;
}

bool wedRxBufferSwap(struct GLUE_INFO *prGlueInfo, uint32_t u4tokeID,
	struct WED_DMABUF *prWedDmaBuf, struct RXD_STRUCT *pRxD,
	struct RTMP_DMABUF *prDmaBuf, struct SW_RFB *prSwRfb)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	dma_addr_t rAddr;
	void *prPacket;

	prHifInfo = &prGlueInfo->rHifInfo;

	/* Unmap RX buffer from warp BM*/
	KAL_DMA_UNMAP_SINGLE_ATTRS(prHifInfo->prDmaDev,
		(dma_addr_t) prWedDmaBuf->AllocPa,
		prWedDmaBuf->AllocSize, KAL_DMA_FROM_DEVICE);
	/* Generate SKB if needed */
	if (wedRxSkbGen(prGlueInfo, prWedDmaBuf, &prPacket) == FALSE)
		return FALSE;

	/* Swap buffer from SwRfb & WedDmaBuf */
	prWedDmaBuf->fgIsSKB = TRUE;
	prWedDmaBuf->pkt = prSwRfb->pvPacket;
	prWedDmaBuf->AllocVa = ((struct sk_buff *)prSwRfb->pvPacket)->data;
	rAddr = KAL_DMA_MAP_SINGLE_ATTRS(prHifInfo->prDmaDev,
		prWedDmaBuf->AllocVa, prWedDmaBuf->AllocSize,
		KAL_DMA_FROM_DEVICE);
	if (KAL_DMA_MAPPING_ERROR(prHifInfo->prDmaDev, rAddr)) {
		DBGLOG(HAL, ERROR, "KAL_DMA_MAP_SINGLE() error!\n");
		ASSERT(0);
		return FALSE;
	}
	prWedDmaBuf->AllocPa = rAddr;

	/* Align information to gen4m's DmaBuf & DMAD content */
	prDmaBuf->AllocPa = prWedDmaBuf->AllocPa;
	prDmaBuf->AllocVa = prWedDmaBuf->AllocVa;
	prDmaBuf->AllocSize = prWedDmaBuf->AllocSize;

	pRxD->SDPtr0 = prWedDmaBuf->AllocPa;
	pRxD->SDLen0 = prWedDmaBuf->AllocSize;
	pRxD->DMADONE = 0;
	pRxD->SDPtr1 = (u4tokeID << RXDMAD_TOKEN_ID_SHIFT);
	pRxD->SDLen1 = BIT(8);

	/* assign RX packet to SwRfb */
	prSwRfb->pvPacket = prPacket;
	prSwRfb->pucRecvBuff = ((struct sk_buff *) prSwRfb->pvPacket)->data;
	prSwRfb->prRxStatus = (void *) prSwRfb->pucRecvBuff;

	return TRUE;
}

bool wedDevReadData(struct GLUE_INFO *prGlueInfo, uint16_t u2Port,
		    struct SW_RFB *prSwRfb)
{
	struct ADAPTER *prAdapter = NULL;
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct HIF_MEM_OPS *prMemOps;
	struct RXD_STRUCT *pRxD;
	struct RTMP_RX_RING *prRxRing;
	struct RTMP_DMACB *pRxCell;
	struct RTMP_DMABUF *prDmaBuf;
	struct WED_DMABUF *prWedDmaBuf;
	u_int8_t fgRet = TRUE;
	uint32_t u4CpuIdx = 0;
	uint32_t u4tokeID;

	ASSERT(prGlueInfo);

	if (!IsWedAttached())
		return kalDevReadData(prGlueInfo, u2Port, prSwRfb);

	prAdapter = prGlueInfo->prAdapter;
	prHifInfo = &prGlueInfo->rHifInfo;
	prMemOps = &prHifInfo->rMemOps;
	prRxRing = &prHifInfo->RxRing[u2Port];
	u4CpuIdx = prRxRing->RxCpuIdx;
	INC_RING_INDEX(u4CpuIdx, prRxRing->u4RingSize);

	pRxCell = &prRxRing->Cell[u4CpuIdx];
	pRxD = (struct RXD_STRUCT *)pRxCell->AllocVa;

	u4tokeID = pRxD->SDPtr1 >> RXDMAD_TOKEN_ID_SHIFT;
	prWedDmaBuf = wedRxtokenGet(NULL, u4tokeID);
	if (!prWedDmaBuf)
		return NULL;

	pRxCell->pPacket = prSwRfb->pvPacket;

	prDmaBuf = &pRxCell->DmaBuf;
	wedRxBufferSwap(prGlueInfo, u4tokeID,
		prWedDmaBuf, pRxD, prDmaBuf, prSwRfb);

	prRxRing->RxCpuIdx = u4CpuIdx;
	prRxRing->fgIsDumpLog = false;

	if (prSwRfb->pvPacket)
		wedRxInfoGet(pRxD, prSwRfb);

	return fgRet;
}

bool IsWedAttached(void)
{
	return grWedInfo.fgAttached;
}

uint32_t wedRxTokenInit(void *priv, uint8_t fgIsSKB, void *pkt,
	unsigned long alloc_size, void *alloc_va, phys_addr_t alloc_pa)
{
	uint32_t u4token_id;
	struct WED_DMABUF *prWedDmaBuf;

	u4token_id = grWedToken.u4FreeIdx;
	prWedDmaBuf = &grWedToken.pkt_token[u4token_id];

	prWedDmaBuf->AllocSize = alloc_size;
	prWedDmaBuf->AllocPa = alloc_pa;
	prWedDmaBuf->AllocVa = alloc_va;
	prWedDmaBuf->pkt = pkt;
	prWedDmaBuf->fgIsSKB = fgIsSKB;

	grWedToken.u4FreeIdx++;

	if (grWedToken.u4FreeIdx == grWedToken.u4MaxSize)
		DBGLOG(HAL, STATE, "WED RX token full [%d]\n",
			grWedToken.u4MaxSize);

	return u4token_id;
}

struct WED_DMABUF *wedRxtokenGet(void *priv, uint32_t u4token_id)
{
	struct WED_DMABUF *prDmaBuf;

	if (u4token_id >= grWedToken.u4FreeIdx) {
		DBGLOG(HAL, STATE, "fail to get token with ID[%d]\n",
			u4token_id);
		return NULL;
	}
	prDmaBuf = &grWedToken.pkt_token[u4token_id];

	return prDmaBuf;
}

/* For debug purpose */
uint32_t wedMirrorRevert(void)
{
	grWedInfo.fgMirrorEnable = !grWedInfo.fgMirrorEnable;

	return 0;
}

static uint32_t wedSendCommand2WO(uint32_t woCmdId, void *prWoCmdContent,
	uint32_t prWoCmdLen)
{
	struct WO_CMD_INFO prWoCmdInfo;

	if (!IsWedAttached())
		return 0;

	prWoCmdInfo.pMsg = prWoCmdContent;
	prWoCmdInfo.u4MsgLen = prWoCmdLen;
	prWoCmdInfo.wo_cmd_id = woCmdId;

	return wedProxyHookCall(PROXY_WLAN_HOOK_SEND_CMD, &prWoCmdInfo);
}

uint32_t wedHwTxRequest(struct ADAPTER *prAdapter,
	struct MSDU_INFO *prMsduInfo)
{
	struct WED_MSDU_INFO *prWedTxInfo;
	uint8_t ucWlanIndex = 0;
	uint32_t u2Port = TX_RING_DATA0;
	struct BSS_INFO *prBssInfo;
	uint32_t ret;

	if (!IsWedAttached())
		return 0;

	prWedTxInfo = kmalloc(sizeof(struct WED_MSDU_INFO), GFP_ATOMIC);

	if (!prWedTxInfo) {
		DBGLOG(HAL, WARN, "Failed to alloc WED Tx Info!!\n");
		kfree(prWedTxInfo);
		return -1;
	}

	prWedTxInfo->fgDrop = FALSE;

	/* prMsduInfo->ucWlanIndex not yet to be set, get wlan_idx here */
	ucWlanIndex = nicTxGetWlanIdx(prAdapter,
		prMsduInfo->ucBssIndex, prMsduInfo->ucStaRecIndex);

	/* wmmIdx to indicate different ring index */
	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter,
		prMsduInfo->ucBssIndex);
	if (prBssInfo) {
		u2Port = (prBssInfo->ucWmmQueSet % 2) ?
			TX_RING_DATA1 : TX_RING_DATA0;
	}

	/* extract something from MSDU_INFO needed by PPE entry */
	prWedTxInfo->pPacket = prMsduInfo->prPacket; /* ptr to sk_buff */
	/* HIF_TxD v0.1 DW9[7:0] */
	prWedTxInfo->ucBssIndex = prMsduInfo->ucBssIndex;
	prWedTxInfo->ucWlanIndex = ucWlanIndex; /* HIF_TxD v0.1 DW9[15:8] */
	prWedTxInfo->ringIdx = u2Port;

	ret = wedProxyHookCall(PROXY_WLAN_HOOK_TX, prWedTxInfo);
	kfree(prWedTxInfo);

	return ret;
}

static uint32_t wedStaRecUpdateBasic(
	struct ADAPTER *prAdapter,
	uint8_t *pMsgBuf,
	void *args)
{
	struct STA_RECORD *pStaRecCfg = (struct STA_RECORD *) args;
	struct STAREC_COMMON_WO StaRecCommon = {0};

	/* Fill TLV format */
	StaRecCommon.u2Tag = STA_REC_BASIC;
	StaRecCommon.u2Length = sizeof(struct STAREC_COMMON_WO);
	StaRecCommon.u4ConnectionType = CPU_TO_LE32(
		nicUniCmdStaRecConnType(prAdapter, pStaRecCfg->eStaType));
	/* OR pStaRecCfg->ucStaState ? */
	StaRecCommon.ucConnectionState = STATE_CONNECTED;
	StaRecCommon.ucIsQBSS = pStaRecCfg->fgIsQoS;
	StaRecCommon.u2AID = CPU_TO_LE16(pStaRecCfg->u2AssocId);
	kalMemCopy(&StaRecCommon.aucPeerMacAddr[0],
			   &pStaRecCfg->aucMacAddr[0], MAC_ADDR_LEN);

	/* New info to indicate this is new way to update STAREC */
	StaRecCommon.u2ExtraInfo = STAREC_COMMON_EXTRAINFO_V2;

	if (pStaRecCfg->ucStaState == STA_STATE_1)
		StaRecCommon.u2ExtraInfo |= STAREC_COMMON_EXTRAINFO_NEWSTAREC;

	/* Append this feature */
	kalMemCopy(pMsgBuf, (char *)&StaRecCommon,
			   sizeof(struct STAREC_COMMON_WO));
	return 0;
}

uint32_t wedStaRecUpdate(struct ADAPTER *prAdapter,
	struct STA_RECORD *pStaRecCfg)
{
	uint32_t rWlanStatus = WLAN_STATUS_SUCCESS;
	uint32_t size;
	struct CMD_STAREC_UPDATE_WO *prCmdContent;

	if (!IsWedAttached())
		return 0;

	if (!prAdapter)
		return 0;

	if (!pStaRecCfg) {
		DBGLOG(HAL, WARN, "STA_REC is NULL, skip!\n");
		return 0;
	}

	size = sizeof(struct CMD_STAREC_UPDATE_WO);
	size += sizeof(struct STAREC_COMMON_WO);

	prCmdContent = cnmMemAlloc(prAdapter, RAM_TYPE_BUF, size);
	if (!prCmdContent) {
		DBGLOG(HAL, WARN, "command allocation failed!\n");
		cnmMemFree(prAdapter, prCmdContent);
		return WLAN_STATUS_RESOURCES;
	}

	prCmdContent->ucBssIndex = pStaRecCfg->ucBssIndex;
	prCmdContent->ucWlanIdx = pStaRecCfg->ucWlanIndex;
	prCmdContent->u2TotalElementNum = 1;
	prCmdContent->ucAppendCmdTLV = TRUE;
	/* Multicast Index */
	prCmdContent->ucMuarIdx = 0;
	/* WCID_SET_H_L(prCmdContent->ucWlanIdxHnVer,
	 * prCmdContent->ucWlanIdx, pStaRecCfg->ucWlanIndex);
	 */
	prCmdContent->ucWlanIdxHnVer = 0;
	prCmdContent->ucWlanIdx = pStaRecCfg->ucWlanIndex;

	wedStaRecUpdateBasic(prAdapter,
		(uint8_t *)prCmdContent + sizeof(struct CMD_STAREC_UPDATE_WO),
		(void *)pStaRecCfg);

	wedSendCommand2WO(WO_CMD_STA_REC, prCmdContent, size);

	/* free prCmdContent */
	cnmMemFree(prAdapter, prCmdContent);

	return rWlanStatus;
}

static uint32_t wedStaRecRxAddBaUpdateBasic(
	struct ADAPTER *prAdapter,
	uint8_t *pMsgBuf,
	void *args)
{
	struct UNI_EVENT_RX_ADDBA *prRxAddBa = NULL;
	struct STAREC_BA_WO StaRecBa = {0};

	prRxAddBa = (struct UNI_EVENT_RX_ADDBA *) args;
	/* Fill TLV format */
	StaRecBa.u2Tag = STA_REC_BA;
	StaRecBa.u2Length = sizeof(struct STAREC_BA_WO);

	/* content */
	StaRecBa.ucTid = prRxAddBa->ucTid;
	StaRecBa.ucBaDirection = BA_SESSION_RECP;
	StaRecBa.ucAmsduCap = TRUE;
	StaRecBa.ucBaEenable = TRUE;
	StaRecBa.u2BaStartSeq = prRxAddBa->u2BAStartSeqCtrl;
	StaRecBa.u2BaWinSize = prRxAddBa->u2WinSize;

	kalMemCopy(pMsgBuf, (char *) &StaRecBa,
		sizeof(struct STAREC_BA_WO));
	return 0;
}
uint32_t wedStaRecRxAddBaUpdate(struct ADAPTER *prAdapter, void *ba)
{
	uint32_t rWlanStatus = WLAN_STATUS_SUCCESS;
	struct UNI_EVENT_RX_ADDBA *prRxAddBa = NULL;
	struct CMD_STAREC_UPDATE_WO *prCmdContent = NULL;
	uint8_t ucBssIndex = 0;
	uint32_t size;

	if (!IsWedAttached())
		return 0;

	if (!prAdapter)
		return 0;

	if (!ba) {
		DBGLOG(HAL, WARN, "RX_ADDBA is NULL, skip!\n");
		return 0;
	}

	size = sizeof(struct CMD_STAREC_UPDATE_WO);
	size += sizeof(struct STAREC_BA_WO);

	prRxAddBa = (struct UNI_EVENT_RX_ADDBA *)ba;

	prCmdContent = cnmMemAlloc(prAdapter, RAM_TYPE_BUF, size);

	if (!prCmdContent) {
		DBGLOG(HAL, WARN,
			"command allocation failed!\n");
		cnmMemFree(prAdapter, prCmdContent);
		return WLAN_STATUS_RESOURCES;
	}

	ucBssIndex = secGetBssIdxByWlanIdx(prAdapter, prRxAddBa->u2WlanIdx);

	prCmdContent->ucBssIndex = ucBssIndex;
	prCmdContent->ucWlanIdx = prRxAddBa->u2WlanIdx;
	prCmdContent->u2TotalElementNum = 1;
	prCmdContent->ucAppendCmdTLV = TRUE;
	/* Multicast Index */
	prCmdContent->ucMuarIdx = 0;
	WCID_SET_H_L(prCmdContent->ucWlanIdxHnVer,
		prCmdContent->ucWlanIdx, prRxAddBa->u2WlanIdx);

	wedStaRecRxAddBaUpdateBasic(prAdapter,
		(uint8_t *) prCmdContent + sizeof(struct CMD_STAREC_UPDATE_WO),
		(void *) prRxAddBa);

	wedSendCommand2WO(WO_CMD_STA_REC, prCmdContent, size);

	/* free prCmdContent */
	cnmMemFree(prAdapter, prCmdContent);

	return rWlanStatus;
}

static u_int8_t wedIsAPUp(struct GLUE_INFO *prGlueInfo)
{
	struct GL_P2P_INFO *prP2pInfo = NULL;
	uint8_t ucIdx = 0;
	int ret = 0;

	if (!prGlueInfo) {
		DBGLOG(HAL, ERROR, "prGlueInfo is NULL!\n");
		return ret;
	}

	for (ucIdx = 0; ucIdx < KAL_P2P_NUM; ucIdx++) {
		prP2pInfo = prGlueInfo->prP2PInfo[ucIdx];
		/* Expect that only create the new dev with the ap0 */
		if (prP2pInfo == NULL || prP2pInfo->prWdev == NULL)
			continue;

		if (prP2pInfo->prDevHandler == NULL)
			continue;

		if (prP2pInfo->prWdev->iftype != NL80211_IFTYPE_AP)
			continue;

		if (netif_carrier_ok(prP2pInfo->prDevHandler)) {
			ret = 1;
			break;
		}
	}

	return ret;
}

void wedSuspendTrigger(void)
{
	struct WED_INFO *prwedinfo;
	struct ADAPTER *prAdapter;
	struct net_device *prNetDev = NULL;

	DBGLOG(HAL, STATE, "WED Suspend Start!\n");

	prwedinfo = &grWedInfo;
	prAdapter = prwedinfo->pAdAdapter;

	if (!wedIsAPUp(prAdapter->prGlueInfo))
		return;

	prNetDev = wlanGetNetDev(prAdapter->prGlueInfo, 1);

	wedDetachWarp(prAdapter, prNetDev, WED_DETACH_SUSPEND);
	INC_CNT(g_u4SuspendCnt);
	DBGLOG(HAL, STATE, "WED Suspend Done! CNT: %u\n", g_u4SuspendCnt);
}

void wedResumeTrigger(void)
{
	struct WED_INFO *prwedinfo;
	struct ADAPTER *prAdapter;
	struct net_device *prNetDev = NULL;

	DBGLOG(HAL, STATE, "WED Resume Start!\n");

	prwedinfo = &grWedInfo;
	prAdapter = prwedinfo->pAdAdapter;

	if (!wedIsAPUp(prAdapter->prGlueInfo))
		return;

	prNetDev = wlanGetNetDev(prAdapter->prGlueInfo, 1);

	wedAttachWarp(prAdapter, prNetDev, WED_ATTACH_RESUME);
	INC_CNT(g_u4ResumeCnt);
	DBGLOG(HAL, STATE, "WED Resume Done! CNT: %u\n", g_u4ResumeCnt);
}
