//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2023 Cyril Hrubis <metan@ucw.cz>

 */

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

#include <utils/gp_vec_str.h>

#include <widgets/gp_widgets.h>
#include <widgets/gp_string.h>
#include <widgets/gp_dir_cache.h>
#include <widgets/gp_date_time.h>
#include <widgets/gp_file_size.h>
#include <widgets/gp_dialog.h>
#include <widgets/gp_dialog_file.h>
#include <widgets/gp_app_poll.h>

#include "dialog_file_open.json.h"
#include "dialog_file_save.json.h"

struct file_dialog {
	gp_widget *show_hidden;
	gp_widget *filter;
	gp_widget *dir_path;
	gp_widget *filename;
	gp_widget *file_table;
	gp_widget *open_save_btn;

	const gp_dialog_file_opts *opts;
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

	return gp_widget_bool_get(dialog->show_hidden);
}

static const char *dialog_filter(struct file_dialog *dialog)
{
	if (!dialog->filter)
		return "";

	return gp_widget_tbox_text(dialog->filter);
}

static int find_next(gp_widget *self)
{
	gp_widget_table_priv *tbl_priv = gp_widget_table_priv_get(self);
	gp_dir_cache *cache = tbl_priv->priv;
	int show_hidden = dialog_show_hidden(self->priv);
	const char *str = dialog_filter(self->priv);
	size_t str_len = strlen(str);
	gp_dir_entry *entry;

	for (;;) {
		int filter = 0;

		if (tbl_priv->row_idx >= cache->used)
			return 0;

		entry = gp_dir_cache_get(cache, tbl_priv->row_idx);

		if ((str_len) && !strstr(entry->name, str))
			filter = 1;

		if (!show_hidden && (entry->name[0] == '.' && entry->name[1] != '.'))
			filter = 1;

		if (filter) {
			gp_dir_cache_set_filter(cache, tbl_priv->row_idx, 1);
			tbl_priv->row_idx++;
		} else {
			gp_dir_cache_set_filter(cache, tbl_priv->row_idx, 0);
			return 1;
		}
	}
}

static enum gp_poll_event_ret notify_callback(gp_fd *self)
{
	struct file_dialog *dialog = self->priv;
	gp_widget_table_priv *tbl_priv = gp_widget_table_priv_get(dialog->file_table);

	if (gp_dir_cache_notify(tbl_priv->priv))
		gp_widget_redraw(dialog->file_table);

	return 0;
}

static gp_dir_cache *load_dir_cache(struct file_dialog *dialog)
{
	gp_dir_cache *cache;
	gp_fd *notify_fd;

	cache = gp_dir_cache_new(gp_widget_tbox_text(dialog->dir_path));
	if (!cache)
		return NULL;

	notify_fd = gp_dir_cache_notify_fd(cache);
	if (notify_fd) {
		notify_fd->event = notify_callback;
		notify_fd->priv = dialog;
		gp_app_poll_add(notify_fd);
	}

	return cache;
}

static void free_dir_cache(struct file_dialog *dialog)
{
	gp_fd *notify_fd;
	gp_widget_table_priv *tbl_priv = gp_widget_table_priv_get(dialog->file_table);
	gp_dir_cache *self = tbl_priv->priv;

	if (!self)
		return;

	notify_fd = gp_dir_cache_notify_fd(self);
	if (notify_fd)
		gp_app_poll_rem(notify_fd);

	gp_dir_cache_destroy(self);

	tbl_priv->priv = NULL;
}

