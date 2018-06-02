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
/*! \file   "hif.h"
 *  \brief  Functions for the driver to register bus and setup the IRQ
 *
 *  Functions for the driver to register bus and setup the IRQ
 */

#ifndef _HIF_H
#define _HIF_H

#include "hif_pdma.h"
#include "connac.h"

#if defined(_HIF_AXI)
#define HIF_NAME "AXI"
#else
#error "No HIF defined!"
#endif

/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */

/*******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 *******************************************************************************
 */
#if CFG_MTK_ANDROID_WMT
struct MTK_WCN_WMT_WLAN_CB_INFO;
extern int mtk_wcn_wmt_wlan_reg(
	struct MTK_WCN_WMT_WLAN_CB_INFO *pWmtWlanCbInfo);
#endif

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */
#define AXI_CFG_PREALLOC_MEMORY_BUFFER    1

#define AXI_TX_MAX_SIZE_PER_FRAME         (NIC_TX_MAX_SIZE_PER_FRAME +      \
					   NIC_TX_DESC_AND_PADDING_LENGTH + \
					   CONNAC_TX_DESC_APPEND_LENGTH)
#define AXI_WLAN_IRQ_NUMBER               16

/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */

/* host interface's private data structure, which is attached to os glue
 ** layer info structure.
 */
struct GL_HIF_INFO {
	struct platform_device *pdev;
	struct device *prDmaDev;
	uint32_t u4IrqId;
	int32_t u4HifCnt;

	/* AXI MMIO Base Address, all access will use */
	uint8_t *CSRBaseAddress;

	/* Shared memory of all 1st pre-allocated
	 * TxBuf associated with each TXD
	 */
	/* Shared memory for Tx descriptors */
	struct RTMP_DMABUF TxDescRing[NUM_OF_TX_RING];
	struct RTMP_TX_RING TxRing[NUM_OF_TX_RING];	/* AC0~3 + HCCA */
	spinlock_t TxRingLock[NUM_OF_TX_RING];	/* Rx Ring spinlock */

	/* Shared memory for RX descriptors */
	struct RTMP_DMABUF RxDescRing[NUM_OF_RX_RING];
	struct RTMP_RX_RING RxRing[NUM_OF_RX_RING];
	spinlock_t RxRingLock[NUM_OF_RX_RING];	/* Rx Ring spinlock */

	u_int8_t fgIntReadClear;
	u_int8_t fgMbxReadClear;

	uint32_t u4IntStatus;

	struct MSDU_TOKEN_INFO rTokenInfo;

	spinlock_t rDynMapRegLock;

	struct ERR_RECOVERY_CTRL_T rErrRecoveryCtl;
	u_int8_t fgIsErrRecovery;
	spinlock_t rSerLock;
	struct timer_list rSerTimer;
	struct list_head rTxCmdQ;
	struct list_head rTxDataQ;
	spinlock_t rTxCmdQLock;
	spinlock_t rTxDataQLock;

	bool fgIsPreAllocMem;

	void *(*allocDmaCoherent)(size_t size, dma_addr_t *dma_handle,
				  bool fgIsTx, uint32_t u4Num);
	struct sk_buff *(*allocRxPacket)(uint32_t u4Len, uint32_t u4Num,
					 uint32_t u4Idx);
	struct sk_buff *(*allocMsduBuf)(uint32_t u4Len, uint32_t u4Idx);
	void (*updateRxPacket)(struct sk_buff *prSkb,
			       uint32_t u4Num, uint32_t u4Idx);
};

struct BUS_INFO {
	const unsigned int top_cfg_base;	/* TOP_CFG_BASE address */
	const struct PCIE_CHIP_CR_MAPPING *bus2chip;
	const unsigned int tx_ring_cmd_idx;
	const unsigned int tx_ring_fwdl_idx;
	const unsigned int tx_ring_data_idx;
	const u_int8_t fgCheckDriverOwnInt;
	const u_int8_t fgInitPCIeInt;
	const uint32_t u4DmaMask;

	void (*pdmaSetup)(struct GLUE_INFO *prGlueInfo, u_int8_t enable);
	void (*enableInterrupt)(struct ADAPTER *prAdapter);
	void (*lowPowerOwnRead)(struct ADAPTER *prAdapter, u_int8_t *pfgResult);
	void (*lowPowerOwnSet)(struct ADAPTER *prAdapter, u_int8_t *pfgResult);
	void (*lowPowerOwnClear)(struct ADAPTER *prAdapter,
		u_int8_t *pfgResult);
	void (*getMailboxStatus)(struct ADAPTER *prAdapter, uint32_t *pu4Val);
	void (*setDummyReg)(struct GLUE_INFO *prGlueInfo);
	void (*checkDummyReg)(struct GLUE_INFO *prGlueInfo);
};

#if CFG_MTK_ANDROID_WMT
struct MTK_WCN_WMT_WLAN_CB_INFO {
	int (*wlan_probe_cb)(void);
	int (*wlan_remove_cb)(void);
	int (*wlan_bus_cnt_get_cb)(void);
	int (*wlan_bus_cnt_clr_cb)(void);
};
#endif

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

#define axi_enable_device(d)      (0)
#define axi_disable_device(d)
#define axi_resource_start(d, v)  (0x18000000)
#define axi_resource_len(d, v)    (0x100000)
#define axi_name(d)               ("AXI-BUS")

/*******************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */

uint32_t glRegisterBus(probe_card pfProbe, remove_card pfRemove);

void glUnregisterBus(remove_card pfRemove);

void glSetHifInfo(struct GLUE_INFO *prGlueInfo, unsigned long ulCookie);

void glClearHifInfo(struct GLUE_INFO *prGlueInfo);

u_int8_t glBusInit(void *pvData);

void glBusRelease(void *pData);

int32_t glBusSetIrq(void *pvData, void *pfnIsr, void *pvCookie);

void glBusFreeIrq(void *pvData, void *pvCookie);

void glSetPowerState(IN struct GLUE_INFO *prGlueInfo, IN uint32_t ePowerMode);

void glGetDev(void *ctx, struct device **dev);

void glGetHifDev(struct GL_HIF_INFO *prHif, struct device **dev);

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */
#endif /* _HIF_H */
