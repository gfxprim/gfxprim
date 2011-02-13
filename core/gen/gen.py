## Generation of pixel formats, basic functions and macros

# pixel format:
#   type
#   - PAL
#   - HSVRGBA
#   per chan
#     name, pos, size
#   bitwidth
#   name

# gen:
#  (s)print
#  conversions
#  

import re, jinja2


# Helper to render jinja2 templates (with strict undefined)
def r(tmpl, **kw):
  return jinja2.Template(tmpl, undefined=jinja2.StrictUndefined).render(**kw)

# Helper returning hex mask for that many bits
def hmask(bits): return hex((1<<bits)-1)

# list of all formats
formats = {}
channels = set()

class PixelFormat(object):
  def __init__(self, name, size, chanslist):
    # name must be valid C identifier
    assert re.match('\A[A-Za-z][A-Za-z0-9_]*\Z', name)
    self.name = name 
    self.chanslist = chanslist # [ (chan_name, offset, size) ]
    self.chans = dict() # { chan_name: (offset, size) }
    assert(size in [1,2,4,8,16,24,32]) # pixel size in bits
    self.size = size
  
    # Verify channel bits
    self.bits = ['x']*size 
    for c in chanslist:
      assert c[0] not in self.chans.keys()
      self.chans[c[0]] = c
      for i in range(c[1],c[1]+c[2]):
	assert(i<self.size)
	assert(self.bits[i]=='x')
	self.bits[i] = c[0]

    assert self.name not in formats.keys()
    formats[self.name] = self
    channels.update(set(self.chans.keys()))

  def g_start(self):
    return r(
    """/* Automatically generated code for pixel type {{ f.name }}
 * 
 * Pixel structure: {{ ''.join(f.bits) }}
 * Channels: {% for c in f.chanslist %} 
 *   {{ c[0] }}  offset:{{ c[1] }} size:{{ c[2] }}{% endfor %}
 */
\n""", f=self)

  def g_print(self, header, code):
    header.append(r(
    """/* print formatted value of pixel type {{ f.name }} */
void GP_Pixel_print_{{ f.name }}(GP_Pixel p);
\n""", f=self))
    code.append(r(
    """/* print formatted value of pixel type {{f.name}} */
void GP_Pixel_print_{{ f.name }}(GP_Pixel p) 
{
  printf("<{{ f.name }} %0{{ (f.size+3)//4 }}x{% for c in f.chanslist %} {{ c[0] }}=%d{% endfor %}>", 
    GP_GET_BITS(0, {{ f.size }}, p){% for c in f.chanslist %}, GP_GET_BITS({{ c[1] }}, {{ c[2] }}, p){% endfor %});
}
\n""", f=self))

  def g_get_chs(self, header, code):
    header.append(r(
    """/* macros to get channels of pixel type {{ f.name }} */
{% for c in f.chanslist -%}
#define GP_Pixel_get_{{ c[0] }}_{{ f.name }}(p) (GP_GET_BITS({{ c[1] }}, {{ c[2] }}, (p)))
{% endfor -%}
\n""", f=self))

  # supports only conversions inside RGB/V(A)
  def g_convert_to(f1, f2, header, code):
    allowed_chansets = [ set(list(s)) for s in ['RGB', 'RGBA', 'V', 'VA'] ]
    assert(set(f1.chans.keys()) in allowed_chansets)
    assert(set(f2.chans.keys()) in allowed_chansets)

    header.append(r("""
/* macro storing p1 ({{ f1.name }} at bit-offset o1) in p2 ({{ f2.name }} at bit-offset o2), 
 * the relevant part of p2 must be cleared (zero) */
#define GP_Pixel_{{ f1.name }}_to_{{ f2.name }}_offset(p1, o1, p2, o2) do {\\\n{# #}

{#- set each TARGET channels -#}
{% for c2 in f2.chanslist -%}

{#- case 1: copy a channel -#}
{%- if c2[0] in f1.chans.keys() %}{% set c1 = f1.chans[c2[0]] -%}
{# #}  /* {{ c2[0] }}:={{ c1[0] }} */ GP_SET_BITS({{c2[1]}}+o2, {# -#}
         GP_SCALE_VAL_{{c1[2]}}_{{c2[2]}}(GP_GET_BITS({{c1[1]}}+o1, {{c1[2]}}, p1)), p2); \\\n{# #}

{#- case 2: set A to full opacity -#}
{% elif c2[0]=='A' -%}
{# #}  /* A:={{ hmask(c2[2]) }} */GP_SET_BITS({{c2[1]}}+o2, {{ hmask(c2[2]) }}, p2); \\\n{# #}

{#- case 3: calculate V as average of RGB -#}
{% elif c2[0]=='V' and set('RGB').issubset(set(f1.chans.keys())) -%}
{# #}  /* V:=RGB_avg */ GP_SET_BITS({{c2[1]}}+o2, (\\\n{# #}
 {%- for c1 in [f1.chans['R'], f1.chans['G'], f1.chans['B']] -%}
{# #}    /* {{c1[0]}} */ GP_SCALE_VAL_{{c1[2]}}_{{c2[2]}}(GP_GET_BITS({{c1[1]}}+o1, {{c1[2]}}, p1), p2) + \\\n{# #}
  {%- endfor -%}
{# #}  0)/3);\\\n{# #}

{#- case 4: set each RGB to V -#}
{% elif c2[0] in 'RGB' and 'V' in f1.chans.keys() -%}{% set c1 = f1.chans['V'] -%}
{##}  /* {{ c2[0] }}:=V */ GP_SET_BITS({{c2[1]}}+o2, {# -#}
  GP_SCALE_VAL_{{c1[2]}}_{{c2[2]}}(GP_GET_BITS({{c1[1]}}+o1, {{c1[2]}}, p1)), p2); \\\n{# #}

{#- invalid mapping (there should be none, but who knows) -#}
{% else %} {{ raise(Error("channel conversion" +f1.name+ " to " +f2.name+ " not supported")) }}

{%- endif -%}
{% endfor -%}
    } while (0)

/* a variant without offsets */
#define GP_Pixel_{{ f1.name }}_to_{{ f2.name }}(p1, p2) (GP_Pixel_{{ f1.name }}_to_{{ f2.name }}_offset(p1, 0, p2, 0))
\n""", f1=f1, f2=f2, hmask=hmask, set=set))


