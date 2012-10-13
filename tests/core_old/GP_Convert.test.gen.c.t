%% extends "base.test.c.t"

%% block body
#include "GP_Tests.h"
#include "GP_Convert.h"
#include "GP_TestingCore.h"

GP_SUITE(GP_Convert)

%% call(pt) test_for_all_pixeltypes("GP_ConvertPixel_for",
				    opts="loop_start=0, loop_end=4",
				    palette=False)
	GP_Pixel p1 = GP_RandomColor(GP_PIXEL_{{ pt.name }});
	GP_Pixel p2 = GP_ConvertPixel(p1, GP_PIXEL_{{ pt.name }}, GP_PIXEL_RGBA8888);
	GP_Pixel p3 = GP_ConvertPixel(p2, GP_PIXEL_RGBA8888, GP_PIXEL_{{ pt.name }});
	GP_CHECK_EqualColors(p1, GP_PIXEL_{{ pt.name }}, p3, GP_PIXEL_{{ pt.name }});
	GP_CHECK_EqualColors(p1, GP_PIXEL_{{ pt.name }}, p2, GP_PIXEL_RGBA8888);
%% endcall

%% call(pt) test_for_all_pixeltypes("WhiteStaysWhite_via",
				    opts="loop_start=0, loop_end=4",
				    palette=False)
	GP_Pixel p1 = GP_RGBToPixel(255, 255, 255, GP_PIXEL_RGB888);
	GP_Pixel p2 = GP_RGB888ToPixel(p1, GP_PIXEL_{{ pt.name }});
	GP_Pixel p3 = GP_PixelToRGB888(p2, GP_PIXEL_{{ pt.name }});
	GP_CHECK_EqualColors(p1, GP_PIXEL_RGB888, p3, GP_PIXEL_RGB888);
%% endcall

%% call(pt) test_for_all_pixeltypes("BlackStaysBlack_via",
				    opts="loop_start=0, loop_end=4",
				    palette=False)
	GP_Pixel p1 = GP_RGBToPixel(0, 0, 0, GP_PIXEL_RGB888);
	GP_Pixel p2 = GP_RGB888ToPixel(p1, GP_PIXEL_{{ pt.name }});
	GP_Pixel p3 = GP_PixelToRGB888(p2, GP_PIXEL_{{ pt.name }});
	GP_CHECK_EqualColors(p1, GP_PIXEL_RGB888, p3, GP_PIXEL_RGB888);
%% endcall

%% endblock body
