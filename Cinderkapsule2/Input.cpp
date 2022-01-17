#include "Input.h"
#include "SDL2/SDL.h"

namespace Cinder {
	struct InputCore::impl {
		const float speed = 20.0f;
		const float mouseSensitivity = 35.0f;

		bool* activePtr = nullptr;

		impl(bool* active) : activePtr(active) {}

		~impl() {}

		const void update() {
			SDL_Event e;
			SDL_PumpEvents();
			const Uint8* state = SDL_GetKeyboardState(NULL);

			if (state[SDL_SCANCODE_ESCAPE])
				*activePtr = !(*activePtr);

			while (SDL_PollEvent(&e)) {
				switch (e.type) {
				case SDL_QUIT:
					*activePtr = !(*activePtr);
					break;
				case SDL_WINDOWEVENT&& SDL_WINDOWEVENT_RESIZED:
					//vulkan->framebufferResizedSwitchPublic();
					break;
				case SDL_MOUSEMOTION:

					break;
				}
			}
		}

	};

	InputCore::InputCore(bool* activePtr) : pImpl(std::make_unique<impl>(activePtr)) {}
	InputCore::~InputCore() {}
}

namespace Cinder {
	const void InputCore::tick() {
		pImpl->update();
	}
}