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
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

	f32 uncentered_rectangle_verts[] = {0.0f, 0.0f, 1.0f, 0.0f,
										1.0f, 1.0f, 0.0f, 1.0f};
	u32 uncentered_rectangle_indices[] = {0, 1, 3, 1, 2, 3};

	RenderPrimitive_t uncentered_rectangle_primitive = rpNewRenderPrimitive(
		uncentered_rectangle_verts, 4, uncentered_rectangle_indices, 6);

	Sprite_t spr_counting =
		imgLoadSprite("64x64.png", 64, 64, 0, 1.0f / 1.0f, 0);

	Sprite_t spr_player =
		imgLoadSprite("cowboy.png", 38, 38, 1, 1.0f / 10.0f, 0);

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
	game.actors[0].position_tags = 0;
	game.actors[0].position[0] = 0;
	game.actors[0].position[1] = 0;
	game.actors[0].hitbox.min.x = game.actors[0].position[0] - (38 / 2);
	game.actors[0].hitbox.min.y = game.actors[0].position[1] - (38 / 2);
	game.actors[0].hitbox.max.x = game.actors[0].position[0] + (38 / 2);
	game.actors[0].hitbox.max.y = game.actors[0].position[1] + (38 / 2);
	game.actors[0].ground.p.x = game.actors[0].position[0];
	game.actors[0].ground.p.y = game.actors[0].position[1];
	game.actors[0].ground.d.x = 0;
	game.actors[0].ground.d.y = 1.0f;
	game.actors[0].ground.t = (38 / 2) + 1;

	c2AABB wall[] = {{.min.x = -10000,
					  .min.y = BUFFER_Y - 100,
					  .max.x = BUFFER_X,
					  .max.y = BUFFER_Y},
					 {.min.x = BUFFER_X,
					  .min.y = BUFFER_Y - 50,
					  .max.x = 1000,
					  .max.y = BUFFER_Y},
					 {.min.x = BUFFER_X + 40,
					  .min.y = BUFFER_Y - 200,
					  .max.x = BUFFER_X + 40 + 100,
					  .max.y = BUFFER_Y - 100}};
	i32 wall_count = sizeof(wall) / sizeof(c2AABB);

	vec2 velocity = {0, 0};

	// timing
	const double dt = 1.0 / 60.0;

	f64 current_time = SDL_GetTicks64() / 1000.0f;
	f64 accumulator = 0.0;

	struct LightMesh l;
	lmGenerateLightMesh(NULL, 0, (vec2){BUFFER_X, 50}, 200, 100, &l);

	while (!quit) {
		i32 start = SDL_GetPerformanceCounter();
		f64 new_time = SDL_GetTicks64() / 1000.0f;
		f64 frame_time = new_time - current_time;
		current_time = new_time;
		accumulator += frame_time;

		while (accumulator >= dt) {

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
			vec2 gravity = {0, 1.0f};

			f32 hori_speed = 3.0f;
			if (!game.actors[0].position_tags & SF_ON_GROUND) {
				hori_speed = 2.5f;
			}
			if (game.input.buttons_state & KM_RIGHT) {
				velocity[0] += hori_speed;
			}
			if (game.input.buttons_state & KM_LEFT) {
				velocity[0] -= hori_speed;
			}

			if (game.input.buttons_state & KM_ACT1) {
				if (game.actors[0].position_tags & SF_ON_GROUND) {
					velocity[1] -= 10.0f;
				}
			}
			glm_vec2_add(gravity, velocity, velocity);

			// set max velo
			velocity[0] = min(20, fabs(velocity[0])) * c2Sign(velocity[0]);
			velocity[1] = min(50, fabs(velocity[1])) * c2Sign(velocity[1]);

			// drag
			if (fabs(velocity[0]) > 0) {
				f32 drag = 2.0f;
				if (game.actors[0].position_tags & SF_ON_GROUND) {
					drag = 1.0f;
				}
				velocity[0] -= drag * c2Sign(velocity[0]);
				if (fabs(velocity[0]) < .0003) {
					velocity[0] = 0;
				}
			}
			game.actors[0].ground.p.x = game.actors[0].position[0];
			game.actors[0].ground.p.y = game.actors[0].position[1];

			game.actors[0].hitbox.min.x =
				game.actors[0].position[0] - (38 / 2) + velocity[0];
			game.actors[0].hitbox.min.y =
				game.actors[0].position[1] - (38 / 2) + velocity[1];
			game.actors[0].hitbox.max.x = game.actors[0].hitbox.min.x + 38;
			game.actors[0].hitbox.max.y = game.actors[0].hitbox.min.y + 38;

			vec2 adjust = {0, 0};
			b32 already_on_ground = false;
			for (int i = 0; i < wall_count; ++i) {
				c2Raycast rc;
				if (!already_on_ground) {
					if (c2RaytoAABB(game.actors[0].ground, wall[i], &rc)) {
						aSetPositionTag(&game.actors[0], SF_ON_GROUND);
						already_on_ground = true;
					} else {
						aResetPositionTag(&game.actors[0], SF_ON_GROUND);
					}
				}

				if (c2AABBtoAABB(game.actors[0].hitbox, wall[i])) {
					c2Manifold col;
					c2AABBtoAABBManifold(game.actors[0].hitbox, wall[i], &col);
					// printf("%f, %f\n", col.depths[0], col.depths[1]);
					adjust[0] += col.depths[0] * col.n.x;
					adjust[1] += col.depths[0] * col.n.y;
				}
			}
			glm_vec2_sub(velocity, adjust, velocity);
			glm_vec2_add(game.actors[0].position, velocity,
						 game.actors[0].position);

			if (game.actors[0].position_tags & SF_ON_GROUND) {
				velocity[1] = 0;
			}

			accumulator -= dt;
		}
		glm_mat4_identity(renderer.view);
		f32 sc = .5f;
		vec2 vscale = {sc, sc};
		glm_scale(renderer.view, (vec3){vscale[0], vscale[1], 1.0});

		glm_translate(renderer.view,
					  (vec3){-game.actors[0].position[0] +
								 (BUFFER_X / (2 / (1 / vscale[0]))),
							 -game.actors[0].position[1] +
								 (BUFFER_Y / (2 / (1 / vscale[1]))),
							 0});

		rSwapFrameBuffer(&renderer, FB_SCENE);

		rClear(&renderer);

		rDrawLightMesh(&renderer, &l);

		for (int i = 0; i < wall_count; ++i) {
			mat4 model;
			glm_mat4_identity(model);
			glm_translate(model, (vec3){wall[i].min.x, wall[i].min.y, 0});
			glm_scale(model, (vec3){wall[i].max.x - wall[i].min.x,
									wall[i].max.y - wall[i].min.y, 1});
			rDrawPrimitive(&renderer, uncentered_rectangle_primitive, model,
						   (vec4){1.0, 0, 0, 1.0});
		}

		rDrawSprite(&renderer, &spr_player, game.actors->position,
					(vec2){1, 1});

		rSwapFrameBuffer(&renderer, FB_WINDOW);
		rClear(&renderer);

		rDrawFrameBuffer(&renderer, FB_SCENE);

		SDL_GL_SwapWindow(win);
	}

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(win);
	return 0;
}