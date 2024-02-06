#include "model_10_06.h"
#include <filesystem>
#include <memory>
#include "imodel.h"
#include "MiniEngine.h"
#include "model_util.h"
#include "../config.h"
#include "../debug_win.h"
#include "../imgui_if.h"
#include "../miniEngine_if.h"

class ModelFactory_10_06 : public IModelFactory
{
public:
	~ModelFactory_10_06() { }
	std::unique_ptr<IModels> create(RenderContext& renderContext);
};

class Models_10_06 : public IModels
{
public:
	Models_10_06() { m_models.resize(static_cast<size_t>(ModelType::kSize)); }
	~Models_10_06() { }
	void createModel(RenderContext& renderContext);
	void resetCamera();
	void handleInput();
	void draw(RenderContext& renderContext);
	void debugRenderParams();

private:
	static constexpr size_t kNumDirectLight = 4;

	struct DirectionalLight
	{
		Vector3 m_direction;
		float pad0 = 0.0f;
		Vector4 color;
	};

	struct Light
	{
		std::array<DirectionalLight, kNumDirectLight> m_directLight;
		Vector3 eyePos;
		float specRow = 0.0f;
		Vector3 ambientLight;
	};

	enum class ModelType {
		kPlayer,
		kSize,
	};

	static constexpr size_t kNumWeights = 8;
	static constexpr float kBlurPower = 10;
	static constexpr DXGI_FORMAT kBufferFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
	const std::string kTkmSampleFile = "Sample_10_05/Sample_10_05/Assets/modelData/sample.tkm";
	const std::string kFx2dFile = "Sample_10_05/Sample_10_05/Assets/shader/sample2D.fx";
	const std::string kFx3dFile = "Sample_10_05/Sample_10_05/Assets/shader/sample3D.fx";
	const std::string kFxPostEffectFile = "Assets/shader/sample_10_06_postEffect.fx";
	std::string getTkmSampleFilePath() { return ModelUtil::getPathFromAssetDir(kTkmSampleFile); }
	std::string getFx2dFilePath() { return ModelUtil::getPathFromAssetDir(kFx2dFile); }
	std::string getFx3dFilePath() { return ModelUtil::getPathFromAssetDir(kFx3dFile); }
	std::string getFxPostEffectFilePath() { return kFxPostEffectFile; }

	RenderTarget m_mainRenderTarget;
	RenderTarget m_luminanceRenderTarget;
	Vector3 m_plPos;
	std::array<GaussianBlur, 4> m_gaussianBlurs;
	std::unique_ptr<Sprite> m_copyToFbSprite = nullptr;
	std::unique_ptr<Sprite> m_luminanceSprite = nullptr;
	std::unique_ptr<Sprite> m_finalSprite = nullptr;
	Light m_light;

	struct ConstantBuffer {
		std::array<float, kNumWeights> m_weights;
	};

	ConstantBuffer m_cb;
};

std::unique_ptr<IModels> ModelFactory_10_06::create(RenderContext& renderContext)
{
	std::unique_ptr<Models_10_06> m(new Models_10_06);
	{
		m->createModel(renderContext);
	}
	return std::move(m);
}

void Models_10_06::resetCamera()
{
	;
}

