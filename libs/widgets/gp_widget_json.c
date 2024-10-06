//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2022 Cyril Hrubis <metan@ucw.cz>

 */

#include "../../config.h"
#ifdef HAVE_DL
# include <dlfcn.h>
#endif
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>

#include <utils/gp_htable.h>

#include <widgets/gp_widget.h>
#include <widgets/gp_widgets.h>
#include <widgets/gp_widget_ops.h>

static gp_widget *color_scheme_switch_from_json(gp_json_reader *json, gp_json_val *val, gp_widget_json_ctx *ctx)
{
	(void) ctx;

	GP_JSON_OBJ_FOREACH_FILTER(json, val, NULL, gp_widget_json_attrs) {
	}

	return gp_widget_color_scheme_switch();
}

gp_widget *hbox_from_json(gp_json_reader *json, gp_json_val *val, gp_widget_json_ctx *ctx);
gp_widget *vbox_from_json(gp_json_reader *json, gp_json_val *val, gp_widget_json_ctx *ctx);

/*
 * This implements loading widgets that are build on top of existing widgets
 * and hence does not have widget ops registered. Which allows us to construct
 * anything from json description as long as we have a function that takes json
 * and returns a widget.
 */
static struct from_json {
	const char *type;
	gp_widget *(*from_json)(gp_json_reader *json, gp_json_val *val, gp_widget_json_ctx *ctx);
} json_loaders[] = {
	{"color_scheme_switch", color_scheme_switch_from_json},
	{"hbox", hbox_from_json},
	{"vbox", vbox_from_json},
};

static void *json_loader_by_type(const char *type)
{
	unsigned int i;

	for (i = 0; i < GP_ARRAY_SIZE(json_loaders); i++) {
		if (!strcmp(type, json_loaders[i].type))
			return json_loaders[i].from_json;
	}

	return NULL;
}

enum keys {
	ALIGN,
	DISABLED,
	FOCUSED,
	HALIGN,
	ON_EVENT,
	SHRINK,
	TYPE,
	UID,
	VALIGN,
};

static const gp_json_obj_attr attrs[] = {
	GP_JSON_OBJ_ATTR_IDX(ALIGN, "align", GP_JSON_STR),
	GP_JSON_OBJ_ATTR_IDX(DISABLED, "disabled", GP_JSON_BOOL),
	GP_JSON_OBJ_ATTR_IDX(FOCUSED, "focused", GP_JSON_BOOL),
	GP_JSON_OBJ_ATTR_IDX(HALIGN, "halign", GP_JSON_STR),
	GP_JSON_OBJ_ATTR_IDX(ON_EVENT, "on_event", GP_JSON_STR),
	GP_JSON_OBJ_ATTR_IDX(SHRINK, "shrink", GP_JSON_BOOL),
	GP_JSON_OBJ_ATTR_IDX(TYPE, "type", GP_JSON_VOID),
	GP_JSON_OBJ_ATTR_IDX(UID, "uid", GP_JSON_VOID),
	GP_JSON_OBJ_ATTR_IDX(VALIGN, "valign", GP_JSON_STR),
};

static const gp_json_obj obj_filter = {
	.attrs = attrs,
	.attr_cnt = GP_ARRAY_SIZE(attrs),
};

const gp_json_obj *gp_widget_json_attrs = &obj_filter;

extern struct gp_widget_ops gp_widget_grid_ops;

static void *ld_handle;

struct on_event_addr {
	int (*on_event)(gp_widget_event *ev);
	void *priv;
};

static void on_event_from_callbacks(const char *name,
                                    const gp_widget_json_callbacks *const callbacks,
                                    struct on_event_addr *ret)
{
	size_t i;

	//TODO: Interval divison!
	for (i = 0; callbacks->addrs[i].id; i++) {
		if (!strcmp(callbacks->addrs[i].id, name)) {
			GP_DEBUG(3, "Function '%s' addres is %p", name , callbacks->addrs[i].addr);
			ret->on_event = callbacks->addrs[i].addr;
			ret->priv = callbacks->default_priv;
			return;
		}
	}

	GP_WARN("Failed to lookup %s in callbacks", name);
}

