#include "model_10_03.h"
#include <filesystem>
#include <memory>
#include "imodel.h"
#include "MiniEngine.h"
#include "model_util.h"
#include "../config.h"
#include "../debug_win.h"
#include "../imgui_if.h"
#include "../miniEngine_if.h"

class ModelFactory_10_03 : public IModelFactory
{
public:
	~ModelFactory_10_03() { }
	std::unique_ptr<IModels> create(RenderContext& renderContext);
};

class Models_10_03 : public IModels
{
public:
	Models_10_03() { m_models.resize(static_cast<size_t>(ModelType::kSize)); }
	~Models_10_03() { }
	void createModel(RenderContext& renderContext);
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

	const std::string kTkmBgFile = "Sample_10_03/Sample_10_03/Assets/modelData/bg/bg.tkm";
	const std::string kTkmSampleFile = "Sample_10_03/Sample_10_03/Assets/modelData/sample.tkm";
	const std::string kFx3dFile = "Sample_10_03/Sample_10_03/Assets/shader/sample3D.fx";
	const std::string kFxPostEffectFile = "Assets/shader/sample_10_03_postEffect.fx";
	std::string getTkmBgFilePath() { return ModelUtil::getPathFromAssetDir(kTkmBgFile); }
	std::string getTkmSampleFilePath() { return ModelUtil::getPathFromAssetDir(kTkmSampleFile); }
	std::string getFx3dFilePath() { return ModelUtil::getPathFromAssetDir(kFx3dFile); }
	std::string getFxPostEffectFilePath() { return kFxPostEffectFile; }

	RenderTarget m_offscreenRenderTarget;
	Vector3 m_plPos;
	std::unique_ptr<Sprite> m_sprite = nullptr;
};

std::unique_ptr<IModels> ModelFactory_10_03::create(RenderContext& renderContext)
{
	std::unique_ptr<Models_10_03> m(new Models_10_03);
	{
		m->createModel(renderContext);
	}
	return std::move(m);
}

void Models_10_03::resetCamera()
{
	;
}

void Models_10_03::createModel([[maybe_unused]] RenderContext& renderContext)
{
	m_offscreenRenderTarget.Create(
		Config::kRenderTargetWidth,
		Config::kRenderTargetHeight,
		1,
		1,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT_D32_FLOAT
	);

	const std::string tkmBgFilePath = getTkmBgFilePath();
	const std::string tkmSampleFilePath = getTkmSampleFilePath();
	const std::string kFx3dFilePath = getFx3dFilePath();
	const std::string kFxPostEffectFilePath = getFxPostEffectFilePath();
	Dbg::assert_(std::filesystem::exists(tkmBgFilePath));
	Dbg::assert_(std::filesystem::exists(tkmSampleFilePath));
	Dbg::assert_(std::filesystem::exists(kFx3dFilePath));
	Dbg::assert_(std::filesystem::exists(kFxPostEffectFilePath));

	{
		SpriteInitData spriteInitData;
		spriteInitData.m_textures.at(0) = &m_offscreenRenderTarget.GetRenderTargetTexture();
		spriteInitData.m_width = Config::kRenderTargetWidth;
		spriteInitData.m_height = Config::kRenderTargetHeight;
		spriteInitData.m_fxFilePath = kFxPostEffectFilePath.c_str();

		std::unique_ptr<Sprite> monochromeSprite = std::make_unique<Sprite>();
		monochromeSprite->Init(spriteInitData);
		m_sprite = std::move(monochromeSprite);
	}

	ModelInitData initData = { };
	{
		initData.m_fxFilePath = kFx3dFilePath.c_str();
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

void Models_10_03::handleInput()
{
	m_plPos.x -= MiniEngineIf::getStick(MiniEngineIf::StickType::kLX);
	m_plPos.z -= MiniEngineIf::getStick(MiniEngineIf::StickType::kLY);

	m_models.at(static_cast<size_t>(ModelType::kPlayer))->UpdateWorldMatrix(m_plPos, g_quatIdentity, g_vec3One);
}

void Models_10_03::draw(RenderContext& renderContext)
{
	// render to offscreen buffer managed in this class
	RenderTarget* rtArray[] = { &m_offscreenRenderTarget };

	renderContext.WaitUntilToPossibleSetRenderTargets(1, rtArray);

	renderContext.SetRenderTargets(1, rtArray);
	renderContext.ClearRenderTargetViews(1, rtArray);
	m_models.at(static_cast<size_t>(ModelType::kBg))->Draw(renderContext);
	m_models.at(static_cast<size_t>(ModelType::kPlayer))->Draw(renderContext);

	renderContext.WaitUntilFinishDrawingToRenderTargets(1, rtArray);

	// render to offscreen buffer managed in MiniengineIf
	MiniEngineIf::setOffscreenRenderTarget();

	m_sprite->Draw(renderContext);
}

void Models_10_03::debugRenderParams()
{
	ImguiIf::printParams<float>(ImguiIf::VarType::kFloat, "Player", std::vector<const float*>{ &m_plPos.x, & m_plPos.y, & m_plPos.z });
}

namespace ModelHandler {
	std::unique_ptr<IModels> loadModelForChap10_03(RenderContext& renderContext)
	{
		ModelFactory_10_03 factory;
		std::unique_ptr<IModels> iModels = factory.create(renderContext);
		iModels->debugRenderParams();
		return std::move(iModels);
	}
} // namespace ModelHandler

