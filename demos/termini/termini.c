/*****************************************************************************
 * This file is part of gfxprim library.                                     *
 *                                                                           *
 * Gfxprim is free software; you can redistribute it and/or                  *
 * modify it under the terms of the GNU Lesser General Public                *
 * License as published by the Free Software Foundation; either              *
 * version 2.1 of the License, or (at your option) any later version.        *
 *                                                                           *
 * Gfxprim is distributed in the hope that it will be useful,                *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Lesser General Public License for more details.                           *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public          *
 * License along with gfxprim; if not, write to the Free Software            *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,                        *
 * Boston, MA  02110-1301  USA                                               *
 *                                                                           *
 * Copyright (C) 2017 Cyril Hrubis <metan@ucw.cz>                            *
 *                                                                           *
 *****************************************************************************/

 /*

   TERMINI -- minimal terminal emulator.

  */

#include <stdio.h>
#include <fcntl.h>
#include <poll.h>
#include <pty.h>
#include <vterm.h>
#include <GP.h>

static GP_Backend *backend;

static VTerm *vt;
static VTermScreen *vts;

static unsigned int cols;
static unsigned int rows;
static unsigned int char_width;
static unsigned int char_height;
static GP_TextStyle *text_style;
static GP_TextStyle *text_style_bold;

static GP_Pixel colors[16];

/* delay before we repaint merged damage */
static int repaint_sleep_ms = -1;

/* HACK to draw frames */
static void draw_utf8_frames(int x, int y, uint32_t val, GP_Pixel fg)
{
	switch (val) {
	case 0x2500: /* Horizontal line */
		GP_HLineXYW(backend->pixmap, x, y + char_height/2, char_width, fg);
	break;
	case 0x2502: /* Vertical line */
		GP_VLineXYH(backend->pixmap, x + char_width/2, y, char_height, fg);
	break;
	case 0x250c: /* Upper left corner */
		GP_HLineXYW(backend->pixmap, x + char_width/2, y + char_height/2, char_width/2, fg);
		GP_VLineXYH(backend->pixmap, x + char_width/2, y + char_height/2, char_height/2+1, fg);
	break;
	case 0x2510: /* Upper right corner */
		GP_HLineXYW(backend->pixmap, x, y + char_height/2, char_width/2, fg);
		GP_VLineXYH(backend->pixmap, x + char_width/2, y + char_height/2, char_height/2+1, fg);
	break;
	case 0x2514: /* Bottom left corner */
		GP_HLineXYW(backend->pixmap, x + char_width/2, y + char_height/2, char_width/2, fg);
		GP_VLineXYH(backend->pixmap, x + char_width/2, y, char_height/2, fg);
	break;
	case 0x2518: /* Bottom right corner */
		GP_HLineXYW(backend->pixmap, x, y + char_height/2, char_width/2, fg);
		GP_VLineXYH(backend->pixmap, x + char_width/2, y, char_height/2+1, fg);
	break;
	case 0x251c: /* Left vertical tee */
		GP_HLineXYW(backend->pixmap, x + char_width/2, y + char_height/2, char_width/2, fg);
		GP_VLineXYH(backend->pixmap, x + char_width/2, y, char_height, fg);
	break;
	case 0x2524: /* Right vertical tee */
		GP_HLineXYW(backend->pixmap, x, y + char_height/2, char_width/2, fg);
		GP_VLineXYH(backend->pixmap, x + char_width/2, y, char_height, fg);
	break;
	default:
		fprintf(stderr, "WARN: unhandled utf8 char %x\n", val);
	}
}

static void draw_cell(VTermPos pos)
{
	VTermScreenCell c;

	vterm_screen_get_cell(vts, pos, &c);

	GP_Pixel bg = colors[c.bg.red];
	GP_Pixel fg = colors[c.fg.red];

	if (c.attrs.reverse)
		GP_SWAP(bg, fg);

	char buf[2] = {c.chars[0], 0};

	int x = pos.col * char_width;
	int y = pos.row * char_height;

	GP_FillRectXYWH(backend->pixmap, x, y, char_width, char_height, bg);

	//fprintf(stderr, "Drawing %x %c %02i %02i\n", buf[0], buf[0], pos.row, pos.col);

	if (c.width > 1)
		fprintf(stderr, "%i\n", c.width);

	if (c.chars[0] > 0x7f) {
		draw_utf8_frames(x, y, c.chars[0], fg);
		return;
	}

	GP_TextStyle *style = c.attrs.bold ? text_style_bold : text_style;

	GP_Text(backend->pixmap, style, x, y, GP_ALIGN_RIGHT | GP_VALIGN_BELOW, fg, bg, buf);
}

