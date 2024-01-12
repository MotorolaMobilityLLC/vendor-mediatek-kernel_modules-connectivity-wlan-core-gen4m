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
/*! \file   fw_dl.c
*/

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/
#include "precomp.h"

/*******************************************************************************
*                              C O N S T A N T S
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

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

#if CFG_ENABLE_FW_DOWNLOAD
UINT_32 wlanGetDataMode(IN P_ADAPTER_T prAdapter, IN enum ENUM_IMG_DL_IDX_T eDlIdx, IN UINT_8 ucFeatureSet)
{
	UINT_32 u4DataMode = 0;

	if (ucFeatureSet & DOWNLOAD_CONFIG_ENCRYPTION_MODE) {
		u4DataMode |= DOWNLOAD_CONFIG_RESET_OPTION;
		u4DataMode |= (ucFeatureSet & DOWNLOAD_CONFIG_KEY_INDEX_MASK);
		u4DataMode |= DOWNLOAD_CONFIG_ENCRYPTION_MODE;
	}

	if (eDlIdx == IMG_DL_IDX_CR4_FW)
		u4DataMode |= DOWNLOAD_CONFIG_WORKING_PDA_OPTION;

#if CFG_ENABLE_FW_DOWNLOAD_ACK
	u4DataMode |= DOWNLOAD_CONFIG_ACK_OPTION;	/* ACK needed */
#endif
	return u4DataMode;
}

VOID wlanGetHarvardFwInfo(IN P_ADAPTER_T prAdapter, IN UINT_8 u4SecIdx, IN enum ENUM_IMG_DL_IDX_T eDlIdx,
			  OUT PUINT_32 pu4Addr, OUT PUINT_32 pu4Len,
			  OUT PUINT_32 pu4DataMode, OUT PBOOLEAN pfgIsEMIDownload)
{
	struct TAILER_FORMAT_T *prTailer;

	if (eDlIdx == IMG_DL_IDX_N9_FW)
		prTailer = &prAdapter->rVerInfo.rN9tailer[u4SecIdx];
	else
		prTailer = &prAdapter->rVerInfo.rCR4tailer[u4SecIdx];

	*pu4Addr = prTailer->addr;
	*pu4Len = (prTailer->len + LEN_4_BYTE_CRC);
	*pu4DataMode = wlanGetDataMode(prAdapter, eDlIdx, prTailer->feature_set);
	*pfgIsEMIDownload = FALSE;
}

VOID wlanGetConnacFwInfo(IN P_ADAPTER_T prAdapter, IN UINT_8 u4SecIdx, IN enum ENUM_IMG_DL_IDX_T eDlIdx,
			 OUT PUINT_32 pu4Addr, OUT PUINT_32 pu4Len,
			 OUT PUINT_32 pu4DataMode, OUT PBOOLEAN pfgIsEMIDownload)
{
	struct TAILER_REGION_FORMAT_T *prTailer = &prAdapter->rVerInfo.rRegionTailers[u4SecIdx];

	*pu4Addr = prTailer->u4Addr;
	*pu4Len = prTailer->u4Len;
	*pu4DataMode = wlanGetDataMode(prAdapter, eDlIdx, prTailer->ucFeatureSet);
	*pfgIsEMIDownload = prTailer->ucFeatureSet & DOWNLOAD_CONFIG_EMI;
}

#if CFG_SUPPORT_COMPRESSION_FW_OPTION
VOID wlanImageSectionGetCompressFwInfo(IN P_ADAPTER_T prAdapter, IN PVOID pvFwImageMapFile,
				       IN UINT_32 u4FwImageFileLength, IN UINT_8 ucTotSecNum,
				       IN UINT_8 ucCurSecNum, IN enum ENUM_IMG_DL_IDX_T eDlIdx,
				       OUT PUINT_32 pu4Addr, OUT PUINT_32 pu4Len,
				       OUT PUINT_32 pu4DataMode, OUT PUINT_32 pu4BlockSize,
				       OUT PUINT_32 pu4CRC, OUT PUINT_32 pu4UncompressedLength)
{
	struct FW_IMAGE_TAILER_T_2 *prFwHead;
	struct TAILER_FORMAT_T_2 *prTailer;
	UINT_8 aucBuf[32];

	prFwHead = (struct FW_IMAGE_TAILER_T_2 *)
		(pvFwImageMapFile + u4FwImageFileLength - sizeof(struct FW_IMAGE_TAILER_T_2));
	if (ucTotSecNum == 1)
		prTailer = &prFwHead->dlm_info;
	else
		prTailer = &prFwHead->ilm_info;

	prTailer = &prTailer[ucCurSecNum];

	*pu4Addr = prTailer->addr;
	*pu4Len = (prTailer->len);
	*pu4BlockSize = (prTailer->block_size);
	*pu4CRC = (prTailer->crc);
	*pu4UncompressedLength = (prTailer->real_size);
	*pu4DataMode = wlanGetDataMode(prAdapter, eDlIdx, prTailer->feature_set);

	/* Dump image information */
	if (ucCurSecNum == 0) {
		DBGLOG(INIT, INFO, "%s INFO: chip_info[%u:E%u] feature[0x%02X]\n",
			(eDlIdx == IMG_DL_IDX_N9_FW)?"N9":"CR4", prTailer->chip_info,
			prTailer->eco_code, prTailer->feature_set);
		kalMemZero(aucBuf, 32);
		kalStrnCpy(aucBuf, prTailer->ram_version, sizeof(prTailer->ram_version));
		DBGLOG(INIT, INFO, "date[%s] version[%s]\n", prTailer->ram_built_date, aucBuf);
	}
    /* Backup to FW version info */
	if (eDlIdx == IMG_DL_IDX_N9_FW) {
		kalMemCopy(&prAdapter->rVerInfo.rN9Compressedtailer, prTailer, sizeof(struct TAILER_FORMAT_T_2));
		prAdapter->rVerInfo.fgIsN9CompressedFW = TRUE;
	} else {
		kalMemCopy(&prAdapter->rVerInfo.rCR4Compressedtailer, prTailer, sizeof(struct TAILER_FORMAT_T_2));
		prAdapter->rVerInfo.fgIsCR4CompressedFW = TRUE;
	}
}
#endif

VOID wlanImageSectionGetPatchInfo(IN P_ADAPTER_T prAdapter,
				  IN PVOID pvFwImageMapFile, IN UINT_32 u4FwImageFileLength,
				  OUT PUINT_32 pu4StartOffset, OUT PUINT_32 pu4Addr, OUT PUINT_32 pu4Len,
				  OUT PUINT_32 pu4DataMode)
{
	struct PATCH_FORMAT_T *prPatchFormat;
	UINT_8 aucBuffer[32];
	struct mt66xx_chip_info *prChipInfo = prAdapter->chip_info;

	prPatchFormat = (struct PATCH_FORMAT_T *) pvFwImageMapFile;

	*pu4StartOffset = offsetof(struct PATCH_FORMAT_T, ucPatchImage);
	*pu4Addr = prChipInfo->patch_addr;
	*pu4Len = u4FwImageFileLength - offsetof(struct PATCH_FORMAT_T, ucPatchImage);
	*pu4DataMode = wlanGetDataMode(prAdapter, IMG_DL_IDX_PATCH, 0);

	/* Dump image information */
	kalMemZero(aucBuffer, 32);
	kalStrnCpy(aucBuffer, prPatchFormat->aucPlatform, 4);
	DBGLOG(INIT, INFO, "PATCH INFO: platform[%s] HW/SW ver[0x%04X] ver[0x%04X]\n",
	       aucBuffer, prPatchFormat->u4SwHwVersion, prPatchFormat->u4PatchVersion);

	kalStrnCpy(aucBuffer, prPatchFormat->aucBuildDate, 16);
	DBGLOG(INIT, INFO, "date[%s]\n", aucBuffer);

	/* Backup to FW version info */
	kalMemCopy(&prAdapter->rVerInfo.rPatchHeader, prPatchFormat, sizeof(struct PATCH_FORMAT_T));
}

WLAN_STATUS wlanDownloadSection(IN P_ADAPTER_T prAdapter, IN UINT_32 u4Addr, IN UINT_32 u4Len,
				IN UINT_32 u4DataMode, IN PUINT_8 pucStartPtr, IN enum ENUM_IMG_DL_IDX_T eDlIdx)
{
	UINT_32 u4ImgSecSize, u4Offset;
	PUINT_8 pucSecBuf;

	if (wlanImageSectionConfig(prAdapter, u4Addr, u4Len, u4DataMode, eDlIdx) != WLAN_STATUS_SUCCESS) {
		DBGLOG(INIT, ERROR, "Firmware download configuration failed!\n");
		return WLAN_STATUS_FAILURE;
	}

	for (u4Offset = 0; u4Offset < u4Len; u4Offset += CMD_PKT_SIZE_FOR_IMAGE) {
		if (u4Offset + CMD_PKT_SIZE_FOR_IMAGE < u4Len)
			u4ImgSecSize = CMD_PKT_SIZE_FOR_IMAGE;
		else
			u4ImgSecSize = u4Len - u4Offset;

		pucSecBuf = (PUINT_8) pucStartPtr + u4Offset;
		if (wlanImageSectionDownload(prAdapter, u4ImgSecSize, pucSecBuf) != WLAN_STATUS_SUCCESS) {
			DBGLOG(INIT, ERROR, "Firmware scatter download failed!\n");
			return WLAN_STATUS_FAILURE;
		}
	}

	return WLAN_STATUS_SUCCESS;
}

WLAN_STATUS wlanDownloadEMISection(IN P_ADAPTER_T prAdapter, IN UINT_32 u4Len,
				   IN UINT_32 u4DestAddr, IN PUINT_8 pucStartPtr)
{
	/* TODO EMI download */
	return WLAN_STATUS_SUCCESS;
}

