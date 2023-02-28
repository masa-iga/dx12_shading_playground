#include "model_util.h"
#include <filesystem>
#include "MiniEngine.h"
#include "../debug_win.h"

namespace ModelUtil {
	std::string getPathFromAssetDir(const std::string& path)
	{
		const std::string kBaseAssetDir = "../import/hlsl-grimoire-sample";
		return kBaseAssetDir + "/" + path;
	}

	std::wstring getPathFromAssetDirWstr(const std::wstring& path)
	{
		const std::wstring kBaseAssetDir = L"../import/hlsl-grimoire-sample";
		return kBaseAssetDir + L"/" + path;
	}

	void initModel(const std::string& tkmFilePath, const std::string& fxFilePath, Model* model, void* constantBuffer, size_t constantBufferSize)
	{
		initModel(tkmFilePath, fxFilePath, model, constantBuffer, constantBufferSize, std::vector<IShaderResource*>());
	}

	void initModel(const std::string& tkmFilePath, const std::string& fxFilePath, Model* model, void* constantBuffer, size_t constantBufferSize, std::vector<IShaderResource*> shaderResources)
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

		for (int32_t i = 0; i < shaderResources.size(); ++i)
		{
			if (i == initData.m_expandShaderResoruceView.size())
				break;

			initData.m_expandShaderResoruceView.at(i) = shaderResources.at(i);
		}

		model->Init(initData);
	}

} // namespace ModelUtil