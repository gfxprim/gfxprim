#!/usr/bin/env python3
#
# Heavily based on http://sourceforge.net/projects/bmptestsuite/
#                  by David Costanzo
#
# Redistributed under GPLv2 
#
# Copyright (C) 2012 Cyril Hrubis <metan@ucw.cz>
#

import struct
import os
import stat
import errno
import string
import sys

def _safe_create_dir(dirname):
    "Create a directory named dirname, if it doesn't already exist"
    try:
        os.stat(dirname)

    except OSError as info:
        if info.errno == errno.ENOENT:
            os.mkdir(dirname)
        else:
            raise


def _safe_unlink(filename):
    "Remove a file if it exists"
    try:
        os.unlink(filename)

    except OSError as info:
        if info.errno != errno.ENOENT:
            raise

TOP_LEFT_LOGO = []
TOP_LEFT_LOGO.append("........................................................")
TOP_LEFT_LOGO.append("........................................................")
TOP_LEFT_LOGO.append("...#######..###...####......#.....####..####..#######...")
TOP_LEFT_LOGO.append("......#....#...#..#...#.....#.....#.....#........#......")
TOP_LEFT_LOGO.append("......#....#...#..#...#.....#.....#.....#........#......")
TOP_LEFT_LOGO.append("......#....#...#..####......#.....###...###......#......")
TOP_LEFT_LOGO.append("......#....#...#..#.........#.....#.....#........#......")
TOP_LEFT_LOGO.append("......#....#...#..#.........#.....#.....#........#......")
TOP_LEFT_LOGO.append("......#.....###...#.........####..####..#........#......")
TOP_LEFT_LOGO.append("........................................................")
TOP_LEFT_LOGO.append("........................................................")

class bitmap:
    "The base class for all test bitmaps"

    SIZEOF_FILEINFOHEADER = 14

    # values for Compression
    BI_RGB            = 0
    BI_RLE8           = 1
    BI_RLE4           = 2
    BI_BITFIELDS      = 3
    BI_JPEG           = 4
    BI_PNG            = 5
    BI_ALPHABITFIELDS = 6

    def __init__(self, bits_per_pixel, width, height):
        self.bits_per_pixel = bits_per_pixel
        self.width          = width
        self.height         = height
        self.palette        = []
        self.pixeldata      = ''

    # methods that make up the FILEINFOHEADER
    def get_magic_number(self):
        return b'BM'

    def get_filesize(self):
        sizeof_fileinfoheader   = self.SIZEOF_FILEINFOHEADER
        sizeof_bitmapinfoheader = self.get_bitmap_info_header_size()
        sizeof_palette          = len(self.get_palette())
        sizeof_pixeldata        = len(self.get_pixeldata())

        return sizeof_fileinfoheader + sizeof_bitmapinfoheader + sizeof_palette + sizeof_pixeldata

    def get_reserved1(self):
        return 0

    def get_reserved2(self):
        return 0

    def get_offset_of_bitmap_data(self):
        sizeof_fileinfoheader   = self.SIZEOF_FILEINFOHEADER
        sizeof_bitmapinfoheader = self.get_bitmap_info_header_size()
        sizeof_palette          = len(self.get_palette())

        return sizeof_fileinfoheader + sizeof_bitmapinfoheader + sizeof_palette


    def get_fileinfoheader(self):
        "Return the packed BMPFILEINFOHEADER structure"

        magic_number = self.get_magic_number()
        filesize     = self.get_filesize()
        reserved1    = self.get_reserved1()
        reserved2    = self.get_reserved2()
        offbits      = self.get_offset_of_bitmap_data()

        fileinfoheader = struct.pack(
            '<2sIHHi',
            magic_number,
            filesize,
            reserved1,
            reserved2,
            offbits)

        return fileinfoheader


    def get_bitmap_info_header_size(self):
        return 40

    def get_bits_per_pixel(self):
        """
        Return the biBitCount to put into the BITMAPINFOHEADER.
        This should be 1, 4, 8, or 24.
        """
        return self.bits_per_pixel

    def get_width(self):
        """
        Return the biWidth to put into the BITMAPINFOHEADER.
        This is equal to the width of the bitmap, in pixels.
        """
        return self.width

    def get_height(self):
        "Return the height of bitmap to put into the BITMAPINFOHEADER"
        return self.height

    def get_planes(self):
        "Return the value of biPlanes in BITMAPINFOHEADER"
        return 1

    def get_compression(self):
        "Return the biCompression to put into the BITMAPINFOHEADER"
        return self.BI_RGB

    def get_image_size(self):
        "Return the biSizeImage to put into the BITMAPINFOHEADER"
        return len(self.get_pixeldata())

    def get_pixels_per_meter_x(self):
        "Return the biXPelsPerMeter to put into the BITMAPINFOHEADER"
        return 1000

    def get_pixels_per_meter_y(self):
        "Return the biYPelsPerMeter to put into the BITMAPINFOHEADER"
        return 1000

    def get_colors_used(self):
        "Return the biClrUsed to put into the BITMAPINFOHEADER"
        # assume that we use all colors
        return len(self.palette)

    def get_colors_important(self):
        "Return the biClrImportant to put into the BITMAPINFOHEADER"
        # assume that all colors are important
        return 0

    def get_bitmapinfoheader(self):
        "Return the BITMAPINFOHEADER"

        size               = self.get_bitmap_info_header_size()
        width              = self.get_width()
        height             = self.get_height()
        planes             = self.get_planes()
        bits_per_pixel     = self.get_bits_per_pixel()
        compression        = self.get_compression()
        image_size         = self.get_image_size()
        pixels_per_meter_x = self.get_pixels_per_meter_x()
        pixels_per_meter_y = self.get_pixels_per_meter_y()
        colors_used        = self.get_colors_used()
        colors_important   = self.get_colors_important()

        bitmapinfoheader = struct.pack(
            '<IiihHIIiiII',
            size,
            width,
            height,
            planes,
            bits_per_pixel,
            compression,
            image_size,
            pixels_per_meter_x,
            pixels_per_meter_y,
            colors_used,
            colors_important)

        return bitmapinfoheader

    def get_palette(self):
        "Return the palette for bit depths <= 8"

        # Generate the packed palette from self.palette
        # This will be the empty string if self.palette == []
        pack_string = '<' + 'I' * len(self.palette)
        pack_params = [pack_string] + self.palette
	
        palette = struct.pack(pack_string, *self.palette)
        
        return palette

    def create_pixeldata(self):
        "Return the pixel data"

        raise 'bitmap.create_pixeldata() should never be called'

    def create_raster(self):
        "Return the image as a raster"

        raise 'bitmap.create_pixeldata() should never be called'

    def get_pixeldata(self):
        """
        Create and cache the pixel data on the first call.
        Return the cached value on all subsequent calls.
        """

        if not self.pixeldata:
            self.pixeldata = self.create_pixeldata()

        return self.pixeldata


    def get_scanline_padding_bits(self):
        "Return how many bits are used to pad each scanline"

        alignment = 32 # All bitmap scanlines are DWORD aligned

        # start with the number of bits in all pixels in a row
        padding_bits = alignment - (self.width * self.bits_per_pixel) % alignment
        padding_bits = padding_bits % alignment

        return padding_bits


    def apply_top_left_logo(self, image, on_value, off_value):
        "Draw the TOP_LEFT_LOGO if there's enough room"

        # leave enough space for the 2 pixel border
        x_offset = 2
        y_offset = 2
        
        # only draw the logo if there's enough room
        if len(TOP_LEFT_LOGO[0]) + x_offset <= self.width:
            if len(TOP_LEFT_LOGO) + y_offset <= self.height:

                for row in range(0, len(TOP_LEFT_LOGO)):
                    # find the current row from the top of the image,
                    # based on if this is a top-down or a bottom-up raster
                    if (self.get_height() < 0):
                        # top-down image
                        current_row = image[row + y_offset]
                    else:
                        # bottom-up image
                        current_row = image[self.height - 1 - row - y_offset]


                    # set the pixels to "on" or "off"
                    for col in range(0, len(TOP_LEFT_LOGO[row])):
                        if (TOP_LEFT_LOGO[row][col] == '.'):
                            # change this pixel to black
                            current_row[col + x_offset] = on_value
                        else:
                            # change this pixel to white
                            current_row[col + x_offset] = off_value


    def draw_double_border(self, image, outer_value, inner_value):
        "Draw two single-pixel borders around the image"

        # image must be at least 5 x 5 to draw the border
        if 5 <= self.height and 5 <= self.width:

            # draw the outer border
            for col in range(0, self.width):
                # top border
                image[0][col] = outer_value 

                # bottom border
                image[self.height - 1][col] = outer_value

            for row in range(0, self.height):
                # left border
                image[row][0] = outer_value

                # right border
                image[row][self.width - 1] = outer_value


            # draw the inner border around the image
            for col in range(1, self.width - 1):
                # top border
                image[1][col] = inner_value

                # bottom border
                image[self.height - 2][col] = inner_value

            for row in range(1, self.height - 1):
                # left border
                image[row][1] = inner_value

                # right border
                image[row][self.width - 2] = inner_value


    def write(self, filename):
        "Write the bitmap out to a file"

        _safe_unlink(filename)
        
        bmpfile = open(filename, 'wb')

        fileinfoheader = self.get_fileinfoheader()
        bmpfile.write(fileinfoheader)

        bmpinfoheader = self.get_bitmapinfoheader()
        bmpfile.write(bmpinfoheader)

        palette = self.get_palette()
        bmpfile.write(palette)
            
        pixeldata = self.get_pixeldata()
        bmpfile.write(pixeldata)

        bmpfile.close()

    def write_croppped_bitmap(self, filename):
        """
        Write the bitmap out to a file, but end the file short in the
        middle of scaneline.
        """

        _safe_unlink(filename)
        
        bmpfile = open(filename, 'wb')

        fileinfoheader = self.get_fileinfoheader()
        bmpfile.write(fileinfoheader)

        bmpinfoheader = self.get_bitmapinfoheader()
        bmpfile.write(bmpinfoheader)

        palette = self.get_palette()
        bmpfile.write(palette)
            
        # crop the pixel data roughly in half
        # really, crop it in the middle of a scanline
        pixeldata = self.get_pixeldata()
        crop_point = len(pixeldata) // 2 - (self.width * self.bits_per_pixel // 8) // 2 - 1
        content = pixeldata[0 : crop_point]
        bmpfile.write(content)

        bmpfile.close()


class bitmap_32bpp(bitmap):
    "A bitmap that is 32 bpp uncompressed RGB."

    def __init__(self, width, height):
        bitmap.__init__(self, 32, width, height)

    def create_raster(self):
        "Return the rasterized form of the canonical image in 32 bpp"

        red_width    = self.width / 3
        green_width  = self.width / 3
        blue_width   = self.width - (red_width + green_width)

        # draw the color pattern
        raster = []
        for i in range(0, self.height):

            row = []
            row += [struct.pack('<I', 0x00FF0000)] * red_width
            row += [struct.pack('<I', 0x0000FF00)] * green_width
            row += [struct.pack('<I', 0x000000FF)] * blue_width

            raster.append(row)

        # draw a border
        self.draw_double_border(
            raster,
            struct.pack('<I', 0x00000000),
            struct.pack('<I', 0x00FFFFFF))

        # add in the TOP_LEFT_LOGO
        self.apply_top_left_logo(
            raster,
            struct.pack('<I', 0x00000000),
            struct.pack('<I', 0x00FFFFFF))

        return raster

    def create_pixeldata(self):
        "Return the pixel data as uncompressed 32 bpp RGB"

        raster = self.create_raster()

        # concatenate the rows in the raster image into a flat buffer
        pixeldata = ''
        for row in raster:
            pixeldata += string.join(row, '')

        return pixeldata


class bitmap_32bpp_topdown(bitmap_32bpp):
    "A 'top down' uncompressed RGB bitmap that has 32 bits per pixel."

    def get_height(self):
        # a negative value makes a bitmap top-down
        return -self.height


class bitmap_32bpp_colortable(bitmap_32bpp):
    """
    A bitmap that is 32 bpp uncompressed RGB.
    This bitmap has an 'optimal color palette', which is used for optimizing colors on
    palette-based devices, but does NOT have any color masks.
    It's unclear to me if this is well-formed or not.
    """

    def __init__(self, width, height):
        bitmap_32bpp.__init__(self, width, height)

        self.palette = [
            0x00FF00FF, # magenta
            0x00000000, # black
            0x00FF0000, # red
            0x0000FF00, # green
            0x000000FF, # blue
            0x00FFFFFF] # white


