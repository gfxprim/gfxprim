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

#ifndef GP_CORE_H
#define GP_CORE_H

/* Library verson */
#include "core/gp_version.h"

/* Common building blocks */
#include "core/gp_common.h"

/* Pixmap ... */
#include "core/gp_pixmap.h"

/* ... and it's trasformations */
#include <core/gp_transform.h>

/* Gamma */
#include <core/gp_gamma.h>

/* Pixeltypes */
#include <core/gp_pixel.h>

/* Pixel conversions */
#include <core/gp_convert.h>

/* Individual pixel access */
#include <core/gp_get_put_pixel.h>

/* Writing pixel blocks */
#include <core/gp_write_pixel.h>

/* Blitting */
#include <core/gp_blit.h>

/* Debug and debug level */
#include "core/gp_debug.h"

/* Progress callback */
#include <core/gp_progress_callback.h>

/* Threads utils */
#include <core/gp_threads.h>

/* Mix Pixel */
#include <core/gp_mix_pixels.h>

#include "core/gp_fill.h"

#endif /* GP_CORE_H */
