#ifndef RENDERER_H
#define RENDERER_H
#include <cglm/cglm.h>

#include "common.h"
#include "image.h"
#include "shader.h"

enum {
	SHADER_SPRITE,
	SHADER_PRIMITIVE,
	SHADER_BUFFER,
	SHADER_LIGHTING_BUFFER,
	SHADER_MAX
};
enum { FB_WINDOW, FB_SCENE, FB_LIGHTING, FB_MAX };

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
	u32 shader_index;
} Framebuffer_t;

typedef struct {
	mat4 projection;
	mat4 view;
	Shader_t shaders[SHADER_MAX];
	u32 current_framebuffer;
	Framebuffer_t framebuffers[FB_MAX];

	struct SpriteGLIndices sprite_gl;
	struct FramebufferRenderGLIndices fb_gl;

	// GL Context??
} Renderer_t;

void rGenerateSpriteGLIndices(Renderer_t *renderer);
void rGenerateFramebufferGLIndices(Renderer_t *renderer);
void rGenerateFrameBuffer(Renderer_t *renderer, vec2 size, u32 framebuffer_idx,
						  u32 shader_idx);
void rReloadShaders(Renderer_t *renderer);

void rDrawSprite(Renderer_t *renderer, Sprite_t *sprite, vec2 position,
				 vec2 scale);

void rResize(Renderer_t *renderer, int w, int h);
void rSwapFrameBuffer(Renderer_t *renderer, u32 framebuffer_idx);
void rDrawFrameBuffer(Renderer_t *renderer, u32 framebuffer_idx);
void rClear(Renderer_t *renderer);
#endif