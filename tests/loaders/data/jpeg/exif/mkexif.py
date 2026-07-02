#!/usr/bin/env python3
"""Inject a synthetic EXIF APP1 segment into a JPEG.

Builds a minimal TIFF structure: IFD0 with a single ExifOffset entry
pointing to an Exif sub-IFD that optionally carries ColorSpace (0xA001)
and/or Gamma (0xA500, a num/den rational, e.g. 22/10 = 2.2 or the
malformed 22/0 with a zero denominator).  Both TIFF byte orders are
supported so the endian-sensitive rational parsing is exercised too.
"""
import struct
import sys
import os

def build_tiff(endian, colorspace, gamma):
    # gamma is None or a (num, den) tuple
    fmt = '<' if endian == 'II' else '>'
    def u16(v): return struct.pack(fmt + 'H', v)
    def u32(v): return struct.pack(fmt + 'I', v)
    def short_value(v):
        # SHORT lives left-justified in the 4-byte value field, in file order
        return u16(v) + b'\x00\x00'

    # TIFF header: byte order, 0x2A, IFD0 offset (= 8, right after header)
    hdr = endian.encode() + u16(0x2A) + u32(8)

    # IFD0: 1 entry (ExifOffset) + next-IFD pointer
    ifd0_off = 8
    ifd0_size = 2 + 1 * 12 + 4
    exif_off = ifd0_off + ifd0_size

    sub_entries = []
    if colorspace is not None:
        sub_entries.append((0xA001, 3, 1, short_value(colorspace)))
    if gamma:
        sub_entries.append((0xA500, 5, 1, None))  # offset patched below

    sub_size = 2 + len(sub_entries) * 12 + 4
    gamma_off = exif_off + sub_size

    ifd0 = u16(1) + u16(0x8769) + u16(4) + u32(1) + u32(exif_off) + u32(0)

    sub = u16(len(sub_entries))
    for tag, typ, cnt, val in sub_entries:
        if val is None:
            val = u32(gamma_off)
        sub += u16(tag) + u16(typ) + u32(cnt) + val
    sub += u32(0)

    tail = (u32(gamma[0]) + u32(gamma[1])) if gamma else b''
    return hdr + ifd0 + sub + tail

def inject(src, dst, endian, colorspace, gamma):
    data = open(src, 'rb').read()
    assert data[:2] == b'\xff\xd8'
    tiff = build_tiff(endian, colorspace, gamma)
    payload = b'Exif\x00\x00' + tiff
    app1 = b'\xff\xe1' + struct.pack('>H', len(payload) + 2) + payload
    open(dst, 'wb').write(data[:2] + app1 + data[2:])

if __name__ == '__main__':
    base = sys.argv[1]
    outdir = sys.argv[2]
    colorspaces = [(None, 'cs-missing'), (1, 'cs-srgb'),
                   (2, 'cs-adobe'), (0xFFFF, 'cs-uncalibrated')]
    for endian in ('II', 'MM'):
        for cs, cs_name in colorspaces:
            for gamma, g_name in ((None, 'nogamma'), ((22, 10), 'gamma22')):
                name = f'{cs_name}_{g_name}_{endian.lower()}.jpg'
                inject(base, os.path.join(outdir, name), endian, cs, gamma)
                print(name)
        # Malformed gamma with a zero denominator: must be ignored by the
        # loader instead of dividing by zero.  Uncalibrated ColorSpace so
        # the gamma handling actually kicks in.
        name = f'cs-uncalibrated_gamma0den_{endian.lower()}.jpg'
        inject(base, os.path.join(outdir, name), endian, 0xFFFF, (22, 0))
        print(name)
