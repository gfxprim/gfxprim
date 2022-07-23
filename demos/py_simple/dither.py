#!/usr/bin/env python3
import sys

import gfxprim.core as core
import gfxprim.loaders as loaders
import gfxprim.filters as filters

def main():
    if len(sys.argv) != 2:
        print("usage: dither.py image")
        sys.exit(1)

    # load Image
    img = loaders.load(sys.argv[1])
    # Use Floyd-Steinberg dithering
    res = img.filters.floyd_steinberg_alloc(core.C.PIXEL_G1)
    # Save result into grayscale png
    res.loaders.save_png("out.png")

if __name__ == '__main__':
    main()
