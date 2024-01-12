
/*****************************************************************************
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
 ***************************************************************************/
/*! \file   gl_dependency.h
 * \brief  List the os-dependent structure/API that need to implement
 * to align with common part
 *
 * not to modify *.c while put current used linux-type strucutre/API here
 * For porting to new OS, the API listed in this file needs to be
 * implemented
 */

#ifndef _GL_DEPENDENT_H
#define _GL_DEPENDENT_H

#ifndef IS_ENABLED
#define IS_ENABLED(_val) defined(_val)
#endif
/*
 * TODO: implement defined structure to let
 * other OS aligned to linux style for common part logic workaround
 */
/*
 * TODO: os-related?, CHN_DIRTY_WEIGHT_UPPERBOUND
 * should we remove gl_*_ioctl.h in os/none?
 * defined in os/linux/include/gl_p2p_ioctl.h
 * used in
 * 1) os/linux/gl_p2p_cfg80211.c
 * 2) mgmt/cnm.c
 * > is it related to ioctl?, or should it put in wlan_p2p.h
 */
#if 0
#define CHN_DIRTY_WEIGHT_UPPERBOUND     4
#endif

/* some arch's have a small-data section that can be accessed register-relative
 * but that can only take up to, say, 4-byte variables. jiffies being part of
 * an 8-byte variable may not be correctly accessed unless we force the issue
 * #define __jiffy_data  __attribute__((section(".data")))
 *
 * The 64-bit value is not atomic - you MUST NOT read it
 * without sampling the sequence number in jiffies_lock.

 * extern u64 __jiffy_data jiffies_64;
 * extern unsigned long volatile __jiffy_data jiffies;
 * TODO: no idea how to implement jiffies here
 */
#ifndef HZ
#define HZ (1000)
#endif
/*
 * comment: cipher type should not related to os
 * defined in os/linux/gl_wext.h,
 * while it looks like also defined in linux/wireless.h
 * are we trying to be an self-fulfilled driver then
 * should not put it under os folder? or we should just include from linux
 */
/* IW_AUTH_PAIRWISE_CIPHER and IW_AUTH_GROUP_CIPHER values (bit field) */
#define IW_AUTH_CIPHER_NONE     0x00000001
#define IW_AUTH_CIPHER_WEP40    0x00000002
#define IW_AUTH_CIPHER_TKIP     0x00000004
#define IW_AUTH_CIPHER_CCMP     0x00000008
#define IW_AUTH_CIPHER_WEP104   0x00000010

/*
 * comment:
 * 1) access GlueInfo member from core logic
 * 2) IW_AUTH_WPA_VERSION_DISABLED is defined in os/linux/include/gl_wext.h
 * > is the an os-dependent value/ protocol value.
 * should implement depends on OS/ its WiFi
 * needed by,
 * mgmt/ais_fsm.c
 * mgmt/assoc.c
 */
/* IW_AUTH_WPA_VERSION values (bit field) */
#define IW_AUTH_WPA_VERSION_DISABLED    0x00000001
#define IW_AUTH_WPA_VERSION_WPA         0x00000002
#define IW_AUTH_WPA_VERSION_WPA2        0x00000004

#define IW_AUTH_ALG_FT			0x00000008

#define IW_PMKID_LEN        16

#define SUITE(oui, id)	(((oui) << 8) | (id))
#define WLAN_AKM_SUITE_SAE			SUITE(0x000FAC, 8)
#define WLAN_EID_BSS_MAX_IDLE_PERIOD 90
#define WLAN_EID_VENDOR_SPECIFIC 221

/*
 * this highly depends on kernel version
 * why can't we just use kalGetTimeTick (?)
 * needed by
 * wmm.c
 */
#if 0
struct timespec {
	__kernel_time_t	tv_sec;			/* seconds */
	long tv_nsec;		/* nanoseconds */
};
#endif
/*
 * needed by cmm_asic_connac.c
 *	Potential risk in this function
 *	#ifdef CONFIG_PHYS_ADDR_T_64BIT
 *	typedef u64 phys_addr_t;
 *	#else
 *	typedef u32 phys_addr_t;
 *	#endif
 *	while anyway the rDmaAddr is transfer to u8Addr still u64
 *	, and filled into u4Ptr0 which is uint32_t (?)
 */
#define phys_addr_t uint32_t

/*
 * needed by nic_tx.h
 * defined in linux/types.h
 * #ifdef CONFIG_ARCH_DMA_ADDR_T_64BIT
 * typedef u64 dma_addr_t;
 * #else
 * typedef u32 dma_addr_t;
 * #endif
 */
#define dma_addr_t uint32_t

