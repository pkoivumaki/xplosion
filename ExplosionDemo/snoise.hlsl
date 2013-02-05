#ifndef SNOISE
#define SNOISE
#include "decl.hlsl"

// Simplex noise implementation by David Watt

/*
 * To create offsets of one texel and one half texel in the
 * texture lookup, we need to know the texture image size.
 */
//#define 0.00390625 0.00390625
//#define 0.001953125 0.001953125
// The numbers above are 1/256 and 0.5/256, change accordingly
// if you change the code to use another texture size.


float snoise(float2 P) 
{

// Skew and unskew factors are a bit hairy for 2D, so define them as constants
// This is (sqrt(3.0)-1.0)/2.0
//#define F2 0.366025403784
// This is (3.0-sqrt(3.0))/6.0
//#define G2 0.211324865405

  // Skew the (x,y) space to determine which cell of 2 simplices we're in
 	float s = (P.x + P.y) * 0.366025403784;   // Hairy factor for 2D skewing
  float2 Pi = floor(P + s);
  float t = (Pi.x + Pi.y) * 0.211324865405; // Hairy factor for unskewing
  float2 P0 = Pi - t; // Unskew the cell origin back to (x,y) space
  Pi = Pi * 0.00390625 + 0.001953125; // Integer part, scaled and offset for texture lookup

  float2 Pf0 = P - P0;  // The x,y distances from the cell origin

  // For the 2D case, the simplex shape is an equilateral triangle.
  // Find out whether we are above or below the x=y diagonal to
  // determine which of the two triangles we're in.
  float2 o1;
  if(Pf0.x > Pf0.y) o1 = float2(1.0, 0.0);  // +x, +y traversal order
  else o1 = float2(0.0, 1.0);               // +y, +x traversal order

  // Noise contribution from simplex origin
  float2 grad0 = permTexture2d.SampleLevel(permSampler2d, Pi, 0).rg * 4.0 - 1.0;
  float t0 = 0.5 - dot(Pf0, Pf0);
  float n0;
  if (t0 < 0.0) n0 = 0.0;
  else {
    t0 *= t0;
    n0 = t0 * t0 * dot(grad0, Pf0);
  }

  // Noise contribution from middle corner
  float2 Pf1 = Pf0 - o1 + 0.211324865405;
  float2 grad1 = permTexture2d.SampleLevel(permSampler2d, Pi + o1*0.00390625, 0).rg * 4.0 - 1.0;
  float t1 = 0.5 - dot(Pf1, Pf1);
  float n1;
  if (t1 < 0.0) n1 = 0.0;
  else {
    t1 *= t1;
    n1 = t1 * t1 * dot(grad1, Pf1);
  }
  
  // Noise contribution from last corner
  float2 Pf2 = Pf0 - float2(1.0-2.0*0.211324865405,1.0-2.0*0.211324865405);
  float2 grad2 = permTexture2d.SampleLevel(permSampler2d, Pi + float2(0.00390625, 0.00390625), 0).rg * 4.0 - 1.0;
  float t2 = 0.5 - dot(Pf2, Pf2);
  float n2;
  if(t2 < 0.0) n2 = 0.0;
  else {
    t2 *= t2;
    n2 = t2 * t2 * dot(grad2, Pf2);
  }

  // Sum up and scale the result to cover the range [-1,1]
  return 70.0 * (n0 + n1 + n2);
}

