#include "model_16_03.h"
#include <filesystem>
#include <memory>
#include <random>
#include "imodel.h"
#include "MiniEngine.h"
#include "model_util.h"
#include "../config.h"
#include "../debug_win.h"
#include "../imgui_if.h"
#include "../miniEngine_if.h"
#include "../winmgr_win.h"
#pragma warning(3 : 4189)

class ModelFactory_16_03 : public IModelFactory
{
public:
	~ModelFactory_16_03() { }
	std::unique_ptr<IModels> create();
};

class Obserber_16_03 : public WinMgr::Iobserber
{
public: 
	void update(WPARAM wParam, LPARAM lParam) override;
	bool isPaused() const { return m_paused; }

private:
	bool m_paused = false;
};

class Models_16_03 : public IModels
{
public:
	Models_16_03() { }
	~Models_16_03()
	{
		WinMgr::removeObserver(&m_obserber);
	}
	void createModel();
	void addObserver();
	void removeObserver();
	void resetCamera();
	void handleInput();
	void draw(RenderContext& renderContext);
	void debugRenderParams();

private:
	static constexpr size_t kNumDirectionLight = 4;
	static constexpr size_t kNumPointLight = 1000;
	static constexpr int32_t kRtWidth = 1280;
	static constexpr int32_t kRtHeight = 720;

	struct alignas(16) DirectionalLight
	{
		Vector3 m_color;
		float m_pad0 = 0.0f;
		Vector3 m_direction;
		float m_pad1 = 0.0f;
	};

	struct alignas(16) PointLight
	{
		Vector3 m_position;
		float m_pad0 = 0.0f;
		Vector3 m_color;
		float m_range = 0.0f;
	};

	struct Light
	{
		std::array<DirectionalLight, kNumDirectionLight> m_directionLights;
		std::array<PointLight, kNumPointLight> m_pointLights;
		Matrix m_mViewProjInv;
		Vector3 m_eyePos;
		float m_specRow = 0.0f;
	};

	const std::string kTkmBgFile = "Sample_16_03/Sample_16_03/Assets/modelData/bg.tkm";
	std::string getTkmBgFilePath() const { return ModelUtil::getPathFromAssetDir(kTkmBgFile); }
	const std::string kTkmTeapotFile = "Sample_16_03/Sample_16_03/Assets/modelData/teapot.tkm";
	std::string getTkmTeapotFilePath() const { return ModelUtil::getPathFromAssetDir(kTkmTeapotFile); }
	const std::string kFxRenderGBuffer = "./Assets/shader/sample_16_02_renderGBuffer.fx";
	std::string getFxRenderGBufferPath() const { return kFxRenderGBuffer; };
	const std::string kFxDefferedLightingFile = "./Assets/shader/sample_16_02_defferedLighting.fx";
	std::string getFxDefferedLightingFilePath() const { return kFxDefferedLightingFile; }

	Obserber_16_03 m_obserber;
	std::unique_ptr<Model> m_modelTeapot = nullptr;
	std::unique_ptr<Model> m_modelBg = nullptr;
	std::unique_ptr<Light> m_light = nullptr;
	RenderTarget m_albedoRenderTarget;
	RenderTarget m_normalRenderTarget;
	RenderTarget m_depthRenderTarget;
	std::unique_ptr<Sprite> m_defferedLightingSprite = nullptr;
};

std::unique_ptr<IModels> ModelFactory_16_03::create()
{
	std::unique_ptr<Models_16_03> m = std::make_unique<Models_16_03>();
	{
		m->createModel();
		m->addObserver();
	}
	return std::move(m);
}

void Obserber_16_03::update(WPARAM wParam, [[maybe_unused]] LPARAM lParam)
{
	switch (wParam) {
	case VK_SPACE:
		m_paused = !m_paused;
		break;
	default:
		break;
	}
}

void Models_16_03::resetCamera()
{
	MiniEngineIf::getCamera3D()->SetPosition({ 0.0f, 200.0f, 400.0f });
	MiniEngineIf::getCamera3D()->Update();
}

