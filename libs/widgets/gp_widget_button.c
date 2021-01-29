//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>
#include <json-c/json.h>

#include <gp_widgets.h>
#include <gp_widget_ops.h>
#include <gp_widget_render.h>
#include <gp_widget_json.h>

static unsigned int min_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	unsigned int ret = 2 * ctx->padd;

	if (self->b->label)
		ret += gp_text_width(ctx->font, self->b->label);

	if (self->b->type & GP_BUTTON_TYPE_MASK)
		ret += GP_ODD_UP(gp_text_ascent(ctx->font));

	if (self->b->label && self->b->type & GP_BUTTON_TYPE_MASK)
		ret += gp_text_width(ctx->font, " ");

	return ret;
}

static unsigned int min_h(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	(void)self;

	return 2 * ctx->padd + GP_ODD_UP(gp_text_ascent(ctx->font));
}

static void magnifying_glass(gp_pixmap *buf, gp_coord cx, gp_coord cy,
                             gp_size sym_r, gp_size asc_half, gp_pixel color)
{
	unsigned int i;

	gp_fill_ring(buf, cx, cy, sym_r, sym_r + asc_half/4, color);

	for (i = 0; i <= asc_half/4; i++)
		gp_line(buf, cx + sym_r, cy+sym_r-i, cx+sym_r+asc_half/2, cy+sym_r+asc_half/2-i, color);

	gp_line(buf, cx + sym_r-1, cy+sym_r-1, cx+sym_r+asc_half/2, cy+sym_r+asc_half/2, color);

	for (i = 0; i <= asc_half/4; i++)
		gp_line(buf, cx + sym_r-i, cy+sym_r, cx+sym_r+asc_half/2-i, cy+sym_r+asc_half/2, color);
}

