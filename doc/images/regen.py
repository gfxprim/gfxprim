#!/usr/bin/env python3

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

def convert(pixmap):
    if (pixmap.pixel_type == core.C.PIXEL_RGB332):
        return pixmap.convert(core.C.PIXEL_RGB888)
    return pixmap

class ImgGen:
    def __init__(self, orig_path):
        self.orig_path = orig_path
        self.img = loaders.load('../' + orig_path + 'lenna.png')
        self.img_small = loaders.load('../' + orig_path + 'lenna_small.png')

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
            head = func_name + ': ' + ', '.join(descs)

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

            res = convert(func(self.img, *params))
            loaders.save(res, '../' + fname)

            res = convert(func(self.img_small, *params))
            loaders.save(res, '../' + fname_small)

        self.write_asciidoc_tail()

def main():
    imggen = ImgGen('images/orig/')

    imggen.gen(filters.invert_alloc, [],
               [[]],
               'images/invert/', 'Inverted')

    imggen.gen(filters.brightness_alloc, ['p'],
               [[-.5], [-.2], [.2], [.5]],
               'images/brightness/', 'Brightness')

    imggen.gen(filters.contrast_alloc, ['p'],
               [[.2], [.5], [1.5], [2], [3]],
               'images/contrast/', 'Contrast')

    imggen.gen(filters.brightness_contrast_alloc, ['b', 'c'],
               [[-.2, .8], [-.5, 2], [.2, .8], [.2, 1.5]],
               'images/brightness_contrast/', 'BrightnessContrast')

    imggen.gen(filters.posterize_alloc, ['s'],
               [[2], [3], [4], [5], [6]],
               'images/posterize/', 'Posterize')

    imggen.gen(filters.mirror_h_alloc, [],
               [[]],
               'images/mirror_h/', 'Mirrored Horizontally')

    imggen.gen(filters.mirror_v_alloc, [],
               [[]],
               'images/mirror_v/', 'Mirrored Vertically')

    imggen.gen(filters.rotate_90_alloc, [],
               [[]],
               'images/rotate_90/', 'Rotated by 90 degrees')

    imggen.gen(filters.rotate_180_alloc, [],
               [[]],
               'images/rotate_180/', 'Rotated by 180 degrees')

    imggen.gen(filters.rotate_270_alloc, [],
               [[]],
               'images/rotate_270/', 'Rotated by 270 degrees')

    imggen.gen(filters.gaussian_blur_alloc, ['xsig', 'ysig'],
               [[2, 2], [0, 4], [4, 0], [4, 4], [10, 10]],
               'images/blur/', 'Gaussian Blur')

    imggen.gen(filters.median_alloc, ['xr', 'yr'],
               [[3, 3], [5, 5], [7, 7], [9, 9], [12, 12]],
               'images/median/', 'Median')

    imggen.gen(filters.edge_sharpening_alloc, ['w'],
               [[0.1], [0.3], [0.5], [0.8], [1.0]],
               'images/edge_sharpening/', 'Edge Sharpening')

    imggen.gen(filters.gaussian_noise_add_alloc, ['s', 'm'],
               [[0.03, 0], [0.05, 0], [0.05, -0.1], [0.05, 0.1], [0.07, 0.0]],
               'images/gaussian_noise/', 'Gaussian Additive Noise')

    imggen.gen(filters.convolution_alloc, ['k', 'kd'],
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

    imggen.gen(core.Convert, ['p'],
               [
                [core.C.PIXEL_RGB332],
                [core.C.PIXEL_G8],
                [core.C.PIXEL_G4],
                [core.C.PIXEL_G2],
                [core.C.PIXEL_G1],
               ],
               'images/convert/',
               'Simple Conversion',
               ['RGB332', 'G8', 'G4', 'G2', 'G1'])

    imggen.gen(filters.floyd_steinberg_alloc, ['p'],
               [
                [core.C.PIXEL_RGB332],
                [core.C.PIXEL_G8],
                [core.C.PIXEL_G4],
                [core.C.PIXEL_G2],
                [core.C.PIXEL_G1],
               ],
               'images/floyd_steinberg/',
               'Floyd Steinberg Dithering',
               ['RGB332', 'G8', 'G4', 'G2', 'G1'])

    imggen.gen(filters.sierra_alloc, ['p'],
               [
                [core.C.PIXEL_RGB332],
                [core.C.PIXEL_G8],
                [core.C.PIXEL_G4],
                [core.C.PIXEL_G2],
                [core.C.PIXEL_G1],
               ],
               'images/sierra/',
               'Sierra Dithering',
               ['RGB332', 'G8', 'G4', 'G2', 'G1'])

    imggen.gen(filters.sierra_lite_alloc, ['p'],
               [
                [core.C.PIXEL_RGB332],
                [core.C.PIXEL_G8],
                [core.C.PIXEL_G4],
                [core.C.PIXEL_G2],
                [core.C.PIXEL_G1],
               ],
               'images/sierra_lite/',
               'Sierra Lite Dithering',
               ['RGB332', 'G8', 'G4', 'G2', 'G1'])

    imggen.gen(filters.hilbert_peano_alloc, ['p'],
               [
                [core.C.PIXEL_RGB332],
                [core.C.PIXEL_G8],
                [core.C.PIXEL_G4],
                [core.C.PIXEL_G2],
                [core.C.PIXEL_G1],
               ],
               'images/hilbert_peano/',
               'Hilbert Peano Dithering',
               ['RGB332', 'G8', 'G4', 'G2', 'G1'])


if __name__ == '__main__':
    main()
