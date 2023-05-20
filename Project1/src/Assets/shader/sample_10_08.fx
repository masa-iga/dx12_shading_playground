/*!
 *@brief �Z�p�`�u���[
 */

// �u���[��������e�N�X�`���̕�
static const float BLUR_TEX_W = 1280.0f;

// �u���[��������e�N�X�`���̍���
static const float BLUR_TEX_H = 720.0f;

// �u���[���a�B���̐��l��傫������ƘZ�p�`�{�P���傫���Ȃ�
static const float BLUR_RADIUS = 8.0f;

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

cbuffer cb : register(b0)
{
    float4x4 mvp;       // MVP�s��
    float4 mulColor;    // ��Z�J���[
};

// �����A�Ίp���u���[�̏o�͍\���̂��`
struct PSOutput
{
    float4 color0 : SV_Target0; // render vertical blur
    float4 color1 : SV_Target1; // render diagonal blur
};

/*!
 *@brief ���_�V�F�[�_�[
 */
PSInput VSMain(VSInput In)
{
    PSInput psIn;
    psIn.pos = mul(mvp, In.pos);
    psIn.uv = In.uv;
    return psIn;
}

Texture2D<float4> srcTexture : register(t0); // �u���[��������O�̃I���W�i���e�N�X�`��

// �T���v���[�X�e�[�g
sampler g_sampler : register(s0);

/*!
 *@brief �����A�΂߃u���[�̃s�N�Z���V�F�[�_�[
 */
PSOutput PSVerticalDiagonalBlur(PSInput pIn)
{
    PSOutput psOut;
    psOut.color0 = 0;
    psOut.color1 = 0;

    // �u���[��������e�N�X�`���̃J���[���擾
    float4 srcColor = srcTexture.Sample(
        g_sampler, pIn.uv );

    // �u���[���a�iBLUR_RADIUS�j����u���[�X�e�b�v�̒��������߂�
    const float blurStepLen = BLUR_RADIUS / 4.0f;

    {
        // ����������UV�I�t�Z�b�g���v�Z
        float2 uvOffset = float2(0.0f, 1.0f / BLUR_TEX_H);
        uvOffset *= blurStepLen;

        // ���������ɃJ���[���T���v�����O���ĕ��ς���
        psOut.color0 += srcTexture.Sample(g_sampler, pIn.uv + uvOffset * 1);
        psOut.color0 += srcTexture.Sample(g_sampler, pIn.uv + uvOffset * 2);
        psOut.color0 += srcTexture.Sample(g_sampler, pIn.uv + uvOffset * 3);
        psOut.color0 += srcTexture.Sample(g_sampler, pIn.uv + uvOffset * 4);
        psOut.color0 /= 4.0f;
    }

    {
        // �Ίp��������UV�I�t�Z�b�g���v�Z
        float2 uvOffset = float2(0.86602f / BLUR_TEX_W, -0.5f / BLUR_TEX_H);
        uvOffset *= blurStepLen;

        // step-11 �Ίp�������ɃJ���[���T���v�����O���ĕ��ω�����
        psOut.color1 += srcColor;
        psOut.color1 += srcTexture.Sample(g_sampler, pIn.uv + uvOffset * 1);
        psOut.color1 += srcTexture.Sample(g_sampler, pIn.uv + uvOffset * 2);
        psOut.color1 += srcTexture.Sample(g_sampler, pIn.uv + uvOffset * 3);
        psOut.color1 += srcTexture.Sample(g_sampler, pIn.uv + uvOffset * 4);
        psOut.color1 /= 5.0f;

        psOut.color1 += psOut.color0;
        psOut.color1 /= 2.0f;
    }

    return psOut;
}

Texture2D<float4> blurTexture_0 : register(t0); // �u���[�e�N�X�`��_0�B1�p�X�ڂō쐬���ꂽ�e�N�X�`��
Texture2D<float4> blurTexture_1 : register(t1); // �u���[�e�N�X�`��_1�B1�p�X�ڂō쐬���ꂽ�e�N�X�`��

/*!
 *@brief �Z�p�`�쐬�u���[
 */
float4 PSRhomboidBlur(PSInput pIn) : SV_Target0
{
    // �u���[�X�e�b�v�̒��������߂�
    float blurStepLen = BLUR_RADIUS / 4.0f;

    float4 color = 0.0f;

    // ���΂߉������ւ�UV�I�t�Z�b�g���v�Z����
    {
        float2 uvOffset = float2(0.86602f / BLUR_TEX_W, -0.5f / BLUR_TEX_H);
        uvOffset *= blurStepLen;

        // ���΂߉������ɃJ���[���T���v�����O����
        color += blurTexture_0.Sample(g_sampler, pIn.uv + uvOffset * 1);
        color += blurTexture_0.Sample(g_sampler, pIn.uv + uvOffset * 2);
        color += blurTexture_0.Sample(g_sampler, pIn.uv + uvOffset * 3);
        color += blurTexture_0.Sample(g_sampler, pIn.uv + uvOffset * 4);
    }

    {
        // �E�΂߉������ւ�UV�I�t�Z�b�g���v�Z����
        float2 uvOffset = float2(-0.86602f / BLUR_TEX_W, -0.5f / BLUR_TEX_H);
        uvOffset *= blurStepLen;

        // �E�΂߉������ɃJ���[���T���v�����O����
        color += blurTexture_1.Sample(g_sampler, pIn.uv);
        color += blurTexture_1.Sample(g_sampler, pIn.uv + uvOffset * 1);
        color += blurTexture_1.Sample(g_sampler, pIn.uv + uvOffset * 2);
        color += blurTexture_1.Sample(g_sampler, pIn.uv + uvOffset * 3);
        color += blurTexture_1.Sample(g_sampler, pIn.uv + uvOffset * 4);
    }

    // step-16 ���ω�
    color /= 9.0f;

    return color;
}