static void update_rect(VTermRect rect)
{
	int x = rect.start_col * char_width;
	int y = rect.start_row * char_height;
	int w = rect.end_col * char_width;
	int h = rect.end_row * char_height;

	GP_BackendUpdateRectXYXY(backend, x, y, w, h);
}

static VTermRect damaged;
static int damage_repainted = 1;

static void merge_damage(VTermRect rect)
{
	if (damage_repainted) {
		damaged = rect;
		damage_repainted = 0;
		return;
	}

	damaged.start_col = GP_MIN(damaged.start_col, rect.start_col);
	damaged.end_col = GP_MAX(damaged.end_col, rect.end_col);

	damaged.start_row = GP_MIN(damaged.start_row, rect.start_row);
	damaged.end_row = GP_MAX(damaged.end_row, rect.end_row);

}

static void repaint_damage(void)
{
	int row, col;

	for (row = damaged.start_row; row < damaged.end_row; row++) {
		for (col = damaged.start_col; col < damaged.end_col; col++) {
			VTermPos pos = {.row = row, .col = col};
			draw_cell(pos);
		}
	}

	update_rect(damaged);
	damage_repainted = 1;
	repaint_sleep_ms = -1;
}


static int term_damage(VTermRect rect, void *user_data)
{
	(void)user_data;

	merge_damage(rect);
//	fprintf(stderr, "rect: %i %i %i %i\n", rect.start_row, rect.end_row, rect.start_col, rect.end_col);
	repaint_sleep_ms = 1;

	return 1;
}

static int term_moverect(VTermRect dest, VTermRect src, void *user_data)
{
	(void)dest;
	(void)src;
	(void)user_data;
	fprintf(stderr, "Move rect!\n");

	return 0;
}

static int term_movecursor(VTermPos pos, VTermPos oldpos, int visible, void *user_data)
{
	(void)user_data;
	unsigned int x = oldpos.col * char_width;
	unsigned int y = oldpos.row * char_height;

	draw_cell(oldpos);
	GP_BackendUpdateRectXYWH(backend, x, y, char_width, char_height);

	x = pos.col * char_width;
	y = pos.row * char_height;

	GP_RectXYWH(backend->pixmap, x, y, char_width, char_height, 0xffffff);
	GP_BackendUpdateRectXYWH(backend, x, y, char_width, char_height);

	//fprintf(stderr, "Move cursor %i %i -> %i %i!\n", oldpos.col, oldpos.row, pos.col, pos.row);

	//vterm_screen_flush_damage(vts);

	return 1;
}

static int term_settermprop(VTermProp prop, VTermValue *val, void *user_data)
{
	(void)user_data;

	switch (prop) {
	case VTERM_PROP_TITLE:
		fprintf(stderr, "caption %s\n", val->string);
		GP_BackendSetCaption(backend, val->string);
		return 1;
	case VTERM_PROP_ALTSCREEN:
		fprintf(stderr, "altscreen\n");
		return 0;
	case VTERM_PROP_ICONNAME:
		fprintf(stderr, "iconname %s\n", val->string);
		return 0;
	case VTERM_PROP_CURSORSHAPE:
		fprintf(stderr, "cursorshape %i\n", val->number);
		return 0;
	case VTERM_PROP_REVERSE:
		fprintf(stderr, "reverse %i\n", val->boolean);
		return 0;
	case VTERM_PROP_CURSORVISIBLE:
		fprintf(stderr, "cursorvisible %i\n", val->boolean);
		return 0;
	case VTERM_PROP_CURSORBLINK:
		fprintf(stderr, "blink %i\n", val->boolean);
		return 0;
	case VTERM_PROP_MOUSE:
		fprintf(stderr, "mouse %i\n", val->number);
		return 0;
	default:
	break;
	}

	fprintf(stderr, "Set term prop!\n");

	return 0;
}

static int term_screen_resize(int new_rows, int new_cols, void *user)
{
	(void)new_rows;
	(void)new_cols;
	(void)user;

	fprintf(stderr, "Resize %i %i\n", new_rows, new_cols);

	return 1;
}

static int term_bell(void *user)
{
	(void)user;
	fprintf(stderr, "Bell!\n");

	return 1;
}

static int term_sb_pushline(int cols, const VTermScreenCell *cells, void *user)
{
	(void)cols;
	(void)cells;
	(void)user;

	fprintf(stderr, "Pushline!\n");

	return 0;
}

