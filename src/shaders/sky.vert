#version 460 core

layout(location=0)in vec3 pos;
out vec3 tc;

uniform mat4 view;
uniform mat4 projection;

void main() {
	tc = pos;
	gl_Position = projection * mat4(mat3(view)) * vec4(pos, 1.f);
}