#version 460 core

out vec4 FRAG_COLOR;

uniform vec3 color;

void main() {
	FRAG_COLOR = vec4(color, 1.f);
}