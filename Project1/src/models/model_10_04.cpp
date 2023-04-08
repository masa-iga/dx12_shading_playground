#include "model_10_04.h"
#include <filesystem>
#include <memory>
#include "imodel.h"
#include "MiniEngine.h"
#include "model_util.h"
#include "../config.h"
#include "../debug_win.h"
#include "../imgui_if.h"
#include "../miniEngine_if.h"

class ModelFactory_10_04 : public IModelFactory
{
public:
	~ModelFactory_10_04() { }
	std::unique_ptr<IModels> create();
};

class Models_10_04 : public IModels
{
public:
	Models_10_04() { m_models.resize(static_cast<size_t>(ModelType::kSize)); }
	~Models_10_04() { }
	void createModel();
	void resetCamera();
	void handleInput();
	void draw(RenderContext& renderContext);
	void debugRenderParams();

private:
	enum class ModelType {
		kBg,
		kPlayer,
		kSize,
	};

	static constexpr size_t kNumWeights = 8;
	static constexpr float kSigma = 8.0f;
	const std::string kTkmBgFile = "Sample_10_04/Sample_10_04/Assets/modelData/bg/bg.tkm";
	const std::string kTkmSampleFile = "Sample_10_04/Sample_10_04/Assets/modelData/sample.tkm";
	const std::string kFx2dFile = "Sample_10_04/Sample_10_04/Assets/shader/sample2D.fx";
	const std::string kFx3dFile = "Sample_10_04/Sample_10_04/Assets/shader/sample3D.fx";
	const std::string kFxPostEffectFile = "Assets/shader/sample_10_04_postEffect.fx";
	std::string getTkmBgFilePath() { return ModelUtil::getPathFromAssetDir(kTkmBgFile); }
	std::string getTkmSampleFilePath() { return ModelUtil::getPathFromAssetDir(kTkmSampleFile); }
	std::string getFx2dFilePath() { return ModelUtil::getPathFromAssetDir(kFx2dFile); }
	std::string getFx3dFilePath() { return ModelUtil::getPathFromAssetDir(kFx3dFile); }
	std::string getFxPostEffectFilePath() { return kFxPostEffectFile; }
	void CalcWeightsTableFromGaussian(float* weightsTbl, int sizeOfWeightsTbl, float sigma);

	RenderTarget m_mainRenderTarget;
	RenderTarget m_xBlurRenderTarget;
	RenderTarget m_yBlurRenderTarget;
	Vector3 m_plPos;
	std::unique_ptr<Sprite> m_xBlurSprite = nullptr;
	std::unique_ptr<Sprite> m_yBlurSprite = nullptr;
	std::unique_ptr<Sprite> m_copyToFbSprite = nullptr;

	enum class Blur {
		kNone,
		kBlur,
		kGaussian,
		kSize,
	};

	struct ConstantBuffer {
		std::array<float, kNumWeights> m_weights;
		Blur blur = Blur::kNone;
	};

	ConstantBuffer m_cb;
};

std::unique_ptr<IModels> ModelFactory_10_04::create()
{
	std::unique_ptr<Models_10_04> m(new Models_10_04);
	{
		m->createModel();
	}
	return std::move(m);
}

void Models_10_04::resetCamera()
{
	;
}

