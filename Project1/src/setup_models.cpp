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
#include "models/model_10_01.h"
#include "models/model_10_02.h"
#include "models/model_10_03.h"
#include "models/model_10_04.h"
#include "models/model_10_05.h"
#include "models/model_10_06.h"
#include "models/model_10_07.h"
#include "models/model_10_08.h"
#include "models/model_11_01.h"
#include "models/model_11_02.h"
#include "models/model_11_03.h"
#include "models/model_11_04.h"
#include "models/model_11_05.h"
#include "models/model_11_06.h"
#include "models/model_12_01.h"
#include "models/model_12_02.h"
#include "models/model_12_03.h"
#include "models/model_12_04.h"
#include "models/model_13_02.h"
#include "models/model_14_01.h"
#include "models/model_14_02.h"
#include "models/model_14_04.h"
#include "models/model_16_01.h"
#include "models/model_16_02.h"

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
#define LOAD_MODEL_CHAP_07_03 (0)
#define LOAD_MODEL_CHAP_10_01 (0)
#define LOAD_MODEL_CHAP_10_02 (0)
#define LOAD_MODEL_CHAP_10_03 (0)
#define LOAD_MODEL_CHAP_10_04 (0)
#define LOAD_MODEL_CHAP_10_05 (0)
#define LOAD_MODEL_CHAP_10_06 (0)
#define LOAD_MODEL_CHAP_10_07 (0)
#define LOAD_MODEL_CHAP_10_08 (0)
#define LOAD_MODEL_CHAP_11_01 (0)
#define LOAD_MODEL_CHAP_11_02 (0)
#define LOAD_MODEL_CHAP_11_03 (0)
#define LOAD_MODEL_CHAP_11_04 (0)
#define LOAD_MODEL_CHAP_11_05 (0)
#define LOAD_MODEL_CHAP_11_06 (0)
#define LOAD_MODEL_CHAP_12_01 (0)
#define LOAD_MODEL_CHAP_12_02 (0)
#define LOAD_MODEL_CHAP_12_03 (0)
#define LOAD_MODEL_CHAP_12_04 (0)
#define LOAD_MODEL_CHAP_13_02 (0)
#define LOAD_MODEL_CHAP_14_01 (0)
#define LOAD_MODEL_CHAP_14_02 (0)
#define LOAD_MODEL_CHAP_14_04 (0)
#define LOAD_MODEL_CHAP_16_01 (0)
#define LOAD_MODEL_CHAP_16_02 (1)

#if LOAD_MODEL_CHAP_14_01
#include <../Sample_14_01/Sample_14_01/Bloom.cpp>
#include <../Sample_14_01/Sample_14_01/CascadeShadowMapMatrix.cpp>
#include <../Sample_14_01/Sample_14_01/Dof.cpp>
#include <../Sample_14_01/Sample_14_01/ModelRender.cpp>
#include <../Sample_14_01/Sample_14_01/RenderingEngine.cpp>
#include <../Sample_14_01/Sample_14_01/ShadowMapRender.cpp>
#include <../Sample_14_01/Sample_14_01/PostEffect.cpp>
#elif LOAD_MODEL_CHAP_14_02
#include <../Sample_14_01/Sample_14_01/Bloom.cpp>
#include <../Sample_14_01/Sample_14_01/CascadeShadowMapMatrix.cpp>
#include <../Sample_14_01/Sample_14_01/Dof.cpp>
#include <../Sample_14_01/Sample_14_01/ModelRender.cpp>
#include <../Sample_14_01/Sample_14_01/RenderingEngine.cpp>
#include <../Sample_14_01/Sample_14_01/ShadowMapRender.cpp>
#include <../Sample_14_01/Sample_14_01/PostEffect.cpp>
#elif LOAD_MODEL_CHAP_14_04
#include <../Sample_14_01/Sample_14_01/Bloom.cpp>
#include <../Sample_14_01/Sample_14_01/CascadeShadowMapMatrix.cpp>
#include <../Sample_14_01/Sample_14_01/Dof.cpp>
#include <../Sample_14_01/Sample_14_01/ModelRender.cpp>
#include <../Sample_14_01/Sample_14_01/RenderingEngine.cpp>
#include <../Sample_14_01/Sample_14_01/ShadowMapRender.cpp>
#include <../Sample_14_01/Sample_14_01/PostEffect.cpp>
#else
#include <../Sample_14_01/Sample_14_01/Bloom.cpp>
#include <../Sample_14_01/Sample_14_01/CascadeShadowMapMatrix.cpp>
#include <../Sample_14_01/Sample_14_01/Dof.cpp>
#include <../Sample_14_01/Sample_14_01/ModelRender.cpp>
#include <../Sample_14_01/Sample_14_01/RenderingEngine.cpp>
#include <../Sample_14_01/Sample_14_01/ShadowMapRender.cpp>
#include <../Sample_14_01/Sample_14_01/PostEffect.cpp>
#endif // #if LOAD_MODEL_CHAP_14_04

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
#if LOAD_MODEL_CHAP_10_01
		loadModelInternal(Models::Chapter::k10_01);
