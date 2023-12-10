//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2021-2023 Cyril Hrubis <metan@ucw.cz>

 */

#include <errno.h>
#include <string.h>
#include <gfxprim.h>

static int err_dialog(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	gp_dialog_msg_printf_run(GP_DIALOG_MSG_ERR, "Fatal error",
	                         "%s", strerror(ENOSYS));

	return 0;
}

static int info_dialog(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	gp_dialog_msg_run(GP_DIALOG_MSG_INFO, "Info", "Press OK to continue.");

	return 0;
}

static int warn_dialog(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	gp_dialog_msg_run(GP_DIALOG_MSG_WARN, "Warning", "Configuration not loaded!");

	return 0;
}

gp_app_info app_info = {
	.name = "Dialogs",
	.desc = "Message dialogs example",
	.version = "1.0",
	.license = "GPL-2.0-or-later",
	.url = "http://gfxprim.ucw.cz",
	.authors = (gp_app_info_author []) {
		{.name = "Cyril Hrubis", .email = "metan@ucw.cz", .years = "2021-2023"},
		{}
	}
};

static int question_dialog(gp_widget_event *ev)
{
	int ans;

	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	ans = gp_dialog_msg_run(GP_DIALOG_MSG_QUESTION, NULL, "Delete all temporary files?");
	switch (ans) {
	case GP_DIALOG_YES:
		gp_dialog_msg_run(GP_DIALOG_MSG_INFO, "Answer was", "YES");
	break;
	case GP_DIALOG_NO:
		gp_dialog_msg_run(GP_DIALOG_MSG_INFO, "Answer was", "NO");
	break;
	default:
		gp_dialog_msg_run(GP_DIALOG_MSG_WARN, "Internal error", "Question dialog returned internal error!");
	}

	return 0;
}

int main(int argc, char *argv[])
{
	gp_widget *btn1 = gp_widget_button_new2("Error", 0, err_dialog, NULL);
	gp_widget *btn2 = gp_widget_button_new2("Info", 0, info_dialog, NULL);
	gp_widget *btn3 = gp_widget_button_new2("Warning", 0, warn_dialog, NULL);
	gp_widget *btn4 = gp_widget_button_new2("Question", 0, question_dialog, NULL);
	gp_widget *grid = gp_widget_grid_new(4, 1, GP_WIDGET_GRID_UNIFORM);
	gp_widget *layout = gp_widget_grid_new(1, 2, 0);

	btn1->align = GP_HFILL;
	btn2->align = GP_HFILL;
	btn3->align = GP_HFILL;
	btn4->align = GP_HFILL;

	gp_widget_grid_put(grid, 0, 0, btn1);
	gp_widget_grid_put(grid, 1, 0, btn2);
	gp_widget_grid_put(grid, 2, 0, btn3);
	gp_widget_grid_put(grid, 3, 0, btn4);

	gp_widget_grid_put(layout, 0, 0, gp_widget_label_new("Choose dialog", GP_TATTR_LARGE, 0));
	gp_widget_grid_put(layout, 0, 1, grid);

	gp_widgets_main_loop(layout, NULL, argc, argv);

	return 0;
}
