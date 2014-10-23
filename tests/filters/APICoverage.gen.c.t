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

#include <core/GP_Context.h>
#include <filters/GP_Filters.h>

#include "tst_test.h"

@ API_List = [
@              ['Brightness', '', 'GP_Context:in', 'GP_Context:out',
@               'float:p', 'GP_ProgressCallback'],
@              ['BrightnessAlloc', '', 'GP_Context:in',
@               'float:p', 'GP_ProgressCallback'],
@
@              ['Contrast', '', 'GP_Context:in', 'GP_Context:out',
@               'float:p', 'GP_ProgressCallback'],
@              ['ContrastAlloc', '', 'GP_Context:in',
@               'float:p', 'GP_ProgressCallback'],
@
@              ['Invert', '', 'GP_Context:in', 'GP_Context:out',
@               'GP_ProgressCallback'],
@              ['InvertAlloc', '', 'GP_Context:in',
@               'GP_ProgressCallback'],
@
@             ['MirrorH', '', 'GP_Context:in', 'GP_Context:out',
@              'GP_ProgressCallback'],
@             ['MirrorHAlloc', '', 'GP_Context:in', 'GP_ProgressCallback'],
@
@             ['MirrorV', '', 'GP_Context:in', 'GP_Context:out',
@              'GP_ProgressCallback'],
@             ['MirrorVAlloc', '', 'GP_Context:in', 'GP_ProgressCallback'],
@
@             ['Rotate90', '', 'GP_Context:in', 'GP_Context:out',
@              'GP_ProgressCallback'],
@             ['Rotate90Alloc', '', 'GP_Context:in', 'GP_ProgressCallback'],
@
@             ['Rotate180', '', 'GP_Context:in', 'GP_Context:out',
@              'GP_ProgressCallback'],
@             ['Rotate180Alloc', '', 'GP_Context:in', 'GP_ProgressCallback'],
@
@             ['Rotate270', '', 'GP_Context:in', 'GP_Context:out',
@              'GP_ProgressCallback'],
@             ['Rotate270Alloc', '', 'GP_Context:in', 'GP_ProgressCallback'],
@
@             ['Symmetry', 'H', 'GP_Context:in', 'GP_Context:out',
@              'GP_MIRROR_H', 'GP_ProgressCallback'],
@             ['SymmetryAlloc', 'H', 'GP_Context:in', 'GP_MIRROR_H',
@              'GP_ProgressCallback'],
@
@             ['Symmetry', 'V', 'GP_Context:in', 'GP_Context:out',
@              'GP_MIRROR_V', 'GP_ProgressCallback'],
@             ['SymmetryAlloc', 'V', 'GP_Context:in', 'GP_MIRROR_V',
@              'GP_ProgressCallback'],
@
@             ['Symmetry', '90', 'GP_Context:in', 'GP_Context:out',
@              'GP_ROTATE_90', 'GP_ProgressCallback'],
@             ['SymmetryAlloc', '90', 'GP_Context:in', 'GP_ROTATE_90',
@              'GP_ProgressCallback'],
@
@             ['Symmetry', '180', 'GP_Context:in', 'GP_Context:out',
@              'GP_ROTATE_180', 'GP_ProgressCallback'],
@             ['SymmetryAlloc', '180', 'GP_Context:in', 'GP_ROTATE_180',
@              'GP_ProgressCallback'],
@
@             ['Symmetry', '270', 'GP_Context:in', 'GP_Context:out',
@              'GP_ROTATE_270', 'GP_ProgressCallback'],
@             ['SymmetryAlloc', '270', 'GP_Context:in', 'GP_ROTATE_270',
@              'GP_ProgressCallback'],
@
@             ['Convolution', '', 'GP_Context:in', 'GP_Context:out',
@              'GP_FilterKernel2D:kernel', 'GP_ProgressCallback'],
@             ['ConvolutionAlloc', '', 'GP_Context:in',
@              'GP_FilterKernel2D:kernel', 'GP_ProgressCallback'],
@
@             ['GaussianBlur', '', 'GP_Context:in', 'GP_Context:out',
@              'float:sigma_x', 'float:sigma_y', 'GP_ProgressCallback'],
@             ['GaussianBlurAlloc', '', 'GP_Context:in', 'float:sigma_x',
@              'float:sigma_y', 'GP_ProgressCallback'],
@
@             ['GaussianNoiseAdd', '', 'GP_Context:in', 'GP_Context:out',
@              'float:sigma', 'float:mu', 'GP_ProgressCallback'],
@             ['GaussianNoiseAddAlloc', '', 'GP_Context:in',
@              'float:sigma', 'float:mu', 'GP_ProgressCallback'],
@
@             ['Median', '', 'GP_Context:in', 'GP_Context:out',
@              'int:xmed', 'int:ymed', 'GP_ProgressCallback'],
@             ['MedianAlloc', '', 'GP_Context:in',
@              'int:xmed', 'int:ymed', 'GP_ProgressCallback'],
@
@             ['WeightedMedian', '', 'GP_Context:in', 'GP_Context:out',
@              'GP_MedianWeights:weights', 'GP_ProgressCallback'],
@             ['WeightedMedianAlloc', '', 'GP_Context:in',
@              'GP_MedianWeights:weights', 'GP_ProgressCallback'],
@
@             ['Sigma', '', 'GP_Context:in', 'GP_Context:out',
@              'int:xrad', 'int:yrad', 'int:min', 'float:sigma',
@              'GP_ProgressCallback'],
@             ['SigmaAlloc', '', 'GP_Context:in',
@              'int:xrad', 'int:yrad', 'int:min', 'float:sigma',
@              'GP_ProgressCallback'],
@
@              ['ResizeNN', '', 'GP_Context:in', 'GP_Context:out',
@               'GP_ProgressCallback'],
@              ['ResizeNNAlloc', '', 'GP_Context:in', 'int:w', 'int:h',
@               'GP_ProgressCallback'],
@
@              ['FloydSteinberg', '', 'GP_Context:in', 'GP_Context:out',
@		'GP_ProgressCallback'],
@              ['FloydSteinbergAlloc', '', 'GP_Context:in', 'GP_PixelType:G8',
@               'GP_ProgressCallback'],
@
@              ['HilbertPeano', '', 'GP_Context:in', 'GP_Context:out',
@               'GP_ProgressCallback'],
@              ['HilbertPeanoAlloc', '', 'GP_Context:in', 'GP_PixelType:G8',
@               'GP_ProgressCallback'],
@ ]
@
@ def prep_context(id):
	GP_Context *{{ id }} = GP_ContextAlloc(331, 331, pt);
