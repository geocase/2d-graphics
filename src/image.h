#ifndef IMAGE_H
#define IMAGE_H
#include "common.h"
typedef struct {
	u32 texture_idx;
	f32 w, h;
	f32 tw, th;
	u16 current_frame;
	u16 max_frames;
	u16 row;
	u16 flags;
	f32 frame_time;
} Sprite_t;

Sprite_t imgLoadSprite(const char *path, f32 frame_width, f32 frame_height,
						u16 max_frames, f32 frame_time, u16 flags);

#endif // IMAGE_H