#include "model_12_01.h"
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

class ModelFactory_12_01 : public IModelFactory
{
public:
	~ModelFactory_12_01() { }
	std::unique_ptr<IModels> create();
};

class Models_12_01 : public IModels
{
public:
	Models_12_01() {
		m_models.resize(static_cast<size_t>(ModelType::kSize));
		m_cascadeAreaTbl.at(2) = MiniEngineIf::getCamera3D()->GetFar();
	}
	~Models_12_01() { }
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
	static constexpr size_t kNumShadowMap = 3;
	const std::string kTkmTeapotFile = "Sample_11_06/Sample_11_06/Assets/modelData/testModel.tkm";
	const std::string kTkmBgFile = "Sample_11_06/Sample_11_06/Assets/modelData/bg/bg.tkm";
	const std::string kTkmTestModelFile = "Sample_11_06/Sample_11_06/Assets/modelData/testModel.tkm";
	const std::string kFxDrawShadowMapFile = "Sample_11_06/Sample_11_06/Assets/shader/sampleDrawShadowMap.fx";
	const std::string kFxShadowReceiverFile = "./Assets/shader/sample_11_06_shadowReceiver.fx";
	const std::string kFxSpriteFile = "Sample_11_06/Sample_11_06/Assets/shader/preset/sprite.fx";
	std::string getTkmTeapotFilePath() { return ModelUtil::getPathFromAssetDir(kTkmTeapotFile); }
	std::string getTkmBgFilePath() { return ModelUtil::getPathFromAssetDir(kTkmBgFile); }
	std::string getTkmTestModelFilePath() { return ModelUtil::getPathFromAssetDir(kTkmTestModelFile); }
	std::string getFxDrawShadowMapPath() { return ModelUtil::getPathFromAssetDir(kFxDrawShadowMapFile); };
	std::string getFxShadowReceiverPath() { return kFxShadowReceiverFile; }
	std::string getFxSpritePath() { return ModelUtil::getPathFromAssetDir(kFxSpriteFile); }

	std::array<float, kNumShadowMap> m_cascadeAreaTbl = { 500, 2000, 0 };
	std::array<RenderTarget, 3> m_shadowMaps;
	Camera m_lightCamera;
	std::unique_ptr<Model> m_bg = nullptr;
	std::unique_ptr<ModelStandard> m_teapotModel = nullptr;
	std::array<std::unique_ptr<Model>, kNumShadowMap> m_testShadowModels;
	std::array<std::unique_ptr<Sprite>, kNumShadowMap> m_spriteShadowMaps;
	std::array<Matrix, kNumShadowMap> m_lvpcMatrices; // light view projection crop matrix
};

std::unique_ptr<IModels> ModelFactory_12_01::create()
{
	std::unique_ptr<Models_12_01> m = std::make_unique<Models_12_01>();
	{
		m->createModel();
	}
	return std::move(m);
}

void Models_12_01::resetCamera()
{
	MiniEngineIf::getCamera3D()->SetPosition(0.0f, 100.0f, 350.0f);
	MiniEngineIf::getCamera3D()->SetTarget(0.0f, 100.0f, 0.0f);
}

void Models_12_01::createModel()
{
	{
		float clearColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };

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
	const std::string tkmTestModelFilePath = getTkmTestModelFilePath();
	const std::string fxDrawShadowMapPath = getFxDrawShadowMapPath();
	const std::string fxShadowReceiverPath = getFxShadowReceiverPath();
	const std::string fxSpritePath = getFxSpritePath();
	Dbg::assert_(std::filesystem::exists(tkmTeapotFilePath));
	Dbg::assert_(std::filesystem::exists(tkmBgFilePath));
	Dbg::assert_(std::filesystem::exists(fxDrawShadowMapPath));
	Dbg::assert_(std::filesystem::exists(fxShadowReceiverPath));
	Dbg::assert_(std::filesystem::exists(fxSpritePath));

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
	for (uint32_t i = 0; i < kNumShadowMap; ++i)
	{
		SpriteInitData d;
		{
			d.m_textures.at(0) = &m_shadowMaps.at(i).GetRenderTargetTexture();
			d.m_fxFilePath = fxSpritePath.c_str();
			d.m_width = 256;
			d.m_height = 256;
		}

		m_spriteShadowMaps.at(i) = std::make_unique<Sprite>();
		m_spriteShadowMaps.at(i)->Init(d);
	}
}

void Models_12_01::handleInput()
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
			pos.x -= getStick(StickType::kLX) * 2.0f;
			target.x -= getStick(StickType::kLX) * 2.0f;
		}
		getCamera3D()->SetPosition(pos);
		getCamera3D()->SetTarget(target);
	}
}

