#include "game/actor_physics.h"
#include "game/game.h"

void aInitPlayer(Game_t *game, Actor_t *actor) {
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

	actor->forward.p.x = actor->position[0];
	actor->forward.p.y = actor->position[1];
	actor->forward.d.x = 1.0;
	actor->forward.d.y = 0.0f;
	actor->forward.t = (38 / 2) + 4;
	actor->velocity[0] = 0;
	actor->velocity[1] = 0;
}

void aPlayerJump(Actor_t *actor) { aJump(actor, 10.0f); }

void aUpdatePlayer(Game_t *game, Actor_t *actor) {
	f32 hori_speed = 3.0f;
	i32 wall_count = 3;
	if (!actor->position_tags & SF_ON_GROUND) {
		hori_speed = 2.5f;
	}
	if (game->input.buttons_state & KM_RIGHT) {
		actor->velocity[0] += hori_speed;
		actor->forward.d.x = 1.0;
	}
	if (game->input.buttons_state & KM_LEFT) {
		actor->velocity[0] -= hori_speed;
		actor->forward.d.x = -1.0;
	}

	if (game->input.buttons_state & KM_ACT1) {
		aPlayerJump(actor);
	}

	aFall(actor);
	aCapSpeed(actor);
	aDrag(actor);
	aUpdateHitbox(actor);
	aAdjustCollisions(game, actor);
	aCommitMovement(actor);
}