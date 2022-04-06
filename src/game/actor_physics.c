#include "actor_physics.h"

void aAdjustCollisions(Game_t *game, Actor_t *actor) {
	i32 wall_count = 3;
	vec2 adjust = {0, 0};
	b32 already_on_ground = false;
	b32 already_next_to_wall = false;
	for (int i = 0; i < wall_count; ++i) {
		c2Raycast rc;
		if (!already_on_ground) {
			if (c2RaytoAABB(actor->ground, game->static_geometry[i], &rc)) {
				aSetPositionTag(actor, SF_ON_GROUND);
				already_on_ground = true;
			} else {
				aResetPositionTag(actor, SF_ON_GROUND);
			}
		}

		if (!already_next_to_wall) {
			if (c2RaytoAABB(actor->forward, game->static_geometry[i], &rc)) {
				aSetPositionTag(actor, SF_TOUCHING_WALL);
				already_next_to_wall = true;
			} else {
				aResetPositionTag(actor, SF_TOUCHING_WALL);
			}
		}

		if (c2AABBtoAABB(actor->hitbox, game->static_geometry[i])) {
			c2Manifold col;
			c2AABBtoAABBManifold(actor->hitbox, game->static_geometry[i], &col);
			// printf("%f, %f\n", col.depths[0], col.depths[1]);
			adjust[0] += col.depths[0] * col.n.x;
			adjust[1] += col.depths[0] * col.n.y;
		}
	}
	glm_vec2_sub(actor->velocity, adjust, actor->velocity);
	return;
}

void aFall(Actor_t *actor) {
	vec2 gravity = {0, 1.0f};
	glm_vec2_add(gravity, actor->velocity, actor->velocity);
	return;
}

void aDrag(Actor_t *actor) {
	if (fabs(actor->velocity[0]) > 0) {
		f32 drag = 2.0f;
		if (actor->position_tags & SF_ON_GROUND) {
			drag = 1.0f;
		}
		actor->velocity[0] -= drag * c2Sign(actor->velocity[0]);
	}
}

void aCommitMovement(Actor_t *actor) {
	glm_vec2_add(actor->position, actor->velocity, actor->position);

	if (actor->position_tags & SF_ON_GROUND) {
		actor->velocity[1] = 0;
	}

	if (fabs(actor->velocity[0]) < .0003) {
		actor->velocity[0] = 0;
	}
}

void aUpdateHitbox(Actor_t *actor) {
	// for player only, generalize this!
	actor->ground.p.x = actor->position[0];
	actor->ground.p.y = actor->position[1];
	actor->forward.p.x = actor->position[0];
	actor->forward.p.y = actor->position[1];

	actor->hitbox.min.x = actor->position[0] - (38 / 2) + actor->velocity[0];
	actor->hitbox.min.y = actor->position[1] - (38 / 2) + actor->velocity[1];
	actor->hitbox.max.x = actor->hitbox.min.x + 38;
	actor->hitbox.max.y = actor->hitbox.min.y + 38;
}

void aJump(Actor_t *actor, f32 force) {
	if (actor->position_tags & SF_ON_GROUND) {
		actor->velocity[1] -= force;
	}
}

void aFollow(Actor_t *const follower, const Actor_t *const followee,
			 f32 range) {
	if (fabs(follower->position[0] - followee->position[0]) < range) {
		return;
	}

	// generalize, maybe store speed in actor struct
	f32 hori_speed = 2.5;
	if (follower->position[0] < followee->position[0]) {
		follower->velocity[0] += hori_speed;
		follower->forward.d.x = 1.0;
	} else {
		follower->velocity[0] -= hori_speed;
		follower->forward.d.x = -1.0;
	}

	if (followee->position[1] < follower->position[1]) {
		if ((follower->position_tags & SF_TOUCHING_WALL)) {
			aJump(follower, 10.0f);
		}
	}
	return;
}