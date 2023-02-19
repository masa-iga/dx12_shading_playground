#include "model_05_02.h"
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

		Vector3 spPosition;
		float pad3 = 0.0f;
		Vector3 spColor;
		float spRange = 0.0f;
		Vector3 spDirection;
		float spAngle = 0.0f;

		Vector3 eyePos;
		float pad4 = 0.0f;

		Vector3 ambientLight;
		float pad5 = 0.0f;
	};

	Light* s_pLight = nullptr;
	Model* s_pModel = nullptr;
} // namespace anonymous

namespace ModelHandler {
	void loadModelForChap05_02(std::vector<Model*>& models)
	{
		MiniEngineIf::getCamera3D()->SetPosition({ 0.0f, 50.0f, 200.0f });
		MiniEngineIf::getCamera3D()->SetTarget({ 0.0f, 50.0f, 0.0f });

		static Light s_light = {
			.dirDirection = { 1.0f, -1.0f, -1.0f},
			.dirColor = { 0.5f, 0.5f, 0.5f },
			.ptPosition = { 0.0f, 50.0f, 50.0f },
			.ptColor = { 0.0f, 0.0f, 0.0f },
			.ptRange = 100.0f,
			.spPosition = { 0.0f, 50.0f, 0.0f },
			.spColor = { 10.0f, 10.0f, 10.0f },
			.spRange = 300.0f,
			.spDirection = { 1.0f, -1.0f, 1.0f },
			.spAngle = Math::DegToRad(25.0f),
			.eyePos = MiniEngineIf::getCamera3D()->GetPosition(),
			.ambientLight = { 0.3f, 0.3f, 0.3f },
		};
		s_light.dirDirection.Normalize();
		s_light.spDirection.Normalize();
		s_pLight = &s_light;

		{
			const std::string tkmFile = "Sample_05_02/Sample_05_02/Assets/modelData/bg.tkm";
			const std::string fxFile = "Assets/shader/sample_05_02.fx";
			const std::string tkmFilePath = ModelUtil::getPathFromAssetDir(tkmFile);
			const std::string fxFilePath = fxFile;
			static Model s_model;
			models.push_back(&s_model);
			ModelUtil::initModel(tkmFilePath, fxFilePath, &s_model, &s_light, sizeof(s_light));
		}

		{
			const std::string tkmFile = "Sample_05_02/Sample_05_02/Assets/modelData/light.tkm";
			const std::string fxFile = "Assets/shader/other/light.fx";
			const std::string tkmFilePath = ModelUtil::getPathFromAssetDir(tkmFile);
			const std::string fxFilePath = fxFile;
			static Model s_model;
			models.push_back(&s_model);
			ModelUtil::initModel(tkmFilePath, fxFilePath, &s_model, &s_light, sizeof(s_light));
			s_pModel = &s_model;
		}
	}

	void handleInputForChap05_02()
	{
		if (s_pLight == nullptr || s_pModel == nullptr)
			return;

		s_pLight->spPosition.x -= MiniEngineIf::getStick(MiniEngineIf::StickType::kLX);

		if (MiniEngineIf::isPress(MiniEngineIf::Button::kB))
		{
			s_pLight->spPosition.y += MiniEngineIf::getStick(MiniEngineIf::StickType::kLY);;
		}
		else
		{
			s_pLight->spPosition.z -= MiniEngineIf::getStick(MiniEngineIf::StickType::kLY);
		}

		{
			Quaternion qRotY;
			qRotY.SetRotationY(MiniEngineIf::getStick(MiniEngineIf::StickType::kRX) * 0.01f);
			qRotY.Apply(s_pLight->spDirection);

			Vector3 rotAxis;
			rotAxis.Cross(g_vec3AxisY, s_pLight->spDirection);

			Quaternion qRotX;
			qRotX.SetRotation(rotAxis, MiniEngineIf::getStick(MiniEngineIf::StickType::kRY) * 0.01f);
			qRotX.Apply(s_pLight->spDirection);
		}

		Quaternion qRot = Quaternion::Identity;
		qRot.SetRotation({ 0.0f, 0.0f, -1.0f }, s_pLight->spDirection);

		s_pModel->UpdateWorldMatrix(s_pLight->spPosition, qRot, g_vec3One);
	}
} // namespace ModelHandler