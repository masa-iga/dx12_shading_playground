#include "model_10_07.h"
#include <filesystem>
#include <memory>
#include "imodel.h"
#include "MiniEngine.h"
#include "model_util.h"
#include "../config.h"
#include "../debug_win.h"
#include "../imgui_if.h"
#include "../miniEngine_if.h"

class ModelFactory_10_07 : public IModelFactory
{
public:
	~ModelFactory_10_07() { }
	std::unique_ptr<IModels> create(RenderContext& renderContext);
};

class Models_10_07 : public IModels
{
public:
	Models_10_07() { m_models.resize(static_cast<size_t>(ModelType::kSize)); }
	~Models_10_07() { }
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
		kObject,
		kSize,
	};

	static constexpr float kBlurPower = 5;
	static constexpr DXGI_FORMAT kBufferFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
	const std::string kTkmSampleFile = "Sample_10_07/Sample_10_07/Assets/modelData/bg/bg.tkm";
	const std::string kFx2dFile = "Sample_10_07/Sample_10_07/Assets/shader/preset/sample2D.fx";
	const std::string kFx3dFile = "Assets/shader/sample_10_07_3D.fx";
	const std::string kFxPostEffectFile = "Assets/shader/sample_10_07_postEffect.fx";
	std::string getTkmSampleFilePath() { return ModelUtil::getPathFromAssetDir(kTkmSampleFile); }
	std::string getFx2dFilePath() { return ModelUtil::getPathFromAssetDir(kFx2dFile); }
	std::string getFx3dFilePath() { return kFx3dFile; }
	std::string getFxPostEffectFilePath() { return kFxPostEffectFile; }

	RenderTarget m_mainRenderTarget;
	RenderTarget m_depthRenderTarget;
	GaussianBlur m_blur;
	std::unique_ptr<Sprite> m_combineBokeImageSprite = nullptr;
	std::unique_ptr<Sprite> m_copyToFbSprite = nullptr;
	Light m_light;
};

std::unique_ptr<IModels> ModelFactory_10_07::create(RenderContext& renderContext)
{
	std::unique_ptr<Models_10_07> m(new Models_10_07);
	{
		m->createModel(renderContext);
	}
	return std::move(m);
}

void Models_10_07::resetCamera()
{
	;
}

