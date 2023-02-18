#pragma once
#include <cstdint>
#include <string>
class Model;

namespace ModelUtil {
	void initModel(const std::string& tkmFilePath, const std::string& fxFilePath, Model* model, void* constantBuffer, size_t constantBufferSize);
} // namespace ModelUtil