/* needed by:
 * common/debug.c
 * source/include/linux/printk.h
 */
enum {
	DUMP_PREFIX_NONE,
	DUMP_PREFIX_ADDRESS,
	DUMP_PREFIX_OFFSET
};

/* needed by
 * common/wlan_lib.c
 * include/mgmt/rsn.h
 */
#ifndef LINUX_VERSION_CODE
#define LINUX_VERSION_CODE 199947
#endif
#define CFG80211_VERSION_CODE LINUX_VERSION_CODE
#define KERNEL_VERSION(a, b, c) (((a) << 16) + ((b) << 8) + (c))
/*
 * needed by
 * include/mgmt/rlm_domain.h
 * mgmt/p2p_func.c
 *
 * enum nl80211_dfs_regions - regulatory DFS regions
 *
 * @NL80211_DFS_UNSET: Country has no DFS master region specified
 * @NL80211_DFS_FCC: Country follows DFS master rules from FCC
 * @NL80211_DFS_ETSI: Country follows DFS master rules from ETSI
 * @NL80211_DFS_JP: Country follows DFS master rules from JP/MKK/Telec
 */
enum nl80211_dfs_regions {
	NL80211_DFS_UNSET	= 0,
	NL80211_DFS_FCC		= 1,
	NL80211_DFS_ETSI	= 2,
	NL80211_DFS_JP		= 3,
};

/*
 * needed by mgmt/rlm_domain.c
 * enum ieee80211_channel_flags - channel flags
 *
 * Channel flags set by the regulatory control code.
 *
 * @IEEE80211_CHAN_DISABLED: This channel is disabled.
 * @IEEE80211_CHAN_NO_IR: do not initiate radiation, this includes
 *      sending probe requests or beaconing.
 * @IEEE80211_CHAN_RADAR: Radar detection is required on this channel.
 * @IEEE80211_CHAN_NO_HT40PLUS: extension channel above this channel
 *      is not permitted.
 * @IEEE80211_CHAN_NO_HT40MINUS: extension channel below this channel
 *      is not permitted.
 * @IEEE80211_CHAN_NO_OFDM: OFDM is not allowed on this channel.
 * @IEEE80211_CHAN_NO_80MHZ: If the driver supports 80 MHz on the band,
 *      this flag indicates that an 80 MHz channel cannot use this
 *      channel as the control or any of the secondary channels.
 *      This may be due to the driver or due to regulatory bandwidth
 *      restrictions.
 * @IEEE80211_CHAN_NO_160MHZ: If the driver supports 160 MHz on the band,
 *      this flag indicates that an 160 MHz channel cannot use this
 *      channel as the control or any of the secondary channels.
 *      This may be due to the driver or due to regulatory bandwidth
 *      restrictions.
 * @IEEE80211_CHAN_INDOOR_ONLY: see %NL80211_FREQUENCY_ATTR_INDOOR_ONLY
 * @IEEE80211_CHAN_GO_CONCURRENT: see %NL80211_FREQUENCY_ATTR_GO_CONCURRENT
 * @IEEE80211_CHAN_NO_20MHZ: 20 MHz bandwidth is not permitted
 *      on this channel.
 * @IEEE80211_CHAN_NO_10MHZ: 10 MHz bandwidth is not permitted
 *      on this channel.
 *
 */
enum ieee80211_channel_flags {
	IEEE80211_CHAN_DISABLED         = 1<<0,
	IEEE80211_CHAN_NO_IR            = 1<<1,
	/* hole at 1<<2 */
	IEEE80211_CHAN_RADAR            = 1<<3,
	IEEE80211_CHAN_NO_HT40PLUS      = 1<<4,
	IEEE80211_CHAN_NO_HT40MINUS     = 1<<5,
	IEEE80211_CHAN_NO_OFDM          = 1<<6,
	IEEE80211_CHAN_NO_80MHZ         = 1<<7,
	IEEE80211_CHAN_NO_160MHZ        = 1<<8,
	IEEE80211_CHAN_INDOOR_ONLY      = 1<<9,
	IEEE80211_CHAN_GO_CONCURRENT    = 1<<10,
	IEEE80211_CHAN_NO_20MHZ         = 1<<11,
	IEEE80211_CHAN_NO_10MHZ         = 1<<12,
};

/* needed by mgmt/rlm_domain.c */
#define IEEE80211_CHAN_NO_HT40 \
	(IEEE80211_CHAN_NO_HT40PLUS | IEEE80211_CHAN_NO_HT40MINUS)

/*
 * too many os dependent in regular domain
 * on/off fail
 */
