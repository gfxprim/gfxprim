#!/usr/bin/env python

import sys

import gfxprim.core as core
import gfxprim.loaders as loaders

def main():
    if len(sys.argv) != 2:
        print("Takes an image as an argument")
        sys.exit(1)

    # Load Image
    img = loaders.Load(sys.argv[1])
    # Save result
    img.loaders.Save("out.png");

if __name__ == '__main__':
    main()