static VTermScreenCallbacks screen_callbacks = {
	.damage      = term_damage,
//	.moverect    = term_moverect,
	.movecursor  = term_movecursor,
	.settermprop = term_settermprop,
	.bell        = term_bell,
//	.sb_pushline = term_sb_pushline,
	.resize      = term_screen_resize,
//	.sb_popline  = term_sb_popline,
};

static void term_init(void)
{
	int i;

	vt = vterm_new(rows, cols);
	vterm_set_utf8(vt, 1);

	vts = vterm_obtain_screen(vt);
	vterm_screen_enable_altscreen(vts, 1);
	vterm_screen_set_callbacks(vts, &screen_callbacks, NULL);
	VTermState *vs = vterm_obtain_state(vt);
	vterm_state_set_bold_highbright(vs, 1);

	//vterm_screen_set_damage_merge(vts, VTERM_DAMAGE_SCROLL);
	//vterm_screen_set_damage_merge(vts, VTERM_DAMAGE_ROW);

	/* We use the vterm color as an array index */
	for (i = 0; i < 16; i++) {
		VTermColor col = {i, i, i};
		vterm_state_set_palette_color(vs, i, &col);
	}

	VTermColor bg = {0, 0, 0};
	VTermColor fg  = {7, 7, 7};

	vterm_state_set_default_colors(vs, &fg, &bg);

	vterm_screen_reset(vts, 1);
}

/*
 * Forks and runs a shell, returns master fd.
 */
static int open_console(void)
{
	int fd, pid, flags;

	pid = forkpty(&fd, NULL, NULL, NULL);
	if (pid < 0)
		return -1;

	if (pid == 0) {
		char *shell = getenv("SHELL");

		if (!shell)
			shell = "/bin/sh";

		putenv("TERM=xterm");

		execl(shell, shell, NULL);
	}

	flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);

	return fd;
}

static void close_console(int fd)
{
	close(fd);
}

static int console_read(int fd)
{
	char buf[1024];
	int len;

	len = read(fd, buf, sizeof(buf));

	if (len > 0)
		vterm_input_write(vt, buf, len);

	return len;
}

static void console_write(int fd, char *buf, int buf_len)
{
	write(fd, buf, buf_len);
}

static void console_resize(int fd, int cols, int rows)
{
	struct winsize size = {rows, cols, 0, 0};
	ioctl(fd, TIOCSWINSZ, &size);
}

static void do_exit(int fd)
{
	close_console(fd);
	GP_BackendExit(backend);
	vterm_free(vt);
}

static void event_to_console(GP_Event *ev, int fd)
{
	int ctrl = GP_EventGetKey(ev, GP_KEY_RIGHT_CTRL) ||
		   GP_EventGetKey(ev, GP_KEY_LEFT_CTRL);

	if (ctrl) {
		if (ev->val.key.ascii >= 'a' && ev->val.key.ascii <= 'z') {
			char buf = ev->val.key.ascii - 'a' + 1;
			console_write(fd, &buf, 1);
		}
		return;
	}

	if (ev->val.key.ascii) {
		write(fd, &ev->val.key.ascii, 1);
		return;
	}

	switch (ev->val.key.key) {
	case GP_KEY_UP:
		console_write(fd, "\eOA", 3);
	break;
	case GP_KEY_DOWN:
		console_write(fd, "\eOB", 3);
	break;
	case GP_KEY_RIGHT:
		console_write(fd, "\eOC", 3);
	break;
	case GP_KEY_LEFT:
		console_write(fd, "\eOD", 3);
	break;
	case GP_KEY_DELETE:
		console_write(fd, "\e[3~", 4);
	break;
	case GP_KEY_PAGE_UP:
		console_write(fd, "\e[5~", 4);
	break;
	case GP_KEY_PAGE_DOWN:
		console_write(fd, "\e[6~", 4);
	break;
	case GP_KEY_HOME:
		console_write(fd, "\e[7~", 4);
	break;
	case GP_KEY_END:
		console_write(fd, "\e[8~", 4);
	break;
	case GP_KEY_F1:
		console_write(fd, "\e[11~", 5);
	break;
	case GP_KEY_F2:
		console_write(fd, "\e[12~", 5);
	break;
	case GP_KEY_F3:
		console_write(fd, "\e[13~", 5);
	break;
	case GP_KEY_F4:
		console_write(fd, "\e[14~", 5);
	break;
	case GP_KEY_F5:
		console_write(fd, "\e[15~", 5);
	break;
	case GP_KEY_F6:
		console_write(fd, "\e[17~", 5);
	break;
	case GP_KEY_F7:
		console_write(fd, "\e[18~", 5);
	break;
	case GP_KEY_F8:
		console_write(fd, "\e[19~", 5);
	break;
	case GP_KEY_F9:
		console_write(fd, "\e[20~", 5);
	break;
	case GP_KEY_F10:
		console_write(fd, "\e[21~", 5);
	break;
	case GP_KEY_F11:
		console_write(fd, "\e[23~", 5);
	break;
	case GP_KEY_F12:
		console_write(fd, "\e[24~", 5);
	break;
	}
}

