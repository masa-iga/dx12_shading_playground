#include "model_13_02.h"
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

class ModelFactory_13_02 : public IModelFactory
{
public:
	~ModelFactory_13_02() { }
	std::unique_ptr<IModels> create(RenderContext& renderContext);
};

class Obserber_13_02 : public WinMgr::Iobserber
{
public: 
	void update(WPARAM wParam, LPARAM lParam) override;
	bool isPaused() const { return m_paused; }

private:
	bool m_paused = false;
};

class Models_13_02 : public IModels
{
public:
	Models_13_02() { }
	~Models_13_02()
	{
		WinMgr::removeObserver(&m_obserber);
	}
	void createModel(RenderContext& renderContext);
	void addObserver();
	void removeObserver();
	void resetCamera();
	void handleInput();
	void draw(RenderContext& renderContext);
	void debugRenderParams();

private:
	struct DirectionalLight
	{
		Vector3 m_color;
		float m_pad0 = 0.0f;
		Vector3 m_direction;
		float m_pad1 = 0.0f;
		Vector3 m_eyePos;
		float m_pad2 = 0.0f;
	};

	static constexpr size_t kGbufferWidth = 1920;
	static constexpr size_t kGbufferHeight = 1080;

	const std::string kTkmSampleFile = "Sample_13_02/Sample_13_02/Assets/modelData/sample.tkm";
	std::string getTkmSampleFilePath() { return ModelUtil::getPathFromAssetDir(kTkmSampleFile); }
	const std::string kTkmBgFile = "Sample_13_02/Sample_13_02/Assets/modelData/bg/bg.tkm";
	std::string getTkmBgFilePath() { return ModelUtil::getPathFromAssetDir(kTkmBgFile); }
	const std::string kTkmSphereFile = "Sample_13_02/Sample_13_02/Assets/modelData/sphere.tkm";
	std::string getTkmSphereFilePath() { return ModelUtil::getPathFromAssetDir(kTkmSphereFile); }
	const std::string kFxModelFile = "./Assets/shader/sample_13_02_model.fx";
	std::string getFxModelFilePath() { return kFxModelFile; }
	const std::string kFxSpriteFile = "Sample_13_02/Sample_13_02/Assets/shader/sprite.fx";
	std::string getFxSpriteFilePath() { return ModelUtil::getPathFromAssetDir(kFxSpriteFile); }
	const std::string kFxPresetSpriteFile = "Sample_11_06/Sample_11_06/Assets/shader/preset/sprite.fx";
	std::string getFxPresetSpriteFilePath() { return ModelUtil::getPathFromAssetDir(kFxPresetSpriteFile); }

	Obserber_13_02 m_obserber;
	DirectionalLight m_light;
	Vector3 m_planePos = { 0.0f, 160.0f, 20.0f };
	std::unique_ptr<Model> m_modelHuman = nullptr;
	std::unique_ptr<Model> m_modelBg = nullptr;
	std::unique_ptr<Model> m_modelSphere = nullptr;
	RenderTarget m_albedRt;
	RenderTarget m_normalRt;
	RenderTarget m_worldPosRt;
	std::unique_ptr<Sprite> m_defferedLightingSprite = nullptr;
	std::array<std::unique_ptr<Sprite>, 3> m_debugSprites = { nullptr, nullptr, nullptr };
};

std::unique_ptr<IModels> ModelFactory_13_02::create(RenderContext& renderContext)
{
	std::unique_ptr<Models_13_02> m = std::make_unique<Models_13_02>();
	{
		m->createModel(renderContext);
		m->addObserver();
	}
	return std::move(m);
}

void Obserber_13_02::update(WPARAM wParam, [[maybe_unused]] LPARAM lParam)
{
	switch (wParam) {
	case VK_SPACE:
		m_paused = !m_paused;
		break;
	default:
		break;
	}
}

void Models_13_02::resetCamera()
{
	MiniEngineIf::getCamera3D()->SetPosition({ 0.0f, 80.0f, 200.0f });
	MiniEngineIf::getCamera3D()->SetTarget({ 0.0f, 80.0f, 0.0f });
	MiniEngineIf::getCamera3D()->Update();
}

