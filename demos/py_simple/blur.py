#!/usr/bin/env python
import sys

import gfxprim.core as core
import gfxprim.loaders as loaders
import gfxprim.filters as filters

def main():
    if len(sys.argv) != 3:
        print("usage: blur blur-radii image")
        sys.exit(1)

    radii = int(sys.argv[1])

    # Load Image
    img = loaders.LoadImage_Wrap(sys.argv[2])
    # Do in-place gaussian blur
    filters.FilterGaussianBlur(img, img, radii, radii, None)
    # Save result
    loaders.SaveJPG("out.jpg", img, None)

if __name__ == '__main__':
    main()
