# Module generating C source and headers for various PixelTypes
# 2011 - Tomas Gavenciak <gavento@ucw.cz> 

"""
The functions gen_* generate both source and header.
Such functions accept (and then extend) two list of strins.
These should be later joined with "" or "\\n".
"""

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
    "void GP_Pixel_Print_{{ f.name }}(GP_Pixel p);\n", f=ptype)
  code.rbody(
    "/* print formatted value of pixel type {{f.name}} */\n"
    "void GP_Pixel_Print_{{ f.name }}(GP_Pixel p)\n"
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

def gen_convert_to(f1, f2, header, code):
  "Generate a macro converting from f1 to f2"
  "This function supports only RGBVA types (no palettes"
  allowed_chansets = [ set(list(s)) for s in ['RGB', 'RGBA', 'V', 'VA'] ]
  assert(set(f1.chans.keys()) in allowed_chansets)
  assert(set(f2.chans.keys()) in allowed_chansets)

  header.rbody(
    "\n/*** {{ f1.name }} -> {{ f2.name }} ***\n"
    " * macro storing p1 ({{ f1.name }} at bit-offset o1) in p2 ({{ f2.name }} at bit-offset o2),\n"
    " * the relevant part of p2 is assumed to be clear (zero) */\n\n"
    "#define GP_Pixel_{{ f1.name }}_TO_{{ f2.name }}_OFFSET(p1, o1, p2, o2) do {\\\n"
    
  ## set each of <TARGET> channels
    "{% for c2 in f2.chanslist %}"
    
    # case 1: just copy a channel 
    "{%- if c2[0] in f1.chans.keys() %}{% set c1 = f1.chans[c2[0]] %}"
    "	/* {{ c2[0] }}:={{ c1[0] }} */ GP_SET_BITS({{c2[1]}}+o2, \\\n"
    "		GP_SCALE_VAL_{{c1[2]}}_{{c2[2]}}(GP_GET_BITS({{c1[1]}}+o1, {{c1[2]}}, p1)), p2); \\\n"
    
    # case 2: set A to full opacity (not present in source)
    "{% elif c2[0]=='A' %}"
    "	/* A:={{ hmask(c2[2]) }} */GP_SET_BITS({{c2[1]}}+o2, {{ hmask(c2[2]) }}, p2); \\\n"
    
    # case 3: calculate V as average of RGB 
    "{% elif c2[0]=='V' and set('RGB').issubset(set(f1.chans.keys())) %}"
    "	/* V:=RGB_avg */ GP_SET_BITS({{c2[1]}}+o2, ( \\\n"
    "{% for c1 in [f1.chans['R'], f1.chans['G'], f1.chans['B']] %}"
    "		/* {{c1[0]}} */ GP_SCALE_VAL_{{c1[2]}}_{{c2[2]}}(GP_GET_BITS({{c1[1]}}+o1, {{c1[2]}}, p1), p2) + \\\n"
    "{% endfor %}"
    "	0)/3);\\\n"
    
    #- case 4: set each RGB to V 
    "{% elif c2[0] in 'RGB' and 'V' in f1.chans.keys() %}{% set c1 = f1.chans['V'] %}"
    "	/* {{ c2[0] }}:=V */ GP_SET_BITS({{c2[1]}}+o2, \\\n"
    "		GP_SCALE_VAL_{{c1[2]}}_{{c2[2]}}(GP_GET_BITS({{c1[1]}}+o1, {{c1[2]}}, p1)), p2); \\\n"
    
    # invalid mapping (there should be none, but who knows ...) 
    "{% else %} {{ raise(Error('channel conversion' +f1.name+ ' to ' +f2.name+ ' not supported')) }}"
    
    # end of the loop
    "{% endif %}"
    "{% endfor %}"
    "} while (0)\n\n"
    
    # add version without offsets
    "/* a version without offsets */\n"
    "#define GP_Pixel_{{ f1.name }}_TO_{{ f2.name }}(p1, p2) "
      "(GP_Pixel_{{ f1.name }}_TO_{{ f2.name }}_OFFSET(p1, 0, p2, 0))\n",
    f1=f1, f2=f2, hmask=hmask, set=set)

def gen_get_pixel_addr(ptype, header, code):
  "Generate GP_PIXEL_ADDR_<TYPE> and _OFFSET_<TYPE> macros"
  header.rbody(
    "/* macro to get address of pixel {{ f.name }} in a context */\n"
    "#define GP_PIXEL_ADDR_{{ f.name }}(context, x, y) GP_PIXEL_ADDR_{{ f.size_suffix }}(context, x, y)\n"
    "/* macro to get bit-offset of pixel {{ f.name }} */\n"
    "#define GP_PIXEL_ADDR_OFFSET_{{ f.name }}(x) \\\n"
    "	GP_PIXEL_ADDR_OFFSET_{{ f.size_suffix }}(x)\n",
    f=ptype)

def gen_get_pixel_addr_bpp(size, size_suffix, header, code):
  "Generate GP_PIXEL_ADDR_<size_suffix> and _OFFSET_<size_suffix> macros"
  bit_endian = size_suffix[-2:]
  if size < 8:
    assert bit_endian in ['LE', 'BE']
  header.rbody(
    "/* macro to get address of pixel in a {{ size_suffix }} context */\n"
    "#define GP_PIXEL_ADDR_{{ size_suffix }}(context, x, y) \\\n"
    "        ((context)->pixels + (context)->bytes_per_row * (y) + {{ size//8 }} * (x))\n"
    "/* macro to get bit-offset of pixel in {{ size_suffix }} context */\n"
    "{% if size >= 8 %}"
      "#define GP_PIXEL_ADDR_OFFSET_{{ size_suffix }}(x) (0)\n"
    "{% else %}" # bit_endian matters
      "{% if bit_endian=='LE' %}"
        "#define GP_PIXEL_ADDR_OFFSET_{{ size_suffix }}(x) \\\n"
	"	(((x) % {{ 8//size }}) * {{ size }})\n"
      "{% else %}"
        "#define GP_PIXEL_ADDR_OFFSET_{{ size_suffix }}(x) \\\n"
	"	({{ 8-size }} - ((x) % {{ 8//size }}) * {{ size }})\n"
      "{% endif %}"
    "{% endif %}",
    size=size, size_suffix=size_suffix, bit_endian=bit_endian)
