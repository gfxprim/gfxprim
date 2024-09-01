//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>

#include <widgets/gp_widgets.h>
#include <widgets/gp_widget_ops.h>
#include <widgets/gp_widget_render.h>
#include <widgets/gp_widget_json.h>

struct pixmap_payload {
	gp_widget_size min_w;
	gp_widget_size min_h;
	gp_pixmap *pixmap;
	int bbox_set:1;
	int redraw_all:1;
	/** Bounding box */
	gp_bbox bbox;
};


static unsigned int min_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	struct pixmap_payload *pixmap = GP_WIDGET_PAYLOAD(self);

	return gp_widget_size_units_get(&pixmap->min_w, ctx);
}

static unsigned int min_h(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	struct pixmap_payload *pixmap = GP_WIDGET_PAYLOAD(self);

	return gp_widget_size_units_get(&pixmap->min_h, ctx);
}

static inline void redraw_passthrough(gp_widget *self,
                                      const gp_widget_render_ctx *ctx,
                                      gp_coord x, gp_coord y,
                                      gp_size w, gp_size h)
{
	struct pixmap_payload *pixmap = GP_WIDGET_PAYLOAD(self);
	gp_pixmap sub_pix;

	/*
	 * If we are passed down bounding box, i.e. library requested partiall update,
	 * compute an intersection with pixmap and pass it down to the event handler.
	 */
	gp_bbox box = gp_bbox_pack(x, y, w, h);
	if (ctx->bbox)
		box = gp_bbox_intersection(box, *ctx->bbox);

	/* Move the box so that it's relative to the pixmap widget */
	gp_bbox sub_box = gp_bbox_move(box, -x, -y);

	gp_sub_pixmap(ctx->buf, &sub_pix, x, y, w, h);

	pixmap->pixmap = &sub_pix;
	int ret = gp_widget_send_event(self, GP_WIDGET_EVENT_REDRAW, ctx, &sub_box);
	pixmap->pixmap = NULL;

	/*
	 * Request full/partial update based on return value from the handler.
	 */
	if (ret) {
		gp_widget_ops_blit(ctx, x, y, w, h);
	} else {
		gp_widget_ops_blit(ctx, box.x, box.y, box.w, box.h);
	}
}

static inline void redraw_buffered(gp_widget *self,
                                   const gp_widget_render_ctx *ctx,
                                   gp_coord x, gp_coord y, gp_size w, gp_size h,
				   int flags)
{
	struct pixmap_payload *pixmap = GP_WIDGET_PAYLOAD(self);
	gp_bbox box = {};

	GP_DEBUG(2, "Blitting buffered pixmap %ux%u",
		 pixmap->pixmap->w, pixmap->pixmap->h);

	/* Complete redraw requested by the application */
	if (pixmap->redraw_all) {
		box = gp_bbox_pack(x, y, w, h);
		goto redraw;
	}

	/* Partial redraw requested by the application */
	if (pixmap->bbox_set) {
		/* Move the bouding box since the application box is relative to widget */
		box = gp_bbox_move(pixmap->bbox, x, y);
		//TODO check where this comes from
		box.w += 1;
		box.h += 1;
	}

	/* Complete redraw requested by the library */
	if (flags & GP_WIDGET_REDRAW) {
		box = gp_bbox_pack(x, y, w, h);
		goto redraw;
	}

	/*
	 * Library requests partiall update there are two cases:
	 *
	 * - if no update is schedulled for the pixmap by the application, we
	 *   just use intersection of the partial update bbox we were passed
	 *   down by the library with the widget itself
	 *
	 * - if update is schedulled we have to unify the application bounding box
	 *   with the partiall update bouding box and then do an intersection with
	 *   the widget itself
	 */
	if (ctx->bbox) {
		if (gp_bbox_empty(box))
			box = *ctx->bbox;
		else
			box = gp_bbox_merge(*ctx->bbox, box);
	}

	/* gp_widget_redraw() was called on pixmap and no bbox set -> redraw all */
	if (gp_bbox_empty(box)) {
		box = gp_bbox_pack(x, y, w, h);
		goto redraw;
	}

	box = gp_bbox_intersection(box, gp_bbox_pack(x, y, w, h));

redraw:
	gp_blit_xywh(pixmap->pixmap, box.x - x, box.y - y,
	             box.w, box.h,
	             ctx->buf, box.x, box.y);

	gp_widget_ops_blit(ctx, box.x, box.y, box.w, box.h);
}

static void render(gp_widget *self, const gp_offset *offset,
                   const gp_widget_render_ctx *ctx, int flags)
{
	struct pixmap_payload *pixmap = GP_WIDGET_PAYLOAD(self);

	gp_coord x = self->x + offset->x;
	gp_coord y = self->y + offset->y;
	gp_size w = self->w;
	gp_size h = self->h;

	if (flags & GP_WIDGET_COLOR_SCHEME)
		gp_widget_send_event(self, GP_WIDGET_EVENT_COLOR_SCHEME, ctx);

	if (!pixmap->pixmap)
		redraw_passthrough(self, ctx, x, y, w, h);
	else
		redraw_buffered(self, ctx, x, y, w, h, flags);

	pixmap->redraw_all = 0;
	pixmap->bbox_set = 0;
}

