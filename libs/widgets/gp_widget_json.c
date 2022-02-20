//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2022 Cyril Hrubis <metan@ucw.cz>

 */

#include <dlfcn.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>

#include <utils/gp_htable.h>

#include <widgets/gp_widget.h>
#include <widgets/gp_widgets.h>
#include <widgets/gp_widget_ops.h>

static gp_widget *color_scheme_switch_from_json(gp_json_buf *json, gp_json_val *val, gp_widget_json_ctx *ctx)
{
	(void) ctx;

	GP_JSON_OBJ_FILTER(json, val, NULL, gp_widget_json_attrs) {
	}

	return gp_widget_color_scheme_switch();
}

/*
 * This implements loading widgets that are build on top of existing widgets
 * and hence does not have widget ops registered. Which allows us to construct
 * anything from json description as long as we have a function that takes json
 * and returns a widget.
 */
static struct from_json {
	const char *type;
	gp_widget *(*from_json)(gp_json_buf *json, gp_json_val *val, gp_widget_json_ctx *ctx);
} json_loaders[] = {
	{"color_scheme_switch", color_scheme_switch_from_json}
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
	FOCUS,
	HALIGN,
	ON_EVENT,
	SHRINK,
	TYPE,
	UID,
	VALIGN,
	VERSION,
};

static const gp_json_obj_attr attrs[] = {
	GP_JSON_OBJ_ATTR("align", GP_JSON_STR),
	GP_JSON_OBJ_ATTR("focused", GP_JSON_BOOL),
	GP_JSON_OBJ_ATTR("halign", GP_JSON_STR),
	GP_JSON_OBJ_ATTR("on_event", GP_JSON_STR),
	GP_JSON_OBJ_ATTR("shrink", GP_JSON_BOOL),
	GP_JSON_OBJ_ATTR("type", GP_JSON_VOID),
	GP_JSON_OBJ_ATTR("uid", GP_JSON_VOID),
	GP_JSON_OBJ_ATTR("valign", GP_JSON_STR),
	GP_JSON_OBJ_ATTR("version", GP_JSON_INT),
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

	ret->on_event = dlsym(ld_handle, fn_name);

	GP_DEBUG(3, "Function '%s' address is %p", fn_name, ret->on_event);
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
	void *ret;

	if (ctx && ctx->callbacks)
		return struct_from_callbacks(struct_name, ctx->callbacks);

	if (!ld_handle)
		return NULL;

	ret = dlsym(ld_handle, struct_name);

	GP_DEBUG(3, "Structure '%s' address is %p", struct_name, ret);

	return ret;
}

void *gp_widget_struct_addr(const char *struct_name,
                            const gp_widget_json_ctx *ctx)
{
	if (ctx && ctx->callbacks)
		return addr_from_callbacks(struct_name, ctx);

	if (!ld_handle)
		return NULL;

	void *addr = dlsym(ld_handle, struct_name);

	GP_DEBUG(3, "Structure '%s' address is %p", struct_name, addr);

	return addr;
}

