//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>
#include <json-c/json.h>

#include <utils/gp_vec_str.h>

#include <gp_widgets.h>
#include <gp_widget_ops.h>
#include <gp_widget_render.h>

static unsigned int min_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	unsigned int ret = 2 * ctx->padd;
	const char *filter = self->tbox->filter;
	size_t text_len = self->tbox->size;

	if (filter)
		ret += gp_text_max_width_chars(ctx->font, filter, text_len);
	else
		ret += gp_text_max_width(ctx->font, text_len);

	return ret;
}

static unsigned int min_h(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	(void)self;

	return 2 * ctx->padd + gp_text_ascent(ctx->font);
}

static const char *hidden_str(const char *buf)
{
	static const char s[] = "********************************************";
	unsigned int len = strlen(buf);

	if (len >= sizeof(s) - 1)
		return s;

	return s + sizeof(s) - len - 1;
}

static void render(gp_widget *self, const gp_offset *offset,
                   const gp_widget_render_ctx *ctx, int flags)
{
	unsigned int x = self->x + offset->x;
	unsigned int y = self->y + offset->y;
	unsigned int w = self->w;
	unsigned int h = self->h;
	const char *str;

	(void)flags;

	gp_widget_ops_blit(ctx, x, y, w, h);

	if (self->tbox->hidden)
		str = hidden_str(self->tbox->buf);
	else
		str = self->tbox->buf;

	gp_pixel color = self->focused ? ctx->sel_color : ctx->text_color;

	if (self->tbox->alert) {
		color = ctx->alert_color;
		gp_widget_render_timer(self, GP_TIMER_RESCHEDULE, 500);
	}

	gp_fill_rrect_xywh(ctx->buf, x, y, w, h, ctx->bg_color, ctx->fg_color, color);

	size_t left = GP_MIN(self->tbox->off_left, self->tbox->cur_pos);
	size_t right = gp_vec_strlen(self->tbox->buf);

	while (gp_text_width_len(ctx->font, str+left, right - left) > self->w - 2 * ctx->padd) {
		if (right > self->tbox->cur_pos)
			right--;
		else
			left++;
	}

	self->tbox->off_left = left;

	gp_coord cy = y + ctx->padd + (gp_text_ascent(ctx->font)+1)/2;
	gp_coord s = ctx->padd/4;

	if (left) {
		gp_coord cx = x + ctx->padd/2;

		gp_line(ctx->buf, cx-s, cy, cx, cy-s, ctx->text_color);
		gp_line(ctx->buf, cx-s, cy, cx, cy+s, ctx->text_color);
	}

	if (right < gp_vec_strlen(self->tbox->buf)) {
		gp_coord cx = x + w - 1 - ctx->padd/2;

		gp_line(ctx->buf, cx+s, cy, cx, cy-s, ctx->text_color);
		gp_line(ctx->buf, cx+s, cy, cx, cy+s, ctx->text_color);
	}

	if (self->focused) {
		unsigned int cursor_x = x + ctx->padd;
		cursor_x += gp_text_width_len(ctx->font, str + left,
		                              self->tbox->cur_pos - left);
		gp_vline_xyh(ctx->buf, cursor_x, y + ctx->padd,
			     gp_text_ascent(ctx->font), ctx->text_color);
	}

	str += left;
	gp_text_ext(ctx->buf, ctx->font,
		    x + ctx->padd, y + ctx->padd,
		    GP_ALIGN_RIGHT|GP_VALIGN_BELOW,
		    ctx->text_color, ctx->bg_color, str, right - left);
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

static void ascii_key(gp_widget *self, char ch)
{
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

	char *tmp = gp_vec_chins(self->tbox->buf, self->tbox->cur_pos, ch);
	if (!tmp)
		return;

	self->tbox->buf = tmp;
	self->tbox->cur_pos++;

	send_edit_event(self);

	gp_widget_redraw(self);
}

static void key_backspace(gp_widget *self)
{
	if (self->tbox->cur_pos <= 0) {
		schedule_alert(self);
		return;
	}

	self->tbox->cur_pos--;

	self->tbox->buf = gp_vec_strdel(self->tbox->buf, self->tbox->cur_pos, 1);

	send_edit_event(self);

	gp_widget_redraw(self);
}

static void key_delete(gp_widget *self)
{
	if (self->tbox->cur_pos == gp_vec_strlen(self->tbox->buf)) {
		schedule_alert(self);
		return;
	}

	self->tbox->buf = gp_vec_strdel(self->tbox->buf, self->tbox->cur_pos, 1);

	send_edit_event(self);

	gp_widget_redraw(self);
}

static void key_left(gp_widget *self)
{
	if (self->tbox->cur_pos > 0) {
		self->tbox->cur_pos--;
		gp_widget_redraw(self);
	}

	clear_alert(self);
}

static void key_right(gp_widget *self)
{
	if (self->tbox->cur_pos < gp_vec_strlen(self->tbox->buf)) {
		self->tbox->cur_pos++;
		gp_widget_redraw(self);
	}

	clear_alert(self);
}

static void key_home(gp_widget *self)
{
	clear_alert(self);

	if (self->tbox->cur_pos == 0)
		return;

	self->tbox->cur_pos = 0;
	gp_widget_redraw(self);
}

static void key_end(gp_widget *self)
{
	clear_alert(self);

	if (!self->tbox->buf[self->tbox->cur_pos])
		return;

	self->tbox->cur_pos = gp_vec_strlen(self->tbox->buf);
	gp_widget_redraw(self);
}

static int event(gp_widget *self, const gp_widget_render_ctx *ctx, gp_event *ev)
{
	(void)ctx;

	switch (ev->type) {
	//TODO: Mouse clicks
	case GP_EV_KEY:
		if (ev->code == GP_EV_KEY_UP)
			return 0;

		switch (ev->val) {
		case GP_KEY_ENTER:
			if (ev->code == GP_EV_KEY_DOWN)
				gp_widget_send_widget_event(self, GP_WIDGET_TBOX_TRIGGER);
			return 1;
		case GP_KEY_LEFT:
			key_left(self);
			return 1;
		case GP_KEY_RIGHT:
			key_right(self);
			return 1;
		case GP_KEY_HOME:
			key_home(self);
			return 1;
		case GP_KEY_END:
			key_end(self);
			return 1;
		case GP_KEY_BACKSPACE:
			key_backspace(self);
			return 1;
		case GP_KEY_DELETE:
			key_delete(self);
			return 1;
		}

		if (ev->key.ascii) {
			ascii_key(self, ev->key.ascii);
			return 1;
		}
	break;
	case GP_EV_TMR:
		self->tbox->alert = 0;
		gp_widget_redraw(self);
		return 1;
	break;
	}

	return 0;
}

static gp_widget *json_to_tbox(json_object *json, void **uid)
{
	gp_widget *ret;
	const char *text = NULL;
	int flags = 0;
	int size = 0;
	int max_size = 0;

	(void)uid;

	json_object_object_foreach(json, key, val) {
		if (!strcmp(key, "text"))
			text = json_object_get_string(val);
		else if (!strcmp(key, "size"))
			size = json_object_get_int(val);
		else if (!strcmp(key, "hidden"))
			flags |= json_object_get_boolean(val) ? GP_WIDGET_TBOX_HIDDEN : 0;
		else if (!strcmp(key, "max_size"))
			max_size = json_object_get_int(val);
		else
			GP_WARN("Invalid tbox key '%s'", key);
	}

	if (size <= 0 && !text) {
		GP_WARN("At least one of size or text has to be set!");
		return NULL;
	}

	if (max_size < 0) {
		GP_WARN("max_size must be >= 0");
		return NULL;
	}

	if (size <= 0)
		size = strlen(text);


	if (text && max_size)
		max_size = GP_MAX(max_size, (int)strlen(text));

	ret = gp_widget_tbox_new(text, size, NULL, NULL, NULL, flags);

	ret->tbox->max_size = max_size;

	return ret;
}

static void free_(gp_widget *self)
{
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

struct gp_widget *gp_widget_tbox_new(const char *text, unsigned int size,
                                        const char *filter,
                                        int (*on_event)(gp_widget_event *),
                                        void *priv, int flags)
{
	gp_widget *ret = gp_widget_new(GP_WIDGET_TBOX, GP_WIDGET_CLASS_NONE, sizeof(struct gp_widget_tbox));
	if (!ret)
		return NULL;

	ret->on_event = on_event;
	ret->priv = priv;
	ret->tbox->size = size ? size : strlen(text);
	ret->tbox->filter = filter;

	if (flags & GP_WIDGET_TBOX_HIDDEN)
		ret->tbox->hidden = 1;

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

int gp_widget_tbox_printf(gp_widget *self, const char *fmt, ...)
{
	va_list ap;
	int len;

	GP_WIDGET_ASSERT(self, GP_WIDGET_TBOX, -1);

	va_start(ap, fmt);
	len = vsnprintf(NULL, 0, fmt, ap)+1;
	va_end(ap);

	char *new_buf = gp_vec_resize(self->tbox->buf, len);
	if (!new_buf)
		return -1;

	va_start(ap, fmt);
	vsprintf(new_buf, fmt, ap);
	va_end(ap);

	self->tbox->buf = new_buf;
	self->tbox->cur_pos = gp_vec_strlen(new_buf);

	gp_widget_redraw(self);

	return len;
}

void gp_widget_tbox_clear(gp_widget *self)
{
	GP_WIDGET_ASSERT(self, GP_WIDGET_TBOX, );

	self->tbox->buf = gp_vec_strclr(self->tbox->buf);
	self->tbox->cur_pos = 0;

	send_edit_event(self);

	gp_widget_redraw(self);
}

const char *gp_widget_tbox_str(gp_widget *self)
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

	size_t max_pos = gp_vec_strlen(self->tbox->buf);
	size_t ins_pos = self->tbox->cur_pos;

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
