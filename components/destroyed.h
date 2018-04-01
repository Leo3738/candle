#ifndef DESTROYED_H
#define DESTROYED_H

#include <ecm.h>

typedef struct
{
	c_t super; /* extends c_t */
	int steps;
} c_destroyed_t;

DEF_CASTER("c_destroyed", c_destroyed, c_destroyed_t)

c_destroyed_t *c_destroyed_new(void);

#endif /* !DESTROYED_H */
