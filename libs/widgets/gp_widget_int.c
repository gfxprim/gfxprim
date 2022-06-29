//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2022 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>

#include <widgets/gp_widgets.h>
#include <widgets/gp_widget_ops.h>
#include <widgets/gp_widget_render.h>

static int check_val(int min, int max, int val)
{
	if (val < min || val > max) {
		GP_WARN("Val %i outside of Min %i Max %i", val, min, max);
		return 1;
	}

	return 0;
}

static int check_min_max(int min, int max)
{
	if (min > max) {
		GP_WARN("Min %i > Max %i", min, max);
		return 1;
	}

	return 0;
}

static gp_widget *widget_int_new(enum gp_widget_type type,
                                 int min, int max, int val)
{
	gp_widget *ret;

	if (check_min_max(min, max))
		return NULL;

	if (check_val(min, max, val))
		return NULL;

	ret = gp_widget_new(type, GP_WIDGET_CLASS_INT, sizeof(struct gp_widget_int));
	if (!ret)
		return NULL;

	ret->i->min = min;
	ret->i->max = max;
	ret->i->val = val;

	return ret;
}

void gp_widget_int_set(gp_widget *self, int min, int max, int val)
{
	GP_WIDGET_CLASS_ASSERT(self, GP_WIDGET_CLASS_INT, );

	if (min > max) {
		GP_WARN("Widget %s (%p) new min %i > new max %i",
			gp_widget_type_name(self->type), self, min, max);
		return;
	}

	if (val < min || val > max) {
		GP_WARN("Widget %s (%p) val %i outside of min %i, max %i",
			gp_widget_type_name(self->type), self, val, min, max);
	}

	if (self->i->min == min && self->i->max == max && self->i->val == val)
		return;

	self->i->min = min;
	self->i->max = max;
	self->i->val = val;

	//TODO: Resize on bounds change?
	gp_widget_redraw(self);
}

void gp_widget_int_val_set(gp_widget *self, int val)
{
	GP_WIDGET_CLASS_ASSERT(self, GP_WIDGET_CLASS_INT, );

	if (check_val(self->i->min, self->i->max, val))
		return;

	self->i->val = val;
	gp_widget_redraw(self);

	//TODO: On event?
}


void gp_widget_int_max_set(gp_widget *self, int max)
{
	GP_WIDGET_CLASS_ASSERT(self, GP_WIDGET_CLASS_INT, );

	if (max < self->i->min) {
		GP_WARN("Widget %s (%p) max (%i) < min (%i)",
			gp_widget_type_name(self->type), self,
			max, self->i->min);
		return;
	}

	self->i->max = max;

	if (self->i->val > max)
		self->i->val = max;

	gp_widget_redraw(self);
}

void gp_widget_int_min_set(gp_widget *self, int min)
{
	GP_WIDGET_CLASS_ASSERT(self, GP_WIDGET_CLASS_INT, );

	if (min > self->i->max) {
		GP_WARN("Widget %s (%p) min (%i) > max (%i)",
			gp_widget_type_name(self->type), self,
			min, self->i->max);
		return;
	}

	self->i->min = min;

	if (self->i->val < min)
		self->i->val = min;

	gp_widget_redraw(self);
}

enum keys {
	DIR,
	MAX,
	MIN,
	VAL,
};

static const gp_json_obj_attr attrs[] = {
	GP_JSON_OBJ_ATTR("dir", GP_JSON_STR),
	GP_JSON_OBJ_ATTR("max", GP_JSON_INT),
	GP_JSON_OBJ_ATTR("min", GP_JSON_INT),
	GP_JSON_OBJ_ATTR("val", GP_JSON_INT),
};

static const gp_json_obj obj_filter = {
	.attrs = attrs,
	.attr_cnt = GP_ARRAY_SIZE(attrs),
};

