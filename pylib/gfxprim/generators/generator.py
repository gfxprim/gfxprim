#
# Module with templates and tools for writing generated code, 
# especially C source and headers
#
# 2011 - Tomas Gavenciak <gavento@ucw.cz> 
#

import sys, os, time
from gfxprim import die
from gfxprim.generators.utils import j2render

# List of known CodeGenerator instances
known_generators = []

class CodeGenerator(object):
  "Args:\n"
  "fname (required) - name of the generated file (without path)\n"
  "fdir ('') - directory prefix to match\n"
  "generating_f (None) - user function called in generate(), should generate content\n"
  "register (False) - if true, register in global generator list\n"
  generator_attributes__ = ['generating_f', 'register']
  def __init__(self, **kwargs):
    self.name = kwargs.pop('name')
    self.fdir, self.fname = os.path.split(self.name)
    self.generating_f = None
    self.register = False
    self.head = []
    self.body = []
    self.foot = []
    self.setargs(**kwargs)
    if kwargs:
      die('Unknown arguments to CodeGenerator: %s' & str(kwargs.keys()))
  def setargs(self, **kwargs):
    for i in self.generator_attributes__:
      if i in kwargs: self.__setattr__(i, kwargs.pop(i))
  def matches_path(self, path):
    return path.endswith(self.name)
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
    # Run user-specified generation f
    if self.generating_f:
      f, pos, params = self.generating_f
      args = [self if i == pos else global_null_generator for i in range(params)]
      f(*args)
    return ''.join(self.head + self.body + self.foot)
  def gen_base(self):
    "Fill basic subclass-dependent content into head/body/foot"
    pass

class NullGenerator(CodeGenerator):
  def rhead(self, *args, **kwargs): pass
  def rbody(self, *args, **kwargs): pass
  def rfoot(self, *args, **kwargs): pass
  def generate(self, *args, **kwargs): return ''
  def r(self, *args, **kwargs): return ''

global_null_generator = NullGenerator(name='null_generator')

class CCodeGenerator(CodeGenerator):
  "Args:\n"
  "authors ([]) - list of author credits\n"
  "descr ("") - (multiline) file description\n"
  generator_attributes__ = ['authors', 'descr'] + CodeGenerator.generator_attributes__
  def __init__(self, **kwargs):
    self.authors = []
    self.descr = ""
    super(CCodeGenerator, self).__init__(**kwargs)
  def gen_base(self):
    super(CCodeGenerator, self).gen_base()
    self.head.append(self.r(
      "/*\n"
      " * {{ g.fname }}\n"
      " *\n"
      " * GENERATED on {{ date }} by generator \"{{ g.name }}\"\n"
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
    super(CSourceGenerator, self).__init__(**kwargs)

class CHeaderGenerator(CCodeGenerator):
  def __init__(self, **kwargs):
    super(CHeaderGenerator, self).__init__(**kwargs)
    self.hdef = 'GP_HEADER_' + self.name.replace('.', '_').replace('/', '_').upper()
  def gen_base(self):
    super(CHeaderGenerator, self).gen_base()
    self.head.append(self.r(
      "#ifndef {{ g.hdef }}\n"
      "#define {{ g.hdef }}\n\n"))
    self.foot.append(self.r(
      "#endif /* {{ g.hdef }} */\n"))

def generator(*args, **kwargs):
  "Decorate functions to be content-creator for given generators.\n"
  "By default also registers the generator to pool of known generators."
  register = kwargs.pop('register', True)
  def decorate(f):
    for i in range(len(args)):
      kwargs['generating_f'] = (f, i, len(args))
      args[i].setargs(**kwargs)
      if register:
	known_generators.append(args[i])
    return f
  return decorate
