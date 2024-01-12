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

#ifndef _IDX_LOG_H
#define _IDX_LOG_H

struct FW_LOG_IDX_DATA {
	/* FW Log Info Bin Data */
	void *prFwBuffer;
	uint32_t u4FwSize;
};

/* for little-endian CPU */
struct IDX_LOG_V2_FORMAT {
	uint8_t ucStx;
	uint8_t ucVerType : 3;
	uint8_t ucNumArgs : 5;
	uint8_t ucModId;
	uint8_t ucLevelId : 4;
	uint8_t ucSeqNo : 4;
	uint32_t u4TimeStamp;
	uint32_t u4IdxId;
	/* following is args 0~31, each is 4 bytes */
};

/* for little-endian CPU */
struct CTRL_TIME_SYNC_FORMAT {
	uint8_t ucStx;
	uint8_t ucVerType : 3;
	uint8_t ucCtrlType : 5;
	uint16_t u2Rsv;
	uint32_t u4TimeStamp;
	uint32_t u4UtcSec;
	uint32_t u4UtcUsec;
};

/* for little-endian CPU */
struct CTRL_LOG_LOST_FORMAT {
	uint8_t ucStx;
	uint8_t ucVerType : 3;
	uint8_t ucCtrlType : 5;
	uint16_t u2Rsv;
	uint32_t u4LogLostCount;
};

/* for little-endian CPU */
struct TEXT_LOG_FORMAT {
	uint8_t ucStx;
	uint8_t ucVerType : 3;
	uint8_t ucRsv0 : 5;
	uint8_t ucPayloadSize_wo_padding;
	uint8_t ucPayloadSize_w_padding;
	uint32_t u4TimeStamp;
	/* following is payload (text log) */
};

uint32_t wlanStoreIdxLogBin(struct ADAPTER *prAdapter, uint8_t *prSrc,
			    uint32_t u4FwSize);
uint32_t wlanOpenIdxLogBin(struct ADAPTER *prAdapter);
void wlanCloseIdxLogBin(struct ADAPTER *prAdapter);
uint32_t wlanFwLogIdxToStr(struct ADAPTER *prAdapter, uint8_t *pucIdxLog,
			   uint16_t u2MsgSize);

#endif /* _IDX_LOG_H */