static gp_widget *json_to_int(enum gp_widget_type type, gp_json_buf *json,
                              gp_json_val *val, gp_widget_json_ctx *ctx)
{
	int min = 0, max = 0, ival = 0, dir = 0, val_set = 0;
	gp_widget *ret;

	(void)ctx;

	GP_JSON_OBJ_FILTER(json, val, &obj_filter, gp_widget_json_attrs) {
		switch (val->idx) {
		case DIR:
			if (!strcmp(val->val_str, "horiz"))
				dir = GP_WIDGET_HORIZ;
			else if (!strcmp(val->val_str, "vert"))
				dir = GP_WIDGET_VERT;
			else
				gp_json_warn(json, "Expected one of 'horiz' or 'vert'!");
		break;
		case MAX:
			max = val->val_int;
		break;
		case MIN:
			min = val->val_int;
		break;
		case VAL:
			ival = val->val_int;
		break;
		}
	}

	if (!val_set)
		ival = min;

	if (check_min_max(min, max))
		return NULL;

	if (check_val(min, max, ival))
		return NULL;

	ret = gp_widget_new(type, GP_WIDGET_CLASS_INT, sizeof(struct gp_widget_int));
	if (!ret)
		return NULL;

	ret->i->min = min;
	ret->i->max = max;
	ret->i->val = ival;
	ret->i->dir = dir;

	return ret;
}

static unsigned int spin_buttons_width(const gp_widget_render_ctx *ctx)
{
	return GP_EVEN_UP(((3 * ctx->padd + gp_text_ascent(ctx->font))/2));
}

static unsigned int spin_min_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	unsigned int min_digits = snprintf(NULL, 0, "%i", self->spin->min);
	unsigned int max_digits = snprintf(NULL, 0, "%i", self->spin->max);

	unsigned int ret = 2 * ctx->padd;

	ret += gp_text_max_width_chars(ctx->font, "-0123456789",
			               GP_MAX(min_digits, max_digits));

	ret += spin_buttons_width(ctx);

	return ret;
}

static unsigned int spin_min_h(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	(void)self;

	return 2 * ctx->padd + gp_text_ascent(ctx->font);
}

static void spin_render(gp_widget *self, const gp_offset *offset,
                        const gp_widget_render_ctx *ctx, int flags)
{
	unsigned int x = self->x + offset->x;
	unsigned int y = self->y + offset->y;
	unsigned int w = self->w;
	unsigned int h = self->h;
	unsigned int s = spin_buttons_width(ctx);
	unsigned int sx = (gp_text_ascent(ctx->font)/2 + ctx->padd)/3;
	unsigned int sy = (gp_text_ascent(ctx->font)/2 + ctx->padd)/5;

	(void)flags;

	gp_widget_ops_blit(ctx, x, y, w, h);

	gp_pixel color = self->focused ? ctx->sel_color : ctx->text_color;

	if (self->spin->alert) {
		color = ctx->alert_color;
		gp_widget_render_timer(self, GP_TIMER_RESCHEDULE, 500);
	}

	gp_fill_rrect_xywh(ctx->buf, x, y, w, h,
	                   ctx->bg_color, ctx->fg_color, color);

	gp_print(ctx->buf, ctx->font, x + w - s - ctx->padd, y + ctx->padd,
		 GP_ALIGN_LEFT | GP_VALIGN_BELOW,
		 ctx->text_color, ctx->bg_color, "%i", self->spin->val);


	gp_coord rx = x + w - s;

	gp_vline_xyh(ctx->buf, rx-1, y, h, color);
	gp_hline_xyw(ctx->buf, rx, y + h/2, s, color);

	gp_symbol(ctx->buf, x + w - s/2 - 1, y + h/4, sx, sy, GP_TRIANGLE_UP, ctx->text_color);
	gp_symbol(ctx->buf, x + w - s/2 - 1, y + (3*h)/4, sx, sy, GP_TRIANGLE_DOWN, ctx->text_color);
}

static void schedule_alert(gp_widget *self)
{
	self->spin->alert = 1;
	gp_widget_redraw(self);
}

static void spin_inc(gp_widget *self)
{
	if (self->spin->val >= self->spin->max) {
		schedule_alert(self);
		return;
	}

	self->spin->val++;

	gp_widget_send_widget_event(self, 0);
	gp_widget_redraw(self);
}

