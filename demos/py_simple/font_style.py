#!/usr/bin/env python3

import sys

import gfxprim.core as core
import gfxprim.gfx as gfx
import gfxprim.backends as backends
import gfxprim.input as input
import gfxprim.text as text

def redraw(win):
    c = win.pixmap

    black = c.rgb_to_pixel(0, 0, 0)
    white = c.rgb_to_pixel(0xff, 0xff, 0xff)

    c.gfx.fill(black)

    align = text.C.ALIGN_CENTER | text.C.VALIGN_CENTER
    style = text.text_style(text.font_gfxprim, 0, 0, 1, 1, 1)

    spacing = 20
    y = 20

    while y < c.h:
        y += text.text_height(style) + spacing
        c.text.text(style, c.w//2, y, align, white, black, "Lorem Ipsum Dolor Sit Amet.")

        style.pixel_xspace += 1
        style.pixel_yspace += 1
        style.pixel_xmul += 1
        style.pixel_ymul += 1

    win.flip()


def main():
    # Create X11 window
    win = backends.x11_init(None, 0, 0, 800, 600, "Fonts", 0)
    assert(win)

    redraw(win)

    # Event loop
    while True:
        ev = win.wait_event()

        if (ev.type == input.EV_KEY and ev.val.val == input.KEY_ESC):
           sys.exit(0)
        elif (ev.type == input.EV_SYS):
           if (ev.code == input.EV_SYS_QUIT):
               sys.exit(0)
           if (ev.code == input.EV_SYS_RESIZE):
               win.resize_ack()
               redraw(win)

if __name__ == '__main__':
    main()
