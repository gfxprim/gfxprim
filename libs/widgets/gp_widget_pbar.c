//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2023 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>

#include <widgets/gp_widgets.h>
#include <widgets/gp_widget_ops.h>
#include <widgets/gp_widget_render.h>

struct gp_widget_pbar {
	uint64_t max;
	uint64_t val;
	int32_t step;
	enum gp_widget_pbar_unit unit;
};

static unsigned int pbar_min_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	(void)self;

	return 2 * ctx->padd + gp_text_max_width(ctx->font, 7);
}

static unsigned int pbar_min_h(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	(void)self;

	return 2 * ctx->padd + gp_text_ascent(ctx->font);
}

static unsigned int val_to_secs(uint64_t sec)
{
	return sec % 60;
}

static unsigned int val_to_mins(int64_t sec)
{
	return (sec/60) % 60;
}

static unsigned int val_to_hours(uint64_t sec)
{
	return (sec/3600) % 60;
}

static void pbar_render(gp_widget *self, const gp_offset *offset,
                        const gp_widget_render_ctx *ctx, int flags)
{
	struct gp_widget_pbar *pbar = GP_WIDGET_PAYLOAD(self);

	unsigned int x = self->x + offset->x;
	unsigned int y = self->y + offset->y;
	unsigned int w = self->w;
	unsigned int h = self->h;
	gp_pixel text_color = ctx->text_color;
	gp_pixel frame_color = gp_widget_frame_color(self, ctx, flags);

	if (gp_widget_is_disabled(self, flags))
		text_color = ctx->col_disabled;

	gp_widget_ops_blit(ctx, x, y, w, h);

	unsigned int wd = 1.00 * pbar->val * w / pbar->max;

	char buf[64] = {};
	uint64_t val = pbar->val;

	unsigned int hours, mins, secs;

	if (pbar->unit & GP_WIDGET_PBAR_INVERSE)
		val = pbar->max - val;

	switch (pbar->unit & GP_WIDGET_PBAR_UNIT_MASK) {
	case GP_WIDGET_PBAR_NONE:
		break;
	case GP_WIDGET_PBAR_PERCENTS:
		snprintf(buf, sizeof(buf), "%.2f%%",
		         100.00 * val / pbar->max);
	break;
	case GP_WIDGET_PBAR_SECONDS:
		hours = val_to_hours(val);
		mins = val_to_mins(val);
		secs = val_to_secs(val);
		if (hours) {
			snprintf(buf, sizeof(buf), "%uh %um %us",
			         hours, mins, secs);
		} else if (mins) {
			snprintf(buf, sizeof(buf), "%um %us",
			         mins, secs);
		} else {
			snprintf(buf, sizeof(buf), "%us", secs);
		}
	break;
	}

	gp_pixmap p;

	int is_1bpp = gp_pixel_size(ctx->pixel_type) == 1;
	int is_disabled = gp_widget_is_disabled(self, flags);
	gp_pixel pbar_color = is_disabled ? ctx->col_disabled : ctx->hl_color;

	gp_sub_pixmap(ctx->buf, &p, x, y, wd, h);
	if (p.w > 0) {
		gp_fill_rrect_xywh_focused(&p, 0, 0, w, h, ctx->bg_color,
		                           pbar_color, frame_color, self->focused);
		if (is_1bpp || is_disabled) {
			gp_print(&p, ctx->font, w/2, ctx->padd,
			         GP_ALIGN_CENTER | GP_VALIGN_BELOW | GP_TEXT_NOBG,
			         ctx->fg_color, ctx->hl_color, "%s", buf);
		}
	}

	gp_sub_pixmap(ctx->buf, &p, x+wd, y, w-wd, h);
	if (p.w > 0) {
		gp_fill_rrect_xywh_focused(&p, -wd, 0, w, h, ctx->bg_color,
		                           ctx->fg_color, frame_color, self->focused);
		if (is_1bpp || is_disabled) {
			gp_print(&p, ctx->font, w/2 -wd, ctx->padd,
			         GP_ALIGN_CENTER | GP_VALIGN_BELOW | GP_TEXT_NOBG,
			         ctx->text_color, ctx->fg_color, "%s", buf);
		}
	}

	if (!is_1bpp && !is_disabled) {
		gp_print(ctx->buf, ctx->font, x + w/2, y + ctx->padd,
		         GP_ALIGN_CENTER | GP_VALIGN_BELOW | GP_TEXT_NOBG,
		         text_color, ctx->fg_color, "%s", buf);
	}
}

