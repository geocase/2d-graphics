#include "game/actor.h"

void aSetPositionTag(Actor_t* actor, u32 tag) {
    actor->position_tags |= tag;
    return;
}

void aResetPositionTag(Actor_t* actor, u32 tag) {
    actor->position_tags &= ~tag;
    return;
}