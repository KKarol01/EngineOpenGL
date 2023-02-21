#version 460 core

layout(location=0) in vec3 pos;
layout(location=1) in vec3 normal;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

out vec3 vp;
out vec3 vn;
flat out int vbasev;

void main() {
	vbasev = gl_BaseVertex;
	vn = (model * vec4(normal, 1.)).xyz;
	vp = (vec4(pos, 1.)).xyz;
	gl_Position = proj * view * model * vec4(pos, 1.);
}