#include "model_16_04.h"
#include <filesystem>
#include <memory>
#include <random>
#include "imodel.h"
#include "MiniEngine.h"
#include "model_util.h"
#include "../config.h"
#include "../debug_win.h"
#include "../imgui_if.h"
#include "../miniEngine_if.h"
#include "../winmgr_win.h"
#pragma warning(3 : 4189)

class ModelFactory_16_04 : public IModelFactory
{
public:
	~ModelFactory_16_04() { }
	std::unique_ptr<IModels> create(RenderContext& renderContext);
};

class Obserber_16_04 : public WinMgr::Iobserber
{
public: 
	void update(WPARAM wParam, LPARAM lParam) override;
	bool isPaused() const { return m_paused; }

private:
	bool m_paused = false;
};

struct Light
{
	static constexpr size_t kNumDirectionLight = 4;
	static constexpr size_t kNumPointLight = 1000;

	struct alignas(16) DirectionalLight
	{
		Vector3 m_color;
		float m_pad0 = 0.0f;
		Vector3 m_direction;
		float m_pad1 = 0.0f;
	};

	struct alignas(16) PointLight
	{
		Vector3 m_position;
		float m_pad0 = 0.0f;
		Vector3 m_positionInView;
		float m_pad1 = 0.0f;
		Vector3 m_color;
		float m_range = 0.0f;
	};

	std::array<DirectionalLight, kNumDirectionLight> m_directionLights;
	std::array<PointLight, kNumPointLight> m_pointLights;
	Matrix m_mViewProjInv;
	Vector4 m_screenParam;
	Vector3 m_eyePos;
	float m_specRow = 0.0f;
	int32_t m_numPointLight = kNumPointLight;
};

class LightCulling
{
public:
	void Init(RootSignature& rs, Light* light, RenderTarget& depthRenderTarget);
	void Dispatch(RenderContext& renderContext, std::unique_ptr<Light>& light);
	RWStructuredBuffer& GetPointLightNoListInTileUAV() { return m_pointLightNoListInTileUAV; }

private:
	static constexpr int32_t kRtWidth = 1280;
	static constexpr int32_t kRtHeight = 720;
	static constexpr size_t kTileWidth = 16;
	static constexpr size_t kTileHeight = 16;

	struct alignas(16) LightCullingCameraData
	{
		Matrix m_mProj;
		Matrix m_mProjInv;
		Matrix m_mCameraRot;
	};

	size_t computeNumTile(const RenderTarget& depthRenderTarget) const;

	const std::string kFxFileLightCulling = "Sample_16_04/Sample_16_04/Assets/shader/lightCulling.fx";
	std::string getFxFilePathLightCulling() const { return ModelUtil::getPathFromAssetDir(kFxFileLightCulling); }

	RootSignature* m_rootSignature = nullptr;
	Shader m_csLightCulling;
	PipelineState m_lightCullingPipelineState;
	RWStructuredBuffer m_pointLightNoListInTileUAV;
	ConstantBuffer m_cameraParamCB;
	ConstantBuffer m_lightCB;
	DescriptorHeap m_lightCullingDescriptorHeap;
};

class ZPrepass
{
public:
	RenderTarget& GetDepthRenderTarget() { return m_depthRenderTarget; }
	void Init();
	void Draw(RenderContext& renderContext);

private:
	static constexpr size_t kRenderTargetWidth = 1280;
	static constexpr size_t kRenderTargetHeight = 720;
	const std::string kTkmFileTeapot = "Sample_16_04/Sample_16_04/Assets/modelData/teapot.tkm";
	std::string getTkmFilePathTeapot() const { return ModelUtil::getPathFromAssetDir(kTkmFileTeapot); }
	const std::string kTkmFileBg = "Sample_16_04/Sample_16_04/Assets/modelData/bg.tkm";
	std::string getTkmFilePathBg() const { return ModelUtil::getPathFromAssetDir(kTkmFileBg); }
	const std::string kFxFileZPrepass = "./Assets/shader/sample_16_04_zprepass.fx";
	std::string getFxFilePathZPrepass() const { return kFxFileZPrepass; };

