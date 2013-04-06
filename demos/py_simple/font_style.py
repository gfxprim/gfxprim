#!/usr/bin/env python

import sys

import gfxprim.core as core
import gfxprim.gfx as gfx
import gfxprim.backends as backends
import gfxprim.input as input
import gfxprim.text as text

def redraw(win):
    c = win.context

    black = c.RGBToPixel(0, 0, 0)
    white = c.RGBToPixel(0xff, 0xff, 0xff)

    c.gfx.Fill(black)

    align = text.C.ALIGN_CENTER | text.C.VALIGN_CENTER
    style = text.TextStyle(text.DefaultProportionalFont, 0, 0, 1, 1, 1)

    spacing = 20
    y = 20

    while y < c.h:
        y += text.TextHeight(style) + spacing
        c.text.Text(style, c.w//2, y, align, white, black, "Lorem Ipsum Dolor Sit Amet.")

        style.pixel_xspace += 1
        style.pixel_yspace += 1
        style.pixel_xmul += 1
        style.pixel_ymul += 1

    win.Flip()


def main():
    # Create X11 window
    win = backends.BackendX11Init(None, 0, 0, 800, 600, "Fonts", 0)
    assert(win)

    redraw(win)

    # Event loop
    while True:
        ev = win.WaitEvent()

        if (ev.type == input.EV_KEY and ev.val.val == input.KEY_ESC):
           sys.exit(0)
        elif (ev.type == input.EV_SYS):
           if (ev.code == input.EV_SYS_QUIT):
               sys.exit(0)
           if (ev.code == input.EV_SYS_RESIZE):
               win.ResizeAck()
               redraw(win)

if __name__ == '__main__':
    main()
