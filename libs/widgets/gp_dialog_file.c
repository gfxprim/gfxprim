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
#include <fcntl.h>
#include <errno.h>

#include <utils/gp_vec_str.h>

#include <widgets/gp_widgets.h>
#include <widgets/gp_string.h>
#include <widgets/gp_dir_cache.h>
#include <widgets/gp_date_time.h>
#include <widgets/gp_file_size.h>
#include <widgets/gp_dialog.h>
#include <widgets/gp_dialog_file.h>

#include "dialog_file_open.json.h"
#include "dialog_file_save.json.h"

struct file_dialog {
	gp_widget *show_hidden;
	gp_widget *filter;
	gp_widget *dir_path;
	gp_widget *filename;
	gp_widget *file_table;
};

static int redraw_table(gp_widget_event *ev)
{
	struct file_dialog *dialog = ev->self->priv;

	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	gp_widget_redraw(dialog->file_table);

	return 0;
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

static int files_seek_row(gp_widget *self, int op, unsigned int pos)
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
	case GP_TABLE_ROW_MAX:
		return -1;
	}

	if (self->tbl->row_idx < cache->used)
		return 1;

	return 0;
}

enum file_attr {
	FILE_NAME,
	FILE_SIZE,
	FILE_MOD_TIME,
};

static int files_get_cell(gp_widget *self, gp_widget_table_cell *cell, unsigned int col)
{
	static char buf[100];
	gp_dir_cache *cache = self->tbl->priv;

	gp_dir_entry *ent = gp_dir_cache_get(cache, self->tbl->row_idx);

	if (!ent)
		return 0;

	switch (col) {
	case FILE_NAME:
		cell->text = ent->name;
		cell->tattr = GP_TATTR_LEFT;
	break;
	case FILE_SIZE:
		cell->text = gp_str_file_size(buf, sizeof(buf), ent->size);
		cell->tattr = GP_TATTR_RIGHT | GP_TATTR_MONO;
	break;
	case FILE_MOD_TIME:
		cell->text = gp_str_time_diff(buf, sizeof(buf), ent->mtime, time(NULL));
		cell->tattr = GP_TATTR_LEFT;
	break;
	}

	return 1;
}

static void files_sort(gp_widget *self, int desc, unsigned int col)
{
	int sort_type = 0;

	switch (col) {
	case FILE_NAME:
		sort_type = GP_DIR_SORT_BY_NAME;
	break;
	case FILE_SIZE:
		sort_type = GP_DIR_SORT_BY_SIZE;
	break;
	case FILE_MOD_TIME:
		sort_type = GP_DIR_SORT_BY_MTIME;
	break;
	}

	if (desc)
		sort_type |= GP_DIR_SORT_DESC;
	else
		sort_type |= GP_DIR_SORT_ASC;

	gp_dir_cache_sort(self->tbl->priv, sort_type);
}

static void exit_dialog(struct file_dialog *dialog, int retval)
{
	gp_dialog *wd = GP_CONTAINER_OF(dialog, gp_dialog, payload);
	gp_dir_cache *cache = dialog->file_table->tbl->priv;
	gp_widget *table = dialog->file_table;
	gp_widget *path = dialog->dir_path;

	/* Append selected entry to the directory -> file_open */
	if (!dialog->filename && table->tbl->row_selected) {
		gp_dir_entry *entry = gp_dir_cache_get_filtered(cache, table->tbl->selected_row);

		gp_widget_tbox_append(path, "/");
		gp_widget_tbox_append(path, entry->name);
	}

	/* Append filename textbox -> file_save */
	if (dialog->filename) {
		gp_widget_tbox_append(path, "/");
		gp_widget_tbox_append(path, gp_widget_tbox_text(dialog->filename));
	}

	free_dir_cache(cache);

	wd->retval = retval;
}

static int open_on_event(gp_widget_event *ev)
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

static int save_on_event(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	if (ev->sub_type)
		return 0;

	//TODO: Disable the save button when filename is empty?
	exit_dialog(ev->self->priv, GP_WIDGET_DIALOG_PATH);

	return 0;
}

static int cancel_on_event(gp_widget_event *ev)
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
	if (dialog->filter)
		gp_widget_tbox_clear(dialog->filter);
	gp_widget_table_off_set(dialog->file_table, 0);
}

static void set_filename(struct file_dialog *dialog, gp_widget_event *ev)
{
	gp_dir_cache *cache = dialog->file_table->tbl->priv;

	if (!dialog->filename)
		return;

	gp_dir_entry *ent = gp_dir_cache_get_filtered(cache, ev->val);

	if (ent->is_dir)
		return;

	gp_widget_tbox_set(dialog->filename, ent->name);
}