static int files_seek_row(gp_widget *self, int op, unsigned int pos)
{
	gp_widget_table_priv *tbl_priv = gp_widget_table_priv_get(self);
	gp_dir_cache *cache = tbl_priv->priv;
	unsigned int i;

	if (!cache)
		cache = tbl_priv->priv = load_dir_cache(self->priv);

	if (!cache)
		return 0;

	switch (op) {
	case GP_TABLE_ROW_RESET:
		tbl_priv->row_idx = 0;
		find_next(self);
	break;
	case GP_TABLE_ROW_ADVANCE:
		for (i = 0; i < pos; i++) {
			tbl_priv->row_idx++;
			if (!find_next(self))
				return 0;
		}
	break;
	case GP_TABLE_ROW_MAX:
		return -1;
	}

	if (tbl_priv->row_idx < cache->used)
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
	gp_widget_table_priv *tbl_priv = gp_widget_table_priv_get(self);
	gp_dir_cache *cache = tbl_priv->priv;

	gp_dir_entry *ent = gp_dir_cache_get(cache, tbl_priv->row_idx);

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
	gp_widget_table_priv *tbl_priv = gp_widget_table_priv_get(self);
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

	gp_dir_cache_sort(tbl_priv->priv, sort_type);
}

static void exit_dialog(struct file_dialog *dialog, int retval)
{
	gp_dialog *wd = GP_CONTAINER_OF(dialog, gp_dialog, payload);
	gp_widget_table_priv *tbl_priv = gp_widget_table_priv_get(dialog->file_table);
	gp_dir_cache *cache = tbl_priv->priv;
	gp_widget *table = dialog->file_table;
	gp_widget *path = dialog->dir_path;

	if (!cache)
		retval = GP_WIDGET_DIALOG_CANCEL;

	if (retval == GP_WIDGET_DIALOG_CANCEL)
		goto exit;

	/* Append selected entry to the directory -> file_open */
	if (!dialog->filename && gp_widget_table_sel_has(table)) {
		gp_dir_entry *entry = gp_dir_cache_get_filtered(cache, gp_widget_table_sel_get(table));

		gp_widget_tbox_append(path, "/");
		gp_widget_tbox_append(path, entry->name);
	}

	/* Append filename textbox -> file_save */
	if (dialog->filename) {
		enum gp_dir_cache_type type;
		const char *name = gp_widget_tbox_text(dialog->filename);
		int retval;

		type = gp_dir_cache_lookup(cache, name);

		switch (type) {
		case GP_DIR_CACHE_FILE:
			retval = gp_dialog_msg_printf_run(GP_DIALOG_MSG_QUESTION,
			                                  "File already exists",
			                                  "Overwrite '%s'?", name);

			if (retval != GP_DIALOG_YES)
				return;
		break;
		case GP_DIR_CACHE_DIR:
			gp_dialog_msg_printf_run(GP_DIALOG_MSG_WARN,
			                         "Directory exists",
			                         "Directory '%s' already exits!", name);
			return;
		break;
		case GP_DIR_CACHE_NONE:
		break;
		}

		gp_widget_tbox_append(path, "/");
		gp_widget_tbox_append(path, gp_widget_tbox_text(dialog->filename));
	}

exit:
	free_dir_cache(dialog);
	wd->retval = retval;
}

static int cannot_open(struct file_dialog *dialog)
{
	gp_widget_table_priv *tbl_priv = gp_widget_table_priv_get(dialog->file_table);
	gp_dir_cache *cache = tbl_priv->priv;
	gp_widget *table = dialog->file_table;

	if (!cache)
		return 0;

	if (!gp_widget_table_sel_has(table))
		return 1;

	if (dialog->opts) {
		gp_dir_entry *entry = gp_dir_cache_get_filtered(cache, gp_widget_table_sel_get(table));

		if (!entry)
			return 1;

		if (entry->is_dir && !(dialog->opts->flags & GP_DIALOG_OPEN_DIR))
			return 1;

		if (!entry->is_dir && !(dialog->opts->flags & GP_DIALOG_OPEN_FILE))
			return 1;
	}

	return 0;
}

static void try_open(struct file_dialog *dialog)
{
	if (cannot_open(dialog))
		return;

	exit_dialog(dialog, GP_WIDGET_DIALOG_PATH);
}

static int open_on_event(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	try_open(ev->self->priv);

	return 0;
}

static void try_save(struct file_dialog *dialog)
{
	if (!gp_widget_tbox_is_empty(dialog->filename))
		exit_dialog(dialog, GP_WIDGET_DIALOG_PATH);
}

