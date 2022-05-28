/*******************************************************************************
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
 * Id: //Department/DaVinci/BRANCHES/MT6620_WIFI_DRIVER_V2_3/os/linux/include
 *     /gl_rst.h#1
 */

/*! \file   gl_rst.h
 *    \brief  Declaration of functions and finite state machine for
 *	    MT6620 Whole-Chip Reset Mechanism
 */

#ifndef _GL_RST_H
#define _GL_RST_H

/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */

/*******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 *******************************************************************************
 */
#include "gl_typedef.h"

#if CFG_MTK_ANDROID_WMT && IS_ENABLED(CFG_SUPPORT_CONNAC1X)
#include "wmt_exp.h"
#endif

#if 0
#include "mtk_porting.h"
#endif
/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */
#if (MTK_WCN_HIF_SDIO == 1) || (MTK_WCN_HIF_AXI == 1)
#define CFG_WMT_RESET_API_SUPPORT   1
#else
#define CFG_WMT_RESET_API_SUPPORT   0
#endif

#define RST_FLAG_CHIP_RESET        0
#define RST_FLAG_DO_CORE_DUMP              BIT(0)
#define RST_FLAG_PREVENT_POWER_OFF         BIT(1)
#define RST_FLAG_DO_WHOLE_RESET            BIT(2)
#define RST_FLAG_DO_L0P5_RESET             BIT(3)
#define RST_FLAG_DO_L1_RESET               BIT(4)

#if CFG_CHIP_RESET_HANG
#define SER_L0_HANG_RST_NONE		0
#define SER_L0_HANG_RST_TRGING		1
#define SER_L0_HANG_RST_HAND_DISABLE	2
#define SER_L0_HANG_RST_HANG		3
#define SER_L0_HANG_RST_CMD_TRG		9

#define SER_L0_HANG_LOG_TIME_INTERVAL	3000
#endif
#if (CFG_SUPPORT_CONNINFRA == 1)
#include "conninfra.h"
#define WIFI_TRIGGER_ASSERT_TIMEOUT 2000
#define GLUE_FLAG_RST_PROCESS (GLUE_FLAG_HALT |\
				GLUE_FLAG_RST_START |\
				GLUE_FLAG_RST_END)
#define RST_FLAG_WHOLE_RESET  (RST_FLAG_DO_CORE_DUMP | \
			       RST_FLAG_PREVENT_POWER_OFF |\
			       RST_FLAG_DO_WHOLE_RESET)
#define RST_FLAG_WF_RESET  (RST_FLAG_DO_CORE_DUMP | RST_FLAG_PREVENT_POWER_OFF)
#endif
/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */
enum ENUM_RESET_STATUS {
	RESET_FAIL,
	RESET_SUCCESS
};

enum _ENUM_CHIP_RESET_REASON_TYPE_T {
	RST_PROCESS_ABNORMAL_INT = 1,
	RST_DRV_OWN_FAIL,
	RST_FW_ASSERT,
	RST_FW_ASSERT_TIMEOUT,
	RST_BT_TRIGGER,
	RST_OID_TIMEOUT,
	RST_CMD_TRIGGER,
	RST_REQ_CHL_FAIL,
	RST_FW_DL_FAIL,
	RST_SER_TIMEOUT,
	RST_SLP_PROT_TIMEOUT,
	RST_REG_READ_DEADFEED,
	RST_P2P_CHNL_GRANT_INVALID_TYPE,
	RST_P2P_CHNL_GRANT_INVALID_STATE,
	RST_SCAN_RECOVERY,
	RST_ACCESS_REG_FAIL,
	RST_WIFI_ON_DRV_OWN_FAIL,
	RST_CHECK_READY_BIT_TIMEOUT,
	RST_ALLOC_CMD_FAIL,
	RST_SDIO_RX_ERROR,
	RST_WHOLE_CHIP_TRIGGER,
	RST_MDDP_MD_TRIGGER_EXCEPTION,
	RST_FWK_TRIGGER,
	RST_SER_L1_FAIL,
	RST_SER_L0P5_FAIL,
	RST_CMD_EVT_FAIL,
	RST_WDT,
	RST_SMC_CMD_FAIL,
	RST_REASON_MAX
};

