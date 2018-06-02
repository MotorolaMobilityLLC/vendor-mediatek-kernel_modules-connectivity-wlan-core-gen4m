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
#include "pse.h"

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
	struct BUS_INFO *prBusInfo = NULL;
	uint32_t u4BusAddr = u4Register;

	ASSERT(prGlueInfo);
	ASSERT(pu4Value);

	prHifInfo = &prGlueInfo->rHifInfo;
	prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;

	if (!prHifInfo->fgIsDumpLog && prBusInfo->isValidRegAccess &&
	    !prBusInfo->isValidRegAccess(prGlueInfo->prAdapter, u4Register)) {
		DBGLOG(HAL, ERROR,
		       "Invalid access! Get CR[0x%08x/0x%08x] value[0x%08x]\n",
		       u4Register, u4BusAddr, *pu4Value);
		*pu4Value = 0xdeadfeed;
		return FALSE;
	}

	/* Static mapping */
	if (halChipToStaticMapBusAddr(prGlueInfo, u4Register, &u4BusAddr)) {
		RTMP_IO_READ32(prHifInfo, u4BusAddr, pu4Value);
	} else {
		DBGLOG(HAL, ERROR, "Not exist CR read[0x%08x]\n", u4Register);
	}

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
	struct BUS_INFO *prBusInfo = NULL;
	uint32_t u4BusAddr = u4Register;

	ASSERT(prGlueInfo);

	prHifInfo = &prGlueInfo->rHifInfo;
	prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;

	if (!prHifInfo->fgIsDumpLog && prBusInfo->isValidRegAccess &&
	    !prBusInfo->isValidRegAccess(prGlueInfo->prAdapter, u4Register)) {
		DBGLOG(HAL, ERROR,
		       "Invalid access! Set CR[0x%08x/0x%08x] value[0x%08x]\n",
		       u4Register, u4BusAddr, u4Value);
		return FALSE;
	}

	/* Static mapping */
	if (halChipToStaticMapBusAddr(prGlueInfo, u4Register, &u4BusAddr)) {
		RTMP_IO_WRITE32(prHifInfo, u4BusAddr, u4Value);
	} else {
		DBGLOG(HAL, ERROR, "Not exist CR write[0x%08x] value[0x%08x]\n",
		       u4Register, u4Value);
	}

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
		if (u4Count > DMA_DONE_WAITING_COUNT) {
			DBGLOG(HAL, INFO,
			       "Rx DMA done P[%u] DMA[%u] CPU[%u]\n",
			       u2Port, prRxRing->RxDmaIdx, prRxRing->RxCpuIdx);
			return false;
		}

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
u_int8_t kalDevPortRead(IN struct GLUE_INFO *prGlueInfo,
	IN uint16_t u2Port, IN uint32_t u4Len,
	OUT uint8_t *pucBuf, IN uint32_t u4ValidOutBufSize)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct HIF_MEM_OPS *prMemOps;
	struct RTMP_RX_RING *prRxRing;
	struct RTMP_DMACB *pRxCell;
	struct RXD_STRUCT *pRxD;
	struct RTMP_DMABUF *prDmaBuf;
	spinlock_t *pRxRingLock;
	u_int8_t fgRet = TRUE;
	unsigned long flags = 0;
	uint32_t u4CpuIdx = 0;

	ASSERT(prGlueInfo);
	ASSERT(pucBuf);
	ASSERT(u4Len <= u4ValidOutBufSize);

	prHifInfo = &prGlueInfo->rHifInfo;
	prMemOps = &prHifInfo->rMemOps;
	prRxRing = &prHifInfo->RxRing[u2Port];
	pRxRingLock = &prHifInfo->RxRingLock[u2Port];

	kalDevRegRead(prGlueInfo, prRxRing->hw_cidx_addr, &prRxRing->RxCpuIdx);
	u4CpuIdx = prRxRing->RxCpuIdx;
	INC_RING_INDEX(u4CpuIdx, prRxRing->u4RingSize);

	pRxCell = &prRxRing->Cell[u4CpuIdx];
	pRxD = (struct RXD_STRUCT *)pRxCell->AllocVa;

	if (halWpdmaGetRxDmaDoneCnt(prGlueInfo, u2Port) == 0)
		return FALSE;

	if (!kalWaitRxDmaDone(prGlueInfo, prRxRing, pRxD, u2Port)) {
		if (!prRxRing->fgIsDumpLog)
			halDumpHifDebugLog(prGlueInfo, false, true);
		prRxRing->fgIsDumpLog = true;
		return FALSE;
	}

	spin_lock_irqsave(pRxRingLock, flags);

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

	if (pRxD->SDLen0 > u4Len) {
		DBGLOG(HAL, WARN,
			"Skip Rx packet, SDL0[%u] > SwRfb max len[%u]\n",
			pRxD->SDLen0, u4Len);
		goto skip;
	}

	prDmaBuf = &pRxCell->DmaBuf;
	if (prMemOps->copyEvent &&
	    !prMemOps->copyEvent(prHifInfo, pRxCell, pRxD,
				 prDmaBuf, pucBuf, u4Len)) {
		spin_unlock_irqrestore(pRxRingLock, flags);
		ASSERT(0);
		return FALSE;
	}

	pRxD->SDPtr0 = (uint64_t)prDmaBuf->AllocPa & DMA_LOWER_32BITS_MASK;
	pRxD->SDPtr1 = ((uint64_t)prDmaBuf->AllocPa >> DMA_BITS_OFFSET) &
		DMA_HIGHER_4BITS_MASK;
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
	struct HIF_MEM_OPS *prMemOps;
	struct RTMP_TX_RING *prTxRing;
	struct RTMP_DMACB *pTxCell;
	struct TXD_STRUCT *pTxD;
	void *pucDst = NULL;
	unsigned long flags = 0;
	spinlock_t *prTxRingLock;

	ASSERT(prGlueInfo);
	ASSERT(pucBuf);
	ASSERT(u4Len <= u4ValidInBufSize);

	prHifInfo = &prGlueInfo->rHifInfo;
	prMemOps = &prHifInfo->rMemOps;
	prTxRing = &prHifInfo->TxRing[u2Port];
	prTxRingLock = &prHifInfo->TxRingLock[u2Port];

	if (prMemOps->allocRuntimeMem)
		pucDst = prMemOps->allocRuntimeMem(u4Len);

	spin_lock_irqsave((spinlock_t *)prTxRingLock, flags);

	kalDevRegRead(prGlueInfo, prTxRing->hw_cidx_addr, &prTxRing->TxCpuIdx);
	if (prTxRing->TxCpuIdx >= TX_RING_SIZE) {
		DBGLOG(HAL, ERROR, "Error TxCpuIdx[%u]\n", prTxRing->TxCpuIdx);
		if (prMemOps->freeBuf)
			prMemOps->freeBuf(pucDst, u4Len);
		spin_unlock_irqrestore((spinlock_t *)prTxRingLock, flags);
		return FALSE;
	}

	pTxCell = &prTxRing->Cell[prTxRing->TxCpuIdx];
	pTxD = (struct TXD_STRUCT *)pTxCell->AllocVa;

	pTxCell->pPacket = NULL;
	pTxCell->pBuffer = pucDst;

	if (prMemOps->copyCmd &&
	    !prMemOps->copyCmd(prHifInfo, pTxCell, pucDst,
			       pucBuf, u4Len, NULL, 0)) {
		if (prMemOps->freeBuf)
			prMemOps->freeBuf(pucDst, u4Len);
		spin_unlock_irqrestore((spinlock_t *)prTxRingLock, flags);
		ASSERT(0);
		return FALSE;
	}

	pTxD->LastSec0 = 1;
	pTxD->LastSec1 = 0;
	pTxD->SDLen0 = u4Len;
	pTxD->SDLen1 = 0;
	pTxD->SDPtr0 = (uint64_t)pTxCell->PacketPa & DMA_LOWER_32BITS_MASK;
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
	struct HIF_MEM_OPS *prMemOps;
	struct RXD_STRUCT *pRxD;
	struct RTMP_RX_RING *prRxRing;
	struct RTMP_DMACB *pRxCell;
	struct RTMP_DMABUF *prDmaBuf;
	spinlock_t *pRxRingLock;
	u_int8_t fgRet = TRUE;
	unsigned long flags = 0;
	uint32_t u4CpuIdx = 0;

	ASSERT(prGlueInfo);

	prHifInfo = &prGlueInfo->rHifInfo;
	prMemOps = &prHifInfo->rMemOps;
	prRxRing = &prHifInfo->RxRing[u2Port];
	pRxRingLock = &prHifInfo->RxRingLock[u2Port];

	kalDevRegRead(prGlueInfo, prRxRing->hw_cidx_addr, &prRxRing->RxCpuIdx);
	u4CpuIdx = prRxRing->RxCpuIdx;
	INC_RING_INDEX(u4CpuIdx, prRxRing->u4RingSize);

	pRxCell = &prRxRing->Cell[u4CpuIdx];
	pRxD = (struct RXD_STRUCT *)pRxCell->AllocVa;

	if (halWpdmaGetRxDmaDoneCnt(prGlueInfo, u2Port) == 0)
		return FALSE;

	if (!kalWaitRxDmaDone(prGlueInfo, prRxRing, pRxD, u2Port)) {
		if (!prRxRing->fgIsDumpLog)
			halDumpHifDebugLog(prGlueInfo, false, true);
		prRxRing->fgIsDumpLog = true;
		return FALSE;
	}

	spin_lock_irqsave(pRxRingLock, flags);

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

	prDmaBuf = &pRxCell->DmaBuf;

	if (prMemOps->flushCache)
		prMemOps->flushCache(prHifInfo, prDmaBuf->AllocVa,
				     pRxD->SDLen0);

	if (prMemOps->copyRxData &&
	    !prMemOps->copyRxData(prHifInfo, pRxCell, prDmaBuf, prSwRfb)) {
		spin_unlock_irqrestore(pRxRingLock, flags);
		return FALSE;
	}

	prSwRfb->pucRecvBuff = ((struct sk_buff *)prSwRfb->pvPacket)->data;
	prSwRfb->prRxStatus = (struct HW_MAC_RX_DESC *)prSwRfb->pucRecvBuff;

