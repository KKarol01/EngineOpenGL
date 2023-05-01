#version 460 core
#extension GL_ARB_bindless_texture : require
#extension GL_ARB_gpu_shader_int64 : require

#define PI     3.14159265
#define INV_PI 0.31830988

#define saturate(x) clamp(x, 0.0, 1.0)

out vec4 FRAG_COL;

struct A {
    uint64_t diffuse;
    uint64_t normal;
    uint64_t metallic;
    uint64_t roughness;
    uint64_t emissive;
    vec4 channels;
    mat4 transform;
};

layout(std430, binding = 0) buffer VT { A a[]; };

flat in uint idx;
flat in mat3 TBN;
in V_OUT { vec3 v_pos; vec3 v_normal; } v_out;

uniform vec3 view_vec;
uniform vec3 view_pos;

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}  

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec4 BRDF(vec3 v) {
    vec2 tc = v_out.v_normal.xy;
    vec3 diffuse_color = texture(sampler2D(a[idx].diffuse),   tc).rgb;
    vec3 normal_color  = texture(sampler2D(a[idx].normal),    tc).rgb;
    float metalness    = texture(sampler2D(a[idx].metallic),  tc)[int(a[idx].channels[1])];
    float roughness    = texture(sampler2D(a[idx].roughness), tc)[int(a[idx].channels[2])];

    const vec3 n = TBN * (2.0 * normal_color - 1.0);

    float NoV = abs(dot(n, v)) + 1e-5;

    const float a = roughness;
    const float a2 = a*a;

    vec3 F0 = vec3(0.04); 
    F0      = mix(F0, diffuse_color, vec3(metalness));

    vec3 Lo = vec3(0.0);
    vec3 ls[4] = {
		vec3(sin(0.0 * 2.0*PI / 4.0), 5.0, cos(0.0 * 2.0*PI / 4.0)),
		vec3(sin(1.0 * 2.0*PI / 4.0), 5.0, cos(1.0 * 2.0*PI / 4.0)),
		vec3(sin(2.0 * 2.0*PI / 4.0), -5.0, cos(2.0 * 2.0*PI / 4.0)),
		vec3(sin(3.0 * 2.0*PI / 4.0), -5.0, cos(3.0 * 2.0*PI / 4.0)),
    };

    for (int i=0; i<4; ++i) {
        vec3 l = normalize(ls[i] - v_out.v_pos);
		vec3 h = normalize(v+l);
		float NoH = saturate(dot(n, h));
		float LoH = saturate(dot(l, h));
		float NoL = saturate(dot(n, l));
		float NDF = DistributionGGX(n, h, a2);        
		float G   = GeometrySmith(n, v, l, a2);      
		vec3 F    = fresnelSchlick(max(dot(h, v), 0.0), F0);       
			
		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - metalness;	  
			
		vec3 numerator    = NDF * G * F;
		float denominator = 4.0 * max(dot(n, v), 0.0) * max(dot(n, l), 0.0) + 1e-5;
		vec3 specular     = numerator / denominator;  
				
		// add to outgoing radiance Lo
		float NdotL = max(dot(n, l), 0.0);                
		Lo += (kD * diffuse_color / PI + specular) * 0.5 * NoL; 
    } 
    vec3 ambient = vec3(0.03) * diffuse_color /* * ao */;
    vec3 color = ambient + Lo;
	
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));  
   
    return vec4(color, 1.0);
}

void main() {
    vec3 lpos = vec3(10.0, 0.0, 10.0);
    
    vec3 v = normalize(view_pos - v_out.v_pos);

    FRAG_COL = BRDF(v);
}