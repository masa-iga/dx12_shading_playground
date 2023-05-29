#include "model_11_06.h"
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
#include <../Sample_11_06/Sample_11_06/ModelStandard.h>
#pragma warning(3 : 4189)

class ModelFactory_11_06 : public IModelFactory
{
public:
	~ModelFactory_11_06() { }
	std::unique_ptr<IModels> create();
};

class Models_11_06 : public IModels
{
public:
	Models_11_06() {
		m_models.resize(static_cast<size_t>(ModelType::kSize));
		m_cascadeAreaTbl.at(2) = MiniEngineIf::getCamera3D()->GetFar();
	}
	~Models_11_06() { }
	void createModel();
	void resetCamera();
	void handleInput();
	void draw(RenderContext& renderContext);
	void debugRenderParams();

private:
	enum class ModelType {
		kSize,
	};

	static constexpr size_t kCascadeShadowMapNearestWidth = 2048;
	static constexpr size_t kCascadeShadowMapNearestHeight = 2048;
	static constexpr DXGI_FORMAT kShadowMapColorFormat = DXGI_FORMAT_R32_FLOAT;
	static constexpr DXGI_FORMAT kShadowMapDepthFormat = DXGI_FORMAT_D32_FLOAT;
#if 1
	static constexpr size_t kNumShadowMap = 3;
#endif
	const std::string kTkmTeapotFile = "Sample_11_06/Sample_11_06/Assets/modelData/testModel.tkm";
	const std::string kTkmBgFile = "Sample_11_06/Sample_11_06/Assets/modelData/bg/bg.tkm";
#if 1
	const std::string kTkmTestModelFile = "Sample_11_06/Sample_11_06/Assets/modelData/testModel.tkm";
#endif
	const std::string kFxDrawShadowMapFile = "Sample_11_06/Sample_11_06/Assets/shader/sampleDrawShadowMap.fx";
	const std::string kFxShadowReceiverFile = "./Assets/shader/sample_11_06_shadowReceiver.fx";
	const std::string kFxSpriteFile = "Sample_11_06/Sample_11_06/Assets/shader/preset/sprite.fx";
	std::string getTkmTeapotFilePath() { return ModelUtil::getPathFromAssetDir(kTkmTeapotFile); }
	std::string getTkmBgFilePath() { return ModelUtil::getPathFromAssetDir(kTkmBgFile); }
#if 1
	std::string getTkmTestModelFilePath() { return ModelUtil::getPathFromAssetDir(kTkmTestModelFile); }
#endif
	std::string getFxDrawShadowMapPath() { return ModelUtil::getPathFromAssetDir(kFxDrawShadowMapFile); };
	std::string getFxShadowReceiverPath() { return kFxShadowReceiverFile; }
	std::string getFxSpritePath() { return ModelUtil::getPathFromAssetDir(kFxSpriteFile); }

#if 1
	std::array<float, kNumShadowMap> m_cascadeAreaTbl = { 500, 2000, 0 };
#endif
	std::array<RenderTarget, 3> m_shadowMaps;
	Camera m_lightCamera;
	std::unique_ptr<Model> m_bg = nullptr;
	std::unique_ptr<ModelStandard> m_teapotModel = nullptr;
#if 1
	std::array<std::unique_ptr<Model>, kNumShadowMap> m_testShadowModels;
#endif
	std::unique_ptr<Sprite> m_spriteShadowMap = nullptr;
#if 1
	std::array<Matrix, kNumShadowMap> m_lvpcMatrices;
#endif
};

std::unique_ptr<IModels> ModelFactory_11_06::create()
{
	std::unique_ptr<Models_11_06> m = std::make_unique<Models_11_06>();
	{
		m->createModel();
	}
	return std::move(m);
}

void Models_11_06::resetCamera()
{
	MiniEngineIf::getCamera3D()->SetPosition(0.0f, 100.0f, 350.0f);
	MiniEngineIf::getCamera3D()->SetTarget(0.0f, 100.0f, 0.0f);
}

void Models_11_06::createModel()
{
	{
		float clearColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };

#if 1
		for (uint32_t i = 0; i < m_shadowMaps.size(); ++i)
		{
			const uint32_t div = (1 << i);

			m_shadowMaps.at(i).Create(
				kCascadeShadowMapNearestWidth / div,
				kCascadeShadowMapNearestHeight / div,
				1,
				1,
				kShadowMapColorFormat,
				kShadowMapDepthFormat,
				clearColor
			);
		}
#endif
	}

	{
		Vector3 toLigDir = { 1, 2, 1 };
		toLigDir.Normalize();
		toLigDir *= 5000.0f;
		m_lightCamera.SetPosition(toLigDir);
		m_lightCamera.SetTarget(0, 0, 0);
		m_lightCamera.SetUp(1, 0, 0);
		m_lightCamera.SetUpdateProjMatrixFunc(Camera::enUpdateProjMatrixFunc_Ortho);
		m_lightCamera.SetWidth(5000.0f);
		m_lightCamera.SetHeight(5000.0f);
		m_lightCamera.SetNear(1.0f);
		m_lightCamera.SetFar(10000.0f);

		m_lightCamera.Update();
	}

	const std::string tkmTeapotFilePath = getTkmTeapotFilePath();
	const std::string tkmBgFilePath = getTkmBgFilePath();
#if 1
	const std::string tkmTestModelFilePath = getTkmTestModelFilePath();
#endif
	const std::string fxDrawShadowMapPath = getFxDrawShadowMapPath();
	const std::string fxShadowReceiverPath = getFxShadowReceiverPath();
	const std::string fxSpritePath = getFxSpritePath();
	Dbg::assert_(std::filesystem::exists(tkmTeapotFilePath));
	Dbg::assert_(std::filesystem::exists(tkmBgFilePath));
	Dbg::assert_(std::filesystem::exists(fxDrawShadowMapPath));
	Dbg::assert_(std::filesystem::exists(fxShadowReceiverPath));
	Dbg::assert_(std::filesystem::exists(fxSpritePath));