#if CFG_TCP_IP_CHKSUM_OFFLOAD
	prSwRfb->u4TcpUdpIpCksStatus = pRxD->RXINFO;
#endif /* CFG_TCP_IP_CHKSUM_OFFLOAD */

	pRxD->SDPtr0 = (uint64_t)prDmaBuf->AllocPa & DMA_LOWER_32BITS_MASK;
	pRxD->SDPtr1 = ((uint64_t)prDmaBuf->AllocPa >>
		DMA_BITS_OFFSET) & DMA_HIGHER_4BITS_MASK;
skip:
	pRxD->SDLen0 = prRxRing->u4BufSize;
	pRxD->DMADONE = 0;

	prRxRing->RxCpuIdx = u4CpuIdx;
	kalDevRegWrite(prGlueInfo, prRxRing->hw_cidx_addr, prRxRing->RxCpuIdx);
	prRxRing->fgIsDumpLog = false;

	spin_unlock_irqrestore(pRxRingLock, flags);

#if CFG_TCP_IP_CHKSUM_OFFLOAD
	DBGLOG(RX, LOUD, "u4TcpUdpIpCksStatus[0x%02x]\n",
		prSwRfb->u4TcpUdpIpCksStatus);
#endif /* CFG_TCP_IP_CHKSUM_OFFLOAD */

	return fgRet;
}

