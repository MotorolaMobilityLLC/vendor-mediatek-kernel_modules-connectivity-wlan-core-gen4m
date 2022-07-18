/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2020 MediaTek Inc.
 */

#include "precomp.h"

static DEFINE_MUTEX(wfsys_mutex);

void wfsys_lock(void)
{
	mutex_lock(&wfsys_mutex);
}

int wfsys_trylock(void)
{
	return mutex_trylock(&wfsys_mutex);
}

void wfsys_unlock(void)
{
	mutex_unlock(&wfsys_mutex);
}

int wfsys_is_locked(void)
{
	return mutex_is_locked(&wfsys_mutex);
}

