#pragma once
#include <memory>
#include <vector>
#include "MiniEngine.h"

class IModels {
public:
	virtual ~IModels() = 0;
	virtual void createModel() = 0;
	virtual void resetCamera() = 0;
	virtual void handleInput() = 0;
	virtual void draw(RenderContext& renderContext);
	virtual void debugRenderParams() { }
	std::vector<Model*> getModels();

protected:
	std::vector<std::unique_ptr<Model>> m_models;
};

class IModelFactory {
public:
	virtual ~IModelFactory() = 0;
	virtual std::unique_ptr<IModels> create() = 0;
};

