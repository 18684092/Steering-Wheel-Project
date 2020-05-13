#pragma once
#include <map>
#include <SDL.h>
#include <string>
#include <iostream>
#include <climits>

namespace HE
{
	// Define NS constants
	constexpr auto CONSTANT_LEFT = 0;
	constexpr auto CONSTANT_RIGHT = 1;
	constexpr auto LEFT = 1;
	constexpr auto RIGHT = -1;
	constexpr auto MIN_DURATION = 10;
	constexpr auto MIN_LEVEL = 0;

	// Define Object
	class hapticEffects
	{
	private:
		SDL_HapticEffect effect;
		SDL_Haptic* haptic = nullptr;
		std::map<int, int> effectsMap = { {CONSTANT_LEFT, -1}, {CONSTANT_RIGHT, -1} };

	public:
		int setConstantForce(Uint32 mS, Uint16 level, int direction);
		SDL_Haptic* setHaptic(int device);
		int runEffect(int effect, Uint32 iterations);


	public:
		hapticEffects();
	};

}

