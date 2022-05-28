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
 *    \brief  Functions for the driver to register bus and setup the IRQ
 *
 *    Functions for the driver to register bus and setup the IRQ
 */

#ifndef _HIF_H
#define _HIF_H

#include "hif_pdma.h"

#if defined(_HIF_PCIE)
#define HIF_NAME "PCIE"
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

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */
#define AXI_CFG_PREALLOC_MEMORY_BUFFER    1
#define AXI_ISR_DEBUG_LOG                 1
#define AXI_TX_MAX_SIZE_PER_FRAME         (NIC_TX_MAX_SIZE_PER_FRAME +      \
					   NIC_TX_DESC_AND_PADDING_LENGTH)
#define AXI_TX_CMD_BUFF_SIZE              4096

#if CFG_SUPPORT_PCIE_ASPM
#define ENABLE_ASPM_L1 2
#define DISABLE_ASPM_L1 0

#define  PCI_EXT_CAP_ID_L1PMSS				0x1E
#define  PCI_L1PMSS_CAP						4
#define  PCI_L1PM_CAP_PCIPM_L12				0x00000001
#define  PCI_L1PM_CAP_PCIPM_L11				0x00000002
#define  PCI_L1PM_CAP_ASPM_L12				0x00000004
#define  PCI_L1PM_CAP_ASPM_L11				0x00000008
#define  PCI_L1PM_CAP_L1PM_SS				0x00000010
#define  PCI_L1PM_CAP_PORT_CMR_TIME_MASK	0x0000FF00
#define  PCI_L1PM_CAP_PWR_ON_SCALE_MASK		0x00030000
#define  PCI_L1PM_CAP_PWR_ON_VALUE_MASK		0x00F80000
#define  PCI_L1PMSS_CTR1					8
#define  PCI_L1PM_CTR1_PCIPM_L12_EN			0x00000001
#define  PCI_L1PM_CTR1_PCIPM_L11_EN			0x00000002
#define  PCI_L1PM_CTR1_ASPM_L12_EN			0x00000004
#define  PCI_L1PM_CTR1_ASPM_L11_EN			0x00000008
#define  PCI_L1PMSS_ENABLE_MASK	            (PCI_L1PM_CTR1_PCIPM_L12_EN |   \
					PCI_L1PM_CTR1_PCIPM_L11_EN |   \
					PCI_L1PM_CTR1_ASPM_L12_EN |    \
					PCI_L1PM_CTR1_ASPM_L11_EN)
#define  PCI_L1PM_ENABLE_MASK			0x3

#define PCIE_ASPM_CHECK_L1(reg)	((((reg) & PCI_EXP_LNKCAP_ASPMS) >> 10) & 0x2)

#endif
/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */

struct GL_HIF_INFO;

struct HIF_MEM {
	phys_addr_t pa;
	void *va;
};

