//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2022 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>
#include <ctype.h>

#include <utils/gp_vec_str.h>

#include <widgets/gp_widgets.h>
#include <widgets/gp_widget_ops.h>
#include <widgets/gp_widget_render.h>

struct tbox_payload {
	/* Text buffer */
	char *buf;
	/* How many characters should the textbox fit. */
	size_t size;
	/* Help text shown when tbox is empty */
	char *help;

	/*
	 * If not NULL the tbox can contain only characters from this
	 * string, this is used as a hint when minimal tbox size is
	 * accounted for.
	 */
	const char *filter;

	/*
	 * Delimiter list for double click selection.
	 *
	 * If NULL defaults to whitespaces.
	 *
	 * This is set automatically by a certain tbox types.
	 */
	const char *delim;

	/* enum gp_widget_tbox_type */
	uint16_t type;

	uint16_t alert:1;
	uint16_t clear_on_input:1;

	/* Maximal number of unicode characters the textbox can hold. */
	size_t max_size;

	/* Cursor position */
	gp_utf8_pos cur_pos;
	gp_utf8_pos cur_pos_saved;
	/* Offset on left size, part of a string that is not shown */
	gp_utf8_pos off_left;

	/* Selection */
	gp_utf8_pos sel_left;
	gp_utf8_pos sel_right;

	gp_widget_tattr tattr;

	//TODO: Move to event state
	uint64_t last_click;
	uint32_t click_cursor_x;
};

static unsigned int min_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);
	unsigned int ret = 2 * ctx->padd;
	const char *filter = tbox->filter;
	size_t text_len = tbox->size;
	const gp_text_style *font = gp_widget_tattr_font(tbox->tattr, ctx);

	if (filter)
		ret += gp_text_max_width_chars(font, filter, text_len);
	else
		ret += gp_text_avg_width(font, text_len);

	return ret;
}

static unsigned int min_h(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);
	const gp_text_style *font = gp_widget_tattr_font(tbox->tattr, ctx);

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

static int is_sel(struct tbox_payload *tbox)
{
	return tbox->sel_left.bytes < tbox->sel_right.bytes;
}

static int is_hidden(struct tbox_payload *tbox)
{
	return tbox->type == GP_WIDGET_TBOX_HIDDEN;
}

static const char *tbox_visible_str(struct tbox_payload *tbox)
{
	if (is_hidden(tbox))
		return hidden_str(tbox->buf);

	return tbox->buf;
}