static void *struct_from_callbacks(const char *name,
                                   const gp_widget_json_callbacks *const callbacks)
{
	size_t i;

	//TODO: Interval divison!
	for (i = 0; callbacks->addrs[i].id; i++) {
		if (!strcmp(callbacks->addrs[i].id, name)) {
			GP_DEBUG(3, "Structure '%s' addres is %p", name , callbacks->addrs[i].addr);
			return callbacks->addrs[i].addr;
		}
	}

	GP_WARN("Failed to lookup %s in structures", name);

	return NULL;
}

void gp_widget_on_event_addr(const char *fn_name,
                             const gp_widget_json_ctx *ctx,
			     struct on_event_addr *ret)
{
	if (ctx && ctx->callbacks) {
		on_event_from_callbacks(fn_name, ctx->callbacks, ret);
		return;
	}

	if (!ld_handle)
		return;

#ifdef HAVE_DL
	ret->on_event = dlsym(ld_handle, fn_name);

	GP_DEBUG(3, "Function '%s' address is %p", fn_name, ret->on_event);
#endif
}

void *gp_widget_callback_addr(const char *fn_name,
                              const gp_widget_json_ctx *ctx)
{
	struct on_event_addr addr = {};

	gp_widget_on_event_addr(fn_name, ctx, &addr);

	return addr.on_event;
}

static void *addr_from_callbacks(const char *struct_name,
                                const gp_widget_json_ctx *ctx)
{
	if (ctx && ctx->callbacks)
		return struct_from_callbacks(struct_name, ctx->callbacks);

	if (!ld_handle)
		return NULL;

#ifdef HAVE_DL
	void *ret = dlsym(ld_handle, struct_name);

	GP_DEBUG(3, "Structure '%s' address is %p", struct_name, ret);

	return ret;
#else
	return NULL;
#endif
}

void *gp_widget_struct_addr(const char *struct_name,
                            const gp_widget_json_ctx *ctx)
{
	if (ctx && ctx->callbacks)
		return addr_from_callbacks(struct_name, ctx);

	if (!ld_handle)
		return NULL;

#ifdef HAVE_DL
	void *addr = dlsym(ld_handle, struct_name);

	GP_DEBUG(3, "Structure '%s' address is %p", struct_name, addr);

	return addr;
#else
	return NULL;
#endif
}

