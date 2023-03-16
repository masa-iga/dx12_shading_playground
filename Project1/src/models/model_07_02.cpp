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
};

class Models_07_02 : public IModels
{
public:
	struct DirectionalLight {
		Vector3 direction;
		float pad0;
		Vector4 color;
	};

	struct Light {
		static constexpr uint32_t kNumDirectionalLight = 4;

		DirectionalLight directionalLight[kNumDirectionalLight];
		Vector3 eyePos;
		float specPow;
		Vector3 ambientLight;
		float pad0;
	};

public:
	Models_07_02() { }
	~Models_07_02() { }
	void createModel();
	void resetCamera();
	void handleInput();
	void setLight(const Light& light) { m_light = light; }
	Light* getLightPtr() { return &m_light; }
	void draw(RenderContext& renderContext);
	void debugRenderParams();

private:
	const std::string kTkmMonsterFile = "Sample_07_02/Sample_07_02/Assets/modelData/monster.tkm";
	const std::string kTkmHumanFile = "Sample_07_02/Sample_07_02/Assets/modelData/Human.tkm";
	const std::string kTkmLanternFile = "Sample_07_02/Sample_07_02/Assets/modelData/Lantern.tkm";
	const std::string kFxFile = "Assets/shader/sample_07_02.fx";
	std::string getTkmMonsterFilePath() { return ModelUtil::getPathFromAssetDir(kTkmMonsterFile); }
	std::string getTkmHumanFile() { return ModelUtil::getPathFromAssetDir(kTkmHumanFile); }
	std::string getTkmLanternFile() { return ModelUtil::getPathFromAssetDir(kTkmLanternFile); }
	std::string getFxFilePath() { return kFxFile; }

	struct DispSetting
	{
		Vector3 cameraPos;
		Vector3 cameraTarget;
	};

	enum class Character : uint32_t
	{
		kMonster,
		kHuman,
		kLantern,
		kNum,
	};

	const DispSetting kDispSettings[static_cast<uint32_t>(Character::kNum)] =
	{
		{
			.cameraPos = { 0.0f, 85.0f, 50.0f },
			.cameraTarget = { 0.0f, 85.0f, 0.0f },
		},
		{
			.cameraPos = { 0.0f, 160.0f, 50.0f },
			.cameraTarget = { 0.0f, 160.0f, 0.0f },
		},
		{
			.cameraPos = { 0.0f, 50.0f, 120.0f },
			.cameraTarget = { 0.0f, 50.0f, 0.0f },
		},
	};

	Character m_dispModelNo = Character::kMonster;
	Light m_light;
};

std::unique_ptr<IModels> ModelFactory_07_02::create()
{
	std::unique_ptr<Models_07_02> m(new Models_07_02);
	{
		{
			Models_07_02::Light light = {
				.directionalLight = {
					{
						.direction = { 2.0f, -1.0f, 3.0f},
						.pad0 = 0.0f,
						.color = { 3.0f, 3.0f, 3.0f, 0.0f },
					},
					{
						.direction = { 0.0f, 0.0f, -1.0f},
						.pad0 = 0.0f,
						.color = { 1.5f, 1.5f, 1.5f, 0.0f },
					},
					{
						.direction = { 0.0f, 0.0f, 0.0f},
						.pad0 = 0.0f,
						.color = { 0.0f, 0.0f, 0.0f, 0.0f },
					},
					{
						.direction = { 0.0f, 0.0f, 0.0f},
						.pad0 = 0.0f,
						.color = { 0.0f, 0.0f, 0.0f, 0.0f },
					},
				},
				.eyePos = MiniEngineIf::getCamera3D()->GetPosition(),
				.specPow = 0.5f,
				.ambientLight = { 0.4f, 0.4f, 0.4f },
				.pad0 = 0.0f,
			};

			for (auto& dirLight : light.directionalLight)
			{
				dirLight.direction.Normalize();
			}

			m->setLight(light);
		}

		m->createModel();

		for (Model* model : m->getModels())
		{
			model->UpdateWorldMatrix(g_vec3Zero, g_quatIdentity, g_vec3One);
		}

		m->resetCamera();
	}

	return std::move(m);
}

void Models_07_02::resetCamera()
{
	MiniEngineIf::getCamera3D()->SetPosition(kDispSettings[static_cast<int32_t>(m_dispModelNo)].cameraPos);
	MiniEngineIf::getCamera3D()->SetTarget(kDispSettings[static_cast<int32_t>(m_dispModelNo)].cameraTarget);
}

void Models_07_02::createModel()
{
	const std::string tkmMonsterFilePath = getTkmMonsterFilePath();
	const std::string tkmHumanFilePath = getTkmHumanFile();
	const std::string tkmLanternFilePath = getTkmLanternFile();
	const std::string fxFilePath = getFxFilePath();
	Dbg::assert_(std::filesystem::exists(tkmMonsterFilePath));
	Dbg::assert_(std::filesystem::exists(fxFilePath));

	ModelInitData initData = { };
	{
		initData.m_tkmFilePath = tkmMonsterFilePath.c_str();
		initData.m_fxFilePath = fxFilePath.c_str();
		initData.m_expandConstantBuffer = &m_light;
		initData.m_expandConstantBufferSize = sizeof(m_light);
	}

	{
		std::unique_ptr<Model> model(new Model);
		model->Init(initData);
		m_models.emplace_back(std::move(model));
	}

	{
		initData.m_tkmFilePath = tkmHumanFilePath.c_str();
		std::unique_ptr<Model> model(new Model);
		model->Init(initData);
		m_models.emplace_back(std::move(model));
	}

	{
		initData.m_tkmFilePath = tkmLanternFilePath.c_str();
		std::unique_ptr<Model> model(new Model);
		model->Init(initData);
		m_models.emplace_back(std::move(model));
	}
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

		for (auto& lig : m_light.directionalLight)
		{
			qRot.Apply(lig.direction);
		}
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
		if (MiniEngineIf::isTrigger(MiniEngineIf::Button::kA))
		{
			m_dispModelNo = static_cast<Character>((static_cast<int32_t>(m_dispModelNo) + 1) % static_cast<int32_t>(Character::kNum));
			resetCamera();
		}
	}
}

void Models_07_02::draw(RenderContext& renderContext)
{
	m_models.at(static_cast<size_t>(m_dispModelNo))->Draw(renderContext);
}

void Models_07_02::debugRenderParams()
{
	ImguiIf::printParams<int32_t>(ImguiIf::VarType::kInt32, "Camera", std::vector<const int32_t*>{ (int32_t*)&m_dispModelNo });
	ImguiIf::printParams<float>(ImguiIf::VarType::kFloat, "Eye         ", std::vector<const float*>{ &(m_light.eyePos.x), & (m_light.eyePos.y), & (m_light.eyePos.z) });

	for (uint32_t i = 0; i < _countof(m_light.directionalLight); ++i)
	{
		DirectionalLight& dl = m_light.directionalLight[static_cast<size_t>(i)];
		std::string str = "Direct light" + std::to_string(i);
		ImguiIf::printParams<float>(ImguiIf::VarType::kFloat, str, std::vector<const float*>{ &(dl.direction.x), & (dl.direction.y), & (dl.direction.z) });
	}
}

namespace ModelHandler {
	std::unique_ptr<IModels> loadModelForChap07_02()
	{
		ModelFactory_07_02 factory;
		std::unique_ptr<IModels> iModels = factory.create();
		iModels->debugRenderParams();
		return std::move(iModels);
	}
} // namespace ModelHandler

