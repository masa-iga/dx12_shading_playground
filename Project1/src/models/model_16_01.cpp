#include "model_16_01.h"
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

class ModelFactory_16_01 : public IModelFactory
{
public:
	~ModelFactory_16_01() { }
	std::unique_ptr<IModels> create();
};

class Obserber_16_01 : public WinMgr::Iobserber
{
public: 
	void update(WPARAM wParam, LPARAM lParam) override;
	bool isPaused() const { return m_paused; }

private:
	bool m_paused = false;
};

class Models_16_01 : public IModels
{
public:
	Models_16_01() { }
	~Models_16_01()
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
	struct SPointLight
	{
		Vector3 m_position;
		float m_pad0 = 0.0f;
		Vector3 m_color;
		float m_pad1 = 0.0f;
		float m_range = 0.0f;
	};
	static constexpr size_t kNumPointLight = 1000;

	const std::string kTkmBgFile = "Sample_16_01/Sample_16_01/Assets/modelData/bg.tkm";
	std::string getTkmBgFilePath() { return ModelUtil::getPathFromAssetDir(kTkmBgFile); }
	const std::string kTkmTeapotFile = "Sample_14_04/Sample_14_04/Assets/modelData/teapot.tkm";
	std::string getTkmTeapotFilePath() { return ModelUtil::getPathFromAssetDir(kTkmTeapotFile); }
	const std::string kFxModelFile = "./Assets/shader/sample_16_01_model.fx";
	std::string getFxModelFilePath() { return kFxModelFile; }

	Obserber_16_01 m_obserber;
	std::unique_ptr<Model> m_modelTeapot = nullptr;
	std::unique_ptr<Model> m_modelBg = nullptr;
	std::array<SPointLight, kNumPointLight> m_pointLights;
};

std::unique_ptr<IModels> ModelFactory_16_01::create()
{
	std::unique_ptr<Models_16_01> m = std::make_unique<Models_16_01>();
	{
		m->createModel();
		m->addObserver();
	}
	return std::move(m);
}

void Obserber_16_01::update(WPARAM wParam, [[maybe_unused]] LPARAM lParam)
{
	switch (wParam) {
	case VK_SPACE:
		m_paused = !m_paused;
		break;
	default:
		break;
	}
}

void Models_16_01::resetCamera()
{
	MiniEngineIf::getCamera3D()->SetPosition({ 0.0f, 200.0f, 400.0f });
}

void Models_16_01::createModel()
{
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

	const std::string tkmBgFilePath = getTkmBgFilePath();
	Dbg::assert_(std::filesystem::exists(tkmBgFilePath));
	const std::string tkmTeapotFilePath = getTkmTeapotFilePath();
	Dbg::assert_(std::filesystem::exists(tkmTeapotFilePath));
	const std::string fxModelFilePath = getFxModelFilePath();
	Dbg::assert_(std::filesystem::exists(fxModelFilePath));

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
}

void Models_16_01::addObserver()
{
	WinMgr::addObserver(&m_obserber);
}

void Models_16_01::removeObserver()
{
	WinMgr::removeObserver(&m_obserber);
}

void Models_16_01::handleInput()
{
	if (!WinMgr::isWindowActive(WinMgr::Handle::kMain))
		return;

	if (m_obserber.isPaused())
		return;

	{
		Quaternion qRot;
		qRot.SetRotationDegY(1.0f);

		for (auto& pt : m_pointLights)
		{
			qRot.Apply(pt.m_position);
		}
	}

	using namespace MiniEngineIf;
	getCamera3D()->MoveForward(getStick(StickType::kLY));
	getCamera3D()->MoveRight(getStick(StickType::kLX));
	getCamera3D()->MoveUp(getStick(StickType::kRY));
}

void Models_16_01::draw(RenderContext& renderContext)
{
	m_modelTeapot->Draw(renderContext);
	m_modelBg->Draw(renderContext);
}

void Models_16_01::debugRenderParams()
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
	std::unique_ptr<IModels> loadModelForChap16_01()
	{
		ModelFactory_16_01 factory;
		std::unique_ptr<IModels> iModels = factory.create();
		iModels->resetCamera();
		iModels->debugRenderParams();
		return std::move(iModels);
	}
} // namespace ModelHandler

