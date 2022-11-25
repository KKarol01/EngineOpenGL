#version 460 core
#extension GL_ARB_bindless_texture : require
#extension GL_ARB_gpu_shader_int64 : require


out vec4 FRAG_COLOR;

in vec3 fragpos;
in vec3 vn;
in vec2 vtc;
flat in int draw_id;
in mat3 tbn;
uniform float time;

layout(std140, binding=2) uniform UserSettings {
    mat4 p, v, m;
    vec4 cam_dir, cam_pos, lpos, lcol;
    float attenuation;
    int use_pbr;
};

layout(std430, binding = 0) buffer sb_textures {
    uint64_t handles[];
};

const float PI = 3.14159265359;
// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec4 pbrr() {
    vec4 albedo         = texture(sampler2D(handles[draw_id*4+ 0]), vtc.xy).rgba;
	vec3 normal         = texture(sampler2D(handles[draw_id*4+ 1]), vtc.xy).rgb;
	float metalicness   = texture(sampler2D(handles[draw_id*4+ 2]), vtc.xy).r;
	float roughness     = texture(sampler2D(handles[draw_id*4+ 2]), vtc.xy).g;


	vec3 ldir = normalize(lpos.xyz - fragpos);
	normal = normal*2.f - 1.f;
	normal = normalize(tbn * normal);
    vec3 view_dir = normalize(cam_pos.xyz - fragpos);

    vec3 f0 = vec3(.04f);
    f0 = mix(f0, albedo.rgb, metalicness);
    
    vec3 lo = vec3(0.f);
    int steps = 1;

    for(int i=0; i<steps; ++i) {
        vec3 L = normalize(lpos.xyz - fragpos);
        vec3 H = normalize(view_dir + L);

        float distance = length(lpos.xyz - fragpos);
        vec3 radiance = lcol.xyz * attenuation * (sin(time*3.)*.5+.5);

        float NDF = DistributionGGX(normal, H, roughness);
        float G = GeometrySmith(normal, view_dir, L, roughness);
        vec3 F = fresnelSchlick(clamp(dot(H, view_dir), .0f, 1.f), f0);

        vec3 numerator = NDF * G * F;
        float denominator = 4.f * max(dot(normal, view_dir), .0f) * max(dot(normal, L), 0.f) + .0001f;
        vec3 specular = numerator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.f) - kS;
        kD *= 1.f - metalicness;

        float NoL = max(dot(normal, L), 0.f);
        lo += (kD * albedo.rgb / (PI) + specular) * radiance * NoL * 1./float(steps);
    }

    vec3 ambient = vec3(.03f) * albedo.rgb;
    vec3 color= ambient + lo;
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2)); 

    return vec4(color, 1.0);
}

void main() {

	if(use_pbr==1) FRAG_COLOR = pbrr();
    else {
        vec4 albedo =0.f.xxxx;// texture(sampler2D(handles[0]), vtc.xy).rgba;
	    vec3 normal =0.f.xxx; //texture(sampler2D(texture_ids.y), vtc.xy).rgb;
	    normal = normal*2.f - 1.f;
	    normal = normalize(tbn * normal);

	    vec3 ldir = normalize(lpos.xyz - fragpos);
        vec3 view_dir = normalize(cam_pos.xyz - fragpos);

        vec3 ambient =lcol.xyz * 0.3;

        float diff = max(dot(normal, ldir), 0.f);

        float specular_str = 0.5f;
        float spec = pow(max(dot(view_dir, reflect(-ldir, normal)), 0.f), 32.f);

        vec3 diffuse = diff * lcol.xyz;
        vec3 specular = spec * lcol.xyz * specular_str;

        FRAG_COLOR = vec4( (ambient + diffuse + specular) * albedo.rgb, 1.f);

    }
}