class bitmap_32bpp_888_colortable(bitmap_32bpp):
    """
    A bitmap that is 32 bpp uncompressed RGB.
    This bitmap has three DWORD color masks, followed by an 'optimal color palette',
    which is used for optimizing colors on palette-based devices.
    """

    def __init__(self, width, height):
        bitmap_32bpp.__init__(self, width, height)

        self.palette = [
            0x00FF0000, # red mask
            0x0000FF00, # green mask
            0x000000FF, # blue mask

            0x00FF00FF, # magenta
            0x00000000, # black
            0x00FF0000, # red
            0x0000FF00, # green
            0x000000FF, # blue
            0x00FFFFFF] # white

    def get_colors_used(self):
        "Return the biClrUsed to put into the BITMAPINFOHEADER"
        # This only includes the optimized color palette--not the color masks.
        return 6

    def get_compression(self):
        "Return the biCompression to put into the BITMAPINFOHEADER"
        # must be BI_BITFIELDS because we have color masks
        return self.BI_BITFIELDS

class bitmap_32bpp_101110(bitmap):
    """
    A bitmap that is 32 bpp uncompressed RGB.
    This bitmap uses 10-11-10 RGB color masks.
    """

    def __init__(self, width, height):
        bitmap.__init__(self, 32, width, height)

        self.palette = [
            0x7FE00000, # red mask   (10 bits)
            0x001FFC00, # green mask (11 bits)
            0x000003FF] # blue mask  (10 bits)

    def get_compression(self):
        "Return the biCompression to put into the BITMAPINFOHEADER"
        return self.BI_BITFIELDS

    def get_colors_used(self):
        "Return the biClrUsed to put into the BITMAPINFOHEADER"
        return 0

    def create_pixeldata(self):
        "Return the pixel data as uncompressed 32 bpp RGB"

        red_width    = self.width / 3
        green_width  = self.width / 3
        blue_width   = self.width - (red_width + green_width)

        # draw the color pattern
        raster = []
        for i in range(0, self.height):

            row = []
            row += [struct.pack('<I', 0x7FE00000)] * red_width
            row += [struct.pack('<I', 0x001FFC00)] * green_width
            row += [struct.pack('<I', 0x000003FF)] * blue_width

            raster.append(row)

        # draw a border
        self.draw_double_border(
            raster,
            struct.pack('<I', 0x00000000),
            struct.pack('<I', 0x7FFFFFFF))

        # add in the TOP_LEFT_LOGO
        self.apply_top_left_logo(
            raster,
            struct.pack('<I', 0x00000000),
            struct.pack('<I', 0x7FFFFFFF))

        # concatenate the rows in the raster image into a flat buffer
        pixeldata = ''
        for row in raster:
            pixeldata += string.join(row, '')

        return pixeldata

class bitmap_32bpp_croppedpixeldata(bitmap_32bpp):
    """
    A 32 bpp bitmap that ends in the middle of the pixel data.
    This tests that what happens when a call to fread() fails.
    This bitmap processor should probably process the data that
    it does have AND display a diagnostic.
    """

    def write(self, filename):
        self.write_croppped_bitmap(filename)

class bitmap_24bpp(bitmap):
    "An uncompressed bitmap with 24 bits per pixel. "

    def __init__(self, width, height):
        bitmap.__init__(self, 24, width, height)

    def create_pixeldata(self):
        "Return the pixel data as uncompressed 24 bpp RGB"

        red_width    = self.width / 3
        green_width  = self.width / 3
        blue_width   = self.width - (red_width + green_width)

        pad_width_in_bytes = self.get_scanline_padding_bits() / 8

        # draw the color pattern
        raster = []
        for i in range(0, self.height):

            row = []
            row += ['\x00\x00\xFF'] * red_width
            row += ['\x00\xFF\x00'] * green_width
            row += ['\xFF\x00\x00'] * blue_width

            # pad the scanline to the nearest DWORD boundry
            row += ['\xCC'] * pad_width_in_bytes

            raster.append(row)

        # draw a border
        self.draw_double_border(
            raster,
            '\x00\x00\x00',
            '\xFF\xFF\xFF')

        # add in the TOP_LEFT_LOGO
        self.apply_top_left_logo(
            raster,
            '\x00\x00\x00',
            '\xFF\xFF\xFF')

        # concatenate the rows in the raster image into a flat buffer
        pixeldata = ''
        for row in raster:
            pixeldata += string.join(row, '')

        return pixeldata


class bitmap_24bpp_topdown(bitmap_24bpp):
    "A 'top down' bitmap that has 24 bits per pixel."

    def get_height(self):
        # a negative value makes a bitmap top-down
        return -self.height

class bitmap_24bpp_zeroimagesize(bitmap_24bpp):
    """
    An uncompressed RGB bitmaps that has 24 bits-per-pixel.
    The bitmap has a biSizeImage field of 0.
    The bitmap specification allows the biSizeImage field to be zero
    for uncompressed images.
    """

    def get_image_size(self):
        "Return the biSizeImage to put into the BITMAPINFOHEADER"
        return 0

class bitmap_24bpp_croppedpixeldata(bitmap_24bpp):
    """
    A 24 bpp bitmap that ends in the middle of the pixel data.
    This tests that what happens when a call to fread() fails.
    This bitmap processor should probably process the data that
    it does have AND display a diagnostic.
    """

    def write(self, filename):
        self.write_croppped_bitmap(filename)

class bitmap_555(bitmap):
    "A bitmap that is 5-5-5 uncompressed RGB"

    def __init__(self, width, height):
        bitmap.__init__(self, 16, width, height)

    def create_pixeldata(self):
        "Return the pixel data as uncompressed 5-5-5 RGB"

        red_width    = self.width / 3
        green_width  = self.width / 3
        blue_width   = self.width - (red_width + green_width)

        pad_width_in_bytes = self.get_scanline_padding_bits() / 8

        # draw the color pattern
        raster = []
        for i in range(0, self.height):

            row = []
            row += [struct.pack('<H', 0x7C00)] * red_width
            row += [struct.pack('<H', 0x03E0)] * green_width
            row += [struct.pack('<H', 0x001F)] * blue_width

            # pad the scanline to the nearest DWORD boundry
            row += ['\xCC'] * pad_width_in_bytes

            raster.append(row)

        # draw a border
        self.draw_double_border(
            raster,
            '\x00\x00',
            '\xFF\xFF')

        # add in the TOP_LEFT_LOGO
        self.apply_top_left_logo(
            raster,
            '\x00\x00',
            '\xFF\xFF')

        # concatenate the rows in the raster image into a flat buffer
        pixeldata = ''
        for row in raster:
            pixeldata += string.join(row, '')

        return pixeldata


class bitmap_555_croppedpixeldata(bitmap_555):
    """
    A 5-5-5 16 bpp bitmap that ends in the middle of the pixel data.
    This tests that what happens when a call to fread() fails.
    This bitmap processor should probably process the data that
    it does have AND display a diagnostic.
    """

    def write(self, filename):
        self.write_croppped_bitmap(filename)

class bitmap_565(bitmap):
    "A bitmap that is 5-6-5 uncompressed RGB"

    def __init__(self, width, height):
        bitmap.__init__(self, 16, width, height)

        # This is the bitfields (or bitmasks)
        self.palette = [
            0xF800, # 5 bits for red
            0x07E0, # 6 bits for green
            0x001F] # 5 bits for blue

    def get_compression(self):
        "Return the biCompression to put into the BITMAPINFOHEADER"
        return self.BI_BITFIELDS

    def get_colors_used(self):
        "Return the biClrUsed to put into the BITMAPINFOHEADER"
        return 0

    def create_pixeldata(self):
        "Return the pixel data as uncompressed 5-6-5 RGB"

        red_width    = self.width / 3
        green_width  = self.width / 3
        blue_width   = self.width - (red_width + green_width)

        pad_width_in_bytes = self.get_scanline_padding_bits() / 8

        # draw the color pattern
        raster = []
        for i in range(0, self.height):

            row = []
            row += [struct.pack('<H', 0xF800)] * red_width
            row += [struct.pack('<H', 0x07E0)] * green_width
            row += [struct.pack('<H', 0x001F)] * blue_width

            # pad the scanline to the nearest DWORD boundry
            row += ['\xCC'] * pad_width_in_bytes

            raster.append(row)

        # draw a border
        self.draw_double_border(
            raster,
            '\x00\x00',
            '\xFF\xFF')

        # add in the TOP_LEFT_LOGO
        self.apply_top_left_logo(
            raster,
            '\x00\x00',
            '\xFF\xFF')

        # concatenate the rows in the raster image into a flat buffer
        pixeldata = ''
        for row in raster:
            pixeldata += string.join(row, '')

        return pixeldata


class bitmap_565_topdown(bitmap_565):
    "A 'top down' bitmap that is 5-6-5 uncompressed RGB."

    def get_height(self):
        # a negative value makes a bitmap top-down
        return -self.height



class bitmap_8bpp(bitmap):
    "An uncompressed RGB bitmap that has 8 bits per pixel."

    def __init__(self, width, height):
        bitmap.__init__(self, 8, width, height)

        # Set the "0" pixel to magenta so we can test the
        # difference between pixels we deliberately set to
        # black and pixels that the bitmap processor left at 0.
        self.palette = [
            0x00FF00FF, # magenta
            0x00000000, # black
            0x00FF0000, # red
            0x0000FF00, # green
            0x000000FF, # blue
            0x00FFFFFF] # white

        self.INDEX_MAGENTA = 0
        self.INDEX_BLACK   = 1
        self.INDEX_RED     = 2
        self.INDEX_GREEN   = 3
        self.INDEX_BLUE    = 4
        self.INDEX_WHITE   = 5

        # TRANSPARENT_PIXEL is a special value that means
        # "use a delta to skip beyond this pixel"
        self.TRANSPARENT_PIXEL = -2

    def create_pixeldata(self):
        "Return the pixel data as uncompressed 8 bpp"

        red_width    = self.width / 3
        green_width  = self.width / 3
        blue_width   = self.width - (red_width + green_width)

        pad_width_in_bytes = self.get_scanline_padding_bits() / 8

        # draw the color pattern
        raster = []
        for i in range(0, self.height):

            row = []
            row += [chr(self.INDEX_RED)]     * red_width
            row += [chr(self.INDEX_GREEN)]   * green_width
            row += [chr(self.INDEX_BLUE)]    * blue_width
            row += [chr(self.INDEX_MAGENTA)] * pad_width_in_bytes

            raster.append(row)

        # draw a border
        self.draw_double_border(
            raster,
            chr(self.INDEX_BLACK),
            chr(self.INDEX_WHITE))

        # add in the TOP_LEFT_LOGO
        self.apply_top_left_logo(
            raster,
            chr(self.INDEX_BLACK),
            chr(self.INDEX_WHITE))

        # concatenate the rows in the raster image into a flat buffer
        pixeldata = ''
        for row in raster:
            pixeldata += string.join(row, '')

        return pixeldata


class bitmap_8bpp_topdown(bitmap_8bpp):
    "A 'top down' uncompressed RGB bitmap that has 8 bits per pixel."

    def get_height(self):
        # a negative value makes a bitmap top-down
        return -self.height



class bitmap_8bpp_pixelnotinpalette(bitmap_8bpp):
    """
    A bitmap that is 8 bits per pixel uncompressed RGB.
    Many of the pixels are indexes that don't exist in the palette.
    """

    def __init__(self, width, height):
        bitmap_8bpp.__init__(self, width, height)

        self.palette = [
            0x00000000, # black
            0x00FFFFFF] # white

    def create_pixeldata(self):
        "Return the pixel data as uncompressed 8 bpp"

        pad_width_in_bytes = self.get_scanline_padding_bits() / 8

        palette_length = len(self.palette)

        # the raster loops through all 256 possible indices
        raster = []
        for row in range(0, self.height):

            new_row = []
            for col in range(0, self.width + pad_width_in_bytes):
                # don't draw anything in the palette
                value = chr((col % (256 - palette_length) + palette_length))
                new_row.append(value)

            raster.append(new_row)

        # draw a border
        self.draw_double_border(
            raster,
            chr(self.INDEX_BLACK),
            chr(self.INDEX_WHITE))

        # add in the TOP_LEFT_LOGO
        self.apply_top_left_logo(
            raster,
            chr(self.INDEX_BLACK),
            chr(self.INDEX_WHITE))

        # concatenate the rows in the raster image into a flat buffer
        pixeldata = ''
        for row in raster:
            pixeldata += string.join(row, '')

        return pixeldata


class bitmap_8bpp_nopalette(bitmap_8bpp):
    """
    A bitmap that has 8 bits per pixel and no palette.
    This is an invalid bitmap, but a bitmap processor could use default colors.
    """

    def __init__(self, width, height):
        bitmap_8bpp.__init__(self, width, height)

        # empty the palette
        self.palette = []

