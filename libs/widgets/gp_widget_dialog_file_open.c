//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <time.h>

#include <utils/gp_vec_str.h>

#include <widgets/gp_widgets.h>
#include <widgets/gp_string.h>
#include <widgets/gp_dir_cache.h>
#include <widgets/gp_date_time.h>
#include <widgets/gp_file_size.h>
#include <widgets/gp_dialog.h>
#include <widgets/gp_dialog_file.h>

#include "dialog_file_open.json.h"

struct file_dialog {
	gp_widget *show_hidden;
	gp_widget *filter;
	gp_widget *dir_path;
	gp_widget *file_table;

	char *file_path;
};

static int redraw_table(gp_widget_event *ev)
{
	struct file_dialog *dialog = ev->self->priv;

	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	gp_widget_redraw(dialog->file_table);

	return 0;
}

static void sort_file_table(gp_widget *self, unsigned int col, int desc)
{
	int sort_type = 0;

	switch (col) {
	case 0:
		sort_type = GP_DIR_SORT_BY_NAME;
	break;
	case 1:
		sort_type = GP_DIR_SORT_BY_SIZE;
	break;
	case 2:
		sort_type = GP_DIR_SORT_BY_MTIME;
	break;
	}

	if (desc)
		sort_type |= GP_DIR_SORT_DESC;
	else
		sort_type |= GP_DIR_SORT_ASC;

	gp_dir_cache_sort(self->tbl->priv, sort_type);
}

static int dialog_show_hidden(struct file_dialog *dialog)
{
	if (!dialog->show_hidden)
		return 0;

	return dialog->show_hidden->checkbox->val;
}

static char *dialog_filter(struct file_dialog *dialog)
{
	if (!dialog->filter)
		return "";

	return dialog->filter->tbox->buf;
}

static int find_next(gp_widget *self)
{
	gp_widget_table *tbl = self->tbl;
	gp_dir_cache *cache = tbl->priv;
	int show_hidden = dialog_show_hidden(self->priv);
	char *str = dialog_filter(self->priv);
	size_t str_len = strlen(str);
	gp_dir_entry *entry;

	for (;;) {
		if (tbl->row_idx >= cache->used)
			return 0;

		entry = gp_dir_cache_get(cache, tbl->row_idx);

		if (str_len) {
			if (strstr(entry->name, str)) {
				gp_dir_cache_set_filter(cache, tbl->row_idx, 0);
				return 1;
			} else {
				goto next;
			}
		}

		if (show_hidden || (entry->name[0] != '.' || entry->name[1] == '.')) {
			gp_dir_cache_set_filter(cache, tbl->row_idx, 0);
			return 1;
		}
next:
		gp_dir_cache_set_filter(cache, tbl->row_idx, 1);
		tbl->row_idx++;
	}
}

static int notify_callback(struct gp_fd *self, struct pollfd *pfd)
{
	(void) pfd;
	struct file_dialog *dialog = self->priv;

	if (gp_dir_cache_inotify(dialog->file_table->tbl->priv))
		gp_widget_redraw(dialog->file_table);

	return 0;
}

static gp_dir_cache *load_dir_cache(struct file_dialog *dialog)
{
	gp_dir_cache *cache = gp_dir_cache_new(dialog->dir_path->tbox->buf);

	if (cache->inotify_fd > 0)
		gp_fds_add(gp_widgets_fds, cache->inotify_fd, POLLIN, notify_callback, dialog);

	return cache;
}

static void free_dir_cache(gp_dir_cache *self)
{
	if (self->inotify_fd > 0)
		gp_fds_rem(gp_widgets_fds, self->inotify_fd);

	gp_dir_cache_free(self);
}

