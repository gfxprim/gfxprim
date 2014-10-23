@ include source.t
/*
 * The purpose of this test is to exercise as much codepaths as possible
 * without checking for result corectness.
 *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdio.h>

#include <core/GP_Context.h>
#include <gfx/GP_Gfx.h>

#include "tst_test.h"

@ API_List = [
@    ['Fill', 'GP_Context:in', 'int:pixel'],
@
@    ['HLine', 'GP_Context:in', 'int:x0', 'int:x1', 'int:y', 'int:pixel'],
@    ['VLine', 'GP_Context:in', 'int:x', 'int:y0', 'int:y1', 'int:pixel'],
@
@    ['Line', 'GP_Context:in', 'int:x0', 'int:y0',
@     'int:x1', 'int:y1', 'int:pixel'],
@
@    ['Circle', 'GP_Context:in', 'int:xcenter', 'int:ycenter',
@     'int:r', 'int:pixel'],
@    ['FillCircle', 'GP_Context:in', 'int:xcenter', 'int:ycenter',
@     'int:r', 'int:pixel'],
@
@    ['Ellipse', 'GP_Context:in', 'int:xcenter', 'int:ycenter',
@     'int:a', 'int:b', 'int:pixel'],
@    ['FillEllipse', 'GP_Context:in', 'int:xcenter', 'int:ycenter',
@     'int:a', 'int:b', 'int:pixel'],
@
@    ['Ring', 'GP_Context:in', 'int:xc', 'int:yc',
@     'int:r1', 'int:r2', 'int:pixel'],
@    ['FillRing', 'GP_Context:in', 'int:xc', 'int:yc',
@     'int:r1', 'int:r2', 'int:pixel'],
@
@    ['Rect', 'GP_Context:in', 'int:x0', 'int:y0',
@     'int:x1', 'int:y1', 'int:pixel'],
@    ['FillRect', 'GP_Context:in', 'int:x0', 'int:y0',
@     'int:x1', 'int:y1', 'int:pixel'],
@
@    ['Triangle', 'GP_Context:in', 'int:x0', 'int:y0',
@     'int:x1', 'int:y1', 'int:x2', 'int:y2', 'int:pixel'],
@    ['FillTriangle', 'GP_Context:in', 'int:x0', 'int:y0',
@     'int:x1', 'int:y1', 'int:x2', 'int:y2', 'int:pixel'],
@
@    ['Tetragon', 'GP_Context:in', 'int:x0', 'int:y0', 'int:x1', 'int:y1',
@     'int:x2', 'int:y2', 'int:x3', 'int:y3', 'int:pixel'],
@    ['FillTetragon', 'GP_Context:in', 'int:x0', 'int:y0', 'int:x1', 'int:y1',
@     'int:x2', 'int:y2', 'int:x3', 'int:y3', 'int:pixel'],
@ ]
@
@ def prep_context(id, pt):
GP_Context *{{ id }} = GP_ContextAlloc(331, 331, GP_PIXEL_{{ pt.name }});
@ end
@
@ def prep_int(id):
int {{ id }} = 2;
@ end
@
@ def prep_param(param, pt):
@     if (param.split(':', 1)[0] == 'GP_Context'):
{@ prep_context(param.split(':', 1)[1], pt) @}
@     if (param.split(':', 1)[0] == 'float'):
{@ prep_float(param.split(':', 1)[1]) @}
@     if (param.split(':', 1)[0] == 'int'):
{@ prep_int(param.split(':', 1)[1]) @}
@ end
@
@ def get_param(param):
@    if len(param.split(':', 1)) == 1:
@         return 'NULL'
@    else:
@         return param.split(':', 1)[1]
@ end
@
@ for fn in API_List:
@     for pt in pixeltypes:
@         if not pt.is_unknown():
static int Gfx_{{ fn[0]}}_{{ pt.name }}(void)
{
@             for param in fn[1:]:
	{@ prep_param(param, pt) @}
@             end

	GP_{{ fn[0] }}({{ ', '.join(map(get_param, fn[1:])) }});

	return TST_SUCCESS;
}

@ end
@
const struct tst_suite tst_suite = {
	.suite_name = "Gfx API Coverage",
	.tests = {
@ for fn in API_List:
@     for pt in pixeltypes:
@         if not pt.is_unknown():
		{.name = "{{ fn[0] }} {{ pt.name }}",
		 .tst_fn = Gfx_{{ fn[0] }}_{{ pt.name }}},
@ end
		{.name = NULL}
	}
};
