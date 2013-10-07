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

%% extends "base.test.c.t"

%% block descr
Filters Compare

Calls different API for the same filter (Alloc vs In-place, etc.) and copares
the results (which must be the same).
%% endblock descr

%% block body

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <core/GP_Context.h>
#include <filters/GP_Filters.h>

#include "tst_test.h"

/*
The format is [[name1, [fn1, [params]], [fn2, [params]], ...],
               [name2, [fn3, [params]], [fn4, [params]], ...],
               ...]

All results from filters listed under one name are compared.
*/
%% set compare_list = [['MirrorH',
                        ['MirrorH', ['dst', 'dst', 'NULL']],
                        ['MirrorHAlloc', ['src', 'NULL']],
                        ['Symmetry', ['dst', 'dst', 'GP_MIRROR_H', 'NULL']],
                        ['SymmetryAlloc', ['src', 'GP_MIRROR_H', 'NULL']],
                       ],
                       ['MirrorV',
                        ['MirrorV', ['dst', 'dst', 'NULL']],
                        ['MirrorVAlloc', ['src', 'NULL']],
                        ['Symmetry', ['dst', 'dst', 'GP_MIRROR_V', 'NULL']],
                        ['SymmetryAlloc', ['src', 'GP_MIRROR_V', 'NULL']],
                       ],
                       ['Rotate90',
                        ['Rotate90', ['dst', 'dst', 'NULL']],
                        ['Rotate90Alloc', ['src', 'NULL']],
                        ['Symmetry', ['dst', 'dst', 'GP_ROTATE_90', 'NULL']],
                        ['SymmetryAlloc', ['src', 'GP_ROTATE_90', 'NULL']],
                       ],
                       ['Rotate180',
                        ['Rotate180', ['dst', 'dst', 'NULL']],
                        ['Rotate180Alloc', ['src', 'NULL']],
                        ['Symmetry', ['dst', 'dst', 'GP_ROTATE_180', 'NULL']],
                        ['SymmetryAlloc', ['src', 'GP_ROTATE_180', 'NULL']],
                       ],
                       ['Rotate270',
                        ['Rotate270', ['dst', 'dst', 'NULL']],
                        ['Rotate270Alloc', ['src', 'NULL']],
                        ['Symmetry', ['dst', 'dst', 'GP_ROTATE_270', 'NULL']],
                        ['SymmetryAlloc', ['src', 'GP_ROTATE_270', 'NULL']],
                       ],

                       ['ResizeNN',
                        ['Resize', ['dst', 'dst', 'GP_INTERP_NN', 'NULL']],
                        ['ResizeAlloc', ['src', 'src->w', 'src->h',
                                         'GP_INTERP_NN', 'NULL']],
                       ],
                       ['ResizeLinearInt',
                        ['Resize', ['dst', 'dst', 'GP_INTERP_LINEAR_INT', 'NULL']],
                        ['ResizeAlloc', ['src', 'src->w', 'src->h',
                                         'GP_INTERP_LINEAR_INT', 'NULL']],
                       ],
                       ['ResizeLinearLFInt',
                        ['Resize', ['dst', 'dst', 'GP_INTERP_LINEAR_LF_INT', 'NULL']],
                        ['ResizeAlloc', ['src', 'src->w', 'src->h',
                                         'GP_INTERP_LINEAR_LF_INT', 'NULL']],
                       ],
                       ['ResizeCubicInt',
                        ['Resize', ['dst', 'dst', 'GP_INTERP_CUBIC_INT', 'NULL']],
                        ['ResizeAlloc', ['src', 'src->w', 'src->h',
                                         'GP_INTERP_CUBIC_INT', 'NULL']],
                       ],
                       ['ResizeCubic',
                        ['Resize', ['dst', 'dst', 'GP_INTERP_CUBIC', 'NULL']],
                        ['ResizeAlloc', ['src', 'src->w', 'src->h',
                                         'GP_INTERP_CUBIC', 'NULL']],
                       ],

		       ['Laplace',
                        ['Laplace', ['dst', 'dst', 'NULL']],
                        ['LaplaceAlloc', ['src', 'NULL']],
                       ],
		       ['EdgeSharpening',
                        ['EdgeSharpening', ['dst', 'dst', '0.2', 'NULL']],
                        ['EdgeSharpeningAlloc', ['src', '0.2', 'NULL']],
                       ],

                       ['Median',
                        ['Median', ['dst', 'dst', '3', '2', 'NULL']],
                        ['MedianAlloc', ['src', '3', '2', 'NULL']],
                       ],

                       ['Sigma',
                        ['Sigma', ['dst', 'dst', '3', '2', '4', '0.5', 'NULL']],
                        ['SigmaAlloc', ['src', '3', '2', '4', '0.5', 'NULL']],
                       ],

                       ['GaussianBlur',
                        ['GaussianBlur', ['dst', 'dst', '10', '12', 'NULL']],
                        ['GaussianBlurAlloc', ['src', '10', '12', 'NULL']]
                       ],
]

