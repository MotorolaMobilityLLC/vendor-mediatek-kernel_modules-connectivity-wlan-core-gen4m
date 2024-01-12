/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2020 MediaTek Inc.
 */

/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */

/*******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 *******************************************************************************
 */
#include "connsys_debug_utility.h"
#include "gl_coredump.h"
#include "precomp.h"

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */

/*******************************************************************************
 *                            P U B L I C   D A T A
 *******************************************************************************
 */

/*******************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */
static int fw_log_reg_readable(void);

/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */
struct coredump_ctx g_ctx;

struct coredump_event_cb g_wifi_coredump_cb = {
	.reg_readable = fw_log_reg_readable,
	.poll_cpupcr = NULL,
};

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */

void fw_log_bug_hang_register(bushang_chk_func_cb func)
{
	g_ctx.fn_check_bus_hang = (bushang_chk_func_cb)func;
}

int fw_log_reg_readable(void)
{
	int ret = 1;

	if (conninfra_reg_readable() == 0) {
		DBGLOG(INIT, ERROR,
			"conninfra_reg_readable check failed.\n");
		ret = 0;
	}

	if (g_ctx.fn_check_bus_hang) {
		if (g_ctx.fn_check_bus_hang(NULL, 0) != 0) {
			DBGLOG(INIT, ERROR, "bus check failed.\n");
			ret = 0;
		}
	}

	return ret;
}

void fw_log_connsys_coredump_init(void)
{
	kalMemZero(&g_ctx, sizeof(g_ctx));

	g_ctx.handler = connsys_coredump_init(CONN_DEBUG_TYPE_WIFI,
		&g_wifi_coredump_cb);
	if (!g_ctx.handler)
		DBGLOG(INIT, ERROR, "connsys_coredump_init init failed.\n");
}

void fw_log_connsys_coredump_deinit(void)
{
	connsys_coredump_deinit(g_ctx.handler);
}

void fw_log_connsys_coredump_start(unsigned int drv, char *reason)
{
	connsys_coredump_start(g_ctx.handler, 0,
		(enum consys_drv_type)drv, reason);
	connsys_coredump_clean(g_ctx.handler);
}

