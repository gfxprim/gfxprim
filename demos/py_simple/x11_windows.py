#!/usr/bin/env python

import sys

import gfxprim.core as core
import gfxprim.gfx as gfx
import gfxprim.backends as backends
import gfxprim.input as input
import gfxprim.text as text

def redraw(bk, id):
    c = bk.context

    black = c.RGBToPixel(0, 0, 0)
    white = c.RGBToPixel(0xff, 0xff, 0xff)

    c.gfx.Fill(black)

    align = text.C.ALIGN_CENTER | text.C.VALIGN_CENTER
    c.text.Text(None, c.w//2, c.h//2, align, white, black, "%s - %sx%s" % (id, c.w, c.h))

    bk.Flip()

def parse_events(bk, id):

    print("------ Window %s -------" % (id))

    while True:
        ev = bk.GetEvent()

        if (ev == None):
            print("--------------------------")
            return

        input.EventDump(ev)

        if (ev.type == input.EV_KEY and ev.val.val == input.KEY_ESC):
           sys.exit(0)
        elif (ev.type == input.EV_SYS):
           if (ev.code == input.EV_SYS_QUIT):
               sys.exit(0)
           if (ev.code == input.EV_SYS_RESIZE):
               bk.ResizeAck()
               redraw(bk, id)

def main():
    # Create X11 windows
    win1 = backends.BackendX11Init(None, 0, 0, 200, 100, "Win 1", 0)
    win2 = backends.BackendX11Init(None, 0, 0, 200, 100, "Win 2", 0)
    assert(win1)
    assert(win2)

    redraw(win1, "win1")
    redraw(win2, "win2")

    # Event loop
    while True:
        win1.Wait()

        if (win1.EventsQueued()):
            parse_events(win1, "win1")

        if (win2.EventsQueued()):
            parse_events(win2, "win2")

if __name__ == '__main__':
    main()
