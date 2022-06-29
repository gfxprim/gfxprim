//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>
#include <ctype.h>

#include <utils/gp_vec_str.h>

#include <widgets/gp_widgets.h>
#include <widgets/gp_widget_ops.h>
#include <widgets/gp_widget_render.h>

static unsigned int min_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	unsigned int ret = 2 * ctx->padd;
	const char *filter = self->tbox->filter;
	size_t text_len = self->tbox->size;
	const gp_text_style *font = gp_widget_tattr_font(self->tbox->tattr, ctx);

	if (filter)
		ret += gp_text_max_width_chars(font, filter, text_len);
	else
		ret += gp_text_avg_width(font, text_len);

	return ret;
}

static unsigned int min_h(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	const gp_text_style *font = gp_widget_tattr_font(self->tbox->tattr, ctx);

	return 2 * ctx->padd + gp_text_ascent(font);
}

static const char *hidden_str(const char *buf)
{
	static const char s[] = "********************************************";
	unsigned int len = gp_utf8_strlen(buf);

	if (len >= sizeof(s) - 1)
		return s;

	return s + sizeof(s) - len - 1;
}

static int is_sel(gp_widget *self)
{
	return self->tbox->sel_len;
}

static int is_hidden(struct gp_widget_tbox *tbox)
{
	return tbox->type == GP_WIDGET_TBOX_HIDDEN;
}

static const char *tbox_visible_str(struct gp_widget_tbox *tbox)
{
	if (is_hidden(tbox))
		return hidden_str(tbox->buf);

	return tbox->buf;
}

static void render(gp_widget *self, const gp_offset *offset,
                   const gp_widget_render_ctx *ctx, int flags)
{
	struct gp_widget_tbox *tbox = self->tbox;
	const gp_text_style *font = gp_widget_tattr_font(self->tbox->tattr, ctx);
	unsigned int x = self->x + offset->x;
	unsigned int y = self->y + offset->y;
	unsigned int w = self->w;
	unsigned int h = self->h;
	const char *str = tbox_visible_str(tbox);
	(void)flags;

	gp_widget_ops_blit(ctx, x, y, w, h);

	gp_pixel color = self->focused ? ctx->sel_color : ctx->text_color;

	if (tbox->alert) {
		color = ctx->alert_color;
		gp_widget_render_timer(self, GP_TIMER_RESCHEDULE, 500);
	}

	gp_fill_rrect_xywh(ctx->buf, x, y, w, h, ctx->bg_color, ctx->fg_color, color);

	size_t left = GP_MIN(tbox->off_left, tbox->cur_pos);
	size_t right = gp_vec_strlen(tbox->buf);

	while (gp_text_wbbox_len(font, str+left, right - left) > self->w - 2 * ctx->padd) {
		if (right > tbox->cur_pos)
			right--;
		else
			left++;
	}

	tbox->off_left = left;

	gp_coord cy = y + ctx->padd + (gp_text_ascent(font)+1)/2;
	gp_coord s = ctx->padd/4;

	if (left) {
		gp_coord cx = x + ctx->padd/2;

		gp_line(ctx->buf, cx-s, cy, cx, cy-s, ctx->text_color);
		gp_line(ctx->buf, cx-s, cy, cx, cy+s, ctx->text_color);
	}

	if (right < gp_vec_strlen(tbox->buf)) {
		gp_coord cx = x + w - 1 - ctx->padd/2;

		gp_line(ctx->buf, cx+s, cy, cx, cy-s, ctx->text_color);
		gp_line(ctx->buf, cx+s, cy, cx, cy+s, ctx->text_color);
	}

	if (self->focused && !is_sel(self)) {
		unsigned int cursor_x = x + ctx->padd;
		cursor_x += gp_text_wbbox_len(font, str + left,
		                              tbox->cur_pos - left);
		gp_vline_xyh(ctx->buf, cursor_x, y + ctx->padd,
			     gp_text_ascent(font), ctx->text_color);
	}

	str += left;
	gp_text_ext(ctx->buf, font,
		    x + ctx->padd, y + ctx->padd,
		    GP_ALIGN_RIGHT|GP_VALIGN_BELOW,
		    ctx->text_color, ctx->bg_color, str, right - left);


	if (!tbox->sel_len)
		return;

	size_t sel_left = GP_MAX(tbox->sel_off, left);
	size_t sel_right = GP_MIN(tbox->sel_off + tbox->sel_len, right);
	size_t sel_len = sel_right - sel_left;

	if (sel_left >= sel_right)
		return;

	gp_coord sel_x_off = x + ctx->padd + gp_text_width_len(font, GP_TEXT_LEN_ADVANCE, str, sel_left - left);

	str += sel_left - left;

	gp_fill_rect_xywh(ctx->buf, sel_x_off, y + ctx->padd,
	                  gp_text_wbbox_len(font, str, sel_len),
	                  gp_text_height(font), ctx->sel_color);

	gp_text_ext(ctx->buf, font, sel_x_off, y + ctx->padd,
	            GP_ALIGN_RIGHT | GP_VALIGN_BELOW,
	            ctx->text_color, ctx->sel_color, str,
		    sel_right - sel_left);
}

