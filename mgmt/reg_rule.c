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

/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */

/*******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 *******************************************************************************
 */

#include "precomp.h"

struct mtk_cc_dfs_regd_mapping {
	uint16_t countryCode;
	enum ENUM_REG_DOMAIN ucDfsRegion;
};

const struct mtk_cc_dfs_regd_mapping my_dfs_ca = {
	.countryCode = COUNTRY_CODE_CA,
	.ucDfsRegion = ENUM_RDM_FCC
};

const struct mtk_cc_dfs_regd_mapping my_dfs_cf = {
	.countryCode = COUNTRY_CODE_CF,
	.ucDfsRegion = ENUM_RDM_FCC
};

const struct mtk_cc_dfs_regd_mapping my_dfs_td = {
	.countryCode = COUNTRY_CODE_TD,
	.ucDfsRegion = ENUM_RDM_FCC
};

const struct mtk_cc_dfs_regd_mapping my_dfs_us = {
	.countryCode = COUNTRY_CODE_US,
	.ucDfsRegion = ENUM_RDM_FCC
};

const struct mtk_cc_dfs_regd_mapping my_dfs_tw = {
	.countryCode = COUNTRY_CODE_TW,
	.ucDfsRegion = ENUM_RDM_FCC
};

const struct mtk_cc_dfs_regd_mapping my_dfs_na = {
	.countryCode = COUNTRY_CODE_NA,
	.ucDfsRegion = ENUM_RDM_FCC
};

const struct mtk_cc_dfs_regd_mapping my_dfs_cn = {
	.countryCode = COUNTRY_CODE_CN,
	.ucDfsRegion = ENUM_RDM_CHN
};

const struct mtk_cc_dfs_regd_mapping my_dfs_jp = {
	.countryCode = COUNTRY_CODE_JP,
	.ucDfsRegion = ENUM_RDM_JAP
};

const struct mtk_cc_dfs_regd_mapping my_dfs_kr = {
	.countryCode = COUNTRY_CODE_KR,
	.ucDfsRegion = ENUM_RDM_KR
};

const struct mtk_cc_dfs_regd_mapping *g_prDfsCountryTable[] = {
	&my_dfs_ca,
	&my_dfs_cf,
	&my_dfs_us,
	&my_dfs_td,
	&my_dfs_tw,
	&my_dfs_na,
	&my_dfs_cn,
	&my_dfs_jp,
	&my_dfs_td,
	&my_dfs_kr,
	NULL /* this NULL SHOULD be at the end of the array */
};

uint8_t regCountryDfsMapping(struct ADAPTER *prAdapter)
{
	u8 idx = 0;
	const struct mtk_cc_dfs_regd_mapping *prRegd = NULL;

	DBGLOG(P2P, TRACE,
		"Country Code = 0x%04x\n",
		prAdapter->rWifiVar.u2CountryCode);
	while (g_prDfsCountryTable[idx]) {
		prRegd = g_prDfsCountryTable[idx];

		if (prRegd->countryCode ==
			prAdapter->rWifiVar.u2CountryCode)
			return (uint8_t) prRegd->ucDfsRegion;

		idx++;
	}

	return (uint8_t) ENUM_RDM_CE;
}


