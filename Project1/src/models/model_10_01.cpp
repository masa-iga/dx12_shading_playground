#include "model_10_01.h"
#include <filesystem>
#include <memory>
#include "imodel.h"
#include "MiniEngine.h"
#include "model_util.h"
#include "../debug_win.h"
#include "../imgui_if.h"
#include "../miniEngine_if.h"

class ModelFactory_10_01 : public IModelFactory
{
public:
	~ModelFactory_10_01() { }
	std::unique_ptr<IModels> create();
};

class Models_10_01 : public IModels
{
public:
	Models_10_01() { m_models.resize(static_cast<size_t>(ModelType::kSize)); }
	~Models_10_01() { }
	void createModel();
	void resetCamera();
	void handleInput();
	void draw(RenderContext& renderContext);
	void debugRenderParams();

private:
	enum class ModelType {
		kBox,
		kBg,
		kPlayer,
		kSize,
	};

	const std::string kTkmBoxFile = "Sample_10_01/Sample_10_01/Assets/modelData/box.tkm";
	const std::string kTkmBgFile = "Sample_10_01/Sample_10_01/Assets/modelData/bg/bg.tkm";
	const std::string kTkmSampleFile = "Sample_10_01/Sample_10_01/Assets/modelData/sample.tkm";
	const std::string kFxFile = "Sample_10_01/Sample_10_01/Assets/shader/sample3D.fx";
	std::string getTkmBoxFilePath() { return ModelUtil::getPathFromAssetDir(kTkmBoxFile); }
	std::string getTkmBgFilePath() { return ModelUtil::getPathFromAssetDir(kTkmBgFile); }
	std::string getTkmSampleFilePath() { return ModelUtil::getPathFromAssetDir(kTkmSampleFile); }
	std::string getFxFilePath() { return ModelUtil::getPathFromAssetDir(kFxFile); }

	RenderTarget m_offscreenRenderTarget;
	Vector3 m_plPos;
};

std::unique_ptr<IModels> ModelFactory_10_01::create()
{
	std::unique_ptr<Models_10_01> m(new Models_10_01);
	{
		m->createModel();
	}
	return std::move(m);
}

void Models_10_01::resetCamera()
{
	;
}

void Models_10_01::createModel()
{
	m_offscreenRenderTarget.Create(
		1280,
		720,
		1,
		1,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT_D32_FLOAT
	);

	const std::string tkmBoxFilePath = getTkmBoxFilePath();
	const std::string tkmBgFilePath = getTkmBgFilePath();
	const std::string tkmSampleFilePath = getTkmSampleFilePath();
	const std::string fxFilePath = getFxFilePath();
	Dbg::assert_(std::filesystem::exists(tkmBoxFilePath));
	Dbg::assert_(std::filesystem::exists(tkmBgFilePath));
	Dbg::assert_(std::filesystem::exists(tkmSampleFilePath));
	Dbg::assert_(std::filesystem::exists(fxFilePath));

	ModelInitData initData = { };
	{
		initData.m_fxFilePath = fxFilePath.c_str();
	}

	{
		initData.m_tkmFilePath = tkmBoxFilePath.c_str();
		std::unique_ptr<Model> model(new Model);
		model->Init(initData);
		model->UpdateWorldMatrix({ 100.0f, 0.0f, 0.0f }, g_quatIdentity, g_vec3One);
		model->ChangeAlbedoMap("", m_offscreenRenderTarget.GetRenderTargetTexture());
		m_models.at(static_cast<size_t>(ModelType::kBox)) = std::move(model);
	}
	{
		initData.m_tkmFilePath = tkmBgFilePath.c_str();
		std::unique_ptr<Model> model(new Model);
		model->Init(initData);
		m_models.at(static_cast<size_t>(ModelType::kBg)) = std::move(model);
	}
	{
		initData.m_tkmFilePath = tkmSampleFilePath.c_str();
		std::unique_ptr<Model> model(new Model);
		model->Init(initData);
		m_models.at(static_cast<size_t>(ModelType::kPlayer)) = std::move(model);
	}
}

void Models_10_01::handleInput()
{
	m_plPos.x -= MiniEngineIf::getStick(MiniEngineIf::StickType::kLX);
	m_plPos.z -= MiniEngineIf::getStick(MiniEngineIf::StickType::kLY);

	m_models.at(static_cast<size_t>(ModelType::kPlayer))->UpdateWorldMatrix(m_plPos, g_quatIdentity, g_vec3One);
}

void Models_10_01::draw(RenderContext& renderContext)
{
	// render to offscreen buffer managed in this class
	RenderTarget* rtArray[] = { &m_offscreenRenderTarget };

	renderContext.WaitUntilToPossibleSetRenderTargets(1, rtArray);

	renderContext.SetRenderTargets(1, rtArray);
	renderContext.ClearRenderTargetViews(1, rtArray);
	m_models.at(static_cast<size_t>(ModelType::kBg))->Draw(renderContext);
	m_models.at(static_cast<size_t>(ModelType::kPlayer))->Draw(renderContext);

	renderContext.WaitUntilFinishDrawingToRenderTargets(1, rtArray);

	// render to offscreen buffer managed in MiniengineIf
	MiniEngineIf::setOffscreenRenderTarget();

	for (std::unique_ptr<Model>& model : m_models)
	{
		model->Draw(renderContext);
	}
}

void Models_10_01::debugRenderParams()
{
	ImguiIf::printParams<float>(ImguiIf::VarType::kFloat, "Player", std::vector<const float*>{ &m_plPos.x, & m_plPos.y, & m_plPos.z });
}

namespace ModelHandler {
	std::unique_ptr<IModels> loadModelForChap10_01()
	{
		ModelFactory_10_01 factory;
		std::unique_ptr<IModels> iModels = factory.create();
		iModels->debugRenderParams();
		return std::move(iModels);
	}
} // namespace ModelHandler

