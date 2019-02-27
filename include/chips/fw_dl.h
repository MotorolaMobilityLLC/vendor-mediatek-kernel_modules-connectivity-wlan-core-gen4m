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
/*! \file  fw_dl.h
*/

#ifndef _FW_DL_H
#define _FW_DL_H

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

/* PDA - Patch Decryption Accelerator */
#define PDA_N9                 0
#define PDA_CR4                1

#define MAX_FWDL_SECTION_NUM   10
#define N9_FWDL_SECTION_NUM    2
#define CR4_FWDL_SECTION_NUM   HIF_CR4_FWDL_SECTION_NUM
#define IMG_DL_STATUS_PORT_IDX HIF_IMG_DL_STATUS_PORT_IDX

#define DOWNLOAD_CONFIG_ENCRYPTION_MODE     BIT(0)
#define DOWNLOAD_CONFIG_KEY_INDEX_MASK		BITS(1, 2)
#define DOWNLOAD_CONFIG_RESET_OPTION        BIT(3)
#define DOWNLOAD_CONFIG_WORKING_PDA_OPTION	BIT(4)
#define DOWNLOAD_CONFIG_VALID_RAM_ENTRY	    BIT(5)
#define DOWNLOAD_CONFIG_EMI			BIT(7)
#define DOWNLOAD_CONFIG_ACK_OPTION          BIT(31)

#if CFG_SUPPORT_COMPRESSION_FW_OPTION
#define COMPRESSION_OPTION_OFFSET   4
#define COMPRESSION_OPTION_MASK     BIT(4)
#endif

enum ENUM_IMG_DL_IDX_T {
	IMG_DL_IDX_N9_FW,
	IMG_DL_IDX_CR4_FW,
	IMG_DL_IDX_PATCH
};

enum FWDL_TAILER_FORMAT_T {
	HARVARD_TAILER_FORMAT,
	CONNAC_TAILER_FORMAT,
};

struct FWDL_OPS_T {
	const unsigned int tailer_format;	/* FW download tailer format */

	void (*constructFirmwarePrio)(P_GLUE_INFO_T prGlueInfo, PPUINT_8 apucNameTable, PPUINT_8 apucName,
				      PUINT_8 pucNameIdx, UINT_8 ucMaxNameIdx); /* load firmware bin priority */
	void (*constructPatchName)(P_GLUE_INFO_T prGlueInfo, PPUINT_8 apucName, PUINT_8 pucNameIdx);

	WLAN_STATUS (*downloadFirmware)(IN P_ADAPTER_T prAdapter, IN enum ENUM_IMG_DL_IDX_T eDlIdx);
	void (*getFwInfo)(IN P_ADAPTER_T prAdapter, IN UINT_8 u4SecIdx, IN enum ENUM_IMG_DL_IDX_T eDlIdx,
			  OUT PUINT_32 pu4Addr, OUT PUINT_32 pu4Len,
			  OUT PUINT_32 pu4DataMode, OUT PBOOLEAN pfgIsEMIDownload);
};

#if (CFG_UMAC_GENERATION >= 0x20)
#define LEN_4_BYTE_CRC	(4)

struct TAILER_COMMON_FORMAT_T {
	UINT_8 ucChipInfo;
	UINT_8 ucEcoCode;
	UINT_8 ucRegionNum;
	UINT_8 aucReserved[4];
	UINT_8 aucRamVersion[10];
	UINT_8 aucRamBuiltDate[15];
	UINT_32 u4CRC;
};

struct TAILER_REGION_FORMAT_T {
	UINT_32 u4CRC;
	UINT_32 u4RealSize;
	UINT_32 u4BlockSize;
	UINT_8 aucReserved1[4];
	UINT_32 u4Addr;
	UINT_32 u4Len;
	UINT_8 ucFeatureSet;
	UINT_8 aucReserved2[15];
};

struct TAILER_FORMAT_T {
	UINT_32 addr;
	UINT_8 chip_info;
	UINT_8 feature_set;
	UINT_8 eco_code;
	UINT_8 ram_version[10];
	UINT_8 ram_built_date[15];
	UINT_32 len;
};

#if CFG_SUPPORT_COMPRESSION_FW_OPTION
struct TAILER_FORMAT_T_2 {
	UINT_32 crc;
	UINT_32 addr;
	UINT_32 block_size;
	UINT_32 real_size;
	UINT_8  chip_info;
	UINT_8  feature_set;
	UINT_8  eco_code;
	UINT_8  ram_version[10];
	UINT_8  ram_built_date[15];
	UINT_32 len;
};

struct FW_IMAGE_TAILER_T_2 {
	struct TAILER_FORMAT_T_2 ilm_info;
	struct TAILER_FORMAT_T_2 dlm_info;
};

struct FW_IMAGE_TAILER_CHECK {
	UINT_8	chip_info;
	UINT_8	feature_set;
	UINT_8	eco_code;
	UINT_8	ram_version[10];
	UINT_8	ram_built_date[15];
	UINT_32 len;
};
#endif

struct PATCH_FORMAT_T {
	UINT_8 aucBuildDate[16];
	UINT_8 aucPlatform[4];
	UINT_32 u4SwHwVersion;
	UINT_32 u4PatchVersion;
	UINT_16 u2CRC;		/* CRC calculated for image only */
	UINT_8 ucPatchImage[0];
};

