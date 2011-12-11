%% extends "base.test.c.t"

%% block body
#include "GP_Tests.h"
#include "GP_Core.h"
#include "GP_MixPixels.gen.h"
#include "GP_TestingCore.h"

GP_SUITE(GP_MixPixels)

%% call(pt) test_for_all_pixeltypes("MixPixelsWhiteStaysWhite_via",
				    opts="loop_start=0, loop_end=4",
				    palette=False)
	GP_Pixel p1 = GP_RGBToPixel(255, 255, 255, GP_PIXEL_{{ pt.name }});
	GP_Pixel p2 = GP_RGBToPixel(255, 255, 255, GP_PIXEL_{{ pt.name }});
	GP_Pixel p3;
	int i;
	for (i = 0; i < 256; i++) {
		p3 = GP_MIX_PIXELS_{{ pt.name }}(p1, p2, i);
		GP_CHECK_EqualColors(p1, GP_PIXEL_{{ pt.name }}, p3, GP_PIXEL_{{ pt.name }});
	}
%% endcall

%% call(pt) test_for_all_pixeltypes("MixPixelsBlackStaysBlack_via",
				    opts="loop_start=0, loop_end=4",
				    palette=False)
	GP_Pixel p1 = GP_RGBToPixel(0, 0, 0, GP_PIXEL_{{ pt.name }});
	GP_Pixel p2 = GP_RGBToPixel(0, 0, 0, GP_PIXEL_{{ pt.name }});
	GP_Pixel p3;
	int i;
	for (i = 0; i < 256; i++) {
		p3 = GP_MIX_PIXELS_{{ pt.name }}(p1, p2, i);
		GP_CHECK_EqualColors(p1, GP_PIXEL_{{ pt.name }}, p3, GP_PIXEL_{{ pt.name }});
	}
%% endcall

%% call(pt) test_for_all_pixeltypes("MixPixelsSymmetry_via",
				    opts="loop_start=0, loop_end=4",
				    palette=False)
	GP_Pixel p1 = GP_RGBToPixel(0, 124, 12, GP_PIXEL_{{ pt.name }});
	GP_Pixel p2 = GP_RGBToPixel(255, 99, 0, GP_PIXEL_{{ pt.name }});
	GP_Pixel p3, p4;
	int i;
	for (i = 0; i < 256; i++) {
		p3 = GP_MIX_PIXELS_{{ pt.name }}(p1, p2, i);
		p4 = GP_MIX_PIXELS_{{ pt.name }}(p2, p1, 255 - i);
		GP_CHECK_EqualColors(p3, GP_PIXEL_{{ pt.name }}, p4, GP_PIXEL_{{ pt.name }});
	}
%% endcall

%% endblock body
