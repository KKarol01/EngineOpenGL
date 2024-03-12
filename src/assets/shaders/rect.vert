#version 460 core

layout(location=0) in vec2 pos;
layout(location=1) in vec2 tc;


out vec2 v_tc;

void main() {
	v_tc = tc;
	gl_Position = vec4(pos.xy, 0.f, 1.f);
}