#include "Wheel.h"

void Wheel::toConsole(std::string msg)
{
	std::cout << "Wheel: " << msg;
}

void Wheel::displayWheelAbilities()
{
	toConsole("Haptic Abilities\n");
	toConsole("----------------\n");
	if ((SDL_HapticQuery(haptic) & SDL_HAPTIC_SINE))  toConsole("has sine effect\n");
	if ((SDL_HapticQuery(haptic) & SDL_HAPTIC_CONSTANT)) toConsole("has constant effect\n");
	if ((SDL_HapticQuery(haptic) & SDL_HAPTIC_LEFTRIGHT)) toConsole("has leftright effect\n");
	if ((SDL_HapticQuery(haptic) & SDL_HAPTIC_TRIANGLE)) toConsole("has triangle wave effect\n");
	if ((SDL_HapticQuery(haptic) & SDL_HAPTIC_SAWTOOTHUP)) toConsole("has saw tooth up effect\n");
	if ((SDL_HapticQuery(haptic) & SDL_HAPTIC_SAWTOOTHDOWN)) toConsole("has saw tooth down effect\n");
	if ((SDL_HapticQuery(haptic) & SDL_HAPTIC_RAMP)) toConsole("has ramp effect\n");
	if ((SDL_HapticQuery(haptic) & SDL_HAPTIC_SPRING)) toConsole("has spring effect\n");
	if ((SDL_HapticQuery(haptic) & SDL_HAPTIC_DAMPER)) toConsole("has damper effect\n");
	if ((SDL_HapticQuery(haptic) & SDL_HAPTIC_INERTIA)) toConsole("has inertia effect\n");
	if ((SDL_HapticQuery(haptic) & SDL_HAPTIC_FRICTION)) toConsole("has friction effect\n");
	if ((SDL_HapticQuery(haptic) & SDL_HAPTIC_CUSTOM)) toConsole("has custom effect\n");
	if ((SDL_HapticQuery(haptic) & SDL_HAPTIC_GAIN)) toConsole("has gain effect\n");
	if ((SDL_HapticQuery(haptic) & SDL_HAPTIC_AUTOCENTER)) toConsole("has auto centre effect\n ");
	if ((SDL_HapticQuery(haptic) & SDL_HAPTIC_PAUSE))  toConsole("can be paused\n");
	if ((SDL_HapticQuery(haptic) & SDL_HAPTIC_STATUS))  toConsole("can have its status queried\n");
}

SDL_HapticEffect Wheel::hapticSine()
{
	SDL_HapticEffect effect;
	// Create the effect
	memset(&effect, 0, sizeof(SDL_HapticEffect)); // 0 is safe default
	effect.type = SDL_HAPTIC_SINE;
	effect.periodic.direction.type = SDL_HAPTIC_CARTESIAN; // Polar coordinates
	effect.periodic.direction.dir[0] = 0; // Force comes from south
	effect.periodic.direction.dir[0] = -1; // Force comes from south
	effect.periodic.direction.dir[0] = 0; // Force comes from south
	effect.periodic.period = 100; // 1000 ms
	effect.periodic.magnitude = 32000; // 20000/32767 strength
	effect.periodic.length = 5000; // 5 seconds long
	effect.periodic.attack_length = 50; // Takes 1 second to get max strength
	effect.periodic.fade_length = 50; // Takes 1 second to fade away

	return effect;
}

SDL_HapticEffect Wheel::hapticConstantRight()
{
	SDL_HapticEffect effect;
	// Create the effect
	memset(&effect, 0, sizeof(SDL_HapticEffect)); // 0 is safe default
	effect.type = SDL_HAPTIC_CONSTANT;
	effect.constant.direction.type = SDL_HAPTIC_CARTESIAN; // Polar coordinates
	effect.constant.direction.dir[0] = -1; // Force comes from south
	effect.constant.direction.dir[1] = 0; // Force comes from south
	effect.constant.direction.dir[2] = 0; // Force comes from south
	effect.constant.length = 3000;
	effect.constant.delay = 0;
	effect.constant.level = 20000;
	effect.constant.attack_length = 500;
	effect.constant.attack_level = 5000;
	effect.constant.fade_length = 0;
	effect.constant.fade_level = 0;
	//effect.constant.type = SDL_HAPTIC_CONSTANT;

	return effect;
}

