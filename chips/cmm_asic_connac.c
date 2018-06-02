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
/*! \file   connac.c
*    \brief  Internal driver stack will export the required procedures here for GLUE Layer.
*
*    This file contains all routines which are exported from MediaTek 802.11 Wireless
*    LAN driver stack to GLUE Layer.
*/

#ifdef CONNAC_MAC

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
VOID asicEnableFWDownload(IN P_ADAPTER_T prAdapter, IN BOOL fgEnable)
{
	P_GLUE_INFO_T prGlueInfo;

	ASSERT(prAdapter);

	prGlueInfo = prAdapter->prGlueInfo;

	switch (prGlueInfo->u4InfType) {
#if defined(_HIF_PCIE)
	case MT_DEV_INF_PCIE:
	{
		WPDMA_GLO_CFG_STRUCT GloCfg;

		kalDevRegRead(prGlueInfo, WPDMA_GLO_CFG, &GloCfg.word);

		GloCfg.field_conn.bypass_dmashdl_txring3 = fgEnable;

		kalDevRegWrite(prGlueInfo, WPDMA_GLO_CFG, GloCfg.word);
	}
	break;
#endif /* _HIF_PCIE */

#if defined(_HIF_USB)
	case MT_DEV_INF_USB:
	{
		UINT_32 u4Value = 0;

		ASSERT(prAdapter);

		HAL_MCR_RD(prAdapter, CONNAC_UDMA_TX_QSEL, &u4Value);

		if (fgEnable)
			u4Value |= FW_DL_EN;
		else
			u4Value &= ~FW_DL_EN;

		HAL_MCR_WR(prAdapter, CONNAC_UDMA_TX_QSEL, u4Value);
	}
	break;
#endif /* _HIF_USB */

	default:
		break;
	}
}

VOID asicDevInit(IN P_ADAPTER_T prAdapter)
{
	P_GLUE_INFO_T prGlueInfo;

	ASSERT(prAdapter);

	prGlueInfo = prAdapter->prGlueInfo;

	switch (prGlueInfo->u4InfType) {
#if defined(_HIF_USB)
	case MT_DEV_INF_USB:
	{
		UINT_32 u4Value = 0;

		ASSERT(prAdapter);

		HAL_MCR_RD(prAdapter, CONNAC_UDMA_WLCFG_0, &u4Value);

		/* enable UDMA TX & RX */
		u4Value = UDMA_WLCFG_0_TX_EN(1) | UDMA_WLCFG_0_RX_EN(1) |
		    UDMA_WLCFG_0_RX_AGG_EN(1) |
		    UDMA_WLCFG_0_RX_MPSZ_PAD0(1) |
		    UDMA_WLCFG_0_RX_AGG_LMT(USB_RX_AGGREGTAION_LIMIT) |
		    UDMA_WLCFG_0_RX_AGG_TO(USB_RX_AGGREGTAION_TIMEOUT);

		HAL_MCR_WR(prAdapter, CONNAC_UDMA_WLCFG_0, u4Value);

		HAL_MCR_RD(prAdapter, CONNAC_UDMA_WLCFG_1, &u4Value);

		u4Value &= ~UDMA_WLCFG_1_RX_AGG_PKT_LMT_MASK;
		u4Value |= UDMA_WLCFG_1_RX_AGG_PKT_LMT(USB_RX_AGGREGTAION_PKT_LIMIT);

		HAL_MCR_WR(prAdapter, CONNAC_UDMA_WLCFG_1, u4Value);
	}
	break;
#endif /* _HIF_USB */

	default:
		break;
	}
}
#endif /* CONNAC_MAC */
