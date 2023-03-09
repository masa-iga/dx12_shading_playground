#include "model_07_02.h"
#include <filesystem>
#include <memory>
#include "imodel.h"
#include "MiniEngine.h"
#include "model_util.h"
#include "../debug_win.h"
#include "../imgui_if.h"
#include "../miniEngine_if.h"

class ModelFactory_07_02 : public IModelFactory
{
public:
	~ModelFactory_07_02() { }
	std::unique_ptr<IModels> create();
	void setCamera3D();
};

class Models_07_02 : public IModels
{
public:
	struct Light {
		Vector3 direction;
		float pad0 = 0.0f;
		Vector4 color;
		Vector3 eyePos;
		float specRow = 0.0f;
		Vector3 ambientLight;
		int32_t enableAmbientMap = 0;
	};

public:
	Models_07_02() { }
	~Models_07_02() { }
	void createModel();
	void handleInput();
	void debugRenderParams();
	void setLight(Light light) { m_light = light; }
	Light* getLightPtr() { return &m_light; }

private:
	const std::string kTkmFile = "Sample_06_03/Sample_06_03/Assets/modelData/sample.tkm";
	const std::string kFxFile = "Assets/shader/sample_06_03.fx";
	const std::wstring kAoFile = L"Sample_06_03/Sample_06_03/Assets/modelData/guitar_01_AO.dds";
	std::string getTkmFilePath() { return ModelUtil::getPathFromAssetDir(kTkmFile); }
	std::string getFxFilePath() { return kFxFile; }
	std::wstring getAoFilePath() { return ModelUtil::getPathFromAssetDirWstr(kAoFile); }

	Texture m_aoMap;
	Light m_light;
};

std::unique_ptr<IModels> ModelFactory_07_02::create()
{
	std::unique_ptr<Models_07_02> m(new Models_07_02);
	{
		Models_07_02::Light light = {
			.direction = { 1.0f, -1.0f, -1.0f },
			.pad0 = 0.0f,
			.color = { 0.0f, 0.0f, 0.0f, 0.0f },
			.eyePos = MiniEngineIf::getCamera3D()->GetPosition(),
			.specRow = 5.0f,
			.ambientLight = { 1.0f, 1.0f, 1.0f },
			.enableAmbientMap = 1,
		};
		m->setLight(light);

		m->createModel();

		for (Model* model : m->getModels())
		{
			model->UpdateWorldMatrix(g_vec3Zero, g_quatIdentity, g_vec3One);
		}
	}

	return m;
}

void ModelFactory_07_02::setCamera3D()
{
	MiniEngineIf::getCamera3D()->SetPosition({ 0.0f, 130.0f, 300.0f });
	MiniEngineIf::getCamera3D()->SetTarget({ 0.0f, 130.0f, 0.0f });
}

void Models_07_02::createModel()
{
	const std::string tkmFilePath = getTkmFilePath();
	const std::string fxFilePath = getFxFilePath();
    const std::wstring aoFilePath = getAoFilePath();
	Dbg::assert_(std::filesystem::exists(tkmFilePath));
	Dbg::assert_(std::filesystem::exists(fxFilePath));
	Dbg::assert_(std::filesystem::exists(aoFilePath));

	ModelInitData initData = { };
	{
		initData.m_tkmFilePath = tkmFilePath.c_str();
		initData.m_fxFilePath = fxFilePath.c_str();
		initData.m_expandConstantBuffer = &m_light;
		initData.m_expandConstantBufferSize = sizeof(m_light);
		m_aoMap.InitFromDDSFile(aoFilePath.c_str());
		initData.m_expandShaderResoruceView.at(0) = &m_aoMap;
	}

	std::unique_ptr<Model> model(new Model);
	model->Init(initData);
	m_models.push_back(std::move(model));
}

void Models_07_02::handleInput()
{
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

		qRot.Apply(m_light.direction);
	}

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
	}

	m_light.eyePos = MiniEngineIf::getCamera3D()->GetPosition();

	{
		if (MiniEngineIf::isPress(MiniEngineIf::Button::kA))
		{
			m_light.enableAmbientMap = 1;
		}
		else if (MiniEngineIf::isPress(MiniEngineIf::Button::kB))
		{
			m_light.enableAmbientMap = 0;
		}
	}
}

void Models_07_02::debugRenderParams()
{
	ImguiIf::printParams<float>(ImguiIf::VarType::kFloat, "Direct light", std::vector<float*>{ &(m_light.direction.x), & (m_light.direction.y), & (m_light.direction.z) });
	ImguiIf::printParams<float>(ImguiIf::VarType::kFloat, "Eye         ", std::vector<float*>{ &(m_light.eyePos.x), & (m_light.eyePos.y), & (m_light.eyePos.z) });
	ImguiIf::printParams<int32_t>(ImguiIf::VarType::kInt32, "SpecularMap ", std::vector<int32_t*>{ &(m_light.enableAmbientMap)});
}

namespace ModelHandler {
	static std::unique_ptr<IModels> m = nullptr;

	void loadModelForChap07_02(std::vector<Model*>& models)
	{
		ModelFactory_07_02 factory;
		factory.setCamera3D();
		m = factory.create();
		m->debugRenderParams();
		models = m->getModels();
	}

	void handleInputForChap07_02()
	{
		m->handleInput();
	}
} // namespace ModelHandler

