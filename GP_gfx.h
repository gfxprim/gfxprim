/*
 * This file is part of the gfxprim library.
 *
 * Copyright (c) 2009 Jiri "BlueBear" Dluhos <jiri.bluebear.dluhos@gmail.com>,
 *                    Cyril Hrubis (Metan) <metan@ucw.cz>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef GP_GFX_H
#define GP_GFX_H

#include <SDL/SDL.h>

void GP_Clear(SDL_Surface *surf, long color);

void GP_Circle(SDL_Surface *surf, long color, int xcenter, int ycenter, int r);
void GP_FillCircle(SDL_Surface *surf, long color, int xcenter, int ycenter, int r);

void GP_Ellipse(SDL_Surface *surf, long color, int xcenter, int ycenter, int a, int b);
void GP_FillEllipse(SDL_Surface *surf, long color, int xcenter, int ycenter, int a, int b);

void GP_Rect(SDL_Surface *surf, long color, int x0, int y0, int x1, int y1);
void GP_FillRect(SDL_Surface *surf, long color, int x0, int y0, int x1, int y1);

void GP_Triangle(SDL_Surface *surf, long color, int x0, int y0, int x1, int y1, int x2, int y2);
void GP_FillTriangle(SDL_Surface *surf, long color, int x0, int y0, int x1, int y1, int x2, int y2);

#endif /* GP_GFX_H */