void Models_10_06::createModel([[maybe_unused]] RenderContext& renderContext)
{
	{
		m_light.m_directLight.at(0) = {
			.m_direction = { 1.0f, 0.0f, 0.0f },
			.pad0 = 0.0f,
			.color = { 40.8f, 40.8f, 40.8f, 0.0f},
		};
		m_light.ambientLight = Vector3(0.5f, 0.5f, 0.5f);
		m_light.eyePos = MiniEngineIf::getCamera3D()->GetPosition();

		m_light.m_directLight.at(0).m_direction.Normalize();
	}

	{
		auto bRet = m_mainRenderTarget.Create(
			Config::kRenderTargetWidth,
			Config::kRenderTargetHeight,
			1,
			1,
			kBufferFormat,
			DXGI_FORMAT_D32_FLOAT
		);
		Dbg::assert_(bRet);
	}
	{
		auto bRet = m_luminanceRenderTarget.Create(
			m_mainRenderTarget.GetWidth(),
			m_mainRenderTarget.GetHeight(),
			1,
			1,
			m_mainRenderTarget.GetColorBufferFormat(),
			DXGI_FORMAT_D32_FLOAT
		);
		Dbg::assert_(bRet);
	}

	const std::string tkmSampleFilePath = getTkmSampleFilePath();
	const std::string fx2dFilePath = getFx2dFilePath();
	const std::string fx3dFilePath = getFx3dFilePath();
	const std::string fxPostEffectFilePath = getFxPostEffectFilePath();
	Dbg::assert_(std::filesystem::exists(tkmSampleFilePath));
	Dbg::assert_(std::filesystem::exists(fx2dFilePath));
	Dbg::assert_(std::filesystem::exists(fx3dFilePath));
	Dbg::assert_(std::filesystem::exists(fxPostEffectFilePath));

	{
		SpriteInitData data;
		{
			data.m_fxFilePath = fxPostEffectFilePath.c_str();
			data.m_vsEntryPointFunc = "VSMain";
			data.m_psEntryPoinFunc = "PSSamplingLuminance";
			data.m_width = m_mainRenderTarget.GetWidth();
			data.m_height = m_mainRenderTarget.GetHeight();
			data.m_textures.at(0) = &m_mainRenderTarget.GetRenderTargetTexture();
			data.m_colorBufferFormat.at(0) = m_mainRenderTarget.GetColorBufferFormat();
		}
		std::unique_ptr<Sprite> sprite = std::make_unique<Sprite>();
		sprite->Init(data);
		m_luminanceSprite = std::move(sprite);
	}

	{
		m_gaussianBlurs.at(0).Init(&m_luminanceRenderTarget.GetRenderTargetTexture());
		m_gaussianBlurs.at(1).Init(&m_gaussianBlurs.at(0).GetBokeTexture());
		m_gaussianBlurs.at(2).Init(&m_gaussianBlurs.at(1).GetBokeTexture());
		m_gaussianBlurs.at(3).Init(&m_gaussianBlurs.at(2).GetBokeTexture());
	}

	{
		SpriteInitData d;
		{
			d.m_textures.at(0) = &m_gaussianBlurs.at(0).GetBokeTexture();
			d.m_textures.at(1) = &m_gaussianBlurs.at(1).GetBokeTexture();
			d.m_textures.at(2) = &m_gaussianBlurs.at(2).GetBokeTexture();
			d.m_textures.at(3) = &m_gaussianBlurs.at(3).GetBokeTexture();
			d.m_width = m_mainRenderTarget.GetWidth();
			d.m_height = m_mainRenderTarget.GetHeight();
			d.m_fxFilePath = fxPostEffectFilePath.c_str();
			d.m_psEntryPoinFunc = "PSBloomFinal";
			d.m_colorBufferFormat.at(0) = kBufferFormat;
		}
		std::unique_ptr<Sprite> sprite = std::make_unique<Sprite>();
		sprite->Init(d);
		m_finalSprite = std::move(sprite);
	}

	{
		SpriteInitData data;
		data.m_fxFilePath = fx2dFilePath.c_str();
		data.m_textures.at(0) = &m_mainRenderTarget.GetRenderTargetTexture();
		data.m_width = m_mainRenderTarget.GetWidth();
		data.m_height = m_mainRenderTarget.GetHeight();

		std::unique_ptr<Sprite> sprite = std::make_unique<Sprite>();
		sprite->Init(data);
		m_copyToFbSprite = std::move(sprite);
	}

	{
		ModelInitData initData = { };
		initData.m_fxFilePath = fx3dFilePath.c_str();
		initData.m_tkmFilePath = tkmSampleFilePath.c_str();
		initData.m_expandConstantBuffer = &m_light;
		initData.m_expandConstantBufferSize = sizeof(m_light);
		initData.m_colorBufferFormat.at(0) = kBufferFormat;

		std::unique_ptr<Model> model = std::make_unique<Model>();
		model->Init(initData);
		m_models.at(static_cast<size_t>(ModelType::kPlayer)) = std::move(model);
	}
}

