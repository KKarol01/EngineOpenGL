#version 460 core

layout(location=0) in vec2 pos;

out vec2 tc;

void main() {
	tc = pos;
	tc.y = 1. - tc.y;
	gl_Position = vec4(pos*2.-1., 0.9, 1.);
}