static void spin_dec(gp_widget *self)
{
	if (self->spin->val <= self->spin->min) {
		schedule_alert(self);
		return;
	}

	self->spin->val--;

	gp_widget_send_widget_event(self, 0);
	gp_widget_redraw(self);
}

static void spin_click(gp_widget *self, const gp_widget_render_ctx *ctx, gp_event *ev)
{
	unsigned int s = spin_buttons_width(ctx);
	unsigned int min_x = self->w - s;
	unsigned int max_x = self->w;
	unsigned int max_y = self->h;
	unsigned int mid_y = max_y / 2;

	if (ev->st->cursor_x < min_x || ev->st->cursor_x > max_x)
		return;

	if (ev->st->cursor_y > max_y)
		return;

	if (ev->st->cursor_y < mid_y)
		spin_inc(self);
	else
		spin_dec(self);
}

static void spin_min(gp_widget *self)
{
	self->spin->val = self->spin->min;
	gp_widget_redraw(self);
}

static void spin_max(gp_widget *self)
{
	self->spin->val = self->spin->max;
	gp_widget_redraw(self);
}

static int spin_event(gp_widget *self, const gp_widget_render_ctx *ctx, gp_event *ev)
{
	switch (ev->type) {
	case GP_EV_KEY:
		if (ev->code == GP_EV_KEY_UP)
			return 0;

		if (gp_widget_key_mod_pressed(ev))
			return 0;

		switch (ev->val) {
		case GP_BTN_PEN:
		case GP_BTN_LEFT:
			spin_click(self, ctx, ev);
			return 1;
		//TODO: Inc by 10 with Shift
		case GP_KEY_UP:
			spin_inc(self);
			return 1;
		case GP_KEY_DOWN:
			spin_dec(self);
			return 1;
		case GP_KEY_HOME:
			spin_min(self);
			return 1;
		case GP_KEY_END:
			spin_max(self);
			return 1;
		}
	break;
	case GP_EV_TMR:
		self->spin->alert = 0;
		gp_widget_redraw(self);
		return 1;
	break;
	}

	return 0;
}

static gp_widget *json_to_spin(gp_json_buf *json, gp_json_val *val, gp_widget_json_ctx *ctx)
{
	return json_to_int(GP_WIDGET_SPINNER, json, val, ctx);
}

struct gp_widget_ops gp_widget_spinner_ops = {
	.min_w = spin_min_w,
	.min_h = spin_min_h,
	.render = spin_render,
	.event = spin_event,
	.from_json = json_to_spin,
	.id = "spinner",
};

gp_widget *gp_widget_spinner_new(int min, int max, int val)
{
	return widget_int_new(GP_WIDGET_SPINNER, min, max, val);
}

/* slider */

static unsigned int ssteps(gp_widget *self)
{
	return self->slider->max - self->slider->min;
}

static unsigned int slider_min_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	unsigned int steps = ssteps(self);
	unsigned int asc = gp_text_ascent(ctx->font) + 4;

	switch (self->slider->dir) {
	case GP_WIDGET_HORIZ:
		return asc + steps;
	case GP_WIDGET_VERT:
		return asc;
	}

	return 0;
}

static unsigned int slider_min_h(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	unsigned int steps = ssteps(self);
	unsigned int asc = gp_text_ascent(ctx->font) + 4;

	switch (self->slider->dir) {
	case GP_WIDGET_HORIZ:
		return asc;
	case GP_WIDGET_VERT:
		return asc + steps;
	}

	return 0;
}

