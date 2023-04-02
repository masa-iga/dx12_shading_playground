cbuffer cb : register(b0)
{
    float4x4 mvp;       // MVP行列
    float4 mulColor;    // 乗算カラー
};

struct VSInput
{
    float4 pos : POSITION;
    float2 uv  : TEXCOORD0;
};

struct PSInput
{
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};

static const float kScreenWidth = 1920.0f;
static const float kScreenHeight = 1080.0f;

Texture2D<float4> sceneTexture : register(t0); // シーンテクスチャ
sampler Sampler : register(s0);

PSInput VSMain(VSInput In)
{
    PSInput psIn;
    psIn.pos = mul(mvp, In.pos);
    psIn.uv = In.uv;
    return psIn;
}

float4 PSMain(PSInput In) : SV_Target0
{
    float4 color = sceneTexture.Sample(Sampler, In.uv);

    // 基準テクセル+近傍8テクセルの平均を計算する
    const float offsetU = 1.5f / kScreenWidth;
    const float offsetV = 1.5f / kScreenHeight;

    color += sceneTexture.Sample(Sampler, In.uv + float2(offsetU, 0.0f));
    color += sceneTexture.Sample(Sampler, In.uv + float2(-offsetU, 0.0f));
    color += sceneTexture.Sample(Sampler, In.uv + float2(0.0f, offsetV));
    color += sceneTexture.Sample(Sampler, In.uv + float2(0.0f, -offsetV));
    color += sceneTexture.Sample(Sampler, In.uv + float2(offsetU, offsetV));
    color += sceneTexture.Sample(Sampler, In.uv + float2(offsetU, -offsetV));
    color += sceneTexture.Sample(Sampler, In.uv + float2(-offsetU, offsetV));
    color += sceneTexture.Sample(Sampler, In.uv + float2(-offsetU, -offsetV));

    color /= 9.0f;

    return color;
}
