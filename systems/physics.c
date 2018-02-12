#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <candle.h>
#include "physics.h"
#include "../components/spacial.h"
#include "../components/force.h"
#include "../components/velocity.h"
#include "../components/rigid_body.h"
#include "../components/light.h"

DEC_CT(ct_physics);
DEC_SIG(collider_callback);

static vec3_t c_physics_handle_forces(c_physics_t *self, vec3_t vel, float *dt)
{
	unsigned long i;

	ct_t *forces = ecm_get(c_ecm(self), ct_force);

	for(i = 0; i < forces->components_size; i++)
	{
		c_force_t *fc = (c_force_t*)ct_get_at(forces, i);
		if(fc->active) vel = vec3_add(vel, vec3_scale(fc->force, *dt));
	}

	return vel;
}

vec3_t handle_dirs(c_t *c, collider_cb cb, vec3_t old_pos, vec3_t new_pos)
{
	vec3_t res = vec3(1.0, 1.0, 1.0);
	if(cb(c, vec3(new_pos.x, old_pos.y, new_pos.z)) < 0)
	{
		/* XZ */
		res.y = 0;
	}
	else if(cb(c, vec3(old_pos.x, new_pos.y, new_pos.z)) < 0)
	{
		/* YZ */
		res.x = 0;
	}
	else if(cb(c, vec3(new_pos.x, new_pos.y, old_pos.z)) < 0)
	{
		/* XY */
		res.z = 0;
	}
	else if(cb(c, vec3(new_pos.x, old_pos.y, old_pos.z)) < 0)
	{
		/* X */
		res.y = 0;
		res.z = 0;
	}
	else if(cb(c, vec3(old_pos.x, old_pos.y, new_pos.z)) < 0)
	{
		/* Z */
		res.y = 0;
		res.x = 0;
	}
	else if(cb(c, vec3(old_pos.x, new_pos.y, old_pos.z)) < 0)
	{
		/* Y */
		res.x = 0;
		res.z = 0;
	}
	else
	{
		res.x = 0;
		res.y = 0;
		res.z = 0;
	}
	return res;
}

static float handle_cols_for_offset(c_t *c, c_t *projectile, collider_cb cb,
		vec3_t offset, vec3_t *new_pos, const vec3_t old_pos)
{
	c_node_t *node = c_node(c_entity(projectile));
	if(node)
	{
		c_node_update_model(node);
		offset = mat4_mul_vec4(node->model,
				vec4(_vec3(offset), 0.0)).xyz;
		/* offset = vec3_sub(offset, old_pos); */
		/* vec3_print(offset); */
	}
	vec3_t o_new_pos = vec3_add(offset, *new_pos);
	vec3_t o_old_pos = vec3_add(offset, old_pos);
	float f = cb(c, o_new_pos);
	if(f >= 0)
	{
		vec3_t delta = handle_dirs(c, cb, o_old_pos, o_new_pos);
		if(!delta.x) new_pos->x = old_pos.x;
		if(!delta.y) new_pos->y = old_pos.y;
		if(!delta.z) new_pos->z = old_pos.z;
		return f;
	}
	return 0.0;
}