class bitmap_8bpp_zerocolorsused(bitmap_8bpp):
    """
    An 8 bpp bitmap with a 'biColorsUsed' field of 0.
    This indicates that the palette contains 256 entries, which it does.
    """

    def __init__(self, width, height):
        bitmap_8bpp.__init__(self, width, height)

        # fill the palette
        while len(self.palette) < 256:
            self.palette.append(0x00CCCCCC)

    def get_colors_used(self):
        return 0

class bitmap_8bpp_largecolorsused(bitmap_8bpp):
    """
    An 8 bpp bitmap with a very large 'biColorsUsed' field.
    This is invalid--biColorsImportant shouldn't exceed 256.
    This attempts to trick the bitmap processor into accessing invalid memory.
    """
    def get_colors_used(self):
        return 0xFFFFFFFF

class bitmap_8bpp_negativecolorsused(bitmap_8bpp):
    """
    An 8 bpp bitmap with a negative 'biColorsUsed' field.
    This is invalid--biColorsImportant can't be below 0.
    This attempts to trick the bitmap processor into accessing invalid memory.
    """
    def get_colors_used(self):
        # -1024 in 32 bit two complement
        return 0xFFFFFC00

class bitmap_8bpp_twocolorsimportant(bitmap_8bpp):
    """
    An 8 bpp bitmap with five colors, but a 'biColorsImportant' field of 2.
    This indicates that only black and white are important.
    """
    def get_colors_important(self):
        return 2

class bitmap_8bpp_largecolorsimportant(bitmap_8bpp):
    """
    An 8 bpp bitmap with five colors, but a very large 'biColorsImportant' field.
    This is invalid--biColorsImportant shouldn't exceed biColorsUsed.
    """
    def get_colors_important(self):
        return 0xFFFFFFFF

class bitmap_8bpp_negativecolorsimportant(bitmap_8bpp):
    """
    An 8 bpp bitmap with a negative 'biColorsImportant' field.
    This is invalid--biColorsImportant can't be below 0.
    """
    def get_colors_used(self):
        # -1024 in 32 bit two complement
        return 0xFFFFFC0

class bitmap_8bpp_croppedpixeldata(bitmap_8bpp):
    """
    An 8 bpp bitmap that ends in the middle of the pixel data.
    This tests that what happens when a call to fread() fails.
    This bitmap processor should probably process the data that
    it does have AND display a diagnostic.
    """

    def write(self, filename):
        self.write_croppped_bitmap(filename)

class bitmap_rle8(bitmap_8bpp):
    "A base class for RLE8 bitmaps that implements some helpers routines."

    def get_compression(self):
        "Return the biCompression to put into the BITMAPINFOHEADER"
        return self.BI_RLE8

    def create_absolute_run(self, row, offset, length):
        "Return the 'absolute mode' encoding of a run of pixels"
        
        if length < 3:
            raise 'bad length: %d' % length

        if 255 < length:
            raise 'bad length: %d' % length
        
        # There are between 3 and 255 pixels left in this row.

        # Mark this run as an absolute encoding
        encoded_run = '\x00' + chr(length)

        for i in range(0, length):
            # Encode each pixel with absolute encoding.
            encoded_run += chr(row[offset + i])

        if len(encoded_run) % 2 != 0:
            # pad the encoded run out to the nearest word boundry
            encoded_run += '\x00'

        return encoded_run

    def create_encoded_run(self, run_length, pixel):
        "Return the 'encoded mode' encoding of a run of pixels"
        return chr(run_length) + chr(pixel)

    def create_end_of_line(self):
        "Return the end-of-line escape sequence"
        return '\x00\x00'

    def create_end_of_bitmap(self):
        "Return the end-of-bitmap escape sequence"
        return '\x00\x01'

    def create_delta(self, right, down):
        "Return a delta escape sequence"
        return '\x00\x02' + chr(right) + chr(down)

    def create_pixeldata(self):
        raise "this should not be called"

    def create_raster(self):
        "Return a rasterized version of the 8bpp canonical image"
        
        # widths are in bytes (pixels)
        red_width   = self.width / 3
        green_width = self.width / 3
        blue_width  = self.width - (red_width + green_width)

        # draw the pattern
        raster = []
        for i in range(0, self.height):

            row = []
            row += [self.INDEX_RED]   * red_width
            row += [self.INDEX_GREEN] * green_width
            row += [self.INDEX_BLUE]  * blue_width

            raster.append(row)

        # draw a border
        self.draw_double_border(
            raster,
            self.INDEX_BLACK,
            self.INDEX_WHITE)

        # add in the TOP_LEFT_LOGO
        self.apply_top_left_logo(
            raster,
            self.INDEX_BLACK,
            self.INDEX_WHITE)

        return raster


class bitmap_rle8_encoded(bitmap_rle8):
    """
    A simple run-length encoded bitmap that has 8 bits per pixel.
    The entire bitmap is in 'encoded mode'.
    """

    def create_pixeldata(self):
        "Return the pixel data as run-length encoded 4 bpp"
        
        raster = self.create_raster()

        pixeldata = ''
        run_length = 0
        prev_pixel = -1
        for row in range(0, len(raster)):
            for col in range(0, len(raster[row])):
                
                cur_pixel = raster[row][col]

                if run_length == 255 or (run_length != 0 and prev_pixel != cur_pixel):
                    # There's no more room on this run OR
                    # The current run has ended.

                    # Write the run and start a new one
                    pixeldata += self.create_encoded_run(run_length, prev_pixel)

                    run_length = 0
                    prev_pixel = -1


                if run_length == 0:
                    # start a new run
                    prev_pixel = cur_pixel
                    run_length = 1

                elif prev_pixel == raster[row][col]:
                    # continue this run
                    run_length += 1

            # flush the last run
            if run_length != 0:
                pixeldata += self.create_encoded_run(run_length, prev_pixel)

                run_length = 0
                prev_pixel = -1

            # end-of-line
            pixeldata += self.create_end_of_line()
              
        # end-of-bitmap
        pixeldata += self.create_end_of_bitmap()
                        
        return pixeldata


class bitmap_rle8_delta(bitmap_rle8):
    """
    A simple run-length encoded bitmap that has 8 bits per pixel.
    The bitmap uses 'delta escapes'.
    """

    def __init__(self, width, height):
        bitmap_rle8.__init__(self, width, height)

        # fill the rest of the palette with grey
        for i in range(len(self.palette), 256):
            self.palette.append(0x00CCCCCC)

    def create_raster(self):
        "Return a rastersized form of a bitmap that is good for making deltas"
        
        # widths are in bytes (pixels)
        red_width   = self.width / 3
        green_width = self.width / 3
        blue_width  = self.width - (red_width + green_width)

        # draw the pattern
        raster = []
        for i in range(0, self.height):

            row = []
            row += [self.INDEX_RED]         * red_width
            row += [self.TRANSPARENT_PIXEL] * green_width
            row += [self.INDEX_BLUE]        * blue_width

            raster.append(row)

        # draw an invisible border
        self.draw_double_border(
            raster,
            self.TRANSPARENT_PIXEL,
            self.TRANSPARENT_PIXEL)

        # add in the TOP_LEFT_LOGO
        self.apply_top_left_logo(
            raster,
            self.INDEX_BLACK,
            self.INDEX_WHITE)

        return raster

    def create_pixeldata(self):
        "Return the pixel data as run-length encoded 8 bpp"

        raster = self.create_raster()

        pixeldata = ''
        run_length = 0
        prev_pixel = -1
        for row in range(0, len(raster)):

            # check if the row contains nothing but transparent pixels
            row_is_all_transparent = 1
            for col in range(0, len(raster[row])):
                if raster[row][col] != self.TRANSPARENT_PIXEL:
                    row_is_all_transparent = 0
                    break
               
            if row_is_all_transparent:
                # the entire row is entirely transparent. Do a delta.
                pixeldata += self.create_delta(0, 1)

            else:
                # there are some non-transparent pixels in this row.
                for col in range(0, len(raster[row])):

                    cur_pixel = raster[row][col]

                    if run_length == 255 or (run_length != 0 and prev_pixel != cur_pixel):
                        # There's no more room on this run OR
                        # The current run has ended.

                        # Write the run and start a new one
                        if prev_pixel == self.TRANSPARENT_PIXEL:
                            # this run is encoded as a delta
                            pixeldata += self.create_delta(
                                run_length,
                                0)
                        else:
                            # this run is encoded as a regular run
                            pixeldata += self.create_encoded_run(
                                run_length,
                                prev_pixel)

                        run_length = 0
                        prev_pixel = -1


                    if run_length == 0:
                        # start a new run
                        prev_pixel = cur_pixel
                        run_length = 1

                    elif prev_pixel == raster[row][col]:
                        # continue this run
                        run_length += 1

                # flush the last run
                if run_length != 0:

                    # We don't have to write a delta for transparent
                    # pixels because the end-of-line marker will take
                    # care of that.
                    if prev_pixel != self.TRANSPARENT_PIXEL:
                        # this run is encoded as a regular run
                        pixeldata += self.create_encoded_run(
                            run_length,
                            prev_pixel)

                    run_length = 0
                    prev_pixel = -1

                # end-of-line
                pixeldata += self.create_end_of_line()
              
        # end-of-bitmap
        pixeldata += self.create_end_of_bitmap()
                        
        return pixeldata             


class bitmap_rle8_absolute(bitmap_rle8):
    """
    A run-length encoded bitmap that has 8 bits per pixel.
    The entire bitmap is in 'absolute mode'.
    """

    def create_pixeldata(self):
        "Return the pixel data as an RLE8 in absolute mode (uncompressed)"
        
        raster = self.create_raster()

        pixeldata = ''
        for row in range(0, len(raster)):

            cur_row = raster[row]

            col = 0
            while col < len(cur_row):
                remaining = len(cur_row) - col
                if 255 < remaining:
                    # There are more than 255 pixels left in this row.
                    # Encode all 255 pixels.
                    pixeldata += self.create_absolute_run(cur_row, col, 255)
                    col += 255

                elif 3 <= remaining:
                    # There are between 3 and 255 pixels left in this row.
                    # Encode them all with absolute encoding.
                    pixeldata += self.create_absolute_run(cur_row, col, remaining)
                    col += remaining

                else:
                    raise 'Unsupported width: %d' % remaining

            # end-of-line
            pixeldata += self.create_end_of_line()
              
        # end-of-bitmap
        pixeldata += self.create_end_of_bitmap()

        return pixeldata


class bitmap_rle8_blank(bitmap_rle8):
    """
    A simple run-length encoded bitmap consists of just an end-of-bitmap marker.
    """

    def __init__(self, width, height):
        bitmap_rle8.__init__(self, width, height)

        # fill the rest of the palette with grey
        # so that we can see the difference between uninitialized
        # memory and pixels that should be unspecified.
        for i in range(len(self.palette), 256):
            self.palette.append(0x00CCCCCC)

    def create_pixeldata(self):
        "Return the pixel data as run-length encoded 8 bpp"
        
        return self.create_end_of_bitmap()


class bitmap_rle8_noendofbitmap(bitmap_rle8_encoded):
    """
    An RLE8 compressed bitmap with no end-of-bitmap sequence.
    This is techinically invalid, but the bitmap processor should
    be able to treat the end-of-file as an end-of-bitmap.
    """

    def create_end_of_bitmap(self):
        return ''


class bitmap_rle8_topdown(bitmap_rle8_encoded):
    """
    An RLE8 compressed bitmap with a negative height.
    This is an illegal value: top-down images cannot be compressed.
    However, many bitmap processors allow top-down compressed images.
    """

    def get_height(self):
        return -self.height


class bitmap_rle8_toomuchdata(bitmap_rle8_encoded):
    """
    A simple run-length encoded bitmap that has 8 bits per pixel.
    The entire bitmap is in 'encoded mode'.
    It has twice as much data as it should.
    Since the RLE8 format is mostly a series of drawing directives,
    this tests that the RLE8 processor keeps memory access within
    the size of the image.
    """

    def create_pixeldata(self):
        "Return the pixel data as run-length encoded in 8 bpp"
        
        raster = self.create_raster()

        pixeldata = ''
        run_length = 0
        prev_pixel = -1
        for row in range(0, len(raster)):
            for col in range(0, len(raster[row])):
                
                cur_pixel = raster[row][col]

                if run_length == 255 or (run_length != 0 and prev_pixel != cur_pixel):
                    # There's no more room on this run OR
                    # The current run has ended.

                    # Write the run and start a new one
                    pixeldata += self.create_encoded_run(run_length, prev_pixel)

                    run_length = 0
                    prev_pixel = -1


                if run_length == 0:
                    # start a new run
                    prev_pixel = cur_pixel
                    run_length = 1

                elif prev_pixel == raster[row][col]:
                    # continue this run
                    run_length += 1

            # flush the last run
            if run_length != 0:
                pixeldata += self.create_encoded_run(run_length, prev_pixel)

                run_length = 0
                prev_pixel = -1

            # end-of-line
            pixeldata += self.create_end_of_line()
              
        # double the height of the image.
        pixeldata *= 2

        # end-of-bitmap
        pixeldata += self.create_end_of_bitmap()
                        
        return pixeldata


