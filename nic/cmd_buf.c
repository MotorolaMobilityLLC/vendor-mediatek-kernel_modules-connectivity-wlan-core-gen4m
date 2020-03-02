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
void cmdBufInitialize(IN struct ADAPTER *prAdapter)
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
	struct CMD_INFO *prCmdInfo = (struct CMD_INFO *)
				     QUEUE_GET_HEAD(prQueue);

	DBGLOG(NIC, INFO, "Dump CMD info for %s, Elem number:%u\n",
	       queName, prQueue->u4NumElem);
	while (prCmdInfo) {
		struct CMD_INFO *prCmdInfo1, *prCmdInfo2, *prCmdInfo3;

		prCmdInfo1 = (struct CMD_INFO *)QUEUE_GET_NEXT_ENTRY((
					struct QUE_ENTRY *)prCmdInfo);
		if (!prCmdInfo1) {
			DBGLOG(NIC, INFO, "CID:%d SEQ:%d\n", prCmdInfo->ucCID,
			       prCmdInfo->ucCmdSeqNum);
			break;
		}
		prCmdInfo2 = (struct CMD_INFO *)QUEUE_GET_NEXT_ENTRY((
					struct QUE_ENTRY *)prCmdInfo1);
		if (!prCmdInfo2) {
			DBGLOG(NIC, INFO, "CID:%d, SEQ:%d; CID:%d, SEQ:%d\n",
			       prCmdInfo->ucCID,
			       prCmdInfo->ucCmdSeqNum, prCmdInfo1->ucCID,
			       prCmdInfo1->ucCmdSeqNum);
			break;
		}
		prCmdInfo3 = (struct CMD_INFO *)QUEUE_GET_NEXT_ENTRY((
					struct QUE_ENTRY *)prCmdInfo2);
		if (!prCmdInfo3) {
			DBGLOG(NIC, INFO,
			       "CID:%d, SEQ:%d; CID:%d, SEQ:%d; CID:%d, SEQ:%d\n",
			       prCmdInfo->ucCID,
			       prCmdInfo->ucCmdSeqNum, prCmdInfo1->ucCID,
			       prCmdInfo1->ucCmdSeqNum,
			       prCmdInfo2->ucCID, prCmdInfo2->ucCmdSeqNum);
			break;
		}
		DBGLOG(NIC, INFO,
		       "CID:%d, SEQ:%d; CID:%d, SEQ:%d; CID:%d, SEQ:%d; CID:%d, SEQ:%d\n",
		       prCmdInfo->ucCID, prCmdInfo->ucCmdSeqNum,
		       prCmdInfo1->ucCID, prCmdInfo1->ucCmdSeqNum,
		       prCmdInfo2->ucCID, prCmdInfo2->ucCmdSeqNum,
		       prCmdInfo3->ucCID, prCmdInfo3->ucCmdSeqNum);
		prCmdInfo = (struct CMD_INFO *)QUEUE_GET_NEXT_ENTRY((
					struct QUE_ENTRY *)prCmdInfo3);
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
struct CMD_INFO *cmdBufAllocateCmdInfoX(IN struct ADAPTER
					   *prAdapter, IN uint32_t u4Length,
					   uint8_t *fileAndLine)
#else
struct CMD_INFO *cmdBufAllocateCmdInfo(IN struct ADAPTER
				       *prAdapter, IN uint32_t u4Length)
#endif
{
	struct CMD_INFO *prCmdInfo;

	KAL_SPIN_LOCK_DECLARATION();

	DEBUGFUNC("cmdBufAllocateCmdInfo");

	ASSERT(prAdapter);

	KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_CMD_RESOURCE);
	QUEUE_REMOVE_HEAD(&prAdapter->rFreeCmdList, prCmdInfo,
			  struct CMD_INFO *);
	KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_CMD_RESOURCE);

	if (prCmdInfo) {
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
		struct TX_TCQ_STATUS *prTc = &prAdapter->rTxCtrl.rTc;

		fgCmdDumpIsDone = TRUE;
		cmdBufDumpCmdQueue(prCmdQue, "waiting Tx CMD queue");
		cmdBufDumpCmdQueue(prPendingCmdQue,
				   "waiting response CMD queue");
		DBGLOG(NIC, INFO, "Tc4 number:%d\n",
		       prTc->au4FreeBufferCount[TC4_INDEX]);
	}

	if (prCmdInfo) {
		DBGLOG(MEM, LOUD,
		       "CMD[0x%p] allocated! LEN[%04u], Rest[%u]\n",
		       prCmdInfo, u4Length, prAdapter->rFreeCmdList.u4NumElem);
	} else {
		DBGLOG(MEM, ERROR,
		       "CMD allocation failed! LEN[%04u], Rest[%u]\n",
		       u4Length, prAdapter->rFreeCmdList.u4NumElem);
#if CFG_CHIP_RESET_SUPPORT
		/* Trigger RESET */
		GL_RESET_TRIGGER(prAdapter, RST_FLAG_CHIP_RESET);
#endif
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
void cmdBufFreeCmdInfo(IN struct ADAPTER *prAdapter,
		       IN struct CMD_INFO *prCmdInfo)
{
	KAL_SPIN_LOCK_DECLARATION();

	DEBUGFUNC("cmdBufFreeCmdInfo");

	ASSERT(prAdapter);

	if (prCmdInfo) {
		if (prCmdInfo->pucInfoBuffer) {
			cnmMemFree(prAdapter, prCmdInfo->pucInfoBuffer);
			prCmdInfo->pucInfoBuffer = NULL;
		}

		KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_CMD_RESOURCE);
		QUEUE_INSERT_TAIL(&prAdapter->rFreeCmdList,
				  &prCmdInfo->rQueEntry);
		KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_CMD_RESOURCE);
	}

	if (prCmdInfo)
		DBGLOG(MEM, LOUD, "CMD[0x%p] freed! Rest[%u]\n", prCmdInfo,
		       prAdapter->rFreeCmdList.u4NumElem);

	return;

}				/* end of cmdBufFreeCmdPacket() */
