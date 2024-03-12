#version 460 core

in VS_OUT {
	vec3 pos, normal;
} vs_out;

uniform vec3 cam_vec;
uniform float appbias;
uniform float apppower;
uniform float appscale;

layout(binding=1) uniform samplerCube skybox;

out vec4 FRAG_COLOR;
void main() {
	vec3 position = vs_out.pos;
	vec3 normal = normalize(vs_out.normal);
	vec3 pos_cam = position - cam_vec;


	float fresnel = max(0.f, min(1.f, appbias + appscale*pow(1.f+dot(normalize(pos_cam), normal), apppower)));
	
	vec3 tex_color = texture(skybox, reflect(normalize(pos_cam), normal)).rgb;
	vec3 blue_color = vec3(96.f, 130.f, 182.f)/255.f * 0.4f;
	
	FRAG_COLOR = vec4(normal, 1.f);
	FRAG_COLOR = vec4(fresnel.xxx, 1.f);
	FRAG_COLOR = vec4(blue_color + (fresnel*tex_color), 1.f);
}