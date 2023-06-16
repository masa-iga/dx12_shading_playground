#include "model_12_02.h"
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

class ModelFactory_12_02 : public IModelFactory
{
public:
	~ModelFactory_12_02() { }
	std::unique_ptr<IModels> create();
};

class Models_12_02 : public IModels
{
public:
	Models_12_02() { }
	~Models_12_02() { }
	void createModel();
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
	};

	static constexpr size_t kGbufferWidth = 1920;
	static constexpr size_t kGbufferHeight = 1080;
	static constexpr DXGI_FORMAT kGbufferColorFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	const std::string kTkmSampleFile = "Sample_12_02/Sample_12_02/Assets/modelData/sample.tkm";
	std::string getTkmSampleFilePath() { return ModelUtil::getPathFromAssetDir(kTkmSampleFile); }
	const std::string kFxModelFile = "./Assets/shader/sample_12_02_model.fx";
	std::string getFxModelFilePath() { return kFxModelFile; }
	const std::string kFxSpriteFile = "./Assets/shader/sample_12_02_sprite.fx";
	std::string getFxSpriteFilePath() { return kFxSpriteFile; }
	const std::string kFxPresetSpriteFile = "Sample_11_06/Sample_11_06/Assets/shader/preset/sprite.fx";
	std::string getFxPresetSpriteFilePath() { return ModelUtil::getPathFromAssetDir(kFxPresetSpriteFile); }

	DirectionalLight m_light;
	std::unique_ptr<Model> m_model = nullptr;
	RenderTarget m_albedRt;
	RenderTarget m_normalRt;
	std::unique_ptr<Sprite> m_defferedLightingSprite = nullptr;
	std::array<std::unique_ptr<Sprite>, 2> m_debugSprites = { nullptr, nullptr };
};

std::unique_ptr<IModels> ModelFactory_12_02::create()
{
	std::unique_ptr<Models_12_02> m = std::make_unique<Models_12_02>();
	{
		m->createModel();
	}
	return std::move(m);
}

void Models_12_02::resetCamera()
{
	;
}

void Models_12_02::createModel()
{
	// create Gbuffer
	{
		bool bRet = m_albedRt.Create(
			kGbufferWidth,
			kGbufferHeight,
			1 /* mipLevel */,
			1 /* arraySize */,
			kGbufferColorFormat,
			DXGI_FORMAT_D32_FLOAT);
		Dbg::assert_(bRet);
	}
	{
		bool bRet = m_normalRt.Create(
			kGbufferWidth,
			kGbufferHeight,
			1 /* mipLevel */,
			1 /* arraySize */,
			kGbufferColorFormat,
			DXGI_FORMAT_UNKNOWN);
		Dbg::assert_(bRet);
	}

	{
		m_light.m_direction = { 1.0f, 0.0f, 0.0f };
		m_light.m_color = { 1.0f, 1.0f, 1.0f };
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

		m_model = std::make_unique<Model>();
		m_model->Init(d);
	}
	{
		SpriteInitData d;
		d.m_width = kGbufferWidth;
		d.m_height = kGbufferHeight;
		d.m_textures.at(0) = &m_albedRt.GetRenderTargetTexture();
		d.m_textures.at(1) = &m_normalRt.GetRenderTargetTexture();
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
	}
}

void Models_12_02::handleInput()
{
	if (!WinMgr::isWindowActive(WinMgr::Handle::kMain))
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
	}
}

void Models_12_02::draw(RenderContext& renderContext)
{
	// rotate directional light
	{
		Quaternion rotLig;
		rotLig.SetRotationDegY(2.0f);
		rotLig.Apply(m_light.m_direction);
	}

	// render to Gbuffer
	{
		RenderTarget* rts[] = {
			&m_albedRt,
			&m_normalRt,
		};

		renderContext.WaitUntilToPossibleSetRenderTargets(2, rts);

		renderContext.SetRenderTargets(2, rts);
		renderContext.ClearRenderTargetViews(2, rts);

		m_model->Draw(renderContext);

		renderContext.WaitUntilFinishDrawingToRenderTargets(2, rts);
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

void Models_12_02::debugRenderParams()
{
#if 0
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
			const Vector3& v = m_lightCamera.GetPosition();
			ImguiIf::printParams<float>(ImguiIf::VarType::kFloat, "LightCameraPos", std::vector<const float*>{ &v.x, &v.y, &v.z });
		}
		{
			const Vector3& v = m_lightCamera.GetTarget();
			ImguiIf::printParams<float>(ImguiIf::VarType::kFloat, "LightCameraTgt", std::vector<const float*>{ &v.x, &v.y, &v.z });
		}
		{
			const Vector3& v = m_lightCamera.GetUp();
			ImguiIf::printParams<float>(ImguiIf::VarType::kFloat, "LightCameraUp", std::vector<const float*>{ &v.x, &v.y, &v.z });
		}
	}
#endif
}

namespace ModelHandler {
	std::unique_ptr<IModels> loadModelForChap12_02()
	{
		ModelFactory_12_02 factory;
		std::unique_ptr<IModels> iModels = factory.create();
		iModels->resetCamera();
		iModels->debugRenderParams();
		return std::move(iModels);
	}
} // namespace ModelHandler

