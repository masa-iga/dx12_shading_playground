#include "model_11_01.h"
#include <filesystem>
#include <memory>
#include "imodel.h"
#include "MiniEngine.h"
#include "model_util.h"
#include "../config.h"
#include "../debug_win.h"
#include "../imgui_if.h"
#include "../miniEngine_if.h"

class ModelFactory_11_01 : public IModelFactory
{
public:
	~ModelFactory_11_01() { }
	std::unique_ptr<IModels> create();
};

class Models_11_01 : public IModels
{
public:
	Models_11_01() { m_models.resize(static_cast<size_t>(ModelType::kSize)); }
	~Models_11_01() { }
	void createModel();
	void resetCamera();
	void handleInput();
	void draw(RenderContext& renderContext);
	void debugRenderParams();

private:
	static constexpr size_t kNumDirectLight = 4;

	struct DirectionalLight
	{
		Vector3 m_direction;
		float m_pad0 = 0.0f;
		Vector4 m_color;
	};

	struct Light
	{
		std::array<DirectionalLight, kNumDirectLight> m_directLight;
		Vector3 m_eyePos;
		float m_specRow = 0.0f;
		Vector3 m_ambientLight;
	};

	enum class ModelType {
		kObject,
		kSize,
	};

	static constexpr size_t kNumOfSphere = 50;
	static constexpr float kBlurPower = 5;
	static constexpr DXGI_FORMAT kColorBufferFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
	static constexpr DXGI_FORMAT kDepthBufferFormat = DXGI_FORMAT_UNKNOWN;
	const std::string kTkmSampleFile = "Sample_10_08/Sample_10_08/Assets/modelData/bg/bg.tkm";
	const std::string kTkmSphereFile = "Sample_10_08/Sample_10_08/Assets/modelData/sphere.tkm";
	const std::string kFx2dFile = "Sample_10_08/Sample_10_08/Assets/shader/preset/sample2D.fx";
	const std::string kFx3dFile = "Sample_10_08/Sample_10_08/Assets/shader/preset/sample3D.fx";
	const std::string kFxPostEffectFile = "Sample_10_08/Sample_10_08/Assets/shader/preset/samplePostEffect.fx";
	const std::string kFxSampleFile = "Assets/shader/sample_10_08.fx";
	std::string getTkmSampleFilePath() { return ModelUtil::getPathFromAssetDir(kTkmSampleFile); }
	std::string getTkmSphereFilePath() { return ModelUtil::getPathFromAssetDir(kTkmSphereFile); }
	std::string getFx2dFilePath() { return ModelUtil::getPathFromAssetDir(kFx2dFile); }
	std::string getFx3dFilePath() { return ModelUtil::getPathFromAssetDir(kFx3dFile); }
	std::string getFxPostEffectPath() { return ModelUtil::getPathFromAssetDir(kFxPostEffectFile); }
	std::string getFxSampleFilePath() { return kFxSampleFile; }

	std::array<std::unique_ptr<Model>, kNumOfSphere> m_sphereModels;
	RenderTarget m_mainRenderTarget;
	RenderTarget m_depthRenderTarget;
	RenderTarget m_rtVerticalBlur;
	RenderTarget m_rtDiagonalBlur;
	RenderTarget m_rtPhomboidBlur;
	std::unique_ptr<Sprite> m_vertDiagonalBlurSprite = nullptr;
	std::unique_ptr<Sprite> m_phomboidBlurSprite = nullptr;
	std::unique_ptr<Sprite> m_combineBokeImageSprite = nullptr;
	std::unique_ptr<Sprite> m_copyToFbSprite = nullptr;
	Light m_light;
	std::array<Light, kNumOfSphere> m_sphereLight;
};

std::unique_ptr<IModels> ModelFactory_11_01::create()
{
	std::unique_ptr<Models_11_01> m(new Models_11_01);
	{
		m->createModel();
	}
	return std::move(m);
}

void Models_11_01::resetCamera()
{
	;
}