struct HIF_MEM_OPS {
	void (*allocTxDesc)(struct GL_HIF_INFO *prHifInfo,
			    struct RTMP_DMABUF *prDescRing,
			    uint32_t u4Num);
	void (*allocRxDesc)(struct GL_HIF_INFO *prHifInfo,
			    struct RTMP_DMABUF *prDescRing,
			    uint32_t u4Num);
	void (*allocExtBuf)(struct GL_HIF_INFO *prHifInfo,
			    struct RTMP_DMABUF *prDescRing);
	bool (*allocTxCmdBuf)(struct RTMP_DMABUF *prDmaBuf,
			      uint32_t u4Num, uint32_t u4Idx);
	void (*allocTxDataBuf)(struct MSDU_TOKEN_ENTRY *prToken,
			       uint32_t u4Idx);
	void *(*allocRxBuf)(struct GL_HIF_INFO *prHifInfo,
			    struct RTMP_DMABUF *prDmaBuf,
			    uint32_t u4Num, uint32_t u4Idx);
	void *(*allocRuntimeMem)(uint32_t u4SrcLen);
	bool (*copyCmd)(struct GL_HIF_INFO *prHifInfo,
			struct RTMP_DMACB *prTxCell, void *pucBuf,
			void *pucSrc1, uint32_t u4SrcLen1,
			void *pucSrc2, uint32_t u4SrcLen2);
	bool (*copyEvent)(struct GL_HIF_INFO *prHifInfo,
			  struct RTMP_DMACB *pRxCell,
			  struct RXD_STRUCT *pRxD,
			  struct RTMP_DMABUF *prDmaBuf,
			  uint8_t *pucDst, uint32_t u4Len);
	bool (*copyTxData)(struct MSDU_TOKEN_ENTRY *prToken,
			   void *pucSrc, uint32_t u4Len);
	bool (*copyRxData)(struct GL_HIF_INFO *prHifInfo,
			   struct RTMP_DMACB *pRxCell,
			   struct RTMP_DMABUF *prDmaBuf,
			   struct SW_RFB *prSwRfb);
	phys_addr_t (*mapTxBuf)(struct GL_HIF_INFO *prHifInfo,
			  void *pucBuf, uint32_t u4Offset, uint32_t u4Len);
	phys_addr_t (*mapRxBuf)(struct GL_HIF_INFO *prHifInfo,
			  void *pucBuf, uint32_t u4Offset, uint32_t u4Len);
	void (*unmapTxBuf)(struct GL_HIF_INFO *prHifInfo,
			   phys_addr_t rDmaAddr, uint32_t u4Len);
	void (*unmapRxBuf)(struct GL_HIF_INFO *prHifInfo,
			   phys_addr_t rDmaAddr, uint32_t u4Len);
	void (*freeDesc)(struct GL_HIF_INFO *prHifInfo,
			 struct RTMP_DMABUF *prDescRing);
	void (*freeExtBuf)(struct GL_HIF_INFO *prHifInfo,
			   struct RTMP_DMABUF *prDescRing);
	void (*freeBuf)(void *pucSrc, uint32_t u4Len);
	void (*freePacket)(struct GL_HIF_INFO *prHifInfo,
			   void *pvPacket, uint32_t u4Num);
	void (*dumpTx)(struct GL_HIF_INFO *prHifInfo,
		       struct RTMP_TX_RING *prTxRing,
		       uint32_t u4Idx, uint32_t u4DumpLen);
	void (*dumpRx)(struct GL_HIF_INFO *prHifInfo,
		       struct RTMP_RX_RING *prRxRing,
		       uint32_t u4Idx, uint32_t u4DumpLen);
};

enum pcie_suspend_state {
	PCIE_STATE_PRE_SUSPEND_WAITING, /* Waiting for FW suspend flow status */
	PCIE_STATE_PRE_SUSPEND_DONE,
	PCIE_STATE_PRE_SUSPEND_FAIL,
	PCIE_STATE_SUSPEND_ENTERING,
	PCIE_STATE_PRE_RESUME_DONE,
	PCIE_STATE_SUSPEND
};

/* host interface's private data structure, which is attached to os glue
 ** layer info structure.
 */

struct GL_HIF_INFO {
	struct pci_dev *pdev;
	struct pci_dev *prDmaDev;
	struct HIF_MEM_OPS rMemOps;

	uint32_t u4IrqId;
	uint32_t u4IrqId_1;
	int32_t u4HifCnt;

	/* PCI MMIO Base Address, all access will use */
	void *CSRBaseAddress;

	/* Shared memory of all 1st pre-allocated
	 * TxBuf associated with each TXD
	 */
	/* Shared memory for Tx descriptors */
	struct RTMP_DMABUF TxDescRing[NUM_OF_TX_RING];
	/* AC0~3 + HCCA */
	struct RTMP_TX_RING TxRing[NUM_OF_TX_RING];

	/* Shared memory for RX descriptors */
	struct RTMP_DMABUF RxDescRing[NUM_OF_RX_RING];
	struct RTMP_RX_RING RxRing[NUM_OF_RX_RING];

#if (CFG_SUPPORT_HOST_OFFLOAD == 1)
	/* MAWD */
	struct RTMP_DMABUF HifTxDescRing[NUM_OF_TX_RING];
	struct RTMP_TX_RING MawdTxRing[NUM_OF_TX_RING];
	struct RTMP_DMABUF ErrRptRing;
	uint32_t u4MawdL2TblCnt;