static void schedule_alert(gp_widget *self)
{
	self->tbox->alert = 1;
	gp_widget_redraw(self);
}

static void clear_alert(gp_widget *self)
{
	if (self->tbox->alert) {
		gp_widget_render_timer_cancel(self);
		self->tbox->alert = 0;
	}
}

static void send_edit_event(gp_widget *self)
{
	clear_alert(self);
	gp_widget_send_widget_event(self, GP_WIDGET_TBOX_EDIT);
}

static int filter(const char *filter, char ch)
{
	if (!filter)
		return 0;

	while (*filter) {
		if (*filter++ == ch)
			return 0;
	}

	return 1;
}

static int sel_clr(gp_widget *self)
{
	if (!is_sel(self))
		return 0;

	self->tbox->sel_len = 0;
	self->tbox->sel_off = 0;

	return 1;
}

static int sel_clr_left(gp_widget *self)
{
	if (!is_sel(self))
		return 0;

	self->tbox->cur_pos = self->tbox->sel_off;
	sel_clr(self);

	return 1;
}

static int sel_clr_right(gp_widget *self)
{
	if (!is_sel(self))
		return 0;

	self->tbox->cur_pos = self->tbox->sel_off + self->tbox->sel_len;
	sel_clr(self);

	return 1;
}

static int sel_del(gp_widget *self)
{
	if (!is_sel(self))
		return 0;

	gp_widget_tbox_del(self, self->tbox->sel_off,
	                   GP_SEEK_SET, self->tbox->sel_len);
	sel_clr(self);

	return 1;
}

static size_t buflen(gp_widget *self)
{
	return gp_vec_strlen(self->tbox->buf);
}

static int cursor_at_end(gp_widget *self)
{
	return self->tbox->cur_pos == gp_vec_strlen(self->tbox->buf);
}

static int cursor_at_home(gp_widget *self)
{
	return !self->tbox->cur_pos;
}

static int sel_all(gp_widget *self)
{
	size_t len = buflen(self);

	if (is_hidden(self->tbox))
		return 0;

	if (self->tbox->sel_len == len)
		return 0;

	self->tbox->sel_off = 0;
	self->tbox->sel_len = len;
	self->tbox->cur_pos = len;

	return 1;
}

static int is_all_sel(gp_widget *self)
{
	return self->tbox->sel_off == 0 &&
	       self->tbox->sel_len == buflen(self);
}


static int sel_end(gp_widget *self)
{
	struct gp_widget_tbox *tbox = self->tbox;

	if (is_hidden(tbox))
		return 0;

	if (cursor_at_end(self))
		return 0;

	if (!tbox->sel_len)
		tbox->sel_off = tbox->cur_pos;
	else if (tbox->sel_off == tbox->cur_pos)
		tbox->sel_off += tbox->sel_len;

	tbox->sel_len = buflen(self) - tbox->sel_off;

	return 1;
}

static int sel_right(gp_widget *self)
{
	struct gp_widget_tbox *tbox = self->tbox;

	if (is_hidden(tbox))
		return 0;

	if (!tbox->sel_len) {
		if (cursor_at_end(self))
			return 0;

		tbox->sel_off = tbox->cur_pos;
		tbox->sel_len = 1;
		return 1;
	}

	if (tbox->cur_pos == tbox->sel_off) {
		tbox->sel_len--;
		tbox->sel_off++;
		return 1;
	}

	if (tbox->sel_len + tbox->sel_off >= buflen(self))
		return 0;

	tbox->sel_len++;
	return 1;
}

