# Module generating C source and headers for various PixelTypes
# 2011 - Tomas Gavenciak <gavento@ucw.cz> 

"""
The functions gen_* generate both source and header.
Such functions accept (and then extend) two list of strins.
These should be later joined with "" or "\\n".
"""

import jinja2
from pixeltype import pixeltypes, channels


def r(tmpl, **kw):
  "Internal helper to render jinja2 templates (with StrictUndefined)"
  return jinja2.Template(tmpl, undefined=jinja2.StrictUndefined).render(**kw)

## Helper returning hex mask for that many bits
def hmask(bits): return hex((1<<bits)-1)


def str_start(ptype):
  "Return a visual separator starting `ptype`-related defs"
  return ("\n"
         "/*************************************************************\n"
         " * Pixel type " + ptype.name + "\n */\n")

def str_description(ptype):
  "Return a short C comment describing the PixelType"
  return r(
    "/* Automatically generated code for pixel type {{ f.name }}\n"
    " *\n"
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
  header.append(r(
    "/* List of all known pixel types */\n"
    "typedef enum GP_PixelType {\n"
    "{% for t in sorted_pts %}"
    "  GP_PIXEL_{{ t.name }} = {{ t.number }},\n"
    "{% endfor %}"
    "  GP_PIXEL_MAX = {{ pt_max }},\n"
    "} GP_PixelType;\n", sorted_pts=sorted_pts, pt_max=pt_max))

def gen_GP_PixelTypes(header, code):
  "Generate the const structure GP_PixelTypes describing all the PixelTypes"
  pt_by_num = sorted([(t.number, t) for t in pixeltypes.values()])
  sorted_pts = [t[1] for t in pt_by_num]
  code.append(r(
    "/* Description of all known pixel types */\n"
    "const GP_PixelTypeDescription const GP_PixelTypes [] = {\n"
    "{% for t in sorted_pts %}"
    "  /* GP_PIXEL_{{ t.name }} */ {\n"
    "    .type        = GP_PIXEL_{{ t.name }},\n"
    '    .name        = "{{ t.name }}",\n'
    '    .size        = {{ t.size }},\n'
    '    .numchannels = {{ len(t.chanslist) }},\n'
    '    .bitmap      = "{{ t.bits|join("") }}",\n'
    '    .channels    = {\n'
    '{% for c in t.chanslist %}'
    '      { .name = "{{ c[0] }}", .offset = {{ c[1] }}, .size = {{ c[2] }} },\n'
    '{% endfor %}'
    '  } },\n'
    '{% endfor %}'
    '};\n', sorted_pts=sorted_pts, len=len))

def gen_print(ptype, header, code):
  "Generate a GP_Pixel_print_<TYPE> function (source and header)"
  header.append(r(
    "/* print formatted value of pixel type {{ f.name }} */\n"
    "void GP_Pixel_print_{{ f.name }}(GP_Pixel p);\n", f=ptype))
  code.append(r(
    "/* print formatted value of pixel type {{f.name}} */\n"
    "void GP_Pixel_print_{{ f.name }}(GP_Pixel p)\n"
    "{\n"
    '  printf("<{{ f.name }} %0{{ (f.size+3)//4 }}x{% for c in f.chanslist %} {{ c[0] }}=%d{% endfor %}>",\n'
    "    GP_GET_BITS(0, {{ f.size }}, p)"
    "{% for c in f.chanslist %}"
      ", GP_GET_BITS({{ c[1] }}, {{ c[2] }}, p)"
    "{% endfor %});\n"
    "}\n", f=ptype))

def gen_get_chs(ptype, header, code):
  "Generate GP_Pixel_get_<CHAN>_<TYPE> macros"
  header.append(r(
  "/* macros to get channels of pixel type {{ f.name }} */\n"
  "{% for c in f.chanslist %}"
  "#define GP_Pixel_get_{{ c[0] }}_{{ f.name }}(p) (GP_GET_BITS({{ c[1] }}, {{ c[2] }}, (p)))\n"
  "{% endfor %}\n", f=ptype))

def gen_convert_to(f1, f2, header, code):
  "Generate a macro converting from f1 to f2"
  "This function supports only RGBVA types (no palettes"
  allowed_chansets = [ set(list(s)) for s in ['RGB', 'RGBA', 'V', 'VA'] ]
  assert(set(f1.chans.keys()) in allowed_chansets)
  assert(set(f2.chans.keys()) in allowed_chansets)

  header.append(r(
    "/* macro storing p1 ({{ f1.name }} at bit-offset o1) in p2 ({{ f2.name }} at bit-offset o2),\n"
    " * the relevant part of p2 is assumed to be clear (zero) */\n"
    "#define GP_Pixel_{{ f1.name }}_to_{{ f2.name }}_offset(p1, o1, p2, o2) do {\\\n"
    
  ## set each of <TARGET> channels
    "{% for c2 in f2.chanslist %}"
    
    # case 1: just copy a channel 
    "{%- if c2[0] in f1.chans.keys() %}{% set c1 = f1.chans[c2[0]] %}"
    "  /* {{ c2[0] }}:={{ c1[0] }} */ GP_SET_BITS({{c2[1]}}+o2, \\\n"
    "     GP_SCALE_VAL_{{c1[2]}}_{{c2[2]}}(GP_GET_BITS({{c1[1]}}+o1, {{c1[2]}}, p1)), p2); \\\n"
    
    # case 2: set A to full opacity (not present in source)
    "{% elif c2[0]=='A' %}"
    "  /* A:={{ hmask(c2[2]) }} */GP_SET_BITS({{c2[1]}}+o2, {{ hmask(c2[2]) }}, p2); \\\n"
    
    # case 3: calculate V as average of RGB 
    "{% elif c2[0]=='V' and set('RGB').issubset(set(f1.chans.keys())) %}"
    "  /* V:=RGB_avg */ GP_SET_BITS({{c2[1]}}+o2, ( \\\n"
    "{% for c1 in [f1.chans['R'], f1.chans['G'], f1.chans['B']] %}"
    "    /* {{c1[0]}} */ GP_SCALE_VAL_{{c1[2]}}_{{c2[2]}}(GP_GET_BITS({{c1[1]}}+o1, {{c1[2]}}, p1), p2) + \\\n"
    "{% endfor %}"
    "  0)/3);\\\n"
    
    #- case 4: set each RGB to V 
    "{% elif c2[0] in 'RGB' and 'V' in f1.chans.keys() %}{% set c1 = f1.chans['V'] %}"
    "  /* {{ c2[0] }}:=V */ GP_SET_BITS({{c2[1]}}+o2, \\\n"
    "     GP_SCALE_VAL_{{c1[2]}}_{{c2[2]}}(GP_GET_BITS({{c1[1]}}+o1, {{c1[2]}}, p1)), p2); \\\n"
    
    # invalid mapping (there should be none, but who knows ...) 
    "{% else %} {{ raise(Error('channel conversion' +f1.name+ ' to ' +f2.name+ ' not supported')) }}"
    
    # end of the loop
    "{% endif %}"
    "{% endfor %}"
    "    } while (0)\n\n"
    
    # add version without offsets
    "/* a version without offsets */\n"
    "#define GP_Pixel_{{ f1.name }}_to_{{ f2.name }}(p1, p2) "
      "(GP_Pixel_{{ f1.name }}_to_{{ f2.name }}_offset(p1, 0, p2, 0))",
      f1=f1, f2=f2, hmask=hmask, set=set))