static int set_row(gp_widget *self, int op, unsigned int pos)
{
	gp_dir_cache *cache = self->tbl->priv;
	unsigned int i;

	if (!cache)
		cache = self->tbl->priv = load_dir_cache(self->priv);

	switch (op) {
	case GP_TABLE_ROW_RESET:
		self->tbl->row_idx = 0;
		find_next(self);
	break;
	case GP_TABLE_ROW_ADVANCE:
		for (i = 0; i < pos; i++) {
			self->tbl->row_idx++;
			if (!find_next(self))
				return 0;
		}
	break;
	case GP_TABLE_ROW_TELL:
		return -1;
	}

	if (self->tbl->row_idx < cache->used)
		return 1;

	return 0;
}

static gp_widget_table_cell *get_elem(gp_widget *self, unsigned int col)
{
	static char buf[100];
	static gp_widget_table_cell cell = {
		.text = "",
	};

	gp_dir_cache *cache = self->tbl->priv;

	gp_dir_entry *ent = gp_dir_cache_get(cache, self->tbl->row_idx);

	if (!ent)
		return &cell;

	switch (col) {
	case 0:
		cell.text = ent->name;
		cell.tattr = GP_TATTR_LEFT;
	break;
	case 1:
		cell.text = gp_str_file_size(buf, sizeof(buf), ent->size);
		cell.tattr = GP_TATTR_RIGHT | GP_TATTR_MONO;
	break;
	case 2:
		cell.text = gp_str_time_diff(buf, sizeof(buf), ent->mtime, time(NULL));
		cell.tattr = GP_TATTR_LEFT;
	break;
	}

	return &cell;
}

static void exit_dialog(struct file_dialog *dialog, int retval)
{
	gp_dialog *wd = GP_CONTAINER_OF(dialog, gp_dialog, payload);
	gp_dir_cache *cache = dialog->file_table->tbl->priv;
	gp_widget *table = dialog->file_table;

	if (table->tbl->row_selected) {
		const char *dir = dialog->dir_path->tbox->buf;
		gp_dir_entry *entry = gp_dir_cache_get_filtered(cache, table->tbl->selected_row);
		dialog->file_path = gp_vec_printf(dialog->file_path, "%s/%s", dir, entry->name);
	}

	free_dir_cache(cache);

	wd->retval = retval;
}

static int do_open(gp_widget_event *ev)
{
	struct file_dialog *dialog = ev->self->priv;

	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	//TODO: Disable the open button?
	if (!dialog->file_table->tbl->row_selected)
		return 0;

	exit_dialog(ev->self->priv, GP_WIDGET_DIALOG_PATH);

	return 0;
}

static int do_cancel(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	exit_dialog(ev->self->priv, GP_WIDGET_DIALOG_CANCEL);

	return 0;
}

static void table_event(gp_widget *self)
{
	struct gp_widget_table *tbl = self->tbl;
	struct file_dialog *dialog = self->priv;

	gp_dir_entry *entry = gp_dir_cache_get_filtered(tbl->priv, tbl->selected_row);

	if (!entry) {
		GP_BUG("Empty entry!");
		return;
	}

	if (!entry->is_dir) {
		exit_dialog(dialog, GP_WIDGET_DIALOG_PATH);
		return;
	}

	char *dpath = gp_aprintf("%s/%s", dialog->dir_path->tbox->buf, entry->name);
	char *dir = realpath(dpath, NULL);

	free(dpath);

	if (access(dir, X_OK)) {
		free(dir);
		return;
	}

	gp_widget_tbox_printf(dialog->dir_path, "%s", dir);

	free(dir);

	free_dir_cache(dialog->file_table->tbl->priv);
	dialog->file_table->tbl->priv = load_dir_cache(self->priv);
	gp_widget_tbox_clear(dialog->filter);
	gp_widget_table_set_offset(dialog->file_table, 0);
}

