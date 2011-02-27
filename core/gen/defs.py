# Module defining known PixelTypes
# 2011 - Tomas Gavenciak <gavento@ucw.cz>

# NOTE: order defines numbering

from pixeltype import PixelType


PixelType(name='RGBx8888', size=32, chanslist=[
    ('R', 0, 8),
    ('G', 8, 8),
    ('B', 16, 8)])


PixelType(name='RGBA8888', size=32, chanslist=[
    ('R', 0, 8),
    ('G', 8, 8),
    ('B', 16, 8),
    ('A', 24, 8)])


PixelType(name='RGB565', size=16, chanslist=[
    ('R', 0, 5),
    ('G', 5, 6),
    ('B', 11, 5)])


PixelType(name='V8', size=8, chanslist=[
    ('V', 0, 8)])


PixelType(name='V2', size=2, chanslist=[
    ('V', 0, 2)])


PixelType(name='VA12', size=4, chanslist=[
    ('A', 1, 2),
    ('V', 3, 1)])

