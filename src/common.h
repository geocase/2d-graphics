#ifndef COMMON_H
#define COMMON_H
#include <cglm/cglm.h>

#include <inttypes.h>
#include <stdbool.h>

struct Block {
	vec2 position;
	vec2 size;
};

bool pointInQuad(vec2 point, struct Block *q);

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t
#define i8 int8_t
#define i16 int16_t
#define i32 int32_t
#define i64 int64_t

#define f32 float
#define f64 double

#define b32 bool

#endif // COMMON_H