// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2021 Richard Palethorpe (richiejp.com)
 */

/* 1D Cell Automata Demo
 *
 * This implements and displays a nearest-neighbor, one-dimensional
 * binary, cell automata. Additionally it implements a reversible
 * automata, which is almost identical except for a small change to
 * make it reversible. The automata displayed over time in two
 * dimensions, time travels from top to bottom. Although in the
 * reversible case time could be played backwards.
 *
 * The automata works as follows:
 * + Each cell has a state, which is on or off, black or white, boolean etc.
 * + At each time step, the state of a cell in the next step is chosen by a rule.
 * + The rule looks at a cell's current value and its two neighbor values.
 * + There are 2^3 = 8 possible state combinations (patterns) for 3 binary cells.
 * + A rule states which patterns result in a true/black cell in the next time step.
 * + There are 2^8 = 256 possible rules. That is, 256 unique combinations of patterns.
 *
 * So a pattern is a 3 digit binary number, where each digit
 * corresponds to a cell. The middle digit is the center cell, the
 * high order bit the left cell, the low order bit the right cell. A
 * rule can be display by showing a row of patterns and a row of next
 * states.
 *
 *  111 110 101 100 011 010 001 000
 *   0   1   1   0   1   1   1   0
 *
 * Above is rule 110, 0x6e or 01101110. It essentially says to match
 * patterns 110, 101, 011, 010, 001. Where a pattern match results in
 * the cell being set to 1 at the next time step. If no pattern is
 * matched or equivalently, an inactive pattern is matched, then the
 * cell will be set to 0.
 *
 * Again note that each pattern resembles a 3bit binary number. Also
 * the values of the active patterns resemble an 8bit binary
 * number. We can use this to perform efficient matching of the
 * patterns using binary operations.
 *
 * Let's assume our CPU natively operates on 64bit integers. We can
 * pack a 64 cell automata into a single 64bit integer. Each bit
 * corresponds to a cell. If a bit is 1 then it is a black cell and 0
 * for white.
 *
 * The CPU can perform bitwise operations on all 64bits in parallel
 * and without branching.
 *
 * If we shift (>>, rotate[1]) all bits to the right by 1, then we
 * get a new integer where the left neighbor of a bit is now in its
 * position. Likewise if we shift all bits to the left, then we get
 * an integer representing the right neighbors. This gives us 3
 * integers where the left, center and right bits are in the same
 * position. For example, using only 8bits:
 *
 *  left:   0100 1011
 *  center: 1001 0110
 *  right:  0010 1101
 *
 * Each pattern can be represented as a 3bit number, plus a 4th bit
 * to say whether it is active in a given rule. As we want to operate
 * on all 64bits at once in the left, right and center bit fields. We
 * can generate 64bit long masks from the value of each bit in a
 * given pattern.
 *
 * So if we have a pattern where the left cell should be 1, then we
 * can create a 64bit mask of all 1s. If it should be zero, then all
 * zeroes. Likewise for the center and right cells. The masks can be
 * xor'ed[2] (^) with the corresponding cell fields to show if no
 * match occurred. That is the pattern is 1 and the cell is 0 or the
 * cell is 1 and the pattern is 0. We can invert this (~) to give 1
 * when a match occurs.
 *
 * To see whether all components (left, right, center) of a pattern
 * matches we can bitwise and (&) them together. We can then bitwise
 * or[3] (|) the result of the pattern matches together to produce
 * the final values.
 *
 * If we wish to operate on an automata larger than 64 cells, then we
 * can combine multiple ints in an array. After performing the left
 * and right shifts, we get the high or low bit from the next or
 * previous integers in the array. Then set the low and high bits of
 * the right and left bit fields.
 *
 * To make the automata "reversible" an extra step can be added. We
 * look at a cell's previous (in addition to the current, left and
 * right) and if it was 1 then *invert* the next value. This is
 * equivalent to xor'ring the previous value with the next.
 *
 * It is not entirely clear to me what the mathematical implications
 * are of being reversible. However it is important to physics and
 * makes some really cool patterns which mimic nature.
 *
 * The automata definition is taken from Stephen Wolfram's "A new
 * kind of science". He proposes at least one obvious C
 * implementation using arrays of cells. He also provides a table of
 * binary expressions for each rule. E.g. rule 90 reduces to just
 * l^r. It may be possible for the compiler to automatically reduce
 * my implementation to these minimal expressions.
 *
 * To see why, let's consider rule 90 for each pattern
 *
 *  111 110 101 100 011 010 001 000
 *   0   1   0   1   1   0   1   0  = 90
 *
 * First for pattern 000.
 *   000:= active & ~(left ^ l) & ~(center ^ c) & ~(right ^ r);
 *   000:= 0 & ~(0 ^ l) & ~(0 ^ c) & ~(0 ^ r),
 *       = 0
 *
 * Active is 0 so the whole line reduces to 0. Now for 001. Note
 * that 1 here actually means ~0UL, that is 64bit integer max.
 *   001:= 1 & ~(0 ^ l) & ~(0 ^ c) & ~(1 ^ r),
 *   001:= ~l & ~c & r
 *
 * As expected pattern 001 matches l=0, c=0, r=1. Let's just list
 * the remaining patterns or'ed together in their reduced state.
 *
 *     l & c & ~r | l & ~c & ~r | ~l & c & r | ~l & ~c & r
 *   = l & ~r | ~l & r
 *   = l ^ r
 *
 * See on the top row that (l & c & ~r | l & ~c & ~r) or's together
 * c and not c. So we can remove it. Then we get an expression
 * equivalent to xor'ring l and r.
 *
 * In theory at least, the compiler can see that rule only has 256
 * values and create a reduced version of ca1d_rule_apply(_row) for
 * each value. Whether it actually does is not of much practical
 * concern when the rendering code is the bottle neck.
 *
 * However judging from the disassembly from "gcc -O3 -mcpu=native
 * -mtune=native", it may actually do this. Additionally it
 * "vectorizes" the code packing 4 64bit ints at a time into 256bit
 * registers and operating on those. I don't know which part of the
 * code it is vectorising or how. It's possible that what I think is
 * the rule being reduced is something related to vectorisation.
 *
 * To render the automata we take the approach of iterating over each
 * pixel in the image. We calculate which cell the pixel falls inside
 * and set the color of the pixel to that of the cell. That's it.
 *
 *  [1]: We perform a rotating shift which moves the end bit to the start.
 *       This causes the automata to wrap around.
 *  [2]: Combined with exclusive or.
 *  [3]: Or we can use xor as the patterns are mutually exclusive, so
 *       only one may match at a time for each bit.
 */

