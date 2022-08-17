#version 460 core

layout(vertices=3) out;

uniform vec3 cam_pos;
uniform mat4 view;
uniform mat4 model;

const int MIN_TES = 2;
const int MAX_TES = 5;
const float MAX_INVDIST = 0.5f; //inverted to avoid further divisions

out vec3 tcposition[];

void main() {
	tcposition[gl_InvocationID] = gl_in[gl_InvocationID].gl_Position.xyz;
	
	if(gl_InvocationID != 0) return;

	vec4 v1 = model * gl_in[0].gl_Position;
	vec4 v2 = model * gl_in[1].gl_Position;
	vec4 v3 = model * gl_in[2].gl_Position;

	vec4 center1 = v1 + 0.5f*(v2-v1);
	vec4 center2 = v1 + 0.5f*(v3-v1);
	vec4 center3 = v2 + 0.5f*(v3-v2);

	float dist1 = length(cam_pos - center1.xyz);
	float dist2 = length(cam_pos - center2.xyz);
	float dist3 = length(cam_pos - center3.xyz);

	int tes0 = int(mix(MAX_TES, MIN_TES, clamp(dist1/MAX_INVDIST,0.f,1.f)));
	int tes1 = int(mix(MAX_TES, MIN_TES, clamp(dist2/MAX_INVDIST,0.f,1.f)));
	int tes2 = int(mix(MAX_TES, MIN_TES, clamp(dist3/MAX_INVDIST,0.f,1.f)));

	gl_TessLevelOuter[0] = tes0;
	gl_TessLevelOuter[1] = tes1;
	gl_TessLevelOuter[2] = tes2;

	gl_TessLevelInner[0] = max(max(tes0,tes1),tes2);
}