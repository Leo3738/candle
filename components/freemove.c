#include "freemove.h"
#include <candle.h>
#include <components/spacial.h>
#include <components/velocity.h>
#include <keyboard.h>
#include <math.h>

extern int window_width, window_height;

#define friction 0.1

static void c_freemove_init(c_freemove_t *self)
{

	self->plane_movement = 0;
	self->forward = 0;
	self->backward = 0;
	self->left = 0;
	self->right = 0;
	self->orientation = entity_null;
	self->force_down = entity_null;
}

c_freemove_t *c_freemove_new(entity_t orientation, int plane_movement, entity_t force_down)
{
	c_freemove_t *self = component_new("freemove");
	self->plane_movement = plane_movement;
	self->force_down = force_down;

	self->orientation = orientation;

	return self;
}

static int c_freemove_update(c_freemove_t *self, float *dt)
{
	/* vec3_t rot = c_spacial(self->orientation)->rot; */
	c_velocity_t *vc = c_velocity(self);
	vec3_t *vel = &vc->velocity;
	float accel = 30 * (*dt);

	c_spacial_t *sc = c_spacial(self);

	vec3_t front;
	vec3_t sideways;

	front = vec3_sub(mat4_mul_vec4(sc->model_matrix,
				vec4(0.0, 0.0, 1.0, 1.0)).xyz, sc->pos);
	sideways = vec3_sub(mat4_mul_vec4(sc->model_matrix,
				vec4(1.0, 0.0, 0.0, 1.0)).xyz, sc->pos);


	if((self->left + self->right) && (self->forward || self->backward))
	{
		accel /= sqrtf(2);
	}

	front = vec3_scale(front, accel);
	sideways = vec3_scale(sideways, accel);

	if(self->left)
	{
		*vel = vec3_sub(*vel, sideways);
	}

	if(self->right)
	{
		*vel = vec3_add(*vel, sideways);
	}

	if(self->forward)
	{
		*vel = vec3_sub(*vel, front);
	}

	if(self->backward)
	{
		*vel = vec3_add(*vel, front);
	}

	entity_signal(self->super.entity, sig("spacial_changed"), &self->super.entity);

	return 1;
}

static int c_freemove_key_up(c_freemove_t *self, char *key)
{
	switch(*key)
	{
		case 'W': case 'w': self->forward = 0; break;
		case 'A': case 'a': self->left = 0; break;
		case 'D': case 'd': self->right = 0; break;
		case 'S': case 's': self->backward = 0; break;
	}
	return 1;
}

static int c_freemove_key_down(c_freemove_t *self, char *key)
{
	switch(*key)
	{
		case 'W': case 'w': self->forward = 1; break;
		case 'A': case 'a': self->left = 1; break;
		case 'D': case 'd': self->right = 1; break;
		case 'S': case 's': self->backward = 1; break;
		default: printf("key: %d pressed\n", *key); break;
	}
	return 1;
}

REG()
{
	ct_t *ct = ct_new("freemove", sizeof(c_freemove_t),
			(init_cb)c_freemove_init, 2, ref("spacial"), ref("velocity"));

	ct_listener(ct, WORLD, sig("key_up"), c_freemove_key_up);

	ct_listener(ct, WORLD, sig("key_down"), c_freemove_key_down);

	ct_listener(ct, WORLD, sig("world_update"), c_freemove_update);
}

