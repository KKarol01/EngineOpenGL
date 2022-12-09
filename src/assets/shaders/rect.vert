#version 460 core

layout(location=0) in vec3 pos;
uniform mat4 proj;
uniform mat4 view;
flat out int iid;
void main() {
iid = gl_InstanceID;
	vec3 offset = gl_InstanceID*vec3(1., 0., 0.) + vec3(-50., 0., 0.);
	gl_Position = proj * view * vec4(pos + offset, 1.);	
}