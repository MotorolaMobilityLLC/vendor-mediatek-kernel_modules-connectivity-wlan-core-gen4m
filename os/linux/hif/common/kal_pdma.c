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
 *[File]             kal_pdma.c
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

/*******************************************************************************
 *                                 M A C R O S
 *******************************************************************************
 */

/*******************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */
static u_int8_t kalDevWriteCmdByQueue(IN struct GLUE_INFO *prGlueInfo,
	IN struct CMD_INFO *prCmdInfo, IN uint8_t ucTC);
static u_int8_t kalDevWriteDataByQueue(IN struct GLUE_INFO *prGlueInfo,
	IN struct MSDU_INFO *prMsduInfo);

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */

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
u_int8_t kalDevRegRead(IN struct GLUE_INFO *prGlueInfo,
	IN uint32_t u4Register, OUT uint32_t *pu4Value)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	uint32_t u4BusAddr = u4Register;
	u_int8_t fgResult = TRUE;

	ASSERT(prGlueInfo);
	ASSERT(pu4Value);

	prHifInfo = &prGlueInfo->rHifInfo;

	if (halChipToStaticMapBusAddr(prGlueInfo, u4Register, &u4BusAddr)) {
		/* Static mapping */
		RTMP_IO_READ32(prHifInfo, u4BusAddr, pu4Value);
	} else {
		/* Dynamic mapping */
		fgResult = halGetDynamicMapReg(prGlueInfo, u4BusAddr, pu4Value);
	}

	if ((u4Register & 0xFFFFF000) != PCIE_HIF_BASE)
		DBGLOG(HAL, TRACE, "Get CR[0x%08x/0x%08x] value[0x%08x]\n",
		       u4Register, u4BusAddr, *pu4Value);

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
u_int8_t kalDevRegWrite(IN struct GLUE_INFO *prGlueInfo,
	IN uint32_t u4Register, IN uint32_t u4Value)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	uint32_t u4BusAddr = u4Register;
	u_int8_t fgResult = TRUE;

	ASSERT(prGlueInfo);

	prHifInfo = &prGlueInfo->rHifInfo;

	if (halChipToStaticMapBusAddr(prGlueInfo, u4Register, &u4BusAddr)) {
		/* Static mapping */
		RTMP_IO_WRITE32(prHifInfo, u4BusAddr, u4Value);
	} else {
		/* Dynamic mapping */
		fgResult = halSetDynamicMapReg(prGlueInfo, u4BusAddr, u4Value);
	}

	if ((u4Register & 0xFFFFF000) != PCIE_HIF_BASE)
		DBGLOG(HAL, TRACE, "Set CR[0x%08x/0x%08x] value[0x%08x]\n",
		       u4Register, u4BusAddr, u4Value);

	prHifInfo->u4HifCnt++;

	return TRUE;
}

