struct PsIn
{
	float4 pos : SV_POSITION;
    float4 color : COLOR;
};

float4x4 g_worldMatrix : register(b0);

PsIn vsmain(float3 pos : POSITION, float4 color : COLOR)
{
    PsIn psIn;
    psIn.pos = mul(g_worldMatrix, float4(pos, 1.0f));
    psIn.color = color;

    return psIn;
}

float4 psmain(PsIn input) : SV_TARGET
{
    return input.color;
}