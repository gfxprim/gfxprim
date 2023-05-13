// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2022 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <widgets/gp_widgets.h>
#include "tst_test.h"
#include "common.h"

static int frame_new_free(void)
{
	const char *text = "Label text";
	gp_widget *label, *frame;

	label = gp_widget_label_new(text, 0, 0);
	if (!label) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	frame = gp_widget_frame_new("frame", 0, label);
	if (!frame) {
		gp_widget_free(label);
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_free(frame);

	return TST_PASSED;
}

const struct tst_suite tst_suite = {
	.suite_name = "label testsuite",
	.tests = {
		{.name = "frame new free",
		 .tst_fn = frame_new_free,
		 .flags = TST_CHECK_MALLOC},

		{},
	}
};
