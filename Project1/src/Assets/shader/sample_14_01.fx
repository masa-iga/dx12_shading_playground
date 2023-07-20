///////////////////////////////////////////
// 構造体
///////////////////////////////////////////
// 頂点シェーダーへの入力
struct SVSIn
{
    float4 pos      : POSITION;
    float2 uv       : TEXCOORD0;
};

// ピクセルシェーダーへの入力
struct SPSIn
{
    float4 pos          : SV_POSITION;
    float2 uv           : TEXCOORD0;
    float4 posInProj    : TEXCOORD1;
};

///////////////////////////////////////////
// 定数バッファー
///////////////////////////////////////////
// モデル用の定数バッファー
cbuffer ModelCb : register(b0)
{
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
};

///////////////////////////////////////////
// シェーダーリソース
///////////////////////////////////////////
// モデルテクスチャ
Texture2D<float4> g_texture : register(t0);

// 深度テクスチャにアクセスするための変数を追加
Texture2D<float4> g_depthTexture : register(t10);

///////////////////////////////////////////
// サンプラーステート
///////////////////////////////////////////
sampler g_sampler : register(s0);

/// <summary>
/// モデル用の頂点シェーダーのエントリーポイント
/// </summary>
SPSIn VSMain(SVSIn vsIn, uniform bool hasSkin)
{
    SPSIn psIn;

    psIn.pos = mul(mWorld, vsIn.pos);   // モデルの頂点をワールド座標系に変換
    psIn.pos = mul(mView, psIn.pos);    // ワールド座標系からカメラ座標系に変換
    psIn.pos = mul(mProj, psIn.pos);    // カメラ座標系からスクリーン座標系に変換
    psIn.uv = vsIn.uv;

    // 頂点の正規化スクリーン座標系の座標をピクセルシェーダーに渡す
    psIn.posInProj = psIn.pos;
    psIn.posInProj.xy /= psIn.pos.w;

    return psIn;
}

/// <summary>
/// モデル用のピクセルシェーダーのエントリーポイント
/// </summary>
float4 PSMain(SPSIn psIn) : SV_Target0
{
    // 近傍8テクセルの深度値を計算して、エッジを抽出する
    const float2 depthUv = psIn.posInProj.xy * float2(0.5, -0.5) + 0.5f;

    if (depthUv.x < 0.0f || 1.0f < depthUv.x || depthUv.y < 0.0f || 1.0f < depthUv.y)
        return g_texture.Sample(g_sampler, psIn.uv);

    {
        uint mipLevel = 0;
        uint width = 0;
        uint height = 0;
        uint level = 0;
        g_depthTexture.GetDimensions(mipLevel, width, height, level);

        const float dx = 1.0f / width * 3.0f;
        const float dy = 1.0f / height * 3.0f;
        const float2 uvOffsets[8] =
        {
            float2(-dx, -dy),
            float2(-dx, 0.0f),
            float2(-dx, dy),
            float2(0.0f, -dy),
            float2(0.0f, dy),
            float2(dx, -dy),
            float2(dx, 0.0f),
            float2(dx, dy),
        };

        float depth2 = 0.0f;

        for (int i = 0; i < 8; ++i)
            depth2 += g_depthTexture.Sample(g_sampler, depthUv + uvOffsets[i]).x;

        depth2 /= 8.0f;

        const float kThreshold = 0.00005f;
        const float4 kEdgeColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
        const float z = psIn.pos.z;

        if (abs(z - depth2) > kThreshold)
            return kEdgeColor;
    }

    float4 col = g_texture.Sample(g_sampler, psIn.uv);
    return col;
}
