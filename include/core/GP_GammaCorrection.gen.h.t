%% extends "base.h.t"

{% block descr %}Gamma corrections.{% endblock %}

%% block body


extern uint16_t *GP_Gamma8_Linear10;
extern uint8_t  *GP_Linear10_Gamma8;

%% for i in range(1, 9)
static inline uint16_t GP_Gamma{{ i }}ToLinear10(uint8_t val)
{
	return GP_Gamma8_Linear10[val<<{{8 - i}}];
}

%% endfor

%% for i in range(1, 9)
static inline uint8_t GP_Linear10ToGamma{{ i }}(uint16_t val)
{
	return (GP_Linear10_Gamma8[val] + {{ int(2 ** (7 - i))}})>>{{8 - i}};
}

%% endfor

%% endblock body
