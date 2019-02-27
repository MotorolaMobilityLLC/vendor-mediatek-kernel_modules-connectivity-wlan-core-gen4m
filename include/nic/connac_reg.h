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
** Id:
*/

/*! \file   "connac_reg.h"
 *   \brief  The common register definition of MT6630
 *
 *   N/A
*/



#ifndef _CONNAC_REG_H
#define _CONNAC_REG_H

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/


/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/
#define CONN_CFG_BASE                                          0x80020000

#define CONN_CFG_ON_BASE                                       0x81021000

#define CONN_CFG_ON_CONN_ON_MISC_ADDR                          (CONN_CFG_ON_BASE + 0x140)

/*
* =====================================================================================
*
*  ---CONN_ON_MISC (0x81021000 + 0x140)---
*
*    HOST_LPCR_FW_OWN[0]          - (W1C)  xxx
*    DRV_FM_STAT_SYNC[3..1]       - (RW)  xxx
*    RBIST_MODE[4]                - (RW)  xxx
*    RESERVED5[31..5]             - (RO) Reserved bits
*
* =====================================================================================
*/
#define CONN_CFG_ON_CONN_ON_MISC_RBIST_MODE_ADDR               CONN_CFG_ON_CONN_ON_MISC_ADDR
#define CONN_CFG_ON_CONN_ON_MISC_RBIST_MODE_MASK               0x00000010/*RBIST_MODE[4]*/
#define CONN_CFG_ON_CONN_ON_MISC_RBIST_MODE_SHFT               4
#define CONN_CFG_ON_CONN_ON_MISC_DRV_FM_STAT_SYNC_ADDR         CONN_CFG_ON_CONN_ON_MISC_ADDR
#define CONN_CFG_ON_CONN_ON_MISC_DRV_FM_STAT_SYNC_MASK         0x0000000E/*DRV_FM_STAT_SYNC[3..1]*/
#define CONN_CFG_ON_CONN_ON_MISC_DRV_FM_STAT_SYNC_SHFT         1
#define CONN_CFG_ON_CONN_ON_MISC_HOST_LPCR_FW_OWN_ADDR         CONN_CFG_ON_CONN_ON_MISC_ADDR
#define CONN_CFG_ON_CONN_ON_MISC_HOST_LPCR_FW_OWN_MASK         0x00000001/*HOST_LPCR_FW_OWN[0]*/
#define CONN_CFG_ON_CONN_ON_MISC_HOST_LPCR_FW_OWN_SHFT         0

#define CONN_HIF_BASE                           0x7000
#define CONN_HIF_ON_LPCTL                       (CONN_HIF_BASE)
#define CONN_HIF_ON_IRQ_STAT                    (CONN_HIF_BASE + 0x4)
#define CONN_HIF_ON_IRQ_ENA                     (CONN_HIF_BASE + 0x8)

#if defined(_HIF_USB)
#define CONNAC_UDMA_BASE                         0x7C000000
#define CONNAC_UDMA_TX_QSEL                      (CONNAC_UDMA_BASE + 0x8)
#define CONNAC_UDMA_RESET                        (CONNAC_UDMA_BASE + 0x14)
#define CONNAC_UDMA_WLCFG_1                      (CONNAC_UDMA_BASE + 0xc)
#define CONNAC_UDMA_WLCFG_0                      (CONNAC_UDMA_BASE + 0x18)

#define UDMA_WLCFG_0_1US_TIMER_EN_MASK          (0x1 << 20)
#define UDMA_WLCFG_0_1US_TIMER_EN(p)            (((p) & 0x1) << 20)
#endif /* _HIF_USB */

#define PLE_PKT_MAX_SIZE_MASK (0xfff << 0)
#define PLE_PKT_MAX_SIZE_NUM(p) (((p) & 0xfff) << 0)
#define GET_PLE_PKT_MAX_SIZE_NUM(p) (((p) & PLE_PKT_MAX_SIZE_MASK) >> 0)

#define PSE_PKT_MAX_SIZE_MASK (0xfff << 16)
#define PSE_PKT_MAX_SIZE_NUM(p) (((p) & 0xfff) << 16)
#define GET_PSE_PKT_MAX_SIZE_NUM(p) (((p) & PSE_PKT_MAX_SIZE_MASK) >> 16)

#define EXTRA_TXD_SIZE_FOR_TX_BYTE_COUNT         32

/*******************************************************************************
*                             D A T A   T Y P E S
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
*                                 M A C R O S
********************************************************************************
*/

/*******************************************************************************
*                   F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/
#endif