void kalDumpTxRing(struct GLUE_INFO *prGlueInfo,
		   struct RTMP_TX_RING *prTxRing,
		   uint32_t u4Num, bool fgDumpContent)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct HIF_MEM_OPS *prMemOps;
	struct RTMP_DMACB *pTxCell;
	struct TXD_STRUCT *pTxD;
	uint32_t u4DumpLen = 64;

	ASSERT(prGlueInfo);
	prHifInfo = &prGlueInfo->rHifInfo;
	prMemOps = &prHifInfo->rMemOps;

	if (u4Num >= TX_RING_SIZE)
		return;

	pTxCell = &prTxRing->Cell[u4Num];
	pTxD = (struct TXD_STRUCT *) pTxCell->AllocVa;

	DBGLOG(HAL, INFO, "Tx Dese Num[%u]\n", u4Num);
	DBGLOG_MEM32(HAL, INFO, pTxD, sizeof(struct TXD_STRUCT));

	if (!fgDumpContent)
		return;

	if (u4DumpLen > pTxD->SDLen0)
		u4DumpLen = pTxD->SDLen0;

	DBGLOG(HAL, INFO, "Tx Contents\n");
	if (prMemOps->dumpTx)
		prMemOps->dumpTx(prHifInfo, prTxRing, u4Num, u4DumpLen);
	DBGLOG(HAL, INFO, "\n\n");
}

void kalDumpRxRing(struct GLUE_INFO *prGlueInfo,
		   struct RTMP_RX_RING *prRxRing,
		   uint32_t u4Num, bool fgDumpContent)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct HIF_MEM_OPS *prMemOps;
	struct RTMP_DMACB *pRxCell;
	struct RXD_STRUCT *pRxD;
	uint32_t u4DumpLen = 64;

	ASSERT(prGlueInfo);
	prHifInfo = &prGlueInfo->rHifInfo;
	prMemOps = &prHifInfo->rMemOps;

	if (u4Num >= prRxRing->u4RingSize)
		return;

	pRxCell = &prRxRing->Cell[u4Num];
	pRxD = (struct RXD_STRUCT *) pRxCell->AllocVa;

	DBGLOG(HAL, INFO, "Rx Dese Num[%u]\n", u4Num);
	DBGLOG_MEM32(HAL, INFO, pRxD, sizeof(struct RXD_STRUCT));

	if (!fgDumpContent)
		return;

	if (u4DumpLen > pRxD->SDLen0)
		u4DumpLen = pRxD->SDLen0;

	DBGLOG(HAL, INFO, "Rx Contents\n");
	if (prMemOps->dumpRx)
		prMemOps->dumpRx(prHifInfo, prRxRing, u4Num, u4DumpLen);
	DBGLOG(HAL, INFO, "\n\n");
}
