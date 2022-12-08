#version 460 core

out vec4 FRAG_COL;
in vec3 vpos;

uniform mat4 proj;
uniform mat4 view;
uniform vec3 pos;
uniform vec3 dir;



void main() {
	
	vec4 cam_ray = inverse(proj) * (vec4(vpos.xy*2.-1., -1,1.));
	cam_ray/=cam_ray.w;
	cam_ray = inverse(view) * cam_ray;
	cam_ray = normalize(cam_ray.xyz - pos).xyzz;

	bool hit = false;

	vec3 a = vec3(-5, 0, 5);
	vec3 b = vec3( 5, 0,-5);
	vec3 n = vec3(0, -1, 0);
	float t = 0.f;
	float d = dot(n, cam_ray.xyz);
	if(d > 1e-6) {
		vec3 p = -pos;
		t = dot(p, n) / d;
		hit = t>=0.f;
	}
	if(hit) {
		vec3 h = pos + t * cam_ray.xyz;

		if(
			(h.x > a.x  && h.z < a.z
			&&h.x < b.x  && h.z > b.z) == false) hit = false;

	}


	if(hit == false) discard;
	FRAG_COL = vec4(1.f.xxx, 1.);
}