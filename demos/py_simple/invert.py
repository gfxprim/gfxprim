#!/usr/bin/env python
import sys

import gfxprim.core as core
import gfxprim.loaders as loaders
import gfxprim.filters as filters

def main():
    if len(sys.argv) != 2:
        print("usage: invert.py image")
        sys.exit(1)

    # Load Image
    img = loaders.Load(sys.argv[1])
    # Invert image in-place
    img.filters.Invert(img);
    # Save result into grayscale png
    img.loaders.SavePNG("out.png")

if __name__ == '__main__':
    main()
