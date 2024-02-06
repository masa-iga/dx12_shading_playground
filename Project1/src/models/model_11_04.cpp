#include "model_11_04.h"
#include <filesystem>
#include <memory>
#include "imodel.h"
#include "MiniEngine.h"
#include "model_util.h"
#include "../config.h"
#include "../debug_win.h"
#include "../imgui_if.h"
#include "../miniEngine_if.h"
#include <../Sample_11_04/Sample_11_04/ModelStandard.h>

class ModelFactory_11_04 : public IModelFactory
{
public:
	~ModelFactory_11_04() { }
	std::unique_ptr<IModels> create(RenderContext& renderContext);
};

class Models_11_04 : public IModels
{
public:
	Models_11_04() { m_models.resize(static_cast<size_t>(ModelType::kSize)); }
	~Models_11_04() { }
	void createModel(RenderContext& renderContext);
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
	static constexpr DXGI_FORMAT kShadowMapColorFormat = DXGI_FORMAT_R32_FLOAT;
	static constexpr DXGI_FORMAT kShadowMapDepthFormat = DXGI_FORMAT_D32_FLOAT;
	const std::string kTkmTeapotFile = "Sample_11_04/Sample_11_04/Assets/modelData/teapot.tkm";
	const std::string kTkmBgFile = "Sample_11_04/Sample_11_04/Assets/modelData/bg/bg.tkm";
	const std::string kFxDrawShadowMapFile = "Sample_11_04/Sample_11_04/Assets/shader/sampleDrawShadowMap.fx";
	const std::string kFxShadowReceiverFile = "./Assets/shader/sample_11_04_shadowReceiver.fx";
	std::string getTkmTeapotFilePath() { return ModelUtil::getPathFromAssetDir(kTkmTeapotFile); }
	std::string getTkmBgFilePath() { return ModelUtil::getPathFromAssetDir(kTkmBgFile); }
	std::string getFxDrawShadowMapPath() { return ModelUtil::getPathFromAssetDir(kFxDrawShadowMapFile); };
	std::string getFxShadowReceiverPath() { return kFxShadowReceiverFile; }

	RenderTarget m_shadowMap;
	std::unique_ptr<Model> m_bg = nullptr;
	std::unique_ptr<ModelStandard> m_teapotModel = nullptr;
	std::unique_ptr<Model> m_teapotShadowModel = nullptr;
	Camera m_lightCamera;
};

std::unique_ptr<IModels> ModelFactory_11_04::create(RenderContext& renderContext)
{
	std::unique_ptr<Models_11_04> m = std::make_unique<Models_11_04>();
	{
		m->createModel(renderContext);
	}
	return std::move(m);
}

void Models_11_04::resetCamera()
{
	MiniEngineIf::getCamera3D()->SetPosition(0, 50.0f, 350.0f);
	MiniEngineIf::getCamera3D()->SetTarget(0.0f, 0.0f, 250.0f);
}

void Models_11_04::createModel([[maybe_unused]] RenderContext& renderContext)
{
	{
		m_lightCamera.SetPosition(0, 500, 0);
		m_lightCamera.SetTarget(0, 0, 0);
		m_lightCamera.SetUp(1, 0, 0);
		m_lightCamera.Update();
	}

	{
		float clearColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };

		auto bRet = m_shadowMap.Create(
			kShadowMapWidth,
			kShadowMapHeight,
			1,
			1,
			kShadowMapColorFormat,
			kShadowMapDepthFormat,
			clearColor
		);
	}

	const std::string tkmTeapotFilePath = getTkmTeapotFilePath();
	const std::string tkmBgFilePath = getTkmBgFilePath();
	const std::string fxDrawShadowMapPath = getFxDrawShadowMapPath();
	const std::string fxShadowReceiverPath = getFxShadowReceiverPath();
	Dbg::assert_(std::filesystem::exists(tkmTeapotFilePath));
	Dbg::assert_(std::filesystem::exists(tkmBgFilePath));
	Dbg::assert_(std::filesystem::exists(fxDrawShadowMapPath));
	Dbg::assert_(std::filesystem::exists(fxShadowReceiverPath));

	{
		ModelInitData d = { };
		{
			d.m_fxFilePath = fxShadowReceiverPath.c_str();
			d.m_tkmFilePath = tkmBgFilePath.c_str();
			d.m_expandShaderResoruceView.at(0) = &m_shadowMap.GetRenderTargetTexture();
			d.m_expandConstantBuffer = (void*)&m_lightCamera.GetViewProjectionMatrix();
			d.m_expandConstantBufferSize = sizeof(m_lightCamera.GetViewProjectionMatrix());
		}

		m_bg = std::make_unique<Model>();
		m_bg->Init(d);
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
			d.m_colorBufferFormat.at(0) = m_shadowMap.GetColorBufferFormat();
		}
		m_teapotShadowModel = std::make_unique<Model>();
		m_teapotShadowModel->Init(d);
		m_teapotShadowModel->UpdateWorldMatrix({ 0, 50, 0 }, g_quatIdentity, g_vec3One);
	}
}

void Models_11_04::handleInput()
{
	// move camera
	{
		MiniEngineIf::getCamera3D()->MoveForward(MiniEngineIf::getStick(MiniEngineIf::StickType::kLY) * 3.0f);
		MiniEngineIf::getCamera3D()->MoveRight(MiniEngineIf::getStick(MiniEngineIf::StickType::kLX) * 3.0f);
		MiniEngineIf::getCamera3D()->MoveUp(MiniEngineIf::getStick(MiniEngineIf::StickType::kRY) * 3.0f);

		Quaternion quaternion = Quaternion::Identity;
		if (MiniEngineIf::getStick(MiniEngineIf::StickType::kRX) > 0.0f)
		{
			quaternion.SetRotationX(0.01f);
		}
		else if (MiniEngineIf::getStick(MiniEngineIf::StickType::kRX) < 0.0f)
		{
			quaternion.SetRotationX(-0.01f);
		}
		MiniEngineIf::getCamera3D()->RotateOriginTarget(quaternion);
	}
}

void Models_11_04::draw(RenderContext& renderContext)
{
	// render to shadow map
	{
		renderContext.WaitUntilToPossibleSetRenderTarget(m_shadowMap);
		renderContext.SetRenderTargetAndViewport(m_shadowMap);
		renderContext.ClearRenderTargetView(m_shadowMap);

		m_teapotShadowModel->Draw(renderContext, m_lightCamera);

		renderContext.WaitUntilFinishDrawingToRenderTarget(m_shadowMap);
	}

	MiniEngineIf::setOffscreenRenderTarget();

	// draw models
	{
		m_teapotModel->Draw(renderContext);

		// draw models which receive a shadow
		m_bg->Draw(renderContext);
	}
}

void Models_11_04::debugRenderParams()
{
	{
		const Vector3& pos = MiniEngineIf::getCamera3D()->GetPosition();
		ImguiIf::printParams<float>(ImguiIf::VarType::kFloat, "Camera", std::vector<const float*>{ &pos.x, & pos.y, & pos.z });
	}
}

namespace ModelHandler {
	std::unique_ptr<IModels> loadModelForChap11_04(RenderContext& renderContext)
	{
		ModelFactory_11_04 factory;
		std::unique_ptr<IModels> iModels = factory.create(renderContext);
		iModels->resetCamera();
		iModels->debugRenderParams();
		return std::move(iModels);
	}
} // namespace ModelHandler