struct RGB {
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

struct RGB RGB_colors[16] = {
	/* BLACK */
	{0x00, 0x00, 0x00},
	/* RED */
	{0xff, 0x00, 0x00},
	/* GREEN */
	{0x00, 0xff, 0x00},
	/* YELLOW */
	{0xff, 0xff, 0x00},
	/* BLUE */
	{0x00, 0x00, 0xff},
	/* MAGENTA */
	{0xff, 0x00, 0xff},
	/* CYAN */
	{0x00, 0xff, 0xff},
	/* GRAY */
	{0xee, 0xee, 0xee},

	/* BRIGHT BLACK */
	{0x44, 0x44, 0x44},
	/* BRIGHT RED */
	{0xff, 0x44, 0x44},
	/* BRIGHT GREEN */
	{0x44, 0xff, 0x44},
	/* BRIGHT YELLOW */
	{0xff, 0xff, 0x44},
	/* BRIGHT BLUE */
	{0x44, 0x44, 0xff},
	/* BRIGHT MAGENTA */
	{0xff, 0x44, 0xff},
	/* BRIGHT CYAN */
	{0x44, 0xff, 0xff},
	/* WHITE */
	{0xff, 0xff, 0xff},
};


static void backend_init(void)
{
	int i;

	backend = GP_BackendInit("X11", "Termini");
	if (backend == NULL) {
		fprintf(stderr, "Failed to initalize backend\n");
		exit(1);
	}

	for (i = 0; i < 16; i++) {
		colors[i] = GP_RGBToPixmapPixel(RGB_colors[i].r,
		                                RGB_colors[i].g,
		                                RGB_colors[i].b,
		                                backend->pixmap);
	}
}

int main(int argc, char *argv[])
{
	backend_init();

	GP_TextStyle style = {
		.font = GP_FontHaxorNarrow17,
		//.font = &GP_DefaultConsoleFont,
	        .pixel_xmul = 1,
		.pixel_ymul = 1,
	};

	GP_TextStyle style_bold = {
		.font = GP_FontHaxorNarrowBold17,
		//.font = &GP_DefaultConsoleFont,
	        .pixel_xmul = 1,
		.pixel_ymul = 1,
	};

	text_style = &style;
	text_style_bold = &style_bold;

	char_width  = GP_TextMaxWidth(text_style, 1);
	char_height = GP_TextHeight(text_style);

	cols = GP_PixmapW(backend->pixmap)/char_width;
	rows = GP_PixmapH(backend->pixmap)/char_height;

	fprintf(stderr, "Cols %i Rows %i\n", cols, rows);

	term_init();

	int fd = open_console();

	struct pollfd fds[2] = {
		{.fd = fd, .events = POLLIN},
		{.fd = backend->fd, .events = POLLIN}
	};

	for (;;) {
		GP_Event ev;

		if (poll(fds, 2, repaint_sleep_ms) == 0)
			repaint_damage();

		while (GP_BackendPollEvent(backend, &ev)) {
			switch (ev.type) {
			case GP_EV_KEY:
				if (ev.code == GP_EV_KEY_UP)
					break;

				event_to_console(&ev, fd);
			break;
			case GP_EV_SYS:
				switch (ev.code) {
				case GP_EV_SYS_RESIZE:
					GP_BackendResizeAck(backend);
					cols = ev.val.sys.w/char_width;
					rows = ev.val.sys.h/char_height;
					vterm_set_size(vt, rows, cols);
					console_resize(fd, cols, rows);
					GP_Fill(backend->pixmap, 0);
					VTermRect rect = {.start_row = 0, .start_col = 0, .end_row = rows, .end_col = cols};
					term_damage(rect, NULL);
					//TODO cursor
				break;
				case GP_EV_SYS_QUIT:
					do_exit(fd);
				break;
				}
			break;
			}
		}

		console_read(fd);
	}

	return 0;
}