#if CFG_SUPPORT_SINGLE_SKU_LOCAL_DB == 1
/* at leat 7 is needed for regdom_jp */
#define MAX_NUMER_REG_RULES	7

struct ieee80211_power_rule {
	u32 max_antenna_gain;
	u32 max_eirp;
};

struct ieee80211_freq_range {
	u32 start_freq_khz;
	u32 end_freq_khz;
	u32 max_bandwidth_khz;
};

struct ieee80211_reg_rule {
	struct ieee80211_freq_range freq_range;
	struct ieee80211_power_rule power_rule;
	u32 flags; /* enum reg_flags */
	u32 dfs_cac_ms;
};

struct ieee80211_regdomain {
	char alpha2[3];
	u32 n_reg_rules;
	enum nl80211_dfs_regions dfs_region;
	struct ieee80211_reg_rule reg_rules[MAX_NUMER_REG_RULES];
};

#define MHZ_TO_KHZ(freq) ((freq) * 1000)
#define KHZ_TO_MHZ(freq) ((freq) / 1000)
#define DBI_TO_MBI(gain) ((gain) * 100)
#define MBI_TO_DBI(gain) ((gain) / 100)
#define DBM_TO_MBM(gain) ((gain) * 100)
#define MBM_TO_DBM(gain) ((gain) / 100)

#define REG_RULE_EXT(start, end, bw, gain, eirp, dfs_cac, reg_flags)    \
{                                                                       \
	.freq_range.start_freq_khz = MHZ_TO_KHZ(start),                 \
	.freq_range.end_freq_khz = MHZ_TO_KHZ(end),                     \
	.freq_range.max_bandwidth_khz = MHZ_TO_KHZ(bw),                 \
	.power_rule.max_eirp = DBM_TO_MBM(eirp),                        \
	.flags = reg_flags,                                             \
	.dfs_cac_ms = dfs_cac,                                          \
}

#define REG_RULE(start, end, bw, gain, eirp, reg_flags) \
{                                                       \
	.freq_range.start_freq_khz = MHZ_TO_KHZ(start), \
	.freq_range.end_freq_khz = MHZ_TO_KHZ(end),     \
	.freq_range.max_bandwidth_khz = MHZ_TO_KHZ(bw), \
	.power_rule.max_antenna_gain = DBI_TO_MBI(gain),\
	.power_rule.max_eirp = DBM_TO_MBM(eirp),        \
	.flags = reg_flags,                             \
}
#endif
/*
 * needed by mgmt/tdls.c
 */
enum gfp_t {
	GFP_KERNEL,
	GFP_ATOMIC,
	__GFP_HIGHMEM,
	__GFP_HIGH
};
/* need by include/hal.h, halDeAggRxPktWorker
 * comment: use os-related structure directly outside headers of gl layer
 * while the implementation is in os/linux/hif*
 * possible actions:
 * 1) should we just move the function prototype to os gl layer?
 */
struct work_struct {
	/* atomic_long_t data; */
	/* struct list_head entry; */
	/* work_func_t func; */
};
/*
 * TODO: Functions need implementation
 */

/****************************************************************************
 * TODO: Functions prototype, which could be realized as follows
 * 1) inline function
 * 2) os API with same functionality
 * 3) implemented in gl_dependent.c
 ****************************************************************************
 */
/*
 * KAL_NEED_IMPLEMENT: wrapper to caution user to implement func when porting
 * @file: from which file
 * @func: called by which func
 * @line: at which line
 */
long KAL_NEED_IMPLEMENT(const char *file, const char *func, int line, ...);

/*
 * kal_dbg_print: print debug message to screen
 *
 * needed by common/debug.c
 * source/include/linux/printk.h
 */
int kal_dbg_print(const char *s, ...);
#define pr_info(fmt, ...) printf(fmt)

/*
 * kal_hex_dump_to_buffer: convert a blob of data to "hex ASCII" in memory
 * @buf: data blob to dump
 * @len: number of bytes in the @buf
 * @rowsize: number of bytes to print per line; must be 16 or 32
 * @groupsize: number of bytes to print at a time (1, 2, 4, 8; default = 1)
 * @linebuf: where to put the converted data
 * @linebuflen: total size of @linebuf, including space for terminating NUL
 * @ascii: include ASCII after the hex output
 *
 * needed by common/debug.c
 * source/include/linux/printk.h
 */
int kal_hex_dump_to_buffer(const void *buf, size_t len, int rowsize,
	int groupsize, char *linebuf, size_t linebuflen,
	bool ascii);
#define hex_dump_to_buffer(_buf, _len, _rowsize, _groupsize, _linebuf, \
	_linebuflen, _ascii) \
	kal_hex_dump_to_buffer(_buf, _len, _rowsize, _groupsize, _linebuf, \
	_linebuflen, _ascii)

