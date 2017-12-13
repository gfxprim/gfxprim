@ include source.t
/*
 * Filters API coverage tests.
 *
 * The purpose of this test is to exercise as much codepaths as possible
 * without checking for result corectness.
 *
 * We check for correct return value correcness though.
 *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <core/GP_Pixmap.h>
#include <filters/GP_Filters.h>

#include "tst_test.h"

@ API_List = [
@              ['brightness', '', 'gp_pixmap:in', 'gp_pixmap:out',
@               'float:p', 'gp_progress_cb'],
@              ['brightness_alloc', '', 'gp_pixmap:in',
@               'float:p', 'gp_progress_cb'],
@
@              ['contrast', '', 'gp_pixmap:in', 'gp_pixmap:out',
@               'float:p', 'gp_progress_cb'],
@              ['contrast_alloc', '', 'gp_pixmap:in',
@               'float:p', 'gp_progress_cb'],
@
@              ['invert', '', 'gp_pixmap:in', 'gp_pixmap:out',
@               'gp_progress_cb'],
@              ['invert_alloc', '', 'gp_pixmap:in',
@               'gp_progress_cb'],
@
@             ['mirror_h', '', 'gp_pixmap:in', 'gp_pixmap:out',
@              'gp_progress_cb'],
@             ['mirror_h_alloc', '', 'gp_pixmap:in', 'gp_progress_cb'],
@
@             ['mirror_v', '', 'gp_pixmap:in', 'gp_pixmap:out',
@              'gp_progress_cb'],
@             ['mirror_v_alloc', '', 'gp_pixmap:in', 'gp_progress_cb'],
@
@             ['rotate_90', '', 'gp_pixmap:in', 'gp_pixmap:out',
@              'gp_progress_cb'],
@             ['rotate_90_alloc', '', 'gp_pixmap:in', 'gp_progress_cb'],
@
@             ['rotate_180', '', 'gp_pixmap:in', 'gp_pixmap:out',
@              'gp_progress_cb'],
@             ['rotate_180_alloc', '', 'gp_pixmap:in', 'gp_progress_cb'],
@
@             ['rotate_270', '', 'gp_pixmap:in', 'gp_pixmap:out',
@              'gp_progress_cb'],
@             ['rotate_270_alloc', '', 'gp_pixmap:in', 'gp_progress_cb'],
@
@             ['symmetry', 'H', 'gp_pixmap:in', 'gp_pixmap:out',
@              'GP_MIRROR_H', 'gp_progress_cb'],
@             ['symmetry_alloc', 'H', 'gp_pixmap:in', 'GP_MIRROR_H',
@              'gp_progress_cb'],
@
@             ['symmetry', 'V', 'gp_pixmap:in', 'gp_pixmap:out',
@              'GP_MIRROR_V', 'gp_progress_cb'],
@             ['symmetry_alloc', 'V', 'gp_pixmap:in', 'GP_MIRROR_V',
@              'gp_progress_cb'],
@
@             ['symmetry', '90', 'gp_pixmap:in', 'gp_pixmap:out',
@              'GP_ROTATE_90', 'gp_progress_cb'],
@             ['symmetry_alloc', '90', 'gp_pixmap:in', 'GP_ROTATE_90',
@              'gp_progress_cb'],
@
@             ['symmetry', '180', 'gp_pixmap:in', 'gp_pixmap:out',
@              'GP_ROTATE_180', 'gp_progress_cb'],
@             ['symmetry_alloc', '180', 'gp_pixmap:in', 'GP_ROTATE_180',
@              'gp_progress_cb'],
@
@             ['symmetry', '270', 'gp_pixmap:in', 'gp_pixmap:out',
@              'GP_ROTATE_270', 'gp_progress_cb'],
@             ['symmetry_alloc', '270', 'gp_pixmap:in', 'GP_ROTATE_270',
@              'gp_progress_cb'],
@
@             ['convolution', '', 'gp_pixmap:in', 'gp_pixmap:out',
@              'gp_filter_kernel_2d:kernel', 'gp_progress_cb'],
@             ['convolution_alloc', '', 'gp_pixmap:in',
@              'gp_filter_kernel_2d:kernel', 'gp_progress_cb'],
@
@             ['gaussian_blur', '', 'gp_pixmap:in', 'gp_pixmap:out',
@              'float:sigma_x', 'float:sigma_y', 'gp_progress_cb'],
@             ['gaussian_blur_alloc', '', 'gp_pixmap:in', 'float:sigma_x',
@              'float:sigma_y', 'gp_progress_cb'],
@
@             ['gaussian_noise_add', '', 'gp_pixmap:in', 'gp_pixmap:out',
@              'float:sigma', 'float:mu', 'gp_progress_cb'],
@             ['gaussian_noise_add_alloc', '', 'gp_pixmap:in',
@              'float:sigma', 'float:mu', 'gp_progress_cb'],
@
@             ['median', '', 'gp_pixmap:in', 'gp_pixmap:out',
@              'int:xmed', 'int:ymed', 'gp_progress_cb'],
@             ['median_alloc', '', 'gp_pixmap:in',
@              'int:xmed', 'int:ymed', 'gp_progress_cb'],
@
@             ['weighted_median', '', 'gp_pixmap:in', 'gp_pixmap:out',
@              'gp_median_weights:weights', 'gp_progress_cb'],
@             ['weighted_median_alloc', '', 'gp_pixmap:in',
@              'gp_median_weights:weights', 'gp_progress_cb'],
@
@             ['sigma', '', 'gp_pixmap:in', 'gp_pixmap:out',
@              'int:xrad', 'int:yrad', 'int:min', 'float:sigma',
@              'gp_progress_cb'],
@             ['sigma_alloc', '', 'gp_pixmap:in',
@              'int:xrad', 'int:yrad', 'int:min', 'float:sigma',
@              'gp_progress_cb'],
@
@              ['resize_nn', '', 'gp_pixmap:in', 'gp_pixmap:out',
@               'gp_progress_cb'],
@              ['resize_nn_alloc', '', 'gp_pixmap:in', 'int:w', 'int:h',
@               'gp_progress_cb'],
@
@              ['floyd_steinberg', '', 'gp_pixmap:in', 'gp_pixmap:out',
@		'gp_progress_cb'],
@              ['floyd_steinberg_alloc', '', 'gp_pixmap:in', 'gp_pixel_type:G8',
@               'gp_progress_cb'],
@
@              ['hilbert_peano', '', 'gp_pixmap:in', 'gp_pixmap:out',
@               'gp_progress_cb'],
@              ['hilbert_peano_alloc', '', 'gp_pixmap:in', 'gp_pixel_type:G8',
@               'gp_progress_cb'],
@ ]
@
@ def prep_pixmap(id):
	gp_pixmap *{{ id }} = gp_pixmap_alloc(331, 331, pt);
@
@ def prep_float(id):
	float {{ id }} = 1;
@
@ def prep_int(id):
	int {{ id }} = 2;
@
@ def prep_pixel_type(id):
	gp_pixel_type {{ id }} = GP_PIXEL_{{ id }};
@
@ def prep_median_weights(id):

	unsigned int {{ id }}_w[] = {
		1, 2, 1,
		2, 4, 2,
		1, 2, 1,
	};

	gp_median_weights {{ id }}_s = {
		.w = 3,
		.h = 3,
		.weights = {{ id }}_w,
	};

	gp_median_weights *{{ id }} = &{{ id }}_s;
@
@ def prep_filter_kernel_2d(id):
	float {{ id }}_kern[] = {
		1, 1, 1,
		1, 1, 1,
		1, 1, 1,
	};

	gp_filter_kernel_2d {{ id }}_s = {
		.w = 3,
		.h = 3,
		.div = 9,
		.kernel = {{ id }}_kern,
	};

	gp_filter_kernel_2d *{{ id }} = &{{ id }}_s;
@
@
@ def prep_param(param):
@     if (param.split(':', 1)[0] == 'gp_pixmap'):
{@ prep_pixmap(param.split(':', 1)[1]) @}
@     if (param.split(':', 1)[0] == 'float'):
{@ prep_float(param.split(':', 1)[1]) @}
@     if (param.split(':', 1)[0] == 'int'):
{@ prep_int(param.split(':', 1)[1]) @}
@     if (param.split(':', 1)[0] == 'gp_median_weights'):
{@ prep_median_weights(param.split(':', 1)[1]) @}
@     if (param.split(':', 1)[0] == 'gp_filter_kernel_2d'):
{@ prep_filter_kernel_2d(param.split(':', 1)[1]) @}
@     if (param.split(':', 1)[0] == 'gp_pixel_type'):
{@ prep_pixel_type(param.split(':', 1)[1]) @}
@
@ def do_param(param):
@     if param == 'gp_progress_cb':
@         return 'NULL'
@     else:
@         return param
@
@ def get_param(param):
@     if len(param.split(':', 1)) == 1:
@         return do_param(param)
@     else:
@         return param.split(':', 1)[1]
@
@ def gen_params(params):
@     res = map(get_param, params)
@     return ', '.join(res)
@
@ for fn in API_List:
static int filter_{{ fn[0] }}_{{ fn[1] }}(gp_pixel_type pt)
{
@     for param in fn[1:]:
{@ prep_param(param) @}
@     if 'alloc' in fn[0]:
	gp_pixmap *res;
@     else:
	int res;
@     end

	res = gp_filter_{{ fn[0] }}({{ gen_params(fn[2:]) }});

@     if 'alloc' in fn[0]:
	if (res == NULL) {
@     else:
	if (res) {
@     end
		switch (errno) {
		case ENOSYS:
			tst_msg("Not Implemented");
			return TST_SUCCESS;
		break;
		case EINVAL:
			tst_msg("Not suitable pixel type");
			return TST_SUCCESS;
		break;
		default:
			tst_msg("Unexpected errno %s", strerror(errno));
			return TST_FAILED;
		}
	}

	return TST_SUCCESS;
}
@ end
@
@ for fn in API_List:
@     for pt in pixeltypes:
@         if not pt.is_unknown():
static int filter_{{ fn[0] }}_{{ fn[1] }}_{{ pt.name }}(void)
{
	return filter_{{ fn[0] }}_{{ fn[1] }}({{ pt.C_type }});
}
@ end

const struct tst_suite tst_suite = {
	.suite_name = "Filters API Coverage",
	.tests = {
@ for fn in API_List:
@     for pt in pixeltypes:
@         if not pt.is_unknown():
		{.name = "Filter {{ fn[0] }} {{ fn[1] }} {{ pt.name }}",
		 .tst_fn = filter_{{ fn[0] }}_{{ fn[1] }}_{{ pt.name }}},
@ end
		{.name = NULL}
	}
};
