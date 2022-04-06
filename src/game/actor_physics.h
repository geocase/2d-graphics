#ifndef ACTOR_PHYSICS_H
#define ACTOR_PHYSICS_H
#include "actor.h"
#include "game.h"
void aAdjustCollisions(Game_t *game, Actor_t *actor);
void aFall(Actor_t *actor);
void aDrag(Actor_t *actor);
void aCommitMovement(Actor_t *actor);
void aUpdateHitbox(Actor_t *actor);
void aJump(Actor_t *actor, f32 force);
void aFollow(Actor_t *const follower, const Actor_t *const followee, f32 range);
void aCapSpeed(Actor_t *const actor);
#endif