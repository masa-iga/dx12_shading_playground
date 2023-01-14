/*!
 * @brief シンプルなモデル表示用のシェーダー
 */
///////////////////////////////////////////
// 構造体
///////////////////////////////////////////
// 頂点シェーダーへの入力
struct SVSIn
{
    float4 pos      : POSITION;
    float3 normal   : NORMAL;
    float2 uv       : TEXCOORD0;
};

// ピクセルシェーダーへの入力
struct SPSIn
{
    float4 pos      : SV_POSITION;
    float3 normal   : NORMAL;
    float2 uv       : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
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

cbuffer DirectionLightCb : register(b1)
{
    float3 ligDirection;
    float3 ligColor;
    float3 eyePos;
};

///////////////////////////////////////////
// シェーダーリソース
///////////////////////////////////////////
// モデルテクスチャ
Texture2D<float4> g_texture : register(t0);

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
    psIn.worldPos = psIn.pos;
    psIn.pos = mul(mView, psIn.pos);    // ワールド座標系からカメラ座標系に変換
    psIn.pos = mul(mProj, psIn.pos);    // カメラ座標系からスクリーン座標系に変換
    psIn.normal = mul(mWorld, vsIn.normal);
    psIn.uv = vsIn.uv;

    return psIn;
}

/// <summary>
/// モデル用のピクセルシェーダーのエントリーポイント
/// </summary>
float3 computeDiffuseLight(float3 normal)
{
    float t = dot(normal, ligDirection);
    t *= -1.0f;
    t = clamp(t, 0.0f, 1.0f);

    return ligColor * t;
}

float3 computeSpecularLight(float3 normal, float3 worldPos)
{
    const float3 refVec = reflect(ligDirection, normal);
    float3 toEye = eyePos - worldPos;
    toEye = normalize(toEye);

    float t = dot(refVec, toEye);
    t = clamp(t, 0.0f, 1.0f);
    t = pow(t, 5.0f);

    return ligColor * t;
}

float4 PSMain(SPSIn psIn) : SV_Target0
{
    const float3 diffuseLig = computeDiffuseLight(psIn.normal);
    const float3 specularLig = computeSpecularLight(psIn.normal, psIn.worldPos);
    const float3 lig = diffuseLig + specularLig;

    float4 finalColor = g_texture.Sample(g_sampler, psIn.uv);
    finalColor.xyz *= lig;

    return finalColor;
}
