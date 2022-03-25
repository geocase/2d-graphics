#include "image.h"
#include <SDL.h>
#include <glad/glad.h>

Sprite_t imgLoadSprite(const char *path, f32 frame_width, f32 frame_height,
						u16 max_frames, f32 frame_time, u16 flags) {
	SDL_Surface *texture =
		SDL_ConvertSurface(SDL_LoadBMP("spr_sheet.bmp"),
						   SDL_AllocFormat(SDL_PIXELFORMAT_RGBA32), 0);
	if (!texture) {
		return (Sprite_t){0};
	}
	printf("%s\n", SDL_GetPixelFormatName(texture->format->format));
	SDL_LockSurface(texture);

	Sprite_t spr = {.w = frame_width,
					.h = frame_height,
					.tw = texture->w,
					.th = texture->h,
					.max_frames = max_frames,
					.frame_time = frame_time,
					.flags = flags,
					.row = 0,
					.current_frame = 0};

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &spr.texture_idx);
	glBindTexture(GL_TEXTURE_2D, spr.texture_idx);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->w, texture->h, 0, GL_RGBA,
				 GL_UNSIGNED_BYTE, texture->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	SDL_UnlockSurface(texture);
	// SDL_FreeSurface(texture);

	return spr;
}