static int sel_home(gp_widget *self)
{
	struct gp_widget_tbox *tbox = self->tbox;

	if (is_hidden(tbox))
		return 0;

	if (cursor_at_home(self))
		return 0;

	if (!tbox->sel_len)
		tbox->sel_len = tbox->cur_pos;
	else if (tbox->cur_pos == tbox->sel_off)
		tbox->sel_len += tbox->sel_off;
	else
		tbox->sel_len = tbox->sel_off;

	tbox->sel_off = 0;
	return 1;
}

static int sel_left(gp_widget *self)
{
	struct gp_widget_tbox *tbox = self->tbox;

	if (is_hidden(tbox))
		return 0;

	if (!tbox->sel_len) {
		if (!tbox->cur_pos)
			return 0;

		tbox->sel_off = tbox->cur_pos - 1;
		tbox->sel_len = 1;
		return 1;
	}

	if (tbox->cur_pos == tbox->sel_off) {
		if (!tbox->sel_off)
			return 0;
		tbox->sel_off--;
		tbox->sel_len++;
		return 1;
	}

	if (!tbox->sel_len)
		return 0;

	tbox->sel_len--;

	return 1;
}

static void utf_key(gp_widget *self, uint32_t ch)
{
	sel_del(self);

	if (self->tbox->max_size &&
	    gp_vec_strlen(self->tbox->buf) >= self->tbox->max_size) {
		schedule_alert(self);
		return;
	}

	int ret = gp_widget_send_widget_event(self, GP_WIDGET_TBOX_FILTER, (long)ch);

	if (ret || filter(self->tbox->filter, ch)) {
		schedule_alert(self);
		return;
	}

	char *tmp = gp_vec_ins_utf8(self->tbox->buf, self->tbox->cur_pos, ch);
	if (!tmp)
		return;

	self->tbox->buf = tmp;
	self->tbox->cur_pos += gp_utf8_bytes(ch);

	send_edit_event(self);

	gp_widget_redraw(self);
}

static void key_backspace(gp_widget *self)
{
	if (sel_del(self))
		goto ret;

	if (self->tbox->cur_pos <= 0) {
		schedule_alert(self);
		return;
	}

	self->tbox->cur_pos--;
	self->tbox->buf = gp_vec_strdel(self->tbox->buf, self->tbox->cur_pos, 1);
ret:
	send_edit_event(self);
	gp_widget_redraw(self);
}

static void key_delete(gp_widget *self)
{
	if (sel_del(self))
		goto ret;

	if (cursor_at_end(self)) {
		schedule_alert(self);
		return;
	}

	self->tbox->buf = gp_vec_strdel(self->tbox->buf, self->tbox->cur_pos, 1);
ret:
	send_edit_event(self);
	gp_widget_redraw(self);
}

static void key_left(gp_widget *self, int shift)
{
	if (!shift && sel_clr_left(self))
		goto redraw;

	if (cursor_at_home(self))
		goto exit;

	if (shift)
		sel_left(self);

	self->tbox->cur_pos--;

redraw:
	gp_widget_redraw(self);
exit:
	clear_alert(self);
}

static void key_right(gp_widget *self, int shift)
{
	if (!shift && sel_clr_right(self))
		goto redraw;

	if (cursor_at_end(self))
		goto exit;

	if (shift)
		sel_right(self);

	self->tbox->cur_pos++;

redraw:
	gp_widget_redraw(self);
exit:
	clear_alert(self);
}

static void key_home(gp_widget *self, int shift)
{
	if (!shift && sel_clr(self))
		goto exit;

	if (self->tbox->cur_pos == 0)
		return;

	if (shift && !sel_home(self))
		return;

exit:
	self->tbox->cur_pos = 0;
	gp_widget_redraw(self);
	clear_alert(self);
}

static void key_end(gp_widget *self, int shift)
{
	if (!shift && sel_clr(self))
		goto exit;

	if (cursor_at_end(self))
		return;

	if (shift && !sel_end(self))
		return;

exit:
	self->tbox->cur_pos = gp_vec_strlen(self->tbox->buf);
	gp_widget_redraw(self);
	clear_alert(self);
}

