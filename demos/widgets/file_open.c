//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2019 Cyril Hrubis <metan@ucw.cz>

 */

#include <gfxprim.h>

int open_file(gp_widget_event *ev)
{
	gp_widget_dialog *dialog;

	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	dialog = gp_widget_dialog_file_open_new(NULL);

	gp_widget_dialog_run(dialog);

	gp_widget_dialog_free(dialog);

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
	gp_widget *btn1 = gp_widget_button_new("Load File", open_file, NULL);
	gp_widget *btn2 = gp_widget_button_new("Exit", do_exit, NULL);
	gp_widget *grid = gp_widget_grid_new(2, 1);

	gp_widget_grid_put(grid, 0, 0, btn1);
	gp_widget_grid_put(grid, 1, 0, btn2);

	gp_widgets_main_loop(grid, "File Open", NULL, argc, argv);

	return 0;
}
