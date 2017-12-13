"""
Module extending the Pixmap class with .filter submodule.

Use as in "import gfxprim.filters; pixmap_foo.filter.resize(...)"
"""

# Import the SWIG wrapper
from . import c_filters

def _init(module):
  "Extend Pixmap with filters submodule"

  from ..utils import extend, extend_submodule
  from ..core import Pixmap as _pixmap

  # New Pixmap submodule
  class FiltersSubmodule(object):
    def __init__(self, ctx):
      self.ctx = ctx

  _pixmap._submodules['filters'] = FiltersSubmodule

  for name in ['invert', 'invert_alloc',
               'brightness', 'brightness_alloc',
               'contrast', 'contrast_alloc',
               'brightness_contrast', 'brightness_contrast_alloc',
               'posterize', 'posterize_alloc',
               'resize', 'resize_alloc',
               'rotate_90', 'rotate_90_alloc',
	       'rotate_180', 'rotate_180_alloc',
	       'rotate_270', 'rotate_270_alloc',
	       'mirror_h', 'mirror_h_alloc',
	       'mirror_v', 'mirror_v_alloc',
	       'add', 'mul', 'diff', 'max', 'min',
	       'gaussian_blur', 'gaussian_blur_alloc',
	       'gaussian_blur_ex', 'gaussian_blur_ex_alloc',
	       'gaussian_noise_add', 'gaussian_noise_add_alloc',
	       'gaussian_noise_add_ex', 'gaussian_noise_add_ex_alloc',
	       'laplace', 'laplace_alloc',
	       'edge_sharpening', 'edge_sharpening_alloc',
	       'median', 'median_alloc', 'median_ex', 'median_ex_alloc',
	       'sigma', 'sigma_alloc', 'sigma_ex', 'sigma_ex_alloc',
	       'floyd_steinberg', 'floyd_steinberg_alloc',
	       'hilbert_peano', 'hilbert_peano_alloc',
	       'sepia', 'sepia_alloc', 'sepia_ex', 'sepia_ex_alloc']:
    extend_submodule(FiltersSubmodule, name, c_filters.__getattribute__('gp_filter_' + name))

  def array_to_kern(kernel, kernel_div):
    h = len(kernel)
    w = len(kernel[0])

    # Assert that array is matrix of numbers
    for i in range(0, h):
      assert(len(kernel[i]) == w)

      for j in kernel[i]:
        assert(isinstance(j, float) or isinstance(j, int))

    # flatten the python array into C array
    karr = c_filters.new_float_array(w * h)

    for i in range(0, h):
      for j in range(0, w):
        c_filters.float_array_setitem(karr, i * w + j, kernel[i][j])

    kern = c_filters.gp_filter_kernel_2d(w, h, karr, kernel_div)

    return kern

  def array_del(kern):
    c_filters.delete_float_array(kern.kernel)

  def convolution(src, dst, kernel, kernel_div, callback=None):
    """
     convolution(src, dst, kernel, kernel_div, callback=None)

     Bilinear convolution. The kernel is two dimensional array of coefficients,
     kern_div is used to divide the kernel weigthed sum.
    """
    kern = array_to_kern(kernel, kernel_div)
    ret = c_filters.gp_filter_convolution(src, dst, kern, callback)
    array_del(kern)
    return ret

  extend_submodule(FiltersSubmodule, 'convolution', convolution)

  def convolution_ex(src, x_src, y_src, w_src, h_src, dst, x_dst, y_dst,
                     kernel, kernel_div, callback=None):
    """
     convolution_ex(src, x_src, y_src, w_src, h_src, dst, x_dst, y_dst,
                    kernel, kernel_div, callback=None)

     Bilinear convolution. The kernel is two dimensional array of coefficients,
     kern_div is used to divide the kernel weigthed sum.
    """
    kern = array_to_kern(kernel, kernel_div);
    ret =  c_filters.gp_filter_convolution_ex(src, x_src, y_src,
                                              w_src, h_src, dst, x_dst, y_dst,
                                              kern, callback)
    array_del(kern)
    return ret

  extend_submodule(FiltersSubmodule, 'convolution_ex', convolution_ex)

  def convolution_alloc(src, kernel, kernel_div, callback=None):
    """
     convolution_alloc(src, kernel, kernel_div, callback=None)

     Bilinear convolution. The kernel is two dimensional array of coefficients,
     kern_div is used to divide the kernel weigthed sum.
    """
    kern = array_to_kern(kernel, kernel_div);
    ret = c_filters.gp_filter_convolution_alloc(src, kern, callback)
    array_del(kern)
    return ret

  extend_submodule(FiltersSubmodule, 'convolution_alloc', convolution_alloc)

  def convolution_ex_alloc(src, x_src, y_src, w_src, h_src,
                           kernel, kernel_div, callback=None):
    """
     convolution_ex_alloc(src, x_src, y_src, w_src, h_src,
                          kernel, kernel_div, callback=None)

     Bilinear convolution. The kernel is two dimensional array of coefficients,
     kern_div is used to divide the kernel weigthed sum.
    """
    kern = array_to_kern(kernel, kernel_div);
    ret = c_filters.gp_filter_convolution_ex_alloc(src, x_src, y_src,
                                                   w_src, h_src, kern, callback)
    array_del(kern)
    return ret

  extend_submodule(FiltersSubmodule, 'convolution_ex_alloc', convolution_ex_alloc)

  # Imports from the SWIG module
  import re
  def strip_gp_filter(s):
    return re.sub('^gp_filter_', '', s)

  # Import functions from the SWIG module
  from ..utils import import_members
  import_members(c_filters, module, sub=strip_gp_filter,
      include=[
        '^gp_filter.*alloc',
        '^gp_filter[A-Za-z0-9]*$',
        ])

  module['convolution'] = convolution
  module['convolution_alloc'] = convolution_alloc
  module['convolution_ex'] = convolution_ex
  module['convolution_ex_alloc'] = convolution_ex_alloc


  #
  # Special handling for weighted median parameters
  #
  def array_to_weights(weights):
    h = len(weights)
    w = len(weights[0])

    # Assert that array is matrix of numbers
    for i in range(0, h):
      assert(len(weights[i]) == w)

      for j in weights[i]:
        assert(isinstance(j, int) and j >= 0)

    # flatten the python array into C array
    warr = c_filters.new_uint_array(w * h)

    for i in range(0, h):
      for j in range(0, w):
        c_filters.uint_array_setitem(warr, i * w + j, weights[i][j])

    ret = c_filters.gp_median_weights(w, h, warr)

    return ret

  def uint_arr_del(w):
    c_filters.delete_uint_array(w.weights)

  def weighted_median(src, dst, weights, callback=None):
    """
     weighted_median(src, dst, weights, callback=None)

     Weighted variant of median filter. Weights is a two dimensional
     array of positive integers that describe weights for neighbour pixels.
    """
    w = array_to_weights(weights)
    ret = c_filters.gp_filter_weighted_median(src, dst, w, callback)
    uint_arr_del(w)
    return ret

#  extend_submodule(FiltersSubmodule, 'weighted_median', weighted_median)
#  module['weighted_median'] = weighted_median

  def weighted_median_alloc(src, weights, callback=None):
    """
     weighted_median_alloc(src, weights, callback=None)

     Weighted variant of median filter. Weights is a two dimensional
     array of positive integers that describe weights for neighbour pixels.
    """
    w = array_to_weights(weights)
    ret = c_filters.gp_filter_weighted_median_alloc(src, w, callback)
    uint_arr_del(w)
    return ret

#  extend_submodule(FiltersSubmodule, 'weighted_median_alloc', weighted_median_alloc)
#  module['weighted_median_alloc'] = weighted_median_alloc

_init(locals())
del _init
