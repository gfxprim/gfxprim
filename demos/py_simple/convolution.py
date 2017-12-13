#!/usr/bin/env python
import sys

import gfxprim.core as core
import gfxprim.loaders as loaders
import gfxprim.filters as filters

def main():
    if len(sys.argv) != 2:
        print("usage: convolution.py image")
        sys.exit(1)

    # Load Image
    img = loaders.load(sys.argv[1])
    # Box blur kernel
    kern = [[1, 1, 1, 1, 1],
            [1, 1, 1, 1, 1],
            [1, 1, 1, 1, 1],
            [1, 1, 1, 1, 1],
            [1, 1, 1, 1, 1]]
    res = img.filters.convolution_alloc(kern, 25);
    # Save result into png
    res.loaders.save_png("out.png")

if __name__ == '__main__':
    main()
