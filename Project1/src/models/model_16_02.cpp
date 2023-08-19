#include "model_16_02.h"
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

class ModelFactory_16_02 : public IModelFactory
{
public:
	~ModelFactory_16_02() { }
	std::unique_ptr<IModels> create();
};

class Obserber_16_02 : public WinMgr::Iobserber
{
public: 
	void update(WPARAM wParam, LPARAM lParam) override;
	bool isPaused() const { return m_paused; }

private:
	bool m_paused = false;
};

class Models_16_02 : public IModels
{
public:
	Models_16_02() { }
	~Models_16_02()
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
#if 1
	static constexpr size_t kNumDirectionLight = 4;
	static constexpr size_t kNumPointLight = 1000;
	static constexpr int32_t kRtWidth = 1280;
	static constexpr int32_t kRtHeight = 720;

	struct alignas(16) DirectionalLight
	{
		Vector3 m_color;
		float m_pad0 = 0.0f;
		Vector3 m_direction;
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
#endif

	struct SPointLight
	{
		Vector3 m_position;
		float m_pad0 = 0.0f;
		Vector3 m_color;
		float m_pad1 = 0.0f;
		float m_range = 0.0f;
		float m_pad2[3] = { 0.0f, 0.0f, 0.0f };
	};
	static_assert(sizeof(SPointLight) % 16 == 0);

