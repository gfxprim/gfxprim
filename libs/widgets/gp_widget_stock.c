//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>

#include <widgets/gp_widgets.h>

struct gp_widget_stock {
	enum gp_widget_stock_type type;
	gp_widget_size min_size;
};

static unsigned int stock_min_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	struct gp_widget_stock *stock = GP_WIDGET_PAYLOAD(self);

	return GP_ODD_UP(gp_widget_size_units_get(&stock->min_size, ctx));
}

static unsigned int stock_min_h(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	struct gp_widget_stock *stock = GP_WIDGET_PAYLOAD(self);

	return GP_ODD_UP(gp_widget_size_units_get(&stock->min_size, ctx));
}

static void render_stock_err_warn(gp_pixmap *pix,
                                  gp_coord x, gp_coord y,
                                  gp_size w, gp_size h, gp_pixel color, gp_pixel bg_col,
                                  const gp_widget_render_ctx *ctx)
{
	gp_coord cx = x + w/2;
	gp_coord cy = y + h/2;
	gp_size c = GP_MIN(w, h)/2 - 2;
	gp_size r = c/5;

	gp_fill_rrect_xywh(pix, x, y, w, h, bg_col, color, ctx->text_color);

	if (r > 1)
		gp_fill_circle(pix, cx, cy+h/4, r, ctx->text_color);
	else
		gp_fill_rect_xyxy(pix, cx-r, cy+h/4-r, cx+r, cy+h/4+r, ctx->text_color);

	gp_fill_rect_xyxy(pix, cx-r, cy-h/4-r, cx+r, cy+r, ctx->text_color);

}

static void render_stock_info(gp_pixmap *pix,
                              gp_coord x, gp_coord y,
                              gp_size w, gp_size h,
                              gp_pixel bg_col, const gp_widget_render_ctx *ctx)
{
	gp_size c = GP_MIN(w, h)/2 - 2;
	gp_size r = c/5;
	gp_coord cx = x + w/2;
	gp_coord cy = y + h/2;

	gp_fill_rrect_xywh(pix, x, y, w, h, bg_col, ctx->sel_color, ctx->text_color);

	if (r > 1)
		gp_fill_circle(pix, cx, cy-h/4, r, ctx->text_color);
	else
		gp_fill_rect_xyxy(pix, cx-r, cy-h/4-r, cx+r, cy-h/4+r, ctx->text_color);

	gp_fill_rect_xyxy(pix, cx-r, cy+h/4, cx+r, cy-r, ctx->text_color);

	gp_fill_rect_xyxy(pix, cx-2*r, cy+h/4, cx+2*r, cy+h/4+r/2, ctx->text_color);

	gp_fill_rect_xyxy(pix, cx-2*r, cy-r, cx, cy-r+r/2, ctx->text_color);
}

static void render_stock_question(gp_pixmap *pix,
                                  gp_coord x, gp_coord y,
                                  gp_size w, gp_size h, gp_pixel bg_col,
				  const gp_widget_render_ctx *ctx)
{
	gp_coord cx = x + w/2;
	gp_coord cy = y + h/2;
	gp_size c = GP_MIN(w, h)/2 - 2;
	gp_size r = c/5;

	gp_fill_rrect_xywh(pix, x, y, w, h, bg_col, ctx->sel_color, ctx->text_color);

	if (r > 1)
		gp_fill_circle(pix, cx, cy+h/4+r, r, ctx->text_color);
	else
		gp_fill_rect_xyxy(pix, cx-r, cy+h/4, cx+r, cy+h/4+2*r, ctx->text_color);

	gp_fill_rect_xyxy(pix, cx-r, cy-h/4+3*r, cx+r, cy+2*r, ctx->text_color);

	gp_fill_ring_seg(pix, cx, cy-h/4 + r, r, 3*r, GP_CIRCLE_SEG1 | GP_CIRCLE_SEG2 |GP_CIRCLE_SEG4, ctx->text_color);

	gp_fill_triangle(pix, cx-r, cy-h/4+3*r, cx, cy-h/4+2*r, cx, cy-h/4+3*r, ctx->text_color);
}

static void render_stock_speaker(gp_pixmap *pix,
                                 gp_coord x, gp_coord y, gp_size w, gp_size h,
                                 enum gp_widget_stock_type type,
                                 const gp_widget_render_ctx *ctx)
{
	gp_pixel col = type & GP_WIDGET_STOCK_FOCUSED ? ctx->sel_color : ctx->text_color;
	gp_coord cx = x + w/2;
	gp_coord cy = y + h/2;

	gp_size rx = w/4;
	gp_size ry = h/2 - rx - h/10;
	gp_size edge = rx/3;

	gp_coord poly[] = {
		cx-rx-edge, cy-ry,
		x, cy-ry,
		x, cy+ry,
		cx-rx-edge, cy+ry,
		cx-edge, cy+rx+ry,
		cx, cy+rx+ry,
		cx, cy-rx-ry,
		cx-edge, cy-rx-ry,
	};

	gp_fill_polygon(pix, 0, 0, GP_ARRAY_SIZE(poly)/2, poly, col);

	/* space between speaker and symbol */
	gp_size sp = GP_MAX(1u, rx/2);
	/* (half of) symbol size */
	gp_size swh = w/4 - (sp+1)/2;
	gp_size sw = 2*swh;
	/* most left x coordinate of symbol */
	gp_coord sx = cx + sp;
	/* line width */
	gp_size lw = (w)/16;
	/* ring radius */
	gp_size r1 = (rx + ry)-1;
	/* circle radius */
	gp_size r2 = r1/2-lw;

	gp_size i;

	gp_coord scx = cx + (sp+1)/2 + 1;

	switch (GP_WIDGET_STOCK_TYPE(type)) {
	case GP_WIDGET_STOCK_SPEAKER_MUTE:
		for (i = 0; i <= (7*lw+2)/5; i++) {
			gp_line(pix, sx+i, cy-swh, sx+sw, cy+swh-i, ctx->alert_color);
			gp_line(pix, sx, cy-swh+i, sx+sw-i, cy+swh, ctx->alert_color);
			gp_line(pix, sx+i, cy+swh, sx+sw, cy-swh+i, ctx->alert_color);
			gp_line(pix, sx, cy+swh-i, sx+sw-i, cy-swh, ctx->alert_color);
		}
	break;
	case GP_WIDGET_STOCK_SPEAKER_MIN:
	break;
	case GP_WIDGET_STOCK_SPEAKER_MAX:
		gp_fill_ring_seg(pix, scx, cy, r1-2*lw, r1, GP_CIRCLE_SEG1 | GP_CIRCLE_SEG4, col);
	/* fallthrough */
	case GP_WIDGET_STOCK_SPEAKER_MID:
		gp_fill_circle_seg(pix, scx, cy, r2, GP_CIRCLE_SEG1 | GP_CIRCLE_SEG4, col);
	break;
	case GP_WIDGET_STOCK_SPEAKER_INC:
		gp_fill_rect(pix, sx+swh-lw, cy-swh, sx+swh+lw, cy+swh, col);
	/* fallthrough */
	case GP_WIDGET_STOCK_SPEAKER_DEC:
		gp_fill_rect(pix, sx, cy-lw, sx+sw, cy+lw, col);
	break;
	default:
	break;
	}
}

