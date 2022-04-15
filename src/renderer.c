#include <glad/glad.h>

#include "fio.h"
#include "renderer.h"

void rGenerateFramebufferGLIndices(Renderer_t *renderer) {
	f32 vertices[] = {-1.0, -1.0, 0.0, 0.0, 1.0, -1.0, 1.0, 0.0,
					  -1.0, 1.0,  0.0, 1.0, 1.0, 1.0,  1.0, 1.0};

	u32 indices[] = {0, 1, 2, 1, 3, 2};

	glGenVertexArrays(1, &renderer->fb_gl.vao);
	glBindVertexArray(renderer->fb_gl.vao);

	glGenBuffers(1, &renderer->fb_gl.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, renderer->fb_gl.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(f32) * 4, (void *)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(f32) * 4,
						  (void *)(sizeof(f32) * 2));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glGenBuffers(1, &renderer->fb_gl.ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->fb_gl.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
				 GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

RenderPrimitive_t rpNewRenderPrimitive(f32 *verts, u32 vert_count, u32 *indices,
									   u32 tri_count) {
	RenderPrimitive_t rp = {.tri_count = tri_count};
	glGenVertexArrays(1, &rp.vao);
	glBindVertexArray(rp.vao);
	glGenBuffers(1, &rp.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, rp.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(f32) * vert_count * 2, verts,
				 GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(f32) * 2, (void *)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glGenBuffers(1, &rp.ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rp.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * tri_count, indices,
				 GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return rp;
}

void rGenerateSpriteGLIndices(Renderer_t *renderer) {
	f32 vertices[] = {-1.0, -1.0, 0.0, 0.0, 1.0, -1.0, 1.0, 0.0,
					  -1.0, 1.0,  0.0, 1.0, 1.0, 1.0,  1.0, 1.0};

	u32 indices[] = {0, 1, 2, 1, 3, 2};

	glGenVertexArrays(1, &renderer->sprite_gl.vao);
	glBindVertexArray(renderer->sprite_gl.vao);

	glGenBuffers(1, &renderer->sprite_gl.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, renderer->sprite_gl.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(f32) * 4, (void *)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(f32) * 4,
						  (void *)(sizeof(f32) * 2));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glGenBuffers(1, &renderer->sprite_gl.ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->sprite_gl.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
				 GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	return;
}

void rReloadShaders(Renderer_t *renderer) {
	for (int i = 0; i < SHADER_MAX; ++i) {
		shdDeleteShader(&renderer->shaders[i]);
	}

	char *vertex_shader_paths[] = {[SHADER_SPRITE] = "shaders/sprite.vs",
								   [SHADER_PRIMITIVE] = "shaders/primitive.vs",
								   [SHADER_BUFFER] = "shaders/buffer.vs",
								   [SHADER_LIGHTING_BUFFER] =
									   "shaders/lighting_buffer.vs"};
	char *fragment_shader_paths[] = {
		[SHADER_SPRITE] = "shaders/sprite.fs",
		[SHADER_PRIMITIVE] = "shaders/primitive.fs",
		[SHADER_BUFFER] = "shaders/buffer.fs",
		[SHADER_LIGHTING_BUFFER] = "shaders/lighting_buffer.fs"};

	for (int i = 0; i < SHADER_MAX; ++i) {
		char *vertex_shader = readFilePathToCStr(vertex_shader_paths[i]);
		char *fragment_shader = readFilePathToCStr(fragment_shader_paths[i]);
		renderer->shaders[i] = shdNewShader(vertex_shader, fragment_shader);
		free(vertex_shader);
		free(fragment_shader);
	}
}

void rResize(Renderer_t *renderer, int w, int h) {
	glViewport(0, 0, w, h);
	renderer->framebuffers[renderer->current_framebuffer].size[0] = w;
	renderer->framebuffers[renderer->current_framebuffer].size[1] = h;
	return;
}

void rDrawPrimitive(Renderer_t *renderer, RenderPrimitive_t primitive,
					mat4 model, vec4 color) {

	shdUseShader(&renderer->shaders[SHADER_PRIMITIVE]);
	glBindVertexArray(primitive.vao);
	glUniform4fv(glGetUniformLocation(
					 renderer->shaders[SHADER_PRIMITIVE].program_idx, "color"),
				 1, color);
	glUniformMatrix4fv(
		glGetUniformLocation(renderer->shaders[SHADER_PRIMITIVE].program_idx,
							 "model"),
		1, GL_FALSE, model);
	glUniformMatrix4fv(
		glGetUniformLocation(renderer->shaders[SHADER_PRIMITIVE].program_idx,
							 "projection"),
		1, GL_FALSE, renderer->projection);
	glUniformMatrix4fv(
		glGetUniformLocation(renderer->shaders[SHADER_PRIMITIVE].program_idx,
							 "view"),
		1, GL_FALSE, renderer->view);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, primitive.ebo);
	glDrawElements(GL_TRIANGLES, primitive.tri_count, GL_UNSIGNED_INT, 0);
	return;
}

void rDrawSprite(Renderer_t *renderer, Sprite_t *sprite, vec2 position,
				 vec2 scale) {
	mat4 model;
	glm_mat4_identity(model);
	glm_translate(model, (vec3){position[0], position[1], 0});
	glm_scale(model, (vec3){(sprite->w * scale[0]) / 2.0f,
							(sprite->h * scale[1]) / 2.0f, 1.0f});

	vec2 frame_dimensions = {sprite->tw / sprite->w, sprite->th / sprite->h};

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sprite->texture_idx);

	shdUseShader(&renderer->shaders[SHADER_SPRITE]);

	glBindVertexArray(renderer->sprite_gl.vao);
	glUniform4f(glGetUniformLocation(
					renderer->shaders[SHADER_SPRITE].program_idx, "color"),
				1.0f, 0.0f, 0.0f, 1.0f);
	glUniformMatrix4fv(
		glGetUniformLocation(renderer->shaders[SHADER_SPRITE].program_idx,
							 "model"),
		1, GL_FALSE, model);
	glUniformMatrix4fv(
		glGetUniformLocation(renderer->shaders[SHADER_SPRITE].program_idx,
							 "projection"),
		1, GL_FALSE, renderer->projection);
	glUniformMatrix4fv(
		glGetUniformLocation(renderer->shaders[SHADER_SPRITE].program_idx,
							 "view"),
		1, GL_FALSE, renderer->view);
	glUniform1i(glGetUniformLocation(
					renderer->shaders[SHADER_SPRITE].program_idx, "sprite"),
				0);
	glUniform1i(glGetUniformLocation(
					renderer->shaders[SHADER_SPRITE].program_idx, "frame"),
				sprite->current_frame);
	glUniform2fv(
		glGetUniformLocation(renderer->shaders[SHADER_SPRITE].program_idx,
							 "frame_dimensions"),
		1, frame_dimensions);

	glActiveTexture(GL_TEXTURE0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->sprite_gl.ebo);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void rGenerateFrameBuffer(Renderer_t *renderer, vec2 size, u32 framebuffer_idx,
						  u32 shader_idx) {

	renderer->framebuffers[framebuffer_idx].shader_index = shader_idx;
	glm_vec2_copy(size, renderer->framebuffers[framebuffer_idx].size);
	renderer->framebuffers[framebuffer_idx].size[0] = size[0];
	glGenFramebuffers(1, &renderer->framebuffers[framebuffer_idx].index);
	glBindFramebuffer(GL_FRAMEBUFFER,
					  renderer->framebuffers[framebuffer_idx].index);

	glGenTextures(1, &renderer->framebuffers[framebuffer_idx].texture);
	glBindTexture(GL_TEXTURE_2D,
				  renderer->framebuffers[framebuffer_idx].texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
				 renderer->framebuffers[framebuffer_idx].size[0],
				 renderer->framebuffers[framebuffer_idx].size[1], 0, GL_RGB,
				 GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
						   renderer->framebuffers[framebuffer_idx].texture, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void rSwapFrameBuffer(Renderer_t *renderer, u32 framebuffer_idx) {
	if (framebuffer_idx == FB_WINDOW) {
		glDisable(GL_DEPTH_TEST);
	} else {
		glEnable(GL_DEPTH_TEST);
	}
	glBindFramebuffer(GL_FRAMEBUFFER,
					  renderer->framebuffers[framebuffer_idx].index);
	glViewport(0, 0, renderer->framebuffers[framebuffer_idx].size[0],
			   renderer->framebuffers[framebuffer_idx].size[1]);
	glm_ortho(0, renderer->framebuffers[framebuffer_idx].size[0],
			  renderer->framebuffers[framebuffer_idx].size[1], 0, -1, 1.0,
			  renderer->projection);
	renderer->current_framebuffer = framebuffer_idx;
}

void rDrawFrameBuffer(Renderer_t *renderer, u32 framebuffer_idx) {
	shdUseShader(
		&renderer
			 ->shaders[renderer->framebuffers[framebuffer_idx].shader_index]);
	glBindVertexArray(renderer->fb_gl.vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->fb_gl.ebo);
	glBindTexture(GL_TEXTURE_2D,
				  renderer->framebuffers[framebuffer_idx].texture);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void rClear(Renderer_t *renderer) {
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}