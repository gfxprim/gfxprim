# Module generating C source and headers for various PixelTypes
# 2011 - Tomas Gavenciak <gavento@ucw.cz> 

from gfxprim.generators.pixeltype import pixeltypes, channels
from gfxprim.generators.utils import j2render as r, hmask


def str_start(ptype):
  "Return a visual separator starting `ptype`-related defs"
  return ("\n/*************************************************************\n"
         " * Pixel type " + ptype.name + "\n */\n")

def str_description(ptype):
  "Return a short C comment describing the PixelType"
  return r(
    "/* Automatically generated code for pixel type {{ f.name }}\n"
    " *\n"
    " * Size (bpp): {{ f.size }} ({{ f.size_suffix }})\n"
    "{% if f.size<8 %} * Bit endian: {{ f.bit_endian }}\n{% endif %}"
    " * Pixel structure: {{ ''.join(f.bits) }}\n"
    " * Channels: \n"
    "{% for c in f.chanslist %}"
    " *   {{ c[0] }}  offset:{{ c[1] }} size:{{ c[2] }}\n"
    "{% endfor %}"
    " */\n", f=ptype)

def gen_GP_PixelType(header, code):
  "Generates definition of GP_PixelType enum"
  pt_by_num = sorted([(t.number, t) for t in pixeltypes.values()])
  sorted_pts = [t[1] for t in pt_by_num]
  pt_max = len(sorted_pts) 
  header.rbody(
    "/* List of all known pixel types */\n"
    "typedef enum GP_PixelType {\n"
    "{% for t in sorted_pts %}"
    "	GP_PIXEL_{{ t.name }} = {{ t.number }},\n"
    "{% endfor %}"
    "	GP_PIXEL_MAX = {{ pt_max }},\n"
    "} GP_PixelType;\n", sorted_pts=sorted_pts, pt_max=pt_max)

def gen_GP_PixelTypes(header, code):
  "Generate the const structure GP_PixelTypes describing all the PixelTypes"
  pt_by_num = sorted([(t.number, t) for t in pixeltypes.values()])
  sorted_pts = [t[1] for t in pt_by_num]
  code.rbody(
    "/* Description of all known pixel types */\n"
    "const GP_PixelTypeDescription const GP_PixelTypes [] = {\n"
    "{% for t in sorted_pts %}"
    "	/* GP_PIXEL_{{ t.name }} */ {\n"
    "		.type        = GP_PIXEL_{{ t.name }},\n"
    '		.name        = "{{ t.name }}",\n'
    '		.size        = {{ t.size }},\n'
    '		.bit_endian  = GP_BIT_ENDIAN_{{ t.bit_endian }},'
    '{% if t.size>=8 %} /* IGNORED for this type */{% endif %}\n'
    '		.numchannels = {{ len(t.chanslist) }},\n'
    '		.bitmap      = "{{ t.bits|join("") }}",\n'
    '		.channels    = {\n'
    '{% for c in t.chanslist %}'
    '			{ .name = "{{ c[0] }}", .offset = {{ c[1] }}, .size = {{ c[2] }} },\n'
    '{% endfor %}'
    '	} },\n'
    '{% endfor %}'
    '};\n', sorted_pts=sorted_pts, len=len)

def gen_print(ptype, header, code):
  "Generate a GP_Pixel_Print_<TYPE> function (source and header)"
  header.rbody(
    "/* print formatted value of pixel type {{ f.name }} */\n"
    "void GP_PixelPrint_{{ f.name }}(GP_Pixel p);\n", f=ptype)
  code.rbody(
    "/* print formatted value of pixel type {{f.name}} */\n"
    "void GP_PixelPrint_{{ f.name }}(GP_Pixel p)\n"
    "{\n"
    '	printf("<{{ f.name }} %0{{ (f.size+3)//4 }}x{% for c in f.chanslist %} {{ c[0] }}=%d{% endfor %}>",\n'
    "		GP_GET_BITS(0, {{ f.size }}, p)"
    "{% for c in f.chanslist %}"
      ",\n		GP_GET_BITS({{ c[1] }}, {{ c[2] }}, p)"
    "{% endfor %});\n"
    "}\n", f=ptype)

def gen_get_chs(ptype, header, code):
  "Generate GP_Pixel_GET_<CHAN>_<TYPE> macros"
  header.rbody(
  "/* macros to get channels of pixel type {{ f.name }} */\n"
  "{% for c in f.chanslist %}"
    "#define GP_Pixel_GET_{{ c[0] }}_{{ f.name }}(p) (GP_GET_BITS({{ c[1] }}, {{ c[2] }}, (p)))\n"
  "{% endfor %}", f=ptype)

def gen_create(ptype, header, code):
  "Generate GP_Pixel_CREATE_<TYPE> macros"
  header.rbody(
  "/* macros to create GP_Pixel of pixel type {{ f.name }} directly from given values.\n"
  " * The values are NOT clipped to actual value ranges.*/\n"
  "#define GP_Pixel_CREATE_{{ f.name }}({{ args }}) (0\\\n"
  "{% for c in f.chanslist %}"
    "	+ (({{ c[0] }}) << {{ c[1] }}) \\\n"
  "{% endfor %}"
  "	)\n", f=ptype, args=', '.join([c[0] for c in ptype.chanslist]))

def gen_get_pixel_addr(ptype, header, code):
  "Generate GP_PIXEL_ADDR_<TYPE> and _OFFSET_<TYPE> macros"
  header.rbody(
    "/* macro to get address of pixel {{ f.name }} in a context */\n"
    "#define GP_PIXEL_ADDR_{{ f.name }}(context, x, y) GP_PIXEL_ADDR_{{ f.size_suffix }}(context, x, y)\n"
    "/* macro to get bit-offset of pixel {{ f.name }} */\n"
    "#define GP_PIXEL_ADDR_OFFSET_{{ f.name }}(x) \\\n"
    "	GP_PIXEL_ADDR_OFFSET_{{ f.size_suffix }}(x)\n",
    f=ptype)

