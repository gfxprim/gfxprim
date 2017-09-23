"""
Module extending the Pixmap class with .filter submodule.

Use as in "import gfxprim.filters; pixmap_foo.filter.Resize(...)"
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

  for name in ['Invert', 'InvertAlloc',
               'Brightness', 'BrightnessAlloc',
               'Contrast', 'ContrastAlloc',
               'BrightnessContrast', 'BrightnessContrastAlloc',
               'Posterize', 'PosterizeAlloc',
               'Resize', 'ResizeAlloc',
               'Rotate90', 'Rotate90Alloc',
	       'Rotate180', 'Rotate180Alloc',
	       'Rotate270', 'Rotate270Alloc',
	       'MirrorH', 'MirrorHAlloc',
	       'MirrorV', 'MirrorVAlloc',
	       'Addition', 'Multiply', 'Difference', 'Max', 'Min',
	       'GaussianBlur', 'GaussianBlurAlloc',
	       'GaussianBlurEx', 'GaussianBlurExAlloc',
	       'GaussianNoiseAdd', 'GaussianNoiseAddAlloc',
	       'GaussianNoiseAddEx', 'GaussianNoiseAddExAlloc',
	       'Laplace', 'LaplaceAlloc',
	       'EdgeSharpening', 'EdgeSharpeningAlloc',
	       'Median', 'MedianAlloc', 'MedianEx', 'MedianExAlloc',
	       'Sigma', 'SigmaAlloc', 'SigmaEx', 'SigmaExAlloc',
	       'FloydSteinberg', 'FloydSteinbergAlloc',
	       'HilbertPeano', 'HilbertPeanoAlloc',
	       'Sepia', 'SepiaAlloc', 'SepiaEx', 'SepiaExAlloc']:
    extend_submodule(FiltersSubmodule, name, c_filters.__getattribute__('GP_Filter' + name))

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

    kern = c_filters.GP_FilterKernel2D(w, h, karr, kernel_div)

    return kern

  def array_del(kern):
    c_filters.delete_float_array(kern.kernel)

  def Convolution(src, dst, kernel, kernel_div, callback=None):
    """
     Convolution(src, dst, kernel, kernel_div, callback=None)

     Bilinear convolution. The kernel is two dimensional array of coefficients,
     kern_div is used to divide the kernel weigthed sum.
    """
    kern = array_to_kern(kernel, kernel_div)
    ret = c_filters.GP_FilterConvolution(src, dst, kern, callback)
    array_del(kern)
    return ret

  extend_submodule(FiltersSubmodule, 'Convolution', Convolution)

  def ConvolutionEx(src, x_src, y_src, w_src, h_src, dst, x_dst, y_dst,
                    kernel, kernel_div, callback=None):
    """
     ConvolutionEx(src, x_src, y_src, w_src, h_src, dst, x_dst, y_dst,
                   kernel, kernel_div, callback=None)

     Bilinear convolution. The kernel is two dimensional array of coefficients,
     kern_div is used to divide the kernel weigthed sum.
    """
    kern = array_to_kern(kernel, kernel_div);
    ret =  c_filters.GP_FilterConvolutionEx(src, x_src, y_src,
                                            w_src, h_src, dst, x_dst, y_dst,
                                            kern, callback)
    array_del(kern)
    return ret

  extend_submodule(FiltersSubmodule, 'ConvolutionEx', ConvolutionEx)

  def ConvolutionAlloc(src, kernel, kernel_div, callback=None):
    """
     ConvolutionAlloc(src, kernel, kernel_div, callback=None)

     Bilinear convolution. The kernel is two dimensional array of coefficients,
     kern_div is used to divide the kernel weigthed sum.
    """
    kern = array_to_kern(kernel, kernel_div);
    ret = c_filters.GP_FilterConvolutionAlloc(src, kern, callback)
    array_del(kern)
    return ret

  extend_submodule(FiltersSubmodule, 'ConvolutionAlloc', ConvolutionAlloc)

  def ConvolutionExAlloc(src, x_src, y_src, w_src, h_src,
                         kernel, kernel_div, callback=None):
    """
     ConvolutionExAlloc(src, x_src, y_src, w_src, h_src,
                        kernel, kernel_div, callback=None)

     Bilinear convolution. The kernel is two dimensional array of coefficients,
     kern_div is used to divide the kernel weigthed sum.
    """
    kern = array_to_kern(kernel, kernel_div);
    ret = c_filters.GP_FilterConvolutionExAlloc(src, x_src, y_src,
                                                w_src, h_src, kern, callback)
    array_del(kern)
    return ret

  extend_submodule(FiltersSubmodule, 'ConvolutionExAlloc', ConvolutionExAlloc)

  # Imports from the SWIG module
  import re
  def strip_GP_Filter(s):
    return re.sub('^GP_Filter', '', s)

  # Import functions from the SWIG module
  from ..utils import import_members
  import_members(c_filters, module, sub=strip_GP_Filter,
      include=[
        '^GP_Filter.*Alloc',
        '^GP_Filter[A-Za-z0-9]*$',
        ])

  module['Convolution'] = Convolution
  module['ConvolutionAlloc'] = ConvolutionAlloc
  module['ConvolutionEx'] = ConvolutionEx
  module['ConvolutionExAlloc'] = ConvolutionExAlloc


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

    ret = c_filters.GP_MedianWeights(w, h, warr)

    return ret

  def uint_arr_del(w):
    c_filters.delete_uint_array(w.weights)

  def WeightedMedian(src, dst, weights, callback=None):
    """
     WeightedMedian(src, dst, weights, callback=None)

     Weighted variant of median filter. Weights is a two dimensional
     array of positive integers that describe weights for neighbour pixels.
    """
    w = array_to_weights(weights)
    ret = c_filters.GP_FilterWeightedMedian(src, dst, w, callback)
    uint_arr_del(w)
    return ret

#  extend_submodule(FiltersSubmodule, 'WeightedMedian', WeightedMedian)
#  module['WeightedMedian'] = WeightedMedian

  def WeightedMedianAlloc(src, weights, callback=None):
    """
     WeightedMedianAlloc(src, weights, callback=None)

     Weighted variant of median filter. Weights is a two dimensional
     array of positive integers that describe weights for neighbour pixels.
    """
    w = array_to_weights(weights)
    ret = c_filters.GP_FilterWeightedMedianAlloc(src, w, callback)
    uint_arr_del(w)
    return ret

#  extend_submodule(FiltersSubmodule, 'WeightedMedianAlloc', WeightedMedianAlloc)
#  module['WeightedMedianAlloc'] = WeightedMedianAlloc

_init(locals())
del _init
