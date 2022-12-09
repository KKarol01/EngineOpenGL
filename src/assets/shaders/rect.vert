#version 460 core

layout(location=0) in vec3 pos;
uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

flat out int iid;
flat out int instance;
out vec3 vpos;
void main() {
iid = gl_InstanceID;
	vec3 offset = gl_InstanceID*vec3(1., 0., 0.) + vec3(-50., 0., 0.);
	vpos = pos + offset;
	instance = gl_BaseInstance + gl_InstanceID;
	gl_Position = proj * view * model * vec4(vpos, 1.);	
}