# Module with templates and tools for writing generated code, 
# especially C source and headers
#
# 2011 - Tomas Gavenciak <gavento@ucw.cz> 
#

import sys, os, time
import jinja2 

# List of known CodeGenerator classes
known_generators = []

class CodeGenerator(object):
  "Args:\n"
  "fname (required) - name of the generated file (without path)\n"
  "fdir ('') - directory prefix to match\n"
  "generating_f (None) - user function called in generate(), should generate content\n"
  "register (False) - if true, register in global generator list\n"
  def __init__(self, **kwargs):
    self.fname = kwargs.pop('fname')
    self.fdir = kwargs.pop('fdir', "")
    self.generating_f = kwargs.pop('generating_f', None)
    self.register = kwargs.pop('register', False)
    self.fname_suffix = os.path.join(self.fdir, self.fname)
    self.head = []
    self.body = []
    self.foot = []
    if self.register:
      known_generators.append(self)
  def matches_path(self, path):
    return path.endswith(self.fname_suffix)
  def r(self, s, *args, **kwargs):
    return j2render(s, g=self, *args, **kwargs)
  def rhead(self, *args, **kwargs):
    self.head.append(self.r(*args, **kwargs))
  def rbody(self, *args, **kwargs):
    self.body.append(self.r(*args, **kwargs))
  def rfoot(self, *args, **kwargs):
    self.foot.append(self.r(*args, **kwargs))
  def generate(self, run_gen_base=True):
    self.head = []
    self.body = []
    self.foot = []
    if run_gen_base:
      self.gen_base()
    # Run user-specified generation 
    if self.generating_f:
      self.generating_f(self)
    return ''.join(self.head + self.body + self.foot)
  def gen_base(self):
    "Fill basic subclass-dependent content into head/body/foot"
    pass

class CCodeGenerator(CodeGenerator):
  "Args:\n"
  "authors ([]) - list of author credits\n"
  "descr ("") - (multiline) file description\n"
  def __init__(self, **kwargs):
    super(CCodeGenerator, self).__init__(**kwargs)
    self.authors = kwargs.pop('authors', [])
    self.descr = kwargs.pop('descr', "")
  def gen_base(self):
    super(CCodeGenerator, self).gen_base()
    self.head.append(self.r(
      "/*\n"
      " * {{ g.fname }}\n"
      " *\n"
      " * GENERATED on {{ date }} for suffix \"{{ g.fname_suffix }}\"\n"
      " * DO NOT MODIFY THIS FILE DIRECTLY!\n"
      " *\n"
      "{% if g.descr %}"
	" * {{ g.descr }}\n *\n"
      "{% endif %}"
      "{% for a in g.authors %}"
	" * {{ a }}\n"
      "{% endfor %}"
      " */\n\n", date = time.ctime()))

class CSourceGenerator(CCodeGenerator):
  def __init__(self, **kwargs):
    print kwargs
    super(CSourceGenerator, self).__init__(**kwargs)

def c_source_generator(*args, **kwargs):
  "Decorator to create C source generators."
  def decorate(f):
    kwargs['generating_f'] = f
    kwargs['register'] = True
    g = CSourceGenerator(*args, **kwargs)
    return f
  return decorate

class CHeaderGenerator(CCodeGenerator):
  def __init__(self, **kwargs):
    super(CHeaderGenerator, self).__init__(**kwargs)
    self.hdef = 'GP_HEADER_' + self.fname_suffix.replace('.', '_').replace('/', '_').upper()
  def gen_base(self):
    super(CHeaderGenerator, self).gen_base()
    self.head.append(self.r(
      "#ifndef {{ g.hdef }}\n"
      "#define {{ g.hdef }}\n\n"))
    self.foot.append(self.r(
      "#endif /* {{ g.hdef }} */\n"))

def c_header_generator(*args, **kwargs):
  "Decorator to create C header generators."
  def decorate(f):
    kwargs['generating_f'] = f
    kwargs['register'] = True
    g = CHeaderGenerator(*args, **kwargs)
    return f
  return decorate
