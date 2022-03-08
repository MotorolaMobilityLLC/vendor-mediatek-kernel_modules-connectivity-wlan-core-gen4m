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

#if defined(_HIF_NONE)
#define HIF_NAME "NONE"
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
#define SDO_HIF_TXD_SIZE         76
#define SDO_PARTIAL_PAYLOAD_SIZE 72
#define NUM_OF_WFDMA1_TX_RING			0

#if (CFG_SUPPORT_CONNAC2X == 1 || CFG_SUPPORT_CONNAC3X == 1)
#undef NUM_OF_WFDMA1_TX_RING
#ifdef CONFIG_NUM_OF_WFDMA_TX_RING
#define NUM_OF_WFDMA1_TX_RING			(CONFIG_NUM_OF_WFDMA_TX_RING)
#else
#define NUM_OF_WFDMA1_TX_RING			1  /* WA CMD Ring */
#endif
#endif

#define NUM_OF_TX_RING				(5+NUM_OF_WFDMA1_TX_RING)
#ifdef CONFIG_MTK_WIFI_HE160
#define TX_RING_SIZE				1024
#define RX_RING_SIZE				1024 /* Max Rx ring size */
/* Data Rx ring */
#define RX_RING0_SIZE				1024
/* Event/MSDU_report Rx ring */
#define RX_RING1_SIZE				128
#elif defined(CONFIG_MTK_WIFI_HE80)
#define TX_RING_SIZE				1024
#define RX_RING_SIZE				1024 /* Max Rx ring size */
/* Data Rx ring */
#define RX_RING0_SIZE				1024
/* Event/MSDU_report Rx ring */
#define RX_RING1_SIZE				16
#elif defined(CONFIG_MTK_WIFI_VHT80)
#define TX_RING_SIZE				512
#define RX_RING_SIZE				512	/* Max Rx ring size */
/* Data Rx ring */
#define RX_RING0_SIZE				512
/* Event/MSDU_report Rx ring */
#define RX_RING1_SIZE				16
#else
#define TX_RING_SIZE				256
#define RX_RING_SIZE				256	/* Max Rx ring size */
/* Data Rx ring */
#define RX_RING0_SIZE				256
/* Event/MSDU_report Rx ring */
#define RX_RING1_SIZE				16
#endif

#define RXD_SIZE					16
#define RX_BUFFER_AGGRESIZE			3840

/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */

struct GL_HIF_INFO;

/*
 *	Data buffer for DMA operation, the buffer must be contiguous
 *	physical memory Both DMA to / from CPU use the same structure.
 */
struct RTMP_DMABUF {
	unsigned long AllocSize;
	void *AllocVa;		/* TxBuf virtual address */
	phys_addr_t AllocPa;		/* TxBuf physical address */
};

/*
 *	Control block (Descriptor) for all ring descriptor DMA operation,
 *	buffer must be contiguous physical memory. NDIS_PACKET stored the
 *	binding Rx packet descriptor which won't be released, driver has to
 *	wait until upper layer return the packet before giveing up this rx
 *	ring descriptor to ASIC. NDIS_BUFFER is assocaited pair to describe
 *	the packet buffer. For Tx, NDIS_PACKET stored the tx packet descriptor
 *  which driver should ACK upper layer when the tx is physically done or
 *  failed.
 */
struct RTMP_DMACB {
	unsigned long AllocSize;	/* Control block size */
	void *AllocVa;			/* Control block virtual address */
	phys_addr_t AllocPa;	        /* Control block physical address */
	void *pPacket;
	void *pBuffer;
	phys_addr_t PacketPa;
	struct RTMP_DMABUF DmaBuf;	/* Associated DMA buffer structure */
	struct MSDU_TOKEN_ENTRY *prToken;
};

struct RTMP_TX_RING {
	struct RTMP_DMACB Cell[TX_RING_SIZE];
	uint32_t TxCpuIdx;
	uint32_t TxDmaIdx;
	uint32_t u4BufSize;
	uint32_t TxSwUsedIdx;
	uint32_t u4UsedCnt;
	uint32_t hw_desc_base;
	uint32_t hw_desc_base_ext;
	uint32_t hw_cidx_addr;
	uint32_t hw_cidx_mask;
	uint32_t hw_cidx_shift;
	uint32_t hw_didx_addr;
	uint32_t hw_didx_mask;
	uint32_t hw_didx_shift;
	uint32_t hw_cnt_addr;
	uint32_t hw_cnt_mask;
	uint32_t hw_cnt_shift;
};

