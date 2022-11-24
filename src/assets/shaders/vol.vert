#version 460 core

layout(location=0) in vec2 pos;

out vec2 vpos;

uniform mat4 model;
uniform mat4 rotmat;
uniform mat4 view;
uniform mat4 projection;

void main() {
	vpos = ( vec4(pos*2.f-1.f, 0., 1.)).xy;
	gl_Position = projection * view * rotmat * vec4(vec2(pos*2.f-1.f), 0.f, 1.f);
}