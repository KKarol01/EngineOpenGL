#version 460 core

layout(location=0) in vec3 pos;
layout(location=1) in vec3 normal;

uniform mat4 v;
uniform mat4 p;

layout(std430, binding=0) buffer obj_data {
	mat4 transforms[];
};
layout(std430, binding=1) buffer obj_attributes {
	vec3 normals[];
};
out vec3 n;

void main() {
	int idx = gl_BaseInstance + gl_InstanceID;
	int vidx = gl_VertexID;
	n = normals[vidx].xyz;
	gl_Position = p * v * transforms[idx] * vec4(pos.xyz, 1.0);
}