#!/usr/bin/env python3
import sys

import gfxprim.core as core
import gfxprim.loaders as loaders
import gfxprim.filters as filters

def main():
    if len(sys.argv) != 3:
        print("usage: blur blur-radii image")
        sys.exit(1)

    radii = float(sys.argv[1])

    # Load Image
    img = loaders.load(sys.argv[2])
    # Do in-place gaussian blur
    filters.gaussian_blur(img, img, radii, radii)
    # Save result
    img.loaders.save_jpg("out.jpg")

if __name__ == '__main__':
    main()
