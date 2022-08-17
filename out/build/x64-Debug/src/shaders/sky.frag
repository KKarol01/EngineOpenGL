#version 460 core

in vec3 tc;
layout(binding=1)uniform samplerCube skybox;

out vec4 FRAG_COLOR;
void main() {
	FRAG_COLOR = texture(skybox, tc);
}