c = []
h = []

h.append(r("""
/* TEMP def of base type */
typedef unsigned long int GP_Pixel;

/* helper macro to read bits */
/* WONDER: is 1ULL a good (fast) idea? */
#define GP_GET_BITS(offset, bits, val) ( ( (val)>>(offset) ) & ( (1ULL<<(bits)) - 1) ) 

/* helper macro to or-mask bits (set if zero beforehead), assuming val fits into bits */
#define GP_OR_BITS(offset, or, val) ( (val) |= ((bits)<<(offset)) )

/* helper macros to transfer s1-bit value to s2-bit value 
 * NOTE and WARNING: scaling is efficient and accurate for upscaling, 
 *                   but requires actual numebrs as the parameters !!! */
#define GP_SCALE_VAL(s1, s2, val) ( GP_SCALE_VAL_##s1##_##s2(val) )
{% for s1 in [1, 2, 4, 8] %}{% for s2 in [1, 2, 4, 8] -%}
{% if s2>s1 -%}
#define GP_SCALE_VAL_{{s1}}_{{s2}}(val) ((val) * {{ multcoef(s1, s2) }})
{% else -%}
#define GP_SCALE_VAL_{{s1}}_{{s2}}(val) ((val) >> {{ s1 - s2 }})
{% endif -%}
{% endfor %}{% endfor %}
\n""", multcoef = lambda s1,s2: hex(sum([1<<i*s1 for i in range(s2/s1)])) ))

# TODO:
#/* helper macro to set bits (clearing them first), size is bit-size of the type */
#define GP_SET_BITS(size, offset, bits, val) ( (val) &= ( ((1<<(bits))-1) - ((1<<(bits))-1))   ( (val)>>(offset) ) & ( (1<<(bits)) - 1) )
#"""]

PixelFormat(name='RGBx8888', size=32, chanslist=[
    ('R', 0, 8),
    ('G', 8, 8),
    ('B', 16, 8)])

PixelFormat(name='RGBA8888', size=32, chanslist=[
    ('R', 0, 8),
    ('G', 8, 8),
    ('B', 16, 8),
    ('A', 24, 8)])

PixelFormat(name='RGB565', size=16, chanslist=[
    ('R', 0, 5),
    ('G', 5, 6),
    ('B', 11, 5)])

PixelFormat(name='V8', size=8, chanslist=[
    ('V', 0, 8)])

PixelFormat(name='V2', size=2, chanslist=[
    ('V', 0, 2)])

PixelFormat(name='VA12', size=4, chanslist=[
    ('A', 1, 2),
    ('V', 3, 1)])

for f in formats.values():
  h.append(f.g_start())
  f.g_print(h,c)
  f.g_get_chs(h,c)

formats['RGB565'].g_convert_to(formats['RGBA8888'], h, c)
formats['RGBA8888'].g_convert_to(formats['V2'], h, c)
formats['VA12'].g_convert_to(formats['RGBA8888'], h, c)

print "/**** HEADER */\n"
print '\n'.join(h)
print "/**** CODE */\n#include<stdio.h>"
print '\n'.join(c)

