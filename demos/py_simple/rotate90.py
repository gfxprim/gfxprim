#!/usr/bin/env python
import sys

import gfxprim.core as core
import gfxprim.loaders as loaders
import gfxprim.filters as filters

def main():
    if len(sys.argv) != 3:
        print("USAGE: %s imput_image output_image" % sys.argv[0]);
        sys.exit(1)

    # Turns on debug messages
    core.SetDebugLevel(10);

    # Load Image
    src = loaders.LoadImage(sys.argv[1], None)
    # Rotate by 90 degrees
    res = filters.FilterRotate90Alloc(src, None)
    # Save Image
    res.Save(sys.argv[2])

if __name__ == '__main__':
    main()
