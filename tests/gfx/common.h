// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef __COMMON_H__
#define __COMMON_H__

#include <core/gp_pixmap.h>

void dump_buffers(const char *pattern, const gp_pixmap *c);

int compare_buffers(const char *pattern, const gp_pixmap *c);

gp_pixmap *pixmap_alloc_canary(gp_size w, gp_size h, gp_pixel_type ptype);

int check_canary(gp_pixmap *pixmap);

#endif /* __COMMON_H__ */
