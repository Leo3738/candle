#ifndef AABB_H
#define AABB_H

#include <ecm.h>
#include <glutil.h>


typedef struct
{
	c_t super; /* extends c_t */

	vec3_t min;
	vec3_t max;

	vec3_t rot;
	vec3_t sca;
	
	int ready;

} c_aabb_t;


DEF_CASTER("aabb", c_aabb, c_aabb_t)

c_aabb_t *c_aabb_new(void);

int c_aabb_intersects(c_aabb_t *self, c_aabb_t *other);

#endif /* !AABB_H */
