@ include functions.t
@
@ def cct_header(filename, template):
@     include license.t
@     guard = filename.upper().replace('.', '_')
#ifndef {{ guard }}
#define {{ guard }}

@ def cct_footer(filename, template):
@     guard = filename.upper().replace('.', '_')
#endif /* {{ guard }} */
