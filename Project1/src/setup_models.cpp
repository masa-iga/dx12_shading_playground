#include "setup_models.h"
#include <cstring>
#include <filesystem>
#include "../import/hlsl-grimoire-sample/MiniEngine/MiniEngine.h"
#include "debug_win.h"
#include "imgui_if.h"
#include "miniEngine_if.h"
#include "models/model_04_03.h"
#include "models/model_05_01.h"
#include "models/model_05_02.h"
#include "models/model_05_03.h"
#include "models/model_05_04.h"
#include "models/model_06_01.h"

#define LOAD_MODEL_CHAP_04_01 (0)
#define LOAD_MODEL_CHAP_04_03 (1)
#define LOAD_MODEL_CHAP_05_01 (0)
#define LOAD_MODEL_CHAP_05_02 (0)
#define LOAD_MODEL_CHAP_05_03 (0)
#define LOAD_MODEL_CHAP_05_04 (0)
#define LOAD_MODEL_CHAP_06_01 (0)

namespace {
	const std::string kBaseAssetDir = "../import/hlsl-grimoire-sample";

	constexpr std::string getPathFromAssetDir(const std::string path);
	void initModel(const std::string& tkmFilePath, const std::string& fxFilePath, Model* model, void* constantBuffer, size_t constantBufferSize);
	void loadModelForChap04_01();

	std::vector<Model*> s_models;
} // namespace anonymous

void Models::loadModel()
{
#if LOAD_MODEL_CHAP_04_01
		loadModelInternal(Models::Chapter::k04_01);
#endif // #if LOAD_MODEL_CHAP_04_01
#if LOAD_MODEL_CHAP_04_03
		loadModelInternal(Models::Chapter::k04_03);
#endif // #if LOAD_MODEL_CHAP_04_03
#if LOAD_MODEL_CHAP_05_01
		loadModelInternal(Models::Chapter::k05_01);
#endif // #if LOAD_MODEL_CHAP_05_01
#if LOAD_MODEL_CHAP_05_02
		loadModelInternal(Models::Chapter::k05_02);
#endif // #if LOAD_MODEL_CHAP_05_02
#if LOAD_MODEL_CHAP_05_03
		loadModelInternal(Models::Chapter::k05_03);
#endif // #if LOAD_MODEL_CHAP_05_03
#if LOAD_MODEL_CHAP_05_04
		loadModelInternal(Models::Chapter::k05_04);
#endif // #if LOAD_MODEL_CHAP_05_04
#if LOAD_MODEL_CHAP_06_01
		loadModelInternal(Models::Chapter::k06_01);
#endif // #if LOAD_MODEL_CHAP_06_01
}

void Models::handleInput()
{
#if LOAD_MODEL_CHAP_04_01
		handleInputInternal(Models::Chapter::k04_01);
#endif // #if LOAD_MODEL_CHAP_04_01
#if LOAD_MODEL_CHAP_04_03
		handleInputInternal(Models::Chapter::k04_03);
#endif // #if LOAD_MODEL_CHAP_04_03
#if LOAD_MODEL_CHAP_05_01
		handleInputInternal(Models::Chapter::k05_01);
#endif // #if LOAD_MODEL_CHAP_05_01
#if LOAD_MODEL_CHAP_05_02
		handleInputInternal(Models::Chapter::k05_02);
#endif // #if LOAD_MODEL_CHAP_05_02
#if LOAD_MODEL_CHAP_05_03
		handleInputInternal(Models::Chapter::k05_03);
#endif // #if LOAD_MODEL_CHAP_05_03
#if LOAD_MODEL_CHAP_05_04
		handleInputInternal(Models::Chapter::k05_04);
#endif // #if LOAD_MODEL_CHAP_05_04
#if LOAD_MODEL_CHAP_06_01
		handleInputInternal(Models::Chapter::k06_01);
#endif // #if LOAD_MODEL_CHAP_06_01
}

void Models::draw(RenderContext& renderContext)
{
	for (auto model : s_models)
	{
		model->Draw(renderContext);
	}
}

void Models::loadModelInternal(Chapter chapter)
{
	switch (chapter) {
	case Chapter::k04_01: loadModelForChap04_01(); break;
	case Chapter::k04_03: ModelHandler::loadModelForChap04_03(s_models); break;
	case Chapter::k05_01: ModelHandler::loadModelForChap05_01(s_models); break;
	case Chapter::k05_02: ModelHandler::loadModelForChap05_02(s_models); break;
	case Chapter::k05_03: ModelHandler::loadModelForChap05_03(s_models); break;
	case Chapter::k05_04: ModelHandler::loadModelForChap05_04(s_models); break;
	case Chapter::k06_01: ModelHandler::loadModelForChap06_01(s_models); break;
	default: break;
	}
}

void Models::handleInputInternal(Chapter chapter)
{
	switch (chapter) {
	case Chapter::k04_01: break;
	case Chapter::k04_03: ModelHandler::handleInputForChap04_03(); break;
	case Chapter::k05_01: ModelHandler::handleInputForChap05_01(); break;
	case Chapter::k05_02: ModelHandler::handleInputForChap05_02(); break;
	case Chapter::k05_03: ModelHandler::handleInputForChap05_03(); break;
	case Chapter::k05_04: ModelHandler::handleInputForChap05_04(); break;
	case Chapter::k06_01: ModelHandler::handleInputForChap06_01(); break;
	default: break;
	}
}

namespace {
	constexpr std::string getPathFromAssetDir(const std::string path)
	{
		return kBaseAssetDir + "/" + path;
	}

	void initModel(const std::string& tkmFilePath, const std::string& fxFilePath, Model* model, void* constantBuffer, size_t constantBufferSize)
	{
		Dbg::assert_(std::filesystem::exists(tkmFilePath));
		Dbg::assert_(std::filesystem::exists(fxFilePath));

		ModelInitData initData = { };
		initData.m_tkmFilePath = tkmFilePath.c_str();
		initData.m_fxFilePath = fxFilePath.c_str();

		if (constantBuffer && constantBufferSize > 0)
		{
			initData.m_expandConstantBuffer = constantBuffer;
			initData.m_expandConstantBufferSize = static_cast<int32_t>(constantBufferSize);
		}

		model->Init(initData);
		s_models.push_back(model);
	}

	void loadModelForChap04_01()
	{
		const std::string tkmFile = "Sample_04_01/Sample_04_01/Assets/modelData/sample.tkm";
		const std::string fxFile = "Assets/shader/sample_04_01.fx";
		const std::string tkmFilePath = getPathFromAssetDir(tkmFile);
		const std::string fxFilePath = fxFile;
		static Model s_model;
		initModel(tkmFilePath, fxFilePath, &s_model, nullptr, 0);
	}
} // namespace anonymous
