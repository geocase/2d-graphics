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
#include "game/actor_physics.h"
#include "game/actor_player.h"
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
	free(screen_shader_vs);
	free(screen_shader_fs);

	rGenerateFrameBuffer(&renderer, (vec2){BUFFER_X, BUFFER_Y}, FB_SCENE,
						 screen_shader);

	screen_shader_vs = readFilePathToCStr("shaders/lighting_buffer.vs");
	screen_shader_fs = readFilePathToCStr("shaders/lighting_buffer.fs");
	screen_shader = shdNewShader(screen_shader_vs, screen_shader_fs);
	rGenerateFrameBuffer(&renderer, (vec2){BUFFER_X, BUFFER_Y}, FB_LIGHTING,
						 screen_shader);

	free(screen_shader_vs);
	free(screen_shader_fs);

	struct LightMesh point3;

	Game_t game = {.input = {.buttons_state = 0},
				   .static_geometry = {{.min.x = -10000,
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
										.max.y = BUFFER_Y - 100}}};
	aInitPlayer(&game, &(game.actors[0]));
	aInitPlayer(&game, &(game.actors[1]));

	i32 wall_count = sizeof(game.static_geometry) / sizeof(c2AABB);

	// timing
	const double dt = 1.0 / 60.0;

	f64 current_time = SDL_GetTicks64() / 1000.0f;
	f64 accumulator = 0.0;

	struct LightMesh l;

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

			aUpdatePlayer(&game, &(game.actors[0]));

			aFall(&(game.actors[1]));
			aFollow(&(game.actors[1]), &(game.actors[0]), 400);
			aDrag(&(game.actors[1]));
			aCapSpeed(&(game.actors[1]));
			aUpdateHitbox(&(game.actors[1]));
			aAdjustCollisions(&game, &(game.actors[1]));
			aCommitMovement(&(game.actors[1]));

			accumulator -= dt;
		}
		glm_mat4_identity(renderer.view);
		f32 sc = .25f;
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

		for (int i = 0; i < wall_count; ++i) {
			mat4 model;
			glm_mat4_identity(model);
			glm_translate(model, (vec3){game.static_geometry[i].min.x,
										game.static_geometry[i].min.y, 0});
			glm_scale(model, (vec3){game.static_geometry[i].max.x -
										game.static_geometry[i].min.x,
									game.static_geometry[i].max.y -
										game.static_geometry[i].min.y,
									1});
			rDrawPrimitive(&renderer, uncentered_rectangle_primitive, model,
						   (vec4){1.0, 0, 0, 1.0});
		}

		rDrawSprite(&renderer, &spr_player, game.actors[0].position,
					(vec2){1, 1});
		rDrawSprite(&renderer, &spr_player, game.actors[1].position,
					(vec2){1, 1});

		rSwapFrameBuffer(&renderer, FB_WINDOW);
		rClear(&renderer);

		rDrawFrameBuffer(&renderer, FB_SCENE);
		rDrawFrameBuffer(&renderer, FB_LIGHTING);

		SDL_GL_SwapWindow(win);
	}

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(win);
	return 0;
}