//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>
#include <json-c/json.h>

#include <gp_widgets.h>

static unsigned int stock_min_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	return GP_ODD_UP(gp_widget_size_units_get(&self->stock->min_size, ctx));
}

static unsigned int stock_min_h(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	return GP_ODD_UP(gp_widget_size_units_get(&self->stock->min_size, ctx));
}

static void render_stock_err_warn(const gp_widget_render_ctx *ctx,
                                  gp_coord x, gp_coord y,
                                  gp_size w, gp_size h, gp_pixel color)
{
	gp_pixmap *pix = ctx->buf;

	gp_coord cx = x + w/2;
	gp_coord cy = y + h/2;
	gp_size c = GP_MIN(w, h)/2 - 2;
	gp_size r = c/5;

	gp_fill_rrect_xywh(pix, x, y, w, h, ctx->bg_color, color, ctx->text_color);

	if (r > 1)
		gp_fill_circle(pix, cx, cy+h/4, r, ctx->text_color);
	else
		gp_fill_rect_xyxy(pix, cx-r, cy+h/4-r, cx+r, cy+h/4+r, ctx->text_color);

	gp_fill_rect_xyxy(pix, cx-r, cy-h/4-r, cx+r, cy+r, ctx->text_color);

}

static void render_stock_info(const gp_widget_render_ctx *ctx,
                              gp_coord x, gp_coord y,
                              gp_size w, gp_size h)
{
	gp_pixmap *pix = ctx->buf;

	gp_size c = GP_MIN(w, h)/2 - 2;
	gp_size r = c/5;
	gp_coord cx = x + w/2;
	gp_coord cy = y + h/2;

	gp_fill_rrect_xywh(pix, x, y, w, h, ctx->bg_color, ctx->sel_color, ctx->text_color);

	if (r > 1)
		gp_fill_circle(pix, cx, cy-h/4, r, ctx->text_color);
	else
		gp_fill_rect_xyxy(pix, cx-r, cy-h/4-r, cx+r, cy-h/4+r, ctx->text_color);

	gp_fill_rect_xyxy(pix, cx-r, cy+h/4, cx+r, cy-r, ctx->text_color);

	gp_fill_rect_xyxy(pix, cx-2*r, cy+h/4, cx+2*r, cy+h/4+r/2, ctx->text_color);

	gp_fill_rect_xyxy(pix, cx-2*r, cy-r, cx, cy-r+r/2, ctx->text_color);
}

static void render_stock_question(const gp_widget_render_ctx *ctx,
                                  gp_coord x, gp_coord y,
                                  gp_size w, gp_size h)
{
	gp_pixmap *pix = ctx->buf;

	gp_coord cx = x + w/2;
	gp_coord cy = y + h/2;
	gp_size c = GP_MIN(w, h)/2 - 2;
	gp_size r = c/5;

	gp_fill_rrect_xywh(pix, x, y, w, h, ctx->bg_color, ctx->sel_color, ctx->text_color);

	if (r > 1)
		gp_fill_circle(pix, cx, cy+h/4+r, r, ctx->text_color);
	else
		gp_fill_rect_xyxy(pix, cx-r, cy+h/4, cx+r, cy+h/4+2*r, ctx->text_color);

	gp_fill_rect_xyxy(pix, cx-r, cy-h/4+3*r, cx+r, cy+2*r, ctx->text_color);

	gp_fill_ring_seg(pix, cx, cy-h/4 + r, r, 3*r, GP_CIRCLE_SEG1 | GP_CIRCLE_SEG2 |GP_CIRCLE_SEG4, ctx->text_color);

	gp_fill_triangle(pix, cx-r, cy-h/4+3*r, cx, cy-h/4+2*r, cx, cy-h/4+3*r, ctx->text_color);
}

