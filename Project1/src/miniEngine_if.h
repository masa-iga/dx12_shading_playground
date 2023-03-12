#pragma once
#include <d3d12.h>
#include <windows.h>
class Camera;

namespace MiniEngineIf {
	enum class StickType {
		kLX,
		kLY,
		kRX,
		kRY,
	};

	enum class Button {
		kUp,		//!<上。
		kDown,		//!<下。
		kLeft,		//!<左。
		kRight,		//!<右。
		kA,			//!<Aボタン。
		kB,			//!<Bボタン。
		kX,			//!<Xボタン。
		kY,			//!<Yボタン。
		kSelect,	//!<セレクトボタン。
		kStart,		//!<スタートボタン。
		kRB1,		//!<RB1ボタン。
		kRB2,		//!<RB2ボタン。
		kRB3,		//!<RB3ボタン。
		kLB1,		//!<LB1ボタン。
		kLB2,		//!<LB2ボタン。
		kLB3,		//!<LB3ボタン。
	};

	void init(ID3D12Device* device, HWND hwnd, UINT frameBufferWidth, UINT frameBufferHeight);
	void end();
	void loadModel();
	ID3D12Resource* getRenderTargetResource();
	void beginFrame();
	void endFrame();
	void clearRenderTarget(ID3D12GraphicsCommandList* commandList);
	void clearDepthRenderTarget(ID3D12GraphicsCommandList* commandList);
	void handleInput();
	void draw(bool renderToOffscreenBuffer = true);
	float getStick(StickType type);
	bool isPress(Button button);
	bool isTrigger(Button button);
	Camera* getCamera2D();
	Camera* getCamera3D();
}
