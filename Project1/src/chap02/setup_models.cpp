#include "setup_models.h"
#include <cstring>
#include <filesystem>
#include "../../import/hlsl-grimoire-sample/MiniEngine/MiniEngine.h"
#include "debug_win.h"

#define LOAD_MODEL_CHAP_04_01 (0)
#define LOAD_MODEL_CHAP_04_03 (0)
#define LOAD_MODEL_CHAP_05_01 (0)
#define LOAD_MODEL_CHAP_05_02 (0)
#define LOAD_MODEL_CHAP_05_03 (0)
#define LOAD_MODEL_CHAP_05_04 (1)

namespace {
	const std::string kBaseAssetDir = "../../import/hlsl-grimoire-sample";

	constexpr std::string getPathFromAssetDir(const std::string path);
	void initModel(const std::string& tkmFilePath, const std::string& fxFilePath, Model* model, void* constantBuffer, size_t constantBufferSize);
	void loadModelForChap04_01();
	void loadModelForChap04_03();
	void loadModelForChap05_01();
	void loadModelForChap05_02();
	void loadModelForChap05_03();
	void loadModelForChap05_04();
	void handleInputForChap05_01();
	void handleInputForChap05_02();
	void handleInputForChap05_03();
	void handleInputForChap05_04();

	std::vector<Model*> s_models;

	Model* s_updateModel = nullptr;
	Vector3* s_updateLightPos = nullptr;
	Vector3* s_updateLightDirection = nullptr;
}

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
	case Chapter::k04_03: loadModelForChap04_03(); break;
	case Chapter::k05_01: loadModelForChap05_01(); break;
	case Chapter::k05_02: loadModelForChap05_02(); break;
	case Chapter::k05_03: loadModelForChap05_03(); break;
	case Chapter::k05_04: loadModelForChap05_04(); break;
	default: break;
	}
}

