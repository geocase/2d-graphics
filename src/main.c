#include <glad/glad.h>

#include <stdio.h>
#include <stdlib.h>

#include "cute_c2.h"
#define CUTE_C2_IMPLEMENTATION

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

#include "game/game.h"

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

	glEnable(GL_BLEND);

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


	f32 uncentered_rectangle_verts[] = {
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f
	};
	u32 uncentered_rectangle_indices[] = {
		0, 1, 3,
		1, 2, 3
	};

	RenderPrimitive_t uncentered_rectangle_primitive =
		rpNewRenderPrimitive(uncentered_rectangle_verts, 4,
							 uncentered_rectangle_indices, 6);

	Sprite_t spr_counting =
		imgLoadSprite("64x64.png", 64, 64, 0, 1.0f / 1.0f, 0);

	Sprite_t spr_player = imgLoadSprite("cowboy.png", 38, 38, 1, 1.0f / 10.0f, 0);

	mat4 model;
	glm_mat4_identity(model);

	glClearColor(0.0, 0.0, 0.0, 1.0);
	SDL_Event ev;
	b32 quit = false;

	u32 mouse_x, mouse_y;
	rReloadShaders(&renderer);

	char *screen_shader_vs = readFilePathToCStr("shaders/buffer.vs");
	char *screen_shader_fs = readFilePathToCStr("shaders/buffer.fs");
	Shader_t screen_shader = shdNewShader(screen_shader_vs, screen_shader_fs);

	rGenerateFrameBuffer(&renderer, (vec2){BUFFER_X, BUFFER_Y}, FB_SCENE,
						 screen_shader);
	struct LightMesh point3;

	Game_t game = {
		.input = {.buttons_state = 0},
	};

	game.actors[0].type = ACT_PLAYER;
	game.actors[0].position[0] = 0;
	game.actors[0].position[1] = 0;
	game.actors[0].hitbox.min.x = game.actors[0].position[0] - (38 / 2);
	game.actors[0].hitbox.min.y = game.actors[0].position[1] - (38 / 2);
	game.actors[0].hitbox.max.x = game.actors[0].position[0] + (38 / 2);
	game.actors[0].hitbox.max.y = game.actors[0].position[1] + (38 / 2);


	c2AABB wall;
	wall.min.x = 100;
	wall.min.y = 10;
	wall.max.x = 120;
	wall.max.y = 100;
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
				switch (ev.key.keysym.sym) {
				case SDLK_F5:
					rReloadShaders(&renderer);
					break;
				case SDLK_w:
					iPressKey(&(game.input), KM_UP);
					break;
				case SDLK_a:
					iPressKey(&(game.input), KM_LEFT);
					break;
				case SDLK_s:
					iPressKey(&(game.input), KM_DOWN);
					break;
				case SDLK_d:
					iPressKey(&(game.input), KM_RIGHT);
					break;
				case SDLK_j:
					iPressKey(&(game.input), KM_ACT1);
					break;
				case SDLK_k:
					iPressKey(&(game.input), KM_ACT2);
					break;
				}
				break;
			case SDL_KEYUP:
				switch (ev.key.keysym.sym) {
				case SDLK_w:
					iReleaseKey(&(game.input), KM_UP);
					break;
				case SDLK_a:
					iReleaseKey(&(game.input), KM_LEFT);
					break;
				case SDLK_s:
					iReleaseKey(&(game.input), KM_DOWN);
					break;
				case SDLK_d:
					iReleaseKey(&(game.input), KM_RIGHT);
					break;
				case SDLK_j:
					iReleaseKey(&(game.input), KM_ACT1);
					break;
				case SDLK_k:
					iReleaseKey(&(game.input), KM_ACT2);
					break;
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
		vec2 velocity = {0, 0};
		if (game.input.buttons_state & KM_RIGHT) {
			velocity[0] += .1f;
		}
		if (game.input.buttons_state & KM_LEFT) {
			velocity[0] -= .1f;
		}

		if (game.input.buttons_state & KM_DOWN) {
			velocity[1] += .1f;
		}
		if (game.input.buttons_state & KM_UP) {
			velocity[1] -= .1f;
		}

		game.actors[0].hitbox.min.x = game.actors[0].position[0] - (38 / 2);
		game.actors[0].hitbox.min.y = game.actors[0].position[1] - (38 / 2);
		game.actors[0].hitbox.max.x = game.actors[0].position[0] + (38 / 2);
		game.actors[0].hitbox.max.y = game.actors[0].position[1] + (38 / 2);
		vec2 adjust = {0, 0};
		if(c2AABBtoAABB(game.actors[0].hitbox, wall)) {
			c2Manifold col;
			c2AABBtoAABBManifold(game.actors[0].hitbox, wall, &col);
			printf("%f, %f\n", col.depths[0], col.depths[1]);
			adjust[0] = col.depths[0] * c2Sign(velocity[0]);
		}

		glm_vec2_add(game.actors[0].position, velocity, game.actors[0].position);
		game.actors[0].position[0] -= adjust[0] * 2;


		rSwapFrameBuffer(&renderer, FB_SCENE);
		rClear(&renderer);
		mat4 model;
		glm_mat4_identity(model);
		glm_translate(model, (vec3){wall.min.x, wall.min.y, 0});
		glm_scale(model, (vec3){wall.max.x - wall.min.x, wall.max.y - wall.min.y, 1});
		rDrawPrimitive(&renderer, uncentered_rectangle_primitive, model, (vec4){1.0, 0, 0, 1.0});
		rDrawSprite(&renderer, &spr_player, game.actors->position, (vec2){1, 1});

		rSwapFrameBuffer(&renderer, FB_WINDOW);
		rClear(&renderer);

		rDrawFrameBuffer(&renderer, FB_SCENE);

		SDL_GL_SwapWindow(win);
	}

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(win);
	return 0;
}