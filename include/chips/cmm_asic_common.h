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
/*! \file  cmm_connac_common.h
 *    \brief This file contains the info of CONNAC
 */

#ifndef _CMM_ASIC_COMMON_H
#define _CMM_ASIC_COMMON_H

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */

/*******************************************************************************
 *                  F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */
uint32_t asicGetFwDlInfo(struct ADAPTER *prAdapter,
	char *pcBuf, int i4TotalLen);
uint32_t asicGetChipID(struct ADAPTER *prAdapter);
void fillNicTxDescAppend(IN struct ADAPTER *prAdapter,
	IN struct MSDU_INFO *prMsduInfo,
	OUT uint8_t *prTxDescBuffer);
void fillTxDescAppendByHostV2(IN struct ADAPTER *prAdapter,
	IN struct MSDU_INFO *prMsduInfo, IN uint16_t u4MsduId,
	IN dma_addr_t rDmaAddr, IN uint32_t u4Idx, IN u_int8_t fgIsLast,
	OUT uint8_t *pucBuffer);
void halDumpTxdInfo(IN struct ADAPTER *prAdapter, uint8_t *tmac_info);
void asicWakeUpWiFi(IN struct ADAPTER *prAdapter);

int connsys_power_on(void);
int connsys_power_done(void);
void connsys_power_off(void);

#if CFG_MTK_ANDROID_WMT
#if !IS_ENABLED(CFG_SUPPORT_CONNAC1X)
void unregister_chrdev_cbs(void);
void register_chrdev_cbs(void);
#endif
void unregister_plat_connsys_cbs(void);
void register_plat_connsys_cbs(void);
#endif

#endif /* _CMM_ASIC_COMMON_H */

