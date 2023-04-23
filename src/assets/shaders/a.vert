#version 460 core

layout(location=0) in vec3 pos;

uniform mat4 v;
uniform mat4 p;

layout(std430, binding=0) buffer obj_data {
	mat4 transforms[];
};

void main() {
	
	gl_Position = p * v * transforms[gl_BaseInstance + gl_InstanceID] * vec4(pos.xyz, 1.0);

}