static int save_on_event(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	if (ev->sub_type)
		return 0;

	try_save(ev->self->priv);

	return 0;
}

static int cancel_on_event(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	exit_dialog(ev->self->priv, GP_WIDGET_DIALOG_CANCEL);

	return 0;
}

static void set_path(struct file_dialog *dialog, const char *path)
{
	if (access(path, X_OK))
		return;

	gp_widget_table_priv *tbl_priv = gp_widget_table_priv_get(dialog->file_table);

	gp_widget_tbox_printf(dialog->dir_path, "%s", path);

	free_dir_cache(dialog);
	tbl_priv->priv = load_dir_cache(dialog);
	if (dialog->filter)
		gp_widget_tbox_clear(dialog->filter);
	gp_widget_table_off_set(dialog->file_table, 0);
}

static void table_event(gp_widget *self)
{
	struct file_dialog *dialog = self->priv;
	gp_widget_table_priv *tbl_priv = gp_widget_table_priv_get(self);
	gp_dir_cache *cache = tbl_priv->priv;

	if (!cache)
		return;

	gp_dir_entry *entry = gp_dir_cache_get_filtered(cache, gp_widget_table_sel_get(self));

	if (!entry) {
		GP_BUG("Empty entry!");
		return;
	}

	if (!entry->is_dir) {
		exit_dialog(dialog, GP_WIDGET_DIALOG_PATH);
		return;
	}

	char *dpath = gp_aprintf("%s/%s", gp_widget_tbox_text(dialog->dir_path), entry->name);
	char *dir = realpath(dpath, NULL);

	free(dpath);

	set_path(dialog, dir);

	free(dir);
}

static inline int is_file_open(struct file_dialog *dialog)
{
	return !dialog->filename;
}

static void set_filename(struct file_dialog *dialog, gp_widget_event *ev)
{
	gp_widget_table_priv *tbl_priv = gp_widget_table_priv_get(dialog->file_table);
	gp_dir_cache *cache = tbl_priv->priv;

	if (!cache)
		return;

	gp_dir_entry *ent = gp_dir_cache_get_filtered(cache, ev->val);

	if (ent->is_dir)
		return;

	gp_widget_tbox_set(dialog->filename, ent->name);
}

