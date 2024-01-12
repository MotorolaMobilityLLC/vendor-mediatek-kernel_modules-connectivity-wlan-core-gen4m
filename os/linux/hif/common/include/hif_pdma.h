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
/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

 Module Name:
 hif_pdma.h
 */

#ifndef __HIF_PDMA_H__
#define __HIF_PDMA_H__

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */

#define NUM_OF_TX_RING				4
#define NUM_OF_RX_RING				2

#define TX_RING_SIZE				4095
/* Max Rx ring size */
#define RX_RING_SIZE				256

/* Data Rx ring */
#define RX_RING0_SIZE				256
/* Event/MSDU_report Rx ring */
#define RX_RING1_SIZE				16

/* TXD_SIZE = TxD + TxInfo */
#define TXD_SIZE					16
#define RXD_SIZE					16

#define RX_BUFFER_AGGRESIZE			3840
#define RX_BUFFER_NORMSIZE			3840
#define TX_BUFFER_NORMSIZE			3840


#define HIF_TX_PREALLOC_DATA_BUFFER			1

#define HIF_NUM_OF_QM_RX_PKT_NUM			4096
#define HIF_IST_LOOP_COUNT					32
/* Min msdu count to trigger Tx during INT polling state */
#define HIF_IST_TX_THRESHOLD				1

#define HIF_TX_BUFF_COUNT_TC0				4096
#define HIF_TX_BUFF_COUNT_TC1				4096
#define HIF_TX_BUFF_COUNT_TC2				4096
#define HIF_TX_BUFF_COUNT_TC3				4096
#define HIF_TX_BUFF_COUNT_TC4				(TX_RING_SIZE - 1)
#define HIF_TX_BUFF_COUNT_TC5				4096

/* enable/disable TX resource control */
#define HIF_TX_RESOURCE_CTRL                1
/* enable/disable TX resource control PLE */
#define HIF_TX_RESOURCE_CTRL_PLE            0


#define HIF_TX_PAGE_SIZE_IN_POWER_OF_2		11
/* in unit of bytes */
#define HIF_TX_PAGE_SIZE					2048

#define HIF_EXTRA_IO_BUFFER_SIZE			0

#define HIF_TX_COALESCING_BUFFER_SIZE		(TX_BUFFER_NORMSIZE)
#define HIF_RX_COALESCING_BUFFER_SIZE		(RX_BUFFER_AGGRESIZE)

#define HIF_CR4_FWDL_SECTION_NUM			1
#define HIF_IMG_DL_STATUS_PORT_IDX			1

#define HIF_TX_INIT_CMD_PORT				TX_RING_FWDL_IDX_3

#define HIF_TX_MSDU_TOKEN_NUM				8192

#define HIF_TX_PAYLOAD_LENGTH				72

#define HIF_SER_TIMEOUT				10000

#define MAX_PCIE_BUS_STATIC_MAP_ADDR		0x00040000

#define MT_RINGREG_DIFF		0x10
#define MT_RINGREG_EXT_DIFF	0x04

#define MT_TX_RING_BASE		WPDMA_TX_RING0_CTRL0
#define MT_TX_RING_PTR		WPDMA_TX_RING0_CTRL0
#define MT_TX_RING_CNT		WPDMA_TX_RING0_CTRL1
#define MT_TX_RING_CIDX		WPDMA_TX_RING0_CTRL2
#define MT_TX_RING_DIDX		WPDMA_TX_RING0_CTRL3

#define MT_RX_RING_BASE		WPDMA_RX_RING0_CTRL0
#define MT_RX_RING_PTR		WPDMA_RX_RING0_CTRL0
#define MT_RX_RING_CNT		WPDMA_RX_RING0_CTRL1
#define MT_RX_RING_CIDX		WPDMA_RX_RING0_CTRL2
#define MT_RX_RING_DIDX		WPDMA_RX_RING0_CTRL3

#define DMA_LOWER_32BITS_MASK   0x00000000FFFFFFFF
#define DMA_HIGHER_4BITS_MASK   0x0000000F
#define DMA_BITS_OFFSET		32

#define MT_TX_RING_BASE_EXT WPDMA_TX_RING0_BASE_PTR_EXT
#define MT_RX_RING_BASE_EXT WPDMA_RX_RING0_BASE_PTR_EXT

/*******************************************************************************
 *                                 M A C R O S
 *******************************************************************************
 */

#define INC_RING_INDEX(_idx, _RingSize)		\
{ \
	(_idx) = (_idx+1) % (_RingSize); \
}

#define RTMP_IO_READ32(_A, _R, _pV) \
{ \
	(*(_pV) = readl((void *)((_A)->CSRBaseAddress + (_R)))); \
}

#define RTMP_IO_WRITE32(_A, _R, _V) \
{ \
	writel(_V, (void *)((_A)->CSRBaseAddress + (_R))); \
}

/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */

enum ENUM_TX_RING_IDX {
	TX_RING_DATA0_IDX_0 = 0,
	TX_RING_DATA1_IDX_1,
	TX_RING_CMD_IDX_2,
	TX_RING_FWDL_IDX_3,
};

enum ENUM_RX_RING_IDX {
	RX_RING_DATA_IDX_0 = 0,
	RX_RING_EVT_IDX_1
};

