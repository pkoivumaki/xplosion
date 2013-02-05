#include "distfield.hlsl"
#include "lighting.hlsl"
#include "fog.hlsl"

#include "decl.hlsl"


struct VS_INPUT
{
    float3 pos : POSITION;
    float2 tex : TEXCOORD;
};

struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD;
};

PS_INPUT vertexShader(VS_INPUT input)
{
    PS_INPUT output;

    output.pos = float4(input.pos.xyz, 1);
    output.tex = input.tex;

    return output;
}


float4 rayMarcher(PS_INPUT input) : SV_TARGET
{	

	float3 rayDir = float3(0,0,0);

	float fv = fov;
	
	float3 xInc;
	float3 yInc;
	
	yInc = 2.0 * tan(fv/2)*xDir.xyz*aspectRatio;
	xInc = -2.0 * tan(fv/2)*yDir.xyz;
	
	rayDir = zDir.xyz + 0.5*(input.tex.y*2.0 -1.0)*yInc + 0.5*(input.tex.x*2.0 -1.0)*xInc;
	
	float3 rayPos = pos.xyz + rayDir;
	rayDir = normalize(rayDir);
	
	//float3 color = float3(176, 215, 225) /255.0;
	float3 color = float3(0,0,0);
	
	float distMarched = 0;
	int iterations = 0;
	bool stepping = false;
	bool beenInBall = false;
	uint stepsInVolume = 0;
	int objId;
	
	float distance = 0;
	
	
	// For Pyrosphere coloring
	float gradient = 0.0;
	float tmpGrad = 0.0;
	float3 ballColor = float3(0,0,0);
	
	while(distMarched < 500 && ++iterations < 150)
	{
		objId=-1;
		
		distance = calcDist(rayPos, objId, true, gradient);
		
		if(stepping && distance > EPSILON)
			stepping = false;
		if(distance < EPSILON)
		{
			//if(objId != 4)
				color = calcLight(rayPos, pos.xyz, objId, gradient);
			break;
		/*
			if(objId == 5)
			{
				if(beenInBall == false)
				{
					ballColor = calcLight(rayPos, pos.xyz, objId);
					gradient = tmpGrad;
					beenInBall = true;
				}
				stepping = true;
				stepsInVolume++;
				int tmpInt;
				float tmpFloat;
				float d = calcDist(rayPos, tmpInt, false, tmpFloat);
				if(d<EPSILON)
				{
					objId = tmpInt;
				}
			}
		
			if(objId != 5)
			{
				if(objId != 4)
					color = calcLight(rayPos, pos.xyz, objId);
				break;
			}*/
		}
		/*
		if(stepping)
		{
			if(stepsInVolume>STEPAMOUNT)
				break;
			rayPos += rayDir*STEPLENGTH;
			distMarched += STEPLENGTH;
			continue;
		}
		*/
		rayPos += rayDir*distance;
		distMarched += distance;
		
	}

	if(objId!=4)
		color = ApplyFog(color, rayPos-pos.xyz, normalize(sunPos));

	
	float3 gradColor = float3(0,0,0);
/*	if(objId == 4)
	{
		color = fireGradTexture.SampleLevel(fireGradSampler, gradient, 0).xyz;
		//color = Normal(rayPos);
	}
*/	
	
	//color = lerp(color, gradColor, stepsInVolume/STEPDIVIDER);
	//color = lerp(color, gradColor, stepsInVolume/200.0);
	
	//color = abs(fBm(float3(input.tex.xy, time), 4));
	//float3 color = fBm(float3(input.tex, time), 6);
	
    return float4(color.xyz, 1.0);
}