void Models_12_01::draw(RenderContext& renderContext)
{
	// rander cascade shadow maps
	{
		using namespace MiniEngineIf;

		const Vector3& cameraForward = getCamera3D()->GetForward();
		const Vector3& cameraRight = getCamera3D()->GetRight();
		Vector3 cameraUp;
		cameraUp.Cross(cameraForward, cameraRight);

		float nearDepth = getCamera3D()->GetNear();

		for (uint32_t i = 0; i < m_lvpcMatrices.size(); ++i)
		{
			Vector3 vertcies[8];
			{
				const float nearY = std::tanf(getCamera3D()->GetViewAngle() * 0.5f) * nearDepth;
				const float nearX = nearY * getCamera3D()->GetAspect();
				const float farY = std::tanf(getCamera3D()->GetViewAngle() * 0.5f) * m_cascadeAreaTbl.at(i);
				const float farX = farY * getCamera3D()->GetAspect();
				const Vector3 nearPos = getCamera3D()->GetPosition() + cameraForward * nearDepth;
				const Vector3 farPos = getCamera3D()->GetPosition() + cameraForward * m_cascadeAreaTbl.at(i);

				vertcies[0] = nearPos + (cameraUp * nearY) + (cameraRight * nearX);
				vertcies[1] = nearPos + (cameraUp * nearY) - (cameraRight * nearX);
				vertcies[2] = nearPos - (cameraUp * nearY) + (cameraRight * nearX);
				vertcies[3] = nearPos - (cameraUp * nearY) - (cameraRight * nearX);
				vertcies[4] = farPos + (cameraUp * farY) + (cameraRight * farX);
				vertcies[5] = farPos + (cameraUp * farY) - (cameraRight * farX);
				vertcies[6] = farPos - (cameraUp * farY) + (cameraRight * farX);
				vertcies[7] = farPos - (cameraUp * farY) - (cameraRight * farX);
			}

			const Matrix& lvpMatrix = m_lightCamera.GetViewProjectionMatrix();
			Matrix clopMatrix;
			{
				Vector3 vMax = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
				Vector3 vMin = { FLT_MAX, FLT_MAX, FLT_MAX };

				for (Vector3& v : vertcies)
				{
					lvpMatrix.Apply(v);
					vMax.Max(v);
					vMin.Min(v);
				}

				const float xScale = 2.0f / (vMax.x - vMin.x);
				const float yScale = 2.0f / (vMax.y - vMin.y);
				const float xOffset = (vMax.x + vMin.x) * -0.5f * xScale;
				const float yOffset = (vMax.y + vMin.y) * -0.5f * yScale;

				clopMatrix.m[0][0] = xScale;
				clopMatrix.m[1][1] = yScale;
				clopMatrix.m[3][0] = xOffset;
				clopMatrix.m[3][1] = yOffset;
			}

			m_lvpcMatrices.at(i) = lvpMatrix * clopMatrix;

			nearDepth = m_cascadeAreaTbl.at(i);
		}
	}

	// render to shadow map
	for (int32_t i = 0; i < m_testShadowModels.size(); ++i)
	{
		renderContext.WaitUntilToPossibleSetRenderTarget(m_shadowMaps.at(i));
		renderContext.SetRenderTargetAndViewport(m_shadowMaps.at(i));
		renderContext.ClearRenderTargetView(m_shadowMaps.at(i));

		m_testShadowModels.at(i)->Draw(renderContext, g_matIdentity, m_lvpcMatrices.at(i));

		renderContext.WaitUntilFinishDrawingToRenderTarget(m_shadowMaps.at(i));
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

	// render cascade shadow maps for debugging
	for (uint32_t i = 0; i < m_spriteShadowMaps.size(); ++i)
	{
		const float posx = (Config::kRenderTargetWidth / -2.0f) + (i * (m_shadowMaps.at(1).GetWidth() / 4.0f)) + 1.0f * i;

		m_spriteShadowMaps.at(i)->Update(
			{ posx, Config::kRenderTargetHeight / 2.0f, 0.0f},
			g_quatIdentity,
			g_vec3One,
			{ 0.0f, 1.0f }
		);
		m_spriteShadowMaps.at(i)->Draw(renderContext);
	}
}

void Models_12_01::debugRenderParams()
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
		{
			const Vector3& v = m_lightCamera.GetPosition();
			ImguiIf::printParams<float>(ImguiIf::VarType::kFloat, "LightCameraPos", std::vector<const float*>{ &v.x, &v.y, &v.z });
		}
		{
			const Vector3& v = m_lightCamera.GetTarget();
			ImguiIf::printParams<float>(ImguiIf::VarType::kFloat, "LightCameraTgt", std::vector<const float*>{ &v.x, &v.y, &v.z });
		}
		{
			const Vector3& v = m_lightCamera.GetUp();
			ImguiIf::printParams<float>(ImguiIf::VarType::kFloat, "LightCameraUp", std::vector<const float*>{ &v.x, &v.y, &v.z });
		}
	}
}

namespace ModelHandler {
	std::unique_ptr<IModels> loadModelForChap12_01()
	{
		ModelFactory_12_01 factory;
		std::unique_ptr<IModels> iModels = factory.create();
		iModels->resetCamera();
		iModels->debugRenderParams();
		return std::move(iModels);
	}
} // namespace ModelHandler

