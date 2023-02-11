struct PsIn
{
	float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

float4x4 g_worldMatrix : register(b0);
Texture2D<float4> g_tex : register(t0);
sampler g_sampler : register(s0);

PsIn vsmain(float3 pos : POSITION, float2 uv : TEXCOORD)
{
    PsIn psIn;
    psIn.pos = mul(g_worldMatrix, float4(pos, 1.0f));
    psIn.uv = uv;
	return psIn;
}

float4 psmain(PsIn input) : SV_TARGET
{
    return g_tex.Sample(g_sampler, input.uv);
}