float snoise(float3 P) 
{

// The skewing and unskewing factors are much simpler for the 3D case
//#define F3 0.333333333333
//#define G3 0.166666666667

  // Skew the (x,y,z) space to determine which cell of 6 simplices we're in
 	float s = (P.x + P.y + P.z) * 0.333333333333; // Factor for 3D skewing
  float3 Pi = floor(P + s);
  float t = (Pi.x + Pi.y + Pi.z) * 0.166666666667;
  float3 P0 = Pi - t; // Unskew the cell origin back to (x,y,z) space
  Pi = Pi * 0.00390625 + 0.001953125; // Integer part, scaled and offset for texture lookup

  float3 Pf0 = P - P0;  // The x,y distances from the cell origin

  // For the 3D case, the simplex shape is a slightly irregular tetrahedron.
  // To find out which of the six possible tetrahedra we're in, we need to
  // determine the magnitude ordering of x, y and z components of Pf0.
  // The method below is explained briefly in the C code. It uses a small
  // 1D texture as a lookup table. The table is designed to work for both
  // 3D and 4D noise, so only 8 (only 6, actually) of the 64 indices are
  // used here.
  float c1 = (Pf0.x > Pf0.y) ? 0.5078125 : 0.0078125; // 1/2 + 1/128
  float c2 = (Pf0.x > Pf0.z) ? 0.25 : 0.0;
  float c3 = (Pf0.y > Pf0.z) ? 0.125 : 0.0;
  float sindex = c1 + c2 + c3;
  float3 offsets = simplexGradTexture.SampleLevel(permGradSampler, sindex, 0).rgb;
  float3 o1 = step(0.375, offsets);
  float3 o2 = step(0.125, offsets);

  // Noise contribution from simplex origin
  float perm0 = permTexture2d.SampleLevel(permSampler2d, Pi.xy, 0).a;
  float3  grad0 = permTexture2d.SampleLevel(permSampler2d, float2(perm0, Pi.z), 0).rgb * 4.0 - 1.0;
  float t0 = 0.6 - dot(Pf0, Pf0);
  float n0;
  if (t0 < 0.0) n0 = 0.0;
  else {
    t0 *= t0;
    n0 = t0 * t0 * dot(grad0, Pf0);
  }

  // Noise contribution from second corner
  float3 Pf1 = Pf0 - o1 + 0.166666666667;
  float perm1 = permTexture2d.SampleLevel(permSampler2d, Pi.xy + o1.xy*0.00390625, 0).a;
  float3  grad1 = permTexture2d.SampleLevel(permSampler2d, float2(perm1, Pi.z + o1.z*0.00390625), 0).rgb * 4.0 - 1.0;
  float t1 = 0.6 - dot(Pf1, Pf1);
  float n1;
  if (t1 < 0.0) n1 = 0.0;
  else {
    t1 *= t1;
    n1 = t1 * t1 * dot(grad1, Pf1);
  }
  
  // Noise contribution from third corner
  float3 Pf2 = Pf0 - o2 + 2.0 * 0.166666666667;
  float perm2 = permTexture2d.SampleLevel(permSampler2d, Pi.xy + o2.xy*0.00390625, 0).a;
  float3  grad2 = permTexture2d.SampleLevel(permSampler2d, float2(perm2, Pi.z + o2.z*0.00390625), 0).rgb * 4.0 - 1.0;
  float t2 = 0.6 - dot(Pf2, Pf2);
  float n2;
  if (t2 < 0.0) n2 = 0.0;
  else {
    t2 *= t2;
    n2 = t2 * t2 * dot(grad2, Pf2);
  }
  
  // Noise contribution from last corner
  float3 Pf3 = Pf0 - float3(1.0-3.0*0.166666666667,1.0-3.0*0.166666666667,1.0-3.0*0.166666666667);
  float perm3 = permTexture2d.SampleLevel(permSampler2d, Pi.xy + float2(0.00390625, 0.00390625), 0).a;
  float3  grad3 = permTexture2d.SampleLevel(permSampler2d, float2(perm3, Pi.z + 0.00390625), 0).rgb * 4.0 - 1.0;
  float t3 = 0.6 - dot(Pf3, Pf3);
  float n3;
  if(t3 < 0.0) n3 = 0.0;
  else {
    t3 *= t3;
    n3 = t3 * t3 * dot(grad3, Pf3);
  }

  // Sum up and scale the result to cover the range [-1,1]
  return 32.0 * (n0 + n1 + n2 + n3);
}

