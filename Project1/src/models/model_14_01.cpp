#include "model_14_01.h"
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
#include <../Sample_14_01/Sample_14_01/MyRenderer.h>
#include <../Sample_14_01/Sample_14_01/RenderingEngine.h>
#include <../Sample_14_01/Sample_14_01/ModelRender.h>
#pragma warning(3 : 4189)

class ModelFactory_14_01 : public IModelFactory
{
public:
	~ModelFactory_14_01() { }
	std::unique_ptr<IModels> create(RenderContext& renderContext);
};

class Obserber_14_01 : public WinMgr::Iobserber
{
public: 
	void update(WPARAM wParam, LPARAM lParam) override;
	bool isPaused() const { return m_paused; }

private:
	bool m_paused = false;
};

class Models_14_01 : public IModels
{
public:
	Models_14_01() { }
	~Models_14_01()
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
	const std::string kTkmBgFile = "Sample_14_01/Sample_14_01/Assets/modelData/bg/bg.tkm";
	std::string getTkmBgFilePath() { return ModelUtil::getPathFromAssetDir(kTkmBgFile); }
	const std::string kTkmTeapotFile = "Sample_14_01/Sample_14_01/Assets/modelData/teapot.tkm";
	std::string getTkmTeapotFilePath() { return ModelUtil::getPathFromAssetDir(kTkmTeapotFile); }
	const std::string kFxSampleFile = "./Assets/shader/sample_14_01.fx";
	std::string getFxSampleFilePath() { return kFxSampleFile; }

	Obserber_14_01 m_obserber;
	myRenderer::RenderingEngine m_renderingEngine;
	myRenderer::ModelRender m_modelRenderBg;
	myRenderer::ModelRender m_modelRenderTeapot;
};

std::unique_ptr<IModels> ModelFactory_14_01::create(RenderContext& renderContext)
{
	std::unique_ptr<Models_14_01> m = std::make_unique<Models_14_01>();
	{
		m->createModel(renderContext);
		m->addObserver();
	}
	return std::move(m);
}

void Obserber_14_01::update(WPARAM wParam, [[maybe_unused]] LPARAM lParam)
{
	switch (wParam) {
	case VK_SPACE:
		m_paused = !m_paused;
		break;
	default:
		break;
	}
}

void Models_14_01::resetCamera()
{
	MiniEngineIf::getCamera3D()->SetPosition({ 0.0f, 80.0f, 200.0f });
	MiniEngineIf::getCamera3D()->SetTarget({ 0.0f, 80.0f, 0.0f });
	MiniEngineIf::getCamera3D()->Update();
}

void Models_14_01::createModel([[maybe_unused]] RenderContext& renderContext)
{
	m_renderingEngine.Init();

	const std::string tkmBgFilePath = getTkmBgFilePath();
	Dbg::assert_(std::filesystem::exists(tkmBgFilePath));
	const std::string tkmTeapotFilePath = getTkmTeapotFilePath();
	Dbg::assert_(std::filesystem::exists(tkmTeapotFilePath));
	const std::string fxSampleFilePath = getFxSampleFilePath();
	Dbg::assert_(std::filesystem::exists(fxSampleFilePath));

	{
		m_modelRenderBg.InitDeferredRendering(m_renderingEngine, tkmBgFilePath.c_str(), true);
	}
	{
		myRenderer::ModelInitDataFR d;
		d.m_tkmFilePath = tkmTeapotFilePath.c_str();
		d.m_fxFilePath = fxSampleFilePath.c_str();
		d.m_expandShaderResoruceView.at(0) = &m_renderingEngine.GetZPrepassDepthTexture();
		m_modelRenderTeapot.InitForwardRendering(m_renderingEngine, d);
		m_modelRenderTeapot.SetShadowCasterFlag(true);
		m_modelRenderTeapot.UpdateWorldMatrix({ 0.0f, 50.0f, 0.0f }, g_quatIdentity, g_vec3One);
	}
}

void Models_14_01::addObserver()
{
	WinMgr::addObserver(&m_obserber);
}

void Models_14_01::removeObserver()
{
	WinMgr::removeObserver(&m_obserber);
}

void Models_14_01::handleInput()
{
	if (!WinMgr::isWindowActive(WinMgr::Handle::kMain))
		return;

	if (m_obserber.isPaused())
		return;

	using namespace MiniEngineIf;
	getCamera3D()->MoveForward(getStick(StickType::kLY));
	getCamera3D()->MoveRight(getStick(StickType::kLX));
	getCamera3D()->MoveUp(getStick(StickType::kRY));
}

void Models_14_01::draw(RenderContext& renderContext)
{
	{
		m_modelRenderBg.Draw();
		m_modelRenderTeapot.Draw();

		m_renderingEngine.Execute(renderContext);
	}

	// copy main frame buffer of in GraphicsEngine to buffer in my engine
	{
		{
			CD3DX12_VIEWPORT vp(MiniEngineIf::getRenderTargetResource());
			renderContext.SetViewportAndScissor(vp);
		}
		{
			D3D12_CPU_DESCRIPTOR_HANDLE rtv = MiniEngineIf::getOffscreenRtvCpuDescHandle();
			D3D12_CPU_DESCRIPTOR_HANDLE dsv = MiniEngineIf::getOffscreenDsvCpuDescHandle();
			m_renderingEngine.CopyMainRenderTarget(renderContext, rtv, dsv);
		}
	}
}

void Models_14_01::debugRenderParams()
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
	std::unique_ptr<IModels> loadModelForChap14_01(RenderContext& renderContext)
	{
		ModelFactory_14_01 factory;
		std::unique_ptr<IModels> iModels = factory.create(renderContext);
		iModels->resetCamera();
		iModels->debugRenderParams();
		return std::move(iModels);
	}
} // namespace ModelHandler

