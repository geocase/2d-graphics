#include <glad/glad.h>

#include "fio.h"
#include "renderer.h"

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
	shdDeleteShader(&renderer->shaders[SHADER_SPRITE]);
	shdDeleteShader(&renderer->shaders[SHADER_PRIMITIVE]);
	char *primitive_vertex_shader = readFilePathToCStr("shaders/primitive.vs");
	char *primitive_fragment_shader =
		readFilePathToCStr("shaders/primitive.fs");
	char *sprite_vertex_shader = readFilePathToCStr("shaders/sprite.vs");
	char *sprite_fragment_shader = readFilePathToCStr("shaders/sprite.fs");
	renderer->shaders[SHADER_SPRITE] =
		shdNewShader(sprite_vertex_shader, sprite_fragment_shader);
	renderer->shaders[SHADER_PRIMITIVE] =
		shdNewShader(primitive_vertex_shader, primitive_fragment_shader);
	free(primitive_vertex_shader);
	free(primitive_fragment_shader);
	free(sprite_vertex_shader);
	free(sprite_fragment_shader);
}

void rResize(Renderer_t *renderer, int w, int h) {
	glViewport(0, 0, w, h);
	glm_ortho(0, w, h, 0, -1.0, 1.0, renderer->projection);
	renderer->size[0] = w;
	renderer->size[1] = h;
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