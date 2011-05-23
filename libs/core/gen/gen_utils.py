# Module with utils for writing C source and headers
# 2011 - Tomas Gavenciak <gavento@ucw.cz> 

import sys
import jinja2 

def die(msg):
  sys.stderr.write(msg)
  sys.exit(1)


def hmask(bits): 
  "Helper returning hex mask for that many bits"
  "WARN: may contain 'L'"
  return hex((1<<bits)-1)


def r(tmpl, **kw):
  "Internal helper to render jinja2 templates (with StrictUndefined)"
  t2 = tmpl.rstrip('\n') # Jinja strips the last '\n', so add these later
  return jinja2.Template(t2, undefined=jinja2.StrictUndefined).render(**kw) + tmpl[len(t2):]


def gen_headers(header, code, descr, authors, generator, hdef):
  "Generate header- and source-file headers"
  "TODO - add license"
  header.append(r(
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
  code.append(r(
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


#import pprint
def main_write(header, code):
  "Helper writing header to argv[1] and code to argv[2]"
  "Skips writing header resp. code if set to None"
  "Expects as many argv params as not-None arguments"

  # Some after-midnight functional crazines:
  proc = [a for a in [header, code] if a is not None]
  if len(sys.argv) != len(proc) + 1:
    die("Generator expects %d parameters (output file names).\n" % len(proc))
  for i in range(len(proc)):
    p = proc[i]
    fname = sys.argv[i+1]
    sys.stderr.write("Writing GP_Pixel generated " + ("header" if p==header else "source") + 
	" to " + fname + "\n")
    f = open(fname, "wt")
    f.write('\n'.join(p))
    f.close()

