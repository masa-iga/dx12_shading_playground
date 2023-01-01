#include "miniEngine_if.h"
#include <filesystem>
#include "../../import/hlsl-grimoire-sample/MiniEngine/MiniEngine.h"
#include "debug_win.h"

#pragma comment(lib, "hlsl-grimoire-sample_miniEngine.lib")

static const std::string kBaseAssetPath = "../../import/hlsl-grimoire-sample/Sample_04_01/Sample_04_01";
static const std::string kTkmPath = "Assets/modelData/sample.tkm";
static const std::string kFxPath = "Assets/shader/sample.fx";

constexpr std::string getFullPath(const std::string path)
{
	return kBaseAssetPath + "/" + path;
}

namespace MiniEngineIf {
	void init(HWND hwnd, UINT frameBufferWidth, UINT frameBufferHeight)
	{
		g_engine = new TkEngine;
		Dbg::assert_(g_engine != nullptr);
		g_engine->Init(hwnd, frameBufferWidth, frameBufferHeight);
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
		const std::string tkmFilePath = getFullPath(kTkmPath);
		const std::string fxFilePath = getFullPath(kFxPath);
		Dbg::assert_(std::filesystem::exists(tkmFilePath));
		Dbg::assert_(std::filesystem::exists(fxFilePath));

		ModelInitData initData = { };
		initData.m_tkmFilePath = tkmFilePath.c_str();
		initData.m_fxFilePath = fxFilePath.c_str();

		Model charaModel;
		charaModel.Init(initData);
	}
}
