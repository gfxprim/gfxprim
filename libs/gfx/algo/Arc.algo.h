// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

/*
 * The elliptical arc algorithm; a generalization of an ellipse that allows
 * to draw only part of it. The math is exactly the same, only it has
 * additional tests of what part to draw.
 */

/*
 * This macro defines a function that draws a segment of an arc within
 * two horizontal quadrants. For a larger arc, two calls are needed.
 *
 * The 'direction' parameter specifies which two quadrants to work in:
 * if <0, the top two quadrants (y < 0) are used, if >0, the bottom two
 * (y > 0) are used.
 *
 * Arguments:
 *     PIXMAP_T - user-defined type of drawing pixmap (passed to PUTPIXEL)
 *     PIXVAL_T  - user-defined pixel value type (passed to PUTPIXEL)
 *     PUTPIXEL  - a pixel drawing function f(pixmap, x, y, pixval)
 *     FN_NAME   - name of the function to be defined
 */
#define DEF_ARCSEGMENT_FN(FN_NAME, PIXMAP_T, PIXVAL_T, PUTPIXEL) \
static void FN_NAME(PIXMAP_T pixmap, int xcenter, int ycenter, \
	unsigned int a, unsigned int b, int direction, \
	double start, double end, PIXVAL_T pixval) \
{ \
	/* Precompute quadratic terms. */ \
	int a2 = a*a; \
	int b2 = b*b; \
\
	/* Compute minimum and maximum value of X from the angles. */ \
	int x1 = (int)(cos(start)*a); \
	int x2 = (int)(cos(end)*a); \
	int xmin = GP_MIN(x1, x2); \
	int xmax = GP_MAX(x1, x2); \
\
	int x, y, error; \
	for (x = 0, error = -b2*a, y = b; y >= 0; y--) { \
		while (error < 0) { \
\
			/* Calculate error(x+1) from error(x). */ \
			error += 2*x*b2 + b2; \
			x++; \
\
			if (direction < 0) { \
				if ((-x+1) >= xmin && (-x+1) <= xmax) { \
					PUTPIXEL(pixmap, xcenter-x+1, ycenter-y, pixval); \
				} \
				if ((x-1) >= xmin && (x-1) <= xmax) { \
					PUTPIXEL(pixmap, xcenter+x-1, ycenter-y, pixval); \
				} \
			} \
			if (direction > 0) { \
				if ((-x+1) >= xmin && (-x+1) <= xmax) { \
					PUTPIXEL(pixmap, xcenter-x+1, ycenter+y, pixval); \
				} \
				if ((x-1) >= xmin && (x-1) <= xmax) { \
					PUTPIXEL(pixmap, xcenter+x-1, ycenter+y, pixval); \
				} \
			} \
		} \
\
		/* Calculate error(y-1) from error(y). */ \
		error += -2*y*a2 + a2; \
\
		if (direction < 0) { \
			if ((-x+1) >= xmin && (-x+1) <= xmax) { \
				PUTPIXEL(pixmap, xcenter-x+1, ycenter-y, pixval); \
			} \
			if ((x-1) >= xmin && (x-1) <= xmax) { \
				PUTPIXEL(pixmap, xcenter+x-1, ycenter-y, pixval); \
			} \
		} \
		if (direction > 0) { \
			if ((-x+1) >= xmin && (-x+1) <= xmax) { \
				PUTPIXEL(pixmap, xcenter-x+1, ycenter+y, pixval); \
			} \
			if ((x-1) >= xmin && (x-1) <= xmax) { \
				PUTPIXEL(pixmap, xcenter+x-1, ycenter+y, pixval); \
			} \
		} \
	} \
}