static void render(gp_widget *self, const gp_offset *offset,
                   const gp_widget_render_ctx *ctx, int flags)
{
	unsigned int x = self->x + offset->x;
	unsigned int y = self->y + offset->y;
	unsigned int w = self->w;
	unsigned int h = self->h;

	(void)flags;

	gp_widget_ops_blit(ctx, x, y, w, h);

	gp_pixel bg_color = self->b->val ? ctx->bg_color : ctx->fg_color;
	gp_pixel fr_color = self->focused ? ctx->sel_color : ctx->text_color;

	gp_fill_rrect_xywh(ctx->buf, x, y, w, h, ctx->bg_color, bg_color, fr_color);

	unsigned int asc = gp_text_ascent(ctx->font);
	unsigned int asc_half = asc/2;
	unsigned int sym_r = GP_EVEN_UP(9 * asc_half/10);
	unsigned int cx = x + self->w/2;
	unsigned int cy = y + self->h/2;
	unsigned int i;

	if (self->b->label) {
		unsigned int tcx = cx, len;
		unsigned int spc = gp_text_width(ctx->font, " ");

		if (self->b->type & GP_BUTTON_TYPE_MASK) {
			if (self->b->type & GP_BUTTON_TEXT_LEFT)
				tcx -= asc/2 + spc/2;
			else
				tcx += asc/2 + spc/2;
		}

		len = gp_text(ctx->buf, ctx->font,
			tcx, cy-(asc-asc_half), GP_ALIGN_CENTER|GP_VALIGN_BELOW,
			ctx->text_color, bg_color, self->b->label);

		if (self->b->type & GP_BUTTON_TEXT_LEFT)
			cx += len/2 + spc/2;
		else
			cx -= len/2 + spc/2;
	}

	switch (self->b->type & GP_BUTTON_TYPE_MASK) {
	case GP_BUTTON_LABEL:
	break;
	case GP_BUTTON_OK:
		gp_fill_tetragon(ctx->buf, cx + sym_r, cy - sym_r,
		                 cx, cy + sym_r,
				 cx, cy + sym_r - 2 * (asc_half/3),
				 cx + sym_r - asc_half/3, cy - sym_r,
				 ctx->accept_color);

		gp_fill_tetragon(ctx->buf, cx - sym_r, cy,
				 cx, cy + sym_r,
				 cx, cy + sym_r - (asc_half/3),
				 cx - sym_r + (asc_half/3), cy,
				 ctx->accept_color);
	break;
	case GP_BUTTON_CANCEL:
		for (i = 0; i <= asc_half/4; i++) {
			gp_line(ctx->buf, cx - sym_r + i, cy - sym_r,
			        cx + sym_r, cy + sym_r - i, ctx->alert_color);
			gp_line(ctx->buf, cx - sym_r, cy - sym_r + i,
				cx + sym_r - i, cy + sym_r, ctx->alert_color);
			gp_line(ctx->buf, cx - sym_r + i, cy + sym_r,
			        cx + sym_r, cy - sym_r + i, ctx->alert_color);
			gp_line(ctx->buf, cx - sym_r, cy + sym_r - i,
			        cx + sym_r - i, cy - sym_r, ctx->alert_color);
		}
	break;
	case GP_BUTTON_OPEN:
		gp_fill_rect(ctx->buf, cx - sym_r, cy - sym_r,
		             cx, cy - sym_r/2, ctx->sel_color);

		for (i = 0; i <= asc_half/4; i++) {
			gp_rect(ctx->buf, cx - sym_r + i, cy - sym_r/2 + i,
			        cx + sym_r - i, cy + sym_r - i, ctx->sel_color);
		}
	break;
	case GP_BUTTON_SAVE:
		for (i = 0; i <= asc_half/4; i++) {
			gp_rect(ctx->buf, cx - sym_r + i, cy - sym_r + i,
			        cx + sym_r - i, cy + sym_r - i, ctx->sel_color);
		}

		gp_fill_rect(ctx->buf, cx - sym_r/2, cy - sym_r + asc_half/2,
		             cx + sym_r/2, cy - sym_r, ctx->text_color);

		gp_hline(ctx->buf, cx - sym_r/2, cx + sym_r/2, cy + sym_r/3, ctx->sel_color);
		if (sym_r/2 - sym_r/3 > 1)
			gp_hline(ctx->buf, cx - sym_r/2, cx + sym_r/2, cy, ctx->sel_color);
	break;
	case GP_BUTTON_PLAY:
	case GP_BUTTON_NEXT:
		gp_symbol(ctx->buf, cx+asc_half/4, cy, sym_r, sym_r,
		          GP_TRIANGLE_RIGHT, ctx->text_color);
	break;
	case GP_BUTTON_PREV:
		gp_symbol(ctx->buf, cx-asc_half/4, cy, sym_r, sym_r,
		          GP_TRIANGLE_LEFT, ctx->text_color);
	break;
	case GP_BUTTON_FIRST:
		gp_symbol(ctx->buf, cx, cy, sym_r, sym_r,
		          GP_TRIANGLE_LEFT, ctx->text_color);
		gp_fill_rect(ctx->buf, cx-sym_r+asc_half/5, cy-sym_r,
		             cx-sym_r, cy+sym_r, ctx->text_color);
	break;
	case GP_BUTTON_LAST:
		gp_symbol(ctx->buf, cx, cy, sym_r, sym_r,
		          GP_TRIANGLE_RIGHT, ctx->text_color);
		gp_fill_rect(ctx->buf, cx+sym_r-asc_half/5, cy-sym_r,
		             cx+sym_r, cy+sym_r, ctx->text_color);
	break;
	case GP_BUTTON_TOP:
		gp_symbol(ctx->buf, cx, cy, sym_r, sym_r,
		          GP_TRIANGLE_UP, ctx->text_color);
		gp_fill_rect(ctx->buf, cx+sym_r, cy-sym_r+asc_half/5,
		             cx-sym_r, cy-sym_r, ctx->text_color);
	break;
	case GP_BUTTON_BOTTOM:
		gp_symbol(ctx->buf, cx, cy, sym_r, sym_r,
		          GP_TRIANGLE_DOWN, ctx->text_color);
		gp_fill_rect(ctx->buf, cx+sym_r, cy+sym_r-asc_half/5,
		             cx-sym_r, cy+sym_r, ctx->text_color);
	break;
	case GP_BUTTON_PAUSE:
		i = sym_r;

		gp_fill_rect(ctx->buf, cx - i,
		             cy - i, cx - asc/5, cy + i,
		             ctx->text_color);
		gp_fill_rect(ctx->buf, cx + i,
		             cy - i, cx + asc/5, cy + i,
		             ctx->text_color);
	break;
	case GP_BUTTON_STOP:
		i = sym_r - asc_half/8;

		gp_fill_rect(ctx->buf,
		             cx - i, cy - i,
		             cx + i, cy + i,
		             ctx->text_color);
	break;
	case GP_BUTTON_REC:
		gp_fill_circle(ctx->buf, cx, cy, sym_r - asc_half/8, ctx->alert_color);
	break;
	case GP_BUTTON_FFORWARD:
		gp_symbol(ctx->buf, cx-(sym_r - sym_r/2)+asc_half/4, cy,
		          sym_r/2, sym_r,
		          GP_TRIANGLE_RIGHT, ctx->text_color);
		gp_symbol(ctx->buf, cx+(sym_r - sym_r/2)+asc_half/4, cy,
		          sym_r/2, sym_r,
		          GP_TRIANGLE_RIGHT, ctx->text_color);
	break;
	case GP_BUTTON_FBACKWARD:
		gp_symbol(ctx->buf, cx-(sym_r - sym_r/2)-asc_half/4, cy,
		          sym_r/2, sym_r,
		          GP_TRIANGLE_LEFT, ctx->text_color);
		gp_symbol(ctx->buf, cx+(sym_r - sym_r/2)-asc_half/4, cy,
		          sym_r/2, sym_r,
		          GP_TRIANGLE_LEFT, ctx->text_color);
	break;
	case GP_BUTTON_UP:
		gp_symbol(ctx->buf, cx, cy-asc_half/5,
		          sym_r, sym_r-sym_r/4,
		          GP_TRIANGLE_UP, ctx->text_color);
	break;
	case GP_BUTTON_DOWN:
		gp_symbol(ctx->buf, cx, cy+asc_half/5,
		          sym_r, sym_r-sym_r/4,
		          GP_TRIANGLE_DOWN, ctx->text_color);
	break;
	case GP_BUTTON_LEFT:
		gp_symbol(ctx->buf, cx-asc_half/5, cy,
		          sym_r - sym_r/4, sym_r,
		          GP_TRIANGLE_LEFT, ctx->text_color);
	break;
	case GP_BUTTON_RIGHT:
		gp_symbol(ctx->buf, cx+asc_half/5, cy,
		          sym_r - sym_r/4, sym_r,
		          GP_TRIANGLE_RIGHT, ctx->text_color);
	break;
	case GP_BUTTON_ADD:
		gp_fill_rect(ctx->buf, cx - asc/8,
		             cy - sym_r, cx + asc/8, cy + sym_r,
		             ctx->text_color);
	/* fallthrough */
	case GP_BUTTON_REM:
		gp_fill_rect(ctx->buf, cx - sym_r,
		             cy - asc/8, cx + sym_r, cy + asc/8,
		             ctx->text_color);
	break;
	case GP_BUTTON_ZOOM_IN:
		gp_fill_rect(ctx->buf, cx - asc/14, cy - sym_r/2,
		             cx + asc/14, cy + sym_r/2,
		             ctx->text_color);
	/* fallthrough */
	case GP_BUTTON_ZOOM_OUT:
		gp_fill_rect(ctx->buf, cx - sym_r/2, cy - asc/14,
		             cx + sym_r/2, cy + asc/14,
		             ctx->text_color);

		magnifying_glass(ctx->buf, cx, cy, sym_r, asc_half, ctx->text_color);
	break;
	case GP_BUTTON_ZOOM_FIT:
	for (i = 0; i < asc_half/4; i++)
		gp_rect(ctx->buf, cx-sym_r/2+i, cy-sym_r/2+i, cx+sym_r/2-i, cy+sym_r/2-i, ctx->text_color);

	/* fallthrough */
	case GP_BUTTON_ZOOM_NORMAL:
		magnifying_glass(ctx->buf, cx, cy, sym_r, asc_half, ctx->text_color);
	break;
	}

	if (self->b->val)
		gp_widget_render_timer(self, 0, 200);
}

