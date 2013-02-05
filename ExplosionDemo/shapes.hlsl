#ifndef SHAPES
#define SHAPES
#include "decl.hlsl"
#include "inoise_old.hlsl"
//#include "snoise.hlsl"

static float E = 0.001;
static float3 sunPos = float3(-120, 40, 30);

// primitive shapes

float Sphere(float3 rayPos, float3 pos, float rad)
{
	return length(pos - rayPos)-rad;
}

float Cube(float3 rayPos, float3 dim)
{
	float3 d = abs(rayPos) - dim;
	return min(max(d.x,max(d.y,d.z)),0.0) + length(max(d,0.0));
		 
	//return length(max(abs(rayPos)-dim,0.0));
}

float Cylinder(float3 rayPos, float3 dim)
{
	return length(rayPos.xz-dim.xy)-dim.z;
}
// w from normal is the height of plane
float Plane(float3 pos, float4 normal)
{
	return dot(pos, normal.xyz) + normal.w;
}

float PyroSphere(float3 rayPos, float3 spherePos, float radius, float dBound, out float gradient)
{
	float l = length(rayPos-spherePos);
	float d = l - radius;
	gradient = 1;
	//return l-radius;
	if(d<0)
		return d;
	if(d <= dBound+EPSILON*2)
	{
		float3 n = spherePos-rayPos;//+time*float3(0,1,0);
		float r = turbulence(n +time*float3(0,2,0), 4);
		gradient = r;
		return d-r;
	}
	return d-dBound+0.5;

} 

#endif