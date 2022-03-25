#include <glad/glad.h>

#include <stdio.h>
#include <stdlib.h>

#include <cglm/cglm.h>
#define SDL_MAIN_HANDLED
#include <SDL.h>

#include "common.h"
#include "fio.h"
#include "image.h"
#include "renderer.h"
#include "shader.h"
#include "render_objects.h"

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

	rGenerateSpriteGLIndices(&renderer);

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
		imgLoadSprite("spr_sheet.bmp", 32, 32, 4, 1.0f / 10.0f, 0);

	vec2 frame_dimensions = {spr_counting.tw / spr_counting.w,
							 spr_counting.th / spr_counting.h};

	mat4 model;
	glm_mat4_identity(model);

	glClearColor(0.0, 0.0, 0.0, 1.0);
	SDL_Event ev;
	b32 quit = false;

	struct Block b[] = {
		{
		.position = {600, 400},
		.size = {32, 32}
		},
		{
		.position = {600, 400},
		.size = {32, 32}
		},
		{
		.position = {100, 480},
		.size = {100, 32}
		},
	};
	u32 mouse_x, mouse_y;
	rReloadShaders(&renderer);

	while (!quit) {
		SDL_GetMouseState(&mouse_x, &mouse_y);
		while (SDL_PollEvent(&ev) != 0) {
			switch (ev.type) {
			case SDL_QUIT:
				quit = true;
				break;
			case SDL_KEYDOWN:
				if (ev.key.keysym.sym == SDLK_F5) {
					rReloadShaders(&renderer);
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
		b[0].position[0] = mouse_x;
		b[0].position[1] = mouse_y;
		glClear(GL_COLOR_BUFFER_BIT);
		for(i32 i = 0; i < 3; ++i) {
			glm_mat4_identity(model);
			glm_translate(model, (vec3){b[i].position[0], b[i].position[1], 0});
			glm_scale(model, (vec3){b[i].size[0] / 2.0f, b[i].size[1] / 2.0f, 1.0f});

			
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, spr_counting.texture_idx);

			shdUseShader(&renderer.shaders[SHADER_SPRITE]);

			glBindVertexArray(renderer.sprite_gl.vao);
			spr_counting.current_frame = (SDL_GetTicks64() / 1000) % 4;
			glUniform4f(glGetUniformLocation(
							renderer.shaders[SHADER_SPRITE].program_idx, "color"),
						1.0f, 0.0f, 0.0f, 1.0f);
			glUniformMatrix4fv(
				glGetUniformLocation(renderer.shaders[SHADER_SPRITE].program_idx,
									"model"),
				1, GL_FALSE, model);
			glUniformMatrix4fv(
				glGetUniformLocation(renderer.shaders[SHADER_SPRITE].program_idx,
									"projection"),
				1, GL_FALSE, renderer.projection);
			glUniformMatrix4fv(
				glGetUniformLocation(renderer.shaders[SHADER_SPRITE].program_idx,
									"view"),
				1, GL_FALSE, renderer.view);
			glUniform1i(glGetUniformLocation(
							renderer.shaders[SHADER_SPRITE].program_idx, "sprite"),
						0);
			glUniform1i(glGetUniformLocation(
							renderer.shaders[SHADER_SPRITE].program_idx, "frame"),
						spr_counting.current_frame);
			glUniform2fv(
				glGetUniformLocation(renderer.shaders[SHADER_SPRITE].program_idx,
									"frame_dimensions"),
			1, frame_dimensions);

			glActiveTexture(GL_TEXTURE0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer.sprite_gl.ebo);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}
		glm_mat4_identity(model);

		// glm_translate(model, (vec3){mouse_x, mouse_y, 0});
		// glm_scale(model, (vec3){100, 100, 0});
		// rDrawPrimitive(&renderer, circle_primitive, model,
		// 			   (vec4){1.0, 0, 0, 1.0});
		struct LightMesh static_lm = lmGenerateLightMesh(b, 3, (vec2){100, 100}, 9, 180);

		struct LightMesh point = lmGenerateLightMesh(b, 3, (vec2){mouse_x, mouse_y}, 500, 10);
		struct LightMesh point1 = lmGenerateLightMesh(b, 3, (vec2){640, 320}, 500, 40);
		struct LightMesh point2 = lmGenerateLightMesh(b, 3, (vec2){640, 0}, 500, 40);
		struct LightMesh point3 = lmGenerateLightMesh(b, 3, (vec2){2, 480}, 500, 40);


		rDrawLightMesh(&renderer, &point);
		rDrawLightMesh(&renderer, &point1);
		rDrawLightMesh(&renderer, &point2);
		rDrawLightMesh(&renderer, &point3);
		rDrawLightMesh(&renderer, &static_lm);

		SDL_GL_SwapWindow(win);
	}

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(win);
	return 0;
}