SDL_HapticEffect Wheel::hapticConstantLeft()
{
	SDL_HapticEffect effect;
	// Create the effect
	memset(&effect, 0, sizeof(SDL_HapticEffect)); // 0 is safe default
	effect.type = SDL_HAPTIC_CONSTANT;
	effect.constant.direction.type = SDL_HAPTIC_CARTESIAN; // Polar coordinates
	effect.constant.direction.dir[0] = 1; // Force comes from south
	effect.constant.direction.dir[1] = 0; // Force comes from south
	effect.constant.direction.dir[2] = 0; // Force comes from south
	effect.constant.length = 3000;
	effect.constant.delay = 0;
	effect.constant.level = 20000;
	effect.constant.attack_length = 400;
	effect.constant.attack_level = 5000;
	effect.constant.fade_length = 0;
	effect.constant.fade_level = 0;
	//effect.constant.type = SDL_HAPTIC_CONSTANT;

	return effect;
}

int Wheel::uploadExecuteEffect(SDL_HapticEffect& effect)
{

	// Upload the effect
	int effect_id = SDL_HapticNewEffect(haptic, &effect);

	if (effect_id < 0)
	{
		std::string msg = SDL_GetError();
		toConsole("Error: " + msg + "\n");
	}
	// Test the effect
	effect_id = SDL_HapticRunEffect(haptic, effect_id, 1);
	if (effect_id < 0)
	{
		std::string msg = SDL_GetError();
		toConsole("Error: " + msg + "\n");
	}

	return effect_id;
}

void Wheel::hapticTest()
{

	SDL_HapticEffect effect;
	int effect_id;
	
	wheel = SDL_JoystickOpen(0);
	haptic = SDL_HapticOpenFromJoystick(wheel);

	if (haptic == NULL) return ; // Most likely joystick isn't haptic
	
	// See if it can do sine waves
	if ((SDL_HapticQuery(haptic) & SDL_HAPTIC_SINE) == 0) 
	{
		SDL_HapticClose(haptic); // No sine effect
		return ;
	}
	
	// Create the effect
	memset(&effect, 0, sizeof(SDL_HapticEffect)); // 0 is safe default

	// test 1
	effect = hapticConstantRight();
	toConsole("Trying Constant Force Right...\n");
	effect_id = uploadExecuteEffect(effect);
	if (effect_id == 0) toConsole("OK\n"); else toConsole("FAILED\n");
	SDL_Delay(5000);

	// test 2
	effect = hapticConstantLeft();
	toConsole("Trying Constant Force Left...\n");
	effect_id = uploadExecuteEffect(effect);
	if (effect_id == 0) toConsole("OK\n"); else toConsole("FAILED\n");
	SDL_Delay(5000);

	// test 3
	effect = hapticSine();
	toConsole("Trying Haptic Sine...\n");
	effect_id = uploadExecuteEffect(effect);
	if (effect_id == 0) toConsole("OK\n"); else toConsole("FAILED\n");
	SDL_Delay(5000);
	
	// We destroy the effect, although closing the device also does this
	SDL_HapticDestroyEffect(haptic, effect_id);
	
	// Close the device
	SDL_HapticClose(haptic);
}

void Wheel::init()
{
	//Initialize SDL
	if (SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC) < 0)
	{
		std::string e = SDL_GetError();
		toConsole("SDL could not initialize! SDL Error:\n"  + e + "\n");
		error = true;
	}
	else
	{
		//Check for joysticks
		int num_wheels = SDL_NumJoysticks();
		
		if (num_wheels < 1)
		{
			toConsole("Error: No joysticks / wheels connected!\n");
		}
		else
		{
			// We have wheels
			int devicePointer = 0;

			// test each wheel upto MAX_WHEEL number
			for (int i = 0; i < num_wheels && i < MAX_WHEELS; ++i)
			{
				wheel = SDL_JoystickOpen(i);

				// Open wheel
				if (wheel != NULL)
				{
					// Display info 
					std::string name = SDL_JoystickName(wheel);
					std::string num = std::to_string(SDL_JoystickNumAxes(wheel));
					toConsole("Found <" + name + "> with " + num + " axis [" + std::to_string(i) + "]\n");

					// Haptic wheel?
					haptic = SDL_HapticOpen(i);
					if (haptic == NULL)
					{
						deviceIndex[devicePointer] = NON_HAPTIC;
						toConsole("has no haptic ability\n");
					}
					else
					{
						// Try Auto centre
						deviceIndex[devicePointer] = HAPTIC;
						if ((SDL_HapticQuery(haptic) & SDL_HAPTIC_AUTOCENTER))
						{
							if (SDL_HapticSetAutocenter(haptic, 50) == 0)
							{
								toConsole("wheel was centred with 50\n");
							 }
							else
							{
								toConsole("Error: wheel centreing failed\n");
							}
						}

						// display abilities
						displayWheelAbilities();
					}
				}
				else
				{
					// Wheel could not be opened - ODD? Something was there!
					deviceIndex[devicePointer] = ERROR;
				}

				// Next device
				++devicePointer;

			} // each wheel

		} // we have wheels

	} // end SDL init tests

} // end init