gp_widget *gp_widget_from_json(gp_json_reader *json, gp_json_val *val, gp_widget_json_ctx *ctx)
{
	const struct gp_widget_ops *ops;
	char *uid = NULL;
	unsigned int halign = 0;
	unsigned int valign = 0;
	unsigned int disabled = 0;
	int shrink = -1;
	gp_htable **uids = ctx->uids;
	int focus = 0;
	struct on_event_addr on_event = {};
	gp_widget *(*from_json)(gp_json_reader *, gp_json_val *, gp_widget_json_ctx *) = gp_widget_grid_ops.from_json;

	if (val->type == GP_JSON_NULL)
		return NULL;

	if (gp_json_next_type(json) != GP_JSON_OBJ) {
		gp_json_warn(json, "Widget must be JSON object!");
		return NULL;
	}

	gp_json_reader_state obj_start = gp_json_reader_state_save(json);

	if (!gp_json_obj_first(json, val))
		return NULL;

	gp_json_reader_state_load(json, obj_start);

	GP_JSON_OBJ_FOREACH_FILTER(json, val, &obj_filter, NULL) {
		switch (val->idx) {
		case ALIGN:
			if (!strcmp(val->val_str, "center")) {
				halign = GP_HCENTER;
				valign = GP_VCENTER;
			} else if (!strcmp(val->val_str, "fill")) {
				halign = GP_HFILL;
				valign = GP_VFILL;
			} else if (!strcmp(val->val_str, "hfill")) {
				halign = GP_HFILL;
			} else if (!strcmp(val->val_str, "vfill")) {
				valign = GP_VFILL;
			} else {
				gp_json_warn(json,
				             "Invalid align='%s'",
				             val->val_str);
			}
		break;
		case DISABLED:
			disabled = val->val_bool;
		break;
		case FOCUSED:
			focus = val->val_bool;
		break;
		case HALIGN:
			if (halign)
				GP_WARN("Only one of halign and align can be defined!");

			if (!strcmp(val->val_str, "center"))
				halign = GP_HCENTER;
			else if (!strcmp(val->val_str, "left"))
				halign = GP_LEFT;
			else if (!strcmp(val->val_str, "right"))
				halign = GP_RIGHT;
			else if (!strcmp(val->val_str, "fill"))
				halign = GP_HFILL;
			else
				gp_json_warn(json, "Invalid halign='%s'", val->val_str);
		break;
		case ON_EVENT:
			gp_widget_on_event_addr(val->val_str, ctx, &on_event);
			if (!on_event.on_event)
				gp_json_warn(json, "No on_event function '%s' defined", val->val_str);
		break;
		case SHRINK:
			shrink = val->val_bool;

			GP_DEBUG(2, "Widget shrink '%i'", shrink);
		break;
		case TYPE:
			if (val->type != GP_JSON_STR) {
				gp_json_warn(json, "Invalid 'type' type expected string");
				goto skip;
			}

			ops = gp_widget_ops_by_id(val->val_str);
			if (ops) {
				from_json = ops->from_json;

				if (!from_json) {
					GP_WARN("Unimplemented from_json for widget '%s'", ops->id);
					goto skip;
				}
			} else {
				from_json = json_loader_by_type(val->val_str);

				if (!from_json) {
					GP_WARN("Invalid widget type '%s'", val->val_str);
					goto skip;
				}
			}
		break;
		case UID:
			if (val->type != GP_JSON_STR) {
				gp_json_warn(json, "Invalid 'uid' type expected string");
				goto skip;
			}

			if (uids)
				uid = strdup(val->val_str);

			GP_DEBUG(2, "Widget uid '%s'", val->val_str);
		break;
		case VALIGN:
			if (valign)
				GP_WARN("Only one of valign and align can be defined!");

			if (!strcmp(val->val_str, "center"))
				valign = GP_VCENTER;
			else if (!strcmp(val->val_str, "top"))
				valign = GP_TOP;
			else if (!strcmp(val->val_str, "bottom"))
				valign = GP_BOTTOM;
			else if (!strcmp(val->val_str, "fill"))
				valign = GP_VFILL;
			else
				GP_WARN("Invalid valign=%s.", val->val_str);
		break;
		}
	}

	if (gp_json_reader_err(json))
		return NULL;

	gp_json_reader_state_load(json, obj_start);

	if (!from_json)
		return NULL;

	gp_widget *wid = from_json(json, val, ctx);
	if (!wid)
		return NULL;

	if (focus) {
		if (ctx->focused) {
			GP_WARN("Widget %p (%s) is already focused!",
				ctx->focused, gp_widget_type_id(ctx->focused));
		} else {
			ctx->focused = wid;
		}
	}

	if (uid) {
		if (!*uids) {
			*uids = gp_htable_new(0, GP_HTABLE_FREE_KEY);
			if (!*uids) {
				GP_WARN("Malloc failed :(");
				free(uid);
				goto ret;
			}
		}

		if (gp_htable_get(*uids, uid))
			GP_WARN("Duplicit widget uid '%s'", uid);

		gp_htable_put(*uids, wid, uid);
	}
ret:
	wid->align = halign | valign;
	if (on_event.on_event) {
		GP_DEBUG(3, "Setting widget %p (%s) on event %p priv %p",
			 wid, gp_widget_type_id(wid), on_event.on_event, on_event.priv);
		gp_widget_on_event_set(wid, on_event.on_event, on_event.priv);
	}

	if (shrink != -1)
		wid->no_shrink = !shrink;

	gp_widget_send_event(wid, GP_WIDGET_EVENT_NEW);

	wid->disabled = disabled;

	return wid;
skip:
	gp_json_reader_state_load(json, obj_start);
	gp_json_obj_skip(json);
	return NULL;
}

static void check_callback_addrs_sorted(const gp_widget_json_callbacks *const callbacks)
{
	size_t i;

	if (!callbacks)
		return;

	if (!callbacks->addrs[1].id)
		return;

	for (i = 1; callbacks->addrs[i].id; i++) {
		int cmp;

		cmp = strcmp(callbacks->addrs[i-1].id, callbacks->addrs[i].id);

		if (!cmp)
			GP_ABORT("Duplicate id '%s'!", callbacks->addrs[i].id);

		if (cmp > 0) {
			GP_ABORT("Wrong order of '%s' and '%s'",
			         callbacks->addrs[i-1].id,
			         callbacks->addrs[i].id);
		}
	}
}

