#ifndef GAME_H
#define GAME_H
#include "actor.h"
#include "input_state.h"
#define MAX_ACTORS 64

typedef struct {
    InputState_t input;
    Actor_t actors[64];
} Game_t;

#endif