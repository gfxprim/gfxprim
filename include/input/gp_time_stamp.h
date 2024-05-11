// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2022 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_time_stamp.h
 * @brief A monotonous timestamps.
 */

#ifndef INPUT_GP_TIME_STAMP_H
#define INPUT_GP_TIME_STAMP_H

#include <stdint.h>

/**
 * @brief Returns current time stamp.
 *
 * @return A monotonously incrementing timestamp starting at some unspecified
 *         point in milliseconds.
 */
uint64_t gp_time_stamp(void);

#endif /* INPUT_GP_TIME_STAMP_H */