class bitmap_rle8_deltaleavesimage(bitmap_rle8_encoded):
    """
    A simple run-length encoded bitmap that has 8 bits per pixel.
    The bitmap contains 'delta' escape sequences the leave the image.
    The intent is to trick the processor into accessing invalid memory.
    """

    def create_pixeldata(self):
        "Return the pixel data as run-length encoded 8 bpp"

        # Tell the image processor to move off of the image
        # before drawing anything.
        pixeldata = ''
        i = 0
        while i < self.height:
            pixeldata += self.create_delta(255, 0)
            i += 255

        # draw the image
        pixeldata += bitmap_rle8_encoded.create_pixeldata(self)
                        
        return pixeldata


class bitmap_rle8_croppedrun(bitmap_rle8):
    """
    A simple run-length encoded bitmap that has 8 bits per pixel.
    The entire bitmap is in 'encoded mode'.
    The pixel data ends prematurely--in the middle of an encoded
    escape sequence.
    """

    def create_pixeldata(self):
        "Return the pixel data as run-length encoded 8 bpp"
        
        raster = self.create_raster()

        pixeldata = ''
        run_length = 0
        prev_pixel = -1
        for row in range(0, len(raster) / 2):
            for col in range(0, len(raster[row])):
                
                cur_pixel = raster[row][col]

                if run_length == 255 or (run_length != 0 and prev_pixel != cur_pixel):
                    # There's no more room on this run OR
                    # The current run has ended.

                    # Write the run and start a new one
                    pixeldata += self.create_encoded_run(run_length, prev_pixel)

                    run_length = 0
                    prev_pixel = -1


                if run_length == 0:
                    # start a new run
                    prev_pixel = cur_pixel
                    run_length = 1

                elif prev_pixel == raster[row][col]:
                    # continue this run
                    run_length += 1

            # flush the last run
            if run_length != 0:
                pixeldata += self.create_encoded_run(run_length, prev_pixel)

                run_length = 0
                prev_pixel = -1

            # end-of-line
            pixeldata += self.create_end_of_line()
              
        # append the cropped run
        last_run = self.create_encoded_run(100, 2)
        cropped_run = last_run[0 : len(last_run) - 1]
        pixeldata += cropped_run
                        
        return pixeldata


class bitmap_rle8_croppedabsolute(bitmap_rle8):
    """
    A simple run-length encoded bitmap that has 8 bits per pixel.
    The entire bitmap is in 'absolute mode'.
    The pixel data ends prematurely--in the middle of an absolute
    escape sequence.
    """

    def create_pixeldata(self):
        "Return the pixel data as RLE8 encoded in 'absolute mode' (uncompressed)"
        raster = self.create_raster()
        
        pixeldata = ''
        for row in range(0, len(raster)):
            
            cur_row = raster[row]

            col = 0
            while col < len(cur_row):
                remaining = len(cur_row) - col
                if 255 < remaining:
                    # There are more than 255 pixels left in this row.
                    # Encode all 255 pixels.
                    absolute_run = self.create_absolute_run(cur_row, col, 255)
                    col += 255

                elif 3 <= remaining:
                    # There are between 3 and 255 pixels left in this row.
                    # Encode them all with absolute encoding.
                    absolute_run = self.create_absolute_run(cur_row, col, remaining)
                    col += remaining

                else:
                    raise 'Unsupported width: %d' % remaining

                if len(raster) / 2 <= row:
                    # crop this encoding and return
                    cropped_run = absolute_run[0 : len(absolute_run) - 1]
                    pixeldata += cropped_run
                    return pixeldata

                # append this encoding and keep processing the current row
                pixeldata += absolute_run

            # end-of-line
            pixeldata += self.create_end_of_line()

        return pixeldata

class bitmap_rle8_croppeddelta(bitmap_rle8_delta):
    """
    A simple run-length encoded bitmap that has 8 bits per pixel.
    The bitmap uses 'delta escapes'.
    The file ends in the middle of a delta escape.
    """

    def create_pixeldata(self):
        "Return the pixel data as run-length encoded 8 bpp"
        
        raster = self.create_raster()

        pixeldata = ''
        run_length = 0
        prev_pixel = -1
        for row in range(0, len(raster)):

            # check if the row contains nothing but transparent pixels
            row_is_all_transparent = 1
            for col in range(0, len(raster[row])):
                if raster[row][col] != self.TRANSPARENT_PIXEL:
                    row_is_all_transparent = 0
                    break

            if row_is_all_transparent:
                # the entire row is entirely transparent. Do a delta.
                pixeldata += self.create_delta(0, 1)

            else:
                # there are some non-transparent pixels in this row.
                for col in range(0, len(raster[row])):

                    cur_pixel = raster[row][col]

                    if run_length == 255 or (run_length != 0 and prev_pixel != cur_pixel):
                        # There's no more room on this run OR
                        # The current run has ended.

                        # Write the run and start a new one
                        if prev_pixel == self.TRANSPARENT_PIXEL:
                            # this run is encoded as a delta
                            delta = self.create_delta(
                                run_length,
                                0)

                            if len(raster) / 2 <= row:
                                # we are half-way through the image.
                                # crop the delta and return what we have
                                cropped_delta = delta[0 : len(delta) - 1]
                                pixeldata += cropped_delta
                                return pixeldata
                                
                            pixeldata += delta

                        else:
                            # this run is encoded as a regular run
                            pixeldata += self.create_encoded_run(
                                run_length,
                                prev_pixel)

                        run_length = 0
                        prev_pixel = -1


                    if run_length == 0:
                        # start a new run
                        prev_pixel = cur_pixel
                        run_length = 1

                    elif prev_pixel == raster[row][col]:
                        # continue this run
                        run_length += 1

                # flush the last run
                if run_length != 0:

                    # We don't have to write a delta for transparent
                    # pixels because the end-of-line marker will take
                    # care of that.
                    if prev_pixel != self.TRANSPARENT_PIXEL:
                        # this run is encoded as a regular run
                        pixeldata += self.create_encoded_run(
                            run_length,
                            prev_pixel)

                    run_length = 0
                    prev_pixel = -1

                # end-of-line
                pixeldata += self.create_end_of_line()

        # end-of-bitmap
        pixeldata += self.create_end_of_bitmap()
                        
        return pixeldata


class bitmap_rle8_noendofline(bitmap_rle8_encoded):
    """
    An RLE8 compressed bitmap with no end-of-line sequences.
    It is unclear if a bitmap processor should implicitly add
    end-of-line markers when the pixel run the row's width,
    or if it should ignore all pixel data beyond the row's width.
    """

    def create_end_of_line(self):
        return ''


class bitmap_4bpp(bitmap):
    "An uncompressed bitmap that has 4 bits per pixel."

    def __init__(self, width, height):
        bitmap.__init__(self, 4, width, height)

        # Set the "0" pixel to magenta so we can test the
        # difference between pixels we deliberately set to
        # black and pixels that the bitmap processor left at 0.
        self.palette = [
            0x00FF00FF, # magenta
            0x00000000, # black
            0x00FF0000, # red
            0x0000FF00, # green
            0x000000FF, # blue
            0x00FFFFFF] # white

        self.INDEX_MAGENTA = 0
        self.INDEX_BLACK   = 1
        self.INDEX_RED     = 2
        self.INDEX_GREEN   = 3
        self.INDEX_BLUE    = 4
        self.INDEX_WHITE   = 5

        # TRANSPARENT_PIXEL is a special value that means
        # "use a delta to skip beyond this pixel"
        self.TRANSPARENT_PIXEL = -2

    def create_pixeldata(self):
        "Return the pixel data as uncompressed 4 bpp"
        
        # widths are in nibbles (pixels)
        red_width   = self.width // 3
        green_width = self.width // 3
        blue_width  = self.width - (red_width + green_width)

        pad_width   = self.get_scanline_padding_bits() // 4

        # draw the pattern
        raster = []
        for i in range(0, self.height):

            row = []
            row += [self.INDEX_RED]     * red_width
            row += [self.INDEX_GREEN]   * green_width
            row += [self.INDEX_BLUE]    * blue_width
            row += [self.INDEX_MAGENTA] * pad_width

            raster.append(row)

        # draw a border
        self.draw_double_border(raster, self.INDEX_BLACK, self.INDEX_WHITE)

        # add in the TOP_LEFT_LOGO
        self.apply_top_left_logo(raster, self.INDEX_BLACK, self.INDEX_WHITE)

        pixeldata = ''
        for row in range(0, len(raster)):

            # a closure for helping to add runs to the image
            class nibblestream:
                def __init__(self):
                    self.nextByte = 0
                    self.index    = 0
                    self.scanline = ''

                def appendnibble(self, value):
                    # shift the byte over and OR-in the low bit
                    self.nextByte = (self.nextByte << 4) | value
                    self.index += 4

                    if (self.index == 8):
                        # this byte is full--write it
                        self.scanline += chr(self.nextByte)
                        self.nextByte = 0
                        self.index    = 0

            appender = nibblestream()
            for col in range(0, len(raster[row])):
                appender.appendnibble(raster[row][col])
            pixeldata += appender.scanline

        return pixeldata


class bitmap_4bpp_topdown(bitmap_4bpp):
    "A 'top down' uncompressed RGB bitmap that has 4 bits per pixel."

    def get_height(self):
        # a negative value makes a bitmap top-down
        return -self.height

class bitmap_4bpp_croppedpixeldata(bitmap_4bpp):
    """
    An 4 bpp bitmap that ends in the middle of the pixel data.
    This tests that what happens when a call to fread() fails.
    This bitmap processor should probably process the data that
    it does have AND display a diagnostic.
    """

    def write(self, filename):
        self.write_croppped_bitmap(filename)


class bitmap_rle4(bitmap_4bpp):
    "A base class for RLE4 bitmaps that implements some helpers routines."

    def get_compression(self):
        "Return the biCompression to put into the BITMAPINFOHEADER"
        return self.BI_RLE4

    def create_absolute_run(self, row, offset, length):
        "Return the 'absolute mode' encoding of a run of pixels"
        
        if length < 3:
            raise 'bad length: %d' % length

        if 255 < length:
            raise 'bad length: %d' % length
        
        # There are between 3 and 255 pixels left in this row.

        # Mark this run as an absolute encoding
        encoded_run = '\x00' + chr(length)

        for i in range(0, length / 2):
            # Encode each pixel with absolute encoding.
            byte = (row[offset + 2 * i] << 4) | (row[offset + 2 * i + 1])
            encoded_run += chr(byte)

        if length % 2 != 0:
            # We have one pixel left, which fills half a byte.
            byte = row[offset + length - 1] << 4
            encoded_run += chr(byte)

        if len(encoded_run) % 2 != 0:
            # pad the encoded run out to the nearest word boundry
            encoded_run += '\x00'

        return encoded_run

    def create_encoded_run(self, run_length, pixel1, pixel2):
        "Return the 'encoded mode' encoding of a run of pixels"
        return chr(run_length) + chr((pixel1 << 4) | pixel2)

    def create_end_of_line(self):
        "Return the end-of-line escape sequence"
        return '\x00\x00'

    def create_end_of_bitmap(self):
        "Return the end-of-bitmap escape sequence"
        return '\x00\x01'

    def create_delta(self, down, right):
        "Return a delta escape sequence"
        return '\x00\x02' + chr(down) + chr(right)

    def create_pixeldata(self):
        raise "this should not be called"

    def create_raster(self):
        "Return a rasterized version of the 4bpp canonical image"

        # widths are in nibbles (pixels)
        red_width   = self.width / 3
        green_width = self.width / 3
        blue_width  = self.width - (red_width + green_width)

        # draw the pattern
        raster = []
        for i in range(0, self.height):

            row = []
            row += [self.INDEX_RED]   * red_width
            row += [self.INDEX_GREEN] * green_width
            row += [self.INDEX_BLUE]  * blue_width

            raster.append(row)

        # draw a border
        self.draw_double_border(raster, self.INDEX_BLACK, self.INDEX_WHITE)

        # add in the TOP_LEFT_LOGO
        self.apply_top_left_logo(raster, self.INDEX_BLACK, self.INDEX_WHITE)

        return raster


