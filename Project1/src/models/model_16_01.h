#pragma once
#include <memory>
#include <vector>
#include "imodel.h"
class Model;

namespace ModelHandler {
	std::unique_ptr<IModels> loadModelForChap16_01(RenderContext& renderContext);
} // namespace ModelHandler
