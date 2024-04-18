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
@ def gamma_in_bits(c):
@     if c.lin_size > 8:
@         return 16
@     else:
@         return 8
@ end
@
@ def gamma_out_bits(c):
@     if c.size > 8:
@         return 16
@     else:
@         return 8
@ end
@
@ def fetch_gamma_lin(pt, pixmap, suffix=''):
@     for c in pt.chanslist:
uint{{ gamma_in_bits(c) }}_t *{{ c.name + suffix}}_gamma_lin = GP_CHAN_TO_LIN_TBL_{{ pt.name }}_{{ c.name }}({{ pixmap }});
@     end
@ end
@
@ def fetch_gamma_enc(pt, pixmap, suffix=''):
@     for c in pt.chanslist:
uint{{ gamma_out_bits(c) }}_t *{{ c.name + suffix}}_gamma_enc = GP_CHAN_TO_ENC_TBL_{{ pt.name }}_{{ c.name }}({{ pixmap }});
@     end
@ end
@
@ def fetch_chan_lin_max(pt, pixmap):
@     for c in pt.chanslist:
gp_pixel {{ c.name }}_lin_max = GP_CHAN_LIN_MAX_VAL_{{ pt.name }}_{{ c.name }}({{ pixmap }});
@     end
@ end
@
@ def chan_lin_max(c):
{{ c.name }}_lin_max
@ end
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
