#version 460 core
#extension GL_ARB_bindless_texture : require
#extension GL_ARB_gpu_shader_int64 : require

layout(location=0) in vec3 pos;
layout(location=1) in vec3 n;
layout(location=2) in vec3 t;
layout(location=3) in vec3 b;
layout(location=4) in vec2 tc;

out vec3 fragpos;
out vec3 vn;
out vec2 vtc;
flat out int draw_id;
out mat3 tbn;

layout(std140, binding=2) uniform UserSettings {
    mat4 p, v, m;
    vec4 cam_dir, cam_pos, lpos, lcol;
    float attenuation;
    int use_pbr;
};

void main() {
	vec4 wpos = m * vec4(pos.xyz + vec3(0.f, 0.f, 3.f)*gl_InstanceID, 1.f);

	fragpos = wpos.xyz;
	vn = n;
	vtc = tc;
	draw_id = gl_DrawID;
	tbn = mat3(t, b, n);

	gl_Position = p * v * wpos;
}