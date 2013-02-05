#ifndef LIGHTING
#define LIGHTING
#include "decl.hlsl"

float calcShadow(float3 pos, float3 lightPos, float time)
{
	float3 rayDir = normalize(lightPos - pos);
	float3 rayPos = pos + rayDir*0.2;
	float dist = 0;
	float totalDist = 0.5;
	float shadow = 1;
	int i;
	float gradient;
	while(dist < 20)
	{
		dist = calcDist(rayPos, i, false, gradient);
		
		if(dist < 0.01*0.5)
			return 0;
		shadow = min(shadow, 16*dist/totalDist);
		rayPos += rayDir*dist;
		totalDist += dist;
	}
	return shadow;
}

float3 calcLight(float3 rayPos, float3 camPos, int objId, float gradient)
{
	float3 N = Normal(rayPos);
	float3 L = normalize(sunPos - rayPos);
	float3 V = normalize(camPos-rayPos);
	//float3 H = normalize((sunPos+camPos)/2.0);
	float3 H = normalize(V+L);//normalize((sunPos+pos.xyz));
	
	float3 color = float3(0.1, 0.3, 0.0);
	float specularTerm = 0.3;
	int specularPow = 32;
	float diffuseTerm = 0.3;
	float3 suncolor = float3(0.996f, 0.988f, 0.843f );

	if(objId == 3)
	{
		color = float3(0.02,0.01,0.01);
		specularPow = 8;
		specularTerm = 0.1;
		diffuseTerm = 0.01;
	}
	if(objId == 1)
	{
		color = float3(0.2,0.2,0.0);
	}
	if(objId == 2)
	{
		color = float3(0.1,0.1,0.1);
	}
	if(objId == 4)
	{
		specularTerm = 0.0;
		diffuseTerm = 0.0;
		color = fireGradTexture.SampleLevel(fireGradSampler, gradient, 0).xyz;
	}
	

	
	float diffuse = saturate(max(dot(N,L), 0))*diffuseTerm;
	float specular = pow(saturate(dot(N,H)), specularPow)*specularTerm;
	float ao = AmbientOcclusion(rayPos, N, time);

	

	
	color = lerp(color, suncolor, diffuse);
	color = lerp(float3(0,0,0), color, ao);
	
	float shadow = 1;
	if(objId!=4)
		shadow = calcShadow(rayPos, sunPos, time);
	//float shadow2 = calcShadow(rayPos, sunPos+(float3(5,0,0)), time);
	//shadow += shadow2;
	//shadow /= 2;
	color = lerp(color/5, color, shadow);
	
	color += suncolor*specular*(shadow);
	

	return color;
}




#endif
