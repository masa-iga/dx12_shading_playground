struct PsIn
{
	float4 pos : SV_POSITION;
    float4 color : COLOR;
};

PsIn vsmain(float3 pos : POSITION, float4 color : COLOR)
{
    PsIn psIn;
    psIn.pos = float4(pos, 1.0f);
    psIn.color = color;

    return psIn;
}

float4 psmain(PsIn input) : SV_TARGET
{
    return input.color;
}