static void render_text_lines(gp_pixmap *pix,
                              gp_coord x0, gp_coord y0,
                              gp_coord x1, gp_coord y1,
                              gp_pixel col)
{
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

static void render_stock_hardware(gp_pixmap *pix,
                                  gp_coord x, gp_coord y,
                                  gp_size w, gp_size h, gp_pixel bg_col,
                                  const gp_widget_render_ctx *ctx)
{
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

	gp_fill_rect_xywh(pix, x, y, w, h, bg_col);

	gp_fill_rect_xyxy(pix, cx-c, cy-c, cx+c, cy+c, ctx->text_color);
	gp_fill_rect_xyxy(pix, cx-c/3, cy-c/3, cx+c/3, cy+c/3, ctx->fg_color);
	gp_putpixel(pix, cx-c/3, cy-c/3, ctx->text_color);

	gp_fill_circle(pix, cx-c+c/4+1, cy-c+c/4+1, c/8, ctx->sel_color);

	c_sp_size = (2*c - legs*l_size - (legs-1) * sp_size+1)/2;

	for (i = 0; i < legs; i++) {
		gp_coord off = -c + c_sp_size + i * (l_size + sp_size);

		gp_fill_rect_xyxy(pix, cx+off, cy-c-l_start, cx+off+l_size-1, cy-c-l_end, ctx->text_color);
		gp_fill_rect_xyxy(pix, cx+off, cy+c+l_start, cx+off+l_size-1, cy+c+l_end, ctx->text_color);

		gp_fill_rect_xyxy(pix, cx-c-l_start, cy+off, cx-c-l_end, cy+off+l_size-1, ctx->text_color);
		gp_fill_rect_xyxy(pix, cx+c+l_start, cy+off, cx+c+l_end, cy+off+l_size-1, ctx->text_color);
	}
}

static void render_stock_software(gp_pixmap *pix,
                                  gp_coord x, gp_coord y,
                                  gp_size w, gp_size h, gp_pixel bg_col,
                                  const gp_widget_render_ctx *ctx)
{
	gp_coord cx = x + w/2;
	gp_coord cy = y + h/2;
	gp_coord i;

	gp_size iw = w/2;
	gp_size ih = GP_MIN(iw, 7*h/16);
	gp_size th = GP_MIN(3*w/8, 3*h/8)/8;

	gp_size header = GP_ODD_UP(ih/2);

	gp_fill_rect_xywh(pix, x, y, w, h, bg_col);

	gp_fill_rect_xyxy(pix, cx-iw, cy-ih, cx+iw, cy+ih, ctx->text_color);
	gp_fill_rect_xyxy(pix, cx-iw+th+1, cy-ih+header, cx+iw-th-1, cy+ih-th-1, ctx->fg_color);

	gp_size r = header/8;
	gp_size off = th + 1 + r;

	for (i = 0; i < 3; i++) {
		gp_fill_circle(pix, cx+iw-off, cy-ih+header/2, r, ctx->fg_color);
		off += r ? 3*r+1 : 2;
	}

	render_text_lines(pix, cx-iw+th+ih/2+1, cy-ih+header+ih/3+1,
                          cx+iw-th-ih/2-1, cy+ih-th-1-ih/3-1, ctx->sel_color);
}

#define ROTATE(s, c, cx, x, cy, y) (1.00 * (cx) + (x)*(c) - (y)*(s) + 0.5), (1.00 * (cy) + (x)*(s) + (y)*(c) + 0.5)
#define ROT_PI_4(cx, x, cy, y) ROTATE(0.707, 0.707, cx, x, cy, y)
#define ROT_1_PI_8(cx, x, cy, y) ROTATE(0.38268343, 0.92387953, cx, x, cy, y)
#define ROT_3_PI_8(cx, x, cy, y) ROTATE(0.92387953, 0.38268343, cx, x, cy, y)

static void render_stock_settings(gp_pixmap *pix,
                                  gp_coord x, gp_coord y,
                                  gp_size w, gp_size h, gp_pixel bg_col,
                                  const gp_widget_render_ctx *ctx)
{
	gp_coord cx = x + w/2;
	gp_coord cy = y + h/2;

	gp_coord co = GP_MIN(w/2, h/2);
	gp_coord ci = co - co/4-2;
	gp_coord cc = GP_MIN(w/6, h/6);
	gp_coord tw = co/11;

	gp_fill_rect_xywh(pix, x, y, w, h, bg_col);

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

	gp_fill_polygon(pix, 0, 0, GP_ARRAY_SIZE(poly)/2, poly, ctx->text_color);

	gp_fill_circle(pix, cx, cy, cc, ctx->fg_color);
}

static void render_stock_home(gp_pixmap *pix,
                              gp_coord x, gp_coord y,
                              gp_size w, gp_size h, gp_pixel bg_col,
                              const gp_widget_render_ctx *ctx)
{
	gp_coord cx = x + w/2;
	gp_coord cy = y + h/2 + h/2 - h/3;
	gp_coord hh = h/2;
	gp_coord hw = h/2;
	gp_coord tw = w/3;
	gp_coord th = h/3;

	gp_fill_rect_xywh(pix, x, y, w, h, bg_col);

	gp_coord poly[] = {
		tw, -th/2,
		hw, -th/2,
		0, -hh-(hh-th),
		-hw, -th/2,
		-tw, -th/2,
		-tw, th,
		-tw/3, th,
		-tw/3, th/3,
		tw/3, th/3,
		tw/3, th,
		tw, th,
	};

	gp_fill_polygon(pix, cx, cy, GP_ARRAY_SIZE(poly)/2, poly, ctx->text_color);
}

static void render_stock_save(gp_pixmap *pix,
                              gp_coord x, gp_coord y,
                              gp_size w, gp_size h, gp_pixel bg_col,
                              const gp_widget_render_ctx *ctx)
{
	gp_coord cx = x + w/2;
	gp_coord cy = y + h/2;
	gp_coord sh = GP_MIN(w/2, h/2);
	gp_size th = GP_MIN(3*w/8, 3*h/8)/8;
	gp_size cs = sh/3;
	gp_size cvr_s = (2 * sh - 2 * th) / 4;
	gp_size cvr_d = 3 * cvr_s / 2+1;

	gp_fill_rect_xywh(pix, x, y, w, h, bg_col);

	gp_coord poly[] = {
		-sh, -sh,
		-sh + th, -sh,
		/* inner polygon starts here */
		-sh + th,  sh - th,
		 sh - th,  sh - th,
		 sh - th, -sh + th + cs,
		 sh - th - cs, -sh + th,
		/* cover start */
		 cvr_s, -sh + th,
		 cvr_s, -sh + th + cvr_d,
		-cvr_s, -sh + th + cvr_d,
		-cvr_s, -sh + th,
		/* cover end */
		-sh + th, -sh + th,
		/* inner polygon ends here */
		-sh + th, -sh,
		 sh - cs - th/2, -sh,
		 sh, -sh + cs + th/2,
		 sh,  sh,
		-sh,  sh,
		-sh, -sh,
	};

	/* fills inner part of floppy */
	gp_fill_polygon(pix, cx, cy, 9, poly + 2, ctx->warn_color);

	/* label */
	gp_coord lx0 = cx - sh + th + sh/3+1;
	gp_coord ly0 = cy - sh + th + cvr_d + sh/3;
	gp_coord lx1 = cx + sh - th - sh/3-1;
	gp_coord ly1 = cy + sh - th - sh/3;

	gp_fill_rect_xyxy(pix, lx0, ly0, lx1, cy+sh, ctx->fg_color);

	render_text_lines(pix, lx0, ly0, lx1, ly1, ctx->text_color);

	/* draws border and cover */
	gp_fill_polygon(pix, cx, cy, GP_ARRAY_SIZE(poly)/2, poly, ctx->text_color);
	/* window in the cover */
	gp_fill_rect_xyxy(pix, cx+cvr_s-1-cvr_s/4, cy - sh + th+1 + cvr_d/5,
			       cx+cvr_s-1-2*(cvr_s/4), cy - sh + th+cvr_d-1 - cvr_d/5,
			       ctx->fg_color);
}

static void render_stock_file(gp_pixmap *pix,
                              gp_coord x, gp_coord y,
                              gp_size w, gp_size h, gp_pixel bg_col,
                              const gp_widget_render_ctx *ctx)
{
	gp_coord cx = x + w/2;
	gp_coord cy = y + h/2;
	gp_size sh = h/2;
	gp_size cs = sh/2;
	gp_size sw = GP_MIN(5*h/16, 3*w/4);
	gp_size th = GP_MIN(3*w/8, 3*h/8)/8;

	gp_fill_rect_xywh(pix, x, y, w, h, bg_col);

	gp_coord poly[] = {
		-sw, -sh,
		-sw + th, -sh,
		/* inner polygon starts here */
		-sw + th, sh - th,
		 sw - th, sh - th,
		/* corner start */
		 sw - th, -sh + 2*th + cs,
		 sw - cs - 2*th, -sh + 2*th + cs,
		 sw - cs - 2*th, -sh + th,
		 sw - cs - th, -sh + th,
		 sw - cs - th, -sh + th + cs,
		 sw - th, -sh + th + cs,
		/* corner end */
		 sw - th - cs, -sh + th,
		-sw + th, -sh + th,
		/* inner polygon ends here */
		-sw + th, -sh,
		 sw - cs - th/2, -sh,
		 sw, -sh + cs + th/2,
		 sw, sh,
		-sw, sh,
		-sw, -sh,
	};

	gp_coord in_poly[] = {
		-sw + th,  sh - th,
		 sw - th,  sh - th,
		 sw - th, -sh + th + cs,
		 sw - th - cs, -sh + th,
		-sw + th, -sh + th,
	};

	gp_fill_polygon(pix, cx, cy, GP_ARRAY_SIZE(in_poly)/2, in_poly, ctx->fg_color);
	gp_fill_polygon(pix, cx, cy, GP_ARRAY_SIZE(poly)/2, poly, ctx->text_color);

	/* text */
	gp_coord lx0 = cx - sw + th + sh/3;
	gp_coord ly0 = cy - sh + cs + th + sh/3+1;
	gp_coord lx1 = cx + sw - th - sh/3;
	gp_coord ly1 = cy + sh - th - sh/3-1;

	render_text_lines(pix, lx0, ly0, lx1, ly1, ctx->sel_color);
}

static void render_stock_dir(gp_pixmap *pix, int new,
                             gp_coord x, gp_coord y,
                             gp_size w, gp_size h, gp_pixel bg_col,
                             const gp_widget_render_ctx *ctx)
{
	gp_coord cx = x + w/2;
	gp_coord cy = y + h/2;
	gp_size w2 = w/2;
	gp_size h2 = h/2;
	gp_size th = GP_MIN(3*w/8, 3*h/8)/8;
	gp_size cs = GP_MAX(2u, w/8);
	gp_size cw = w2;
	/* corner "roundess" */
	gp_size r = w2/10;
	gp_size fw = th+2;

	gp_fill_rect_xywh(pix, x, y, w, h, bg_col);

	gp_coord poly[] = {
		-w2, -h2 + r,
		-w2 + th, -h2,
		-w2 + th, -h2 + th + cs,
		-w2 + th, -h2 + th + cs,

		-w2 + th, h2 - th,

		 w2 - fw - th, h2 - th,
		 w2 - th, h2 - th,

		 w2 - th, -h2 + cs + 2*th+fw,
		/* inner part starts here */
		 w2 - th, -h2 + cs + fw + th,
		 w2 - th, -h2 + cs + th,
		-w2 + th + cw, -h2 + th + cs,
		-w2 + th + cw, -h2 + th,
		-w2 + th, -h2 + th,
		/* inner part ends here */
		-w2 + th, -h2,
		-w2 + 2*th + cw-r, -h2,
		-w2 + 2*th + cw, -h2 + r,
		-w2 + 2*th + cw, -h2 + cs,

		 w2 - r, -h2 + cs,
		 w2, -h2 + cs+r,

		 w2, h2,
		-w2, h2,
		-w2, -h2 + cs,
	};

	gp_coord in_poly[] = {
		 w2 - th, -h2 + cs + fw + th,
		 w2 - th, -h2 + cs + th,
		-w2 + th + cw, -h2 + th + cs,
		-w2 + th + cw, -h2 + th,
		-w2 + th, -h2 + th,
		-w2 + th, -h2 + cs + fw + th,
	};

	/* fill the innter parts */
	gp_fill_rect_xyxy(pix, cx - w2 + th, cy - h2 + cs + th + fw,
	                       cx + w2 - th, cy + h2 - th, ctx->sel_color);
	gp_fill_polygon(pix, cx, cy, GP_ARRAY_SIZE(in_poly)/2, in_poly, ctx->fg_color);

	if (new) {
		gp_size ps = w/5;
		gp_size pt = th;//GP_MAX(1u, th);
		gp_coord px = cx;
		gp_coord py = cy + cs;

		gp_coord plus_poly[] = {
			 ps,  pt,
			 pt,  pt,
			 pt,  ps,
			-pt,  ps,
			-pt,  pt,
			-ps,  pt,
			-ps, -pt,
			-pt, -pt,
			-pt, -ps,
			 pt, -ps,
			 pt, -pt,
			 ps, -pt,
		};

		gp_fill_polygon(pix, px, py, GP_ARRAY_SIZE(plus_poly)/2, plus_poly, ctx->accept_color);
		gp_polygon(pix, px, py, GP_ARRAY_SIZE(plus_poly)/2, plus_poly, ctx->text_color);
	}

	/* draw the lines */
	gp_fill_polygon(pix, cx, cy, GP_ARRAY_SIZE(poly)/2, poly, ctx->text_color);
	gp_hline_xxy(pix, cx-w2+th+1, cx+w2-th-1, cy-h2+cs+fw+th, ctx->text_color);

	/* final touch for low res */
	gp_putpixel(pix, cx + w2, cy - h2 + cs, bg_col);
	gp_putpixel(pix, cx - w2, cy - h2, bg_col);
	gp_putpixel(pix, cx - w2 + 2*th + cw, cy - h2, bg_col);
}

static void render_stock_close(gp_pixmap *pix,
                               gp_coord x, gp_coord y,
                               gp_size w, gp_size h, gp_pixel bg_col,
                               const gp_widget_render_ctx *ctx)
{
	gp_size a = GP_MAX((gp_size)1, GP_MIN(w/6, h/6));

	gp_fill_rect_xywh(pix, x, y, w, h, bg_col);

	w--; h--;

	gp_size bw = (w-2*a)/2;
	gp_size bh = (h-2*a)/2;

	gp_coord poly[] = {
		0, 0,
		a, 0,
		w/2, bh,
		w/2 + w%2, bh,
		w - a, 0,
		w, 0,
		w, a,
		w - bw, h/2,
		w - bw, h/2 + h%2,
		w, h - a,
		w, h,
		w - a, h,
		w/2, h - bh,
		w/2 + w%2, h - bh,
		a, h,
		0, h,
		0, h - a,
		0 + bw, h/2 + h%2,
		0 + bw, h/2,
		0, a,
	};

	gp_fill_polygon(pix, x, y, GP_ARRAY_SIZE(poly)/2, poly, ctx->alert_color);
}

static void render_stock_arrow(gp_pixmap *pix,
                               int type,
                               gp_coord x, gp_coord y,
                               gp_size w, gp_size h, gp_pixel bg_col,
                               const gp_widget_render_ctx *ctx)
{
	gp_fill_rect_xywh(pix, x, y, w, h, bg_col);

	h--;
	w--;

	gp_coord up_poly[] = {
		w/2, 0,
		(w+1)/2, 0,
		w, h/2,
		w, (h+1)/2,
		w-3*w/8, (h+1)/2,
		w-3*w/8, h,
		3*w/8, h,
		3*w/8, (h+1)/2,
		0, (h+1)/2,
		0, h/2,
	};

	gp_coord down_poly[] = {
		w/2, h,
		(w+1)/2, h,
		w, (h+1)/2,
		w, h/2,
		w-3*w/8, h/2,
		w-3*w/8, 0,
		3*w/8, 0,
		3*w/8, h/2,
		0, h/2,
		0, (h+1)/2,
	};

	gp_coord left_poly[] = {
		0, h/2,
		0, (h+1)/2,
		w/2, h,
		(w+1)/2, h,
		(w+1)/2, h - 3*h/8,
		w, h - 3*h/8,
		w, 3*h/8,
		(w+1)/2, 3*h/8,
		(w+1)/2, 0,
		w/2, 0,
	};

	gp_coord right_poly[] = {
		w, h/2,
		w, (h+1)/2,
		(w+1)/2, h,
		w/2, h,
		w/2, h - 3*h/8,
		0, h - 3*h/8,
		0, 3*h/8,
		w/2, 3*h/8,
		w/2, 0,
		(w+1)/2, 0,
	};

	switch (GP_WIDGET_STOCK_TYPE(type)) {
	case GP_WIDGET_STOCK_ARROW_UP:
		gp_fill_polygon(pix, x, y, GP_ARRAY_SIZE(up_poly)/2, up_poly, ctx->text_color);
	break;
	case GP_WIDGET_STOCK_ARROW_DOWN:
		gp_fill_polygon(pix, x, y, GP_ARRAY_SIZE(down_poly)/2, down_poly, ctx->text_color);
	break;
	case GP_WIDGET_STOCK_ARROW_LEFT:
		gp_fill_polygon(pix, x, y, GP_ARRAY_SIZE(left_poly)/2, left_poly, ctx->text_color);
	break;
	case GP_WIDGET_STOCK_ARROW_RIGHT:
		gp_fill_polygon(pix, x, y, GP_ARRAY_SIZE(right_poly)/2, right_poly, ctx->text_color);
	break;
	}
}

static void render_stock_rotate_cw(gp_pixmap *pix,
                                   gp_coord x, gp_coord y,
                                   gp_size w, gp_size h, gp_pixel bg_col,
                                   const gp_widget_render_ctx *ctx)
{
	gp_coord cx = x + w/2;
	gp_coord cy = y + h/2;

	gp_size th = GP_MIN(w, h)/4;
	gp_size s = GP_MAX((GP_MIN(w, h)+1)/4, 1u);
	gp_size r = GP_MIN(w, h)/2-s;

	gp_fill_rect_xywh(pix, x, y, w, h, bg_col);

	gp_size sr = GP_MAX((s+1)/3, 1u);
	gp_size lr = th/3;

	gp_coord poly_1[] = {
		 th-sr, -r-s,
		 th, -r-s,
		 th+s, -r,
		 th, -r+s,
		 th-sr, -r+s,

		 th-sr, -r+lr,
		-th, -r+lr,

		-th, r+s,

		-th-2*lr, r+s,
		-th-2*lr, -r+lr,

		-th, -r-lr,
		 th-sr, -r-lr,
	};

	gp_fill_polygon(pix, cx, cy, GP_ARRAY_SIZE(poly_1)/2, poly_1, ctx->text_color);
}

static void render_stock_rotate_ccw(gp_pixmap *pix,
                                    gp_coord x, gp_coord y,
                                    gp_size w, gp_size h, gp_pixel bg_col,
                                    const gp_widget_render_ctx *ctx)
{
	gp_coord cx = x + w/2;
	gp_coord cy = y + h/2;

	gp_size th = GP_MIN(w, h)/4;
	gp_size s = GP_MAX((GP_MIN(w, h)+1)/4, 1u);
	gp_size r = GP_MIN(w, h)/2-s;

	gp_fill_rect_xywh(pix, x, y, w, h, bg_col);

	gp_size sr = GP_MAX((s+1)/3, 1u);
	gp_size lr = th/3;

	gp_coord poly_1[] = {
		-th+sr, -r-s,
		-th, -r-s,
		-th-s, -r,
		-th, -r+s,
		-th+sr, -r+s,

		-th+sr, -r+lr,
		th, -r+lr,

		th, r+s,

		th+2*lr, r+s,
		th+2*lr, -r+lr,

		th, -r-lr,
		-th+sr, -r-lr,
	};

	gp_fill_polygon(pix, cx, cy, GP_ARRAY_SIZE(poly_1)/2, poly_1, ctx->text_color);
}


static void render_stock_refresh(gp_pixmap *pix,
                                 gp_coord x, gp_coord y,
                                 gp_size w, gp_size h, gp_pixel bg_col,
                                 const gp_widget_render_ctx *ctx)
{
	gp_coord cx = x + w/2;
	gp_coord cy = y + h/2;

	gp_size th = GP_MIN(w, h)/16;
	gp_size s = GP_MAX(GP_MIN(w, h)/5, 1u);
	gp_size r = GP_MIN(w, h)/2-s;

	gp_fill_rect_xywh(pix, x, y, w, h, bg_col);

	gp_fill_ring_seg(pix, cx, cy, r - th, r + th,
	                 GP_CIRCLE_SEG2|GP_CIRCLE_SEG3|GP_CIRCLE_SEG4,
	                 ctx->text_color);

	gp_size sr = GP_MAX(s/3, 1u);

	gp_coord poly_1[] = {
		th-sr, -r-s,
		th, -r-s,
		th+s, -r,
		th, -r+s,
		th-sr, -r+s,
	};

	gp_fill_polygon(pix, cx, cy, GP_ARRAY_SIZE(poly_1)/2, poly_1, ctx->text_color);
}

static void render_stock_shuffle_off(gp_pixmap *pix,
                                     gp_coord x, gp_coord y,
                                     gp_size w, gp_size h, gp_pixel bg_col,
                                     const gp_widget_render_ctx *ctx)
{
	gp_coord cx = x + w/2;
	gp_coord cy = y + h/2;

	gp_size wh = w/3;

	gp_size th = GP_MIN(w, h)/16;

	gp_fill_rect_xywh(pix, x, y, w, h, bg_col);

	gp_size yd = h/4;

	gp_coord poly_1[] = {
		-wh-3*th, yd+th,
		-wh-3*th, yd-th,
		 wh-th, yd-th,
		 wh-th, yd-3*th,
		 wh, yd-3*th,
		 wh+3*th, yd,
		 wh, yd+3*th,
		 wh-th, yd+3*th,
		 wh-th, yd+th,
	};

	gp_fill_polygon(pix, cx, cy, GP_ARRAY_SIZE(poly_1)/2, poly_1, ctx->text_color);

	gp_coord poly_2[] = {
		-wh-3*th, -yd+th,
		-wh-3*th, -yd-th,
		 wh-th, -yd-th,
		 wh-th, -yd-3*th,
		 wh, -yd-3*th,
		 wh+3*th, -yd,
		 wh, -yd+3*th,
		 wh-th, -yd+3*th,
		 wh-th, -yd+th,
	};

	gp_fill_polygon(pix, cx, cy, GP_ARRAY_SIZE(poly_2)/2, poly_2, ctx->text_color);
}

static void render_stock_shuffle_on(gp_pixmap *pix,
                                    gp_coord x, gp_coord y,
                                    gp_size w, gp_size h, gp_pixel bg_col,
                                    const gp_widget_render_ctx *ctx)
{
	gp_coord cx = x + w/2;
	gp_coord cy = y + h/2;

	gp_size wh = w/3;

	gp_size th = GP_MIN(w, h)/16;

	gp_fill_rect_xywh(pix, x, y, w, h, bg_col);

	gp_size yd = h/4;

	cx -= 2*th;
	gp_size xs = yd/2;

	gp_coord poly_1[] = {
		-wh-th, -yd-th,
		-wh-th, -yd+th,
		-wh+xs, -yd+th,
		 wh-xs,  yd+th,
		 wh+th,  yd+th,

		 wh+th,   yd+3*th,
		 wh+2*th, yd+3*th,
		 wh+5*th, yd,
		 wh+2*th, yd-3*th,
		 wh+th,   yd-3*th,

		 wh+th,    yd-th,
		 wh-xs+th, yd-th,
		-wh+xs+th, -yd-th,
	};

	gp_fill_polygon(pix, cx, cy, GP_ARRAY_SIZE(poly_1)/2, poly_1, ctx->text_color);

	gp_coord poly_2[] = {
		-wh-th,  yd+th,
		-wh-th,  yd-th,
		-wh+xs,  yd-th,
		 wh-xs, -yd-th,
		 wh+th, -yd-th,

		 wh+th,   -yd-3*th,
		 wh+2*th, -yd-3*th,
		 wh+5*th, -yd,
		 wh+2*th, -yd+3*th,
		 wh+th,   -yd+3*th,

		 wh+th,    -yd+th,
		 wh-xs+th, -yd+th,
		-wh+xs+th,  yd+th,
	};

	gp_fill_polygon(pix, cx, cy, GP_ARRAY_SIZE(poly_2)/2, poly_2, ctx->text_color);
}

static void render_stock_repeat(gp_pixmap *pix,
                                gp_coord x, gp_coord y,
                                gp_size w, gp_size h, gp_pixel bg_col,
                                const gp_widget_render_ctx *ctx, int on)
{
	gp_coord cx = x + w/2;
	gp_coord cy = y + h/2;

	gp_size wh = w/3;

	gp_size th = GP_MIN(w, h)/16;

	gp_fill_rect_xywh(pix, x, y, w, h, bg_col);

	gp_size yd = h/4;

	gp_coord poly_1[] = {
		-wh-3*th, yd+th,
		-wh-3*th, yd-th -4*th,
		-wh-3*th+2*th, yd-th -4*th,
		-wh-3*th+2*th, yd-th,

		 wh-th, yd-th,
		 wh-th, yd-3*th,
		 wh, yd-3*th,
		 wh+3*th, yd,
		 wh, yd+3*th,
		 wh-th, yd+3*th,
		 wh-th, yd+th,
	};

	gp_fill_polygon(pix, cx, cy, GP_ARRAY_SIZE(poly_1)/2, poly_1, ctx->text_color);

	gp_coord poly_2[] = {
		 wh+3*th-2*th, -yd+th,
		 wh+3*th-2*th, -yd+th + 4*th,
		 wh+3*th, -yd+th + 4*th,
		 wh+3*th, -yd-th,

		 -wh+th, -yd-th,
		 -wh+th, -yd-3*th,
		 -wh, -yd-3*th,
		 -wh-3*th, -yd,
		 -wh, -yd+3*th,
		 -wh+th, -yd+3*th,
		 -wh+th, -yd+th,
	};

	gp_fill_polygon(pix, cx, cy, GP_ARRAY_SIZE(poly_2)/2, poly_2, ctx->text_color);

	if (on)
		return;

	th=(th+1)/2;
	gp_size cross_r = GP_MIN(w/2, h/2) - 2*th;

	gp_line_th(pix, cx-cross_r, cy+cross_r, cx+cross_r, cy-cross_r, th, ctx->alert_color);
}

static void render_stock_filter(gp_pixmap *pix,
                                gp_coord x, gp_coord y,
                                gp_size w, gp_size h, gp_pixel bg_col,
                                const gp_widget_render_ctx *ctx)
{
	gp_coord cx = x + w/2;
	gp_size pp = GP_MIN(w, h)/8;

	gp_fill_rect_xywh(pix, x, y, w, h, bg_col);

	gp_coord poly[] = {
		x, y,
		x + w-1, y,
		cx + pp, y + h/2,
		cx + pp, y+h-1,
		cx - pp, y+h-1 - pp,
		cx - pp, y + h/2,
	};

	gp_fill_polygon(pix, 0, 0, GP_ARRAY_SIZE(poly)/2, poly, ctx->text_color);
}

static void render_stock_day(gp_pixmap *pix,
                             gp_coord x, gp_coord y,
                             gp_size w, gp_size h, gp_pixel bg_col,
                             enum gp_widget_stock_type type,
                             const gp_widget_render_ctx *ctx)
{
	gp_pixel col = type & GP_WIDGET_STOCK_FOCUSED ? ctx->sel_color : ctx->text_color;
	gp_coord cx = x + w/2;
	gp_coord cy = y + h/2;
	gp_size sp = GP_MAX(4u, GP_MIN(w, h)/8);

	gp_size r = (GP_MIN(w, h)+2)/4;
	gp_size r2 = (GP_MIN(w, h) - 2*r - 2*sp + 3)/4;
	gp_size l = r + sp + (r2+1)/2;

	if (r2 <= 1)
		l--;

	gp_fill_rect_xywh(pix, x, y, w, h, bg_col);

	gp_fill_circle(pix, cx, cy, r-1, col);

	gp_fill_circle(pix, cx+l, cy, r2, col);
	gp_fill_circle(pix, cx+l-1, cy, r2, col);

	gp_fill_circle(pix, cx-l, cy, r2, col);
	gp_fill_circle(pix, cx-l+1, cy, r2, col);

	gp_fill_circle(pix, cx, cy+l, r2, col);
	gp_fill_circle(pix, cx, cy+l-1, r2, col);

	gp_fill_circle(pix, cx, cy-l, r2, col);
	gp_fill_circle(pix, cx, cy-l+1, r2, col);

	gp_size lxy = l/1.41 + 0.5;

	gp_fill_circle(pix, cx+lxy-1, cy+lxy-1, r2, col);
	gp_fill_circle(pix, cx+lxy, cy+lxy, r2, col);

	gp_fill_circle(pix, cx-lxy+1, cy+lxy-1, r2, col);
	gp_fill_circle(pix, cx-lxy, cy+lxy, r2, col);

	gp_fill_circle(pix, cx+lxy-1, cy-lxy+1, r2, col);
	gp_fill_circle(pix, cx+lxy, cy-lxy, r2, col);

	gp_fill_circle(pix, cx-lxy+1, cy-lxy+1, r2, col);
	gp_fill_circle(pix, cx-lxy, cy-lxy, r2, col);
}

static void render_stock_night(gp_pixmap *pix,
                               gp_coord x, gp_coord y,
                               gp_size w, gp_size h, gp_pixel bg_col,
                               enum gp_widget_stock_type type,
                               const gp_widget_render_ctx *ctx)
{
	gp_pixel col = type & GP_WIDGET_STOCK_FOCUSED ? ctx->sel_color : ctx->text_color;

	gp_coord cx = x + w/2;
	gp_coord cy = y + h/2;

	gp_size r = 2*GP_MIN(w, h)/5;

	gp_fill_rect_xywh(pix, x, y, w, h, bg_col);

	gp_fill_circle(pix, cx, cy, r, col);
	gp_fill_circle_seg(pix, cx+r, cy, r, GP_CIRCLE_SEG2 | GP_CIRCLE_SEG3, bg_col);
}

static void render_stock_star(gp_pixmap *pix,
                              gp_coord x, gp_coord y,
                              gp_size w, gp_size h, gp_pixel bg_col,
                              const gp_widget_render_ctx *ctx)
{
	gp_coord cx = x + w/2;
	gp_coord cy = y + h/2;

	gp_coord co = GP_MIN(w/2, h/2);
	gp_coord ci = co/2;

	gp_fill_rect_xywh(pix, x, y, w, h, bg_col);

	gp_coord poly[] = {
		cx, cy - co,
		ROTATE(0.588, 0.809, cx, 0, cy, -ci),
		ROTATE(0.951, 0.309, cx, 0, cy, -co),
		ROTATE(0.951, -0.309, cx, 0, cy, -ci),
		ROTATE(0.588, -0.809, cx, 0, cy, -co),
		cx, cy + ci,
		ROTATE(-0.588, -0.809, cx, 0, cy, -co),
		ROTATE(-0.951, -0.309, cx, 0, cy, -ci),
		ROTATE(-0.951, 0.309, cx, 0, cy, -co),
		ROTATE(-0.588, 0.809, cx, 0, cy, -ci),
	};

	gp_fill_polygon(pix, 0, 0, GP_ARRAY_SIZE(poly)/2, poly, ctx->warn_color);
	gp_polygon_th(pix, 0, 0, GP_ARRAY_SIZE(poly)/2, poly, 1, gp_pixel_chans_add(ctx->pixel_type, ctx->warn_color, -5));
}

static void render_stock_zoom(gp_pixmap *pix, enum gp_widget_stock_type type,
                              gp_coord x, gp_coord y,
                              gp_size w, gp_size h, gp_pixel bg_col,
                              const gp_widget_render_ctx *ctx)
{
	gp_size th = 2*(GP_MIN(3*w/8, 3*h/8)+1)/8;
	gp_size co = GP_MIN(w/2, h/2) - th;
	gp_size ch = GP_MIN(w/2, h/2);

	gp_size sym_line_w = (th+1)/2;
	gp_size sym_len = co-2*th;

	gp_coord cx = x + w/2 - th;
	gp_coord cy = y + h/2 - th;

	switch (GP_WIDGET_STOCK_TYPE(type)) {
	case GP_WIDGET_STOCK_ZOOM:
	break;
	case GP_WIDGET_STOCK_ZOOM_IN:
		gp_fill_rect_xyxy(pix, cx-sym_line_w, cy-sym_len, cx+sym_line_w, cy+sym_len, ctx->text_color);
	/* fallthrough */
	case GP_WIDGET_STOCK_ZOOM_OUT:
		gp_fill_rect_xyxy(pix, cx-sym_len, cy-sym_line_w, cx+sym_len, cy+sym_line_w, ctx->text_color);
	break;
	case GP_WIDGET_STOCK_ZOOM_FIT:
		gp_fill_rect_xyxy(pix, cx-sym_len+1, cy-sym_len+1, cx+sym_len-1, cy+sym_len-1, ctx->text_color);
		gp_fill_rect_xyxy(pix, cx-sym_len+2*sym_line_w+1, cy-sym_len+2*sym_line_w+1,
		                       cx+sym_len-2*sym_line_w-1, cy+sym_len-2*sym_line_w-1, bg_col);
	break;
	}

	gp_fill_ring(pix, cx, cy, co, co - th, ctx->text_color);

	gp_line_th(pix, cx+co/1.44, cy+co/1.44, cx+ch, cy+ch, th, ctx->text_color);
	gp_fill_circle(pix, cx+ch, cy+ch, th, ctx->text_color);
}

static void render_stock_on(gp_pixmap *pix,
                            gp_coord x, gp_coord y,
                            gp_size w, gp_size h, gp_pixel bg_col,
                            const gp_widget_render_ctx *ctx)
{
	gp_size th = GP_MIN(w, h)/12;
	gp_size sh = (h-2)/2;

	gp_fill_rect_xywh(pix, x, y, w, h, bg_col);

	gp_fill_rect_xyxy(pix, x + w/2 + (th-th/2), y + h/2-sh, x + w/2 - th/2, y + h/2+sh, ctx->text_color);
}

static void render_stock_off(gp_pixmap *pix,
                             gp_coord x, gp_coord y,
                             gp_size w, gp_size h, gp_pixel bg_col,
                             const gp_widget_render_ctx *ctx)
{
	gp_size th = GP_MIN(w, h)/12;
	gp_size r = w/3;

	gp_fill_rect_xywh(pix, x, y, w, h, bg_col);

	gp_fill_ring_seg(pix, x + w/2, y + r, r, r-th, GP_CIRCLE_SEG1 | GP_CIRCLE_SEG2,  ctx->text_color);
	gp_fill_rect_xyxy(pix, x + w/2+r-th, y + r, x+w/2+r, y + h - r - 1, ctx->text_color);
	gp_fill_rect_xyxy(pix, x + w/2-r+th, y + r, x+w/2-r, y + h - r - 1, ctx->text_color);
	gp_fill_ring_seg(pix, x + w/2, y + h - r - 1, r, r-th, GP_CIRCLE_SEG3 | GP_CIRCLE_SEG4,  ctx->text_color);
}

static void widget_stock_render(gp_pixmap *pix, enum gp_widget_stock_type type,
                                gp_coord x, gp_coord y, gp_size w, gp_size h,
                                gp_pixel bg_col, const gp_widget_render_ctx *ctx)
{
	switch (GP_WIDGET_STOCK_TYPE(type)) {
	case GP_WIDGET_STOCK_SPEAKER_MIN:
	case GP_WIDGET_STOCK_SPEAKER_MAX:
	case GP_WIDGET_STOCK_SPEAKER_MID:
	case GP_WIDGET_STOCK_SPEAKER_MUTE:
	case GP_WIDGET_STOCK_SPEAKER_INC:
	case GP_WIDGET_STOCK_SPEAKER_DEC:
		render_stock_speaker(pix, x, y, w, h, type, ctx);
	break;
	case GP_WIDGET_STOCK_INFO:
		render_stock_info(pix, x, y, w, h, bg_col, ctx);
	break;
	case GP_WIDGET_STOCK_WARN:
		render_stock_err_warn(pix, x, y, w, h, ctx->warn_color, bg_col, ctx);
	break;
	case GP_WIDGET_STOCK_ERR:
		render_stock_err_warn(pix, x, y, w, h, ctx->alert_color, bg_col, ctx);
	break;
	case GP_WIDGET_STOCK_QUESTION:
		render_stock_question(pix, x, y, w, h, bg_col, ctx);
	break;
	case GP_WIDGET_STOCK_HARDWARE:
		render_stock_hardware(pix, x, y, w, h, bg_col, ctx);
	break;
	case GP_WIDGET_STOCK_SOFTWARE:
		render_stock_software(pix, x, y, w, h, bg_col, ctx);
	break;
	case GP_WIDGET_STOCK_SETTINGS:
		render_stock_settings(pix, x, y, w, h, bg_col, ctx);
	break;
	case GP_WIDGET_STOCK_HOME:
		render_stock_home(pix, x, y, w, h, bg_col, ctx);
	break;
	case GP_WIDGET_STOCK_SAVE:
		render_stock_save(pix, x, y, w, h, bg_col, ctx);
	break;
	case GP_WIDGET_STOCK_FILE:
		render_stock_file(pix, x, y, w, h, bg_col, ctx);
	break;
	case GP_WIDGET_STOCK_DIR:
		render_stock_dir(pix, 0, x, y, w, h, bg_col, ctx);
	break;
	case GP_WIDGET_STOCK_NEW_DIR:
		render_stock_dir(pix, 1, x, y, w, h, bg_col, ctx);
	break;
	case GP_WIDGET_STOCK_CLOSE:
		render_stock_close(pix, x, y, w, h, bg_col, ctx);
	break;
	case GP_WIDGET_STOCK_REFRESH:
		render_stock_refresh(pix, x, y, w, h, bg_col, ctx);
	break;
	case GP_WIDGET_STOCK_ARROW_UP:
	case GP_WIDGET_STOCK_ARROW_DOWN:
	case GP_WIDGET_STOCK_ARROW_LEFT:
	case GP_WIDGET_STOCK_ARROW_RIGHT:
		render_stock_arrow(pix, type, x, y, w, h, bg_col, ctx);
	break;
	case GP_WIDGET_STOCK_ROTATE_CW:
		render_stock_rotate_cw(pix, x, y, w, h, bg_col, ctx);
	break;
	case GP_WIDGET_STOCK_ROTATE_CCW:
		render_stock_rotate_ccw(pix, x, y, w, h, bg_col, ctx);
	break;
	case GP_WIDGET_STOCK_SHUFFLE_ON:
		render_stock_shuffle_on(pix, x, y, w, h, bg_col, ctx);
	break;
	case GP_WIDGET_STOCK_SHUFFLE_OFF:
		render_stock_shuffle_off(pix, x, y, w, h, bg_col, ctx);
	break;
	case GP_WIDGET_STOCK_REPEAT_ON:
		render_stock_repeat(pix, x, y, w, h, bg_col, ctx, 1);
	break;
	case GP_WIDGET_STOCK_REPEAT_OFF:
		render_stock_repeat(pix, x, y, w, h, bg_col, ctx, 0);
	break;
	case GP_WIDGET_STOCK_FILTER:
		render_stock_filter(pix, x, y, w, h, bg_col, ctx);
	break;
	case GP_WIDGET_STOCK_DAY:
		render_stock_day(pix, x, y, w, h, bg_col, type, ctx);
	break;
	case GP_WIDGET_STOCK_NIGHT:
		render_stock_night(pix, x, y, w, h, bg_col, type, ctx);
	break;
	case GP_WIDGET_STOCK_STAR:
		render_stock_star(pix, x, y, w, h, bg_col, ctx);
	break;
	case GP_WIDGET_STOCK_ZOOM:
	case GP_WIDGET_STOCK_ZOOM_IN:
	case GP_WIDGET_STOCK_ZOOM_OUT:
	case GP_WIDGET_STOCK_ZOOM_FIT:
		render_stock_zoom(pix, type, x, y, w, h, bg_col, ctx);
	break;
	case GP_WIDGET_STOCK_ON:
		render_stock_on(pix, x, y, w, h, bg_col, ctx);
	break;
	case GP_WIDGET_STOCK_OFF:
		render_stock_off(pix, x, y, w, h, bg_col, ctx);
	break;
	}

}

void gp_widget_stock_render(gp_pixmap *pix, enum gp_widget_stock_type type,
                            gp_coord x, gp_coord y, gp_size w, gp_size h,
			    gp_pixel bg_col, const gp_widget_render_ctx *ctx)
{
	gp_fill_rect_xywh(pix, x, y, w, h, bg_col);

	widget_stock_render(pix, type, x, y, w, h, bg_col, ctx);
}

static void stock_render(gp_widget *self, const gp_offset *offset,
                         const gp_widget_render_ctx *ctx, int flags)
{
	struct gp_widget_stock *stock = GP_WIDGET_PAYLOAD(self);

	gp_coord x = self->x + offset->x;
	gp_coord y = self->y + offset->y;
	gp_coord w = self->w;
	gp_coord h = self->h;

	gp_size off;

	(void) flags;

	switch (stock->type) {
	case GP_WIDGET_STOCK_ARROW_UP:
	case GP_WIDGET_STOCK_ARROW_DOWN:
	case GP_WIDGET_STOCK_ARROW_LEFT:
	case GP_WIDGET_STOCK_ARROW_RIGHT:
		off = GP_MAX(1u, (gp_size)GP_MIN(w/6, h/6));
	break;
	default:
		off = 0;
	break;
	}

	enum gp_widget_stock_type type = stock->type;

	if (self->focused)
		type |= GP_WIDGET_STOCK_FOCUSED;

	gp_fill_rect_xywh(ctx->buf, x, y, w, h, ctx->bg_color);

	gp_widget_stock_render(ctx->buf, type,
	                       x+off, y+off, w-2*off, h-2*off,
	                       ctx->bg_color, ctx);

	gp_widget_ops_blit(ctx, x, y, w, h);
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
	{"home", GP_WIDGET_STOCK_HOME},
	{"save", GP_WIDGET_STOCK_SAVE},
	{"file", GP_WIDGET_STOCK_FILE},
	{"dir", GP_WIDGET_STOCK_DIR},
	{"new_dir", GP_WIDGET_STOCK_NEW_DIR},
	{"close", GP_WIDGET_STOCK_CLOSE},

	{"refresh", GP_WIDGET_STOCK_REFRESH},

	{"shuffle_on", GP_WIDGET_STOCK_SHUFFLE_ON},
	{"shuffle_off", GP_WIDGET_STOCK_SHUFFLE_OFF},
	{"repeat_on", GP_WIDGET_STOCK_REPEAT_ON},
	{"repeat_off", GP_WIDGET_STOCK_REPEAT_OFF},

	{"filter", GP_WIDGET_STOCK_FILTER},

	{"arrow_up", GP_WIDGET_STOCK_ARROW_UP},
	{"arrow_down", GP_WIDGET_STOCK_ARROW_DOWN},
	{"arrow_left", GP_WIDGET_STOCK_ARROW_LEFT},
	{"arrow_right", GP_WIDGET_STOCK_ARROW_RIGHT},

	{"rotate_cw", GP_WIDGET_STOCK_ROTATE_CW},
	{"rotate_ccw", GP_WIDGET_STOCK_ROTATE_CCW},

	{"day", GP_WIDGET_STOCK_DAY},
	{"night", GP_WIDGET_STOCK_NIGHT},

	{"star", GP_WIDGET_STOCK_STAR},

	{"zoom", GP_WIDGET_STOCK_ZOOM},
	{"zoom_in", GP_WIDGET_STOCK_ZOOM_IN},
	{"zoom_out", GP_WIDGET_STOCK_ZOOM_OUT},
	{"zoom_fit", GP_WIDGET_STOCK_ZOOM_FIT},

	{"on", GP_WIDGET_STOCK_ON},
	{"off", GP_WIDGET_STOCK_OFF},
};

gp_widget_stock_type gp_widget_stock_type_by_name(const char *name)
{
	unsigned int i;

	for (i = 0; i < GP_ARRAY_SIZE(stock_types); i++) {
		if (!strcmp(name, stock_types[i].str_type))
			return stock_types[i].type;
	}

	return GP_WIDGET_STOCK_TYPE_INVALID;
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

enum keys {
	MIN_SIZE,
	STOCK,
};

static const gp_json_obj_attr attrs[] = {
	GP_JSON_OBJ_ATTR("min_size", GP_JSON_STR),
	GP_JSON_OBJ_ATTR("stock", GP_JSON_STR),
};

static const gp_json_obj obj_filter = {
	.attrs = attrs,
	.attr_cnt = GP_ARRAY_SIZE(attrs),
};

static gp_widget *json_to_stock(gp_json_reader *json, gp_json_val *val, gp_widget_json_ctx *ctx)
{
	gp_widget_size min_size = GP_WIDGET_SIZE_DEFAULT;
	int type = -1;

	(void)ctx;

	GP_JSON_OBJ_FOREACH_FILTER(json, val, &obj_filter, gp_widget_json_attrs) {
		switch (val->idx) {
		case MIN_SIZE:
			if (gp_widget_size_units_parse(val->val_str, &min_size))
				gp_json_warn(json, "Invalid size string!");
		break;
		case STOCK:
			type = gp_widget_stock_type_by_name(val->val_str);
			if (type == GP_WIDGET_STOCK_TYPE_INVALID)
				gp_json_warn(json, "Unknown stock type!");
		break;
		}
	}

	if (type < 0) {
		gp_json_warn(json, "Missing stock type!");
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

	if (gp_widget_key_mod_pressed(ev))
		return 0;

	switch (ev->val) {
	case GP_BTN_LEFT:
	case GP_BTN_TOUCH:
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
	.from_json = json_to_stock,
	.id = "stock",
};

bool gp_widget_stock_type_valid(gp_widget_stock_type type)
{
	return type < GP_WIDGET_STOCK_TYPE_MAX;
}

gp_widget *gp_widget_stock_new(enum gp_widget_stock_type type, gp_widget_size min_size)
{
	gp_widget *ret;

	if (!gp_widget_stock_type_valid(type)) {
		GP_WARN("Invalid stock type %u", type);
		return NULL;
	}

	ret = gp_widget_new(GP_WIDGET_STOCK, GP_WIDGET_CLASS_NONE, sizeof(struct gp_widget_stock));
	if (!ret)
		return NULL;

	struct gp_widget_stock *stock = GP_WIDGET_PAYLOAD(ret);

	if (GP_WIDGET_SIZE_EQ(min_size, GP_WIDGET_SIZE_DEFAULT))
		stock->min_size = GP_WIDGET_SIZE(0, 2, 1);
	else
		stock->min_size = min_size;

	stock->type = type;
	ret->no_events = 1;

	return ret;
}

void gp_widget_stock_type_set(gp_widget *self, enum gp_widget_stock_type type)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_STOCK, );
	struct gp_widget_stock *stock = GP_WIDGET_PAYLOAD(self);

	if (stock->type == type)
		return;

	if (!gp_widget_stock_type_name(type)) {
		GP_WARN("Invalid stock type %u", type);
		return;
	}

	stock->type = type;
	gp_widget_redraw(self);
}
