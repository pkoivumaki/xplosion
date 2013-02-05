#ifndef FIREBALL
#define FIREBALL
#include "shapes.hlsl"
#include "decl.hlsl"


float pyroclasticSpheres(float3 rayPos, out float gradient)
{
	float3 pos = float3(0,-4,0);

	float3 sphere1pos = float3(0,0,0)+pos;
	float3 sphere2pos = float3(2,1,1)+pos;
	float3 sphere3pos = float3(-1, 1.5, 0.2)+pos;
	
	//float3 sphere4pos
	float radius = 1.5+sin(time/2)*4;
	float dBound = 2.0;
	
	float l = min(min(length(rayPos-sphere1pos), length(rayPos -sphere2pos)), length(rayPos - sphere3pos));
	float d = l - radius;
	gradient = 1;
	//return l-radius;
	if(d<0)
		return d;
	if(d <= dBound+EPSILON*2)
	{
		float3 n = sphere1pos-rayPos;//+time*float3(0,1,0);
		float r = abs(turbulence(n +time*float3(0,2,0), 4, lacunarity, gain));
		gradient = abs((r*gradScale)+gradOffset);
		return d-r;
	}
	return d-dBound+0.5;

}

#endif