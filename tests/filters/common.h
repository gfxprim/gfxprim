// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef TESTS_FILTERS_COMMON_H
#define TESTS_FILTERS_COMMON_H

#include <core/gp_pixmap.h>

void dump_buffers(const char *pattern, const gp_pixmap *c);

int compare_buffers(const char *pattern, const gp_pixmap *c);

#endif /* TESTS_FILTERS_COMMON_H */
