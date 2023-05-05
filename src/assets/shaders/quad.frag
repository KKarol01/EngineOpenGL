#version 460 core

out vec4 FRAG_COL;
in vec2 v_tc;

uniform sampler2D tex;

void main() {

	FRAG_COL = vec4(texture(tex, v_tc).rgb, 1.0);
}