	/* RRO */
	struct RTMP_DMABUF RxBlkDescRing;
	struct RTMP_RX_RING RxBlkRing;
	struct RTMP_DMABUF BaBitmapCache;
	struct RTMP_DMABUF AddrArray;
	struct RTMP_DMABUF IndCmdRing;
	struct RTMP_DMABUF AckSnCmdRing;
	struct list_head rRcbUsedList[NUM_OF_RX_RING];
	uint32_t u4RcbUsedListCnt[NUM_OF_RX_RING];
	struct list_head rRcbFreeList;
	uint32_t u4RcbFreeListCnt;
	struct hlist_head arRcbHTbl[RRO_PREALLOC_RX_BUF_NUM];
	struct hlist_head rRcbHTblFreeList;
	uint32_t u4RroMagicCnt;
	uint32_t u4IndCmdDmaIdx;
	uint32_t u4OffloadIntStatus;
#endif /* CFG_SUPPORT_HOST_OFFLOAD == 1 */

	u_int8_t fgIntReadClear;
	u_int8_t fgMbxReadClear;

	uint32_t u4IntStatus;
	unsigned long ulIntFlag;

	struct MSDU_TOKEN_INFO rTokenInfo;

	struct ERR_RECOVERY_CTRL_T rErrRecoveryCtl;
	struct timer_list rSerTimer;
	unsigned long rSerTimerData;
	struct list_head rTxCmdQ;
	spinlock_t rTxCmdQLock;
	struct list_head rTxDataQ[NUM_OF_TX_RING];
	uint32_t u4TxDataQLen[NUM_OF_TX_RING];
	spinlock_t rTxDataQLock[NUM_OF_TX_RING];

	bool fgIsPowerOff;
	bool fgIsDumpLog;
	bool fgIsEnTxRingSizeCtrl;

	uint32_t u4WakeupIntSta;
	bool fgIsBackupIntSta;

	enum pcie_suspend_state eSuspendtate;
};

struct BUS_INFO {
	const uint32_t top_cfg_base;	/* TOP_CFG_BASE address */
	const struct PCIE_CHIP_CR_MAPPING *bus2chip;
	const struct PCIE_CHIP_CR_REMAPPING *bus2chip_remap;
	const uint32_t tx_ring_cmd_idx;
	const uint32_t tx_ring_wa_cmd_idx;
	const uint32_t tx_ring_fwdl_idx;
	const uint32_t tx_ring0_data_idx;
	const uint32_t tx_ring1_data_idx;
	const uint32_t tx_ring2_data_idx;
	const uint32_t tx_ring3_data_idx;
	const unsigned int max_static_map_addr;
	const uint32_t fw_own_clear_addr;
	const uint32_t fw_own_clear_bit;
	const bool fgCheckDriverOwnInt;
	const uint32_t u4DmaMask;
	/* host pdma/wfdma0 base address */
	const uint32_t host_dma0_base;
	/* host wfdma1 base address */
	const uint32_t host_dma1_base;
	/* host ext conn hif wrap */
	const uint32_t host_ext_conn_hif_wrap_base;
	const uint32_t host_int_status_addr;
	const uint32_t host_int_txdone_bits;
	const uint32_t host_int_rxdone_bits;

	/* tx pdma/wfdma ring base address */
	const uint32_t host_tx_ring_base;
	/* tx pdma/wfdma ring ext control base address */
	const uint32_t host_tx_ring_ext_ctrl_base;
	/* tx pdma/wfdma ring cpu index address */
	const uint32_t host_tx_ring_cidx_addr;
	/* tx pdma/wfdma ring dma index address */
	const uint32_t host_tx_ring_didx_addr;
	/* tx pdma/wfdma ring count address */
	const uint32_t host_tx_ring_cnt_addr;

