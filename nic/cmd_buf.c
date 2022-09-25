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
 * Id: //Department/DaVinci/BRANCHES/MT6620_WIFI_DRIVER_V2_3/nic/cmd_buf.c#1
 */

/*! \file   "cmd_buf.c"
 *    \brief  This file contain the management function of internal Command
 *	    Buffer for CMD_INFO_T.
 *
 *	We'll convert the OID into Command Packet and then send to FW.
 *  Thus we need to copy the OID information to Command Buffer
 *  for following reasons.
 *    1. The data structure of OID information may not equal to the data
 *       structure of Command, we cannot use the OID buffer directly.
 *    2. If the Command was not generated by driver we also need a place
 *       to store the information.
 *    3. Because the CMD is NOT FIFO when doing memory allocation (CMD will be
 *       generated from OID or interrupt handler), thus we'll use the Block
 *       style of Memory Allocation here.
 */


/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */

/*******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 *******************************************************************************
 */
#include "precomp.h"

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
u_int8_t fgCmdDumpIsDone = FALSE;
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
 * @brief This function is used to initial the MGMT memory pool for CMD Packet.
 *
 * @param prAdapter  Pointer to the Adapter structure.
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
void cmdBufInitialize(struct ADAPTER *prAdapter)
{
	struct CMD_INFO *prCmdInfo;
	uint32_t i;

	ASSERT(prAdapter);

	QUEUE_INITIALIZE(&prAdapter->rFreeCmdList);

	for (i = 0; i < CFG_TX_MAX_CMD_PKT_NUM; i++) {
		prCmdInfo = &prAdapter->arHifCmdDesc[i];
		QUEUE_INSERT_TAIL(&prAdapter->rFreeCmdList,
				  &prCmdInfo->rQueEntry);
	}

}				/* end of cmdBufInitialize() */

#define CMD_DUMP_NUM_PER_LINE 5
/*----------------------------------------------------------------------------*/
/*!
 * @brief dump CMD queue and print to trace, for debug use only
 * @param[in] prQueue	Pointer to the command Queue to be dumped
 * @param[in] quename	Name of the queue
 */
