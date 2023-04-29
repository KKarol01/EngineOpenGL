#version 460 core

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNorm;

layout(std430, binding = 0) buffer VData { mat4 transforms[]; }
vdata;

uniform mat4 v;
uniform mat4 p;

//out V_OUT { vec3 vNorm; }
//v_out;

void main() {
    const uint idx = gl_BaseInstance + gl_InstanceID;
   // v_out.vNorm    = vNorm;
    gl_Position    = p * v * vdata.transforms[idx] * vec4(vPos, 1.0);
}