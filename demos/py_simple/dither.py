#!/usr/bin/env python
import sys

import gfxprim.core as core
import gfxprim.loaders as loaders
import gfxprim.filters as filters

def main():
    if len(sys.argv) != 2:
        print("usage: dither.py image")
        sys.exit(1)

    # Load Image
    img = loaders.Load(sys.argv[1])
    # Use Floyd-Steinberg dithering
    res = filters.FloydSteinberg_RGB888_Alloc(img, core.C.PIXEL_G1, None)
    # Save result into grayscale png
    res.loaders.SavePNG("out.png")

if __name__ == '__main__':
    main()