#include <stdlib.h>
#include <errno.h>
#include <gfxprim.h>

/* If bit n is 1 then make all bits 1 otherwise 0 */
#define BIT_TO_MAX(b, n) (((b >> n) & 1) * ~0UL)

/* Number of bitfields in a row */
static size_t width = 1;
/* Number of steps in the simulation */
static size_t height = 64;
/* Matrix of bitfields representing the automata's state over time */
static uint64_t *steps;
/* Initial conditions */
static uint64_t *init;
/* Zero row mask */
static uint64_t *zeroes;
/* Numeric representation of the current update rule */
static uint8_t rule = 110;
/* Whether to use the reversible version of the current rule */
static int reversible;

static void *uids;

static void ca1d_allocate(void)
{
	if (init)
		gp_vec_free(init);
	init = gp_vec_new(width, sizeof(uint64_t));
	init[width / 2] = 1UL << (63 - (width * 32) % 64);

	if (zeroes)
		gp_vec_free(zeroes);
	zeroes = gp_vec_new(width, sizeof(uint64_t));

	if (steps)
		gp_vec_free(steps);
	steps = gp_matrix_new(width, height, sizeof(uint64_t));
}

/* Apply the current rule to a 64bit segment of a row */
static inline uint64_t ca1d_rule_apply(uint64_t c_prev,
				       uint64_t c,
				       uint64_t c_next,
				       uint64_t c_prev_step)
{
	int i;
	uint64_t l = (c >> 1) ^ (c_prev << 63);
	uint64_t r = (c << 1) ^ (c_next >> 63);
	uint64_t c_next_step = 0;

	for (i = 0; i < 8; i++) {
		uint64_t active = BIT_TO_MAX(rule, i);
		uint64_t left   = BIT_TO_MAX(i, 2);
		uint64_t center = BIT_TO_MAX(i, 1);
		uint64_t right  = BIT_TO_MAX(i, 0);

		c_next_step |=
			active & ~(left ^ l) & ~(center ^ c) & ~(right ^ r);
	}

	return c_next_step ^ c_prev_step;
}

