#include "decal.h"
#include "spacial.h"
#include "candle.h"
#include <components/mesh_gl.h>
#include <components/model.h>
#include <components/node.h>
#include <stdlib.h>


void c_decal_init(c_decal_t *self) { } 

mesh_t *g_decal_mesh = NULL;

c_decal_t *c_decal_new(mat_t *mat)
{
	c_decal_t *self = component_new(ct_decal);

	self->visible = 1;
	self->mat = mat;

	if(!g_decal_mesh)
	{
		g_decal_mesh = sauces_mesh("cube.ply");
	}

	if(!c_mesh_gl(self))
	{
		entity_add_component(c_entity(self),
				c_model_new(g_decal_mesh, mat, 0));
		c_model(self)->visible = 0;
	}

	return self;
}

int c_decal_render(c_decal_t *self)
{
	return c_model_render(c_model(self));
}

DEC_CT(ct_decal)
{
	ct_t *ct = ct_new("c_decal", &ct_decal, sizeof(c_decal_t),
			(init_cb)c_decal_init, 1, ct_spacial);

	ct_listener(ct, WORLD, render_decals, c_decal_render);
}


