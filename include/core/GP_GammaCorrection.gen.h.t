@ include header.t
/*
 * Gamma corrections
 *
 * Copyright (C) 2012-2014 Cyril Hrubis <metan@ucw.cz>
 */

extern uint16_t *gp_gamma8_linear10;
extern uint8_t  *gp_linear10_gamma8;

@ for i in range(1, 9):
static inline uint16_t gp_gamma{{ i }}_to_linear10(uint8_t val)
{
	return gp_gamma8_linear10[val<<{{8 - i}}];
}

@ end
@ for i in range(1, 9):
static inline uint8_t gp_linear10_to_gamma{{ i }}(uint16_t val)
{
	return (gp_linear10_gamma8[val] + {{ int(2 ** (7 - i))}})>>{{8 - i}};
}

static inline uint8_t gp_linear16_to_gamma{{ i }}(uint16_t val)
{
	return (gp_linear10_gamma8[val>>6] + {{ int(2 ** (7 - i))}})>>{{8 - i}};
}

@ end
