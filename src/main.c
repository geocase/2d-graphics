#include <glad/glad.h>

#include <stdio.h>
#include <stdlib.h>

#include <cglm/cglm.h>
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_image.h>

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
	IMG_Init(IMG_INIT_PNG);
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
	renderer.framebuffers[FB_WINDOW].index = 0;
	rSwapFrameBuffer(&renderer, FB_WINDOW);
	glm_mat4_identity(renderer.view);

	{
		int w, h;
		SDL_GetWindowSize(win, &w, &h);
		rResize(&renderer, w, h);
	}

	rGenerateSpriteGLIndices(&renderer);
	rGenerateFramebufferGLIndices(&renderer);
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
		imgLoadSprite("cowboy.png", 38, 38, 0, 1.0f / 10.0f, 0);

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

	char *screen_shader_vs = readFilePathToCStr("shaders/buffer.vs");
	char *screen_shader_fs = readFilePathToCStr("shaders/buffer.fs");
	Shader_t screen_shader = shdNewShader(screen_shader_vs, screen_shader_fs);

	rGenerateFrameBuffer(&renderer, (vec2){BUFFER_X, BUFFER_Y}, FB_SCENE,
						 screen_shader);

	while (!quit) {
		SDL_GetMouseState(&mouse_x, &mouse_y);
		mouse_x /= renderer.framebuffers[FB_WINDOW].size[0] /
				   renderer.framebuffers[FB_SCENE].size[0];
		mouse_y /= renderer.framebuffers[FB_WINDOW].size[1] /
				   renderer.framebuffers[FB_SCENE].size[1];
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
					rSwapFrameBuffer(&renderer, FB_WINDOW);
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
		rSwapFrameBuffer(&renderer, FB_SCENE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		rClear(&renderer);

		for (i32 i = 0; i < 3; ++i) {
			rDrawSprite(&renderer, &spr_counting, b[i].position, (vec2){1, 1});
		}
		struct LightMesh point3 =
			lmGenerateLightMesh(b, 3, (vec2){50, 50}, 100, 9999);

		rDrawLightMesh(&renderer, &point3);

		rSwapFrameBuffer(&renderer, FB_WINDOW);
		// rClear(&renderer);

		rDrawFrameBuffer(&renderer, FB_SCENE);

		SDL_GL_SwapWindow(win);
	}

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(win);
	return 0;
}