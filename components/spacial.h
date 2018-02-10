#ifndef SPACIAL_H
#define SPACIAL_H

#include <ecm.h>
#include "../glutil.h"

typedef struct
{
	c_t super; /* extends c_t */
	vec3_t forward;
	vec3_t sideways;
	vec3_t upwards;

	vec3_t pos;
	vec3_t rot;
	vec3_t scale;
	vec3_t up;
	mat4_t rot_matrix;
	mat4_t model_matrix;
} c_spacial_t;

extern unsigned long ct_spacial;

DEF_CASTER(ct_spacial, c_spacial, c_spacial_t)

c_spacial_t *c_spacial_new(void);
void c_spacial_init(c_spacial_t *self);
void c_spacial_register(ecm_t *ecm);
vec3_t c_spacial_up(c_spacial_t *self);

void c_spacial_scale(c_spacial_t *self, vec3_t scale);
void c_spacial_look_at(c_spacial_t *self, vec3_t eye, vec3_t center, vec3_t up);
void c_spacial_set_pos(c_spacial_t *self, vec3_t pos);
void c_spacial_set_rot(c_spacial_t *self, float x, float y, float z, float angle);
void c_spacial_update_model_matrix(c_spacial_t *self);

void c_spacial_rotate_X(c_spacial_t *self, float angle);
void c_spacial_rotate_Y(c_spacial_t *self, float angle);
void c_spacial_rotate_Z(c_spacial_t *self, float angle);

extern unsigned long spacial_changed;

#endif /* !SPACIAL_H */
