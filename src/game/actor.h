#ifndef ACTOR_H
#define ACTOR_H
#include <cglm/cglm.h>
typedef enum {ACT_PLAYER, ACT_MAX} ActorType_e;

typedef struct {
    ActorType_e type;
} Actor_t;


#endif