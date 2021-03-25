@ include source.t
/*
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 *
 * Calls different API for the same filter (_alloc vs In-place, etc.) and copares
 * the results (which must be the same).
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <core/gp_pixmap.h>
#include <core/gp_threads.h>
#include <filters/gp_filters.h>

#include "tst_test.h"
@
@ # The format is [[name1, [fn1, [params]], [fn2, [params]], ...],
@ #                [name2, [fn3, [params]], [fn4, [params]], ...],
@ #                ...]
@ #
@ # All results from filters listed under one name are compared.
@
@ compare_list = [
@                 ['brightness',
@                  ['brightness', ['dst', 'dst', '0.1', 'NULL']],
@                  ['brightness_alloc', ['src', '0.1', 'NULL']],
@                 ],
@                 ['contrast',
@                  ['contrast', ['dst', 'dst', '1.2', 'NULL']],
@                  ['contrast_alloc', ['src', '1.2', 'NULL']],
@                 ],
@                 ['invert',
@                  ['invert', ['dst', 'dst', 'NULL']],
@                  ['invert_alloc', ['src', 'NULL']],
@                 ],
@                 ['mirror_h',
@                  ['mirror_h', ['dst', 'dst', 'NULL']],
@                  ['mirror_h_alloc', ['src', 'NULL']],
@                  ['symmetry', ['dst', 'dst', 'GP_MIRROR_H', 'NULL']],
@                  ['symmetry_alloc', ['src', 'GP_MIRROR_H', 'NULL']],
@                 ],
@                 ['mirror_v',
@                  ['mirror_v', ['dst', 'dst', 'NULL']],
@                  ['mirror_v_alloc', ['src', 'NULL']],
@                  ['symmetry', ['dst', 'dst', 'GP_MIRROR_V', 'NULL']],
@                  ['symmetry_alloc', ['src', 'GP_MIRROR_V', 'NULL']],
@                 ],
@                 ['rotate_90',
@                  ['rotate_90_alloc', ['src', 'NULL']],
@                  ['symmetry_alloc', ['src', 'GP_ROTATE_90', 'NULL']],
@                 ],
@                 ['rotate_180',
@                  ['rotate_180', ['src', 'dst', 'NULL']],
@                  ['rotate_180_alloc', ['src', 'NULL']],
@                  ['symmetry', ['src', 'dst', 'GP_ROTATE_180', 'NULL']],
@                  ['symmetry_alloc', ['src', 'GP_ROTATE_180', 'NULL']],
@                 ],
@                 ['rotate_270',
@                  ['rotate_270_alloc', ['src', 'NULL']],
@                  ['symmetry_alloc', ['src', 'GP_ROTATE_270', 'NULL']],
@                 ],
@                 ['resize_nn',
@                  ['resize', ['dst', 'dst', 'GP_INTERP_NN', 'NULL']],
@                  ['resize_alloc', ['src', 'src->w', 'src->h',
@                                   'GP_INTERP_NN', 'NULL']],
@                 ],
@                 ['resize_linear_int',
@                  ['resize', ['dst', 'dst', 'GP_INTERP_LINEAR_INT', 'NULL']],
@                  ['resize_alloc', ['src', 'src->w', 'src->h',
@                                   'GP_INTERP_LINEAR_INT', 'NULL']],
@                 ],
@                 ['resize_linear_lf_int',
@                  ['resize', ['dst', 'dst', 'GP_INTERP_LINEAR_LF_INT', 'NULL']],
@                  ['resize_alloc', ['src', 'src->w', 'src->h',
@                                   'GP_INTERP_LINEAR_LF_INT', 'NULL']],
@                 ],
@                 ['resize_cubic_int',
@                  ['resize', ['dst', 'dst', 'GP_INTERP_CUBIC_INT', 'NULL']],
@                  ['resize_alloc', ['src', 'src->w', 'src->h',
@                                   'GP_INTERP_CUBIC_INT', 'NULL']],
@                 ],
@                 ['resize_cubic',
@                  ['resize', ['dst', 'dst', 'GP_INTERP_CUBIC', 'NULL']],
@                  ['resize_alloc', ['src', 'src->w', 'src->h',
@                                   'GP_INTERP_CUBIC', 'NULL']],
@                 ],
@                 ['laplace',
@                  ['laplace', ['src', 'dst', 'NULL']],
@                  ['laplace_alloc', ['src', 'NULL']],
@                 ],
@                 ['edge_sharpening',
@                  ['edge_sharpening', ['src', 'dst', '0.2', 'NULL']],
@                  ['edge_sharpening_alloc', ['src', '0.2', 'NULL']],
@                 ],
@                 ['median',
@                  ['median', ['src', 'dst', '3', '2', 'NULL']],
@                  ['median_alloc', ['src', '3', '2', 'NULL']],
@                 ],
@                 ['sigma',
@                  ['sigma', ['dst', 'dst', '3', '2', '4', '0.5', 'NULL']],
@                  ['sigma_alloc', ['src', '3', '2', '4', '0.5', 'NULL']],
@                 ],
@                 ['gaussian_blur',
@                  ['gaussian_blur', ['dst', 'dst', '10', '12', 'NULL']],
@                  ['gaussian_blur_alloc', ['src', '10', '12', 'NULL']]
@                 ],
@ ]
@
@ def apply_filter(filter):
@     if 'dst' in filter[1]:
	dst = gp_pixmap_copy(src, GP_COPY_WITH_PIXELS);
	if (gp_filter_{{ filter[0] }}({{ arr_to_params(filter[1]) }})) {
		int err = errno;
		gp_pixmap_free(dst);
		dst = NULL;
		errno = err;
	}

@     else:
	dst = gp_filter_{{ filter[0] }}({{ arr_to_params(filter[1]) }});

@ end
@
@ for fs in compare_list:
static int compare_{{ fs[0] }}(gp_pixel_type pt)
{
	gp_pixmap *src = gp_pixmap_alloc(134, 1072, pt);
	gp_pixmap *dst, *ref;
	int fail = 0;

	/* disable threads, causes memleaks due to glibc tls caching */
	gp_nr_threads_set(1);

	errno = 0;

	if (!src) {
		tst_msg("Malloc Failed :(");
		return TST_UNTESTED;
	}

	/* randomize pixmap content */
	gp_filter_gaussian_noise_add(src, src, 10, 0, NULL);

	/* Create reference result */
{@ apply_filter(fs[1]) @}
	ref = dst;

	if (ref == NULL) {
		gp_pixmap_free(src);
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

	if (!gp_pixmap_equal(ref, dst)) {
		fail++;
		tst_msg("Results for {{ fs[1][0] }} and {{ i[0] }} differs");
	}

	gp_pixmap_free(dst);
@     end

	gp_pixmap_free(src);
	gp_pixmap_free(ref);

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