static int table_on_event(gp_widget_event *ev)
{
	struct file_dialog *dialog = ev->self->priv;

	switch (ev->type) {
	case GP_WIDGET_EVENT_WIDGET:
		switch (ev->sub_type) {
		case GP_WIDGET_TABLE_TRIGGER:
			table_event(ev->self);
		break;
		case GP_WIDGET_TABLE_SELECT:
			set_filename(dialog, ev);
		break;
		}
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

const gp_widget_table_col_ops gp_dialog_files_col_ops = {
	.get_cell = files_get_cell,
	.sort = files_sort,
	.seek_row = files_seek_row,
	.col_map = {
		{.id = "name", .idx = FILE_NAME, .sortable = 1},
		{.id = "size", .idx = FILE_SIZE, .sortable = 1},
		{.id = "mod_time", .idx = FILE_MOD_TIME, .sortable = 1},
		{}
	}
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

static int new_dir_on_event(gp_widget_event *ev)
{
	struct file_dialog *dialog = ev->self->priv;
	gp_dir_cache *cache = dialog->file_table->tbl->priv;
	char *dir_name;
	unsigned int pos;

	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	dir_name = gp_dialog_input_run("Enter directory name");
	if (!dir_name)
		return 0;

	if (mkdirat(cache->dirfd, dir_name, 0755)) {
		gp_dialog_msg_printf_run(GP_DIALOG_MSG_ERR,
		                        "Failed to create directory",
		                        "%s", strerror(errno));
		goto exit;
	}

	gp_dir_cache_new_dir(cache, dir_name);

	pos = gp_dir_cache_pos_by_name_filtered(cache, dir_name);

	gp_widget_table_off_set(dialog->file_table, pos);
	gp_widget_table_sel_set(dialog->file_table, pos);

	gp_widget_redraw(dialog->file_table);

	//TODO: focus the table here

exit:
	free(dir_name);

	return 0;
}

gp_dialog *gp_dialog_file_save_new(const char *path,
                                   const char *const ext_hints[])
{
	gp_htable *uids = NULL;
	gp_widget *layout, *w;
	gp_dialog *ret;
	struct file_dialog *dialog;

	layout = gp_dialog_layout_load("file_save", dialog_file_save, &uids);
	if (!layout)
		return NULL;

	ret = gp_dialog_new(sizeof(struct file_dialog));
	if (!ret)
		goto err0;

	ret->layout = layout;
	ret->input_event = file_open_input_event;

	dialog = (void*)ret->payload;

	dialog->show_hidden = gp_widget_by_uid(uids, "hidden", GP_WIDGET_CHECKBOX);
	dialog->filename = gp_widget_by_uid(uids, "filename", GP_WIDGET_TBOX);
	dialog->dir_path = gp_widget_by_uid(uids, "path", GP_WIDGET_TBOX);
	dialog->file_table = gp_widget_by_uid(uids, "files", GP_WIDGET_TABLE);

	gp_widget_event_handler_set(dialog->file_table, table_on_event, dialog);
	gp_widget_event_unmask(dialog->file_table, GP_WIDGET_EVENT_INPUT);

	w = gp_widget_by_uid(uids, "save", GP_WIDGET_BUTTON);
	if (w)
		gp_widget_event_handler_set(w, save_on_event, dialog);

	w = gp_widget_by_uid(uids, "cancel", GP_WIDGET_BUTTON);
	if (w)
		gp_widget_event_handler_set(w, cancel_on_event, dialog);

	w = gp_widget_by_uid(uids, "new_dir", GP_WIDGET_BUTTON);
	if (w)
		gp_widget_event_handler_set(w, new_dir_on_event, dialog);

	gp_htable_free(uids);

	if (!dialog->dir_path) {
		GP_WARN("Missing path widget!");
		goto err1;
	}

	if (dialog->show_hidden)
		gp_widget_event_handler_set(dialog->show_hidden, redraw_table, dialog);

	if (dialog->filename)
		gp_widget_event_handler_set(dialog->filename, save_on_event, dialog);

	gp_widget_tbox_printf(dialog->dir_path, "%s", get_path(path));

	return ret;
err1:
	free(ret);
err0:
	gp_widget_free(layout);
	return NULL;
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
	dialog->file_table = gp_widget_by_uid(uids, "files", GP_WIDGET_TABLE);

	if (!dialog->file_table) {
		GP_WARN("No file table defined!");
		goto err1;
	}

	gp_widget_event_handler_set(dialog->file_table, table_on_event, dialog);
	gp_widget_event_unmask(dialog->file_table, GP_WIDGET_EVENT_INPUT);

	w = gp_widget_by_uid(uids, "open", GP_WIDGET_BUTTON);
	if (w)
		gp_widget_event_handler_set(w, open_on_event, dialog);

	w = gp_widget_by_uid(uids, "cancel", GP_WIDGET_BUTTON);
	if (w)
		gp_widget_event_handler_set(w, cancel_on_event, dialog);

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

	return ret;
err1:
	free(ret);
err0:
	gp_widget_free(layout);
	return NULL;
}

const char *gp_dialog_file_path(gp_dialog *self)
{
	struct file_dialog *dialog = (void*)self->payload;

	return gp_widget_tbox_text(dialog->dir_path);
}