static void slider_render(gp_widget *self, const gp_offset *offset,
                          const gp_widget_render_ctx *ctx, int flags)
{
	unsigned int x = self->x + offset->x;
	unsigned int y = self->y + offset->y;
	unsigned int w = self->w;
	unsigned int h = self->h;

	unsigned int steps = ssteps(self);
	unsigned int asc = gp_text_ascent(ctx->font);
	int val = GP_ABS(self->slider->val);

	(void)flags;

	gp_widget_ops_blit(ctx, x, y, w, h);

	gp_pixel fr_color = self->focused ? ctx->sel_color : ctx->text_color;

	gp_fill_rrect_xywh(ctx->buf, x, y, w, h, ctx->bg_color, ctx->fg_color, fr_color);

	switch (self->slider->dir) {
	case GP_WIDGET_HORIZ:
		w = asc;
		x = x + (self->w - w - 4) * val / steps + 2;
		y += 2;
		h -= 4;
	break;
	case GP_WIDGET_VERT:
		//TODO!
		val = self->i->max - val;
		h = asc;
		y = y + (self->h - h - 4) * val / steps + 2;
		x += 2;
		w -= 4;
	break;
	}

	gp_fill_rrect_xywh(ctx->buf, x, y, w, h, ctx->fg_color, ctx->bg_color, ctx->text_color);
}

static int coord_to_val(gp_widget *self, int coord,
                        int ascent, unsigned int size)
{
	int steps = ssteps(self);
	int div = (size - ascent - 4);

	return ((coord - 2 - ascent/2) * steps + div/2) / div;
}

static void slider_set_val(gp_widget *self, unsigned int ascent, gp_event *ev)
{
	int val = 0;
	int coord;

	if (ev->type == GP_EV_REL && !gp_event_key_pressed(ev, GP_BTN_LEFT))
		return;

	if (ev->st->cursor_x > self->w)
		return;

	if (ev->st->cursor_y > self->h)
		return;

	switch (self->slider->dir) {
	case GP_WIDGET_HORIZ:
		coord = (int)ev->st->cursor_x;
		val = coord_to_val(self, coord, ascent, self->w);
	break;
	case GP_WIDGET_VERT:
		coord = (int)self->h - ((int)ev->st->cursor_y);
		val = coord_to_val(self, coord, ascent, self->h);
	break;
	}

	if (val > self->i->max)
		val = self->i->max;

	if (val < self->i->min)
		val = self->i->min;

	self->i->val = val;

	gp_widget_send_widget_event(self, 0);

	gp_widget_redraw(self);
}

static int slider_event(gp_widget *self, const gp_widget_render_ctx *ctx, gp_event *ev)
{
	unsigned int asc = gp_text_ascent(ctx->font);

	if (gp_widget_key_mod_pressed(ev))
		return 0;

	switch (ev->type) {
	case GP_EV_REL:
		if (ev->code == GP_EV_REL_WHEEL) {
			if (ev->val < 0)
				spin_dec(self);
			else
				spin_inc(self);
			return 1;
		}
		/* fallthrough */
	case GP_EV_ABS:
		slider_set_val(self, asc, ev);
	break;
	case GP_EV_KEY:
		if (ev->code == GP_EV_KEY_UP)
			return 0;

		switch (ev->val) {
		case GP_BTN_PEN:
		case GP_BTN_LEFT:
			slider_set_val(self, asc, ev);
			return 1;
		//TODO: Inc by 10 with Shift
		case GP_KEY_UP:
		case GP_KEY_RIGHT:
			spin_inc(self);
			return 1;
		case GP_KEY_DOWN:
		case GP_KEY_LEFT:
			spin_dec(self);
			return 1;
		case GP_KEY_HOME:
			spin_min(self);
			return 1;
		case GP_KEY_END:
			spin_max(self);
			return 1;
		}
	break;
	}

	return 0;
}

static gp_widget *json_to_slider(gp_json_buf *json, gp_json_val *val, gp_widget_json_ctx *ctx)
{
	return json_to_int(GP_WIDGET_SLIDER, json, val, ctx);
}

struct gp_widget_ops gp_widget_slider_ops = {
	.min_w = slider_min_w,
	.min_h = slider_min_h,
	.render = slider_render,
	.event = slider_event,
	.from_json = json_to_slider,
	.id = "slider",
};

gp_widget *gp_widget_slider_new(int min, int max, int val, int dir)
{
	gp_widget *ret;

	ret = widget_int_new(GP_WIDGET_SLIDER, min, max, val);
	if (!ret)
		return NULL;

	ret->i->dir = dir;

	return ret;
}