static void c_physics_handle_collisions(c_rigid_body_t *c1,
		c_rigid_body_t *c2)
{
	collider_cb cb1 = (collider_cb)c1->costum;
	collider_cb cb2 = (collider_cb)c2->costum;

	c_t *c = NULL;
	c_t *d = NULL;
	collider_cb cb = NULL;
	if(cb1)
	{
		c = (c_t*)c1;
		d = (c_t*)c2;
		cb = cb1;
	}
	else if(cb2)
	{
		c = (c_t*)c2;
		d = (c_t*)c1;
		cb = cb2;
	}

	if(cb)
	{
		const float width = 0.20;
		c_velocity_t *vc = c_velocity(c_entity(d));
		/* if(!vc) return; */

		vec3_t offsets[] = {
			vec3( width, 0.0,  width),
			vec3( width, 0.0, -width),
			vec3(-width, 0.0,  width),
			vec3(-width, 0.0, -width),
			vec3( width, 0.8,  width),
			vec3( width, 0.8, -width),
			vec3(-width, 0.8,  width),
			vec3(-width, 0.8, -width)
		};
		int o;
		float friction = 0.0;
		for(o = 0; o < sizeof(offsets) / sizeof(*offsets); o++)
		{
			friction = fmax(handle_cols_for_offset(c, d, cb, offsets[o],
						&vc->computed_pos, vc->pre_movement_pos), friction);

		}
		if(vc->computed_pos.x == vc->pre_movement_pos.x) vc->velocity.x = 0;
		if(vc->computed_pos.y == vc->pre_movement_pos.y) vc->velocity.y = 0;
		if(vc->computed_pos.z == vc->pre_movement_pos.z) vc->velocity.z = 0;
		/* if(friction) */
		/* { */
		/*	 vec3_t dec = vec3_scale(*new_vel, friction); */
		/*	 *new_vel = vec3_sub(*new_vel, vec3_scale(dec, *dt)); */
		/* } */
	}
	else
	{
		contact_t contact;
		if(c_rigid_body_intersects(c1, c2, &contact))
		{
			c_velocity_t *v;
			if((v = c_velocity(c_entity(c1))))
			{
				v->velocity = vec3(0.0);
				v->computed_pos = v->pre_movement_pos;
			}
			if((v = c_velocity(c_entity(c2))))
			{
				v->velocity = vec3(0.0);
				v->computed_pos = v->pre_movement_pos;
			}
		}
	}
}

static int c_physics_update(c_physics_t *self, float *dt)
{
	unsigned long i, j;

	ct_t *vels = ecm_get(c_ecm(self), ct_velocity);
	ct_t *bodies = ecm_get(c_ecm(self), ct_rigid_body);

	for(i = 0; i < vels->components_size; i++)
	{
		c_velocity_t *vc = (c_velocity_t*)ct_get_at(vels, i);

		entity_t projectile = c_entity(vc);
		c_spacial_t *sc = c_spacial(projectile);

		vc->pre_movement_pos = sc->pos;
		vc->velocity = c_physics_handle_forces(self, vc->velocity, dt);

		vc->pre_collision_pos = vc->computed_pos =
			vec3_add(sc->pos, vec3_scale(vc->velocity, *dt));
	}

	for(i = 0; i < bodies->components_size; i++)
	{
		c_rigid_body_t *c1 = (c_rigid_body_t*)ct_get_at(bodies, i);
		c_velocity_t *v1 = c_velocity(c_entity(c1));

		for(j = i + 1; j < bodies->components_size; j++)
		{
			c_rigid_body_t *c2 = (c_rigid_body_t*)ct_get_at(bodies, j);
			c_velocity_t *v2 = c_velocity(c_entity(c2));

			if(!v1 && !v2) continue;

			c_physics_handle_collisions(c1, c2);
		}
	}

	for(i = 0; i < vels->components_size; i++)
	{
		c_velocity_t *vc = (c_velocity_t*)ct_get_at(vels, i);

		c_spacial_t *sc = c_spacial(c_entity(vc));

		vc->normal = vec3_sub(vc->computed_pos, vc->pre_collision_pos);
		if(vc->normal.x != vc->normal.x) vc->normal = vec3(0.0);

		c_spacial_set_pos(sc, vc->computed_pos);
	}

	return 1;
}

static void c_physics_init(c_physics_t *self)
{
	self->super = component_new(ct_physics);
}

c_physics_t *c_physics_new()
{
	c_physics_t *self = calloc(1, sizeof *self);
	c_physics_init(self);

	return self;
}

void c_physics_register(ecm_t *ecm)
{
	ct_t *ct = ecm_register(ecm, "Physics", &ct_physics,
			sizeof(c_physics_t), (init_cb)c_physics_init, 1, ct_window);

	ct_register_listener(ct, WORLD, world_update,
			(signal_cb)c_physics_update);

	ecm_register_signal(ecm, &collider_callback, 0);
}

