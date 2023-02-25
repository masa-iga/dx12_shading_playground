#pragma once
#include <cstdint>
#include <string>
#include <vector>
class Model;
class IShaderResource;

namespace ModelUtil {
	std::string getPathFromAssetDir(const std::string& path);
	std::wstring getPathFromAssetDirWstr(const std::wstring& path);
	void initModel(const std::string& tkmFilePath, const std::string& fxFilePath, Model* model, void* constantBuffer, size_t constantBufferSize);
	void initModel(const std::string& tkmFilePath, const std::string& fxFilePath, Model* model, void* constantBuffer, size_t constantBufferSize, std::vector<IShaderResource*> shaderResources);
} // namespace ModelUtil
