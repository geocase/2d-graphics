#ifndef ACTOR_H
#define ACTOR_H
#include "common.h"
#include "cute_c2.h"

typedef enum { ACT_NONE, ACT_PLAYER, ACT_MAX } ActorType_e;
enum {
	SF_ON_GROUND = 1 << 0,
	SF_TOUCHING_WALL = 1 << 1,
};
typedef struct {
	ActorType_e type;
	vec2 position;
	vec2 velocity;
	c2AABB hitbox;
	c2Ray ground;
	c2Ray forward;
	u32 position_tags;
} Actor_t;

void aSetPositionTag(Actor_t *actor, u32 tag);
void aResetPositionTag(Actor_t *actor, u32 tag);

#endif