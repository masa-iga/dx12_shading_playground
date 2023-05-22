#include "model_11_05.h"
#include <filesystem>
#include <memory>
#include "imodel.h"
#include "MiniEngine.h"
#include "model_util.h"
#include "../config.h"
#include "../debug_win.h"
#include "../imgui_if.h"
#include "../miniEngine_if.h"
#include "../winmgr_win.h"
#include <../Sample_11_05/Sample_11_05/ModelStandard.h>

class ModelFactory_11_05 : public IModelFactory
{
public:
	~ModelFactory_11_05() { }
	std::unique_ptr<IModels> create();
};

class Models_11_05 : public IModels
{
public:
	Models_11_05() { m_models.resize(static_cast<size_t>(ModelType::kSize)); }
	~Models_11_05() { }
	void createModel();
	void resetCamera();
	void handleInput();
	void draw(RenderContext& renderContext);
	void debugRenderParams();

private:
	enum class ModelType {
		kSize,
	};

	struct ShadowParam
	{
		Matrix mLVP;
		Vector3 lightPos;
	};

	static constexpr size_t kShadowMapWidth = 2048;
	static constexpr size_t kShadowMapHeight = 2048;
	static constexpr DXGI_FORMAT kShadowMapColorFormat = DXGI_FORMAT_R32G32_FLOAT;
	static constexpr DXGI_FORMAT kShadowMapDepthFormat = DXGI_FORMAT_D32_FLOAT;
	const std::string kTkmTeapotFile = "Sample_11_05/Sample_11_05/Assets/modelData/teapot.tkm";
	const std::string kTkmBgFile = "Sample_11_05/Sample_11_05/Assets/modelData/bg/bg.tkm";
	const std::string kFxDrawShadowMapFile = "./Assets/shader/sample_11_05_drawShadowMap.fx";
	const std::string kFxShadowReceiverFile = "./Assets/shader/sample_11_05_shadowReceiver.fx";
	const std::string kFxSpriteFile = "Sample_11_05/Sample_11_05/Assets/shader/preset/sprite.fx";
	std::string getTkmTeapotFilePath() { return ModelUtil::getPathFromAssetDir(kTkmTeapotFile); }
	std::string getTkmBgFilePath() { return ModelUtil::getPathFromAssetDir(kTkmBgFile); }
	std::string getFxDrawShadowMapPath() { return kFxDrawShadowMapFile; };
	std::string getFxShadowReceiverPath() { return kFxShadowReceiverFile; }
	std::string getFxSpritePath() { return ModelUtil::getPathFromAssetDir(kFxSpriteFile); }

	RenderTarget m_shadowMap;
	Camera m_lightCamera;
	ShadowParam m_sp;
	std::unique_ptr<GaussianBlur> m_shadowBlur = nullptr;
	std::unique_ptr<Model> m_bg = nullptr;
	std::unique_ptr<ModelStandard> m_teapotModel = nullptr;
	std::unique_ptr<Model> m_teapotShadowModel = nullptr;
	std::unique_ptr<Sprite> m_spriteShadowMap = nullptr;
};

std::unique_ptr<IModels> ModelFactory_11_05::create()
{
	std::unique_ptr<Models_11_05> m = std::make_unique<Models_11_05>();
	{
		m->createModel();
	}
	return std::move(m);
}

void Models_11_05::resetCamera()
{
	MiniEngineIf::getCamera3D()->SetPosition(0.0f, 50.0f, 250.0f);
	MiniEngineIf::getCamera3D()->SetTarget(0.0f, 0.0f, 0.0f);
}

