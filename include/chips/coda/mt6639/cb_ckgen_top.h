/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2020 MediaTek Inc.
 */

#ifndef __CB_CKGEN_TOP_REGS_H__
#define __CB_CKGEN_TOP_REGS_H__

#include "hal_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CB_CKGEN_TOP_BASE \
	0x70000000

#define CB_CKGEN_TOP_CBTOP_ULPOSC_1_ADDR \
	(CB_CKGEN_TOP_BASE + 0x244)
#define CB_CKGEN_TOP_CBTOP_ULPOSC_2_ADDR \
	(CB_CKGEN_TOP_BASE + 0x248)


#define CB_CKGEN_TOP_CBTOP_ULPOSC_1_rg_mux_sel_ADDR \
	CB_CKGEN_TOP_CBTOP_ULPOSC_1_ADDR
#define CB_CKGEN_TOP_CBTOP_ULPOSC_1_rg_mux_sel_MASK \
	0xF0000000
#define CB_CKGEN_TOP_CBTOP_ULPOSC_1_rg_mux_sel_SHFT \
	28
#define CB_CKGEN_TOP_CBTOP_ULPOSC_1_rg_ckgen_tri_cal_ADDR \
	CB_CKGEN_TOP_CBTOP_ULPOSC_1_ADDR
#define CB_CKGEN_TOP_CBTOP_ULPOSC_1_rg_ckgen_tri_cal_MASK \
	0x01000000
#define CB_CKGEN_TOP_CBTOP_ULPOSC_1_rg_ckgen_tri_cal_SHFT \
	24
#define CB_CKGEN_TOP_CBTOP_ULPOSC_1_rg_abist_meter_sw_en_ADDR \
	CB_CKGEN_TOP_CBTOP_ULPOSC_1_ADDR
#define CB_CKGEN_TOP_CBTOP_ULPOSC_1_rg_abist_meter_sw_en_MASK \
	0x00100000
#define CB_CKGEN_TOP_CBTOP_ULPOSC_1_rg_abist_meter_sw_en_SHFT \
	20
#define CB_CKGEN_TOP_CBTOP_ULPOSC_1_rg_freq_offset_value_ADDR \
	CB_CKGEN_TOP_CBTOP_ULPOSC_1_ADDR
#define CB_CKGEN_TOP_CBTOP_ULPOSC_1_rg_freq_offset_value_MASK \
	0x000F0000
#define CB_CKGEN_TOP_CBTOP_ULPOSC_1_rg_freq_offset_value_SHFT \
	16

#define CB_CKGEN_TOP_CBTOP_ULPOSC_2_ulposc_ready_ADDR \
	CB_CKGEN_TOP_CBTOP_ULPOSC_2_ADDR
#define CB_CKGEN_TOP_CBTOP_ULPOSC_2_ulposc_ready_MASK \
	0x80000000
#define CB_CKGEN_TOP_CBTOP_ULPOSC_2_ulposc_ready_SHFT \
	31
#define CB_CKGEN_TOP_CBTOP_ULPOSC_2_kdone_OSC_CALI_ADDR \
	CB_CKGEN_TOP_CBTOP_ULPOSC_2_ADDR
#define CB_CKGEN_TOP_CBTOP_ULPOSC_2_kdone_OSC_CALI_MASK \
	0x7F000000
#define CB_CKGEN_TOP_CBTOP_ULPOSC_2_kdone_OSC_CALI_SHFT \
	24
#define CB_CKGEN_TOP_CBTOP_ULPOSC_2_ckgen_cal_valid_ADDR \
	CB_CKGEN_TOP_CBTOP_ULPOSC_2_ADDR
#define CB_CKGEN_TOP_CBTOP_ULPOSC_2_ckgen_cal_valid_MASK \
	0x00100000
#define CB_CKGEN_TOP_CBTOP_ULPOSC_2_ckgen_cal_valid_SHFT \
	20
#define CB_CKGEN_TOP_CBTOP_ULPOSC_2_ckgen_cal_ok_ADDR \
	CB_CKGEN_TOP_CBTOP_ULPOSC_2_ADDR
#define CB_CKGEN_TOP_CBTOP_ULPOSC_2_ckgen_cal_ok_MASK \
	0x00010000
#define CB_CKGEN_TOP_CBTOP_ULPOSC_2_ckgen_cal_ok_SHFT \
	16
#define CB_CKGEN_TOP_CBTOP_ULPOSC_2_ckgen_cal_cnt_ADDR \
	CB_CKGEN_TOP_CBTOP_ULPOSC_2_ADDR
#define CB_CKGEN_TOP_CBTOP_ULPOSC_2_ckgen_cal_cnt_MASK \
	0x0000FFFF
#define CB_CKGEN_TOP_CBTOP_ULPOSC_2_ckgen_cal_cnt_SHFT \
	0

#ifdef __cplusplus
}
#endif

#endif
