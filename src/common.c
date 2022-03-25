#include "common.h"

bool pointInQuad(vec2 point, struct Block* q) {
    f32 x = q->position[0];
    f32 y = q->position[1];
    f32 w = q->size[0];
    f32 h = q->size[1];
    f32 px = point[0];
    f32 py = point[1];
    if(x <= px && px <= x + w) {
        if(y <= py && py <= y + h) {
            return true;
        }
    }
    return false;
}