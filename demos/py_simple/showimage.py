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

    # load Image
    img = loaders.load(sys.argv[1])

    # Create X11 window
    bk = backends.x11_init(None, 0, 0, img.w, img.h, sys.argv[1], 0)
    assert(bk)
    img.blit(0, 0, bk.pixmap, 0, 0, img.w, img.h)
    bk.flip()

    # Event loop
    while True:
        ev = bk.wait_event()

        input.event_dump(ev)

        if (ev.type == input.EV_KEY):
           sys.exit(0)
        elif (ev.type == input.EV_SYS):
           if (ev.code == input.EV_SYS_QUIT):
               sys.exit(0)

if __name__ == '__main__':
    main()
