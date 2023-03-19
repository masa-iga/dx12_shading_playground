#include "setup_models.h"
#include <cstring>
#include "../import/hlsl-grimoire-sample/MiniEngine/MiniEngine.h"
#include "debug_win.h"
#include "imgui_if.h"
#include "miniEngine_if.h"
#include "models/imodel.h"
#include "models/model_04_01.h"
#include "models/model_04_03.h"
#include "models/model_05_01.h"
#include "models/model_05_02.h"
#include "models/model_05_03.h"
#include "models/model_05_04.h"
#include "models/model_06_01.h"
#include "models/model_06_02.h"
#include "models/model_06_03.h"
#include "models/model_07_02.h"
#include "models/model_07_03.h"

#define LOAD_MODEL_CHAP_04_01 (0)
#define LOAD_MODEL_CHAP_04_03 (0)
#define LOAD_MODEL_CHAP_05_01 (0)
#define LOAD_MODEL_CHAP_05_02 (0)
#define LOAD_MODEL_CHAP_05_03 (0)
#define LOAD_MODEL_CHAP_05_04 (0)
#define LOAD_MODEL_CHAP_06_01 (0)
#define LOAD_MODEL_CHAP_06_02 (0)
#define LOAD_MODEL_CHAP_06_03 (0)
#define LOAD_MODEL_CHAP_07_02 (0)
#define LOAD_MODEL_CHAP_07_03 (1)

namespace {
	std::vector<Model*> s_models;
	std::unique_ptr<IModels> s_iModels = nullptr;
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
#if LOAD_MODEL_CHAP_06_02
		loadModelInternal(Models::Chapter::k06_02);
#endif // #if LOAD_MODEL_CHAP_06_02
#if LOAD_MODEL_CHAP_06_03
		loadModelInternal(Models::Chapter::k06_03);
#endif // #if LOAD_MODEL_CHAP_06_03
#if LOAD_MODEL_CHAP_07_02
		loadModelInternal(Models::Chapter::k07_02);
#endif // #if LOAD_MODEL_CHAP_07_02
#if LOAD_MODEL_CHAP_07_03
		loadModelInternal(Models::Chapter::k07_03);
#endif // #if LOAD_MODEL_CHAP_07_03
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
#if LOAD_MODEL_CHAP_06_02
		handleInputInternal(Models::Chapter::k06_02);
#endif // #if LOAD_MODEL_CHAP_06_02
#if LOAD_MODEL_CHAP_06_03
		handleInputInternal(Models::Chapter::k06_03);
#endif // #if LOAD_MODEL_CHAP_06_03
#if LOAD_MODEL_CHAP_07_02
		handleInputInternal(Models::Chapter::k07_02);
#endif // #if LOAD_MODEL_CHAP_07_02
#if LOAD_MODEL_CHAP_07_03
		handleInputInternal(Models::Chapter::k07_03);
#endif // #if LOAD_MODEL_CHAP_07_03
}

void Models::draw(RenderContext& renderContext)
{
	if (s_iModels != nullptr)
	{
		s_iModels->draw(renderContext);
		return;
	}

	for (auto model : s_models)
	{
		model->Draw(renderContext);
	}
}

void Models::loadModelInternal(Chapter chapter)
{
	switch (chapter) {
	case Chapter::k04_01: ModelHandler::loadModelForChap04_01(s_models); break;
	case Chapter::k04_03: ModelHandler::loadModelForChap04_03(s_models); break;
	case Chapter::k05_01: ModelHandler::loadModelForChap05_01(s_models); break;
	case Chapter::k05_02: ModelHandler::loadModelForChap05_02(s_models); break;
	case Chapter::k05_03: ModelHandler::loadModelForChap05_03(s_models); break;
	case Chapter::k05_04: ModelHandler::loadModelForChap05_04(s_models); break;
	case Chapter::k06_01: ModelHandler::loadModelForChap06_01(s_models); break;
	case Chapter::k06_02: ModelHandler::loadModelForChap06_02(s_models); break;
	case Chapter::k06_03: ModelHandler::loadModelForChap06_03(s_models); break;
	case Chapter::k07_02: s_iModels = ModelHandler::loadModelForChap07_02(); break;
	case Chapter::k07_03: s_iModels = ModelHandler::loadModelForChap07_03(); break;
	default: break;
	}
}

void Models::handleInputInternal(Chapter chapter)
{
	switch (chapter) {
	case Chapter::k04_01: ModelHandler::handleInputForChap04_01(); break;
	case Chapter::k04_03: ModelHandler::handleInputForChap04_03(); break;
	case Chapter::k05_01: ModelHandler::handleInputForChap05_01(); break;
	case Chapter::k05_02: ModelHandler::handleInputForChap05_02(); break;
	case Chapter::k05_03: ModelHandler::handleInputForChap05_03(); break;
	case Chapter::k05_04: ModelHandler::handleInputForChap05_04(); break;
	case Chapter::k06_01: ModelHandler::handleInputForChap06_01(); break;
	case Chapter::k06_02: ModelHandler::handleInputForChap06_02(); break;
	case Chapter::k06_03: ModelHandler::handleInputForChap06_03(); break;
	case Chapter::k07_02: s_iModels->handleInput(); break;
	case Chapter::k07_03: s_iModels->handleInput(); break;
	default: break;
	}
}