	RenderTarget m_depthRenderTarget;
	std::unique_ptr<Model> m_teapotModel = nullptr;
	std::unique_ptr<Model> m_bgModel = nullptr;
};

class Models_16_04 : public IModels
{
public:
	Models_16_04() { }
	~Models_16_04()
	{
		WinMgr::removeObserver(&m_obserber);
	}
	void createModel(RenderContext& renderContext);
	void addObserver();
	void removeObserver();
	void resetCamera();
	void handleInput();
	void draw(RenderContext& renderContext);
	void debugRenderParams();

private:
	static constexpr int32_t kRtWidth = 1280;
	static constexpr int32_t kRtHeight = 720;

	const std::string kTkmFileBg = "Sample_16_04/Sample_16_04/Assets/modelData/bg.tkm";
	std::string getTkmFilePathBg() const { return ModelUtil::getPathFromAssetDir(kTkmFileBg); }
	const std::string kTkmFileTeapot = "Sample_16_04/Sample_16_04/Assets/modelData/teapot.tkm";
	std::string getTkmFilePathTeapot() const { return ModelUtil::getPathFromAssetDir(kTkmFileTeapot); }
	const std::string kFxFileModel = "./Assets/shader/sample_16_04_model.fx";
	std::string getFxFilePathModel() const { return kFxFileModel; };

	Obserber_16_04 m_obserber;
	ZPrepass m_zprepass;
	LightCulling m_lightCulling;
	std::unique_ptr<Model> m_modelTeapot = nullptr;
	std::unique_ptr<Model> m_modelBg = nullptr;
	RootSignature m_rootSignature;
	std::unique_ptr<Light> m_light = nullptr;
};

void LightCulling::Init(RootSignature& rs, Light* light, RenderTarget& depthRenderTarget)
{
	m_rootSignature = &rs;

	{
		const std::string fxFilePathLightCulling = getFxFilePathLightCulling();
		Dbg::assert_(std::filesystem::exists(fxFilePathLightCulling));

		m_csLightCulling.LoadCS(fxFilePathLightCulling.c_str(), "CSMain");

		const D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc =
		{
			.pRootSignature = rs.Get(),
			.CS = CD3DX12_SHADER_BYTECODE(m_csLightCulling.GetCompiledBlob()),
			.NodeMask = 0,
			.CachedPSO = nullptr,
			.Flags = D3D12_PIPELINE_STATE_FLAG_NONE,
		};

		m_lightCullingPipelineState.Init(psoDesc);
	}

	{
		m_pointLightNoListInTileUAV.Init(
			sizeof(int32_t),
			static_cast<int32_t>(Light::kNumPointLight * computeNumTile(depthRenderTarget)),
			nullptr);
	}

	{
		struct alignas(16) LightCullingCameraData
		{
			Matrix mProj;
			Matrix mProjInv;
			Matrix mCameraRot;
			Vector4 screenParam;
		};

		LightCullingCameraData lightCullingCameraData;
		{
			lightCullingCameraData.mProj = MiniEngineIf::getCamera3D()->GetProjectionMatrix();
			lightCullingCameraData.mProjInv.Inverse(MiniEngineIf::getCamera3D()->GetProjectionMatrix());
			lightCullingCameraData.mCameraRot = MiniEngineIf::getCamera3D()->GetCameraRotation();
			lightCullingCameraData.screenParam.x = MiniEngineIf::getCamera3D()->GetNear();
			lightCullingCameraData.screenParam.y = MiniEngineIf::getCamera3D()->GetFar();
			lightCullingCameraData.screenParam.z = kRtWidth;
			lightCullingCameraData.screenParam.w = kRtHeight;
		}

		m_cameraParamCB.Init(sizeof(lightCullingCameraData), &lightCullingCameraData);
		m_lightCB.Init(sizeof(*light), light);

		m_lightCullingDescriptorHeap.RegistShaderResource(0, depthRenderTarget.GetRenderTargetTexture());
		m_lightCullingDescriptorHeap.RegistUnorderAccessResource(0, m_pointLightNoListInTileUAV);
		m_lightCullingDescriptorHeap.RegistConstantBuffer(0, m_cameraParamCB);
		m_lightCullingDescriptorHeap.RegistConstantBuffer(1, m_lightCB);
		m_lightCullingDescriptorHeap.Commit();
	}

}

