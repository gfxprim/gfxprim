#!/usr/bin/env python3
import sys

import gfxprim.core as core
import gfxprim.loaders as loaders
import gfxprim.filters as filters

def main():
    if len(sys.argv) != 3:
        print("USAGE: %s imput_image output_image" % sys.argv[0]);
        sys.exit(1)

    # load Image
    src = loaders.load(sys.argv[1])
    # Resize image to the half of the original
    res = src.filters.resize_linear_lf_int_alloc(src.w//2, src.h//2)
    # save Image
    res.loaders.save(sys.argv[2])

if __name__ == '__main__':
    main()