static void selection_to_clipboard(gp_widget *self)
{
	struct gp_widget_tbox *tbox = self->tbox;

	if (!is_sel(self))
		return;

	gp_widgets_clipboard_set(tbox->buf + tbox->sel_off, tbox->sel_len);
}

static void clipboard_event(gp_widget *self)
{
	char *clip;

	sel_del(self);

	clip = gp_widgets_clipboard_get();
	if (!clip)
		return;

	gp_widget_tbox_ins(self, 0, GP_SEEK_CUR, clip);
	free(clip);
}

static int is_delim(struct gp_widget_tbox *tbox, char c)
{
	if (tbox->delim)
		return !filter(tbox->delim, c);

	return isspace(c);
}

static void sel_substr_cycle(gp_widget *self, size_t cur_pos, int cur_save_restore)
{
	size_t left = cur_pos, right = cur_pos;
	struct gp_widget_tbox *tbox = self->tbox;

	if (is_hidden(tbox))
		return;

	if (is_all_sel(self)) {
		sel_clr(self);

		if (cur_save_restore)
			tbox->cur_pos = tbox->cur_pos_saved;
		else
			tbox->cur_pos = cur_pos;

		return;
	}

	if (is_sel(self)) {
		sel_all(self);
		return;
	}

	while (left > 0 && !is_delim(tbox, tbox->buf[left-1]))
		left--;

	while (tbox->buf[right] && !is_delim(tbox, tbox->buf[right]))
		right++;

	if (cur_save_restore)
		tbox->cur_pos_saved = tbox->cur_pos;

	tbox->cur_pos = right;
	tbox->sel_off = left;
	tbox->sel_len = right - left;
}

static int mouse_click(gp_widget *self, const gp_widget_render_ctx *ctx, int shift, gp_event *ev)
{
	struct gp_widget_tbox *tbox = self->tbox;
	const gp_text_style *font = gp_widget_tattr_font(tbox->tattr, ctx);
	const char *str = tbox_visible_str(tbox);
	size_t cur_pos;

	cur_pos = tbox->off_left + gp_text_cur_pos(font, str, ev->st->cursor_x - ctx->padd);

	if (gp_timeval_diff_ms(ev->time, tbox->last_click) < ctx->dclick_ms) {
		sel_substr_cycle(self, cur_pos, 0);
	} else {
		if (shift) {
			size_t sel_origin, sel_end;

			if (!is_sel(self)) {
				sel_origin = tbox->cur_pos;
			} else {
				if (tbox->cur_pos == tbox->sel_off)
					sel_origin = tbox->sel_off + tbox->sel_len;
				else
					sel_origin = tbox->sel_off;
			}

			sel_end = GP_MAX(sel_origin, cur_pos);

			tbox->sel_off = GP_MIN(sel_origin, cur_pos);
			tbox->sel_len = sel_end - tbox->sel_off;
		} else {
			sel_clr(self);
		}

		tbox->cur_pos = cur_pos;
	}


	tbox->click_cursor_x = ev->st->cursor_x;
	tbox->last_click = ev->time;
	gp_widget_redraw(self);

	return 1;
}

static int mouse_drag(gp_widget *self, const gp_widget_render_ctx *ctx, gp_event *ev)
{
	struct gp_widget_tbox *tbox = self->tbox;
	const gp_text_style *font = gp_widget_tattr_font(tbox->tattr, ctx);
	const char *str = tbox_visible_str(tbox);
	size_t cur_pos;

	//TODO: Get font minimal character size
	if (GP_ABS_DIFF(ev->st->cursor_x, tbox->click_cursor_x) <= gp_text_wbbox(font, " ")/2)
		return 1;

	cur_pos = tbox->off_left + gp_text_cur_pos(font, str, ev->st->cursor_x - ctx->padd);

	if (cur_pos == tbox->sel_off + tbox->sel_len)
		return 1;

	tbox->sel_off = GP_MIN(tbox->cur_pos, cur_pos);
	tbox->sel_len = GP_MAX(tbox->cur_pos, cur_pos) - tbox->sel_off;

	gp_widget_redraw(self);

	return 1;
}