/* Apply the current rule to an entire row */
static inline void ca1d_rule_apply_row(const uint64_t *prev,
				       const uint64_t *cur,
				       uint64_t *next)
{
	size_t i;

	next[0] = ca1d_rule_apply(cur[width - 1], cur[0],
				  cur[GP_MIN(1, width - 1)], prev[0]);

	for (i = 1; i < width - 1; i++) {
		next[i] = ca1d_rule_apply(cur[i - 1], cur[i], cur[i + 1],
					  prev[i]);
	}

	if (i >= width)
		return;

	next[i] = ca1d_rule_apply(cur[i - 1], cur[i], cur[0], prev[i]);
}

static void ca1d_run(void)
{
	const uint64_t *prev = zeroes;
	const uint64_t *cur = steps;
	uint64_t *next = steps + gp_matrix_idx(width, 1, 0);
	size_t i = 1;

	memcpy(steps, init, width * sizeof(uint64_t));

	for (;;) {
		ca1d_rule_apply_row(prev, cur, next);

		if (++i >= height)
			break;

		prev = reversible ? cur : zeroes;
		cur = next;
		next = steps + gp_matrix_idx(width, i, 0);
	}

}

/* Note that i & 63 = i % 64 and i >> 6 = i / 64 as 2**6 = 64. Also
 * use putpixel_raw because it is inlined and we know x and y are
 * inside the pixmap.
 */
static inline void shade_pixel(gp_pixmap *p, gp_coord x, gp_coord y,
			       gp_pixel bg, gp_pixel fg)
{
	gp_pixel px;
	size_t i = (x * (64 * width)) / p->w;
	size_t j = (y * height) / p->h;
	size_t k = 63 - (i & 63);
	uint64_t c = steps[gp_matrix_idx(width, j, i >> 6)];

	c = BIT_TO_MAX(c, k);
	px = (fg & c) | (bg & ~c);

	gp_putpixel_raw(p, x, y, px);
}

static void fill_pixmap(gp_pixmap *p)
{
	uint32_t x, y;
	gp_pixel bg = gp_rgb_to_pixmap_pixel(0xff, 0xff, 0xff, p);
	gp_pixel fg = gp_rgb_to_pixmap_pixel(0x00, 0x00, 0x00, p);
	gp_pixel fill = gp_rgb_to_pixmap_pixel(0xff, 0x00, 0x00, p);
	uint64_t s, t;

	s = gp_time_stamp();
	gp_fill(p, fill);
	t = gp_time_stamp();

	printf("Fill time %lums\n", t - s);

	s = gp_time_stamp();
	ca1d_run();
	t = gp_time_stamp();

	printf("Automata time %lums\n", t - s);

	if (width * 64 > p->w || height > p->h) {
		printf("Automata is larger than screen\n");
		return;
	}

	s = gp_time_stamp();
	for (x = 0; x < p->w; x++) {
		for (y = 0; y < p->h; y++)
			shade_pixel(p, x, y, bg, fg);
	}
	t = gp_time_stamp();

	printf("Fill rects time %lums\n", t - s);
}

static void allocate_backing_pixmap(gp_widget_event *ev)
{
	gp_widget *w = ev->self;
	gp_size l = w->w & 63 ? w->w + 64 - (w->w & 63) : w->w;
	gp_size h = w->h;

	gp_pixmap_free(w->pixmap->pixmap);

	w->pixmap->pixmap = gp_pixmap_alloc(l, h, ev->ctx->pixel_type);

	fill_pixmap(w->pixmap->pixmap);
}

int pixmap_on_event(gp_widget_event *ev)
{
	gp_widget_event_dump(ev);

	switch (ev->type) {
	case GP_WIDGET_EVENT_RESIZE:
		allocate_backing_pixmap(ev);
	break;
	default:
	break;
	}

	return 0;
}

static void pixmap_do_redraw(void)
{
	gp_widget *pixmap = gp_widget_by_uid(uids, "pixmap", GP_WIDGET_PIXMAP);

	fill_pixmap(pixmap->pixmap->pixmap);
	gp_widget_redraw(pixmap);
}

int rule_widget_on_event(gp_widget_event *ev)
{
	struct gp_widget_tbox *tb = ev->self->tbox;
	char tbuf[4] = { 0 };
	char c;
	int r;

	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	switch(ev->self->type) {
	case GP_WIDGET_TBOX:
		switch(ev->sub_type) {
		case GP_WIDGET_TBOX_FILTER:
			c = (char)ev->val;

			if (c < '0' || c > '9')
				return 1;

			if (!gp_vec_strlen(tb->buf))
				return 0;

			strcpy(tbuf, tb->buf);
			tbuf[tb->cur_pos] = c;

			r = strtol(tbuf, NULL, 10);

			return r > 255;
			break;
		case GP_WIDGET_TBOX_EDIT:
			rule = strtol(tb->buf, NULL, 10);
			break;
		default:
			break;
		}
		break;
	case GP_WIDGET_CHECKBOX:
		reversible = ev->self->checkbox->val;
		break;
	default:
		return 0;
	}

	pixmap_do_redraw();

	return 0;
}

