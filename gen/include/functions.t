@ from gfxprim_config import config
@ pixelpacks = config.pixelpacks
@ pixeltypes_dict = config.pixeltypes_dict
@ pixeltypes = config.pixeltypes
@ pixelpacks_by_bpp = config.pixelpacks_by_bpp
@
@ def arr_to_params(array, prefix='', suffix=''):
@     res = []
@     for i in array:
@         res.append(prefix + str(i) + suffix)
@     return ', '.join(res)
@ end
@
@ def gamma_in_bits(size):
@     if size + 2 > 8:
@         return 16
@     else:
@         return 8
@ end
@
@ def gamma_out_bits(size):
@     if size > 8:
@         return 16
@     else:
@         return 8
@ end
@
@ def fetch_gamma_tables(pt, ctx, pref="", suff=""):
/* prepare Gamma tables */
@     for c in pt.chanslist:
uint{{ gamma_in_bits(c[2]) }}_t *{{ pref + c.name }}_2_LIN{{ suff }} = NULL;
@     end
@     for c in pt.chanslist:
uint{{ gamma_out_bits(c[2]) }}_t *{{ pref + c.name }}_2_GAMMA{{ suff }} = NULL;
@     end

if ({{ ctx }}->gamma) {
@     i = 0
@     for c in pt.chanslist:
	{{ pref + c.name }}_2_LIN{{ suff }} = {{ ctx }}->gamma->lin[{{ i }}]->u{{ gamma_in_bits(c[2]) }};
@         i = i + 1
@     i = 0
@     for c in pt.chanslist:
	{{ pref + c.name }}_2_GAMMA{{ suff }} = {{ ctx }}->gamma->enc[{{ i }}]->u{{ gamma_out_bits(c[2]) }};
@         i = i + 1
@     end
}
@
@ def maybe_opts_r(opts):
@     if (opts):
@         return str(opts) + ','
@     else:
@         return ''
@
@ def maybe_opts_l(opts):
@     if (opts):
@         return ', ' + str(opts)
@     else:
@         return ''
