#version 460 core
#extension GL_ARB_bindless_texture : require
#extension GL_ARB_gpu_shader_int64 : require

flat in uint idx;
layout(std430, binding = 1) buffer VTextures { uint64_t handles[]; };

in V_OUT { vec3 vNorm; }
v_out;

out vec4 FRAG_COL;
void main() { FRAG_COL = vec4(texture(sampler2D(handles[idx]), v_out.vNorm.xy).rgb, 1.0); }