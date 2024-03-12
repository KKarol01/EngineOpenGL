#version 460 core

layout(location = 0) in vec2 pos;

out vec2 vpos;

void main() {
    vec4 ndcpos = vec4(pos * 2. - 1., 0, 1.);
    vpos        = ndcpos.xy;

    gl_Position = ndcpos;
}