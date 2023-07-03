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
		k07_02,
		k07_03,
		k10_01,
		k10_02,
		k10_03,
		k10_04,
		k10_05,
		k10_06,
		k10_07,
		k10_08,
		k11_01,
		k11_02,
		k11_03,
		k11_04,
		k11_05,
		k11_06,
		k12_01,
		k12_02,
		k12_03,
		k12_04,
	};
	void loadModel();
	void releaseResource();
	void handleInput();
	void draw(RenderContext& renderContext);

private:
	void loadModelInternal(Chapter chapter);
	void handleInputInternal(Chapter chapter);
};
