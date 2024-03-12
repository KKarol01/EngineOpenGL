#version 460 core
#extension GL_ARB_bindless_texture : require

#define PI     3.14159265
#define INV_PI 0.31830988

#define saturate(x) clamp(x, 0.0, 1.0)

out vec4 FRAG_COL;

struct A {
    uvec2 diffuse;
    uvec2 normal;
    uvec2 metallic;
    uvec2 roughness;
    uvec2 emissive;
    vec4 channels;
    mat4 transform;
};

layout(std430, binding = 0) buffer VT { A a[]; };
layout(binding=5) uniform sampler2D texas;

flat in uint idx;
flat in mat3 TBN;
in V_OUT { vec3 v_pos; vec3 v_normal; } v_out;

uniform vec3 view_vec;
uniform vec3 view_pos;

/*

f = fd + fr



*/

vec3 fresnelSchlick(float u, vec3 f0, float f90)
{
    return f0 + (vec3(f90) - f0) * pow(1.0 - u, 5.0);
}  

float Fd_Lambert() {
    return INV_PI;
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	float NoH = saturate(dot(N, H));
	float a = roughness;
    float a2 = a * a;
    float f = (NoH * a2 - NoH) * NoH + 1.0;
    return a2 / (PI * f * f);
}

float GeometrySchlickGGX(float NoV, float NoL, float roughness)
{
	float a = roughness;
    float a2 = a * a;
    float GGXL = NoV * sqrt((-NoL * a2 + NoL) * NoL + a2);
    float GGXV = NoL * sqrt((-NoV * a2 + NoV) * NoV + a2);
    return 0.5 / (GGXV + GGXL);
}

vec4 BRDF(vec3 v) {
    vec2 tc = v_out.v_normal.xy;
    vec3 diffuse_color = texture(sampler2D(a[idx].diffuse),   tc).rgb;
    vec3 normal_color  = texture(sampler2D(a[idx].normal),    tc).rgb;
	vec3 emissive_color  = texture(sampler2D(a[idx].emissive),tc).rgb;
    float metalness    = texture(sampler2D(a[idx].metallic),  tc)[int(a[idx].channels[1])];
    float roughness    = texture(sampler2D(a[idx].roughness), tc)[int(a[idx].channels[2])];

    const vec3 n = TBN * (2.0 * normal_color - 1.0);
    const float a = clamp(pow(roughness, 2.0), 0.089, 1.0);
    const float a2 = a*a;

    vec3 F0 = 0.16 * 0.5*0.5 * (1.0-metalness) + diffuse_color*metalness;

    vec3 Lo = vec3(0.0);
    vec3 ls[4] = {
		vec3(sin(0.0 * 2.0*PI / 4.0), 0.0, cos(0.0 * 2.0*PI / 4.0)),
		vec3(sin(1.0 * 2.0*PI / 4.0), 0.0, cos(1.0 * 2.0*PI / 4.0)),
		vec3(sin(2.0 * 2.0*PI / 4.0), 0.0, cos(2.0 * 2.0*PI / 4.0)),
		vec3(sin(3.0 * 2.0*PI / 4.0), 0.0, cos(3.0 * 2.0*PI / 4.0)),
    };

    for (int i=0; i<4; ++i) {
        vec3 l = normalize(ls[i]*10.0 - v_out.v_pos);
		vec3 h = normalize(v+l);
		float NoH = saturate(dot(n, h));
		float LoH = saturate(dot(l, h));
		float NoL = saturate(dot(n, l));
		float NoV = abs(dot(n,v))+1e-5;
		float D 	= DistributionGGX(n, h, a);        
		float V   = GeometrySchlickGGX(NoV, NoL, a);      
		vec3 F    = fresnelSchlick(LoH, F0, 1.0);       
			
		Lo += ((D*V) * F + diffuse_color*Fd_Lambert()) * NoH;
    } 
	
    vec3 color = Lo + emissive_color * 30.0;

	
    return vec4(color, 1.0);
}

void main() {
    vec3 lpos = vec3(10.0, 0.0, 10.0);
    
    vec3 v = normalize(view_pos - v_out.v_pos);

    FRAG_COL = BRDF(v);
}