#!/usr/bin/env python

import sys

import gfxprim.core as core
import gfxprim.loaders as loaders
import gfxprim.backends as backends
import gfxprim.input as input

def main():
    if len(sys.argv) != 2:
        print("Takes an image as an argument")
        sys.exit(1)

    # Load Image
    img = loaders.Load(sys.argv[1])

    # Create X11 window
    bk = backends.BackendX11Init(None, 0, 0, img.w, img.h, sys.argv[1], 0)
    assert(bk)
    img.Blit(0, 0, bk.pixmap, 0, 0, img.w, img.h)
    bk.Flip()

    # Event loop
    while True:
        ev = bk.WaitEvent()

        input.EventDump(ev)

        if (ev.type == input.EV_KEY):
           sys.exit(0)
        elif (ev.type == input.EV_SYS):
           if (ev.code == input.EV_SYS_QUIT):
               sys.exit(0)

if __name__ == '__main__':
    main()
