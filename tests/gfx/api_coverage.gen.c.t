@ include source.t
/*
 * The purpose of this test is to exercise as much codepaths as possible
 * without checking for result corectness.
 *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdio.h>

#include <core/gp_pixmap.h>
#include <gfx/gp_gfx.h>

#include "common.h"
#include "tst_test.h"

@ API_List = [
@    ['fill', 'gp_pixmap:in', 'int:pixel'],
@
@    ['hline', 'gp_pixmap:in', 'int:x0', 'int:x1', 'int:y', 'int:pixel'],
@    ['vline', 'gp_pixmap:in', 'int:x', 'int:y0', 'int:y1', 'int:pixel'],
@
@    ['line', 'gp_pixmap:in', 'int:x0', 'int:y0',
@     'int:x1', 'int:y1', 'int:pixel'],
@
@    ['circle', 'gp_pixmap:in', 'int:xcenter', 'int:ycenter',
@     'int:r', 'int:pixel'],
@    ['fill_circle', 'gp_pixmap:in', 'int:xcenter', 'int:ycenter',
@     'int:r', 'int:pixel'],
@
@    ['ellipse', 'gp_pixmap:in', 'int:xcenter', 'int:ycenter',
@     'int:a', 'int:b', 'int:pixel'],
@    ['fill_ellipse', 'gp_pixmap:in', 'int:xcenter', 'int:ycenter',
@     'int:a', 'int:b', 'int:pixel'],
@
@    ['ring', 'gp_pixmap:in', 'int:xc', 'int:yc',
@     'int:r1', 'int:r2', 'int:pixel'],
@    ['fill_ring', 'gp_pixmap:in', 'int:xc', 'int:yc',
@     'int:r1', 'int:r2', 'int:pixel'],
@
@    ['rect', 'gp_pixmap:in', 'int:x0', 'int:y0',
@     'int:x1', 'int:y1', 'int:pixel'],
@    ['fill_rect', 'gp_pixmap:in', 'int:x0', 'int:y0',
@     'int:x1', 'int:y1', 'int:pixel'],
@
@    ['triangle', 'gp_pixmap:in', 'int:x0', 'int:y0',
@     'int:x1', 'int:y1', 'int:x2', 'int:y2', 'int:pixel'],
@    ['fill_triangle', 'gp_pixmap:in', 'int:x0', 'int:y0',
@     'int:x1', 'int:y1', 'int:x2', 'int:y2', 'int:pixel'],
@
@    ['tetragon', 'gp_pixmap:in', 'int:x0', 'int:y0', 'int:x1', 'int:y1',
@     'int:x2', 'int:y2', 'int:x3', 'int:y3', 'int:pixel'],
@    ['fill_tetragon', 'gp_pixmap:in', 'int:x0', 'int:y0', 'int:x1', 'int:y1',
@     'int:x2', 'int:y2', 'int:x3', 'int:y3', 'int:pixel'],
@ ]
@
@ def prep_pixmap(id, pt):
gp_pixmap *{{ id }} = pixmap_alloc_canary(331, 331, GP_PIXEL_{{ pt.name }});
@ end
@
@ def prep_int(id):
int {{ id }} = 2;
@ end
@
@ def prep_param(param, pt):
@     if (param.split(':', 1)[0] == 'gp_pixmap'):
{@ prep_pixmap(param.split(':', 1)[1], pt) @}
@     if (param.split(':', 1)[0] == 'float'):
{@ prep_float(param.split(':', 1)[1]) @}
@     if (param.split(':', 1)[0] == 'int'):
{@ prep_int(param.split(':', 1)[1]) @}
@ end
@
@ def check_canary(param):
@     if (param.split(':', 1)[0] == 'gp_pixmap'):
if (check_canary({{ param.split(':', 1)[1] }}))
	return TST_FAILED;
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
static int gfx_{{ fn[0]}}_{{ pt.name }}(void)
{
@             for param in fn[1:]:
	{@ prep_param(param, pt) @}
@             end

	gp_{{ fn[0] }}({{ ', '.join(map(get_param, fn[1:])) }});

@             for param in fn[1:]:
	{@ check_canary(param) @}
@             end

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
		 .tst_fn = gfx_{{ fn[0] }}_{{ pt.name }}},
@ end
		{.name = NULL}
	}
};
