#include "editmode.h"
#include <candle.h>
#include <nk.h>
#include <keyboard.h>
#include <stdlib.h>
#include "renderer.h"

DEC_CT(ct_editmode);

DEC_SIG(global_menu);
DEC_SIG(component_menu);

#define MAX_VERTEX_MEMORY 512 * 1024
#define MAX_ELEMENT_MEMORY 128 * 1024


void c_editmode_init(c_editmode_t *self)
{
	self->super = component_new(ct_editmode);
	self->control = 1;
	self->visible = 0;
	self->dragging = 0;
	self->pressing = 0;
	self->mouse_position = vec3(0.0f);
	/* self->outside = 0; */
	self->nkctx = NULL;
	self->selected = entity_null();
	self->over = entity_null();
}

vec3_t bind_selected(entity_t caller)
{
	vec3_t id_color = int_to_vec3(c_editmode(caller)->over.id);

	return id_color;
}


c_editmode_t *c_editmode_new()
{
	c_editmode_t *self = malloc(sizeof *self);
	c_editmode_init(self);

	return self;
}


static int c_editmode_resize(c_editmode_t *self)
{
	if(!self->nkctx)
	{
		self->nkctx = nk_sdl_init(c_window(c_entity(self))->window); 

		{ 
			struct nk_font_atlas *atlas; 
			nk_sdl_font_stash_begin(&atlas); 
			/* struct nk_font *droid = nk_font_atlas_add_from_file(atlas, "../../../extra_font/DroidSans.ttf", 14, 0); */
			/* struct nk_font *roboto = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Roboto-Regular.ttf", 16, 0); */
			/* struct nk_font *future = nk_font_atlas_add_from_file(atlas, "../../../extra_font/kenvector_future_thin.ttf", 13, 0); */
			/* struct nk_font *clean = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyClean.ttf", 12, 0); */
			/* struct nk_font *tiny = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyTiny.ttf", 10, 0); */
			/* struct nk_font *cousine = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Cousine-Regular.ttf", 13, 0); */
			nk_sdl_font_stash_end(); 
			/* nk_style_load_all_cursors(self->nkctx, atlas->cursors); */
			/* nk_style_set_font(self->nkctx, &roboto->handle); */
		} 
		c_renderer_add_pass(c_renderer(c_entity(self)), "hightlight",
				PASS_SCREEN_SCALE,
				1.0f, 1.0f, "highlight",
					(bind_t[]){
				{BIND_GBUFFER, "gbuffer", .gbuffer.name = "opaque"},
				{BIND_VEC3, "id_color", .getter = (ptr_getter)bind_selected,
				.entity = c_entity(self)},
				{BIND_NONE}
			}
		);

	}

	return 1;
}

int c_editmode_mouse_move(c_editmode_t *self, mouse_move_data *event)
{
	entity_t camera = ecm_get_camera(c_ecm(self));
	c_camera_t *cam = c_camera(camera);
	c_renderer_t *renderer = c_renderer(c_entity(self));

	float px = event->x / renderer->width;
	float py = 1.0f - event->y / renderer->height;
	if(self->control && !candle->pressing)
	{
		if(self->pressing)
		{
			c_spacial_t *sc = c_spacial(self->selected);
			if(!self->dragging)
			{
				self->dragging = 1;
				self->drag_diff = vec3_sub(sc->pos, self->mouse_position);
			}
			vec3_t pos = c_camera_real_pos(cam, self->mouse_depth, vec2(px, py));

			vec3_t new_pos = vec3_add(self->drag_diff, pos);
			c_spacial_set_pos(sc, new_pos);
		}
		else
		{
			entity_t result = c_renderer_entity_at_pixel(renderer,
					event->x, event->y, &self->mouse_depth);

			vec3_t pos = c_camera_real_pos(cam, self->mouse_depth, vec2(px, py));
			self->mouse_position = pos;

			if(result.id > 0)
			{
				self->over = result;
			}
			else
			{
				self->over = entity_null();
			}
		}
	}
	return 1;
}

int c_editmode_mouse_press(c_editmode_t *self, mouse_button_data *event)
{
	if(event->button == SDL_BUTTON_LEFT)
	{
		self->pressing = 1;

		entity_t result = c_renderer_entity_at_pixel(c_renderer(c_entity(self)),
				event->x, event->y, NULL);
		/* TODO entity 0 should be valid */
		if(result.id > 0)
		{
			self->selected = result;
		}
		else
		{
			self->selected = entity_null();
		}
	}
	return 1;
}

int c_editmode_mouse_release(c_editmode_t *self, mouse_button_data *event)
{
	if(self->dragging)
	{
		self->dragging = 0;
		self->selected = entity_null();
	}
	self->pressing = 0;
	return 1;
}


int c_editmode_key_up(c_editmode_t *self, char *key)
{
	switch(*key)
	{
		case '`':
			{
				self->control = !self->control;
				if(!self->control)
				{
					self->over = entity_null();
				}
				break;
			}
	}
	return 1;
}

int c_editmode_key_down(c_editmode_t *self, char *key)
{
	/* switch(*key) */
	/* { */
	/* } */
	return 1;
}

