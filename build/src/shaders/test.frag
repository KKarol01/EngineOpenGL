#version 460 core

layout(binding=0) uniform sampler2D diffuse_texture;

uniform vec3 color;
in vec2 vs_tc;

out vec4 FRAG_COLOR;
void main() { 
    FRAG_COLOR = vec4(texture(diffuse_texture, vs_tc).rgb, 1.f);
    //FRAG_COLOR = vec4(1.f);
}