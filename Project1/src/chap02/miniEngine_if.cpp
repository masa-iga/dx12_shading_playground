#include "miniEngine_if.h"
#include <filesystem>
#include "../../import/hlsl-grimoire-sample/MiniEngine/MiniEngine.h"
#include "debug_win.h"

#pragma comment(lib, "hlsl-grimoire-sample_miniEngine.lib")

#define LOAD_MODEL_CHAP_04_01 (0)
#define LOAD_MODEL_CHAP_04_03 (0)
#define LOAD_MODEL_CHAP_05_01 (0)
#define LOAD_MODEL_CHAP_05_02 (1)

namespace {
	void createRenderTarget(ID3D12Device* device);
	void createDepthRenderTarget(ID3D12Device* device);
	void loadModelForChap04_01();
	void loadModelForChap04_03();
	void loadModelForChap05_01();
	void loadModelForChap05_02();
	void initModel(const std::string& tkmFilePath, const std::string& fxFilePath, Model* model, void* constantBuffer, size_t constantBufferSize);
	void handleInputInternal();
	void drawInternal(bool renderToOffscreenBuffer);
	constexpr std::string getPathFromAssetDir(const std::string path);

	const std::string kBaseAssetDir = "../../import/hlsl-grimoire-sample";
	constexpr DXGI_FORMAT kRenderTargetFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	constexpr UINT kRenderTargetWidth = 1920;
	constexpr UINT kRenderTargetHeight = 1080;
	constexpr float kRenderTargetClearColor[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	constexpr FLOAT kDepthClearVal = 1.0f;
	constexpr UINT kStencilClearVal = 0;

	std::vector<Model*> s_models;

	ComPtr<ID3D12Resource> s_renderTarget = nullptr;
	ComPtr<ID3D12DescriptorHeap> s_descHeapRt = nullptr;
	ComPtr<ID3D12Resource> s_depthRenderTarget = nullptr;
	ComPtr<ID3D12DescriptorHeap> s_descHeapDrt = nullptr;
}

namespace MiniEngineIf {
	void init(ID3D12Device* device, HWND hwnd, UINT frameBufferWidth, UINT frameBufferHeight)
	{
		g_engine = new TkEngine;
		Dbg::assert_(g_engine != nullptr);
		g_engine->Init(hwnd, frameBufferWidth, frameBufferHeight);

		createRenderTarget(device);
		createDepthRenderTarget(device);
	}

	void end()
	{
		if (g_engine == nullptr)
			return;

		delete g_engine;
		g_engine = nullptr;
	}

	void loadModel()
	{
#if LOAD_MODEL_CHAP_04_01
		loadModelForChap04_01();
#endif // #if LOAD_MODEL_CHAP_04_01
#if LOAD_MODEL_CHAP_04_03
		loadModelForChap04_03();
#endif // #if LOAD_MODEL_CHAP_04_03
#if LOAD_MODEL_CHAP_05_01
		loadModelForChap05_01();
#endif // #if LOAD_MODEL_CHAP_05_01
#if LOAD_MODEL_CHAP_05_02
		loadModelForChap05_02();
#endif // #if LOAD_MODEL_CHAP_05_02
	}

	ID3D12Resource* getRenderTargetResource()
	{
		return s_renderTarget.Get();
	}

	void beginFrame()
	{
		g_engine->BeginFrame();
	}

	void endFrame()
	{
		g_engine->EndFrame();
	}

	void clearRenderTarget(ID3D12GraphicsCommandList* commandList)
	{
		commandList->ClearRenderTargetView(s_descHeapRt->GetCPUDescriptorHandleForHeapStart(), kRenderTargetClearColor, 0, nullptr);
	}

	void clearDepthRenderTarget(ID3D12GraphicsCommandList* commandList)
	{
		commandList->ClearDepthStencilView(
			s_descHeapDrt->GetCPUDescriptorHandleForHeapStart(),
            D3D12_CLEAR_FLAG_DEPTH,
            kDepthClearVal,
            kStencilClearVal,
            0,
            nullptr);
	}

	void handleInput()
	{
		handleInputInternal();
	}

	void draw(bool renderToOffscreenBuffer)
	{
		drawInternal(renderToOffscreenBuffer);
	}
}

namespace {
	void createRenderTarget(ID3D12Device* device)
	{
		{
			const CD3DX12_HEAP_PROPERTIES heapProp(D3D12_HEAP_TYPE_DEFAULT);
			auto resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(kRenderTargetFormat, kRenderTargetWidth, kRenderTargetHeight);
			resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			const D3D12_CLEAR_VALUE clearVal = {
				.Format = kRenderTargetFormat,
				.Color = { kRenderTargetClearColor[0],  kRenderTargetClearColor[1], kRenderTargetClearColor[2], kRenderTargetClearColor[3]},
			};

			Dbg::ThrowIfFailed(device->CreateCommittedResource(
				&heapProp,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				&clearVal,
				IID_PPV_ARGS(s_renderTarget.ReleaseAndGetAddressOf())));
			Dbg::ThrowIfFailed(s_renderTarget->SetName(L"MiniEngine_RenderTarget"));
		}

		{
			const D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {
				.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
				.NumDescriptors = 1,
				.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
				.NodeMask = 0,
			};

			Dbg::ThrowIfFailed(device->CreateDescriptorHeap(
				&heapDesc,
				IID_PPV_ARGS(s_descHeapRt.ReleaseAndGetAddressOf())));
			Dbg::ThrowIfFailed(s_descHeapRt->SetName(L"MiniEngine_DescHeapRenderTarget"));

			const D3D12_RENDER_TARGET_VIEW_DESC rtViewDesc = {
				.Format = kRenderTargetFormat,
				.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D,
				.Texture2D = { 0, 0 },
			};
			const auto handle = s_descHeapRt->GetCPUDescriptorHandleForHeapStart();

			device->CreateRenderTargetView(
				s_renderTarget.Get(),
				&rtViewDesc,
				handle);
		}
	}

