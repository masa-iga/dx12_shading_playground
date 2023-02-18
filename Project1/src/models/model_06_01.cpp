#include "model_06_01.h"
#include <filesystem>
#include "MiniEngine.h"
#include "model_util.h"
#include "../debug_win.h"
#include "../imgui_if.h"
#include "../miniEngine_if.h"
#include "../util.h"

namespace {
	struct Light {
		Vector3 direction;
		float pad0 = 0.0f;
		Vector4 color;
		Vector3 eyePos;
		float specRow = 0.0f;
		Vector3 ambientLight;
		float pad1 = 0.0f;
		int32_t enableTangentSpaceNormal = 0;
	};

	const std::string tkmFile = "Sample_06_01/Sample_06_01/Assets/modelData/sample.tkm";
	const std::string fxFile = "Assets/shader/sample_06_01.fx";
	Light* s_pLight = nullptr;
}

namespace ModelHandler {
	void loadModelForChap06_01(std::vector<Model*>& models)
	{
		MiniEngineIf::getCamera3D()->SetPosition({ 0.0f, 200.0f, 300.0f });
		MiniEngineIf::getCamera3D()->SetTarget({ 0.0f, 100.0f, 0.0f });

		static Light s_light = {
			.direction = { 1.0f, -1.0f, -1.0f },
			.pad0 = 0.0f,
			.color = { 0.6f, 0.6f, 0.6f, 0.0f },
			.eyePos = MiniEngineIf::getCamera3D()->GetPosition(),
			.specRow = 5.0f,
			.ambientLight = { 0.4f, 0.4f, 0.4f },
			.pad1 = 0.0f,
		};
		s_pLight = &s_light;

		{
			const std::string tkmFilePath = Util::getPathFromAssetDir(tkmFile);
			const std::string fxFilePath = fxFile;
			static Model s_model;
			ModelUtil::initModel(tkmFilePath, fxFilePath, &s_model, &s_light, sizeof(s_light));
			models.push_back(&s_model);
			s_model.UpdateWorldMatrix(g_vec3Zero, g_quatIdentity, g_vec3One);
		}

		{
			ImguiIf::printParams<float>(ImguiIf::ParamType::kFloat, "Direct light", std::vector<float*>{ &(s_pLight->direction.x), & (s_pLight->direction.y), & (s_pLight->direction.z) });
			ImguiIf::printParams<float>(ImguiIf::ParamType::kFloat, "Eye         ", std::vector<float*>{ &(s_pLight->eyePos.x), &(s_pLight->eyePos.y), &(s_pLight->eyePos.z) });
			ImguiIf::printParams<int32_t>(ImguiIf::ParamType::kInt32, "TangentNormal", std::vector<int32_t*>{ &s_pLight->enableTangentSpaceNormal });
		}
	}

	void handleInputForChap06_01()
	{
		if (s_pLight == nullptr)
			return;

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

		// rotate camera
		{
			Quaternion qRot;
			qRot.SetRotationDegY(MiniEngineIf::getStick(MiniEngineIf::StickType::kLX));
			Vector3 camPos = MiniEngineIf::getCamera3D()->GetPosition();
			qRot.Apply(camPos);
			MiniEngineIf::getCamera3D()->SetPosition(camPos);
		}

		{
			Vector3 toPos = MiniEngineIf::getCamera3D()->GetPosition() - MiniEngineIf::getCamera3D()->GetTarget();
			Vector3 dir = toPos;
			dir.Normalize();
			Vector3 rotAxis;
			rotAxis.Cross(dir, g_vec3AxisY);
			Quaternion qRot;
			qRot.SetRotationDeg(rotAxis, MiniEngineIf::getStick(MiniEngineIf::StickType::kLY));
			qRot.Apply(toPos);
			MiniEngineIf::getCamera3D()->SetPosition(MiniEngineIf::getCamera3D()->GetTarget() + toPos);

			s_pLight->eyePos = MiniEngineIf::getCamera3D()->GetPosition();
		}

		{
			if (MiniEngineIf::isPress(MiniEngineIf::Button::kA))
			{
				s_pLight->enableTangentSpaceNormal = 1;
			}
			else if (MiniEngineIf::isPress(MiniEngineIf::Button::kB))
			{
				s_pLight->enableTangentSpaceNormal = 0;
			}
		}
	}
} // namespace ModelHandler