#endif // #if LOAD_MODEL_CHAP_10_01
#if LOAD_MODEL_CHAP_10_02
		loadModelInternal(Models::Chapter::k10_02);
#endif // #if LOAD_MODEL_CHAP_10_02
#if LOAD_MODEL_CHAP_10_03
		loadModelInternal(Models::Chapter::k10_03);
#endif // #if LOAD_MODEL_CHAP_10_03
#if LOAD_MODEL_CHAP_10_04
		loadModelInternal(Models::Chapter::k10_04);
#endif // #if LOAD_MODEL_CHAP_10_04
#if LOAD_MODEL_CHAP_10_05
		loadModelInternal(Models::Chapter::k10_05);
#endif // #if LOAD_MODEL_CHAP_10_05
#if LOAD_MODEL_CHAP_10_06
		loadModelInternal(Models::Chapter::k10_06);
#endif // #if LOAD_MODEL_CHAP_10_06
#if LOAD_MODEL_CHAP_10_07
		loadModelInternal(Models::Chapter::k10_07);
#endif // #if LOAD_MODEL_CHAP_10_07
#if LOAD_MODEL_CHAP_10_08
		loadModelInternal(Models::Chapter::k10_08);
#endif // #if LOAD_MODEL_CHAP_10_08
#if LOAD_MODEL_CHAP_11_01
		loadModelInternal(Models::Chapter::k11_01);
#endif // #if LOAD_MODEL_CHAP_11_01
#if LOAD_MODEL_CHAP_11_02
		loadModelInternal(Models::Chapter::k11_02);
#endif // #if LOAD_MODEL_CHAP_11_02
#if LOAD_MODEL_CHAP_11_03
		loadModelInternal(Models::Chapter::k11_03);
#endif // #if LOAD_MODEL_CHAP_11_03
#if LOAD_MODEL_CHAP_11_04
		loadModelInternal(Models::Chapter::k11_04);
#endif // #if LOAD_MODEL_CHAP_11_04
#if LOAD_MODEL_CHAP_11_05
		loadModelInternal(Models::Chapter::k11_05);
#endif // #if LOAD_MODEL_CHAP_11_05
#if LOAD_MODEL_CHAP_11_06
		loadModelInternal(Models::Chapter::k11_06);
#endif // #if LOAD_MODEL_CHAP_11_06
#if LOAD_MODEL_CHAP_12_01
		loadModelInternal(Models::Chapter::k12_01);
#endif // #if LOAD_MODEL_CHAP_12_01
#if LOAD_MODEL_CHAP_12_02
		loadModelInternal(Models::Chapter::k12_02);
#endif // #if LOAD_MODEL_CHAP_12_02
#if LOAD_MODEL_CHAP_12_03
		loadModelInternal(Models::Chapter::k12_03);
#endif // #if LOAD_MODEL_CHAP_12_03
#if LOAD_MODEL_CHAP_12_04
		loadModelInternal(Models::Chapter::k12_04);
#endif // #if LOAD_MODEL_CHAP_12_04
#if LOAD_MODEL_CHAP_13_02
		loadModelInternal(Models::Chapter::k13_02);
#endif // #if LOAD_MODEL_CHAP_13_02
#if LOAD_MODEL_CHAP_14_01
		loadModelInternal(Models::Chapter::k14_01);
#endif // #if LOAD_MODEL_CHAP_14_01
#if LOAD_MODEL_CHAP_14_02
		loadModelInternal(Models::Chapter::k14_02);
#endif // #if LOAD_MODEL_CHAP_14_02
#if LOAD_MODEL_CHAP_14_04
		loadModelInternal(Models::Chapter::k14_04);
