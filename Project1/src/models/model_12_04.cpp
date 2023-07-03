#include "model_12_04.h"
#include <filesystem>
#include <memory>
#include "imodel.h"
#include "MiniEngine.h"
#include "model_util.h"
#include "../config.h"
#include "../debug_win.h"
#include "../imgui_if.h"
#include "../miniEngine_if.h"
#include "../winmgr_win.h"
#include <../Sample_11_06/Sample_11_06/ModelStandard.h>
#pragma warning(3 : 4189)

class ModelFactory_12_04 : public IModelFactory
{
public:
	~ModelFactory_12_04() { }
	std::unique_ptr<IModels> create();
};

class Obserber_12_04 : public WinMgr::Iobserber
{
public: 
	void update(WPARAM wParam, LPARAM lParam) override;
	bool isPaused() const { return m_paused; }

private:
	bool m_paused = false;
};

class Models_12_04 : public IModels
{
public:
	Models_12_04() { }
	~Models_12_04()
	{
		WinMgr::removeObserver(&m_obserber);
	}
	void createModel();
	void addObserver();
	void removeObserver();
	void resetCamera();
	void handleInput();
	void draw(RenderContext& renderContext);
	void debugRenderParams();

private:
	struct DirectionalLight
	{
		Vector3 m_color;
		float m_pad0 = 0.0f;
		Vector3 m_direction;
		float m_pad1 = 0.0f;
		Vector3 m_eyePos;
		float specPow= 0.0f;
	};

	static constexpr size_t kGbufferWidth = 1920;
	static constexpr size_t kGbufferHeight = 1080;

	const std::string kTkmSampleFile = "Sample_12_04/Sample_12_04/Assets/modelData/sample.tkm";
	std::string getTkmSampleFilePath() { return ModelUtil::getPathFromAssetDir(kTkmSampleFile); }
	const std::string kFxModelFile = "./Assets/shader/sample_12_04_model.fx";
	std::string getFxModelFilePath() { return kFxModelFile; }
	const std::string kFxSpriteFile = "./Assets/shader/sample_12_04_sprite.fx";
	std::string getFxSpriteFilePath() { return kFxSpriteFile; }
	const std::string kFxPresetSpriteFile = "Sample_11_06/Sample_11_06/Assets/shader/preset/sprite.fx";
	std::string getFxPresetSpriteFilePath() { return ModelUtil::getPathFromAssetDir(kFxPresetSpriteFile); }

	Obserber_12_04 m_obserber;
	DirectionalLight m_light;
	std::unique_ptr<Model> m_model = nullptr;
	RenderTarget m_albedRt;
	RenderTarget m_normalRt;
	RenderTarget m_worldPosRt;
	std::unique_ptr<Sprite> m_defferedLightingSprite = nullptr;
	std::array<std::unique_ptr<Sprite>, 3> m_debugSprites = { nullptr, nullptr, nullptr };
};

std::unique_ptr<IModels> ModelFactory_12_04::create()
{
	std::unique_ptr<Models_12_04> m = std::make_unique<Models_12_04>();
	{
		m->createModel();
		m->addObserver();
	}
	return std::move(m);
}

void Obserber_12_04::update(WPARAM wParam, [[maybe_unused]] LPARAM lParam)
{
	switch (wParam) {
	case VK_SPACE:
		m_paused = !m_paused;
		break;
	default:
		break;
	}
}

void Models_12_04::resetCamera()
{
	MiniEngineIf::getCamera3D()->SetPosition({ 0.0f, 80.0f, 200.0f });
	MiniEngineIf::getCamera3D()->SetTarget({ 0.0f, 80.0f, 0.0f });
	MiniEngineIf::getCamera3D()->Update();
}

