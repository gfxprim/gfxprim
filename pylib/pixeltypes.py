#
#  pixeltypes.py - module defining known PixelTypes
# 

#
# 2011 - Tomas Gavenciak <gavento@ucw.cz>
#
# Every call to PixelType defines one new GP_PixelType, order defines 
# the numbering. Undefined type is defined automatically.
# No other functionality than PixelType() should be defined here.
# 
# This file is sourced by all the generating scripts. 
# Moreover, the generated files are sourced by almost all Gfxprim sources,
# a complete recompilation is required after any change. 
#

#
# Standard RGB types
#

PixelType(name='RGBx8888', size=32, chanslist=[
    ('R', 0, 8),
    ('G', 8, 8),
    ('B', 16, 8)])


PixelType(name='RGBA8888', size=32, chanslist=[
    ('R', 0, 8),
    ('G', 8, 8),
    ('B', 16, 8),
    ('A', 24, 8)])


PixelType(name='RGB888', size=24, chanslist=[
    ('R', 0, 8),
    ('G', 8, 8),
    ('B', 16, 8)])


PixelType(name='RGB565', size=16, chanslist=[
    ('R', 0, 5),
    ('G', 5, 6),
    ('B', 11, 5)])

#
# Palette types
#

PixelType(name='P2', size=2, bit_endian='LE', chanslist=[
    ('P', 0, 2)])


PixelType(name='P4', size=4, bit_endian='LE', chanslist=[
    ('P', 0, 4)])


PixelType(name='P8', size=8, bit_endian='LE', chanslist=[
    ('P', 0, 8)])

#
# Gray-only pixel types
#

PixelType(name='V1', size=1, bit_endian='LE', chanslist=[
    ('V', 0, 1)])


PixelType(name='V2', size=2, bit_endian='LE', chanslist=[
    ('V', 0, 2)])


PixelType(name='V4', size=4, bit_endian='LE', chanslist=[
    ('V', 0, 4)])


PixelType(name='V8', size=8, bit_endian='LE', chanslist=[
    ('V', 0, 8)])


#
# Experiments
#

PixelType(name='VA12', size=4, bit_endian='BE', chanslist=[
    ('A', 1, 2),
    ('V', 3, 1)])

