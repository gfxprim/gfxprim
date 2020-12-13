@ include source.t
@ include savers.t
/*
 * Iterate over all pixel types, try to save pixmap but abort it from callback.
 *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include <core/gp_pixmap.h>
#include <loaders/gp_loaders.h>

#include "tst_test.h"

typedef int (*Save)(const gp_pixmap *src, const char *path, gp_progress_cb *callback);

static int progress_callback(gp_progress_cb *self)
{
	(void) self;
	return 1;
}

static int test(Save Saver, gp_pixel_type pixel_type)
{
	gp_pixmap *src;
	gp_progress_cb callback ={.callback = progress_callback};
	int ret = TST_SUCCESS;

	src = gp_pixmap_alloc(100, 100, pixel_type);

	if (!src) {
		tst_msg("Malloc failed");
		return TST_UNTESTED;
	}

	if (Saver(src, "testfile", &callback)) {
		if (errno == ENOSYS) {
			tst_msg("Unimplemented pixel value");
			ret = TST_SKIPPED;
			goto err;
		}

		if (errno == EINVAL) {
			tst_msg("Invalid pixel value for the format");
			ret = TST_SKIPPED;
			goto err;
		}

		if (errno == ECANCELED) {
			if (access("testfile", F_OK) == 0) {
				tst_msg("Operation canceled but file exists");
				ret = TST_FAILED;
				goto err;
			} else {
				goto err;
			}
		}

		tst_msg("Saver failed with %s", strerror(errno));
		ret = TST_FAILED;
		goto err;
	} else {
		tst_msg("Succedded unexpectedly");
		ret = TST_FAILED;
	}

err:
	gp_pixmap_free(src);
	return ret;
}

@ for fmt in fmts:
@     for pt in pixeltypes:
@         if not pt.is_unknown():
static int test_{{ fmt }}_{{ pt.name }}(void)
{
	return test(gp_save_{{ fmt }}, GP_PIXEL_{{ pt.name }});
}

@ end
@
const struct tst_suite tst_suite = {
	.suite_name = "SaveAbort",
	.tests = {
@ for fmt in fmts:
@     for pt in pixeltypes:
@         if not pt.is_unknown():
		{.name = "{{ fmt }} {{ pt.name }}",
		 .tst_fn = test_{{ fmt }}_{{ pt.name }},
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},
@ end
		{.name = NULL},
	}
};