void Models_11_01::createModel()
{
	{
		auto bRet = m_mainRenderTarget.Create(
			Config::kRenderTargetWidth,
			Config::kRenderTargetHeight,
			1,
			1,
			kColorBufferFormat,
			DXGI_FORMAT_D32_FLOAT
		);
		Dbg::assert_(bRet);
	}
	{
		float clearColor[] = { 10000.0f, 10000.0f, 10000.0f, 1.0f };

		auto bRet = m_depthRenderTarget.Create(
			m_mainRenderTarget.GetWidth(),
			m_mainRenderTarget.GetHeight(),
			1,
			1,
			DXGI_FORMAT_R32_FLOAT,
			DXGI_FORMAT_UNKNOWN,
			clearColor
		);
		Dbg::assert_(bRet);
	}
	{
		auto bRet = m_rtVerticalBlur.Create(
			Config::kRenderTargetWidth,
			Config::kRenderTargetHeight,
			1,
			1,
			kColorBufferFormat,
			kDepthBufferFormat
		);
		Dbg::assert_(bRet);
	}
	{
		auto bRet = m_rtDiagonalBlur.Create(
			Config::kRenderTargetWidth,
			Config::kRenderTargetHeight,
			1,
			1,
			kColorBufferFormat,
			kDepthBufferFormat
		);
		Dbg::assert_(bRet);
	}
	{
		auto bRet = m_rtPhomboidBlur.Create(
			Config::kRenderTargetWidth,
			Config::kRenderTargetHeight,
			1,
			1,
			kColorBufferFormat,
			kDepthBufferFormat
		);
		Dbg::assert_(bRet);
	}

	const std::string tkmSampleFilePath = getTkmSampleFilePath();
	const std::string tkmSphereFilePath = getTkmSphereFilePath();
	const std::string fx2dFilePath = getFx2dFilePath();
	const std::string fx3dFilePath = getFx3dFilePath();
	const std::string fxPostEffectFilePath = getFxPostEffectPath();
	const std::string fxSampleFilePath = getFxSampleFilePath();
	Dbg::assert_(std::filesystem::exists(tkmSampleFilePath));
	Dbg::assert_(std::filesystem::exists(tkmSphereFilePath));
	Dbg::assert_(std::filesystem::exists(fx2dFilePath));
	Dbg::assert_(std::filesystem::exists(fx3dFilePath));
	Dbg::assert_(std::filesystem::exists(fxPostEffectFilePath));
	Dbg::assert_(std::filesystem::exists(fxSampleFilePath));

	{
		SpriteInitData d;
		{
			d.m_textures.at(0) = &m_mainRenderTarget.GetRenderTargetTexture();
			d.m_width = m_mainRenderTarget.GetWidth();
			d.m_height = m_mainRenderTarget.GetHeight();
			d.m_fxFilePath = fxSampleFilePath.c_str();
			d.m_psEntryPoinFunc = "PSVerticalDiagonalBlur";
			d.m_colorBufferFormat.at(0) = kColorBufferFormat;
			d.m_colorBufferFormat.at(1) = kColorBufferFormat;
		}
		std::unique_ptr<Sprite> s = std::make_unique<Sprite>();
		s->Init(d);
		m_vertDiagonalBlurSprite = std::move(s);
	}
	{
		SpriteInitData d;
		{
			d.m_textures.at(0) = &m_rtVerticalBlur.GetRenderTargetTexture();
			d.m_textures.at(1) = &m_rtDiagonalBlur.GetRenderTargetTexture();
			d.m_width = m_mainRenderTarget.GetWidth();
			d.m_height = m_mainRenderTarget.GetHeight();
			d.m_fxFilePath = fxSampleFilePath.c_str();
			d.m_psEntryPoinFunc = "PSRhomboidBlur";
			d.m_colorBufferFormat.at(0) = kColorBufferFormat;
		}
		std::unique_ptr<Sprite> s = std::make_unique<Sprite>();
		s->Init(d);
		m_phomboidBlurSprite = std::move(s);
	}
	{
		SpriteInitData d;
		{
			d.m_textures.at(0) = &m_rtPhomboidBlur.GetRenderTargetTexture();
			d.m_textures.at(1) = &m_depthRenderTarget.GetRenderTargetTexture();
			d.m_width = m_mainRenderTarget.GetWidth();
			d.m_height = m_mainRenderTarget.GetHeight();
			d.m_fxFilePath = fxPostEffectFilePath.c_str();
			d.m_colorBufferFormat.at(0) = kColorBufferFormat;
			d.m_alphaBlendMode = AlphaBlendMode::AlphaBlendMode_Trans;
		}

		std::unique_ptr<Sprite> s = std::make_unique<Sprite>();
		s->Init(d);
		m_combineBokeImageSprite = std::move(s);
	}
	{
		SpriteInitData d;
		{
			d.m_fxFilePath = fx2dFilePath.c_str();
			d.m_textures.at(0) = &m_mainRenderTarget.GetRenderTargetTexture();
			d.m_width = m_mainRenderTarget.GetWidth();
			d.m_height = m_mainRenderTarget.GetHeight();
		}

		std::unique_ptr<Sprite> sprite = std::make_unique<Sprite>();
		sprite->Init(d);
		m_copyToFbSprite = std::move(sprite);
	}

	{
		{
			m_light.m_directLight.at(0) = {
				.m_direction = { 0.0f, 0.0f, -1.0f },
				.m_pad0 = 0.0f,
				.m_color = { 2.0f, 2.0f, 2.0f, 0.0f},
			};
			m_light.m_ambientLight = Vector3(0.5f, 0.5f, 0.5f);
			m_light.m_eyePos = MiniEngineIf::getCamera3D()->GetPosition();

			m_light.m_directLight.at(0).m_direction.Normalize();
		}

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
	{
		ModelInitData d = { };
		{
			d.m_tkmFilePath = tkmSphereFilePath.c_str();
			d.m_fxFilePath = fx3dFilePath.c_str();
			d.m_colorBufferFormat.at(0) = kColorBufferFormat;
			d.m_colorBufferFormat.at(1) = kDepthBufferFormat;
		}

		for (int32_t i = 0; i < kNumOfSphere; ++i)
		{
			{
				Light& l = m_sphereLight.at(i);
				l.m_directLight.at(0).m_direction = { 0.0f, 0.0f, -1.0f };
				l.m_directLight.at(0).m_direction.Normalize();
				l.m_eyePos = MiniEngineIf::getCamera3D()->GetPosition();

				switch (rand() % 3)
				{
				case 0:
					l.m_directLight.at(0).m_color = { 200.0f, 0.0f, 0.0f, 0.0f };
					l.m_ambientLight = { 0.5f, 0.0f, 0.0f };
					break;
				case 1:
					l.m_directLight.at(0).m_color = { 0.0f, 200.0f, 0.0f, 0.0f };
					l.m_ambientLight = { 0.0f, 0.5f, 0.0f };
					break;
				case 2:
					l.m_directLight.at(0).m_color = { 0.0f, 0.0f, 200.0f, 0.0f };
					l.m_ambientLight = { 0.0f, 0.0f, 0.5f };
					break;
				default:
					break;
				}

				d.m_expandConstantBuffer = &l;
				d.m_expandConstantBufferSize = sizeof(l);

				std::unique_ptr<Model> m = std::make_unique<Model>();
				m->Init(d);
				m_sphereModels.at(i) = std::move(m);
			}

			{
				auto getRand = []()
				{
					return (rand() % 100) / 100.0f;
				};
				const Vector3 pos = {
					Math::Lerp(getRand(), -250.0f, 250.0f),
					Math::Lerp(getRand(), 20.0f, 200.0f),
					Math::Lerp(getRand(), 0.0f, -2000.0f),
				};
				m_sphereModels.at(i)->UpdateWorldMatrix(
					pos,
					g_quatIdentity,
					{ 0.5f, 0.5f, 0.5f }
				);
			}
		}
	}
}

void Models_11_01::handleInput()
{
	// move camera
	{
		MiniEngineIf::getCamera3D()->MoveForward(MiniEngineIf::getStick(MiniEngineIf::StickType::kLY) * 3.0f);
		MiniEngineIf::getCamera3D()->MoveRight(MiniEngineIf::getStick(MiniEngineIf::StickType::kLX) * 3.0f);
	}
}

void Models_11_01::draw(RenderContext& renderContext)
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

		// draw models
		{
			m_models.at(static_cast<size_t>(ModelType::kObject))->Draw(renderContext);

			for (auto& m : m_sphereModels)
				m->Draw(renderContext);
		}

		renderContext.WaitUntilFinishDrawingToRenderTargets(2, rts);
	}

	// apply vertical & diagonal blur
	{
		RenderTarget* blurRts[] = {
			&m_rtVerticalBlur,
			&m_rtDiagonalBlur,
		};
		renderContext.WaitUntilToPossibleSetRenderTargets(2, blurRts);

		renderContext.SetRenderTargets(2, blurRts);
		renderContext.ClearRenderTargetViews(2, blurRts);
		m_vertDiagonalBlurSprite->Draw(renderContext);

		renderContext.WaitUntilFinishDrawingToRenderTargets(2, blurRts);
	}

	// apply phomboid blur
	{
		renderContext.WaitUntilToPossibleSetRenderTarget(m_rtPhomboidBlur);

		renderContext.SetRenderTargetAndViewport(m_rtPhomboidBlur);
		m_phomboidBlurSprite->Draw(renderContext);

		renderContext.WaitUntilFinishDrawingToRenderTarget(m_rtPhomboidBlur);
	}

	// render boke image with m_phomboidBlurSprite + depth render target
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

void Models_11_01::debugRenderParams()
{
	{
		const Vector3& pos = MiniEngineIf::getCamera3D()->GetPosition();
		ImguiIf::printParams<float>(ImguiIf::VarType::kFloat, "Camera", std::vector<const float*>{ &pos.x, & pos.y, & pos.z });
	}
}

namespace ModelHandler {
	std::unique_ptr<IModels> loadModelForChap11_01()
	{
		ModelFactory_11_01 factory;
		std::unique_ptr<IModels> iModels = factory.create();
		iModels->debugRenderParams();
		return std::move(iModels);
	}
} // namespace ModelHandler

