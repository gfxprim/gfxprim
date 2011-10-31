#
#  gfxprim_config.py - module configuring GfxPrim code generation and
#                      known PixelTypes
# 

#
# 2011 - Tomas Gavenciak <gavento@ucw.cz>
# 2011 - Cyril Hrubis <metan@ucw.cz>
#
# This file is sourced by all the generating scripts.
# Moreover, the generated files are sourced by almost all Gfxprim sources,
# so a complete recompilation is required after any change. 
#

# Declared pixel sizes:
PS_1BPP_LE = PixelSize(1, bit_endian=LE)
PS_1BPP_BE = PixelSize(1, bit_endian=BE)
PS_2BPP_LE = PixelSize(2, bit_endian=LE)
PS_2BPP_BE = PixelSize(2, bit_endian=BE)
PS_4BPP_LE = PixelSize(4, bit_endian=LE)
PS_4BPP_BE = PixelSize(4, bit_endian=BE)
PS_8BPP = PixelSize(8)
PS_16BPP = PixelSize(16)
PS_24BPP = PixelSize(24)
PS_32BPP = PixelSize(32)
# Experimental:
PS_18BPP_LE = PixelSize(18, bit_endian=LE)

config = GfxPrimConfig(

    # C name and bit-size of the GP_pixel type
    pixel_type = "uint32_t",
    pixel_size = 32,

    # List of pixel sizes (bpp), explicit on purpose
    pixelsizes = [PS_1BPP_LE, PS_1BPP_BE, PS_2BPP_LE, PS_2BPP_BE, PS_4BPP_LE, PS_4BPP_BE,
                  PS_8BPP, PS_16BPP, PS_24BPP, PS_32BPP,
                  PS_18BPP_LE,
                 ],

    # List of PixelTypes, order defines the numbering. 
    # The "Undefined" type is added automatically.
    pixeltypes = [

      #
      # Standard RGB types
      #

      PixelType(name='xRGB8888', pixelsize=PS_32BPP, chanslist=[
	('R', 16, 8),
	('G', 8, 8),
	('B', 0, 8)]),

      PixelType(name='RGBA8888', pixelsize=PS_32BPP, chanslist=[
	  ('R', 24, 8),
	  ('G', 16, 8),
	  ('B', 8, 8),
	  ('A', 0, 8)]),

      PixelType(name='RGB888', pixelsize=PS_24BPP, chanslist=[
	  ('R', 16, 8),
	  ('G', 8, 8),
	  ('B', 0, 8)]),

      PixelType(name='RGB565', pixelsize=PS_16BPP, chanslist=[
	  ('R', 11, 5),
	  ('G', 5, 6),
	  ('B', 0, 5)]),

      PixelType(name='RGB666', pixelsize=PS_18BPP_LE, chanslist=[
	  ('R', 12, 6),
	  ('G', 6, 6),
	  ('B', 0, 6)]),
      
      PixelType(name='xRGB4666', pixelsize=PS_32BPP, chanslist=[
	  ('R', 12, 6),
	  ('G', 6, 6),
	  ('B', 0, 6)]),
      #
      # Palette types
      #
      PixelType(name='P2', pixelsize=PS_2BPP_LE, chanslist=[
	  ('P', 0, 2)]),

      PixelType(name='P4', pixelsize=PS_4BPP_LE, chanslist=[
	  ('P', 0, 4)]),

      PixelType(name='P8', pixelsize=PS_8BPP, chanslist=[
	  ('P', 0, 8)]),

      #
      # Gray-only pixel types
      #
      PixelType(name='G1', pixelsize=PS_1BPP_LE, chanslist=[
	  ('V', 0, 1)]),

      PixelType(name='G2', pixelsize=PS_2BPP_LE, chanslist=[
	  ('V', 0, 2)]),

      PixelType(name='G4', pixelsize=PS_4BPP_LE, chanslist=[
	  ('V', 0, 4)]),

      PixelType(name='G8', pixelsize=PS_8BPP, chanslist=[
	  ('V', 0, 8)]),

      #
      # Experiments
      #

      PixelType(name='VA12', pixelsize=PS_4BPP_BE, chanslist=[
	  ('A', 1, 2),
	  ('V', 3, 1)]),
      ]
    )
