//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2023 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>

#include <widgets/gp_widgets.h>
#include <widgets/gp_widget_ops.h>
#include <widgets/gp_widget_render.h>
#include <widgets/gp_widget_json.h>

struct button_priv {
	int type;
	int set;
	char *label;
	char data[];
};

#define BUTTON_PRIV(self) ((struct button_priv *)GP_WIDGET_PAYLOAD(self))

static unsigned int min_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	unsigned int ret = 2 * ctx->padd;
	const gp_text_style *font = gp_widget_focused_font(ctx, 1);
	struct button_priv *b = BUTTON_PRIV(self);

	if (b->label)
		ret += gp_text_wbbox(font, b->label);

	if (b->type & GP_BUTTON_TYPE_MASK)
		ret += GP_ODD_UP(gp_text_ascent(font));

	if (b->label && b->type & GP_BUTTON_TYPE_MASK)
		ret += gp_text_wbbox(font, " ");

	return ret;
}

static unsigned int min_h(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	(void)self;

	return 2 * ctx->padd + GP_ODD_UP(gp_text_ascent(ctx->font));
}

static void cross(gp_pixmap *buf, gp_coord cx, gp_coord cy,
                  gp_size sym_r, gp_size thickness, gp_pixel color)
{
	unsigned int i;

	for (i = 0; i <= thickness; i++) {
		gp_line(buf, cx - sym_r + i, cy - sym_r,
		             cx + sym_r, cy + sym_r - i, color);
		gp_line(buf, cx - sym_r, cy - sym_r + i,
			     cx + sym_r - i, cy + sym_r, color);
		gp_line(buf, cx - sym_r + i, cy + sym_r,
		             cx + sym_r, cy - sym_r + i, color);
		gp_line(buf, cx - sym_r, cy + sym_r - i,
		             cx + sym_r - i, cy - sym_r, color);
	}
}

static void backspace(gp_pixmap *pix, gp_coord cx, gp_coord cy,
                      gp_coord sym_r, gp_pixel fg, gp_pixel bg)
{
	gp_coord poly[] = {
		sym_r, -sym_r/2-sym_r/4,
		sym_r, sym_r/2+sym_r/4,
		-sym_r/2+sym_r/8, sym_r/2+sym_r/4,
		-sym_r, 0,
		-sym_r/2+sym_r/8, -sym_r/2-sym_r/4,
	};

	gp_fill_polygon(pix, cx, cy, GP_ARRAY_SIZE(poly)/2, poly, fg);
	cross(pix, cx+sym_r/4, cy, sym_r/3, sym_r/7, bg);
}

