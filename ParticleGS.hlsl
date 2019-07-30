#include "Particle.hlsli"

static const uint vnum = 3;

[maxvertexcount(vnum)]
void main(
	triangle PS_INPUT input[vnum],
	inout TriangleStream<PS_INPUT> output
)
{
	for (uint i = 0; i < vnum; i++)
	{
		PS_INPUT element;
		element.Pos = input[i].Pos;
		element.Pos = mul(element.Pos, matWorld);
		element.Pos = mul(element.Pos, matView);
		element.Pos = mul(element.Pos, matProj);
		element.Tex = input[i].Tex;
		output.Append(element);
	}
}