static void render(gp_widget *self, const gp_offset *offset,
                   const gp_widget_render_ctx *ctx, int flags)
{
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);
	const gp_text_style *font = gp_widget_tattr_font(tbox->tattr, ctx);
	unsigned int x = self->x + offset->x;
	unsigned int y = self->y + offset->y;
	unsigned int w = self->w;
	unsigned int h = self->h;

	gp_pixel text_color = gp_widget_text_color(self, ctx, flags);
	gp_pixel fr_color = gp_widget_frame_color(self, ctx, flags);

	gp_widget_ops_blit(ctx, x, y, w, h);

	if (tbox->alert) {
		fr_color = ctx->alert_color;
		gp_widget_render_timer(self, GP_TIMER_RESCHEDULE, ctx->feedback_ms);
	}

	gp_fill_rrect_xywh(ctx->buf, x, y, w, h, ctx->bg_color, ctx->fg_color, fr_color);

	if (!(*tbox->buf) && !self->focused) {
		if (!tbox->help)
			return;

		gp_text_fit(ctx->buf, font, x + ctx->padd, y + ctx->padd,
		            w - 2 * ctx->padd, GP_ALIGN_LEFT|GP_VALIGN_BELOW,
		            ctx->col_disabled, ctx->bg_color, tbox->help);
		return;
	}

	const char *str = tbox_visible_str(tbox);

	//TODO: Make this faster?
	gp_utf8_pos left = gp_utf8_pos_min(tbox->off_left, tbox->cur_pos);
	gp_utf8_pos right = gp_utf8_pos_last(tbox->buf);

	gp_size tbox_width = self->w - 2 * ctx->padd;

	for (;;) {
		if (gp_text_wbbox_len(font, str + left.bytes, right.chars-left.chars) <= tbox_width)
			break;

		if (gp_utf8_pos_gt(right, tbox->cur_pos)) {
			if (!gp_utf8_pos_move(str, &right, -1))
				break;
		} else {
			if (!gp_utf8_pos_move(str, &left, 1))
				break;
		}
	}

	tbox->off_left = left;

	gp_coord cy = y + ctx->padd + (gp_text_ascent(font)+1)/2;
	gp_coord s = ctx->padd/4;

	if (left.bytes) {
		gp_coord cx = x + ctx->padd/2;

		gp_line(ctx->buf, cx-s, cy, cx, cy-s, text_color);
		gp_line(ctx->buf, cx-s, cy, cx, cy+s, text_color);
	}

	if (right.bytes < gp_vec_strlen(tbox->buf)) {
		gp_coord cx = x + w - 1 - ctx->padd/2;

		gp_line(ctx->buf, cx+s, cy, cx, cy-s, text_color);
		gp_line(ctx->buf, cx+s, cy, cx, cy+s, text_color);
	}

	if (self->focused && !is_sel(tbox)) {
		unsigned int cursor_x = x + ctx->padd;
		cursor_x += gp_text_wbbox_len(font, str + left.bytes,
		                              tbox->cur_pos.chars - left.chars);

		gp_fill_rect_xywh(ctx->buf, cursor_x - (ctx->cur_thick+1)/2, y + ctx->padd,
			          ctx->cur_thick, gp_text_ascent(font), text_color);
	}

	str += left.bytes;
	gp_text_ext(ctx->buf, font,
		    x + ctx->padd, y + ctx->padd,
		    GP_ALIGN_RIGHT|GP_VALIGN_BELOW,
		    text_color, ctx->bg_color, str, right.chars - left.chars);

	if (!is_sel(tbox))
		return;

	gp_utf8_pos sel_left = gp_utf8_pos_max(tbox->sel_left, left);
	gp_utf8_pos sel_right = gp_utf8_pos_min(tbox->sel_right, right);

	if (gp_utf8_pos_ge(sel_left, sel_right))
		return;

	gp_coord sel_x_off = x + ctx->padd + gp_text_width_len(font, GP_TEXT_LEN_ADVANCE, str, sel_left.chars - left.chars);

	str += sel_left.bytes - left.bytes;

	size_t sel_len = sel_right.chars - sel_left.chars;

	gp_fill_rect_xywh(ctx->buf, sel_x_off, y + ctx->padd,
	                  gp_text_wbbox_len(font, str, sel_len),
	                  gp_text_height(font), ctx->sel_color);


	gp_pixel rev_text_col = gp_pixel_size(ctx->pixel_type) == 1 ? ctx->fg_color : ctx->text_color;

	gp_text_ext(ctx->buf, font, sel_x_off, y + ctx->padd,
	            GP_ALIGN_RIGHT | GP_VALIGN_BELOW,
	            rev_text_col, ctx->sel_color, str,
		    sel_len);
}

static void schedule_alert(gp_widget *self)
{
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);

	tbox->alert = 1;
	gp_widget_redraw(self);
}

static void clear_alert(gp_widget *self)
{
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);

	if (tbox->alert) {
		gp_widget_render_timer_cancel(self);
		tbox->alert = 0;
	}
}

static void send_edit_event(gp_widget *self)
{
	clear_alert(self);
	gp_widget_send_widget_event(self, GP_WIDGET_TBOX_EDIT);
}

static void send_set_event(gp_widget *self)
{
	clear_alert(self);
	gp_widget_send_widget_event(self, GP_WIDGET_TBOX_SET);
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
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);

	if (!is_sel(tbox))
		return 0;

	tbox->sel_left = gp_utf8_pos_first();
	tbox->sel_right = gp_utf8_pos_first();

	return 1;
}