static int table_on_event(gp_widget_event *ev)
{
	struct file_dialog *dialog = ev->self->priv;

	switch (ev->type) {
	case GP_WIDGET_EVENT_WIDGET:
		table_event(ev->self);
		return 0;
	case GP_WIDGET_EVENT_INPUT:
		if (ev->input_ev->type == GP_EV_KEY &&
		    ev->input_ev->val == GP_KEY_ESC &&
		    ev->input_ev->code == GP_EV_KEY_DOWN) {

			if (gp_widget_tbox_is_empty(dialog->filter))
				return 0;

			gp_widget_tbox_clear(dialog->filter);
			gp_widget_redraw(ev->self);
			return 1;
		}

		return 0;
	default:
		return 0;
	}
}

static const char *get_path(const char *path)
{
	if (path)
		return path;

	path = getenv("HOME");
	if (path)
		return path;

	return ".";
}

static const gp_widget_table_header header[] = {
	{.label = "File", .sortable = 1, .col_min_size = 20, .col_fill = 1},
	{.label = "Size", .sortable = 1, .col_min_size = 7},
	{.label = "Modified", .sortable = 1, .col_min_size = 7},
};

static int file_open_input_event(gp_dialog *self, gp_event *ev)
{
	struct file_dialog *dialog = (void*)self->payload;

	if (ev->type == GP_EV_KEY &&
	    ev->val == GP_KEY_ESC &&
	    ev->code == GP_EV_KEY_DOWN) {
		exit_dialog(dialog, GP_WIDGET_DIALOG_CANCEL);
		return 1;
	}

	if (ev->type == GP_EV_KEY)
		return gp_widget_ops_event(dialog->filter, gp_widgets_render_ctx(), ev);

	return 0;
}

gp_dialog *gp_dialog_file_open_new(const char *path)
{
	gp_htable *uids = NULL;
	gp_widget *layout, *w;
	gp_dialog *ret;
	struct file_dialog *dialog;

	layout = gp_dialog_layout_load("file_open", dialog_file_open, &uids);
	if (!layout)
		return NULL;

	ret = gp_dialog_new(sizeof(struct file_dialog));
	if (!ret)
		goto err0;

	ret->layout = layout;
	ret->input_event = file_open_input_event;

	dialog = (void*)ret->payload;

	dialog->show_hidden = gp_widget_by_uid(uids, "hidden", GP_WIDGET_CHECKBOX);
	dialog->filter = gp_widget_by_uid(uids, "filter", GP_WIDGET_TBOX);
	dialog->dir_path = gp_widget_by_uid(uids, "path", GP_WIDGET_TBOX);

	w = gp_widget_by_uid(uids, "open", GP_WIDGET_BUTTON);
	if (w)
		gp_widget_event_handler_set(w, do_open, dialog);

	w = gp_widget_by_uid(uids, "cancel", GP_WIDGET_BUTTON);
	if (w)
		gp_widget_event_handler_set(w, do_cancel, dialog);

	gp_htable_free(uids);

	if (!dialog->dir_path) {
		GP_WARN("Missing path widget!");
		goto err1;
	}

	if (dialog->show_hidden)
		gp_widget_event_handler_set(dialog->show_hidden, redraw_table, dialog);

	if (dialog->filter)
		gp_widget_event_handler_set(dialog->filter, redraw_table, dialog);

	gp_widget_tbox_printf(dialog->dir_path, "%s", get_path(path));

	gp_widget *table = gp_widget_table_new(3, 25, header, set_row, get_elem);
	if (!table)
		goto err1;

	dialog->file_table = table;

	//TODO: Move to JSON!
	table->align = GP_FILL;
	table->tbl->priv = NULL;

	table->tbl->sort = sort_file_table;

	gp_widget_event_handler_set(table, table_on_event, dialog);

	gp_widget_event_unmask(table, GP_WIDGET_EVENT_INPUT);
        gp_widget_grid_put(layout, 0, 1, table);

	return ret;
err1:
	free(ret);
err0:
	gp_widget_free(layout);
	return NULL;
}

const char *gp_dialog_file_open_path(gp_dialog *self)
{
	struct file_dialog *dialog = (void*)self->payload;

	return dialog->file_path;
}
