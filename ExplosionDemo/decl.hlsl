#ifndef DECL_HLSL
#define DECL_HLSL

sampler permSampler2d : register(s0);
sampler permGradSampler : register(s1);
sampler fireGradSampler : register(s2);
sampler groundTextureSampler : register(s3);

Texture2D permTexture2d : register(t0);
Texture1D permGradTexture : register (t1);
Texture1D fireGradTexture : register(t2);
Texture1D simplexGradTexture : register(t3);

cbuffer viewportData : register(c0)
{
	float4 pos;
	float4 xDir;
	float4 yDir;
	float4 zDir;
	float fov;
	float aspectRatio;
	float time;
}

cbuffer shaderData : register(c1)
{
	float lacunarity;
	float gain;
	float gradOffset;
	float gradScale;
}

static float EPSILON = 0.02;
static float STEPLENGTH = 0.1;
static uint STEPAMOUNT = 32;
static uint STEPDIVIDER = 32;


#endif