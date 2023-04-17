/*!
 * @brief   ディズニーベースの物理ベースシェーダー
 */

///////////////////////////////////////////////////
// 構造体
///////////////////////////////////////////////////

// モデル用の定数バッファー
cbuffer ModelCb : register(b0)
{
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
};

// 頂点シェーダーへの入力
struct SVSIn
{
    float4 pos : POSITION;      // モデルの頂点座標
    float3 normal : NORMAL;     // 法線
    float3 tangent : TANGENT;
    float3 biNormal : BINORMAL;
    float2 uv : TEXCOORD0;      // UV座標
};

// ピクセルシェーダーへの入力
struct SPSIn
{
    float4 pos : SV_POSITION;       // スクリーン空間でのピクセルの座標
    float3 normal : NORMAL;         // 法線
    float3 tangent : TANGENT;       // 接ベクトル
    float3 biNormal : BINORMAL;     // 従法線ベクトル
    float2 uv : TEXCOORD0;          // UV座標
    float3 worldPos : TEXCOORD1;    // ワールド空間でのピクセルの座標
    float depthInView : TEXCOORD2; // カメラ空間でのZ値を記録する変数
};

// ピクセルシェーダーからの出力構造体を定義する。
struct SPSOut
{
    float4 color : SV_Target0;
    float depth : SV_Target1;
};

///////////////////////////////////////////////////
// グローバル変数
///////////////////////////////////////////////////
// サンプラーステート
sampler g_sampler : register(s0);

//物理ベースの処理をインクルード
#include "preset/pbr.h"

/// <summary>
/// 頂点シェーダー
/// <summary>
SPSIn VSMain(SVSIn vsIn)
{
    SPSIn psIn;
    psIn.pos = mul(mWorld, vsIn.pos);
    psIn.worldPos = psIn.pos.xyz;
    psIn.pos = mul(mView, psIn.pos);

    // 頂点シェーダーでカメラ空間でのZ値を設定する
    psIn.depthInView = psIn.pos.z;

    psIn.pos = mul(mProj, psIn.pos);
    psIn.normal = normalize(mul(mWorld, float4(vsIn.normal, 0.0f))).xyz;
    psIn.tangent = normalize(mul(mWorld, float4(vsIn.tangent, 0.0f))).xyz;
    psIn.biNormal = normalize(mul(mWorld, float4(vsIn.biNormal, 0.0f))).xyz;
    psIn.uv = vsIn.uv;

    return psIn;
}

/// <summary>
/// ピクセルシェーダー
/// </summary>
SPSOut PSMain(SPSIn psIn)
{
    // ピクセルシェーダーからカラーとZ値を出力する。
    SPSOut psOut;
    psOut.color = CalcPBR(psIn);
    psOut.depth = psIn.depthInView;
    return psOut;
}
