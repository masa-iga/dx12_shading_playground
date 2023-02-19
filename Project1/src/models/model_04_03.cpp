#include "model_04_03.h"
#include <filesystem>
#include "MiniEngine.h"
#include "model_util.h"
#include "../debug_win.h"
#include "../imgui_if.h"
#include "../miniEngine_if.h"

namespace {
	struct DirectionLight
	{
		Vector3 ligDirection;
		float pad0 = 0.0f;
		Vector3 ligColor;
		float pad1 = 0.0f;
		Vector3 eyePos;
		float pad2 = 0.0f;
	};
} // namespace anonymous

namespace ModelHandler {
	void loadModelForChap04_03(std::vector<Model*>& models)
	{
		static DirectionLight s_directionLig = {
			.ligDirection = { 1.0f, -1.0f, -1.0f},
			.ligColor = { 0.3f, 0.3f, 0.3f },
			.eyePos = g_camera3D->GetPosition(),
		};
		s_directionLig.ligDirection.Normalize();

		const std::string tkmFile = "Sample_04_02/Sample_04_02/Assets/modelData/teapot.tkm";
		const std::string fxFile = "Assets/shader/sample_04_02.fx";
		const std::string tkmFilePath = ModelUtil::getPathFromAssetDir(tkmFile);
		const std::string fxFilePath = fxFile;
		static Model s_model;
		ModelUtil::initModel(tkmFilePath, fxFilePath, &s_model, &s_directionLig, sizeof(s_directionLig));
		models.push_back(&s_model);
	}

	void handleInputForChap04_03()
	{
		;
	}
} // namespace ModelHandler
