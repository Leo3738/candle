#ifndef COMPONENT_H
#define COMPONENT_H

#include <limits.h>
#include "material.h"
#include "texture.h"
#include "mesh.h"
#include "entity.h"

typedef struct ecm_t ecm_t;

typedef struct c_t c_t;

typedef void(*init_cb)(c_t *self);
typedef int(*signal_cb)(c_t *self, void *data);
typedef void(*c_reg_cb)(ecm_t *ecm);

/* TODO: find appropriate place */
typedef int(*before_draw_cb)(c_t *self);

#define WORLD 0x00
#define SAME_ENTITY 0x01

typedef struct
{
	uint signal;
	signal_cb cb;
	int flags;
} listener_t;

typedef struct
{
	uint ct;
	int is_interaction;
} dep_t;

typedef struct ct_t
{
	ecm_t *ecm;

	char name[32];

	init_cb init;

	uint id;
	uint size;

	uint *offsets;
	uint offsets_size;

	char *components;
	uint components_size;

	dep_t *depends;
	uint depends_size;

	listener_t *listeners;
	uint listeners_size;

	int is_interaction;

	/* void *system_info; */
	/* uint system_info_size; */
} ct_t;

typedef struct
{
	uint size;

	uint *cts;
	uint cts_size;

} signal_t;

typedef struct ecm_t
{
	int *entities_busy;
	uint entities_busy_size;

	ct_t *cts;
	uint cts_size;

	signal_t *signals;
	uint signals_size;

	uint global;

	entity_t none;
	entity_t common;
} ecm_t; /* Entity Component System */

typedef struct c_t
{
	uint comp_type;
	entity_t entity;
} c_t;

#define c_entity(c) (((c_t*)c)->entity)
#define c_ecm(c) (((c_t*)c)->entity.ecm)

#define _type(a, b) __builtin_types_compatible_p(typeof(a), b)
#define _if(c, a, b) __builtin_choose_expr(c, a, b)

#define IDENT_NULL UINT_MAX

#define DEC_CT(var) uint var = IDENT_NULL
#define DEC_SIG(var) uint var = IDENT_NULL
#define DEF_SIG(var) extern uint var

#define DEF_CASTER(ct, cn, nc_t) extern uint ct; \
	static inline nc_t *cn(const entity_t entity)\
{ return ct==IDENT_NULL?NULL:(nc_t*)ct_get(ecm_get(entity.ecm, ct), entity); } \

static inline c_t *ct_get_at(ct_t *self, uint i)
{
	return (c_t*)&(self->components[i * self->size]);
}

static inline c_t *ct_get(ct_t *self, entity_t entity)
{
	if(entity.id >= self->offsets_size) return NULL;
	uint offset = self->offsets[entity.id];
	if(offset == -1) return NULL;
	return (c_t*)&(self->components[offset]);
}

void ct_register_listener(ct_t *self, int flags,
		uint signal, signal_cb cb);

listener_t *ct_get_listener(ct_t *self, uint signal);

/* void ct_register_callback(ct_t *self, uint callback, void *cb); */

void ct_add(ct_t *self, c_t *comp);

ecm_t *ecm_new(void);
entity_t ecm_new_entity(ecm_t *ecm);
void ecm_register_signal(ecm_t *ecm, uint *target, uint size);

void ecm_add_entity(ecm_t *self, entity_t *entity);
/* uint ecm_register_system(ecm_t *self, void *system); */

ct_t *ecm_register(ecm_t *self, const char *name, uint *target, uint size,
		init_cb init, int depend_size, ...);

void ct_add_dependency(ct_t *ct, ct_t *dep);
void ct_add_interaction(ct_t *ct, ct_t *dep);

static inline ct_t *ecm_get(ecm_t *self, uint comp_type) {
	return &self->cts[comp_type]; }

c_t component_new(int comp_type);

/* builtin signals */
extern uint entity_created;

#endif /* !COMPONENT_H */
