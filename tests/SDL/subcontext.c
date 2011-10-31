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
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos                            *
 *                         <jiri.bluebear.dluhos@gmail.com>                  *
 *                                                                           *
 * Copyright (C) 2009-2011 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL/SDL.h>

#include "GP.h"
#include "GP_SDL.h"

static GP_Pixel black, white, gray;

SDL_Surface *display = NULL;
GP_Context context, *sub_context;

SDL_TimerID timer;

SDL_UserEvent timer_event;

static int pause_flag = 0;
static int draw_flag = 0;

Uint32 timer_callback(__attribute__((unused)) Uint32 interval,
			__attribute__((unused)) void *param)
{
	timer_event.type = SDL_USEREVENT;
	SDL_PushEvent((SDL_Event *) &timer_event);
	return 30;
}

static void draw_line(GP_Context *dest, GP_Coord x, GP_Coord y,
                      GP_Size w, GP_Size h, GP_Color col)
{
	GP_Coord x1, x2, y1, y2;

	x1 = random() % w + x;
	y1 = random() % h + y;
	x2 = random() % w + x;
	y2 = random() % h + y;

	GP_Line(dest, x1, y1, x2, y2, col);
}

static void draw_triangle(GP_Context *dest, GP_Coord x, GP_Coord y,
		          GP_Size w, GP_Size h, GP_Color col, int flag)
{
	GP_Coord x1, x2, x3, y1, y2, y3;

	x1 = random() % w + x;
	y1 = random() % h + y;
	x2 = random() % w + x;
	y2 = random() % h + y;
	x3 = random() % w + x;
	y3 = random() % h + y;

	if (flag)
		GP_FillTriangle(dest, x1, y1, x2, y2, x3, y3, col);
	else
		GP_Triangle(dest, x1, y1, x2, y2, x3, y3, col);
}

static void draw_rect(GP_Context *dest, GP_Coord x, GP_Coord y,
		      GP_Size w, GP_Size h, GP_Color col, int flag)
{
	GP_Coord x1, x2, y1, y2;

	x1 = random() % w + x;
	y1 = random() % h + y;
	x2 = random() % w + x;
	y2 = random() % h + y;

	if (flag)
		GP_FillRect(dest, x1, y1, x2, y2, col);
	else
		GP_Rect(dest, x1, y1, x2, y2, col);
}

static void draw_circle(GP_Context *dest, GP_Coord x, GP_Coord y,
		        GP_Size w, GP_Size h, GP_Color col, int flag)
{
	GP_Coord x1, y1, r;

	r = random() % 150;

	x1 = random() % (w - 2*r) + x + r;
	y1 = random() % (h - 2*r) + y + r;

	if (flag)
		GP_FillCircle(dest, x1, y1, r, col);
	else
		GP_Circle(dest, x1, y1, r, col);
}

#define TEXT "Lorem Ipsum Dolor Sit Amet"

static void draw_text(GP_Context *dest, GP_Coord x, GP_Coord y,
                      GP_Size w, GP_Size h, GP_Color col)
{
	GP_Coord x1, y1;
	GP_Size tw, th;

	tw = GP_TextWidth(NULL, TEXT);
	th = GP_TextHeight(NULL);

	x1 = random() % (w - tw) + x;
	y1 = random() % (h - th) + y;

	GP_Text(dest, NULL, x1, y1, GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM, TEXT, col);
}

void redraw_screen(void)
{
	if (pause_flag)
		return;

	SDL_LockSurface(display);

	uint8_t v = random() % 128 + 50;
	GP_Color col;
	if (sub_context->pixel_type == GP_PIXEL_P8)
		col = random() % 256;
	else
		col = GP_RGBToContextPixel(v, v, 255, sub_context); 
	
	/* frame around subcontext */
	GP_Rect(&context, 99, 99,  context.w - 100, context.h - 100, white);

	switch (draw_flag) {
	case 0:
		draw_line(sub_context, -100, -100, sub_context->w + 200, sub_context->h + 200, col);
	break;
	case 1:
		draw_triangle(sub_context, -100, -100, sub_context->w + 200, sub_context->h + 200, col, 0);
	break;
	case 2:
		draw_triangle(sub_context, -100, -100, sub_context->w + 200, sub_context->h + 200, col, 1);
	break;
	case 3:
		draw_rect(sub_context, -100, -100, sub_context->w + 200, sub_context->h + 200, col, 0);
	break;
	case 4:
		draw_rect(sub_context, -100, -100, sub_context->w + 200, sub_context->h + 200, col, 1);
	break;
	case 5:
		draw_circle(sub_context, -100, -100, sub_context->w + 200, sub_context->h + 200, col, 0);
	break;
	case 6:
		draw_circle(sub_context, -100, -100, sub_context->w + 200, sub_context->h + 200, col, 1);
	break;
	case 7:
		draw_text(sub_context, -100, -100, sub_context->w + 200, sub_context->h + 200, col);
	break;
	}

	SDL_Flip(display);

	SDL_UnlockSurface(display);
}

void clear_screen(void)
{
	GP_Fill(&context, black);
	GP_Fill(sub_context, gray);
}

void event_loop(void)
{
	SDL_Event event;

	while (SDL_WaitEvent(&event) > 0) {

		switch (event.type) {

		case SDL_USEREVENT:
			redraw_screen();
			break;
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {
			case SDLK_p:
				pause_flag = !pause_flag;
			break;
			case SDLK_SPACE:
				draw_flag = (draw_flag + 1) % 8;
				clear_screen();
			break;
			case SDLK_ESCAPE:
				return;

			default:
				break;
			}
			break;
		case SDL_QUIT:
			return;
		default:
			break;
		}
	}
}

int main(int argc, char *argv[])
{
	/* Bits per pixel to be set for the display surface. */
	int display_bpp = 0;

	int i;
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-8") == 0) {
			display_bpp = 8;
		} else if (strcmp(argv[i], "-16") == 0) {
			display_bpp = 16;
		}
		else if (strcmp(argv[i], "-24") == 0) {
			display_bpp = 24;
		}
		else if (strcmp(argv[i], "-32") == 0) {
			display_bpp = 32;
		}
	}

	GP_SetDebugLevel(10);

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
		fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
		return 1;
	}

	display = SDL_SetVideoMode(640, 480, display_bpp, SDL_SWSURFACE);
	if (display == NULL) {
		fprintf(stderr, "Could not open display: %s\n", SDL_GetError());
		goto fail;
	}

	GP_SDL_ContextFromSurface(&context, display);
	
	black = GP_ColorToContextPixel(GP_COL_BLACK, &context);
	white = GP_ColorToContextPixel(GP_COL_WHITE, &context);
	gray  = GP_ColorToContextPixel(GP_COL_GRAY_DARK, &context);

	sub_context = GP_ContextSubContext(&context, NULL, 100, 100, 440, 280);
	GP_Fill(sub_context, gray);

	timer = SDL_AddTimer(60, timer_callback, NULL);
	if (timer == 0) {
		fprintf(stderr, "Could not set up timer: %s\n", SDL_GetError());
		goto fail;
	}

	event_loop();

	SDL_Quit();
	return 0;

fail:
	SDL_Quit();
	return 1;
}

