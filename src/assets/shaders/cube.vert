#version 460 core
#define SIDE_LEN 1000
precision highp float;
layout(location=0) in vec2 pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;




flat out int inst_id;
out vec3 vpos;
out vec3 vnormal;
void main() {
	vec3 p = vec3(pos.x, 0., pos.y) * 0.001;
	
	float offsetX = float(gl_InstanceID % SIDE_LEN) * 0.001;
	float offsetZ = float(gl_InstanceID / SIDE_LEN) * 0.001;
	
	p += vec3(offsetX, 0., offsetZ);
	p.xz = p.xz * 3.14*2. - 3.14;

	p.y = sin(p.x);

	inst_id = int(gl_InstanceID);
	vpos = p;

	vec3 T = vec3(p.x+0.001, sin(p.x+0.001), p.z);
	vec3 B = vec3(p.x, p.y, p.z + 0.001);
	vnormal = normalize(cross(T - p, B - p));

	gl_Position = projection * view * model * vec4(p, 1.);
}