@ include source.t
/*
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 *
 * Calls different API for the same filter (Alloc vs In-place, etc.) and copares
 * the results (which must be the same).
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <core/GP_Context.h>
#include <filters/GP_Filters.h>

#include "tst_test.h"
@
@ # The format is [[name1, [fn1, [params]], [fn2, [params]], ...],
@ #                [name2, [fn3, [params]], [fn4, [params]], ...],
@ #                ...]
@ #
@ # All results from filters listed under one name are compared.
@
@ compare_list = [
@                 ['Brightness',
@                  ['Brightness', ['dst', 'dst', '0.1', 'NULL']],
@                  ['BrightnessAlloc', ['src', '0.1', 'NULL']],
@                 ],
@                 ['Contrast',
@                  ['Contrast', ['dst', 'dst', '1.2', 'NULL']],
@                  ['ContrastAlloc', ['src', '1.2', 'NULL']],
@                 ],
@                 ['Invert',
@                  ['Invert', ['dst', 'dst', 'NULL']],
@                  ['InvertAlloc', ['src', 'NULL']],
@                 ],
@                 ['MirrorH',
@                  ['MirrorH', ['dst', 'dst', 'NULL']],
@                  ['MirrorHAlloc', ['src', 'NULL']],
@                  ['Symmetry', ['dst', 'dst', 'GP_MIRROR_H', 'NULL']],
@                  ['SymmetryAlloc', ['src', 'GP_MIRROR_H', 'NULL']],
@                 ],
@                 ['MirrorV',
@                  ['MirrorV', ['dst', 'dst', 'NULL']],
@                  ['MirrorVAlloc', ['src', 'NULL']],
@                  ['Symmetry', ['dst', 'dst', 'GP_MIRROR_V', 'NULL']],
@                  ['SymmetryAlloc', ['src', 'GP_MIRROR_V', 'NULL']],
@                 ],
@                 ['Rotate90',
@                  ['Rotate90Alloc', ['src', 'NULL']],
@                  ['SymmetryAlloc', ['src', 'GP_ROTATE_90', 'NULL']],
@                 ],
@                 ['Rotate180',
@                  ['Rotate180', ['src', 'dst', 'NULL']],
@                  ['Rotate180Alloc', ['src', 'NULL']],
@                  ['Symmetry', ['src', 'dst', 'GP_ROTATE_180', 'NULL']],
@                  ['SymmetryAlloc', ['src', 'GP_ROTATE_180', 'NULL']],
@                 ],
@                 ['Rotate270',
@                  ['Rotate270Alloc', ['src', 'NULL']],
@                  ['SymmetryAlloc', ['src', 'GP_ROTATE_270', 'NULL']],
@                 ],
@                 ['ResizeNN',
@                  ['Resize', ['dst', 'dst', 'GP_INTERP_NN', 'NULL']],
@                  ['ResizeAlloc', ['src', 'src->w', 'src->h',
@                                   'GP_INTERP_NN', 'NULL']],
@                 ],
@                 ['ResizeLinearInt',
@                  ['Resize', ['dst', 'dst', 'GP_INTERP_LINEAR_INT', 'NULL']],
@                  ['ResizeAlloc', ['src', 'src->w', 'src->h',
@                                   'GP_INTERP_LINEAR_INT', 'NULL']],
@                 ],
@                 ['ResizeLinearLFInt',
@                  ['Resize', ['dst', 'dst', 'GP_INTERP_LINEAR_LF_INT', 'NULL']],
@                  ['ResizeAlloc', ['src', 'src->w', 'src->h',
@                                   'GP_INTERP_LINEAR_LF_INT', 'NULL']],
@                 ],
@                 ['ResizeCubicInt',
@                  ['Resize', ['dst', 'dst', 'GP_INTERP_CUBIC_INT', 'NULL']],
@                  ['ResizeAlloc', ['src', 'src->w', 'src->h',
@                                   'GP_INTERP_CUBIC_INT', 'NULL']],
@                 ],
@                 ['ResizeCubic',
@                  ['Resize', ['dst', 'dst', 'GP_INTERP_CUBIC', 'NULL']],
@                  ['ResizeAlloc', ['src', 'src->w', 'src->h',
@                                   'GP_INTERP_CUBIC', 'NULL']],
@                 ],
@                 ['Laplace',
@                  ['Laplace', ['src', 'dst', 'NULL']],
@                  ['LaplaceAlloc', ['src', 'NULL']],
@                 ],
@                 ['EdgeSharpening',
@                  ['EdgeSharpening', ['src', 'dst', '0.2', 'NULL']],
@                  ['EdgeSharpeningAlloc', ['src', '0.2', 'NULL']],
@                 ],
@                 ['Median',
@                  ['Median', ['src', 'dst', '3', '2', 'NULL']],
@                  ['MedianAlloc', ['src', '3', '2', 'NULL']],
@                 ],
@                 ['Sigma',
@                  ['Sigma', ['dst', 'dst', '3', '2', '4', '0.5', 'NULL']],
@                  ['SigmaAlloc', ['src', '3', '2', '4', '0.5', 'NULL']],
@                 ],
@                 ['GaussianBlur',
@                  ['GaussianBlur', ['dst', 'dst', '10', '12', 'NULL']],
@                  ['GaussianBlurAlloc', ['src', '10', '12', 'NULL']]
@                 ],
@ ]
@
@ def apply_filter(filter):
@     if 'dst' in filter[1]:
	dst = GP_ContextCopy(src, GP_COPY_WITH_PIXELS);
	if (GP_Filter{{ filter[0] }}({{ arr_to_params(filter[1]) }})) {
		int err = errno;
		GP_ContextFree(dst);
		dst = NULL;
		errno = err;
	}

@     else:
	dst = GP_Filter{{ filter[0] }}({{ arr_to_params(filter[1]) }});

@ end
@
@ for fs in compare_list:
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
{@ apply_filter(fs[1]) @}
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
@
@     for i in fs[2:]:
{@ apply_filter(i) @}

	if (!GP_ContextEqual(ref, dst)) {
		fail++;
		tst_msg("Results for {{ fs[1][0] }} and {{ i[0] }} differs");
	}

	GP_ContextFree(dst);
@     end

	GP_ContextFree(src);
	GP_ContextFree(ref);

	if (fail) {
		tst_msg("%i failure(s)", fail);
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

@ end
@
@ for fs in compare_list:
@     for pt in pixeltypes:
@         if not pt.is_unknown():
static int compare_{{ fs[0] }}_{{ pt.name }}(void)
{
	return compare_{{ fs[0] }}({{ pt.C_type }});
}
@ end

const struct tst_suite tst_suite = {
	.suite_name = "Filters Compare",
	.tests = {
@ for fs in compare_list:
@     for pt in pixeltypes:
@         if not pt.is_unknown():
		{.name = "Compare {{ fs[0] }} {{ pt.name }}",
		 .tst_fn = compare_{{ fs[0] }}_{{ pt.name }},
		 .flags = TST_CHECK_MALLOC},
@ end
		{.name = NULL}
	}
};