/* ============================================================================
 * PCI/RBUS TX / RX Frame Descriptors format
 *
 * Memory Layout
 *
 * 1. Tx Descriptor
 * TxD (12 bytes) + TXINFO (4 bytes)
 * 2. Packet Buffer
 * TXWI + 802.11
 * 31                                                                         0
 * +--------------------------------------------------------------------------+
 * |                                   SDP0[31:0]                             |
 * +-+--+---------------------+-+--+------------------------------------------+
 * |D |L0|       SDL0[13:0]              |B|L1|                    SDL1[13:0] |
 * +-+--+---------------------+-+--+------------------------------------------+
 * |                                   SDP1[31:0]                             |
 * +--------------------------------------------------------------------------+
 * |                                  TX / RX INFO                            |
 * +--------------------------------------------------------------------------+
 * =========================================================================
 */
/*
 *  TX descriptor format for Tx Data/Mgmt Rings
 */
struct TXD_STRUCT {
	/* Word 0 */
	uint32_t SDPtr0;

	/* Word 1 */
	uint32_t SDLen1:14;
	uint32_t LastSec1:1;
	uint32_t Burst:1;
	uint32_t SDLen0:14;
	uint32_t LastSec0:1;
	uint32_t DMADONE:1;

	/*Word2 */
	uint32_t SDPtr1;

	/*Word3 */
	uint16_t SDPtr0Ext;
	uint16_t SDPtr1Ext;
};

/*
 *  Rx descriptor format for Rx Rings
 */
struct RXD_STRUCT {
	/* Word 0 */
	uint32_t SDPtr0;

	/* Word 1 */
	uint32_t SDLen1:14;
	uint32_t LastSec1:1;
	uint32_t Burst:1;
	uint32_t SDLen0:14;
	uint32_t LastSec0:1;
	uint32_t DMADONE:1;

	/* Word 2 */
	uint32_t SDPtr1;

	/* Word 3 */
	uint32_t RXINFO;
};

/*
 *	Data buffer for DMA operation, the buffer must be contiguous
 *	physical memory Both DMA to / from CPU use the same structure.
 */
struct RTMP_DMABUF {
	unsigned long AllocSize;
	void *AllocVa;		/* TxBuf virtual address */
	dma_addr_t AllocPa;	/* TxBuf physical address */
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
	void *AllocVa;		/* Control block virtual address */
	dma_addr_t AllocPa;	/* Control block physical address */
	void *pPacket;
	void *pBuffer;
	dma_addr_t PacketPa;
	struct RTMP_DMABUF DmaBuf;	/* Associated DMA buffer structure */
};

struct RTMP_TX_RING {
	struct RTMP_DMACB Cell[TX_RING_SIZE];
	uint32_t TxCpuIdx;
	uint32_t TxDmaIdx;
	uint32_t TxSwUsedIdx;
	uint32_t u4UsedCnt;
	uint32_t hw_desc_base;
	uint32_t hw_desc_base_ext;
	uint32_t hw_cidx_addr;
	uint32_t hw_didx_addr;
	uint32_t hw_cnt_addr;
};

struct RTMP_RX_RING {
	struct RTMP_DMACB Cell[RX_RING_SIZE];
	uint32_t RxCpuIdx;
	uint32_t RxDmaIdx;
	int32_t RxSwReadIdx;	/* software next read index */
	uint32_t u4BufSize;
	uint32_t u4RingSize;
	u_int8_t fgRxSegPkt;

	uint32_t hw_desc_base;
	uint32_t	hw_desc_base_ext;
	uint32_t hw_cidx_addr;
	uint32_t hw_didx_addr;
	uint32_t hw_cnt_addr;
};

struct PCIE_CHIP_CR_MAPPING {
	uint32_t u4ChipAddr;
	uint32_t u4BusAddr;
	uint32_t u4Range;
};


struct MSDU_TOKEN_ENTRY {
	uint32_t u4Token;
	u_int8_t fgInUsed;
	struct MSDU_INFO *prMsduInfo;
	void *prPacket;
	dma_addr_t rDmaAddr;
	uint32_t u4DmaLength;
	dma_addr_t rPktDmaAddr;
	uint32_t u4PktDmaLength;
};

struct MSDU_TOKEN_INFO {
	int32_t i4UsedCnt;
	struct MSDU_TOKEN_ENTRY *aprTokenStack[HIF_TX_MSDU_TOKEN_NUM];
	spinlock_t rTokenLock;
	struct MSDU_TOKEN_ENTRY arToken[HIF_TX_MSDU_TOKEN_NUM];
};

struct TX_CMD_REQ_T {
	struct CMD_INFO *prCmdInfo;
	uint8_t ucTC;
	struct list_head list;
};

struct TX_DATA_REQ_T {
	struct MSDU_INFO *prMsduInfo;
	struct list_head list;
};

enum ERR_RECOVERY_STATE {
	ERR_RECOV_STOP_IDLE = 0,
	ERR_RECOV_STOP_PDMA0,
	ERR_RECOV_RESET_PDMA0,
	ERR_RECOV_STOP_IDLE_DONE,
	ERR_RECOV_WAIT_N9_NORMAL,
	ERR_RECOV_EVENT_REENTRY,
	ERR_RECOV_STATE_NUM
};

struct ERR_RECOVERY_CTRL_T {
	enum ERR_RECOVERY_STATE eErrRecovState;
	uint32_t u4Status;
};

#endif /* HIF_PDMA_H__ */
