#pragma once
#include "../../import/hlsl-grimoire-sample/MiniEngine/MiniEngine.h"

class Models {
public:
	enum class Chapter {
		k04_01,
		k04_03,
		k05_01,
		k05_02,
		k05_03,
		k05_04,
		k06_01,
		k06_02,
		k06_03,
	};
	void loadModel();
	void handleInput();
	void draw(RenderContext& renderContext);

private:
	void loadModelInternal(Chapter chapter);
	void handleInputInternal(Chapter chapter);
};
