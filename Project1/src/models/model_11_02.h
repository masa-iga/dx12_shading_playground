#pragma once
#include <memory>
#include <vector>
#include "imodel.h"
class Model;

namespace ModelHandler {
	std::unique_ptr<IModels> loadModelForChap11_02();
} // namespace ModelHandler
