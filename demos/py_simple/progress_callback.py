#!/usr/bin/env python

import sys

import gfxprim.core as core
import gfxprim.loaders as loaders
import gfxprim.filters as filters

def progress_callback(callback):
    sys.stdout.write("\rLoading {:3.2f}%".format(callback))
    return 0

def main():
    if len(sys.argv) != 2:
        print("Takes an image as an argument")
        sys.exit(1)

    callback = core.c_core.GP_ProgressCallback(progress_callback)

    img = loaders.Load(sys.argv[1], callback)
    print()

    img = filters.FilterGaussianBlurAlloc(img, 50, 50, callback)
    print()

if __name__ == '__main__':
    main()
