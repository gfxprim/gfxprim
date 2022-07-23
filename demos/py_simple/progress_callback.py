#!/usr/bin/env python3

import sys

import gfxprim.core as core
import gfxprim.loaders as loaders
import gfxprim.filters as filters

def progress_callback1(perc):
    sys.stdout.write("\rloading %3.2f%%" % perc)
    sys.stdout.flush()
    return 0

def progress_callback2(perc, args):
    sys.stdout.write("\r%s %3.2f%%" % (args[1], perc))
    sys.stdout.flush()
    return 0

def main():
    if len(sys.argv) != 2:
        print("Takes an image as an argument")
        sys.exit(1)

    try:
       img = loaders.load(sys.argv[1], progress_callback1)
       print('')
    except OSError as detail:
       print("Failed to load image '%s': %s" % (sys.argv[1], detail))
       exit(1)

    try:
        callback = (progress_callback2, "Gaussian Blur")
        img = img.filters.gaussian_blur_alloc(50, 50, callback)
        print('')
    except OSError:
        print("Filter Aborted")

if __name__ == '__main__':
    main()
