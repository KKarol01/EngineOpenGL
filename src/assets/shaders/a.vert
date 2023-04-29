#version 460 core
// #extension GL_ARB_bindless_texture : require
// #extension GL_ARB_gpu_shader_int64 : require

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNorm;

uniform mat4 v;
uniform mat4 p;

flat out uint idx;
out V_OUT { vec3 vNorm; }
v_out;

void main() {
    idx         = gl_BaseInstance + gl_InstanceID;
    v_out.vNorm = vNorm;
    gl_Position = p * v * vec4(vPos, 1.0);
}