static void render(gp_widget *self, const gp_offset *offset,
                   const gp_widget_render_ctx *ctx, int flags)
{
	struct button_priv *b = BUTTON_PRIV(self);
	unsigned int x = self->x + offset->x;
	unsigned int y = self->y + offset->y;
	unsigned int w = self->w;
	unsigned int h = self->h;

	const gp_text_style *font = gp_widget_focused_font(ctx, self->focused);
	gp_pixel text_color = gp_widget_text_color(self, ctx, flags);
	gp_pixel fr_color = gp_widget_frame_color(self, ctx, flags);
	gp_pixel bg_color = b->set ? ctx->bg_color : ctx->fg_color;

	gp_widget_ops_blit(ctx, x, y, w, h);

	gp_fill_rrect_xywh(ctx->buf, x, y, w, h, ctx->bg_color, bg_color, fr_color);

	unsigned int asc = gp_text_ascent(ctx->font);
	unsigned int asc_half = asc/2;
	unsigned int sym_r = GP_EVEN_UP(9 * asc_half/10);
	unsigned int cx = x + self->w/2;
	unsigned int cy = y + self->h/2;
	unsigned int i, spc;

	if (b->label) {
		unsigned int tcx = cx, len;
		unsigned int spc = gp_text_wbbox(ctx->font, " ");

		if (b->type & GP_BUTTON_TYPE_MASK) {
			if (b->type & GP_BUTTON_TEXT_LEFT)
				tcx -= asc/2 + spc/2;
			else
				tcx += asc/2 + spc/2;
		}

		len = gp_text(ctx->buf, font,
			tcx, cy-(asc-asc_half), GP_ALIGN_CENTER|GP_VALIGN_BELOW,
			text_color, bg_color, b->label);

		if (b->type & GP_BUTTON_TEXT_LEFT)
			cx += len/2 + spc/2;
		else
			cx -= len/2 + spc/2;
	}

	gp_size sw = GP_ODD_UP(gp_text_ascent(ctx->font));
	gp_size sh = sw;

	gp_size sx = cx - sw/2;
	gp_coord sy = cy - (sh+1)/2;

	switch (b->type & GP_BUTTON_TYPE_MASK) {
	case GP_BUTTON_LABEL:
	break;
	case GP_BUTTON_YES:
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
	case GP_BUTTON_NO:
	case GP_BUTTON_CANCEL:
		gp_widget_stock_render(ctx->buf, GP_WIDGET_STOCK_CLOSE,
		                       sx, sy, sw, sh,
		                       bg_color, ctx);
	break;
	case GP_BUTTON_OPEN:
		gp_widget_stock_render(ctx->buf, GP_WIDGET_STOCK_DIR,
		                       sx, sy, sw, sh,
		                       bg_color, ctx);
	break;
	case GP_BUTTON_NEW_DIR:
		gp_widget_stock_render(ctx->buf, GP_WIDGET_STOCK_NEW_DIR,
		                       sx, sy, sw, sh,
		                       bg_color, ctx);
	break;
	case GP_BUTTON_SAVE:
		gp_widget_stock_render(ctx->buf, GP_WIDGET_STOCK_SAVE,
		                       sx, sy, sw, sh,
				       bg_color, ctx);
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
	case GP_BUTTON_STEP_FORWARD:
		spc = GP_MAX(1u, sym_r/6);

		gp_fill_rect(ctx->buf, cx-spc, cy-sym_r,
		             cx-spc-sym_r/4, cy+sym_r, ctx->text_color);

		gp_fill_rect(ctx->buf, cx-3*spc-sym_r/4, cy-sym_r,
		             cx-3*spc-2*(sym_r/4), cy+sym_r, ctx->text_color);

		gp_symbol(ctx->buf, cx+(sym_r - sym_r/2)+spc, cy,
		          sym_r/2, sym_r,
		          GP_TRIANGLE_RIGHT, ctx->text_color);
	break;
	case GP_BUTTON_STEP_BACKWARD:
		spc = GP_MAX(1u, sym_r/6);

		gp_fill_rect(ctx->buf, cx+spc, cy-sym_r,
		             cx+spc+sym_r/4, cy+sym_r, ctx->text_color);

		gp_fill_rect(ctx->buf, cx+3*spc+sym_r/4, cy-sym_r,
		             cx+3*spc+2*(sym_r/4), cy+sym_r, ctx->text_color);

		gp_symbol(ctx->buf, cx-(sym_r - sym_r/2)-spc, cy,
		          sym_r/2, sym_r,
		          GP_TRIANGLE_LEFT, ctx->text_color);
	break;
	case GP_BUTTON_RANDOMIZE:
		gp_fill_rrect_xywh(ctx->buf, cx-sym_r, cy-sym_r, 2*sym_r+1, 2*sym_r+1, ctx->fg_color, ctx->bg_color, ctx->text_color);
		i = (sym_r+1)/3;
		gp_fill_circle(ctx->buf, cx-i, cy-i, sym_r/5, ctx->text_color);
		gp_fill_circle(ctx->buf, cx+i, cy+i, sym_r/5, ctx->text_color);
		gp_fill_circle(ctx->buf, cx-i, cy+i, sym_r/5, ctx->text_color);
		gp_fill_circle(ctx->buf, cx+i, cy-i, sym_r/5, ctx->text_color);
	break;
	case GP_BUTTON_UP:
		gp_widget_stock_render(ctx->buf, GP_WIDGET_STOCK_ARROW_UP,
		                       sx, sy, sw, sh, bg_color, ctx);
	break;
	case GP_BUTTON_DOWN:
		gp_widget_stock_render(ctx->buf, GP_WIDGET_STOCK_ARROW_DOWN,
		                       sx, sy, sw, sh, bg_color, ctx);
	break;
	case GP_BUTTON_LEFT:
		gp_widget_stock_render(ctx->buf, GP_WIDGET_STOCK_ARROW_LEFT,
		                       sx, sy, sw, sh, bg_color, ctx);
	break;
	case GP_BUTTON_RIGHT:
		gp_widget_stock_render(ctx->buf, GP_WIDGET_STOCK_ARROW_RIGHT,
		                       sx, sy, sw, sh, bg_color, ctx);
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
	case GP_BUTTON_CLEAR:
		cross(ctx->buf, cx, cy, sym_r-sym_r/6, 2 * (asc/8)-asc/16, ctx->text_color);
	break;
	case GP_BUTTON_BACKSPACE:
		backspace(ctx->buf, cx-sym_r/5, cy, sym_r+sym_r/2, ctx->text_color, ctx->bg_color);
	break;
	case GP_BUTTON_ZOOM_IN:
		gp_widget_stock_render(ctx->buf, GP_WIDGET_STOCK_ZOOM_IN,
		                       sx, sy, sw, sh, bg_color, ctx);
	break;
	case GP_BUTTON_ZOOM_OUT:
		gp_widget_stock_render(ctx->buf, GP_WIDGET_STOCK_ZOOM_OUT,
		                       sx, sy, sw, sh, bg_color, ctx);
	break;
	case GP_BUTTON_ZOOM_FIT:
		gp_widget_stock_render(ctx->buf, GP_WIDGET_STOCK_ZOOM_FIT,
		                       sx, sy, sw, sh, bg_color, ctx);
	break;
	case GP_BUTTON_ZOOM_NORMAL:
		gp_widget_stock_render(ctx->buf, GP_WIDGET_STOCK_ZOOM,
		                       sx, sy, sw, sh, bg_color, ctx);
	break;
	case GP_BUTTON_ROTATE_CW:
		gp_widget_stock_render(ctx->buf, GP_WIDGET_STOCK_ROTATE_CW,
		                       sx, sy, sw, sh, bg_color, ctx);
	break;
	case GP_BUTTON_ROTATE_CCW:
		gp_widget_stock_render(ctx->buf, GP_WIDGET_STOCK_ROTATE_CCW,
		                       sx, sy, sw, sh, bg_color, ctx);
	break;
	case GP_BUTTON_SETTINGS:
		gp_widget_stock_render(ctx->buf, GP_WIDGET_STOCK_SETTINGS,
		                       sx-1, sy, sw+2, sh+2, bg_color, ctx);
	break;
	case GP_BUTTON_HOME:
		gp_widget_stock_render(ctx->buf, GP_WIDGET_STOCK_HOME,
				       sx, sy, sw, sh, bg_color, ctx);
	break;
	case GP_BUTTON_DOWNLOAD:
		gp_widget_stock_render(ctx->buf, GP_WIDGET_STOCK_ARROW_DOWN,
		                       sx, sy+sh/8, sw, sh, bg_color, ctx);
	break;
	}

	if (b->set)
		gp_widget_render_timer(self, 0, ctx->feedback_ms);
}

