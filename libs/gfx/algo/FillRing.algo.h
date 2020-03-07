// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

/*
 * A filled ring drawing algorithm.
 */

/*
 * This macro defines a filled circle drawing function.
 * Arguments:
 *     PIXMAP_T - user-defined type of drawing pixmap (passed to HLINE)
 *     PIXVAL_T  - user-defined pixel value type (passed to HLINE)
 *     HLINE     - horizontal line drawing function f(pixmap, x0, x1, y, pixval)
 *     FN_NAME   - name of the function to be defined
 */
#define DEF_FILLRING_FN(FN_NAME, PIXMAP_T, PIXVAL_T, HLINE) \
static void FN_NAME(PIXMAP_T pixmap, int xcenter, int ycenter, \
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
			HLINE(pixmap, xcenter - outer_x + 1, xcenter - inner_x,     ycenter - y, pixval); \
			HLINE(pixmap, xcenter + inner_x,     xcenter + outer_x - 1, ycenter - y, pixval); \
			HLINE(pixmap, xcenter - outer_x + 1, xcenter - inner_x,     ycenter + y, pixval); \
			HLINE(pixmap, xcenter + inner_x,     xcenter + outer_x - 1, ycenter + y, pixval); \
		} else { \
			HLINE(pixmap, xcenter - outer_x + 1, xcenter + outer_x - 1, ycenter-y, pixval); \
			HLINE(pixmap, xcenter - outer_x + 1, xcenter + outer_x - 1, ycenter+y, pixval); \
		} \
	} \
}
