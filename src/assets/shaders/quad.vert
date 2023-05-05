#version 460 core

layout(location=0) in vec2 pos;

out vec2 v_tc;

void main() {
	v_tc = pos*0.5+0.5;
	gl_Position = vec4(pos.xy, 0.f, 1.f);
}