void Models_10_04::createModel()
{
	{
		auto bRet = m_mainRenderTarget.Create(
			Config::kRenderTargetWidth,
			Config::kRenderTargetHeight,
			1,
			1,
			DXGI_FORMAT_R8G8B8A8_UNORM,
			DXGI_FORMAT_D32_FLOAT
		);
		Dbg::assert_(bRet);
	}
	{
		auto bRet = m_xBlurRenderTarget.Create(
			m_mainRenderTarget.GetWidth() / 2,
			m_mainRenderTarget.GetHeight(),
			1,
			1,
			m_mainRenderTarget.GetColorBufferFormat(),
			DXGI_FORMAT_D32_FLOAT
		);
		Dbg::assert_(bRet);
	}
	{
		auto bRet = m_yBlurRenderTarget.Create(
			m_xBlurRenderTarget.GetWidth(),
			m_xBlurRenderTarget.GetHeight() / 2,
			1,
			1,
			m_mainRenderTarget.GetColorBufferFormat(),
			DXGI_FORMAT_D32_FLOAT
		);
		Dbg::assert_(bRet);
	}

	const std::string tkmBgFilePath = getTkmBgFilePath();
	const std::string tkmSampleFilePath = getTkmSampleFilePath();
	const std::string fx2dFilePath = getFx2dFilePath();
	const std::string fx3dFilePath = getFx3dFilePath();
	const std::string fxPostEffectFilePath = getFxPostEffectFilePath();
	Dbg::assert_(std::filesystem::exists(tkmBgFilePath));
	Dbg::assert_(std::filesystem::exists(tkmSampleFilePath));
	Dbg::assert_(std::filesystem::exists(fx2dFilePath));
	Dbg::assert_(std::filesystem::exists(fx3dFilePath));
	Dbg::assert_(std::filesystem::exists(fxPostEffectFilePath));

	{
		CalcWeightsTableFromGaussian(m_cb.m_weights.data(), kNumWeights, kSigma);
	}

	{
		SpriteInitData data;
		data.m_fxFilePath = fxPostEffectFilePath.c_str();
		data.m_vsEntryPointFunc = "VSXBlur";
		data.m_psEntryPoinFunc = "PSBlur";
		data.m_width = m_xBlurRenderTarget.GetWidth();
		data.m_height = m_xBlurRenderTarget.GetHeight();
		data.m_textures.at(0) = &m_mainRenderTarget.GetRenderTargetTexture();
		data.m_expandConstantBuffer = &m_cb;
		data.m_expandConstantBufferSize = sizeof(m_cb);

		std::unique_ptr<Sprite> sprite = std::make_unique<Sprite>();
		sprite->Init(data);
		m_xBlurSprite = std::move(sprite);
	}
	{
		SpriteInitData data;
		data.m_fxFilePath = fxPostEffectFilePath.c_str();
		data.m_vsEntryPointFunc = "VSYBlur";
		data.m_psEntryPoinFunc = "PSBlur";
		data.m_width = m_yBlurRenderTarget.GetWidth();
		data.m_height = m_yBlurRenderTarget.GetHeight();
		data.m_textures.at(0) = &m_xBlurRenderTarget.GetRenderTargetTexture();
		data.m_expandConstantBuffer = &m_cb;
		data.m_expandConstantBufferSize = sizeof(m_cb);

		std::unique_ptr<Sprite> sprite = std::make_unique<Sprite>();
		sprite->Init(data);
		m_yBlurSprite = std::move(sprite);
	}
	{
		SpriteInitData data;
		data.m_fxFilePath = fx2dFilePath.c_str();
		data.m_textures.at(0) = &m_yBlurRenderTarget.GetRenderTargetTexture();
		data.m_width = m_mainRenderTarget.GetWidth();
		data.m_height = m_mainRenderTarget.GetHeight();

		std::unique_ptr<Sprite> sprite = std::make_unique<Sprite>();
		sprite->Init(data);
		m_copyToFbSprite = std::move(sprite);
	}

	ModelInitData initData = { };
	{
		initData.m_fxFilePath = fx3dFilePath.c_str();
	}
	{
		initData.m_tkmFilePath = tkmBgFilePath.c_str();
		std::unique_ptr<Model> model = std::make_unique<Model>();
		model->Init(initData);
		m_models.at(static_cast<size_t>(ModelType::kBg)) = std::move(model);
	}
	{
		initData.m_tkmFilePath = tkmSampleFilePath.c_str();
		std::unique_ptr<Model> model = std::make_unique<Model>();
		model->Init(initData);
		m_models.at(static_cast<size_t>(ModelType::kPlayer)) = std::move(model);
	}
}