static int sel_clr_left(gp_widget *self)
{
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);

	if (!is_sel(tbox))
		return 0;

	tbox->cur_pos = tbox->sel_left;
	sel_clr(self);

	return 1;
}

static int sel_clr_right(gp_widget *self)
{
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);

	if (!is_sel(tbox))
		return 0;

	tbox->cur_pos = tbox->sel_right;
	sel_clr(self);

	return 1;
}

static int sel_del(gp_widget *self)
{
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);

	if (!is_sel(tbox))
		return 0;

	//TODO: Optimize? we have the right offsets at hand, we can simply cut the string
	gp_widget_tbox_del(self, tbox->sel_left.chars,
	                   GP_SEEK_SET, tbox->sel_right.chars - tbox->sel_left.chars);
	sel_clr(self);

	return 1;
}

static int cursor_at_end(gp_widget *self)
{
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);

	return tbox->cur_pos.bytes == gp_vec_strlen(tbox->buf);
}

static int cursor_at_home(gp_widget *self)
{
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);

	return !tbox->cur_pos.bytes;
}

static int is_all_sel(gp_widget *self)
{
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);

	return tbox->sel_left.bytes == 0 &&
	       tbox->sel_right.bytes == gp_vec_strlen(tbox->buf);
}

static int sel_all(gp_widget *self)
{
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);

	if (is_hidden(tbox))
		return 0;

	if (is_all_sel(self))
		return 0;

	tbox->sel_left = gp_utf8_pos_first();
	tbox->sel_right = gp_utf8_pos_last(tbox->buf);

	return 1;
}

static int sel_end(gp_widget *self)
{
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);

	if (is_hidden(tbox))
		return 0;

	if (cursor_at_end(self))
		return 0;

	if (!is_sel(tbox))
		tbox->sel_left = tbox->cur_pos;
	else if (gp_utf8_pos_eq(tbox->sel_left, tbox->cur_pos))
		tbox->sel_left = tbox->sel_right;

	//TODO: pos move to end?
	tbox->sel_right = gp_utf8_pos_last(tbox->buf);
	return 1;
}

static int sel_right(gp_widget *self)
{
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);

	if (is_hidden(tbox))
		return 0;

	if (!is_sel(tbox)) {
		if (cursor_at_end(self))
			return 0;

		tbox->sel_left = tbox->cur_pos;

		//TODO: Simplify?
		tbox->sel_right = tbox->sel_left;
		gp_utf8_pos_move(tbox->buf, &tbox->sel_right, 1);

		return 1;
	}

	if (gp_utf8_pos_eq(tbox->cur_pos, tbox->sel_left)) {
		gp_utf8_pos_move(tbox->buf, &tbox->sel_left, 1);
		return 1;
	}

	if (gp_utf8_pos_at_end(tbox->buf, tbox->sel_right))
		return 0;

	return gp_utf8_pos_move(tbox->buf, &tbox->sel_right, 1);
}

static int sel_home(gp_widget *self)
{
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);

	if (is_hidden(tbox))
		return 0;

	if (cursor_at_home(self))
		return 0;

	if (!is_sel(tbox))
		tbox->sel_right = tbox->cur_pos;
	else if (!gp_utf8_pos_eq(tbox->cur_pos, tbox->sel_left))
		tbox->sel_right = tbox->sel_left;

	tbox->sel_left = gp_utf8_pos_first();
	return 1;
}

static int sel_left(gp_widget *self)
{
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);

	if (is_hidden(tbox))
		return 0;

	if (!is_sel(tbox)) {
		if (gp_utf8_pos_at_home(tbox->cur_pos))
			return 0;

		tbox->sel_right = tbox->cur_pos;
		tbox->sel_left = tbox->sel_right;
		//TODO: Simplify?
		gp_utf8_pos_move(tbox->buf, &tbox->sel_left, -1);
		return 1;
	}

	if (gp_utf8_pos_eq(tbox->cur_pos, tbox->sel_right)) {
		gp_utf8_pos_move(tbox->buf, &tbox->sel_right, -1);
		return 1;
	}

	if (gp_utf8_pos_at_home(tbox->sel_left))
		return 0;

	gp_utf8_pos_move(tbox->buf, &tbox->sel_left, -1);
	return 1;
}