/* host interface's private data structure, which is attached to os glue
 ** layer info structure.
 */
struct GL_HIF_INFO {
	uint32_t u4MawdL2TblCnt;
#if (CFG_SUPPORT_HOST_OFFLOAD == 1)
	/* MAWD */
	struct RTMP_TX_RING MawdTxRing[NUM_OF_TX_RING];
#endif
	uint32_t u4IntStatus;
	uint32_t u4MawdIntStatus;
};

struct BUS_INFO {
	void (*processAbnormalInterrupt)(struct ADAPTER *prAdapter);
	struct DMASHDL_CFG *prDmashdlCfg;
	const uint32_t host_int_rxdone_bits;
	const uint32_t host_int_txdone_bits;
	const uint32_t host_rx_ring_ext_ctrl_base;
};

#if (CFG_SUPPORT_HOST_OFFLOAD == 1)
union mawd_l2tbl {
	struct {
		uint8_t key_ip[16];
		uint8_t d_mac[MAC_ADDR_LEN];
		uint8_t s_mac[MAC_ADDR_LEN];
		uint32_t wlan_id:12;
		uint32_t bss_id:8;
		uint32_t reserved:12;
	} sram;

	uint32_t data[8];
};
#endif

struct RTMP_RX_RING {
	struct RTMP_DMACB Cell[RX_RING_SIZE];
	uint32_t RxCpuIdx;
	uint32_t RxDmaIdx;
	uint32_t u4BufSize;
	uint32_t u4RingSize;
	u_int8_t fgRxSegPkt;
	uint32_t hw_desc_base;
	uint32_t hw_desc_base_ext;
	uint32_t hw_cidx_addr;
	uint32_t hw_cidx_mask;
	uint32_t hw_cidx_shift;
	uint32_t hw_didx_addr;
	uint32_t hw_didx_mask;
	uint32_t hw_didx_shift;
	uint32_t hw_cnt_addr;
	uint32_t hw_cnt_mask;
	uint32_t hw_cnt_shift;
	bool fgIsDumpLog;
	uint32_t u4PendingCnt;
	void *pvPacket;
	uint32_t u4PacketLen;
};

struct pcie2ap_remap {
	uint32_t reg_base;
	uint32_t reg_mask;
	uint32_t reg_shift;
	uint32_t base_addr;
};

struct ap2wf_remap {
	uint32_t reg_base;
	uint32_t reg_mask;
	uint32_t reg_shift;
	uint32_t base_addr;
};

struct PCIE_CHIP_CR_REMAPPING {
	const struct pcie2ap_remap *pcie2ap;
	const struct ap2wf_remap *ap2wf;
};

/*******************************************************************************
 *                            P U B L I C   D A T A
 *******************************************************************************
 */
/* Common data type */
#define HIF_NUM_OF_QM_RX_PKT_NUM        512

/* chip dependent? used in wlanHarvardFormatDownload */
#define HIF_CR4_FWDL_SECTION_NUM            2
#define HIF_IMG_DL_STATUS_PORT_IDX          0

/* enable/disable TX resource control */
#define HIF_TX_RESOURCE_CTRL                1

/* enable/disable TX resource control PLE */
#define HIF_TX_RESOURCE_CTRL_PLE            0

#define HIF_IST_LOOP_COUNT              128

/* Min msdu count to trigger Tx during INT polling state */
#define HIF_IST_TX_THRESHOLD            32

#define HIF_TX_BUFF_COUNT_TC0               3
#define HIF_TX_BUFF_COUNT_TC1               3
#define HIF_TX_BUFF_COUNT_TC2               3
#define HIF_TX_BUFF_COUNT_TC3               3
#define HIF_TX_BUFF_COUNT_TC4               2
/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */

/*******************************************************************************
 *                                 M A C R O S
 *******************************************************************************
 */

/*******************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */
/* kal internal use */
#define glRegisterBus(_pfProbe, _pfRemove)

#define glUnregisterBus(_pfRemove)

#define glSetHifInfo(_prGlueInfo, _ulCookie)

#define glClearHifInfo(_prGlueInfo)

#define glResetHifInfo(_prGlueInfo)

#define glBusInit(_pvData)

#define glBusRelease(_pData)

#define glBusSetIrq(_pvData, _pfnIsr, _pvCookie)

#define glBusFreeIrq(_pvData, _pvCookie)

#define glSetPowerState(_prGlueInfo, _ePowerMode)

#define glGetDev(_prCtx, _ppDev)

#define glGetHifDev(_prHif, _ppDev)