#if CFG_SUPPORT_COMPRESSION_FW_OPTION
BOOLEAN wlanImageSectionCheckFwCompressInfo(IN P_ADAPTER_T prAdapter,
	IN PVOID pvFwImageMapFile, IN UINT_32 u4FwImageFileLength, IN enum ENUM_IMG_DL_IDX_T eDlIdx) {
	UINT_8 ucCompression;
	struct FW_IMAGE_TAILER_CHECK *prCheckInfo;

	if (eDlIdx == IMG_DL_IDX_PATCH)
		return FALSE;

	prCheckInfo = (struct FW_IMAGE_TAILER_CHECK *)
		(pvFwImageMapFile + u4FwImageFileLength - sizeof(struct FW_IMAGE_TAILER_CHECK));
	DBGLOG(INIT, INFO, "feature_set %d\n", prCheckInfo->feature_set);
	ucCompression = (UINT_8)((prCheckInfo->feature_set & COMPRESSION_OPTION_MASK)
					>> COMPRESSION_OPTION_OFFSET);
	DBGLOG(INIT, INFO, "Compressed Check INFORMATION %d\n", ucCompression);
	if (ucCompression == 1) {
		DBGLOG(INIT, INFO, "Compressed FW\n");
		return TRUE;
	}
	return FALSE;
}

WLAN_STATUS wlanCompressedImageSectionDownloadStage(IN P_ADAPTER_T prAdapter, IN PVOID pvFwImageMapFile,
	IN UINT_32 u4FwImageFileLength, IN UINT_8 ucSectionNumber, IN enum ENUM_IMG_DL_IDX_T eDlIdx,
	OUT PUINT_8 pucIsCompressed, OUT P_INIT_CMD_WIFI_DECOMPRESSION_START prFwImageInFo)
{
	UINT_32 i;
	INT_32  i4TotalLen;
	UINT_32 u4FileOffset = 0;
	UINT_32 u4StartOffset = 0;
	UINT_32 u4DataMode = 0;
	UINT_32 u4Addr, u4Len, u4BlockSize, u4CRC, u4UnCompressedLength;
	WLAN_STATUS u4Status = WLAN_STATUS_SUCCESS;
	PUINT_8 pucStartPtr;
	UINT_32 u4offset = 0, u4ChunkSize;
	/* 3a. parse file header for decision of divided firmware download or not */
	if (wlanImageSectionCheckFwCompressInfo(prAdapter, pvFwImageMapFile,
		u4FwImageFileLength, eDlIdx) == TRUE){
		for (i = 0; i < ucSectionNumber; ++i) {
			wlanImageSectionGetCompressFwInfo(prAdapter, pvFwImageMapFile,
					u4FwImageFileLength, ucSectionNumber, i, eDlIdx,
					&u4Addr, &u4Len, &u4DataMode,
					&u4BlockSize, &u4CRC, &u4UnCompressedLength);
			u4offset = 0;
			if (i == 0) {
				prFwImageInFo->u4BlockSize = u4BlockSize;
				prFwImageInFo->u4Region1Address = u4Addr;
				prFwImageInFo->u4Region1CRC = u4CRC;
				prFwImageInFo->u4Region1length = u4UnCompressedLength;
			} else {
				prFwImageInFo->u4Region2Address = u4Addr;
				prFwImageInFo->u4Region2CRC = u4CRC;
				prFwImageInFo->u4Region2length = u4UnCompressedLength;
			}
			i4TotalLen = u4Len;
			DBGLOG(INIT, INFO, "DL Offset[%u] addr[0x%08x] len[%u] datamode[0x%08x]\n",
					u4FileOffset, u4Addr, u4Len, u4DataMode);
			DBGLOG(INIT, INFO, "DL BLOCK[%u]  COMlen[%u] CRC[%u]\n",
					u4BlockSize, u4UnCompressedLength, u4CRC);
			pucStartPtr = (PUINT_8)pvFwImageMapFile + u4StartOffset;
			while (i4TotalLen) {
				u4ChunkSize =  *((unsigned int *)(pucStartPtr+u4FileOffset));
				u4FileOffset += 4;
				DBGLOG(INIT, INFO, "Downloaded Length %d! Addr %x\n", i4TotalLen, u4Addr + u4offset);
				DBGLOG(INIT, INFO, "u4ChunkSize Length %d!\n", u4ChunkSize);

				u4Status = wlanDownloadSection(prAdapter, u4Addr + u4offset, u4ChunkSize,
							u4DataMode, pvFwImageMapFile + u4FileOffset, eDlIdx);
				/* escape from loop if any pending error occurs */
				if (u4Status == WLAN_STATUS_FAILURE)
					break;

				i4TotalLen -= u4ChunkSize;
				u4offset += u4BlockSize;
				u4FileOffset += u4ChunkSize;
				if (i4TotalLen < 0) {
					DBGLOG(INIT, ERROR, "Firmware scatter download failed!\n");
					u4Status = WLAN_STATUS_FAILURE;
					break;
				}
			}
		}
		*pucIsCompressed = TRUE;
	} else {
		u4Status = wlanImageSectionDownloadStage(prAdapter, pvFwImageMapFile, u4FwImageFileLength,
			ucSectionNumber, eDlIdx);
		*pucIsCompressed = FALSE;
	}
	return u4Status;
}
#endif