#endif // #if LOAD_MODEL_CHAP_14_04
#if LOAD_MODEL_CHAP_16_01
		loadModelInternal(Models::Chapter::k16_01);
#endif // #if LOAD_MODEL_CHAP_16_01
#if LOAD_MODEL_CHAP_16_02
		loadModelInternal(Models::Chapter::k16_02);
#endif // #if LOAD_MODEL_CHAP_16_02
}

void Models::releaseResource()
{
	for (auto& model : s_models)
	{
		delete(model);
	}
	s_models.clear();

	s_iModels.reset();
	s_iModels = nullptr;
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
#if LOAD_MODEL_CHAP_10_01
		handleInputInternal(Models::Chapter::k10_01);
#endif // #if LOAD_MODEL_CHAP_10_01
#if LOAD_MODEL_CHAP_10_02
		handleInputInternal(Models::Chapter::k10_02);
#endif // #if LOAD_MODEL_CHAP_10_02
#if LOAD_MODEL_CHAP_10_03
		handleInputInternal(Models::Chapter::k10_03);
#endif // #if LOAD_MODEL_CHAP_10_03
#if LOAD_MODEL_CHAP_10_04
		handleInputInternal(Models::Chapter::k10_04);
#endif // #if LOAD_MODEL_CHAP_10_04
#if LOAD_MODEL_CHAP_10_05
		handleInputInternal(Models::Chapter::k10_05);
#endif // #if LOAD_MODEL_CHAP_10_05
#if LOAD_MODEL_CHAP_10_06
		handleInputInternal(Models::Chapter::k10_06);
#endif // #if LOAD_MODEL_CHAP_10_06
#if LOAD_MODEL_CHAP_10_07
		handleInputInternal(Models::Chapter::k10_07);
#endif // #if LOAD_MODEL_CHAP_10_07
#if LOAD_MODEL_CHAP_10_08
		handleInputInternal(Models::Chapter::k10_08);
#endif // #if LOAD_MODEL_CHAP_10_08
#if LOAD_MODEL_CHAP_11_01
		handleInputInternal(Models::Chapter::k11_01);
#endif // #if LOAD_MODEL_CHAP_11_01
#if LOAD_MODEL_CHAP_11_02
		handleInputInternal(Models::Chapter::k11_02);
#endif // #if LOAD_MODEL_CHAP_11_02
#if LOAD_MODEL_CHAP_11_03
		handleInputInternal(Models::Chapter::k11_03);
#endif // #if LOAD_MODEL_CHAP_11_03
#if LOAD_MODEL_CHAP_11_04
		handleInputInternal(Models::Chapter::k11_04);
#endif // #if LOAD_MODEL_CHAP_11_04
#if LOAD_MODEL_CHAP_11_05
		handleInputInternal(Models::Chapter::k11_05);
#endif // #if LOAD_MODEL_CHAP_11_05
#if LOAD_MODEL_CHAP_11_06
		handleInputInternal(Models::Chapter::k11_06);
#endif // #if LOAD_MODEL_CHAP_11_06
#if LOAD_MODEL_CHAP_12_01
		handleInputInternal(Models::Chapter::k12_01);
#endif // #if LOAD_MODEL_CHAP_12_01
#if LOAD_MODEL_CHAP_12_02
		handleInputInternal(Models::Chapter::k12_02);
#endif // #if LOAD_MODEL_CHAP_12_02
#if LOAD_MODEL_CHAP_12_03
		handleInputInternal(Models::Chapter::k12_03);
#endif // #if LOAD_MODEL_CHAP_12_03
#if LOAD_MODEL_CHAP_12_04
		handleInputInternal(Models::Chapter::k12_04);
#endif // #if LOAD_MODEL_CHAP_12_04
#if LOAD_MODEL_CHAP_13_02
		handleInputInternal(Models::Chapter::k13_02);
#endif // #if LOAD_MODEL_CHAP_13_02
#if LOAD_MODEL_CHAP_14_01
		handleInputInternal(Models::Chapter::k14_01);
#endif // #if LOAD_MODEL_CHAP_14_01
#if LOAD_MODEL_CHAP_14_02
		handleInputInternal(Models::Chapter::k14_02);
#endif // #if LOAD_MODEL_CHAP_14_02
#if LOAD_MODEL_CHAP_14_04
		handleInputInternal(Models::Chapter::k14_04);
#endif // #if LOAD_MODEL_CHAP_14_04
#if LOAD_MODEL_CHAP_16_01
		handleInputInternal(Models::Chapter::k16_01);
#endif // #if LOAD_MODEL_CHAP_16_01
#if LOAD_MODEL_CHAP_16_02
		handleInputInternal(Models::Chapter::k16_02);
#endif // #if LOAD_MODEL_CHAP_16_02
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
	case Chapter::k10_01: s_iModels = ModelHandler::loadModelForChap10_01(); break;
	case Chapter::k10_02: s_iModels = ModelHandler::loadModelForChap10_02(); break;
	case Chapter::k10_03: s_iModels = ModelHandler::loadModelForChap10_03(); break;
	case Chapter::k10_04: s_iModels = ModelHandler::loadModelForChap10_04(); break;
	case Chapter::k10_05: s_iModels = ModelHandler::loadModelForChap10_05(); break;
	case Chapter::k10_06: s_iModels = ModelHandler::loadModelForChap10_06(); break;
	case Chapter::k10_07: s_iModels = ModelHandler::loadModelForChap10_07(); break;
	case Chapter::k10_08: s_iModels = ModelHandler::loadModelForChap10_08(); break;
	case Chapter::k11_01: s_iModels = ModelHandler::loadModelForChap11_01(); break;
	case Chapter::k11_02: s_iModels = ModelHandler::loadModelForChap11_02(); break;
	case Chapter::k11_03: s_iModels = ModelHandler::loadModelForChap11_03(); break;
	case Chapter::k11_04: s_iModels = ModelHandler::loadModelForChap11_04(); break;
	case Chapter::k11_05: s_iModels = ModelHandler::loadModelForChap11_05(); break;
	case Chapter::k11_06: s_iModels = ModelHandler::loadModelForChap11_06(); break;
	case Chapter::k12_01: s_iModels = ModelHandler::loadModelForChap12_01(); break;
	case Chapter::k12_02: s_iModels = ModelHandler::loadModelForChap12_02(); break;
	case Chapter::k12_03: s_iModels = ModelHandler::loadModelForChap12_03(); break;
	case Chapter::k12_04: s_iModels = ModelHandler::loadModelForChap12_04(); break;
	case Chapter::k13_02: s_iModels = ModelHandler::loadModelForChap13_02(); break;
	case Chapter::k14_01: s_iModels = ModelHandler::loadModelForChap14_01(); break;
	case Chapter::k14_02: s_iModels = ModelHandler::loadModelForChap14_02(); break;
	case Chapter::k14_04: s_iModels = ModelHandler::loadModelForChap14_04(); break;
	case Chapter::k16_01: s_iModels = ModelHandler::loadModelForChap16_01(); break;
	case Chapter::k16_02: s_iModels = ModelHandler::loadModelForChap16_02(); break;
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
	case Chapter::k10_01: s_iModels->handleInput(); break;
	case Chapter::k10_02: s_iModels->handleInput(); break;
	case Chapter::k10_03: s_iModels->handleInput(); break;
	case Chapter::k10_04: s_iModels->handleInput(); break;
	case Chapter::k10_05: s_iModels->handleInput(); break;
	case Chapter::k10_06: s_iModels->handleInput(); break;
	case Chapter::k10_07: s_iModels->handleInput(); break;
	case Chapter::k10_08: s_iModels->handleInput(); break;
	case Chapter::k11_01: s_iModels->handleInput(); break;
	case Chapter::k11_02: s_iModels->handleInput(); break;
	case Chapter::k11_03: s_iModels->handleInput(); break;
	case Chapter::k11_04: s_iModels->handleInput(); break;
	case Chapter::k11_05: s_iModels->handleInput(); break;
	case Chapter::k11_06: s_iModels->handleInput(); break;
	case Chapter::k12_01: s_iModels->handleInput(); break;
	case Chapter::k12_02: s_iModels->handleInput(); break;
	case Chapter::k12_03: s_iModels->handleInput(); break;
	case Chapter::k12_04: s_iModels->handleInput(); break;
	case Chapter::k13_02: s_iModels->handleInput(); break;
	case Chapter::k14_01: s_iModels->handleInput(); break;
	case Chapter::k14_02: s_iModels->handleInput(); break;
	case Chapter::k14_04: s_iModels->handleInput(); break;
	case Chapter::k16_01: s_iModels->handleInput(); break;
	case Chapter::k16_02: s_iModels->handleInput(); break;
	default: break;
	}
}
