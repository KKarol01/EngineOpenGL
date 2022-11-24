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
	float off = 1.0;
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

vec3 noiseStackUV(vec3 pos,int octaves,float falloff,float diff){
	float displaceA = noiseStack(pos,octaves,falloff);
	float displaceB = noiseStack(pos+vec3(3984.293,423.21,5235.19),octaves,falloff);
	float displaceC = noiseStack(pos+vec3(34.293,12423.21,52535.11349),octaves,falloff);
	return vec3(displaceA,displaceB,displaceC);
}

out vec4 FRAG_COL;
in vec2 vpos;

uniform mat4 model;
uniform mat4 rotmat;
uniform mat4 ortho;
uniform vec3 cam_view;
uniform vec3 cam_pos;
uniform float time;

uniform mat4 view;
uniform mat4 projection;

struct Ray {
	vec3 o, d;
};
struct RayHitInfo {
	bool hit;
	float tmin, tmax;
};



void ray_box(const in Ray r,
			 const in vec3 bmin,
			 const in vec3 bmax,
			 out RayHitInfo info) 
{
	vec3 invdir = 1./r.d;

    float tmin = 0.0, tmax = 1e20;

    for (int i = 0; i < 3; ++i) {
        float t1 = (bmin[i] - r.o[i]) * invdir[i];
        float t2 = (bmax[i] - r.o[i]) * invdir[i];

        tmin = min(max(t1, tmin), max(t2, tmin));
        tmax = max(min(t1, tmax), min(t2, tmax));
    }

	info.hit = tmin <= tmax;
	info.tmin = tmin;
	info.tmax = tmax;
}

layout(binding=0) uniform sampler3D tex;

void main() {
	vec4 bmin = vec4(-1.f.xxx, 1.f);
	vec4 bmax = vec4(1.f.xxx, 1.f);
	Ray r;
	RayHitInfo info;
	vec4 rd = vec4(vpos, -1., 1.);
	rd = inverse(projection) * rd;
	rd/=rd.w;
	rd = rotmat * rd;
	r.o = normalize(cam_pos)*2.;
	r.d = normalize(rd.xyz);

	ray_box(r, bmin.xyz, bmax.xyz, info); //Tavian Barnes 
									//branchless ray/bb intersection alg.

	if(!info.hit) {discard;}
	vec3 a = r.o + info.tmin*r.d;
	vec3 b = r.o + info.tmax*r.d;


	int samples = 5;
	vec3 ds = 0.6*(b-a) / float(samples);
	float dsl =length(ds);
	vec3 acc = vec3(0.);
	for(int i=0; i<samples;++i) {
		vec3 tc = a + ds*i;

		vec3 nc = tc*1.2;
		vec3 nc2 = nc*nc;
		float lnc2 = nc2.x+nc2.y+nc2.z;
		float yatt = .4;
		float xatt = smoothstep(1., 0.1, length(nc.xz + (1.2-yatt)*snoise(nc+time*vec3(0., -2., 0.))*.13)*1.);
		float n = smoothstep(.8 + snoise(nc*vec3(1., 5., 3.)*(1.-yatt)+time*vec3(1., -4., 0.2))*.07, 0.1, lnc2);
		n*= yatt * xatt;
		n = pow(n, 2.);
		acc += n * dsl * 18. * samples;

	}
	acc /= samples;
	acc = 1.5*pow(acc, vec3(1., 2., 4.));	
	FRAG_COL = vec4(acc, acc.x);
}