class bitmap_rle4_encoded(bitmap_rle4):
    """
    A simple run-length encoded bitmap that has 4 bits per pixel.
    The entire bitmap is in 'encoded mode'.
    """

    def create_pixeldata(self):
        "Return the pixel data as run-length encoded 4 bpp"

        raster = self.create_raster()

        pixeldata = ''
        run_length = 0
        prev_pixel = -1
        for row in range(0, len(raster)):
            for col in range(0, len(raster[row])):
                
                cur_pixel = raster[row][col]

                if run_length == 255 or (run_length != 0 and prev_pixel != cur_pixel):
                    # There's no more room on this run OR
                    # The current run has ended.

                    # Write the run and start a new one
                    pixeldata += self.create_encoded_run(run_length, prev_pixel, prev_pixel)

                    run_length = 0
                    prev_pixel = -1


                if run_length == 0:
                    # start a new run
                    prev_pixel = cur_pixel
                    run_length = 1

                elif prev_pixel == raster[row][col]:
                    # continue this run
                    run_length += 1

            # flush the last run
            if run_length != 0:
                pixeldata += self.create_encoded_run(run_length, prev_pixel, prev_pixel)

                run_length = 0
                prev_pixel = -1

            # end-of-line
            pixeldata += self.create_end_of_line()

        # end-of-bitmap
        pixeldata += self.create_end_of_bitmap()

        return pixeldata


class bitmap_rle4_absolute(bitmap_rle4):
    """
    A run-length encoded bitmap that has 4 bits per pixel.
    The entire bitmap is in 'absolute mode'.
    """

    def create_pixeldata(self):
        "Return the pixel data as RLE4 encoded in 'absolute mode' (uncompressed)"

        raster = self.create_raster()
        
        pixeldata = ''
        for row in range(0, len(raster)):
            
            cur_row = raster[row]

            col = 0
            while col < len(cur_row):
                remaining = len(cur_row) - col
                if 255 < remaining:
                    # There are more than 255 pixels left in this row.
                    # Encode all 255 pixels.
                    pixeldata += self.create_absolute_run(cur_row, col, 255)
                    col += 255

                elif 3 <= remaining:
                    # There are between 3 and 255 pixels left in this row.
                    # Encode them all with absolute encoding.
                    pixeldata += self.create_absolute_run(cur_row, col, remaining)
                    col += remaining

                else:
                    raise 'Unsupported width: %d' % remaining

            # end-of-line
            pixeldata += self.create_end_of_line()
              
        # end-of-bitmap
        pixeldata += self.create_end_of_bitmap()

        return pixeldata



class bitmap_rle4_alternate(bitmap_rle4):
    """
    A simple run-length encoded bitmap that has 4 bits per pixel.
    The entire bitmap is in 'encoded mode'.
    The runs encode alternating pixel colors, such as dithering would create.
    """

    def __init__(self, width, height):
        bitmap_rle4.__init__(self, width, height)

        self.palette = [
            0x00000000, # black
            0x00000000, # black
            0x00FF0000, # red
            0x0000FF00, # green
            0x000000FF, # blue
            0x00FF0000, # red
            0x00FFFFFF, # white
            0x00FFFFFF] # white

    def create_pixeldata(self):
        "Return the pixel data as run-length encoded 4 bpp"
        
        # widths are in nibbles (pixels)
        red_width   = self.width / 3
        green_width = self.width / 3
        blue_width  = self.width - (red_width + green_width)

        # draw the pattern
        raster = []
        for i in range(0, self.height):

            row = []
            row += [2] * red_width
            row += [3] * green_width
            row += [4] * blue_width

            raster.append(row)

        # draw a border
        self.draw_double_border(raster, 0, 6)

        # add in the TOP_LEFT_LOGO
        self.apply_top_left_logo(raster, 0, 6)

        pixeldata = ''
        run_length = 0
        prev_pixel = -1
        for row in range(0, len(raster)):
            for col in range(0, len(raster[row])):
                
                cur_pixel = raster[row][col]

                if run_length == 255 or (run_length != 0 and prev_pixel != cur_pixel):
                    # There's no more room on this run OR
                    # The current run has ended.

                    # Write the run and start a new one
                    pixeldata += self.create_encoded_run(
                        run_length,
                        prev_pixel,
                        prev_pixel + 1)

                    run_length = 0
                    prev_pixel = -1


                if run_length == 0:
                    # start a new run
                    prev_pixel = cur_pixel
                    run_length = 1

                elif prev_pixel == raster[row][col]:
                    # continue this run
                    run_length += 1

            # flush the last run
            if run_length != 0:
                pixeldata += self.create_encoded_run(
                    run_length,
                    prev_pixel,
                    prev_pixel + 1)

                run_length = 0
                prev_pixel = -1

            # end-of-line
            pixeldata += self.create_end_of_line()
              
        # end-of-bitmap
        pixeldata += self.create_end_of_bitmap()
                        
        return pixeldata


class bitmap_rle4_delta(bitmap_rle4):
    """
    A simple run-length encoded bitmap that has 4 bits per pixel.
    The bitmap uses 'delta escapes'.
    """

    def __init__(self, width, height):
        bitmap_rle4.__init__(self, width, height)

        # fill the rest of the palette with grey
        # so that we can see the difference between uninitialized
        # memory and pixels that should be unspecified.
        for i in range(len(self.palette), 16):
            self.palette.append(0x00CCCCCC)

    def create_raster(self):
        "Return a raster that is suitable for creating an RLE image with deltas"

        # widths are in bytes (pixels)
        red_width   = self.width / 3
        green_width = self.width / 3
        blue_width  = self.width - (red_width + green_width)

        # draw the pattern
        raster = []
        for i in range(0, self.height):

            row = []
            row += [self.INDEX_RED]         * red_width
            row += [self.TRANSPARENT_PIXEL] * green_width
            row += [self.INDEX_BLUE]        * blue_width

            raster.append(row)

        # draw an invisible border
        self.draw_double_border(
            raster,
            self.TRANSPARENT_PIXEL,
            self.TRANSPARENT_PIXEL)

        # add in the TOP_LEFT_LOGO
        self.apply_top_left_logo(
            raster,
            self.INDEX_BLACK,
            self.INDEX_WHITE)

        return raster


    def create_pixeldata(self):
        "Return the pixel data as run-length encoded 8 bpp"
        
        raster = self.create_raster()

        pixeldata = ''
        run_length = 0
        prev_pixel = -1
        for row in range(0, len(raster)):

            # check if the row contains nothing but transparent pixels
            row_is_all_transparent = 1
            for col in range(0, len(raster[row])):
                if raster[row][col] != self.TRANSPARENT_PIXEL:
                    row_is_all_transparent = 0
                    break

            if row_is_all_transparent:
                # the entire row is entirely transparent. Do a delta.
                pixeldata += self.create_delta(0, 1)

            else:
                # there are some non-transparent pixels in this row.
                for col in range(0, len(raster[row])):

                    cur_pixel = raster[row][col]

                    if run_length == 255 or (run_length != 0 and prev_pixel != cur_pixel):
                        # There's no more room on this run OR
                        # The current run has ended.

                        # Write the run and start a new one
                        if prev_pixel == self.TRANSPARENT_PIXEL:
                            # this run is encoded as a delta
                            pixeldata += self.create_delta(
                                run_length,
                                0)
                        else:
                            # this run is encoded as a regular run
                            pixeldata += self.create_encoded_run(
                                run_length,
                                prev_pixel,
                                prev_pixel)

                        run_length = 0
                        prev_pixel = -1


                    if run_length == 0:
                        # start a new run
                        prev_pixel = cur_pixel
                        run_length = 1

                    elif prev_pixel == raster[row][col]:
                        # continue this run
                        run_length += 1

                # flush the last run
                if run_length != 0:

                    # We don't have to write a delta for transparent
                    # pixels because the end-of-line marker will take
                    # care of that.
                    if prev_pixel != self.TRANSPARENT_PIXEL:
                        # this run is encoded as a regular run
                        pixeldata += self.create_encoded_run(
                            run_length,
                            prev_pixel,
                            prev_pixel)

                    run_length = 0
                    prev_pixel = -1

                # end-of-line
                pixeldata += self.create_end_of_line()
              
        # end-of-bitmap
        pixeldata += self.create_end_of_bitmap()
                        
        return pixeldata

class bitmap_rle4_topdown(bitmap_rle4_encoded):
    """
    An RLE4 compressed bitmap with a negative height.
    This is an illegal bitmap: top-down images cannot be compressed.
    Still, many bitmap processors can understand it.
    """

    def get_height(self):
        return -self.height


class bitmap_rle4_noendofline(bitmap_rle4_encoded):
    """
    An RLE4 compressed bitmap with no end-of-line sequences.
    It is unclear if a bitmap processor should implicitly add
    end-of-line markers when the pixel run the row's width,
    or if it should ignore all pixel data beyond the row's width.
    """

    def create_end_of_line(self):
        return ''

class bitmap_rle4_noendofbitmap(bitmap_rle4_encoded):
    """
    An RLE4 compressed bitmap with no end-of-bitmap sequence.
    This is techinically invalid, but the bitmap processor should
    be able to treat the end-of-file as an end-of-bitmap.
    """

    def create_end_of_bitmap(self):
        return ''


class bitmap_rle4_croppedrun(bitmap_rle4):
    """
    A simple run-length encoded bitmap that has 4 bits per pixel.
    The entire bitmap is in 'encoded mode'.
    The pixel data ends prematurely--in the middle of an encoded
    escape sequence.
    """

    def create_pixeldata(self):
        "Return the pixel data as run-length encoded 4 bpp"
        
        raster = self.create_raster()

        pixeldata = ''
        run_length = 0
        prev_pixel = -1
        for row in range(0, len(raster) / 2):
            for col in range(0, len(raster[row])):
                
                cur_pixel = raster[row][col]

                if run_length == 255 or (run_length != 0 and prev_pixel != cur_pixel):
                    # There's no more room on this run OR
                    # The current run has ended.

                    # Write the run and start a new one
                    pixeldata += self.create_encoded_run(run_length, prev_pixel, prev_pixel)

                    run_length = 0
                    prev_pixel = -1


                if run_length == 0:
                    # start a new run
                    prev_pixel = cur_pixel
                    run_length = 1

                elif prev_pixel == raster[row][col]:
                    # continue this run
                    run_length += 1

            # flush the last run
            if run_length != 0:
                pixeldata += self.create_encoded_run(run_length, prev_pixel, prev_pixel)

                run_length = 0
                prev_pixel = -1

            # end-of-line
            pixeldata += self.create_end_of_line()
              
        # append the cropped run
        last_run = self.create_encoded_run(100, 1, 2)
        cropped_run = last_run[0 : len(last_run) - 1]
        pixeldata += cropped_run
                        
        return pixeldata

class bitmap_rle4_croppedabsolute(bitmap_rle4):
    """
    A simple run-length encoded bitmap that has 4 bits per pixel.
    The entire bitmap is in 'absolute mode'.
    The pixel data ends prematurely--in the middle of an absolute
    escape sequence.
    """

    def create_pixeldata(self):
        "Return the pixel data as RLE4 encoded in 'absolute mode' (uncompressed)"
        raster = self.create_raster()
        
        pixeldata = ''
        for row in range(0, len(raster)):
            
            cur_row = raster[row]

            col = 0
            while col < len(cur_row):
                remaining = len(cur_row) - col
                if 255 < remaining:
                    # There are more than 255 pixels left in this row.
                    # Encode all 255 pixels.
                    absolute_run = self.create_absolute_run(cur_row, col, 255)
                    col += 255

                elif 3 <= remaining:
                    # There are between 3 and 255 pixels left in this row.
                    # Encode them all with absolute encoding.
                    absolute_run = self.create_absolute_run(cur_row, col, remaining)
                    col += remaining

                else:
                    raise 'Unsupported width: %d' % remaining

                if len(raster) / 2 <= row:
                    # crop this encoding and return
                    cropped_run = absolute_run[0 : len(absolute_run) - 1]
                    pixeldata += cropped_run
                    return pixeldata

                # append this encoding and keep processing the current row
                pixeldata += absolute_run

            # end-of-line
            pixeldata += self.create_end_of_line()

        return pixeldata