void Models_13_02::createModel([[maybe_unused]] RenderContext& renderContext)
{
	// create Gbuffer
	{
		bool bRet = m_albedRt.Create(
			kGbufferWidth,
			kGbufferHeight,
			1 /* mipLevel */,
			1 /* arraySize */,
			DXGI_FORMAT_R8G8B8A8_UNORM,
			DXGI_FORMAT_D32_FLOAT);
		Dbg::assert_(bRet);
	}
	{
		bool bRet = m_normalRt.Create(
			kGbufferWidth,
			kGbufferHeight,
			1 /* mipLevel */,
			1 /* arraySize */,
			DXGI_FORMAT_R8G8B8A8_UNORM,
			DXGI_FORMAT_UNKNOWN);
		Dbg::assert_(bRet);
	}
	{
		bool bRet = m_worldPosRt.Create(
			kGbufferWidth,
			kGbufferHeight,
			1 /* mipLevel */,
			1 /* arraySize */,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			DXGI_FORMAT_UNKNOWN);
		Dbg::assert_(bRet);
	}

	{
		m_light.m_direction = { 1.0f, -1.0f, -1.0f };
		m_light.m_color = { 1.0f, 1.0f, 1.0f };
		m_light.m_eyePos = MiniEngineIf::getCamera3D()->GetPosition();
	}

	const std::string tkmSampleFilePath = getTkmSampleFilePath();
	Dbg::assert_(std::filesystem::exists(tkmSampleFilePath));
	const std::string tkmBgFilePath = getTkmBgFilePath();
	Dbg::assert_(std::filesystem::exists(tkmBgFilePath));
	const std::string tkmSphereFilePath = getTkmSphereFilePath();
	Dbg::assert_(std::filesystem::exists(tkmSphereFilePath));
	const std::string fxModelFilePath = getFxModelFilePath();
	Dbg::assert_(std::filesystem::exists(fxModelFilePath));
	const std::string fxSpriteFilePath = getFxSpriteFilePath();
	Dbg::assert_(std::filesystem::exists(fxSpriteFilePath));
	const std::string fxPresetSpriteFilePath = getFxPresetSpriteFilePath();
	Dbg::assert_(std::filesystem::exists(fxPresetSpriteFilePath));

	{
		ModelInitData d;
		d.m_tkmFilePath = tkmSampleFilePath.c_str();
		d.m_fxFilePath = fxModelFilePath.c_str();
		d.m_colorBufferFormat.at(0) = m_albedRt.GetColorBufferFormat();
		d.m_colorBufferFormat.at(1) = m_normalRt.GetColorBufferFormat();
		d.m_colorBufferFormat.at(2) = m_worldPosRt.GetColorBufferFormat();

		m_modelHuman = std::make_unique<Model>();
		m_modelHuman->Init(d);
		m_modelHuman->UpdateWorldMatrix({ 0.0f, 0.0f, 0.0f }, g_quatIdentity, g_vec3One);
	}
	{
		ModelInitData d;
		d.m_tkmFilePath = tkmBgFilePath.c_str();
		d.m_fxFilePath = fxModelFilePath.c_str();
		d.m_colorBufferFormat.at(0) = m_albedRt.GetColorBufferFormat();
		d.m_colorBufferFormat.at(1) = m_normalRt.GetColorBufferFormat();
		d.m_colorBufferFormat.at(2) = m_worldPosRt.GetColorBufferFormat();

		m_modelBg = std::make_unique<Model>();
		m_modelBg->Init(d);
	}
	{
		ModelInitData d;
		d.m_tkmFilePath = tkmSphereFilePath.c_str();
		d.m_fxFilePath = fxModelFilePath.c_str();
		d.m_expandConstantBuffer = &m_light;
		d.m_expandConstantBufferSize = sizeof(m_light);
		d.m_psEntryPointFunc = "PSMainTrans";

		m_modelSphere = std::make_unique<Model>();
		m_modelSphere->Init(d);
	}
	{
		SpriteInitData d;
		d.m_width = kGbufferWidth;
		d.m_height = kGbufferHeight;
		d.m_textures.at(0) = &m_albedRt.GetRenderTargetTexture();
		d.m_textures.at(1) = &m_normalRt.GetRenderTargetTexture();
		d.m_textures.at(2) = &m_worldPosRt.GetRenderTargetTexture();
		d.m_fxFilePath = fxSpriteFilePath.c_str();
		d.m_expandConstantBuffer = &m_light;
		d.m_expandConstantBufferSize = sizeof(m_light);

		m_defferedLightingSprite = std::make_unique<Sprite>();
		m_defferedLightingSprite->Init(d);
	}

	{
		SpriteInitData d;
		d.m_width = 256;
		d.m_height = 256;
		d.m_fxFilePath = fxPresetSpriteFilePath.c_str();

		d.m_textures.at(0) = &m_albedRt.GetRenderTargetTexture();
		m_debugSprites.at(0) = std::make_unique<Sprite>();
		m_debugSprites.at(0)->Init(d);

		d.m_textures.at(0) = &m_normalRt.GetRenderTargetTexture();
		m_debugSprites.at(1) = std::make_unique<Sprite>();
		m_debugSprites.at(1)->Init(d);

		d.m_textures.at(0) = &m_worldPosRt.GetRenderTargetTexture();
		m_debugSprites.at(2) = std::make_unique<Sprite>();
		m_debugSprites.at(2)->Init(d);
	}
}

