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
		kUp,		//!<��B
		kDown,		//!<���B
		kLeft,		//!<���B
		kRight,		//!<�E�B
		kA,			//!<A�{�^���B
		kB,			//!<B�{�^���B
		kX,			//!<X�{�^���B
		kY,			//!<Y�{�^���B
		kSelect,	//!<�Z���N�g�{�^���B
		kStart,		//!<�X�^�[�g�{�^���B
		kRB1,		//!<RB1�{�^���B
		kRB2,		//!<RB2�{�^���B
		kRB3,		//!<RB3�{�^���B
		kLB1,		//!<LB1�{�^���B
		kLB2,		//!<LB2�{�^���B
		kLB3,		//!<LB3�{�^���B
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