static void clear_on_input(gp_widget *self)
{
	if (self->tbox->clear_on_input) {
		self->tbox->clear_on_input = 0;
		gp_widget_tbox_clear(self);
	}
}

static int event(gp_widget *self, const gp_widget_render_ctx *ctx, gp_event *ev)
{
	(void)ctx;
	int shift = gp_event_any_key_pressed(ev, GP_KEY_LEFT_SHIFT, GP_KEY_RIGHT_SHIFT);
	int ctrl = gp_event_any_key_pressed(ev, GP_KEY_LEFT_CTRL, GP_KEY_RIGHT_CTRL);

	switch (ev->type) {
	case GP_EV_KEY:
		if (ev->code == GP_EV_KEY_UP)
			return 0;

		clear_on_input(self);

		if (ctrl) {
			switch (ev->val) {
			case GP_KEY_A:
				if (sel_all(self)) {
					gp_widget_redraw(self);
					return 1;
				}
			break;
			case GP_KEY_X:
				selection_to_clipboard(self);
				if (sel_del(self)) {
					gp_widget_redraw(self);
					return 1;
				}
			break;
			case GP_KEY_V:
				gp_widgets_clipboard_request(self);
			break;
			case GP_KEY_C:
				selection_to_clipboard(self);
			break;
			}

			return 0;
		}

		if (gp_widget_key_mod_pressed(ev))
			return 0;

		switch (ev->val) {
		case GP_KEY_ENTER:
			if (ev->code == GP_EV_KEY_DOWN)
				gp_widget_send_widget_event(self, GP_WIDGET_TBOX_TRIGGER);
			return 1;
		case GP_KEY_LEFT:
			key_left(self, shift);
			return 1;
		case GP_KEY_RIGHT:
			key_right(self, shift);
			return 1;
		case GP_KEY_HOME:
			key_home(self, shift);
			return 1;
		case GP_KEY_END:
			key_end(self, shift);
			return 1;
		case GP_KEY_BACKSPACE:
			key_backspace(self);
			return 1;
		case GP_KEY_DELETE:
			key_delete(self);
			return 1;
		case GP_KEY_ESC:
			if (sel_clr(self))
				gp_widget_redraw(self);
			return 1;
		case GP_BTN_LEFT:
		case GP_BTN_PEN:
			if (ev->code != GP_EV_KEY_REPEAT)
				return mouse_click(self, ctx, shift, ev);
		break;
		case GP_KEY_SPACE:
			if (shift) {
				sel_substr_cycle(self, self->tbox->cur_pos, 1);
				gp_widget_redraw(self);
				return 1;
			}
		break;
		}

	break;
	case GP_EV_TMR:
		self->tbox->alert = 0;
		gp_widget_redraw(self);
		return 1;
	break;
	case GP_EV_SYS:
		if (ev->code == GP_EV_SYS_CLIPBOARD) {
			clipboard_event(self);
			return 1;
		}
	break;
	case GP_EV_REL:
	case GP_EV_ABS:
		if (!gp_event_any_key_pressed(ev, GP_BTN_PEN, GP_BTN_LEFT))
			return 0;
		return mouse_drag(self, ctx, ev);
	break;
	case GP_EV_UTF:
		if (gp_widget_key_mod_pressed(ev))
			return 0;

		clear_on_input(self);
		utf_key(self, ev->utf.ch);
		return 1;
	break;
	}

	return 0;
}

static enum gp_widget_tbox_type type_by_name(const char *type_name)
{
	if (!strcmp(type_name, "none"))
		return GP_WIDGET_TBOX_NONE;

	if (!strcmp(type_name, "hidden"))
		return GP_WIDGET_TBOX_HIDDEN;

	if (!strcmp(type_name, "URL"))
		return GP_WIDGET_TBOX_URL;

	if (!strcmp(type_name, "path"))
		return GP_WIDGET_TBOX_PATH;

	if (!strcmp(type_name, "filename"))
		return GP_WIDGET_TBOX_FILENAME;

	return GP_WIDGET_TBOX_MAX;
}

enum keys {
	LEN,
	MAX_LEN,
	SEL_DELIM,
	TATTR,
	TEXT,
	TTYPE,
};

