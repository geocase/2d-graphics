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

enum { SHADER_SPRITE, SHADER_PRIMITIVE, SHADER_MAX };
enum { FB_WINDOW, FB_SCENE, FB_MAX };
struct SpriteGLIndices {
	u32 vao, vbo, ebo;
};

struct FramebufferRenderGLIndices {
	u32 vao, vbo, ebo;
};

typedef struct {
	u32 index;
	u32 texture;
	vec2 size;
	Shader_t shader;
} Framebuffer_t;

typedef struct {
	mat4 projection;
	mat4 view;
	Shader_t shaders[SHADER_MAX];
	Framebuffer_t framebuffers[FB_MAX];
	vec2 size[FB_MAX];
	struct SpriteGLIndices sprite_gl;
	struct FramebufferRenderGLIndices fb_gl;

	// GL Context??
} Renderer_t;

void rGenerateSpriteGLIndices(Renderer_t *renderer);
void rReloadShaders(Renderer_t *renderer);
void rResize(Renderer_t *renderer, int w, int h);
void rDrawPrimitive(Renderer_t *renderer, RenderPrimitive_t primitive,
					mat4 model, vec4 color);
void rGenerateFrameBuffer(Renderer_t* renderer, vec2 size, u32 framebuffer_idx);
#endif