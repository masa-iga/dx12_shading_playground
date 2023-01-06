struct PsIn
{
	float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

float4x4 g_worldMatrix : register(b0);
Texture2D<float4> g_tex : register(t0);
sampler g_sampler : register(s0);

PsIn vsmain(float4 pos : POSITION, float2 uv : TEXCOORD)
{
    PsIn psIn;
    psIn.pos = mul(g_worldMatrix, pos);
    psIn.uv = float2(0, 0);
	return psIn;
}

float4 psmain(PsIn input) : SV_TARGET
{
    return g_tex.Sample(g_sampler, input.uv);
}