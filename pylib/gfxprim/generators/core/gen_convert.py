# Module generating C source and headers for various PixelType conversions
# 2011 - Tomas Gavenciak <gavento@ucw.cz>

from gfxprim.generators.pixeltype import pixeltypes, channels
from gfxprim.generators.utils import hmask

def gen_fixedtype_to_type(fixedtype, header, code):
  "Generate functions converting a fixed PixelType to any other"
  "Does not work on palette types at all (yet)"
  code.rbody(
    "GP_Pixel GP_{{ f.name }}ToPixel(GP_Pixel pixel, GP_PixelType type)\n"
    "{\n"
    "	GP_Pixel p = 0;\n"
    "	switch(type) {\n"
    "{% for tf in types %}"
    "{% if tf.number == 0 %}"
      "		case GP_PIXEL_UNKNOWN:\n"
      "			GP_ABORT(\"Cannot convert to GP_PIXEL_UNKNOWN\");\n"
      "			break;\n"
    "{% elif tf.is_palette() %}"
      "		case GP_PIXEL_{{ tf.name }}:\n"
      "			GP_ABORT(\"Cannot convert to palette type {{ tf.name }}\");\n"
      "			break;\n"
    "{% else %}"
      "		case GP_PIXEL_{{ tf.name }}:\n"
      "			GP_Pixel_{{ f.name }}_TO_{{ tf.name }}(pixel, p);\n"
      "			break;\n"
    "{% endif %}"
    "{% endfor %}"
    "		default:\n"
    "			GP_ABORT(\"Unknown PixelType %ud\", type);\n"
    "	}\n"
    "	return p;\n"
    "}\n\n", f=fixedtype, types=pixeltypes.values())

def gen_type_to_fixedtype(fixedtype, header, code):
  "Generate functions converting to a fixed PixelType from any other"
  "Does not work on palette types at all (yet)"
  code.rbody(
    "GP_Pixel GP_PixelTo{{ f.name }}(GP_Pixel pixel, GP_PixelType type)\n"
    "{\n"
    "	GP_Pixel p = 0;\n"
    "	switch(type) {\n"
    "{% for sf in types %}"
    "{% if sf.number == 0 %}"
      "		case GP_PIXEL_UNKNOWN:\n"
      "			GP_ABORT(\"Cannot convert from GP_PIXEL_UNKNOWN\");\n"
      "			break;\n"
    "{% elif sf.is_palette() %}"
      "		case GP_PIXEL_{{ sf.name }}:\n"
      "			GP_ABORT(\"Cannot convert from palette type {{ sf.name }} (yet)\");\n"
      "			break;\n"
    "{% else %}"
      "		case GP_PIXEL_{{ sf.name }}:\n"
      "			GP_Pixel_{{ sf.name }}_TO_{{ f.name }}(pixel, p);\n"
      "			break;\n"
    "{% endif %}"
    "{% endfor %}"
    "		default:\n"
    "			GP_ABORT(\"Unknown PixelType %u\", type);\n"
    "	}\n"
    "	return p;\n"
    "}\n\n", f=fixedtype, types=pixeltypes.values())

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
    "	/* {{ c2[0] }}:={{ c1[0] }} */ GP_SET_BITS({{c2[1]}}+o2, {{c2[2]}}, p2,\\\n"
    "		GP_SCALE_VAL_{{c1[2]}}_{{c2[2]}}(GP_GET_BITS({{c1[1]}}+o1, {{c1[2]}}, p1))); \\\n"

    # case 2: set A to full opacity (not present in source)
    "{% elif c2[0]=='A' %}"
    "	/* A:={{ hmask(c2[2]) }} */GP_SET_BITS({{c2[1]}}+o2, {{c2[2]}}, p2, {{ hmask(c2[2]) }}); \\\n"

    # case 3: calculate V as average of RGB
    "{% elif c2[0]=='V' and set('RGB').issubset(set(f1.chans.keys())) %}"
    "	/* V:=RGB_avg */ GP_SET_BITS({{c2[1]}}+o2, {{c2[2]}}, p2, ( \\\n"
    "{% for c1 in [f1.chans['R'], f1.chans['G'], f1.chans['B']] %}"
    "		/* {{c1[0]}} */ GP_SCALE_VAL_{{c1[2]}}_{{c2[2]}}(GP_GET_BITS({{c1[1]}}+o1, {{c1[2]}}, p1)) + \\\n"
    "{% endfor %}"
    "	0)/3);\\\n"

    #- case 4: set each RGB to V
    "{% elif c2[0] in 'RGB' and 'V' in f1.chans.keys() %}{% set c1 = f1.chans['V'] %}"
    "	/* {{ c2[0] }}:=V */ GP_SET_BITS({{c2[1]}}+o2, {{c2[2]}}, p2,\\\n"
    "		GP_SCALE_VAL_{{c1[2]}}_{{c2[2]}}(GP_GET_BITS({{c1[1]}}+o1, {{c1[2]}}, p1))); \\\n"

    # invalid mapping (there should be none, but who knows ...)
    "{% else %} {{ raise(Error('channel conversion' +f1.name+ ' to ' +f2.name+ ' not supported')) }}"

    # end of the loop
    "{% endif %}"
    "{% endfor %}"
    "} while (0)\n\n"

    # add version without offsets
    "/* a version without offsets */\n"
    "#define GP_Pixel_{{ f1.name }}_TO_{{ f2.name }}(p1, p2) "
      "GP_Pixel_{{ f1.name }}_TO_{{ f2.name }}_OFFSET(p1, 0, p2, 0)\n",
    f1=f1, f2=f2, hmask=hmask, set=set)