	/* rx pdma/wfdma ring base address */
	const uint32_t host_rx_ring_base;
	/* rx pdma/wfdma ring ext control base address */
	const uint32_t host_rx_ring_ext_ctrl_base;
	/* rx pdma/wfdma ring cpu index address */
	const uint32_t host_rx_ring_cidx_addr;
	/* rx pdma/wfdma ring dma index address */
	const uint32_t host_rx_ring_didx_addr;
	/* rx pdma/wfdma ring count address */
	const uint32_t host_rx_ring_cnt_addr;

#if (CFG_SUPPORT_CONNAC2X == 1)
	/* rx wfdma_1 ring base address */
	const uint32_t host_wfdma1_rx_ring_base;
	/* rx wfdma_1 ring cpu index address */
	const uint32_t host_wfdma1_rx_ring_cidx_addr;
	/* rx wfdma_1 ring dma index address */
	const uint32_t host_wfdma1_rx_ring_didx_addr;
	/* rx wfdma_1 ring count address */
	const uint32_t host_wfdma1_rx_ring_cnt_addr;
	/* rx wfdma_1 ring ext control base address */
	const uint32_t host_wfdma1_rx_ring_ext_ctrl_base;
#endif /* CFG_SUPPORT_CONNAC2X == 1 */

#if (CFG_SUPPORT_HOST_OFFLOAD == 1)
	/* MAWD */
	const uint32_t mawd_array_base_l;
	const uint32_t mawd_array_base_m;
	const uint32_t mawd_rx_blk_ctrl0;
	const uint32_t mawd_rx_blk_ctrl1;
	const uint32_t mawd_rx_blk_ctrl2;
	const uint32_t mawd_ring_ctrl0;
	const uint32_t mawd_ring_ctrl1;
	const uint32_t mawd_ring_ctrl2;
	const uint32_t mawd_ring_ctrl3;
	const uint32_t mawd_ring_ctrl4;
	const uint32_t mawd_hif_txd_ctrl0;
	const uint32_t mawd_hif_txd_ctrl1;
	const uint32_t mawd_hif_txd_ctrl2;
	const uint32_t mawd_err_rpt_ctrl0;
	const uint32_t mawd_err_rpt_ctrl1;
	const uint32_t mawd_err_rpt_ctrl2;
	const uint32_t mawd_settings0;
	const uint32_t mawd_settings1;
	const uint32_t mawd_settings2;
	const uint32_t mawd_settings3;
	const uint32_t mawd_settings4;
	const uint32_t mawd_settings5;
	const uint32_t mawd_settings6;
#endif /* CFG_SUPPORT_HOST_OFFLOAD == 1 */

	struct wfdma_group_info *wfmda_host_tx_group;
	const uint32_t wfmda_host_tx_group_len;
	struct wfdma_group_info *wfmda_host_rx_group;
	const uint32_t wfmda_host_rx_group_len;
	struct wfdma_group_info *wfmda_wm_tx_group;
	const uint32_t wfmda_wm_tx_group_len;
	struct wfdma_group_info *wfmda_wm_rx_group;
	const uint32_t wfmda_wm_rx_group_len;

	struct DMASHDL_CFG *prDmashdlCfg;
	struct PLE_TOP_CR *prPleTopCr;
	struct PSE_TOP_CR *prPseTopCr;
	struct PP_TOP_CR *prPpTopCr;
	struct pse_group_info *prPseGroup;
	const uint32_t u4PseGroupLen;
	struct pcie_msi_info pcie_msi_info;