void LightCulling::Dispatch(RenderContext& renderContext, std::unique_ptr<Light>& light)
{
	renderContext.SetComputeRootSignature(*m_rootSignature);
	m_lightCB.CopyToVRAM(light.get());
	renderContext.SetComputeDescriptorHeap(m_lightCullingDescriptorHeap);
	renderContext.SetPipelineState(m_lightCullingPipelineState);

	constexpr UINT tgx_count = kRtWidth / kTileWidth;
	constexpr UINT tgy_count = kRtHeight / kTileHeight;

	renderContext.Dispatch(
		tgx_count,
		tgy_count,
		1);
}

size_t LightCulling::computeNumTile(const RenderTarget& depthRenderTarget) const
{
	const int32_t rt_width = depthRenderTarget.GetWidth();
	const int32_t rt_height = depthRenderTarget.GetHeight();
	return ((rt_width + kTileWidth - 1) / kTileWidth) * ((rt_height + kTileHeight - 1) / kTileHeight);
}

void ZPrepass::Init()
{
	constexpr int32_t mip_level = 1;
	constexpr int32_t array_size = 1;

	m_depthRenderTarget.Create(
		kRenderTargetWidth,
		kRenderTargetHeight,
		mip_level,
		array_size,
		DXGI_FORMAT_R32_FLOAT,
		DXGI_FORMAT_D32_FLOAT
	);

	const std::string fxFilePathZPrepass = getFxFilePathZPrepass();
	Dbg::assert_(std::filesystem::exists(fxFilePathZPrepass));
	{
		const std::string tkmFilePathTeapot = getTkmFilePathTeapot();
		Dbg::assert_(std::filesystem::exists(tkmFilePathTeapot));

		ModelInitData d;
		{
			d.m_tkmFilePath = tkmFilePathTeapot.c_str();
			d.m_fxFilePath = fxFilePathZPrepass.c_str();
			d.m_colorBufferFormat.at(0) = DXGI_FORMAT_R32_FLOAT;
		}

		m_teapotModel = std::make_unique<Model>();
		m_teapotModel->Init(d);
	}

	{
		const std::string tkmFilePathBg = getTkmFilePathBg();
		Dbg::assert_(std::filesystem::exists(tkmFilePathBg));

		ModelInitData d;
		{
			d.m_tkmFilePath = tkmFilePathBg.c_str();
			d.m_fxFilePath = fxFilePathZPrepass.c_str();
			d.m_colorBufferFormat.at(0) = DXGI_FORMAT_R32_FLOAT;
		}

		m_bgModel = std::make_unique<Model>();
		m_bgModel->Init(d);
	}
}

void ZPrepass::Draw(RenderContext& renderContext)
{
	RenderTarget* rts[] = { &m_depthRenderTarget };

	renderContext.WaitUntilToPossibleSetRenderTargets(1, rts);

	renderContext.SetRenderTargets(1, rts);

	{
		CD3DX12_VIEWPORT vp(0.0f, 0.0f, static_cast<float>(m_depthRenderTarget.GetWidth()), static_cast<float>(m_depthRenderTarget.GetHeight()));
		renderContext.SetViewportAndScissor(vp);
	}

	renderContext.ClearRenderTargetViews(1, rts);

	m_teapotModel->Draw(renderContext);
	m_bgModel->Draw(renderContext);

	renderContext.WaitUntilFinishDrawingToRenderTargets(1, rts);
}

std::unique_ptr<IModels> ModelFactory_16_04::create(RenderContext& renderContext)
{
	std::unique_ptr<Models_16_04> m = std::make_unique<Models_16_04>();
	{
		m->createModel(renderContext);
		m->addObserver();
	}
	return std::move(m);
}

