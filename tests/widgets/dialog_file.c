// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2022 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <widgets/gp_widgets.h>
#include "tst_test.h"
#include "common.h"

static int file_save_new_free(void)
{
	gp_dialog *file_save;

	file_save = gp_dialog_file_save_new(NULL, NULL);
	if (!file_save) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_dialog_free(file_save);

	return TST_SUCCESS;
}

static int file_open_new_free(void)
{
	gp_dialog *file_open;

	file_open = gp_dialog_file_open_new(NULL, NULL);
	if (!file_open) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_dialog_free(file_open);

	return TST_SUCCESS;
}

static int file_open(void)
{
	gp_dialog *file_open;

	file_open = gp_dialog_file_open_new(".", NULL);
	if (!file_open) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	dummy_render(file_open->layout);

	send_dialog_keypress(file_open, GP_KEY_DOWN);
	send_dialog_keypress(file_open, GP_KEY_DOWN);
	state_press(GP_KEY_LEFT_CTRL);
	send_dialog_keypress(file_open, GP_KEY_O);

	if (file_open->retval != GP_WIDGET_DIALOG_PATH) {
		tst_msg("Wrong dialog exit value (%li)", file_open->retval);
		return TST_FAILED;
	}

	const char *path = gp_dialog_file_path(file_open);

	if (!path) {
		tst_msg("No path set");
		return TST_FAILED;
	}

	if (!strcmp(path, "../")) {
		tst_msg("Wrong path '%s' selected", path);
		return TST_FAILED;
	}

	gp_dialog_free(file_open);

	return TST_SUCCESS;
}

static int file_save(void)
{
	gp_dialog *file_save;

	file_save = gp_dialog_file_save_new(".", NULL);
	if (!file_save) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	dummy_render(file_save->layout);

	type_string(file_save->layout, "test");
	state_press(GP_KEY_LEFT_CTRL);
	send_dialog_keypress(file_save, GP_KEY_S);

	if (file_save->retval != GP_WIDGET_DIALOG_PATH) {
		tst_msg("Wrong dialog exit value (%li)", file_save->retval);
		return TST_FAILED;
	}

	const char *path = gp_dialog_file_path(file_save);

	if (!path) {
		tst_msg("No path set");
		return TST_FAILED;
	}

	if (!strcmp(path, "test")) {
		tst_msg("Wrong path '%s' selected", path);
		return TST_FAILED;
	}

	gp_dialog_free(file_save);

	return TST_SUCCESS;
}

const struct tst_suite tst_suite = {
	.suite_name = "file dialogs testsuite",
	.tests = {
		{.name = "file save new free",
		 .tst_fn = file_save_new_free,
		 .flags = TST_CHECK_MALLOC},

		{.name = "file open new free",
		 .tst_fn = file_open_new_free,
		 .flags = TST_CHECK_MALLOC},

		{.name = "file open",
		 .tst_fn = file_open,
		 .flags = TST_TMPDIR},

		{.name = "file save",
		 .tst_fn = file_save,
		 .flags = TST_TMPDIR},

		{.name = NULL},
	}
};
