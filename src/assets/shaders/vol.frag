#version 460 core

out vec4 FRAG_POS;
in vec2 vpos;

uniform vec3 cam_view;
uniform vec3 cam_pos;

uniform mat4 view;
uniform mat4 projection;

struct Ray {
	vec3 o, d;
};
struct RayHitInfo {
	bool hit;
	float tmin, tmax;
};



void ray_box(const in Ray r,
			 const in vec3 bmin,
			 const in vec3 bmax,
			 out RayHitInfo info) 
{
	float tmin = 0.f, tmax = 1e20;

	for(int i=0; i< 3; ++i) {
		float t1 = (bmin[i] - r.o[i]) / r.d[i];
		float t2 = (bmax[i] - r.o[i]) / r.d[i];

		tmin = max(tmin, min(t1, t2));
		tmax = min(tmax, max(t1, t2));
	}

	info.hit  = tmin < tmax;
	info.tmin = tmin;
	info.tmax = tmax;
}

void main() {
	vec4 cam_bmin = vec4(0.f.xxx, 1.f);
	vec4 cam_bmax = vec4(1.f.xxx, 1.f);

	cam_bmin /= cam_bmin.w;
	cam_bmax /= cam_bmax.w;

	Ray r;
	RayHitInfo info;

	r.o = vec3(0.f);

	vec2 ppos = vpos;

	float cx = ppos.x * (1920./1080.)*tan(45./180.*3.14);
	float cy = (ppos.y *tan(45./180.*3.14));
	mat4 cam2world = inverse(view);
	r.o = (cam2world * vec4(r.o, 1.)).xyz;
	r.d = (cam2world*vec4(cx, cy, -1., 1.)).xyz ;
	r.d = r.d - r.o;
	r.d = normalize(r.d);


	ray_box(r, cam_bmin.xyz, cam_bmax.xyz, info);

	if(info.hit) FRAG_POS = vec4(1.f);
	else discard;
}