//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2022 Cyril Hrubis <metan@ucw.cz>

 */

#include <stdio.h>
#include <widgets/gp_widgets.h>

#include "dialog_app_info.json.h"

static gp_app_info *app_info;

void gp_app_info_set(gp_app_info *app_info_)
{
	app_info = app_info_;
}

static void format_authors(gp_app_info_author *authors)
{
	size_t i;

	printf("\n");

	for (i = 0; authors[i].name; i++) {
		printf("Copyright (C) ");

		if (authors[i].years)
			printf("%s ", authors[i].years);

		printf("%s", authors[i].name);

		if (authors[i].email)
			printf(" <%s>", authors[i].email);

		printf("\n");
	}
}

void gp_app_info_print(void)
{
	if (!app_info) {
		printf("app_info not passed!\n");
		return;
	}

	printf("%s %s\n", app_info->name, app_info->version);

	if (app_info->desc)
		printf(" %s\n", app_info->desc);

	if (app_info->url)
		printf("\n%s\n", app_info->url);

	if (app_info->authors)
		format_authors(app_info->authors);

	if (app_info->license)
		printf("\nLicensed under %s\n", app_info->license);
}

static int do_ok(gp_widget_event *ev)
{
	gp_dialog *wd = ev->self->priv;

	if (ev->type == GP_WIDGET_EVENT_WIDGET)
		wd->retval = 1;

	return 0;
}

static const gp_widget_json_addr addrs[] = {
	{.id = "ok", .on_event = do_ok},
	{}
};

static gp_widget *authors(gp_app_info_author *authors)
{
	gp_widget *ret;
	size_t i;

	ret = gp_widget_grid_new(1, 0, 0);
	if (!ret)
		return NULL;

	for (i = 0; authors[i].name; i++) {
		gp_widget *author = gp_widget_label_new("\u00a9 ", 0, 0);

		if (authors[i].years) {
			gp_widget_label_append(author, authors[i].years);
			gp_widget_label_append(author, " ");
		}

		gp_widget_label_append(author, authors[i].name);

		if (authors[i].email) {
			gp_widget_label_append(author, " <");
			gp_widget_label_append(author, authors[i].email);
			gp_widget_label_append(author, ">");
		}

		gp_widget_grid_row_append(ret);
		gp_widget_grid_put(ret, 0, i, author);
	}

	gp_widget_grid_border_set(ret, GP_WIDGET_BORDER_ALL, 0, 0);

	return ret;
}

void gp_app_info_dialog_run(void)
{
	gp_htable *uids;
	gp_dialog dialog = {};
	gp_widget *w;

	if (!app_info) {
		gp_dialog_msg_run(GP_DIALOG_MSG_WARN, NULL, "Application info not set!");
		return;
	}

	gp_widget_json_callbacks callbacks = {
		.default_priv = &dialog,
		.addrs = addrs
	};

	dialog.layout = gp_dialog_layout_load("about", &callbacks, dialog_app_info, &uids);
	if (!dialog.layout)
		return;

	w = gp_widget_by_uid(uids, "app_name", GP_WIDGET_LABEL);
	if (w)
		gp_widget_label_set(w, app_info->name);

	w = gp_widget_by_uid(uids, "app_version", GP_WIDGET_LABEL);
	if (w)
		gp_widget_label_printf(w, "Ver: %s", app_info->version);

	w = gp_widget_by_uid(uids, "app_desc", GP_WIDGET_LABEL);
	if (w)
		gp_widget_label_set(w, app_info->desc);

	w = gp_widget_by_uid(uids, "app_url", GP_WIDGET_LABEL);
	if (w)
		gp_widget_label_set(w, app_info->url);

	w = gp_widget_by_uid(uids, "app_license", GP_WIDGET_LABEL);
	if (w)
		gp_widget_label_printf(w, "License: %s", app_info->license);

	w = gp_widget_by_uid(uids, "app_info", GP_WIDGET_GRID);
	if (w) {
		if (app_info->authors) {
			gp_widget_grid_row_append(w);
			gp_widget_grid_put(w, 0, w->grid->rows-1, authors(app_info->authors));
		}
	}

	gp_htable_free(uids);

	gp_dialog_run(&dialog);

	gp_widget_free(dialog.layout);
}