/*----------------------------------------------------------------------------*/
void cmdBufDumpCmdQueue(struct QUE *prQueue,
			int8_t *queName)
{
	struct CMD_INFO *prCmdInfo = QUEUE_GET_HEAD(prQueue);
	uint8_t i = 1, pos = 0;
	char buf[500] = {0};

	DBGLOG(NIC, INFO, "Dump CMD info for %s, Elem number:%u\n",
			queName, prQueue->u4NumElem);
	kalMemZero(buf, sizeof(buf));
	while (prCmdInfo) {
		u_int8_t fgEndLine = i == prQueue->u4NumElem ||
				i % CMD_DUMP_NUM_PER_LINE == 0;

		pos += kalSnprintf(buf + pos, 30,
				"CID:0x%02x,SEQ:%d,Type:%d%s",
				prCmdInfo->ucCID,
				prCmdInfo->ucCmdSeqNum,
				prCmdInfo->eCmdType,
				fgEndLine ? "\n" : "; ");
		if (fgEndLine) {
			DBGLOG(NIC, INFO, "%s", buf);
			kalMemZero(buf, sizeof(buf));
			pos = 0;
		}
		prCmdInfo = QUEUE_GET_NEXT_ENTRY(prCmdInfo);
		i++;
	}
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief Allocate CMD_INFO_T from a free list and MGMT memory pool.
 *
 * @param[in] prAdapter      Pointer to the Adapter structure.
 * @param[in] u4Length       Length of the frame buffer to allocate.
 *
 * @retval NULL      Pointer to the valid CMD Packet handler
 * @retval !NULL     Fail to allocat CMD Packet
 */
/*----------------------------------------------------------------------------*/
#if CFG_DBG_MGT_BUF
struct CMD_INFO *cmdBufAllocateCmdInfoX(struct ADAPTER
					   *prAdapter, uint32_t u4Length,
					   uint8_t *fileAndLine)
#else
struct CMD_INFO *cmdBufAllocateCmdInfo(struct ADAPTER
				       *prAdapter, uint32_t u4Length)
#endif
{
	struct CMD_INFO *prCmdInfo = NULL;

	KAL_SPIN_LOCK_DECLARATION();

	DEBUGFUNC("cmdBufAllocateCmdInfo");

	ASSERT(prAdapter);

	KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_CMD_RESOURCE);
	QUEUE_REMOVE_HEAD(&prAdapter->rFreeCmdList, prCmdInfo,
			  struct CMD_INFO *);
	KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_CMD_RESOURCE);

	if (prCmdInfo) {
		kalMemZero(prCmdInfo, sizeof(struct CMD_INFO));
		/* Setup initial value in CMD_INFO_T */
		prCmdInfo->u2InfoBufLen = 0;
		prCmdInfo->fgIsOid = FALSE;

		if (u4Length) {
			/* Start address of allocated memory */
			u4Length = TFCB_FRAME_PAD_TO_DW(u4Length);

#if CFG_DBG_MGT_BUF
			prCmdInfo->pucInfoBuffer = cnmMemAllocX(prAdapter,
				RAM_TYPE_BUF, u4Length, fileAndLine);
#else
			prCmdInfo->pucInfoBuffer = cnmMemAlloc(prAdapter,
				RAM_TYPE_BUF, u4Length);
#endif

			if (prCmdInfo->pucInfoBuffer == NULL) {
				KAL_ACQUIRE_SPIN_LOCK(prAdapter,
					SPIN_LOCK_CMD_RESOURCE);
				QUEUE_INSERT_TAIL(&prAdapter->rFreeCmdList,
						  &prCmdInfo->rQueEntry);
				KAL_RELEASE_SPIN_LOCK(prAdapter,
					SPIN_LOCK_CMD_RESOURCE);

				prCmdInfo = NULL;
			} else {
				kalMemZero(prCmdInfo->pucInfoBuffer, u4Length);
			}
		} else {
			prCmdInfo->pucInfoBuffer = NULL;
		}
		fgCmdDumpIsDone = FALSE;
	} else if (!fgCmdDumpIsDone) {
		struct GLUE_INFO *prGlueInfo = prAdapter->prGlueInfo;
		struct QUE *prCmdQue = &prGlueInfo->rCmdQueue;
		struct QUE *prPendingCmdQue = &prAdapter->rPendingCmdQueue;
#if CFG_SUPPORT_MULTITHREAD
		struct QUE *prTxCmdQueue = &prAdapter->rTxCmdQueue;
		struct QUE *prTxCmdDoneQueue = &prAdapter->rTxCmdDoneQueue;
#endif
		struct TX_TCQ_STATUS *prTc = &prAdapter->rTxCtrl.rTc;

		fgCmdDumpIsDone = TRUE;
		cmdBufDumpCmdQueue(prCmdQue, "waiting CMD queue");
		cmdBufDumpCmdQueue(prPendingCmdQue,
				   "waiting response CMD queue");
#if CFG_SUPPORT_MULTITHREAD
		cmdBufDumpCmdQueue(prTxCmdQueue, "waiting Tx CMD queue");
		cmdBufDumpCmdQueue(prTxCmdDoneQueue,
				   "waiting Tx CMD Done queue");
#endif
		DBGLOG(NIC, INFO, "Tc4 number:%d\n",
		       prTc->au4FreeBufferCount[TC4_INDEX]);
	}

	if (prCmdInfo) {
		DBGLOG(MEM, TRACE,
		       "CMD[0x%p] allocated! LEN[%04u], Rest[%u]\n",
		       prCmdInfo, u4Length, prAdapter->rFreeCmdList.u4NumElem);

		prAdapter->fgIsCmdAllocFail = FALSE;
	} else {
		/* dump debug log */
		prAdapter->u4HifDbgFlag |= DEG_HIF_DEFAULT_DUMP;
		kalSetHifDbgEvent(prAdapter->prGlueInfo);

		DBGLOG(MEM, ERROR,
		       "CMD allocation failed! LEN[%04u], Rest[%u]\n",
		       u4Length, prAdapter->rFreeCmdList.u4NumElem);

		if (!prAdapter->fgIsCmdAllocFail) {
			prAdapter->fgIsCmdAllocFail = TRUE;
			prAdapter->u4CmdAllocStartFailTime = kalGetTimeTick();
		} else if (CHECK_FOR_TIMEOUT(kalGetTimeTick(),
					    prAdapter->u4CmdAllocStartFailTime,
					    CFG_CMD_ALLOC_FAIL_TIMEOUT_MS))
			GL_DEFAULT_RESET_TRIGGER(prAdapter,
					     RST_CMD_EVT_FAIL);
	}

	return prCmdInfo;

}				/* end of cmdBufAllocateCmdInfo() */