static const gp_json_obj_attr attrs[] = {
	GP_JSON_OBJ_ATTR("len", GP_JSON_INT),
	GP_JSON_OBJ_ATTR("max_len", GP_JSON_INT),
	GP_JSON_OBJ_ATTR("sel_delim", GP_JSON_STR),
	GP_JSON_OBJ_ATTR("tattr", GP_JSON_STR),
	GP_JSON_OBJ_ATTR("text", GP_JSON_STR),
	GP_JSON_OBJ_ATTR("ttype", GP_JSON_STR),
};

static const gp_json_obj obj_filter = {
	.attrs = attrs,
	.attr_cnt = GP_ARRAY_SIZE(attrs),
};

static gp_widget *json_to_tbox(gp_json_buf *json, gp_json_val *val, gp_widget_json_ctx *ctx)
{
	char *text = NULL, *sel_delim = NULL;
	int len = 0;
	int max_len = 0;
	gp_widget_tattr attr = 0;
	gp_widget *ret;
	enum gp_widget_tbox_type type = GP_WIDGET_TBOX_NONE;

	(void)ctx;

	GP_JSON_OBJ_FILTER(json, val, &obj_filter, gp_widget_json_attrs) {
		switch (val->idx) {
		case LEN:
			if (val->val_int <= 0)
				gp_json_warn(json, "Invalid lenght!");
			else
				len = val->val_int;
		break;
		case MAX_LEN:
			if (val->val_int <= 0)
				gp_json_warn(json, "Invalid lenght!");
			else
				max_len = val->val_int;
		break;
		case SEL_DELIM:
			sel_delim = strdup(val->val_str);
		break;
		case TATTR:
			if (gp_widget_tattr_parse(val->val_str, &attr, GP_TATTR_FONT))
				gp_json_warn(json, "Invalid text attribute!");
		break;
		case TEXT:
			text = strdup(val->val_str);
		break;
		case TTYPE:
			type = type_by_name(val->val_str);
			if (type == GP_WIDGET_TBOX_MAX) {
				gp_json_warn(json, "Invalid type!");
				type = GP_WIDGET_TBOX_NONE;
			}
		break;
		}
	}

	if (len <= 0 && !text) {
		GP_WARN("At least one of len or text has to be set!");
		return NULL;
	}

	ret = gp_widget_tbox_new(text, attr, len, max_len, NULL, type, NULL, NULL);

	//TODO: leak on tbox free
	if (sel_delim) {
		if (ret)
			gp_widget_tbox_sel_delim_set(ret, sel_delim);
		else
			free(sel_delim);
	}

	free(text);

	return ret;
}

static void free_(gp_widget *self)
{
	gp_widgets_clipboard_request_cancel(self);

	gp_vec_free(self->tbox->buf);
}

struct gp_widget_ops gp_widget_tbox_ops = {
	.min_w = min_w,
	.min_h = min_h,
	.render = render,
	.event = event,
	.free = free_,
	.from_json = json_to_tbox,
	.id = "tbox",
};

static const char *type_name(enum gp_widget_tbox_type type)
{
	switch (type) {
	case GP_WIDGET_TBOX_NONE:
		return "none";
	case GP_WIDGET_TBOX_HIDDEN:
		return "hidden";
	case GP_WIDGET_TBOX_URL:
		return "URL";
	case GP_WIDGET_TBOX_PATH:
		return "path";
	case GP_WIDGET_TBOX_FILENAME:
		return "filename";
	case GP_WIDGET_TBOX_MAX:
	break;
	}

	return "invalid!";
}

static void set_type(gp_widget *self, enum gp_widget_tbox_type type)
{
	struct gp_widget_tbox *tbox = self->tbox;

	switch (type) {
	case GP_WIDGET_TBOX_HIDDEN:
	case GP_WIDGET_TBOX_NONE:
		tbox->delim = NULL;
	break;
	case GP_WIDGET_TBOX_PATH:
	case GP_WIDGET_TBOX_URL:
		tbox->delim = "/";
	break;
	case GP_WIDGET_TBOX_FILENAME:
		tbox->delim = ".";
	break;
	default:
		GP_WARN("Invalid textbox type %i!", type);
		return;
	}

	GP_DEBUG(2, "Setting tbox (%p) type to '%s' delim '%s'",
	         self, type_name(type), tbox->delim);

	tbox->type = type;
}

