//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2023 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>

#include <widgets/gp_widgets.h>
#include <widgets/gp_widget_ops.h>
#include <widgets/gp_widget_render.h>

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

static unsigned int val_to_secs(float sec)
{
	return (unsigned int)(sec + 0.5) % 60;
}

static unsigned int val_to_mins(float sec)
{
	return ((unsigned int)sec/60) % 60;
}

static unsigned int val_to_hours(float sec)
{
	return ((unsigned int)sec/3600) % 60;
}

static void pbar_render(gp_widget *self, const gp_offset *offset,
                        const gp_widget_render_ctx *ctx, int flags)
{
	unsigned int x = self->x + offset->x;
	unsigned int y = self->y + offset->y;
	unsigned int w = self->w;
	unsigned int h = self->h;
	gp_pixel text_color = ctx->text_color;

	if (gp_widget_is_disabled(self, flags))
		text_color = ctx->col_disabled;

	gp_widget_ops_blit(ctx, x, y, w, h);

	unsigned int wd = self->pbar->val * w / self->pbar->max;

	char buf[64] = {};
	float val = self->pbar->val;

	unsigned int hours, mins, secs;

	if (self->pbar->unit & GP_WIDGET_PBAR_INVERSE)
		val = self->pbar->max - val;

	switch (self->pbar->unit & GP_WIDGET_PBAR_TMASK) {
	case GP_WIDGET_PBAR_NONE:
		break;
	case GP_WIDGET_PBAR_PERCENTS:
		snprintf(buf, sizeof(buf), "%.2f%%",
		         100 * val / self->pbar->max);
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

	gp_sub_pixmap(ctx->buf, &p, x, y, wd, h);
	if (p.w > 0) {
		gp_fill_rrect_xywh(&p, 0, 0, w, h, ctx->bg_color,
		                   ctx->hl_color, text_color);
		if (is_1bpp) {
			gp_print(&p, ctx->font, w/2, ctx->padd,
			         GP_ALIGN_CENTER | GP_VALIGN_BELOW | GP_TEXT_NOBG,
			         ctx->fg_color, ctx->hl_color, "%s", buf);
		}
	}

	gp_sub_pixmap(ctx->buf, &p, x+wd, y, w-wd, h);
	if (p.w > 0) {
		gp_fill_rrect_xywh(&p, -wd, 0, w, h, ctx->bg_color,
		                   ctx->fg_color, text_color);
		if (is_1bpp) {
			gp_print(&p, ctx->font, w/2 -wd, ctx->padd,
			         GP_ALIGN_CENTER | GP_VALIGN_BELOW | GP_TEXT_NOBG,
			         ctx->text_color, ctx->fg_color, "%s", buf);
		}
	}

	if (!is_1bpp) {
		gp_print(ctx->buf, ctx->font, x + w/2, y + ctx->padd,
		         GP_ALIGN_CENTER | GP_VALIGN_BELOW | GP_TEXT_NOBG,
		         text_color, ctx->fg_color, "%s", buf);
	}
}

static int check_val(double val, double max)
{
	if (val < 0 || val > max) {
		GP_WARN("Invalid progressbar value %lf", val);
		return 1;
	}

	return 0;
}

static int check_max(double max)
{
	if (max <= 0) {
		GP_WARN("Invalid progressbar max %lf", max);
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
	GP_JSON_OBJ_ATTR("max", GP_JSON_FLOAT),
	GP_JSON_OBJ_ATTR("unit", GP_JSON_STR),
	GP_JSON_OBJ_ATTR("val", GP_JSON_FLOAT),
};

static const gp_json_obj obj_filter = {
	.attrs = attrs,
	.attr_cnt = GP_ARRAY_SIZE(attrs),
};

static gp_widget *json_to_pbar(gp_json_reader *json, gp_json_val *val, gp_widget_json_ctx *ctx)
{
	double pbval = 0, max = 100;
	enum gp_widget_pbar_unit unit = GP_WIDGET_PBAR_PERCENTS;
	int inverse = 0;

	(void)ctx;

	GP_JSON_OBJ_FILTER(json, val, &obj_filter, gp_widget_json_attrs) {
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
		max = 100;

	if (check_val(pbval, max))
		pbval = 0;

	if (inverse)
		unit |= GP_WIDGET_PBAR_INVERSE;

	return gp_widget_pbar_new(pbval, max, unit);
}

struct gp_widget_ops gp_widget_pbar_ops = {
	.min_w = pbar_min_w,
	.min_h = pbar_min_h,
	.render = pbar_render,
	.from_json = json_to_pbar,
	.id = "pbar",
};

gp_widget *gp_widget_pbar_new(float val, float max, enum gp_widget_pbar_unit unit)
{
	size_t size = sizeof(struct gp_widget_pbar);
	gp_widget *ret;

	if (check_val(val, max))
		val = 0;

	ret = gp_widget_new(GP_WIDGET_PROGRESSBAR, GP_WIDGET_CLASS_NONE, size);
	if (!ret)
		return NULL;

	ret->pbar->val = val;
	ret->pbar->unit = unit;
	ret->pbar->max = max;

	return ret;
}

void gp_widget_pbar_set(gp_widget *self, float val)
{
	GP_WIDGET_ASSERT(self, GP_WIDGET_PROGRESSBAR, );

	GP_DEBUG(3, "Setting widget (%p) progressbar val '%.2f' -> '%.2f'",
		 self, self->pbar->val, val);

	if (check_val(val, self->pbar->max))
		return;

	self->pbar->val = val;

	//TODO: check when we should redraw
	gp_widget_redraw(self);
}

void gp_widget_pbar_set_max(gp_widget *self, float max)
{
	GP_WIDGET_ASSERT(self, GP_WIDGET_PROGRESSBAR, );

	GP_DEBUG(3, "Setting widget (%p) progressbar max '%.2f' -> '%.2f'",
		 self, self->pbar->max, max);

	if (check_max(max))
		return;

	self->pbar->val = GP_MIN(self->pbar->val, max);
	self->pbar->max = max;

	gp_widget_redraw(self);
}
