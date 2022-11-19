#version 460 core

layout(location=0) in vec2 pos;
layout(std140, binding=2) uniform UserSettings {
    mat4 p, v;
    vec4 cam_dir, cam_pos, lpos, lcol;
    float attenuation;
    int use_pbr;
};
uniform mat4 m;

out vec2 tc;

void main() {
	tc = pos;
	tc.y = 1. - tc.y;

	gl_Position = p * v * m * vec4(vec3(pos.xy*2.f-1.f, 0.), 1.);
}