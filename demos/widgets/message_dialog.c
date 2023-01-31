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
	gp_widget *grid = gp_widget_grid_new(4, 1, 0);

	gp_widget_grid_put(grid, 0, 0, btn1);
	gp_widget_grid_put(grid, 1, 0, btn2);
	gp_widget_grid_put(grid, 2, 0, btn3);
	gp_widget_grid_put(grid, 3, 0, btn4);

	gp_widgets_main_loop(grid, "Message dialogs", NULL, argc, argv);

	return 0;
}