/* L0.5 reset state */
enum ENUM_WFSYS_RESET_STATE_TYPE_T {
	WFSYS_RESET_STATE_IDLE = 0,
	WFSYS_RESET_STATE_DETECT,
	WFSYS_RESET_STATE_RESET,
	WFSYS_RESET_STATE_REINIT,
	WFSYS_RESET_STATE_POSTPONE,
	WFSYS_RESET_STATE_MAX
};

struct RESET_STRUCT {
	struct GLUE_INFO *prGlueInfo;
	struct work_struct rst_work;
#if CFG_WMT_RESET_API_SUPPORT
	enum ENUM_RESET_STATUS rst_data;
	struct work_struct rst_trigger_work;
	uint32_t rst_trigger_flag;
#endif
};

#if CFG_CHIP_RESET_SUPPORT
#if (CFG_SUPPORT_CONNINFRA == 1)
/* duplicated from wmt_exp.h for better driver isolation */
enum ENUM_WMTDRV_TYPE {
	WMTDRV_TYPE_BT = 0,
	WMTDRV_TYPE_FM = 1,
	WMTDRV_TYPE_GPS = 2,
	WMTDRV_TYPE_WIFI = 3,
	WMTDRV_TYPE_WMT = 4,
	WMTDRV_TYPE_STP = 5,
	WMTDRV_TYPE_SDIO1 = 6,
	WMTDRV_TYPE_SDIO2 = 7,
	WMTDRV_TYPE_LPBK = 8,
	WMTDRV_TYPE_MAX
};

enum ENUM_WMTMSG_TYPE {
	WMTMSG_TYPE_POWER_ON = 0,
	WMTMSG_TYPE_POWER_OFF = 1,
	WMTMSG_TYPE_RESET = 2,
	WMTMSG_TYPE_STP_RDY = 3,
	WMTMSG_TYPE_HW_FUNC_ON = 4,
	WMTMSG_TYPE_MAX
};

enum ENUM_WMTRSTMSG_TYPE {
	WMTRSTMSG_RESET_START = 0x0,  /*whole chip reset (include other radio)*/
	WMTRSTMSG_RESET_END = 0x1,
	WMTRSTMSG_RESET_END_FAIL = 0x2,
	WMTRSTMSG_0P5RESET_START = 0x3, /*wfsys reset ( wifi only )*/
	WMTRSTMSG_RESET_MAX,
	WMTRSTMSG_RESET_INVALID = 0xff
};

enum ENUM_WF_RST_SOURCE {
	WF_RST_SOURCE_NONE = 0x0,
	WF_RST_SOURCE_DRIVER = 0x1,
	WF_RST_SOURCE_FW = 0x2,
	WF_RST_SOURCE_MAX
};
#endif
#endif

/*******************************************************************************
 *                    E X T E R N A L   F U N C T I O N S
 *******************************************************************************
 */
#if CFG_CHIP_RESET_SUPPORT

#if CFG_WMT_RESET_API_SUPPORT
extern int wifi_reset_start(void);
extern int wifi_reset_end(enum ENUM_RESET_STATUS);
#endif /* CFG_WMT_RESET_API_SUPPORT */

#if (CFG_SUPPORT_CONNINFRA == 1)
extern int hifAxiRemove(void);
extern void kalSetRstEvent(void);
extern void update_driver_reset_status(uint8_t fgIsResetting);
extern int32_t get_wifi_process_status(void);
extern int32_t get_wifi_powered_status(void);
#endif /* CFG_SUPPORT_CONNINFRA */

#endif /* CFG_CHIP_RESET_SUPPORT */