static int utf_key(gp_widget *self, uint32_t ch)
{
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);
	int ret;

	sel_del(self);

	if (tbox->max_size &&
	    gp_utf8_strlen(tbox->buf) >= tbox->max_size) {
		schedule_alert(self);
		return 1;
	}

	if (filter(tbox->filter, ch)) {
		schedule_alert(self);
		return 1;
	}

	ret = gp_widget_send_widget_event(self, GP_WIDGET_TBOX_PRE_FILTER, (long)ch);
	if (ret) {
		schedule_alert(self);
		return 1;
	}

	char *tmp = gp_vec_ins_utf8(tbox->buf, tbox->cur_pos.bytes, ch);
	if (!tmp)
		return 1;

	tbox->buf = tmp;

	ret = gp_widget_send_widget_event(self, GP_WIDGET_TBOX_POST_FILTER, (long)ch);
	if (ret) {
		tbox->buf = gp_vec_del(tbox->buf, tbox->cur_pos.bytes, gp_utf8_bytes(ch));
		schedule_alert(self);
		return 1;
	}

	gp_utf8_pos_move(tbox->buf, &tbox->cur_pos, 1);

	send_edit_event(self);
	gp_widget_redraw(self);

	return 0;
}

static void key_backspace(gp_widget *self)
{
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);

	if (sel_del(self))
		goto ret;

	if (gp_utf8_pos_at_home(tbox->cur_pos)) {
		schedule_alert(self);
		return;
	}

	size_t bytes = tbox->cur_pos.bytes;
	gp_utf8_pos_move(tbox->buf, &tbox->cur_pos, -1);
	tbox->buf = gp_vec_del(tbox->buf, tbox->cur_pos.bytes, bytes - tbox->cur_pos.bytes);
ret:
	send_edit_event(self);
	gp_widget_redraw(self);
}

static void key_delete(gp_widget *self)
{
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);

	if (sel_del(self))
		goto ret;

	if (cursor_at_end(self)) {
		schedule_alert(self);
		return;
	}

	size_t off = tbox->cur_pos.bytes;
	int8_t len = gp_utf8_next_chsz(tbox->buf, off);

	if (len <= 0)
		return;

	tbox->buf = gp_vec_del(tbox->buf, off, len);
ret:
	send_edit_event(self);
	gp_widget_redraw(self);
}

static void key_left(gp_widget *self, int shift)
{
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);

	if (!shift && sel_clr_left(self))
		goto redraw;

	if (cursor_at_home(self))
		goto exit;

	if (shift)
		sel_left(self);

	gp_utf8_pos_move(tbox->buf, &tbox->cur_pos, -1);

redraw:
	gp_widget_redraw(self);
exit:
	clear_alert(self);
}

static void key_right(gp_widget *self, int shift)
{
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);

	if (!shift && sel_clr_right(self))
		goto redraw;

	if (cursor_at_end(self))
		goto exit;

	if (shift)
		sel_right(self);

	gp_utf8_pos_move(tbox->buf, &tbox->cur_pos, 1);
redraw:
	gp_widget_redraw(self);
exit:
	clear_alert(self);
}

static void key_home(gp_widget *self, int shift)
{
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);

	if (!shift && sel_clr(self))
		goto exit;

	if (gp_utf8_pos_at_home(tbox->cur_pos))
		return;

	if (shift && !sel_home(self))
		return;

exit:
	tbox->cur_pos = gp_utf8_pos_first();
	gp_widget_redraw(self);
	clear_alert(self);
}

static void key_end(gp_widget *self, int shift)
{
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);

	if (!shift && sel_clr(self))
		goto exit;

	if (cursor_at_end(self))
		return;

	if (shift && !sel_end(self))
		return;

exit:
	tbox->cur_pos = gp_utf8_pos_last(tbox->buf);
	gp_widget_redraw(self);
	clear_alert(self);
}

