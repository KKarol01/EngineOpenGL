#version 460 core


out vec4 FRAG_COLOR;

in vec3 vpos;

void main() {
	FRAG_COLOR = vec4(vpos, 1.f);
}