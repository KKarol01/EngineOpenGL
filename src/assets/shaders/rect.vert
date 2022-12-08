#version 460 core

layout(location=0) in vec3 pos;

out vec3 vpos;

void main() {
	vpos = pos;
	gl_Position = vec4(pos*2.-1., 1.);	
}