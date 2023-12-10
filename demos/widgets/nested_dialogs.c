//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2023 Cyril Hrubis <metan@ucw.cz>

 */

#include <gfxprim.h>

static int info_dialog(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	gp_dialog_msg_run(GP_DIALOG_MSG_INFO, "Info", "This is a nested dialog!");

	return 0;
}

static int exit_dialog(gp_widget_event *ev)
{
	gp_dialog *dialog = ev->self->priv;

	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	dialog->retval = 1;

	return 0;
}

static int custom_dialog(gp_widget_event *ev)
{
	gp_dialog dialog = {};

	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	gp_widget *btn1 = gp_widget_button_new2("Run nested", 0, info_dialog, NULL);
	gp_widget *btn2 = gp_widget_button_new2("Exit", 0, exit_dialog, &dialog);
	gp_widget *grid = gp_widget_grid_new(2, 1, 0);

	gp_widget_grid_put(grid, 0, 0, btn1);
	gp_widget_grid_put(grid, 1, 0, btn2);

	gp_widget *frame = gp_widget_frame_new("Custom dialog", 0, grid);
	gp_widget *layout = gp_widget_grid_new(1, 1, 0);

	gp_widget_grid_put(layout, 0, 0, frame);

	dialog.layout = layout;

	gp_dialog_run(&dialog);

	return 0;
}

int do_exit(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	gp_widgets_exit(0);
}

gp_app_info app_info = {
	.name = "Nested Dialogs",
	.desc = "Nested dialogs example application",
	.version = "1.0",
	.license = "GPL-2.0-or-later",
	.url = "http://gfxprim.ucw.cz",
	.authors = (gp_app_info_author []) {
		{.name = "Cyril Hrubis", .email = "metan@ucw.cz", .years = "2023"},
		{}
	}
};

int main(int argc, char *argv[])
{
	gp_widget *btn1 = gp_widget_button_new2("Custom dialog", 0, custom_dialog, NULL);
	gp_widget *btn2 = gp_widget_button_new2("Exit", 0, do_exit, NULL);
	gp_widget *grid = gp_widget_grid_new(2, 1, GP_WIDGET_GRID_UNIFORM);

	btn1->align = GP_HFILL;
	btn2->align = GP_HFILL;

	gp_widget_grid_put(grid, 0, 0, btn1);
	gp_widget_grid_put(grid, 1, 0, btn2);

	gp_widgets_main_loop(grid, NULL, argc, argv);

	return 0;
}