static void init_from_text(void)
{
	gp_widget *self = gp_widget_by_uid(uids, "init", GP_WIDGET_TBOX);
	const char *text = gp_widget_tbox_text(self);
	size_t len = gp_vec_strlen(text);

	memset(init, 0, width * sizeof(uint64_t));

	if (!len)
		init[width / 2] = 1UL << (63 - (width * 32) % 64);
	else
		memcpy(init, text, GP_MIN(width * sizeof(uint64_t), len));
}

int width_widget_on_event(gp_widget_event *ev)
{
	struct gp_widget_tbox *tb = ev->self->tbox;
	char c;

	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	switch(ev->sub_type) {
	case GP_WIDGET_TBOX_FILTER:
		c = (char)ev->val;

		return c < '0' || c > '9';

		break;
	case GP_WIDGET_TBOX_EDIT:
		if (!gp_vec_strlen(tb->buf))
			return 0;

		width = GP_MAX(1, strtol(tb->buf, NULL, 10));
		ca1d_allocate();
		init_from_text();
		pixmap_do_redraw();
		break;
	default:
		break;
	}

	return 0;
}

int height_widget_on_event(gp_widget_event *ev)
{
	struct gp_widget_tbox *tb = ev->self->tbox;
	char c;

	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	switch(ev->sub_type) {
	case GP_WIDGET_TBOX_FILTER:
		c = (char)ev->val;

		return c < '0' || c > '9';

		break;
	case GP_WIDGET_TBOX_EDIT:
		if (!gp_vec_strlen(tb->buf))
			return 0;

		height = GP_MAX(2, strtol(tb->buf, NULL, 10));
		ca1d_allocate();
		init_from_text();
		pixmap_do_redraw();
		break;
	default:
		break;
	}

	return 0;
}

int init_widget_on_event(gp_widget_event *ev)
{

	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	switch(ev->sub_type) {
	case GP_WIDGET_TBOX_EDIT:
		init_from_text();
		pixmap_do_redraw();
		break;
	default:
		break;
	}

	return 0;
}

int select_dir_on_event(gp_widget_event *ev)
{
	const char *path;
	gp_widget_dialog *dialog;
	gp_widget *path_tbox;

	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	dialog = gp_widget_dialog_file_open_new(NULL);

	if (gp_widget_dialog_run(dialog) != GP_WIDGET_DIALOG_PATH)
		goto out;

	path = gp_widget_dialog_file_open_path(dialog);
	printf("Selected path '%s'\n", path);

	path_tbox = gp_widget_by_uid(uids, "file path", GP_WIDGET_TBOX);
	gp_widget_tbox_printf(path_tbox, "%s1dca.jpeg", path);

out:
	gp_widget_dialog_free(dialog);

	return 0;
}

int save_on_event(gp_widget_event *ev)
{
	const char *path;
	gp_widget *pixmap_w;
	gp_pixmap *pixmap;
	gp_widget *path_tbox;

	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	path_tbox = gp_widget_by_uid(uids, "file path", GP_WIDGET_TBOX);
	path = gp_widget_tbox_text(path_tbox);

	pixmap_w = gp_widget_by_uid(uids, "pixmap", GP_WIDGET_PIXMAP);
	pixmap = pixmap_w->pixmap->pixmap;
	if (gp_save_image(pixmap, path, NULL)) {
		switch(errno) {
		case EINVAL:
			perror("File extension not found or pixel type not supported by format");
			break;
		case ENOSYS:
			perror("Image format not supported");
			break;
		default:
			perror("Save image failed");
		}
	}

	return 0;
}

int main(int argc, char *argv[])
{
	gp_widget *layout = gp_widget_layout_json("automata.json", &uids);

	if (!layout)
		return 0;

	gp_widget *pixmap = gp_widget_by_uid(uids, "pixmap", GP_WIDGET_PIXMAP);

	gp_widget_event_unmask(pixmap, GP_WIDGET_EVENT_RESIZE);

	ca1d_allocate();
	gp_widgets_main_loop(layout, "Pixmap example", NULL, argc, argv);

	return 0;
}