static void selection_to_clipboard(gp_widget *self)
{
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);

	if (!is_sel(tbox))
		return;

	gp_widgets_clipboard_set(tbox->buf + tbox->sel_left.bytes,
	                         tbox->sel_right.bytes - tbox->sel_left.bytes);
}

static void clear_on_input(gp_widget *self)
{
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);

	if (tbox->clear_on_input) {
		tbox->clear_on_input = 0;
		gp_widget_tbox_clear(self);
	}
}

static void clipboard_event(gp_widget *self)
{
	char *clip;
	uint32_t ch;

	clear_on_input(self);
	sel_del(self);

	gp_widget_send_widget_event(self, GP_WIDGET_TBOX_PASTE);

	clip = gp_widgets_clipboard_get();
	if (!clip)
		return;

	const char *pos = clip;

	while ((ch = gp_utf8_next(&pos))) {
		if (utf_key(self, ch))
			break;
	}

	free(clip);
}

static int is_delim(struct tbox_payload *tbox, char c)
{
	if (tbox->delim)
		return !filter(tbox->delim, c);

	return isspace(c);
}

static void sel_substr_cycle(gp_widget *self, gp_utf8_pos cur_pos, int cur_save_restore)
{
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);
	gp_utf8_pos left = cur_pos, right = cur_pos;

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

	if (is_sel(tbox)) {
		sel_all(self);
		return;
	}

	//TODO: UTF8 delimiter!
	while (left.bytes > 0 && !is_delim(tbox, tbox->buf[left.bytes-1]))
		gp_utf8_pos_move(tbox->buf, &left, -1);

	while (tbox->buf[right.bytes] && !is_delim(tbox, tbox->buf[right.bytes]))
		gp_utf8_pos_move(tbox->buf, &right, 1);

	if (cur_save_restore)
		tbox->cur_pos_saved = tbox->cur_pos;

	tbox->cur_pos = right;
	tbox->sel_left = left;
	tbox->sel_right = right;
}

static int mouse_click(gp_widget *self, const gp_widget_render_ctx *ctx, int shift, gp_event *ev)
{
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);
	const gp_text_style *font = gp_widget_tattr_font(tbox->tattr, ctx);
	const char *str = tbox_visible_str(tbox);
	size_t cur_pos_chars;
	gp_utf8_pos cur_pos = tbox->off_left;

	cur_pos_chars = gp_text_cur_pos(font, str + tbox->off_left.bytes, ev->st->cursor_x - ctx->padd);

	gp_utf8_pos_move(tbox->buf, &cur_pos, cur_pos_chars);

	if (gp_widgets_is_dclick(ev->time, tbox->last_click, ctx)) {
		sel_substr_cycle(self, cur_pos, 0);
	} else {
		if (shift) {
			gp_utf8_pos sel_origin;

			if (!is_sel(tbox)) {
				sel_origin = tbox->cur_pos;
			} else {
				if (gp_utf8_pos_eq(tbox->cur_pos, tbox->sel_left))
					sel_origin = tbox->sel_right;
				else
					sel_origin = tbox->sel_left;
			}

			tbox->sel_right = gp_utf8_pos_max(sel_origin, cur_pos);
			tbox->sel_left = gp_utf8_pos_min(sel_origin, cur_pos);
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
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);
	const gp_text_style *font = gp_widget_tattr_font(tbox->tattr, ctx);
	const char *str = tbox_visible_str(tbox);
	size_t cur_pos_chars;
	gp_utf8_pos cur_pos = tbox->off_left;

	//TODO: Get font minimal character size
	if (GP_ABS_DIFF(ev->st->cursor_x, tbox->click_cursor_x) <= gp_text_wbbox(font, " ")/2)
		return 1;

	cur_pos_chars = gp_text_cur_pos(font, str + cur_pos.bytes, ev->st->cursor_x - ctx->padd);
	if (cur_pos_chars == tbox->sel_right.chars)
		return 1;

	gp_utf8_pos_move(tbox->buf, &cur_pos, cur_pos_chars);

	tbox->sel_left = gp_utf8_pos_min(tbox->cur_pos, cur_pos);
	tbox->sel_right = gp_utf8_pos_max(tbox->cur_pos, cur_pos);

	gp_widget_redraw(self);

	return 1;
}

