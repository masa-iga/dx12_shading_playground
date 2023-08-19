/*!
 * @brief シンプルなモデル表示用のシェーダー
 */

// ポイントライト構造体を定義する
struct SPointLight
{
    float4 m_position;
    float4 m_color;
    float m_range;
};

// ポイントライトの数を表す定数を定義する
static const uint kNumPointLight = 1000;

// モデル用の定数バッファー
cbuffer ModelCb : register(b0)
{
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
};

// ポイントライトの定数バッファーにアクセスするための変数を定義する
cbuffer PointLightCb : register(b1)
{
    SPointLight pointLights[kNumPointLight];
}

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
    float3 worldPos : TEXCOORD1;    // ワールド座標
};

// モデルテクスチャ
Texture2D<float4> g_texture : register(t0);

// サンプラーステート
sampler g_sampler : register(s0);

/// <summary>
/// モデル用の頂点シェーダーのエントリーポイント
/// </summary>
SPSIn VSMain(SVSIn vsIn, uniform bool hasSkin)
{
    SPSIn psIn;

    psIn.pos = mul(mWorld, vsIn.pos);   // モデルの頂点をワールド座標系に変換

    psIn.worldPos = psIn.pos.xyz;

    psIn.pos = mul(mView, psIn.pos);    // ワールド座標系からカメラ座標系に変換
    psIn.pos = mul(mProj, psIn.pos);    // カメラ座標系からスクリーン座標系に変換
    psIn.normal = normalize(mul(mWorld, float4(vsIn.normal, 0.0f))).xyz;
    psIn.uv = vsIn.uv;

    return psIn;
}

/// <summary>
/// モデル用のピクセルシェーダーのエントリーポイント
/// </summary>
float4 PSMain(SPSIn psIn) : SV_Target0
{
    // アルベドカラーを出力
    float4 albedo = g_texture.Sample(g_sampler, psIn.uv);

    float3 lig = 0.0f;

    // ポイントライトから光によるLambert拡散反射を計算する
    for (int i = 0; i < kNumPointLight; ++i)
    {
        const float3 light = normalize(pointLights[i].m_position.xyz - psIn.worldPos);
        const float3 normal = psIn.normal;
        const float t = max(0.0f, dot(normal, light));

        const float distance = length(psIn.worldPos - pointLights[i].m_position.xyz);
        const float affect = max(0.0f, 1.0f - distance / pointLights[i].m_range);

        lig += affect * t * pointLights[i].m_color.xyz;
    }

    // 環境光を加算
    lig += 0.3f;
    albedo.xyz *= lig;
    return albedo;
}