static void render_stock_speaker(const gp_widget_render_ctx *ctx,
                                 gp_coord cx, gp_coord cy, gp_coord r,
                                 gp_widget *self)
{
	gp_pixmap *pix = ctx->buf;
	gp_pixel col = self->focused ? ctx->sel_color : ctx->text_color;

	gp_coord poly[] = {
		cx-2*(r/2), cy-2*r/3,
		cx-4*(r/2), cy-2*r/3,
		cx-4*(r/2), cy+2*r/3,
		cx-2*(r/2), cy+2*r/3,
		cx-r/4, cy+3*(r/2),
		cx, cy+3*(r/2),
		cx, cy-3*(r/2),
		cx-r/4, cy-3*(r/2),
	};

	gp_fill_polygon(pix, GP_ARRAY_SIZE(poly)/2, poly, col);

	gp_coord rd = r/3;
	gp_coord i;

	if (rd < 1)
		cx++;

	/* Line thickness */
	gp_coord lt = (rd)/2;

	switch (self->stock->type) {
	case GP_WIDGET_STOCK_SPEAKER_MUTE:
		for (i = 0; i <= (rd+1)/2; i++) {
			gp_line(pix, cx+r/2+i, cy-r/2, cx+3*(r/2), cy+r/2-i, ctx->alert_color);
			gp_line(pix, cx+r/2, cy-r/2+i, cx+3*(r/2)-i, cy+r/2, ctx->alert_color);
			gp_line(pix, cx+r/2+i, cy+r/2, cx+3*(r/2), cy-r/2+i, ctx->alert_color);
			gp_line(pix, cx+r/2, cy+r/2-i, cx+3*(r/2)-i, cy-r/2, ctx->alert_color);
		}
	break;
	case GP_WIDGET_STOCK_SPEAKER_MIN:
	break;
	case GP_WIDGET_STOCK_SPEAKER_MAX:
		gp_fill_ring_seg(pix, cx+r/2, cy, 2*(r/2)+1, 2*(r/2)+1+rd, GP_CIRCLE_SEG1 | GP_CIRCLE_SEG4, col);
	/* fallthrough */
	case GP_WIDGET_STOCK_SPEAKER_MID:
		gp_fill_ring_seg(pix, cx+r/2, cy, r/3, r/3+rd, GP_CIRCLE_SEG1 | GP_CIRCLE_SEG4, col);
	break;
	case GP_WIDGET_STOCK_SPEAKER_INC:
		gp_fill_rect(pix, cx+2*(r/2)-lt, cy-r/2, cx+2*(r/2)+lt, cy+r/2, col);
	/* fallthrough */
	case GP_WIDGET_STOCK_SPEAKER_DEC:
		gp_fill_rect(pix, cx+r/2, cy-lt, cx+3*(r/2), cy+lt, col);
	break;
	default:
	break;
	}
}

static void render_text_lines(const gp_widget_render_ctx *ctx,
                              gp_coord x0, gp_coord y0,
                              gp_coord x1, gp_coord y1,
			      gp_pixel col)
{
	gp_pixmap *pix = ctx->buf;
	gp_size dy = (y1 - y0)+1;
	gp_size ls = dy/10;
	gp_size sp;

	if (dy < 5)
		sp = dy - 2*(ls + 1);
	else
		sp = (dy - 3*(ls + 1))/2+1;

	gp_fill_rect_xyxy(pix, x0, y1, x1, y1-ls, col);
	y1-=sp+ls;
	gp_fill_rect_xyxy(pix, x0, y1, x1, y1-ls, col);

	if (dy < 5)
		return;

	y1-=sp+ls;
	gp_fill_rect_xyxy(pix, x0, y1, x1, y1-ls, col);
}

static void render_stock_hardware(const gp_widget_render_ctx *ctx,
                                  gp_coord x, gp_coord y,
                                  gp_size w, gp_size h)
{
	gp_pixmap *pix = ctx->buf;
	gp_coord cx = x + w/2;
	gp_coord cy = y + h/2;
	gp_size c = GP_MIN(w, h)/3;
	gp_size dc = c/2;

	gp_coord l_end = GP_MIN(w, h)/2-c;
	gp_coord l_start = dc/10+2;
	gp_coord l_size = 1 + c/9;
	gp_coord sp_size = 1 + c/20;
	gp_coord c_sp_size = 1 + c/8;
	gp_coord legs = (2*c - 2*c_sp_size + sp_size)/(l_size + sp_size);
	gp_coord i;

	gp_fill_rect_xywh(pix, x, y, w, h, ctx->bg_color);

	gp_fill_rect_xyxy(pix, cx-c, cy-c, cx+c, cy+c, ctx->text_color);
	gp_fill_rect_xyxy(pix, cx-c/3, cy-c/3, cx+c/3, cy+c/3, ctx->fg_color);
	gp_putpixel(pix, cx-c/3, cy-c/3, ctx->text_color);

	gp_fill_circle(pix, cx-c+c/4+1, cy-c+c/4+1, c/8, ctx->fg_color);

	c_sp_size = (2*c - legs*l_size - (legs-1) * sp_size+1)/2;

	for (i = 0; i < legs; i++) {
		gp_coord off = -c + c_sp_size + i * (l_size + sp_size);

		gp_fill_rect_xyxy(pix, cx+off, cy-c-l_start, cx+off+l_size-1, cy-c-l_end, ctx->text_color);
		gp_fill_rect_xyxy(pix, cx+off, cy+c+l_start, cx+off+l_size-1, cy+c+l_end, ctx->text_color);

		gp_fill_rect_xyxy(pix, cx-c-l_start, cy+off, cx-c-l_end, cy+off+l_size-1, ctx->text_color);
		gp_fill_rect_xyxy(pix, cx+c+l_start, cy+off, cx+c+l_end, cy+off+l_size-1, ctx->text_color);
	}
}

