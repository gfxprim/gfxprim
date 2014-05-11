#!/usr/bin/env python
import sys

import gfxprim.core as core
import gfxprim.loaders as loaders
import gfxprim.filters as filters

def main():
    if len(sys.argv) != 3:
        print("USAGE: %s imput_image output_image" % sys.argv[0]);
        sys.exit(1)

    # Load Image
    src = loaders.Load(sys.argv[1])
    # Resize image to the half of the original
    res = src.filters.ResizeLFIntAlloc(src.w//2, src.h//2)
    # Save Image
    res.loaders.Save(sys.argv[2])

if __name__ == '__main__':
    main()