#if 1
	{
		auto createShadowCaster = [&]()
		{
			ModelInitData d;
			{
				d.m_fxFilePath = fxDrawShadowMapPath.c_str();
				d.m_tkmFilePath = tkmTestModelFilePath.c_str();
				d.m_colorBufferFormat.at(0) = DXGI_FORMAT_R32_FLOAT;
			}
			std::unique_ptr<Model> m = std::make_unique<Model>();
			m->Init(d);
			return std::move(m);
		};

		for (auto& m : m_testShadowModels)
		{
			m = createShadowCaster();
		}
	}
#endif
	{
		ModelInitData d = { };
		{
			d.m_fxFilePath = fxShadowReceiverPath.c_str();
			d.m_tkmFilePath = tkmBgFilePath.c_str();
			d.m_expandShaderResoruceView.at(0) = &m_shadowMaps.at(0).GetRenderTargetTexture();
			d.m_expandShaderResoruceView.at(1) = &m_shadowMaps.at(1).GetRenderTargetTexture();
			d.m_expandShaderResoruceView.at(2) = &m_shadowMaps.at(2).GetRenderTargetTexture();
			d.m_expandConstantBuffer = m_lvpcMatrices.data();
			d.m_expandConstantBufferSize = sizeof(m_lvpcMatrices);
		}

		m_bg = std::make_unique<Model>();
		m_bg->Init(d);
	}
	{
		m_teapotModel = std::make_unique<ModelStandard>();
		m_teapotModel->Init(tkmTeapotFilePath.c_str());
	}
	{
		SpriteInitData d;
		{
			d.m_textures.at(0) = &m_shadowMaps.at(0).GetRenderTargetTexture();
			d.m_fxFilePath = fxSpritePath.c_str();
			d.m_width = 256;
			d.m_height = 256;
		}

		m_spriteShadowMap = std::make_unique<Sprite>();
		m_spriteShadowMap->Init(d);
	}
}

void Models_11_06::handleInput()
{
	if (!WinMgr::isWindowActive(WinMgr::Handle::kMain))
		return;

	// move camera
	{
		using namespace MiniEngineIf;
		Vector3 pos = getCamera3D()->GetPosition();
		Vector3 target = getCamera3D()->GetTarget();
		{
			pos.z -= getStick(StickType::kLY) * 2.0f;
			target.z -= getStick(StickType::kLY) * 2.0f;
			pos.y += getStick(StickType::kRY) * 2.0f;
			target.y += getStick(StickType::kRY) * 2.0f;
		}
		getCamera3D()->SetPosition(pos);
		getCamera3D()->SetTarget(target);
	}
}

void Models_11_06::draw(RenderContext& renderContext)
{
	// rander cascade shadow maps
	{
		using namespace MiniEngineIf;

		const Vector3& cameraForward = getCamera3D()->GetForward();
		const Vector3& cameraRight = getCamera3D()->GetRight();
		Vector3 cameraUp;
		cameraUp.Cross(cameraForward, cameraRight);

		float nearDepth = getCamera3D()->GetNear();

		for (uint32_t i = 0; i < kNumShadowMap; ++i)
		{
			const float nearY = std::tanf(getCamera3D()->GetViewAngle() * 0.5f) * nearDepth;
			const float nearX = nearY * getCamera3D()->GetAspect();
			const float farY = std::tanf(getCamera3D()->GetViewAngle() * 0.5f) * m_cascadeAreaTbl.at(i);
			const float farX = farY * getCamera3D()->GetAspect();
			const Vector3 nearPos = getCamera3D()->GetPosition() + cameraForward * nearDepth;
			const Vector3 farPos = getCamera3D()->GetPosition() + cameraForward * m_cascadeAreaTbl.at(i);

			Vector3 vertcies[8];
			vertcies[0] = nearPos + (cameraUp * nearY) + (cameraRight * nearX);
			vertcies[1] = nearPos + (cameraUp * nearY) - (cameraRight * nearX);
			vertcies[2] = nearPos - (cameraUp * nearY) + (cameraRight * nearX);
			vertcies[3] = nearPos - (cameraUp * nearY) - (cameraRight * nearX);
			vertcies[4] = farPos + (cameraUp * nearY) + (cameraRight * nearX);
			vertcies[5] = farPos + (cameraUp * nearY) - (cameraRight * nearX);
			vertcies[6] = farPos - (cameraUp * nearY) + (cameraRight * nearX);
			vertcies[7] = farPos - (cameraUp * nearY) - (cameraRight * nearX);

			// TODO:
		}
	}

	MiniEngineIf::setOffscreenRenderTarget();

	// draw models
	{
		m_teapotModel->Draw(renderContext);
	}

	// draw models which receive a shadow
	{
		m_bg->Draw(renderContext);
	}

#if 0
	{
		m_spriteShadowMap->Update(
			{ Config::kRenderTargetWidth / -2.0f, Config::kRenderTargetHeight / 2.0f, 0.0f },
			g_quatIdentity,
			g_vec3One,
			{ 0.0f, 1.0f }
		);
		m_spriteShadowMap->Draw(renderContext);
	}
#endif
}

void Models_11_06::debugRenderParams()
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
	std::unique_ptr<IModels> loadModelForChap11_06()
	{
		ModelFactory_11_06 factory;
		std::unique_ptr<IModels> iModels = factory.create();
		iModels->resetCamera();
		iModels->debugRenderParams();
		return std::move(iModels);
	}
} // namespace ModelHandler