%%- macro arr2params(arr)
{{ arr[0] }}{% for i in arr[1:] %}, {{ i }}{% endfor%}
%%- endmacro

%% macro apply_filter(filter)
%%  if 'dst' in filter[1]
	dst = GP_ContextCopy(src, GP_COPY_WITH_PIXELS);
	if (GP_Filter{{ filter[0] }}({{ arr2params(filter[1]) }})) {
		int err = errno;
		GP_ContextFree(dst);
		dst = NULL;
		errno = err;
	}

%%  else
	dst = GP_Filter{{ filter[0] }}({{ arr2params(filter[1]) }});
%%  endif

%% endmacro


%% for fs in compare_list
static int compare_{{ fs[0] }}(GP_PixelType pt)
{
	GP_Context *src = GP_ContextAlloc(134, 1072, pt);
	GP_Context *dst, *ref;
	int fail = 0;

	errno = 0;

	if (!src) {
		tst_msg("Malloc Failed :(");
		return TST_UNTESTED;
	}

	/* randomize context content */
	GP_FilterGaussianNoiseAdd(src, src, 10, 0, NULL);

	/* Create reference result */
{{ apply_filter(fs[1]) }}
	ref = dst;

	if (ref == NULL) {
		GP_ContextFree(src);
		switch (errno) {
		case ENOSYS:
			tst_msg("Not implemented");
			return TST_SKIPPED;
		case EINVAL:
			tst_msg("Couldn't be implemented");
			return TST_SKIPPED;
		default:
			tst_msg("Unexpected errno %s", strerror(errno));
			return TST_FAILED;
		}
	}

%%  for i in fs[2:]
{{ apply_filter(i) }}

	if (!GP_ContextEqual(ref, dst)) {
		fail++;
		tst_msg("Results for {{ fs[1][0] }} and {{ i[0] }} differs");
	}


	GP_ContextFree(dst);
%%  endfor

	GP_ContextFree(src);
	GP_ContextFree(ref);

	if (fail) {
		tst_msg("%i failure(s)", fail);
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

%% endfor

%% for fs in compare_list
%%  for pt in pixeltypes
%%   if not pt.is_unknown()
static int compare_{{ fs[0] }}_{{ pt.name }}(void)
{
	return compare_{{ fs[0] }}({{ pt.C_type }});
}
%%   endif
%%  endfor
%% endfor

const struct tst_suite tst_suite = {
	.suite_name = "Filters Compare",
	.tests = {
%% for fs in compare_list
%%  for pt in pixeltypes
%%   if not pt.is_unknown()
		{.name = "Compare {{ fs[0] }} {{ pt.name }}",
		 .tst_fn = compare_{{ fs[0] }}_{{ pt.name }},
		 .flags = TST_CHECK_MALLOC},
%%   endif
%%  endfor
%% endfor
		{.name = NULL}
	}
};

%% endblock body
