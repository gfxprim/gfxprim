#!/usr/bin/env python

import sys

import gfxprim.core as core
import gfxprim.loaders as loaders
import gfxprim.filters as filters

orig_path='images/orig/'
self_path='images/blur/'

def write_asciidoc_head(f):
    f.write('.Example Images: Original Image; Gaussian Blur 2 2, 0 4, 4 0, 10 10\n')
    f.write('image:' + orig_path + 'lenna_small.png[\n')
    f.write('\t"Original Image",\n')
    f.write('\tlink="' + orig_path + 'lenna.png"]\n')

def write_img_asciidoc(f, name, fname):
    f.write('image:' + self_path + 'lenna_small_' + fname + '.png[\n')
    f.write('\t"' + name + '",\n')
    f.write('\tlink="' + self_path + 'lenna_' + fname + '.png"]\n')

def write_asciidoc_tail(f):
    f.write('\n')

def gen_blur(f, img, img_small, x_sigma, y_sigma):
    print("Generating Blur %s %s" % (x_sigma, y_sigma))
    name = '%s_%s' % (x_sigma, y_sigma)
    res = img.filters.GaussianBlurAlloc(x_sigma, y_sigma);
    res.loaders.Save('lenna_' + name + '.png')
    res = res.filters.ResizeAlloc(img_small.w, img_small.h, 2)
    res.loaders.Save('lenna_small_' + name + '.png')
    write_img_asciidoc(f, 'Gaussian Blur x_sig=%s y_sig=%s' % (y_sigma, y_sigma), name)

def main():
    lenna = loaders.Load('../../' + orig_path + 'lenna.png')
    lenna_small = loaders.Load('../../' + orig_path + 'lenna_small.png')

    f = open('images.txt', 'w')

    write_asciidoc_head(f)

    gen_blur(f, lenna, lenna_small, 2, 2)
    gen_blur(f, lenna, lenna_small, 0, 4)
    gen_blur(f, lenna, lenna_small, 4, 0)
    gen_blur(f, lenna, lenna_small, 10, 10)

    write_asciidoc_tail(f)

    f.close()

if __name__ == '__main__':
    main()
