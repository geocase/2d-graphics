#include <glad/glad.h>

#include "render_objects.h"

void generateLightMeshGLObjects(struct LightMesh* lm, f32 *verts, u32 vert_count, u32 *indices,
								u32 tri_count) {
    glGenVertexArrays(1, &lm->vao);
	glBindVertexArray(lm->vao);
	glGenBuffers(1, &lm->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, lm->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(f32) * vert_count * 2, verts,
				 GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(f32) * 2, (void *)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glGenBuffers(1, &lm->ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lm->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * tri_count, indices,
				 GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    lm->tri_count = tri_count;
    return;
}

struct LightMesh lmGenerateLightMesh(struct Block* shadow_casters, vec2 position, f32 radius,
						 f32 resolution) {
	// Just copy the circle primitive rendering code to test
#define CIRCLE_RESOLUTION 50
	vec2 circle_verts[CIRCLE_RESOLUTION + 1];
	u32 circle_indices[CIRCLE_RESOLUTION * 3];
	vec2 center = {640, 480};
	f32 angle = 0.0f;
	f32 max_angle = 2.0f * M_PI;
	for (i32 i = 0; i < CIRCLE_RESOLUTION; ++i) {
		circle_verts[i][0] = cosf(angle) + center[0];
		circle_verts[i][1] = sinf(angle) + center[1];
		angle += max_angle / CIRCLE_RESOLUTION;
	}
	glm_vec2_copy(center, circle_verts[CIRCLE_RESOLUTION]);
	for (i32 i = 0; i < CIRCLE_RESOLUTION; ++i) {
		// previous -> center -> next
		i32 set = i * 3;
		circle_indices[set] = i;
		circle_indices[set + 1] = (i + 1) % CIRCLE_RESOLUTION;
		circle_indices[set + 2] = CIRCLE_RESOLUTION;
	}
    struct LightMesh lm;
    generateLightMeshGLObjects(&lm, circle_verts, CIRCLE_RESOLUTION + 1, circle_indices, CIRCLE_RESOLUTION * 3);
    return lm;
}

void rDrawLightMesh(Renderer_t* renderer, struct LightMesh* lm) {
    vec4 color = {1.0f, 1.0f, 0.0f, 0.5f};
    mat4 model;
    glm_mat4_identity(model);
    shdUseShader(&renderer->shaders[SHADER_PRIMITIVE]);
	glBindVertexArray(lm->vao);
	glUniform4fv(glGetUniformLocation(
					 renderer->shaders[SHADER_PRIMITIVE].program_idx, "color"),
				 1, color);
	glUniformMatrix4fv(
		glGetUniformLocation(renderer->shaders[SHADER_PRIMITIVE].program_idx,
							 "model"),
		1, GL_FALSE, model);
	glUniformMatrix4fv(
		glGetUniformLocation(renderer->shaders[SHADER_PRIMITIVE].program_idx,
							 "projection"),
		1, GL_FALSE, renderer->projection);
	glUniformMatrix4fv(
		glGetUniformLocation(renderer->shaders[SHADER_PRIMITIVE].program_idx,
							 "view"),
		1, GL_FALSE, renderer->view);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lm->ebo);
	glDrawElements(GL_TRIANGLES, lm->tri_count, GL_UNSIGNED_INT, 0);
	return;
}