static bool kalWaitRxDmaDone(struct GLUE_INFO *prGlueInfo,
			     struct RTMP_RX_RING *prRxRing,
			     struct RXD_STRUCT *pRxD,
			     uint16_t u2Port)
{
	uint32_t u4Count = 0;

	for (u4Count = 0; pRxD->DMADONE == 0; u4Count++) {
		kalDevRegRead(prGlueInfo, prRxRing->hw_didx_addr,
			      &prRxRing->RxDmaIdx);
		DBGLOG(HAL, TRACE, "Rx DMA done P[%u] DMA[%u] CPU[%u]\n",
		       u2Port, prRxRing->RxDmaIdx, prRxRing->RxCpuIdx);
		if (u4Count > DMA_DONE_WAITING_COUNT)
			return false;

		kalMdelay(DMA_DONE_WAITING_TIME);
	}
	return true;
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
u_int8_t
kalDevPortRead(IN struct GLUE_INFO *prGlueInfo,
	IN uint16_t u2Port, IN uint32_t u4Len,
	OUT uint8_t *pucBuf, IN uint32_t u4ValidOutBufSize)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct RTMP_RX_RING *prRxRing;
	struct RTMP_DMACB *pRxCell;
	struct RXD_STRUCT *pRxD;
	struct RTMP_DMABUF *prDmaBuf;
	spinlock_t *pRxRingLock;
	u_int8_t fgRet = TRUE;
	unsigned long flags = 0;
	void *pRxPacket = NULL;
	uint8_t *pucDst = NULL;
	uint32_t u4CpuIdx = 0;

	ASSERT(prGlueInfo);
	prHifInfo = &prGlueInfo->rHifInfo;

	ASSERT(pucBuf);
	pucDst = pucBuf;

	ASSERT(u4Len <= u4ValidOutBufSize);

	prRxRing = &prHifInfo->RxRing[u2Port];
	pRxRingLock = &prHifInfo->RxRingLock[u2Port];

	spin_lock_irqsave(pRxRingLock, flags);

	kalDevRegRead(prGlueInfo, prRxRing->hw_cidx_addr, &prRxRing->RxCpuIdx);
	u4CpuIdx = prRxRing->RxCpuIdx;
	INC_RING_INDEX(u4CpuIdx, prRxRing->u4RingSize);

	pRxCell = &prRxRing->Cell[u4CpuIdx];
	pRxD = (struct RXD_STRUCT *)pRxCell->AllocVa;

	if (!kalWaitRxDmaDone(prGlueInfo, prRxRing, pRxD, u2Port)) {
		spin_unlock_irqrestore(pRxRingLock, flags);
		if (!prRxRing->fgIsDumpLog)
			kalDumpRxRingDebugLog(prGlueInfo, prRxRing,
					      prRxRing->u4RingSize);
		prRxRing->fgIsDumpLog = true;
		return FALSE;
	}

	prDmaBuf = &pRxCell->DmaBuf;

	if (pRxD->LastSec0 == 0 || prRxRing->fgRxSegPkt) {
		/* Rx segmented packet */
		DBGLOG(HAL, WARN,
			"Skip Rx segmented packet, SDL0[%u] LS0[%u]\n",
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

	KAL_DMA_UNMAP_SINGLE(prHifInfo->prDmaDev, prDmaBuf->AllocPa,
		prDmaBuf->AllocSize, KAL_DMA_FROM_DEVICE);

	if (pRxD->SDLen0 <= u4Len) {
		pRxPacket = pRxCell->pPacket;
		ASSERT(pRxPacket);
		kalMemCopy(pucDst, ((uint8_t *)
			((struct sk_buff *)(pRxPacket))->data), pRxD->SDLen0);
	} else
		DBGLOG(HAL, WARN,
			"Skip Rx packet, SDL0[%u] > SwRfb max len[%u]\n",
			pRxD->SDLen0, u4Len);

	DBGLOG(HAL, TRACE, "Rx Event\n");
	DBGLOG_MEM32(HAL, TRACE, ((struct sk_buff *)pRxCell->pPacket)->data,
		     pRxD->SDLen0);

	prDmaBuf->AllocVa = ((struct sk_buff *)pRxCell->pPacket)->data;
	prDmaBuf->AllocPa = KAL_DMA_MAP_SINGLE(prHifInfo->prDmaDev,
		prDmaBuf->AllocVa, prDmaBuf->AllocSize, KAL_DMA_FROM_DEVICE);
	if (KAL_DMA_MAPPING_ERROR(prHifInfo->prDmaDev, prDmaBuf->AllocPa)) {
		DBGLOG(HAL, ERROR, "KAL_DMA_MAP_SINGLE() error!\n");
		spin_unlock_irqrestore(pRxRingLock, flags);
		ASSERT(0);
		return FALSE;
	}

	pRxD->SDPtr0 = prDmaBuf->AllocPa & DMA_LOWER_32BITS_MASK;
	pRxD->SDPtr1 = ((uint64_t)prDmaBuf->AllocPa >>
		DMA_BITS_OFFSET) & DMA_HIGHER_4BITS_MASK;
skip:
	pRxD->SDLen0 = prRxRing->u4BufSize;
	pRxD->DMADONE = 0;

	prRxRing->RxCpuIdx = u4CpuIdx;
	kalDevRegWrite(prGlueInfo, prRxRing->hw_cidx_addr, prRxRing->RxCpuIdx);
	prRxRing->fgIsDumpLog = false;

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
u_int8_t
kalDevPortWrite(IN struct GLUE_INFO *prGlueInfo,
	IN uint16_t u2Port, IN uint32_t u4Len, IN uint8_t *pucBuf,
	IN uint32_t u4ValidInBufSize)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct RTMP_TX_RING *prTxRing;
	struct RTMP_DMACB *pTxCell;
	struct TXD_STRUCT *pTxD;
	uint8_t *pucSrc = NULL;
	uint32_t u4SrcLen = u4Len;
	unsigned long flags = 0;
	spinlock_t *prTxRingLock;

	ASSERT(prGlueInfo);
	prHifInfo = &prGlueInfo->rHifInfo;

	ASSERT(pucBuf);

	ASSERT(u4Len <= u4ValidInBufSize);

	pucSrc = kalMemAlloc(u4SrcLen, PHY_MEM_TYPE);
	ASSERT(pucSrc);

	kalMemCopy(pucSrc, pucBuf, u4SrcLen);

	prTxRing = &prHifInfo->TxRing[u2Port];
	prTxRingLock = &prHifInfo->TxRingLock[u2Port];

	spin_lock_irqsave((spinlock_t *)prTxRingLock, flags);

	kalDevRegRead(prGlueInfo, prTxRing->hw_cidx_addr, &prTxRing->TxCpuIdx);

	pTxCell = &prTxRing->Cell[prTxRing->TxCpuIdx];
	pTxD = (struct TXD_STRUCT *)pTxCell->AllocVa;

	pTxCell->pPacket = NULL;
	pTxCell->pBuffer = pucSrc;
	pTxCell->PacketPa = KAL_DMA_MAP_SINGLE(prHifInfo->prDmaDev,
					       pucSrc, u4SrcLen,
					       KAL_DMA_TO_DEVICE);
	if (KAL_DMA_MAPPING_ERROR(prHifInfo->prDmaDev, pTxCell->PacketPa)) {
		DBGLOG(HAL, ERROR, "KAL_DMA_MAP_SINGLE() error!\n");
		kalMemFree(pucSrc, PHY_MEM_TYPE, u4SrcLen);
		spin_unlock_irqrestore((spinlock_t *)prTxRingLock, flags);
		ASSERT(0);
		return FALSE;
	}

	pTxD->LastSec0 = 1;
	pTxD->LastSec1 = 0;
	pTxD->SDLen0 = u4SrcLen;
	pTxD->SDLen1 = 0;
	pTxD->SDPtr0 = pTxCell->PacketPa & DMA_LOWER_32BITS_MASK;
	pTxD->SDPtr0Ext = ((uint64_t)pTxCell->PacketPa >> DMA_BITS_OFFSET) &
		DMA_HIGHER_4BITS_MASK;
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

void kalDevReadIntStatus(IN struct ADAPTER *prAdapter,
	OUT uint32_t *pu4IntStatus)
{
	uint32_t u4RegValue;
	struct GL_HIF_INFO *prHifInfo = &prAdapter->prGlueInfo->rHifInfo;

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

u_int8_t kalDevWriteCmd(IN struct GLUE_INFO *prGlueInfo,
	IN struct CMD_INFO *prCmdInfo, IN uint8_t ucTC)
{
	struct GL_HIF_INFO *prHifInfo = NULL;

	ASSERT(prGlueInfo);
	prHifInfo = &prGlueInfo->rHifInfo;

	if (prHifInfo->fgIsErrRecovery)
		return kalDevWriteCmdByQueue(prGlueInfo, prCmdInfo, ucTC);

	return halWpdmaWriteCmd(prGlueInfo, prCmdInfo, ucTC);
}

static u_int8_t kalDevWriteCmdByQueue(IN struct GLUE_INFO *prGlueInfo,
	IN struct CMD_INFO *prCmdInfo, IN uint8_t ucTC)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	unsigned long flags = 0;
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

u_int8_t kalDevKickCmd(IN struct GLUE_INFO *prGlueInfo)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	unsigned long flags = 0;
	struct list_head *prCur, *prNext;
	struct TX_CMD_REQ_T *prTxReq;

	ASSERT(prGlueInfo);
	prHifInfo = &prGlueInfo->rHifInfo;

	spin_lock_irqsave(&prHifInfo->rTxCmdQLock, flags);

	list_for_each_safe(prCur, prNext, &prHifInfo->rTxCmdQ) {
		prTxReq = list_entry(prCur, struct TX_CMD_REQ_T, list);
		if (prTxReq->prCmdInfo)
			halWpdmaWriteCmd(prGlueInfo,
				prTxReq->prCmdInfo, prTxReq->ucTC);
		list_del(prCur);
		kfree(prTxReq);
	}

	spin_unlock_irqrestore(&prHifInfo->rTxCmdQLock, flags);

	return TRUE;
}

u_int8_t kalDevWriteData(IN struct GLUE_INFO *prGlueInfo,
	IN struct MSDU_INFO *prMsduInfo)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct BUS_INFO *prBusInfo = NULL;

	ASSERT(prGlueInfo);
	prHifInfo = &prGlueInfo->rHifInfo;
	prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;

	if (prHifInfo->fgIsErrRecovery)
		return kalDevWriteDataByQueue(prGlueInfo, prMsduInfo);

	return halWpdmaWriteData(prGlueInfo, prMsduInfo);
}

static u_int8_t kalDevWriteDataByQueue(IN struct GLUE_INFO *prGlueInfo,
	IN struct MSDU_INFO *prMsduInfo)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	unsigned long flags = 0;
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
u_int8_t kalDevKickData(IN struct GLUE_INFO *prGlueInfo)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	unsigned long flags = 0;
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

u_int8_t kalDevReadData(IN struct GLUE_INFO *prGlueInfo,
	IN uint16_t u2Port, IN OUT struct SW_RFB *prSwRfb)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct RXD_STRUCT *pRxD;
	struct RTMP_RX_RING *prRxRing;
	struct RTMP_DMACB *pRxCell;
	struct RTMP_DMABUF *prDmaBuf;
	spinlock_t *pRxRingLock;
	void *pRxPacket = NULL;
	u_int8_t fgRet = TRUE;
	unsigned long flags = 0;
	uint32_t u4CpuIdx = 0;

	ASSERT(prGlueInfo);
	prHifInfo = &prGlueInfo->rHifInfo;

	prRxRing = &prHifInfo->RxRing[u2Port];
	pRxRingLock = &prHifInfo->RxRingLock[u2Port];

	spin_lock_irqsave(pRxRingLock, flags);

	kalDevRegRead(prGlueInfo, prRxRing->hw_cidx_addr, &prRxRing->RxCpuIdx);
	u4CpuIdx = prRxRing->RxCpuIdx;
	INC_RING_INDEX(u4CpuIdx, prRxRing->u4RingSize);

	pRxCell = &prRxRing->Cell[u4CpuIdx];
	pRxD = (struct RXD_STRUCT *)pRxCell->AllocVa;

	if (!kalWaitRxDmaDone(prGlueInfo, prRxRing, pRxD, u2Port)) {
		spin_unlock_irqrestore(pRxRingLock, flags);
		if (!prRxRing->fgIsDumpLog)
			kalDumpRxRingDebugLog(prGlueInfo, prRxRing,
					      prRxRing->u4RingSize);
		prRxRing->fgIsDumpLog = true;
		return FALSE;
	}

	if (pRxD->LastSec0 == 0 || prRxRing->fgRxSegPkt) {
		/* Rx segmented packet */
		DBGLOG(HAL, WARN,
			"Skip Rx segmented data packet, SDL0[%u] LS0[%u]\n",
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
	if (prHifInfo->fgIsPreAllocMem)
		prHifInfo->updateRxPacket(pRxCell->pPacket, u2Port, u4CpuIdx);

	KAL_DMA_UNMAP_SINGLE(prHifInfo->prDmaDev, prDmaBuf->AllocPa,
		prDmaBuf->AllocSize, KAL_DMA_FROM_DEVICE);
	prSwRfb->pvPacket = pRxPacket;
	prSwRfb->pucRecvBuff = ((struct sk_buff *)pRxPacket)->data;
	prSwRfb->prRxStatus = (struct HW_MAC_RX_DESC *)prSwRfb->pucRecvBuff;

#if CFG_TCP_IP_CHKSUM_OFFLOAD
	prSwRfb->u4TcpUdpIpCksStatus = pRxD->RXINFO;
	DBGLOG(RX, TRACE, "u4TcpUdpIpCksStatus[0x%02x]\n",
		prSwRfb->u4TcpUdpIpCksStatus);
#endif /* CFG_TCP_IP_CHKSUM_OFFLOAD */

	prDmaBuf->AllocVa = ((struct sk_buff *)pRxCell->pPacket)->data;
	prDmaBuf->AllocPa = KAL_DMA_MAP_SINGLE(prHifInfo->prDmaDev,
		prDmaBuf->AllocVa, prDmaBuf->AllocSize, KAL_DMA_FROM_DEVICE);
	if (KAL_DMA_MAPPING_ERROR(prHifInfo->prDmaDev, prDmaBuf->AllocPa)) {
		DBGLOG(HAL, ERROR, "KAL_DMA_MAP_SINGLE() error!\n");
		spin_unlock_irqrestore(pRxRingLock, flags);
		ASSERT(0);
		return FALSE;
	}

	pRxD->SDPtr0 = prDmaBuf->AllocPa & DMA_LOWER_32BITS_MASK;
	pRxD->SDPtr1 = ((uint64_t)prDmaBuf->AllocPa >>
		DMA_BITS_OFFSET) & DMA_HIGHER_4BITS_MASK;
skip:
	pRxD->SDLen0 = prRxRing->u4BufSize;
	pRxD->DMADONE = 0;

	prRxRing->RxCpuIdx = u4CpuIdx;
	kalDevRegWrite(prGlueInfo, prRxRing->hw_cidx_addr, prRxRing->RxCpuIdx);
	prRxRing->fgIsDumpLog = false;

	spin_unlock_irqrestore(pRxRingLock, flags);

	return fgRet;
}


void kalPciUnmapToDev(IN struct GLUE_INFO *prGlueInfo,
	IN dma_addr_t rDmaAddr, IN uint32_t u4Length)
{
	struct GL_HIF_INFO *prHifInfo = NULL;

	prHifInfo = &prGlueInfo->rHifInfo;
	KAL_DMA_UNMAP_SINGLE(prHifInfo->prDmaDev,
		rDmaAddr, u4Length, KAL_DMA_TO_DEVICE);
}

void kalDumpTxRing(struct GLUE_INFO *prGlueInfo,
		   struct RTMP_TX_RING *prTxRing,
		   uint32_t u4Num, bool fgDumpContent)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct RTMP_DMACB *pTxCell;
	struct TXD_STRUCT *pTxD;
	struct RTMP_DMABUF *prDmaBuf;
	uint32_t u4DumpLen = 64;

	ASSERT(prGlueInfo);
	prHifInfo = &prGlueInfo->rHifInfo;

	if (u4Num >= TX_RING_SIZE)
		return;

	pTxCell = &prTxRing->Cell[u4Num];
	pTxD = (struct TXD_STRUCT *) pTxCell->AllocVa;

	prDmaBuf = &pTxCell->DmaBuf;

	DBGLOG(HAL, INFO, "Tx Dese Num[%u]\n", u4Num);
	DBGLOG_MEM32(HAL, INFO, pTxD, sizeof(struct TXD_STRUCT));

	if (!fgDumpContent || !prDmaBuf->AllocPa)
		return;

	KAL_DMA_UNMAP_SINGLE(prHifInfo->prDmaDev, prDmaBuf->AllocPa,
			     prDmaBuf->AllocSize, KAL_DMA_TO_DEVICE);

	DBGLOG(HAL, INFO, "Tx Contents\n");
	if (u4DumpLen > pTxD->SDLen0)
		u4DumpLen = pTxD->SDLen0;
	DBGLOG_MEM32(HAL, INFO, ((struct sk_buff *)pTxCell->pPacket)->data,
		     u4DumpLen);
	DBGLOG(HAL, INFO, "\n\n");

	prDmaBuf->AllocVa = ((struct sk_buff *)pTxCell->pPacket)->data;
	prDmaBuf->AllocPa = KAL_DMA_MAP_SINGLE(prHifInfo->prDmaDev,
					       prDmaBuf->AllocVa,
					       prDmaBuf->AllocSize,
					       KAL_DMA_TO_DEVICE);
	if (KAL_DMA_MAPPING_ERROR(prHifInfo->prDmaDev, prDmaBuf->AllocPa))
		DBGLOG(HAL, ERROR, "KAL_DMA_MAP_SINGLE() error!\n");
}

void kalDumpRxRing(struct GLUE_INFO *prGlueInfo,
		   struct RTMP_RX_RING *prRxRing,
		   uint32_t u4Num, bool fgDumpContent)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct RTMP_DMACB *pRxCell;
	struct RXD_STRUCT *pRxD;
	struct RTMP_DMABUF *prDmaBuf;
	uint32_t u4DumpLen = 64;

	ASSERT(prGlueInfo);
	prHifInfo = &prGlueInfo->rHifInfo;

	if (u4Num >= prRxRing->u4RingSize)
		return;

	pRxCell = &prRxRing->Cell[u4Num];
	pRxD = (struct RXD_STRUCT *) pRxCell->AllocVa;

	prDmaBuf = &pRxCell->DmaBuf;

	DBGLOG(HAL, INFO, "Rx Dese Num[%u]\n", u4Num);
	DBGLOG_MEM32(HAL, INFO, pRxD, sizeof(struct RXD_STRUCT));

	if (!fgDumpContent || !prDmaBuf->AllocPa)
		return;

	KAL_DMA_UNMAP_SINGLE(prHifInfo->prDmaDev, prDmaBuf->AllocPa,
			     prDmaBuf->AllocSize, KAL_DMA_FROM_DEVICE);

	DBGLOG(HAL, INFO, "Rx Contents\n");
	if (u4DumpLen > pRxD->SDLen0)
		u4DumpLen = pRxD->SDLen0;
	DBGLOG_MEM32(HAL, INFO, ((struct sk_buff *)pRxCell->pPacket)->data,
		     u4DumpLen);
	DBGLOG(HAL, INFO, "\n\n");

	prDmaBuf->AllocVa = ((struct sk_buff *)pRxCell->pPacket)->data;
	prDmaBuf->AllocPa = KAL_DMA_MAP_SINGLE(prHifInfo->prDmaDev,
					       prDmaBuf->AllocVa,
					       prDmaBuf->AllocSize,
					       KAL_DMA_FROM_DEVICE);
	if (KAL_DMA_MAPPING_ERROR(prHifInfo->prDmaDev, prDmaBuf->AllocPa))
		DBGLOG(HAL, ERROR, "KAL_DMA_MAP_SINGLE() error!\n");
}

void kalDumpRxRingDebugLog(struct GLUE_INFO *prGlueInfo,
			   struct RTMP_RX_RING *prRxRing,
			   uint32_t u4RingSize)
{
	uint32_t u4Index = 0;
	uint32_t u4Value = 0;

	halShowPseInfo(prGlueInfo->prAdapter);
	halShowPleInfo(prGlueInfo->prAdapter);
	halShowHostCsrInfo(prGlueInfo->prAdapter);
	halShowPdmaInfo(prGlueInfo->prAdapter, false);

	for (u4Index = 0; u4Index < u4RingSize; u4Index++)
		kalDumpRxRing(prGlueInfo, prRxRing, u4Index, true);

	kalDevRegRead(prGlueInfo, 0x0000C024, &u4Value);
	DBGLOG(HAL, INFO, "0x1800C024 = 0x%08x\n", u4Value);
	kalDevRegRead(prGlueInfo, 0x0000C028, &u4Value);
	DBGLOG(HAL, INFO, "0x1800C028 = 0x%08x\n", u4Value);
	kalDevRegRead(prGlueInfo, 0x0000C0B0, &u4Value);
	DBGLOG(HAL, INFO, "0x1800C0B0 = 0x%08x\n", u4Value);
	kalDevRegRead(prGlueInfo, 0x0000C1F0, &u4Value);
	DBGLOG(HAL, INFO, "0x1800C1F0 = 0x%08x\n", u4Value);
	kalDevRegRead(prGlueInfo, 0x0000C1F4, &u4Value);
	DBGLOG(HAL, INFO, "0x1800C1F4 = 0x%08x\n", u4Value);
	kalDevRegRead(prGlueInfo, 0x0000C1F8, &u4Value);
	DBGLOG(HAL, INFO, "0x1800C1F8 = 0x%08x\n", u4Value);
	kalDevRegRead(prGlueInfo, 0x000C1100, &u4Value);
	DBGLOG(HAL, INFO, "0x180C1100 = 0x%08x\n", u4Value);
	kalDevRegRead(prGlueInfo, 0x00002150, &u4Value);
	DBGLOG(HAL, INFO, "0x18002150 = 0x%08x\n", u4Value);
}
