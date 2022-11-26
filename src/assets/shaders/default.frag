#version 460 core

in vec2 vpos;
out vec4 FRAG_COLOR;

layout(binding=0) uniform sampler2D pbralb;
layout(binding=1) uniform sampler2D pbrdepth;
layout(binding=2) uniform sampler2D flamealb;
layout(binding=3) uniform sampler2D flamedepth;
layout(binding=4) uniform sampler2D flamedist;

void main() {
	vec2 tc = vec2(vpos*.5+.5);

	float pbrd = texture(pbrdepth, tc).x;
	float flamed = texture(flamedepth, tc).x;

	vec2 dtc = tc;
	if(flamed < pbrd)  {
		dtc += texture(flamedist, tc).rb*0.01;
		pbrd = texture(pbrdepth, dtc).x;
		flamed = texture(flamedepth, dtc).x;
		if(flamed > pbrd)  { dtc = tc;}
		pbrd = texture(pbrdepth, dtc).x;
		flamed = texture(flamedepth, dtc).x;
	}

	FRAG_COLOR = texture(pbralb,dtc);

	if(flamed < pbrd)  {
		FRAG_COLOR += texture(flamealb, tc);
	}
}
