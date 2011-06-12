/*
 * 2011 - Tomas Gavenciak <gavento@ucw.cz>
 */

#include "GP_Tests.h"
#include "GP_Convert.h"
#include "GP_TestingCore.h"

GP_SUITE(GP_Convert)

GP_TEST(BasicPixelConversions)
{
	GP_Pixel p1, p2;
	p1 = GP_RGBAToPixel(255, 255, 255, 255, GP_PIXEL_RGBA8888);
	fail_unless(p1 == 0xffffffff);
	p1 = GP_RGBAToPixel(0, 0, 0, 0, GP_PIXEL_RGBA8888);
	fail_unless(p1 == 0x0);
	p1 = GP_RGBToPixel(0x12, 0x34, 0x56, GP_PIXEL_RGB888);
	fail_unless(p1 == 0x563412);
	GP_CHECK_EqualColors(p1, GP_PIXEL_RGB888, p1, GP_PIXEL_RGB888);
	
	p1 = GP_RGB888ToPixel(GP_RGBToPixel(0x12, 0x34, 0x56, GP_PIXEL_RGB888), GP_PIXEL_V4);
	p2 = GP_RGBAToPixel(0x12, 0x34, 0x56, 0x78, GP_PIXEL_V2);
	GP_CHECK_EqualColors(p1, GP_PIXEL_V4, p2, GP_PIXEL_V2);
}
GP_ENDTEST

