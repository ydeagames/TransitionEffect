#include "Particle.hlsli"

Texture2D _MainTex : register(t0);
Texture2D _MaskTex : register(t1);
SamplerState _SamLinear : register(s0);

float4 main(PS_INPUT input) : SV_TARGET
{
	float4 mainTex = _MainTex.Sample(_SamLinear, input.Tex);
	float4 maskTex = _MaskTex.Sample(_SamLinear, input.Tex);
	float grayscale = dot(maskTex.rgb, float3(0.2126, 0.7152, 0.0722));
	float mask = grayscale - (-1 + range);
	clip(mask - 0.9999f);
	return mainTex;
}