static void set(gp_widget *self)
{
	struct button_priv *b = BUTTON_PRIV(self);

	if (b->set)
		return;

	b->set = 1;

	gp_widget_redraw(self);

	gp_widget_send_widget_event(self, GP_WIDGET_BUTTON_TRIGGER);
}

static void click(gp_widget *self, gp_event *ev)
{
	if (ev->st->cursor_x > self->w)
		return;

	if (ev->st->cursor_y > self->h)
		return;

	set(self);
}

static int event(gp_widget *self, const gp_widget_render_ctx *ctx, gp_event *ev)
{
	(void) ctx;
	struct button_priv *b = BUTTON_PRIV(self);

	switch (ev->type) {
	case GP_EV_KEY:
		if (ev->code == GP_EV_KEY_UP)
			return 0;

		if (gp_widget_key_mod_pressed(ev))
			return 0;

		switch (ev->val) {
		case GP_KEY_SPACE:
		case GP_KEY_ENTER:
			set(self);
			return 1;
		case GP_BTN_LEFT:
		case GP_BTN_TOUCH:
			click(self, ev);
			return 1;
		}
	break;
	case GP_EV_TMR:
		b->set = 0;
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
	{"yes", GP_BUTTON_YES | GP_BUTTON_TEXT_RIGHT},
	{"no", GP_BUTTON_NO | GP_BUTTON_TEXT_RIGHT},
	{"open", GP_BUTTON_OPEN | GP_BUTTON_TEXT_RIGHT},
	{"save", GP_BUTTON_SAVE | GP_BUTTON_TEXT_RIGHT},
	{"new_dir", GP_BUTTON_NEW_DIR | GP_BUTTON_TEXT_RIGHT},
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
	{"step_forward", GP_BUTTON_STEP_FORWARD | GP_BUTTON_TEXT_RIGHT},
	{"step_backward", GP_BUTTON_STEP_BACKWARD | GP_BUTTON_TEXT_RIGHT},
	{"randomize", GP_BUTTON_RANDOMIZE | GP_BUTTON_TEXT_RIGHT},
	{"up", GP_BUTTON_UP | GP_BUTTON_TEXT_RIGHT},
	{"down", GP_BUTTON_DOWN | GP_BUTTON_TEXT_RIGHT},
	{"left", GP_BUTTON_LEFT | GP_BUTTON_TEXT_RIGHT},
	{"right", GP_BUTTON_RIGHT | GP_BUTTON_TEXT_LEFT},
	{"add", GP_BUTTON_ADD | GP_BUTTON_TEXT_RIGHT},
	{"rem", GP_BUTTON_REM | GP_BUTTON_TEXT_RIGHT},
	{"clear", GP_BUTTON_CLEAR | GP_BUTTON_TEXT_RIGHT},
	{"backspace", GP_BUTTON_BACKSPACE | GP_BUTTON_TEXT_RIGHT},
	{"zoom_in", GP_BUTTON_ZOOM_IN | GP_BUTTON_TEXT_RIGHT},
	{"zoom_out", GP_BUTTON_ZOOM_OUT | GP_BUTTON_TEXT_RIGHT},
	{"zoom_fit", GP_BUTTON_ZOOM_FIT | GP_BUTTON_TEXT_RIGHT},
	{"zoom_normal", GP_BUTTON_ZOOM_NORMAL | GP_BUTTON_TEXT_RIGHT},
	{"rotate_cw", GP_BUTTON_ROTATE_CW | GP_BUTTON_TEXT_RIGHT},
	{"rotate_ccw", GP_BUTTON_ROTATE_CCW | GP_BUTTON_TEXT_RIGHT},
	{"settings", GP_BUTTON_SETTINGS | GP_BUTTON_TEXT_RIGHT},
	{"home", GP_BUTTON_HOME | GP_BUTTON_TEXT_RIGHT},
	{"download", GP_BUTTON_DOWNLOAD | GP_BUTTON_TEXT_RIGHT},
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

enum keys {
	BTYPE,
	LABEL,
	TEXT_ALIGN,
};

static const gp_json_obj_attr attrs[] = {
	GP_JSON_OBJ_ATTR("btype", GP_JSON_STR),
	GP_JSON_OBJ_ATTR("label", GP_JSON_STR),
	GP_JSON_OBJ_ATTR("text_align", GP_JSON_STR),
};

static const gp_json_obj obj_filter = {
	.attrs = attrs,
	.attr_cnt = GP_ARRAY_SIZE(attrs),
};

static gp_widget *json_to_button(gp_json_reader *json, gp_json_val *val, gp_widget_json_ctx *ctx)
{
	char *label = NULL;
	int t = GP_BUTTON_LABEL;

	(void)ctx;

	GP_JSON_OBJ_FOREACH_FILTER(json, val, &obj_filter, gp_widget_json_attrs) {
		switch (val->idx) {
		case BTYPE:
			t = type_from_str(val->val_str);
			if (t < 0) {
				gp_json_warn(json, "Invalid button type!");
				t = GP_BUTTON_LABEL;
			}
		break;
		case LABEL:
			label = strdup(val->val_str);
		break;
		case TEXT_ALIGN:
			t &= ~GP_BUTTON_ALIGN_MASK;

			if (!strcmp(val->val_str, "left"))
				t |= GP_BUTTON_TEXT_LEFT;
			else if (!strcmp(val->val_str, "right"))
				t |= GP_BUTTON_TEXT_RIGHT;
			else
				gp_json_warn(json, "Invalid text align!");
		break;
		}
	}

	if (t == GP_BUTTON_LABEL && !label) {
		gp_json_warn(json, "Labeled button without label!");
		return NULL;
	}

	gp_widget *ret = gp_widget_button_new(label, t);
	free(label);
	return ret;
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
                                enum gp_widget_button_type type)
{
	gp_widget *ret;
	size_t size = sizeof(struct gp_widget_bool) + sizeof(struct button_priv);

	if (label)
		size += strlen(label) + 1;

	ret = gp_widget_new(GP_WIDGET_BUTTON, GP_WIDGET_CLASS_NONE, size);
	if (!ret)
		return NULL;

	struct button_priv *b = BUTTON_PRIV(ret);

	if (label) {
		b->label = b->data;
		strcpy(b->label, label);
	}

	b->type = align_for_type(type);

	return ret;
}

enum gp_widget_button_type gp_widget_button_type_get(gp_widget *self)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_BUTTON, 0);
	struct button_priv *b = BUTTON_PRIV(self);

	return GP_BUTTON_TYPE_MASK & b->type;
}

const char *gp_widget_button_label_get(gp_widget *self)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_BUTTON, 0);
	struct button_priv *b = BUTTON_PRIV(self);

	return b->label;
}

void gp_widget_button_type_set(gp_widget *self, enum gp_widget_button_type type)
{
	struct button_priv *b = BUTTON_PRIV(self);

	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_BUTTON, );

	b->type = type;

	gp_widget_resize(self);
	gp_widget_redraw(self);
}
