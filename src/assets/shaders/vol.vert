#version 460 core

layout(location=0) in vec2 pos;

out vec2 vpos;

uniform mat4 view;
uniform mat4 projection;

void main() {
	vpos = pos*2.f-1.f;
	gl_Position = vec4(vec2(pos*2.f-1.f), 0.f, 1.f);
}