float snoise(float4 P) 
{

// The skewing and unskewing factors are hairy again for the 4D case
// This is (sqrt(5.0)-1.0)/4.0
//#define F4 0.309016994375
// This is (5.0-sqrt(5.0))/20.0
//#define G4 0.138196601125

  // Skew the (x,y,z,w) space to determine which cell of 24 simplices we're in
 	float s = (P.x + P.y + P.z + P.w) * 0.309016994375; // Factor for 4D skewing
  float4 Pi = floor(P + s);
  float t = (Pi.x + Pi.y + Pi.z + Pi.w) * 0.138196601125;
  float4 P0 = Pi - t; // Unskew the cell origin back to (x,y,z,w) space
  Pi = Pi * 0.00390625 + 0.001953125; // Integer part, scaled and offset for texture lookup

  float4 Pf0 = P - P0;  // The x,y distances from the cell origin

  // For the 4D case, the simplex is a 4D shape I won't even try to describe.
  // To find out which of the 24 possible simplices we're in, we need to
  // determine the magnitude ordering of x, y, z and w components of Pf0.
  // The method below is presented without explanation. It uses a small 1D
  // texture as a lookup table. The table is designed to work for both
  // 3D and 4D noise and contains 64 indices, of which only 24 are actually
  // used. An extension to 5D would require a larger texture here.
  float c1 = (Pf0.x > Pf0.y) ? 0.5078125 : 0.0078125; // 1/2 + 1/128
  float c2 = (Pf0.x > Pf0.z) ? 0.25 : 0.0;
  float c3 = (Pf0.y > Pf0.z) ? 0.125 : 0.0;
  float c4 = (Pf0.x > Pf0.w) ? 0.0625 : 0.0;
  float c5 = (Pf0.y > Pf0.w) ? 0.03125 : 0.0;
  float c6 = (Pf0.z > Pf0.w) ? 0.015625 : 0.0;
  float sindex = c1 + c2 + c3 + c4 + c5 + c6;
  float4 offsets = simplexGradTexture.SampleLevel(permGradSampler, sindex, 0).rgba;
  float4 o1 = step(0.625, offsets);
  float4 o2 = step(0.375, offsets);
  float4 o3 = step(0.125, offsets);

  // Noise contribution from simplex origin
  float perm0xy = permTexture2d.SampleLevel(permSampler2d, Pi.xy, 0).a;
  float perm0zw = permTexture2d.SampleLevel(permSampler2d, Pi.zw, 0).a;
  float4  grad0 = permTexture2d.SampleLevel(permSampler2d, float2(perm0xy, perm0zw), 0).rgba * 4.0 - 1.0;
  float t0 = 0.6 - dot(Pf0, Pf0);
  float n0;
  if (t0 < 0.0) n0 = 0.0;
  else {
    t0 *= t0;
    n0 = t0 * t0 * dot(grad0, Pf0);
  }

  // Noise contribution from second corner
  float4 Pf1 = Pf0 - o1 + 0.138196601125;
  o1 = o1 * 0.00390625;
  float perm1xy = permTexture2d.SampleLevel(permSampler2d, Pi.xy + o1.xy, 0).a;
  float perm1zw = permTexture2d.SampleLevel(permSampler2d, Pi.zw + o1.zw, 0).a;
  float4  grad1 = permTexture2d.SampleLevel(permSampler2d, float2(perm1xy, perm1zw), 0).rgba * 4.0 - 1.0;
  float t1 = 0.6 - dot(Pf1, Pf1);
  float n1;
  if (t1 < 0.0) n1 = 0.0;
  else {
    t1 *= t1;
    n1 = t1 * t1 * dot(grad1, Pf1);
  }
  
  // Noise contribution from third corner
  float4 Pf2 = Pf0 - o2 + 2.0 * 0.138196601125;
  o2 = o2 * 0.00390625;
  float perm2xy = permTexture2d.SampleLevel(permSampler2d, Pi.xy + o2.xy, 0).a;
  float perm2zw = permTexture2d.SampleLevel(permSampler2d, Pi.zw + o2.zw, 0).a;
  float4  grad2 = permTexture2d.SampleLevel(permSampler2d, float2(perm2xy, perm2zw), 0).rgba * 4.0 - 1.0;
  float t2 = 0.6 - dot(Pf2, Pf2);
  float n2;
  if (t2 < 0.0) n2 = 0.0;
  else {
    t2 *= t2;
    n2 = t2 * t2 * dot(grad2, Pf2);
  }
  
  // Noise contribution from fourth corner
  float4 Pf3 = Pf0 - o3 + 3.0 * 0.138196601125;
  o3 = o3 * 0.00390625;
  float perm3xy = permTexture2d.SampleLevel(permSampler2d, Pi.xy + o3.xy, 0).a;
  float perm3zw = permTexture2d.SampleLevel(permSampler2d, Pi.zw + o3.zw, 0).a;
  float4  grad3 = permTexture2d.SampleLevel(permSampler2d, float2(perm3xy, perm3zw), 0).rgba * 4.0 - 1.0;
  float t3 = 0.6 - dot(Pf3, Pf3);
  float n3;
  if (t3 < 0.0) n3 = 0.0;
  else {
    t3 *= t3;
    n3 = t3 * t3 * dot(grad3, Pf3);
  }
  
  // Noise contribution from last corner
  float f = 1.0-4.0*0.138196601125;
  float4 Pf4 = Pf0 - float4(f,f,f,f);
  float perm4xy = permTexture2d.SampleLevel(permSampler2d, Pi.xy + float2(0.00390625, 0.00390625), 0).a;
  float perm4zw = permTexture2d.SampleLevel(permSampler2d, Pi.zw + float2(0.00390625, 0.00390625), 0).a;
  float4  grad4 = permTexture2d.SampleLevel(permSampler2d, float2(perm4xy, perm4zw), 0).rgba * 4.0 - 1.0;
  float t4 = 0.6 - dot(Pf4, Pf4);
  float n4;
  if(t4 < 0.0) n4 = 0.0;
  else {
    t4 *= t4;
    n4 = t4 * t4 * dot(grad4, Pf4);
  }

  // Sum up and scale the result to cover the range [-1,1]
  return 27.0 * (n0 + n1 + n2 + n3 + n4);
}

// fractal sum

float fBm(float4 p, int octaves, float lacunarity = 2.0, float gain = 0.5)
{
	float freq = 1.0, amp = 0.5;
	float sum = 0;
	for(int i=0; i<octaves; i++) {
		sum += snoise(p*freq)*amp;
		freq *= lacunarity;
		amp *= gain;
	}
	return sum;
}

float turbulence(float4 p, int octaves, float lacunarity = 2.0, float gain = 0.5)
{
	float sum = 0;
	float freq = 1.0, amp = 1.0;
	[unroll]
	for(int i=0; i<octaves; i++) {
		sum += abs(snoise(p*freq))*amp;
		freq *= lacunarity;
		amp *= gain;
	}
	return sum;
}
/*
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
		float n = ridge(snoise(p*freq), offset);
		sum += n*amp*prev;
		prev = n;
		freq *= lacunarity;
		amp *= gain;
	}
	return sum;
}
*/
#endif