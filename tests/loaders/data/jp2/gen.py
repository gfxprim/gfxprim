#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.1-or-later
#
# Generates the JPEG 2000 test images in valid/, lossless, from the pattern
# below, the jp2 test computes the expected pixels with the same function.
#
# Needs opj_compress from openjpeg, run from this directory.
#

import os
import subprocess
import tempfile

W = 16
H = 12


def pat(x, y, c):
    """ Sample value 0-255 of the component c at x, y, must match jp2.c """
    return (x * x * 7 + y * y * 11 + x * y * 5 + x * 29 + y * 47 + c * 83) & 0xff


def pnm(path, magic, maxval, ncomp, sample):
    with open(path, 'wb') as f:
        f.write(b'%s\n%i %i\n%i\n' % (magic, W, H, maxval))
        size = 2 if maxval > 255 else 1
        for y in range(H):
            for x in range(W):
                for c in range(ncomp):
                    f.write(sample(x, y, c).to_bytes(size, 'big'))


def raw(path, ncomp, sample):
    """ opj_compress reads raw images planar, a component after component """
    with open(path, 'wb') as f:
        f.write(bytes(sample(x, y, c) for c in range(ncomp) for y in range(H)
                      for x in range(W)))


def compress(src, dst, *opts):
    subprocess.run(['opj_compress', '-i', src, '-o', dst, '-n', '3', *opts],
                   check=True, stdout=subprocess.DEVNULL)


def patch_colr(path, enumcs):
    """ Rewrites the enumerated color space in the JP2 colr box """
    with open(path, 'r+b') as f:
        data = f.read()
        i = data.find(b'colr')
        if i < 0 or data[i + 4] != 1:
            raise RuntimeError('No enumerated colr box in ' + path)
        f.seek(i + 7)
        f.write(enumcs.to_bytes(4, 'big'))


with tempfile.TemporaryDirectory() as tmp:
    ppm = os.path.join(tmp, 'rgb.ppm')
    pnm(ppm, b'P6', 255, 3, pat)

    pgm = os.path.join(tmp, 'gray.pgm')
    pnm(pgm, b'P5', 255, 1, pat)

    # PGM with maxval 15 would be encoded with 8 bits, raw keeps 4
    gray4 = os.path.join(tmp, 'gray4.raw')
    raw(gray4, 1, lambda x, y, c: pat(x, y, c) >> 4)

    # The low byte is the pattern of the next component, it's dropped on load
    pgm16 = os.path.join(tmp, 'gray16.pgm')
    pnm(pgm16, b'P5', 65535, 1,
        lambda x, y, c: pat(x, y, c) << 8 | pat(x, y, c + 1))

    cmyk = os.path.join(tmp, 'cmyk.raw')
    raw(cmyk, 4, pat)

    compress(ppm, 'valid/rgb.jp2')
    compress(ppm, 'valid/rgb.j2k')
    compress(ppm, 'valid/sub.jp2', '-s', '2,2')
    compress(pgm, 'valid/gray.jp2')
    compress(gray4, 'valid/gray4.jp2', '-F', '%i,%i,1,4,u' % (W, H))
    compress(pgm16, 'valid/gray16.jp2')

    # The pattern samples are Y, Cb, Cr
    compress(ppm, 'valid/sycc.jp2')
    patch_colr('valid/sycc.jp2', 18)

    compress(cmyk, 'valid/cmyk.jp2', '-F', '%i,%i,4,8,u' % (W, H))
    patch_colr('valid/cmyk.jp2', 12)
