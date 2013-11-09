#!/usr/bin/env python

import sys

import gfxprim.core as core
import gfxprim.loaders as loaders
import gfxprim.filters as filters

def str_esc(x):
    res = str(x)
    if (res[0] == '-'):
        res = '\\' + res
    return res

def to_str(x, to_str):
    res = ''
    if hasattr(x, '__iter__'):
        for i in x:
            if hasattr(i, '__iter__'):
                res += ('(')
                first = 1
                for j in i:
                    if (first):
                        first = 0
                    else:
                        res += ','
                    res += to_str(j)
                res += (')')
            else:
                res += ('(' + to_str(i)+ ')')
    else:
        return to_str(x)

    return res

class ImgGen:
    def __init__(self, orig_path):
        self.orig_path = orig_path
        self.img = loaders.Load('../' + orig_path + 'lenna.png')
        self.img_small = loaders.Load('../' + orig_path + 'lenna_small.png')

    def write_asciidoc_head(self, dst_path, heading):
        self.f = open('../' + dst_path + 'images.txt', 'w')
        f = self.f
        f.write('.Original Image; ' + heading + '\n')
        f.write('image:' + self.orig_path + 'lenna_small.png[\n')
        f.write('\t"Original Image",\n')
        f.write('\tlink="' + self.orig_path + 'lenna.png"]\n')

    def write_img_asciidoc(self, desc, fname, fname_small):
        f = self.f
        f.write('image:' + fname_small + '[\n')
        f.write('\t"' + desc + '",\n')
        f.write('\tlink="' + fname + '"]\n')

    def write_asciidoc_tail(self):
        self.f.write('\n')
        self.f.close()

    def gen(self, func, func_param_desc, func_params_arr, dst_path, func_name, descs=None):

        print("Generating " + func_name)

        head = func_name + ' '
        head += ', '.join(map(lambda x: ' '.join([func_param_desc[i] + '=' +
                str(x[i]) for i in range(0, len(x))]), func_params_arr))

        if (descs is not None):
            head = ', '.join(descs)

        self.write_asciidoc_head(dst_path, head)

        for i in range(0, len(func_params_arr)):
            params = func_params_arr[i]
            str_pars = [to_str(x, str) for x in params]

            if (descs is not None):
                desc = descs[i]
            else:
                desc = func_name + ' ' + ' '.join(str_pars)

            print(' > ' + desc)

            fname = dst_path + 'lenna_' + ','.join(str_pars) + '.png'
            fname_small = dst_path + 'lenna_small_' + ','.join(str_pars) + '.png'

            self.write_img_asciidoc(desc, fname, fname_small)

            res = func(self.img, *params)
            res.loaders.Save('../' + fname)

            res = func(self.img_small, *params)
            res.loaders.Save('../' + fname_small)

        self.write_asciidoc_tail()

def main():
    imggen = ImgGen('images/orig/')

    imggen.gen(filters.InvertAlloc, [],
               [[]],
               'images/invert/', 'Inverted')

    imggen.gen(filters.BrightnessAlloc, ['p'],
               [[-.5], [-.2], [.2], [.5]],
               'images/brightness/', 'Brightness')

    imggen.gen(filters.ContrastAlloc, ['p'],
               [[.2], [.5], [1.5], [2], [3]],
               'images/contrast/', 'Contrast')

    imggen.gen(filters.BrightnessContrastAlloc, ['b', 'c'],
               [[-.2, .8], [-.5, 2], [.2, .8], [.2, 1.5]],
               'images/brightness_contrast/', 'BrightnessContrast')

    imggen.gen(filters.PosterizeAlloc, ['s'],
               [[2], [3], [4], [5], [6]],
               'images/posterize/', 'Posterize')

    imggen.gen(filters.MirrorHAlloc, [],
               [[]],
               'images/mirror_h/', 'Mirrored Horizontally')

    imggen.gen(filters.MirrorVAlloc, [],
               [[]],
               'images/mirror_v/', 'Mirrored Vertically')

    imggen.gen(filters.Rotate90Alloc, [],
               [[]],
               'images/rotate_90/', 'Rotated by 90 degrees')

    imggen.gen(filters.Rotate180Alloc, [],
               [[]],
               'images/rotate_180/', 'Rotated by 180 degrees')

    imggen.gen(filters.Rotate270Alloc, [],
               [[]],
               'images/rotate_270/', 'Rotated by 270 degrees')

    imggen.gen(filters.GaussianBlurAlloc, ['xsig', 'ysig'],
               [[2, 2], [0, 4], [4, 0], [4, 4], [10, 10]],
               'images/blur/', 'Gaussian Blur')

    imggen.gen(filters.MedianAlloc, ['xr', 'yr'],
               [[3, 3], [5, 5], [7, 7], [9, 9], [12, 12]],
               'images/median/', 'Median')

    imggen.gen(filters.EdgeSharpeningAlloc, ['w'],
               [[0.1], [0.3], [0.5], [0.8], [1.0]],
               'images/edge_sharpening/', 'Edge Sharpening')

    imggen.gen(filters.GaussianNoiseAddAlloc, ['s', 'm'],
               [[0.03, 0], [0.05, 0], [0.05, -0.1], [0.05, 0.1], [0.07, 0.0]],
               'images/gaussian_noise/', 'Gaussian Additive Noise')

    imggen.gen(filters.ConvolutionAlloc, ['k', 'kd'],
               [
                [[[1, 1, 1],
                  [1, 1, 1],
                  [1, 1, 1]], 9],
                [[[1, 1, 1, 1, 1],
                  [1, 1, 1, 1, 1],
                  [1, 1, 1, 1, 1],
                  [1, 1, 1, 1, 1],
                  [1, 1, 1, 1, 1]], 25],
                [[[ 0,  -1,  0],
                  [-1,   4, -1],
                  [ 0,  -1,  0]], 1],
                [[[1, 0, -1],
                  [2, 0, -2],
                  [1, 0, -1]], 1],
                [[[0,  0,  0],
                  [1, -1,  0],
                  [0,  0,  0]], 1],
               ],
               'images/convolution/', 'Convolution',
               ['3x3 Box Blur',
                '5x5 Box Blur',
                '3x3 Laplacian',
                '3x3 Sobel',
                '3x3 Roberts'])

if __name__ == '__main__':
    main()