static void render_stock_software(const gp_widget_render_ctx *ctx,
                                  gp_coord x, gp_coord y,
                                  gp_size w, gp_size h)
{
	gp_pixmap *pix = ctx->buf;
	gp_coord cx = x + w/2;
	gp_coord cy = y + h/2;
	gp_coord i;

	gp_size iw = w/2;
	gp_size ih = GP_MIN(iw, 7*h/16);
	gp_size th = GP_MIN(3*w/8, 3*h/8)/8;

	gp_size header = GP_ODD_UP(ih/2);

	gp_fill_rect_xywh(pix, x, y, w, h, ctx->bg_color);

	gp_fill_rect_xyxy(pix, cx-iw, cy-ih, cx+iw, cy+ih, ctx->text_color);
	gp_fill_rect_xyxy(pix, cx-iw+th+1, cy-ih+header, cx+iw-th-1, cy+ih-th-1, ctx->fg_color);

	gp_size r = header/8;
	gp_size off = th + 1 + r;

	for (i = 0; i < 3; i++) {
		gp_fill_circle(pix, cx+iw-off, cy-ih+header/2, r, ctx->fg_color);
		off += r ? 3*r+1 : 2;
	}

	render_text_lines(ctx, cx-iw+th+ih/2+1, cy-ih+header+ih/3+1,
                          cx+iw-th-ih/2-1, cy+ih-th-1-ih/3-1, ctx->sel_color);
}

#define ROTATE(s, c, cx, x, cy, y) (1.00 * (cx) + (x)*(c) - (y)*(s) + 0.5), (1.00 * (cy) + (x)*(s) + (y)*(c) + 0.5)
#define ROT_PI_4(cx, x, cy, y) ROTATE(0.707, 0.707, cx, x, cy, y)
#define ROT_1_PI_8(cx, x, cy, y) ROTATE(0.38268343, 0.92387953, cx, x, cy, y)
#define ROT_3_PI_8(cx, x, cy, y) ROTATE(0.92387953, 0.38268343, cx, x, cy, y)

static void render_stock_settings(const gp_widget_render_ctx *ctx,
                                  gp_coord x, gp_coord y,
                                  gp_size w, gp_size h)
{
	gp_pixmap *pix = ctx->buf;
	gp_coord cx = x + w/2;
	gp_coord cy = y + h/2;

	gp_coord co = GP_MIN(w/2, h/2);
	gp_coord ci = co - co/4-2;
	gp_coord cc = GP_MIN(w/6, h/6);
	gp_coord tw = co/11;

	gp_fill_rect_xywh(pix, x, y, w, h, ctx->bg_color);

	gp_coord poly[] = {
		cx - co, cy + tw,
		cx - co, cy - tw,
		ROT_1_PI_8(cx, -ci, cy,  tw),
		ROT_1_PI_8(cx, -ci, cy, -tw),
		ROT_PI_4(cx, -co, cy,  tw),
		ROT_PI_4(cx, -co, cy, -tw),
		ROT_3_PI_8(cx, -ci, cy,  tw),
		ROT_3_PI_8(cx, -ci, cy, -tw),
		cx - tw, cy - co,
		cx + tw, cy - co,
		ROT_1_PI_8(cx, -tw, cy, -ci),
		ROT_1_PI_8(cx,  tw, cy, -ci),
		ROT_PI_4(cx, -tw, cy, -co),
		ROT_PI_4(cx,  tw, cy, -co),
		ROT_3_PI_8(cx, -tw, cy, -ci),
		ROT_3_PI_8(cx,  tw, cy, -ci),
		cx + co, cy - tw,
		cx + co, cy + tw,
		ROT_1_PI_8(cx, ci, cy, -tw),
		ROT_1_PI_8(cx, ci, cy,  tw),
		ROT_PI_4(cx, co, cy, -tw),
		ROT_PI_4(cx, co, cy,  tw),
		ROT_3_PI_8(cx, ci, cy, -tw),
		ROT_3_PI_8(cx, ci, cy,  tw),
		cx + tw, cy + co,
		cx - tw, cy + co,
		ROT_1_PI_8(cx,  tw, cy, ci),
		ROT_1_PI_8(cx, -tw, cy, ci),
		ROT_PI_4(cx,  tw, cy, co),
		ROT_PI_4(cx, -tw, cy, co),
		ROT_3_PI_8(cx,  tw, cy, ci),
		ROT_3_PI_8(cx, -tw, cy, ci),
	};

	gp_fill_polygon(pix, GP_ARRAY_SIZE(poly)/2, poly, ctx->text_color);

	gp_fill_circle(pix, cx, cy, cc, ctx->fg_color);
}

