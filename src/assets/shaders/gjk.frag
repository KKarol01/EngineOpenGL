#version 460 core

out vec4 FRAG_COL;
in vec3 vp;
in vec3 vn;
flat in int vbasev;
uniform int silhouette;
uniform int hit;

const vec3 lp = vec3(21., 0., 31.);

void main() {
	float diff = dot(normalize(vp-lp), vn);
	diff = clamp(diff, .8, 1.);
	FRAG_COL = vec4(
	
	(hit==1&&vbasev==0)?vec3(1.):vec3(.6)  * diff, 1.);

	FRAG_COL.z += (vbasev!=0?1.:0.)*.3;

	if(silhouette==1) {
		FRAG_COL.xyz = vec3(255.f/255.f, 68.f/255.f, 31.f/255.f);
	}
}