static int check_val(uint64_t val, uint64_t max)
{
	if (val > max) {
		GP_WARN("Invalid progressbar value %"PRIu64" > max %"PRIu64, val, max);
		return 1;
	}

	return 0;
}

static int check_max(uint64_t max)
{
	if (max == 0) {
		GP_WARN("Invalid progressbar max %"PRIu64, max);
		return 1;
	}

	return 0;
}

enum keys {
	INVERSE,
	MAX,
	UNIT,
	VAL,
};

static const gp_json_obj_attr attrs[] = {
	GP_JSON_OBJ_ATTR("inverse", GP_JSON_BOOL),
	GP_JSON_OBJ_ATTR("max", GP_JSON_INT),
	GP_JSON_OBJ_ATTR("unit", GP_JSON_STR),
	GP_JSON_OBJ_ATTR("val", GP_JSON_INT),
};

static const gp_json_obj obj_filter = {
	.attrs = attrs,
	.attr_cnt = GP_ARRAY_SIZE(attrs),
};

static gp_widget *json_to_pbar(gp_json_reader *json, gp_json_val *val, gp_widget_json_ctx *ctx)
{
	double pbval = 0, max = 10000;
	enum gp_widget_pbar_unit unit = GP_WIDGET_PBAR_PERCENTS;
	int inverse = 0;

	(void)ctx;

	GP_JSON_OBJ_FOREACH_FILTER(json, val, &obj_filter, gp_widget_json_attrs) {
		switch (val->idx) {
		case INVERSE:
			inverse = val->val_bool;
		break;
		case MAX:
			max = val->val_float;
		break;
		case UNIT:
			if (!strcmp(val->val_str, "none"))
				unit = GP_WIDGET_PBAR_NONE;
			else if (!strcmp(val->val_str, "percents"))
				unit = GP_WIDGET_PBAR_PERCENTS;
			else if (!strcmp(val->val_str, "seconds"))
				unit = GP_WIDGET_PBAR_SECONDS;
			else
				gp_json_warn(json, "Invalid unit!");
		break;
		case VAL:
			pbval = val->val_float;
		break;
		}
	}

	if (check_max(max))
		max = 10000;

	if (check_val(pbval, max))
		pbval = 0;

	if (inverse)
		unit |= GP_WIDGET_PBAR_INVERSE;

	return gp_widget_pbar_new(pbval, max, unit);
}

static int pbar_val_add(gp_widget *self, float val)
{
	struct gp_widget_pbar *pbar = GP_WIDGET_PAYLOAD(self);

	if (val == 0)
		return 0;

	if (val > 0) {
		if (pbar->val + val > pbar->max) {
			if (pbar->val == pbar->max)
				return 0;
			pbar->val = pbar->max;
		} else {
			pbar->val += val;
		}
	}

	if (val < 0) {
		if (pbar->val + val < 0) {
			if (pbar->val == 0)
				return 0;
			pbar->val = 0;
		} else {
			pbar->val += val;
		}
	}

	gp_widget_send_widget_event(self, GP_WIDGET_PBAR_VAL_CHANGED);
	gp_widget_redraw(self);

	return 1;
}

static int pbar_val_set(gp_widget *self, float val)
{
	struct gp_widget_pbar *pbar = GP_WIDGET_PAYLOAD(self);

	if (pbar->val == val)
		return 0;

	pbar->val = val;

	gp_widget_send_widget_event(self, GP_WIDGET_PBAR_VAL_CHANGED);
	gp_widget_redraw(self);

	return 1;
}

