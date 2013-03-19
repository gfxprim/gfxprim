#!/usr/bin/env python

import sys

import gfxprim.core as core
import gfxprim.loaders as loaders
import gfxprim.backends as backends

def main():
    if len(sys.argv) != 2:
        print("Takes an image as an argument")
        sys.exit(1)

    # Load Image
    img = loaders.Load(sys.argv[1])
    
    bk = backends.BackendX11Init(None, 0, 0, img.w, img.h, sys.argv[1], 0)
    assert(bk)
    img.Blit(0, 0, bk.context, 0, 0, img.w, img.h)
    bk.Flip()

    # TODO: Input events
    while True:
        bk.Poll()

if __name__ == '__main__':
    main()
