#version 460 core

out vec3 FRAG_COLOR;

uniform sampler2D src_tex;

in vec2 v_tc;

void main() {
	vec2 src_texel_size = 1.0 / vec2(textureSize(src_tex, 0).xy);

    vec2 texCoord = v_tc;
    float x = src_texel_size.x;
    float y = src_texel_size.y;

    vec3 a = texture(src_tex, vec2(texCoord.x - 2*x, texCoord.y + 2*y)).rgb;
    vec3 b = texture(src_tex, vec2(texCoord.x,       texCoord.y + 2*y)).rgb;
    vec3 c = texture(src_tex, vec2(texCoord.x + 2*x, texCoord.y + 2*y)).rgb;

    vec3 d = texture(src_tex, vec2(texCoord.x - 2*x, texCoord.y)).rgb;
    vec3 e = texture(src_tex, vec2(texCoord.x,       texCoord.y)).rgb;
    vec3 f = texture(src_tex, vec2(texCoord.x + 2*x, texCoord.y)).rgb;

    vec3 g = texture(src_tex, vec2(texCoord.x - 2*x, texCoord.y - 2*y)).rgb;
    vec3 h = texture(src_tex, vec2(texCoord.x,       texCoord.y - 2*y)).rgb;
    vec3 i = texture(src_tex, vec2(texCoord.x + 2*x, texCoord.y - 2*y)).rgb;

    vec3 j = texture(src_tex, vec2(texCoord.x - x, texCoord.y + y)).rgb;
    vec3 k = texture(src_tex, vec2(texCoord.x + x, texCoord.y + y)).rgb;
    vec3 l = texture(src_tex, vec2(texCoord.x - x, texCoord.y - y)).rgb;
    vec3 m = texture(src_tex, vec2(texCoord.x + x, texCoord.y - y)).rgb;

    FRAG_COLOR = e*0.125;
    FRAG_COLOR += (a+c+g+i)*0.03125;
    FRAG_COLOR += (b+d+f+h)*0.0625;
    FRAG_COLOR += (j+k+l+m)*0.125;
}