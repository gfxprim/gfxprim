// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2022 Cyril Hrubis <metan@ucw.cz>
 */

#include <input/gp_time_stamp.h>
#include "FreeRTOS.h"
#include "task.h"

uint64_t gp_time_stamp(void)
{
	return xTaskGetTickCount() * 1000 / configTICK_RATE_HZ;
}
