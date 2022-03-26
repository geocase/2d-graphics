#include <glad/glad.h>

#include <stdio.h>
#include <stdlib.h>

#include <cglm/cglm.h>
#define SDL_MAIN_HANDLED
#include <SDL.h>

#include "common.h"
#include "fio.h"
#include "image.h"
#include "render_objects.h"
#include "renderer.h"
#include "shader.h"

#define BUFFER_Y 240
#define BUFFER_X 426


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
		{.position = {0, 0}, .size = {32, 32}},
		{.position = {0, 0}, .size = {32, 32}},
		{.position = {100, 480}, .size = {100, 32}},
	};
	u32 mouse_x, mouse_y;
	rReloadShaders(&renderer);

	u32 fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	u32 fbo_texture;
	glGenTextures(1, &fbo_texture);
	glBindTexture(GL_TEXTURE_2D, fbo_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, BUFFER_X, BUFFER_Y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo_texture, 0);

	u32 rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, BUFFER_X, BUFFER_Y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
	int p = glCheckFramebufferStatus(GL_FRAMEBUFFER);
 	if(p != GL_FRAMEBUFFER_COMPLETE) {
		 printf("%d\n", p);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	f32 vertices[] = {-1.0, -1.0, 0.0, 0.0, 1.0, -1.0, 1.0, 0.0,
					  -1.0, 1.0,  0.0, 1.0, 1.0, 1.0,  1.0, 1.0};

	u32 indices[] = {0, 1, 2, 1, 3, 2};
	u32 fbo_vao;
	u32 fbo_vbo;
	u32 fbo_ebo;
	glGenVertexArrays(1, &fbo_vao);
	glBindVertexArray(fbo_vao);

	glGenBuffers(1, &fbo_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, fbo_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(f32) * 4, (void *)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(f32) * 4,
						  (void *)(sizeof(f32) * 2));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glGenBuffers(1, &fbo_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fbo_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
				 GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	char* screen_shader_vs = readFilePathToCStr("shaders/buffer.vs");
	char* screen_shader_fs = readFilePathToCStr("shaders/buffer.fs");
	Shader_t screen_shader = shdNewShader(screen_shader_vs, screen_shader_fs);

	while (!quit) {
		SDL_GetMouseState(&mouse_x, &mouse_y);
		mouse_x /= renderer.size[0] / BUFFER_X;
		mouse_y /= renderer.size[1] / BUFFER_Y;
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
					rResize(&renderer, x, y);
					break;
				}
				break;
			}
		}
		b[0].position[0] = mouse_x;
		b[0].position[1] = mouse_y;
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glViewport(0, 0, BUFFER_X, BUFFER_Y);
		glm_ortho(0, BUFFER_X, BUFFER_Y, 0, -1.0, 1.0, renderer.projection);


		glEnable(GL_DEPTH_TEST);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		for (i32 i = 0; i < 3; ++i) {
			glm_mat4_identity(model);
			glm_translate(model, (vec3){b[i].position[0], b[i].position[1], 0});
			glm_scale(model,
					  (vec3){b[i].size[0] / 2.0f, b[i].size[1] / 2.0f, 1.0f});

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, spr_counting.texture_idx);

			shdUseShader(&renderer.shaders[SHADER_SPRITE]);

			glBindVertexArray(renderer.sprite_gl.vao);
			spr_counting.current_frame = (SDL_GetTicks64() / 1000) % 4;
			glUniform4f(
				glGetUniformLocation(
					renderer.shaders[SHADER_SPRITE].program_idx, "color"),
				1.0f, 0.0f, 0.0f, 1.0f);
			glUniformMatrix4fv(
				glGetUniformLocation(
					renderer.shaders[SHADER_SPRITE].program_idx, "model"),
				1, GL_FALSE, model);
			glUniformMatrix4fv(
				glGetUniformLocation(
					renderer.shaders[SHADER_SPRITE].program_idx, "projection"),
				1, GL_FALSE, renderer.projection);
			glUniformMatrix4fv(
				glGetUniformLocation(
					renderer.shaders[SHADER_SPRITE].program_idx, "view"),
				1, GL_FALSE, renderer.view);
			glUniform1i(
				glGetUniformLocation(
					renderer.shaders[SHADER_SPRITE].program_idx, "sprite"),
				0);
			glUniform1i(
				glGetUniformLocation(
					renderer.shaders[SHADER_SPRITE].program_idx, "frame"),
				spr_counting.current_frame);
			glUniform2fv(glGetUniformLocation(
							 renderer.shaders[SHADER_SPRITE].program_idx,
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
		// struct LightMesh static_lm =
		// 	lmGenerateLightMesh(b, 3, (vec2){100, 100}, 9, 180);

		// struct LightMesh point =
		// 	lmGenerateLightMesh(b, 3, (vec2){mouse_x, mouse_y}, 500, 10);
		// struct LightMesh point1 =
		// 	lmGenerateLightMesh(b, 3, (vec2){640, 320}, 500, 40);
		// struct LightMesh point2 =
		// 	lmGenerateLightMesh(b, 3, (vec2){640, 0}, 500, 40);
		// struct LightMesh point3 =
		// 	lmGenerateLightMesh(b, 3, (vec2){2, 480}, 500, 40);

		// rDrawLightMesh(&renderer, &point);
		// rDrawLightMesh(&renderer, &point1);
		// rDrawLightMesh(&renderer, &point2);
		// rDrawLightMesh(&renderer, &point3);
		// rDrawLightMesh(&renderer, &static_lm);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, renderer.size[0], renderer.size[1]);

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f); 
		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);

		// glm_mat4_identity(model);
		// glm_translate(model, (vec3){0, 0, 0});
		// glm_scale(model, (vec3){100, 100, 1});
		// rDrawPrimitive(&renderer, circle_primitive, model, (vec4){1.0, 0, 0, 1.0});

		shdUseShader(&screen_shader);
		glBindVertexArray(fbo_vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fbo_ebo);
		glBindTexture(GL_TEXTURE_2D, fbo_texture);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		SDL_GL_SwapWindow(win);
	}

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(win);
	return 0;
}