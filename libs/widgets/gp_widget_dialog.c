//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>
#include <core/gp_debug.h>
#include <utils/gp_vec_str.h>
#include <widgets/gp_widget_json.h>
#include <widgets/gp_widget_dialog.h>

gp_widget_dialog *gp_widget_dialog_new(size_t payload_size)
{
	size_t size = sizeof(gp_widget_dialog) + payload_size;
	gp_widget_dialog *ret = malloc(size);

	if (!ret) {
		GP_WARN("Malloc failed :-(");
		return NULL;
	}

	GP_DEBUG(1, "Allocated dialog %p", ret);

	memset(ret, 0, size);

	return ret;
}

void gp_widget_dialog_free(gp_widget_dialog *self)
{
	GP_DEBUG(1, "Destroying dialog %p", self);

	gp_widget_free(self->layout);
	free(self);
}

static gp_widget *try_load_layout(const char *pathname, void **uids)
{
	if (access(pathname, R_OK)) {
		GP_DEBUG(3, "File '%s' does not exists or is not readable", pathname);
		return NULL;
	}

	GP_DEBUG(3, "Trying '%s'", pathname);

	return gp_widget_layout_json(pathname, uids);
}

gp_widget *gp_dialog_layout_load(const char *dialog_name,
                                 const char *fallback_json, void **uids)
{
	gp_widget *layout;
	char *pathname = NULL;
	const char *home = getenv("HOME");

	if (home) {
		pathname = gp_vec_printf(pathname, "%s/.config/gfxprim/dialogs/%s.json", home, dialog_name);
		if (!pathname)
			return NULL;

		layout = try_load_layout(pathname, uids);
		if (layout)
			goto ret;
	}

	pathname = gp_vec_printf(pathname, "/etc/gfxprim/dialogs/%s.json", dialog_name);
	if (!pathname)
		return NULL;

	layout = try_load_layout(pathname, uids);
	if (layout)
		goto ret;

	layout = gp_widget_from_json_str(fallback_json, uids);
ret:
	gp_vec_free(pathname);
	return layout;
}
