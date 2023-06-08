/*!
 * @brief 影が落とされるモデル用のシェーダー
 */

// モデル用の定数バッファー
cbuffer ModelCb : register(b0)
{
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
};

// ライトビュープロジェクションクロップ行列の定数バッファーを定義
cbuffer ShadowParamCb : register(b1)
{
    float4x4 mLVPC[3];
};

// 頂点シェーダーへの入力
struct SVSIn
{
    float4 pos : POSITION;  // スクリーン空間でのピクセルの座標
    float3 normal : NORMAL; // 法線
    float2 uv : TEXCOORD0;  // uv座標
};

// ピクセルシェーダーへの入力
struct SPSIn
{
    float4 pos : SV_POSITION;   // スクリーン空間でのピクセルの座標
    float3 normal : NORMAL;     // 法線
    float2 uv : TEXCOORD0;      // uv座標

    // ライトビュースクリーン空間での座標を追加
    float4 posInLVP[3]: TEXCOORD1;
};

///////////////////////////////////////////////////
// グローバル変数
///////////////////////////////////////////////////

Texture2D<float4> g_albedo : register(t0); // アルベドマップ

// 近～中距離のシャドウマップにアクセスするための変数を定義
Texture2D<float4> g_shadowMap_0 : register(t10);
Texture2D<float4> g_shadowMap_1 : register(t11);
Texture2D<float4> g_shadowMap_2 : register(t12);

sampler g_sampler : register(s0); //  サンプラーステート

/// <summary>
/// 影が落とされる3Dモデル用の頂点シェーダー
/// </summary>
SPSIn VSMain(SVSIn vsIn)
{
    // 通常の座標変換
    SPSIn psIn;
    float4 worldPos = mul(mWorld, vsIn.pos);
    psIn.pos = mul(mView, worldPos);
    psIn.pos = mul(mProj, psIn.pos);
    psIn.uv = vsIn.uv;
    psIn.normal = mul(mWorld, float4(vsIn.normal, 0.0f)).xyz;

    // ライトビュースクリーン空間の座標を計算する
    psIn.posInLVP[0] = mul(mLVPC[0], worldPos);
    psIn.posInLVP[1] = mul(mLVPC[1], worldPos);
    psIn.posInLVP[2] = mul(mLVPC[2], worldPos);

    return psIn;
}

/// <summary>
/// 影が落とされる3Dモデル用のピクセルシェーダー
/// </summary>
float4 PSMain(SPSIn psIn) : SV_Target0
{
    float4 color = g_albedo.Sample(g_sampler, psIn.uv);

    // 3枚のシャドウマップを使って、シャドウレシーバーに影を落とす
    for (int cascadeIndex = 0; cascadeIndex < 3; cascadeIndex++)
    {
        const float zInLVP = psIn.posInLVP[cascadeIndex].z / psIn.posInLVP[cascadeIndex].w;

        if (zInLVP < 0.0f || 1.0f < zInLVP)
            continue;

        float2 shadowMapUV = psIn.posInLVP[cascadeIndex].xy / psIn.posInLVP[cascadeIndex].w;
        shadowMapUV *= float2(0.5f, -0.5f);
        shadowMapUV += 0.5f;

        if (shadowMapUV.x < 0.0f || 1.0f < shadowMapUV.x || shadowMapUV.y < 0.0f || 1.0f < shadowMapUV.y)
            continue;

        float2 shadowValue = 0;

        switch (cascadeIndex)
        {
            case 0:
                shadowValue = g_shadowMap_0.Sample(g_sampler, shadowMapUV).xy;
                break;
            case 1:
                shadowValue = g_shadowMap_1.Sample(g_sampler, shadowMapUV).xy;
                break;
            case 2:
                shadowValue = g_shadowMap_2.Sample(g_sampler, shadowMapUV).xy;
                break;
            default:
                break;
        }

        if (shadowValue.x < zInLVP)
        {
            color.xyz *= 0.5f;
            break;
        }
    }

    return color;
}