/*
 * Dummy event handler so that events are propagated to app event handler since
 * widgets without event handler can't get focus and events.
 */
static int event(gp_widget *self, const gp_widget_render_ctx *ctx, gp_event *ev)
{
	(void)self;
	(void)ctx;
	(void)ev;
	return 0;
}

enum keys {
	H,
	W,
};

static const gp_json_obj_attr attrs[] = {
	GP_JSON_OBJ_ATTR("h", GP_JSON_VOID),
	GP_JSON_OBJ_ATTR("w", GP_JSON_VOID),
};

static const gp_json_obj obj_filter = {
	.attrs = attrs,
	.attr_cnt = GP_ARRAY_SIZE(attrs),
};

static gp_widget *json_to_pixmap(gp_json_reader *json, gp_json_val *val, gp_widget_json_ctx *ctx)
{
	gp_widget_size w = {};
	gp_widget_size h = {};

	(void)ctx;

	GP_JSON_OBJ_FOREACH_FILTER(json, val, &obj_filter, gp_widget_json_attrs) {
		switch (val->idx) {
		case H:
			switch (val->type) {
			case GP_JSON_INT:
				if (val->val_int < 0)
					gp_json_warn(json, "Size must be > 0!");
				else
					h.px = val->val_int;
			break;
			case GP_JSON_STR:
				if (gp_widget_size_units_parse(val->val_str, &h))
					gp_json_warn(json, "Invalid size!");
			break;
			default:
				gp_json_warn(json, "Invalid size type!");
			}
		break;
		case W:
			switch (val->type) {
			case GP_JSON_INT:
				if (val->val_int < 0)
					gp_json_warn(json, "Size must be > 0!");
				else
					w.px = val->val_int;
			break;
			case GP_JSON_STR:
				if (gp_widget_size_units_parse(val->val_str, &w))
					gp_json_warn(json, "Invalid size!");
			break;
			default:
				gp_json_warn(json, "Invalid size type!");
			}
		break;
		}
	}

	return gp_widget_pixmap_new(w, h, NULL, NULL);
}

struct gp_widget_ops gp_widget_pixmap_ops = {
	.min_w = min_w,
	.min_h = min_h,
	.render = render,
	.event = event,
	.from_json = json_to_pixmap,
	.id = "pixmap",
};

gp_widget *gp_widget_pixmap_new(gp_widget_size w, gp_widget_size h,
                                int (*on_event)(gp_widget_event *ev),
                                void *priv)
{
	gp_widget *ret;

	ret = gp_widget_new(GP_WIDGET_PIXMAP, GP_WIDGET_CLASS_NONE, sizeof(struct pixmap_payload));
	if (!ret)
		return NULL;

	struct pixmap_payload *pixmap = GP_WIDGET_PAYLOAD(ret);

	ret->on_event = on_event;
	ret->priv = priv;
	pixmap->min_w = w;
	pixmap->min_h = h;
	pixmap->pixmap = NULL;

	return ret;
}

void gp_widget_pixmap_redraw(gp_widget *self,
                             gp_coord x, gp_coord y,
                             gp_size w, gp_size h)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_PIXMAP, );
	struct pixmap_payload *pixmap = GP_WIDGET_PAYLOAD(self);

	if (pixmap->redraw_all)
		return;

	gp_bbox box = gp_bbox_pack(x, y, w, h);
	if (pixmap->bbox_set) {
		pixmap->bbox = gp_bbox_merge(pixmap->bbox, box);
		goto exit;
	}

	pixmap->bbox_set = 1;
	pixmap->bbox = box;

	gp_widget_redraw(self);
exit:
	GP_DEBUG(2, "Schedulling partiall " GP_BBOX_FMT " pixmap (%p) update",
	         GP_BBOX_PARS(pixmap->bbox), self);
}

void gp_widget_pixmap_redraw_all(gp_widget *self)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_PIXMAP, );
	struct pixmap_payload *pixmap = GP_WIDGET_PAYLOAD(self);

	pixmap->redraw_all = 1;
	pixmap->bbox_set = 0;

	GP_DEBUG(2, "Schedulling pixmap (%p) update", self);

	gp_widget_redraw(self);
}

gp_pixmap *gp_widget_pixmap_get(gp_widget *self)
{

	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_PIXMAP, NULL);
	struct pixmap_payload *pixmap = GP_WIDGET_PAYLOAD(self);

	return pixmap->pixmap;
}

gp_pixmap *gp_widget_pixmap_set(gp_widget *self, gp_pixmap *pix)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_PIXMAP, NULL);
	struct pixmap_payload *pixmap = GP_WIDGET_PAYLOAD(self);
	gp_pixmap *ret = pixmap->pixmap;

	pixmap->pixmap = pix;

	return ret;
}
