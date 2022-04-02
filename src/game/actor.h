#ifndef ACTOR_H
#define ACTOR_H
#include <cglm/cglm.h>
#include "cute_c2.h"

typedef enum {ACT_PLAYER, ACT_MAX} ActorType_e;

typedef struct {
    ActorType_e type;
    vec2 position;
    c2AABB hitbox;
} Actor_t;


#endif