#include <glad/glad.h>

#include "render_objects.h"
#include "common.h"

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

struct LightMesh lmGenerateLightMesh(struct Block* shadow_casters, u32 shadow_caster_count, vec2 position, f32 radius,
						 u32 resolution) {
	// Just copy the circle primitive rendering code to test
    const f32 cast_step = 5.0f;
	vec2* circle_verts = malloc(sizeof(vec2) * (resolution + 1));
	u32* circle_indices = malloc(sizeof(u32) * (resolution * 3));;
	vec2 center = {position[0], position[1]};
	f32 angle = 0.0f;
	f32 max_angle = 2.0f * M_PI;
    for (i32 i = 0; i < resolution; ++i) {
        circle_verts[i][0] = position[0];
		circle_verts[i][1] = position[1];
        f32 dist = 0.0f;
        while(true) {
            b32 broke = false;
            for(i32 p = 0; p < shadow_caster_count; ++p) {
                if(pointInQuad(circle_verts[i], &shadow_casters[p])) {
                    broke = true;
                    break;
                }
            }
            if(broke) {
                break;
            }
            circle_verts[i][0] += cosf(angle) * cast_step;
		    circle_verts[i][1] += sinf(angle) * cast_step;
            dist += cast_step;
            if(dist > radius) {
                break;
            }
        }
		angle += max_angle / resolution;
	}
	glm_vec2_copy(center, circle_verts[resolution]);
	for (i32 i = 0; i < resolution; ++i) {
		// previous -> center -> next
		i32 set = i * 3;
		circle_indices[set] = i;
		circle_indices[set + 1] = (i + 1) % resolution;
		circle_indices[set + 2] = resolution;
	}
    struct LightMesh lm;
    generateLightMeshGLObjects(&lm, circle_verts, resolution + 1, circle_indices, resolution * 3);
    free(circle_indices);
    free(circle_verts);
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