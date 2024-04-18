@ include header.t
/**
 * @file gp_gamma_correction.gen.h
 * @brief Gamma corrections.
 */
/*
 * Copyright (C) 2012-2024 Cyril Hrubis <metan@ucw.cz>
 */

@ for pt in pixeltypes:
@     if not pt.is_unknown() and not pt.is_palette():
/**
 * @brief A macro to compose a {{ pt.name }} pixel from linearized channel values.
 *
@         for c in pt.chanslist:
 * @param {{ c.name }} A {{ pt.name }} linearized {{ c.name }} channel value.
@         end
@         for c in pt.chanslist:
 * @param {{ c.name }}_gamma_enc A pointer to the #gp_gamma_table u8 or u16
 *        lookup table for the corresponding pixel channel.
@         end
 *
 * @return A {{ pt.name }} pixel value.
 */
#define GP_PIXEL_CREATE_{{ pt.name }}_ENC({{ ', '.join(pt.chan_names) }}, {{ arr_to_params(pt.chan_names, '', '_gamma_enc')  }}) \
	(0\
@         for c in pt.chanslist:
	+ (({{ c.name }}_gamma_enc ? {{ c.name }}_gamma_enc[({{ c.name }})] : {{ c.name }}) << {{ c.off }}) \
@         end
	)
@
@         for c in pt.chanslist:
/**
 * @brief Macro to get linearized {{ c.name }} channel value for {{ pt.name }} pixel.
 *
 * @param p A pixel value.
 * @param gamma_lin A pointer to the #gp_gamma_table u8 or u16 lookup table for
 *        the corresponding pixel channel.
 *
 * @return A linearized pixel channel value.
 */
#define GP_PIXEL_GET_{{ c.name }}_{{ pt.name }}_LIN(p, gamma_lin) (         \
	gamma_lin ? gamma_lin[GP_PIXEL_GET_{{ c.name }}_{{ pt.name }}(p)] : \
	            GP_PIXEL_GET_{{ c.name }}_{{ pt.name }}(p)              \
)

/**
 * @brief Returns a lookup table to linearize {{ pt.name }} pixel {{ c.name }} channel.
 *
 * @param pixmap A pixmap to get the table from.
 * @return A lookup table or NULL if channel is linear.
 */
#define GP_CHAN_TO_LIN_TBL_{{ pt.name }}_{{ c.name }}(pixmap) \
	(((pixmap)->gamma && (pixmap)->gamma->lin[{{ c.idx }}]) ? (pixmap)->gamma->lin[{{ c.idx }}]->u{{ gamma_in_bits(c) }} : NULL)

/**
 * @brief Returns a maximum value for a linearized {{ pt.name  }} pixel {{ c.name }} channel.
 *
 * The maximum value for a linearized pixel value depends on the #gp_pixmap #gp_gamma_table.
 *
 * @param pixmap A pixmap to get the maximum for.
 * @return A maximum value for the linearized pixel channel.
 */
#define GP_CHAN_LIN_MAX_VAL_{{ pt.name }}_{{ c.name }}(pixmap) \
	(((pixmap)->gamma && (pixmap)->gamma->lin[{{ c.idx }}]) ? {{ c.lin_max }} : {{ c.max }})

/**
 * @brief Returns a lookup table to encode {{ pt.name }} pixel {{ c.name }} channel.
 *
 * @param pixmap A pixmap to get the table from.
 * @return A lookup table or NULL if channel is linear.
 */
#define GP_CHAN_TO_ENC_TBL_{{ pt.name }}_{{ c.name }}(pixmap) \
	(((pixmap)->gamma && (pixmap)->gamma->enc[{{ c.idx }}]) ? (pixmap)->gamma->enc[{{ c.idx }}]->u{{ gamma_out_bits(c) }} : NULL)

@ end
