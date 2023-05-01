#version 460 core
#extension GL_ARB_bindless_texture : require
#extension GL_ARB_gpu_shader_int64 : require

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNorm;
layout(location = 2) in vec3 vTan;
layout(location = 3) in vec3 vBTan;

struct A {
    uint64_t diffuse;
    uint64_t normal;
    uint64_t metallic;
    uint64_t roughness;
    uint64_t emissive;
    vec4 channels;
    mat4 transform;
};

layout(std430, binding = 0) buffer VT { A a[]; };

uniform mat4 v;
uniform mat4 p;

flat out uint idx;
flat out mat3 TBN;
out V_OUT { vec3 v_pos; vec3 v_normal; } v_out;

void main() {
    idx         = gl_BaseInstance + gl_InstanceID;
    v_out.v_pos = (a[idx].transform * vec4(vPos, 1.0)).xyz;
    v_out.v_normal = vNorm;
    
    vec3 N = cross(vTan, vBTan);
    TBN = mat3(vTan, vBTan, N);

    gl_Position = p * v * a[idx].transform * vec4(vPos, 1.0);
}