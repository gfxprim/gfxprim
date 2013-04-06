#!/usr/bin/env python

import sys

import gfxprim.core as core
import gfxprim.backends as backends
import gfxprim.gfx as gfx
import gfxprim.text as text
import gfxprim.input as input

def redraw(bk):
    c = bk.context

    black = c.RGBToPixel(0, 0, 0)
    white = c.RGBToPixel(0xff, 0xff, 0xff)

    c.gfx.Fill(black)

    align = text.C.ALIGN_CENTER | text.C.VALIGN_CENTER
    c.text.Text(None, c.w//2, c.h//2, align, white, black, "Hello World!")

    bk.Flip()

def main():
    backend_string = "X11:100x100"

    if len(sys.argv) == 2:
        backend_string = sys.argv[1]

    if len(sys.argv) > 2:
        print("Usage: {} [backend init string]".format(sys.argv[0]))
        sys.exit(1)

    # Create backend window
    bk = backends.BackendInit(backend_string, "Backend Example", sys.stderr)
    assert(bk)

    redraw(bk)

    # Event loop
    while True:
        ev = bk.WaitEvent()

        input.EventDump(ev)

        if (ev.type == input.EV_KEY):
           sys.exit(0)
        elif (ev.type == input.EV_SYS):
           if (ev.code == input.EV_SYS_QUIT):
               sys.exit(0)
           if (ev.code == input.EV_SYS_RESIZE):
               bk.ResizeAck()
               redraw(bk)

if __name__ == '__main__':
    main()