WLAN_STATUS wlanImageSectionDownloadStage(IN P_ADAPTER_T prAdapter, IN PVOID pvFwImageMapFile,
					  IN UINT_32 u4FwImageFileLength, IN UINT_8 ucSectionNumber,
					  IN enum ENUM_IMG_DL_IDX_T eDlIdx)
{
	UINT_32 u4SecIdx, u4Offset = 0;
	UINT_32 u4Addr, u4Len, u4DataMode = 0;
	BOOLEAN fgIsEMIDownload = FALSE;
	WLAN_STATUS u4Status = WLAN_STATUS_SUCCESS;
	struct mt66xx_chip_info *prChipInfo = prAdapter->chip_info;

	/* 3a. parse file header for decision of divided firmware download or not */
	if (eDlIdx == IMG_DL_IDX_PATCH) {
		wlanImageSectionGetPatchInfo(prAdapter, pvFwImageMapFile, u4FwImageFileLength,
			&u4Offset, &u4Addr, &u4Len, &u4DataMode);

		DBGLOG(INIT, INFO, "DL Offset[%u] addr[0x%08x] len[%u] datamode[0x%08x]\n",
			u4Offset, u4Addr, u4Len, u4DataMode);

		u4Status = wlanDownloadSection(prAdapter, u4Addr, u4Len, u4DataMode,
			pvFwImageMapFile + u4Offset, eDlIdx);
	} else {
		for (u4SecIdx = 0; u4SecIdx < ucSectionNumber; u4SecIdx++, u4Offset += u4Len) {
			prChipInfo->fw_dl_ops->getFwInfo(prAdapter, u4SecIdx, eDlIdx, &u4Addr,
				&u4Len, &u4DataMode, &fgIsEMIDownload);

			DBGLOG(INIT, INFO, "DL Offset[%u] addr[0x%08x] len[%u] datamode[0x%08x]\n",
				u4Offset, u4Addr, u4Len, u4DataMode);

			if (fgIsEMIDownload)
				u4Status = wlanDownloadEMISection(prAdapter, u4Addr, u4Len,
					pvFwImageMapFile + u4Offset);
			else
				u4Status = wlanDownloadSection(prAdapter, u4Addr, u4Len, u4DataMode,
					pvFwImageMapFile + u4Offset, eDlIdx);

			/* escape from loop if any pending error occurs */
			if (u4Status == WLAN_STATUS_FAILURE)
				break;
		}
	}
	return u4Status;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to confirm the status of
*        previously patch semaphore control
*
* @param prAdapter      Pointer to the Adapter structure.
*        ucCmdSeqNum    Sequence number of previous firmware scatter
*
* @return WLAN_STATUS_SUCCESS
*         WLAN_STATUS_FAILURE
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS wlanPatchRecvSemaResp(IN P_ADAPTER_T prAdapter, IN UINT_8 ucCmdSeqNum, OUT PUINT_8 pucPatchStatus)
{
	UINT_8 aucBuffer[sizeof(INIT_HIF_RX_HEADER_T) + sizeof(INIT_EVENT_CMD_RESULT)];
	P_INIT_HIF_RX_HEADER_T prInitHifRxHeader;
	P_INIT_EVENT_CMD_RESULT prEventCmdResult;
	UINT_32 u4RxPktLength;

	ASSERT(prAdapter);

	if (kalIsCardRemoved(prAdapter->prGlueInfo) == TRUE || fgIsBusAccessFailed == TRUE)
		return WLAN_STATUS_FAILURE;


	if (nicRxWaitResponse(prAdapter, 0, aucBuffer,
			      sizeof(INIT_HIF_RX_HEADER_T) + sizeof(INIT_EVENT_CMD_RESULT),
			      &u4RxPktLength) != WLAN_STATUS_SUCCESS) {

		DBGLOG(INIT, WARN, "Wait patch semaphore response fail\n");
		return WLAN_STATUS_FAILURE;
	}

	prInitHifRxHeader = (P_INIT_HIF_RX_HEADER_T) aucBuffer;
	if (prInitHifRxHeader->rInitWifiEvent.ucEID != INIT_EVENT_ID_PATCH_SEMA_CTRL) {
		DBGLOG(INIT, WARN, "Unexpected EVENT ID, get 0x%0x\n", prInitHifRxHeader->rInitWifiEvent.ucEID);
		return WLAN_STATUS_FAILURE;
	}

	if (prInitHifRxHeader->rInitWifiEvent.ucSeqNum != ucCmdSeqNum) {
		DBGLOG(INIT, WARN, "Unexpected SeqNum %d, %d\n", ucCmdSeqNum,
		       prInitHifRxHeader->rInitWifiEvent.ucSeqNum);
		return WLAN_STATUS_FAILURE;
	}

	prEventCmdResult = (P_INIT_EVENT_CMD_RESULT) (prInitHifRxHeader->rInitWifiEvent.aucBuffer);

	*pucPatchStatus = prEventCmdResult->ucStatus;

#if 0
	if (prEventCmdResult->ucStatus != PATCH_STATUS_GET_SEMA_NEED_PATCH) {
		DBGLOG(INIT, INFO, "Patch status[%d], skip patch\n", prEventCmdResult->ucStatus);
		return WLAN_STATUS_FAILURE;
	}
		DBGLOG(INIT, INFO, "Status[%d], ready to patch\n", prEventCmdResult->ucStatus);
		return WLAN_STATUS_SUCCESS;

#endif

	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to check the patch semaphore control.
*
* @param prAdapter      Pointer to the Adapter structure.
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS wlanPatchSendSemaControl(IN P_ADAPTER_T prAdapter, OUT PUINT_8 pucSeqNum)
{
	P_CMD_INFO_T prCmdInfo;
	P_INIT_HIF_TX_HEADER_T prInitHifTxHeader;
	WLAN_STATUS u4Status = WLAN_STATUS_SUCCESS;
	P_INIT_CMD_PATCH_SEMA_CONTROL prPatchSemaControl;
	struct mt66xx_chip_info *prChipInfo;

	ASSERT(prAdapter);
	prChipInfo = prAdapter->chip_info;

	DEBUGFUNC("wlanImagePatchSemaphoreCheck");

	/* 1. Allocate CMD Info Packet and its Buffer. */
	prCmdInfo =
	    cmdBufAllocateCmdInfo(prAdapter, sizeof(INIT_HIF_TX_HEADER_T) + sizeof(INIT_CMD_PATCH_SEMA_CONTROL));

	/* DBGLOG(INIT, ERROR, "sizeof INIT_HIF_TX_HEADER_T = %d\n", sizeof(INIT_HIF_TX_HEADER_T)); */
	if (!prCmdInfo) {
		DBGLOG(INIT, ERROR, "Allocate CMD_INFO_T ==> FAILED.\n");
		return WLAN_STATUS_FAILURE;
	}

	prCmdInfo->u2InfoBufLen = sizeof(INIT_HIF_TX_HEADER_T) + sizeof(INIT_CMD_PATCH_SEMA_CONTROL);

	/* 2. Setup common CMD Info Packet */
	prInitHifTxHeader = (P_INIT_HIF_TX_HEADER_T) (prCmdInfo->pucInfoBuffer);
	prInitHifTxHeader->u2TxByteCount = prCmdInfo->u2InfoBufLen;
	prInitHifTxHeader->u2PQ_ID = INIT_CMD_PQ_ID;
	prInitHifTxHeader->ucHeaderFormat = INIT_CMD_PACKET_TYPE_ID;
	prInitHifTxHeader->ucPktFt = INIT_PKT_FT_CMD;

	prInitHifTxHeader->rInitWifiCmd.ucCID = INIT_CMD_ID_PATCH_SEMAPHORE_CONTROL;
	prInitHifTxHeader->rInitWifiCmd.ucPktTypeID = INIT_CMD_PDA_PACKET_TYPE_ID;
	prInitHifTxHeader->rInitWifiCmd.ucSeqNum = nicIncreaseCmdSeqNum(prAdapter);

	*pucSeqNum = prInitHifTxHeader->rInitWifiCmd.ucSeqNum;

	/* 3. Setup DOWNLOAD_BUF */
	prPatchSemaControl = (P_INIT_CMD_PATCH_SEMA_CONTROL) prInitHifTxHeader->rInitWifiCmd.aucBuffer;
	kalMemZero(prPatchSemaControl, sizeof(INIT_CMD_PATCH_SEMA_CONTROL));
	prPatchSemaControl->ucGetSemaphore = PATCH_GET_SEMA_CONTROL;

	/* 4. Send FW_Download command */
	if (nicTxInitCmd(prAdapter, prCmdInfo, prChipInfo->u2TxInitCmdPort) != WLAN_STATUS_SUCCESS) {
		u4Status = WLAN_STATUS_FAILURE;
		DBGLOG(INIT, ERROR, "Fail to transmit image download command\n");
	}
	/* 5. Free CMD Info Packet. */
	cmdBufFreeCmdInfo(prAdapter, prCmdInfo);

	return u4Status;
}

BOOLEAN wlanPatchIsDownloaded(IN P_ADAPTER_T prAdapter)
{
	UINT_8 ucSeqNum, ucPatchStatus;
	WLAN_STATUS rStatus;
	UINT_32 u4Count;

	ucPatchStatus = PATCH_STATUS_NO_SEMA_NEED_PATCH;
	u4Count = 0;

	while (ucPatchStatus == PATCH_STATUS_NO_SEMA_NEED_PATCH) {
		if (u4Count)
			kalMdelay(100);

		rStatus = wlanPatchSendSemaControl(prAdapter, &ucSeqNum);
		if (rStatus != WLAN_STATUS_SUCCESS) {
			DBGLOG(INIT, WARN, "Send patch SEMA control CMD failed!!\n");
			break;
		}

		rStatus = wlanPatchRecvSemaResp(prAdapter, ucSeqNum, &ucPatchStatus);
		if (rStatus != WLAN_STATUS_SUCCESS) {
			DBGLOG(INIT, WARN, "Recv patch SEMA control EVT failed!!\n");
			break;
		}

		u4Count++;

		if (u4Count > 50) {
			DBGLOG(INIT, WARN, "Patch status check timeout!!\n");
			break;
		}
	}

	if (ucPatchStatus == PATCH_STATUS_NO_NEED_TO_PATCH)
		return TRUE;
	else
		return FALSE;
}

WLAN_STATUS wlanPatchSendComplete(IN P_ADAPTER_T prAdapter)
{
	P_CMD_INFO_T prCmdInfo;
	P_INIT_HIF_TX_HEADER_T prInitHifTxHeader;
	UINT_8 ucTC, ucCmdSeqNum;
	WLAN_STATUS u4Status = WLAN_STATUS_SUCCESS;
	struct mt66xx_chip_info *prChipInfo;

	ASSERT(prAdapter);
	prChipInfo = prAdapter->chip_info;

	/* 1. Allocate CMD Info Packet and its Buffer. */
	prCmdInfo = cmdBufAllocateCmdInfo(prAdapter, sizeof(INIT_HIF_TX_HEADER_T));

	if (!prCmdInfo) {
		DBGLOG(INIT, ERROR, "Allocate CMD_INFO_T ==> FAILED.\n");
		return WLAN_STATUS_FAILURE;
	}

	kalMemZero(prCmdInfo->pucInfoBuffer, sizeof(INIT_HIF_TX_HEADER_T));
	prCmdInfo->u2InfoBufLen = sizeof(INIT_HIF_TX_HEADER_T);

#if (CFG_USE_TC4_RESOURCE_FOR_INIT_CMD == 1)
	/* 2. Always use TC4 (TC4 as CPU) */
	ucTC = TC4_INDEX;
#else
	/* 2. Use TC0's resource to send patch finish command.
	 * Only TC0 is allowed because SDIO HW always reports
	 * MCU's TXQ_CNT at TXQ0_CNT in CR4 architecutre)
	 */
	ucTC = TC0_INDEX;
#endif

	/* 3. increase command sequence number */
	ucCmdSeqNum = nicIncreaseCmdSeqNum(prAdapter);

	/* 4. Setup common CMD Info Packet */
	prInitHifTxHeader = (P_INIT_HIF_TX_HEADER_T) (prCmdInfo->pucInfoBuffer);
	prInitHifTxHeader->u2TxByteCount = prCmdInfo->u2InfoBufLen;
	prInitHifTxHeader->u2PQ_ID = INIT_CMD_PQ_ID;
	prInitHifTxHeader->ucPktFt = INIT_PKT_FT_CMD;

	prInitHifTxHeader->rInitWifiCmd.ucCID = INIT_CMD_ID_PATCH_FINISH;
	prInitHifTxHeader->rInitWifiCmd.ucPktTypeID = INIT_CMD_PACKET_TYPE_ID;
	prInitHifTxHeader->rInitWifiCmd.ucSeqNum = ucCmdSeqNum;

	/* 5. Seend WIFI start command */
	while (1) {
		/* 5.1 Acquire TX Resource */
		if (nicTxAcquireResource(prAdapter, ucTC,
			nicTxGetPageCount(prAdapter, prCmdInfo->u2InfoBufLen, TRUE), TRUE) == WLAN_STATUS_RESOURCES) {
			if (nicTxPollingResource(prAdapter, ucTC) != WLAN_STATUS_SUCCESS) {
				u4Status = WLAN_STATUS_FAILURE;
				DBGLOG(INIT, ERROR, "Fail to get TX resource return within timeout\n");
				goto exit;
			}
			continue;
		}
		/* 5.2 Send CMD Info Packet */
		if (nicTxInitCmd(prAdapter, prCmdInfo, prChipInfo->u2TxInitCmdPort) != WLAN_STATUS_SUCCESS) {
			u4Status = WLAN_STATUS_FAILURE;
			DBGLOG(INIT, ERROR, "Fail to transmit WIFI start command\n");
			goto exit;
		}

		break;
	};

	DBGLOG(INIT, INFO, "PATCH FINISH CMD send, waiting for RSP\n");

	/* kalMdelay(10000); */

	u4Status = wlanConfigWifiFuncStatus(prAdapter, ucCmdSeqNum);

	if (u4Status != WLAN_STATUS_SUCCESS)
		DBGLOG(INIT, INFO, "PATCH FINISH EVT failed\n");
	else
		DBGLOG(INIT, INFO, "PATCH FINISH EVT success!!\n");

exit:
	/* 6. Free CMD Info Packet. */
	cmdBufFreeCmdInfo(prAdapter, prCmdInfo);

	return u4Status;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to configure FWDL parameters
*
* @param prAdapter      Pointer to the Adapter structure.
*        u4DestAddr     Address of destination address
*        u4ImgSecSize   Length of the firmware block
*        fgReset        should be set to TRUE if this is the 1st configuration
*
* @return WLAN_STATUS_SUCCESS
*         WLAN_STATUS_FAILURE
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS wlanImageSectionConfig(IN P_ADAPTER_T prAdapter,
				   IN UINT_32 u4DestAddr, IN UINT_32 u4ImgSecSize, IN UINT_32 u4DataMode,
				   IN enum ENUM_IMG_DL_IDX_T eDlIdx)
{
	P_CMD_INFO_T prCmdInfo;
	P_INIT_HIF_TX_HEADER_T prInitHifTxHeader;
	P_INIT_CMD_DOWNLOAD_CONFIG prInitCmdDownloadConfig;
	UINT_8 ucTC, ucCmdSeqNum;
	WLAN_STATUS u4Status = WLAN_STATUS_SUCCESS;
	struct mt66xx_chip_info *prChipInfo;

	ASSERT(prAdapter);
	prChipInfo = prAdapter->chip_info;

	DEBUGFUNC("wlanImageSectionConfig");

	if (u4ImgSecSize == 0)
		return WLAN_STATUS_SUCCESS;
	/* 1. Allocate CMD Info Packet and its Buffer. */
	prCmdInfo = cmdBufAllocateCmdInfo(prAdapter,
		sizeof(INIT_HIF_TX_HEADER_T) + sizeof(INIT_CMD_DOWNLOAD_CONFIG));

	if (!prCmdInfo) {
		DBGLOG(INIT, ERROR, "Allocate CMD_INFO_T ==> FAILED.\n");
		return WLAN_STATUS_FAILURE;
	}

	prCmdInfo->u2InfoBufLen = sizeof(INIT_HIF_TX_HEADER_T) + sizeof(INIT_CMD_DOWNLOAD_CONFIG);

#if (CFG_USE_TC4_RESOURCE_FOR_INIT_CMD == 1)
	/* 2. Use TC4's resource to download image. (TC4 as CPU) */
	ucTC = TC4_INDEX;
#else
	/* 2. Use TC0's resource to send init_cmd.
	 * Only TC0 is allowed because SDIO HW always reports
	 * MCU's TXQ_CNT at TXQ0_CNT in CR4 architecutre)
	 */
	ucTC = TC0_INDEX;
#endif

	/* 3. increase command sequence number */
	ucCmdSeqNum = nicIncreaseCmdSeqNum(prAdapter);

	/* 4. Setup common CMD Info Packet */
	prInitHifTxHeader = (P_INIT_HIF_TX_HEADER_T) (prCmdInfo->pucInfoBuffer);
	prInitHifTxHeader->u2TxByteCount = prCmdInfo->u2InfoBufLen;
	prInitHifTxHeader->u2PQ_ID = INIT_CMD_PQ_ID;
	prInitHifTxHeader->ucHeaderFormat = INIT_CMD_PACKET_TYPE_ID;
	prInitHifTxHeader->ucPktFt = INIT_PKT_FT_CMD;

	if (eDlIdx == IMG_DL_IDX_PATCH)
		prInitHifTxHeader->rInitWifiCmd.ucCID = INIT_CMD_ID_PATCH_START;
	else
		prInitHifTxHeader->rInitWifiCmd.ucCID = INIT_CMD_ID_DOWNLOAD_CONFIG;


	prInitHifTxHeader->rInitWifiCmd.ucPktTypeID = INIT_CMD_PACKET_TYPE_ID;
	prInitHifTxHeader->rInitWifiCmd.ucSeqNum = ucCmdSeqNum;

	/* 5. Setup CMD_DOWNLOAD_CONFIG */
	prInitCmdDownloadConfig = (P_INIT_CMD_DOWNLOAD_CONFIG) (prInitHifTxHeader->rInitWifiCmd.aucBuffer);
	prInitCmdDownloadConfig->u4Address = u4DestAddr;
	prInitCmdDownloadConfig->u4Length = u4ImgSecSize;
	prInitCmdDownloadConfig->u4DataMode = u4DataMode;

	/* 6. Send FW_Download command */
	while (1) {
		/* 6.1 Acquire TX Resource */
		if (nicTxAcquireResource(prAdapter, ucTC,
			nicTxGetPageCount(prAdapter, prCmdInfo->u2InfoBufLen, TRUE), TRUE) == WLAN_STATUS_RESOURCES) {
			if (nicTxPollingResource(prAdapter, ucTC) != WLAN_STATUS_SUCCESS) {
				u4Status = WLAN_STATUS_FAILURE;
				DBGLOG(INIT, ERROR, "Fail to get TX resource return within timeout\n");
				goto exit;
			}
			continue;
		}
		/* 6.2 Send CMD Info Packet */
		if (nicTxInitCmd(prAdapter, prCmdInfo, prChipInfo->u2TxInitCmdPort) != WLAN_STATUS_SUCCESS) {
			u4Status = WLAN_STATUS_FAILURE;
			DBGLOG(INIT, ERROR, "Fail to transmit image download command\n");
			goto exit;
		}

		break;
	};

#if CFG_ENABLE_FW_DOWNLOAD_ACK
	/* 7. Wait for INIT_EVENT_ID_CMD_RESULT */
	u4Status = wlanConfigWifiFuncStatus(prAdapter, ucCmdSeqNum);
#endif

exit:
	/* 8. Free CMD Info Packet. */
	cmdBufFreeCmdInfo(prAdapter, prCmdInfo);

	return u4Status;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to download FW image.
*
* @param prAdapter      Pointer to the Adapter structure.
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS wlanImageSectionDownload(IN P_ADAPTER_T prAdapter, IN UINT_32 u4ImgSecSize, IN PUINT_8 pucImgSecBuf)
{
	P_CMD_INFO_T prCmdInfo;
	P_INIT_HIF_TX_HEADER_T prInitHifTxHeader;
	WLAN_STATUS u4Status = WLAN_STATUS_SUCCESS;
	struct mt66xx_chip_info *prChipInfo;

	ASSERT(prAdapter);
	ASSERT(pucImgSecBuf);
	ASSERT(u4ImgSecSize <= CMD_PKT_SIZE_FOR_IMAGE);

	DEBUGFUNC("wlanImageSectionDownload");

	prChipInfo = prAdapter->chip_info;

	if (u4ImgSecSize == 0)
		return WLAN_STATUS_SUCCESS;
	/* 1. Allocate CMD Info Packet and its Buffer. */
	prCmdInfo = cmdBufAllocateCmdInfo(prAdapter, sizeof(INIT_HIF_TX_HEADER_T) + u4ImgSecSize);

	if (!prCmdInfo) {
		DBGLOG(INIT, ERROR, "Allocate CMD_INFO_T ==> FAILED.\n");
		return WLAN_STATUS_FAILURE;
	}

	prCmdInfo->u2InfoBufLen = sizeof(INIT_HIF_TX_HEADER_T) + (UINT_16) u4ImgSecSize;

	/* 2. Setup common CMD Info Packet */
	prInitHifTxHeader = (P_INIT_HIF_TX_HEADER_T) (prCmdInfo->pucInfoBuffer);
	prInitHifTxHeader->u2TxByteCount = prCmdInfo->u2InfoBufLen;
	prInitHifTxHeader->u2PQ_ID = INIT_CMD_PDA_PQ_ID;
	prInitHifTxHeader->ucHeaderFormat = INIT_CMD_PDA_PACKET_TYPE_ID;
	prInitHifTxHeader->ucPktFt = INIT_PKT_FT_PDA_FWDL;

	prInitHifTxHeader->rInitWifiCmd.ucCID = 0;
	prInitHifTxHeader->rInitWifiCmd.ucPktTypeID = INIT_CMD_PDA_PACKET_TYPE_ID;
	prInitHifTxHeader->rInitWifiCmd.ucSeqNum = 0;

	/* 3. Setup DOWNLOAD_BUF */
	kalMemCopy(prInitHifTxHeader->rInitWifiCmd.aucBuffer, pucImgSecBuf, u4ImgSecSize);

	/* 4. Send FW_Download command */
	if (nicTxInitCmd(prAdapter, prCmdInfo, prChipInfo->u2TxFwDlPort) != WLAN_STATUS_SUCCESS) {
		u4Status = WLAN_STATUS_FAILURE;
		DBGLOG(INIT, ERROR, "Fail to transmit image download command\n");
	}
	/* 5. Free CMD Info Packet. */
	cmdBufFreeCmdInfo(prAdapter, prCmdInfo);

	return u4Status;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to confirm previously firmware download is done without error
*
* @param prAdapter      Pointer to the Adapter structure.
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS wlanImageQueryStatus(IN P_ADAPTER_T prAdapter)
{
	P_CMD_INFO_T prCmdInfo;
	P_INIT_HIF_TX_HEADER_T prInitHifTxHeader;
	UINT_8 aucBuffer[sizeof(INIT_HIF_RX_HEADER_T) + sizeof(INIT_EVENT_PENDING_ERROR)];
	UINT_32 u4RxPktLength;
	P_INIT_HIF_RX_HEADER_T prInitHifRxHeader;
	P_INIT_EVENT_PENDING_ERROR prEventPendingError;
	WLAN_STATUS u4Status = WLAN_STATUS_SUCCESS;
	UINT_8 ucTC, ucCmdSeqNum;
	struct mt66xx_chip_info *prChipInfo;

	ASSERT(prAdapter);
	prChipInfo = prAdapter->chip_info;

	DEBUGFUNC("wlanImageQueryStatus");

	/* 1. Allocate CMD Info Packet and it Buffer. */
	prCmdInfo = cmdBufAllocateCmdInfo(prAdapter, sizeof(INIT_HIF_TX_HEADER_T));

	if (!prCmdInfo) {
		DBGLOG(INIT, ERROR, "Allocate CMD_INFO_T ==> FAILED.\n");
		return WLAN_STATUS_FAILURE;
	}

	kalMemZero(prCmdInfo->pucInfoBuffer, sizeof(INIT_HIF_TX_HEADER_T));
	prCmdInfo->u2InfoBufLen = sizeof(INIT_HIF_TX_HEADER_T);

#if (CFG_USE_TC4_RESOURCE_FOR_INIT_CMD == 1)
		/* 2. Always use TC4 */
		ucTC = TC4_INDEX;
#else
		/* 2. Use TC0's resource to send init_cmd
		 * Only TC0 is allowed because SDIO HW always reports
		 * CPU's TXQ_CNT at TXQ0_CNT in CR4 architecutre)
		 */
		ucTC = TC0_INDEX;
#endif

	/* 3. increase command sequence number */
	ucCmdSeqNum = nicIncreaseCmdSeqNum(prAdapter);

	/* 4. Setup common CMD Info Packet */
	prInitHifTxHeader = (P_INIT_HIF_TX_HEADER_T) (prCmdInfo->pucInfoBuffer);

	prInitHifTxHeader->u2TxByteCount = prCmdInfo->u2InfoBufLen;
	prInitHifTxHeader->u2PQ_ID = INIT_CMD_PQ_ID;

	prInitHifTxHeader->rInitWifiCmd.ucCID = INIT_CMD_ID_QUERY_PENDING_ERROR;
	prInitHifTxHeader->rInitWifiCmd.ucPktTypeID = INIT_CMD_PACKET_TYPE_ID;
	prInitHifTxHeader->rInitWifiCmd.ucSeqNum = ucCmdSeqNum;

	/* 5. Send command */
	while (1) {
		/* 5.1 Acquire TX Resource */
		if (nicTxAcquireResource(prAdapter, ucTC,
			nicTxGetPageCount(prAdapter, prCmdInfo->u2InfoBufLen, TRUE), TRUE) == WLAN_STATUS_RESOURCES) {
			if (nicTxPollingResource(prAdapter, ucTC) != WLAN_STATUS_SUCCESS) {
				u4Status = WLAN_STATUS_FAILURE;
				DBGLOG(INIT, ERROR, "Fail to get TX resource return within timeout\n");
				break;
			}
				continue;

		}
		/* 5.2 Send CMD Info Packet */
		if (nicTxInitCmd(prAdapter, prCmdInfo, prChipInfo->u2TxInitCmdPort) != WLAN_STATUS_SUCCESS) {
			u4Status = WLAN_STATUS_FAILURE;
			DBGLOG(INIT, ERROR, "Fail to transmit image download command\n");
		}

		break;
	};

	/* 6. Wait for INIT_EVENT_ID_PENDING_ERROR */
	do {
		if (kalIsCardRemoved(prAdapter->prGlueInfo) == TRUE || fgIsBusAccessFailed == TRUE) {
			u4Status = WLAN_STATUS_FAILURE;
		} else if (nicRxWaitResponse(prAdapter,
					     0,
					     aucBuffer,
					     sizeof(INIT_HIF_RX_HEADER_T) +
					     sizeof(INIT_EVENT_PENDING_ERROR), &u4RxPktLength) != WLAN_STATUS_SUCCESS) {
			u4Status = WLAN_STATUS_FAILURE;
		} else {
			prInitHifRxHeader = (P_INIT_HIF_RX_HEADER_T) aucBuffer;

			/* EID / SeqNum check */
			if (prInitHifRxHeader->rInitWifiEvent.ucEID != INIT_EVENT_ID_PENDING_ERROR) {
				u4Status = WLAN_STATUS_FAILURE;
			} else if (prInitHifRxHeader->rInitWifiEvent.ucSeqNum != ucCmdSeqNum) {
				u4Status = WLAN_STATUS_FAILURE;
			} else {
				prEventPendingError =
				    (P_INIT_EVENT_PENDING_ERROR) (prInitHifRxHeader->rInitWifiEvent.aucBuffer);
				if (prEventPendingError->ucStatus != 0) {	/* 0 for download success */
					u4Status = WLAN_STATUS_FAILURE;
				} else {
					u4Status = WLAN_STATUS_SUCCESS;
				}
			}
		}
	} while (FALSE);

	/* 7. Free CMD Info Packet. */
	cmdBufFreeCmdInfo(prAdapter, prCmdInfo);

	return u4Status;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to confirm the status of
*        previously downloaded firmware scatter
*
* @param prAdapter      Pointer to the Adapter structure.
*        ucCmdSeqNum    Sequence number of previous firmware scatter
*
* @return WLAN_STATUS_SUCCESS
*         WLAN_STATUS_FAILURE
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS wlanConfigWifiFuncStatus(IN P_ADAPTER_T prAdapter, IN UINT_8 ucCmdSeqNum)
{
	UINT_8 aucBuffer[sizeof(INIT_HIF_RX_HEADER_T) + sizeof(INIT_EVENT_CMD_RESULT)];
	P_INIT_HIF_RX_HEADER_T prInitHifRxHeader;
	P_INIT_EVENT_CMD_RESULT prEventCmdResult;
	UINT_32 u4RxPktLength;
	WLAN_STATUS u4Status;

	UINT_8 ucPortIdx = IMG_DL_STATUS_PORT_IDX;

	ASSERT(prAdapter);

	do {
		if (kalIsCardRemoved(prAdapter->prGlueInfo) == TRUE || fgIsBusAccessFailed == TRUE) {
			u4Status = WLAN_STATUS_FAILURE;
		} else if (nicRxWaitResponse(prAdapter,
					     ucPortIdx,
					     aucBuffer,
					     sizeof(INIT_HIF_RX_HEADER_T) +
					     sizeof(INIT_EVENT_CMD_RESULT), &u4RxPktLength) != WLAN_STATUS_SUCCESS) {
			u4Status = WLAN_STATUS_FAILURE;
		} else {
			prInitHifRxHeader = (P_INIT_HIF_RX_HEADER_T) aucBuffer;

			/* EID / SeqNum check */
			if (prInitHifRxHeader->rInitWifiEvent.ucEID != INIT_EVENT_ID_CMD_RESULT) {
				u4Status = WLAN_STATUS_FAILURE;
			} else if (prInitHifRxHeader->rInitWifiEvent.ucSeqNum != ucCmdSeqNum) {
				u4Status = WLAN_STATUS_FAILURE;
			} else {
				prEventCmdResult =
				    (P_INIT_EVENT_CMD_RESULT) (prInitHifRxHeader->rInitWifiEvent.aucBuffer);
				if (prEventCmdResult->ucStatus != 0) {	/* 0 for download success */
					DBGLOG(INIT, ERROR, "Start CMD failed, status[%u]\n",
					       prEventCmdResult->ucStatus);
#if CFG_SUPPORT_COMPRESSION_FW_OPTION
				if (prEventCmdResult->ucStatus == WIFI_FW_DECOMPRESSION_FAILED)
					DBGLOG(INIT, ERROR, "Start Decompression CMD failed, status[%u]\n",
					       prEventCmdResult->ucStatus);
#endif
					u4Status = WLAN_STATUS_FAILURE;
				} else {
					u4Status = WLAN_STATUS_SUCCESS;
				}
			}
		}
	} while (FALSE);

	return u4Status;
}

WLAN_STATUS wlanConfigWifiFunc(IN P_ADAPTER_T prAdapter,
			       IN BOOLEAN fgEnable, IN UINT_32 u4StartAddress, IN UINT_8 ucPDA)
{
	P_CMD_INFO_T prCmdInfo;
	P_INIT_HIF_TX_HEADER_T prInitHifTxHeader;
	P_INIT_CMD_WIFI_START prInitCmdWifiStart;
	UINT_8 ucTC, ucCmdSeqNum;
	WLAN_STATUS u4Status = WLAN_STATUS_SUCCESS;
	struct mt66xx_chip_info *prChipInfo;

	ASSERT(prAdapter);
	prChipInfo = prAdapter->chip_info;

	DEBUGFUNC("wlanConfigWifiFunc");

	/* 1. Allocate CMD Info Packet and its Buffer. */
	prCmdInfo = cmdBufAllocateCmdInfo(prAdapter, sizeof(INIT_HIF_TX_HEADER_T) + sizeof(INIT_CMD_WIFI_START));

	if (!prCmdInfo) {
		DBGLOG(INIT, ERROR, "Allocate CMD_INFO_T ==> FAILED.\n");
		return WLAN_STATUS_FAILURE;
	}

	kalMemZero(prCmdInfo->pucInfoBuffer, sizeof(INIT_HIF_TX_HEADER_T) + sizeof(INIT_CMD_WIFI_START));
	prCmdInfo->u2InfoBufLen = sizeof(INIT_HIF_TX_HEADER_T) + sizeof(INIT_CMD_WIFI_START);

#if (CFG_USE_TC4_RESOURCE_FOR_INIT_CMD == 1)
	/* 2. Always use TC4 (TC4 as CPU) */
	ucTC = TC4_INDEX;
#else
	/* 2. Use TC0's resource to send init_cmd.
	 * Only TC0 is allowed because SDIO HW always reports
	 * CPU's TXQ_CNT at TXQ0_CNT in CR4 architecutre)
	 */
	ucTC = TC0_INDEX;
#endif

	/* 3. increase command sequence number */
	ucCmdSeqNum = nicIncreaseCmdSeqNum(prAdapter);

	/* 4. Setup common CMD Info Packet */
	prInitHifTxHeader = (P_INIT_HIF_TX_HEADER_T) (prCmdInfo->pucInfoBuffer);
	prInitHifTxHeader->u2TxByteCount = prCmdInfo->u2InfoBufLen;
	prInitHifTxHeader->u2PQ_ID = INIT_CMD_PQ_ID;
	prInitHifTxHeader->ucPktFt = INIT_PKT_FT_CMD;

	prInitHifTxHeader->rInitWifiCmd.ucCID = INIT_CMD_ID_WIFI_START;
	prInitHifTxHeader->rInitWifiCmd.ucPktTypeID = INIT_CMD_PACKET_TYPE_ID;
	prInitHifTxHeader->rInitWifiCmd.ucSeqNum = ucCmdSeqNum;

	prInitCmdWifiStart = (P_INIT_CMD_WIFI_START) (prInitHifTxHeader->rInitWifiCmd.aucBuffer);
	prInitCmdWifiStart->u4Override = 0;
	if (fgEnable)
		prInitCmdWifiStart->u4Override |= START_OVERRIDE_START_ADDRESS;

    /* 5G cal until send efuse buffer mode CMD */
#if (CFG_EFUSE_BUFFER_MODE_DELAY_CAL == 1)
	if (prAdapter->fgIsSupportDelayCal == TRUE)
		prInitCmdWifiStart->u4Override |= START_DELAY_CALIBRATION;
#endif

	if (ucPDA == PDA_CR4)
		prInitCmdWifiStart->u4Override |= START_WORKING_PDA_OPTION;

	prInitCmdWifiStart->u4Address = u4StartAddress;

	/* 5. Seend WIFI start command */
	while (1) {
		/* 5.1 Acquire TX Resource */
		if (nicTxAcquireResource(prAdapter, ucTC,
			nicTxGetPageCount(prAdapter, prCmdInfo->u2InfoBufLen, TRUE), TRUE) == WLAN_STATUS_RESOURCES) {
			if (nicTxPollingResource(prAdapter, ucTC) != WLAN_STATUS_SUCCESS) {
				u4Status = WLAN_STATUS_FAILURE;
				DBGLOG(INIT, ERROR, "Fail to get TX resource return within timeout\n");
				goto exit;
			}
			continue;
		}
		/* 5.2 Send CMD Info Packet */
		if (nicTxInitCmd(prAdapter, prCmdInfo, prChipInfo->u2TxInitCmdPort) != WLAN_STATUS_SUCCESS) {
			u4Status = WLAN_STATUS_FAILURE;
			DBGLOG(INIT, ERROR, "Fail to transmit WIFI start command\n");
			goto exit;
		}

		break;
	};

	DBGLOG(INIT, INFO, "FW_START CMD send, waiting for RSP\n");

	u4Status = wlanConfigWifiFuncStatus(prAdapter, ucCmdSeqNum);

	if (u4Status != WLAN_STATUS_SUCCESS)
		DBGLOG(INIT, INFO, "FW_START EVT failed\n");
	else
		DBGLOG(INIT, INFO, "FW_START EVT success!!\n");

exit:
	/* 6. Free CMD Info Packet. */
	cmdBufFreeCmdInfo(prAdapter, prCmdInfo);

	return u4Status;
}
#if CFG_SUPPORT_COMPRESSION_FW_OPTION
WLAN_STATUS
wlanCompressedFWConfigWifiFunc(IN P_ADAPTER_T prAdapter, IN BOOLEAN fgEnable,
	IN UINT_32 u4StartAddress, IN UINT_8 ucPDA, IN P_INIT_CMD_WIFI_DECOMPRESSION_START prFwImageInFo)
{
	P_CMD_INFO_T prCmdInfo;
	P_INIT_HIF_TX_HEADER_T prInitHifTxHeader;
	P_INIT_CMD_WIFI_DECOMPRESSION_START prInitCmdWifiStart;
	UINT_8 ucTC, ucCmdSeqNum;
	WLAN_STATUS u4Status = WLAN_STATUS_SUCCESS;
	struct mt66xx_chip_info *prChipInfo;

	ASSERT(prAdapter);
	prChipInfo = prAdapter->chip_info;

	DEBUGFUNC("wlanConfigWifiFunc");
	/* 1. Allocate CMD Info Packet and its Buffer. */
	prCmdInfo = cmdBufAllocateCmdInfo(prAdapter,
					  sizeof(INIT_HIF_TX_HEADER_T) +
					  sizeof(INIT_CMD_WIFI_DECOMPRESSION_START));

	if (!prCmdInfo) {
		DBGLOG(INIT, ERROR, "Allocate CMD_INFO_T ==> FAILED.\n");
		return WLAN_STATUS_FAILURE;
	}

	kalMemZero(prCmdInfo->pucInfoBuffer,
		sizeof(INIT_HIF_TX_HEADER_T) + sizeof(INIT_CMD_WIFI_DECOMPRESSION_START));
	prCmdInfo->u2InfoBufLen =
		sizeof(INIT_HIF_TX_HEADER_T) + sizeof(INIT_CMD_WIFI_DECOMPRESSION_START);

	/* 2. Always use TC0 */
	ucTC = TC0_INDEX;

	/* 3. increase command sequence number */
	ucCmdSeqNum = nicIncreaseCmdSeqNum(prAdapter);

	/* 4. Setup common CMD Info Packet */
	prInitHifTxHeader = (P_INIT_HIF_TX_HEADER_T) (prCmdInfo->pucInfoBuffer);
	prInitHifTxHeader->u2TxByteCount = prCmdInfo->u2InfoBufLen;
	prInitHifTxHeader->u2PQ_ID = INIT_CMD_PQ_ID;
	prInitHifTxHeader->ucPktFt = INIT_PKT_FT_CMD;
	prInitHifTxHeader->rInitWifiCmd.ucCID = INIT_CMD_ID_DECOMPRESSED_WIFI_START;
	prInitHifTxHeader->rInitWifiCmd.ucPktTypeID = INIT_CMD_PACKET_TYPE_ID;
	prInitHifTxHeader->rInitWifiCmd.ucSeqNum = ucCmdSeqNum;

	prInitCmdWifiStart = (P_INIT_CMD_WIFI_DECOMPRESSION_START) (prInitHifTxHeader->rInitWifiCmd.aucBuffer);
	prInitCmdWifiStart->u4Override = 0;
	if (fgEnable)
		prInitCmdWifiStart->u4Override |= START_OVERRIDE_START_ADDRESS;

    /* 5G cal until send efuse buffer mode CMD */
#if (CFG_EFUSE_BUFFER_MODE_DELAY_CAL == 1)
	if (prAdapter->fgIsSupportDelayCal == TRUE)
		prInitCmdWifiStart->u4Override |= START_DELAY_CALIBRATION;
#endif
	if (ucPDA == PDA_CR4)
		prInitCmdWifiStart->u4Override |= START_WORKING_PDA_OPTION;

#if CFG_COMPRESSION_DEBUG
		prInitCmdWifiStart->u4Override |= START_CRC_CHECK;
#endif
#if CFG_DECOMPRESSION_TMP_ADDRESS
		prInitCmdWifiStart->u4Override |= CHANGE_DECOMPRESSION_TMP_ADDRESS;
		prInitCmdWifiStart->u4DecompressTmpAddress = 0xE6000;
#endif
	prInitCmdWifiStart->u4Address = u4StartAddress;
	prInitCmdWifiStart->u4Region1Address = prFwImageInFo->u4Region1Address;
	prInitCmdWifiStart->u4Region1CRC = prFwImageInFo->u4Region1CRC;
	prInitCmdWifiStart->u4BlockSize = prFwImageInFo->u4BlockSize;
	prInitCmdWifiStart->u4Region1length = prFwImageInFo->u4Region1length;
	prInitCmdWifiStart->u4Region2Address = prFwImageInFo->u4Region2Address;
	prInitCmdWifiStart->u4Region2CRC = prFwImageInFo->u4Region2CRC;
	prInitCmdWifiStart->u4Region2length = prFwImageInFo->u4Region2length;

	while (1) {
		/* 5.1 Acquire TX Resource */
		if (nicTxAcquireResource(prAdapter, ucTC,
			nicTxGetPageCount(prAdapter, prCmdInfo->u2InfoBufLen, TRUE), TRUE) == WLAN_STATUS_RESOURCES) {
			if (nicTxPollingResource(prAdapter, ucTC) != WLAN_STATUS_SUCCESS) {
				u4Status = WLAN_STATUS_FAILURE;
				DBGLOG(INIT, ERROR, "Fail to get TX resource return within timeout\n");
				break;
			}
				continue;

		}
		/* 5.2 Send CMD Info Packet */
		if (nicTxInitCmd(prAdapter, prCmdInfo, prChipInfo->u2TxInitCmdPort) != WLAN_STATUS_SUCCESS) {
			u4Status = WLAN_STATUS_FAILURE;
			DBGLOG(INIT, ERROR, "Fail to transmit WIFI start command\n");
		}

		break;
	};

	DBGLOG(INIT, INFO, "FW_START CMD send, waiting for RSP\n");

	u4Status = wlanConfigWifiFuncStatus(prAdapter, ucCmdSeqNum);

	if (u4Status != WLAN_STATUS_SUCCESS)
		DBGLOG(INIT, INFO, "FW_START EVT failed\n");
	else
		DBGLOG(INIT, INFO, "FW_START EVT success!!\n");


	/* 6. Free CMD Info Packet. */
	cmdBufFreeCmdInfo(prAdapter, prCmdInfo);

	return u4Status;
}
#endif
#if 0
/*----------------------------------------------------------------------------*/
/*!
* @brief This function is used to generate CRC32 checksum
*
* @param buf Pointer to the data.
* @param len data length
*
* @return crc32 value
*/
/*----------------------------------------------------------------------------*/
UINT_32 wlanCRC32(PUINT_8 buf, UINT_32 len)
{
	UINT_32 i, crc32 = 0xFFFFFFFF;
	const UINT_32 crc32_ccitt_table[256] = {
		0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419,
		0x706af48f, 0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4,
		0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07,
		0x90bf1d91, 0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
		0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7, 0x136c9856,
		0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
		0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4,
		0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
		0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3,
		0x45df5c75, 0xdcd60dcf, 0xabd13d59, 0x26d930ac, 0x51de003a,
		0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599,
		0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
		0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190,
		0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f,
		0x9fbfe4a5, 0xe8b8d433, 0x7807c9a2, 0x0f00f934, 0x9609a88e,
		0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
		0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed,
		0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
		0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3,
		0xfbd44c65, 0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
		0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a,
		0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5,
		0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa, 0xbe0b1010,
		0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
		0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17,
		0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6,
		0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615,
		0x73dc1683, 0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
		0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1, 0xf00f9344,
		0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
		0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a,
		0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
		0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1,
		0xa6bc5767, 0x3fb506dd, 0x48b2364b, 0xd80d2bda, 0xaf0a1b4c,
		0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef,
		0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
		0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe,
		0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31,
		0x2cd99e8b, 0x5bdeae1d, 0x9b64c2b0, 0xec63f226, 0x756aa39c,
		0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
		0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b,
		0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
		0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1,
		0x18b74777, 0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
		0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45, 0xa00ae278,
		0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7,
		0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc, 0x40df0b66,
		0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
		0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605,
		0xcdd70693, 0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8,
		0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b,
		0x2d02ef8d
	};

	for (i = 0; i < len; i++)
		crc32 = crc32_ccitt_table[(crc32 ^ buf[i]) & 0xff] ^ (crc32 >> 8);

	return ~crc32;
}
#endif

WLAN_STATUS wlanGetHarvardTailerInfo(IN P_ADAPTER_T prAdapter, IN PVOID prFwBuffer, IN UINT_32 u4FwSize,
				     IN UINT_32 ucTotSecNum, IN enum ENUM_IMG_DL_IDX_T eDlIdx)
{
	struct TAILER_FORMAT_T *prTailers;
	PUINT_8 pucStartPtr;
	UINT_32 u4SecIdx;
	UINT_8 aucBuf[32];

	pucStartPtr = prFwBuffer + u4FwSize - sizeof(struct TAILER_FORMAT_T) * ucTotSecNum;
	if (eDlIdx == IMG_DL_IDX_N9_FW) {
		kalMemCopy(&prAdapter->rVerInfo.rN9tailer, pucStartPtr,
			   sizeof(struct TAILER_FORMAT_T) * ucTotSecNum);
		prTailers = prAdapter->rVerInfo.rN9tailer;
	} else {
		kalMemCopy(&prAdapter->rVerInfo.rCR4tailer, pucStartPtr,
			   sizeof(struct TAILER_FORMAT_T) * ucTotSecNum);
		prTailers = prAdapter->rVerInfo.rCR4tailer;
	}

	for (u4SecIdx = 0; u4SecIdx < ucTotSecNum; u4SecIdx++) {
		/* Dump image information */
		DBGLOG(INIT, INFO, "%s Section[%d]: chip_info[%u:E%u] feature[0x%02X]\n",
		       (eDlIdx == IMG_DL_IDX_N9_FW) ? "N9" : "CR4", u4SecIdx, prTailers[u4SecIdx].chip_info,
			prTailers[u4SecIdx].eco_code + 1, prTailers[u4SecIdx].feature_set);

		kalMemZero(aucBuf, 32);
		kalStrnCpy(aucBuf, prTailers[u4SecIdx].ram_version, sizeof(prTailers[u4SecIdx].ram_version));
		DBGLOG(INIT, INFO, "date[%s] version[%s]\n", prTailers[u4SecIdx].ram_built_date, aucBuf);
	}

	return WLAN_STATUS_SUCCESS;
}

WLAN_STATUS wlanGetConnacTailerInfo(IN P_ADAPTER_T prAdapter, IN PVOID prFwBuffer,
				    IN UINT_32 u4FwSize, IN enum ENUM_IMG_DL_IDX_T eDlIdx)
{
	P_WIFI_VER_INFO_T prVerInfo = &prAdapter->rVerInfo;
	struct TAILER_COMMON_FORMAT_T *prComTailer;
	struct TAILER_REGION_FORMAT_T *prRegTailer;
	PUINT_8 pucStartPtr;
	UINT_32 u4SecIdx;
	UINT_8 aucBuf[32];

	pucStartPtr = prFwBuffer + u4FwSize - sizeof(struct TAILER_COMMON_FORMAT_T);
	prComTailer = (struct TAILER_COMMON_FORMAT_T *) pucStartPtr;
	kalMemCopy(&prVerInfo->rCommonTailer, prComTailer, sizeof(struct TAILER_COMMON_FORMAT_T));

	/* Dump image information */
	DBGLOG(INIT, INFO, "%s INFO: chip_info[%u:E%u] region_num[%d]\n",
		(eDlIdx == IMG_DL_IDX_N9_FW) ? "N9" : "CR4", prComTailer->ucChipInfo,
		prComTailer->ucEcoCode + 1, prComTailer->ucRegionNum);

	kalMemZero(aucBuf, 32);
	kalStrnCpy(aucBuf, prComTailer->aucRamVersion, sizeof(prComTailer->aucRamVersion));
	DBGLOG(INIT, INFO, "date[%s] version[%s]\n",
		prComTailer->aucRamBuiltDate, aucBuf);

	if (prComTailer->ucRegionNum > MAX_FWDL_SECTION_NUM) {
		DBGLOG(INIT, INFO, "Regions number[%d] > max section number[%d]\n",
		       prComTailer->ucRegionNum, MAX_FWDL_SECTION_NUM);
		return WLAN_STATUS_FAILURE;
	}

	pucStartPtr -= (prComTailer->ucRegionNum * sizeof(struct TAILER_REGION_FORMAT_T));
	for (u4SecIdx = 0; u4SecIdx < prComTailer->ucRegionNum; u4SecIdx++) {
		prRegTailer = (struct TAILER_REGION_FORMAT_T *) pucStartPtr;
		kalMemCopy(&prVerInfo->rRegionTailers[u4SecIdx], prRegTailer, sizeof(struct TAILER_REGION_FORMAT_T));

		/* Dump image information */
		DBGLOG(INIT, INFO, "Region[%d]: addr[0x%08X] feature[0x%02X] size[%u]\n",
		       u4SecIdx, prRegTailer->u4Addr, prRegTailer->ucFeatureSet, prRegTailer->u4Len);
		DBGLOG(INIT, INFO, "uncompress_crc[0x%08X] uncompress_size[0x%08X] block_size[0x%08X]\n",
		       prRegTailer->u4CRC, prRegTailer->u4RealSize, prRegTailer->u4BlockSize);
		pucStartPtr += sizeof(struct TAILER_REGION_FORMAT_T);
	}

	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is used to extract the wifi ram code start address
*
* @param prVerInfo      Pointer to the P_WIFI_VER_INFO_T structure.
*
* @return addr          The ram code entry address.
*                       0: use firmware defaut setting
*                       others: use it as the start address
*/
/*----------------------------------------------------------------------------*/

UINT_32 wlanDetectRamEntry(IN P_WIFI_VER_INFO_T prVerInfo)
{
	UINT_32 addr = 0;
	UINT_32 u4SecIdx;
	struct TAILER_COMMON_FORMAT_T *prComTailer = &prVerInfo->rCommonTailer;
	struct TAILER_REGION_FORMAT_T *prRegTailer;

	for (u4SecIdx = 0; u4SecIdx < prComTailer->ucRegionNum; u4SecIdx++) {
		prRegTailer = &(prVerInfo->rRegionTailers[u4SecIdx]);

		if (prRegTailer->ucFeatureSet & DOWNLOAD_CONFIG_VALID_RAM_ENTRY) {
			addr = prRegTailer->u4Addr;
			break;
		}
	}

	return addr;
}

WLAN_STATUS wlanHarvardFormatDownload(IN P_ADAPTER_T prAdapter, IN enum ENUM_IMG_DL_IDX_T eDlIdx)
{
	UINT_32 u4FwSize = 0;
	PVOID prFwBuffer = NULL;
	WLAN_STATUS rDlStatus = 0;
	WLAN_STATUS rCfgStatus = 0;
	UINT_32 ucTotSecNum;
	UINT_8 ucPDA;
#if CFG_SUPPORT_COMPRESSION_FW_OPTION
	BOOLEAN fgIsCompressed = FALSE;
	INIT_CMD_WIFI_DECOMPRESSION_START rFwImageInFo;
#endif

	if (eDlIdx == IMG_DL_IDX_N9_FW) {
		ucTotSecNum = N9_FWDL_SECTION_NUM;
		ucPDA = PDA_N9;
	} else {
		ucTotSecNum = CR4_FWDL_SECTION_NUM;
		ucPDA = PDA_CR4;
	}

	kalFirmwareImageMapping(prAdapter->prGlueInfo, &prFwBuffer, &u4FwSize, eDlIdx);
	if (prFwBuffer == NULL) {
		DBGLOG(INIT, WARN, "FW[%u] load error!\n", eDlIdx);
		return WLAN_STATUS_FAILURE;
	}

	wlanGetHarvardTailerInfo(prAdapter, prFwBuffer, u4FwSize, ucTotSecNum, eDlIdx);
#if CFG_SUPPORT_COMPRESSION_FW_OPTION
	rDlStatus = wlanCompressedImageSectionDownloadStage(prAdapter, prFwBuffer, u4FwSize, ucTotSecNum,
		eDlIdx, &fgIsCompressed, &rFwImageInFo);
	if (eDlIdx == IMG_DL_IDX_CR4_FW)
		prAdapter->fgIsCr4FwDownloaded = TRUE;
	if (fgIsCompressed == TRUE)
		rCfgStatus = wlanCompressedFWConfigWifiFunc(prAdapter, FALSE, 0, ucPDA, &rFwImageInFo);
	else
		rCfgStatus = wlanConfigWifiFunc(prAdapter, FALSE, 0, ucPDA);
#else
	rDlStatus = wlanImageSectionDownloadStage(prAdapter, prFwBuffer, u4FwSize, ucTotSecNum, eDlIdx);
	if (eDlIdx == IMG_DL_IDX_CR4_FW)
		prAdapter->fgIsCr4FwDownloaded = TRUE;
	rCfgStatus = wlanConfigWifiFunc(prAdapter, FALSE, 0, ucPDA);
#endif
	kalFirmwareImageUnmapping(prAdapter->prGlueInfo, NULL, prFwBuffer);

	if ((rDlStatus != WLAN_STATUS_SUCCESS) || (rCfgStatus != WLAN_STATUS_SUCCESS))
		return WLAN_STATUS_FAILURE;

	return WLAN_STATUS_SUCCESS;
}

WLAN_STATUS wlanConnacFormatDownload(IN P_ADAPTER_T prAdapter, IN enum ENUM_IMG_DL_IDX_T eDlIdx)
{
	PVOID prFwBuffer = NULL;
	UINT_32 u4FwSize = 0;
	UINT_32 ram_entry = 0;
	WLAN_STATUS rDlStatus = 0;
	WLAN_STATUS rCfgStatus = 0;
	UINT_8 ucRegionNum;
	UINT_8 ucPDA;

	kalFirmwareImageMapping(prAdapter->prGlueInfo, &prFwBuffer, &u4FwSize, eDlIdx);
	if (prFwBuffer == NULL) {
		DBGLOG(INIT, WARN, "FW[%u] load error!\n", eDlIdx);
		return WLAN_STATUS_FAILURE;
	}

	if (wlanGetConnacTailerInfo(prAdapter, prFwBuffer, u4FwSize, eDlIdx) != WLAN_STATUS_SUCCESS) {
		DBGLOG(INIT, WARN, "Get tailer info error!\n");
		return WLAN_STATUS_FAILURE;
	}

	ucRegionNum = prAdapter->rVerInfo.rCommonTailer.ucRegionNum;
	ucPDA = (eDlIdx == IMG_DL_IDX_N9_FW) ? PDA_N9 : PDA_CR4;

	rDlStatus = wlanImageSectionDownloadStage(prAdapter, prFwBuffer, u4FwSize, ucRegionNum, eDlIdx);

	ram_entry = wlanDetectRamEntry(&prAdapter->rVerInfo);
	rCfgStatus = wlanConfigWifiFunc(prAdapter, (ram_entry == 0)?FALSE:TRUE, ram_entry, ucPDA);

	kalFirmwareImageUnmapping(prAdapter->prGlueInfo, NULL, prFwBuffer);

	if ((rDlStatus != WLAN_STATUS_SUCCESS) || (rCfgStatus != WLAN_STATUS_SUCCESS))
		return WLAN_STATUS_FAILURE;

	return WLAN_STATUS_SUCCESS;
}

WLAN_STATUS wlanDownloadFW(IN P_ADAPTER_T prAdapter)
{
	WLAN_STATUS rStatus = 0;
	BOOLEAN fgReady;
	struct mt66xx_chip_info *prChipInfo;

	if (!prAdapter)
		return WLAN_STATUS_FAILURE;

	prChipInfo = prAdapter->chip_info;
	DBGLOG(INIT, INFO, "wlanDownloadFW:: Check ready_bits(=0x%x)\n", prChipInfo->sw_ready_bits);
	HAL_WIFI_FUNC_READY_CHECK(prAdapter, prChipInfo->sw_ready_bits, &fgReady);

	if (fgReady) {
		DBGLOG(INIT, INFO, "Wi-Fi is already ON!, turn off before FW DL!\n");

		if (wlanPowerOffWifi(prAdapter) != WLAN_STATUS_SUCCESS)
			return WLAN_STATUS_FAILURE;

		nicpmWakeUpWiFi(prAdapter);
		HAL_HIF_INIT(prAdapter);
	}

	HAL_ENABLE_FWDL(prAdapter, TRUE);

#if (MTK_WCN_HIF_SDIO == 0)
	wlanDownloadPatch(prAdapter);
#endif

	DBGLOG(INIT, INFO, "FW download Start\n");

	rStatus = prChipInfo->fw_dl_ops->downloadFirmware(prAdapter, IMG_DL_IDX_N9_FW);
	if (prChipInfo->is_support_cr4 && rStatus == WLAN_STATUS_SUCCESS)
		rStatus = prChipInfo->fw_dl_ops->downloadFirmware(prAdapter, IMG_DL_IDX_CR4_FW);

	DBGLOG(INIT, INFO, "FW download End\n");

	HAL_ENABLE_FWDL(prAdapter, FALSE);

	return rStatus;
}

WLAN_STATUS wlanDownloadPatch(IN P_ADAPTER_T prAdapter)
{
	UINT_32 u4FwSize = 0;
	PVOID prFwBuffer = NULL;
#if CFG_SUPPORT_COMPRESSION_FW_OPTION
	UINT_8 ucIsCompressed;
#endif
	if (!prAdapter)
		return WLAN_STATUS_FAILURE;


	DBGLOG(INIT, INFO, "Patch download start\n");

	prAdapter->rVerInfo.fgPatchIsDlByDrv = FALSE;

	kalFirmwareImageMapping(prAdapter->prGlueInfo, &prFwBuffer, &u4FwSize, IMG_DL_IDX_PATCH);
	if (prFwBuffer == NULL) {
		DBGLOG(INIT, WARN, "FW[%u] load error!\n", IMG_DL_IDX_PATCH);
		return WLAN_STATUS_FAILURE;
	}

	if (wlanPatchIsDownloaded(prAdapter)) {
		kalFirmwareImageUnmapping(prAdapter->prGlueInfo, NULL, prFwBuffer);
		DBGLOG(INIT, INFO, "No need to download patch\n");
		return WLAN_STATUS_SUCCESS;
	}

	/* Patch DL */
	do {
#if CFG_SUPPORT_COMPRESSION_FW_OPTION
		wlanCompressedImageSectionDownloadStage(prAdapter, prFwBuffer, u4FwSize, 1,
			IMG_DL_IDX_PATCH, &ucIsCompressed, NULL);
#else
		wlanImageSectionDownloadStage(prAdapter, prFwBuffer, u4FwSize, 1, IMG_DL_IDX_PATCH);
#endif
		wlanPatchSendComplete(prAdapter);
		kalFirmwareImageUnmapping(prAdapter->prGlueInfo, NULL, prFwBuffer);

		prAdapter->rVerInfo.fgPatchIsDlByDrv = TRUE;
	} while (0);

	DBGLOG(INIT, INFO, "Patch download end\n");

	return WLAN_STATUS_SUCCESS;
}

WLAN_STATUS wlanGetPatchInfo(IN P_ADAPTER_T prAdapter)
{
	UINT_32 u4FwSize = 0;
	PVOID prFwBuffer = NULL;
	UINT_32 u4StartOffset, u4Addr, u4Len, u4DataMode;

	if (!prAdapter)
		return WLAN_STATUS_FAILURE;

	kalFirmwareImageMapping(prAdapter->prGlueInfo, &prFwBuffer, &u4FwSize, IMG_DL_IDX_PATCH);
	if (prFwBuffer == NULL) {
		DBGLOG(INIT, WARN, "FW[%u] load error!\n", IMG_DL_IDX_PATCH);
		return WLAN_STATUS_FAILURE;
	}

	wlanImageSectionGetPatchInfo(prAdapter, prFwBuffer, u4FwSize, &u4StartOffset, &u4Addr, &u4Len, &u4DataMode);

	kalFirmwareImageUnmapping(prAdapter->prGlueInfo, NULL, prFwBuffer);

	return WLAN_STATUS_SUCCESS;
}

VOID wlanPrintFwdlInfo(IN P_ADAPTER_T prAdapter)
{
	P_WIFI_VER_INFO_T prVerInfo = &prAdapter->rVerInfo;
	struct TAILER_COMMON_FORMAT_T *prComTailer = &prAdapter->rVerInfo.rCommonTailer;
	struct mt66xx_chip_info *prChipInfo = prAdapter->chip_info;
	UINT_8 aucBuf[32];
#if CFG_SUPPORT_COMPRESSION_FW_OPTION
	struct TAILER_FORMAT_T_2 *prTailer;
#else
	struct TAILER_FORMAT_T *prTailer;
#endif

	kalMemZero(aucBuf, 32);

	if (prChipInfo->fw_dl_ops->tailer_format == HARVARD_TAILER_FORMAT) {
#if CFG_SUPPORT_COMPRESSION_FW_OPTION
		prTailer = &prVerInfo->rN9Compressedtailer;
#else
		prTailer = &prVerInfo->rN9tailer[0];
#endif
		kalMemCopy(aucBuf, prTailer->ram_version, 10);
		DBGLOG(SW4, INFO, "N9 tailer version %s (%s) info %u:E%u\n",
			aucBuf, prTailer->ram_built_date, prTailer->chip_info,
			prTailer->eco_code + 1);

		if (prChipInfo->is_support_cr4) {
#if CFG_SUPPORT_COMPRESSION_FW_OPTION
			prTailer = &prVerInfo->rCR4Compressedtailer;
#else
			prTailer = &prVerInfo->rCR4tailer[0];
#endif
			kalMemCopy(aucBuf, prTailer->ram_version, 10);
			DBGLOG(SW4, INFO, "CR4 tailer version %s (%s) info %u:E%u\n",
				aucBuf, prTailer->ram_built_date, prTailer->chip_info,
				prTailer->eco_code + 1);
		}
	} else {
		kalMemCopy(aucBuf, prComTailer->aucRamVersion, 10);
		DBGLOG(SW4, INFO, "N9 tailer version %s (%s) info %u:E%u\n",
			aucBuf, prComTailer->aucRamBuiltDate, prComTailer->ucChipInfo,
			prComTailer->ucEcoCode + 1);
	}

	if (!prVerInfo->fgPatchIsDlByDrv) {
		DBGLOG(SW4, TRACE, "MCU patch is not downloaded by wlan driver, read patch info\n");
		wlanGetPatchInfo(prAdapter);
	}
}

#endif  /* CFG_ENABLE_FW_DOWNLOAD */
