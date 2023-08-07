#include "fps.h"
#include <cstdint>
#include <windows.h>
#include "debug_win.h"

#pragma comment(lib, "Winmm.lib")

namespace {
	float s_fps = 0.0f;
}

namespace Fps {
	void compute()
	{
		static uint32_t s_frameCount = 0;
		static float s_prevTime = 0.0f;
		static float s_elapsedTime = 0.0f;

		const float currentTime = static_cast<float>(timeGetTime()) * 0.001f; // milli seconds
		const float deltaTime = currentTime - s_prevTime;

		s_prevTime = currentTime;
		s_elapsedTime += deltaTime;
		s_frameCount++;

		if (s_elapsedTime >= 1.0f)
		{
			s_fps = static_cast<float>(s_frameCount) / s_elapsedTime;
			s_frameCount = 0;
			s_elapsedTime = 0;
		}
	}

	float* getFpsRef()
	{
		return &s_fps;
	}
} // namespace Fps