#ifndef INOISE
#define INOISE
#include "decl.hlsl"

/*
    FX implementation of Ken Perlin's "Improved Noise"
    sgg 6/26/04
    http://mrl.nyu.edu/~perlin/noise/
*/



float3 fade(float3 t)
{
	return t * t * t * (t * (t * 6 - 15) + 10); // new curve
//	return t * t * (3 - 2 * t); // old curve
}

float4 perm2d(float2 p)
{
	//return tex2D(permSampler2d, p);
	return permTexture2d.SampleLevel(permSampler2d, p, 0 );
}

float gradperm(float x, float3 p)
{
	//return dot(tex2D(permGradSampler, x), p);
	return dot(permGradTexture.SampleLevel(permGradSampler, x, 0).xyz, p);
}


// 3D noise

// optimized version
float inoise(float3 p)
{

	float3 P = fmod(floor(p), 256.0);	// FIND UNIT CUBE THAT CONTAINS POINT
  	p -= floor(p);                      // FIND RELATIVE X,Y,Z OF POINT IN CUBE.
	float3 f = fade(p);                 // COMPUTE FADE CURVES FOR EACH OF X,Y,Z.

	P = P / 256.0;
	const float one = 1.0 / 256.0;
	
    // HASH COORDINATES OF THE 8 CUBE CORNERS
	float4 AA = perm2d(P.xy) + P.z;
 
	// AND ADD BLENDED RESULTS FROM 8 CORNERS OF CUBE
  	return lerp( lerp( lerp( gradperm(AA.x, p ),  
                             gradperm(AA.z, p + float3(-1, 0, 0) ), f.x),
                       lerp( gradperm(AA.y, p + float3(0, -1, 0) ),
                             gradperm(AA.w, p + float3(-1, -1, 0) ), f.x), f.y),
                             
                 lerp( lerp( gradperm(AA.x+one, p + float3(0, 0, -1) ),
                             gradperm(AA.z+one, p + float3(-1, 0, -1) ), f.x),
                       lerp( gradperm(AA.y+one, p + float3(0, -1, -1) ),
                             gradperm(AA.w+one, p + float3(-1, -1, -1) ), f.x), f.y), f.z);
}

// fractal sum
float fBm(float3 p, int octaves, float lacunarity = 2.0, float gain = 0.5)
{
	float freq = 1.0, amp = 0.5;
	float sum = 0;	
	for(int i=0; i<octaves; i++) {
		sum += inoise(p*freq)*amp;
		freq *= lacunarity;
		amp *= gain;
	}
	return sum;
}

float turbulence(float3 p, int octaves, float lacunarity = 2.0, float gain = 0.5)
{
	float sum = 0;
	float freq = 1.0, amp = 1.0;
	for(int i=0; i<octaves; i++) {
		sum += abs(inoise(p*freq))*amp;
		freq *= lacunarity;
		amp *= gain;
	}
	return sum;
}

float ridge(float h, float offset)
{
    h = abs(h);
    h = offset - h;
    h = h * h;
    return h;
}

float ridgedmf(float3 p, int octaves, float lacunarity = 2.0, float gain = 0.5, float offset = 1.0)
{
	float sum = 0;
	float freq = 1.0, amp = 0.5;
	float prev = 1.0;
	for(int i=0; i<octaves; i++) {
		float n = ridge(inoise(p*freq), offset);
		sum += n*amp*prev;
		prev = n;
		freq *= lacunarity;
		amp *= gain;
	}
	return sum;
}

#endif
