// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef FILTERS_GP_EDGE_DETECTION_H
#define FILTERS_GP_EDGE_DETECTION_H

#include <filters/gp_filter.h>

int gp_filter_edge_sobel(const gp_pixmap *src,
                         gp_pixmap **E, gp_pixmap **Phi,
                         gp_progress_cb *callback);

int gp_filter_edge_prewitt(const gp_pixmap *src,
                           gp_pixmap **E, gp_pixmap **Phi,
                           gp_progress_cb *callback);


#endif /* FILTERS_GP_EDGE_DETECTION_H */