#define HAL_WAKE_UP_WIFI(_prAdapter)

#define halWpdmaInitRing(_glueinfo, __fgResetHif) \
	KAL_NEED_IMPLEMENT(__FILE__, __func__, __LINE__)

uint8_t halTxRingDataSelect(IN struct ADAPTER *prAdapter,
	IN struct MSDU_INFO *prMsduInfo);

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */

/* for nic/hal.h */
/*
 * kal_virt_write_tx_port: Write frame to data port
 * @ad: structure for adapter private data
 * @pid: port id to write data, eg.NIC_TX_INIT_CMD_PORT
 * @len: data len to write
 * @buf: data buf pointer
 * @buf_size: maximum size for data buffer, buf_size >= len
 *
 * not: implementation for different HIF may refer to nic/hal.h
 */
void kal_virt_write_tx_port(struct ADAPTER *ad,
	uint16_t pid, uint32_t len, uint8_t *buf, uint32_t buf_size);

/*
 * kal_virt_get_wifi_func_stat: check HW status when check ready fail
 * @ad: structure for adapter private data
 * @res: status for return
 *
 * not: implementation for different HIF may refer to nic/hal.h
 */
void kal_virt_get_wifi_func_stat(struct ADAPTER *ad, uint32_t *res);

/*
 * kal_virt_chk_wifi_func_off: check HW status for function OFF
 * @ad: structure for adapter private data
 * @ready_bits: asserted bit if function is off correctly
 * @res: status for return
 *
 * not: implementation for different HIF may refer to nic/hal.h
 */
void kal_virt_chk_wifi_func_off(struct ADAPTER *ad, uint32_t ready_bits,
	uint8_t *res);

/*
 * kal_virt_chk_wifi_func_off: check HW status for function ready
 * @ad: structure for adapter private data
 * @ready_bits: asserted bit if function is ready
 * @res: status for return
 *
 * not: implementation for different HIF may refer to nic/hal.h
 */
void kal_virt_chk_wifi_func_ready(struct ADAPTER *ad, uint32_t ready_bits,
	uint8_t *res);

/*
 * kal_virt_set_mailbox_readclear: set read clear on hw mailbox (?)
 * @ad: structure for adapter private data
 * @enable: enable read clear or not
 *
 * not: implementation for different HIF may refer to nic/hal.h
 */
void kal_virt_set_mailbox_readclear(struct ADAPTER *ad, bool enable);

/*
 * kal_virt_set_int_stat_readclear: set hardware interrupt read clear
 * @ad: structure for adapter private data
 *
 * not: no disable command
 * not: implementation for different HIF may refer to nic/hal.h
 */
void kal_virt_set_int_stat_readclear(struct ADAPTER *ad);

/*
 * kal_virt_init_hif: do device related initialization
 * @ad: structure for adapter private data
 *
 * not: implementation for different HIF may refer to nic/hal.h
 */
void kal_virt_init_hif(struct ADAPTER *ad);

/*
 * kal_virt_enable_fwdl: enable firmware download
 * @ad: structure for adapter private data
 *
 * not: implementation for different HIF may refer to nic/hal.h
 */
void kal_virt_enable_fwdl(struct ADAPTER *ad, bool enable);

/*
 * kal_virt_get_int_status: read interrupt status
 * @ad: structure for adapter private data
 * @status: return value for interrupt status
 *
 * not: implementation for different HIF may refer to nic/hal.h
 */
void kal_virt_get_int_status(struct ADAPTER *ad, uint32_t *status);

/*
 * kal_virt_uhw_rd: read chip CR via USB UHW.
 * @ad: structure for adapter private data
 * @u4Offset: CR address
 * @pu4Value: return CR value
 * @pfgSts: return TRUE if IO operation is successful; otherwise, return FALSE
 *
 * note: implementation for different HIF may refer to nic/hal.h
 */
void kal_virt_uhw_rd(struct ADAPTER *ad, uint32_t u4Offset, uint32_t *pu4Value,
		     u_int8_t *pfgSts);

/*
 * kal_virt_uhw_wr: write chip CR via USB UHW.
 * @ad: structure for adapter private data
 * @u4Offset: CR address
 * @u4Value: CR value
 * @pfgSts: return TRUE if IO operation is successful; otherwise, return FALSE
 *
 * note: implementation for different HIF may refer to nic/hal.h
 */
void kal_virt_uhw_wr(struct ADAPTER *ad, uint32_t u4Offset, uint32_t u4Value,
		     u_int8_t *pfgSts);

#endif /* _HIF_H */