void Models_16_03::createModel()
{
	{
		constexpr int32_t mipLevel = 1;
		constexpr int32_t arraySize = 1;

		bool bRet = false;

		bRet = m_albedoRenderTarget.Create(
			kRtWidth,
			kRtHeight,
			mipLevel,
			arraySize,
			DXGI_FORMAT_R8G8B8A8_UNORM,
			DXGI_FORMAT_D32_FLOAT
		);
		Dbg::assert_(bRet);

		bRet = m_normalRenderTarget.Create(
			kRtWidth,
			kRtHeight,
			mipLevel,
			arraySize,
			DXGI_FORMAT_R16G16B16A16_FLOAT,
			DXGI_FORMAT_UNKNOWN
		);
		Dbg::assert_(bRet);

		bRet = m_depthRenderTarget.Create(
			kRtWidth,
			kRtHeight,
			mipLevel,
			arraySize,
			DXGI_FORMAT_R32_FLOAT,
			DXGI_FORMAT_UNKNOWN
		);
		Dbg::assert_(bRet);
	}

	m_light = std::make_unique<Light>();
	{
		resetCamera();
		m_light->m_eyePos = MiniEngineIf::getCamera3D()->GetPosition();
		m_light->m_specRow = 5.0f;
		m_light->m_mViewProjInv.Inverse(MiniEngineIf::getCamera3D()->GetViewProjectionMatrix());
	}
	{
		m_light->m_directionLights.at(0).m_direction.Set(1.0f, 0.0f, 0.0f);
		m_light->m_directionLights.at(0).m_color.Set(0.5f, 0.5f, 0.5f);
		m_light->m_directionLights.at(1).m_direction.Set(-1.0f, 0.0f, 0.0f);
		m_light->m_directionLights.at(1).m_color.Set(0.5f, 0.0f, 0.0f);
		m_light->m_directionLights.at(2).m_direction.Set(0.0f, 0.0f, 1.0f);
		m_light->m_directionLights.at(2).m_color.Set(0.0f, 0.5f, 0.0f);
		m_light->m_directionLights.at(3).m_direction.Set(0.0f, -1.0f, 0.0f);
		m_light->m_directionLights.at(3).m_color.Set(0.0f, 0.0f, 0.5f);
	}
	{
		std::random_device seed_gen;
		std::mt19937 random(seed_gen());

		for (auto& pt : m_light->m_pointLights)
		{
			pt.m_position = {
				static_cast<float>(random() % 1000) - 500.0f,
				20.0f,
				static_cast<float>(random() % 1000) - 500.0f
			};
			pt.m_range = 50.0f;
			pt.m_color = {
				static_cast<float>(random() % 255) / 255.0f,
				static_cast<float>(random() % 255) / 255.0f,
				static_cast<float>(random() % 255) / 255.0f
			};
		}
	}

	const std::string tkmBgFilePath = getTkmBgFilePath();
	Dbg::assert_(std::filesystem::exists(tkmBgFilePath));
	const std::string tkmTeapotFilePath = getTkmTeapotFilePath();
	Dbg::assert_(std::filesystem::exists(tkmTeapotFilePath));
	const std::string fxRenderGBufferPath = getFxRenderGBufferPath();
	Dbg::assert_(std::filesystem::exists(fxRenderGBufferPath));
	const std::string fxDefferedLightingFilePath = getFxDefferedLightingFilePath();
	Dbg::assert_(std::filesystem::exists(fxDefferedLightingFilePath));

	{
		ModelInitData d = { };
		{
			d.m_tkmFilePath = tkmTeapotFilePath.c_str();
			d.m_fxFilePath = fxRenderGBufferPath.c_str();
			d.m_colorBufferFormat.at(0) = m_albedoRenderTarget.GetColorBufferFormat();
			d.m_colorBufferFormat.at(1) = m_normalRenderTarget.GetColorBufferFormat();
			d.m_colorBufferFormat.at(2) = m_depthRenderTarget.GetColorBufferFormat();
		}
		m_modelTeapot = std::make_unique<Model>();
		m_modelTeapot->Init(d);
	}

	{
		ModelInitData d = { };
		{
			d.m_tkmFilePath = tkmBgFilePath.c_str();
			d.m_fxFilePath = fxRenderGBufferPath.c_str();
			d.m_colorBufferFormat.at(0) = m_albedoRenderTarget.GetColorBufferFormat();
			d.m_colorBufferFormat.at(1) = m_normalRenderTarget.GetColorBufferFormat();
			d.m_colorBufferFormat.at(2) = m_depthRenderTarget.GetColorBufferFormat();
		}
		m_modelBg = std::make_unique<Model>();
		m_modelBg->Init(d);
	}

	{
		SpriteInitData d;
		{
			d.m_width = Config::kRenderTargetWidth;
			d.m_height = Config::kRenderTargetHeight;
			d.m_textures.at(0) = &m_albedoRenderTarget.GetRenderTargetTexture();
			d.m_textures.at(1) = &m_normalRenderTarget.GetRenderTargetTexture();
			d.m_textures.at(2) = &m_depthRenderTarget.GetRenderTargetTexture();
			d.m_fxFilePath = fxDefferedLightingFilePath.c_str();
			d.m_expandConstantBuffer = m_light.get();
			d.m_expandConstantBufferSize = sizeof(*m_light);
		}
		m_defferedLightingSprite = std::make_unique<Sprite>();
		m_defferedLightingSprite->Init(d);
	}
}