#endif

/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

#if CFG_ENABLE_FW_DOWNLOAD
UINT_32 wlanGetDataMode(IN P_ADAPTER_T prAdapter, IN enum ENUM_IMG_DL_IDX_T eDlIdx, IN UINT_8 ucFeatureSet);

VOID wlanGetHarvardFwInfo(IN P_ADAPTER_T prAdapter, IN UINT_8 u4SecIdx, IN enum ENUM_IMG_DL_IDX_T eDlIdx,
			 OUT PUINT_32 pu4Addr, OUT PUINT_32 pu4Len,
			 OUT PUINT_32 pu4DataMode, OUT PBOOLEAN pfgIsEMIDownload);

VOID wlanGetConnacFwInfo(IN P_ADAPTER_T prAdapter, IN UINT_8 u4SecIdx, IN enum ENUM_IMG_DL_IDX_T eDlIdx,
			 OUT PUINT_32 pu4Addr, OUT PUINT_32 pu4Len,
			 OUT PUINT_32 pu4DataMode, OUT PBOOLEAN pfgIsEMIDownload);

#if CFG_SUPPORT_COMPRESSION_FW_OPTION
WLAN_STATUS wlanCompressedImageSectionDownloadStage(IN P_ADAPTER_T prAdapter,
						    IN PVOID pvFwImageMapFile, IN UINT_32 u4FwImageFileLength,
						    UINT_8 ucSectionNumber, IN enum ENUM_IMG_DL_IDX_T eDlIdx,
						    OUT PUINT_8 ucIsCompressed,
						    OUT P_INIT_CMD_WIFI_DECOMPRESSION_START prFwImageInFo);
#endif
WLAN_STATUS wlanImageSectionDownloadStage(IN P_ADAPTER_T prAdapter, IN PVOID pvFwImageMapFile,
					  IN UINT_32 u4FwImageFileLength, IN UINT_8 ucSectionNumber,
					  IN enum ENUM_IMG_DL_IDX_T eDlIdx);

WLAN_STATUS wlanDownloadSection(IN P_ADAPTER_T prAdapter, IN UINT_32 u4Addr, IN UINT_32 u4Len,
				IN UINT_32 u4DataMode, IN PUINT_8 pucStartPtr, IN enum ENUM_IMG_DL_IDX_T eDlIdx);

WLAN_STATUS wlanDownloadEMISection(IN P_ADAPTER_T prAdapter, IN UINT_32 u4Len,
				   IN UINT_32 u4DestAddr, IN PUINT_8 pucStartPtr);

WLAN_STATUS wlanGetHarvardTailerInfo(IN P_ADAPTER_T prAdapter, IN PVOID prFwBuffer, IN UINT_32 u4FwSize,
				     IN UINT_32 ucTotSecNum, IN enum ENUM_IMG_DL_IDX_T eDlIdx);

WLAN_STATUS wlanGetConnacTailerInfo(IN P_ADAPTER_T prAdapter, IN PVOID prFwBuffer,
				    IN UINT_32 u4FwSize, IN enum ENUM_IMG_DL_IDX_T eDlIdx);

WLAN_STATUS wlanImageSectionConfig(IN P_ADAPTER_T prAdapter,
				   IN UINT_32 u4DestAddr, IN UINT_32 u4ImgSecSize, IN UINT_32 u4DataMode,
				   IN enum ENUM_IMG_DL_IDX_T eDlIdx);

WLAN_STATUS wlanImageSectionDownload(IN P_ADAPTER_T prAdapter, IN UINT_32 u4ImgSecSize, IN PUINT_8 pucImgSecBuf);

WLAN_STATUS wlanImageQueryStatus(IN P_ADAPTER_T prAdapter);

WLAN_STATUS wlanConfigWifiFuncStatus(IN P_ADAPTER_T prAdapter, IN UINT_8 ucCmdSeqNum);

WLAN_STATUS wlanConfigWifiFunc(IN P_ADAPTER_T prAdapter,
			       IN BOOLEAN fgEnable, IN UINT_32 u4StartAddress, IN UINT_8 ucPDA);

UINT_32 wlanCRC32(PUINT_8 buf, UINT_32 len);

WLAN_STATUS wlanDownloadCR4FW(IN P_ADAPTER_T prAdapter, PVOID prFwBuffer);

WLAN_STATUS wlanDownloadFW(IN P_ADAPTER_T prAdapter);

WLAN_STATUS wlanDownloadPatch(IN P_ADAPTER_T prAdapter);

WLAN_STATUS wlanHarvardFormatDownload(IN P_ADAPTER_T prAdapter, IN enum ENUM_IMG_DL_IDX_T eDlIdx);

WLAN_STATUS wlanConnacFormatDownload(IN P_ADAPTER_T prAdapter, IN enum ENUM_IMG_DL_IDX_T eDlIdx);

WLAN_STATUS wlanGetPatchInfo(IN P_ADAPTER_T prAdapter);

VOID wlanPrintFwdlInfo(IN P_ADAPTER_T prAdapter);

#endif

#endif /* _FW_DL_H */

