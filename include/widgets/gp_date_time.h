//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_DATE_TIME_H
#define GP_DATE_TIME_H

#include <time.h>

const char *gp_str_time_diff(char *buf, size_t buf_len, time_t time, time_t now);

#endif /* GP_TIME_DATE_H */
