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
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <errno.h>

#include "core/GP_Context.h"
#include "core/GP_Debug.h"

#include "GP_ResizeNN.h"
#include "GP_ResizeLinear.h"
#include "GP_ResizeCubic.h"
#include "GP_Resize.h"

static const char *interp_types[] = {
	"Nearest Neighbour",
	"Linear (Int)",
	"Linear with Low Pass (Int)",
	"Cubic (Float)",
	"Cubic (Int)",
};

const char *GP_InterpolationTypeName(enum GP_InterpolationType interp_type)
{
	if (interp_type > GP_INTERP_MAX)
		return "Unknown";

	return interp_types[interp_type];
}

static int resize(const GP_Context *src, GP_Context *dst,
                  GP_InterpolationType type,
                  GP_ProgressCallback *callback)
{
	switch (type) {
	case GP_INTERP_NN:
		return GP_FilterResizeNN(src, dst, callback);
	case GP_INTERP_LINEAR_INT:
		return GP_FilterResizeLinearInt(src, dst, callback);
	case GP_INTERP_LINEAR_LF_INT:
		return GP_FilterResizeLinearLFInt(src, dst, callback);
	case GP_INTERP_CUBIC:
		return GP_FilterResizeCubic(src, dst, callback);
	case GP_INTERP_CUBIC_INT:
		return GP_FilterResizeCubicInt(src, dst, callback);
	}

	GP_WARN("Invalid interpolation type %u", (unsigned int)type);

	errno = EINVAL;
	return 1;
}

int GP_FilterResize(const GP_Context *src, GP_Context *dst,
                    GP_InterpolationType type,
                    GP_ProgressCallback *callback)
{
	if (src->pixel_type != dst->pixel_type) {
		GP_WARN("The src and dst pixel types must match");
		errno = EINVAL;
		return 1;
	}

	return resize(src, dst, type, callback);
}

GP_Context *GP_FilterResizeAlloc(const GP_Context *src,
                                 GP_Size w, GP_Size h,
                                 GP_InterpolationType type,
                                 GP_ProgressCallback *callback)
{
	GP_Context *res = GP_ContextAlloc(w, h, src->pixel_type);

	if (!res)
		return NULL;

	if (resize(src, res, type, callback)) {
		GP_ContextFree(res);
		return NULL;
	}

	return res;
}