void Models_10_04::handleInput()
{
	{
		m_plPos.x -= MiniEngineIf::getStick(MiniEngineIf::StickType::kLX);
		m_plPos.z -= MiniEngineIf::getStick(MiniEngineIf::StickType::kLY);

		m_models.at(static_cast<size_t>(ModelType::kPlayer))->UpdateWorldMatrix(m_plPos, g_quatIdentity, g_vec3One);
	}

	if (MiniEngineIf::isTrigger(MiniEngineIf::Button::kA))
	{
		m_cb.blur = (m_cb.blur == static_cast<Blur>(static_cast<size_t>(Blur::kSize) - 1)) ?
			static_cast<Blur>(0) :
			static_cast<Blur>(static_cast<size_t>(m_cb.blur) + 1);
	}
}

void Models_10_04::draw(RenderContext& renderContext)
{
	// render to main render target
	{
		renderContext.WaitUntilToPossibleSetRenderTarget(m_mainRenderTarget);

		renderContext.SetRenderTargetAndViewport(m_mainRenderTarget);
		renderContext.ClearRenderTargetView(m_mainRenderTarget);

		m_models.at(static_cast<size_t>(ModelType::kPlayer))->Draw(renderContext);
		m_models.at(static_cast<size_t>(ModelType::kBg))->Draw(renderContext);

		renderContext.WaitUntilFinishDrawingToRenderTarget(m_mainRenderTarget);
	}

	// apply blur in X axis
	{
		renderContext.WaitUntilToPossibleSetRenderTarget(m_xBlurRenderTarget);

		renderContext.SetRenderTargetAndViewport(m_xBlurRenderTarget);
		renderContext.ClearRenderTargetView(m_xBlurRenderTarget);

		m_xBlurSprite->Draw(renderContext);

		renderContext.WaitUntilFinishDrawingToRenderTarget(m_xBlurRenderTarget);
	}

	// apply blur in Y axis
	{
		renderContext.WaitUntilToPossibleSetRenderTarget(m_yBlurRenderTarget);

		renderContext.SetRenderTargetAndViewport(m_yBlurRenderTarget);
		renderContext.ClearRenderTargetView(m_yBlurRenderTarget);

		m_yBlurSprite->Draw(renderContext);

		renderContext.WaitUntilFinishDrawingToRenderTarget(m_yBlurRenderTarget);
	}

	// render to offscreen buffer managed in MiniengineIf
	{
		MiniEngineIf::setOffscreenRenderTarget();

		m_copyToFbSprite->Draw(renderContext);
	}
}

void Models_10_04::debugRenderParams()
{
	ImguiIf::printParams<float>(ImguiIf::VarType::kFloat, "Player", std::vector<const float*>{ &m_plPos.x, & m_plPos.y, & m_plPos.z });
	ImguiIf::printParams<int32_t>(ImguiIf::VarType::kInt32, "Blur", std::vector<const int32_t*>{ reinterpret_cast<int32_t*>(&m_cb.blur)});
}

void Models_10_04::CalcWeightsTableFromGaussian(float* weightsTbl, int sizeOfWeightsTbl, float sigma)
{
	// 重みの合計を記録する変数を定義する
	float total = 0;

	// ここからガウス関数を用いて重みを計算している
	// ループ変数のxが基準テクセルからの距離
	for (int x = 0; x < sizeOfWeightsTbl; x++)
	{
		weightsTbl[x] = expf(-0.5f * (float)(x * x) / sigma);
		total += 2.0f * weightsTbl[x];
	}

	// 重みの合計で除算することで、重みの合計を1にしている
	for (int i = 0; i < sizeOfWeightsTbl; i++)
	{
		weightsTbl[i] /= total;
	}
}

namespace ModelHandler {
	std::unique_ptr<IModels> loadModelForChap10_04()
	{
		ModelFactory_10_04 factory;
		std::unique_ptr<IModels> iModels = factory.create();
		iModels->debugRenderParams();
		return std::move(iModels);
	}
} // namespace ModelHandler