gp_widget *gp_widget_from_json(gp_json_buf *json, gp_json_val *val, gp_widget_json_ctx *ctx)
{
	const struct gp_widget_ops *ops;
	char *uid = NULL;
	unsigned int halign = 0;
	unsigned int valign = 0;
	unsigned int shrink_set = 0;
	unsigned int shrink;
	gp_htable **uids = ctx->uids;
	int focus = 0;
	struct on_event_addr on_event = {};
	gp_widget *(*from_json)(gp_json_buf *, gp_json_val *, gp_widget_json_ctx *) = gp_widget_grid_ops.from_json;

	if (val->type == GP_JSON_NULL)
		return NULL;

	if (gp_json_next_type(json) != GP_JSON_OBJ) {
		gp_json_warn(json, "Widget must be JSON object!");
		return NULL;
	}

	gp_json_state obj_start = gp_json_state_start(json);

	if (!gp_json_obj_first(json, val))
		return NULL;

	gp_json_state_load(json, obj_start);

	GP_JSON_OBJ_FILTER(json, val, &obj_filter, NULL) {
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
		case FOCUS:
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

			shrink_set = 1;
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

	if (gp_json_is_err(json))
		return NULL;

	gp_json_state_load(json, obj_start);

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
	if (on_event.on_event)
		gp_widget_on_event_set(wid, on_event.on_event, on_event.priv);

	if (shrink_set)
		wid->no_shrink = !shrink;

	gp_widget_send_event(wid, GP_WIDGET_EVENT_NEW);

	return wid;
skip:
	gp_json_state_load(json, obj_start);
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

static gp_widget *gp_widgets_from_json(gp_json_buf *json,
                                       const gp_widget_json_callbacks *const callbacks,
                                       gp_htable **uids)
{
	gp_widget_json_ctx ctx = {.uids = uids, .callbacks = callbacks};
	gp_widget *ret;
	char buf[128];
	gp_json_val val = {.buf = buf, .buf_size = sizeof(buf)};

	check_callback_addrs_sorted(callbacks);

	if (gp_json_next_type(json) != GP_JSON_OBJ) {
		gp_json_err(json, "Widget must be a JSON object!");
		return NULL;
	}

	gp_json_state obj_start = gp_json_state_start(json);

	if (!gp_json_obj_first(json, &val) ||
	    strcmp(val.id, "version") ||
	    val.type != GP_JSON_INT) {
		gp_json_err(json, "JSON layout must start with a version number!");
		return NULL;
	}

	GP_DEBUG(1, "Loading JSON layout version %li", val.val_int);

	if (val.val_int != 1) {
		gp_json_err(json, "Unknown version number %li", val.val_int);
		return NULL;
	}

	ld_handle = dlopen(NULL, RTLD_LAZY);
	if (!ld_handle)
		GP_WARN("Failed to dlopen()");

	if (!gp_json_obj_next(json, &val))
		return NULL;

	gp_json_state_load(json, obj_start);

	ret = gp_widget_from_json(json, &val, &ctx);
	if (ret && ctx.focused) {
		if (!gp_widget_focus_set(ctx.focused)) {
			GP_WARN("Failed to focus %p (%s)",
			        ctx.focused, gp_widget_type_id(ctx.focused));
		}
	}

	dlclose(ld_handle);

	ld_handle = NULL;

	return ret;
}

gp_widget *gp_widget_layout_json(const char *path,
                                 const gp_widget_json_callbacks *const callbacks,
                                 gp_htable **uids)
{
	gp_widget *ret = NULL;
	gp_json_buf *json;

	if (uids)
		*uids = NULL;

	json = gp_json_load(path);
	if (!json) {
		GP_WARN("Failed to load '%s': %s", path, strerror(errno));
		return NULL;
	}

	json->msgf = stderr;

	ret = gp_widgets_from_json(json, callbacks, uids);

	if (gp_json_is_err(json))
		gp_json_err_print(json);
	else if (!gp_json_empty(json))
		gp_json_warn(json, "Garbage after JSON string!");

	gp_json_free(json);

	return ret;
}

gp_widget *gp_widget_from_json_str(const char *str,
                                   const gp_widget_json_callbacks *const callbacks,
                                   gp_htable **uids)
{
	gp_widget *ret = NULL;
	gp_json_buf json = {
		.json = str,
		.len = strlen(str),
		.max_depth = GP_JSON_RECURSION_MAX,
		.msgf = stderr,
	};

	if (uids)
		*uids = NULL;

	ret = gp_widgets_from_json(&json, callbacks, uids);

	if (gp_json_is_err(&json))
		gp_json_err_print(&json);
	else if (!gp_json_empty(&json))
		gp_json_warn(&json, "Garbage after JSON string!");

	return ret;
}

gp_widget *gp_widget_by_uid(void *uids, const char *uid, enum gp_widget_type type)
{
	gp_widget *ret = gp_htable_get(uids, uid);

	if (!ret)
		return NULL;

	if (ret->type != type) {
		GP_WARN("Unexpected widget (uid='%s') type %s expected %s",
		        uid, gp_widget_type_name(ret->type),
		        gp_widget_type_name(type));
		return NULL;
	}

	return ret;
}

gp_widget *gp_widget_by_cuid(void *uids, const char *uid, enum gp_widget_class widget_class)
{
	gp_widget *ret = gp_htable_get(uids, uid);

	if (!ret)
		return NULL;

	if (ret->widget_class != widget_class) {
		GP_WARN("Unexpected widget (uid='%s') class %s expected %s",
		        uid, gp_widget_class_name(ret->widget_class),
		        gp_widget_class_name(widget_class));
		return NULL;
	}

	return ret;
}
