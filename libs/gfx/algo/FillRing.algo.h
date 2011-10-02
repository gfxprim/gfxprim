/*****************************************************************************
 * This file is part of gfxprim library.                                     *
 *                                                                           *
 * Gfxprim is free software; you can redistribute it and/or                  *
 * modify it under the terms of the GNU Lesser General Public                *
 * License as published by the Free Software Foundation; either              *
 * version 2.1 of the License, or (at your option) any later version.        *
 *                                                                           *
 * Gfxprim is distributed in the hope that it will be useful,                *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Lesser General Public License for more details.                           *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public          *
 * License along with gfxprim; if not, write to the Free Software            *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,                        *
 * Boston, MA  02110-1301  USA                                               *
 *                                                                           *
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos                            *
 *                         <jiri.bluebear.dluhos@gmail.com>                  *
 *                                                                           *
 * Copyright (C) 2009-2010 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

/*
 * A filled ring drawing algorithm.
 */

/*
 * This macro defines a filled circle drawing function.
 * Arguments:
 *     CONTEXT_T - user-defined type of drawing context (passed to HLINE)
 *     PIXVAL_T  - user-defined pixel value type (passed to HLINE)
 *     HLINE     - horizontal line drawing function f(context, x0, x1, y, pixval)
 *     FN_NAME   - name of the function to be defined
 */
#define DEF_FILLRING_FN(FN_NAME, CONTEXT_T, PIXVAL_T, HLINE) \
void FN_NAME(CONTEXT_T context, int xcenter, int ycenter, \
	unsigned int r1, unsigned int r2, PIXVAL_T pixval) \
{ \
	int outer_r = (int) GP_MAX(r1, r2); \
	int inner_r = (int) GP_MIN(r1, r2); \
	int outer_x = 0; \
	int inner_x = 0; \
	int y; \
	int outer_error = -outer_r; \
	int inner_error = -inner_r; \
	for (y = outer_r; y >= 0; y--) { \
\
		while (outer_error < 0) { \
			outer_error += 2*outer_x + 1; \
			outer_x++; \
		} \
		outer_error += -2*y + 1; \
\
		if (y < inner_r && y > -inner_r) { \
			while (inner_error < 0) { \
				inner_error += 2*inner_x + 1; \
				inner_x++; \
			} \
			inner_error += -2*y + 1; \
\
			HLINE(context, xcenter - outer_x + 1, xcenter - inner_x,     ycenter - y, pixval); \
			HLINE(context, xcenter + inner_x,     xcenter + outer_x - 1, ycenter - y, pixval); \
			HLINE(context, xcenter - outer_x + 1, xcenter - inner_x,     ycenter + y, pixval); \
			HLINE(context, xcenter + inner_x,     xcenter + outer_x - 1, ycenter + y, pixval); \
		} else { \
			HLINE(context, xcenter - outer_x + 1, xcenter + outer_x - 1, ycenter-y, pixval); \
			HLINE(context, xcenter - outer_x + 1, xcenter + outer_x - 1, ycenter+y, pixval); \
		} \
	} \
}