void Models_10_06::handleInput()
{
	{
		DirectionalLight& light = m_light.m_directLight.at(0);
		light.color.x += MiniEngineIf::getStick(MiniEngineIf::StickType::kLX) * 0.5f;
		light.color.y += MiniEngineIf::getStick(MiniEngineIf::StickType::kLX) * 0.5f;
		light.color.z += MiniEngineIf::getStick(MiniEngineIf::StickType::kLX) * 0.5f;

		light.color.x = std::clamp(m_light.m_directLight.at(0).color.x, 0.0f, 100.0f);
		light.color.y = std::clamp(m_light.m_directLight.at(0).color.y, 0.0f, 100.0f);
		light.color.z = std::clamp(m_light.m_directLight.at(0).color.z, 0.0f, 100.0f);
	}
}

void Models_10_06::draw(RenderContext& renderContext)
{
	// render to main render target
	{
		renderContext.WaitUntilToPossibleSetRenderTarget(m_mainRenderTarget);
		renderContext.SetRenderTargetAndViewport(m_mainRenderTarget);
		renderContext.ClearRenderTargetView(m_mainRenderTarget);

		m_models.at(static_cast<size_t>(ModelType::kPlayer))->Draw(renderContext);

		renderContext.WaitUntilFinishDrawingToRenderTarget(m_mainRenderTarget);
	}

	// render to luminance render target
	{
		renderContext.WaitUntilToPossibleSetRenderTarget(m_luminanceRenderTarget);
		renderContext.SetRenderTargetAndViewport(m_luminanceRenderTarget);
		renderContext.ClearRenderTargetView(m_luminanceRenderTarget);

		m_luminanceSprite->Draw(renderContext);

		renderContext.WaitUntilFinishDrawingToRenderTarget(m_luminanceRenderTarget);
	}

	// render boke textures
	{
		m_gaussianBlurs.at(0).ExecuteOnGPU(renderContext, kBlurPower);
		m_gaussianBlurs.at(1).ExecuteOnGPU(renderContext, kBlurPower);
		m_gaussianBlurs.at(2).ExecuteOnGPU(renderContext, kBlurPower);
		m_gaussianBlurs.at(3).ExecuteOnGPU(renderContext, kBlurPower);
	}

	// render boke image to main render target
	{
		renderContext.WaitUntilToPossibleSetRenderTarget(m_mainRenderTarget);
		renderContext.SetRenderTargetAndViewport(m_mainRenderTarget);

		m_finalSprite->Draw(renderContext);

		renderContext.WaitUntilFinishDrawingToRenderTarget(m_mainRenderTarget);
	}

	// render to offscreen buffer managed in MiniengineIf
	{
		MiniEngineIf::setOffscreenRenderTarget();

		m_copyToFbSprite->Draw(renderContext);
	}
}

void Models_10_06::debugRenderParams()
{
	const DirectionalLight& l = m_light.m_directLight.at(0);
	ImguiIf::printParams<float>(ImguiIf::VarType::kFloat, "Light dir", std::vector<const float*>{ &l.m_direction.x, & l.m_direction.y, & l.m_direction.z });
	ImguiIf::printParams<float>(ImguiIf::VarType::kFloat, "Light col", std::vector<const float*>{ &l.color.x, & l.color.y, & l.color.z });
}

namespace ModelHandler {
	std::unique_ptr<IModels> loadModelForChap10_06(RenderContext& renderContext)
	{
		ModelFactory_10_06 factory;
		std::unique_ptr<IModels> iModels = factory.create(renderContext);
		iModels->debugRenderParams();
		return std::move(iModels);
	}
} // namespace ModelHandler