	void (*pdmaSetup)(struct GLUE_INFO *prGlueInfo, u_int8_t enable,
		bool fgResetHif);
	uint32_t (*updateTxRingMaxQuota)(struct ADAPTER *prAdapter,
		uint8_t ucWmmIndex, uint32_t u4MaxQuota);
	void (*pdmaStop)(struct GLUE_INFO *prGlueInfo, u_int8_t enable);
	u_int8_t (*pdmaPollingIdle)(struct GLUE_INFO *prGlueInfo);
	void (*enableInterrupt)(struct ADAPTER *prAdapter);
	void (*disableInterrupt)(struct ADAPTER *prAdapter);
	void (*disableSwInterrupt)(struct ADAPTER *prAdapter);
	void (*processTxInterrupt)(struct ADAPTER *prAdapter);
	void (*processRxInterrupt)(struct ADAPTER *prAdapter);
	void (*processAbnormalInterrupt)(struct ADAPTER *prAdapter);
	void (*lowPowerOwnRead)(struct ADAPTER *prAdapter, u_int8_t *pfgResult);
	void (*lowPowerOwnSet)(struct ADAPTER *prAdapter, u_int8_t *pfgResult);
	void (*lowPowerOwnClear)(struct ADAPTER *prAdapter,
		u_int8_t *pfgResult);
	void (*wakeUpWiFi)(struct ADAPTER *prAdapter);
	bool (*isValidRegAccess)(struct ADAPTER *prAdapter,
				 uint32_t u4Register);
	void (*getMailboxStatus)(struct ADAPTER *prAdapter, uint32_t *pu4Val);
	void (*setDummyReg)(struct GLUE_INFO *prGlueInfo);
	void (*checkDummyReg)(struct GLUE_INFO *prGlueInfo);
	void (*tx_ring_ext_ctrl)(struct GLUE_INFO *prGlueInfo,
		struct RTMP_TX_RING *tx_ring, uint32_t index);
	void (*rx_ring_ext_ctrl)(struct GLUE_INFO *prGlueInfo,
		struct RTMP_RX_RING *rx_ring, uint32_t index);
	void (*wfdmaManualPrefetch)(struct GLUE_INFO *prGlueInfo);
	void (*processSoftwareInterrupt)(IN struct ADAPTER *prAdapter);
	void (*softwareInterruptMcu)(IN struct ADAPTER *prAdapter,
		u_int32_t intrBitMask);
	void (*hifRst)(struct GLUE_INFO *prGlueInfo);
	void (*initPcieInt)(struct GLUE_INFO *prGlueInfo);
	void (*devReadIntStatus)(struct ADAPTER *prAdapter,
		OUT uint32_t *pu4IntStatus);
	/* Do DMASDHL init when WIFISYS is initialized at probe, L0.5 reset,
	 * etc.
	 */
	void (*DmaShdlInit)(IN struct ADAPTER *prAdapter);
	/* Although DMASHDL was init, we need to reinit it again due to falcon
	 * L1 reset, etc. Take MT7961 as example. The difference between
	 * mt7961DmashdlInit and mt7961DmashdlReInit is that we don't init CRs
	 * such as refill, min_quota, max_quota in mt7961DmashdlReInit, which
	 * are backup and restored in fw. The reason why some DMASHDL CRs are
	 * reinit by driver and some by fw is
	 *     1. Some DMASHDL CRs shall be inited before fw releases UMAC reset
	 *        in L1 procedure. Then, these CRs are backup and restored by fw
	 *     2. However, the backup and restore of each DMASHDL CR in fw needs
	 *        wm DLM space. So, we save DLM space by reinit the remaining
	 *        DMASHDL CRs in driver.
	 */
	void (*DmaShdlReInit)(IN struct ADAPTER *prAdapter);
	uint8_t (*setRxRingHwAddr)(struct RTMP_RX_RING *prRxRing,
		struct BUS_INFO *prBusInfo,
		uint32_t u4SwRingIdx);
	bool (*wfdmaAllocRxRing)(
		struct GLUE_INFO *prGlueInfo,
		bool fgAllocMem);
	void (*setDmaIntMask)(struct GLUE_INFO *prGlueInfo,
		uint8_t ucType, u_int8_t fgEnable);
	void (*enableFwDlMode)(struct ADAPTER *prAdapter);
	void (*setupMcuEmiAddr)(struct ADAPTER *prAdapter);

	struct SW_WFDMA_INFO rSwWfdmaInfo;

#if (CFG_COALESCING_INTERRUPT == 1)
	uint32_t (*setWfdmaCoalescingInt)(struct ADAPTER *prAdapter,
		u_int8_t fgEnable);
#endif

#if CFG_SUPPORT_HOST_RX_WM_EVENT_FROM_PSE
	u_int8_t (*checkPortForRxEventFromPse)(struct ADAPTER *prAdapter,
		uint8_t u2Port);
#endif
};