	const std::string kTkmBgFile = "Sample_16_01/Sample_16_01/Assets/modelData/bg.tkm";
	std::string getTkmBgFilePath() { return ModelUtil::getPathFromAssetDir(kTkmBgFile); }
	const std::string kTkmTeapotFile = "Sample_16_01/Sample_16_01/Assets/modelData/teapot.tkm";
	std::string getTkmTeapotFilePath() { return ModelUtil::getPathFromAssetDir(kTkmTeapotFile); }
	const std::string kFxModelFile = "./Assets/shader/sample_16_01_model.fx";
	std::string getFxModelFilePath() { return kFxModelFile; }
#if 1
	const std::string kFxDefferedLightingFile = "./Assets/shader/sample_16_02_defferedLighting.fx";
	std::string getFxDefferedLightingFilePath() { return kFxDefferedLightingFile; }
#endif
	Obserber_16_02 m_obserber;
	std::unique_ptr<Model> m_modelTeapot = nullptr;
	std::unique_ptr<Model> m_modelBg = nullptr;
	std::array<SPointLight, kNumPointLight> m_pointLights;

#if 1
	std::unique_ptr<Light> m_light = nullptr;
	RenderTarget m_albedoRenderTarget;
	RenderTarget m_normalRenderTarget;
	std::unique_ptr<Sprite> m_defferedLightingSprite = nullptr;
#endif
};

std::unique_ptr<IModels> ModelFactory_16_02::create()
{
	std::unique_ptr<Models_16_02> m = std::make_unique<Models_16_02>();
	{
		m->createModel();
		m->addObserver();
	}
	return std::move(m);
}

void Obserber_16_02::update(WPARAM wParam, [[maybe_unused]] LPARAM lParam)
{
	switch (wParam) {
	case VK_SPACE:
		m_paused = !m_paused;
		break;
	default:
		break;
	}
}

void Models_16_02::resetCamera()
{
	MiniEngineIf::getCamera3D()->SetPosition({ 0.0f, 200.0f, 400.0f });
}

void Models_16_02::createModel()
{
#if 1
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
	}
#endif
	{
		std::random_device seed_gen;
		std::mt19937 random(seed_gen());

		for (auto& pt : m_pointLights)
		{
			pt.m_position.x = static_cast<float>(random() % 1000) - 500.0f;
			pt.m_position.y = 20.0f;
			pt.m_position.z = static_cast<float>(random() % 1000) - 500.0f;
			pt.m_range = 50.0f;
			pt.m_color.x = static_cast<float>(random() % 255) / 255.0f;
			pt.m_color.y = static_cast<float>(random() % 255) / 255.0f;
			pt.m_color.z = static_cast<float>(random() % 255) / 255.0f;
		}
	}
#if 1
	m_light = std::make_unique<Light>();
	{
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
#endif

	const std::string tkmBgFilePath = getTkmBgFilePath();
	Dbg::assert_(std::filesystem::exists(tkmBgFilePath));
	const std::string tkmTeapotFilePath = getTkmTeapotFilePath();
	Dbg::assert_(std::filesystem::exists(tkmTeapotFilePath));
	const std::string fxModelFilePath = getFxModelFilePath();
	Dbg::assert_(std::filesystem::exists(fxModelFilePath));
#if 1
	const std::string fxDefferedLightingFilePath = getFxDefferedLightingFilePath();
	Dbg::assert_(std::filesystem::exists(fxDefferedLightingFilePath));
#endif

	{
		ModelInitData d = { };
		{
			d.m_expandConstantBuffer = m_pointLights.data();
			d.m_expandConstantBufferSize = sizeof(m_pointLights);
			d.m_tkmFilePath = tkmTeapotFilePath.c_str();
			d.m_fxFilePath = fxModelFilePath.c_str();
		}
		m_modelTeapot = std::make_unique<Model>();
		m_modelTeapot->Init(d);
	}

	{
		ModelInitData d = { };
		{
			d.m_expandConstantBuffer = m_pointLights.data();
			d.m_expandConstantBufferSize = sizeof(m_pointLights);
			d.m_tkmFilePath = tkmBgFilePath.c_str();
			d.m_fxFilePath = fxModelFilePath.c_str();
		}
		m_modelBg = std::make_unique<Model>();
		m_modelBg->Init(d);
	}

#if 1
	{
		SpriteInitData d;
		{
			d.m_width = Config::kRenderTargetWidth;
			d.m_height = Config::kRenderTargetHeight;
			d.m_textures.at(0) = &m_albedoRenderTarget.GetRenderTargetTexture();
			d.m_textures.at(1) = &m_normalRenderTarget.GetRenderTargetTexture();
			d.m_fxFilePath = fxDefferedLightingFilePath.c_str();
			d.m_expandConstantBuffer = nullptr; // TODO
			d.m_expandConstantBufferSize = 0; // TODO
		}
		m_defferedLightingSprite = std::make_unique<Sprite>();
		m_defferedLightingSprite->Init(d);
	}
#endif
}

void Models_16_02::addObserver()
{
	WinMgr::addObserver(&m_obserber);
}

void Models_16_02::removeObserver()
{
	WinMgr::removeObserver(&m_obserber);
}

void Models_16_02::handleInput()
{
	{
		using namespace MiniEngineIf;
		getCamera3D()->MoveForward(getStick(StickType::kLY));
		getCamera3D()->MoveRight(getStick(StickType::kLX));
		getCamera3D()->MoveUp(getStick(StickType::kRY));
	}

	if (!WinMgr::isWindowActive(WinMgr::Handle::kMain))
		return;

	if (m_obserber.isPaused())
		return;

	{
		Quaternion qRot;
		qRot.SetRotationDegY(0.2f);

		for (auto& pt : m_pointLights)
		{
			qRot.Apply(pt.m_position);
		}
	}
	// TODO: bottne neck‚ð’T‚·Brendering time‚ª³‚µ‚­‚È‚¢‚©‚à
}

void Models_16_02::draw(RenderContext& renderContext)
{
#if 0
	m_modelTeapot->Draw(renderContext);
	m_modelBg->Draw(renderContext);
#else
	RenderTarget* gbuffers[] = {
		&m_albedoRenderTarget,
		&m_normalRenderTarget
	};

	renderContext.WaitUntilToPossibleSetRenderTargets(ARRAYSIZE(gbuffers), gbuffers);

	renderContext.SetRenderTargets(ARRAYSIZE(gbuffers), gbuffers);
	renderContext.ClearRenderTargetViews(ARRAYSIZE(gbuffers), gbuffers);

	m_modelTeapot->Draw(renderContext);
	m_modelBg->Draw(renderContext);

	renderContext.WaitUntilToPossibleSetRenderTargets(ARRAYSIZE(gbuffers), gbuffers);

	MiniEngineIf::setOffscreenRenderTarget();

	m_defferedLightingSprite->Draw(renderContext);
#endif
}

void Models_16_02::debugRenderParams()
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
			constexpr size_t n = 5;
			const auto& light = m_pointLights[n];
			{
				const Vector3& v = light.m_position;
				ImguiIf::printParams<float>(ImguiIf::VarType::kFloat, "Light pos  ", std::vector<const float*>{ &v.x, &v.y, &v.z });
			}
			{
				const Vector3& v = light.m_color;
				ImguiIf::printParams<float>(ImguiIf::VarType::kFloat, "Light col  ", std::vector<const float*>{ &v.x, &v.y, &v.z });
			}
			{
				const float r = light.m_range;
				ImguiIf::printParams<float>(ImguiIf::VarType::kFloat, "Light range", std::vector<const float*>{ &r });
			}
		}
	}
}

namespace ModelHandler {
	std::unique_ptr<IModels> loadModelForChap16_02()
	{
		ModelFactory_16_02 factory;
		std::unique_ptr<IModels> iModels = factory.create();
		iModels->resetCamera();
		iModels->debugRenderParams();
		return std::move(iModels);
	}
} // namespace ModelHandler