class bitmap_rle4_croppeddelta(bitmap_rle4_delta):
    """
    A simple run-length encoded bitmap that has 4 bits per pixel.
    The bitmap uses 'delta escapes'.
    The file ends in the middle of a delta escape.
    """

    def create_pixeldata(self):
        """
        Return the pixel data as run-length encoded 4 bpp that ends in the
        middle of a delta escape sequence
        """

        raster = self.create_raster()

        pixeldata = ''
        run_length = 0
        prev_pixel = -1
        for row in range(0, len(raster)):

            # check if the row contains nothing but transparent pixels
            row_is_all_transparent = 1
            for col in range(0, len(raster[row])):
                if raster[row][col] != self.TRANSPARENT_PIXEL:
                    row_is_all_transparent = 0
                    break

            if row_is_all_transparent:
                # the entire row is entirely transparent. Do a delta.
                pixeldata += self.create_delta(0, 1)

            else:
                # there are some non-transparent pixels in this row.
                for col in range(0, len(raster[row])):

                    cur_pixel = raster[row][col]

                    if run_length == 255 or (run_length != 0 and prev_pixel != cur_pixel):
                        # There's no more room on this run OR
                        # The current run has ended.

                        # Write the run and start a new one
                        if prev_pixel == self.TRANSPARENT_PIXEL:
                            # this run is encoded as a delta
                            delta = self.create_delta(
                                run_length,
                                0)

                            if len(raster) / 2 <= row:
                                # we are half-way through the image.
                                # crop the delta and return what we have
                                cropped_delta = delta[0 : len(delta) - 1]
                                pixeldata += cropped_delta
                                return pixeldata
                                
                            pixeldata += delta

                        else:
                            # this run is encoded as a regular run
                            pixeldata += self.create_encoded_run(
                                run_length,
                                prev_pixel,
                                prev_pixel)

                        run_length = 0
                        prev_pixel = -1


                    if run_length == 0:
                        # start a new run
                        prev_pixel = cur_pixel
                        run_length = 1

                    elif prev_pixel == raster[row][col]:
                        # continue this run
                        run_length += 1

                # flush the last run
                if run_length != 0:

                    # We don't have to write a delta for transparent
                    # pixels because the end-of-line marker will take
                    # care of that.
                    if prev_pixel != self.TRANSPARENT_PIXEL:
                        # this run is encoded as a regular run
                        pixeldata += self.create_encoded_run(
                            run_length,
                            prev_pixel,
                            prev_pixel)

                    run_length = 0
                    prev_pixel = -1

                # end-of-line
                pixeldata += self.create_end_of_line()
              
        # end-of-bitmap
        pixeldata += self.create_end_of_bitmap()
                        
        return pixeldata


class bitmap_4bpp_nopalette(bitmap_4bpp):
    """
    A bitmap that has 4 bit per pixel and no palette.
    This is an invalid bitmap, but a bitmap processor could use default colors.
    """

    def __init__(self, width, height):
        bitmap_4bpp.__init__(self, width, height)

        # empty the palette
        self.palette = []


class bitmap_1bpp(bitmap):
    "A bitmap that has 1 bit per pixel."

    def __init__(self, width, height):
        bitmap.__init__(self, 1, width, height)

        self.palette = [
            0x00000000,  # black
            0x00FFFFFF]  # white

    def create_pixeldata(self):
        "Return the pixel data as uncompressed 1 bpp."
        
        # widths are in bits (pixels)
        stripe1_width = self.width // 3
        stripe2_width = self.width // 3
        stripe3_width = self.width - (stripe1_width + stripe2_width)

        pad_width_in_bits = self.get_scanline_padding_bits()

        # draw the pattern
        raster = []
        for i in range(0, self.height):

            row = []
            row += [0] * stripe1_width
            row += [1] * stripe2_width
            row += [0] * stripe3_width
            row += [1] * pad_width_in_bits

            raster.append(row)

        # draw a border
        self.draw_double_border(raster, 0, 1)

        # add in the TOP_LEFT_LOGO
        self.apply_top_left_logo(raster, 0, 1)


        pixeldata = b''
        for row in range(0, len(raster)):

            # a closure for helping to add runs to the image
            class bitstream:
                def __init__(self):
                    self.nextByte = 0
                    self.index    = 0
                    self.scanline = b''

                def appendbit(self, value):
                    # shift the byte over and OR-in the low bit
                    self.nextByte = (self.nextByte << 1) | value
                    self.index += 1

                    if (self.index == 8):
                        # this byte is full--write it
                        self.scanline += chr(self.nextByte)
                        self.nextByte = 0
                        self.index    = 0

            appender = bitstream()
            for col in range(0, len(raster[row])):
                appender.appendbit(raster[row][col])
            pixeldata += appender.scanline

        return pixeldata

class bitmap_1bpp_topdown(bitmap_1bpp):
    "A 'top down' uncompressed RGB bitmap that has 1 bit per pixel."

    def get_height(self):
        # a negative value makes a bitmap top-down
        return -self.height


class bitmap_1bpp_color(bitmap_1bpp):
    "A bitmap that has 1 bit per pixel and a color palette."

    def __init__(self, width, height):
        bitmap_1bpp.__init__(self, width, height)

        self.palette = [
            0x00FFF000,  # yellow
            0x00000FFF]  # mostly blue

class bitmap_1bpp_overlappingcolor(bitmap_1bpp):
    "A bitmap that has 1 bit per pixel and a color palette with colors that overlap."

    def __init__(self, width, height):
        bitmap_1bpp.__init__(self, width, height)

        self.palette = [
            0x00FFFF00,  # yellow
            0x0000FFFF]  # cyan

class bitmap_1bpp_nopalette(bitmap_1bpp):
    """
    A bitmap that has 1 bit per pixel and no palette.
    This is technically invalid, but a bitmap processor could default to black and white.
    """

    def __init__(self, width, height):
        bitmap_1bpp.__init__(self, width, height)

        # empty the palette
        self.palette = []


class bitmap_1bpp_palettetoobig(bitmap_1bpp):
    "A bitmap that has 1 bit per pixel that has a palette with 5000 colors."

    def __init__(self, width, height):
        bitmap_1bpp.__init__(self, width, height)

        for i in range(0,5000):
            self.palette.append(i)

class bitmap_1bpp_croppedpixeldata(bitmap_1bpp):
    """
    A 1 bpp bitmap that ends in the middle of the pixel data.
    This tests that what happens when a call to fread() fails.
    This bitmap processor should probably process the data that
    it does have AND display a diagnostic.
    """

    def write(self, filename):
        self.write_croppped_bitmap(filename)


class bitmap_width_height_overflow(bitmap_565):
    """
    A bitmap whose reported width and height cause a 32bit overflow when
    they are multiplied together.
    This tries to trick the image processor into allocating a very small
    buffer that it thinks is very large.
    """

    def get_width(self):
        return 0x10000

    def get_height(self):
        return 0x10000

class bitmap_emptyfile(bitmap_1bpp):
    """
    A zero-byte file.
    This tests that what happens when the first call to fread() fails.
    """

    def write(self, filename):

        _safe_unlink(filename)
        
        bmpfile = open(filename, 'wb')
        bmpfile.close()
	
class bitmap_badmagicnumber(bitmap_1bpp):
    """
    A bitmap with an invalid magic number (it uses 'Bm' instead of 'BM')
    A bitmap processor that ignores this field is probably trusting the file extension
    or doing a case-insensitive compare.
    """

    def get_magic_number(self):
        "return the bad magic number"
        return 'Bm'


class bitmap_croppedmagicnumber(bitmap_1bpp):
    """
    A one byte bitmap that only contains the 'B' of the magic number.
    This tests that what happens when the first call to fread() returns
    fewer bytes than expected.
    """

    def write(self, filename):

        _safe_unlink(filename)
        
        bmpfile = open(filename, 'wb')
        bmpfile.write('B')
        bmpfile.close()

class bitmap_badfilesize(bitmap_1bpp):
    """
    A bitmap with a filesize that's half of what it should be.
    Most bitmap processors ignore this field.
    """

    def get_filesize(self):
        "Return a filesize that is half of what it should be"
        return bitmap_1bpp.get_filesize(self) / 2


class bitmap_zerofilesize(bitmap_1bpp):
    """
    A bitmap with an filesize of 0.
    Most bitmap processors ignore this field.
    """

    def get_filesize(self):
        return 0

class bitmap_badreserved1(bitmap_1bpp):
    """
    A bitmap with an 'wReserved1' field that is not 0.
    This is technically illegal, but most bitmap processors ignore this field.
    """

    def get_reserved1(self):
        return 1

class bitmap_badreserved2(bitmap_1bpp):
    """
    A bitmap with an 'wReserved2' field that is not 0.
    This is technically illegal, but most bitmap processors ignore this field.
    """

    def get_reserved2(self): 
        return 1

class bitmap_negativeoffbits(bitmap_1bpp):
    """
    A bitmap with an 'dwOffBits' field that is -1.
    This is supposed to be interpreted as an unsigned value, so it will
    either be understood as a very large (illegal) value,
    or a negative value (also illegal).
    """

    def get_offset_of_bitmap_data(self):
        return -1

class bitmap_largeoffbits(bitmap_1bpp):
    """
    A bitmap with an 'dwOffBits' field that is larger than the file size.
    """

    def get_offset_of_bitmap_data(self):
        return self.get_filesize() + 1

        
class bitmap_zerooffbits(bitmap_1bpp):
    """
    A bitmap with an 'dwOffBits' field that is 0.
    A bitmap processor may recover from this by assuming that the pixel data
    immediately follows the palette.
    """

    def get_offset_of_bitmap_data(self):
        return 0

class bitmap_croppedfileinfoheader(bitmap_1bpp):
    """
    A bitmap that is one byte short of having a complete fileinfoheader.
    This tests that what happens when fread() returns fewer bytes
    than expected.
    """

    def write(self, filename):

        _safe_unlink(filename)
        
        bmpfile = open(filename, 'wb')

        fileinfoheader = self.get_fileinfoheader()

        # remove the last byte of the FILEINFOHEADER
        content = fileinfoheader[0 : len(fileinfoheader) - 1]
        bmpfile.write(content)

        # don't write anything else.
        bmpfile.close()

class bitmap_missinginfoheader(bitmap_1bpp):
    """
    A bitmap file that is so short that it doesn't include a BITMAPINFOHEADER.
    This tests that what happens when a call to fread() fails.
    """

    def write(self, filename):

        _safe_unlink(filename)
        
        bmpfile = open(filename, 'wb')

        fileinfoheader = self.get_fileinfoheader()
        bmpfile.write(fileinfoheader)

        # don't write any other part of the bitmap
        bmpfile.close()

class bitmap_croppedinfoheader(bitmap_1bpp):
    """
    A bitmap that is one byte short of having a complete BITMAPINFOHEADER.
    This tests that what happens when fread() returns fewer bytes
    than expected.
    """

    def write(self, filename):

        _safe_unlink(filename)
        
        bmpfile = open(filename, 'wb')

        fileinfoheader = self.get_fileinfoheader()
        bmpfile.write(fileinfoheader)

        bmpinfoheader = self.get_bitmapinfoheader()

        # remove the last byte of the BITMAPINFOHEADER
        content = bmpinfoheader[0 : len(bmpinfoheader) - 1]
        bmpfile.write(content)

        # don't write any other part of the bitmap
        bmpfile.close()

class bitmap_smallbmpinfoheadersize(bitmap_1bpp):
    """
    A bitmap with a 'biSize' field in its BMPINFOHEADER that is too small.
    """

    def get_bitmap_info_header_size(self):
        return 24

    def get_bitmapinfoheader(self):
        "Return the short BITMAPINFOHEADER"

        size               = self.get_bitmap_info_header_size()
        width              = self.get_width()
        height             = self.get_height()
        planes             = self.get_planes()
        bits_per_pixel     = self.get_bits_per_pixel()
        compression        = self.get_compression()
        image_size         = self.get_image_size()

        bitmapinfoheader = struct.pack(
            '<IiihhII',
            size,
            width,
            height,
            planes,
            bits_per_pixel,
            compression,
            image_size)

        return bitmapinfoheader

class bitmap_largebmpinfoheadersize(bitmap_1bpp):
    """
    A bitmap with a 'biSize' field in its BMPINFOHEADER that is too large.
    """

    def get_bitmap_info_header_size(self):
        return 64 * 1024 * 1024

class bitmap_zerobmpinfoheadersize(bitmap_1bpp):
    """
    A bitmap with a 'biSize' field in its BMPINFOHEADER that is 0.
    """

    def get_bitmap_info_header_size(self):
        return 0


class bitmap_zeroheight(bitmap_1bpp):
    """
    A bitmap with a 'biHeight' field in its BMPINFOHEADER that is 0.
    """

    def get_height(self):
        return 0

class bitmap_zerowidth(bitmap_1bpp):
    """
    A bitmap with a 'biWidth' field in its BMPINFOHEADER that is 0.
    """

    def get_width(self):
        return 0

class bitmap_negativewidth(bitmap_1bpp):
    """
    A bitmap with a negative 'biWidth' field in its BMPINFOHEADER.
    """

    def get_width(self):
        return -self.width

class bitmap_zeroplanes(bitmap_1bpp):
    """
    A bitmap with a 'biPlanes' field in its BMPINFOHEADER that is zero.
    This is an invalid bitmap, but many bitmap processors ignore this field.
    """

    def get_planes(self):
        return 0

class bitmap_largeplanes(bitmap_1bpp):
    """
    A bitmap with a 'biPlanes' field in its BMPINFOHEADER that is large.
    This is an invalid bitmap, but many bitmap processors ignore this field.
    """

    def get_planes(self):
        return 5000


