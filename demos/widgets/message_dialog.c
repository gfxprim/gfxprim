//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#include <errno.h>
#include <string.h>
#include <gfxprim.h>

static int err_dialog(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	gp_dialog_msg_printf_run(GP_DIALOG_MSG_ERR,
	                         "Failed to save file 'pict.bmp': %s",
	                         strerror(ENOSYS));

	return 0;
}

static int info_dialog(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	gp_dialog_msg_run(GP_DIALOG_MSG_INFO, "Press Ok to continue...");

	return 0;
}

static int warn_dialog(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	gp_dialog_msg_run(GP_DIALOG_MSG_WARN, "This is a last warning!");

	return 0;
}

int main(int argc, char *argv[])
{
	gp_widget *btn1 = gp_widget_button_new("Error dialog", 0, err_dialog, NULL);
	gp_widget *btn2 = gp_widget_button_new("Info dialog", 0, info_dialog, NULL);
	gp_widget *btn3 = gp_widget_button_new("Warning dialog", 0, warn_dialog, NULL);
	gp_widget *grid = gp_widget_grid_new(3, 1, 0);

	gp_widget_grid_put(grid, 0, 0, btn1);
	gp_widget_grid_put(grid, 1, 0, btn2);
	gp_widget_grid_put(grid, 2, 0, btn3);

	gp_widgets_main_loop(grid, "Message dialogs", NULL, argc, argv);

	return 0;
}