void Models_10_07::createModel([[maybe_unused]] RenderContext& renderContext)
{
	{
		m_light.m_directLight.at(0) = {
			.m_direction = { 0.0f, 0.0f, -1.0f },
			.pad0 = 0.0f,
			.color = { 2.0f, 2.0f, 2.0f, 0.0f},
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
		auto bRet = m_depthRenderTarget.Create(
			m_mainRenderTarget.GetWidth(),
			m_mainRenderTarget.GetHeight(),
			1,
			1,
			DXGI_FORMAT_R32_FLOAT,
			DXGI_FORMAT_UNKNOWN
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
		m_blur.Init(&m_mainRenderTarget.GetRenderTargetTexture());
	}

	{
		SpriteInitData d;
		{
			d.m_textures.at(0) = &m_blur.GetBokeTexture();
			d.m_textures.at(1) = &m_depthRenderTarget.GetRenderTargetTexture();
			d.m_width = m_mainRenderTarget.GetWidth();
			d.m_height = m_mainRenderTarget.GetHeight();
			d.m_fxFilePath = fxPostEffectFilePath.c_str();
			d.m_colorBufferFormat.at(0) = kBufferFormat;
			d.m_alphaBlendMode = AlphaBlendMode::AlphaBlendMode_Trans;

			std::unique_ptr<Sprite> sprite = std::make_unique<Sprite>();
			sprite->Init(d);
			m_combineBokeImageSprite = std::move(sprite);
		}
	}
	{
		SpriteInitData d;
		d.m_fxFilePath = fx2dFilePath.c_str();
		d.m_textures.at(0) = &m_mainRenderTarget.GetRenderTargetTexture();
		d.m_width = m_mainRenderTarget.GetWidth();
		d.m_height = m_mainRenderTarget.GetHeight();

		std::unique_ptr<Sprite> sprite = std::make_unique<Sprite>();
		sprite->Init(d);
		m_copyToFbSprite = std::move(sprite);
	}

	{
		ModelInitData d = { };
		{
			d.m_tkmFilePath = tkmSampleFilePath.c_str();
			d.m_fxFilePath = fx3dFilePath.c_str();
			d.m_expandConstantBuffer = &m_light;
			d.m_expandConstantBufferSize = sizeof(m_light);
			d.m_colorBufferFormat.at(0) = m_mainRenderTarget.GetColorBufferFormat();
			d.m_colorBufferFormat.at(1) = m_depthRenderTarget.GetColorBufferFormat();
		}

		std::unique_ptr<Model> model = std::make_unique<Model>();
		model->Init(d);
		m_models.at(static_cast<size_t>(ModelType::kObject)) = std::move(model);
	}
}

void Models_10_07::handleInput()
{
	// move camera
	{
		MiniEngineIf::getCamera3D()->MoveForward(MiniEngineIf::getStick(MiniEngineIf::StickType::kLY) * 10.0f);
		MiniEngineIf::getCamera3D()->MoveRight(MiniEngineIf::getStick(MiniEngineIf::StickType::kLX) * 10.0f);
	}
}

void Models_10_07::draw(RenderContext& renderContext)
{
	// render color & depth to main render target
	{
		RenderTarget* rts[] = {
			&m_mainRenderTarget,
			&m_depthRenderTarget,
		};

		renderContext.WaitUntilToPossibleSetRenderTargets(2, rts);
		renderContext.SetRenderTargetsAndViewport(2, rts);

		renderContext.ClearRenderTargetViews(2, rts);

		m_models.at(static_cast<size_t>(ModelType::kObject))->Draw(renderContext);

		renderContext.WaitUntilFinishDrawingToRenderTargets(2, rts);
	}

	// render boke image to m_blur's render targets
	{
		m_blur.ExecuteOnGPU(renderContext, kBlurPower);
	}

	// render final image w/ DoF to main render target
	{
		renderContext.WaitUntilToPossibleSetRenderTarget(m_mainRenderTarget);
		renderContext.SetRenderTargetAndViewport(m_mainRenderTarget);

		m_combineBokeImageSprite->Draw(renderContext);

		renderContext.WaitUntilFinishDrawingToRenderTarget(m_mainRenderTarget);
	}

	// copy to offscreen buffer managed in MiniEngineIf
	{
		MiniEngineIf::setOffscreenRenderTarget();

		m_copyToFbSprite->Draw(renderContext);
	}
}

void Models_10_07::debugRenderParams()
{
	{
		const Vector3& pos = MiniEngineIf::getCamera3D()->GetPosition();
		ImguiIf::printParams<float>(ImguiIf::VarType::kFloat, "Camera", std::vector<const float*>{ &pos.x, & pos.y, & pos.z });
	}

	{
		const DirectionalLight& l = m_light.m_directLight.at(0);
		ImguiIf::printParams<float>(ImguiIf::VarType::kFloat, "Light dir", std::vector<const float*>{ &l.m_direction.x, & l.m_direction.y, & l.m_direction.z });
		ImguiIf::printParams<float>(ImguiIf::VarType::kFloat, "Light col", std::vector<const float*>{ &l.color.x, & l.color.y, & l.color.z });
	}
}

namespace ModelHandler {
	std::unique_ptr<IModels> loadModelForChap10_07(RenderContext& renderContext)
	{
		ModelFactory_10_07 factory;
		std::unique_ptr<IModels> iModels = factory.create(renderContext);
		iModels->debugRenderParams();
		return std::move(iModels);
	}
} // namespace ModelHandler

