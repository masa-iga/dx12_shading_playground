#include "model_05_01.h"
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

		Vector3 ptPosition;
		float pad2 = 0.0f;
		Vector3 ptColor;
		float ptRange = 0.0f;

		Vector3 eyePos;
		float pad3 = 0.0f;

		Vector3 ambientLight;
		float pad4 = 0.0f;
	};

	Light* s_pLight = nullptr;
	Model* s_pModel = nullptr;
} // namespace anonymous

namespace ModelHandler {
	void loadModelForChap05_01(std::vector<Model*>& models)
	{
		MiniEngineIf::getCamera3D()->SetPosition({ 0.0f, 50.0f, 200.0f });
		MiniEngineIf::getCamera3D()->SetTarget({ 0.0f, 50.0f, 0.0f });

		static Light s_light = {
			.dirDirection = { 1.0f, -1.0f, -1.0f},
			.dirColor = { 0.5f, 0.5f, 0.5f },
			.ptPosition = { 0.0f, 50.0f, 50.0f },
			.ptColor = { 15.0f, 0.0f, 0.0f },
			.ptRange = 100.0f,
			.eyePos = g_camera3D->GetPosition(),
			.ambientLight = { 0.3f, 0.3f, 0.3f },
		};
		s_light.dirDirection.Normalize();
		s_pLight = &s_light;

		{
			const std::string tkmFile = "Sample_05_01/Sample_05_01/Assets/modelData/teapot.tkm";
			const std::string fxFile = "Assets/shader/sample_05_01.fx";
			const std::string tkmFilePath = ModelUtil::getPathFromAssetDir(tkmFile);
			const std::string fxFilePath = fxFile;
			static Model s_model;
			ModelUtil::initModel(tkmFilePath, fxFilePath, &s_model, &s_light, sizeof(s_light));
			models.push_back(&s_model);
			s_model.UpdateWorldMatrix({ 0.0f, 20.0f, 0.0f }, g_quatIdentity, g_vec3One);
		}

		{
			const std::string tkmFile = "Sample_05_01/Sample_05_01/Assets/modelData/bg.tkm";
			const std::string fxFile = "Assets/shader/sample_05_01.fx";
			const std::string tkmFilePath = ModelUtil::getPathFromAssetDir(tkmFile);
			const std::string fxFilePath = fxFile;
			static Model s_model;
			ModelUtil::initModel(tkmFilePath, fxFilePath, &s_model, &s_light, sizeof(s_light));
			models.push_back(&s_model);
		}

		{
			const std::string tkmFile = "Sample_05_01/Sample_05_01/Assets/modelData/light.tkm";
			const std::string fxFile = "Assets/shader/other/light.fx";
			const std::string tkmFilePath = ModelUtil::getPathFromAssetDir(tkmFile);
			const std::string fxFilePath = fxFile;
			static Model s_model;
			ModelUtil::initModel(tkmFilePath, fxFilePath, &s_model, &s_light, sizeof(s_light));
			models.push_back(&s_model);
			s_pModel = &s_model;
		}
	}

	void handleInputForChap05_01()
	{
		if (s_pLight == nullptr || s_pModel == nullptr)
			return;

		s_pLight->ptPosition.x -= MiniEngineIf::getStick(MiniEngineIf::StickType::kLX);

		if (MiniEngineIf::isPress(MiniEngineIf::Button::kB))
		{
			s_pLight->ptPosition.y += MiniEngineIf::getStick(MiniEngineIf::StickType::kLY);
		}
		else
		{
			s_pLight->ptPosition.z -= MiniEngineIf::getStick(MiniEngineIf::StickType::kLY);;
		}

		s_pModel->UpdateWorldMatrix(s_pLight->ptPosition, g_quatIdentity, g_vec3One);
	}
} // namespace ModelHandler
