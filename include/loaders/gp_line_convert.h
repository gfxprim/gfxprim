// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Converts a continuous line of pixels from buffer A to a line of pixels in
  buffer B.

  Supports only trivial conversions i.e. RGB888 to BGR888 and G1_LE to G1_BE,
  etc.

  The code is mainly used in image loaders when saving image from memory buffer
  that has exactly same channels (in size and names) but placed differently in
  pixel buffer.

 */


#ifndef LOADERS_LINE_CONVERT_H
#define LOADERS_LINE_CONVERT_H

#include <core/gp_pixel.h>

typedef void (*gp_line_convert)(const uint8_t *in, uint8_t *out, unsigned int len);

/*
 * The out array is terminated by GP_PIXEL_UNKNOWN.
 *
 * Returns output pixel type given input pixel type and table of posible output
 * types.
 *
 * Returns GP_PIXEL_UNKNOWN if no conversion is posible.
 */
gp_pixel_type gp_line_convertible(gp_pixel_type in, gp_pixel_type out[]);

/*
 * Returns pointer to conversion function or NULL if there is none.
 */
gp_line_convert gp_line_convert_get(gp_pixel_type in, gp_pixel_type out);

#endif /* LOADERS_LINE_CONVERT_H */
