#include "hapticEffects.h"

namespace HE
{
	// Constructor sets effect structure
	hapticEffects::hapticEffects()
	{
		memset(&effect, 0, sizeof(SDL_HapticEffect));
	}

	// Set maximum wheel position
	void hapticEffects::setMaxPosition(int32_t position)
	{
		maxPosition = position;
	}

	// Set minimum wheel position
	void hapticEffects::setMinPosition(int32_t position)
	{
		minPosition = position;
	}

	// Get maximum wheel position
	int32_t hapticEffects::getMaxPosition()
	{
		return minPosition;
	}

	// get minimum wheel position
	int32_t hapticEffects::getMinPosition()
	{
		return minPosition;
	}

	// Find maximum wheel position
	// uses Constant Force in right direction for 4 seconds
	void hapticEffects::findMaxWheelPosition()
	{
		//Event handler
		SDL_Event e;

		int maxPosition = 0;

		// Timer
		std::clock_t start = std::clock();

		while (std::clock() - start < 4000)
		{
			SDL_PollEvent(&e);
			if (e.type == SDL_JOYAXISMOTION)
			{
				if (e.jaxis.value > maxPosition)
				{
					maxPosition = e.jaxis.value;
				}
			}
		}

		std::cout << "Max wheel position: " << maxPosition << std::endl;
		setMaxPosition(maxPosition);
	}


	// Find minimum wheel position
	// uses Constant Force in right direction for 4 seconds
	void hapticEffects::findMinWheelPosition()
	{
		//Event handler
		SDL_Event e;

		int minPosition = 0;

		// Timer
		std::clock_t start = std::clock();

		while (std::clock() - start < 4000)
		{
			SDL_PollEvent(&e);
			if (e.type == SDL_JOYAXISMOTION)
			{
				if (e.jaxis.value < minPosition)
				{
					minPosition = e.jaxis.value;
				}
			}
		}

		std::cout << "Min wheel position: " << minPosition << std::endl;
		setMinPosition(minPosition);
	}
	// Open Haptic Device
	SDL_Haptic* hapticEffects::setHaptic(int device)
	{
		haptic = SDL_HapticOpen(device);
		if (haptic == NULL) std::cout << "Error: (setHaptic)" << std::endl;
		return haptic;
	}

	// Run Haptic Effect
	// Returns effect_id
	// The wheel runs effect
	int hapticEffects::runEffect(int effect, Uint32 iterations)
	{
		if (haptic == NULL)
		{
			std::cout << "Error: (runEffect) haptic not set" << std::endl;
			return -1;
		}

		return SDL_HapticRunEffect(haptic, effectsMap[effect], iterations);
	}

	/* Define SDL_HAPTIC_CONSTANT force
	Returns effect id (negative is and error)
	Checks input parameters for bounds
	Outputs to console errors if found */
	int hapticEffects::setConstantForce(Uint32 mS, Uint16 level, int direction)
	{
		// Duration must be between range
		if (mS < MIN_DURATION || mS > SDL_MAX_UINT32)
		{
			std::cout << "Error: (setConstantForce) duration in mS Out of Bounds" << std::endl;
			return -1;
		}

		// Level must be between range
		if (level < MIN_LEVEL || level > SDL_MAX_UINT16)
		{
			std::cout << "Error: (setConstantForce) level Out of Bounds" << std::endl;
			return -1;
		}

		// Is there a haptic device?
		if (haptic == nullptr)
		{
			std::cout << "Error: (setConstantForce) haptic not set" << std::endl;
			return -1;
		}

		// Destroy current effect if exists
		if (effectsMap[direction == 1 ? CONSTANT_LEFT : CONSTANT_RIGHT] != -1)
		{
			SDL_HapticDestroyEffect(haptic, effectsMap[direction == 1 ? CONSTANT_LEFT : CONSTANT_RIGHT]);
			effectsMap[direction == 1 ? CONSTANT_LEFT : CONSTANT_RIGHT] = -1;
		}

		// Check direction 
		if (direction != LEFT && direction != RIGHT)
		{
			std::cout << "Error: (setConstantForce) direction not set" << std::endl;
			return -1;
		}

		// SDL_HAPTIC_CONSTANT;
		effect.type = SDL_HAPTIC_CONSTANT;
		effect.constant.direction.type = SDL_HAPTIC_CARTESIAN;
		effect.constant.direction.dir[0] = direction == 1 ? LEFT : RIGHT;
		effect.constant.direction.dir[1] = 0;
		effect.constant.direction.dir[2] = 0; // not used - included for completeness
		effect.constant.length = mS;
		effect.constant.delay = 0;
		effect.constant.level = level;
		effect.constant.attack_length = 0;
		effect.constant.attack_level = 0;
		effect.constant.fade_length = 0;
		effect.constant.fade_level = 0;

		// Upload the effect
		int effect_id = SDL_HapticNewEffect(haptic, &effect);

		// error?
		if (effect_id < 0)
		{
			std::string msg = SDL_GetError();
			std::cout << "Error: (setConstantForce) " << msg << std::endl;
			return -1;
		}

		// Update map with effect ID
		effectsMap[direction == 1 ? CONSTANT_LEFT : CONSTANT_RIGHT] = effect_id;

		return effect_id;
	
	} // end setConstantForce



	// Set direction property
	void Profile::sDirection(Sint8 dir)
	{
		direction = (dir == ::HE::LEFT || dir == HE::RIGHT) ? dir : HE::BAD_DIRECTION;
	}

	// Return profile direction
	Sint8 Profile::gDirection()
	{
		return direction;
	}

	// Set this reading's index
	void Profile::sReading(Sint8 read)
	{
		// reading numbers are an index 
		reading = (read < 0) ? HE::BAD_PROFILE_INDEX : read;
	}

	// Return reading index number
	Sint8 Profile::gReading()
	{
		return reading;
	}



} // end namespace HE