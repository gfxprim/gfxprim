# Module with utils for writing C source and headers
# 2011 - Tomas Gavenciak <gavento@ucw.cz> 

import sys
import jinja2 
import gfxprim
from gfxprim import die

def hmask(bits): 
  "Helper returning hex mask for that many bits"
  "WARN: may contain 'L'"
  return hex((1<<bits)-1)


def j2render(tmpl, **kw):
  "Internal helper to render jinja2 templates (with StrictUndefined)"
  t2 = tmpl.rstrip('\n') # Jinja strips the last '\n', so add these later
  return jinja2.Template(t2, undefined=jinja2.StrictUndefined).render(**kw) + tmpl[len(t2):]


def gen_headers(header, code, descr, authors, generator, hdef):
  "Generate header- and source-file headers"
  "TODO - add license"
  header.append(j2render(
    "/* GENERATED header -- {{ descr }}\n"
    " * DO NOT MODIFY THIS FILE DIRECTLY! *\n"
    " *\n"
    " * generator: {{ generator }}\n"
    "{% for a in authors %}"
    " * {{ a }}\n"
    "{% endfor %}"
    " */\n\n"
    "#ifndef {{ hdef }}\n"
    "#define {{ hdef }}\n\n", descr=descr, authors=authors, 
      generator=generator, hdef=hdef))
  code.append(j2render(
    "/* GENERATED header -- {{ descr }}\n"
    " * DO NOT MODIFY THIS FILE DIRECTLY! *\n"
    " *\n"
    " * generator: {{ generator }}\n"
    "{% for a in authors %}"
    " * {{ a }}\n"
    "{% endfor %}"
    " */\n\n", descr=descr, authors=authors, 
      generator=generator))


def gen_footers(header, code):
  "Generate header-file footer"
  header.append("#endif /* Header file #include guard */\n")


def main_write(*files):
  "Helper writing files to argv[1:], "
  "usually header to argv[1] and code to argv[2].\n"
  "Skips writing header resp. code if set to None"
  "Expects as many argv params as not-None arguments"
  if len(sys.argv) != len(files) + 1:
    die("Generator expects %d parameters (output file names).\n" % len(files))
  for i in range(len(files)):
    p = files[i]
    fname = sys.argv[i+1]
    sys.stderr.write("Writing generated source to " + fname + "\n")
    f = open(fname, "wt")
    f.write('\n'.join(p))
    f.close()