static void render_stock_save(const gp_widget_render_ctx *ctx,
                              gp_coord x, gp_coord y,
                              gp_size w, gp_size h)
{
	gp_pixmap *pix = ctx->buf;
	gp_coord cx = x + w/2;
	gp_coord cy = y + h/2;
	gp_coord sh = GP_MIN(7*w/16, 7*h/16);
	gp_size th = GP_MIN(3*w/8, 3*h/8)/8;
	gp_size cs = sh/3;
	gp_size cvr_s = (2 * sh - 2 * th) / 4;
	gp_size cvr_d = 3 * cvr_s / 2+1;

	gp_fill_rect_xywh(pix, x, y, w, h, ctx->bg_color);

	gp_coord poly[] = {
		cx - sh, cy - sh,
		cx - sh + th, cy - sh,
		/* inner polygon starts here */
		cx - sh + th, cy + sh - th,
		cx + sh - th, cy + sh - th,
		cx + sh - th, cy - sh + th + cs,
		cx + sh - th - cs, cy - sh + th,
		/* cover start */
		cx + cvr_s, cy - sh + th,
		cx + cvr_s, cy - sh + th + cvr_d,
		cx - cvr_s, cy - sh + th + cvr_d,
		cx - cvr_s, cy - sh + th,
		/* cover end */
		cx - sh + th, cy - sh + th,
		/* inner polygon ends here */
		cx - sh + th, cy - sh,
		cx + sh - cs - th/2, cy - sh,
		cx + sh, cy - sh + cs + th/2,
		cx + sh, cy + sh,
		cx - sh, cy + sh,
		cx - sh, cy - sh,
	};

	/* fills inner part of floppy */
	gp_fill_polygon(pix, 9, poly + 2, ctx->warn_color);
	/* draws border and cover */
	gp_fill_polygon(pix, GP_ARRAY_SIZE(poly)/2, poly, ctx->text_color);
	/* window in the cover */
	gp_fill_rect_xyxy(pix, cx+cvr_s-1-cvr_s/4, cy - sh + th+1 + cvr_d/5,
			       cx+cvr_s-1-2*(cvr_s/4), cy - sh + th+cvr_d-1 - cvr_d/5,
			       ctx->warn_color);

	/* label */
	gp_coord lx0 = cx - sh + th + sh/3;
	gp_coord ly0 = cy - sh + th + cvr_d + sh/3;
	gp_coord lx1 = cx + sh - th - sh/3;
	gp_coord ly1 = cy + sh - th - sh/3;

	render_text_lines(ctx, lx0, ly0, lx1, ly1, ctx->text_color);
}


