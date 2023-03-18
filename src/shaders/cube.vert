#version 460 core

#define INSTANCES 100.


layout(location=0) in vec2 pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 vpos;

void main() {
	vec3 p = vec3(pos.x, 0.f, pos.y);
	vpos = p;
	gl_Position = projection * view * model * vec4(p, 1.);
}