#ifndef DISTFIELD
#define DISTFIELD
#include "shapes.hlsl"
#include "fireball.hlsl"


/*
1 = Circle-cube
2 = Cylinder
3 = Plane
4 = Pyrosphere
*/
float calcDist(float3 rayPos, out int objID, bool volumetric, out float gradient)
{
	objID = 0;
	
	float tmp = 0.0;
	float dist = 60.0;
	
	gradient = 0.5;
	
/*
	// Circle-cube
	tmp = max(Sphere(rayPos, float3(0,0,0), 1), -Cube(rayPos, float3(0.7,0.7,0.7)));
	if(tmp<dist)
	{
		dist = tmp;
		objID = 1;
	}	
	tmp = Cylinder(rayPos, float3(0.5, 0.5, 0.5));
	if(tmp<dist)
	{
		dist = tmp;
		objID=2;
	}
		*/
	tmp = Plane(rayPos, float4(float3(0, 1,0), 4));
	if(tmp<dist)
	{
		dist = tmp;
		objID = 3;
	}
	/*
	// Volumetric circle
	if(volumetric)
	{
		tmp = Sphere(rayPos, float3(0,1,0), 1.5);
		if(tmp<dist)
		{
			dist = tmp;
			objID = 4;
		}
	}
	*/
	//tmp = PyroSphere(rayPos, float3(0,sin(time),0), 2+6*sin(time/3.0), 2, gradient);
	//tmp = PyroSphere(rayPos, float3(0,4,0), 2, 2, gradient);
	//tmp = Sphere(rayPos, float3(0,-3,0), 3);
	tmp = pyroclasticSpheres(rayPos, gradient);
	if(tmp <dist)
	{
		dist = tmp;
		objID = 4;
	}
	return dist;
}

float3 Normal(float3 pos)
{
	float3 n = float3(0.0,0.0,0.0);
	float3 offset = float3(0.02,0.0,0.0);
	int objID;
	float gradient;
	
	
	n.x = calcDist(pos+offset.xyz, objID, true, gradient) - calcDist(pos-offset.xyz, objID, true, gradient);
	n.y = calcDist(pos+offset.yxz, objID, true, gradient) - calcDist(pos-offset.yxz, objID, true, gradient);
	n.z = calcDist(pos+offset.yzx, objID, true, gradient) - calcDist(pos-offset.yzx, objID, true, gradient);
	
	return normalize(n);
}


float AmbientOcclusion( float3 pos, float3 normal, float time)
{
	float total = 0.0f;
	float weight = 0.5f;
	int materialID;
	float gradient;
	
	[unroll]
	for(int i=0;i<5;++i)
	{
		float delta = (i+1)*(i+1)*0.01 * 16.0f;
		total += weight*(delta - calcDist(pos + normal*delta, materialID, true, gradient));
		weight *= 0.5;
	}
	return 1.0 -saturate(total);
}

#endif