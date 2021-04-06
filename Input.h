#pragma once
#include <memory>

namespace Cinder {
	class InputCore {
	public:
		InputCore(bool* activePtr); //For now this will take pointer to game loop bool, will remove when engine passes stage of needing it
		~InputCore();
		const void tick();
	private:
		struct impl;
		const std::unique_ptr<impl> pImpl;
	};
}

