#version 460 core

out vec4 FRAG_COLOR;
in vec3 vnormal;
in vec3 vpos;
flat in int inst_id;
void main() {
	FRAG_COLOR = vec4(vnormal*0.5+.5, 1.f);
}