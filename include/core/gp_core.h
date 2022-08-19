// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

/*

  This is header file for public core API.

 */

#ifndef CORE_GP_CORE_H
#define CORE_GP_CORE_H

#include "../../config.h"

#include <core/gp_version.h>
#include <core/gp_common.h>
#include <core/gp_pixmap.h>
#include <core/gp_transform.h>
#include <core/gp_gamma.h>
#include <core/gp_pixel.h>
#include <core/gp_convert.h>
#include <core/gp_get_put_pixel.h>
#include <core/gp_write_pixel.h>
#include <core/gp_blit.h>
#include <core/gp_debug.h>
#include <core/gp_fill.h>
#include <core/gp_progress_callback.h>
#include <core/gp_mix_pixels.h>

#ifdef HAVE_PTHREAD
# include <core/gp_threads.h>
#endif

#endif /* CORE_GP_CORE_H */