static int event(gp_widget *self, const gp_widget_render_ctx *ctx, gp_event *ev)
{
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);

	int shift = gp_ev_any_key_pressed(ev, GP_KEY_LEFT_SHIFT, GP_KEY_RIGHT_SHIFT);
	int ctrl = gp_ev_any_key_pressed(ev, GP_KEY_LEFT_CTRL, GP_KEY_RIGHT_CTRL);

	(void)ctx;

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
					send_edit_event(self);
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
		case GP_BTN_TOUCH:
			if (ev->code != GP_EV_KEY_REPEAT)
				return mouse_click(self, ctx, shift, ev);
		break;
		case GP_KEY_SPACE:
			if (shift) {
				sel_substr_cycle(self, tbox->cur_pos, 1);
				gp_widget_redraw(self);
				return 1;
			}
		break;
		}

	break;
	case GP_EV_TMR:
		tbox->alert = 0;
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
		if (!gp_ev_any_key_pressed(ev, GP_BTN_TOUCH, GP_BTN_LEFT))
			return 0;
		return mouse_drag(self, ctx, ev);
	break;
	case GP_EV_UTF:
		if (gp_widget_key_mod_pressed(ev))
			return 0;

		if (gp_ev_utf_is_ctrl(ev))
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
	HELP,
	LEN,
	MAX_LEN,
	SEL_DELIM,
	TATTR,
	TEXT,
	TTYPE,
};

static const gp_json_obj_attr attrs[] = {
	GP_JSON_OBJ_ATTR("help", GP_JSON_STR),
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

static gp_widget *json_to_tbox(gp_json_reader *json, gp_json_val *val, gp_widget_json_ctx *ctx)
{
	char *text = NULL, *sel_delim = NULL, *help = NULL;
	int len = 0;
	int max_len = 0;
	gp_widget_tattr attr = 0;
	gp_widget *ret;
	enum gp_widget_tbox_type type = GP_WIDGET_TBOX_NONE;

	(void)ctx;

	GP_JSON_OBJ_FOREACH_FILTER(json, val, &obj_filter, gp_widget_json_attrs) {
		switch (val->idx) {
		case HELP:
			help = strdup(val->val_str);
		break;
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

	ret = gp_widget_tbox_new(text, attr, len, max_len, NULL, type);

	//TODO: leak on tbox free
	if (sel_delim) {
		if (ret)
			gp_widget_tbox_sel_delim_set(ret, sel_delim);
		else
			free(sel_delim);
	}

	if (ret) {
		struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(ret);
		tbox->help = help;
	}

	free(text);

	return ret;
}

static void free_(gp_widget *self)
{
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);

	gp_widgets_clipboard_request_cancel(self);

	free(tbox->help);

	gp_vec_free(tbox->buf);
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
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);

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
                              const char *filter, enum gp_widget_tbox_type type)
{
	gp_widget *ret = gp_widget_new(GP_WIDGET_TBOX, GP_WIDGET_CLASS_NONE, sizeof(struct tbox_payload));
	if (!ret)
		return NULL;

	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(ret);

	if (text && max_len)
		max_len = GP_MAX(max_len, gp_utf8_strlen(text));

	tbox->max_size = max_len;
	tbox->size = len ? len : strlen(text);
	tbox->filter = filter;
	tbox->tattr = tattr;

	set_type(ret, type);

	if (text) {
		tbox->buf = gp_vec_strdup(text);

		if (!tbox->buf)
			goto err;

		tbox->cur_pos = gp_utf8_pos_last(text);
	} else {
		tbox->buf = gp_vec_str_new();

		if (!tbox->buf)
			goto err;
	}

	return ret;
err:
	free(ret);
	return NULL;
}

static int resize_buffer(gp_widget *self, size_t len)
{
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);

	char *new_buf = gp_vec_resize(tbox->buf, len+1);
	if (!new_buf)
		return 1;

	tbox->buf = new_buf;

	return 0;
}

