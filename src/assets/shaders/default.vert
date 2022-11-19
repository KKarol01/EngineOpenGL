#version 460 core

layout(location=0) in vec3 pos;
layout(location=1) in vec3 norm;
layout(location=2) in vec2 tc;

layout(std140, binding=2) uniform UserSettings {
    mat4 p, v;
    vec4 cam_dir, cam_pos, lpos, lcol;
    float attenuation;
    int use_pbr;
};
layout(std140, binding=4) uniform ModelMats { 
    mat4 models[2];
};

float prng(in vec2 seed) {
	seed = fract (seed * vec2 (5.3983, 5.4427));
	seed += dot (seed.yx, seed.xy + vec2 (21.5351, 14.3137));
	return fract (seed.x * seed.y * 95.4337);
}

out vec3 n;
out vec2 tcc;
out vec3 wpos;
flat out int instanceid;
uniform float time;

void main() {
    instanceid = gl_InstanceID;
    n = norm;
    tcc = tc;

    vec4 npos = vec4(pos, 1.);

    if(gl_InstanceID == 0){
        npos = models[gl_InstanceID] * npos;    
    }
    else npos = (mat4(1.)*0.9999) * npos;
    npos.w = 1.f;
    //npos = npos + normalize(vec4(norm*prng(tc), 1.)) * pow(tc.y, 2.) * (sin(time*(5.*prng(vec2(dot(norm.xy, tc), dot(tc, norm.xy)))))*.5+.5)*.1;

	gl_Position = p * v * npos;
}