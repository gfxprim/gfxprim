// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Gamma correction.

  What is gamma and what is it doing in my computer?
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  First of all gamma is a function, or better there is a gamma function and
  it's inverse function. Both gamma function and it's inverse are defined on
  interval [0,1] and are defined as out = in^(gamma) and it's inverse as
  out = in^(1/gamma).

  The purpose of this function is to compensate nonlinearity of human eye
  perception. The human eye is more sensitive to dark tones than the light ones
  so without gamma correction storage and manipulation with image data would
  either be less efficient in space (in case you decided to use more bits and
  encode the image lineary) or quantization in darker tones would be more
  visible resulting in "pixelated" images (aliasing).

  So there is a gamma, the internet seems to suggest that usual values for
  gamma are 2.5 for old CRT monitors and about 2.2 for LCD ones, ideally you
  should have color profile for your device (you need special hardware to
  measure it). So if you are trying to draw linear gradient on the screen
  you need to generate sequence of numbers accordinly to gamma function
  (the 50% intensity is around 186 for gamma = 2.2 and 8bit grayscale pixel).

  Moreover image formats tend to save data in nonlinear fashion (some formats
  include gama value used to for the image) so before you apply filter that
  manipulates with pixel values, you need to convert it to linear space (adding
  some more bits to compensate for rounding errors).

  Also it's important to take gamma into accound if you start drawing anti
  aliased shapes, you can't get right results if you don't consider gamma.

 */

#ifndef CORE_GP_GAMMA_CORRECTION_H
#define CORE_GP_GAMMA_CORRECTION_H

#include <stdint.h>
#include <core/gp_types.h>
#include <core/gp_gamma_correction.gen.h>

#endif /* CORE_GP_GAMMA_CORRECTION_H */
