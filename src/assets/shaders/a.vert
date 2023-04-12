#version 460 core

layout(location=0) in vec3 pos;

uniform mat4 v;
uniform mat4 p;

void main() {

	gl_Position =p * v * vec4(pos.xyz, 1.0);

}