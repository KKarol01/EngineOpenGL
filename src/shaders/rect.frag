#version 460 core

out vec4 FRAG_COL;
flat in int instance;
in vec3 vpos;

#define inv_255 0.00392156
#define rgb(r, g, b) vec3(r*inv_255, g*inv_255, b*inv_255)
#define red_axis_color rgb(180, 94, 94)
#define blue_axis_color rgb(69, 121, 163)

void main() {
	float n =gl_DepthRange.near;
	float f =gl_DepthRange.far;
	n = 0.01;
	f = 100.0;
	float d = gl_FragCoord.z*2.-1.;
	float ldepth = (2.*n*f)/(f+n-d*(f-n));

	float alpha = smoothstep(1., 0., ldepth/f*3.);
	vec3 col = vec3(.3);

	int relinst = instance > 101 ? instance - 102 : instance;
	if(relinst % 10 == 0) col = vec3(0.4);

	if(vpos.x == 0. || vpos.z==0.) {
		if(instance > 101) col = red_axis_color;
		else col = blue_axis_color;
	}

	FRAG_COL = vec4(col, alpha);
}