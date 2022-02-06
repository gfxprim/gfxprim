//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2022 Cyril Hrubis <metan@ucw.cz>

 */

#include <widgets/gp_widgets.h>

#include "dialog_input.json.h"

static int ok_on_event(gp_widget_event *ev)
{
	gp_dialog *dialog = ev->self->priv;

	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	dialog->retval = GP_DIALOG_YES;

	return 0;
}

static int cancel_on_event(gp_widget_event *ev)
{
	gp_dialog *dialog = ev->self->priv;

	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	dialog->retval = GP_DIALOG_NO;

	return 0;
}

static int input_on_event(gp_widget_event *ev)
{
	gp_dialog *dialog = ev->self->priv;

	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	if (ev->sub_type == GP_WIDGET_TBOX_TRIGGER)
		dialog->retval = GP_DIALOG_YES;

	return 0;
}

static const gp_widget_json_addr addrs[] = {
	{.id = "cancel", .on_event = cancel_on_event},
	{.id = "input", .on_event = input_on_event},
	{.id = "ok", .on_event = ok_on_event},
	{}
};

static gp_widget *load_layout(const char *title, gp_dialog *dialog, gp_widget **input)
{
	gp_widget *ret, *w;
	gp_htable *uids = NULL;

	gp_widget_json_callbacks callbacks = {
		.default_priv = dialog,
		.addrs = addrs,
	};

	ret = gp_dialog_layout_load("text_input", &callbacks, dialog_input, &uids);
	if (!ret)
		return NULL;

	if (title) {
		w = gp_widget_by_uid(uids, "title", GP_WIDGET_FRAME);
		gp_widget_frame_title_set(w, title);
	}

	*input = gp_widget_by_uid(uids, "input", GP_WIDGET_TBOX);

	gp_htable_free(uids);

	return ret;
}

char *gp_dialog_input_run(const char *title)
{
	gp_dialog dialog = {};
	gp_widget *input = NULL;
	char *ret = NULL;

	dialog.layout = load_layout(title, &dialog, &input);
	if (!dialog.layout)
		return NULL;

	if (!input) {
		GP_WARN("Missing input tbox!");
		goto ret;
	}

	if (gp_dialog_run(&dialog) == GP_DIALOG_YES)
		ret = strdup(gp_widget_tbox_text(input));

ret:
	gp_widget_free(dialog.layout);
	return ret;
}
