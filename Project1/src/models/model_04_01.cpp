#include "model_04_01.h"
#include <filesystem>
#include "MiniEngine.h"
#include "model_util.h"
#include "../debug_win.h"
#include "../imgui_if.h"
#include "../miniEngine_if.h"

namespace {

} // namespace anonymous

namespace ModelHandler {
	void loadModelForChap04_01(std::vector<Model*>& models)
	{
		const std::string tkmFile = "Sample_04_01/Sample_04_01/Assets/modelData/sample.tkm";
		const std::string fxFile = "Assets/shader/sample_04_01.fx";
		const std::string tkmFilePath = ModelUtil::getPathFromAssetDir(tkmFile);
		const std::string fxFilePath = fxFile;
		static Model s_model;
		ModelUtil::initModel(tkmFilePath, fxFilePath, &s_model, nullptr, 0);
		models.push_back(&s_model);
	}

	void handleInputForChap04_01()
	{
		;
	}
} // namespace ModelHandler
