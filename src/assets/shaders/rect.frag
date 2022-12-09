#version 460 core

out vec4 FRAG_COL;
in vec3 vpos;
flat in int iid;

void main() {
	
	vec3 coll = vec3(.5); 
	if(iid%10==0) coll = vec3(1.);

	float n =gl_DepthRange.near;
	float f =gl_DepthRange.far;
	n = 0.01;
	f = 100.0;
	float d = gl_FragCoord.z*2.-1.;

	float ldepth = (2.*n*f)/(f+n-d*(f-n));
	FRAG_COL = vec4(coll, 1.-ldepth/f*2.);
}