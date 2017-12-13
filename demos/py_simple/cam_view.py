#!/usr/bin/env python

from sys import exit
from time import sleep

import gfxprim.core as core
import gfxprim.backends as backends
import gfxprim.input as input
import gfxprim.grabbers as grabbers

def main():
    # Open grabber (i.e. web camera)
    grabber = grabbers.grabber_v4l2_init("/dev/video0", 320, 240);
    assert(grabber)

    # Create X11 window
    bk = backends.x11_init(None, 0, 0, grabber.frame.w, grabber.frame.h, "Grabbers test", 0)
    assert(bk)

    # Start grabber capture
    grabber.start();

    # Event loop
    while True:
        sleep(0.01)

        if (grabber.poll()):
            grabber.frame.blit(0, 0, bk.pixmap, 0, 0, grabber.frame.w, grabber.frame.h)
            bk.flip()

        ev = bk.poll_event()

        if (ev is None):
            continue

        input.event_dump(ev)

        if (ev.type == input.EV_KEY):
           exit(0)
        elif (ev.type == input.EV_SYS):
           if (ev.code == input.EV_SYS_QUIT):
               exit(0)
           if (ev.code == input.EV_SYS_RESIZE):
               bk.resize_ack()

if __name__ == '__main__':
    main()
