// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef FILTERS_GP_LINEAR_THREADS_H
#define FILTERS_GP_LINEAR_THREADS_H

#include <filters/gp_filter.h>
#include <filters/gp_linear.h>

int gp_filter_convolution_mp_raw(const gp_convolution_params *params);

int gp_filter_vconvolution_mp_raw(const gp_convolution_params *params);

int gp_filter_hconvolution_mp_raw(const gp_convolution_params *params);

#endif /* FILTERS_GP_LINEAR_THREADS_H */