@
@ def prep_float(id):
	float {{ id }} = 1;
@
@ def prep_int(id):
	int {{ id }} = 2;
@
@ def prep_pixel_type(id):
	GP_PixelType {{ id }} = GP_PIXEL_{{ id }};
@
@ def prep_median_weights(id):

	unsigned int {{ id }}_w[] = {
		1, 2, 1,
		2, 4, 2,
		1, 2, 1,
	};

	GP_MedianWeights {{ id }}_s = {
		.w = 3,
		.h = 3,
		.weights = {{ id }}_w,
	};

	GP_MedianWeights *{{ id }} = &{{ id }}_s;
@
@ def prep_filter_kernel_2d(id):
	float {{ id }}_kern[] = {
		1, 1, 1,
		1, 1, 1,
		1, 1, 1,
	};

	GP_FilterKernel2D {{ id }}_s = {
		.w = 3,
		.h = 3,
		.div = 9,
		.kernel = {{ id }}_kern,
	};

	GP_FilterKernel2D *{{ id }} = &{{ id }}_s;
@
@
@ def prep_param(param):
@     if (param.split(':', 1)[0] == 'GP_Context'):
{@ prep_context(param.split(':', 1)[1]) @}
@     if (param.split(':', 1)[0] == 'float'):
{@ prep_float(param.split(':', 1)[1]) @}
@     if (param.split(':', 1)[0] == 'int'):
{@ prep_int(param.split(':', 1)[1]) @}
@     if (param.split(':', 1)[0] == 'GP_MedianWeights'):
{@ prep_median_weights(param.split(':', 1)[1]) @}
@     if (param.split(':', 1)[0] == 'GP_FilterKernel2D'):
{@ prep_filter_kernel_2d(param.split(':', 1)[1]) @}
@     if (param.split(':', 1)[0] == 'GP_PixelType'):
{@ prep_pixel_type(param.split(':', 1)[1]) @}
@
@ def do_param(param):
@     if param == 'GP_ProgressCallback':
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
static int filter_{{ fn[0] }}_{{ fn[1] }}(GP_PixelType pt)
{
@     for param in fn[1:]:
{@ prep_param(param) @}
@     if 'Alloc' in fn[0]:
	GP_Context *res;
@     else:
	int res;
@     end

	res = GP_Filter{{ fn[0] }}({{ gen_params(fn[2:]) }});

@     if 'Alloc' in fn[0]:
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
