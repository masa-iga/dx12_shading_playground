#include "model_17_01.h"
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

class ModelFactory_17_01 : public IModelFactory
{
public:
	~ModelFactory_17_01() { }
	std::unique_ptr<IModels> create();
};

class Obserber_17_01 : public WinMgr::Iobserber
{
public: 
	void update(WPARAM wParam, LPARAM lParam) override;
	bool isPaused() const { return m_paused; }

private:
	bool m_paused = false;
};

class Models_17_01 : public IModels
{
public:
	Models_17_01() { }
	~Models_17_01()
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
	const std::string kTkmFileSample = "Sample_17_01/Sample_17_01/Assets/modelData/sample.tkm";
	std::string getTkmFilePathSample() const { return ModelUtil::getPathFromAssetDir(kTkmFileSample); }

	Obserber_17_01 m_obserber;
};

std::unique_ptr<IModels> ModelFactory_17_01::create()
{
	std::unique_ptr<Models_17_01> m = std::make_unique<Models_17_01>();
	{
		m->createModel();
		m->addObserver();
	}
	return std::move(m);
}

void Obserber_17_01::update(WPARAM wParam, [[maybe_unused]] LPARAM lParam)
{
	switch (wParam) {
	case VK_SPACE:
		m_paused = !m_paused;
		break;
	default:
		break;
	}
}

void Models_17_01::resetCamera()
{
	MiniEngineIf::getCamera3D()->SetPosition({ 0.0f, 50.0f, 120.0f });
	MiniEngineIf::getCamera3D()->SetTarget(0.0f, 50.0f, 200.0f);
	MiniEngineIf::getCamera3D()->Update();
}

void Models_17_01::createModel()
{
	const std::string tkmFilePathSample = getTkmFilePathSample();
	Dbg::assert_(std::filesystem::exists(tkmFilePathSample));

	ModelInitData d;
	{
		d.m_tkmFilePath = tkmFilePathSample.c_str();
	}
	std::unique_ptr<Model> m_modelHuman = nullptr;
	m_modelHuman = std::make_unique<Model>();
	m_modelHuman->Init(d);

	g_graphicsEngine->RegistModelToRaytracingWorld(*m_modelHuman);
}

void Models_17_01::addObserver()
{
	WinMgr::addObserver(&m_obserber);
}

void Models_17_01::removeObserver()
{
	WinMgr::removeObserver(&m_obserber);
}

void Models_17_01::handleInput()
{
	using namespace MiniEngineIf;
	if (isPress(Button::kLB1))
	{
		getCamera3D()->MoveUp(g_pad[0]->GetLStickYF());
	}
	else
	{
		getCamera3D()->MoveForward(-g_pad[0]->GetLStickYF());
	}

	getCamera3D()->MoveRight(g_pad[0]->GetLStickXF());
	Quaternion qRotX, qRotY;
	qRotX.SetRotationX(g_pad[0]->GetRStickYF() * -0.005f);
	getCamera3D()->RotateOriginTarget(qRotX);
	qRotY.SetRotationY(g_pad[0]->GetRStickXF() * 0.005f);
	getCamera3D()->RotateOriginTarget(qRotY);
}

void Models_17_01::draw(RenderContext& renderContext)
{
	(void)renderContext;
}

void Models_17_01::debugRenderParams()
{
	;
}

namespace ModelHandler {
	std::unique_ptr<IModels> loadModelForChap17_01()
	{
		ModelFactory_17_01 factory;
		std::unique_ptr<IModels> iModels = factory.create();
		iModels->resetCamera();
		iModels->debugRenderParams();
		return std::move(iModels);
	}
} // namespace ModelHandler