static void set(gp_widget *self)
{
	if (self->b->val)
		return;

	self->b->val = 1;

	gp_widget_redraw(self);

	gp_widget_send_widget_event(self, 0);
}

static void click(gp_widget *self, gp_event *ev)
{
	if (ev->cursor_x > self->w)
		return;

	if (ev->cursor_y > self->h)
		return;

	set(self);
}

static int event(gp_widget *self, const gp_widget_render_ctx *ctx, gp_event *ev)
{
	(void) ctx;

	switch (ev->type) {
	case GP_EV_KEY:
		if (ev->code == GP_EV_KEY_UP)
			return 0;

		switch (ev->val) {
		case GP_KEY_SPACE:
		case GP_KEY_ENTER:
			set(self);
			return 1;
		case GP_BTN_LEFT:
		case GP_BTN_PEN:
			click(self, ev);
			return 1;
		}
	break;
	case GP_EV_TMR:
		self->b->val = 0;
		gp_widget_redraw(self);
		return 1;
	break;
	}

	return 0;
}

static struct btn_type_names {
	const char *name;
	enum gp_widget_button_type type;
} type_names[] = {
	{"ok", GP_BUTTON_OK | GP_BUTTON_TEXT_RIGHT},
	{"cancel", GP_BUTTON_CANCEL | GP_BUTTON_TEXT_RIGHT},
	{"open", GP_BUTTON_OPEN | GP_BUTTON_TEXT_RIGHT},
	{"save", GP_BUTTON_SAVE | GP_BUTTON_TEXT_RIGHT},
	{"prev", GP_BUTTON_PREV | GP_BUTTON_TEXT_RIGHT},
	{"next", GP_BUTTON_NEXT | GP_BUTTON_TEXT_LEFT},
	{"first", GP_BUTTON_FIRST | GP_BUTTON_TEXT_RIGHT},
	{"last", GP_BUTTON_LAST | GP_BUTTON_TEXT_LEFT},
	{"top", GP_BUTTON_TOP | GP_BUTTON_TEXT_RIGHT},
	{"bottom", GP_BUTTON_BOTTOM | GP_BUTTON_TEXT_RIGHT},
	{"play", GP_BUTTON_PLAY | GP_BUTTON_TEXT_RIGHT},
	{"pause", GP_BUTTON_PAUSE | GP_BUTTON_TEXT_RIGHT},
	{"stop", GP_BUTTON_STOP | GP_BUTTON_TEXT_RIGHT},
	{"rec", GP_BUTTON_REC | GP_BUTTON_TEXT_RIGHT},
	{"fforward", GP_BUTTON_FFORWARD | GP_BUTTON_TEXT_RIGHT},
	{"fbackward", GP_BUTTON_FBACKWARD | GP_BUTTON_TEXT_RIGHT},
	{"up", GP_BUTTON_UP | GP_BUTTON_TEXT_RIGHT},
	{"down", GP_BUTTON_DOWN | GP_BUTTON_TEXT_RIGHT},
	{"left", GP_BUTTON_LEFT | GP_BUTTON_TEXT_RIGHT},
	{"right", GP_BUTTON_RIGHT | GP_BUTTON_TEXT_LEFT},
	{"add", GP_BUTTON_ADD | GP_BUTTON_TEXT_RIGHT},
	{"rem", GP_BUTTON_REM | GP_BUTTON_TEXT_RIGHT},
	{"zoom_in", GP_BUTTON_ZOOM_IN | GP_BUTTON_TEXT_RIGHT},
	{"zoom_out", GP_BUTTON_ZOOM_OUT | GP_BUTTON_TEXT_RIGHT},
	{"zoom_fit", GP_BUTTON_ZOOM_FIT | GP_BUTTON_TEXT_RIGHT},
	{"zoom_normal", GP_BUTTON_ZOOM_NORMAL | GP_BUTTON_TEXT_RIGHT},
};