static void render_stock_file(const gp_widget_render_ctx *ctx,
                              gp_coord x, gp_coord y,
                              gp_size w, gp_size h)
{
	gp_pixmap *pix = ctx->buf;
	gp_coord cx = x + w/2;
	gp_coord cy = y + h/2;
	gp_size sh = 7*h/16;
	gp_size cs = sh/2;
	gp_size sw = GP_MIN(5*h/16, 3*w/4);
	gp_size th = GP_MIN(3*w/8, 3*h/8)/8;

	gp_fill_rect_xywh(pix, x, y, w, h, ctx->bg_color);

	gp_coord poly[] = {
		cx - sw, cy - sh,
		cx - sw + th, cy - sh,
		/* inner polygon starts here */
		cx - sw + th, cy + sh - th,
		cx + sw - th, cy + sh - th,
		/* corner start */
		cx + sw - th, cy - sh + 2*th + cs,
		cx + sw - cs - 2*th, cy - sh + 2*th + cs,
		cx + sw - cs - 2*th, cy - sh + th,
		cx + sw - cs - th, cy - sh + th,
		cx + sw - cs - th, cy - sh + th + cs,
		cx + sw - th, cy - sh + th + cs,
		/* corner end */
		cx + sw - th - cs, cy - sh + th,
		cx - sw + th, cy - sh + th,
		/* inner polygon ends here */
		cx - sw + th, cy - sh,
		cx + sw - cs - th/2, cy - sh,
		cx + sw, cy - sh + cs + th/2,
		cx + sw, cy + sh,
		cx - sw, cy + sh,
		cx - sw, cy - sh,
	};

	gp_coord in_poly[] = {
		cx - sw + th, cy + sh - th,
		cx + sw - th, cy + sh - th,
		cx + sw - th, cy - sh + th + cs,
		cx + sw - th - cs, cy - sh + th,
		cx - sw + th, cy - sh + th,
	};

	gp_fill_polygon(pix, GP_ARRAY_SIZE(in_poly)/2, in_poly, ctx->fg_color);
	gp_fill_polygon(pix, GP_ARRAY_SIZE(poly)/2, poly, ctx->text_color);

	/* text */
	gp_coord lx0 = cx - sw + th + sh/3;
	gp_coord ly0 = cy - sh + cs + th + sh/3+1;
	gp_coord lx1 = cx + sw - th - sh/3;
	gp_coord ly1 = cy + sh - th - sh/3-1;

	render_text_lines(ctx, lx0, ly0, lx1, ly1, ctx->sel_color);
}

static void render_stock_arrow(const gp_widget_render_ctx *ctx,
                               int type,
                               gp_coord x, gp_coord y,
                               gp_size w, gp_size h)
{
	gp_pixmap *pix = ctx->buf;
	gp_coord cx = x + w/2;
	gp_coord cy = y + h/2;

	gp_size th = GP_MAX(GP_MIN(w, h)/10, 1u);
	gp_size rh = h/3;
	gp_size wh = w/3;

	gp_fill_rect_xywh(pix, x, y, w, h, ctx->bg_color);

	switch (type) {
	case GP_WIDGET_STOCK_ARROW_UP:
		gp_fill_rect_xyxy(pix, cx-th, cy-rh+2*th, cx+th, cy+rh, ctx->text_color);
		gp_symbol(pix, cx, cy-rh+2*th, 4*th, 2*th, GP_TRIANGLE_UP, ctx->text_color);
	break;
	case GP_WIDGET_STOCK_ARROW_DOWN:
		gp_fill_rect_xyxy(pix, cx-th, cy-rh, cx+th, cy+rh-2*th, ctx->text_color);
		gp_symbol(pix, cx, cy+rh-2*th, 4*th, 2*th, GP_TRIANGLE_DOWN, ctx->text_color);
	break;
	case GP_WIDGET_STOCK_ARROW_LEFT:
		gp_fill_rect_xyxy(pix, cx-wh+2*th, cy-th, cx+wh, cy+th, ctx->text_color);
		gp_symbol(pix, cx-wh+2*th, cy, 2*th, 4*th, GP_TRIANGLE_LEFT, ctx->text_color);
	break;
	case GP_WIDGET_STOCK_ARROW_RIGHT:
		gp_fill_rect_xyxy(pix, cx-wh, cy-th, cx+wh-2*th, cy+th, ctx->text_color);
		gp_symbol(pix, cx+wh-2*th, cy, 2*th, 4*th, GP_TRIANGLE_RIGHT, ctx->text_color);
	break;
	}
}

static void render_stock_refresh(const gp_widget_render_ctx *ctx,
                                 gp_coord x, gp_coord y,
                                 gp_size w, gp_size h)
{
	gp_pixmap *pix = ctx->buf;
	gp_coord cx = x + w/2;
	gp_coord cy = y + h/2;

	gp_size th = GP_MIN(w, h)/16;
	gp_size s = GP_MAX(GP_MIN(w, h)/5, 1u);
	gp_size r = GP_MIN(w, h)/2-s;

	gp_fill_rect_xywh(pix, x, y, w, h, ctx->bg_color);

	gp_fill_ring_seg(pix, cx, cy, r - th, r + th,
	                 GP_CIRCLE_SEG2|GP_CIRCLE_SEG3|GP_CIRCLE_SEG4,
	                 ctx->text_color);

	gp_size sr = GP_MAX(s/3, 1u);

	gp_coord poly_1[] = {
		cx+th-sr, cy-r-s,
		cx+th, cy-r-s,
		cx+th+s, cy-r,
		cx+th, cy-r+s,
		cx+th-sr, cy-r+s,
	};

	gp_fill_polygon(pix, GP_ARRAY_SIZE(poly_1)/2, poly_1, ctx->text_color);
}