enum info_keys {
	INFO_AUTHOR,
	INFO_LICENSE,
	INFO_VERSION,
};

static const gp_json_obj_attr info_attrs[] = {
	GP_JSON_OBJ_ATTR_IDX(INFO_AUTHOR, "author", GP_JSON_STR),
	GP_JSON_OBJ_ATTR_IDX(INFO_LICENSE, "license", GP_JSON_STR),
	GP_JSON_OBJ_ATTR_IDX(INFO_VERSION, "version", GP_JSON_INT),
};

static const gp_json_obj info_obj_filter = {
	.attrs = info_attrs,
	.attr_cnt = GP_ARRAY_SIZE(info_attrs),
};

static long parse_info_block(gp_json_reader *json, gp_json_val *val)
{
	long version = -1;
	int license_set = 0;

	GP_JSON_OBJ_FOREACH_FILTER(json, val, &info_obj_filter, NULL) {
		switch (val->idx) {
		case INFO_AUTHOR:
			GP_DEBUG(1, "Layout author: %s", val->val_str);
		break;
		case INFO_LICENSE:
			GP_DEBUG(1, "Layout license: %s", val->val_str);
			license_set = 1;
		break;
		case INFO_VERSION:
			GP_DEBUG(1, "Layout version: %lli", val->val_int);
			version = val->val_int;
		break;
		}
	}

	if (!license_set)
		gp_json_err(json, "Layout license missing!");

	return version;
}

static gp_widget *gp_widgets_from_json(gp_json_reader *json,
                                       const gp_widget_json_callbacks *const callbacks,
                                       gp_htable **uids)
{
	gp_widget_json_ctx ctx = {.uids = uids, .callbacks = callbacks};
	gp_widget *ret;
	char buf[1024];
	gp_json_val val = {.buf = buf, .buf_size = sizeof(buf)};

	check_callback_addrs_sorted(callbacks);

	if (!gp_json_obj_first(json, &val) ||
	    strcmp(val.id, "info") ||
	    val.type != GP_JSON_OBJ) {
		gp_json_err(json, "JSON layout must start with a info block!");
		return NULL;
	}

	long version = parse_info_block(json, &val);
	if (version != 1) {
		gp_json_err(json, "Unknown version number %li", version);
		return NULL;
	}

#ifdef HAVE_DL
	ld_handle = dlopen(NULL, RTLD_LAZY);
	if (!ld_handle)
		GP_WARN("Failed to dlopen()");
#endif

	if (!gp_json_obj_next(json, &val) ||
	    strcmp(val.id, "layout") ||
	    val.type != GP_JSON_OBJ) {
		gp_json_err(json, "Expected 'layout' block!");
		return NULL;
	}

	ret = gp_widget_from_json(json, &val, &ctx);
	if (ret && ctx.focused) {
		if (!gp_widget_focus_set(ctx.focused)) {
			GP_WARN("Failed to focus %p (%s)",
			        ctx.focused, gp_widget_type_id(ctx.focused));
		}
	}

	if (gp_json_obj_next(json, &val))
		gp_json_warn(json, "Unexpected JSON key '%s'", val.id);

#ifdef HAVE_DL
	dlclose(ld_handle);
	ld_handle = NULL;
#endif

	return ret;
}

gp_widget *gp_widget_layout_json(const char *path,
                                 const gp_widget_json_callbacks *const callbacks,
                                 gp_htable **uids)
{
	gp_widget *ret = NULL;
	gp_json_reader *json;

	if (uids)
		*uids = NULL;

	json = gp_json_reader_load(path);
	if (!json) {
		GP_WARN("Failed to load '%s': %s", path, strerror(errno));
		return NULL;
	}

	ret = gp_widgets_from_json(json, callbacks, uids);

	gp_json_reader_finish(json);

	gp_json_reader_free(json);

	return ret;
}

gp_widget *gp_widget_from_json_str(const char *str,
                                   const gp_widget_json_callbacks *const callbacks,
                                   gp_htable **uids)
{
	gp_widget *ret = NULL;
	gp_json_reader json = GP_JSON_READER_INIT(str, strlen(str));

	if (uids)
		*uids = NULL;

	ret = gp_widgets_from_json(&json, callbacks, uids);

	gp_json_reader_finish(&json);

	return ret;
}
