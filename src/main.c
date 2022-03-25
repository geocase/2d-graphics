#include <glad/glad.h>

#include <stdio.h>
#include <stdlib.h>

#include <cglm/cglm.h>
#define SDL_MAIN_HANDLED
#include <SDL.h>

#include "common.h"
#include "fio.h"
#include "image.h"
#include "shader.h"

Shader_t sprite_shader;
Shader_t primitive_shader;

void reloadShaders() {
	shdDeleteShader(&sprite_shader);
	shdDeleteShader(&primitive_shader);
	char *primitive_vertex_shader = readFilePathToCStr("shaders/primitive.vs");
	char *primitive_fragment_shader =
		readFilePathToCStr("shaders/primitive.fs");
	char *sprite_vertex_shader = readFilePathToCStr("shaders/sprite.vs");
	char *sprite_fragment_shader = readFilePathToCStr("shaders/sprite.fs");
	sprite_shader = shdNewShader(sprite_vertex_shader, sprite_fragment_shader);
	primitive_shader =
		shdNewShader(primitive_vertex_shader, primitive_fragment_shader);
	free(primitive_vertex_shader);
	free(primitive_fragment_shader);
	free(sprite_vertex_shader);
	free(sprite_fragment_shader);
}

typedef struct {
	u32 vao, vbo, ebo;
	u32 tri_count;
} RenderPrimitive_t;

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

typedef struct {
	mat4 projection;
	mat4 view;
	vec2 size;
	// GL Context??
} Renderer_t;

void rResize(Renderer_t *renderer, int w, int h) {
	glViewport(0, 0, w, h);
	glm_ortho(0, w, h, 0, -1.0, 1.0, renderer->projection);
	renderer->size[0] = w;
	renderer->size[1] = h;
	return;
}

void rDrawPrimitive(Renderer_t *renderer, RenderPrimitive_t primitive,
					mat4 model, vec4 color) {
	glBindVertexArray(primitive.vao);
	glUniform4fv(glGetUniformLocation(primitive_shader.program_idx, "color"), 1,
				 color);
	glUniformMatrix4fv(
		glGetUniformLocation(primitive_shader.program_idx, "model"), 1,
		GL_FALSE, model);
	glUniformMatrix4fv(
		glGetUniformLocation(primitive_shader.program_idx, "projection"), 1,
		GL_FALSE, renderer->projection);
	glUniformMatrix4fv(
		glGetUniformLocation(primitive_shader.program_idx, "view"), 1, GL_FALSE,
		renderer->view);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, primitive.ebo);
	glDrawElements(GL_TRIANGLES, primitive.tri_count, GL_UNSIGNED_INT, 0);
	return;
}

struct Block {
	vec2 position;
	vec2 size;
};

int main() {
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window *win = SDL_CreateWindow(
		"Platformer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
						SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GLContext *context = SDL_GL_CreateContext(win);
	SDL_GL_MakeCurrent(win, context);

	if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
		printf("L\n");
		exit(-1);
	}

	Renderer_t renderer;
	glm_mat4_identity(renderer.view);

	{
		int w, h;
		SDL_GetWindowSize(win, &w, &h);
		rResize(&renderer, w, h);
	}

	f32 vertices[] = {-1.0, -1.0, 0.0, 0.0, 1.0, -1.0, 1.0, 0.0,
					  -1.0, 1.0,  0.0, 1.0, 1.0, 1.0,  1.0, 1.0};

	u32 indices[] = {0, 1, 2, 1, 3, 2};

	u32 vao;
	u32 vbo;
	u32 ebo;

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(f32) * 4, (void *)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(f32) * 4,
						  (void *)(sizeof(f32) * 2));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
				 GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

