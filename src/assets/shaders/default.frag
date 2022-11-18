#version 460 core

vec3 mod289(vec3 x) {
	return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x) {
	return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
		 return mod289(((x*34.0)+1.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
	return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v)
	{ 
	const vec2	C = vec2(1.0/6.0, 1.0/3.0) ;
	const vec4	D = vec4(0.0, 0.5, 1.0, 2.0);

// First corner
	vec3 i	= floor(v + dot(v, C.yyy) );
	vec3 x0 =	 v - i + dot(i, C.xxx) ;

// Other corners
	vec3 g = step(x0.yzx, x0.xyz);
	vec3 l = 1.0 - g;
	vec3 i1 = min( g.xyz, l.zxy );
	vec3 i2 = max( g.xyz, l.zxy );

	//	 x0 = x0 - 0.0 + 0.0 * C.xxx;
	//	 x1 = x0 - i1	+ 1.0 * C.xxx;
	//	 x2 = x0 - i2	+ 2.0 * C.xxx;
	//	 x3 = x0 - 1.0 + 3.0 * C.xxx;
	vec3 x1 = x0 - i1 + C.xxx;
	vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
	vec3 x3 = x0 - D.yyy;			// -1.0+3.0*C.x = -0.5 = -D.y

// Permutations
	i = mod289(i); 
	vec4 p = permute( permute( permute( 
						 i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
					 + i.y + vec4(0.0, i1.y, i2.y, 1.0 )) 
					 + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

// Gradients: 7x7 points over a square, mapped onto an octahedron.
// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
	float n_ = 0.142857142857; // 1.0/7.0
	vec3	ns = n_ * D.wyz - D.xzx;

	vec4 j = p - 49.0 * floor(p * ns.z * ns.z);	//	mod(p,7*7)

	vec4 x_ = floor(j * ns.z);
	vec4 y_ = floor(j - 7.0 * x_ );		// mod(j,N)

	vec4 x = x_ *ns.x + ns.yyyy;
	vec4 y = y_ *ns.x + ns.yyyy;
	vec4 h = 1.0 - abs(x) - abs(y);

	vec4 b0 = vec4( x.xy, y.xy );
	vec4 b1 = vec4( x.zw, y.zw );

	//vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
	//vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
	vec4 s0 = floor(b0)*2.0 + 1.0;
	vec4 s1 = floor(b1)*2.0 + 1.0;
	vec4 sh = -step(h, vec4(0.0));

	vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
	vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

	vec3 p0 = vec3(a0.xy,h.x);
	vec3 p1 = vec3(a0.zw,h.y);
	vec3 p2 = vec3(a1.xy,h.z);
	vec3 p3 = vec3(a1.zw,h.w);

//Normalise gradients
	//vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
	vec4 norm = inversesqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
	p0 *= norm.x;
	p1 *= norm.y;
	p2 *= norm.z;
	p3 *= norm.w;

// Mix final noise value
	vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
	m = m * m;
	return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1), 
																dot(p2,x2), dot(p3,x3) ) );
	}

//////////////////////////////////////////////////////////////

// PRNG
// From https://www.shadertoy.com/view/4djSRW
float prng(in vec2 seed) {
	seed = fract (seed * vec2 (5.3983, 5.4427));
	seed += dot (seed.yx, seed.xy + vec2 (21.5351, 14.3137));
	return fract (seed.x * seed.y * 95.4337);
}

//////////////////////////////////////////////////////////////

float PI = 3.1415926535897932384626433832795;


float noiseStack(vec3 pos,int octaves,float falloff){
	float noise = snoise(vec3(pos));
	float off = 1.;
	if (octaves>1) {
		pos *= 2.0;
		off *= falloff;
		noise = (1.0-off)*noise + off*snoise(vec3(pos));
	}
	if (octaves>2) {
		pos *= 2.0;
		off *= falloff;
		noise = (1.0-off)*noise + off*snoise(vec3(pos));
	}
	if (octaves>3) {
		pos *= 2.0;
		off *= falloff;
		noise = (1.0-off)*noise + off*snoise(vec3(pos));
	}
	return (1.0+noise)/2.0;
}
vec2 noiseStackUV(vec3 pos,int octaves,float falloff){
	float displaceA = noiseStack(pos,octaves,falloff);
	float displaceB = noiseStack(pos+vec3(3984.293,423.21,5235.19),octaves,falloff);
	return vec2(displaceA,displaceB);
}



layout(std140, binding=5) uniform FIRE_SETTINGS {
	float clip;
	float xatt;
	float speed;
	float scale;
	float flowxmult;
	float flowymult;
	vec4 flow_dir;
	vec4 dsp;
	float dspmult;
	float dsp3_noiseatt;
	vec4 noiseres;
	float noisedsp3factor;
	float noisesmoothness;
	vec4 color_weights_mult;
	float alpha_threshold;
};


vec3 aces_approx(vec3 v)
{
    v *= 0.6f;
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return clamp((v*(a*v+b))/(v*(c*v+d)+e), 0.0f, 1.0f);
}

out vec4 FRAG_COLOR;
in vec3 wpos;
in vec3 n;
in vec2 tcc;
uniform float time;
void main() {
float realTime = time * speed;
    vec2 p = tcc.xy * 2.;
    vec3 sp = vec3(p, 0.)*scale;

    float yclip = p.y/clip;
    float yclipped = min(yclip, 1.);
    float yclipn = 1.-yclipped;
    float yatt = clamp(1.-yclip, 0., 1.);

    //zoom in
    
	vec3 position = sp;
	position.x += time+pow(tcc.y, 2.);
	
    //x-makes fire "go" to the middle
    //y-makes fire "go" upwards
    vec3 flow = vec3(flowxmult*(0.5-p.x)*pow(yclipn,4.),flowymult*xatt*pow(yclipn,64.0),0.0);
	vec3 timing = realTime*flow_dir.xyz + flow;
	vec3 displacePos = dsp.xyz*dspmult*position+realTime*vec3(0.01,-0.7,1.3);
	vec3 displace3 = vec3(noiseStackUV(displacePos,3,dsp3_noiseatt),0.0);
	vec3 noiseCoord = (noiseres.xyz* position+timing+noisedsp3factor*displace3)/noisesmoothness;
	float noise = noiseStack(noiseCoord,3,0.5);
    float flame = pow(yclipped, 0.32*xatt)*pow(noise, 0.1*xatt);
    flame = yatt*pow(1.-pow(flame, 3.3), 6.5);

    float f = pow(flame, color_weights_mult.x), f3 = pow(flame, color_weights_mult.y), f6 = pow(flame, color_weights_mult.z);
	
    FRAG_COLOR =  color_weights_mult.w*vec4(aces_approx(vec3(f, f3, f6)), flame< alpha_threshold ? 0. : 1.);
}

/*
	
		float realTime = time * speed;
    vec2 p = tcc.xy;
    vec3 sp = vec3(p, 0.)*scale * 2.;

    float yclip = p.y/clip;
    float yclipped = min(yclip, 1.);
    float yclipn = 1.-yclipped;
    float yatt = clamp(1.-yclip, 0., 1.);

	vec3 position = sp;
	position.x += time+pow(tcc.y, 2.);
	
    //x-makes fire "go" to the middle
    //y-makes fire "go" upwards
    vec3 flow = vec3(flowxmult*(0.5-p.x)*pow(yclipn,4.),flowymult*xatt*pow(yclipn,64.0),0.0);
	vec3 timing = realTime*flow_dir.xyz + flow;
	vec3 displacePos = dsp.xyz*dspmult*position+realTime*vec3(0.01,-0.7,1.3);
	vec3 displace3 = vec3(noiseStackUV(displacePos,3,dsp3_noiseatt),0.0);
	vec3 noiseCoord = (noiseres.xyz* position+timing+noisedsp3factor*displace3)/noisesmoothness;
    
    //again, adds details with octaves.
	float noise = noiseStack(noiseCoord,3,0.5);
  
	float alpha = noise < alpha_threshold ? 0. : 1.;
	if(alpha == 0.f) discard;

    FRAG_COLOR =  color_weights_mult.w*vec4(
	pow(noise, color_weights_mult.x),
	pow(noise, color_weights_mult.y),
	pow(noise, color_weights_mult.z), 1.);
*/