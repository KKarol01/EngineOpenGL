#version 460 core

layout(triangles, equal_spacing, ccw) in;

uniform vec3 cam_pos;

uniform float time;
uniform float amp;
uniform float freq;
uniform float speed;
uniform float steepness;
uniform vec2 dir;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

in vec3 tcposition[];
out vec3 position;
out vec3 normal;


float rand(vec2 n) { 
	return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}

float noise(vec2 p){
	vec2 ip = floor(p);
	vec2 u = fract(p);
	u = u*u*(3.0-2.0*u);
	
	float res = mix(
		mix(rand(ip),rand(ip+vec2(1.0,0.0)),u.x),
		mix(rand(ip+vec2(0.0,1.0)),rand(ip+vec2(1.0,1.0)),u.x),u.y);
	return res*res;
}

const vec2 dirs[5] = vec2[](
normalize(vec2(1.2, 4.2)),normalize(vec2(15.2, 4.2)),normalize(vec2(1.2, 0.2))
,normalize(vec2(50.9, 154.2)),normalize(vec2(1.2, 43.2)));

vec3 wave(vec2 p) {
	vec2 d = normalize(dir);
	float phase = speed * freq;
	
	float sum_x = 0.f;
	float sum_y = 0.f;
	float sum_z = 0.f;

	for(int i=0; i<5; ++i) {
		sum_x += steepness*amp*dirs[i].x*cos(freq*dot(dirs[i],p) + phase*time);
		sum_y += amp*sin(freq*dot(dirs[i],p) + phase*time);
		sum_z += steepness*amp*dirs[i].y*cos(freq*dot(dirs[i],p) + phase*time);
	}

	return vec3(p.x + sum_x, sum_y, p.y + sum_z);
}
vec3 N(vec2 p) {
	float sum_x = 0.f;
	float sum_y = 0.f;
	float sum_z = 0.f;
	float phase = speed * freq;
	vec2 d = normalize(dir);

	for(int i=0; i<5; ++i) {
		sum_x += dirs[i].x*freq*amp*cos(freq*dot(dirs[i],p)+phase*time);
		sum_y += dirs[i].y*freq*amp*cos(freq*dot(dirs[i],p)+phase*time);
		sum_z += steepness*freq*amp*sin(freq*dot(dirs[i],p)+phase*time);
	}

	return vec3(-sum_x, 1.f-sum_z, -sum_y);
}

void main() {
	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;
	float w = gl_TessCoord.z;
	vec3 p0 = (model * vec4(tcposition[0], 1.f)).xyz;
	vec3 p1 = (model * vec4(tcposition[1], 1.f)).xyz;
	vec3 p2 = (model * vec4(tcposition[2], 1.f)).xyz;

	vec3 pos = u*p0 + v*p1 + w*p2;
	vec3 wv = wave(pos.xz);
	normal = normalize(N(pos.xz));
	position = (vec4(wv, 1.f)).xyz;

	gl_Position = projection * view * vec4(wv, 1.f);
	gl_Position = projection * view * vec4(pos, 1.f);
}