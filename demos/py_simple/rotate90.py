#!/usr/bin/env python3
import sys

import gfxprim.core as core
import gfxprim.loaders as loaders
import gfxprim.filters as filters

def main():
    if len(sys.argv) != 3:
        print("USAGE: %s imput_image output_image" % sys.argv[0]);
        sys.exit(1)

    # Turns on debug messages
    core.set_debug_level(10);

    # load Image
    src = loaders.load(sys.argv[1])
    # Rotate by 90 degrees
    res = src.filters.rotate_90_alloc()
    # save Image
    res.loaders.save(sys.argv[2])

if __name__ == '__main__':
    main()
