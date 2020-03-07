// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

 /*

   Logs finished job result into the file.

  */

#ifndef TST_LOG_H
#define TST_LOG_H

#include <stdio.h>

struct tst_suite;
struct tst_job;

FILE *tst_log_open(const struct tst_suite *suite, const char *path);

int tst_log_append(struct tst_job *job, FILE *f);

int tst_log_close(FILE *f);

#endif /* TST_LOG_H */
