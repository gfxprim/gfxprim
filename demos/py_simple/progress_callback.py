#!/usr/bin/env python

import sys

import gfxprim.core as core
import gfxprim.loaders as loaders
import gfxprim.filters as filters

def progress_callback(callback):
    sys.stdout.write("\rLoading %3.2f%%" % callback)
    sys.stdout.flush()
    return 0

def main():
    if len(sys.argv) != 2:
        print("Takes an image as an argument")
        sys.exit(1)

    callback = core.c_core.GP_ProgressCallback(progress_callback)

    try:
       img = loaders.Load(sys.argv[1], callback)
       print('')
    except OSError as detail:
       print("Failed to load image '%s': %s" % (sys.argv[1], detail))
       exit(1)

    try:
        img = filters.FilterGaussianBlurAlloc(img, 50, 50, callback)
        print('')
    except OSError:
        print("Filter Aborted")

if __name__ == '__main__':
    main()