void Models_12_04::createModel()
{
	// create Gbuffer
	{
		bool bRet = m_albedRt.Create(
			kGbufferWidth,
			kGbufferHeight,
			1 /* mipLevel */,
			1 /* arraySize */,
			DXGI_FORMAT_R8G8B8A8_UNORM,
			DXGI_FORMAT_D32_FLOAT);
		Dbg::assert_(bRet);
	}
	{
		bool bRet = m_normalRt.Create(
			kGbufferWidth,
			kGbufferHeight,
			1 /* mipLevel */,
			1 /* arraySize */,
			DXGI_FORMAT_R8G8B8A8_UNORM,
			DXGI_FORMAT_UNKNOWN);
		Dbg::assert_(bRet);
	}
	{
		bool bRet = m_worldPosRt.Create(
			kGbufferWidth,
			kGbufferHeight,
			1 /* mipLevel */,
			1 /* arraySize */,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			DXGI_FORMAT_UNKNOWN);
		Dbg::assert_(bRet);
	}

	{
		m_light.m_direction = { 1.0f, 0.0f, 0.0f };
		m_light.m_color = { 1.0f, 1.0f, 1.0f };
		m_light.m_eyePos = MiniEngineIf::getCamera3D()->GetPosition();
	}

	const std::string tkmSampleFilePath = getTkmSampleFilePath();
	Dbg::assert_(std::filesystem::exists(tkmSampleFilePath));
	const std::string fxModelFilePath = getFxModelFilePath();
	Dbg::assert_(std::filesystem::exists(fxModelFilePath));
	const std::string fxSpriteFilePath = getFxSpriteFilePath();
	Dbg::assert_(std::filesystem::exists(fxSpriteFilePath));
	const std::string fxPresetSpriteFilePath = getFxPresetSpriteFilePath();
	Dbg::assert_(std::filesystem::exists(fxPresetSpriteFilePath));

	{
		ModelInitData d;
		d.m_tkmFilePath = tkmSampleFilePath.c_str();
		d.m_fxFilePath = fxModelFilePath.c_str();
		d.m_colorBufferFormat.at(0) = m_albedRt.GetColorBufferFormat();
		d.m_colorBufferFormat.at(1) = m_normalRt.GetColorBufferFormat();
		d.m_colorBufferFormat.at(2) = m_worldPosRt.GetColorBufferFormat();

		m_model = std::make_unique<Model>();
		m_model->Init(d);
	}
	{
		SpriteInitData d;
		d.m_width = kGbufferWidth;
		d.m_height = kGbufferHeight;
		d.m_textures.at(0) = &m_albedRt.GetRenderTargetTexture();
		d.m_textures.at(1) = &m_normalRt.GetRenderTargetTexture();
		d.m_textures.at(2) = &m_worldPosRt.GetRenderTargetTexture();
		d.m_fxFilePath = fxSpriteFilePath.c_str();
		d.m_expandConstantBuffer = &m_light;
		d.m_expandConstantBufferSize = sizeof(m_light);

		m_defferedLightingSprite = std::make_unique<Sprite>();
		m_defferedLightingSprite->Init(d);
	}

	{
		SpriteInitData d;
		d.m_width = 256;
		d.m_height = 256;
		d.m_fxFilePath = fxPresetSpriteFilePath.c_str();

		d.m_textures.at(0) = &m_albedRt.GetRenderTargetTexture();
		m_debugSprites.at(0) = std::make_unique<Sprite>();
		m_debugSprites.at(0)->Init(d);

		d.m_textures.at(0) = &m_normalRt.GetRenderTargetTexture();
		m_debugSprites.at(1) = std::make_unique<Sprite>();
		m_debugSprites.at(1)->Init(d);

		d.m_textures.at(0) = &m_worldPosRt.GetRenderTargetTexture();
		m_debugSprites.at(2) = std::make_unique<Sprite>();
		m_debugSprites.at(2)->Init(d);
	}
}

void Models_12_04::addObserver()
{
	WinMgr::addObserver(&m_obserber);
}

void Models_12_04::removeObserver()
{
	WinMgr::removeObserver(&m_obserber);
}

