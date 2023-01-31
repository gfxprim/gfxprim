//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2019 Cyril Hrubis <metan@ucw.cz>

 */

#include <gfxprim.h>

int open_file(gp_widget_event *ev)
{
	gp_dialog *dialog;

	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	gp_dialog_file_opts opts = {
		.flags = GP_DIALOG_OPEN_FILE,
	};

	dialog = gp_dialog_file_open_new(NULL, &opts);

	if (gp_dialog_run(dialog) == GP_WIDGET_DIALOG_PATH)
		printf("Selected file/path '%s'\n", gp_dialog_file_path(dialog));

	gp_dialog_free(dialog);

	return 0;
}

int save_file(gp_widget_event *ev)
{
	gp_dialog *dialog;

	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	dialog = gp_dialog_file_save_new(NULL, NULL);

	if (gp_dialog_run(dialog) == GP_WIDGET_DIALOG_PATH)
		printf("Selected file '%s'\n", gp_dialog_file_path(dialog));

	gp_dialog_free(dialog);

	return 0;
}

int do_exit(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	gp_widgets_exit(0);
}

int main(int argc, char *argv[])
{
	gp_widget *btn_open = gp_widget_button_new2("Load File", 0, open_file, NULL);
	gp_widget *btn_save = gp_widget_button_new2("Save File", 0, save_file, NULL);
	gp_widget *btn_exit = gp_widget_button_new2("Exit", 0, do_exit, NULL);
	gp_widget *grid = gp_widget_grid_new(3, 1, 0);

	gp_widget_grid_put(grid, 0, 0, btn_open);
	gp_widget_grid_put(grid, 1, 0, btn_save);
	gp_widget_grid_put(grid, 2, 0, btn_exit);

	gp_widgets_main_loop(grid, "File Open Save", NULL, argc, argv);

	return 0;
}