/*******************************************************************************
 *                            P U B L I C   D A T A
 *******************************************************************************
 */
#if CFG_CHIP_RESET_SUPPORT
extern u_int8_t fgIsResetting;
extern u_int8_t fgIsRstPreventFwOwn;
#if (CFG_SUPPORT_CONNINFRA == 1)
extern enum ENUM_WF_RST_SOURCE g_eWfRstSource;
#endif

#if CFG_CHIP_RESET_HANG
extern u_int8_t fgIsResetHangState;
#endif

#endif
/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */
/*******************************************************************************
 *                                 M A C R O S
 *******************************************************************************
 */
#if CFG_CHIP_RESET_SUPPORT
/* Each reset trigger reason has corresponding default reset action, which is
 * defined in glResetSelectAction(). You can use this macro to trigger default
 * action.
 */
#define GL_DEFAULT_RESET_TRIGGER(_prAdapter, _eReason)		\
do { \
	glSetRstReason(_eReason);    \
	glResetTrigger(_prAdapter, glResetSelectAction(_prAdapter),	\
		       (const uint8_t *)__FILE__, __LINE__);    \
} while (FALSE)

/* You can use this macro to trigger user defined reset actions instead of the
 * default ones.
 */
#define GL_USER_DEFINE_RESET_TRIGGER(_prAdapter, _eReason, _u4Flags)    \
do { \
	glSetRstReason(_eReason);    \
	glResetTrigger(_prAdapter, _u4Flags,	\
		       (const uint8_t *)__FILE__, __LINE__);    \
} while (FALSE)
#else
#define GL_DEFAULT_RESET_TRIGGER(_prAdapter, _eReason) \
	DBGLOG(INIT, INFO, "DO NOT support chip reset\n")

#define GL_USER_DEFINE_RESET_TRIGGER(_prAdapter, _eReason, _u4Flags) \
	DBGLOG(INIT, INFO, "DO NOT support chip reset\n")
#endif

/*******************************************************************************
 *                  F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */
#if CFG_CHIP_RESET_SUPPORT
extern uint64_t u8ResetTime;
extern u_int8_t fgSimplifyResetFlow;
extern char *g_reason;
#endif
/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */
void glSetRstReason(enum _ENUM_CHIP_RESET_REASON_TYPE_T eReason);
int glGetRstReason(void);

u_int8_t kalIsResetting(void);
u_int8_t kalIsRstPreventFwOwn(void);

#if CFG_CHIP_RESET_SUPPORT
void glResetInit(struct GLUE_INFO *prGlueInfo);

void glResetUninit(void);

void glSendResetRequest(void);

void glResetWholeChipResetTrigger(char *pcReason);

uint32_t glResetSelectAction(IN struct ADAPTER *prAdapter);

void glResetTrigger(struct ADAPTER *prAdapter,
		    uint32_t u4RstFlag, const uint8_t *pucFile,
		    uint32_t u4Line);

#if CFG_WMT_RESET_API_SUPPORT
int32_t glIsWmtCodeDump(void);
#if (CFG_SUPPORT_CONNINFRA == 1)
int wlan_reset_thread_main(void *data);
int glRstwlanPreWholeChipReset(enum consys_drv_type type, char *reason);
int glRstwlanPostWholeChipReset(void);
u_int8_t kalIsWholeChipResetting(void);
void glSetRstReasonString(char *reason);
#endif /* CFG_SUPPORT_CONNINFRA */
#else
void glSetWfsysResetState(struct ADAPTER *prAdapter,
			  enum ENUM_WFSYS_RESET_STATE_TYPE_T state);

u_int8_t glReSchWfsysReset(struct ADAPTER *prAdapter);

void WfsysResetHdlr(struct work_struct *work);
#endif /* CFG_WMT_RESET_API_SUPPORT */
#endif /* CFG_CHIP_RESET_SUPPORT */
#endif /* _GL_RST_H */
