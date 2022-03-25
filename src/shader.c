#include "shader.h"
#include <glad/glad.h>

Shader_t shdNewShader(const char *vertex, const char *fragment) {
	u32 vshader_idx;
	u32 fshader_idx;
	u32 shader_program_idx;
	u32 success;
	vshader_idx = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vshader_idx, 1, &vertex, NULL);
	glCompileShader(vshader_idx);
	glGetShaderiv(vshader_idx, GL_COMPILE_STATUS, &success);
	if (!success) {
		char log[1024];
		glGetShaderInfoLog(vshader_idx, 1024, NULL, log);
		printf("vertex:\n%s\n%s\n", vertex, log);
	}

	fshader_idx = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fshader_idx, 1, &fragment, NULL);
	glCompileShader(fshader_idx);
	glGetShaderiv(fshader_idx, GL_COMPILE_STATUS, &success);
	if (!success) {
		char log[1024];
		glGetShaderInfoLog(fshader_idx, 1024, NULL, log);
		printf("fragment:\n%s\n%s\n", fragment, log);
	}

	shader_program_idx = glCreateProgram();
	glAttachShader(shader_program_idx, vshader_idx);
	glAttachShader(shader_program_idx, fshader_idx);
	glLinkProgram(shader_program_idx);

	glGetProgramiv(shader_program_idx, GL_LINK_STATUS, &success);
	if (!success) {
		char log[1024];
		glGetProgramInfoLog(shader_program_idx, 1024, NULL, log);
		printf("PROGRAM\n%s\n", log);
	}

	Shader_t s = {.program_idx = shader_program_idx,
				  .vertex_idx = vshader_idx,
				  .fragment_idx = fshader_idx};

	return s;
}

void shdUseShader(Shader_t *shd) {
	glUseProgram(shd->program_idx);
	return;
}

void shdDeleteShader(Shader_t *shd) {
	glDeleteShader(shd->vertex_idx);
	glDeleteShader(shd->fragment_idx);
	glDeleteProgram(shd->program_idx);
	return;
}