static enum gp_widget_button_type type_from_str(const char *string)
{
	unsigned int i;

	if (!string)
		return GP_BUTTON_LABEL;

	for (i = 0; i < GP_ARRAY_SIZE(type_names); i++) {
		if (!strcmp(string, type_names[i].name))
			return type_names[i].type;
	}

	return -1;
}

static enum gp_widget_button_type align_for_type(enum gp_widget_button_type type)
{
	unsigned int i;

	if (!type)
		return 0;

	for (i = 0; i < GP_ARRAY_SIZE(type_names); i++) {
		if (type == (type_names[i].type & GP_BUTTON_TYPE_MASK))
			return type_names[i].type;
	}

	return type;
}

static gp_widget *json_to_button(json_object *json, void **uids)
{
	const char *label = NULL;
	const char *type = NULL;
	const char *text_align = NULL;
	enum gp_widget_button_type t = GP_BUTTON_LABEL;

	(void)uids;

	json_object_object_foreach(json, key, val) {
		if (!strcmp(key, "label"))
			label = json_object_get_string(val);
		else if (!strcmp(key, "btype"))
			type = json_object_get_string(val);
		else if (!strcmp(key, "text_align"))
			text_align = json_object_get_string(val);
		else
			GP_WARN("Invalid button key '%s'", key);
	}

	if (type)
		t = type_from_str(type);

	if (t < 0) {
		GP_WARN("Invalid button type '%s'", type);
		t = GP_BUTTON_LABEL;
	}

	if (text_align) {
		t &= ~GP_BUTTON_ALIGN_MASK;
		if (!strcmp(text_align, "left"))
			t |= GP_BUTTON_TEXT_LEFT;
		else if (!strcmp(text_align, "right"))
			t |= GP_BUTTON_TEXT_RIGHT;
		else
			GP_WARN("Invalid text align '%s'", text_align);
	}

	if (t == GP_BUTTON_LABEL && !label) {
		GP_WARN("Labeled button without label!");
		label = "Unlabeled";
	}

	return gp_widget_button_new(label, t, NULL, NULL);
}

struct gp_widget_ops gp_widget_button_ops = {
	.min_w = min_w,
	.min_h = min_h,
	.render = render,
	.event = event,
	.from_json = json_to_button,
	.id = "button",
};

gp_widget *gp_widget_button_new(const char *label,
                                enum gp_widget_button_type type,
                                int (*on_event)(gp_widget_event *ev),
                                void *priv)
{
	gp_widget *ret;
	size_t size = sizeof(struct gp_widget_bool);

	if (label)
		size += strlen(label) + 1;

	ret = gp_widget_new(GP_WIDGET_BUTTON, GP_WIDGET_CLASS_NONE, size);
	if (!ret)
		return NULL;

	if (label) {
		ret->button->label = ret->button->payload;
		strcpy(ret->button->payload, label);
	}

	ret->button->type = align_for_type(type);

	gp_widget_event_handler_set(ret, on_event, priv);

	return ret;
}
