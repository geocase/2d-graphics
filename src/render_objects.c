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

struct LightMesh lmGenerateLightMesh(struct Block* shadow_casters, u32 shadow_caster_count, vec2 position, f32 radius,
						 f32 resolution) {
	// Just copy the circle primitive rendering code to test
    const f32 cast_step = 0.001f;
	vec2* circle_verts = malloc(sizeof(vec2) * (shadow_caster_count * 4 + 1));
	u32* circle_indices = malloc(sizeof(u32) * (shadow_caster_count * 4 * 3));;
	vec2 center = {position[0], position[1]};
	f32 angle = 0.0f;
	f32 max_angle = 2.0f * M_PI;

    // get shadow caster vertices
    vec2* sc_verts = malloc(sizeof(vec2) * (shadow_caster_count * 4));
    for(u32 i = 0; i < shadow_caster_count; i += 1) {
        u32 p = i * 4;
        vec2 adjusted_size = {
            shadow_casters[i].size[0] / 2.0f,
            shadow_casters[i].size[1] / 2.0f
        };
        vec2 ps;
        glm_vec2_add(shadow_casters[i].position, adjusted_size, ps);
        
        vec2 pw = {shadow_casters[i].position[0] + adjusted_size[0], shadow_casters[i].position[1]};
        vec2 ph = {shadow_casters[i].position[0], shadow_casters[i].position[1] + adjusted_size[1]};
        glm_vec2_copy(shadow_casters[i].position, sc_verts[p + 0]);
        glm_vec2_copy(ps, sc_verts[p + 1]);
        glm_vec2_copy(pw, sc_verts[p + 2]);
        glm_vec2_copy(ph, sc_verts[p + 3]);
    }

    for(u32 i = 0; i < shadow_caster_count * 4; ++i) {
        // get angle
        f32 angle = atan2f(position[1] - sc_verts[i][1], position[0] - sc_verts[i][0]);
        vec2 cast_position;
        glm_vec2_copy(position, cast_position);
        while(glm_vec2_distance(cast_position, position) < radius) {
            b32 col = false;
            for(u32 caster_index = 0; caster_index < shadow_caster_count; ++caster_index) {
                cast_position[0] -= cosf(angle) * cast_step;
                cast_position[1] -= sinf(angle) * cast_step;
                if(pointInQuad(cast_position, &(shadow_casters[caster_index]))) {
                    printf("FUCK\n");
                    printf("%d, %f, %f\n", i, cast_position[0], cast_position[1]);
                    col = true;
                    break;
                }
            }
            if(col) {
                break;
            }
            
        }
        glm_vec2_copy(cast_position, circle_verts[i]);

    }
    glm_vec2_copy(position, circle_verts[shadow_caster_count * 4]);
    // gen indices
	for (i32 i = 0; i < shadow_caster_count * 4; ++i) {
		// previous -> center -> next
		i32 set = i * 3;
		circle_indices[set] = i;
		circle_indices[set + 1] = (i + 1) % shadow_caster_count * 4;
		circle_indices[set + 2] = shadow_caster_count * 4;
	}
    struct LightMesh lm;
    generateLightMeshGLObjects(&lm, circle_verts, shadow_caster_count * 4 + 1, circle_indices, shadow_caster_count * 4 * 3);
    free(circle_indices);
    free(circle_verts);
    free(sc_verts);
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