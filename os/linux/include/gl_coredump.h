/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2020 MediaTek Inc.
 */

#ifndef _GL_COREDUMP_H
#define _GL_COREDUMP_H

typedef int (*bushang_chk_func_cb)(void *, uint8_t);

struct coredump_ctx {
	struct connsys_dump_ctx *handler;
	bushang_chk_func_cb fn_check_bus_hang;
};

void fw_log_connsys_coredump_init(void);
void fw_log_connsys_coredump_deinit(void);
void fw_log_connsys_coredump_start(unsigned int, char *);
void fw_log_bug_hang_register(bushang_chk_func_cb func);

#endif /* _GL_COREDUMP_H */
