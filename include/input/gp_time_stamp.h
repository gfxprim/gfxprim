// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Returns monotonously incrementing timestamps in milliseconds.

 */

#ifndef INPUT_GP_TIME_STAMP_H
#define INPUT_GP_TIME_STAMP_H

#include <stdint.h>

uint64_t gp_time_stamp(void);

#endif /* INPUT_GP_TIME_STAMP_H */