static void render_stock_shuffle_off(const gp_widget_render_ctx *ctx,
                                     gp_coord x, gp_coord y,
                                     gp_size w, gp_size h)
{
	gp_pixmap *pix = ctx->buf;
	gp_coord cx = x + w/2;
	gp_coord cy = y + h/2;

	gp_size wh = w/3;

	gp_size th = GP_MIN(w, h)/16;

	gp_fill_rect_xywh(pix, x, y, w, h, ctx->bg_color);

	gp_size yd = h/4;

	gp_coord poly_1[] = {
		cx-wh-3*th, cy+yd+th,
		cx-wh-3*th, cy+yd-th,
		cx+wh-th, cy+yd-th,
		cx+wh-th, cy+yd-3*th,
		cx+wh, cy+yd-3*th,
		cx+wh+3*th, cy+yd,
		cx+wh, cy+yd+3*th,
		cx+wh-th, cy+yd+3*th,
		cx+wh-th, cy+yd+th,
	};

	gp_fill_polygon(pix, GP_ARRAY_SIZE(poly_1)/2, poly_1, ctx->text_color);

	gp_coord poly_2[] = {
		cx-wh-3*th, cy-yd+th,
		cx-wh-3*th, cy-yd-th,
		cx+wh-th, cy-yd-th,
		cx+wh-th, cy-yd-3*th,
		cx+wh, cy-yd-3*th,
		cx+wh+3*th, cy-yd,
		cx+wh, cy-yd+3*th,
		cx+wh-th, cy-yd+3*th,
		cx+wh-th, cy-yd+th,
	};

	gp_fill_polygon(pix, GP_ARRAY_SIZE(poly_2)/2, poly_2, ctx->text_color);
}

static void render_stock_shuffle_on(const gp_widget_render_ctx *ctx,
                                    gp_coord x, gp_coord y,
                                    gp_size w, gp_size h)
{
	gp_pixmap *pix = ctx->buf;
	gp_coord cx = x + w/2;
	gp_coord cy = y + h/2;

	gp_size wh = w/3;

	gp_size th = GP_MIN(w, h)/16;

	gp_fill_rect_xywh(pix, x, y, w, h, ctx->bg_color);

	gp_size yd = h/4;

	cx -= 2*th;
	gp_size xs = yd/2;

	gp_coord poly_1[] = {
		cx-wh-th, cy-yd-th,
		cx-wh-th, cy-yd+th,
		cx-wh+xs, cy-yd+th,
		cx+wh-xs, cy+yd+th,
		cx+wh+th, cy+yd+th,

		cx+wh+th, cy+yd+3*th,
		cx+wh+2*th, cy+yd+3*th,
		cx+wh+5*th, cy+yd,
		cx+wh+2*th, cy+yd-3*th,
		cx+wh+th, cy+yd-3*th,

		cx+wh+th, cy+yd-th,
		cx+wh-xs+th, cy+yd-th,
		cx-wh+xs+th, cy-yd-th,
	};

	gp_fill_polygon(pix, GP_ARRAY_SIZE(poly_1)/2, poly_1, ctx->text_color);

	gp_coord poly_2[] = {
		cx-wh-th, cy+yd+th,
		cx-wh-th, cy+yd-th,
		cx-wh+xs, cy+yd-th,
		cx+wh-xs, cy-yd-th,
		cx+wh+th, cy-yd-th,

		cx+wh+th, cy-yd-3*th,
		cx+wh+2*th, cy-yd-3*th,
		cx+wh+5*th, cy-yd,
		cx+wh+2*th, cy-yd+3*th,
		cx+wh+th, cy-yd+3*th,

		cx+wh+th, cy-yd+th,
		cx+wh-xs+th, cy-yd+th,
		cx-wh+xs+th, cy+yd+th,
	};

	gp_fill_polygon(pix, GP_ARRAY_SIZE(poly_2)/2, poly_2, ctx->text_color);
}

