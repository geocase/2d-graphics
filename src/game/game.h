#ifndef GAME_H
#define GAME_H
#include "actor.h"
#include "input_state.h"
#include "cute_c2.h"

#define MAX_ACTORS 64

typedef struct {
    InputState_t input;
    Actor_t actors[64];
    c2AABB static_geometry[3]; // 3 for now
} Game_t;

#endif