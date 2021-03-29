//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#include <widgets/gp_widgets.h>

#include "dialog_info.json.h"
#include "dialog_warn.json.h"
#include "dialog_err.json.h"

static int do_exit(gp_widget_event *ev)
{
	gp_widget_dialog *wd = ev->self->priv;

	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	wd->dialog_exit = 1;

	return 0;
}

static gp_widget_dialog *dialog_msg_new(enum gp_dialog_msg_type type, gp_widget **label)
{
	gp_widget_dialog *ret;
	gp_widget *w;
	void *uids = NULL;

	ret = gp_widget_dialog_new(0);
	if (!ret)
		return NULL;

	switch (type) {
	case GP_DIALOG_MSG_INFO:
		ret->layout = gp_dialog_layout_load("info", dialog_info, &uids);
	break;
	case GP_DIALOG_MSG_WARN:
		ret->layout = gp_dialog_layout_load("warn", dialog_warn, &uids);
	break;
	case GP_DIALOG_MSG_ERR:
		ret->layout = gp_dialog_layout_load("err", dialog_err, &uids);
	break;
	default:
		GP_WARN("Invalid dialog type %i", type);
		gp_widget_dialog_free(ret);
		return NULL;
	}

	w = gp_widget_by_uid(uids, "btn_ok", GP_WIDGET_BUTTON);
	if (w)
		gp_widget_event_handler_set(w, do_exit, ret);

	*label = gp_widget_by_uid(uids, "text", GP_WIDGET_LABEL);

	gp_htable_free(uids);

	return ret;
}

gp_widget_dialog *gp_dialog_msg_new(enum gp_dialog_msg_type type,
                                    const char *msg)
{
	gp_widget *label = NULL;
	gp_widget_dialog *dialog = dialog_msg_new(type, &label);

	if (label)
		gp_widget_label_set(label, msg);

	return dialog;
}

static int dialog_run_free(gp_widget_dialog *dialog)
{
	int ret = gp_widget_dialog_run(dialog);

	gp_widget_dialog_free(dialog);

	return ret;
}

int gp_dialog_msg_run(enum gp_dialog_msg_type type, const char *msg)
{
	gp_widget_dialog *dialog = gp_dialog_msg_new(type, msg);

	if (!dialog)
		return GP_DIALOG_ERR;

	return dialog_run_free(dialog);
}

int gp_dialog_msg_printf_run(enum gp_dialog_msg_type type, const char *fmt, ...)
{
	va_list ap;
	gp_widget *label = NULL;
	gp_widget_dialog *dialog = dialog_msg_new(type, &label);

	if (!dialog)
		return GP_DIALOG_ERR;

	va_start(ap, fmt);
	if (label)
		gp_widget_label_vprintf(label, fmt, ap);
	va_end(ap);

	return dialog_run_free(dialog);
}