int gp_widget_tbox_printf(gp_widget *self, const char *fmt, ...)
{
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);
	va_list ap;
	int len;

	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_TBOX, -1);

	sel_clr(self);

	va_start(ap, fmt);
	len = vsnprintf(NULL, 0, fmt, ap);
	va_end(ap);

	if (resize_buffer(self, len))
		return -1;

	va_start(ap, fmt);
	vsprintf(tbox->buf, fmt, ap);
	va_end(ap);

	tbox->cur_pos = gp_utf8_pos_last(tbox->buf);

	send_set_event(self);
	gp_widget_redraw(self);

	return len;
}

void gp_widget_tbox_set(gp_widget *self, const char *str)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_TBOX, );
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);

	size_t len = strlen(str);

	if (resize_buffer(self, len))
		return;

	strcpy(tbox->buf, str);

	tbox->cur_pos = gp_utf8_pos_last(tbox->buf);

	send_set_event(self);
	gp_widget_redraw(self);
}

void gp_widget_tbox_clear(gp_widget *self)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_TBOX, );
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);

	sel_clr(self);

	tbox->buf = gp_vec_strclr(tbox->buf);
	tbox->cur_pos = gp_utf8_pos_first();

	send_set_event(self);
	gp_widget_redraw(self);
}

const char *gp_widget_tbox_text(gp_widget *self)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_TBOX, NULL);
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);

	return tbox->buf;
}

gp_utf8_pos gp_widget_tbox_cursor_get(gp_widget *self)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_TBOX, (gp_utf8_pos){});
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);

	return tbox->cur_pos;
}

void gp_widget_tbox_cursor_set(gp_widget *self, ssize_t off,
                               enum gp_seek_whence whence)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_TBOX, );
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);

	size_t max_pos_chars = gp_utf8_strlen(tbox->buf);
	size_t cur_pos_chars = tbox->cur_pos.chars;

	sel_clr(self);

	if (gp_seek_off(off, whence, &cur_pos_chars, max_pos_chars)) {
		schedule_alert(self);
		return;
	}

	tbox->cur_pos = gp_utf8_pos_first();

	gp_utf8_pos_move(tbox->buf, &tbox->cur_pos, cur_pos_chars);

	if (self->focused)
		gp_widget_redraw(self);
}

void gp_widget_tbox_ins(gp_widget *self, ssize_t off,
                        enum gp_seek_whence whence, const char *str)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_TBOX, );
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);

	clear_on_input(self);

	size_t max_pos = gp_utf8_strlen(tbox->buf);
	size_t ins_pos = tbox->cur_pos.chars;

	sel_clr(self);

	if (gp_seek_off(off, whence, &ins_pos, max_pos)) {
		schedule_alert(self);
		return;
	}

	gp_utf8_pos cur_pos = gp_utf8_pos_first();
	gp_utf8_pos_move(tbox->buf, &cur_pos, ins_pos);

	char *new_buf = gp_vec_strins(tbox->buf, cur_pos.bytes, str);

	if (!new_buf)
		return;

	tbox->buf = new_buf;

	if (ins_pos <= tbox->cur_pos.chars)
		gp_utf8_pos_move(tbox->buf, &tbox->cur_pos, gp_utf8_strlen(str));

	send_set_event(self);
	gp_widget_redraw(self);
}