static void stock_render(gp_widget *self, const gp_offset *offset,
                         const gp_widget_render_ctx *ctx, int flags)
{
	gp_coord x = self->x + offset->x;
	gp_coord y = self->y + offset->y;
	gp_coord cx = x + self->w/2;
	gp_coord cy = y + self->h/2;
	gp_coord r = GP_MIN(self->w/2, self->h/2);

	(void)flags;

	switch (self->stock->type) {
	case GP_WIDGET_STOCK_SPEAKER_MIN:
	case GP_WIDGET_STOCK_SPEAKER_MAX:
	case GP_WIDGET_STOCK_SPEAKER_MID:
	case GP_WIDGET_STOCK_SPEAKER_MUTE:
	case GP_WIDGET_STOCK_SPEAKER_INC:
	case GP_WIDGET_STOCK_SPEAKER_DEC:
		gp_fill_rect_xywh(ctx->buf, x, y, self->w, self->h, ctx->bg_color);
		render_stock_speaker(ctx, cx, cy, r/2, self);
	break;
	case GP_WIDGET_STOCK_INFO:
		render_stock_info(ctx, x, y, self->w, self->h);
	break;
	case GP_WIDGET_STOCK_WARN:
		render_stock_err_warn(ctx, x, y, self->w, self->h, ctx->warn_color);
	break;
	case GP_WIDGET_STOCK_ERR:
		render_stock_err_warn(ctx, x, y, self->w, self->h, ctx->alert_color);
	break;
	case GP_WIDGET_STOCK_QUESTION:
		render_stock_question(ctx, x, y, self->w, self->h);
	break;
	case GP_WIDGET_STOCK_HARDWARE:
		render_stock_hardware(ctx, x, y, self->w, self->h);
	break;
	case GP_WIDGET_STOCK_SOFTWARE:
		render_stock_software(ctx, x, y, self->w, self->h);
	break;
	case GP_WIDGET_STOCK_SETTINGS:
		render_stock_settings(ctx, x, y, self->w, self->h);
	break;
	case GP_WIDGET_STOCK_SAVE:
		render_stock_save(ctx, x, y, self->w, self->h);
	break;
	case GP_WIDGET_STOCK_FILE:
		render_stock_file(ctx, x, y, self->w, self->h);
	break;
	case GP_WIDGET_STOCK_REFRESH:
		render_stock_refresh(ctx, x, y, self->w, self->h);
	break;
	case GP_WIDGET_STOCK_ARROW_UP:
	case GP_WIDGET_STOCK_ARROW_DOWN:
	case GP_WIDGET_STOCK_ARROW_LEFT:
	case GP_WIDGET_STOCK_ARROW_RIGHT:
		render_stock_arrow(ctx, self->stock->type, x, y, self->w, self->h);
	break;
	case GP_WIDGET_STOCK_SHUFFLE_ON:
		render_stock_shuffle_on(ctx, x, y, self->w, self->h);
	break;
	case GP_WIDGET_STOCK_SHUFFLE_OFF:
		render_stock_shuffle_off(ctx, x, y, self->w, self->h);
	break;
	}

	gp_widget_ops_blit(ctx, x, y, self->w, self->h);
}

static struct stock_types {
	const char *str_type;
	enum gp_widget_stock_type type;
} stock_types[] = {
	{"err", GP_WIDGET_STOCK_ERR},
	{"warn", GP_WIDGET_STOCK_WARN},
	{"info", GP_WIDGET_STOCK_INFO},
	{"question", GP_WIDGET_STOCK_QUESTION},

	{"speaker_min", GP_WIDGET_STOCK_SPEAKER_MIN},
	{"speaker_mid", GP_WIDGET_STOCK_SPEAKER_MID},
	{"speaker_max", GP_WIDGET_STOCK_SPEAKER_MAX},
	{"speaker_mute", GP_WIDGET_STOCK_SPEAKER_MUTE},
	{"speaker_inc", GP_WIDGET_STOCK_SPEAKER_INC},
	{"speaker_dec", GP_WIDGET_STOCK_SPEAKER_DEC},

	{"hardware", GP_WIDGET_STOCK_HARDWARE},
	{"software", GP_WIDGET_STOCK_SOFTWARE},
	{"settings", GP_WIDGET_STOCK_SETTINGS},
	{"save", GP_WIDGET_STOCK_SAVE},
	{"file", GP_WIDGET_STOCK_FILE},

	{"refresh", GP_WIDGET_STOCK_REFRESH},

	{"shuffle_on", GP_WIDGET_STOCK_SHUFFLE_ON},
	{"shuffle_off", GP_WIDGET_STOCK_SHUFFLE_OFF},

	{"arrow_up", GP_WIDGET_STOCK_ARROW_UP},
	{"arrow_down", GP_WIDGET_STOCK_ARROW_DOWN},
	{"arrow_left", GP_WIDGET_STOCK_ARROW_LEFT},
	{"arrow_right", GP_WIDGET_STOCK_ARROW_RIGHT},
};