static int pbar_coord_to_val(gp_widget *self, const gp_widget_render_ctx *ctx, int coord)
{
	struct gp_widget_pbar *pbar = GP_WIDGET_PAYLOAD(self);

	float new_val = pbar->max * (coord - ctx->fr_thick) / (self->w - 2*ctx->fr_thick);

	if (new_val < 0)
		new_val = 0;

	if (new_val > pbar->max)
		new_val = pbar->max;

	return pbar_val_set(self, new_val);
}

static int pbar_event(gp_widget *self, const gp_widget_render_ctx *ctx,
                      gp_event *ev)
{
	struct gp_widget_pbar *pbar = GP_WIDGET_PAYLOAD(self);

	switch (ev->type) {
	case GP_EV_ABS:
		if (ev->code == GP_EV_ABS_POS)
			return pbar_coord_to_val(self, ctx, ev->st->cursor_x);
	break;
	case GP_EV_REL:
                if (ev->code == GP_EV_REL_WHEEL)
			return pbar_val_add(self, ev->val);
		if (ev->code == GP_EV_REL_POS && gp_ev_key_pressed(ev, GP_BTN_LEFT))
			return pbar_coord_to_val(self, ctx, ev->st->cursor_x);
	break;
	case GP_EV_KEY:
		if (ev->code == GP_EV_KEY_UP)
			return 0;

		switch (ev->val) {
		case GP_BTN_LEFT:
			return pbar_coord_to_val(self, ctx, ev->st->cursor_x);
		case GP_KEY_RIGHT:
			return pbar_val_add(self, pbar->step);
		case GP_KEY_LEFT:
			return pbar_val_add(self, -pbar->step);
		case GP_KEY_HOME:
			return pbar_val_set(self, 0);
		case GP_KEY_END:
			return pbar_val_set(self, pbar->max);
		}
	break;
	}

	return 0;
}

struct gp_widget_ops gp_widget_pbar_ops = {
	.min_w = pbar_min_w,
	.min_h = pbar_min_h,
	.render = pbar_render,
	.event = pbar_event,
	.from_json = json_to_pbar,
	.id = "pbar",
};

gp_widget *gp_widget_pbar_new(uint64_t val, uint64_t max, enum gp_widget_pbar_unit unit)
{
	size_t size = sizeof(struct gp_widget_pbar);
	gp_widget *ret;

	if (check_val(val, max))
		val = 0;

	ret = gp_widget_new(GP_WIDGET_PROGRESSBAR, GP_WIDGET_CLASS_NONE, size);
	if (!ret)
		return NULL;

	struct gp_widget_pbar *pbar = GP_WIDGET_PAYLOAD(ret);

	pbar->val = val;
	pbar->unit = unit;
	pbar->max = max;
	pbar->step = 1;

	ret->no_events = 1;

	return ret;
}

void gp_widget_pbar_val_set(gp_widget *self, uint64_t val)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_PROGRESSBAR, );
	struct gp_widget_pbar *pbar = GP_WIDGET_PAYLOAD(self);

	GP_DEBUG(3, "Setting widget (%p) progressbar val %"PRIu64" -> %"PRIu64,
		 self, pbar->val, val);

	if (check_val(val, pbar->max))
		return;

	if (pbar->val == val)
		return;

	pbar->val = val;

	gp_widget_redraw(self);
}

void gp_widget_pbar_max_set(gp_widget *self, uint64_t max)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_PROGRESSBAR, );
	struct gp_widget_pbar *pbar = GP_WIDGET_PAYLOAD(self);

	GP_DEBUG(3, "Setting widget (%p) progressbar max %"PRIu64" -> %"PRIu64,
		 self, pbar->max, max);

	pbar->val = GP_MIN(pbar->val, max);
	pbar->max = max;

	gp_widget_redraw(self);
}

uint64_t gp_widget_pbar_val_get(gp_widget *self)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_PROGRESSBAR, 0);
	struct gp_widget_pbar *pbar = GP_WIDGET_PAYLOAD(self);

	return pbar->val;
}

uint64_t gp_widget_pbar_max_get(gp_widget *self)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_PROGRESSBAR, 0);
	struct gp_widget_pbar *pbar = GP_WIDGET_PAYLOAD(self);

	return pbar->max;
}