static void enable_disable_open_btn(struct file_dialog *dialog)
{
	gp_widget_disabled_set(dialog->open_save_btn, cannot_open(dialog));
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
			if (is_file_open(dialog))
				enable_disable_open_btn(dialog);
			else
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

	path = getenv("PWD");
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

static int file_dialog_input_event(gp_dialog *self, gp_event *ev, int is_open)
{
	struct file_dialog *dialog = (void*)self->payload;

	if (ev->type == GP_EV_KEY && ev->code == GP_EV_KEY_DOWN) {
		if (ev->val == GP_KEY_ESC && ev->code == GP_EV_KEY_DOWN) {
			exit_dialog(dialog, GP_WIDGET_DIALOG_CANCEL);
			return 1;
		}

		int ctrl_pressed = gp_ev_any_key_pressed(ev, GP_KEY_LEFT_CTRL, GP_KEY_RIGHT_CTRL);

		if (is_open && ctrl_pressed && ev->val == GP_KEY_O) {
			try_open(dialog);
			return 1;
		}

		if (!is_open && ctrl_pressed && ev->val == GP_KEY_S) {
			try_save(dialog);
			return 1;
		}
	}

	if (ev->type == GP_EV_UTF || (ev->type == GP_EV_KEY && ev->val == GP_KEY_BACKSPACE))
		return gp_widget_ops_event(dialog->filter, gp_widgets_render_ctx(), ev);

	return 0;
}

static int file_open_input_event(gp_dialog *self, gp_event *ev)
{
	return file_dialog_input_event(self, ev, 1);
}

static int file_save_input_event(gp_dialog *self, gp_event *ev)
{
	return file_dialog_input_event(self, ev, 0);
}

static int new_dir_on_event(gp_widget_event *ev)
{
	struct file_dialog *dialog = ev->self->priv;
	char *dir_name;
	unsigned int pos;
	int err;

	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	gp_widget_table_priv *tbl_priv = gp_widget_table_priv_get(dialog->file_table);
	gp_dir_cache *cache = tbl_priv->priv;

	dir_name = gp_dialog_input_run("Enter directory name");
	if (!dir_name)
		goto ret0;

	err = gp_dir_cache_mkdir(cache, dir_name);
	if (err) {
		gp_dialog_msg_printf_run(GP_DIALOG_MSG_ERR,
		                        "Failed to create directory",
		                        "%s", strerror(err));
		goto ret1;
	}

	pos = gp_dir_cache_pos_by_name_filtered(cache, dir_name);

	gp_widget_table_off_set(dialog->file_table, pos);
	gp_widget_table_sel_set(dialog->file_table, pos);

	gp_widget_redraw(dialog->file_table);

ret1:
	free(dir_name);
ret0:
	//TODO: Should we save and restore focus?
	gp_widget_focus_set(dialog->file_table);

	return 0;
}

static int filename_on_event(gp_widget_event *ev)
{
	struct file_dialog *dialog = ev->self->priv;

	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	switch (ev->sub_type) {
	case GP_WIDGET_TBOX_TRIGGER:
		try_save(dialog);
	break;
	case GP_WIDGET_TBOX_EDIT:
		gp_widget_disabled_set(dialog->open_save_btn,
		                       gp_widget_tbox_is_empty(ev->self));
	break;
	}

	return 0;
}

static int filter_on_event(gp_widget_event *ev)
{
	struct file_dialog *dialog = ev->self->priv;

	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	gp_widget_table_priv *tbl_priv = gp_widget_table_priv_get(dialog->file_table);
	gp_dir_cache *cache = tbl_priv->priv;

	switch (ev->sub_type) {
	case GP_WIDGET_TBOX_POST_FILTER:
		return !gp_dir_cache_entry_name_contains(cache, gp_widget_tbox_text(ev->self));
	break;
	case GP_WIDGET_TBOX_EDIT:
		gp_widget_redraw(dialog->file_table);
		//TODO: We need stable selected row!!
		//enable_disable_open_btn(dialog);
	break;
	}

	return 0;
}

static int path_on_event(gp_widget_event *ev)
{
	struct file_dialog *dialog = ev->self->priv;

	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	switch (ev->sub_type) {
	case GP_WIDGET_TBOX_TRIGGER:
		free_dir_cache(dialog);
		gp_widget_redraw(dialog->file_table);
	break;
	}

	return 0;
}

static int home_on_event(gp_widget_event *ev)
{
	struct file_dialog *dialog = ev->self->priv;

	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	const char *home = getenv("HOME");

	if (home)
		set_path(dialog, home);

	return 0;
}

static const gp_widget_json_addr addrs[] = {
	{.id = "cancel", .on_event = cancel_on_event},
	{.id = "file_table", .table_col_ops = &gp_dialog_files_col_ops},
	{.id = "filename", .on_event = filename_on_event},
	{.id = "filter", .on_event = filter_on_event},
	{.id = "home", .on_event = home_on_event},
	{.id = "new_dir", .on_event = new_dir_on_event},
	{.id = "open", .on_event = open_on_event},
	{.id = "path", .on_event = path_on_event},
	{.id = "save", .on_event = save_on_event},
	{}
};

gp_dialog *gp_dialog_file_save_new(const char *path,
                                   const gp_dialog_file_opts *opts)
{
	gp_htable *uids = NULL;
	gp_widget *layout;
	gp_dialog *ret;
	struct file_dialog *dialog;

	ret = gp_dialog_new(sizeof(struct file_dialog));
	if (!ret)
		return NULL;

	dialog = (void*)ret->payload;

	dialog->opts = opts;

	gp_widget_json_callbacks callbacks = {
		.default_priv = dialog,
		.addrs = addrs,
	};

	layout = gp_dialog_layout_load("file_save", &callbacks, dialog_file_save, &uids);
	if (!layout)
		goto err0;

	ret->layout = layout;
	ret->input_event = file_save_input_event;

	dialog->show_hidden = gp_widget_by_uid(uids, "hidden", GP_WIDGET_CHECKBOX);
	dialog->filename = gp_widget_by_uid(uids, "filename", GP_WIDGET_TBOX);
	dialog->dir_path = gp_widget_by_uid(uids, "path", GP_WIDGET_TBOX);
	dialog->file_table = gp_widget_by_uid(uids, "files", GP_WIDGET_TABLE);
	dialog->open_save_btn = gp_widget_by_uid(uids, "save", GP_WIDGET_BUTTON);

	if (!dialog->file_table) {
		GP_WARN("No file table defined!");
		goto err1;
	}

	if (dialog->open_save_btn)
		gp_widget_disable(dialog->open_save_btn);

	gp_widget_on_event_set(dialog->file_table, table_on_event, dialog);
	gp_widget_events_unmask(dialog->file_table, GP_WIDGET_EVENT_INPUT);

	gp_htable_free(uids);

	if (!dialog->dir_path) {
		GP_WARN("Missing path widget!");
		goto err1;
	}

	if (dialog->show_hidden)
		gp_widget_on_event_set(dialog->show_hidden, redraw_table, dialog);

	gp_widget_tbox_printf(dialog->dir_path, "%s", get_path(path));

	return ret;
err1:
	gp_widget_free(layout);
err0:
	free(ret);
	return NULL;
}

gp_dialog *gp_dialog_file_open_new(const char *path,
                                   const gp_dialog_file_opts *opts)
{
	gp_htable *uids = NULL;
	gp_widget *layout;
	gp_dialog *ret;
	struct file_dialog *dialog;

	ret = gp_dialog_new(sizeof(struct file_dialog));
	if (!ret)
		return NULL;

	dialog = (void*)ret->payload;

	dialog->opts = opts;

	gp_widget_json_callbacks callbacks = {
		.default_priv = dialog,
		.addrs = addrs,
	};

	layout = gp_dialog_layout_load("file_open", &callbacks, dialog_file_open, &uids);
	if (!layout)
		goto err0;

	ret->layout = layout;
	ret->input_event = file_open_input_event;

	dialog->show_hidden = gp_widget_by_uid(uids, "hidden", GP_WIDGET_CHECKBOX);
	dialog->filter = gp_widget_by_uid(uids, "filter", GP_WIDGET_TBOX);
	dialog->dir_path = gp_widget_by_uid(uids, "path", GP_WIDGET_TBOX);
	dialog->file_table = gp_widget_by_uid(uids, "files", GP_WIDGET_TABLE);
	dialog->open_save_btn = gp_widget_by_uid(uids, "open", GP_WIDGET_BUTTON);

	if (!dialog->file_table) {
		GP_WARN("No file table defined!");
		goto err1;
	}

	if (dialog->open_save_btn)
		gp_widget_disable(dialog->open_save_btn);

	gp_widget_on_event_set(dialog->file_table, table_on_event, dialog);
	gp_widget_events_unmask(dialog->file_table, GP_WIDGET_EVENT_INPUT);

	gp_htable_free(uids);

	if (!dialog->dir_path) {
		GP_WARN("Missing path widget!");
		goto err1;
	}

	if (dialog->show_hidden)
		gp_widget_on_event_set(dialog->show_hidden, redraw_table, dialog);

	gp_widget_tbox_printf(dialog->dir_path, "%s", get_path(path));

	return ret;
err1:
	gp_widget_free(layout);
err0:
	free(ret);
	return NULL;
}

const char *gp_dialog_file_path(gp_dialog *self)
{
	struct file_dialog *dialog = (void*)self->payload;

	return gp_widget_tbox_text(dialog->dir_path);
}