void Models_12_04::handleInput()
{
	if (!WinMgr::isWindowActive(WinMgr::Handle::kMain))
		return;

	if (m_obserber.isPaused())
		return;

	// move camera
	{
		using namespace MiniEngineIf;
		Vector3 pos = getCamera3D()->GetPosition();
		Vector3 target = getCamera3D()->GetTarget();
		{
			pos.z -= getStick(StickType::kLY) * 2.0f;
			target.z -= getStick(StickType::kLY) * 2.0f;
			pos.y += getStick(StickType::kRY) * 2.0f;
			target.y += getStick(StickType::kRY) * 2.0f;
			pos.x -= getStick(StickType::kLX) * 2.0f;
			target.x -= getStick(StickType::kLX) * 2.0f;
		}
		getCamera3D()->SetPosition(pos);
		getCamera3D()->SetTarget(target);

		m_light.m_eyePos = MiniEngineIf::getCamera3D()->GetPosition();
	}

	// rotate directional light
	{
		Quaternion rotLig;
		rotLig.SetRotationDegY(1.0f);
		rotLig.Apply(m_light.m_direction);
	}
}

void Models_12_04::draw(RenderContext& renderContext)
{
	// render to Gbuffer
	{
		RenderTarget* rts[] = {
			&m_albedRt,
			&m_normalRt,
			&m_worldPosRt,
		};

		renderContext.WaitUntilToPossibleSetRenderTargets(3, rts);

		renderContext.SetRenderTargets(3, rts);
		renderContext.ClearRenderTargetViews(3, rts);

		m_model->Draw(renderContext);

		renderContext.WaitUntilFinishDrawingToRenderTargets(3, rts);
	}

	// render to sprite
	{
		MiniEngineIf::setOffscreenRenderTarget();

		m_defferedLightingSprite->Draw(renderContext);
	}

	// render each Gbuffer for debugging
	for (uint32_t i = 0; i < m_debugSprites.size(); ++i)
	{
		const float posy = (i * (m_albedRt.GetHeight() / 4.0f)) + 1.0f * i;

		m_debugSprites.at(i)->Update(
			{ Config::kRenderTargetWidth / -2.0f, posy, 0.0f},
			g_quatIdentity,
			g_vec3One,
			{ 0.0f, 1.0f }
		);
		m_debugSprites.at(i)->Draw(renderContext);
	}
}

void Models_12_04::debugRenderParams()
{
	{
		{
			const Vector3& v = MiniEngineIf::getCamera3D()->GetPosition();
			ImguiIf::printParams<float>(ImguiIf::VarType::kFloat, "CameraPos", std::vector<const float*>{ &v.x, &v.y, &v.z });
		}
		{
			const Vector3& v = MiniEngineIf::getCamera3D()->GetTarget();
			ImguiIf::printParams<float>(ImguiIf::VarType::kFloat, "CameraTgt", std::vector<const float*>{ &v.x, &v.y, &v.z });
		}
		{
			const Vector3& v = MiniEngineIf::getCamera3D()->GetUp();
			ImguiIf::printParams<float>(ImguiIf::VarType::kFloat, "CameraUp", std::vector<const float*>{ &v.x, &v.y, &v.z });
		}
		{
			const Vector3& v = m_light.m_color;
			ImguiIf::printParams<float>(ImguiIf::VarType::kFloat, "LightColor", std::vector<const float*>{ &v.x, &v.y, &v.z });
		}
		{
			const Vector3& v = m_light.m_direction;
			ImguiIf::printParams<float>(ImguiIf::VarType::kFloat, "LightDirection", std::vector<const float*>{ &v.x, &v.y, &v.z });
		}
	}
}

namespace ModelHandler {
	std::unique_ptr<IModels> loadModelForChap12_04()
	{
		ModelFactory_12_04 factory;
		std::unique_ptr<IModels> iModels = factory.create();
		iModels->resetCamera();
		iModels->debugRenderParams();
		return std::move(iModels);
	}
} // namespace ModelHandler

