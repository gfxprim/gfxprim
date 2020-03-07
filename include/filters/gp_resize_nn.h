// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Nearest neighbour interpolation.

 */

#ifndef FILTERS_GP_RESIZE_NN_H
#define FILTERS_GP_RESIZE_NN_H

#include <filters/gp_filter.h>
#include <filters/gp_resize.h>

int gp_filter_resize_nn(const gp_pixmap *src, gp_pixmap *dst,
                      gp_progress_cb *callback);

static inline gp_pixmap *gp_filter_resize_nn_alloc(const gp_pixmap *src,
                                   gp_size w, gp_size h,
                                   gp_progress_cb *callback)
{
	return gp_filter_resize_alloc(src, w, h, GP_INTERP_NN, callback);
}

#endif /* FILTERS_GP_RESIZE_NN_H */
