#!/usr/bin/env python3

import sys

import gfxprim.core as core
import gfxprim.loaders as loaders

def main():
    if len(sys.argv) != 2:
        print("Takes an image as an argument")
        sys.exit(1)

    # load Image
    img = loaders.load(sys.argv[1])
    # save result
    img.loaders.save("out.png");

if __name__ == '__main__':
    main()
