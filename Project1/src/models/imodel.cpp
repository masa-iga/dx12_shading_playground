#include "imodel.h"

IModelFactory::~IModelFactory() { }

IModels::~IModels() { }

void IModels::draw(RenderContext& renderContext)
{
	for (std::unique_ptr<Model>& pModel : m_models)
		pModel->Draw(renderContext);
}

std::vector<Model*> IModels::getModels() {
	std::vector<Model*> ms;

	for (std::unique_ptr<Model>& model : m_models)
		ms.emplace_back(model.get());

	return ms;
}