struct HIF_PREALLOC_MEM {
	struct HIF_MEM rTxDesc[NUM_OF_TX_RING];
	struct HIF_MEM rRxDesc[NUM_OF_RX_RING];
	/* Tx Command */
	struct HIF_MEM rTxCmdBuf[TX_RING_SIZE];
	/* FWDL */
	struct HIF_MEM rTxFwdlBuf[TX_RING_SIZE];
	/* Rx Data */
	struct HIF_MEM rRxDataBuf[RX_RING0_SIZE];
	/* Rx Event */
	struct HIF_MEM rRxEventBuf[RX_RING1_SIZE];

#if (CFG_SUPPORT_CONNAC2X == 1 || CFG_SUPPORT_CONNAC3X == 1)
	/* Connac1.0 = RX Event, Connac2.0 = Rx Data band1 */
	struct HIF_MEM rRxData1Buf[RX_RING0_SIZE];
	struct HIF_MEM rRxData2Buf[RX_RING0_SIZE];
	/* Band 0 TxFreeDoneEvent */
	struct HIF_MEM rTxFreeDoneEvent0Buf[RX_RING1_SIZE];
	/* Band 1 TxFreeDoneEvent */
	struct HIF_MEM rTxFreeDoneEvent1Buf[RX_RING1_SIZE];
	struct HIF_MEM rTxFreeDoneEvent2Buf[RX_RING1_SIZE];
#else
	/* Connac1.0 = RX Event, Connac2.0 = Rx Data band1 */
#endif /* CFG_SUPPORT_CONNAC2X == 1 */

#if HIF_TX_PREALLOC_DATA_BUFFER
	/* Tx Data */
	struct HIF_MEM rMsduBuf[HIF_TX_MSDU_TOKEN_NUM];
#endif
	phys_addr_t pucRsvMemBase;
	void *pucRsvMemVirBase;
	uint64_t u4RsvMemSize;
	uint32_t u4Offset;
};

/*******************************************************************************
 *                            P U B L I C   D A T A
 *******************************************************************************
 */
extern struct platform_device *g_prPlatDev;

/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */

/*******************************************************************************
 *                                 M A C R O S
 *******************************************************************************
 */

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

void glResetHifInfo(struct GLUE_INFO *prGlueInfo);

u_int8_t glBusInit(void *pvData);

void glBusRelease(void *pData);

int32_t glBusSetIrq(void *pvData, void *pfnIsr, void *pvCookie);

void glBusFreeIrq(void *pvData, void *pvCookie);

void glSetPowerState(IN struct GLUE_INFO *prGlueInfo, IN uint32_t ePowerMode);

void glGetDev(void *ctx, struct device **dev);

void glGetHifDev(struct GL_HIF_INFO *prHif, struct device **dev);

struct mt66xx_hif_driver_data *get_platform_driver_data(void);

void glGetChipInfo(void **prChipInfo);
void halPciePreSuspendDone(IN struct ADAPTER *prAdapter,
	IN struct CMD_INFO *prCmdInfo, IN uint8_t *pucEventBuf);
void halPciePreSuspendTimeout(IN struct ADAPTER *prAdapter,
	IN struct CMD_INFO *prCmdInfo);

int32_t glBusFunOn(void);
void glBusFunOff(void);

irqreturn_t mtk_pci_interrupt(int irq, void *dev_instance);
irqreturn_t pcie_sw_int_top_handler(int irq, void *dev_instance);
irqreturn_t pcie_sw_int_thread_handler(int irq, void *dev_instance);

#if CFG_SUPPORT_PCIE_ASPM
bool glBusConfigASPM(struct pci_dev *dev, int val);
bool glBusConfigASPML1SS(struct pci_dev *dev, int enable);
#endif

#if IS_ENABLED(CFG_MTK_PCIE_PROBE_SUPPORT)
extern int mtk_pcie_probe_port(int port) __attribute__((weak));
extern int mtk_pcie_remove_port(int port) __attribute__((weak));
#endif

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */
#endif /* _HIF_H */
