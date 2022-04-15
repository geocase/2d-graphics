#ifndef RENDER_OBJECTS_H
#define RENDER_OBJECTS_H
#include "common.h"
#include "renderer.h"
#include <cute_c2.h>

struct LightMesh {
	u32 vao, vbo, ebo;
	u64 last_update;
	u32 tri_count;
};

void lmGenerateLightMesh(c2AABB *shadow_casters, u32 shadow_caster_count,
						 vec2 position, f32 radius, u32 resolution,
						 struct LightMesh *lm);
void rDrawLightMesh(Renderer_t *renderer, struct LightMesh *lm);

typedef struct {
	u32 vao, vbo, ebo;
	u32 tri_count;
} RenderPrimitive_t;

RenderPrimitive_t rpNewRenderPrimitive(f32 *verts, u32 vert_count, u32 *indices,
									   u32 tri_count);
void rDrawPrimitive(Renderer_t *renderer, RenderPrimitive_t primitive,
					mat4 model, vec4 color);
					
#endif