class bitmap_oddbitdepth(bitmap_8bpp):
    """
    A bitmap with a 'biBitCount' field in its BMPINFOHEADER that is odd.
    """

    def get_bits_per_pixel(self):
        return 7

class bitmap_zerobitdepth(bitmap_1bpp):
    """
    A bitmap with a 'biBitCount' field in its BMPINFOHEADER that is 0.
    """

    def get_bits_per_pixel(self):
        return 0

class bitmap_largebitdepth(bitmap_1bpp):
    """
    A bitmap with a 'biBitCount' field in its BMPINFOHEADER that is very large.
    This attempts to trick the bitmap processor into thinking the bit depth is negative.
    """

    def get_bits_per_pixel(self):
        return 0xFFFF

class bitmap_unknowncompression(bitmap_1bpp):
    """
    A bitmap with an unrecognized 'biCompression' field.
    """

    def get_compression(self):
        return 0xFFFFFFFF

class bitmap_4bpp_rle8compression(bitmap_rle4_encoded):
    """
    A 4 bpp bitmap with a 'biCompression' field of BI_RLE8.
    Only 8 bpp bitmaps may use BI_RLE8.
    """

    def get_compression(self):
        return self.BI_RLE8


class bitmap_8bpp_rle4compression(bitmap_rle8_encoded):
    """
    An 8 bpp bitmap with a 'biCompression' field of BI_RLE4.
    Only 4 bpp bitmaps may use BI_RLE4.
    """

    def get_compression(self):
        return self.BI_RLE4

class bitmap_8bpp_zeroxpelspermeter(bitmap_8bpp):
    """
    An 8 bpp bitmap with a 'biXPelsPerMeter' field of 0.
    Most bitmap processors ignore this field, but it is most likely used when
    printing the image.
    """

    def get_pixels_per_meter_x(self):
        "Return the biXPelsPerMeter to put into the BITMAPINFOHEADER"
        return 0

class bitmap_8bpp_negativexpelspermeter(bitmap_8bpp):
    """
    An 8 bpp bitmap with a negative 'biXPelsPerMeter' field.
    Most bitmap processors ignore this field, but it is most likely used when
    printing the image.
    """

    def get_pixels_per_meter_x(self):
        "Return the biXPelsPerMeter to put into the BITMAPINFOHEADER"
        return -10000

class bitmap_8bpp_largexpelspermeter(bitmap_8bpp):
    """
    An 8 bpp bitmap with a large 'biXPelsPerMeter' field.
    Most bitmap processors ignore this field, but it is most likely used when
    printing the image.
    """

    def get_pixels_per_meter_x(self):
        "Return the biXPelsPerMeter to put into the BITMAPINFOHEADER"
        return 0x7FFFFFFF

class bitmap_8bpp_zeroypelspermeter(bitmap_8bpp):
    """
    An 8 bpp bitmap with a 'biYPelsPerMeter' field of 0.
    Most bitmap processors ignore this field, but it is most likely used when
    printing the image.
    """

    def get_pixels_per_meter_y(self):
        "Return the biYPelsPerMeter to put into the BITMAPINFOHEADER"
        return 0

class bitmap_8bpp_negativeypelspermeter(bitmap_8bpp):
    """
    An 8 bpp bitmap with a negative 'biYPelsPerMeter' field.
    Most bitmap processors ignore this field, but it is most likely used when
    printing the image.
    """

    def get_pixels_per_meter_y(self):
        "Return the biYPelsPerMeter to put into the BITMAPINFOHEADER"
        return -10000

class bitmap_8bpp_largeypelspermeter(bitmap_8bpp):
    """
    An 8 bpp bitmap with a large 'biYPelsPerMeter' field.
    Most bitmap processors ignore this field, but it is most likely used when
    printing the image.
    """

    def get_pixels_per_meter_y(self):
        "Return the biYPelsPerMeter to put into the BITMAPINFOHEADER"
        return 0x7FFFFFF



class bitmap_missingpalette(bitmap_1bpp):
    """
    A bitmap file that is so short that it doesn't include the palette.
    This tests that what happens when a call to fread() fails.
    """

    def write(self, filename):

        _safe_unlink(filename)
        
        bmpfile = open(filename, 'wb')

        fileinfoheader = self.get_fileinfoheader()
        bmpfile.write(fileinfoheader)

        bmpinfoheader = self.get_bitmapinfoheader()
        bmpfile.write(bmpinfoheader)

        # don't write any other part of the bitmap
        bmpfile.close()

class bitmap_croppedpalette(bitmap_1bpp):
    """
    A bitmap that is one byte short of having a complete palette.
    This tests that what happens when fread() returns fewer bytes
    than expected.
    """

    def write(self, filename):

        _safe_unlink(filename)
        
        bmpfile = open(filename, 'wb')

        fileinfoheader = self.get_fileinfoheader()
        bmpfile.write(fileinfoheader)

        bmpinfoheader = self.get_bitmapinfoheader()
        bmpfile.write(bmpinfoheader)

        palette = self.get_palette()

        # remove the last byte of the BITMAPINFOHEADER
        content = palette[0 : len(palette) - 1]
        bmpfile.write(content)

        # don't write any other part of the bitmap
        bmpfile.close()

class bitmap_missingcolormasks(bitmap_565):
    """
    A BI_BITFIELDS bitmap file that is so short that it doesn't include the
    colormask array.
    This tests that what happens when a call to fread() fails.
    """

    def write(self, filename):

        _safe_unlink(filename)
        
        bmpfile = open(filename, 'wb')

        fileinfoheader = self.get_fileinfoheader()
        bmpfile.write(fileinfoheader)

        bmpinfoheader = self.get_bitmapinfoheader()
        bmpfile.write(bmpinfoheader)

        # don't write any other part of the bitmap
        bmpfile.close()

class bitmap_croppedcolormasks(bitmap_565):
    """
    A BI_BITFIELDS bitmap that is one byte short of having a complete
    colormask array.
    This tests that what happens when fread() returns fewer bytes
    than expected.
    """

    def write(self, filename):

        _safe_unlink(filename)
        
        bmpfile = open(filename, 'wb')

        fileinfoheader = self.get_fileinfoheader()
        bmpfile.write(fileinfoheader)

        bmpinfoheader = self.get_bitmapinfoheader()
        bmpfile.write(bmpinfoheader)

        palette = self.get_palette()

        # remove the last byte of the BITMAPINFOHEADER
        content = palette[0 : len(palette) - 1]
        bmpfile.write(content)

        # don't write any other part of the bitmap
        bmpfile.close()


class bitmap_missingpixeldata(bitmap_24bpp):
    """
    A bitmap file that doesn't include any of the pixel data.
    """

    def write(self, filename):

        _safe_unlink(filename)
        
        bmpfile = open(filename, 'wb')

        fileinfoheader = self.get_fileinfoheader()
        bmpfile.write(fileinfoheader)

        bmpinfoheader = self.get_bitmapinfoheader()
        bmpfile.write(bmpinfoheader)

        palette = self.get_palette()
        bmpfile.write(palette)

        # don't write any other part of the bitmap
        bmpfile.close()

class bitmap_toomuchdata(bitmap_1bpp):
    """
    A bitmap with twice as much payload as expected.
    This attempts to overflow an internal buffer.
    """

    def create_pixeldata(self):
        return bitmap_1bpp.create_pixeldata(self) * 2


class testcase_logger:
    def __init__(self, path):
        # a map from filename to English description of the file.
        self.descriptions = {}

        # the file path to the directory containing the bitmap
        self.path         = path

    def write_index(self, basename):
        keys = self.descriptions.keys()
        keys.sort()

        index_filename = self.path + '/' + basename
        _safe_unlink(index_filename)
        indexfile = open(index_filename, 'wb')

        indexfile.write('<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">\n')
        indexfile.write('<html>\n')
        indexfile.write('  <head>\n')
        indexfile.write('    <title>Directory Listing</title>\n')
        indexfile.write('  </head>\n')
        indexfile.write('  <body>\n')
        indexfile.write('  <table border="1">\n')
        indexfile.write('\n')
        indexfile.write('    <tr>\n')
        indexfile.write('      <th>Filename</th>\n')
        indexfile.write('      <th>Description</th>\n')
        indexfile.write('    </tr>\n')
        indexfile.write('\n')

        for key in keys:
            indexfile.write('    <tr>\n')
            indexfile.write('      <td><a href="' + key + '">' + key + '</a></td>\n')
            indexfile.write('      <td>' + self.descriptions[key] + '</td>\n')
            indexfile.write('    </tr>\n')
            indexfile.write('\n')

        indexfile.write('    </table>\n')
        indexfile.write('  </body>\n')
        indexfile.write('</html>\n')

        indexfile.close()


    def do_testcase(self, filename, bitmap, *rest):
        bitmap.write(self.path + '/' + filename)

        # Figure out the bitmap's English description.
        # Start with the doc string and append the optional
        # complementary description argument.
        description = bitmap.__doc__
        if len(rest) != 0:
            description += ' ' + rest[0]

        self.descriptions[filename] = description

def generate_valid_bitmaps():
    """
    Create valid bitmaps of various encodings.
    """

    _safe_create_dir('bitmaps')
    _safe_create_dir('bitmaps/valid')

    log = testcase_logger('bitmaps/valid')

    # valid 1 bpp bitmaps
    for width in range(320,336):
        log.do_testcase(
            '1bpp-%ix240.bmp' % width,
            bitmap_1bpp(width, 240),
            'Each scanline has %i bits of padding.' % (width - 320))

    log.do_testcase(
      '1bpp-1x1.bmp',
      bitmap_1bpp(1, 1),
      'The image is a single black pixel.')

    log.do_testcase(
        '1bpp-320x240-color.bmp',
        bitmap_1bpp_color(320, 240))

    log.do_testcase(
        '1bpp-320x240-overlappingcolor.bmp',
        bitmap_1bpp_overlappingcolor(320, 240))

    log.do_testcase(
        '1bpp-topdown-320x240.bmp',
        bitmap_1bpp_topdown(320, 240))

    # valid 4 bpp bitmaps
    for width in range(320,328):
        log.do_testcase(
            '4bpp-%ix240.bmp' % width,
            bitmap_4bpp(width, 240),
            'Each scanline has %i nibbles of padding.' % (width - 320))

    for width in range (320, 321):
        log.do_testcase(
            'rle4-encoded-%ix240.bmp' % width,
            bitmap_rle4_encoded(width, 240))

    for width in range (320, 321):
        log.do_testcase(
            'rle4-absolute-%ix240.bmp' % width,
            bitmap_rle4_absolute(width, 240))

    for width in range (320, 321):
        log.do_testcase(
            'rle4-alternate-%ix240.bmp' % width,
            bitmap_rle4_alternate(width, 240))

    log.do_testcase(
        'rle4-delta-320x240.bmp',
        bitmap_rle4_delta(320, 240))

    log.do_testcase(
        '4bpp-1x1.bmp',
        bitmap_4bpp(1, 1),
        'The image is a single blue pixel.')

    log.do_testcase(
        '4bpp-topdown-320x240.bmp',
        bitmap_4bpp_topdown(320, 240))


    # valid 8 bpp bitmaps
    for width in range(320,324):
        log.do_testcase(
            '8bpp-%ix240.bmp' % width,
            bitmap_8bpp(width, 240),
            'Each scanline has %i bytes of padding.' % (width - 320))

    for width in range (320, 321):
        log.do_testcase(
            'rle8-encoded-%ix240.bmp' % width,
            bitmap_rle8_encoded(width, 240))

    for width in range (320, 321):
        log.do_testcase(
            'rle8-absolute-%ix240.bmp' % width,
            bitmap_rle8_absolute(width, 240))

    log.do_testcase(
        'rle8-delta-320x240.bmp',
         bitmap_rle8_delta(320, 240))

    log.do_testcase(
        'rle8-blank-160x120.bmp',
        bitmap_rle8_blank(160, 120))

    log.do_testcase(
        '8bpp-1x1.bmp',
        bitmap_8bpp(1, 1),
        'The image is a single blue pixel.')

    log.do_testcase(
        'rle8-64000x1.bmp',
        bitmap_rle8_encoded(64000, 1),
        'The image is a very long red, green, and blue horizontal line.')

    log.do_testcase(
        '8bpp-1x64000.bmp',
        bitmap_8bpp(1, 64000),
        'The image is a very long, blue vertical line.')

    log.do_testcase(
        '8bpp-colorsused-zero.bmp',
        bitmap_8bpp_zerocolorsused(320, 240))

    log.do_testcase(
        '8bpp-colorsimportant-two.bmp',
        bitmap_8bpp_twocolorsimportant(320, 240))

    log.do_testcase(
        '8bpp-topdown-320x240.bmp',
        bitmap_8bpp_topdown(320, 240))

    # valid 5-5-5 bitmaps
    for width in range(320,322):
        log.do_testcase(
            '555-%ix240.bmp' % width,
            bitmap_555(width, 240),
            'Each scanline has %i bytes of padding.' % ((width - 320) / 2))

    log.do_testcase(
        '555-1x1.bmp',
        bitmap_555(1, 1),
        'The image is a single blue pixel.')

    # valid 5-6-5 bitmaps
    for width in range(320,323):
        log.do_testcase(
            '565-%ix240.bmp' % width,
            bitmap_565(width, 240),
            'Each scanline has %i bytes of padding.' % ((width - 320) / 2))

        log.do_testcase(
            '565-%ix240-topdown.bmp' % width,
            bitmap_565_topdown(width, 240),
            'Each scanline has %i bytes of padding.' % ((width - 320) / 2))

    log.do_testcase(
        '565-1x1.bmp',
        bitmap_565(1, 1),
        'The image is a single blue pixel.')


    # valid 24 bpp bitmaps
    for width in range(320,324):
        log.do_testcase(
            '24bpp-%ix240.bmp' % width,
            bitmap_24bpp(width, 240),
            'Each scanline has %i bytes of padding.' % ((width - 320) / 3))

    log.do_testcase(
        '24bpp-topdown-320x240.bmp',
        bitmap_24bpp_topdown(320, 240))

    log.do_testcase(
        '24bpp-1x1.bmp',
        bitmap_24bpp(1, 1),
        'The image is a single blue pixel.')

    log.do_testcase(
        '24bpp-imagesize-zero.bmp',
        bitmap_24bpp_zeroimagesize(320, 240))

    # valid 32 bpp bitmaps
    log.do_testcase(
        '32bpp-320x240.bmp',
        bitmap_32bpp(320, 240))

    log.do_testcase(
        '32bpp-topdown-320x240.bmp',
        bitmap_32bpp_topdown(320, 240))

    log.do_testcase(
        '32bpp-optimalpalette-320x240.bmp',
        bitmap_32bpp_colortable(320, 240))

    log.do_testcase(
        '32bpp-888-optimalpalette-320x240.bmp',
        bitmap_32bpp_888_colortable(320, 240))

    log.do_testcase(
        '32bpp-101110-320x240.bmp',
        bitmap_32bpp_101110(320, 240))

    log.do_testcase(
        '32bpp-1x1.bmp',
        bitmap_32bpp(1, 1),
        'The image is a single blue pixel.')

    # write out the HTML index
    log.write_index('index.html')


