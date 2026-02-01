#include <core/gp_core.h>
#include <gfx/gp_gfx.h>
#include <loaders/gp_loaders.h>
#include <backends/gp_backends.h>
#include <input/gp_input.h>

/*
 * The event structure is correctly aligned accordingly to C but rust wants
 * maximal machine alignment for all structures, hence we need to get the
 * values in C.
 */
static inline uint32_t gp_ev_get_type(const gp_event *ev)
{
	return ev->type;
}

static inline uint32_t gp_ev_get_code(const gp_event *ev)
{
	return ev->code;
}

static inline uint32_t gp_ev_new_w(const gp_event *ev)
{
	return ev->resize.w;
}

static inline uint32_t gp_ev_new_h(const gp_event *ev)
{
	return ev->resize.h;
}
