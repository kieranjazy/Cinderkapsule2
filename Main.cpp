//#include "vld.h"
#include "Main.h"
#include "VulkanCore.h"
#include "Input.h"
#include <SDL2/SDL_timer.h>

using namespace Cinder;

int main(int argc, char* argv[]) {
	auto vulkanCore = std::make_unique<VulkanCore>();

	// Time Logic //
	bool notN = false;
	bool firstLoop = true;

	const float timestep = 1000 / 144.0f; //TODO Change denominator to refresh rate of display
	int timeLast = 0;
	int timeCurrent = 0;
	float deltaTime = 0.0f;
	float accumulatedTime = 0.0f;
	//            //

	bool active = true; //Set this out here so we can control it from other classes with a pointer if needed
	auto inputCore = std::make_unique<InputCore>(&active);

	while (active) {
		
		// Game Loop Time Logic //
		if (!notN) {
			timeLast = SDL_GetTicks();
			notN = !notN;
			continue;
		} else {
			if (!firstLoop)
				timeLast = timeCurrent;

			timeCurrent = SDL_GetTicks();
			deltaTime = (timeCurrent - timeLast) / 1000.0f;
			accumulatedTime += deltaTime * 1000;
			firstLoop = false; //idk if this needs to be set every time...
		}
		//                      //

		while (accumulatedTime >= timestep) {
			inputCore->tick();
			accumulatedTime -= timestep;
		}
	}

	return 1;
}
