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

	dialog = gp_dialog_file_open_new(NULL);

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
	gp_widget *btn1 = gp_widget_button_new("Load File", 0, open_file, NULL);
	gp_widget *btn2 = gp_widget_button_new("Save File", 0, save_file, NULL);
	gp_widget *btn3 = gp_widget_button_new("Exit", 0, do_exit, NULL);
	gp_widget *grid = gp_widget_grid_new(3, 1, 0);

	gp_widget_grid_put(grid, 0, 0, btn1);
	gp_widget_grid_put(grid, 1, 0, btn2);
	gp_widget_grid_put(grid, 2, 0, btn3);

	gp_widgets_main_loop(grid, "File Open Save", NULL, argc, argv);

	return 0;
}
