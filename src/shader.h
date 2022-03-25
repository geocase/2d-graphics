#ifndef SHADER_H
#define SHADER_H
#include "common.h"
typedef struct {
	u32 program_idx;
	u32 vertex_idx;
	u32 fragment_idx;
} Shader_t;

Shader_t shdNewShader(const char *vertex, const char *fragment);
void shdUseShader(Shader_t *shd);
void shdDeleteShader(Shader_t *shd);
#endif