/*
 * purpose:
 * polite version of BUG_ON() - WARN_ON() which doesn't
 * kill the machine, replace panic() to dump_stack()
 * needed by mgmt/rlm_domain.c
 */
bool kal_warn_on(uint8_t condition);
#define WARN_ON(_condition) kal_warn_on(_condition)
/*
 * kstrto* - convert a string to an *
 * @s: The start of the string. The string must be null-terminated, and may also
 * include a single newline before its terminating null. The first character
 * may also be a plus sign or a minus sign.
 * @base: The number base to use. The maximum supported base is 16. If base is
 * given as 0, then the base of the string is automatically detected with the
 * conventional semantics - If it begins with 0x the number will be parsed as a
 * hexadecimal (case insensitive), if it otherwise begins with 0, it will be
 * parsed as an octal number. Otherwise it will be parsed as a decimal.
 * @res: Where to write the result of the conversion on success.
 *
 * Returns 0 on success, -ERANGE on overflow and -EINVAL on parsing error.
 * Used as a replacement for the obsolete simple_strtoull. Return code must
 * be checked.
 */
int kal_strtoint(const char *s, unsigned int base, int *res);
/*
 * kstrtoul - convert a string to an unsigned long
 * @s: The start of the string. The string must be null-terminated, and may also
 * include a single newline before its terminating null. The first character
 * may also be a plus sign, but not a minus sign.
 * @base: The number base to use. The maximum supported base is 16. If base is
 * given as 0, then the base of the string is automatically detected with the
 * conventional semantics - If it begins with 0x the number will be parsed as a
 * hexadecimal (case insensitive), if it otherwise begins with 0, it will be
 * parsed as an octal number. Otherwise it will be parsed as a decimal.
 * @res: Where to write the result of the conversion on success.
 *
 * Returns 0 on success, -ERANGE on overflow and -EINVAL on parsing error.
 * Used as a replacement for the obsolete simple_strtoull. Return code must
 * be checked.
 */
int kal_strtoul(const char *s, unsigned int base, unsigned long *res);

/*
 * scnprintf - Format a string and place it in a buffer
 * @buf: The buffer to place the result into
 * @size: The size of the buffer, including the trailing null space
 * @fmt: The format string to use
 * @...: Arguments for the format string
 *
 * The return value is the number of characters written into @buf not including
 * the trailing '\0'. If @size is == 0 the function returns 0.
 */
int kal_scnprintf(char *buf, size_t size, const char *fmt, ...);

void *kal_kmalloc(size_t size, enum gfp_t type);
void *kal_vmalloc(size_t size);

void kal_kfree(void *addr);
void kal_vfree(void *addr);
/* looks like min/max not in C
 * https://stackoverflow.com/questions/3437404/min-and-max-in-c
 * needed by:
 * common/debug.c
 * mgmt/scan.c
 */
#define min(_a, _b) KAL_NEED_IMPLEMENT(__FILE__, __func__, __LINE__)

/*	implemented: os/linux/gl_wext.c
 *	used: common/wlan_oid.c, wlanoidSetWapiAssocInfo
 *	why function called "search WPAIIE" has been implement under wext
 *	first used in wext_get_scan. Should it be part of wlan_oid?
 */
#if CFG_SUPPORT_WAPI
#define wextSrchDesiredWAPIIE(_pucIEStart, _i4TotalIeLen, \
	_ppucDesiredIE) KAL_NEED_IMPLEMENT(__FILE__, __func__, __LINE__)
#endif

/* needed by mgmt/rlm_domain.c
 * implementation in linux is in gl_cfg80211.c
 * while other operating system may also need to notify reg change
 */
#define mtk_reg_notify(_pWiphy, _pRequest) \
	KAL_NEED_IMPLEMENT(__FILE__, __func__, __LINE__)

/* needed by mgmt/rlm_domain.c */
#define regulatory_hint(_wiphy, _alpha2) \
	KAL_NEED_IMPLEMENT(__FILE__, __func__, __LINE__)

/*
 * needed by: nic/nic_cmd_event.c
 * 0: no error !0: error
 * defeined in include/linux/err.h
 */
#define IS_ERR(_ptr) \
	KAL_NEED_IMPLEMENT(__FILE__, __func__, __LINE__)

#define div_u64(_val, _div) \
	KAL_NEED_IMPLEMENT(__FILE__, __func__, __LINE__)

#define iounmap(_vir_addr) \
	KAL_NEED_IMPLEMENT(__FILE__, __func__, __LINE__)
#endif