void node_node(c_editmode_t *self, c_node_t *node)
{
	char buffer[64];
	char *final_name = buffer;
	const entity_t entity = c_entity(node);
	c_name_t *name = c_name(entity);
	if(name)
	{
		final_name = name->name;
	}
	else
	{
		sprintf(buffer, "%ld", entity.id);
	}
	if(nk_tree_push_id(self->nkctx, NK_TREE_NODE, final_name, NK_MINIMIZED,
				entity.id))
	{
		if(nk_button_label(self->nkctx, "select"))
		{
			self->selected = entity;
		}
		int i;
		for(i = 0; i < node->children_size; i++)
		{
			node_node(self, c_node(node->children[i]));
		}
		nk_tree_pop(self->nkctx);
	}
}

void node_tree(c_editmode_t *self)
{
	int i;
	ct_t *nodes = ecm_get(c_ecm(self), ct_node);

	if(nk_tree_push(self->nkctx, NK_TREE_TAB, "nodes", NK_MINIMIZED))
	{
		for(i = 0; i < nodes->components_size; i++)
		{
			c_node_t *node = (c_node_t*)ct_get_at(nodes, i);
			if(node->parent.id != -1) continue;
			node_node(self, node);
		}
		nk_tree_pop(self->nkctx);
	}
}


int c_editmode_draw(c_editmode_t *self)
{
	if(self->nkctx && (self->visible || self->control))
	{
		if (nk_begin(self->nkctx, "clidian", nk_rect(50, 50, 230, 180),
					NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
					NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
		{

			nk_layout_row_static(self->nkctx, 30, 110, 1);
			if (nk_button_label(self->nkctx, "redraw probes"))
			{
				c_renderer_scene_changed(c_renderer(c_entity(self)));
			}
			entity_signal(c_entity(self), global_menu, self->nkctx);

			node_tree(self);
		}
		nk_end(self->nkctx);

		if(!entity_is_null(self->selected))
		{
			char buffer[64];
			char *final_name = buffer;
			c_name_t *name = c_name(self->selected);
			if(name)
			{
				final_name = name->name;
			}
			else
			{
				sprintf(buffer, "%ld", self->selected.id);
			}
			if (nk_begin(self->nkctx, final_name, nk_rect(300, 50, 230, 280),
						NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
						NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
			{
				int i;

				signal_t *sig = &c_ecm(self)->signals[component_menu];

				for(i = 0; i < sig->cts_size; i++)
				{
					ct_t *ct = ecm_get(c_ecm(self), sig->cts[i]);
					c_t *comp = ct_get(ct, self->selected);
					if(comp && !ct->is_interaction)
					{
						if(nk_tree_push_id(self->nkctx, NK_TREE_TAB, ct->name,
									NK_MINIMIZED, i))
						{
							component_signal(comp, ct, component_menu, self->nkctx);
							nk_tree_pop(self->nkctx);
						}
						int j;
						for(j = 0; j < ct->depends_size; j++)
						{
							if(ct->depends[j].is_interaction)
							{
								c_t *inter = ct_get(ct, self->selected);
								ct_t *inter_ct = ecm_get(c_ecm(self),
										ct->depends[j].ct);
								component_signal(inter, inter_ct,
										component_menu, self->nkctx);
							}
						}
					}
				}
			}
			nk_end(self->nkctx);
		}
		nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);
	}
	return 1;
}

int c_editmode_events_begin(c_editmode_t *self)
{
	if(self->nkctx) nk_input_begin(self->nkctx);
	return 1;
}

int c_editmode_events_end(c_editmode_t *self)
{
	if(self->nkctx) nk_input_end(self->nkctx);
	return 1;
}

int c_editmode_event(c_editmode_t *self, SDL_Event *event)
{

	if(self->nkctx)
	{
		nk_sdl_handle_event(event);
		if(nk_window_is_any_hovered(self->nkctx))
		{
			self->over = entity_null();
			return 0;
		}
		if(nk_item_is_any_active(self->nkctx))
		{
			return 0;
		}
	}

	return 1;
}

void c_editmode_register(ecm_t *ecm)
{
	ct_t *ct = ecm_register(ecm, "EditMode", &ct_editmode,
			sizeof(c_editmode_t), (init_cb)c_editmode_init, 0);

	ecm_register_signal(ecm, &global_menu, sizeof(struct nk_context*));
	ecm_register_signal(ecm, &component_menu, sizeof(struct nk_context*));

	ct_register_listener(ct, WORLD, key_up, (signal_cb)c_editmode_key_up);

	ct_register_listener(ct, WORLD, key_down, (signal_cb)c_editmode_key_down);

	/* ct_register_listener(ct, WORLD, world_update, */
	/*		 (signal_cb)c_editmode_update); */

	ct_register_listener(ct, WORLD, mouse_move,
			(signal_cb)c_editmode_mouse_move);

	ct_register_listener(ct, WORLD, world_draw,
			(signal_cb)c_editmode_draw);

	ct_register_listener(ct, WORLD, mouse_press,
			(signal_cb)c_editmode_mouse_press);

	ct_register_listener(ct, WORLD, mouse_release,
			(signal_cb)c_editmode_mouse_release);

	ct_register_listener(ct, WORLD, event_handle,
			(signal_cb)c_editmode_event);

	ct_register_listener(ct, WORLD, events_begin,
			(signal_cb)c_editmode_events_begin);

	ct_register_listener(ct, WORLD, events_end,
			(signal_cb)c_editmode_events_end);

	ct_register_listener(ct, WORLD, window_resize,
			(signal_cb)c_editmode_resize);

}

