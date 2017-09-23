#!/usr/bin/env python

import sys

import gfxprim.core as core
import gfxprim.gfx as gfx
import gfxprim.backends as backends
import gfxprim.input as input

def fill(bk):
    color = bk.pixmap.RGBToPixel(0xee, 0xee, 0xee)
    bk.pixmap.gfx.Fill(color)
    bk.Flip()

def hline(bk):
    fg = bk.pixmap.RGBToPixel(0xee, 0xee, 0xee)
    bg = bk.pixmap.RGBToPixel(0, 0, 0);

    bk.pixmap.gfx.Fill(bg)
    for i in range(0, bk.pixmap.h, 10):
        bk.pixmap.gfx.HLine(0, bk.pixmap.w, i, fg)
    bk.Flip()

def vline(bk):
    fg = bk.pixmap.RGBToPixel(0xee, 0xee, 0xee)
    bg = bk.pixmap.RGBToPixel(0, 0, 0);

    bk.pixmap.gfx.Fill(bg)

    for i in range(0, bk.pixmap.w, 10):
        bk.pixmap.gfx.VLine(i, 0, bk.pixmap.h, fg)

    bk.Flip()

def line(bk):
    fg = bk.pixmap.RGBToPixel(0xee, 0xee, 0xee)
    bg = bk.pixmap.RGBToPixel(0, 0, 0);

    bk.pixmap.gfx.Fill(bg)

    for i in range(0, 2 * max(bk.pixmap.w, bk.pixmap.h), 13):
           bk.pixmap.gfx.Line(0, i, i, 0, fg)

    bk.Flip()

def rect(bk):
    fg = bk.pixmap.RGBToPixel(0xee, 0xee, 0xee)
    bg = bk.pixmap.RGBToPixel(0, 0, 0);

    bk.pixmap.gfx.Fill(bg)

    for i in range(10, 130, 10):
        bk.pixmap.gfx.Rect(i, i, bk.pixmap.w - i, bk.pixmap.h - i, fg)

    bk.Flip()

def triangle(bk):
    fg = bk.pixmap.RGBToPixel(0xee, 0xee, 0xee)
    bg = bk.pixmap.RGBToPixel(0, 0, 0);

    bk.pixmap.gfx.Fill(bg)

    w = bk.pixmap.w
    h = bk.pixmap.h

    for i in range(10, 90, 10):
        bk.pixmap.gfx.Triangle(2*i, i, w - 2*i, i, w//2, h - 2*i, fg)

    bk.Flip()

def tetragon(bk):
    fg = bk.pixmap.RGBToPixel(0xee, 0xee, 0xee)
    bg = bk.pixmap.RGBToPixel(0, 0, 0);

    bk.pixmap.gfx.Fill(bg)

    w = bk.pixmap.w
    h = bk.pixmap.h

    for i in range(10, 70, 10):
        bk.pixmap.gfx.Tetragon(i, i, w-2*i, i, w-i, h-i, 2*i, h-i, fg)

    bk.Flip()

def polygon(bk):
    fg = bk.pixmap.RGBToPixel(0xee, 0xee, 0xee)
    bg = bk.pixmap.RGBToPixel(0, 0, 0);

    bk.pixmap.gfx.Fill(bg)

    w = bk.pixmap.w
    h = bk.pixmap.h

    polygon = [(10, 10), (10, (h-10)//3), ((w-10)//3, (h-10)//2),
               (10, 2*(h-10)//3), (10, h-10), ((w-10)//3, h-10),
               ((w-10)//2, 2*(h-10)//3), (2*(w-10)//3, h-10),
               (w-10, h-10), (w-10, 2*(h-10)//3), (2*(w-10)//3, (h-10)//2),
               (w-10, (h-10)//3), (w-10, 10), (2*(w-10)//3, 10),
               ((w-10)//2, (h-10)//3), ((w-10)//3, 10)]

    bk.pixmap.gfx.Polygon(polygon, fg)

    bk.Flip()

def next(bk, i):

    if (i == 0):
        fill(bk)

    if (i == 1):
        hline(bk)

    if (i == 2):
        vline(bk)

    if (i == 3):
        line(bk)

    if (i == 4):
        rect(bk)

    if (i == 5):
        triangle(bk)

    if (i == 6):
        tetragon(bk)

    if (i == 7):
        polygon(bk)

    i = i + 1;

    if (i >= 8):
        i = 0

    return i

def main():
    # Create X11 window
    bk = backends.BackendX11Init(None, 0, 0, 320, 240, "GFX demo", 0)
    assert(bk)

    bk.Flip()

    i = 0

    print("Press SPACE to draw different shape, ESC to exit")

    # Event loop
    while True:
        ev = bk.WaitEvent()

        input.EventDump(ev)

        if (ev.type == input.EV_KEY and ev.code == input.EV_KEY_DOWN):

            if (ev.val.val == input.KEY_ESC):
                sys.exit(0)

            i = next(bk, i)

        if (ev.type == input.EV_SYS):
           if (ev.code == input.EV_SYS_QUIT):
               sys.exit(0)
           elif (ev.code == input.EV_SYS_RESIZE):
               bk.ResizeAck()
               fill(bk)
               bk.Flip()

if __name__ == '__main__':
    main()