void Models_11_05::createModel()
{
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

	{
		m_lightCamera.SetPosition(0, 300, 0);
		m_lightCamera.SetTarget(0, 0, 0);
		m_lightCamera.SetUp(1, 0, 0);
		m_lightCamera.Update();
	}

	{
		memset(&m_sp, 0x0, sizeof(m_sp));
		m_sp.mLVP = m_lightCamera.GetViewProjectionMatrix();
		m_sp.lightPos.Set(m_lightCamera.GetPosition());
	}

	{
		m_shadowBlur = std::make_unique<GaussianBlur>();
		m_shadowBlur->Init(&m_shadowMap.GetRenderTargetTexture());
	}

	const std::string tkmTeapotFilePath = getTkmTeapotFilePath();
	const std::string tkmBgFilePath = getTkmBgFilePath();
	const std::string fxDrawShadowMapPath = getFxDrawShadowMapPath();
	const std::string fxShadowReceiverPath = getFxShadowReceiverPath();
	const std::string fxSpritePath = getFxSpritePath();
	Dbg::assert_(std::filesystem::exists(tkmTeapotFilePath));
	Dbg::assert_(std::filesystem::exists(tkmBgFilePath));
	Dbg::assert_(std::filesystem::exists(fxDrawShadowMapPath));
	Dbg::assert_(std::filesystem::exists(fxShadowReceiverPath));
	Dbg::assert_(std::filesystem::exists(fxSpritePath));

	{
		ModelInitData d = { };
		{
			d.m_fxFilePath = fxShadowReceiverPath.c_str();
			d.m_tkmFilePath = tkmBgFilePath.c_str();
			d.m_expandShaderResoruceView.at(0) = &m_shadowBlur->GetBokeTexture();
			d.m_expandConstantBuffer = reinterpret_cast<void*>(&m_sp);
			d.m_expandConstantBufferSize = sizeof(m_sp);
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
			d.m_expandConstantBuffer = reinterpret_cast<void*>(&m_sp);
			d.m_expandConstantBufferSize = sizeof(m_sp);
		}
		m_teapotShadowModel = std::make_unique<Model>();
		m_teapotShadowModel->Init(d);
		m_teapotShadowModel->UpdateWorldMatrix({ 0, 50, 0 }, g_quatIdentity, g_vec3One);
	}
	{
		SpriteInitData d;
		{
			d.m_textures.at(0) = &m_shadowMap.GetRenderTargetTexture();
			d.m_fxFilePath = fxSpritePath.c_str();
			d.m_width = 256;
			d.m_height = 256;
		}

		m_spriteShadowMap = std::make_unique<Sprite>();
		m_spriteShadowMap->Init(d);
	}
}

void Models_11_05::handleInput()
{
	if (!WinMgr::isWindowActive(WinMgr::Handle::kMain))
		return;

	// move camera
	{
		{
			Quaternion q = Quaternion::Identity;
			{
				q.SetRotationDegX(MiniEngineIf::getStick(MiniEngineIf::StickType::kRY) * 0.5f);
				MiniEngineIf::getCamera3D()->RotateOriginTarget(q);
			}

			{
				Vector3 pos = MiniEngineIf::getCamera3D()->GetPosition();
				Vector3 target = MiniEngineIf::getCamera3D()->GetTarget();
				pos.z -= MiniEngineIf::getStick(MiniEngineIf::StickType::kLY) * 0.5f;
				target.z -= MiniEngineIf::getStick(MiniEngineIf::StickType::kLY) * 0.5f;
				MiniEngineIf::getCamera3D()->SetPosition(pos);
				MiniEngineIf::getCamera3D()->SetTarget(target);
			}
		}
	}
}

void Models_11_05::draw(RenderContext& renderContext)
{
	// render to shadow map
	{
		renderContext.WaitUntilToPossibleSetRenderTarget(m_shadowMap);
		renderContext.SetRenderTargetAndViewport(m_shadowMap);
		renderContext.ClearRenderTargetView(m_shadowMap);

		m_teapotShadowModel->Draw(renderContext, m_lightCamera);

		renderContext.WaitUntilFinishDrawingToRenderTarget(m_shadowMap);
	}

	// apply Gaussian blur to shadow map
	{
		const float blurPower = 5.0f;
		m_shadowBlur->ExecuteOnGPU(renderContext, blurPower);
	}

	MiniEngineIf::setOffscreenRenderTarget();

	// draw models
	{
		m_teapotModel->Draw(renderContext);

		// draw models which receive a shadow
		m_bg->Draw(renderContext);
	}

	{
		m_spriteShadowMap->Update(
			{ Config::kRenderTargetWidth / -2.0f, Config::kRenderTargetHeight / 2.0f, 0.0f },
			g_quatIdentity,
			g_vec3One,
			{ 0.0f, 1.0f }
		);
		m_spriteShadowMap->Draw(renderContext);
	}
}

void Models_11_05::debugRenderParams()
{
	{
		{
			const Vector3& v = MiniEngineIf::getCamera3D()->GetPosition();
			ImguiIf::printParams<float>(ImguiIf::VarType::kFloat, "CameraPos", std::vector<const float*>{ &v.x, &v.y, &v.z });
		}
		{
			const Vector3& v = MiniEngineIf::getCamera3D()->GetTarget();
			ImguiIf::printParams<float>(ImguiIf::VarType::kFloat, "CameraTgt", std::vector<const float*>{ &v.x, &v.y, &v.z });
		}
		{
			const Vector3& v = MiniEngineIf::getCamera3D()->GetUp();
			ImguiIf::printParams<float>(ImguiIf::VarType::kFloat, "CameraUp", std::vector<const float*>{ &v.x, &v.y, &v.z });
		}
	}
}

namespace ModelHandler {
	std::unique_ptr<IModels> loadModelForChap11_05()
	{
		ModelFactory_11_05 factory;
		std::unique_ptr<IModels> iModels = factory.create();
		iModels->resetCamera();
		iModels->debugRenderParams();
		return std::move(iModels);
	}
} // namespace ModelHandler

