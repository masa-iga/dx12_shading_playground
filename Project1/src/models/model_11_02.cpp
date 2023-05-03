#include "model_11_02.h"
#include <filesystem>
#include <memory>
#include "imodel.h"
#include "MiniEngine.h"
#include "model_util.h"
#include "../config.h"
#include "../debug_win.h"
#include "../imgui_if.h"
#include "../miniEngine_if.h"
#include <../Sample_11_01/Sample_11_01/ModelStandard.h>

class ModelFactory_11_02 : public IModelFactory
{
public:
	~ModelFactory_11_02() { }
	std::unique_ptr<IModels> create();
};

class Models_11_02 : public IModels
{
public:
	Models_11_02() { m_models.resize(static_cast<size_t>(ModelType::kSize)); }
	~Models_11_02() { }
	void createModel();
	void resetCamera();
	void handleInput();
	void draw(RenderContext& renderContext);
	void debugRenderParams();

private:
	enum class ModelType {
		kSize,
	};

	static constexpr size_t kShadowMapWidth = 1024;
	static constexpr size_t kShadowMapHeight = 1024;
	static constexpr DXGI_FORMAT kColorBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	static constexpr DXGI_FORMAT kDepthBufferFormat = DXGI_FORMAT_D32_FLOAT;
	const std::string kTkmTeapotFile = "Sample_11_02/Sample_11_02/Assets/modelData/teapot.tkm";
	const std::string kTkmBgFile = "Sample_11_02/Sample_11_02/Assets/modelData/bg/bg.tkm";
	const std::string kFxSpriteFile = "Sample_11_02/Sample_11_02/Assets/shader/preset/sprite.fx";
	const std::string kFxDrawShadowMapFile = "Sample_11_02/Sample_11_02/Assets/shader/sampleDrawShadowMap.fx";
	std::string getTkmTeapotFilePath() { return ModelUtil::getPathFromAssetDir(kTkmTeapotFile); }
	std::string getTkmBgFilePath() { return ModelUtil::getPathFromAssetDir(kTkmBgFile); }
	std::string getFxSpritePath() { return ModelUtil::getPathFromAssetDir(kFxSpriteFile); }
	std::string getFxDrawShadowMapPath() { return ModelUtil::getPathFromAssetDir(kFxDrawShadowMapFile); };

	RenderTarget m_shadowMap;
	std::unique_ptr<Sprite> m_sprite = nullptr;
	std::unique_ptr<ModelStandard> m_bg = nullptr;
	std::unique_ptr<ModelStandard> m_teapotModel = nullptr;
	std::unique_ptr<Model> m_teapotShadowModel = nullptr;
	Camera m_lightCamera;
};

std::unique_ptr<IModels> ModelFactory_11_02::create()
{
	std::unique_ptr<Models_11_02> m(new Models_11_02);
	{
		m->createModel();
	}
	return std::move(m);
}

void Models_11_02::resetCamera()
{
	;
}

void Models_11_02::createModel()
{
	{
		float clearColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };

		auto bRet = m_shadowMap.Create(
			kShadowMapWidth,
			kShadowMapHeight,
			1,
			1,
			kColorBufferFormat,
			kDepthBufferFormat,
			clearColor
		);
	}

	const std::string tkmTeapotFilePath = getTkmTeapotFilePath();
	const std::string tkmBgFilePath = getTkmBgFilePath();
	const std::string fxSpriteFilePath = getFxSpritePath();
	const std::string fxDrawShadowMapPath = getFxDrawShadowMapPath();
	Dbg::assert_(std::filesystem::exists(tkmTeapotFilePath));
	Dbg::assert_(std::filesystem::exists(tkmBgFilePath));
	Dbg::assert_(std::filesystem::exists(fxSpriteFilePath));
	Dbg::assert_(std::filesystem::exists(fxDrawShadowMapPath));

	{
		m_bg = std::make_unique<ModelStandard>();
		m_bg->Init(tkmBgFilePath.c_str());
	}
	{
		SpriteInitData d;
		{
			d.m_textures.at(0) = &m_shadowMap.GetRenderTargetTexture();
			d.m_fxFilePath = fxSpriteFilePath.c_str();
			d.m_width = 256;
			d.m_height = 256;
		}
		m_sprite = std::make_unique<Sprite>();
		m_sprite->Init(d);
	}
	{
		m_teapotModel = std::make_unique<ModelStandard>();
		m_teapotModel->Init(tkmTeapotFilePath.c_str());
		m_teapotModel->Update({ 0, 50, 0 }, g_quatIdentity, g_vec3One);
	}
	{
		ModelInitData d = { };
		{
			d.m_fxFilePath = fxDrawShadowMapPath.c_str();
			d.m_tkmFilePath = tkmTeapotFilePath.c_str();
		}
		m_teapotShadowModel = std::make_unique<Model>();
		m_teapotShadowModel->Init(d);
		m_teapotShadowModel->UpdateWorldMatrix({ 0, 50, 0 }, g_quatIdentity, g_vec3One);
	}

	{
		m_lightCamera.SetPosition(0, 600, 0);
		m_lightCamera.SetTarget(0, 0, 0);
		m_lightCamera.SetUp(1, 0, 0);
		m_lightCamera.SetViewAngle(Math::DegToRad(20.0f));
		m_lightCamera.Update();
	}
}

void Models_11_02::handleInput()
{
	// move camera
	{
		MiniEngineIf::getCamera3D()->MoveForward(MiniEngineIf::getStick(MiniEngineIf::StickType::kLY) * 3.0f);
		MiniEngineIf::getCamera3D()->MoveRight(MiniEngineIf::getStick(MiniEngineIf::StickType::kLX) * 3.0f);
	}
}

void Models_11_02::draw(RenderContext& renderContext)
{
	// render to shadow map
	{
		RenderTarget& r = m_shadowMap;
		renderContext.WaitUntilToPossibleSetRenderTarget(m_shadowMap);
		renderContext.SetRenderTargetAndViewport(m_shadowMap);
		renderContext.ClearRenderTargetView(m_shadowMap);

		m_teapotShadowModel->Draw(renderContext, m_lightCamera);

		renderContext.WaitUntilFinishDrawingToRenderTarget(m_shadowMap);
	}

	// draw models
	{
		MiniEngineIf::setOffscreenRenderTarget();

		m_bg->Draw(renderContext);
		m_teapotModel->Draw(renderContext);
	}

	// draw shadow map to the current binding render target
	{
		m_sprite->Update(
			{ Config::kRenderTargetWidth / -2.0f, Config::kRenderTargetHeight / 2.0f, 0.0f },
			g_quatIdentity,
			g_vec3One,
			{ 0.0f, 1.0f }
		);
		m_sprite->Draw(renderContext);
	}
}

void Models_11_02::debugRenderParams()
{
	{
		const Vector3& pos = MiniEngineIf::getCamera3D()->GetPosition();
		ImguiIf::printParams<float>(ImguiIf::VarType::kFloat, "Camera", std::vector<const float*>{ &pos.x, & pos.y, & pos.z });
	}
}

namespace ModelHandler {
	std::unique_ptr<IModels> loadModelForChap11_02()
	{
		ModelFactory_11_02 factory;
		std::unique_ptr<IModels> iModels = factory.create();
		iModels->debugRenderParams();
		return std::move(iModels);
	}
} // namespace ModelHandler