gp_widget *gp_widget_tbox_new(const char *text, gp_widget_tattr tattr,
                              unsigned int len, unsigned int max_len,
                              const char *filter, enum gp_widget_tbox_type type,
                              int (*on_event)(gp_widget_event *),
                              void *priv)
{
	gp_widget *ret = gp_widget_new(GP_WIDGET_TBOX, GP_WIDGET_CLASS_NONE, sizeof(struct gp_widget_tbox));
	if (!ret)
		return NULL;

	if (text && max_len)
		max_len = GP_MAX(max_len, strlen(text));

	ret->on_event = on_event;
	ret->priv = priv;
	ret->tbox->max_size = max_len;
	ret->tbox->size = len ? len : strlen(text);
	ret->tbox->filter = filter;
	ret->tbox->tattr = tattr;

	set_type(ret, type);

	if (text) {
		ret->tbox->buf = gp_vec_strdup(text);

		if (!ret->tbox->buf)
			goto err;

		ret->tbox->cur_pos = strlen(text);
	} else {
		ret->tbox->buf = gp_vec_str_new();

		if (!ret->tbox->buf)
			goto err;
	}

	return ret;
err:
	free(ret);
	return NULL;
}

static int resize_buffer(gp_widget *self, size_t len)
{
	char *new_buf = gp_vec_resize(self->tbox->buf, len+1);
	if (!new_buf)
		return 1;

	self->tbox->buf = new_buf;
	self->tbox->cur_pos = len;

	return 0;
}

int gp_widget_tbox_printf(gp_widget *self, const char *fmt, ...)
{
	va_list ap;
	int len;

	GP_WIDGET_ASSERT(self, GP_WIDGET_TBOX, -1);

	sel_clr(self);

	va_start(ap, fmt);
	len = vsnprintf(NULL, 0, fmt, ap);
	va_end(ap);

	if (resize_buffer(self, len))
		return -1;

	va_start(ap, fmt);
	vsprintf(self->tbox->buf, fmt, ap);
	va_end(ap);

	send_edit_event(self);
	gp_widget_redraw(self);

	return len;
}

void gp_widget_tbox_set(gp_widget *self, const char *str)
{
	GP_WIDGET_ASSERT(self, GP_WIDGET_TBOX, );

	size_t len = strlen(str);

	if (resize_buffer(self, len))
		return;

	strcpy(self->tbox->buf, str);

	send_edit_event(self);
	gp_widget_redraw(self);
}

void gp_widget_tbox_clear(gp_widget *self)
{
	GP_WIDGET_ASSERT(self, GP_WIDGET_TBOX, );

	sel_clr(self);

	self->tbox->buf = gp_vec_strclr(self->tbox->buf);
	self->tbox->cur_pos = 0;

	send_edit_event(self);
	gp_widget_redraw(self);
}

const char *gp_widget_tbox_text(gp_widget *self)
{
	GP_WIDGET_ASSERT(self, GP_WIDGET_TBOX, NULL);

	return self->tbox->buf;
}

size_t gp_widget_tbox_cursor_get(gp_widget *self)
{
	GP_WIDGET_ASSERT(self, GP_WIDGET_TBOX, 0);

	return self->tbox->cur_pos;
}

void gp_widget_tbox_cursor_set(gp_widget *self, ssize_t off,
                               enum gp_seek_whence whence)
{
	GP_WIDGET_ASSERT(self, GP_WIDGET_TBOX, );

	size_t max_pos = gp_vec_strlen(self->tbox->buf);
	size_t cur_pos = self->tbox->cur_pos;

	sel_clr(self);

	if (gp_seek_off(off, whence, &cur_pos, max_pos)) {
		schedule_alert(self);
		return;
	}

	self->tbox->cur_pos = cur_pos;

	if (self->focused)
		gp_widget_redraw(self);
}

void gp_widget_tbox_ins(gp_widget *self, ssize_t off,
                        enum gp_seek_whence whence, const char *str)
{
	GP_WIDGET_ASSERT(self, GP_WIDGET_TBOX, );

	clear_on_input(self);

	size_t max_pos = gp_vec_strlen(self->tbox->buf);
	size_t ins_pos = self->tbox->cur_pos;

