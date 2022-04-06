#include "game/game.h"

void aInitPlayer(Game_t* game, Actor_t* actor) {
    actor->type = ACT_PLAYER;
	actor->position_tags = 0;
	actor->position[0] = 0;
	actor->position[1] = 0;
	actor->hitbox.min.x = actor->position[0] - (38 / 2);
	actor->hitbox.min.y = actor->position[1] - (38 / 2);
	actor->hitbox.max.x = actor->position[0] + (38 / 2);
	actor->hitbox.max.y = actor->position[1] + (38 / 2);
	actor->ground.p.x = actor->position[0];
	actor->ground.p.y = actor->position[1];
	actor->ground.d.x = 0;
	actor->ground.d.y = 1.0f;
	actor->ground.t = (38 / 2) + 1;
	actor->velocity[0] = 0;
	actor->velocity[1] = 0;
}

void aUpdatePlayer(Game_t* game, Actor_t* actor) {
	vec2 gravity = {0, 1.0f};
    f32 hori_speed = 3.0f;
    i32 wall_count = 3;
    if (!game->actors[0].position_tags & SF_ON_GROUND) {
        hori_speed = 2.5f;
    }
    if (game->input.buttons_state & KM_RIGHT) {
        actor->velocity[0] += hori_speed;
    }
    if (game->input.buttons_state & KM_LEFT) {
        actor->velocity[0] -= hori_speed;
    }

    if (game->input.buttons_state & KM_ACT1) {
        if (game->actors[0].position_tags & SF_ON_GROUND) {
            actor->velocity[1] -= 10.0f;
        }
    }
    glm_vec2_add(gravity, actor->velocity, actor->velocity);

    // set max velo
    actor->velocity[0] = min(20, fabs(actor->velocity[0])) * c2Sign(actor->velocity[0]);
    actor->velocity[1] = min(50, fabs(actor->velocity[1])) * c2Sign(actor->velocity[1]);

    // drag
    if (fabs(actor->velocity[0]) > 0) {
        f32 drag = 2.0f;
        if (game->actors[0].position_tags & SF_ON_GROUND) {
            drag = 1.0f;
        }
        actor->velocity[0] -= drag * c2Sign(actor->velocity[0]);
        
    }
    game->actors[0].ground.p.x = game->actors[0].position[0];
    game->actors[0].ground.p.y = game->actors[0].position[1];

    game->actors[0].hitbox.min.x =
        game->actors[0].position[0] - (38 / 2) + actor->velocity[0];
    game->actors[0].hitbox.min.y =
        game->actors[0].position[1] - (38 / 2) + actor->velocity[1];
    game->actors[0].hitbox.max.x = game->actors[0].hitbox.min.x + 38;
    game->actors[0].hitbox.max.y = game->actors[0].hitbox.min.y + 38;

    vec2 adjust = {0, 0};
    b32 already_on_ground = false;
    for (int i = 0; i < wall_count; ++i) {
        c2Raycast rc;
        if (!already_on_ground) {
            if (c2RaytoAABB(game->actors[0].ground, game->static_geometry[i], &rc)) {
                aSetPositionTag(&game->actors[0], SF_ON_GROUND);
                already_on_ground = true;
            } else {
                aResetPositionTag(&game->actors[0], SF_ON_GROUND);
            }
        }

        if (c2AABBtoAABB(game->actors[0].hitbox, game->static_geometry[i])) {
            c2Manifold col;
            c2AABBtoAABBManifold(game->actors[0].hitbox, game->static_geometry[i], &col);
            // printf("%f, %f\n", col.depths[0], col.depths[1]);
            adjust[0] += col.depths[0] * col.n.x;
            adjust[1] += col.depths[0] * col.n.y;
        }
    }
    glm_vec2_sub(actor->velocity, adjust, actor->velocity);
    glm_vec2_add(game->actors[0].position, actor->velocity,
                    game->actors[0].position);

    if (game->actors[0].position_tags & SF_ON_GROUND) {
        actor->velocity[1] = 0;
    }

    if (fabs(actor->velocity[0]) < .0003) {
        actor->velocity[0] = 0;
    }
}