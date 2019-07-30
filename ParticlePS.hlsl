#include "Particle.hlsli"

Texture2D tex : register(t0);
Texture2D tex2 : register(t1);
SamplerState samLinear : register(s0);

float4 main(PS_INPUT input) : SV_TARGET
{
	float4 diff = tex.Sample(samLinear, input.Tex);
	float4 diff2 = tex2.Sample(samLinear, input.Tex);
	float t = (sin(time) + 1) * .5f;
	return float4(t, 1, t, diff.a);
}