void Models_13_02::addObserver()
{
	WinMgr::addObserver(&m_obserber);
}

void Models_13_02::removeObserver()
{
	WinMgr::removeObserver(&m_obserber);
}

void Models_13_02::handleInput()
{
	if (!WinMgr::isWindowActive(WinMgr::Handle::kMain))
		return;

	if (m_obserber.isPaused())
		return;

	using namespace MiniEngineIf;

	if (isPress(Button::kRight))
	{
		m_planePos.x -= 1.0f;
	}
	if (isPress(Button::kLeft))
	{
		m_planePos.x += 1.0f;
	}
	if (isPress(Button::kUp))
	{
		m_planePos.z -= 1.0f;
	}
	if (isPress(Button::kDown))
	{
		m_planePos.z += 1.0f;
	}

	m_modelSphere->UpdateWorldMatrix(m_planePos, g_quatIdentity, g_vec3One);
}

void Models_13_02::draw(RenderContext& renderContext)
{
	RenderTarget* rts[] = {
		&m_albedRt,
		&m_normalRt,
		&m_worldPosRt,
	};

	// deferred rendering: render to Gbuffer
	{
		renderContext.WaitUntilToPossibleSetRenderTargets(ARRAYSIZE(rts), rts);

		renderContext.SetRenderTargets(ARRAYSIZE(rts), rts);
		renderContext.ClearRenderTargetViews(ARRAYSIZE(rts), rts);

		m_modelHuman->Draw(renderContext);
		m_modelBg->Draw(renderContext);

		renderContext.WaitUntilFinishDrawingToRenderTargets(ARRAYSIZE(rts), rts);
	}

	// render to sprite
	{
		MiniEngineIf::setOffscreenRenderTarget();

		m_defferedLightingSprite->Draw(renderContext);
	}

	// forwared rendering: render translucent object
	{
		renderContext.SetRenderTarget(
			MiniEngineIf::getOffscreenRtvCpuDescHandle(),
			rts[0]->GetDSVCpuDescriptorHandle()
		);
		m_modelSphere->Draw(renderContext);
	}

	// render each Gbuffer for debugging
	for (uint32_t i = 0; i < m_debugSprites.size(); ++i)
	{
		const float posy = (i * (m_albedRt.GetHeight() / 4.0f)) + 1.0f * i;

		m_debugSprites.at(i)->Update(
			{ Config::kRenderTargetWidth / -2.0f, posy, 0.0f},
			g_quatIdentity,
			g_vec3One,
			{ 0.0f, 1.0f }
		);
		m_debugSprites.at(i)->Draw(renderContext);
	}
}

void Models_13_02::debugRenderParams()
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
			const Vector3& v = m_planePos;
			ImguiIf::printParams<float>(ImguiIf::VarType::kFloat, "PlanePos", std::vector<const float*>{ &v.x, &v.y, &v.z });
		}
		{
			const Vector3& v = m_light.m_color;
			ImguiIf::printParams<float>(ImguiIf::VarType::kFloat, "LightColor", std::vector<const float*>{ &v.x, &v.y, &v.z });
		}
		{
			const Vector3& v = m_light.m_direction;
			ImguiIf::printParams<float>(ImguiIf::VarType::kFloat, "LightDirection", std::vector<const float*>{ &v.x, &v.y, &v.z });
		}
	}
}

namespace ModelHandler {
	std::unique_ptr<IModels> loadModelForChap13_02(RenderContext& renderContext)
	{
		ModelFactory_13_02 factory;
		std::unique_ptr<IModels> iModels = factory.create(renderContext);
		iModels->resetCamera();
		iModels->debugRenderParams();
		return std::move(iModels);
	}
} // namespace ModelHandler

