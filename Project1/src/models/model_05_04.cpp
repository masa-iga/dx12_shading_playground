#include "model_05_04.h"
#include <filesystem>
#include "MiniEngine.h"
#include "model_util.h"
#include "../debug_win.h"
#include "../imgui_if.h"
#include "../miniEngine_if.h"

namespace {
	struct Light
	{
		Vector3 dirDirection;
		float pad0 = 0.0f;
		Vector3 dirColor;
		float pad1 = 0.0f;

		Vector3 eyePos;
		float pad2 = 0.0f;

		Vector3 ambientLight;
		float pad3 = 0.0f;

		Vector3 groundColor;
		float pad4 = 0.0f;
		Vector3 skyColor;
		float pad5 = 0.0f;
		Vector3 groundNormal;
	};

	const std::string tkmFile = "Sample_05_04/Sample_05_04/Assets/modelData/teapot.tkm";
	const std::string fxFile = "Assets/shader/sample_05_04.fx";
	Light* s_pLight = nullptr;
} // namespace anonymous

namespace ModelHandler {
	void loadModelForChap05_04(std::vector<Model*>& models)
	{
		MiniEngineIf::getCamera3D()->SetPosition({ 0.0f, 50.0f, 200.0f });
		MiniEngineIf::getCamera3D()->SetTarget({ 0.0f, 50.0f, 0.0f });

		static Light s_light = {
			.dirDirection = { 0.0f, 0.0f, 1.0f },
			.pad0 = 0.0f,
			.dirColor = { 0.5f, 0.5f, 0.5f },
			.pad1 = 0.0f,
			.eyePos = MiniEngineIf::getCamera3D()->GetPosition(),
			.pad2 = 0.0f,
			.ambientLight = { 0.3f, 0.3f, 0.3f },
			.pad3 = 0.0f,
			.groundColor = { 0.7f, 0.5f, 0.3f },
			.pad4 = 0.0f,
			.skyColor = { 0.15f, 0.7f, 0.95f },
			.pad5 = 0.0f,
			.groundNormal = { 0.0f, 1.0f, 0.0f },
		};
		s_light.dirDirection.Normalize();
		s_pLight = &s_light;

		{
			const std::string tkmFilePath = ModelUtil::getPathFromAssetDir(tkmFile);
			const std::string fxFilePath = fxFile;
			static Model s_model;
			ModelUtil::initModel(tkmFilePath, fxFilePath, &s_model, &s_light, sizeof(s_light));
			models.push_back(&s_model);
			s_model.UpdateWorldMatrix({ 0.0f, 20.0f, 0.0f }, g_quatIdentity, g_vec3One);
		}
	}

	void handleInputForChap05_04()
	{
		if (s_pLight == nullptr)
			return;

		Quaternion qRotY;
		qRotY.SetRotation(g_vec3AxisY, MiniEngineIf::getStick(MiniEngineIf::StickType::kLX) * 0.02f);
		qRotY.Apply(s_pLight->dirDirection);
	}
} // namespace ModelHandler
