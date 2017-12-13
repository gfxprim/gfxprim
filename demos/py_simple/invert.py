#!/usr/bin/env python
import sys

import gfxprim.core as core
import gfxprim.loaders as loaders
import gfxprim.filters as filters

def main():
    if len(sys.argv) != 2:
        print("usage: invert.py image")
        sys.exit(1)

    # load Image
    img = loaders.load(sys.argv[1])
    # invert image in-place
    img.filters.invert(img);
    # Save result into png
    img.loaders.save_png("out.png")

if __name__ == '__main__':
    main()
