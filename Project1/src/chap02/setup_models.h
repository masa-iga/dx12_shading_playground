#pragma once
#include "../../import/hlsl-grimoire-sample/MiniEngine/MiniEngine.h"

class Models {
public:
	enum class Chapter {
		k04_01,
		k04_03,
		k05_01,
		k05_02,
	};
	// TODO: modify interface
	void loadModel(Chapter chapter);
	void handleInput(Chapter chapter);

	void draw(RenderContext& renderContext);

private:
};