void Obserber_16_04::update(WPARAM wParam, [[maybe_unused]] LPARAM lParam)
{
	switch (wParam) {
	case VK_SPACE:
		m_paused = !m_paused;
		break;
	default:
		break;
	}
}

void Models_16_04::resetCamera()
{
	MiniEngineIf::getCamera3D()->SetPosition({ 0.0f, 200.0f, 400.0f });
	MiniEngineIf::getCamera3D()->Update();
}

void Models_16_04::createModel([[maybe_unused]] RenderContext& renderContext)
{
	resetCamera();

	{
		bool bRet = m_rootSignature.Init(
			D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP);
		Dbg::assert_(bRet);
	}

	m_light = std::make_unique<Light>();
	{
		m_light->m_screenParam = { MiniEngineIf::getCamera3D()->GetNear(), MiniEngineIf::getCamera3D()->GetFar(), kRtWidth, kRtHeight };
		m_light->m_eyePos = MiniEngineIf::getCamera3D()->GetPosition();
		m_light->m_specRow = 5.0f;
		m_light->m_mViewProjInv.Inverse(MiniEngineIf::getCamera3D()->GetViewProjectionMatrix());
	}
	{
		m_light->m_directionLights.at(0).m_direction.Set(1.0f, 0.0f, 0.0f);
		m_light->m_directionLights.at(0).m_color.Set(0.5f, 0.5f, 0.5f);
		m_light->m_directionLights.at(1).m_direction.Set(-1.0f, 0.0f, 0.0f);
		m_light->m_directionLights.at(1).m_color.Set(0.5f, 0.0f, 0.0f);
		m_light->m_directionLights.at(2).m_direction.Set(0.0f, 0.0f, 1.0f);
		m_light->m_directionLights.at(2).m_color.Set(0.0f, 0.5f, 0.0f);
		m_light->m_directionLights.at(3).m_direction.Set(0.0f, -1.0f, 0.0f);
		m_light->m_directionLights.at(3).m_color.Set(0.0f, 0.0f, 0.5f);
	}
	{
		std::random_device seed_gen;
		std::mt19937 random(seed_gen());

		for (auto& pt : m_light->m_pointLights)
		{
			pt.m_position = {
				static_cast<float>(random() % 1000) - 500.0f,
				20.0f,
				static_cast<float>(random() % 1000) - 500.0f
			};
			pt.m_positionInView = pt.m_position;
			{
				const Matrix& mView = MiniEngineIf::getCamera3D()->GetViewMatrix();
				mView.Apply(pt.m_positionInView);
			}
			pt.m_range = 50.0f;
			pt.m_color = {
				static_cast<float>(random() % 255) / 255.0f,
				static_cast<float>(random() % 255) / 255.0f,
				static_cast<float>(random() % 255) / 255.0f
			};
		}
	}

	m_zprepass.Init();
	m_lightCulling.Init(m_rootSignature, m_light.get(), m_zprepass.GetDepthRenderTarget());

	{
		const std::string fxFilePathModel = getFxFilePathModel();
		Dbg::assert_(std::filesystem::exists(fxFilePathModel));

		{
			const std::string tkmFilePathTeapot = getTkmFilePathTeapot();
			Dbg::assert_(std::filesystem::exists(tkmFilePathTeapot));

			ModelInitData d = { };
			{
				d.m_tkmFilePath = tkmFilePathTeapot.c_str();
				d.m_fxFilePath = fxFilePathModel.c_str();
				d.m_expandConstantBuffer = m_light.get();
				d.m_expandConstantBufferSize = sizeof(*m_light.get());
				d.m_expandShaderResoruceView.at(0) = &m_lightCulling.GetPointLightNoListInTileUAV();
			}
			m_modelTeapot = std::make_unique<Model>();
			m_modelTeapot->Init(d);
		}

		{
			const std::string tkmFilePathBg = getTkmFilePathBg();
			Dbg::assert_(std::filesystem::exists(tkmFilePathBg));

			ModelInitData d = { };
			{
				d.m_tkmFilePath = tkmFilePathBg.c_str();
				d.m_fxFilePath = fxFilePathModel.c_str();
				d.m_expandConstantBuffer = m_light.get();
				d.m_expandConstantBufferSize = sizeof(*m_light.get());
				d.m_expandShaderResoruceView.at(0) = &m_lightCulling.GetPointLightNoListInTileUAV();
			}
			m_modelBg = std::make_unique<Model>();
			m_modelBg->Init(d);
		}
	}
}