	sel_clr(self);

	if (gp_seek_off(off, whence, &ins_pos, max_pos)) {
		schedule_alert(self);
		return;
	}

	char *new_buf = gp_vec_strins(self->tbox->buf, ins_pos, str);

	if (!new_buf)
		return;

	self->tbox->buf = new_buf;

	if (ins_pos <= self->tbox->cur_pos)
		self->tbox->cur_pos += strlen(str);

	gp_widget_redraw(self);
}

void gp_widget_tbox_del(gp_widget *self, ssize_t off,
                        enum gp_seek_whence whence, size_t len)
{
	GP_WIDGET_ASSERT(self, GP_WIDGET_TBOX, );

	sel_clr(self);

	size_t max_pos = gp_vec_strlen(self->tbox->buf);
	size_t del_pos = self->tbox->cur_pos;
	size_t del_size;

	if (gp_seek_off(off, whence, &del_pos, max_pos)) {
		schedule_alert(self);
		return;
	}

	del_size = GP_MIN(len, max_pos - del_pos);

	char *new_buf = gp_vec_strdel(self->tbox->buf, del_pos, del_size);

	if (!new_buf)
		return;

	self->tbox->buf = new_buf;

	if (del_pos < self->tbox->cur_pos) {
		if (self->tbox->cur_pos < del_pos + del_size)
			self->tbox->cur_pos = del_pos;
		else
			self->tbox->cur_pos -= del_size;
	}

	gp_widget_redraw(self);
}

void gp_widget_tbox_sel_set(gp_widget *self, ssize_t off,
                            enum gp_seek_whence whence, size_t len)
{
	GP_WIDGET_ASSERT(self, GP_WIDGET_TBOX, );

	size_t max_pos = gp_vec_strlen(self->tbox->buf);
	size_t sel_pos = self->tbox->cur_pos;

	if (is_hidden(self->tbox)) {
		GP_WARN("Attempt to select hidden text!");
		return;
	}

	if (gp_seek_off(off, whence, &sel_pos, max_pos)) {
		GP_WARN("Selection start out of tbox text!");
		return;
	}

	if (max_pos - sel_pos < len) {
		GP_WARN("Selection length out of tbox text!");
		return;
	}

	self->tbox->sel_off = sel_pos;
	self->tbox->sel_len = len;
	self->tbox->cur_pos = sel_pos + len;

	gp_widget_redraw(self);
}

void gp_widget_tbox_sel_all(gp_widget *self)
{
	GP_WIDGET_ASSERT(self, GP_WIDGET_TBOX, );

	if (is_hidden(self->tbox)) {
		GP_WARN("Attempt to select hidden text!");
		return;
	}

	if (sel_all(self))
		gp_widget_redraw(self);
}

void gp_widget_tbox_sel_clr(gp_widget *self)
{
	GP_WIDGET_ASSERT(self, GP_WIDGET_TBOX, );

	if (sel_clr(self))
		gp_widget_redraw(self);
}

void gp_widget_tbox_sel_del(gp_widget *self)
{
	GP_WIDGET_ASSERT(self, GP_WIDGET_TBOX, );

	if (sel_del(self))
		gp_widget_redraw(self);
}

size_t gp_widget_tbox_sel_len(gp_widget *self)
{
	GP_WIDGET_ASSERT(self, GP_WIDGET_TBOX, 0);

	return self->tbox->sel_len;
}

size_t gp_widget_tbox_sel_off(gp_widget *self)
{
	GP_WIDGET_ASSERT(self, GP_WIDGET_TBOX, 0);

	return self->tbox->sel_off;
}

void gp_widget_tbox_sel_delim_set(gp_widget *self, const char *delim)
{
	GP_WIDGET_ASSERT(self, GP_WIDGET_TBOX, );

	self->tbox->delim = delim;
}

void gp_widget_tbox_type_set(gp_widget *self, enum gp_widget_tbox_type type)
{
	GP_WIDGET_ASSERT(self, GP_WIDGET_TBOX, );

	set_type(self, type);
}

void gp_widget_tbox_clear_on_input(gp_widget *self)
{
	GP_WIDGET_ASSERT(self, GP_WIDGET_TBOX, );
	self->tbox->clear_on_input = 1;
}