void Models::handleInputInternal(Chapter chapter)
{
	switch (chapter) {
	case Chapter::k04_01: break;
	case Chapter::k04_03: break;
	case Chapter::k05_01: handleInputForChap05_01(); break;
	case Chapter::k05_02: handleInputForChap05_02(); break;
	case Chapter::k05_03: handleInputForChap05_03(); break;
	case Chapter::k05_04: handleInputForChap05_04(); break;
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

	void loadModelForChap04_03()
	{
		struct DirectionLight
		{
			Vector3 ligDirection;
			float pad0 = 0.0f;
			Vector3 ligColor;
			float pad1 = 0.0f;
			Vector3 eyePos;
			float pad2 = 0.0f;
		};

		static DirectionLight s_directionLig = {
			.ligDirection = { 1.0f, -1.0f, -1.0f},
			.ligColor = { 0.3f, 0.3f, 0.3f },
			.eyePos = g_camera3D->GetPosition(),
		};
		s_directionLig.ligDirection.Normalize();

		const std::string tkmFile = "Sample_04_02/Sample_04_02/Assets/modelData/teapot.tkm";
		const std::string fxFile = "Assets/shader/sample_04_02.fx";
		const std::string tkmFilePath = getPathFromAssetDir(tkmFile);
		const std::string fxFilePath = fxFile;
		static Model s_model;
		initModel(tkmFilePath, fxFilePath, &s_model, &s_directionLig, sizeof(s_directionLig));
	}

	void loadModelForChap05_01()
	{
		g_camera3D->SetPosition({ 0.0f, 50.0f, 200.0f });
		g_camera3D->SetTarget({ 0.0f, 50.0f, 0.0f });

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
		s_updateLightPos = &s_light.ptPosition;

		{
			const std::string tkmFile = "Sample_05_01/Sample_05_01/Assets/modelData/teapot.tkm";
			const std::string fxFile = "Assets/shader/sample_05_01.fx";
			const std::string tkmFilePath = getPathFromAssetDir(tkmFile);
			const std::string fxFilePath = fxFile;
			static Model s_model;
			initModel(tkmFilePath, fxFilePath, &s_model, &s_light, sizeof(s_light));
			s_model.UpdateWorldMatrix({ 0.0f, 20.0f, 0.0f }, g_quatIdentity, g_vec3One);
		}

		{
			const std::string tkmFile = "Sample_05_01/Sample_05_01/Assets/modelData/bg.tkm";
			const std::string fxFile = "Assets/shader/sample_05_01.fx";
			const std::string tkmFilePath = getPathFromAssetDir(tkmFile);
			const std::string fxFilePath = fxFile;
			static Model s_model;
			initModel(tkmFilePath, fxFilePath, &s_model, &s_light, sizeof(s_light));
		}

		{
			const std::string tkmFile = "Sample_05_01/Sample_05_01/Assets/modelData/light.tkm";
			const std::string fxFile = "Assets/shader/other/light.fx";
			const std::string tkmFilePath = getPathFromAssetDir(tkmFile);
			const std::string fxFilePath = fxFile;
			static Model s_model;
			initModel(tkmFilePath, fxFilePath, &s_model, &s_light, sizeof(s_light));
			s_updateModel = &s_model;
		}
	}

	void loadModelForChap05_02()
	{
		g_camera3D->SetPosition({ 0.0f, 50.0f, 200.0f });
		g_camera3D->SetTarget({ 0.0f, 50.0f, 0.0f });

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
			.eyePos = g_camera3D->GetPosition(),
			.ambientLight = { 0.3f, 0.3f, 0.3f },
		};
		s_light.dirDirection.Normalize();
		s_light.spDirection.Normalize();
		s_updateLightDirection = &s_light.spDirection;
		s_updateLightPos = &s_light.spPosition;

		{
			const std::string tkmFile = "Sample_05_02/Sample_05_02/Assets/modelData/bg.tkm";
			const std::string fxFile = "Assets/shader/sample_05_02.fx";
			const std::string tkmFilePath = getPathFromAssetDir(tkmFile);
			const std::string fxFilePath = fxFile;
			static Model s_model;
			initModel(tkmFilePath, fxFilePath, &s_model, &s_light, sizeof(s_light));
		}

		{
			const std::string tkmFile = "Sample_05_02/Sample_05_02/Assets/modelData/light.tkm";
			const std::string fxFile = "Assets/shader/other/light.fx";
			const std::string tkmFilePath = getPathFromAssetDir(tkmFile);
			const std::string fxFilePath = fxFile;
			static Model s_model;
			initModel(tkmFilePath, fxFilePath, &s_model, &s_light, sizeof(s_light));
			s_updateModel = &s_model;
		}
	}

	void loadModelForChap05_03()
	{
		g_camera3D->SetPosition({ 0.0f, 30.0f, 200.0f });
		g_camera3D->SetTarget({ 0.0f, 30.0f, 0.0f });

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
		};

		static Light s_light = {
			.dirDirection = { 0.0f, 0.0f, 1.0f },
			.dirColor = { 0.5f, 0.5f, 0.5f},
			.eyePos = g_camera3D->GetPosition(),
			.ambientLight = { 0.3f, 0.3f, 0.3f },
		};
		s_light.dirDirection.Normalize();
		s_updateLightDirection = &s_light.dirDirection;

		{
			const std::string tkmFile = "Sample_05_03/Sample_05_03/Assets/modelData/teapot.tkm";
			const std::string fxFile = "Assets/shader/sample_05_03.fx";
			const std::string tkmFilePath = getPathFromAssetDir(tkmFile);
			const std::string fxFilePath = fxFile;
			static Model s_model;
			initModel(tkmFilePath, fxFilePath, &s_model, &s_light, sizeof(s_light));
			s_model.UpdateWorldMatrix({ 0.0f, 20.0f, 0.0f }, g_quatIdentity, g_vec3One);
		}
	}

	void loadModelForChap05_04()
	{
		g_camera3D->SetPosition({ 0.0f, 50.0f, 200.0f });
		g_camera3D->SetTarget({ 0.0f, 50.0f, 0.0f });

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

		static Light s_light = {
			.dirDirection = { 0.0f, 0.0f, 1.0f },
			.pad0 = 0.0f,
			.dirColor = { 0.5f, 0.5f, 0.5f },
			.pad1 = 0.0f,
			.eyePos = g_camera3D->GetPosition(),
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
		s_updateLightDirection = &s_light.dirDirection;

		{
			const std::string tkmFile = "Sample_05_04/Sample_05_04/Assets/modelData/teapot.tkm";
			const std::string fxFile = "Assets/shader/sample_05_04.fx";
			const std::string tkmFilePath = getPathFromAssetDir(tkmFile);
			const std::string fxFilePath = fxFile;
			static Model s_model;
			initModel(tkmFilePath, fxFilePath, &s_model, &s_light, sizeof(s_light));
			s_model.UpdateWorldMatrix({ 0.0f, 20.0f, 0.0f }, g_quatIdentity, g_vec3One);
		}
	}

	void handleInputForChap05_01()
	{
		if (!s_updateLightPos || !s_updateModel)
			return;

		s_updateLightPos->x -= g_pad[0]->GetLStickXF();

		if (g_pad[0]->IsPress(enButtonB))
		{
			s_updateLightPos->y += g_pad[0]->GetLStickYF();
		}
		else
		{
			s_updateLightPos->z -= g_pad[0]->GetLStickYF();
		}

		s_updateModel->UpdateWorldMatrix(*s_updateLightPos, g_quatIdentity, g_vec3One);
	}

	void handleInputForChap05_02()
	{
		if (!s_updateLightDirection || !s_updateLightPos || !s_updateModel)
			return;

		s_updateLightPos->x -= g_pad[0]->GetLStickXF();

		if (g_pad[0]->IsPress(enButtonB))
		{
			s_updateLightPos->y += g_pad[0]->GetLStickYF();
		}
		else
		{
			s_updateLightPos->z -= g_pad[0]->GetLStickYF();
		}

		{
			Quaternion qRotY;
			qRotY.SetRotationY(g_pad[0]->GetRStickXF() * 0.01f);
			qRotY.Apply(*s_updateLightDirection);

			Vector3 rotAxis;
			rotAxis.Cross(g_vec3AxisY, *s_updateLightDirection);

			Quaternion qRotX;
			qRotX.SetRotation(rotAxis, g_pad[0]->GetRStickYF() * 0.01f);
			qRotX.Apply(*s_updateLightDirection);
		}

		Quaternion qRot = Quaternion::Identity;
		qRot.SetRotation({ 0.0f, 0.0f, -1.0f }, *s_updateLightDirection);

		s_updateModel->UpdateWorldMatrix(*s_updateLightPos, qRot, g_vec3One);
	}

	void handleInputForChap05_03()
	{
		if (!s_updateLightDirection)
			return;

		Quaternion qRotY;
		qRotY.SetRotation(g_vec3AxisY, g_pad[0]->GetLStickXF() * 0.02f);
		qRotY.Apply(*s_updateLightDirection);
	}

	void handleInputForChap05_04()
	{
		if (!s_updateLightDirection)
			return;

		Quaternion qRotY;
		qRotY.SetRotation(g_vec3AxisY, g_pad[0]->GetLStickXF() * 0.02f);
		qRotY.Apply(*s_updateLightDirection);
	}
}
