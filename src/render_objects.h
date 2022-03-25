#ifndef RENDER_OBJECTS_H
#define RENDER_OBJECTS_H
#include "common.h"
#include "renderer.h"

struct LightMesh {
	u32 vao, vbo, ebo;
	u64 last_update;
    u32 tri_count;
};

struct LightMesh lmGenerateLightMesh(struct Block* shadow_casters, vec2 position, f32 radius,
						 f32 resolution);
void rDrawLightMesh(Renderer_t* renderer, struct LightMesh* lm);
#endif