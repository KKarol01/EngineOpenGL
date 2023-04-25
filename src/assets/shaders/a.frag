#version 460 core

out vec4 FRAGCOL;
in vec3 n;
void main() {
	FRAGCOL = vec4(n*0.5+0.5, 1.f);
}