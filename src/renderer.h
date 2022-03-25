#ifndef RENDERER_H
#define RENDERER_H
#include <cglm/cglm.h>

#include "common.h"
#include "shader.h"

typedef struct {
	u32 vao, vbo, ebo;
	u32 tri_count;
} RenderPrimitive_t;
RenderPrimitive_t rpNewRenderPrimitive(f32 *verts, u32 vert_count, u32 *indices,
									   u32 tri_count);

enum {
    SHADER_SPRITE,
    SHADER_PRIMITIVE,
    SHADER_MAX
};

typedef struct {
	mat4 projection;
	mat4 view;
	vec2 size;
    Shader_t shaders[SHADER_MAX];
	// GL Context??
} Renderer_t;

void rReloadShaders(Renderer_t* renderer);
void rResize(Renderer_t *renderer, int w, int h);
void rDrawPrimitive(Renderer_t *renderer, RenderPrimitive_t primitive,
					mat4 model, vec4 color);
#endif