void gp_widget_tbox_del(gp_widget *self, ssize_t off,
                        enum gp_seek_whence whence, size_t len)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_TBOX, );
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);

	sel_clr(self);

	size_t max_pos_chars = gp_utf8_strlen(tbox->buf);
	size_t del_pos_chars = tbox->cur_pos.chars;
	size_t del_size_chars;

	if (gp_seek_off(off, whence, &del_pos_chars, max_pos_chars)) {
		schedule_alert(self);
		return;
	}

	del_size_chars = GP_MIN(len, max_pos_chars - del_pos_chars);

	gp_utf8_pos del_pos = gp_utf8_pos_first();
	gp_utf8_pos_move(tbox->buf, &del_pos, del_pos_chars);
	size_t del_pos_bytes = del_pos.bytes;
	//TODO: gp_utf8_chars_to_bytes(const char *string, size_t off, size_t chars)
	gp_utf8_pos end_pos = del_pos;
	gp_utf8_pos_move(tbox->buf, &end_pos, del_size_chars);
	size_t del_len_bytes = end_pos.bytes - del_pos_bytes;

	gp_utf8_pos cur_pos = tbox->cur_pos;

	if (del_pos_chars < cur_pos.chars) {
		if (cur_pos.chars < del_pos_chars + del_size_chars)
			cur_pos = del_pos;
		else
			gp_utf8_pos_move(tbox->buf, &cur_pos, -(ssize_t)del_size_chars);
	}

	char *new_buf = gp_vec_del(tbox->buf, del_pos_bytes, del_len_bytes);
	if (!new_buf)
		return;

	tbox->buf = new_buf;
	tbox->cur_pos = cur_pos;

	send_set_event(self);
	gp_widget_redraw(self);
}

void gp_widget_tbox_sel_set(gp_widget *self, ssize_t off,
                            enum gp_seek_whence whence, size_t len)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_TBOX, );
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);

	if (is_hidden(tbox)) {
		GP_WARN("Attempt to select hidden text!");
		return;
	}

	size_t max_pos = gp_utf8_strlen(tbox->buf);
	size_t sel_pos = tbox->cur_pos.chars;

	if (gp_seek_off(off, whence, &sel_pos, max_pos)) {
		GP_WARN("Selection start out of tbox text!");
		return;
	}

	if (max_pos - sel_pos < len) {
		GP_WARN("Selection length out of tbox text!");
		return;
	}

	tbox->sel_left = gp_utf8_pos_first();
	gp_utf8_pos_move(tbox->buf, &tbox->sel_left, sel_pos);
	tbox->sel_right = tbox->sel_left;
	gp_utf8_pos_move(tbox->buf, &tbox->sel_right, len);
	tbox->cur_pos = tbox->sel_right;

	gp_widget_redraw(self);
}

void gp_widget_tbox_sel_all(gp_widget *self)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_TBOX, );
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);

	if (is_hidden(tbox)) {
		GP_WARN("Attempt to select hidden text!");
		return;
	}

	if (sel_all(self))
		gp_widget_redraw(self);
}

void gp_widget_tbox_sel_clr(gp_widget *self)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_TBOX, );

	if (sel_clr(self))
		gp_widget_redraw(self);
}

void gp_widget_tbox_sel_del(gp_widget *self)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_TBOX, );

	if (sel_del(self))
		gp_widget_redraw(self);
}

gp_utf8_pos gp_widget_tbox_sel_len(gp_widget *self)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_TBOX, (gp_utf8_pos){});
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);

	return gp_utf8_pos_sub(tbox->sel_right, tbox->sel_left);
}

gp_utf8_pos gp_widget_tbox_sel_off(gp_widget *self)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_TBOX, (gp_utf8_pos){});
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);

	return tbox->sel_left;
}

void gp_widget_tbox_sel_delim_set(gp_widget *self, const char *delim)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_TBOX, );
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);

	tbox->delim = delim;
}

void gp_widget_tbox_type_set(gp_widget *self, enum gp_widget_tbox_type type)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_TBOX, );

	set_type(self, type);
}

void gp_widget_tbox_help_set(gp_widget *self, const char *help)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_TBOX, );
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);

	free(tbox->help);

	tbox->help = help ? strdup(help) : NULL;
}

void gp_widget_tbox_clear_on_input(gp_widget *self)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_TBOX, );
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);

	tbox->clear_on_input = 1;
}

void gp_widget_tbox_filter_set(gp_widget *self, const char *filter)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_TBOX, );
	struct tbox_payload *tbox = GP_WIDGET_PAYLOAD(self);

	tbox->filter = filter;
}
