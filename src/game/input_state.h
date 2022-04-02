#ifndef INPUT_STATE_H
#define INPUT_STATE_H
#include "renderer.h"

enum {
    KM_UP = 1 << 0,
    KM_DOWN = 1 << 1,
    KM_LEFT = 1 << 2,
    KM_RIGHT = 1 << 3,
    KM_ACT1 = 1 << 4,
    KM_ACT2 = 1 << 5,
    KM_START = 1 << 6,
    KM_SELECT = 1 << 7
};

typedef struct {
    vec2 fb_mouse_positions[FB_MAX];
    u8 buttons_state;
} InputState_t;

void iPressKey(InputState_t* i_state, u8 key);
void iReleaseKey(InputState_t* i_state, u8 key);

#endif 
