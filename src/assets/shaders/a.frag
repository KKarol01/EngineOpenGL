#version 460 core
#extension GL_ARB_bindless_texture : require
#extension GL_ARB_gpu_shader_int64 : require

// struct Payload {
//     mat4 transform;
//     uint64_t bindless_handle;
// };

layout(std430, binding = 0) buffer VData { uint64_t handles[]; };

out vec4 FRAG_COL;
flat in uint idx;
in V_OUT { vec3 vNorm; }
v_out;

void main() {
    // FRAG_COL
    //     = vec4(texture(sampler2D(vdata.transforms[idx].bindless_handle),
    //     v_out.vNorm.xy).rgb, 1.0);
    FRAG_COL = vec4(texture(sampler2D(handles[idx]), v_out.vNorm.xy).rgb, 1.0);
}