	void createDepthRenderTarget(ID3D12Device* device)
	{
		{
			const CD3DX12_HEAP_PROPERTIES heapProp(D3D12_HEAP_TYPE_DEFAULT);
			auto resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, kRenderTargetWidth, kRenderTargetHeight);
			resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
			const D3D12_CLEAR_VALUE clearVal = {
				.Format = resourceDesc.Format,
				.DepthStencil = { kDepthClearVal, kStencilClearVal },
			};

			Dbg::ThrowIfFailed(device->CreateCommittedResource(
				&heapProp,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_DEPTH_WRITE,
				&clearVal,
				IID_PPV_ARGS(s_depthRenderTarget.ReleaseAndGetAddressOf())));
			Dbg::ThrowIfFailed(s_depthRenderTarget->SetName(L"MiniEngine_DepthRenderTarget"));
		}

		{
			const D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {
				.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
				.NumDescriptors = 1,
				.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
				.NodeMask = 0,
			};

			Dbg::ThrowIfFailed(device->CreateDescriptorHeap(
				&heapDesc,
				IID_PPV_ARGS(s_descHeapDrt.ReleaseAndGetAddressOf())));
			Dbg::ThrowIfFailed(s_descHeapDrt->SetName(L"MiniEngine_DescHeapDepthRenderTarget"));

			const D3D12_DEPTH_STENCIL_VIEW_DESC viewDesc = {
				.Format = s_depthRenderTarget->GetDesc().Format,
				.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D,
				.Flags = D3D12_DSV_FLAG_NONE,
				.Texture2D = { 0 },
			};
			const auto handle = s_descHeapDrt->GetCPUDescriptorHandleForHeapStart();

			device->CreateDepthStencilView(
				s_depthRenderTarget.Get(),
				&viewDesc,
				handle);
		}
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

	Vector3* s_ptLightPosition = nullptr;
	Model* s_lightModel = nullptr;

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
		s_ptLightPosition = &s_light.ptPosition;

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
			s_lightModel = &s_model;
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
		s_ptLightPosition = &s_light.ptPosition;

		{
			const std::string tkmFile = "Sample_05_02/Sample_05_02/Assets/modelData/teapot.tkm";
			const std::string fxFile = "Assets/shader/sample_05_02.fx";
			const std::string tkmFilePath = getPathFromAssetDir(tkmFile);
			const std::string fxFilePath = fxFile;
			static Model s_model;
			initModel(tkmFilePath, fxFilePath, &s_model, &s_light, sizeof(s_light));
			s_model.UpdateWorldMatrix({ 0.0f, 20.0f, 0.0f }, g_quatIdentity, g_vec3One);
		}

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
			s_lightModel = &s_model;
		}
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

	void handleInputInternal()
	{
#if LOAD_MODEL_CHAP_05_01
		if (s_ptLightPosition)
		{
			s_ptLightPosition->x -= g_pad[0]->GetLStickXF();

			if (g_pad[0]->IsPress(enButtonB))
			{
				s_ptLightPosition->y += g_pad[0]->GetLStickYF();
			}
			else
			{
				s_ptLightPosition->z -= g_pad[0]->GetLStickYF();
			}
		}

		if (s_lightModel && s_ptLightPosition)
		{
			s_lightModel->UpdateWorldMatrix(*s_ptLightPosition, g_quatIdentity, g_vec3One);
		}
#endif // #if LOAD_MODEL_CHAP_05_01
	}

	void drawInternal(bool renderToOffscreenBuffer)
	{
		auto& renderContext = g_graphicsEngine->GetRenderContext();

		if (renderToOffscreenBuffer)
		{
			renderContext.SetRenderTarget(s_descHeapRt->GetCPUDescriptorHandleForHeapStart(), s_descHeapDrt->GetCPUDescriptorHandleForHeapStart());

			CD3DX12_VIEWPORT vp(s_renderTarget.Get());
			renderContext.SetViewportAndScissor(vp);
		}

		for (auto model : s_models)
		{
			model->Draw(renderContext);
		}
	}

	constexpr std::string getPathFromAssetDir(const std::string path)
	{
		return kBaseAssetDir + "/" + path;
	}
}