static int gp_widget_stock_type_from_str(const char *type)
{
	unsigned int i;

	for (i = 0; i < GP_ARRAY_SIZE(stock_types); i++) {
		if (!strcmp(type, stock_types[i].str_type))
			return stock_types[i].type;
	}

	return -1;
}

static const char *gp_widget_stock_type_name(enum gp_widget_stock_type type)
{
	unsigned int i;

	for (i = 0; i < GP_ARRAY_SIZE(stock_types); i++) {
		if (stock_types[i].type == type)
			return stock_types[i].str_type;
	}

	return NULL;
}

static gp_widget *stock_from_json(json_object *json, gp_htable **uids)
{
	const char *stock = NULL;
	const char *min_size_str = NULL;
	gp_widget_size min_size = GP_WIDGET_SIZE_DEFAULT;
	int type;

	(void)uids;

	json_object_object_foreach(json, key, val) {
		if (!strcmp(key, "stock"))
			stock = json_object_get_string(val);
		else if (!strcmp(key, "min_size"))
			min_size_str = json_object_get_string(val);
		else
			GP_WARN("Invalid stock key '%s'", key);
	}

	if (!stock) {
		GP_WARN("Missing stock type");
		return NULL;
	}

	type = gp_widget_stock_type_from_str(stock);
	if (type < 0) {
		GP_WARN("Unknown stock '%s'", stock);
		return NULL;
	}

	if (min_size_str) {
		if (gp_widget_size_units_parse(min_size_str, &min_size))
			return NULL;
	}

	return gp_widget_stock_new(type, min_size);
}

static int stock_event(gp_widget *self, const gp_widget_render_ctx *ctx, gp_event *ev)
{
	(void) ctx;

	if (ev->type != GP_EV_KEY)
		return 0;

	if (ev->code == GP_EV_KEY_UP)
		return 0;

	switch (ev->val) {
	case GP_BTN_LEFT:
	case GP_BTN_PEN:
	case GP_KEY_ENTER:
	        gp_widget_send_widget_event(self, 0);
		return 1;
	}

	return 0;
}

struct gp_widget_ops gp_widget_stock_ops = {
	.min_w = stock_min_w,
	.min_h = stock_min_h,
	.render = stock_render,
	.event = stock_event,
	.from_json = stock_from_json,
	.id = "stock",
};

gp_widget *gp_widget_stock_new(enum gp_widget_stock_type type, gp_widget_size min_size)
{
	const gp_widget_render_ctx *ctx = gp_widgets_render_ctx();
	gp_widget *ret;

	if (!gp_widget_stock_type_name(type)) {
		GP_WARN("Invalid stock type %u", type);
		return NULL;
	}

	ret = gp_widget_new(GP_WIDGET_STOCK, GP_WIDGET_CLASS_NONE, sizeof(struct gp_widget_stock));
	if (!ret)
		return NULL;

		GP_ODD_UP(gp_text_ascent(ctx->font) + 2 * ctx->padd);

	if (GP_WIDGET_SIZE_EQ(min_size, GP_WIDGET_SIZE_DEFAULT))
		ret->stock->min_size = GP_WIDGET_SIZE(0, 2, 1);
	else
		ret->stock->min_size = min_size;

	ret->stock->type = type;
	ret->no_events = 1;

	return ret;
}

void gp_widget_stock_type_set(gp_widget *self, enum gp_widget_stock_type type)
{
	GP_WIDGET_ASSERT(self, GP_WIDGET_STOCK, );

	if (self->stock->type == type)
		return;

	if (!gp_widget_stock_type_name(type)) {
		GP_WARN("Invalid stock type %u", type);
		return;
	}

	self->stock->type = type;
	gp_widget_redraw(self);
}