void Models_16_04::addObserver()
{
	WinMgr::addObserver(&m_obserber);
}

void Models_16_04::removeObserver()
{
	WinMgr::removeObserver(&m_obserber);
}

void Models_16_04::handleInput()
{
	if (WinMgr::isWindowActive(WinMgr::Handle::kMain))
	{
		using namespace MiniEngineIf;
		constexpr float kAffect = 5.0f;
		getCamera3D()->MoveForward(getStick(StickType::kLY) * kAffect);
		getCamera3D()->MoveRight(getStick(StickType::kLX) * kAffect);
		getCamera3D()->MoveUp(getStick(StickType::kRY) * kAffect);
		getCamera3D()->Update();
		m_light->m_eyePos = getCamera3D()->GetPosition();
		m_light->m_mViewProjInv.Inverse(getCamera3D()->GetViewProjectionMatrix());
	}

	if (!m_obserber.isPaused())
	{
		Quaternion qRot;
		qRot.SetRotationDegY(1.0f);

		for (auto& pt : m_light->m_pointLights)
		{
			qRot.Apply(pt.m_position);
			pt.m_positionInView = pt.m_position;
			{
				const Matrix& mView = MiniEngineIf::getCamera3D()->GetViewMatrix();
				mView.Apply(pt.m_positionInView);
			}
		}
	}
}

void Models_16_04::draw(RenderContext& renderContext)
{
	// clear render target
	{
		MiniEngineIf::setOffscreenRenderTarget();
		float clearColor[] = { 0.5f, 0.5f, 0.5f, 1.0f };
		renderContext.ClearRenderTargetView(MiniEngineIf::getOffscreenRtvCpuDescHandle(), clearColor);
	}

	// Z prepass
	m_zprepass.Draw(renderContext);

	// light culling
	m_lightCulling.Dispatch(renderContext, m_light);

	// sync
	renderContext.TransitionResourceState(
		m_lightCulling.GetPointLightNoListInTileUAV().GetD3DResoruce(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);

	// forward rendering
	MiniEngineIf::setOffscreenRenderTarget();

	m_modelTeapot->Draw(renderContext);
	m_modelBg->Draw(renderContext);

	// sync
	renderContext.TransitionResourceState(
		m_lightCulling.GetPointLightNoListInTileUAV().GetD3DResoruce(),
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS
	);
}

void Models_16_04::debugRenderParams()
{
	{
		const Vector3& v = MiniEngineIf::getCamera3D()->GetPosition();
		ImguiIf::printParams<float>(ImguiIf::VarType::kFloat, "CameraPos", std::vector<const float*>{ &v.x, & v.y, & v.z });
	}
	{
		const Vector3& v = MiniEngineIf::getCamera3D()->GetTarget();
		ImguiIf::printParams<float>(ImguiIf::VarType::kFloat, "CameraTgt", std::vector<const float*>{ &v.x, & v.y, & v.z });
	}
	{
		const Vector3& v = MiniEngineIf::getCamera3D()->GetUp();
		ImguiIf::printParams<float>(ImguiIf::VarType::kFloat, "CameraUp", std::vector<const float*>{ &v.x, & v.y, & v.z });
	}
}

namespace ModelHandler {
	std::unique_ptr<IModels> loadModelForChap16_04(RenderContext& renderContext)
	{
		ModelFactory_16_04 factory;
		std::unique_ptr<IModels> iModels = factory.create(renderContext);
		iModels->resetCamera();
		iModels->debugRenderParams();
		return std::move(iModels);
	}
} // namespace ModelHandler