#define CIRCLE_RESOLUTION 50
	vec2 circle_verts[CIRCLE_RESOLUTION + 1];
	u32 circle_indices[CIRCLE_RESOLUTION * 3];
	vec2 center = {0, 0};
	f32 angle = 0.0f;
	f32 max_angle = 2.0f * M_PI;
	for (i32 i = 0; i < CIRCLE_RESOLUTION; ++i) {
		circle_verts[i][0] = cosf(angle);
		circle_verts[i][1] = sinf(angle);
		angle += max_angle / CIRCLE_RESOLUTION;
	}
	glm_vec2_copy(center, circle_verts[CIRCLE_RESOLUTION]);
	for (i32 i = 0; i < CIRCLE_RESOLUTION; ++i) {
		// previous -> center -> next
		i32 set = i * 3;
		circle_indices[set] = i;
		circle_indices[set + 1] = (i + 1) % CIRCLE_RESOLUTION;
		circle_indices[set + 2] = CIRCLE_RESOLUTION;
	}

	RenderPrimitive_t circle_primitive =
		rpNewRenderPrimitive(circle_verts, CIRCLE_RESOLUTION + 1,
							 circle_indices, CIRCLE_RESOLUTION * 3);

	Sprite_t spr_counting =
		img_loadSprite("spr_sheet.bmp", 32, 32, 4, 1.0f / 10.0f, 0);

	vec2 frame_dimensions = {spr_counting.tw / spr_counting.w,
							 spr_counting.th / spr_counting.h};

	mat4 model;
	glm_mat4_identity(model);

	glClearColor(0.0, 0.0, 0.0, 1.0);
	SDL_Event ev;
	b32 quit = false;

	struct Block b = {
		.position = {320, 240},
		.size = {32, 32},
	};
	u32 mouse_x, mouse_y;
	reloadShaders();

	while (!quit) {
		SDL_GetMouseState(&mouse_x, &mouse_y);
		while (SDL_PollEvent(&ev) != 0) {
			switch (ev.type) {
			case SDL_QUIT:
				quit = true;
				break;
			case SDL_KEYDOWN:
				if (ev.key.keysym.sym == SDLK_F5) {
					reloadShaders();
				}
			case SDL_WINDOWEVENT:
				switch (ev.window.event) {
				case SDL_WINDOWEVENT_RESIZED:
					u32 x, y;
					x = ev.window.data1;
					y = ev.window.data2;
					glViewport(0, 0, x, y);
					glm_ortho(0, x, y, 0, -1.0, 1.0, renderer.projection);
					break;
				}
				break;
			}
		}

		glm_mat4_identity(model);
		glm_translate(model, (vec3){b.position[0], b.position[1], 0});
		glm_scale(model, (vec3){b.size[0] / 2.0f, b.size[1] / 2.0f, 1.0f});

		glClear(GL_COLOR_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, spr_counting.texture_idx);

		shdUseShader(&sprite_shader);

		glBindVertexArray(vao);
		spr_counting.current_frame = (SDL_GetTicks64() / 1000) % 4;
		glUniform4f(glGetUniformLocation(sprite_shader.program_idx, "color"),
					1.0f, 0.0f, 0.0f, 1.0f);
		glUniformMatrix4fv(
			glGetUniformLocation(sprite_shader.program_idx, "model"), 1,
			GL_FALSE, model);
		glUniformMatrix4fv(
			glGetUniformLocation(sprite_shader.program_idx, "projection"), 1,
			GL_FALSE, renderer.projection);
		glUniformMatrix4fv(
			glGetUniformLocation(sprite_shader.program_idx, "view"), 1,
			GL_FALSE, renderer.view);
		glUniform1i(glGetUniformLocation(sprite_shader.program_idx, "sprite"),
					0);
		glUniform1i(glGetUniformLocation(sprite_shader.program_idx, "frame"),
					spr_counting.current_frame);
		glUniform2fv(
			glGetUniformLocation(sprite_shader.program_idx, "frame_dimensions"),
			1, frame_dimensions);

		glActiveTexture(GL_TEXTURE0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		shdUseShader(&primitive_shader);
		glm_mat4_identity(model);

		glm_translate(model, (vec3){mouse_x, mouse_y, 0});
		glm_scale(model, (vec3){100, 100, 0});
		rDrawPrimitive(&renderer, circle_primitive, model,
					   (vec4){1.0, 0, 0, 1.0});

		SDL_GL_SwapWindow(win);
	}

	glDeleteBuffers(1, &ebo);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
	shdDeleteShader(&sprite_shader);
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(win);
	return 0;
}