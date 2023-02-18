#version 460 core
layout(location = 0) in vec3 vert;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texture_coordinates;
layout(location = 3) in ivec4 bone_id;
layout(location = 4) in vec4 bone_weight;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 bone_mats[100];

uniform float time;

out vec2 vs_tc;
void main() {
    vs_tc = texture_coordinates;
    vec4 position = vec4(0.f);
    for(int i=0; i<4;++i) {
        position += (bone_mats[bone_id[i]] * vec4(vert, 1.f)) * bone_weight[i];
    }
    gl_Position = projection * view * model * position;
}