void Models_16_03::addObserver()
{
	WinMgr::addObserver(&m_obserber);
}

void Models_16_03::removeObserver()
{
	WinMgr::removeObserver(&m_obserber);
}

void Models_16_03::handleInput()
{
	if (WinMgr::isWindowActive(WinMgr::Handle::kMain))
	{
		using namespace MiniEngineIf;
		constexpr float kAffect = 5.0f;
		getCamera3D()->MoveForward(getStick(StickType::kLY) * kAffect);
		getCamera3D()->MoveRight(getStick(StickType::kLX) * kAffect);
		getCamera3D()->MoveUp(getStick(StickType::kRY) * kAffect);
		getCamera3D()->Update();
		m_light->m_eyePos = getCamera3D()->GetPosition();
		m_light->m_mViewProjInv.Inverse(getCamera3D()->GetViewProjectionMatrix());
	}

	if (!m_obserber.isPaused())
	{
		Quaternion qRot;
		qRot.SetRotationDegY(1.0f);

		for (auto& pt : m_light->m_pointLights)
		{
			qRot.Apply(pt.m_position);
		}
	}
}

void Models_16_03::draw(RenderContext& renderContext)
{
	RenderTarget* gbuffers[] = {
		&m_albedoRenderTarget,
		&m_normalRenderTarget,
		&m_depthRenderTarget
	};

	renderContext.WaitUntilToPossibleSetRenderTargets(ARRAYSIZE(gbuffers), gbuffers);

	renderContext.SetRenderTargets(ARRAYSIZE(gbuffers), gbuffers);
	// set viewport; should be same to Gbuffer size
	{
		CD3DX12_VIEWPORT vp(0.0f, 0.0f, kRtWidth, kRtHeight);
		renderContext.SetViewportAndScissor(vp);
	}
	renderContext.ClearRenderTargetViews(ARRAYSIZE(gbuffers), gbuffers);

	m_modelTeapot->Draw(renderContext);
	m_modelBg->Draw(renderContext);

	renderContext.WaitUntilFinishDrawingToRenderTargets(ARRAYSIZE(gbuffers), gbuffers);

	MiniEngineIf::setOffscreenRenderTarget();

	m_defferedLightingSprite->Draw(renderContext);
}

void Models_16_03::debugRenderParams()
{
	{
		const Vector3& v = MiniEngineIf::getCamera3D()->GetPosition();
		ImguiIf::printParams<float>(ImguiIf::VarType::kFloat, "CameraPos", std::vector<const float*>{ &v.x, & v.y, & v.z });
	}
	{
		const Vector3& v = MiniEngineIf::getCamera3D()->GetTarget();
		ImguiIf::printParams<float>(ImguiIf::VarType::kFloat, "CameraTgt", std::vector<const float*>{ &v.x, & v.y, & v.z });
	}
	{
		const Vector3& v = MiniEngineIf::getCamera3D()->GetUp();
		ImguiIf::printParams<float>(ImguiIf::VarType::kFloat, "CameraUp", std::vector<const float*>{ &v.x, & v.y, & v.z });
	}
}

namespace ModelHandler {
	std::unique_ptr<IModels> loadModelForChap16_03()
	{
		ModelFactory_16_03 factory;
		std::unique_ptr<IModels> iModels = factory.create();
		iModels->resetCamera();
		iModels->debugRenderParams();
		return std::move(iModels);
	}
} // namespace ModelHandler

