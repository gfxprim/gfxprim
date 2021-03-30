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
	gp_dialog *wd = ev->self->priv;

	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	wd->retval = 1;

	return 0;
}

static gp_widget *load_layout(enum gp_dialog_msg_type type,
                              gp_dialog *dialog, gp_widget **label)
{
	gp_widget *ret, *w;
	void *uids = NULL;

	switch (type) {
	case GP_DIALOG_MSG_INFO:
		ret = gp_dialog_layout_load("info", dialog_info, &uids);
	break;
	case GP_DIALOG_MSG_WARN:
		ret = gp_dialog_layout_load("warn", dialog_warn, &uids);
	break;
	case GP_DIALOG_MSG_ERR:
		ret = gp_dialog_layout_load("err", dialog_err, &uids);
	break;
	default:
		GP_WARN("Invalid dialog type %i", type);
		return NULL;
	}

	w = gp_widget_by_uid(uids, "btn_ok", GP_WIDGET_BUTTON);
	if (w)
		gp_widget_event_handler_set(w, do_exit, dialog);

	*label = gp_widget_by_uid(uids, "text", GP_WIDGET_LABEL);

	gp_htable_free(uids);

	return ret;
}

static int run_and_free(gp_dialog *dialog)
{
	long ret;

	ret = gp_dialog_run(dialog);
	gp_widget_free(dialog->layout);

	return ret;
}

int gp_dialog_msg_run(enum gp_dialog_msg_type type, const char *msg)
{
	gp_widget *label = NULL;
	gp_dialog dialog = {};

	dialog.layout = load_layout(type, &dialog, &label);
	if (!dialog.layout)
		return GP_DIALOG_ERR;

	if (label)
		gp_widget_label_set(label, msg);

	return run_and_free(&dialog);
}

int gp_dialog_msg_printf_run(enum gp_dialog_msg_type type, const char *fmt, ...)
{
	va_list ap;
	gp_widget *label = NULL;
	gp_dialog dialog = {};

	dialog.layout = load_layout(type, &dialog, &label);
	if (!dialog.layout)
		return GP_DIALOG_ERR;

	va_start(ap, fmt);
	if (label)
		gp_widget_label_vprintf(label, fmt, ap);
	va_end(ap);

	return run_and_free(&dialog);
}
