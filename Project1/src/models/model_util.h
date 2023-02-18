#pragma once
#include <cstdint>
#include <string>
class Model;

namespace ModelUtil {
	std::string getPathFromAssetDir(const std::string& path);
	void initModel(const std::string& tkmFilePath, const std::string& fxFilePath, Model* model, void* constantBuffer, size_t constantBufferSize);
} // namespace ModelUtil