/*----------------------------------------------------------------------------*/
/*!
 * @brief This function is used to free the CMD Packet to the MGMT memory pool.
 *
 * @param prAdapter  Pointer to the Adapter structure.
 * @param prCmdInfo  CMD Packet handler
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
void cmdBufFreeCmdInfo(struct ADAPTER *prAdapter,
		       struct CMD_INFO *prCmdInfo)
{
	KAL_SPIN_LOCK_DECLARATION();

	DEBUGFUNC("cmdBufFreeCmdInfo");

	ASSERT(prAdapter);

	if (prCmdInfo) {
		if (prCmdInfo->pucInfoBuffer) {
			cnmMemFree(prAdapter, prCmdInfo->pucInfoBuffer);
			prCmdInfo->pucInfoBuffer = NULL;
		} else {
			DBGLOG(MEM, WARN, "CMD[0x%x] not freed Buffer NULL!\n",
				prCmdInfo->ucCID);
		}

		KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_CMD_RESOURCE);
		QUEUE_INSERT_TAIL(&prAdapter->rFreeCmdList,
				  &prCmdInfo->rQueEntry);
		KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_CMD_RESOURCE);
	}

	if (prCmdInfo)
		DBGLOG(MEM, LOUD, "CMD[0x%d] SEQ[%d] freed! Rest[%u]\n",
			prCmdInfo->ucCID, prCmdInfo->ucCmdSeqNum,
			prAdapter->rFreeCmdList.u4NumElem);

	return;

}				/* end of cmdBufFreeCmdPacket() */

uint32_t
wlanSendSetQueryCmd(struct ADAPTER *prAdapter,
		    uint8_t ucCID,
		    u_int8_t fgSetQuery,
		    u_int8_t fgNeedResp,
		    u_int8_t fgIsOid,
		    PFN_CMD_DONE_HANDLER pfCmdDoneHandler,
		    PFN_CMD_TIMEOUT_HANDLER pfCmdTimeoutHandler,
		    uint32_t u4SetQueryInfoLen,
		    uint8_t *pucInfoBuffer, void *pvSetQueryBuffer,
		    uint32_t u4SetQueryBufferLen)
{
#ifdef CFG_SUPPORT_UNIFIED_COMMAND
	return wlanSendSetQueryCmdHelper(
#else
	return wlanSendSetQueryCmdAdv(
#endif
		prAdapter, ucCID, 0, fgSetQuery,
		fgNeedResp, fgIsOid, pfCmdDoneHandler,
		pfCmdTimeoutHandler, u4SetQueryInfoLen,
		pucInfoBuffer, pvSetQueryBuffer, u4SetQueryBufferLen,
		CMD_SEND_METHOD_ENQUEUE);
}