def generate_questionable_bitmaps():
    """
    Create bitmaps that are technically invalid, but which most
    bitmap processors can process, anyway.
    """

    _safe_create_dir('bitmaps')
    _safe_create_dir('bitmaps/questionable')

    log = testcase_logger('bitmaps/questionable')

    log.do_testcase(
        'filesize-bad.bmp',
        bitmap_badfilesize(320, 240))

    log.do_testcase(
        'filesize-zero.bmp',
        bitmap_zerofilesize(320, 240))

    log.do_testcase(
        'reserved1-bad.bmp',
        bitmap_badreserved1(320, 240))

    log.do_testcase(
        'reserved2-bad.bmp',
        bitmap_badreserved2(320, 240))

    log.do_testcase(
        'rle8-height-negative.bmp',
        bitmap_rle8_topdown(320, 240))

    log.do_testcase(
        'rle4-height-negative.bmp',
        bitmap_rle4_topdown(320, 240))

    log.do_testcase(
        'planes-zero.bmp',
        bitmap_zeroplanes(320, 240))

    log.do_testcase(
        'planes-large.bmp',
        bitmap_largeplanes(320, 240))
    
    log.do_testcase(
        'pels-per-meter-x-zero.bmp',
        bitmap_8bpp_zeroxpelspermeter(320, 240))

    log.do_testcase(
        'pels-per-meter-x-negative.bmp',
        bitmap_8bpp_negativexpelspermeter(320, 240))

    log.do_testcase(
        'pels-per-meter-x-large.bmp',
        bitmap_8bpp_largexpelspermeter(320, 240))

    log.do_testcase(
        'pels-per-meter-y-zero.bmp',
        bitmap_8bpp_zeroypelspermeter(320, 240))

    log.do_testcase(
        'pels-per-meter-y-negative.bmp',
        bitmap_8bpp_negativeypelspermeter(320, 240))

    log.do_testcase(
        'pels-per-meter-y-large.bmp',
        bitmap_8bpp_largeypelspermeter(320, 240))

    log.do_testcase(
        'pixeldata-toomuch.bmp',
        bitmap_toomuchdata(320, 240))

    log.do_testcase(
        'pixeldata-rle8-toomuch.bmp',
        bitmap_rle8_toomuchdata(320, 240))

    log.do_testcase(
        'rle8-no-end-of-bitmap-marker.bmp',
        bitmap_rle8_noendofbitmap(320, 240))

    log.do_testcase(
        '8bpp-pixels-not-in-palette.bmp',
        bitmap_8bpp_pixelnotinpalette(254, 128))

    log.do_testcase(
        '32bpp-0x0.bmp',
        bitmap_32bpp(0, 0),
        'The image is 0 pixels wide and 0 pixels high.  Even though this is technically valid, most bitmap procesors consider it to be corrupt.')

    log.do_testcase(
        '32bpp-320x0.bmp',
        bitmap_32bpp(320, 0),
        'The image is 320 pixels wide and zero pixels high.  This is a sneaky way of making a 0x0 bitmap.  Even though this is technically valid, most bitmap procesors consider it to be corrupt.')

    log.do_testcase(
        '32bpp-0x240.bmp',
        bitmap_32bpp(0, 240),
        'The image is 0 pixels wide and 240 pixels high.  This is a sneaky way of making a 0x0 bitmap.  Even though this is technically valid, most bitmap procesors consider it to be corrupt.')

    log.do_testcase(
        'rle4-no-end-of-bitmap-marker.bmp',
        bitmap_rle4_noendofbitmap(320, 240))

    # write out the HTML index
    log.write_index('index.html')



def generate_corrupt_bitmaps():
    """
    Create corrupt/malicious bitmaps that are either impossible to process,
    or that most bitmap processors will refuse to process.
    For most of these files, the best a bitmap processor can do is
    display an informative diagnostic and not crash or leak memory.
    """

    _safe_create_dir('bitmaps')
    _safe_create_dir('bitmaps/corrupt')

    log = testcase_logger('bitmaps/corrupt')

    # invalid images
    log.do_testcase(
        'emptyfile.bmp',
        bitmap_emptyfile(320, 240))

    log.do_testcase(
        'fileinfoheader-cropped.bmp',
        bitmap_croppedfileinfoheader(320, 240))

    log.do_testcase(
        'magicnumber-cropped.bmp',
        bitmap_croppedmagicnumber(320, 240))
    
    log.do_testcase(
        'magicnumber-bad.bmp',
        bitmap_badmagicnumber(320, 240))

    log.do_testcase(
        'offbits-zero.bmp',
        bitmap_zerooffbits(320, 240))

    log.do_testcase(
        'offbits-negative.bmp',
        bitmap_negativeoffbits(320, 240))

    log.do_testcase(
        'offbits-large.bmp',
        bitmap_largeoffbits(320, 240))

    log.do_testcase(
        'infoheader-missing.bmp',
        bitmap_missinginfoheader(320, 240))

    log.do_testcase(
        'infoheader-cropped.bmp',
        bitmap_croppedinfoheader(320, 240))

    log.do_testcase(
        'infoheadersize-small.bmp',
        bitmap_smallbmpinfoheadersize(320, 240))

    log.do_testcase(
        'infoheadersize-large.bmp',
        bitmap_largebmpinfoheadersize(320, 240))

    log.do_testcase(
        'infoheadersize-zero.bmp',
        bitmap_zerobmpinfoheadersize(320, 240))

    log.do_testcase(
        'height-zero.bmp',
        bitmap_zeroheight(320, 240))

    log.do_testcase(
        'width-zero.bmp',
        bitmap_zerowidth(320, 240))

    log.do_testcase(
        'width-negative.bmp',
        bitmap_negativewidth(320, 240))

    log.do_testcase(
        'width-times-height-overflow.bmp',
        bitmap_width_height_overflow(320, 240))

    log.do_testcase(
        'bitdepth-zero.bmp',
        bitmap_zerobitdepth(320, 240))

    log.do_testcase(
        'bitdepth-odd.bmp',
        bitmap_oddbitdepth(320, 240))

    log.do_testcase(
        'bitdepth-large.bmp',
        bitmap_largebitdepth(320, 240))

    log.do_testcase(
        'compression-unknown.bmp',
        bitmap_unknowncompression(320, 240))

    log.do_testcase(
        'compression-bad-rle4-for-8bpp.bmp',
        bitmap_4bpp_rle8compression(320, 240))
    
    log.do_testcase(
        'compression-bad-rle8-for-4bpp.bmp',
        bitmap_8bpp_rle4compression(320, 240))

    log.do_testcase(
        '8bpp-colorsused-large.bmp',
        bitmap_8bpp_largecolorsused(320, 240))

    log.do_testcase(
        '8bpp-colorsused-negative.bmp',
        bitmap_8bpp_negativecolorsused(320, 240))

    log.do_testcase(
        '8bpp-colorsimportant-large.bmp',
        bitmap_8bpp_largecolorsimportant(320, 240))

    log.do_testcase(
        '8bpp-colorsimportant-negative.bmp',
        bitmap_8bpp_negativecolorsimportant(320, 240))

    log.do_testcase(
        'rle8-deltaleavesimage.bmp',
        bitmap_rle8_deltaleavesimage(320, 240))

    log.do_testcase(
        'palette-too-big.bmp',
        bitmap_1bpp_palettetoobig(320, 240))

    log.do_testcase(
        '1bpp-no-palette.bmp',
        bitmap_1bpp_nopalette(320, 240))

    log.do_testcase(
        '4bpp-no-palette.bmp',
        bitmap_4bpp_nopalette(320, 240))

    log.do_testcase(
        '8bpp-no-palette.bmp',
        bitmap_8bpp_nopalette(320, 240))

    log.do_testcase(
        'palette-missing.bmp',
        bitmap_missingpalette(320, 240))

    log.do_testcase(
        'palette-cropped.bmp',
        bitmap_croppedpalette(320, 240))

    log.do_testcase(
        'colormasks-missing.bmp',
        bitmap_missingcolormasks(320, 240))

    log.do_testcase(
        'colormasks-cropped.bmp',
        bitmap_croppedcolormasks(320, 240))

    log.do_testcase(
        'pixeldata-missing.bmp',
        bitmap_missingpixeldata(320, 240))

    log.do_testcase(
        '32bpp-pixeldata-cropped.bmp',
        bitmap_32bpp_croppedpixeldata(320, 240))

    log.do_testcase(
        '24bpp-pixeldata-cropped.bmp',
        bitmap_24bpp_croppedpixeldata(320, 240))

    log.do_testcase(
        '555-pixeldata-cropped.bmp',
        bitmap_555_croppedpixeldata(320, 240))

    log.do_testcase(
        '8bpp-pixeldata-cropped.bmp',
        bitmap_8bpp_croppedpixeldata(320, 240))

    log.do_testcase(
        '4bpp-pixeldata-cropped.bmp',
        bitmap_4bpp_croppedpixeldata(320, 240))

    log.do_testcase(
        '1bpp-pixeldata-cropped.bmp',
        bitmap_1bpp_croppedpixeldata(320, 240))

    log.do_testcase(
        'rle4-runlength-cropped.bmp',
        bitmap_rle4_croppedrun(320, 240))

    log.do_testcase(
        'rle4-absolute-cropped.bmp',
        bitmap_rle4_croppedabsolute(320, 240))

    log.do_testcase(
        'rle4-delta-cropped.bmp',
        bitmap_rle4_croppeddelta(320, 240))

    log.do_testcase(
        'rle4-no-end-of-line-marker.bmp',
        bitmap_rle4_noendofline(320, 240))

    log.do_testcase(
        'rle8-runlength-cropped.bmp',
        bitmap_rle8_croppedrun(320, 240))

    log.do_testcase(
        'rle8-absolute-cropped.bmp',
        bitmap_rle8_croppedabsolute(320, 240))

    log.do_testcase(
        'rle8-delta-cropped.bmp',
        bitmap_rle8_croppeddelta(320, 240))

    log.do_testcase(
        'rle8-no-end-of-line-marker.bmp',
        bitmap_rle8_noendofline(320, 240))

    # write out the HTML index
    log.write_index('index.html')

if __name__ == "__main__":

    generate_valid_bitmaps()
    generate_questionable_bitmaps()
    generate_corrupt_bitmaps()
