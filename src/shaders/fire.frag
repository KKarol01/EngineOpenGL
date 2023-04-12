#version 460 core

in vec2 vpos;
out vec4 FRAG_COLOR;

layout(binding = 0) uniform sampler2D pbralb;
layout(binding = 1) uniform sampler2D pbrdepth;
layout(binding = 2) uniform sampler2D flamealb;
layout(binding = 3) uniform sampler2D flamedepth;
layout(binding = 4) uniform sampler2D flamedist;
layout(binding = 5) uniform sampler2D flamesmoke;

uniform float exposure;

void main() {
    vec2 tc = vec2(vpos * .5 + .5);

    float pbrd   = texture(pbrdepth, tc).x;
    float flamed = texture(flamedepth, tc).x;

    vec2 dtc = tc;
    if (flamed < pbrd) {
        dtc += texture(flamedist, tc).rb * 0.003;
        pbrd   = texture(pbrdepth, dtc).x;
        flamed = texture(flamedepth, dtc).x;
    }

    vec4 alb = texture(pbralb, dtc);
    if (length(alb.xyz) != 0.) FRAG_COLOR = alb;

    alb = texture(flamealb, tc);
    if (flamed < pbrd) {
        FRAG_COLOR += vec4(alb.xyz, alb.x);

        vec4 smoke = texture(flamesmoke, tc);
        FRAG_COLOR += vec4(0.f.xxx, smoke.x * .3);
    }

    const float gamma = 2.2;
    vec3 hdrColor     = FRAG_COLOR.rgb;
    vec3 mapped       = vec3(1.0) - exp(-hdrColor * exposure);
    mapped            = pow(mapped, vec3(1.0 / gamma));
    FRAG_COLOR        = vec4(mapped, FRAG_COLOR.a);
}
