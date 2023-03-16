#include "model_06_02.h"
#include <filesystem>
#include "MiniEngine.h"
#include "model_util.h"
#include "../debug_win.h"
#include "../imgui_if.h"
#include "../miniEngine_if.h"

namespace {
	struct Light {
		Vector3 direction;
		float pad0 = 0.0f;
		Vector4 color;
		Vector3 eyePos;
		float specRow = 0.0f;
		Vector3 ambientLight;
		int32_t enableSpecularMap = 0;
	};

	const std::string tkmFile = "Sample_06_02/Sample_06_02/Assets/modelData/sample.tkm";
	const std::string fxFile = "Assets/shader/sample_06_02.fx";
	Light* s_pLight = nullptr;
}

namespace ModelHandler {
	void loadModelForChap06_02(std::vector<Model*>& models)
	{
		MiniEngineIf::getCamera3D()->SetPosition({ 0.0f, 150.0f, 300.0f });
		MiniEngineIf::getCamera3D()->SetTarget({ 0.0f, 150.0f, 0.0f });

		static Light s_light = {
			.direction = { 1.0f, -1.0f, -1.0f },
			.pad0 = 0.0f,
			.color = { 0.6f, 0.6f, 0.6f, 0.0f },
			.eyePos = MiniEngineIf::getCamera3D()->GetPosition(),
			.specRow = 5.0f,
			.ambientLight = { 0.4f, 0.4f, 0.4f },
			.enableSpecularMap = 1,
		};
		s_pLight = &s_light;

		{
			const std::string tkmFilePath = ModelUtil::getPathFromAssetDir(tkmFile);
			const std::string fxFilePath = fxFile;
			static Model s_model;
			ModelUtil::initModel(tkmFilePath, fxFilePath, &s_model, &s_light, sizeof(s_light));
			models.push_back(&s_model);
			s_model.UpdateWorldMatrix(g_vec3Zero, g_quatIdentity, g_vec3One);
		}

		{
			ImguiIf::printParams<float>(ImguiIf::VarType::kFloat, "Direct light", std::vector<const float*>{ &(s_pLight->direction.x), & (s_pLight->direction.y), & (s_pLight->direction.z) });
			ImguiIf::printParams<float>(ImguiIf::VarType::kFloat, "Eye         ", std::vector<const float*>{ &(s_pLight->eyePos.x), &(s_pLight->eyePos.y), &(s_pLight->eyePos.z) });
			ImguiIf::printParams<int32_t>(ImguiIf::VarType::kInt32, "SpecularMap ", std::vector<const int32_t*>{ &(s_pLight->enableSpecularMap)});
		}
	}

	void handleInputForChap06_02()
	{
		// rotate light
		{
			Quaternion qRot;

			if (MiniEngineIf::isPress(MiniEngineIf::Button::kRight))
			{
				qRot.SetRotationDegY(1.0f);
			}
			else if (MiniEngineIf::isPress(MiniEngineIf::Button::kLeft))
			{
				qRot.SetRotationDegY(-1.0f);
			}

			qRot.Apply(s_pLight->direction);
		}

		// rotate camera position
		{
			Quaternion qRot;
			qRot.SetRotationDegY(MiniEngineIf::getStick(MiniEngineIf::StickType::kLX));
			Vector3 camPos = MiniEngineIf::getCamera3D()->GetPosition();
			qRot.Apply(camPos);
			MiniEngineIf::getCamera3D()->SetPosition(camPos);

			Vector3 rotAxis;
			auto toPos = MiniEngineIf::getCamera3D()->GetPosition() - MiniEngineIf::getCamera3D()->GetTarget();
			auto dir = toPos;
			dir.Normalize();
			rotAxis.Cross(dir, g_vec3AxisY);
			qRot.SetRotationDeg(rotAxis, MiniEngineIf::getStick(MiniEngineIf::StickType::kLY));
			qRot.Apply(toPos);
			MiniEngineIf::getCamera3D()->SetPosition(MiniEngineIf::getCamera3D()->GetTarget() + toPos);

			s_pLight->eyePos = MiniEngineIf::getCamera3D()->GetPosition();
		}

		{
			if (MiniEngineIf::isPress(MiniEngineIf::Button::kA))
			{
				s_pLight->enableSpecularMap= 1;
			}
			else if (MiniEngineIf::isPress(MiniEngineIf::Button::kB))
			{
				s_pLight->enableSpecularMap= 0;
			}
		}
	}
} // namespace ModelHandler
