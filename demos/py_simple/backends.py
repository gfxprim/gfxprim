#!/usr/bin/env python

import sys

import gfxprim.core as core
import gfxprim.backends as backends
import gfxprim.gfx as gfx
import gfxprim.text as text
import gfxprim.input as input

def redraw(bk):
    c = bk.pixmap

    black = c.rgb_to_pixel(0, 0, 0)
    white = c.rgb_to_pixel(0xff, 0xff, 0xff)

    c.gfx.fill(black)

    align = text.C.ALIGN_CENTER | text.C.VALIGN_CENTER
    c.text.text(None, c.w//2, c.h//2, align, white, black, "Hello World!")

    bk.flip()

def main():
    backend_string = "X11:100x100"

    if len(sys.argv) == 2:
        backend_string = sys.argv[1]

    if len(sys.argv) > 2:
        print("Usage: %s [backend init string]" % sys.argv[0])
        sys.exit(1)

    # Create backend window
    bk = backends.backend_init(backend_string, "Backend Example")
    assert(bk)

    redraw(bk)

    # Event loop
    while True:
        ev = bk.wait_event()

        input.event_dump(ev)

        if (ev.type == input.EV_KEY):
           sys.exit(0)
        elif (ev.type == input.EV_SYS):
           if (ev.code == input.EV_SYS_QUIT):
               sys.exit(0)
           if (ev.code == input.EV_SYS_RESIZE):
               bk.resize_ack()
               redraw(bk)

if __name__ == '__main__':
    main()
